#ifndef PUPFILE_H
#define PUPFILE_H

#include "BaseFile.h"

#define PUP_SIGNATURE   0x50555023

#define PUP_CUSTOM_ID_START 0x80

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
} PACKED PUPHeader;

STATIC_ASSERT_STRUCT(PUPHeader, 0x10);

typedef struct
{
    uint32_t id; // 0
    uint32_t unk_04;
    uint32_t unk_08;
    uint32_t unk_0C;
    float hea; // 0x10
    float unk_14;
    float ki; // 0x18
    float ki_recovery;
    float stm; // 0x20
    float stamina_recovery; // 0x24
    float enemy_stamina_eraser; // 0x28
    float stamina_eraser; // 0x2C
    float unk_30;
    float atk; // 0x34
    float basic_ki_attack; // 0x38
    float str; // 0x3C
    float bla; // 0x40
    float atk_damage; // 0x44
    float ki_damage; // 0x48
    float str_damage; // 0x4C
    float bla_damage; // 0x50
    float ground_speed; // 0x54
    float air_speed; // 0x58
    float boosting_speed; // 0x5C
    float dash_speed; // 0x60
    float unk_64;
    float unk_68;
    float unk_6C;
    float unk_70;
    float unk_74;
    float unk_78;
    float unk_7C;
    float unk_80;
    float unk_84;
    float unk_88;
    float unk_8C;
    float unk_90;
    float unk_94;
} PACKED PUPEntry;

STATIC_ASSERT_STRUCT(PUPEntry, 0x98);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct PupEntry
{
    uint32_t id;
    uint32_t unk_04;
    uint32_t unk_08;
    uint32_t unk_0C;
    float hea;
    float unk_14;
    float ki;
    float ki_recovery;
    float stm;
    float stamina_recovery;
    float enemy_stamina_eraser;
    float stamina_eraser;
    float unk_30;
    float atk;
    float basic_ki_attack;
    float str;
    float bla;
    float atk_damage;
    float ki_damage;
    float str_damage;
    float bla_damage;
    float ground_speed;
    float air_speed;
    float boosting_speed;
    float dash_speed;
    float unk_64;
    float unk_68;
    float unk_6C;
    float unk_70;
    float unk_74;
    float unk_78;
    float unk_7C;
    float unk_80;
    float unk_84;
    float unk_88;
    float unk_8C;
    float unk_90;
    float unk_94;

    PupEntry()
    {
        id = unk_04 = unk_08 = unk_0C = 0xFFFFFFFF;
        hea = unk_14 = ki = ki_recovery = 0.0f;
        stm = stamina_recovery = enemy_stamina_eraser = stamina_eraser = 0.0f;
        unk_30 = atk = basic_ki_attack = str = 0.0f;
        bla = atk_damage = ki_damage = str_damage = bla_damage = 0.0f;
        ground_speed = air_speed = boosting_speed = dash_speed = 0.0f;
        unk_64 = unk_68 = unk_6C = 0.0f;
        unk_70 = unk_74 = unk_78 = unk_7C = 0.0f;
        unk_80 = unk_84 = unk_88 = unk_8C = 0.0f;
        unk_90 = unk_94 = 0.0f;
    }

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

class PupFile : public BaseFile
{
private:

    std::vector<PupEntry> entries;

protected:

    void Reset();

public:

    PupFile();
    virtual ~PupFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    inline size_t GetNumEntries() const { return entries.size(); }

    PupEntry *FindEntryByID(uint32_t id);

    bool AddEntry(PupEntry &entry);
    size_t RemoveEntry(uint32_t id);

    bool AddConsecutiveEntries(std::vector<PupEntry> &input_entries);

    inline const std::vector<PupEntry> &GetEntries() const { return entries; }
    inline std::vector<PupEntry> &GetEntries() { return entries; }

    inline const PupEntry &operator[](size_t n) const { return entries[n]; }
    inline PupEntry &operator[](size_t n) { return entries[n]; }

    inline std::vector<PupEntry>::const_iterator begin() const { return entries.begin(); }
    inline std::vector<PupEntry>::const_iterator end() const { return entries.end(); }

    inline std::vector<PupEntry>::iterator begin() { return entries.begin(); }
    inline std::vector<PupEntry>::iterator end() { return entries.end(); }
};

#endif // PUPFILE_H
