#include "EskFile.h"
#include "TransformMatrix.h"
#include "debug.h"

#include <math.h>

EskFile::EskFile()
{
    this->big_endian = false;
}

EskFile::~EskFile()
{

}

void EskFile::Reset()
{
    data.clear();
}

bool EskFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(ESKHeader))
        return false;

    if (*(uint32_t *)buf != ESK_SIGNATURE)
        return false;

    data.resize(size);
    memcpy(data.data(), buf, size);

    return true;
}

uint8_t *EskFile::Save(size_t *psize)
{
    uint8_t *buf = new uint8_t[data.size()];
    memcpy(buf, data.data(), data.size());

    *psize = data.size();
    return buf;
}

uint16_t EskFile::FindBoneByName(const std::string &name) const
{
    const ESKHeader *hdr = (const ESKHeader *)data.data();
    const ESKHeader2 *hdr2 = (const ESKHeader2 *)(data.data() + hdr->header2_start);

    const uint32_t *names_table = (const uint32_t *)GetOffsetPtr(hdr2, hdr2->bone_names_offset);

    for (uint16_t i = 0; i < hdr2->num_bones; i++)
    {
        const char *bone_name = (const char *)GetOffsetPtr(hdr2, names_table, i);
        if (name == bone_name)
            return i;
    }

    return 0xFFFF;
}

uint16_t EskFile::GetParent(uint16_t bone) const
{
    const ESKHeader *hdr = (const ESKHeader *)data.data();
    const ESKHeader2 *hdr2 = (const ESKHeader2 *)(data.data() + hdr->header2_start);

    if (bone >= hdr2->num_bones)
        return 0xFFFF;

    ESKBone *bones_indices = (ESKBone *)GetOffsetPtr(hdr2, hdr2->bone_indices_offset);
    return bones_indices[bone].parent;
}

size_t EskFile::FindChildren(uint16_t bone, std::vector<uint16_t> &children)
{
    children.clear();

    ESKHeader *hdr = (ESKHeader *)data.data();
    ESKHeader2 *hdr2 = (ESKHeader2 *)(data.data() + hdr->header2_start);

    if (bone >= hdr2->num_bones)
        return 0;

    ESKBone *bones_indices = (ESKBone *)GetOffsetPtr(hdr2, hdr2->bone_indices_offset);

    for (uint16_t i = 0; i < hdr2->num_bones; i++)
    {
        if (bones_indices[i].parent == bone)
            children.push_back(i);
    }

    return children.size();
}

bool EskFile::GetBoneScale(uint16_t bone, float *x, float *y, float *z)
{
    ESKHeader *hdr = (ESKHeader *)data.data();
    ESKHeader2 *hdr2 = (ESKHeader2 *)(data.data() + hdr->header2_start);

    if (bone >= hdr2->num_bones)
        return false;

    ESKSkinningMatrix *skinning_matrices = (ESKSkinningMatrix *)GetOffsetPtr(hdr2, hdr2->skinning_matrix_offset);

    *x = skinning_matrices[bone].scale[0];
    *y = skinning_matrices[bone].scale[1];
    *z = skinning_matrices[bone].scale[2];

    return true;
}

bool EskFile::GetBoneScale(const std::string &name, float *x, float *y, float *z)
{
    uint16_t bone = FindBoneByName(name);
    if (bone == 0xFFFF)
        return false;

    return GetBoneScale(bone, x, y, z);
}

bool EskFile::ChangeBoneScale(uint16_t bone, float x, float y, float z)
{
    ESKHeader *hdr = (ESKHeader *)data.data();
    ESKHeader2 *hdr2 = (ESKHeader2 *)(data.data() + hdr->header2_start);

    if (bone >= hdr2->num_bones)
        return false;

    ESKSkinningMatrix *skinning_matrices = (ESKSkinningMatrix *)GetOffsetPtr(hdr2, hdr2->skinning_matrix_offset);

    skinning_matrices[bone].scale[0] *= x;
    skinning_matrices[bone].scale[1] *= y;
    skinning_matrices[bone].scale[2] *= z;

    /*skinning_matrices[bone].position[0] *= x;
    skinning_matrices[bone].position[1] *= y;
    skinning_matrices[bone].position[2] *= z;

    skinning_matrices[bone].orientation[0] *= x;
    skinning_matrices[bone].orientation[1] *= y;
    skinning_matrices[bone].orientation[2] *= z;*/

    return true;
}

bool EskFile::ChangeBoneScale(const std::string &name, float x, float y, float z)
{
    uint16_t bone = FindBoneByName(name);
    if (bone == 0xFFFF)
        return false;

    return ChangeBoneScale(bone, x, y, z);
}

void EskFile::FixTransforms(uint16_t i)
{
    ESKHeader *hdr = (ESKHeader *)data.data();
    ESKHeader2 *hdr2 = (ESKHeader2 *)(data.data() + hdr->header2_start);

    ESKTransformMatrix *transform_matrices = (ESKTransformMatrix *)GetOffsetPtr(hdr2, hdr2->transform_matrix_offset);
    ESKSkinningMatrix *skinning_matrices = (ESKSkinningMatrix *)GetOffsetPtr(hdr2, hdr2->skinning_matrix_offset);
    //ESKBone *bones_indices = (ESKBone *)GetOffsetPtr(hdr2, hdr2->bone_indices_offset);

    const uint32_t *names_table = (const uint32_t *)GetOffsetPtr(hdr2, hdr2->bone_names_offset);

    TransformMatrix t, r, s, transform, m, c;
    uint16_t parent = GetParent(i);

    if (parent != 0xFFFF)
    {
        if (parent >= hdr2->num_bones)
        {
            DPRINTF("%s: Internal error.\n", FUNCNAME);
            exit(-1);
        }

        m = TransformMatrix(transform_matrices[parent].matrix);
    }
    else
    {

    }

    std::string name = (const char *)GetOffsetPtr(hdr2, names_table, i);
    std::string parent_name;
    if (parent == 0xFFFF)
        parent_name ="NONE";
    else
        parent_name = (const char *)GetOffsetPtr(hdr2, names_table, parent);

    /*if (name.find("Head") != std::string::npos)
        return;*/


    if (fabs(skinning_matrices[i].scale[1] - 1) < 0.00001)
    {
        /*if (name.find("Eye") == std::string::npos && name.find("Face") == std::string::npos &&
            name.find("ArmorParts") == std::string::npos && name.find("g_x_LND") == std::string::npos &&
            name[0] != 'g' && name[0] != 'f' && name[0] != 'h' && name[0] != 'x' && name != "b_C_Spine2" && name != "b_C_Pelvis")*/
        //if (name[0] == 'x' || name[0] == 'h' || name[0] == 'g' || name == "b_R_ArmorParts" || name == "b_L_ArmorParts")
        //if (name.find("ArmorParts") != std::string::npos)
        /*if (name == "g_C_Pelvis")
        {
            printf("Skipping %s %f %f %f (with parent %s)\n", name.c_str(), skinning_matrices[i].scale[0], skinning_matrices[i].scale[1], skinning_matrices[i].scale[2], parent_name.c_str());
            goto process_children;
        }*/
    }


    printf("Fixing matrix for bone: %s  %f %f %f\n", (char *)GetOffsetPtr(hdr2, names_table, i), skinning_matrices[i].scale[0], skinning_matrices[i].scale[1], skinning_matrices[i].scale[2]);

    c = TransformMatrix(transform_matrices[i].matrix);
    //c.Print();

    t = TransformMatrix::FromTranslation(-skinning_matrices[i].position[0], -skinning_matrices[i].position[1], -skinning_matrices[i].position[2]);
    r = TransformMatrix::FromRotation(-skinning_matrices[i].orientation[0], -skinning_matrices[i].orientation[1], -skinning_matrices[i].orientation[2], skinning_matrices[i].orientation[3]);
    s = TransformMatrix::FromScaling(skinning_matrices[i].scale[0], skinning_matrices[i].scale[1], skinning_matrices[i].scale[2]);

    transform = t*r*s;
    m = m*transform;
    //m.Print();

    m.CopyTo(transform_matrices[i].matrix);

//process_children:

    std::vector<uint16_t> children;
    FindChildren(i, children);

    for (uint16_t bone : children)
        FixTransforms(bone);
}

void EskFile::FixTransforms(const std::string &name)
{
    uint16_t bone = FindBoneByName(name);
    if (bone == 0xFFFF)
        return;

    FixTransforms(bone);
}

void EskFile::FixTransforms2(uint16_t i)
{
    ESKHeader *hdr = (ESKHeader *)data.data();
    ESKHeader2 *hdr2 = (ESKHeader2 *)(data.data() + hdr->header2_start);

    ESKTransformMatrix *transform_matrices = (ESKTransformMatrix *)GetOffsetPtr(hdr2, hdr2->transform_matrix_offset);
    ESKSkinningMatrix *skinning_matrices = (ESKSkinningMatrix *)GetOffsetPtr(hdr2, hdr2->skinning_matrix_offset);

    TransformMatrix t, r, s, transform, m, c, rel;
    uint16_t parent = GetParent(i);

    if (parent != 0xFFFF)
    {
        if (parent >= hdr2->num_bones)
        {
            DPRINTF("%s: Internal error.\n", FUNCNAME);
            exit(-1);
        }

        m = TransformMatrix(transform_matrices[parent].matrix);
    }

    c = TransformMatrix(transform_matrices[i].matrix);

    rel = m.Inverse() * c;

    t = rel.GetTranslation();
    r = rel.GetRotation();
    s = TransformMatrix::FromScaling(skinning_matrices[i].scale[0], skinning_matrices[i].scale[1], skinning_matrices[i].scale[2]);

    transform = t*r*s;
    m = m*transform;

    m.CopyTo(transform_matrices[i].matrix);

    //skinning_matrices[i].scale[0] = skinning_matrices[i].scale[1] = skinning_matrices[i].scale[2] = 1.0f;

    std::vector<uint16_t> children;
    FindChildren(i, children);

    for (uint16_t bone : children)
        FixTransforms(bone);
}

void EskFile::Debug()
{
    const ESKHeader *hdr = (const ESKHeader *)data.data();
    const ESKHeader2 *hdr2 = (const ESKHeader2 *)(data.data() + hdr->header2_start);
    const ESKBone *bones_indices = (ESKBone *)GetOffsetPtr(hdr2, hdr2->bone_indices_offset);
    const uint32_t *names_table = (const uint32_t *)GetOffsetPtr(hdr2, hdr2->bone_names_offset);
    const ESKTransformMatrix *transform_matrices = (const ESKTransformMatrix *)GetOffsetPtr(hdr2, hdr2->transform_matrix_offset);
    const ESKSkinningMatrix *skinning_matrices = (const ESKSkinningMatrix *)GetOffsetPtr(hdr2, hdr2->skinning_matrix_offset);

    for (uint16_t bone = 0; bone < hdr2->num_bones; bone++)
    {
        std::string name, parent_name;
        uint16_t parent = bones_indices[bone].parent;

        name = (const char *)GetOffsetPtr(hdr2, names_table, bone);
        if (parent != 0xFFFF)
            parent_name = (const char *)GetOffsetPtr(hdr2, names_table, parent);
        else
            parent_name = "NONE";

        DPRINTF("---------------Bone: %s  Parent = %s\n", name.c_str(), parent_name.c_str());

        TransformMatrix tmc(transform_matrices[bone].matrix);

        float tx, ty, tz;
        float sx, sy, sz;
        float rx, ry, rz, rw;

        tmc.GetTranslation(&tx, &ty, &tz);
        tmc.GetScale(&sx, &sy, &sz);
        tmc.GetRotation(&rx, &ry, &rz, &rw);

        DPRINTF("[TRASNFORM MATRIX]\n");
        DPRINTF("Translation: %f  %f %f\n", tx, ty, tz);
        DPRINTF("Rotation: %f %f %f %f\n", rx, ry, rz, rw);
        DPRINTF("Scale: %f %f %f\n", sx, sy, sz);

        if (parent != 0xFFFF)
        {
            TransformMatrix tmp(transform_matrices[parent].matrix);
            TransformMatrix m = tmp.Inverse() * tmc;

            m.GetTranslation(&tx, &ty, &tz);
            m.GetScale(&sx, &sy, &sz);
            m.GetRotation(&rx, &ry, &rz, &rw);

            DPRINTF("[TRASNFORM MATRIX - RELATIVE TO PARENT]\n");
            DPRINTF("Translation: %f  %f %f\n", tx, ty, tz);
            DPRINTF("Rotation: %f %f %f %f\n", rx, ry, rz, rw);
            DPRINTF("Scale: %f %f %f\n", sx, sy, sz);
        }

        DPRINTF("[SKINNING MATRIX]\n");
        DPRINTF("Translation: %f %f %f\n", skinning_matrices[bone].position[0], skinning_matrices[bone].position[1], skinning_matrices[bone].position[2]);

        if (skinning_matrices[bone].position[3] != 1.0f)
        {
            DPRINTF("Warning, w not 1 in position.\n");
        }

        DPRINTF("Rotation: %f %f %f %f\n", skinning_matrices[bone].orientation[0], skinning_matrices[bone].orientation[1], skinning_matrices[bone].orientation[2], skinning_matrices[bone].orientation[3]);
        DPRINTF("Scale: %f %f %f\n", skinning_matrices[bone].scale[0], skinning_matrices[bone].scale[1], skinning_matrices[bone].scale[2]);

        if (skinning_matrices[bone].scale[3] != 1.0f)
        {
            DPRINTF("Warning, w not 1 in scale.\n");
        }
    }
}
