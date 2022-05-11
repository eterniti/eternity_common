#ifndef __CMLFILE_H__
#define __CMLFILE_H__

#include "BaseFile.h"

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    uint16_t char_id; // 0
    uint16_t costume_id; // 2
    uint32_t unk_04; // 4
    float css_pos_z; // 8
    float unk_0C;

    float neo_unk_10; // 10 - Added in 1.09

    float unk_10; // 14 (Offsets from here are 4 off because of the neo_unk_10)
    float unk_14;
    float unk_18;
    float unk_1C;
    float unk_20;
    float unk_24; // Always zero, duno if float or not
    float unk_28;
    float unk_2C;
    float unk_30;
    float unk_34;
    float unk_38;
    float unk_3C;
    float unk_40;
    float unk_44;
    float unk_48;
    float css_rot_x; // 0x4C
    float css_rot_y; // 0x50
    float css_pos_y; // 0x54
    float css_pos_x; // 0x58
    float unk_5C;
    float unk_60;
    float unk_64;
    float unk_68;
} CMLEntry;
CHECK_STRUCT_SIZE(CMLEntry, 0x70);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct CmlEntry
{
    uint16_t char_id;
    uint16_t costume_id;
    uint32_t unk_04;

    float css_pos[3];
    float css_rot[2];

    float unk_0C;
    float unk_10;

    float neo_unk_10; // 10 - Added in 1.09

    float unk_14;
    float unk_18;
    float unk_1C;
    float unk_20;
    float unk_24;
    float unk_28;
    float unk_2C;
    float unk_30;
    float unk_34;
    float unk_38;
    float unk_3C;
    float unk_40;
    float unk_44;
    float unk_48;
    float unk_5C;
    float unk_60;
    float unk_64;
    float unk_68;

    CmlEntry()
    {
        char_id = costume_id = 0xFFFF;
        unk_04 = 0;
        css_pos[0] = css_pos[1] = css_pos[2] = 0;
        css_rot[0] = css_rot[1] = 0;
        unk_0C = unk_10 = unk_14 = unk_18 = unk_1C = 0;
        unk_20 = unk_24 = unk_28 = unk_2C = unk_30 = 0;
        unk_34 = unk_38 = unk_3C = unk_40 = unk_44 = 0;
        unk_48 = unk_5C = unk_60 = unk_64 = unk_68 = 0;
        neo_unk_10 = 0;
    }

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

class CmlFile : public BaseFile
{
private:

    std::vector<CmlEntry> entries;

protected:

    void Reset();

public:

    CmlFile();
    virtual ~CmlFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    inline size_t GetNumEntries() const { return entries.size(); }

    size_t FindEntriesByCharID(uint32_t char_id, std::vector<CmlEntry *> &cml_entries);
    bool AddEntry(const CmlEntry &entry, bool unique_char_id);
    size_t RemoveEntries(uint32_t char_id);

    CmlEntry *FindEntry(uint32_t char_id, uint32_t costume_id);

    inline const std::vector<CmlEntry> &GetEntries() const { return entries; }
    inline std::vector<CmlEntry> &GetEntries() { return entries; }

    inline const CmlEntry &operator[](size_t n) const { return entries[n]; }
    inline const CmlEntry &operator[](size_t n) { return entries[n]; }

    inline std::vector<CmlEntry>::const_iterator begin() const { return entries.begin(); }
    inline std::vector<CmlEntry>::const_iterator end() const { return entries.end(); }

    inline std::vector<CmlEntry>::iterator begin() { return entries.begin(); }
    inline std::vector<CmlEntry>::iterator end() { return entries.end(); }
};

#endif // __CMLFILE_H__
