#ifndef __PALFILE_H__
#define __PALFILE_H__

#include "BaseFile.h"

#define PAL_SIGNATURE   0x4C415023

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t unk_06;
    uint32_t num_entries; // 8
    uint32_t data_start; // 0xC
} PACKED PALHeader;

STATIC_ASSERT_STRUCT(PALHeader, 0x10);

typedef struct
{
    uint16_t id; // 0
    uint16_t name_id; // 2
    uint16_t cms_entry; // 4
    uint16_t voice; // 6
    uint16_t team_mate; // 8
} PACKED PALInfo;

STATIC_ASSERT_STRUCT(PALInfo, 0xA);

typedef struct
{
    uint16_t face_base; // 0
    uint16_t face_forehead; // 2
    uint16_t eyes; // 4
    uint16_t nose; // 6
    uint16_t ears; // 8
    uint16_t hair; // 0xA
    uint16_t top; // 0xC
    uint16_t bottom; // 0xE
    uint16_t gloves; // 0x10
    uint16_t shoes; // 0x12
    uint16_t body_shape; // 0x14
    uint16_t unk_16;
    uint16_t skin_color1; // 0x18
    uint16_t skin_color2; // 0x1A
    uint16_t skin_color3; // 0x1C
    uint16_t skin_color4;  // 0x1E
    uint16_t hair_color; // 0x20
    uint16_t eye_color; // 0x22
    uint16_t top_color1; // 0x24
    uint16_t top_color2; // 0x26
    uint16_t top_color3; // 0x28
    uint16_t top_color4; // 0x2A
    uint16_t bottom_color1; // 0x2C
    uint16_t bottom_color2; // 0x2E
    uint16_t bottom_color3; // 0x30
    uint16_t bottom_color4;  // 0x32
    uint16_t gloves_color1; // 0x34
    uint16_t gloves_color2; // 0x36
    uint16_t gloves_color3; // 0x38
    uint16_t gloves_color4; // 0x3A
    uint16_t shoes_color1; // 0x3C
    uint16_t shoes_color2; // 0x3E
    uint16_t shoes_color3; // 0x40
    uint16_t shoes_color4; // 0x42
    uint16_t makeup_color1; // 0x44
    uint16_t makeup_color2; // 0x46
    uint16_t makeup_color3; // 0x48 lips_color
    uint16_t accesory; // 0x4A
    uint16_t talisman; // 0x4C
} PACKED PALEquipment;

STATIC_ASSERT_STRUCT(PALEquipment, 0x4E);

typedef struct
{
    uint16_t level; // 0
    uint16_t hea; // 2
    uint16_t ki; // 4
    uint16_t stm; // 6
    uint16_t atk; // 8
    uint16_t str; // 0xA
    uint16_t bla; // 0xC
    uint16_t unk_0E; // 0xE
    uint16_t super_skills[4]; // 0x10
    uint16_t ult_skills[2]; // 0x18
    uint16_t evasive_skill; // 0x1A
    uint16_t blast_skill; // 0x1E
    uint16_t awaken_skill; // 0x20
} PACKED PALStats;

STATIC_ASSERT_STRUCT(PALStats, 0x22);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct PalEquipment
{
    uint16_t face_base;
    uint16_t face_forehead;
    uint16_t eyes;
    uint16_t nose;
    uint16_t ears;
    uint16_t hair;
    uint16_t top; //
    uint16_t bottom;
    uint16_t gloves;
    uint16_t shoes;
    uint16_t body_shape;
    uint16_t unk_16;
    uint16_t skin_color1;
    uint16_t skin_color2;
    uint16_t skin_color3;
    uint16_t skin_color4;
    uint16_t hair_color;
    uint16_t eye_color;
    uint16_t top_color1;
    uint16_t top_color2;
    uint16_t top_color3;
    uint16_t top_color4;
    uint16_t bottom_color1;
    uint16_t bottom_color2;
    uint16_t bottom_color3;
    uint16_t bottom_color4;
    uint16_t gloves_color1;
    uint16_t gloves_color2;
    uint16_t gloves_color3;
    uint16_t gloves_color4;
    uint16_t shoes_color1;
    uint16_t shoes_color2;
    uint16_t shoes_color3;
    uint16_t shoes_color4;
    uint16_t makeup_color1;
    uint16_t makeup_color2;
    uint16_t makeup_color3; // lips_color
    uint16_t accesory;
    uint16_t talisman;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);

    inline bool operator==(const PalEquipment &rhs) const
    {
        return (memcmp(this, &rhs, sizeof(PalEquipment)) == 0);
    }

    inline bool operator!=(const PalEquipment &rhs) const
    {
        return !(*this == rhs);
    }
};

struct PalStats
{
    uint16_t level;
    uint16_t hea;
    uint16_t ki;
    uint16_t stm;
    uint16_t atk;
    uint16_t str;
    uint16_t bla;
    uint16_t unk_0E;
    uint16_t super_skills[4];
    uint16_t ult_skills[2];
    uint16_t evasive_skill;
    uint16_t blast_skill;
    uint16_t awaken_skill;
    bool nostats;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);

    PalStats()
    {
        level = 1;
        hea = 0;
        ki = 0;
        stm = 0;
        atk = 0;
        str = 0;
        bla = 0;
        unk_0E = 100;
        memset(super_skills, 0xFF, sizeof(super_skills));
        memset(ult_skills, 0xFF, sizeof(ult_skills));
        evasive_skill = 0xFFFF;
        blast_skill = 0xFFFF;
        awaken_skill = 0xFFFF;
        nostats = false;
    }

    inline bool IsDummy() const
    {
        PalStats dummy;
        dummy.nostats = nostats;

        return (*this == dummy);
    }

    inline bool operator==(const PalStats &rhs) const
    {
        return (level == rhs.level && hea == rhs.hea && ki == rhs.ki &&
                stm == rhs.stm && atk == rhs.atk && str == rhs.str &&
                bla == rhs.bla && unk_0E == rhs.unk_0E &&
                memcmp(super_skills, rhs.super_skills, sizeof(super_skills)) == 0 &&
                memcmp(ult_skills, rhs.ult_skills, sizeof(ult_skills)) == 0 &&
                evasive_skill == rhs.evasive_skill && blast_skill == rhs.blast_skill &&
                awaken_skill == rhs.awaken_skill);
    }

    inline bool operator!=(const PalStats &rhs) const
    {
        return !(*this == rhs);
    }
};

struct PalEntry
{
    uint16_t id;
    uint16_t name_id;
    uint16_t cms_entry;
    uint16_t voice;
    uint16_t team_mate;

    PalEquipment equipment;
    PalStats stats;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);

    inline bool operator==(const PalEntry &rhs) const
    {
        return (this->id == rhs.id && this->name_id == rhs.name_id && this->cms_entry == rhs.cms_entry && this->voice == rhs.voice && this->team_mate == rhs.team_mate
                && this->equipment == rhs.equipment && this->stats == rhs.stats);
    }

    inline bool operator!=(const PalEntry &rhs) const
    {
        return !(*this == rhs);
    }
};

class PalFile : public BaseFile
{
private:

    std::vector<PalEntry> entries;

    size_t GetNumValidStats() const;

protected:

    void Reset();

public:

    PalFile();
    virtual ~PalFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    inline size_t GetNumEntries() const { return entries.size(); }

    inline size_t AddEntry(const PalEntry &entry) { entries.push_back(entry); return (entries.size()-1); }
    inline bool RemoveEntryByIndex(size_t index)
    {
        if (index >= entries.size())
            return false;

        entries.erase(entries.begin()+index);
        return true;
    }

    PalEntry *FindEntryByID(uint32_t id);

    inline const std::vector<PalEntry> &GetEntries() const { return entries; }
    inline std::vector<PalEntry> &GetEntries() { return entries; }

    inline const PalEntry &operator[](size_t n) const { return entries[n]; }
    inline PalEntry &operator[](size_t n) { return entries[n]; }

    inline bool operator==(const PalFile &rhs) const
    {
        return (this->entries == rhs.entries);
    }

    inline bool operator!=(const PalFile &rhs) const
    {
        return !(*this == rhs);
    }

    inline std::vector<PalEntry>::const_iterator begin() const { return entries.begin(); }
    inline std::vector<PalEntry>::const_iterator end() const { return entries.end(); }

    inline std::vector<PalEntry>::iterator begin() { return entries.begin(); }
    inline std::vector<PalEntry>::iterator end() { return entries.end(); }
};

#endif // __PALFILE_H__
