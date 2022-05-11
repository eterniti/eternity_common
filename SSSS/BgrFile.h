#ifndef __BGRFILE_H__
#define __BGRFILE_H__

#include <vector>
#include "BaseFile.h"

#define BGR_SIGNATURE   "#BGR"

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    char signature[4]; // 0
    uint16_t endianess_check; // 4;
    uint16_t unk_06; // 6
    uint32_t num_entries; // 8
    uint32_t unk_0C; // 0xC
    uint32_t data_start; // 0x10
    uint32_t unk_14; // 0x14
} PACKED BGRHeader;

static_assert(sizeof(BGRHeader) == 0x18, "Incorrect structure size.");

typedef struct
{
    uint32_t level; // 0
    uint32_t hour; // 4
    uint32_t unk_08; // 8
    uint32_t cms_entry; // 0xC
    uint32_t cms_model_spec_idx; // 0x10
    uint32_t unk_14; // 0x14
    uint32_t unk_18; // 0x18
    uint32_t assist_phrase1; // 0x1C
    uint32_t assist_phrase2; // 0x20
    uint32_t assist_phrase3; // 0x24
    uint32_t assist_phrase4; // 0x28
    uint32_t assist_phrase5; // 0x2C
    uint32_t assist_phrase6; // 0x30
    uint32_t assist_phrase7; // 0x34
    uint32_t assist_phrase8; // 0x38
    uint32_t stage; // 0x3C
    uint32_t hp; // 0x40
    uint32_t unk_44; // 0x44
} PACKED BGREntry;

static_assert(sizeof(BGREntry) == 0x48, "Incorrect structure size.");

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct BgrEntry
{
    uint32_t level;
    uint32_t hour;
    uint32_t unk_08;
    uint32_t cms_entry;
    uint32_t cms_model_spec_idx;
    uint32_t unk_14;
    uint32_t unk_18;
    uint32_t assist_phrase1;
    uint32_t assist_phrase2;
    uint32_t assist_phrase3;
    uint32_t assist_phrase4;
    uint32_t assist_phrase5;
    uint32_t assist_phrase6;
    uint32_t assist_phrase7;
    uint32_t assist_phrase8;
    uint32_t stage;
    uint32_t hp;
    uint32_t unk_44;

    void Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);

private:

    void DecompileAssistPhrase(TiXmlElement *root, const char *name, uint32_t assist_phrase) const;
};

class BgrFile : public BaseFile
{
private:

    std::vector<BgrEntry> entries;

    void Reset();

public:

    BgrFile();
    virtual ~BgrFile();

    inline uint32_t GetNumEntries() const
    {
        return entries.size();
    }

    inline const BgrEntry *GetEntry(uint32_t idx) const
    {
        if (idx >= entries.size())
            return nullptr;

        return &entries[idx];
    }

    inline BgrEntry *GetEntry(uint32_t idx)
    {
        if (idx >= entries.size())
            return nullptr;

        return &entries[idx];
    }

    inline void AddEntry(const BgrEntry &entry)
    {
        entries.push_back(entry);
    }

    inline bool RemoveEntry(uint32_t idx)
    {
        if (idx >= entries.size())
            return false;

        entries.erase(entries.begin()+idx);
    }

    size_t RemoveChar(uint32_t cms_entry, uint32_t cms_model_spec_idx);

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *size) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;
};

#endif
