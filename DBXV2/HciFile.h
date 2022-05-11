#ifndef __HCIFILE_H__
#define __HCIFILE_H__

#include "BaseFile.h"

#define HCI_SIGNATURE   0x49484323

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t unk_06; // Pad
    uint32_t num_entries; // 8
    uint32_t entries_offset; // 0xC
} PACKED HCIHeader;

STATIC_ASSERT_STRUCT(HCIHeader, 0x10);

typedef struct
{
    uint16_t char_id; // 0
    uint16_t costume_id; // 2
    uint16_t state1; // 4
    uint16_t state2; // 6
    uint32_t emb_index; // 8
    uint32_t unk_0C; // always zero
} PACKED HCIEntry;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct HciEntry
{
    uint16_t char_id;
    uint16_t costume_id;
    uint16_t state1;
    uint16_t state2;
    uint32_t emb_index;

    HciEntry()
    {
        char_id = 0;
        costume_id = 0xFFFF;
        state1 = 0xFFFF;
        state2 = 0xFFFF;
        emb_index = 0;
    }

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);

    inline bool operator==(const HciEntry &other) const
    {
        return (this->char_id == other.char_id &&
                this->costume_id == other.costume_id &&
                this->state1 == other.state1 &&
                this->state2 == other.state2 &&
                this->emb_index == other.emb_index);
    }

    inline bool operator!=(const HciEntry &other) const
    {
        return !(*this == other);
    }
};

class HciFile : public BaseFile
{
private:

    std::vector<HciEntry> entries;

    bool LoadCommon(const uint8_t *buf, size_t size);

protected:

    void Reset();

public:

    HciFile();
    virtual ~HciFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    bool LoadMultiple(const std::vector<const uint8_t *> &bufs, const std::vector<size_t> &sizes);
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    inline size_t GetNumEntries() const { return entries.size(); }

    size_t FindEntriesByCharID(uint32_t char_id, std::vector<HciEntry *> &hci_entries);
    bool AddEntry(const HciEntry &entry, bool unique_char_id);
    size_t RemoveEntries(uint32_t char_id);

    inline const std::vector<HciEntry> &GetEntries() const { return entries; }
    inline std::vector<HciEntry> &GetEntries() { return entries; }

    inline const HciEntry &operator[](size_t n) const { return entries[n]; }
    inline const HciEntry &operator[](size_t n) { return entries[n]; }

    inline std::vector<HciEntry>::const_iterator begin() const { return entries.begin(); }
    inline std::vector<HciEntry>::const_iterator end() const { return entries.end(); }

    inline std::vector<HciEntry>::iterator begin() { return entries.begin(); }
    inline std::vector<HciEntry>::iterator end() { return entries.end(); }
};

#endif // HCIFILE_H
