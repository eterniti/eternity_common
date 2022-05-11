#ifndef __MSGFILE_H__
#define __MSGFILE_H__

#include "BaseFile.h"

// "#MSG"
#define MSG_SIGNATURE	0x47534D23

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    uint32_t signature; // 0
    uint16_t type; // 4    0x100 = unicode for vars names. 0 ascii for vars names.
    uint16_t unicode_values; // 6   1 if unicode is used in values
    uint32_t num_entries; // 8
    uint32_t names_section_start; // 0xC
    uint32_t id_section_start; // 0x10
    uint32_t lines_section_start; // 0x14
    uint32_t num_strings; // 0x18
    uint32_t strings_section_start; // 0x1C
} PACKED MSGHeader;

STATIC_ASSERT_STRUCT(MSGHeader, 0x20);

typedef struct
{
    uint32_t string_offset;
    uint32_t num_chars; // Doesn't include then null byte
    uint32_t num_bytes; // Includes the null byte. If ascii, num_bytes=num_chars+1. If unicode, num_bytes=(num_chars*2)+2
    uint32_t num_vars; // variables are put with %ls
} PACKED MSGStr;

STATIC_ASSERT_STRUCT(MSGStr, 0x10);

typedef struct
{
    uint32_t num_lines;
    uint32_t top_line; // Points to first MSGStr
} PACKED MSGLines;

STATIC_ASSERT_STRUCT(MSGLines, 8);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct MsgEntry
{
    std::string name;
    uint32_t id;
    std::vector<std::string> lines;
    std::vector<std::u16string> u16_lines;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

class MsgFile : public BaseFile
{
private:

    std::vector<MsgEntry> entries;
    bool unicode_names;
    bool unicode_values;
    bool u16_mode;

    void ReadMsgStr(const uint8_t *top, const MSGStr *msg_str, std::string &str, bool unicode);
    void ReadMsgStr(const uint8_t *top, const MSGStr *msg_str, std::u16string &str, bool unicode);
    uint32_t FindHighestID();

protected:

    void Reset();
    size_t CalculateFileSize(uint32_t *str_data_offset) const;

public:

    MsgFile();
    virtual ~MsgFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    inline void SetU16Mode(bool u16) { u16_mode = u16; }

    size_t GetNumEntries() const { return entries.size(); }
    MsgEntry *FindEntryByName(const std::string &name);
    MsgEntry *FindEntryByID(uint32_t id);

    bool AddEntry(MsgEntry &entry, bool auto_id);

    // This version adds the entry if it doesn't exist, or update existing one
    bool AddEntryUpdate(MsgEntry &entry);

    // Both return true if the entry doesn't exist after the function has returned.
    // Either because it was deleted or it never existed.
    bool RemoveEntry(size_t idx, bool *existed=nullptr);
    bool RemoveEntry(const std::string &name, bool *existed=nullptr);

    inline const MsgEntry &operator[](size_t n) const { return entries[n]; }
    inline MsgEntry &operator[](size_t n) { return entries[n]; }

    inline std::vector<MsgEntry>::const_iterator begin() const { return entries.begin(); }
    inline std::vector<MsgEntry>::const_iterator end() const { return entries.end(); }

    inline std::vector<MsgEntry>::iterator begin() { return entries.begin(); }
    inline std::vector<MsgEntry>::iterator end() { return entries.end(); }
};

#endif // __MSGFILE_H__
