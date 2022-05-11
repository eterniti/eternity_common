#ifndef CNSFILE_H
#define CNSFILE_H

#include "BaseFile.h"

#define CNS_SIGNATURE   0x534E4323

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED CNSHeader
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t unk_06;
    uint32_t num_entries; // 8
    uint32_t unk_0C; // Seen value of 0x14
    uint32_t data_start; // 0x10
    uint16_t unk_14; // Seen value of 0
    uint16_t unk_16; // Seen value of 300 (0x12C)
    uint16_t unk_18; // Seen value of 2
    uint16_t unk_1A; // Seen value of 100 (0x64)
};
CHECK_STRUCT_SIZE(CNSHeader, 0x1C);

struct PACKED CNSEntry
{
    char code[8]; // 0
    uint16_t id; // 8
    uint16_t unk_0A[33]; // All 0 except in empty entries?
    uint16_t unk_4C[33]; // All 10 (0xa) except in empty entries?
    uint16_t unk_8E; // always 20 (0x14) except in empty entries?
    uint16_t unk_90; // Always -1?
    uint16_t unk_92[7]; // always 21 (0x15) except in empty entries?
    uint16_t owner_cms; // 0xA0
    uint16_t unk_A2[7]; // always -1?
    uint16_t owner_cms2; // 0xB0 - always same as owner_cms?
    uint16_t unk_B2; // Always 0, even in empty entries?

    CNSEntry()
    {
        memset(code, 0, sizeof(code));
        id = unk_8E = unk_90 = owner_cms = owner_cms2 = 0xFFFF;
        memset(unk_0A, 0xFF, sizeof(unk_0A));
        memset(unk_4C, 0xFF, sizeof(unk_4C));
        memset(unk_92, 0xFF, sizeof(unk_92));
        memset(unk_A2, 0xFF, sizeof(unk_A2));
        unk_B2 = 0;
    }

    inline bool IsValid() const
    {
        return (code[0] != 0 && id < 0x8000);
    }
};
CHECK_STRUCT_SIZE(CNSEntry, 0xB4);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

typedef CNSEntry CnsEntry;

class CnsFile : public BaseFile
{
private:

    std::vector<CnsEntry> entries;

protected:

    void Reset();

public:

    CnsFile();
    virtual ~CnsFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;

    inline size_t GetNumEntries() const { return entries.size(); }

    CnsEntry *FindEntry(uint16_t skill_id);

    inline const std::vector<CnsEntry> &GetEntries() const { return entries; }
    inline std::vector<CnsEntry> &GetEntries() { return entries; }

    inline const CnsEntry &operator[](size_t n) const { return entries[n]; }
    inline CnsEntry &operator[](size_t n) { return entries[n]; }

    inline std::vector<CnsEntry>::const_iterator begin() const { return entries.begin(); }
    inline std::vector<CnsEntry>::const_iterator end() const { return entries.end(); }

    inline std::vector<CnsEntry>::iterator begin() { return entries.begin(); }
    inline std::vector<CnsEntry>::iterator end() { return entries.end(); }

};

#endif // CNSFILE_H
