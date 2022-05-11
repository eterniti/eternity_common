#ifndef TMCFILE_H
#define TMCFILE_H

#include "BaseFile.h"

#define TMC_SIGNATURE 0x434D54

#include <d3dx9.h>

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED TMCHeader
{
    uint64_t signature; // 0
    uint32_t flags1; // 8
    uint32_t header_size; // 0x0C
    uint32_t chunk_size; // 0x10
    uint32_t count1; // 0x14
    uint32_t count2; // 0x18
    uint32_t count3; // 0x1C
    uint32_t offset1; // 0x20
    uint32_t offset2; // 0x24
    uint32_t offset3; // 0x28
    uint32_t flags2; // 0x2C;
};
CHECK_STRUCT_SIZE(TMCHeader, 0x30);

struct TMCMatrix
{
    float matrix[16];
};
CHECK_STRUCT_SIZE(TMCMatrix, 0x40);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct TmcNodeObj
{
    uint32_t unk_30;
    uint32_t master;
    uint32_t index;
    uint32_t unk_3C;

    std::string name;
    bool has_blend_data;

    // Extra data
    uint32_t obj_index;
    TMCMatrix matrix;
    std::vector<uint32_t> blend_indices;
};

struct TmcBone
{
    TMCMatrix matrix;
    uint32_t parent;
    uint32_t bone_level;

    std::vector<uint32_t> children;
};

class TmcFile : public BaseFile
{
protected:

    std::vector<TmcBone> bones;
    std::vector<TmcNodeObj> nodes;
    std::vector<TMCMatrix> matrices;

    void Reset();

    bool LoadNode(const uint8_t *buf, TmcNodeObj &node);
    bool LoadBone(const uint8_t *buf, TmcBone &bone);

    bool LoadHieLayer(const uint8_t *buf);
    bool LoadNodeLayer(const uint8_t *buf);
    bool LoadMatrices(const uint8_t *buf);

    D3DXMATRIX GetGlobalTransform(uint32_t idx) const;

public:

    TmcFile();
    virtual ~TmcFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    //virtual uint8_t *Save(size_t *psize) override;

    void DebugBones();
};

#endif // TMCFILE_H
