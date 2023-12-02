#ifndef BDMFILE_H
#define BDMFILE_H

// Note: only skill BDM supported. No support for chara, the shot/cmn, XV1 (511_GGT_BRK_PLAYER.shot.bdm) and whatever 671_JCO_SEC_PLAYER.bdm is.

#include "BaseFile.h"

#define BDM_SIGNATURE   0x4D444223
#define NUM_BDM_SUBENTRIES  10
#define NUM_BDM_EFFECTS 3

;
#pragma pack(push,1)

struct PACKED BDMHeader
{
    uint32_t signature; // 0
    uint16_t endianess_check;  // 4
    uint16_t unk_06;
    uint32_t num_entries; // 8
    uint32_t data_start; // 0xC

    BDMHeader()
    {
        signature = BDM_SIGNATURE;
        endianess_check = 0xFFFE;
        unk_06 = 0;
        num_entries = 0;
        data_start = sizeof(BDMHeader);
    }
};
CHECK_STRUCT_SIZE(BDMHeader, 0x10);

struct PACKED BDMEffect
{
    uint16_t effect_id;
    uint16_t skill_id; // 2
    uint16_t eepk_type; // 4
    uint16_t unk_06;
};
CHECK_STRUCT_SIZE(BDMEffect, 8);

struct PACKED BDMSubEntry
{
    uint16_t damage_type;
    uint16_t unk_02;
    uint16_t damage_amount; // 4
    uint16_t unk_06;
    float unk_08;
    uint16_t acb_type; // C
    uint16_t cue_id; // E
    BDMEffect effects[NUM_BDM_EFFECTS]; // 0x10
    float pushback_strength; // 0x28
    float pushback_acceleration; // 0x2C
    uint16_t user_stun; // 0x30
    uint16_t victim_stun; // 0x32
    uint16_t knockback_duration; // 0x34
    uint16_t knockback_recovery_time; // 0x36
    uint16_t knockback_impact_time; // 0x38;
    uint16_t unk_3A;
    float knockback_strength[3]; // 0x3C
    float knockback_dragY; // 0x48
    uint16_t unk_4C;
    uint16_t knockback_gravity_time; // 0x4E
    uint16_t victim_invincibility_time; // 0x50
    uint16_t unk_52;
    uint16_t transformation_type; // 0x54
    uint16_t aliment_type; // 0x56
    uint16_t unk_58[3];
    uint16_t damage_special; // 0x5E
    uint16_t unk_60[2];
    uint16_t stumble_type; // 0x64
    uint16_t damage_secondary_type; // 0x66
    uint16_t camera_shake_type; // 0x68
    uint16_t camera_shake_time; // 0x6A
    uint16_t user_bpe_id; // 0x6C
    uint16_t victim_bpe_id; // 0x6E
    uint16_t stamina_broken_override_bdm_id; // 0x70
    uint16_t Zvanish_enable_time; // 0x72
    uint16_t user_animation_time; // 0x74
    uint16_t victim_animation_time; // 0x76
    float user_animation_speed; // 0x78
    float victim_animation_speed; // 0x7C
};
CHECK_STRUCT_SIZE(BDMSubEntry, 0x80);

struct PACKED BDMEntry
{
    uint32_t id;
    BDMSubEntry sub_entries[NUM_BDM_SUBENTRIES]; // 4
};
CHECK_STRUCT_SIZE(BDMEntry, 0x504);

#pragma pack(pop)

class BdmFile : public BaseFile
{
private:

    std::vector<BDMEntry> entries;

protected:

    void Reset();

public:
    BdmFile();
    virtual ~BdmFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    size_t ChangeReferencesToSkill(uint16_t old_skill, uint16_t new_skill);
};

#endif // BDMFILE_H
