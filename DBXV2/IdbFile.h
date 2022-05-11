#ifndef __IDBFILE_H__
#define __IDBFILE_H__

#include "BaseFile.h"

#define IDB_SIGNATURE   0x42444923

#define IDB_RACE_HUM    1
#define IDB_RACE_HUF    2
#define IDB_RACE_SYM    4
#define IDB_RACE_SYF    8
#define IDB_RACE_NMC    0x10
#define IDB_RACE_FRI    0x20
#define IDB_RACE_MAM    0x40
#define IDB_RACE_MAF    0x80

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t unk_06; // set to 7
    uint32_t num_entries; // 8
    uint32_t data_start; // 0xC
} PACKED IDBHeader;

STATIC_ASSERT_STRUCT(IDBHeader, 0x10);

typedef struct
{
    uint32_t type; // 0
    uint32_t activation_type; // 4
    uint32_t num_act_times; // 8
    float timer; // 0xC
    float ability_values[6]; // 0x10
    uint32_t unk_28; //
    uint32_t activation_chance; // 0x2C
    float multipliers[6]; // 0x30
    uint32_t unk_48[6];
    float hea; // 0x60
    float ki; // 0x64
    float ki_recovery; // 0x68
    float stm; // 0x6C
    float stamina_recovery; // 0x70
    float enemy_stamina_eraser; // 0x74
    float unk_78;
    float ground_speed; // 0x7C
    float air_speed; // 0x80
    float boosting_speed; // 0x84
    float dash_speed; // 0x88
    float atk; // 0x8C
    float basic_ki_attack; // 0x90
    float str; // 0x94
    float bla; // 0x98
    float atk_damage; // 0x9C
    float ki_damage; // 0xA0
    float str_damage; // 0xA4
    float bla_damage; // 0xA8
    float unk_AC[13];
} PACKED IDBEffect;

STATIC_ASSERT_STRUCT(IDBEffect, 0xE0);

typedef struct
{
    uint16_t id; // 0
    uint16_t stars; // 2
    uint16_t name_id; // 4
    uint16_t desc_id; // 6
    uint16_t type; // 8
    uint16_t unk_0A;
    uint16_t unk_0C;
    uint16_t unk_0E;
    uint32_t buy; // 0x10
    uint32_t sell; // 0x14
    uint32_t racelock; // 0x18
    uint32_t tp; // 0x1C
    uint32_t model; // 0x20
    uint32_t unk_24[3]; //
    IDBEffect effects[3]; // 0x30
} PACKED IDBEntry;

STATIC_ASSERT_STRUCT(IDBEntry, 0x2D0);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

enum class IdbCommentType
{
    NONE,
    SKILL,
    COSTUME,
    ACCESORY,
    TALISMAN,
    MATERIAL,
    BATTLE,
    EXTRA,
    PET
};

struct IdbEffect
{
    uint32_t type;
    uint32_t activation_type;
    uint32_t num_act_times;
    float timer;
    float ability_values[6];
    uint32_t unk_28;
    uint32_t activation_chance;
    float multipliers[6];
    uint32_t unk_48[6];
    float hea;
    float ki;
    float ki_recovery;
    float stm;
    float stamina_recovery;
    float enemy_stamina_eraser;
    float unk_78;
    float ground_speed;
    float air_speed;
    float boosting_speed;
    float dash_speed;
    float atk;
    float basic_ki_attack;
    float str;
    float bla;
    float atk_damage;
    float ki_damage;
    float str_damage;
    float bla_damage;
    float unk_AC[13];

    IdbEffect()
    {
        type = activation_type = num_act_times = 0xFFFFFFFF;
        timer = -1.0f;
        ability_values[0] = ability_values[1] = ability_values[2] = ability_values[3] = ability_values[4] = ability_values[5] = -1.0f;
        unk_28 = activation_chance = 0xFFFFFFFF;
        memset(multipliers, 0, sizeof(multipliers));
        unk_48[0] = unk_48[2] = 0;
        unk_48[1] = unk_48[3] = unk_48[4] = unk_48[5] = 0xFFFFFFFF;
        hea = ki = ki_recovery = stm = stamina_recovery = enemy_stamina_eraser = 0.0f;
        unk_78 = 0;
        ground_speed = air_speed = boosting_speed = dash_speed = 0.0f;
        atk = basic_ki_attack = str = bla = 0.0f;
        atk_damage = ki_damage = str_damage = bla_damage = 0.0f;

        memset(unk_AC, 0, sizeof(unk_AC));
    }

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct IdbEntry
{
    uint16_t id;
    uint16_t stars;
    uint16_t name_id;
    uint16_t desc_id;
    uint16_t type;
    uint16_t unk_0A;
    uint16_t unk_0C;
    uint16_t unk_0E;
    uint32_t buy;
    uint32_t sell;
    uint32_t racelock;
    uint32_t tp;
    uint32_t model;
    uint32_t unk_24[3];
    IdbEffect effects[3];

    IdbEntry()
    {
        id = 0xFFFF;
        stars = 0;
        name_id = desc_id = 0xFFFF;
        type = unk_0A = unk_0C = unk_0E = 0;
        buy = sell = racelock = 0;
        tp = model = 0;
        unk_24[0] = unk_24[1] = unk_24[2] = 0;
    }

    TiXmlElement *Decompile(TiXmlNode *root, IdbCommentType comm_type) const;
    bool Compile(const TiXmlElement *root);
};

class IdbFile : public BaseFile
{
private:

    std::vector<IdbEntry> entries;
    IdbCommentType comm_type;

    void Reset();

public:

    IdbFile();
    virtual ~IdbFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    inline size_t GetNumEntries() const { return entries.size(); }

    IdbEntry *FindEntryByID(uint32_t id);
    IdbEntry *FindEntryByIdAndType(uint16_t id, uint16_t type);

    bool AddEntry(const IdbEntry &entry, bool overwrite_existing);
    bool AddEntryAuto(IdbEntry &entry, uint16_t id_start);
    size_t RemoveEntry(uint16_t id, uint16_t type);
    size_t RemoveEntry(uint16_t id);

    inline void SetCommentType(IdbCommentType type) { comm_type = type; }

    inline const std::vector<IdbEntry> &GetEntries() const { return entries; }
    inline std::vector<IdbEntry> &GetEntries() { return entries; }

    inline const IdbEntry &operator[](size_t n) const { return entries[n]; }
    inline const IdbEntry &operator[](size_t n) { return entries[n]; }

    inline std::vector<IdbEntry>::const_iterator begin() const { return entries.begin(); }
    inline std::vector<IdbEntry>::const_iterator end() const { return entries.end(); }

    inline std::vector<IdbEntry>::iterator begin() { return entries.begin(); }
    inline std::vector<IdbEntry>::iterator end() { return entries.end(); }
};

#endif // IDBFILE_H
