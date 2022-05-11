#ifndef EANFILE_H
#define EANFILE_H

// Note: this is a simple low level parser. It can't do fancy things like adding new data, its only purpose is to quickly modify existing data

#include "BaseFile.h"

#define EAN_SIGNATURE   0x4E414523

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t header_size; // 6
    uint16_t unk_08;
    uint16_t unk_0A;
    uint32_t unk_0C;
    uint16_t unk_10;
    uint16_t anim_count; // 0x12
    uint32_t skeleton_offset; // 0x14
    uint32_t anim_keyframes_offset; // 0x18
    uint32_t anim_names_offset; // 0x1C
} PACKED EANHeader;

STATIC_ASSERT_STRUCT(EANHeader, 0x20);

typedef struct
{
    uint16_t unk_00;
    uint8_t frame_index_size; // 2
    uint8_t frame_float_size; // 3
    uint32_t frame_count; // 4
    uint32_t nodes_count; // 8
    uint32_t nodes_offset; // 0xC
} PACKED EANAnimHeader;

STATIC_ASSERT_STRUCT(EANAnimHeader, 0x10);

typedef struct
{
    uint16_t bone_index; // 0
    uint16_t keyframed_animation_count; // 2
    uint32_t keyframed_animation_offset; // 4
} PACKED EANAnimNode;

STATIC_ASSERT_STRUCT(EANAnimNode, 8);

typedef struct
{
    uint32_t flag; // 0
    uint32_t keyframe_count; // 4
    uint32_t indices_offset; // 8
    uint32_t keyframes_offset; // 0xC
} PACKED EANKeyframedAnim;

STATIC_ASSERT_STRUCT(EANKeyframedAnim, 0x10);

typedef struct
{
    uint16_t x;
    uint16_t y;
    uint16_t z;
    uint16_t w;
} PACKED EANCoords16;

STATIC_ASSERT_STRUCT(EANCoords16, 8);

typedef struct
{
    float x;
    float y;
    float z;
    float w;
} PACKED EANCoords32;

STATIC_ASSERT_STRUCT(EANCoords32, 16);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

class EanFile : public BaseFile
{
private:

    std::vector<uint8_t> data;

protected:

    void Reset();

    bool ScaleAnim32(uint16_t idx, float x, float y, float z);

public:

    EanFile();
    virtual ~EanFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    EANAnimHeader *GetAnimHeader(uint16_t idx);
    EANAnimNode *GetAnimNode(EANAnimHeader *ahdr, uint16_t idx);
    EANKeyframedAnim *GetKeyFrameAnim(EANAnimNode *node, uint16_t idx);

    bool ScaleAnim(uint16_t idx, float x, float y, float z);
};

#endif // EANFILE_H
