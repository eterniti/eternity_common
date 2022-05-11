#include "Utils.h"
#include "EanFile.h"
#include "debug.h"

EanFile::EanFile()
{
    this->big_endian = false;
}

EanFile::~EanFile()
{

}

void EanFile::Reset()
{
    data.clear();
}

bool EanFile::Load(const uint8_t *buf, size_t size)
{
    if (!buf || size < sizeof(EANHeader))
        return false;

    if (*(uint32_t *)buf != EAN_SIGNATURE)
        return false;

    data.resize(size);
    memcpy(data.data(), buf, size);

    return true;
}

uint8_t *EanFile::Save(size_t *psize)
{
    uint8_t *buf = new uint8_t[data.size()];
    memcpy(buf, data.data(), data.size());

    *psize = data.size();
    return buf;
}

EANAnimHeader *EanFile::GetAnimHeader(uint16_t idx)
{
    EANHeader *hdr = (EANHeader *)data.data();

    if (idx >= hdr->anim_count)
        return nullptr;

    uint32_t *table = (uint32_t *)GetOffsetPtr(hdr, hdr->anim_keyframes_offset);
    return (EANAnimHeader *)GetOffsetPtr(hdr, table, idx);
}

EANAnimNode *EanFile::GetAnimNode(EANAnimHeader *ahdr, uint16_t idx)
{
    if (idx >= ahdr->nodes_count)
        return nullptr;

    uint32_t *table = (uint32_t *)GetOffsetPtr(ahdr, ahdr->nodes_offset);
    return (EANAnimNode *)GetOffsetPtr(ahdr, table, idx);
}

EANKeyframedAnim *EanFile::GetKeyFrameAnim(EANAnimNode *node, uint16_t idx)
{
    if (idx >= node->keyframed_animation_count)
        return nullptr;

    uint32_t *table = (uint32_t *)GetOffsetPtr(node, node->keyframed_animation_offset);
    return (EANKeyframedAnim *)GetOffsetPtr(node, table, idx);
}

bool EanFile::ScaleAnim32(uint16_t idx, float x, float y, float z)
{
    EANAnimHeader *ahdr = GetAnimHeader(idx);
    if (!ahdr)
        return false;

    if (ahdr->frame_index_size != 0 || ahdr->frame_float_size != 2)
    {
        DPRINTF("%s: FIXME, not supported, frame_index_size=%d, frame_float_size=%d\n", FUNCNAME, ahdr->frame_index_size, ahdr->frame_float_size);
        return false;
    }

    for (uint16_t i = 0; i < ahdr->nodes_count; i++)
    {
        EANAnimNode *node = GetAnimNode(ahdr, i);
        if (!node)
            return false;

        for (uint16_t j = 0; j < node->keyframed_animation_count; j++)
        {
            EANKeyframedAnim *kfa = GetKeyFrameAnim(node, j);
            if (!kfa)
                return false;

            EANCoords32 *c32 = (EANCoords32 *)GetOffsetPtr(kfa, kfa->keyframes_offset);

            /*uint8_t *buf = data.data();
            DPRINTF("%x  %f  %f  %f\n", Utils::DifPointer(c16, buf), x, y, z);*/

            for (uint16_t k = 0; k < kfa->keyframe_count; k++)
            {
                float fx = c32[k].x;
                float fy = c32[k].y;
                float fz = c32[k].z;

                /*fx *= x;
                fy *= y;
                fz *= z;*/
                fx += x;
                fy += y;
                fz += z;

                c32[k].x = fx;
                c32[k].y = fy;
                c32[k].z = fz;
            }
        }
    }

    return true;
}

bool EanFile::ScaleAnim(uint16_t idx, float x, float y, float z)
{
    EANAnimHeader *ahdr = GetAnimHeader(idx);
    if (!ahdr)
        return false;

    if (ahdr->frame_index_size != 0 || ahdr->frame_float_size != 1)
    {
        if (ahdr->frame_float_size == 2)
        {
            return ScaleAnim32(idx, x, y, z);
        }

        DPRINTF("%s: FIXME, not supported, frame_index_size=%d, frame_float_size=%d\n", FUNCNAME, ahdr->frame_index_size, ahdr->frame_float_size);
        return false;
    }

    for (uint16_t i = 0; i < ahdr->nodes_count; i++)
    {
        EANAnimNode *node = GetAnimNode(ahdr, i);
        if (!node)
            return false;

        for (uint16_t j = 0; j < node->keyframed_animation_count; j++)
        {
            EANKeyframedAnim *kfa = GetKeyFrameAnim(node, j);
            if (!kfa)
                return false;

            EANCoords16 *c16 = (EANCoords16 *)GetOffsetPtr(kfa, kfa->keyframes_offset);

            /*uint8_t *buf = data.data();
            DPRINTF("%x  %f  %f  %f\n", Utils::DifPointer(c16, buf), x, y, z);*/

            for (uint16_t k = 0; k < kfa->keyframe_count; k++)
            {
                float fx = Utils::HalfToFloat(c16[k].x);
                float fy = Utils::HalfToFloat(c16[k].y);
                float fz = Utils::HalfToFloat(c16[k].z);

                fx *= x;
                fy *= y;
                fz *= z;

                c16[k].x = Utils::FloatToHalf(fx);
                c16[k].y = Utils::FloatToHalf(fy);
                c16[k].z = Utils::FloatToHalf(fz);
            }
        }
    }

    return true;
}
