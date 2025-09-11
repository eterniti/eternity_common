#ifndef QXDFILE_H
#define QXDFILE_H
#include "CusFile.h"
// "#QXD"
#define QXD_SIGNATURE  0x44585123

// This enum matches the numeric code used in the game exe
enum QxdQuestType
{
    QUEST_TYPE_TPQ, // Main quest (includes legend patrol)
    QUEST_TYPE_TMQ, // Parallel quests
    QUEST_TYPE_BAQ, // Time rifts quests
    QUEST_TYPE_TCQ, // Teacher quests (includes the tests of the robot)
    QUEST_TYPE_HLQ, // Expert missions
    QUEST_TYPE_RBQ, // Raid quests
    QUEST_TYPE_CHQ, // Training quests (talk with old kai thing)
    QUEST_TYPE_LEQ, // Freezer siege quests
    QUEST_TYPE_TTQ, // Hero colosseum (main quest)
    QUEST_TYPE_TFB, // Hero colosseum (free battles)
    QUEST_TYPE_TNB, // Hero colosseum (bulma quests and battles with conton citizens)
    QUEST_TYPE_OSQ, // Fuu quests
    QUEST_TYPE_PRB, // Player Raid Boss
    QUEST_TYPE_PRD, // Crystal Raid
    QUEST_TYPE_RBD, // Extra Raid quests
    QUEST_TYPE_RBS, // Million Raid quest
    QUEST_TYPE_GBB, // Cross Versus
    QUEST_TYPE_EVT, // Festival of Universes

    NUM_QUEST_TYPES
};

// Logic of episode number and dialogue file loaded
// TPQ -> episode <= 13 -> qs_ep%02d(episode); episode == 99 -> qs_cep99; episode >= 19 &&  episode <= 60 -> qs_cep%02d(episode-19)
// Audio: if flag 0x40000 is set, root is TPQ/BDP_epXX_VOX, otherwise from TPQ/tpq_cepXX_VOX, where XX is calculated like following:
// episode < 19 -> XX = episode; episode == 99 -> X = 99; otherwise XX = episode - 19

// TMQ -> episode == 0 -> qe_athor, episode >= 1 && episode <= 13 -> qe_ep%02d(episode); episode == 99 -> qe_cep99; episode > 13 && episode <= 60 -> qe_cep%02d(episode-19);
// Audio: episode >= 19 CAQM_cepXX_vox, where XX is episode-19
// episode < 19: CBQM_epXX_vox where XX is episode

// BAQ -> always qb_battle. Audio from CABA_ALL_VOX
// TCQ -> always qt_battle. Audio from CAM_XXX_VOX, apparently the game obtains the XXX from whoever you talked to in the lobby.
// HLQ -> always qh_battle. Audio CAHQ_TRX_VOX.
// RBQ -> always qr_battle. no audio?
// CHQ -> always qch_battle. Audio CATQ_TOK_VOX. There are also individual XXX files loaded for the current teacher, but those cannot be used directly by normal Dialogue.
// LEQ -> always ql_battle. No audio.
// TTQ -> always ttq_battle. No audio.
// TFB -> always tfb_battle. No audio.
// TNB -> always tnb_battle. No audio.
// OSQ -> qo_%s(quest_name). Audio: CAOS_QN_VOX, where QN is quest_name

enum QxdUpdate
{
    QXD_UPDATE_ANY = 0,
    QXD_UPDATE_FIRST_RAID = 1,
    QXD_UPDATE_DLC1 = 2,
    QXD_UPDATE_DLC2 = 4,
    QXD_UPDATE_DLC3 = 8,
    QXD_UPDATE_DLC4 = 0x10,
    QXD_UPDATE_LEGEND_PATROL = 0x20,
    QXD_UPDATE_DLC5 = 0x40,
    QXD_UPDATE_HERO_COLOSSEUM = 0x80,
    QXD_UPDATE_DLC6 = 0x100,
    QXD_UPDATE_DLC7 = 0x200,
    QXD_UPDATE_DLC8 = 0x400,
    QXD_UPDATE_DLC9 = 0x800,
    QXD_UPDATE_DLC10 = 0x1000,
    QXD_UPDATE_EL0 = 0x2000,
    QXD_UPDATE_DLC11 = 0x4000,
    QXD_UPDATE_DLC11_RAID = 0x4001,
    QXD_UPDATE_DLC13 = 0x8000,
    QXD_UPDATE_DLC14 = 0x10000,
    QXD_UPDATE_DLC15 = 0x20000,
    QXD_UPDATE_CELL_MAX = 0x40000,
    QXD_UPDATE_DLC16 = 0x80000,
    QXD_UPDATE_GBB = 0x100000,
    QXD_UPDATE_DLC17 = 0x200000,
    QXD_UPDATE_DLC18 = 0x400000,
	QXD_UPDATE_DLC19 = 0X800000,
    QXD_UPDATE_DEVELOPER = 0x10000000,
};

enum QxdDlc
{
    QXD_DLC_NONE = 0,
    QXD_DLC_DLC1 = 1,
    QXD_DLC_DLC2 = 2,
    QXD_DLC_DLC3 = 4,
    QXD_DLC_DLC4 = 8,
    QXD_DLC_LEGEND_PATROL = 0x10,
    QXD_DLC_DLC5 = 0x20,
    QXD_DLC_DLC6 = 0x40,
    QXD_DLC_DLC7 = 0x80,
    QXD_DLC_DLC8 = 0x100,
    QXD_DLC_DLC9 = 0x200,
    QXD_DLC_DLC10 = 0x400,
    QXD_DLC_DLC11 = 0x800,
    QXD_DLC_DLC13 = 0x1000,
    QXD_DLC_DLC14 = 0x2000,
    QXD_DLC_DLC15 = 0x4000,
    QXD_DLC_DLC16 = 0x8000,
    QXD_DLC_DLC17 = 0x20000,
    QXD_DLC_DLC18 = 0x40000,
    QXD_DLC_DLC19 = 0x80000
};

// Matches order used by game exe
enum QxdSpecialChar
{
    QXD_PTN,
    QXD_AVB,
    QXD_MST,
    QXD_HUM,
    QXD_OWN,
    QXD_CPY,
    QXD_AVC,
    QXD_LPA,
    QXD_XEG,
    QXD_XEN
};

enum QxdItemType
{
    QXD_ITEM_TOP = 0,
    QXD_ITEM_BOTTOM = 1,
    QXD_ITEM_GLOVES = 2,
    QXD_ITEM_SHOES = 3,
    QXD_ITEM_ACCESSORY = 4,
    QXD_ITEM_SUPERSOUL = 5,
    QXD_ITEM_MATERIAL = 6, // material_item.idb
    QXD_ITEM_EXTRA = 7,
    QXD_ITEM_BATTLE = 8, // battle_item.idb
    QXD_ITEM_COLLECTION = 10, // Qxd collection
    QXD_ITEM_PET = 16,
    QXD_ITEM_ILLUSTRATION = 17,
    QXD_ITEM_TTL_OBJECT = 100,
    QXD_ITEM_TTL_SKILL = 101, // I guess id is one from those tdb files
    QXD_ITEM_TTL_FIGURE = 102, // I guess id is one from those tdb files
    // There is a 999 referenced in a collection in some .qxd, but the collection isn't used in any item reward
};

enum QxdSkillType
{
    QXD_SKILL_SUPER,
    QXD_SKILL_ULTIMATE,
    QXD_SKILL_EVASIVE,
    QXD_SKILL_BLAST,
    QXD_SKILL_AWAKEN
};

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED QXDHeader
{
    uint32_t signature;
    uint16_t endianess_check;
    uint16_t header_size;
    uint32_t num_quests; // 8
    uint32_t quests_start;  // 0xC
    uint32_t num_chars; // 0x10
    uint32_t chars_start; // 0x14
    uint32_t num_special_chars; // 0x18
    uint32_t special_chars_start; // 0x1C
    uint32_t num_collections_entries; // 0x20
    uint32_t collections_entries_start; // 0x24
    uint32_t unk_28;
    uint32_t floats_offset; // Points to the weirds floats
};
CHECK_STRUCT_SIZE(QXDHeader, 0x30);

struct PACKED QXDUnk
{
    uint16_t unk_00[16];

    QXDUnk()
    {
        memset(unk_00, 0, sizeof(unk_00));
    }

    inline bool operator==(const QXDUnk &rhs) const
    {
        return (memcmp(unk_00, rhs.unk_00, sizeof(unk_00)) == 0);
    }

    inline bool operator!=(const QXDUnk &rhs) const
    {
        return !(*this == rhs);
    }
};
CHECK_STRUCT_SIZE(QXDUnk, 0x20);

struct PACKED QXDCharPortrait
{
    uint16_t cms_id;
    uint16_t costume_index;
    uint16_t trans;

    QXDCharPortrait()
    {
        cms_id = trans = 0xFFFF;
        costume_index = 0;
    }

    inline bool IsDummy() const
    {
        return (cms_id == 0xFFFF && trans == 0xFFFF && costume_index == 0);
    }

    inline bool operator==(const QXDCharPortrait &rhs) const
    {
        if (cms_id != rhs.cms_id)
            return false;

        if (costume_index != rhs.costume_index)
            return false;

        if (trans != rhs.trans)
            return false;

        return true;
    }

    inline bool operator!=(const QXDCharPortrait &rhs) const
    {
        return !(*this == rhs);
    }
};
CHECK_STRUCT_SIZE(QXDCharPortrait, 0x6);

struct PACKED QXDItemReward
{
    uint32_t type; // 0, 1, 2, 3, 4, 5, 6, 8, 10, 101, 102
    uint32_t id;
    uint32_t condition;  // 0, 1, 4, 5
    uint32_t unk_0C;  //
    uint32_t flags; // 0, 2   0 = Show in description, 2 = don't show. Values 5 and 6 spotted since 1.11, in PRB quests.
    uint32_t unk_14; // 1-5
    float chance;
    uint32_t unk_1C; // Always 0

    QXDItemReward()
    {
        type = 0;
        id = 0xFFFFFFFF;
        condition = 0;
        unk_0C = 0;
        flags = 0;
        unk_14 = 1;
        chance = 0.0f;
        unk_1C = 0;
    }

    inline bool operator==(const QXDItemReward &rhs) const
    {
        if (type != rhs.type)
            return false;

        if (id != rhs.id)
            return false;

        if (condition != rhs.condition)
            return false;

        if (unk_0C != rhs.unk_0C)
            return false;

        if (flags != rhs.flags)
            return false;

        if (unk_14 != rhs.unk_14)
            return false;

        if (chance != rhs.chance)
            return false;

        if (unk_1C != rhs.unk_1C)
            return false;

        return true;
    }

    inline bool operator!=(const QXDItemReward &rhs) const
    {
        return !(*this == rhs);
    }
};
CHECK_STRUCT_SIZE(QXDItemReward, 0x20);

struct PACKED QXDSkillReward
{
    uint32_t type;
    uint32_t id2;
    uint32_t condition;
    uint32_t unk_0C;
    float chance;

    QXDSkillReward()
    {
        type = 0;
        id2 = 0xFFFFFFFF;
        condition = 0;
        unk_0C = 0;
        chance = 0.0f;
    }

    inline bool operator==(const QXDSkillReward &rhs) const
    {
        if (type != rhs.type)
            return false;

        if (id2 != rhs.id2)
            return false;

        if (condition != rhs.condition)
            return false;

        if (unk_0C != rhs.unk_0C)
            return false;

        if (chance != rhs.chance)
            return false;

        return true;
    }

    inline bool operator!=(const QXDSkillReward &rhs) const
    {
        return !(*this == rhs);
    }
};
CHECK_STRUCT_SIZE(QXDSkillReward, 0x14);

struct PACKED QXDCharReward
{
    char cms_name[4];
    uint16_t costume_index;
    uint16_t unk_06;

    QXDCharReward()
    {
        memset(cms_name, 0, sizeof(cms_name));
        costume_index = 0;
        unk_06 = 0;
    }

    inline bool operator==(const QXDCharReward &rhs) const
    {
        if (memcmp(cms_name, rhs.cms_name, sizeof(cms_name)) != 0)
            return false;

        if (costume_index != rhs.costume_index)
            return false;

        if (unk_06 != rhs.unk_06)
            return false;

        return true;
    }

    inline bool operator!=(const QXDCharReward &rhs) const
    {
        return !(*this == rhs);
    }
};
CHECK_STRUCT_SIZE(QXDCharReward, 8);

// Offsets are absolute
struct PACKED QXDQuest
{
    char name[16];
    uint32_t id; // 0x10
    uint32_t episode; // 0x14
    uint32_t sub_type; // 0x18
    uint32_t num_players; // 0x1C
    uint32_t num_msg_entries; // 0x20
    uint32_t msg_entries_offset; // 0x24
    uint16_t unk_28;
    uint16_t parent_quest; // 0x2A - (quest ID)
    uint16_t unk_2C;
    uint16_t unk_2E; // Found zero in all quest of all files
    uint32_t unk_30[4];
    uint32_t unlock_requirement; // 0x40
    uint32_t unk_44[5];
    uint32_t num_unk1; // 0x58 - num of 16 * 16-bit numbers array
    uint32_t unk1_offset; // 0x5C
    uint32_t num_unk2; // 0x60 - num of 16 * 16-bit numbers array
    uint32_t unk2_offset; // 0x64
    uint16_t time_limit; // 0x68
    uint16_t difficulty; // 0x6A
    uint32_t level; // 0x6C --- New in 1.21. Only used by TMQ, I think
    uint32_t unk_70; // 0x70 --- New in 1.21 As of 1.21, all values have been found to be zero
    uint16_t start_stage; // 0x74
    uint16_t start_demo; // 0x76   Only used in TPQ and OSQ quests
    uint32_t num_scripts; // 0x78
    uint32_t scripts_offset; // 0x7C
    uint32_t xp_reward; // 0x80
    uint32_t ult_xp_reward; // 0x84
    uint32_t fail_xp_reward; // 0x88
    uint32_t zeni_reward; // 0x8C
    uint32_t ult_zeni_reward; // 0x90
    uint32_t fail_zeni_reward; // 0x94
    uint32_t tp_medals_once; // 0x98
    uint32_t tp_medals; // 0x9C
    uint32_t tp_medals_special; // 0xA0
    uint32_t resistance_points; // 0xA4
    uint32_t num_item_rewards; // 0xA8
    uint32_t item_rewards_offset; // 0xAC
    uint32_t num_skill_rewards; // 0xB0
    uint32_t skill_rewards_offset; // 0xB4
    uint32_t num_chars_rewards; // 0xB8
    uint32_t chars_rewards_offset; // 0xBC
    uint32_t num_stages; // 0xC0 - Found to be 1 in 100% of cases
    uint32_t stages_offset; // 0xC4
    uint32_t unk_C8; // It could be a "has portraits" ? It is mostly 1, 0 in empty quests or in hero colisseum, and 2 in TCQ_ZMS_00
    QXDCharPortrait enemy_portraits[6]; // 0xCC
    uint16_t unk_F0[10];
    uint32_t flags; // 0x104
    uint32_t update_requirement; // 0x108
    uint32_t dlc_requirement; // 0x10C
    uint32_t unk_108; // 0x110
    uint16_t no_enemy_music; // 0x114
    uint16_t enemy_near_music; // 0x116
    uint16_t battle_music; // 0x118
    uint16_t ult_finish_music; // 0x11A
    float unk_11C;
    uint32_t unk_120; // 0 in everything but CHQ_0200, CHQ_0500, CHQ_0700, CHQ_1100
};
CHECK_STRUCT_SIZE(QXDQuest, 0x124);

// Backup of pre 1.21
/*struct PACKED QXDQuest
{
    char name[16];
    uint32_t id; // 0x10
    uint32_t episode; // 0x14
    uint32_t sub_type; // 0x18
    uint32_t num_players; // 0x1C
    uint32_t num_msg_entries; // 0x20
    uint32_t msg_entries_offset; // 0x24
    uint16_t unk_28;
    uint16_t parent_quest; // 0x2A - (quest ID)
    uint16_t unk_2C;
    uint16_t unk_2E; // Found zero in all quest of all files
    uint32_t unk_30[4];
    uint32_t unlock_requirement; // 0x40
    uint32_t unk_44[5];
    uint32_t num_unk1; // 0x58 - num of 16 * 16-bit numbers array
    uint32_t unk1_offset; // 0x5C
    uint32_t num_unk2; // 0x60 - num of 16 * 16-bit numbers array
    uint32_t unk2_offset; // 0x64
    uint16_t time_limit; // 0x68
    uint16_t difficulty; // 0x6A
    uint16_t start_stage; // 0x6C
    uint16_t start_demo; // 0x6E   Only used in TPQ and OSQ quests
    uint32_t num_scripts; // 0x70
    uint32_t scripts_offset; // 0x74
    uint32_t xp_reward; // 0x78
    uint32_t ult_xp_reward; // 0x7C
    uint32_t fail_xp_reward; // 0x80
    uint32_t zeni_reward; // 0x84
    uint32_t ult_zeni_reward; // 0x88
    uint32_t fail_zeni_reward; // 0x8C
    uint32_t tp_medals_once; // 0x90
    uint32_t tp_medals; // 0x94
    uint32_t tp_medals_special; // 0x98
    uint32_t resistance_points; // 0x9C
    uint32_t num_item_rewards; // 0xA0
    uint32_t item_rewards_offset; // 0xA4
    uint32_t num_skill_rewards; // 0xA8
    uint32_t skill_rewards_offset; // 0xAC
    uint32_t num_chars_rewards; // 0xB0
    uint32_t chars_rewards_offset; // 0xB4
    uint32_t num_stages; // 0xB8 - Found to be 1 in 100% of cases
    uint32_t stages_offset; // 0xBC
    uint32_t unk_C0; // It could be a "has portraits" ? It is mostly 1, 0 in empty quests or in hero colisseum, and 2 in TCQ_ZMS_00
    QXDCharPortrait enemy_portraits[6]; // 0xC4
    uint16_t unk_E8[10];
    uint32_t flags; // 0xFC
    uint32_t update_requirement; // 0x100
    uint32_t dlc_requirement; // 0x104
    uint32_t unk_108; // 0x108
    uint16_t no_enemy_music; // 0x10C
    uint16_t enemy_near_music; // 0x10E
    uint16_t battle_music; // 0x110
    uint16_t ult_finish_music; // 0x112
    float unk_114;
    uint32_t unk_118; // 0 in everything but CHQ_0200, CHQ_0500, CHQ_0700, CHQ_1100
};
CHECK_STRUCT_SIZE(QXDQuest, 0x11C);*/

struct PACKED QXDCharacter
{
    uint32_t id; // 0
    char cms_name[4]; // 4
    uint32_t costume; // 8
    uint32_t unk_0C;
    uint32_t level; // 0x10
    float health; // 0x14
    float unk_18;
    float ki; // 0x1C
    float stamina; // 0x20
    float basic_melee; // 0x24
    float ki_blast; // 0x28
    float strike_super; // 0x2C
    float ki_super; // 0x30
    float basic_melee_damage; // 0x34
    float ki_blast_damage; // 0x38
    float strike_super_damage; // 0x3C
    float ki_super_damage; // 0x40
    float unk_44;
    float unk_48;
    float air_speed; // 0x4C
    float boost_speed; // 0x50
    uint32_t ait_table_entry; // 0x54
    uint16_t skills[NUM_SKILL_SLOTS]; // 0x58
    uint16_t unk_6A[7]; //  [0] -> misc values (mostly 0 or -1), [1,3] -> only used in chars from HLQ and RBQ (exact decimal quantities) [2,4-6] -> always zero
    uint16_t transformation; // 0x78
    uint16_t special_effect; // 0x7A
    uint16_t unk_7C; // New in 1.21  Found to be always 0xFFFF(-1) except in characters from gbb_data.qxd, where it gets values from 100-105
    uint16_t unk_7E; // New in 1.21  As of 1.21, all entries in all .qxd files are zero.

    QXDCharacter()
    {
        id = 0xFFFFFFFF;
        memset(cms_name, 0, sizeof(cms_name));
        costume = 0;
        unk_0C = 0;
        level = 1;
        health = unk_18 = ki = stamina = basic_melee = ki_blast = strike_super = ki_super = -1.0f;
        basic_melee_damage = ki_blast_damage = strike_super_damage = ki_super_damage = -1.0f;
        unk_44 = unk_48 = air_speed = boost_speed = -1.0f;
        ait_table_entry = 0;
        memset(skills, 0xFF, sizeof(skills));
        memset(unk_6A, 0, sizeof(unk_6A));
        transformation = special_effect = 0xFFFF;
        unk_7C = 0xFFFF;
        unk_7E = 0;
    }

    bool ComparePartial(const QXDCharacter &rhs) const
    {
        if (memcmp(cms_name, rhs.cms_name, sizeof(cms_name)) != 0)
            return false;

        if (costume != rhs.costume)
            return false;

        if (unk_0C != rhs.unk_0C)
            return false;

        if (level != rhs.level)
            return false;

        if (health != rhs.health)
            return false;

        if (unk_18 != rhs.unk_18)
            return false;

        if (ki != rhs.ki)
            return false;

        if (stamina != rhs.stamina)
            return false;

        if (basic_melee != rhs.basic_melee)
            return false;

        if (ki_blast != rhs.ki_blast)
            return false;

        if (strike_super != rhs.strike_super)
            return false;

        if (ki_super != rhs.ki_super)
            return false;

        if (basic_melee_damage != rhs.basic_melee_damage)
            return false;

        if (ki_blast_damage != rhs.ki_blast_damage)
            return false;

        if (strike_super_damage != rhs.strike_super_damage)
            return false;

        if (ki_super_damage != rhs.ki_super_damage)
            return false;

        if (unk_44 != rhs.unk_44)
            return false;

        if (unk_48 != rhs.unk_48)
            return false;

        if (air_speed != rhs.air_speed)
            return false;

        if (boost_speed != rhs.boost_speed)
            return false;

        if (ait_table_entry != rhs.ait_table_entry)
            return false;

        if (memcmp(skills, rhs.skills, sizeof(skills)) != 0)
            return false;

        if (memcmp(unk_6A, rhs.unk_6A, sizeof(unk_6A)) != 0)
            return false;

        if (transformation != rhs.transformation)
            return false;

        if (special_effect != rhs.special_effect)
            return false;

        if (unk_7C != rhs.unk_7C)
            return false;

        if (unk_7E != rhs.unk_7E)
            return false;

        return true;
    }

    inline bool operator==(const QXDCharacter &rhs) const
    {
        if (id != rhs.id)
            return false;

        return ComparePartial(rhs);
    }

    inline bool operator!=(const QXDCharacter &rhs) const
    {
        return !(*this == rhs);
    }
};
CHECK_STRUCT_SIZE(QXDCharacter, 0x80);

struct PACKED QXDCollectionEntry
{
    uint16_t id; // 0
    uint16_t item_type; // 2
    uint16_t item_id; // 4
    uint16_t unk_06;
    uint16_t unk_08;
    uint16_t unk_0A;
};
CHECK_STRUCT_SIZE(QXDCollectionEntry, 0xC);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

typedef QXDUnk  QxdUnk;
typedef QXDItemReward QxdItemReward;
typedef QXDSkillReward QxdSkillReward;
typedef QXDCharReward QxdCharReward;
typedef QXDCharPortrait QxdCharPortrait;

struct QxdQuest
{
    std::string name;
    uint32_t id;
    uint32_t episode;
    uint32_t sub_type; // 0 -> normal, 1 -> dragon ball quest (a time machine is added automatically), 3 -> mind control, 4 -> scatter attack, 5 -> scatter attack too?
    uint32_t num_players;
    std::vector<std::string> msg_entries;
    uint16_t unk_28;
    uint16_t parent_quest; //(quest ID)
    uint16_t unk_2C;
    uint16_t unk_2E; // Found zero in all quest of all files
    uint32_t unk_30[4];
    uint32_t unlock_requirement; //  (quest ID)
    uint32_t unk_44[5];
    std::vector<QxdUnk> unk1s;
    std::vector<QxdUnk> unk2s;
    uint16_t time_limit;
    uint16_t difficulty;
    uint32_t level; // New in 1.21. Only used by TMQ, I think
    uint32_t unk_70; // New in 1.21 As of 1.21, all values have been found to be zero
    uint16_t start_stage;
    uint16_t start_demo; //  Only used in TPQ and OSQ quests. Does it work outside?
    std::vector<std::string> scripts;
    uint32_t xp_reward;
    uint32_t ult_xp_reward;
    uint32_t fail_xp_reward;
    uint32_t zeni_reward;
    uint32_t ult_zeni_reward;
    uint32_t fail_zeni_reward;
    uint32_t tp_medals_once;
    uint32_t tp_medals;
    uint32_t tp_medals_special;
    uint32_t resistance_points;
    std::vector<QxdItemReward> item_rewards;
    std::vector<QxdSkillReward> skill_rewards;
    std::vector<QxdCharReward> char_rewards;
    uint16_t stages[16];
    uint32_t unk_C8; // It could be a "has portraits" ? It is mostly 1, 0 in empty quests or in hero colisseum, and 2 in TCQ_ZMS_00
    QxdCharPortrait enemy_portraits[6];
    uint16_t unk_F0[10];
    uint32_t flags;
    uint32_t update_requirement;
    uint32_t dlc_requirement;
    uint32_t unk_108; // Only gets values of 0 and 1, may be a boolean. Usually 1 for main quests and for first OSQ
    uint16_t no_enemy_music;
    uint16_t enemy_near_music;
    uint16_t battle_music;
    uint16_t ult_finish_music;
    float unk_11C;
    uint32_t unk_120; // 0 in everything but CHQ_0200, CHQ_0500, CHQ_0700, CHQ_1100

    QxdQuest()
    {
        id = 0xFFFFFFFF;
        episode = 0;
        sub_type = 0;
        num_players = 1;
        unk_28 = 0;
        parent_quest = 0xFFFF;
        unk_2C = 0;
        unk_2E = 0;
        memset(unk_30, 0xFF, sizeof(unk_30));
        unlock_requirement = 0xFFFFFFFF;
        memset(unk_44, 0xFF, sizeof(unk_44));
        time_limit = 0;
        difficulty = 0;
        level = 0;
        unk_70 = 0;
        start_stage = 0;
        start_demo = 0;
        xp_reward = ult_xp_reward = fail_xp_reward = 0;
        zeni_reward = ult_zeni_reward = fail_zeni_reward = 0;
        tp_medals_once = tp_medals = tp_medals_special = 0;
        resistance_points = 0;
        memset(stages, 0xFF, sizeof(stages));
        unk_C8 = 0;
        memset(unk_F0, 0, sizeof(unk_F0));
        flags = 0;
        update_requirement = dlc_requirement = 0;
        unk_108 = 0;
        no_enemy_music = enemy_near_music = battle_music = ult_finish_music = 0;
        unk_11C = 0.0f;
        unk_120 = 0;
    }

    inline bool operator==(const QxdQuest &rhs) const
    {
        if (name != rhs.name)
            return false;

        if (id != rhs.id)
            return false;

        if (episode != rhs.episode)
            return false;

        if (sub_type != rhs.sub_type || num_players != rhs.num_players)
            return false;

        if (msg_entries != rhs.msg_entries)
            return false;

        if (unk_28 != rhs.unk_28)
            return false;

        if (parent_quest != rhs.parent_quest)
            return false;

        if (unk_2C != rhs.unk_2C || unk_2E != rhs.unk_2E)
            return false;

        if (memcmp(unk_30, rhs.unk_30, sizeof(unk_30)) != 0)
            return false;

        if (unk1s != rhs.unk1s || unk2s != rhs.unk2s)
            return false;

        if (time_limit != rhs.time_limit)
            return false;

        if (difficulty != rhs.difficulty)
            return false;

        if (level != rhs.level)
           return false;

        if (unk_70 != rhs.unk_70)
            return false;

        if (start_stage != rhs.start_stage)
            return false;

        if (start_demo != rhs.start_demo)
            return false;

        if (scripts != rhs.scripts)
            return false;

        if (xp_reward != rhs.xp_reward)
            return false;

        if (ult_xp_reward != rhs.ult_xp_reward)
            return false;

        if (fail_xp_reward != rhs.fail_xp_reward)
            return false;

        if (zeni_reward != rhs.zeni_reward)
            return false;

        if (ult_zeni_reward != rhs.ult_zeni_reward)
            return false;

        if (fail_zeni_reward != rhs.fail_zeni_reward)
            return false;

        if (tp_medals_once != rhs.tp_medals_once)
            return false;

        if (tp_medals != rhs.tp_medals)
            return false;

        if (tp_medals_special != rhs.tp_medals_special)
            return false;

        if (resistance_points != rhs.resistance_points)
            return false;

        if (item_rewards != rhs.item_rewards)
            return false;

        if (skill_rewards != rhs.skill_rewards)
            return false;

        if (char_rewards != rhs.char_rewards)
            return false;

        if (memcmp(stages, rhs.stages, sizeof(stages)) != 0)
            return false;

        if (unk_C8 != rhs.unk_C8)
            return false;

        for (int i = 0; i < 6; i++)
        {
            if (enemy_portraits[i] != rhs.enemy_portraits[i])
                return false;
        }

        if (memcmp(unk_F0, rhs.unk_F0, sizeof(unk_F0)) != 0)
            return false;

        if (flags != rhs.flags)
            return false;

        if (update_requirement != rhs.update_requirement || dlc_requirement != rhs.dlc_requirement)
            return false;

        if (unk_108 != rhs.unk_108)
            return false;

        if (no_enemy_music != rhs.no_enemy_music)
            return false;

        if (enemy_near_music != rhs.enemy_near_music)
            return false;

        if (battle_music != rhs.battle_music)
            return false;

        if (ult_finish_music != rhs.ult_finish_music)
            return false;

        if (unk_11C != rhs.unk_11C)
            return false;

        if (unk_120 != rhs.unk_120)
            return false;

        return true;
    }

    inline bool operator!=(const QxdQuest &rhs) const
    {
        return !(*this == rhs);
    }
};

typedef QXDCharacter QxdCharacter;

struct QxdCollectionEntry
{
    uint16_t item_type;
    uint16_t item_id;
    uint16_t unk_06;
    uint16_t unk_08;
    uint16_t unk_0A;

    QxdCollectionEntry()
    {
        item_type = 0;
        item_id = 0;
        unk_06 = unk_08 = unk_0A = 0;
    }

    inline bool operator==(const QxdCollectionEntry &rhs) const
    {
        if (item_type != rhs.item_type)
            return false;

        if (item_id != rhs.item_id)
            return false;

        if (unk_06 != rhs.unk_06)
            return false;

        if (unk_08 != rhs.unk_08)
            return false;

        if (unk_0A != rhs.unk_0A)
            return false;

        return true;
    }

    inline bool operator!=(const QxdCollectionEntry &rhs) const
    {
        return !(*this == rhs);
    }
};

struct QxdCollection
{
    uint16_t id;
    std::vector<QxdCollectionEntry> entries;

    QxdCollection()
    {
        id = 0xFFFF;
    }

    inline bool operator==(const QxdCollection &rhs) const
    {
        return (entries == rhs.entries);
    }

    inline bool operator!=(const QxdCollection &rhs) const
    {
        return !(*this == rhs);
    }
};

class QxdFile : public BaseFile
{
private:

    std::vector<QxdQuest> quests;
    std::vector<QxdCharacter> chars;
    std::vector<QxdCharacter> special_chars;
    std::vector<QxdCollection> collections;

    uint32_t unk_28;
    float unk_floats[8];

    static void ReadFixedStringArray(const char *start, std::vector<std::string> &array, uint32_t entry_size=32);

protected:

    void Reset();
    size_t CalculateQuestsSize(size_t *pmsg_size, size_t *punk1_size, size_t *punk2_size, size_t *pscripts_size,
                               size_t *pequip_rewards_size, size_t *pskill_rewards_size, size_t *pchar_rewards_size) const;

    uint32_t GetHighestQuestID() const;
    size_t GetNumCollectionEntries() const;

    uint32_t GetHighestCharId() const;

public:
    QxdFile();
    virtual ~QxdFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    inline size_t GetNumQuests() const { return quests.size(); }
    inline const QxdQuest &GetQuest(size_t idx) const { return quests[idx]; }
    inline QxdQuest &GetQuest(size_t idx) { return quests[idx]; }

    QxdQuest *FindQuestByName(const std::string &name);
    int GetQuestIndex(const std::string &name);
    QxdQuest *FindQuestById(uint32_t id);
    bool AddQuest(QxdQuest &quest, int new_id_search_start, int limit=0x7FFFFFFF);
    void RemoveQuest(uint32_t id, bool only_erase);

    inline size_t GetNumChars() const { return chars.size(); }
    inline const QxdCharacter &GetChar(size_t idx) const { return chars[idx]; }
    inline QxdCharacter &GetChar(size_t idx) { return chars[idx]; }

    QxdCharacter *FindCharById(uint32_t id);
    QxdCharacter *FindSimilarChar(const QxdCharacter &ch);

    bool AddCharWithId(const QxdCharacter &ch, bool overwrite);
    void AddCharWithNewId(QxdCharacter &ch, int new_id_search_start);

    inline size_t GetNumSpecialChars() const { return special_chars.size(); }
    inline const QxdCharacter &GetSpecialChar(size_t idx) const { return special_chars[idx]; }
    inline QxdCharacter &GetSpecialChar(size_t idx) { return special_chars[idx]; }

    QxdCharacter *FindSpecialCharById(uint32_t id);
    QxdCharacter *FindSimilarSpecialChar(const QxdCharacter &ch);

    bool AddSpecialCharWithId(const QxdCharacter &ch, bool overwrite);
    void AddSpecialCharWithNewId(QxdCharacter &ch, int new_id_search_start);

    size_t RemoveCharById(uint32_t id); // Removes either char or special char

    inline size_t GetNumCollections() const { return collections.size(); }
    inline const QxdCollection &GetCollection(size_t idx) const { return collections[idx]; }
    inline QxdCollection &GetCollection(size_t idx) { return collections[idx]; }

    QxdCollection *FindCollection(uint32_t id);
    size_t FindSimilarCollections(const QxdCollection &collection, std::vector<QxdCollection *> &result);

    bool AddCollection(QxdCollection &collection, bool auto_id, int new_id_search_start);
    size_t RemoveCollectionById(uint32_t id);

    inline const std::vector<QxdQuest> &GetQuests() const { return quests; }
    inline std::vector<QxdQuest> &GetQuests() { return quests; }

    inline const std::vector<QxdCharacter> &GetChars() const { return chars; }
    inline std::vector<QxdCharacter> &GetChars() { return chars; }

    inline const std::vector<QxdCharacter> &GetSpecialChars() const { return special_chars; }
    inline std::vector<QxdCharacter> &GetSpecialChars() { return special_chars; }

    inline bool operator==(const QxdFile &rhs) const
    {
        if (quests != rhs.quests)
            return false;

        if (chars != rhs.chars)
            return false;

        if (special_chars != rhs.special_chars)
            return false;

        if (collections != rhs.collections)
            return false;

        if (unk_28 != rhs.unk_28)
            return false;

        if (memcmp(unk_floats, rhs.unk_floats, sizeof(unk_floats)) != 0)
            return false;

        return true;
    }

    inline bool operator!=(const QxdFile &rhs) const
    {
        return !(*this == rhs);
    }
};

#endif // QXDFILE_H
