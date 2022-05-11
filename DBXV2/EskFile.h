#ifndef ESKFILE_H
#define ESKFILE_H

// Note: this is a simple low level parser. It can't do fancy things like adding new data, its only purpose is to quickly modify existing data

#include "BaseFile.h"

#define ESK_SIGNATURE   0x4B534523

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
    uint32_t header2_start; // 0x10
    uint32_t unk_14[2]; // 0x14
} PACKED ESKHeader;

STATIC_ASSERT_STRUCT(ESKHeader, 0x1C);

typedef struct
{
    uint16_t num_bones; // 0
    uint16_t flags; // 2
    uint32_t bone_indices_offset; // 4
    uint32_t bone_names_offset; // 8
    uint32_t skinning_matrix_offset; // 0xC
    uint32_t transform_matrix_offset;// 0x10
    uint32_t unk_offsets[4]; // 0x14
} PACKED ESKHeader2;

STATIC_ASSERT_STRUCT(ESKHeader2, 0x24);

typedef struct
{
    uint16_t parent;
    uint16_t child;
    uint16_t sibling;
    uint16_t index4;
} PACKED ESKBone;

STATIC_ASSERT_STRUCT(ESKBone, 8);

typedef struct
{
    float position[4];
    float orientation[4];
    float scale[4];
} PACKED ESKSkinningMatrix;

STATIC_ASSERT_STRUCT(ESKSkinningMatrix, 0x30);

typedef struct
{
    float matrix[16];
} PACKED ESKTransformMatrix;

STATIC_ASSERT_STRUCT(ESKTransformMatrix, 0x40);

#ifdef _MSC_VER
#pragma pack(pop)
#endif


class EskFile : public BaseFile
{
private:

    std::vector<uint8_t> data;

    size_t FindChildren(uint16_t bone, std::vector<uint16_t> &children);

protected:

    void Reset();

public:

    EskFile();
    virtual ~EskFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    uint16_t FindBoneByName(const std::string &name) const;
    uint16_t GetParent(uint16_t bone) const;

    bool GetBoneScale(uint16_t bone, float *x, float *y, float *z);
    bool GetBoneScale(const std::string &name, float *x, float *y, float *z);

    bool ChangeBoneScale(uint16_t bone, float x, float y, float z);
    bool ChangeBoneScale(const std::string &name, float x, float y, float z);

    void FixTransforms(uint16_t i);
    void FixTransforms(const std::string &name);

    void FixTransforms2(uint16_t i);

    void Debug();
};

#endif // ESKFILE_H
