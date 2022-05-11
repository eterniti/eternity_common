#ifndef __SPMFILE_H__
#define __SPMFILE_H__

#include "BaseFile.h"

#define SPM_SIGNATURE   "#SPM"

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    char signature[4]; // 0
    char name[4]; // 4
    uint32_t num_entries; // 8
    uint32_t data_start; // 0xC
} PACKED SPMHeader;

static_assert(sizeof(SPMHeader) == 0x10, "Incorrect structure size.");

typedef struct
{
    char name[16];   // 0
    uint16_t unk_10[8]; // 0x10
    float unk_20[128]; // 0x20
    uint16_t unk_220[8]; // 0x220
    float unk_230[32]; // 0x230
    uint64_t unk_2B0[12]; // 0x2B0 check for zero
} PACKED SPMEntry;

static_assert(sizeof(SPMEntry) == 0x310, "Incorrect structure size.");

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct SpmEntry
{
    std::string name;

    uint16_t unk_10[8];
    float unk_20[128];
    uint16_t unk_220[8];
    float unk_230[32];

    void Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

class SpmFile : public BaseFile
{
private:

    std::string name;
    std::vector<SpmEntry> entries;

    void Reset();

public:

    SpmFile();
    virtual ~SpmFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;
};

#endif
