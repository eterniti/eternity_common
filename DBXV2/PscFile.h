#ifndef PSCFILE_H
#define PSCFILE_H

#include "BaseFile.h"

#define PSC_SIGNATURE   0x43535023

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t header_size; // 6
    uint32_t num_entries; // 8
    uint32_t unk_0C; // Always zero
    uint32_t num_configs; // 0x10 - Added in game version 1.13
} PACKED PSCHeader;
CHECK_STRUCT_SIZE(PSCHeader, 0x14);

typedef struct
{
    uint32_t char_id; // 0
    uint32_t num_specs; // 4
    uint32_t unk_08; // Always zero. It is used at runtime by the parser, to store the relative offset to the first PSCSpecEntry
} PACKED PSCEntry;

STATIC_ASSERT_STRUCT(PSCEntry, 0xC);

typedef struct
{
    uint32_t costume_id; // 0
    uint32_t costume_id2; // 4
    uint32_t camera_position; // 8
    uint32_t unk_0C;
    uint32_t unk_10;
    float health; // 0x14
    float unk_18;
    float ki; // 0x1C
    float ki_recharge; // 0x20
    uint32_t unk_24;
    uint32_t unk_28;
    uint32_t unk_2C;
    float stamina; // 0x30
    float stamina_recharge_move; // 0x34
    float stamina_recharge_air; // 0x38
    float stamina_recharge_ground; // 0x3C
    float stamina_drain_rate1; // 0x40 Found 0 in all entries
    float stamina_drain_rate2; // 0x44 Found 0 in all entries
    float unk_48; // Found 0 in all entries
    float basic_attack; // 0x4C
    float basic_ki_attack; // 0x50
    float strike_attack; // 0x54
    float ki_blast_super; // 0x58
    float basic_phys_defense; // 0x5C
    float basic_ki_defense; // 0x60
    float strike_atk_defense; // 0x64
    float super_ki_blast_defense; // 0x68
    float ground_speed; // 0x6C
    float air_speed; // 0x70
    float boosting_speed; // 0x74
    float dash_distance; // 0x78
    float unk_7C; // Found 0 in all entries
    float reinf_skill_duration; // Found 0 in all entries
    float unk_84; // Found 0 in all entries
    float revival_hp_amount; // 0x88
    float unk_8C;
    float reviving_speed; // 0x90 Found 0 in all entries
    uint32_t unk_94; // Found 0 in all entries
    uint32_t unk_98;
    uint32_t unk_9C; // Found 0 in all entries
    uint32_t unk_A0; // Found 0 in all entries
    uint32_t unk_A4; // Found 0 in all entries
    uint32_t unk_A8; // Found 0 in all entries
    uint32_t unk_AC; // Found 0 in all entries
    uint32_t unk_B0; // Found 0 in all entries
    uint32_t talisman;
    uint32_t unk_B8;
    uint32_t unk_BC;
    float unk_C0;
} PACKED PSCSpecEntry;

STATIC_ASSERT_STRUCT(PSCSpecEntry, 0xC4);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct PscSpecEntry
{
    uint32_t costume_id;
    uint32_t costume_id2;
    uint32_t camera_position;
    uint32_t unk_0C;
    uint32_t unk_10;
    float health;
    float unk_18;
    float ki;
    float ki_recharge;
    uint32_t unk_24;
    uint32_t unk_28;
    uint32_t unk_2C;
    float stamina;
    float stamina_recharge_move;
    float stamina_recharge_air;
    float stamina_recharge_ground;
    float stamina_drain_rate1;
    float stamina_drain_rate2;
    float unk_48;
    float basic_attack;
    float basic_ki_attack;
    float strike_attack;
    float ki_blast_super;
    float basic_phys_defense;
    float basic_ki_defense;
    float strike_atk_defense;
    float super_ki_blast_defense;
    float ground_speed;
    float air_speed;
    float boosting_speed;
    float dash_distance;
    float unk_7C;
    float reinf_skill_duration;
    float unk_84;
    float revival_hp_amount;
    float unk_8C;
    float reviving_speed;
    uint32_t unk_98;
    uint32_t talisman;
    uint32_t unk_B8;
    uint32_t unk_BC;
    float unk_C0;

    PscSpecEntry()
    {
        costume_id = costume_id2 = camera_position = unk_0C = unk_10 = 0;
        health = unk_18 = ki = ki_recharge = 0.0;
        unk_24 = unk_28 = unk_2C = 0;
        stamina = stamina_recharge_move = stamina_recharge_air = stamina_recharge_ground = 0.0;
        stamina_drain_rate1 = stamina_drain_rate2 = unk_48 = 0.0;
        basic_attack = basic_ki_attack = strike_attack = ki_blast_super = 0.0;
        basic_phys_defense = basic_ki_defense = strike_atk_defense = super_ki_blast_defense = 0.0;
        ground_speed = air_speed = boosting_speed = dash_distance = 0.0;
        unk_7C = reinf_skill_duration = unk_84 = revival_hp_amount = unk_8C = reviving_speed = unk_C0 = 0.0;
        unk_98 = unk_B8 = unk_BC = 0;
        talisman = 0xFFFFFFFF;
    }

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct PscEntry
{
    uint32_t char_id;
    std::vector<PscSpecEntry> specs;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

class PscFile : public BaseFile
{
private:

    //std::vector<PscEntry> entries;
    std::vector<std::vector<PscEntry>> configurations;

    bool CheckNumEntries() const;

    TiXmlElement *DecompileConfig(TiXmlNode *root, size_t cfg) const;
    bool CompileConfig(const TiXmlElement *root, size_t cfg);

protected:

    void Reset();

public:

    PscFile();
    virtual ~PscFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    inline size_t GetNumConfigs() const { return configurations.size(); }
    inline size_t GetNumEntries() const { return configurations[0].size(); }
    //size_t GetTotalSpecEntries() const;

    PscSpecEntry *FindSpecFromAbsolutePos(size_t pos, size_t cfg);
    //PscEntry *FindEntry(uint32_t char_id);
    size_t FindEntries(uint32_t char_id, std::vector<PscEntry *> &ret);

    PscSpecEntry *FindSpec(uint32_t char_id, uint32_t costume_id, size_t cfg);

    bool AddEntry(const PscEntry &entry, int cfg=-1);
    void RemoveEntry(uint32_t char_id);

    const std::vector<PscEntry> &GetEntries(size_t cfg) const { return configurations[cfg]; }
    std::vector<PscEntry> &GetEntries(size_t cfg) { return configurations[cfg]; }

    /*inline const PscEntry &operator[](size_t n) const { return entries[n]; }
    inline const PscEntry &operator[](size_t n) { return entries[n]; }

    inline std::vector<PscEntry>::const_iterator begin() const { return entries.begin(); }
    inline std::vector<PscEntry>::const_iterator end() const { return entries.end(); }

    inline std::vector<PscEntry>::iterator begin() { return entries.begin(); }
    inline std::vector<PscEntry>::iterator end() { return entries.end(); }*/
};

#endif // PSCFILE_H
