#ifndef TTBFILE_H
#define TTBFILE_H

#include <unordered_map>
#include <set>
#include "BaseFile.h"


// "#TTB"
#define TTB_SIGNATURE  0x42545423

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED TTBHeader
{
    uint32_t signature;
    uint16_t endianess_check;
    uint16_t unk_06;
    uint32_t unk_08;
    uint32_t h2_start;
};
CHECK_STRUCT_SIZE(TTBHeader, 0x10);

struct PACKED TTBHeader2
{
    uint32_t num_lists;
    uint32_t data_start;
    uint64_t unk_08; // 0, it may be padding
};
CHECK_STRUCT_SIZE(TTBHeader2, 0x10);

struct PACKED TTBEventList
{
    uint32_t num_events;
    uint32_t data_start;
    uint32_t start_index;
    uint32_t list_id;
};
CHECK_STRUCT_SIZE(TTBEventList, 0x10);

struct PACKED TTBEvent
{
    uint32_t id; // 0
    uint32_t cms_id; // 4
    uint32_t costume; // 8
    uint32_t transformation; // 0xC
    uint32_t cms2_id; // 0x10
    uint32_t costume2; // 0x14
    uint32_t transformation2; // 0x18
    uint32_t cms3_id; // 0x1C
    uint32_t costume3; // 0x20
    uint32_t transformation3; // 0x24
    uint32_t cms4_id; // 0x28
    uint32_t costume4; // 0x2C
    uint32_t transformation4; // 0x30
    uint32_t cms5_id; // 0x34
    uint32_t costume5; // 0x38
    uint32_t transformation5; // 0x3C
    uint32_t type;
    uint32_t unk_44;
    uint32_t unk_48;
    uint32_t unk_4C;
    uint32_t voice_name; // 50
    uint32_t unk_54;
    uint32_t voice2_name; // 58
    uint32_t unk_5C; // Always 2
    uint32_t voice3_name; // 60
    uint32_t unk_64; // Always 3
    uint32_t voice4_name; // 68
    uint32_t unk_6C; // Always 4
    uint32_t voice5_name; // 70
};
CHECK_STRUCT_SIZE(TTBEvent, 0x74);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct TtbEvent
{
    uint32_t id;
    uint32_t costume;
    uint32_t transformation;
    uint32_t cms2_id;
    uint32_t costume2;
    uint32_t transformation2;
    uint32_t cms3_id;
    uint32_t costume3;
    uint32_t transformation3;
    uint32_t cms4_id;
    uint32_t costume4;
    uint32_t transformation4;
    uint32_t cms5_id;
    uint32_t costume5;
    uint32_t transformation5;
    uint32_t type; // 0 - Encounter; 1 - Damaged, 2 - Death
    uint32_t unk_44;
    uint32_t unk_48;

    bool reverse_order; // if false, unk_4C = 0, and unk_54 = 1; otherwise, the opposite
    bool dummy_order1;
    bool dummy_order2;

    std::string voice_name;
    std::string voice2_name;
    std::string voice3_name;
    std::string voice4_name;
    std::string voice5_name;

    TtbEvent()
    {
        id = 0;
        reverse_order = dummy_order1 = dummy_order2 = false;
        cms2_id = 0;
        cms3_id = cms4_id = cms5_id = 0xFFFFFFFF;
        costume = costume2 = costume3 = costume4 = costume5 = 0xFFFFFFFF;
        transformation = transformation2 = 0;
        transformation3 = transformation4 = transformation5 = 0xFFFFFFFF;
        type = 0;
        unk_44 = unk_48 = 0xFFFFFFFF;
    }

    TiXmlElement *Decompile(TiXmlNode *root, uint32_t cms_parent, bool subtitles_comment=false) const;
    bool Compile(const TiXmlElement *root);

    inline bool Compare(const TtbEvent &other, bool ignore_id) const
    {
        if (!ignore_id)
        {
            if (id != other.id)
                return false;
        }

        if (costume != other.costume)
            return false;

        if (transformation != other.transformation)
            return false;

        if (cms2_id != other.cms2_id)
            return false;

        if (costume2 != other.costume2)
            return false;

        if (transformation2 != other.transformation2)
            return false;

        if (cms3_id != other.cms3_id)
            return false;

        if (costume3 != other.costume3)
            return false;

        if (transformation3 != other.transformation3)
            return false;

        if (cms4_id != other.cms4_id)
            return false;

        if (costume4 != other.costume4)
            return false;

        if (transformation4 != other.transformation4)
            return false;

        if (cms5_id != other.cms5_id)
            return false;

        if (costume5 != other.costume5)
            return false;

        if (transformation5 != other.transformation5)
            return false;

        if (reverse_order != other.reverse_order)
            return false;

        if (type != other.type)
            return false;

        if (unk_44 != other.unk_44)
            return false;

        if (unk_48 != other.unk_48)
            return false;

        if (voice_name != other.voice_name)
            return false;

        if (voice2_name != other.voice2_name)
            return false;

        if (voice3_name != other.voice3_name)
            return false;

        if (voice4_name != other.voice4_name)
            return false;

        if (voice5_name != other.voice5_name)
            return false;

        return true;
    }

    inline bool operator < (const TtbEvent &other) const
    {
        return (this->id < other.id);
    }
};

struct TtbEntry
{
    uint32_t cms_id;
    std::vector<TtbEvent> events;

    TtbEntry()
    {
        cms_id = 0xFFFFFFFF;
    }

    inline bool operator < (const TtbEntry &other) const
    {
        return (this->cms_id < other.cms_id);
    }

    TiXmlElement *Decompile(TiXmlNode *root, bool subtitles_comment=false) const;
    bool Compile(const TiXmlElement *root);

    bool AddUniqueEvent(const TtbEvent &event); // Returns true if event was added, false if the event already existed
};

class TtbFile : public BaseFile
{
private:

    std::vector<TtbEntry> entries;
    bool subtitles_comments; // For xml

    size_t BuildStrings(std::unordered_map<std::string, size_t> &map, std::vector<std::string> &strings) const;

    void Preprocess();

protected:

    void Reset();

public:
    TtbFile();
    virtual ~TtbFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    inline size_t GetNumEventsLists() const { return entries.size(); }
    size_t GetNumEvents() const;

    inline void SetSubtitleComments(bool value) { subtitles_comments = value; }

    std::set<TtbEvent> GetOrderedEvents() const;

    void FixEventsID(uint32_t from_id);
    size_t RemoveAllReferencesToChar(uint32_t char_id);

    TtbEntry *FindEntry(uint32_t char_id);

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    inline const std::vector<TtbEntry> &GetEntries() const { return entries; }
    inline std::vector<TtbEntry> &GetEntries() { return entries; }

    inline const TtbEntry &operator[](size_t n) const { return entries[n]; }
    inline const TtbEntry &operator[](size_t n) { return entries[n]; }

    inline std::vector<TtbEntry>::const_iterator begin() const { return entries.begin(); }
    inline std::vector<TtbEntry>::const_iterator end() const { return entries.end(); }

    inline std::vector<TtbEntry>::iterator begin() { return entries.begin(); }
    inline std::vector<TtbEntry>::iterator end() { return entries.end(); }
};

#endif // TTBFILE_H
