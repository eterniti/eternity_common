#include "TmcFile.h"
#include "debug.h"

TmcFile::TmcFile()
{
    this->big_endian = false;
}

TmcFile::~TmcFile()
{

}

void TmcFile::Reset()
{
    bones.clear();
    nodes.clear();
    matrices.clear();
}

bool TmcFile::LoadNode(const uint8_t *buf, TmcNodeObj &node)
{
    const TMCHeader *hdr = (const TMCHeader *)buf;

    node.unk_30 = *(uint32_t *)buf + 0x30;
    node.master = *(uint32_t *)buf + 0x34;
    node.index = *(uint32_t *)buf + 0x38;
    node.unk_3C = *(uint32_t *)buf + 0x3C;

    size_t str_length;

    if (hdr->offset1 == 0)
    {
        str_length = hdr->chunk_size - 0x40;
    }
    else
    {
        str_length = hdr->offset1 - 0x40;
    }

    for (size_t i = 0; i < str_length; i++)
    {
        char ch = (char) buf[0x40+i];
        if (!ch)
            break;

        node.name.push_back(ch);
    }

    if (hdr->offset1 > 0)
    {
        node.has_blend_data = true;

        uint32_t offset = *(const uint32_t *)(buf + hdr->offset1);
        node.obj_index = *(const uint32_t *)(buf + offset);

        uint32_t count = *(const uint32_t *)(buf + offset + 4);
        node.blend_indices.resize(count);

        node.matrix = *(const TMCMatrix *)(buf + offset + 0x10);
        const uint32_t *indices = (const uint32_t *)(buf + offset + 0x50);

        for (size_t i = 0; i < node.blend_indices.size(); i++)
            node.blend_indices[i] = indices[i];

        //DPRINTF("Loaded node %s  Obj index: %d Child count: %Id\n", node.name.c_str(), node.obj_index, node.blend_indices.size());
    }
    else
    {
        node.has_blend_data = false;
    }

    return true;
}

bool TmcFile::LoadBone(const uint8_t *buf, TmcBone &bone)
{
    bone.matrix = *(const TMCMatrix *)buf;
    bone.parent = *(const uint32_t *)(buf + 0x40);
    bone.bone_level = *(const uint32_t *)(buf + 0x48);

    bone.children.resize(*(const uint32_t *)(buf + 0x44));
    const uint32_t *children = (const uint32_t *)(buf + 0x50);

    for (size_t i = 0; i < bone.children.size(); i++)
        bone.children[i] = children[i];

    return true;
}

bool TmcFile::LoadHieLayer(const uint8_t *buf)
{
    const TMCHeader *hdr = (const TMCHeader *)buf;
    if (hdr->count1 == 0)
        return true;

    bones.resize(hdr->count1);
    const uint32_t *offsets = (const uint32_t *)GetOffsetPtr(buf, hdr->offset1);

    for (size_t i = 0; i < bones.size(); i++)
    {
        if (!LoadBone(buf + offsets[i], bones[i]))
            return false;
    }

    return true;
}

bool TmcFile::LoadNodeLayer(const uint8_t *buf)
{
    const TMCHeader *hdr = (const TMCHeader *)buf;
    if (hdr->count1 == 0)
        return true;

    nodes.resize(hdr->count1);
    const uint32_t *offsets = (const uint32_t *)GetOffsetPtr(buf, hdr->offset1);

    for (size_t i = 0; i < nodes.size(); i++)
    {
        if (!LoadNode(buf + offsets[i], nodes[i]))
            return false;
    }

    return true;
}

bool TmcFile::LoadMatrices(const uint8_t *buf)
{
    const TMCHeader *hdr = (const TMCHeader *)buf;
    if (hdr->count1 == 0)
        return true;

    matrices.resize(hdr->count1);
    const uint32_t *offsets = (const uint32_t *)GetOffsetPtr(buf, hdr->offset1);

    for (size_t i = 0; i < matrices.size(); i++)
    {
        const TMCMatrix *matrix = (const TMCMatrix *)GetOffsetPtr(buf, offsets, (uint32_t)i);
        matrices[i] = *matrix;
    }

    return true;
}

D3DXMATRIX TmcFile::GetGlobalTransform(uint32_t idx) const
{
    const TmcBone &bone = bones[idx];
    D3DXMATRIX m = D3DXMATRIX(bone.matrix.matrix);

    if (bone.parent == 0xFFFFFFFF)
        return m;

    return m * GetGlobalTransform(bone.parent);
}

bool TmcFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(TMCHeader))
        return false;

    const TMCHeader *hdr = (const TMCHeader *)buf;
    if (hdr->signature != TMC_SIGNATURE)
        return false;

    const uint32_t *offsets = (const uint32_t *)GetOffsetPtr(buf, hdr->offset1);

    if (hdr->count1 > 6 && !LoadHieLayer(buf + offsets[6]))
        return false;

    if (hdr->count1 > 8 && !LoadNodeLayer(buf + offsets[8]))
        return false;

    if (hdr->count1 > 9 && !LoadMatrices(buf + offsets[9]))
        return false;

    return true;
}

void TmcFile::DebugBones()
{
    for (size_t i = 0; i < bones.size(); i++)
    {
        //const TmcBone &bone = bones[i];
        const TmcNodeObj &node = nodes[i];

        D3DXMATRIX m = GetGlobalTransform(i);
        D3DXVECTOR3 s, t;
        D3DXQUATERNION r;

        D3DXMatrixDecompose(&s, &r, &t, &m);
        DPRINTF("%s %f %f %f\n", node.name.c_str(), t.x, t.y, t.z);
    }
}

