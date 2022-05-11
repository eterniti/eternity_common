#ifndef __BAIFILE_H__
#define __BAIFILE_H__

#include "BaseFile.h"

#define BAI_SIGNATURE   0x49414223

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t version; // 6
    uint32_t num_entries; // 8
    uint32_t data_start; // 0xC
} PACKED BAIHeader;

STATIC_ASSERT_STRUCT(BAIHeader, 0x10);

typedef struct
{
    uint32_t unk_00;
    uint32_t unk_04;
    uint32_t unk_08;
    uint32_t unk_0C;
    uint32_t num_subentries; // 0x10
    uint32_t subentries_offset; // 0x14  Absolute offset
} PACKED BAIEntry;

STATIC_ASSERT_STRUCT(BAIEntry, 0x18);

typedef struct
{
    char table_name[8]; // 0  Warning: may not include the null char!
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
    float unk_44;
    float unk_48;
    float unk_4C;
    uint32_t unk_50;
} PACKED BAISubEntrry;

STATIC_ASSERT_STRUCT(BAISubEntrry, 0x54);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct BaiSubEntry
{
    std::string table_name;
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
    uint32_t unk_34; // table selector (name is just a comment) 0 -> INIT, 1-10 -> TABLE 1-10. COMMON -> -1 TUIGEKI -> -2
    uint32_t unk_38;
    uint32_t unk_3C;
    uint32_t unk_40;
    float unk_44;
    float unk_48;
    float unk_4C;
    uint32_t unk_50;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct BaiEntry
{
    uint32_t unk_00;
    uint32_t unk_04;
    uint32_t unk_08;
    uint32_t unk_0C;

    std::vector<BaiSubEntry> sub_entries;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

class BaiFile : public BaseFile
{
private:

    std::vector<BaiEntry> entries;

protected:

    void Reset();
    size_t CalculateFileSize() const;

public:

    BaiFile();
    virtual ~BaiFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;
};

#endif // __BAIFILE_H__
