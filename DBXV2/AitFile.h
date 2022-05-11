#ifndef AITFILE_H
#define AITFILE_H

#include "BaseFile.h"

#define AIT_SIGNATURE   0x54494123

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t header_size; // 6
    uint32_t num_entries; // 8
    uint32_t unk_0C;
} PACKED AITHeader;

STATIC_ASSERT_STRUCT(AITHeader, 0x10);

typedef struct
{
    uint32_t id; // 0
    uint32_t unk_04;
    uint32_t unk_08;
    uint32_t unk_0C;
    uint32_t unk_10;
    uint32_t unk_14;
    uint32_t unk_18;
    uint32_t unk_1C;
    uint32_t unk_20;
    uint32_t unk_24;
    uint32_t unk_28;
    uint32_t unk_2C;
    uint32_t unk_30;
    uint32_t unk_34;
    uint32_t unk_38;
    uint32_t unk_3C;
    uint32_t unk_40;
    uint32_t unk_44;
    uint32_t unk_48;
    uint32_t unk_4C;
    uint32_t unk_50;
    uint32_t unk_54;
    uint32_t unk_58;
    uint32_t unk_5C;
    uint32_t unk_60;
    uint32_t unk_64;
    uint32_t unk_68;
    uint32_t unk_6C;
    uint32_t unk_70;
    uint32_t unk_74;
    uint32_t unk_78;
    uint32_t unk_7C;
    uint32_t unk_80;
    uint32_t unk_84;
    uint32_t unk_88;
    uint32_t unk_8C;
    uint32_t unk_90;
    uint32_t unk_94;
    uint32_t unk_98;
    uint32_t unk_9C;
    uint32_t unk_A0;
    uint32_t unk_A4;
    uint32_t unk_A8;
    uint32_t unk_AC;
    uint32_t unk_B0;
    uint32_t unk_B4;
    uint32_t unk_B8;
    uint32_t unk_BC;
    uint32_t unk_C0;
    uint32_t unk_C4;
    uint32_t unk_C8;
    uint32_t unk_CC;
} PACKED AITEntry;

STATIC_ASSERT_STRUCT(AITEntry, 0xD0);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct AitEntry
{
    uint32_t id; // 0
    uint32_t unk_04;
    uint32_t unk_08;
    uint32_t unk_0C;
    uint32_t unk_10;
    uint32_t unk_14;
    uint32_t unk_18;
    uint32_t unk_1C;
    uint32_t unk_20;
    uint32_t unk_24;
    uint32_t unk_28;
    uint32_t unk_2C;
    uint32_t unk_30;
    uint32_t unk_34;
    uint32_t unk_38;
    uint32_t unk_3C;
    uint32_t unk_40;
    uint32_t unk_44;
    uint32_t unk_48;
    uint32_t unk_4C;
    uint32_t unk_50;
    uint32_t unk_54;
    uint32_t unk_58;
    uint32_t unk_5C;
    uint32_t unk_60;
    uint32_t unk_64;
    uint32_t unk_68;
    uint32_t unk_6C;
    uint32_t unk_70;
    uint32_t unk_74;
    uint32_t unk_78;
    uint32_t unk_7C;
    uint32_t unk_80;
    uint32_t unk_84;
    uint32_t unk_88;
    uint32_t unk_8C;
    uint32_t unk_90;
    uint32_t unk_94;
    uint32_t unk_98;
    uint32_t unk_9C;
    uint32_t unk_A0;
    uint32_t unk_A4;
    uint32_t unk_A8;
    uint32_t unk_AC;
    uint32_t unk_B0;
    uint32_t unk_B4;
    uint32_t unk_B8;
    uint32_t unk_BC;
    uint32_t unk_C0;
    uint32_t unk_C4;
    uint32_t unk_C8;
    uint32_t unk_CC;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

class AitFile : public BaseFile
{
private:

    std::vector<AitEntry> entries;
    uint32_t unk_0C;

protected:

    void Reset();

public:

    AitFile();
    virtual ~AitFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;
};

#endif // AITFILE_H
