#ifndef __VPMFILE_H__
#define __VPMFILE_H__

#include "BaseFile.h"

#define VPM_SIGNATURE   "#VPM"

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    char signature[4]; // 0
    uint32_t unk_04; // 4
    uint32_t num_entries; // 8
    uint32_t data_start; // 0xC
} PACKED VPMHeader;

static_assert(sizeof(VPMHeader) == 0x10, "Incorrect structure size.");

typedef struct
{
    char name[16];   // 0
    float unk_10[12]; // 0x10
    uint16_t unk_40[10]; // 0x40
    uint32_t unk_54[5]; // 0x54
    float unk_68[6]; // 0x68
} PACKED VPMEntry;

static_assert(sizeof(VPMEntry) == 0x80, "Incorrect structure size.");

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct VpmEntry
{
    std::string name;
    float unk_10[12];
    uint16_t unk_40[10];
    uint32_t unk_54[5];
    float unk_68[6];

    void Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

class VpmFile : public BaseFile
{
private:

    std::vector <VpmEntry> entries;

    void Reset();

public:

    VpmFile();
    virtual ~VpmFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;
};

#endif
