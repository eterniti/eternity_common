#ifndef BPEFILE_H
#define BPEFILE_H

// Note: atm this is a very minimal parser for the current functionality for chara creator / skill creator. No save support.

#include <BaseFile.h>

enum BpeSubEntryType
{
    BPE_ST_BLUR = 0,
    BPE_ST_WHITESHINE = 1,
    BPE_ST_DISORIENTATION = 4,
    BPE_ST_ZOOM = 6,
    BPE_ST_HUE = 8,
    BPE_ST_BODY_OUTLINE = 9
};

#define BPE_SIGNATURE   0x45504223

;
#pragma pack(push,1)

struct PACKED BPEHeader
{
    uint32_t signature; // 0
    uint16_t endianess_check;  // 4
    uint16_t header_size; // 6
    uint64_t unk_08;
    uint16_t unk_10;
    uint16_t num_entries; // 0x12
    uint32_t data_start; // 0x14

    BPEHeader()
    {
        signature = BPE_SIGNATURE;
        endianess_check = 0xFFFE;
        header_size = sizeof(BPEHeader);
        unk_08 = 0;
        unk_10 = 0;
        num_entries = 0;
        data_start = sizeof(BPEHeader);
    }
};
CHECK_STRUCT_SIZE(BPEHeader, 0x18);

struct PACKED BPEEntry
{
    uint32_t unk_00;
    uint16_t duration; // 4
    uint16_t unk_06;
    uint16_t unk_08;
    uint16_t num_sub_entries; // A
    uint32_t sub_offset; // C - offset from start of this entry
};
CHECK_STRUCT_SIZE(BPEEntry, 0x10);

struct PACKED BPESubEntry
{
    int32_t type;
    uint8_t todo[0x2C];
};
CHECK_STRUCT_SIZE(BPESubEntry, 0x30);

#pragma pack(pop)

struct BpeSubEntry
{
    int type;
};

struct BpeEntry
{
    uint32_t unk_00;
    uint16_t duration;
    uint16_t unk_06;
    uint16_t unk_08;
    bool valid;
    std::vector<BpeSubEntry> sub_entries;

    BpeEntry()
    {
        unk_00 = 0;
        duration = unk_06 = unk_08 = 0;
        valid = false;
    }

    BpeEntry(const BPEEntry &entry)
    {
        this->unk_00 = entry.unk_00;
        this->duration = entry.duration;
        this->unk_06 = entry.unk_06;
        this->unk_08 = entry.unk_08;
        valid = true;
    }
};

class BpeFile : public BaseFile
{
private:
    std::vector<BpeEntry> entries;

protected:
    void Reset();

public:
    BpeFile();
    virtual ~BpeFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;

    inline size_t GetNumEntries() const { return entries.size(); }
    inline bool IsValidEntry(size_t idx) const { return (idx < entries.size() && entries[idx].valid); }

    bool HasType(size_t idx, int type) const;
    std::string GetTypeString(size_t idx) const;
};

#endif // BPEFILE_H
