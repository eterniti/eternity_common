#ifndef __SEVFILE_H__
#define __SEVFILE_H__

#include "BaseFile.h"

#define SEV_SIGNATURE   0x56455323

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t header_size; // 6
    uint32_t num_entries; // 8
    uint32_t unk_0C; // zero
} PACKED SEVHeader;
CHECK_STRUCT_SIZE(SEVHeader, 0x10);

typedef struct
{
    uint32_t char_id; // 0
    uint32_t costume_id; // 4
    uint32_t unk_08; // Added in 1.15
    uint32_t num_cev_entries; // C
    uint32_t unk_10; // Always zero
} PACKED SEVEntry;
CHECK_STRUCT_SIZE(SEVEntry, 0x14);

typedef struct
{
    uint32_t char_id; // 0
    uint32_t costume_id;// 4
    uint32_t num_events; // 8
    uint32_t unk_0C; // 0
} PACKED SEVCharEvents;
CHECK_STRUCT_SIZE(SEVCharEvents, 0x10);

typedef struct
{
    uint32_t type;
    uint32_t num_entries;
    uint32_t unk_08; // Always zero
} PACKED SEVEvent;
CHECK_STRUCT_SIZE(SEVEvent, 0xC);

typedef struct
{
    uint32_t unk_00;
    uint32_t cue_id; // 4
    uint32_t file_id; // 8
    uint32_t new_unk_0C; // Added in game version 1.06
    uint32_t response_cue_id; // Added in game version 1.06
    uint32_t response_file_id; // Added in game version 1.06
    uint32_t unk_18; // Added in game version 1.06
    uint32_t old_unk_0C; // 0x1C
    uint32_t old_unk_10; // 0x20
} PACKED SEVEventEntry;
CHECK_STRUCT_SIZE(SEVEventEntry, 0x24);

#ifdef _MSC_VER
#pragma pack(pop)
#endif


struct SevEventEntry
{
    uint32_t unk_00;
    uint32_t cue_id;
    uint32_t file_id;
    uint32_t new_unk_0C; // Added in game version 1.06
    uint32_t response_cue_id; // Added in game version 1.06
    uint32_t response_file_id; // Added in game version 1.06
    uint32_t unk_18; // Added in game version 1.06
    uint32_t old_unk_0C;
    uint32_t old_unk_10;

    SevEventEntry()
    {
        unk_00 = 0;
        cue_id = 0;
        file_id = 0;
        new_unk_0C = 0;
        response_cue_id = 0xFFFFFFFF;
        response_file_id = 0;
        unk_18 = 0;
        old_unk_0C = 0;
        old_unk_10 = 0;
    }

    inline bool operator==(const SevEventEntry &rhs) const
    {
        return (unk_00 == rhs.unk_00 && cue_id == rhs.cue_id && file_id == rhs.file_id &&
                new_unk_0C == rhs.new_unk_0C && response_cue_id == rhs.response_cue_id &&
                response_file_id == rhs.response_file_id && unk_18 == rhs.unk_18 && old_unk_0C == rhs.old_unk_0C &&
                old_unk_10 == rhs.old_unk_10);
    }

    inline bool operator!=(const SevEventEntry &rhs) const
    {
        return !(*this == rhs);
    }

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct SevEvent
{
    uint32_t type;
    std::vector<SevEventEntry> entries;

    SevEvent()
    {
        type = 0;
    }

    inline bool operator==(const SevEvent &rhs) const
    {
        return (type == rhs.type && entries == rhs.entries);
    }

    inline bool operator!=(const SevEvent &rhs) const
    {
        return !(*this == rhs);
    }

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct SevCharEvents
{
    uint32_t char_id;
    uint32_t costume_id;
    std::vector<SevEvent> events;

    inline bool operator==(const SevCharEvents &rhs) const
    {
        return (char_id == rhs.char_id && costume_id == rhs.costume_id && events == rhs.events);
    }

    inline bool operator!=(const SevCharEvents &rhs) const
    {
        return !(*this == rhs);
    }

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct SevEntry
{
    uint32_t char_id;
    uint32_t costume_id;
    uint32_t unk_08;
    std::vector<SevCharEvents> chars_events;

    SevEntry()
    {
        char_id = 0; costume_id = 0;
        unk_08 = 0;
    }

    inline bool operator==(const SevEntry &rhs) const
    {
        return (char_id == rhs.char_id && costume_id == rhs.costume_id && unk_08 == rhs.unk_08 && chars_events == rhs.chars_events);
    }

    inline bool operator!=(const SevEntry &rhs) const
    {
        return !(*this == rhs);
    }

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

class SevFile : public BaseFile
{
private:

    std::vector<SevEntry> entries;

protected:

    void Reset();
    size_t CalculateFileLayout(uint32_t *events_offset, uint32_t *ev_entries_offset) const;

public:

    SevFile();
    virtual ~SevFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    inline size_t GetNumEntries() const { return entries.size(); }
    SevEntry *FindEntry(uint32_t char_id, uint32_t costume_id);

    size_t RemoveAllReferencesToChar(uint32_t char_id);

    inline const std::vector<SevEntry> &GetEntries() const { return entries; }
    inline std::vector<SevEntry> &GetEntries() { return entries; }

    inline const SevEntry &operator[](size_t n) const { return entries[n]; }
    inline const SevEntry &operator[](size_t n) { return entries[n]; }

    inline std::vector<SevEntry>::const_iterator begin() const { return entries.begin(); }
    inline std::vector<SevEntry>::const_iterator end() const { return entries.end(); }

    inline std::vector<SevEntry>::iterator begin() { return entries.begin(); }
    inline std::vector<SevEntry>::iterator end() { return entries.end(); }
};

#endif // _SEVFILE_H__
