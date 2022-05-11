#ifndef __VSTFILE_H__
#define __VSTFILE_H__

#include "BaseFile.h"

#define VST_SIGNATURE   "#VST"

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    char signature[4]; // 0
    uint32_t unk_04; // 4
    uint32_t num_entries; // 8
    uint32_t data_start; // 0xC
} PACKED VSTHeader;

static_assert(sizeof(VSTHeader) == 0x10, "Incorrect structure size.");

typedef struct
{
    char name[16]; // 0
    uint32_t unk_10; // 0x10
    uint16_t unk_14[4]; // 0x14
    float unk_1C[10]; // 0x1C
    uint32_t unk_44; // 0x44
    uint32_t unk_48; // 0x48
    float unk_4C[13]; // 0x4C
    uint32_t unk_80; // 0x80
    float unk_84[9]; // 0x84
    uint16_t unk_A8[78]; // 0xA8
} PACKED VSTEntry;

static_assert(sizeof(VSTEntry) == 0x144, "Incorrect structure size.");

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct VstEntry
{
    std::string name;
    uint32_t unk_10;
    uint16_t unk_14[4];
    float unk_1C[10];
    uint32_t unk_44;
    uint32_t unk_48;
    float unk_4C[13];
    uint32_t unk_80;
    float unk_84[9];
    uint16_t unk_A8[78];

    void Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

class VstFile : public BaseFile
{
private:

    std::vector<VstEntry> entries;

    void Reset();

public:

    VstFile();
    virtual ~VstFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;
};

#endif
