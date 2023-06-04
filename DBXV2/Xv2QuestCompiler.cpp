#include <algorithm>

#include "Xv2QuestCompiler.h"
#include "debug.h"

#define QSF_PATH    "data/system/QuestSort.qsf"

#define MAX_PQ  192
#define MAX_EQ  96

static const std::vector<int> start_new_quest_id_search =
{
    106,
    133,
    56,
    168,
    30,
    18,
    16,
    42,
    23,
    31,
    46,
    11,
    2
};

static const std::vector<int> start_new_qxd_char_id_search =
{
    102001,
    30352,
    382,
    810,
    251,
    251,
    21,
    121,
    238,
    106,
    3782,
    1061,
};

static const std::vector<int> start_new_col_id_search =
{
    3,
    20100,
    2,
    0,
    0,
    0,
    0,
    21,
    0,
    0,
    0,
    0,
    1001
};

static const std::unordered_set<ci_string, CIStrHash> reserved_kw =
{
    "Quest",
    "QXDUnk1",
    "QXDUnk2",
    "ItemReward",
    "SkillReward",
    "CharReward",
    "CharPortrait",
    "true",
    "false",
    "ItemCollection",
    "ItemCollectionEntry",
    "X2mMod",
    "QxdChar",
    "QxdSpecialChar",
    "QmlChar",
    "TextEntry",
    "TextAudioEntry",
    "Dialogue",
    "DialoguePart",
    "InteractiveDialogue",
    "EventDialogue",
    "CharPosition",
    "InteractiveCharPosition",
    "ItemPosition",
    "Script",
    "Event",
    "State",
    "Action",
    "Condition",
    "Flag",
    "StringVar",
};

static const std::unordered_set<ci_string, CIStrHash> all_types =
{
    "Quest",
    "QXDUnk1",
    "QXDUnk2",
    "ItemReward",
    "SkillReward",
    "CharReward",
    "CharPortrait",
    "ItemCollection",
    "ItemCollectionEntry",
    "X2mMod",
    "QxdChar",
    "QxdSpecialChar",
    "QmlChar",
    "TextEntry",
    "TextAudioEntry",
    "Dialogue",
    "DialoguePart",
    "InteractiveDialogue",
    "EventDialogue",
    "CharPosition",
    "InteractiveCharPosition",
    "ItemPosition",
    "Script",
    "Event",
    "State",
    "Action",
    "Condition",
    "Flag",
    "StringVar",
};

static const std::unordered_set<ci_string, CIStrHash> top_types =
{
    "Quest",
    "ItemCollection",
    "X2mMod",
    "QxdChar",
    "QxdSpecialChar",
    "QmlChar",
    "TextEntry",
    "TextAudioEntry",
    "Dialogue",
    "InteractiveDialogue",
    "EventDialogue",
    "CharPosition",
    "InteractiveCharPosition",
    "ItemPosition",
    "Script",
    "Flag",
    "StringVar",
};

static const std::vector<ci_string> x2m_mod_params =
{
    "name",
    "guid"
};

static const std::vector<int> x2m_mod_types =
{
    TOKEN_STRING,
    TOKEN_STRING
};

static const std::vector<ci_string> it_col_entry_params =
{
    "item",
    "type",
    "i6",
    "i8",
    "i10"
};

static const std::vector<int> it_col_entry_types =
{
    TOKEN_POLI_INTEGER_IDENTIFIER,
    TOKEN_POLI_INTEGER_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER
};


static const std::vector<ci_string> item_reward_params =
{
    "item",
    "type",
    "condition",
    "i12",
    "flags",
    "i20",
    "chance"
};

static const std::vector<int> item_reward_types =
{
    TOKEN_POLI_INTEGER_IDENTIFIER,
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_POLI_INTEGER_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_FLOAT
};

static const std::vector<ci_string> skill_reward_params =
{
    "skill",
    "condition",
    "i12",
    "chance"
};

static const std::vector<int> skill_reward_types =
{
    TOKEN_POLI_INTEGER_STRING_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_FLOAT
};

static const std::vector<ci_string> char_reward_params =
{
    "char",
    "costume",
    "i6"
};

static const std::vector<int> char_reward_types =
{
    TOKEN_POLI_INTEGER_STRING_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_INTEGER
};

static const std::vector<ci_string> char_portrait_params =
{
    "char",
    "costume",
    "trans"
};

static const std::vector<int> char_portrait_types =
{
    TOKEN_POLI_INTEGER_STRING_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_INTEGER
};

static const std::vector<ci_string> quest_params =
{
    "episode",
    "sub_type",
    "num_players",

    "title",
    "success",
    "failure",
    "outline",
    "warning",
    "ex_success",

    "i40",
    "parent_quest",
    "i44",
    "i48",
    "i52",
    "i56",
    "i60",
    "unlock_requirement",
    "i68",
    "i72",
    "i76",
    "i80",
    "i84",

    "time_limit",
    "difficulty",
    "start_stage",
    "start_demo",

    "xp_reward",
    "ult_xp_reward",
    "fail_xp_reward",
    "zeni_reward",
    "ult_zeni_reward",
    "fail_zeni_reward",
    "tp_medals_once",
    "tp_medals",
    "tp_medals_special",
    "resistance_points",

    "stages",
    "i192",

    "i232",
    "i234",
    "i236",
    "i238",
    "i240",
    "i242",
    "i244",
    "i246",
    "i248",
    "i250",
    "flags",

    "update_requirement",
    "dlc_requirement",

    "i264",
    "no_enemy_bgm",
    "enemy_near_bgm",
    "battle_bgm",
    "ultimate_finish_bgm",
    "f276",
    "i280",
};

static const std::vector<int> quest_types =
{
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,

    TOKEN_POLI_STRING_IDENTIFIER,
    TOKEN_POLI_STRING_IDENTIFIER,
    TOKEN_POLI_STRING_IDENTIFIER,
    TOKEN_POLI_STRING_IDENTIFIER,
    TOKEN_POLI_STRING_IDENTIFIER,
    TOKEN_POLI_STRING_IDENTIFIER,

    TOKEN_INTEGER,
    TOKEN_POLI_INTEGER_STRING, // TODO: quest x2m mod (replace with TOKEN_POLI_INTEGER_STRING_IDENTIFIER
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_POLI_INTEGER_STRING, // TODO: quest x2m mod (replace with TOKEN_POLI_INTEGER_STRING_IDENTIFIER
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,

    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_POLI_INTEGER_STRING_IDENTIFIER,
    TOKEN_INTEGER,

    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,

    TOKEN_GROUP,
    TOKEN_INTEGER,

    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,    

    TOKEN_IDENTIFIER,
    TOKEN_IDENTIFIER,

    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_INTEGER
};

static const std::vector<ci_string> qxd_char_params =
{
    "char",
    "costume",
    "transformation",
    "special_effect",

    "i12",
    "level",
    "health",
    "f24",
    "ki",
    "stamina",
    "atk",
    "ki_atk",
    "super_atk",
    "super_ki",
    "atk_damage",
    "ki_damage",
    "super_atk_damage",
    "super_ki_damage",
    "guard_atk",
    "guard_damage",
    "move_speed",
    "boost_speed",
    "ait_table_entry",

    "super1",
    "super2",
    "super3",
    "super4",
    "ultimate1",
    "ultimate2",
    "evasive",
    "blast",
    "awaken",

    "i106",
    "i108",
    "i112"
};

static const std::vector<int> qxd_char_types =
{
    TOKEN_POLI_INTEGER_STRING_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,

    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_FLOAT,
    TOKEN_FLOAT,
    TOKEN_FLOAT,
    TOKEN_FLOAT,
    TOKEN_FLOAT,
    TOKEN_FLOAT,
    TOKEN_FLOAT,
    TOKEN_FLOAT,
    TOKEN_FLOAT,
    TOKEN_FLOAT,
    TOKEN_FLOAT,
    TOKEN_FLOAT,
    TOKEN_FLOAT,
    TOKEN_FLOAT,
    TOKEN_FLOAT,
    TOKEN_INTEGER,

    TOKEN_POLI_INTEGER_IDENTIFIER,
    TOKEN_POLI_INTEGER_IDENTIFIER,
    TOKEN_POLI_INTEGER_IDENTIFIER,
    TOKEN_POLI_INTEGER_IDENTIFIER,
    TOKEN_POLI_INTEGER_IDENTIFIER,
    TOKEN_POLI_INTEGER_IDENTIFIER,
    TOKEN_POLI_INTEGER_IDENTIFIER,
    TOKEN_POLI_INTEGER_IDENTIFIER,
    TOKEN_POLI_INTEGER_IDENTIFIER,

    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER
};

static const std::vector<ci_string> qml_char_params =
{
    "battle_index",
    "i12",
    "stage",
    "spawn_at_start",

    "ai",
    "team",

    "i36",
    "i40",
    "i44",
    "i48",
    "i50",
    "i52",
    "i56",

    "super1",
    "super2",
    "super3",
    "super4",
    "ultimate1",
    "ultimate2",
    "evasive",
    "blast",
    "awaken"
};

static const std::vector<int> qml_char_types =
{
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_POLI_INTEGER_STRING_IDENTIFIER,
    TOKEN_BOOLEAN,

    TOKEN_IDENTIFIER,
    TOKEN_IDENTIFIER,

    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,

    TOKEN_POLI_INTEGER_STRING_IDENTIFIER,
    TOKEN_POLI_INTEGER_STRING_IDENTIFIER,
    TOKEN_POLI_INTEGER_STRING_IDENTIFIER,
    TOKEN_POLI_INTEGER_STRING_IDENTIFIER,
    TOKEN_POLI_INTEGER_STRING_IDENTIFIER,
    TOKEN_POLI_INTEGER_STRING_IDENTIFIER,
    TOKEN_POLI_INTEGER_STRING_IDENTIFIER,
    TOKEN_POLI_INTEGER_STRING_IDENTIFIER,
    TOKEN_POLI_INTEGER_STRING_IDENTIFIER
};

static const std::vector<ci_string> dialogue_part_params =
{
    "actor",
    "costume",
    "transformation",
    "text_audio",
    "i8",
    "debug_actor",
    "special_case_osq_0301",
    "out_of_order",
    "dont_change",
    "continue_next"
};

static const std::vector<int> dialogue_part_types =
{
    TOKEN_POLI_INTEGER_STRING_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_POLI_STRING_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_POLI_INTEGER_STRING_IDENTIFIER,
    TOKEN_BOOLEAN,
    TOKEN_INTEGER,
    TOKEN_BOOLEAN,
    TOKEN_BOOLEAN
};

static const std::vector<ci_string> dialogue_params =
{
    "i28",
    "debug_actor",
    "b_use_i2"
};

static const std::vector<int> dialogue_types =
{
    TOKEN_INTEGER,
    TOKEN_POLI_INTEGER_STRING_IDENTIFIER,
    TOKEN_BOOLEAN
};

static const std::vector<ci_string> interactive_dialogue_params =
{
    "interaction_type",
    "param",
    "item_type",

    "i0",
    "debug_actor",
    "b_use_i2"
};

static const std::vector<int> interactive_dialogue_types =
{
    TOKEN_POLI_INTEGER_IDENTIFIER,
    TOKEN_POLI_INTEGER_IDENTIFIER,
    TOKEN_IDENTIFIER,

    TOKEN_INTEGER,
    TOKEN_POLI_INTEGER_STRING_IDENTIFIER,
    TOKEN_BOOLEAN
};

static const std::vector<ci_string> event_dialogue_params =
{
    "devent",
    "on_event_end",
    "stage",

    "i12",
    "i16",
    "i34",
    "i36",
    "i44",
    "i48"
};

static const std::vector<int> event_dialogue_types =
{
    TOKEN_POLI_INTEGER_IDENTIFIER,
    TOKEN_BOOLEAN,
    TOKEN_POLI_INTEGER_STRING_IDENTIFIER,

    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER
};

static const std::vector<ci_string> char_position_params =
{
    "char",
    "stage",
    "position",
    "type",
    "i40",
    "i50"
};

static const std::vector<int> char_position_types =
{
    TOKEN_POLI_INTEGER_IDENTIFIER,
    TOKEN_POLI_INTEGER_STRING_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER
};

static const std::vector<ci_string> ichar_position_params =
{
    "char",
    "stage",
    "position",

    "idialogue",
    "change_to",
    "i38",

    "default_pose",
    "talk_pose",
    "effect_pose",
    "i50"
};

static const std::vector<int> ichar_position_types =
{
    TOKEN_POLI_INTEGER_IDENTIFIER,
    TOKEN_POLI_INTEGER_STRING_IDENTIFIER,
    TOKEN_STRING,

    TOKEN_POLI_INTEGER_IDENTIFIER,
    TOKEN_POLI_INTEGER_IDENTIFIER,
    TOKEN_INTEGER,

    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER,
    TOKEN_INTEGER
};

static const std::vector<ci_string> item_position_params =
{
    "item_collection",
    "stage",
    "position",
    "chance"
};

static const std::vector<int> item_position_types =
{
    TOKEN_IDENTIFIER,
    TOKEN_POLI_INTEGER_STRING_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_INTEGER
};

static const std::unordered_map<std::string, int > quest_chars =
{
    { "PTN", QXD_PTN },
    { "AVB", QXD_AVB },
    { "MST", QXD_MST },
    { "HUM", QXD_HUM },
    { "OWN", QXD_OWN },
    { "CPY", QXD_CPY },
    { "AVC", QXD_AVC },
    { "LPA", QXD_LPA },
    { "XEG", QXD_XEG },
    { "XEN", QXD_XEN }
};

static const std::unordered_map<int, std::string> quest_chars_description =
{
    { QXD_PTN, "Partner" },
    { QXD_AVB, "Random Avatar"},
    { QXD_MST, "Teacher" },
    { QXD_HUM, "Player" },
    { QXD_OWN, "Player Clone" },
    { QXD_CPY, "Controlled Player" },
    { QXD_AVC, "Random Avatar" },
    { QXD_LPA, "[Lobby char]"},
    { QXD_XEG, "XV1 Hero (time breaker)"},
    { QXD_XEN, "XV1 Hero" }
};

static const std::unordered_map<int, std::string> item_type_to_constant =
{
    { QXD_ITEM_TOP, "TOP" },
    { QXD_ITEM_BOTTOM, "BOTTOM" },
    { QXD_ITEM_GLOVES, "GLOVES" },
    { QXD_ITEM_SHOES, "SHOES" },
    { QXD_ITEM_ACCESSORY, "ACCESSORY" },
    { QXD_ITEM_SUPERSOUL, "SUPERSOUL" },
    { QXD_ITEM_MATERIAL, "MATERIAL" },
    { QXD_ITEM_EXTRA, "EXTRA" },
    { QXD_ITEM_BATTLE, "BATTLE" },
    { QXD_ITEM_PET, "PET" },
    { QXD_ITEM_ILLUSTRATION, "ILLUSTRATION" },
    { QXD_ITEM_COLLECTION, "COLLECTION" },
    { QXD_ITEM_TTL_OBJECT, "HC_OBJECT" },
    { QXD_ITEM_TTL_SKILL, "HC_SKILL" },
    { QXD_ITEM_TTL_FIGURE, "HC_FIGURE" }
};

static const std::unordered_map<ci_string, int, CIStrHash> constant_to_item_type =
{
    { "TOP", QXD_ITEM_TOP },
    { "BUST", QXD_ITEM_TOP },
    { "BOTTOM", QXD_ITEM_BOTTOM },
    { "PANTS", QXD_ITEM_BOTTOM },
    { "GLOVES", QXD_ITEM_GLOVES },
    { "RIST", QXD_ITEM_GLOVES },
    { "SHOES", QXD_ITEM_SHOES },
    { "BOOTS", QXD_ITEM_SHOES },
    { "ACCESSORY", QXD_ITEM_ACCESSORY },
    { "ACCESORY", QXD_ITEM_ACCESSORY },
    { "SUPERSOUL", QXD_ITEM_SUPERSOUL },
    { "SUPER_SOUL", QXD_ITEM_SUPERSOUL },
    { "TALISMAN", QXD_ITEM_SUPERSOUL },
    { "MATERIAL", QXD_ITEM_MATERIAL },
    { "EXTRA", QXD_ITEM_EXTRA },
    { "BATTLE", QXD_ITEM_BATTLE },
    { "PET", QXD_ITEM_PET },
    { "ILLUSTRATION", QXD_ITEM_ILLUSTRATION },
    { "COLLECTION", QXD_ITEM_COLLECTION },
    { "HC_OBJECT", QXD_ITEM_TTL_OBJECT },
    { "HC_SKILL", QXD_ITEM_TTL_SKILL },
    { "HC_FIGURE", QXD_ITEM_TTL_FIGURE }
};

static const std::unordered_map<int, std::string> update_to_constant =
{
    { QXD_UPDATE_ANY, "ANY" },
    { QXD_UPDATE_FIRST_RAID, "FIRST_RAID" },
    { QXD_UPDATE_DLC1, "DLC1" },
    { QXD_UPDATE_DLC2, "DLC2" },
    { QXD_UPDATE_DLC3, "DLC3" },
    { QXD_UPDATE_DLC4, "DLC4" },
    { QXD_UPDATE_LEGEND_PATROL, "LEGEND_PATROL" },
    { QXD_UPDATE_DLC5, "DLC5" },
    { QXD_UPDATE_HERO_COLOSSEUM, "HERO_COLOSSEUM" },
    { QXD_UPDATE_DLC6, "DLC6" },
    { QXD_UPDATE_DLC7, "DLC7" },
    { QXD_UPDATE_DLC8, "DLC8" },
    { QXD_UPDATE_DLC9, "DLC9" },
    { QXD_UPDATE_DLC10, "DLC10" },
    { QXD_UPDATE_EL0, "EL0" },
    { QXD_UPDATE_DLC11, "DLC11" },
    { QXD_UPDATE_DLC11_RAID, "DLC11_RAID" },
    { QXD_UPDATE_DLC13, "DLC13" },
    { QXD_UPDATE_DLC14, "DLC14" },
    { QXD_UPDATE_DLC15, "DLC15" },
    { QXD_UPDATE_CELL_MAX, "CELL_MAX" },
    { QXD_UPDATE_DLC16, "DLC16" },
    { QXD_UPDATE_DEVELOPER, "DEVELOPER" }
};

static const std::unordered_map<ci_string, int, CIStrHash> constant_to_update =
{
    { "ANY", QXD_UPDATE_ANY },
    { "FIRST_RAID", QXD_UPDATE_FIRST_RAID },
    { "DLC1", QXD_UPDATE_DLC1 },
    { "DLC2", QXD_UPDATE_DLC2 },
    { "DLC3", QXD_UPDATE_DLC3 },
    { "DLC4", QXD_UPDATE_DLC4 },
    { "LEGEND_PATROL", QXD_UPDATE_LEGEND_PATROL },
    { "DLC5", QXD_UPDATE_DLC5 },
    { "HERO_COLOSSEUM", QXD_UPDATE_HERO_COLOSSEUM },
    { "DLC6", QXD_UPDATE_DLC6 },
    { "DLC7", QXD_UPDATE_DLC7 },
    { "DLC8", QXD_UPDATE_DLC8 },
    { "DLC9", QXD_UPDATE_DLC9 },
    { "DLC10", QXD_UPDATE_DLC10 },
    { "EL0", QXD_UPDATE_EL0 },
    { "DLC11", QXD_UPDATE_DLC11 },
    { "DLC11_RAID", QXD_UPDATE_DLC11_RAID },
    { "DLC13", QXD_UPDATE_DLC13 },
    { "DLC14", QXD_UPDATE_DLC14 },
    { "DLC15", QXD_UPDATE_DLC15 },
    { "CELL_MAX", QXD_UPDATE_CELL_MAX },
    { "DLC16", QXD_UPDATE_DLC16 },
    { "DEVELOPER", QXD_UPDATE_DEVELOPER }
};

static const std::unordered_map<int, std::string> dlc_to_constant =
{
    { QXD_DLC_NONE, "NONE" },
    { QXD_DLC_DLC1, "DLC1" },
    { QXD_DLC_DLC2, "DLC2" },
    { QXD_DLC_DLC3, "DLC3" },
    { QXD_DLC_DLC4, "DLC4" },
    { QXD_DLC_LEGEND_PATROL, "LEGEND_PATROL" },
    { QXD_DLC_DLC5, "DLC5" },
    { QXD_DLC_DLC6, "DLC6" },
    { QXD_DLC_DLC7, "DLC7" },
    { QXD_DLC_DLC8, "DLC8" },
    { QXD_DLC_DLC9, "DLC9" },
    { QXD_DLC_DLC10, "DLC10" },
    { QXD_DLC_DLC11, "DLC11" },
    { QXD_DLC_DLC13, "DLC13" },
    { QXD_DLC_DLC14, "DLC14" },
    { QXD_DLC_DLC15, "DLC15" },
    { QXD_DLC_DLC16, "DLC16" },
};

static const std::unordered_map<ci_string, int, CIStrHash> constant_to_dlc =
{
    { "NONE", QXD_DLC_NONE },
    { "DLC1", QXD_DLC_DLC1 },
    { "DLC2", QXD_DLC_DLC2 },
    { "DLC3", QXD_DLC_DLC3 },
    { "DLC4", QXD_DLC_DLC4 },
    { "LEGEND_PATROL", QXD_DLC_LEGEND_PATROL },
    { "DLC5", QXD_DLC_DLC5 },
    { "DLC6", QXD_DLC_DLC6 },
    { "DLC7", QXD_DLC_DLC7 },
    { "DLC8", QXD_DLC_DLC8 },
    { "DLC9", QXD_DLC_DLC9 },
    { "DLC10", QXD_DLC_DLC10 },
    { "DLC11", QXD_DLC_DLC11 },
    { "DLC13", QXD_DLC_DLC13 },
    { "DLC14", QXD_DLC_DLC14 },
    { "DLC15", QXD_DLC_DLC15 },
    { "DLC16", QXD_DLC_DLC16 },
};

static const std::unordered_map<int, std::string> ai_to_constant =
{
    { QML_AI_HUMAN, "HUMAN" },
    { QML_AI_STILL,  "STILL" },
    { QML_AI_NORMAL, "NORMAL" },
    { 3, "TYPE3" },
    { 4, "TYPE4" },
    { 5, "TYPE5" }
};

static const std::unordered_map<ci_string, int, CIStrHash> constant_to_ai =
{
    { "HUMAN", QML_AI_HUMAN },
    { "STILL", QML_AI_STILL },
    { "NORMAL", QML_AI_NORMAL },
    { "TYPE3", 3 },
    { "TYPE4", 4 },
    { "TYPE5", 5 }
};

static const std::unordered_map<int, std::string> team_to_constant =
{
    { QML_TEAM_NONE, "NONE" },
    { QML_TEAM_A, "A" },
    { QML_TEAM_B, "B" }
};

static const std::unordered_map<ci_string, int, CIStrHash>  constant_to_team =
{
    { "NONE", QML_TEAM_NONE },
    { "A", QML_TEAM_A },
    { "B", QML_TEAM_B }
};

static const std::unordered_map<int, std::string> interaction_type_to_constant =
{
    { QBT_ITYPE_DEFAULT, "DEFAULT" },
    { QBT_ITYPE_EFFECT, "EFFECT" },
    { QBT_ITYPE_GIVE_ITEM, "GIVE_ITEM" },
    { QBT_ITYPE_JOIN, "JOIN "}
};

static const std::unordered_map<ci_string, int, CIStrHash> constant_to_interaction_type =
{
    { "DEFAULT", QBT_ITYPE_DEFAULT },
    { "EFFECT", QBT_ITYPE_EFFECT },
    { "GIVE_ITEM", QBT_ITYPE_GIVE_ITEM },
    { "JOIN", QBT_ITYPE_JOIN }
};

static const std::unordered_map<int, std::string> dialogue_event_to_constant =
{
    { QBT_EVENT_HYPNO_ALLY, "HYPNO_ALLY" },
    { QBT_EVENT_HYPNO_ATTACK, "HYPNO_ATTACK" },
    { QBT_EVENT_GIANT_KI_BLAST, "GIANT_KI_BLAST" },
    { QBT_EVENT_GIANT_KI_BLAST_RETURNED, "GIANT_KI_BLAST_RETURNED" },
    { QBT_EVENT_AREA_CHANGE, "AREA_CHANGE" },
    { QBT_EVENT_CONTROLLED_ALLY_DEFEATED, "CONTROLLED_ALLY_DEFEATED" },
    { QBT_EVENT_TELEPORT_END, "TELEPORT_END" },
    { QBT_EVENT_CRYSTALS_DESTROYED, "CRYSTALS_DESTROYED" },
    { QBT_EVENT_TELEPORT_ATTACK_SUCCEDED, "TELEPORT_ATTACK_SUCCEEDED" },
    { QBT_EVENT_TELEPORT_ATTACK_FAILED, "TELEPORT_ATTACK_FAILED" }
};

static const std::unordered_map<ci_string, int, CIStrHash> constant_to_dialogue_event =
{
    { "HYPNO_ALLY", QBT_EVENT_HYPNO_ALLY },
    { "HYPNO_ATTACK", QBT_EVENT_HYPNO_ATTACK },
    { "GIANT_KI_BLAST", QBT_EVENT_GIANT_KI_BLAST },
    { "GIANT_KI_BLAST_RETURNED", QBT_EVENT_GIANT_KI_BLAST_RETURNED },
    { "AREA_CHANGE", QBT_EVENT_AREA_CHANGE },
    { "CONTROLLED_ALLY_DEFEATED", QBT_EVENT_CONTROLLED_ALLY_DEFEATED },
    { "TELEPORT_END", QBT_EVENT_TELEPORT_END },
    { "CRYSTALS_DESTROYED", QBT_EVENT_CRYSTALS_DESTROYED },
    { "TELEPORT_ATTACK_SUCCEEDED", QBT_EVENT_TELEPORT_ATTACK_SUCCEDED },
    { "TELEPORT_ATTACK_FAILED", QBT_EVENT_TELEPORT_ATTACK_FAILED }
};

static const std::unordered_map<int, std::string> partner_to_constant =
{
    { 0, "PARTNER_KRILLIN" },
    { 1, "PARTNER_TIEN" },
    { 2, "PARTNER_YAMCHA" },
    { 3, "PARTNER_PICCOLO" },
    { 4, "PARTNER_RADITZ" },
    { 5, "PARTNER_GOHAN_KID" },
    { 6, "PARTNER_NAPPA" },
    { 7, "PARTNER_VEGETA" },
    { 8, "PARTNER_ZARBON" },
    { 9, "PARTNER_DODORIA" },
    { 10, "PARTNER_GINYU" },
    { 11, "PARTNER_FRIEZA" },
    { 12, "PARTNER_ANDROID18" },
    { 13, "PARTNER_CELL" },
    { 14, "PARTNER_LORD_SLUG" },
    { 15, "PARTNER_MAJIN_BUU" },
    { 16, "PARTNER_MR_SATAN" },
    { 17, "PARTNER_GOHAN" },
    { 18, "PARTNER_GOTENKS" },
    { 19, "PARTNER_TURLES" },
    { 20, "PARTNER_BROLY" },
    { 21, "PARTNER_BEERUS" },
    { 22, "PARTNER_PAN" },
    { 23, "PARTNER_JACO" },
    { 24, "PARTNER_GOKU" },
    { 25, "PARTNER_WHIS" },
    { 26, "PARTNER_COOLER" },
    { 27, "PARTNER_ANDROID16" },
    { 28, "PARTNER_FUTURE_GOHAN" },
    { 29, "PARTNER_BARDOCK" },
    { 30, "PARTNER_HIT" },
    { 31, "PARTNER_BOJACK" },
    { 32, "PARTNER_ZAMASU" },
    { 45, "PARTNER_VIDEL" },
    { 46, "PARTNER_FUU" }
};

static const std::unordered_map<ci_string, int, CIStrHash> constant_to_partner =
{
    { "PARTNER_KRILLIN", 0 },
    { "PARTNER_TIEN", 1 },
    { "PARTNER_YAMCHA", 2 },
    { "PARTNER_PICCOLO", 3 },
    { "PARTNER_RADITZ", 4 },
    { "PARTNER_GOHAN_KID", 5 },
    { "PARTNER_NAPPA", 6 },
    { "PARTNER_VEGETA", 7 },
    { "PARTNER_ZARBON", 8 },
    { "PARTNER_DODORIA", 9 },
    { "PARTNER_GINYU", 10 },
    { "PARTNER_FRIEZA", 11 },
    { "PARTNER_ANDROID18", 12 },
    { "PARTNER_CELL", 13 },
    { "PARTNER_LORD_SLUG", 14 },
    { "PARTNER_MAJIN_BUU", 15 },
    { "PARTNER_MR_SATAN", 16 },
    { "PARTNER_GOHAN", 17 },
    { "PARTNER_GOTENKS", 18 },
    { "PARTNER_TURLES", 19 },
    { "PARTNER_BROLY", 20 },
    { "PARTNER_BEERUS", 21 },
    { "PARTNER_PAN", 22 },
    { "PARTNER_JACO", 23 },
    { "PARTNER_GOKU", 24 },
    { "PARTNER_WHIS", 25 },
    { "PARTNER_COOLER", 26 },
    { "PARTNER_ANDROID16", 27 },
    { "PARTNER_FUTURE_GOHAN", 28 },
    { "PARTNER_BARDOCK", 29 },
    { "PARTNER_HIT", 30 },
    { "PARTNER_BOJACK", 31 },
    { "PARTNER_ZAMASU", 32 },
    { "PARTNER_VIDEL", 45 },
    { "PARTNER_FUU", 46 }
};

static const std::unordered_map<int, std::string> race_to_constant =
{
    { 0, "HUMAN" },
    { 1, "SAIYAN" },
    { 2, "NAMEKIAN" },
    { 3, "FRIEZA" },
    { 4, "MAJIN" },
    // 5 is ...?
    { 6, "ANY" }
};

static const std::unordered_map<ci_string, int, CIStrHash> constant_to_race =
{
    { "HUMAN", 0 },
    { "SAIYAN", 1 },
    { "NAMEKIAN", 2 },
    { "FRIEZA", 3 },
    { "MAJIN", 4 },
    { "ANY", 6 }
};

static const std::unordered_map<int, std::string> sex_to_constant =
{
    { 0, "MALE" },
    { 1, "FEMALE" },
    { 2, "ANY" },
};

static const std::unordered_map<ci_string, int, CIStrHash> constant_to_sex =
{
    { "MALE", 0 },
    { "FEMALE", 1 },
    { "ANY", 2 }
};

static const std::unordered_map<int, std::string> fade_to_constant =
{
    { QED_FADE_BLACK, "BLACK" },
    { QED_FADE_WHITE, "WHITE" },
    { QED_FADE_NONBG, "NONBG" },
    { 3, "DEFAULT"},
};

static const std::unordered_map<ci_string, int, CIStrHash> constant_to_fade =
{
    { "BLACK", QED_FADE_BLACK },
    { "WHITE", QED_FADE_WHITE },
    { "NONBG", QED_FADE_NONBG },
    { "DEFAULT", 3 }
};

static const std::unordered_map<int, std::string> time_ctrl_to_constant =
{
    { 0, "ADD_TIME" },
    { 1, "REMOVE_TIME" },
    { 2, "SET_ELAPSED" },
};

static const std::unordered_map<ci_string, int, CIStrHash> constant_to_time_ctrl =
{
    { "ADD_TIME", 0 },
    { "REMOVE_TIME", 1 },
    { "SET_ELAPSED", 2 }
};

static const std::unordered_map<int, std::string> team_type_to_constant =
{
    { 1, "PLAYER_TEAM" },
    { 2, "PLAYER_TEAM2" },
    { 3, "ENEMY_TEAM" }
};

static const std::unordered_map<ci_string, int, CIStrHash> constant_to_team_type =
{
    { "PLAYER_TEAM", 1 },
    { "PLAYER_TEAM2", 2 },
    { "ENEMY_TEAM", 3 }
};

static const std::unordered_map<int, std::string> team_type2_to_constant =
{
    { 0, "UNK_TEAM0" },
    { 1, "PLAYER_TEAM" },
    { 2, "ENEMY_TEAM" }
};

static const std::unordered_map<ci_string, int, CIStrHash> constant_to_team_type2 =
{
    { "UNK_TEAM0", 0 },
    { "PLAYER_TEAM", 1 },
    { "ENEMY_TEAM", 2 }
};

static const std::unordered_map<int, std::string> stats_to_constant =
{
    { QED_STAT_LEVEL, "LEVEL" },
    { QED_STAT_HP, "HEALTH" },
    { QED_STAT_STAMINA, "STAMINA" },
    { QED_STAT_KI, "KI" },
    { QED_STAT_ATK, "ATK" },
    { QED_STAT_KI_ATK, "KI_ATK" },
    { QED_STAT_SUPER_ATK, "SUPER_ATK" },
    { QED_STAT_SUPER_KI, "SUPER_KI" },
    { QED_STAT_ATK_DAMAGE, "ATK_DAMAGE" },
    { QED_STAT_KI_DAMAGE, "KI_DAMAGE" },
    { QED_STAT_SUPER_ATK_DAMAGE, "SUPER_ATK_DAMAGE" },
    { QED_STAT_SUPER_KI_DAMAGE, "SUPER_KI_DAMAGE" },
    { QED_STAT_MAX_HP, "MAX_HEALTH" },
    { QED_STAT_MAX_STAMINA, "MAX_STAMINA" },
    { QED_STAT_MAX_KI, "MAX_KI" },
    { QED_STAT_GUARD_ATTACK, "GUARD_ATK" },
    { QED_STAT_GUARD_DAMAGE, "GUARD_DAMAGE" },
    { QED_STAT_MOVE_SPEED, "MOVE_SPEED" },
    { QED_STAT_BOOST_SPEED, "BOOST_SPEED" }
};

static const std::unordered_map<ci_string, int, CIStrHash> constant_to_stats =
{
    { "LEVEL", QED_STAT_LEVEL },
    { "HEALTH", QED_STAT_HP },
    { "STAMINA", QED_STAT_STAMINA },
    { "KI", QED_STAT_KI },
    { "ATK", QED_STAT_ATK },
    { "KI_ATK", QED_STAT_KI_ATK },
    { "SUPER_ATK", QED_STAT_SUPER_ATK },
    { "SUPER_KI", QED_STAT_SUPER_KI },
    { "ATK_DAMAGE", QED_STAT_ATK_DAMAGE },
    { "KI_DAMAGE", QED_STAT_KI_DAMAGE },
    { "SUPER_ATK_DAMAGE", QED_STAT_SUPER_ATK_DAMAGE },
    { "SUPER_KI_DAMAGE", QED_STAT_SUPER_KI_DAMAGE },
    { "MAX_HEALTH", QED_STAT_MAX_HP },
    { "MAX_STAMINA", QED_STAT_MAX_STAMINA },
    { "MAX_KI", QED_STAT_MAX_KI },
    { "GUARD_ATK", QED_STAT_GUARD_ATTACK },
    { "GUARD_DAMAGE", QED_STAT_GUARD_DAMAGE },
    { "MOVE_SPEED", QED_STAT_MOVE_SPEED },
    { "BOOST_SPEED", QED_STAT_BOOST_SPEED }
};

static const std::unordered_map<int, std::string> skill_slot_to_constant =
{
    { 0, "SUPER1" },
    { 1, "SUPER2" },
    { 2, "SUPER3" },
    { 3, "SUPER4" },
    { 4, "ULTIMATE1" },
    { 5, "ULTIMATE2" },
    { 6, "EVASIVE" },
    { 7, "BLAST" },
    { 8, "AWAKEN" }
};

static const std::unordered_map<ci_string, int, CIStrHash> constant_to_skill_slot =
{
    { "SUPER1", 0 },
    { "SUPER2", 1 },
    { "SUPER3", 2 },
    { "SUPER4", 3 },
    { "ULTIMATE1", 4 },
    { "ULTIMATE2", 5 },
    { "EVASIVE", 6 },
    { "BLAST", 7 },
    { "AWAKEN", 8 }
};

static const std::unordered_map<int, std::string> quest_fs_to_constant =
{
    { 0, "COMPLETE" },
    { 1, "ULTIMATE_FINISH" },
    { 2, "FAIL" }
};

static const std::unordered_map<ci_string, int, CIStrHash> constant_to_quest_fs =
{
    { "COMPLETE", 0 },
    { "ULTIMATE_FINISH", 1 },
    { "FAIL", 2 }
};

static const std::unordered_map<int, std::string> stats_to_constant2 =
{
    { 0, "HEALTH" },
    { 1, "STAMINA" },
    { 2, "KI" },
};

static const std::unordered_map<ci_string, int, CIStrHash> constant_to_stats2 =
{
    { "HEALTH", 0 },
    { "STAMINA", 1 },
    { "KI", 2 },
};


static const std::vector<InstructionDef> instructions_defs =
{
    { QED_COND_NEVER, "Never", false, { } },
    { QED_COND_ALWAYS, "Always", false, { } },
    { QED_COND_ALWAYS_, "Always_", false, { } },
    { QED_COND_ALWAYS__, "Always__", false, { } },
    { QED_COND_ALWAYS___, "Always___", false, { } },
    { QED_COND_IS_SCENE_INIT_COMPLETED, "IsDemoLoadCompleted", false, { } },
    { QED_COND_DEMO_PLAY_ENDED, "DemoPlayEnded", false, { } },
    { QED_COND_CLEAR_SCREEN_CLOSED, "ClearScreenClosed", false, { } },
    { QED_COND_BEV_PLAY_COMPLETED, "BevPlayEnded", false, { } },
    { QED_COND_IS_BEV_LOAD_COMPLETED, "IsBevLoadCompleted", false, { } },
    { QED_COND_COMBO_SUCCESS, "ComboSuccess", false, { } },
    { QED_COND_TTL_GAME_FINISHED, "HCFinished", false, { } },
    { QED_ACT_NOP, "Nop", true, { } },
    { QED_ACT_INIT_QUEST, "InitQuest", true, { } },
    { QED_ACT_PLAY_DEMO, "PlayDemo", true, { } },
    { QED_ACT_QUEST_CLEAR, "QuestClear", true, { } },
    { QED_ACT_UNK_20, "Unk20", true, { } },
    { QED_ACT_START_BATTLE_MODE, "BattleModeStart", true, { } },
    { QED_ACT_SHOW_QUEST_SUMMARY, "ShowQuestSummary", true, { } },
    { QED_ACT_SHOW_WARNING, "ShowWarning", true, { } },
    { QED_ACT_CANCEL_DIALOGUE, "CancelDialogue", true, { } },
    { QED_ACT_CREDITS, "Credits", true, { } },    
    { QED_ACT_PLAY_AIC, "PlayAic", true, { } },
    { QED_ACT_RESET_CAMERA, "ResetCamera", true, { } },
    { QED_ACT_UNK_117, "Unk117", true, { } },

    { QED_COND_UNK_2, "Unk2", false, { PARAM_STAGE } },
    { QED_COND_TIME_PASSED_LOCAL, "TimeWithinState", false, { PARAM_FLOAT } },
    { QED_COND_FADE_COMPLETED, "FadeCompleted", false, { PARAM_BOOLEAN } },
    { QED_COND_RANDOM_INT, "RandomPercentSmallerThan", false, { PARAM_INTEGER } },    
    { QED_COND_DIALOGUE_FINISH, "DialogueFinish", false, { PARAM_QBT } },
    { QED_COND_IS_FIGHTING, "IsFighting", false, { PARAM_QCHAR } },
    { QED_COND_IS_ALIVE, "IsAlive", false, { PARAM_QCHAR } },
    { QED_COND_REMOVED, "Removed", false, { PARAM_QCHAR } },
    { QED_COND_FIGHT_STARTED_WITH_CHAR, "FightStartedWithChar", false, { PARAM_QCHAR } },
    { QED_COND_UNK_41, "Unk41", false, { PARAM_INTEGER } },
    { QED_COND_UNK_43, "Unk43", false, { PARAM_INTEGER } },
    { QED_COND_IS_QUEST_FINISHED, "IsQuestFinished", false, { PARAM_QUEST_WITH_TYPE } },
    { QED_COND_IS_QUEST_UNFINISHED, "IsQuestUnfinished", false, { PARAM_QUEST_WITH_TYPE } },
    { QED_COND_TIME_IN_DEMO, "TimeInDemo", false, { PARAM_INTEGER } },
    { QED_COND_REVIVE_CHECK, "ReviveCheck", false, { PARAM_QCHAR } },
    { QED_COND_DETECTED_WITH_SCOUTER, "DetectedWithScouter", false, { PARAM_QCHAR } },
    { QED_COND_UNK_52, "Unk52", false, { PARAM_INTEGER } },
    { QED_COND_ITEM_USED, "ItemUsed", false, { PARAM_INTEGER } },
    { QED_COND_TEACHER_DIALOGUE_FINISH, "TeacherDialogueFinish", false, { PARAM_INTEGER } },
    { QED_COND_CURRENT_PARTNER_IS, "PartnerIs", false, { PARAM_PARTNER } },
    { QED_COND_UNK_61, "Unk61", false, { PARAM_INTEGER } },
    { QED_ACT_REGISTER_ACTOR_FOR_DEMO, "RegisterActorForDemo", true, { PARAM_QCHAR} },
    { QED_ACT_UNK_22, "Unk22", true, { PARAM_BOOLEAN } },
    { QED_ACT_GOTO_STATE, "GotoState", true, { PARAM_INTEGER } },
    { QED_ACT_GOTO_STATE_, "GotoState_", true, { PARAM_INTEGER } },
    { QED_ACT_UNLOCK_QUEST, "UnlockQuest", true, { PARAM_QUEST } },
    { QED_ACT_QUEST_FINISH_STATE, "QuestFinishState", true, { PARAM_QUEST_FINISH_STATE } },
    { QED_ACT_LOAD_CHARA, "LoadChara", true, { PARAM_QCHAR } },
    { QED_ACT_DONT_REMOVE_ON_KO, "DontRemoveOnKo", true, { PARAM_QCHAR } },
    { QED_ACT_WAIT, "Wait", true, { PARAM_FLOAT } },
    { QED_ACT_LOAD_BEV, "LoadBev", true, { PARAM_STRING } },
    { QED_ACT_UNK_73, "Unk73", true, { PARAM_QCHAR } },
    { QED_ACT_UNK_74, "Unk74", true, { PARAM_QCHAR } },
    { QED_ACT_PLAY_CLOCK, "PlayClock", true, { PARAM_BOOLEAN } },
    { QED_ACT_UNK_78, "Unk78", true, { PARAM_STRING } },
    { QED_ACT_UNK_84, "Unk84", true, { PARAM_STAGE} },
    { QED_ACT_SHOW_RADAR, "ShowRadar", true, { PARAM_BOOLEAN } },
    { QED_ACT_SHOW_DEMONSTRATION_TEXT, "ShowDemonstrationText", true, { PARAM_BOOLEAN } },
    { QED_ACT_UNK_94, "Unk94", true, { PARAM_INTEGER } },
    { QED_ACT_LISTEN_FOR_ACTION, "ListenForAction", true, { PARAM_INTEGER } },
    { QED_ACT_TEACHER_DIALOGUE, "TeacherDialogue", true, { PARAM_INTEGER } },
    { QED_ACT_ENABLE_ITEMS, "EnableItems", true, { PARAM_BOOLEAN } },
    { QED_ACT_UNK_103, "Unk103", true, { PARAM_QCHAR } },
    { QED_ACT_HERO_COLOSSEUM_HINT, "HCHint", true, { PARAM_STRING } },
    { QED_ACT_CHARA_RESET, "CharaReset", true, { PARAM_QCHAR } },
    { QED_ACT_UNK_119, "Unk119", true, { PARAM_QCHAR } },

    { QED_COND_TIME_PASSED, "TimePassed", false, { PARAM_OPERATOR, PARAM_FLOAT } },
    { QED_COND_PLAYER_HEALTH, "PlayerHealth", false, { PARAM_OPERATOR, PARAM_FLOAT } },
    { QED_COND_CHECK_FLAG, "CheckFlag", false, { PARAM_FLAG, PARAM_BOOLEAN } },
    { QED_COND_PLAYER_RACE, "PlayerRaceIs", false, { PARAM_RACE, PARAM_SEX } },
    { QED_COND_NUM_HUMAN_PLAYERS, "NumHumanPlayers", false, { PARAM_OPERATOR, PARAM_INTEGER } },
    { QED_COND_USES_SKILL, "UseSkill", false, { PARAM_QCHAR, PARAM_SKILL } },
    { QED_COND_HIT_WITH_SKILL, "HitWithSkill", false, { PARAM_QCHAR, PARAM_SKILL } },
    { QED_COND_KO_WITH_SKILL, "KoWithSkill", false, { PARAM_QCHAR, PARAM_SKILL } },
    { QED_COND_CARRIES_DRAGON_BALLS, "CarriesDragonBalls", false, { PARAM_QCHAR, PARAM_INTEGER } },
    { QED_COND_DRAGON_BALLS_SENT, "NumOfDragonBallsSent", false, { PARAM_INTEGER, PARAM_INTEGER } },
    { QED_COND_DRAGON_BALL_OBTAINED, "TeamCarriesDragonBalls", false, { PARAM_TEAM_TYPE, PARAM_INTEGER } },
    { QED_COND_REMAINING_TIME, "RemainingTime", false, { PARAM_OPERATOR, PARAM_FLOAT } },
    { QED_COND_HEALTH_DAMAGE_OVER, "HealthDamageOver", false, { PARAM_QCHAR, PARAM_INTEGER } },
    { QED_ACT_LOAD_DEMO, "LoadDemo", true, { PARAM_STRING, PARAM_FADE } },
    { QED_ACT_UNK_10, "Unk10", true, { PARAM_QCHAR, PARAM_INTEGER } },
    { QED_ACT_QUICK_POWERUP, "QuickPowerup", true, { PARAM_QCHAR, PARAM_INTEGER } }, // Used mostly for demos.
    { QED_ACT_REMOVE_CHARA, "RemoveChara", true, { PARAM_QCHAR, PARAM_BOOLEAN } },
    { QED_ACT_REMOVE_CHARA_, "RemoveChara_", true, { PARAM_QCHAR, PARAM_BOOLEAN } },
    { QED_ACT_SET_CHAR_VISIBLE, "SetCharVisible", true, { PARAM_QCHAR, PARAM_BOOLEAN } },
    { QED_ACT_SET_CHAR_VISIBLE_, "SetCharVisible_", true, { PARAM_QCHAR, PARAM_BOOLEAN } },
    { QED_ACT_RECOVER_HP, "RecoverHp", true, { PARAM_QCHAR, PARAM_FLOAT } },
    { QED_ACT_UNK_29, "Unk29", true, { PARAM_BOOLEAN, PARAM_BOOLEAN } },
    { QED_ACT_AI_TABLE, "SetAITable", true, { PARAM_QCHAR, PARAM_INTEGER } },
    { QED_ACT_SET_FLAG, "SetFlag", true, { PARAM_FLAG, PARAM_BOOLEAN } },
    { QED_ACT_SET_TIME, "TimeControl", true, { PARAM_TIME_CTRL_MODE, PARAM_FLOAT } },
    { QED_ACT_UNK_40, "Unk40", true, { PARAM_QCHAR, PARAM_BOOLEAN } },
    { QED_ACT_UNK_47, "Unk47", true, { PARAM_QCHAR, PARAM_INTEGER } },
    { QED_ACT_SET_THERE_ARE_ENEMIES, "SetThereAreEnemies", true, { PARAM_STAGE, PARAM_BOOLEAN } },
    { QED_ACT_UNK_51, "Unk51", true, { PARAM_QCHAR, PARAM_INTEGER } },
    { QED_ACT_MOVE_TO, "MoveTo", true, { PARAM_STRING_LONG, PARAM_QCHAR } },
    { QED_ACT_CHANGE_STAGE, "ChangeStage", true, { PARAM_STAGE, PARAM_FADE } },
    { QED_ACT_REMOVE_HP, "DamageHp", true, { PARAM_QCHAR, PARAM_FLOAT } },
    { QED_ACT_UNK_63, "Unk63", true, { PARAM_QCHAR, PARAM_FLOAT } },
    { QED_ACT_UNK_64, "Unk64", true, { PARAM_QCHAR, PARAM_INTEGER } },
    { QED_ACT_SET_CAN_CARRY_DB, "SetCanCarryDragonBall", true, { PARAM_QCHAR, PARAM_BOOLEAN } },
    { QED_ACT_SET_INVULNERABLE, "SetInvulnerable", true, { PARAM_QCHAR, PARAM_BOOLEAN } },
    { QED_ACT_TUT_POPUP, "Tutorial", true, { PARAM_INTEGER, PARAM_INTEGER } },
    { QED_ACT_COPY_HEALTH, "CopyHealth", true, { PARAM_QCHAR, PARAM_QCHAR } },
    { QED_ACT_UNK_80, "Unk80", true, { PARAM_INTEGER, PARAM_INTEGER } },
    { QED_ACT_UNK_81, "Unk81", true, { PARAM_QCHAR, PARAM_BOOLEAN } },
    { QED_ACT_ADD_TIME, "AddTime", true, { PARAM_REVERSE_BOOLEAN, PARAM_FLOAT } },
    { QED_ACT_SHOW_ENEMY_KO_COUNTER, "ShowEnemyKoCounter", true, { PARAM_BOOLEAN, PARAM_INTEGER } },
    { QED_ACT_SHOW_NAME, "ShowName", true, { PARAM_QCHAR, PARAM_BOOLEAN } },
    { QED_ACT_ENABLE_MOVEMENT, "EnableMovement", true, { PARAM_BOOLEAN, PARAM_QCHAR } },
    { QED_ACT_TIP_MSG, "ShowTip", true, { PARAM_BATTLE_MSG, PARAM_BOOLEAN } },
    { QED_ACT_UNK_105, "Unk105", true, { PARAM_STRING_LONG, PARAM_BOOLEAN, PARAM_INTEGER } },
    { QED_ACT_BODY_CHANGE, "BodyChange", true, { PARAM_QCHAR, PARAM_QCHAR } },
    { QED_ACT_UNK_108, "Unk108", true, { PARAM_INTEGER, PARAM_BOOLEAN } },
    { QED_ACT_SET_STRING_VAR, "SetStringVar", true, { PARAM_STRING_LONG, PARAM_STRINGVAR } },    
    { QED_ACT_UNK_116, "Unk116", true, { PARAM_INTEGER, PARAM_INTEGER } },    
    { QED_ACT_UNK_122, "Unk122", true, { PARAM_QCHAR, PARAM_INTEGER } },

    { QED_COND_KO, "Ko", false, { PARAM_QCHAR, PARAM_BOOLEAN, PARAM_INTEGER } },
    { QED_COND_HEALTH, "Health", false, { PARAM_OPERATOR, PARAM_QCHAR, PARAM_FLOAT } },
    { QED_COND_IN_STAGE, "InStage", false, { PARAM_QCHAR, PARAM_STAGE, PARAM_BOOLEAN } },
    { QED_COND_TEAM_HEALTH, "TeamHealth", false, { PARAM_OPERATOR, PARAM_TEAM_TYPE, PARAM_FLOAT } },
    { QED_COND_NUM_CHARS_DEFEATED, "NumCharsDefeated", false, { PARAM_INTEGER, PARAM_TEAM_TYPE2, PARAM_STAGE } },
    { QED_COND_OBJECTS_PICKED, "ObjectsPicked", false, { PARAM_INTEGER, PARAM_INTEGER, PARAM_INTEGER } },
    { QED_COND_EVENT_DONE, "EventDone", false, { PARAM_INTEGER, PARAM_INTEGER, PARAM_INTEGER } },
    { QED_COND_UNK_55, "Unk55", false, { PARAM_QCHAR, PARAM_INTEGER, PARAM_BOOLEAN } },
    { QED_COND_UNK_60, "Unk60", false, { PARAM_BOOLEAN, PARAM_INTEGER, PARAM_INTEGER } },
    { QED_ACT_PLAY_BGM, "PlayBgm", true, { PARAM_INTEGER, PARAM_INTEGER, PARAM_INTEGER } },
    { QED_ACT_STOP_BGM, "StopBgm", true, { PARAM_BOOLEAN, PARAM_INTEGER, PARAM_INTEGER } }, // Set boolean to false
    { QED_ACT_UNK_9, "Unk9", true, { PARAM_INTEGER, PARAM_INTEGER, PARAM_BOOLEAN } },
    { QED_ACT_HEALTH_CAP, "HealthCap", true, { PARAM_BOOLEAN, PARAM_QCHAR, PARAM_FLOAT } },
    { QED_ACT_STATS, "SetStat", true, { PARAM_QCHAR, PARAM_STATS, PARAM_INTEGER } },
    { QED_ACT_USE_SKILL, "UseSkill", true, { PARAM_QCHAR, PARAM_SKILL_SLOT, PARAM_BOOLEAN } },
    { QED_ACT_SET_ATTACK_TARGET, "SetAttackTarget", true, { PARAM_QCHAR, PARAM_QCHAR, PARAM_BOOLEAN } },
    { QED_ACT_ACTIVATE_INTERACTIVE_CHAR, "ActivateInteractiveChar", true, { PARAM_QCHAR, PARAM_BOOLEAN, PARAM_BOOLEAN } },
    { QED_ACT_POWERUP, "PowerUp", true, { PARAM_QCHAR, PARAM_QBT, PARAM_INTEGER } },
    { QED_ACT_AI_LOCK_SKILL, "LockAISkill", true, { PARAM_QCHAR, PARAM_SKILL_SLOT, PARAM_REVERSE_BOOLEAN } },
    { QED_ACT_SKILLS_EQUIP, "EquipSkills", true, { PARAM_QCHAR, PARAM_BOOLEAN, PARAM_SKILL_COMBINATION } },
    { QED_ACT_SHOW_COMBO_AND_LISTEN, "ShowComboAndListen", true, { PARAM_BOOLEAN, PARAM_INTEGER, PARAM_QCHAR } },
    { QED_ACT_LOAD_AIC, "LoadAic", true, { PARAM_STRING, PARAM_INTEGER, PARAM_BOOLEAN } },
    { QED_ACT_UNK_114, "Unk114", true, { PARAM_BOOLEAN, PARAM_INTEGER, PARAM_INTEGER } },
    { QED_ACT_UNK_121, "Unk121", true, { PARAM_QCHAR, PARAM_INTEGER, PARAM_INTEGER } },

    { QED_COND_UNK_10, "Unk10", false, { PARAM_SCMS, PARAM_INTEGER, PARAM_INTEGER, PARAM_INTEGER } },
    { QED_COND_UNK_13, "Unk13", false, { PARAM_SCMS, PARAM_INTEGER, PARAM_INTEGER, PARAM_INTEGER } },
    { QED_COND_ACTION_DONE, "ActionDone", false, { PARAM_INTEGER, PARAM_INTEGER, PARAM_INTEGER, PARAM_INTEGER } },
    { QED_COND_UNK_62, "Unk62", false, { PARAM_QCHAR, PARAM_INTEGER, PARAM_INTEGER, PARAM_INTEGER } },
    { QED_ACT_FADE_IN, "FadeIn", true, { PARAM_FLOAT, PARAM_FADE, PARAM_REVERSE_BOOLEAN, PARAM_INTEGER } }, // NOTICE: reverse boolean (if true, ui is hidden, that should be description of param, same for FADE_IN)
    { QED_ACT_FADE_OUT, "FadeOut", true, { PARAM_FLOAT, PARAM_FADE, PARAM_REVERSE_BOOLEAN, PARAM_INTEGER } },
    { QED_ACT_SET_REPLACE_CHARA_LOAD, "SetReplaceCharaLoad", true, { PARAM_STRING, PARAM_QCHAR, PARAM_BOOLEAN, PARAM_FADE } },
    { QED_ACT_SET_REPLACE_CHARA_LOAD_, "SetReplaceCharaLoad_", true, { PARAM_STRING, PARAM_QCHAR, PARAM_BOOLEAN, PARAM_FADE } },
    { QED_ACT_PORTAL, "PortalControl", true, { PARAM_STAGE, PARAM_STAGE, PARAM_BOOLEAN, PARAM_BOOLEAN } },
    { QED_ACT_PLAY_DIALOGUE, "PlayDialogue", true, { PARAM_QBT, PARAM_INTEGER, PARAM_STAGE, PARAM_INTEGER } },
    { QED_ACT_PLAY_DIALOGUE2, "PlayDialogue2", true, { PARAM_QBT, PARAM_INTEGER, PARAM_STAGE, PARAM_INTEGER, PARAM_QCHAR } },
    { QED_ACT_PLAY_DIALOGUE3, "PlayDialogue3", true, { PARAM_QBT, PARAM_INTEGER, PARAM_STAGE, PARAM_INTEGER } },
    { QED_ACT_CHARA_SPAWN, "CharaSpawn", true, { PARAM_QCHAR, PARAM_INTEGER, PARAM_QBT, PARAM_INTEGER, PARAM_STAGE, PARAM_INTEGER } },
    { QED_ACT_CHARA_SPAWN2, "CharaSpawn2", true, { PARAM_QCHAR, PARAM_QCHAR, PARAM_INTEGER, PARAM_QBT, PARAM_STAGE, PARAM_INTEGER, PARAM_INTEGER, PARAM_INTEGER } },
    { QED_ACT_CHARA_SPAWN3, "CharaSpawn3", true, { PARAM_QCHAR, PARAM_QCHAR, PARAM_QCHAR, PARAM_INTEGER, PARAM_QBT, PARAM_STAGE, PARAM_INTEGER, PARAM_INTEGER } },
    { QED_ACT_PLAY_ETOOL_EVENT, "PlayScene", true, { PARAM_QCHAR, PARAM_INTEGER, PARAM_QBT, PARAM_INTEGER} },
    { QED_ACT_CHARA_LEAVE, "CharaLeave", true, { PARAM_QCHAR, PARAM_INTEGER, PARAM_QBT, PARAM_INTEGER, PARAM_BOOLEAN, PARAM_STAGE } },
    { QED_ACT_REVIVE, "Revive", true, { PARAM_QCHAR, PARAM_QBT, PARAM_INTEGER, PARAM_FLOAT } },
    { QED_ACT_REVIVE_EX, "ReviveEx", true, { PARAM_QCHAR, PARAM_QBT, PARAM_INTEGER, PARAM_STRINGVAR, PARAM_FLOAT } },
    { QED_ACT_TRANSFORM, "ModelChange", true, { PARAM_QCHAR, PARAM_QCHAR, PARAM_INTEGER, PARAM_QBT, PARAM_INTEGER } },
    { QED_ACT_TRANSFORM2, "ModelChange2", true, { PARAM_QCHAR, PARAM_QCHAR, PARAM_INTEGER, PARAM_QBT, PARAM_INTEGER } },
    { QED_ACT_PLAY_BEV, "PlayBev", true, { PARAM_INTEGER, PARAM_QCHAR, PARAM_QCHAR, PARAM_QCHAR, PARAM_QCHAR, PARAM_QCHAR, PARAM_QCHAR, PARAM_QCHAR } },
    { QED_ACT_UNK_61, "Unk61", true, { PARAM_QCHAR, PARAM_INTEGER, PARAM_INTEGER, PARAM_INTEGER } },
    { QED_ACT_PLACE_DRAGON_BALL, "PlaceDragonBall", true, { PARAM_INTEGER, PARAM_INTEGER, PARAM_INTEGER, PARAM_INTEGER } },
    { QED_ACT_DISABLE_COMMON_DIALOGUE, "DisableCommonDialogue", true, { PARAM_INTEGER, PARAM_QCHAR, PARAM_QCHAR, PARAM_QCHAR, PARAM_INTEGER, PARAM_INTEGER, PARAM_INTEGER, PARAM_INTEGER } },
    { QED_ACT_SCATTER_ATTACK, "ScatterAttack", true, { PARAM_INTEGER, PARAM_INTEGER, PARAM_INTEGER, PARAM_INTEGER } },
    { QED_ACT_HERO_COLOSSEUM, "HCStart", true, { PARAM_QCHAR, PARAM_QCHAR, PARAM_QCHAR, PARAM_QCHAR, PARAM_QCHAR, PARAM_QCHAR, PARAM_INTEGER, PARAM_INTEGER } },
    { QED_ACT_STAT_REGENERATION, "StatsRegeneration", true, { PARAM_QCHAR, PARAM_BOOLEAN, PARAM_STATS_COMBINATION, PARAM_FLOAT } },
    { QED_ACT_POWERUP_EX, "PowerUpEx", true, { PARAM_QCHAR, PARAM_QBT, PARAM_INTEGER, PARAM_STRINGVAR } },
    { QED_ACT_UNK_120, "Unk120", true, { PARAM_INTEGER, PARAM_INTEGER, PARAM_INTEGER, PARAM_INTEGER } },

    // XV2Patcher extensions
    { QED_COND_EXT_IS_AVATAR, "XV2P_IsAvatar", false, { PARAM_QCHAR } },
 };

Xv2QuestCompiler::Xv2QuestCompiler()
{
    Reset();

    scripts.resize(6);
    scripts[0] = &active_qed0;
    scripts[1] = &active_qed1;
    scripts[2] = &active_qed2;
    scripts[3] = &active_qed3;
    scripts[4] = &active_qed4;
    scripts[5] = &active_qed5;

    test_mode = false;
}

void Xv2QuestCompiler::Reset()
{
    // Common
    quest_type = -1;
    active_qxd.Load(nullptr, 0);
    active_qml.Load(nullptr, 0);
    active_qbt.Load(nullptr, 0);
    active_qsl.Load(nullptr, 0);
    active_qed0.Load(nullptr, 0);
    active_qed1.Load(nullptr, 0);
    active_qed2.Load(nullptr, 0);
    active_qed3.Load(nullptr, 0);
    active_qed4.Load(nullptr, 0);
    active_qed5.Load(nullptr, 0);
    qsf.Load(nullptr, 0);
    title.clear();
    title.resize(XV2_LANG_NUM);
    dialogue.clear();
    dialogue.resize(XV2_LANG_NUM+1);
    dialogue_acb.clear();
    dialogue_awb.clear();
    use_internal_awb.clear();
    defined_names.clear();
    linked_mods.clear();

    // Decompiler
    referenced_collections.clear();
    referenced_chars.clear();
    referenced_special_chars.clear();    
    referenced_qchars.clear();
    referenced_dialogues.clear();
    referenced_interactive_dialogues.clear();
    referenced_event_dialogues.clear();
    referenced_title.clear();
    referenced_dialogue_ta.clear();
    referenced_chars_positions.clear();
    referenced_ichars_positions.clear();
    referenced_items_positions.clear();
    referenced_flags.clear();
    referenced_string_vars.clear();
    indent_level = 0;

    // Compiler
    while (!tokens.empty())
        tokens.pop();

    compiled_quest = QxdQuest();
    compiled_mods.clear();
    compiled_item_collections.clear();
    compiled_chars.clear();
    compiled_special_chars.clear();
    compiled_qml_chars.clear();
    compiled_dialogues.clear();
    compiled_interactive_dialogues.clear();
    compiled_chars_positions.clear();
    compiled_ichars_positions.clear();
    compiled_items_positions.clear();
    compiled_text_entry.clear();
    compiled_flags.clear();
    compiled_string_vars.clear();
    processed_audio_files.clear();
    used_flags_slots.clear();
    used_flags_slots.resize(QED_MAX_NUM_FLAGS, false);
    used_string_vars.clear();
    used_string_vars.resize(QED_MAX_NUM_STRING_VARS, false);

    self_reference_unlock = false;
    self_reference_parent = false;
    global_dialogue_index = 0;
    next_script = 0;
    title_touched = dialogue_touched = audio_touched = false;
    qsf_loaded = false;
}

int Xv2QuestCompiler::GetQuestType(const std::string &name)
{
    if (Utils::BeginsWith(name, "TPQ_", false) || Utils::BeginsWith(name, "CTP_", false))
        return QUEST_TYPE_TPQ;

    else if (Utils::BeginsWith(name, "TMQ_", false))
        return QUEST_TYPE_TMQ;

    else if (Utils::BeginsWith(name, "BAQ_", false))
        return QUEST_TYPE_BAQ;

    else if (Utils::BeginsWith(name, "TCQ_", false))
        return QUEST_TYPE_TCQ;

    else if (Utils::BeginsWith(name, "HLQ_", false))
        return QUEST_TYPE_HLQ;

    else if (Utils::BeginsWith(name, "RBQ_", false) || Utils::BeginsWith(name, "L_RBQ_", false))
        return QUEST_TYPE_RBQ;

    else if (Utils::BeginsWith(name, "CHQ_", false))
        return QUEST_TYPE_CHQ;

    else if (Utils::BeginsWith(name, "LEQ_", false))
        return QUEST_TYPE_LEQ;

    else if (Utils::BeginsWith(name, "TTQ_", false))
        return QUEST_TYPE_TTQ;

    else if (Utils::BeginsWith(name, "TFB_", false))
        return QUEST_TYPE_TFB;

    else if (Utils::BeginsWith(name, "TNB_", false))
        return QUEST_TYPE_TNB;

    else if (Utils::BeginsWith(name, "OSQ_", false))
        return QUEST_TYPE_OSQ;

    else if (Utils::BeginsWith(name, "PRB_", false))
        return QUEST_TYPE_PRB;

    return -1;
}

std::string Xv2QuestCompiler::GetDialogueFile(const std::string &name, uint32_t episode)
{
    std::string ret;

    if (quest_type == QUEST_TYPE_TPQ)
    {
        if (episode <= 13)
        {
            ret = "qs_ep" + Utils::ToStringAndPad(episode, 2);
        }
        else if (episode == 42)
        {
            ret = "qs_cep50";
        }
        else if (episode == 99)
        {
            ret = "qs_cep99";
        }
        else if (episode >= 19 && episode <= 60)
        {
            ret = "qs_cep" + Utils::ToStringAndPad(episode-19, 2);
        }
    }
    else if (quest_type == QUEST_TYPE_TMQ)
    {
        if (episode == 0)
        {
            ret = "qs_athor";
        }
        else if (episode <= 13)
        {
            ret = "qe_ep"+ Utils::ToStringAndPad(episode, 2);
        }
        else if (episode == 99)
        {
            ret = "qe_cep99";
        }
        else if (episode >= 19 && episode <= 60)
        {
            ret = "qe_cep" + Utils::ToStringAndPad(episode-19, 2);
        }
    }
    else if (quest_type == QUEST_TYPE_BAQ)
    {
        ret = "qb_battle";
    }
    else if (quest_type == QUEST_TYPE_TCQ)
    {
        ret = "qt_battle";
    }
    else if (quest_type == QUEST_TYPE_HLQ)
    {
        ret = "qh_battle";
    }
    else if (quest_type == QUEST_TYPE_RBQ)
    {
        ret = "qr_battle";
    }
    else if (quest_type == QUEST_TYPE_CHQ)
    {
        ret = "qch_battle";
    }
    else if (quest_type == QUEST_TYPE_LEQ)
    {
        ret = "ql_battle";
    }
    else if (quest_type == QUEST_TYPE_TTQ)
    {
        ret = "ttq_battle";
    }
    else if (quest_type == QUEST_TYPE_TFB)
    {
        ret = "tfb_battle";
    }
    else if (quest_type == QUEST_TYPE_TNB)
    {
        ret = "tnb_battle";
    }
    else if (quest_type == QUEST_TYPE_OSQ)
    {
        ret = "qo_"  + name;
    }
    else if (quest_type == QUEST_TYPE_PRB)
    {
        ret = "prb_battle";
    }

    return ret;
}

bool Xv2QuestCompiler::SupportsAudio()
{
    if (quest_type == QUEST_TYPE_RBQ || quest_type == QUEST_TYPE_LEQ || quest_type == QUEST_TYPE_TTQ ||
        quest_type == QUEST_TYPE_TFB || quest_type == QUEST_TYPE_TNB || quest_type == QUEST_TYPE_PRB)
    {
        return false;
    }

    return true;
}

std::string Xv2QuestCompiler::GetAudioFile(const std::string &name, uint32_t episode, uint32_t flags, bool english)
{
    std::string file;
    std::string prefix;

    if (!SupportsAudio())
        return file;

    if (quest_type == QUEST_TYPE_TPQ)
    {
        prefix = "TPQ";

        if (flags & 0x40000)
        {
            if (episode >= 19)
            {
                DPRINTF("Error: if episode is >= 19 in a TPQ/CTP quest, flags 0x40000 must be cleared!\n");
                return file;
            }

            file = "BDP_ep" + Utils::ToStringAndPad(episode, 2);
        }
        else
        {
            if (episode < 19)
            {
                DPRINTF("Error: if episode is < 19 in a TPQ/CTP quest, flags 0x40000 must be set.\n");
                return file;
            }

            if (episode != 99)
                episode = episode - 19;

            file = "tpq_cep" + Utils::ToStringAndPad(episode, 2);
        }
    }
    else if (quest_type == QUEST_TYPE_TMQ)
    {
        prefix = "TMQ";

        if (episode < 19)
        {
            file = "CBQM_ep" + Utils::ToStringAndPad(episode, 2);
        }
        else
        {
            file = "CAQM_cep" + Utils::ToStringAndPad(episode-19, 2);
        }
    }
    else if (quest_type == QUEST_TYPE_BAQ)
    {
        prefix = "BAQ";
        file = "CABA_ALL";
    }
    else if (quest_type == QUEST_TYPE_TCQ)
    {
        prefix = "TCQ";

        if (Utils::BeginsWith(name, "TCQ_TEST", false))
        {
            file = "CAM_ROK";
        }
        else
        {
            if (name.length() < 7)
                return file;

            const std::string cms = name.substr(4, 3);
            if (!game_cms->FindEntryByName(cms))
                return file;

            file = "CAM_" + cms;
        }
    }
    else if (quest_type == QUEST_TYPE_HLQ)
    {
        prefix = "HLQ";
        file = "CAHQ_TRX";
    }
    else if (quest_type == QUEST_TYPE_CHQ)
    {
        prefix = "CHQ";
        file = "CATQ_TOK";
    }
    else if (quest_type == QUEST_TYPE_OSQ)
    {
        prefix = "OSQ";
        file = "CAOS_" + name;
    }

    if (english)
    {
        file = prefix + "/en/" + file;
    }
    else
    {
        file = prefix + "/" + file;
    }

    file += "_VOX";
    return file;
}

std::string Xv2QuestCompiler::UpdateToConstant(uint32_t update)
{
    auto it = update_to_constant.find(update);
    if (it != update_to_constant.end())
        return it->second;

    return std::string();
}

std::string Xv2QuestCompiler::DlcToConstant(uint32_t dlc)
{
    auto it = dlc_to_constant.find(dlc);
    if (it != dlc_to_constant.end())
        return it->second;

    return std::string();
}

int Xv2QuestCompiler::ConstantToUpdate(const ci_string &ct)
{
    auto it = constant_to_update.find(ct);
    if (it != constant_to_update.end())
        return it->second;

    return -1;
}

int Xv2QuestCompiler::ConstantToDlc(const ci_string &ct)
{
    auto it = constant_to_dlc.find(ct);
    if (it != constant_to_dlc.end())
        return it->second;

    return -1;
}

std::string Xv2QuestCompiler::AiToConstant(uint32_t ai)
{
    auto it = ai_to_constant.find(ai);
    if (it != ai_to_constant.end())
        return it->second;

    return std::string();
}

int Xv2QuestCompiler::ConstantToAi(const ci_string &ct)
{
    auto it = constant_to_ai.find(ct);
    if (it != constant_to_ai.end())
        return it->second;

    return -1;
}

std::string Xv2QuestCompiler::TeamToConstant(uint32_t team)
{
    auto it = team_to_constant.find(team);
    if (it != team_to_constant.end())
        return it->second;

    return std::string();
}

int Xv2QuestCompiler::ConstantToTeam(const ci_string &ct)
{
    auto it = constant_to_team.find(ct);
    if (it != constant_to_team.end())
        return it->second;

    return -1;
}

std::string Xv2QuestCompiler::DialogueEventToConstant(uint32_t event)
{
    auto it = dialogue_event_to_constant.find(event);
    if (it != dialogue_event_to_constant.end())
        return it->second;

    return std::string();
}

int Xv2QuestCompiler::ConstantToDialogueEvent(const ci_string &ct)
{
    auto it = constant_to_dialogue_event.find(ct);
    if (it != constant_to_dialogue_event.end())
        return it->second;

    return -1;
}

std::string Xv2QuestCompiler::GetTitlePath() const
{
    std::string file;

    switch (quest_type)
    {
        case QUEST_TYPE_TPQ:
            file = "qs_title";
        break;

        case QUEST_TYPE_BAQ:
            file = "qb_title";
        break;

        case QUEST_TYPE_TCQ:
            file = "qt_title";
        break;

        case QUEST_TYPE_HLQ:
            file = "qh_title";
        break;

        case QUEST_TYPE_RBQ:
            file = "qr_title";
        break;

        case QUEST_TYPE_CHQ:
            file = "qch_title";
        break;

        case QUEST_TYPE_LEQ:
            file = "ql_title";
        break;

        case QUEST_TYPE_TTQ:
            file = "ttq_title";
        break;

        case QUEST_TYPE_TFB:
            file = "tfb_title";
        break;

        case QUEST_TYPE_TNB:
            file = "tnb_title";
        break;

        case QUEST_TYPE_OSQ:
            file = "qo_title";
        break;

        case QUEST_TYPE_PRB:
            file = "prb_title";
        break;

        default:
            file = "qe_title";
    }

    file = "data/msg/" + file;
    return file;
}

std::string Xv2QuestCompiler::GetCommonFilePath(const std::string &quest_name) const
{
    std::string path = "data/quest/";
    std::string prefix;

    if (Utils::BeginsWith(quest_name, "CTP_", false))
        prefix = "TPQ";
    else if (Utils::BeginsWith(quest_name, "L_RBQ_", false))
        prefix = "RBQ";
    else
        prefix = quest_name.substr(0, 3);

    path += prefix;
    path = Utils::MakePathString(path, quest_name);
    path = Utils::MakePathString(path, quest_name);

    return path;
}

bool Xv2QuestCompiler::LoadQxd(QxdFile &qxd, bool vanilla) const
{
    std::string file;

    if (!xv2fs)
        return false;

    switch (quest_type)
    {
        case QUEST_TYPE_TPQ:
            file = "TPQ/tpq_data.qxd";
        break;

        case QUEST_TYPE_TMQ:
            file = "TMQ/tmq_data.qxd";
        break;

        case QUEST_TYPE_BAQ:
            file = "BAQ/baq_data.qxd";
        break;

        case QUEST_TYPE_TCQ:
            file = "TCQ/tcq_data.qxd";
        break;

        case QUEST_TYPE_HLQ:
            file = "HLQ/hlq_data.qxd";
        break;

        case QUEST_TYPE_RBQ:
            file = "RBQ/rbq_data.qxd";
        break;

        case QUEST_TYPE_CHQ:
            file = "CHQ/chq_data.qxd";
        break;

        case QUEST_TYPE_LEQ:
            file = "LEQ/leq_data.qxd";
        break;

        case QUEST_TYPE_TTQ:
            file = "TTQ/ttq_data.qxd";
        break;

        case QUEST_TYPE_TFB:
            file = "TFB/tfb_data.qxd";
        break;

        case QUEST_TYPE_TNB:
            file = "TNB/tnb_data.qxd";
        break;

        case QUEST_TYPE_OSQ:
            file =" OSQ/osq_data.qxd";
        break;

        case QUEST_TYPE_PRB:
            file = "PRB/prb_data.qxd";
        break;
    }

    if (file.length() == 0)
        return false;

    file = "data/quest/" + file;
    return xv2fs->LoadFile(&qxd, file, vanilla);
}

bool Xv2QuestCompiler::LoadQsf()
{
    qsf_loaded = xv2fs->LoadFile(&qsf, QSF_PATH);
    return qsf_loaded;
}

bool Xv2QuestCompiler::LoadActiveQxd()
{
    return LoadQxd(active_qxd, false);
}

bool Xv2QuestCompiler::LoadActiveQml(const std::string &quest_name)
{
    return xv2fs->LoadFile(&active_qml, GetCommonFilePath(quest_name) + ".qml");
}

bool Xv2QuestCompiler::LoadActiveQbt(const std::string &quest_name)
{
    return xv2fs->LoadFile(&active_qbt, GetCommonFilePath(quest_name) + ".qbt");
}

bool Xv2QuestCompiler::LoadActiveQsl(const std::string &quest_name)
{
    return xv2fs->LoadFile(&active_qsl, GetCommonFilePath(quest_name) + ".qsl");
}

bool Xv2QuestCompiler::LoadActiveQed(const std::string &quest_name, size_t index)
{
    std::string path = GetCommonFilePath(quest_name);

    if (index > 0)
    {
        path.push_back('_');
        path += Utils::ToString(index);
    }

    path += ".qed";
    if (!xv2fs->LoadFile(scripts[index], path))
    {
        // Fix for RBQ_2500 missing a script file
        if (!xv2fs->FileExists(path))
        {
            scripts[index]->Load(nullptr, 0); // Force reset
            return true;
        }

        DPRINTF("%s: Failed loading qed \"%s\"\n", FUNCNAME, path.c_str());
        return false;
    }

    return true;
}

bool Xv2QuestCompiler::CommitCompiledQml()
{
    std::string path = GetCommonFilePath(compiled_quest.name) + ".qml";

    if (!xv2fs->SaveFile(&active_qml, path))
    {
        DPRINTF("Saving \"%s\" failed.\n", path.c_str());
        return false;
    }

    return true;
}

bool Xv2QuestCompiler::CommitCompiledQbt()
{
    std::string path = GetCommonFilePath(compiled_quest.name) + ".qbt";

    if (!xv2fs->SaveFile(&active_qbt, path))
    {
        DPRINTF("Saving \"%s\" failed.\n", path.c_str());
        return false;
    }

    return true;
}

bool Xv2QuestCompiler::CommitCompiledQsl()
{
    std::string path = GetCommonFilePath(compiled_quest.name) + ".qsl";

    if (!xv2fs->SaveFile(&active_qsl, path))
    {
        DPRINTF("Saving \"%s\" failed.\n", path.c_str());
        return false;
    }

    return true;
}

bool Xv2QuestCompiler::CommitCompiledQeds()
{
    for (size_t index = 0; index < compiled_quest.scripts.size(); index++)
    {
        std::string path = GetCommonFilePath(compiled_quest.name);

        if (index > 0)
        {
            path.push_back('_');
            path += Utils::ToString(index);
        }

        path += ".qed";

        if (!xv2fs->SaveFile(scripts[index], path))
        {
            DPRINTF("Saving \"%s\" failed.\n", path.c_str());
            return false;
        }
    }

    return true;
}

bool Xv2QuestCompiler::MaintenanceQxdChar(QxdFile &vanilla_qxd)
{
    std::unordered_set<uint32_t> delete_candidates;

    // First, put into the set the chars that are not vanilla
    for (size_t i = 0; i < active_qxd.GetNumChars(); i++)
    {
        const QxdCharacter &ch = active_qxd.GetChar(i);
        QxdCharacter *pch = vanilla_qxd.FindCharById(ch.id);
        if (!pch)
        {
            pch = vanilla_qxd.FindSpecialCharById(ch.id);
        }

        if (!pch)
        {
            delete_candidates.insert(ch.id);
        }
    }

    for (size_t i = 0; i < active_qxd.GetNumSpecialChars(); i++)
    {
        const QxdCharacter &ch = active_qxd.GetSpecialChar(i);
        QxdCharacter *pch = vanilla_qxd.FindSpecialCharById(ch.id);
        if (!pch)
        {
            pch = vanilla_qxd.FindCharById(ch.id);
        }

        if (!pch)
        {
            delete_candidates.insert(ch.id);
        }
    }

    if (delete_candidates.size() == 0)
        return true;

    // Now remove from the candidates list those that are referenced in some qml
    for (size_t i = 0; i < active_qxd.GetNumQuests(); i++)
    {
        const QxdQuest &quest = active_qxd.GetQuest(i);
        QmlFile qml;
        QmlFile *pqml = nullptr;

        if (quest.update_requirement == QXD_UPDATE_DEVELOPER || Utils::BeginsWith(quest.name, "empty", false))
            continue;

        if (quest.name == compiled_quest.name)
        {
            pqml = &active_qml;
        }
        else
        {
            std::string base_qpath = "data/quest/";
            if (Utils::BeginsWith(quest.name, "CTP_", false))
            {
                base_qpath += "TPQ";
            }
            else if (Utils::BeginsWith(quest.name, "L_RBQ_", false))
            {
                base_qpath += "RBQ";
            }
            else
            {
                base_qpath += quest.name.substr(0, 3);
            }

            base_qpath.push_back('/');
            base_qpath += quest.name;
            base_qpath.push_back('/');
            base_qpath += quest.name;

            // Check only those that are loose files, to avoid looking at vanilla ones (as those wouldn't reference non-vanilla qxd)
            if (!xv2fs->FileExists(base_qpath + ".qml", false, true))
                continue;

            if (!xv2fs->LoadFile(&qml, base_qpath + ".qml"))
            {
                DPRINTF("%s: Failed to load qml %s.qml\n", FUNCNAME, base_qpath.c_str());
                return false;
            }

            pqml = &qml;
        }

        for (const QmlEntry &entry : *pqml)
        {
            auto it = delete_candidates.find(entry.qxd_id);
            if (it != delete_candidates.end())
            {
                delete_candidates.erase(entry.qxd_id);
            }
        }
    }

    // At this point delete_candidates only contains those qxd char that are: a) not vanilla b) not used by any custom qml
    for (uint32_t to_delete : delete_candidates)
    {
        //DPRINTF("Deleting char %d\n", to_delete);
        active_qxd.RemoveCharById(to_delete);
    }

    return true;
}

bool Xv2QuestCompiler::MaintenanceQxdCollection(QxdFile &vanilla_qxd)
{
    std::unordered_set<uint32_t> delete_candidates;

    // First, put into the set the collections that are not vanilla
    for (size_t i = 0; i < active_qxd.GetNumCollections(); i++)
    {
        const QxdCollection &collection = active_qxd.GetCollection(i);

        if (!vanilla_qxd.FindCollection(collection.id))
        {
            delete_candidates.insert(collection.id);
        }
    }

    if (delete_candidates.size() == 0)
        return true;

    // Now remove from the candidates list those that are referenced in the qxd ItemRewards
    for (size_t i = 0; i < active_qxd.GetNumQuests(); i++)
    {
        const QxdQuest &quest = active_qxd.GetQuest(i);

        for (const QxdItemReward &reward: quest.item_rewards)
        {
            if (reward.type == QXD_ITEM_COLLECTION)
            {
                if (delete_candidates.find(reward.id) != delete_candidates.end())
                {
                    delete_candidates.erase(reward.id);
                }
            }
        }
    }

    // And now we must remove from the candidates list those referenced in some qsl
    for (size_t i = 0; i < active_qxd.GetNumQuests(); i++)
    {
        const QxdQuest &quest = active_qxd.GetQuest(i);
        QslFile qsl;
        QslFile *pqsl = nullptr;

        if (quest.update_requirement == QXD_UPDATE_DEVELOPER || Utils::BeginsWith(quest.name, "empty", false))
            continue;

        if (quest.name == compiled_quest.name)
        {
            pqsl = &active_qsl;
        }
        else
        {
            std::string base_qpath = "data/quest/";
            if (Utils::BeginsWith(quest.name, "CTP_", false))
            {
                base_qpath += "TPQ";
            }
            else if (Utils::BeginsWith(quest.name, "L_RBQ_", false))
            {
                base_qpath += "RBQ";
            }
            else
            {
                base_qpath += quest.name.substr(0, 3);
            }

            base_qpath.push_back('/');
            base_qpath += quest.name;
            base_qpath.push_back('/');
            base_qpath += quest.name;

            // Check only those that are loose files, to avoid looking at vanilla ones (as those wouldn't reference non-vanilla qxd)
            if (!xv2fs->FileExists(base_qpath + ".qsl", false, true))
                continue;

            if (!xv2fs->LoadFile(&qsl, base_qpath + ".qsl"))
            {
                DPRINTF("%s: Failed to load qsl %s.qsl\n", FUNCNAME, base_qpath.c_str());
                return false;
            }

            pqsl = &qsl;
        }

        for (const QslStage &stage : *pqsl)
        {
           for (const QslEntry &entry: stage.entries)
           {
               if (entry.type == QSL_POSITION_ITEM)
               {
                   auto it = delete_candidates.find(entry.qml_item_id);
                   if (it != delete_candidates.end())
                   {
                       delete_candidates.erase(entry.qml_item_id);
                   }
               }
           }
        }
    }

    // At this point delete_candidates only contains those qxd collections that are: a) not vanilla b) not used by qxd c) not used by any custom qsl
    for (uint32_t to_delete : delete_candidates)
    {
        //DPRINTF("Deleting collection %d\n", to_delete);
        active_qxd.RemoveCollectionById(to_delete);
    }

    return true;
}

bool Xv2QuestCompiler::MaintenanceQxd()
{
    QxdFile vanilla_qxd;

    if (!LoadQxd(vanilla_qxd, true))
    {
        DPRINTF("%s: Failed to load vanilla qxd from cpk (quest type = %d).\n", FUNCNAME, quest_type);
        return false;
    }

    if (!MaintenanceQxdChar(vanilla_qxd))
        return false;

    if (!MaintenanceQxdCollection(vanilla_qxd))
        return false;

    return true;
}

bool Xv2QuestCompiler::MaintenanceTitle()
{
    for (size_t i = 0; i < title.size(); i++)
    {
        std::unordered_set<std::string> to_delete;

        for (size_t j = 0; j < title[i].GetNumEntries(); j++)
        {
            MsgEntry &entry = (title[i])[j];

            if (Utils::BeginsWith(entry.name, "X2Q_", false))
            {
                size_t pos = entry.name.rfind('_');

                if (pos == 3 || pos == std::string::npos) // Cannot really be npos here
                    continue;

                std::string quest_name = entry.name.substr(4, pos-4);
                if (Utils::BeginsWith(quest_name, compiled_quest.name.substr(0, 4), false))
                {
                    // If quest doesn't exist, this title entry is dead
                    if (!active_qxd.FindQuestByName(quest_name))
                        to_delete.insert(entry.name);
                }
            }
        }

        for (const std::string &del : to_delete)
        {
            //DPRINTF("Deleting %s\n", del.c_str());
            title[i].RemoveEntry(del);
            title_touched = true;
        }
    }

    return true;
}

bool Xv2QuestCompiler::MaintenanceDialogue()
{
    for (size_t i = 0; i < dialogue.size(); i++)
    {
        if (i == XV2_LANG_NUM && !SupportsAudio())
            break;

        std::unordered_set<std::string> to_delete;

        for (size_t j = 0; j < dialogue[i].GetNumEntries(); j++)
        {
            MsgEntry &entry = (dialogue[i])[j];

            if (Utils::BeginsWith(entry.name, "X2", false))
            {
                size_t pos = entry.name.rfind('_');

                if (pos == std::string::npos)
                    continue;

                std::string quest_name = entry.name.substr(2, pos-2);
                if (Utils::BeginsWith(quest_name, compiled_quest.name.substr(0, 4), false))
                {
                    // If quest doesn't exist, this title entry is dead
                    if (!active_qxd.FindQuestByName(quest_name))
                        to_delete.insert(entry.name);
                }
            }
        }

        for (const std::string &del : to_delete)
        {
            //DPRINTF("Deleting %s\n", del.c_str());
            dialogue[i].RemoveEntry(del);
            dialogue_touched = true;
        }
    }

    return true;
}

bool Xv2QuestCompiler::MaintenanceAudio()
{
    if (!SupportsAudio())
        return true;

    for (size_t i = 0; i < dialogue_acb.size(); i++)
    {
        AcbFile &acb = dialogue_acb[i];
        std::unordered_set<std::string> to_delete;

        for (uint32_t t = 0; t < acb.GetNumTracks(); t++)
        {
            std::vector<uint32_t> cue_ids;
            std::vector<std::string> names;

            acb.TrackIndexToCueIds(t, cue_ids);
             names.resize(cue_ids.size());

            for (size_t c = 0; c < cue_ids.size(); c++)
            {
                acb.GetCueName(cue_ids[c], &names[c]);
            }

            for (const std::string &tn : names)
            {
                if (Utils::BeginsWith(tn, "X2", false))
                {
                    size_t pos = tn.rfind('_');

                    if (pos == std::string::npos)
                        continue;

                    std::string quest_name = tn.substr(2, pos-2);
                    if (Utils::BeginsWith(quest_name, compiled_quest.name.substr(0, 4), false))
                    {
                        // If quest doesn't exist, this title entry is dead
                        if (!active_qxd.FindQuestByName(quest_name))
                            to_delete.insert(tn);
                    }
                }
            }
        }

        for (const std::string &del : to_delete)
        {
            //DPRINTF("Deleting %s\n", del.c_str());
            Xenoverse2::FreeSound(&acb, del);
            audio_touched = true;
        }
    }

    return true;
}

bool Xv2QuestCompiler::CommitCompiledFiles()
{
    if (!CommitCompiledQml())
        return false;

    if (!CommitCompiledQbt())
        return false;

    if (!CommitCompiledQsl())
        return false;

    if (!CommitCompiledQeds())
        return false;

    return true;
}

bool Xv2QuestCompiler::CommitActiveQxd()
{
    std::string file;

    switch (quest_type)
    {
        case QUEST_TYPE_TPQ:
            file = "TPQ/tpq_data.qxd";
        break;

        case QUEST_TYPE_TMQ:
            file = "TMQ/tmq_data.qxd";
        break;

        case QUEST_TYPE_BAQ:
            file = "BAQ/baq_data.qxd";
        break;

        case QUEST_TYPE_TCQ:
            file = "TCQ/tcq_data.qxd";
        break;

        case QUEST_TYPE_HLQ:
            file = "HLQ/hlq_data.qxd";
        break;

        case QUEST_TYPE_RBQ:
            file = "RBQ/rbq_data.qxd";
        break;

        case QUEST_TYPE_CHQ:
            file = "CHQ/chq_data.qxd";
        break;

        case QUEST_TYPE_LEQ:
            file = "LEQ/leq_data.qxd";
        break;

        case QUEST_TYPE_TTQ:
            file = "TTQ/ttq_data.qxd";
        break;

        case QUEST_TYPE_TFB:
            file = "TFB/tfb_data.qxd";
        break;

        case QUEST_TYPE_TNB:
            file = "TNB/tnb_data.qxd";
        break;

        case QUEST_TYPE_OSQ:
            file =" OSQ/osq_data.qxd";
        break;

        case QUEST_TYPE_PRB:
            file = "PRB/prb_data.qxd";
        break;
    }

    file = "data/quest/" + file;
    if (!xv2fs->SaveFile(&active_qxd, file))
    {
        DPRINTF("Failed to save \"%s\".\n", file.c_str());
        return false;
    }

    return true;
}

bool Xv2QuestCompiler::CommitQsf()
{
    if (!qsf_loaded)
        return true;

    return xv2fs->SaveFile(&qsf, QSF_PATH);
}

bool Xv2QuestCompiler::LoadTitle()
{
    const std::string path = GetTitlePath();

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        const std::string file = path + '_' + xv2_lang_codes[i] + ".msg";
        if (!xv2fs->LoadFile(&title[i], file))
        {
            DPRINTF("%s: Failed to load file %s\n", FUNCNAME, file.c_str());
            return false;
        }
    }

    return true;
}

bool Xv2QuestCompiler::CommitTitle()
{
    if (!title_touched)
        return true;

    const std::string path = GetTitlePath();

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        const std::string file = path + '_' + xv2_lang_codes[i] + ".msg";
        if (!xv2fs->SaveFile(&title[i], file))
        {
            DPRINTF("Failed to save \"%s\"\n", file.c_str());
            return false;
        }
    }

    return true;
}

bool Xv2QuestCompiler::LoadDialogue(const std::string &quest_name, uint32_t episode)
{
    const std::string path = "data/msg/" + GetDialogueFile(quest_name, episode);

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        const std::string file = path + '_' + xv2_lang_codes[i] + ".msg";
        if (!xv2fs->LoadFile(&dialogue[i], file))
        {
            DPRINTF("%s: Failed to load file %s\n", FUNCNAME, file.c_str());
            return false;
        }
    }

    if (quest_type == QUEST_TYPE_TTQ || quest_type == QUEST_TYPE_TNB || quest_type == QUEST_TYPE_TFB)
    {
        // Nothing, they don't have voice.msg
    }
    else
    {
        const std::string file = path + '_' + "voice.msg";
        if (!xv2fs->LoadFile(&dialogue[XV2_LANG_NUM], file))
        {
            DPRINTF("%s: Failed to load file %s\n", FUNCNAME, file.c_str());
            return false;
        }
    }

    return true;
}

bool Xv2QuestCompiler::CommitDialogue()
{
    if (!dialogue_touched)
        return true;

    const std::string path = "data/msg/" + GetDialogueFile(compiled_quest.name, compiled_quest.episode);

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        const std::string file = path + '_' + xv2_lang_codes[i] + ".msg";
        if (!xv2fs->SaveFile(&dialogue[i], file))
        {
            DPRINTF("Failed to save \"%s\".\n", file.c_str());
            return false;
        }
    }

    if (quest_type == QUEST_TYPE_TTQ || quest_type == QUEST_TYPE_TNB || quest_type == QUEST_TYPE_TFB)
    {
        // Nothing, they don't have voice.msg
    }
    else
    {
        const std::string file = path + '_' + "voice.msg";
        if (!xv2fs->SaveFile(&dialogue[XV2_LANG_NUM], file))
        {
            DPRINTF("Failed to save file \"%s\"\n", file.c_str());
            return false;
        }
    }

    return true;
}

bool Xv2QuestCompiler::LoadDialogueAudio(const std::string &quest_name, uint32_t episode, uint32_t flags)
{
    dialogue_acb.resize(2);
    dialogue_awb.resize(2);    
    use_internal_awb.resize(2, false);

    for (int i = 0; i < 2; i++)
    {
        std::string file = GetAudioFile(quest_name, episode, flags, (i==1));

        if (file.length() == 0)
        {
            DPRINTF("Cannot load audio file for this quest because couldn't deduce which file it uses.\n");
            return false;
        }

        file = Utils::MakePathString("data/sound/VOX/Quest", file);
        const std::string acb = file + ".acb";
        const std::string awb = file + ".awb";

        if (!xv2fs->LoadFile(dynamic_cast<CriwareAudioContainer *>(&dialogue_acb[i]), acb))
        {
            DPRINTF("Failed to load acb: %s\n", acb.c_str());
            return false;
        }

        if (dialogue_acb[i].CanUseExternalAwb())
        {
            if (!xv2fs->LoadFile(&dialogue_awb[i], awb))
            {
                DPRINTF("Failed to load awb: %s\n", awb.c_str());
                return false;
            }
        }
        else
        {
            uint8_t *awb_buf;
            uint32_t awb_size;

            awb_buf = dialogue_acb[i].GetAwb(&awb_size); // pointer, not copy
            if (!awb_buf)
            {
                DPRINTF("%s: Cannot load internal awb of \"%s\" because it doesn't exist.\n", FUNCNAME, acb.c_str());
                return false;
            }

            if (!dialogue_awb[i].Load(awb_buf, awb_size))
            {
                DPRINTF("%s: Failed to laod the internal awb of \"%s\".\n", FUNCNAME, acb.c_str());
                return false;
            }

            use_internal_awb[i] = true;
        }
    }

    return true;
}

bool Xv2QuestCompiler::CommitDialogueAudio()
{
    if (!audio_touched || !SupportsAudio())
        return true;

    for (int i = 0; i < 2; i++)
    {
        std::string file = GetAudioFile(compiled_quest.name, compiled_quest.episode, compiled_quest.flags, (i==1));

        if (file.length() == 0)
        {
            DPRINTF("Cannot save audio file for this quest because couldn't deduce which file it uses.\n");
            return false;
        }

        file = Utils::MakePathString("data/sound/VOX/Quest", file);
        const std::string acb = file + ".acb";
        const std::string awb = file + ".awb";

        if (use_internal_awb[i])
        {
            size_t size;
            uint8_t *data;

            data = dialogue_awb[i].Save(&size);
            if (!data)
            {
                DPRINTF("%s: Failed to save awb data (internal)\n", FUNCNAME);
                return false;
            }

            if (!dialogue_acb[i].SetAwb(data, (uint32_t)size, true))
            {
                delete[] data;
                DPRINTF("%s: SetAwb failed (internal).\n", FUNCNAME);
                return false;
            }

            // data now belongs to acb, don't delete!!!!
        }
        else
        {
            if (!Xenoverse2::SetAcbAwbData(&dialogue_acb[i], &dialogue_awb[i]))
            {
                DPRINTF("%s: SetAcbAwbData failed.\n", FUNCNAME);
                return false;
            }

            if (!xv2fs->SaveFile(&dialogue_awb[i], awb))
            {
                DPRINTF("Failed to save \"%s\".\n", awb.c_str());
                return false;
            }
        }

        if (!xv2fs->SaveFile(dynamic_cast<CriwareAudioContainer *>(&dialogue_acb[i]), acb))
        {
            DPRINTF("Failed to save \"%s\".\n", acb.c_str());
            return false;
        }
    }

    return true;
}

bool Xv2QuestCompiler::UninstallCompiledQuest(bool remove_empty_dir)
{
    if (!qsf_loaded && !LoadQsf())
    {
        DPRINTF("%s: Qsf file loading failed.\n", FUNCNAME);
        return false;
    }

    qsf.RemoveQuest(compiled_quest.name);

    if (!active_qxd.FindQuestById(compiled_quest.id))
        return true; // Yes, true

    active_qxd.RemoveQuest(compiled_quest.id, true);

    std::string quest_dir = Utils::GetDirNameString(GetCommonFilePath(compiled_quest.name));
    xv2fs->RemoveDir(quest_dir, remove_empty_dir);

    return true;
}

void Xv2QuestCompiler::DebugDumpTitle(const std::string &path)
{
    static std::unordered_set<std::string> processed_titles;
    std::string file_path = Utils::MakePathString(path, Utils::GetFileNameString(GetTitlePath()));
    file_path += ".txt";

    if (processed_titles.find(file_path) != processed_titles.end())
        return;

    processed_titles.insert(file_path);

    std::ostringstream oss;

    for (const MsgEntry &entry : title[0])
    {
        if (entry.lines.size() == 0)
            continue;

        std::string text = Xenoverse2::UnescapeHtml(entry.lines[0]);

        oss << "TextEntry " << entry.name << "\n{\n";
        indent_level++;

        WriteStringParam(oss, "en", text, true);

        for (size_t lang = 1; lang < XV2_LANG_NUM; lang++)
        {
            MsgEntry *this_lang_entry = title[lang].FindEntryByName(entry.name);

            if (!this_lang_entry || this_lang_entry->lines.size() == 0)
                continue;

            std::string lang_name;
            std::string lang_text = Xenoverse2::UnescapeHtml(this_lang_entry->lines[0]);

            if (lang_text == text)
                continue;

            lang_name = xv2_lang_codes[lang];
            WriteStringParam(oss, lang_name, lang_text);
        }

        oss << "}\n";
        indent_level--;
    }

    Utils::WriteFileBool(file_path, (const uint8_t *)oss.str().c_str(), oss.str().length(), true, true);
}

void Xv2QuestCompiler::DebugDumpDialogue(const std::string &path)
{
    static std::unordered_set<std::string> processed_dialogues;
    std::string file_path = Utils::MakePathString(path, GetDialogueFile(compiled_quest.name, compiled_quest.episode));
    file_path += ".txt";

    if (processed_dialogues.find(file_path) != processed_dialogues.end())
        return;

    processed_dialogues.insert(file_path);

    std::ostringstream oss;

    for (const MsgEntry &entry : dialogue[0])
    {
        if (entry.lines.size() == 0)
            continue;

        std::string text = Xenoverse2::UnescapeHtml(entry.lines[0]);

        if (dialogue.size() > XV2_LANG_NUM)
            oss << "TextAudioEntry ";
        else
            oss << "TextEntry ";

        oss << entry.name << "\n{\n";
        indent_level++;

        WriteStringParam(oss, "en", text, true);

        for (size_t lang = 1; lang <= XV2_LANG_NUM; lang++)
        {
            MsgEntry *this_lang_entry = dialogue[lang].FindEntryByName(entry.name);

            if (!this_lang_entry || this_lang_entry->lines.size() == 0)
                continue;

            std::string lang_name;
            std::string lang_text = Xenoverse2::UnescapeHtml(this_lang_entry->lines[0]);

            if (lang == XV2_LANG_NUM)
            {
                lang_name = "voice";

                if (lang_text.length() == 0)
                {
                    lang_text = entry.name;
                }
            }
            else
            {
                if (lang_text == text)
                    continue;

                lang_name = xv2_lang_codes[lang];
            }

            WriteStringParam(oss, lang_name, lang_text);
        }

        oss << "}\n";
        indent_level--;
    }

    Utils::WriteFileBool(file_path, (const uint8_t *)oss.str().c_str(), oss.str().length(), true, true);
}

bool Xv2QuestCompiler::LoadHcaResource(HcaFile &hca, const std::string &resource)
{
    bool ret = false;

    if (res_x2m)
    {
        std::string path = Utils::MakePathString("AUDIO", resource);
        size_t size;
        uint8_t *buf = res_x2m->ReadFile(path, &size);

        if (!buf)
            return false;

        ret = hca.Load(buf, size);
        delete[] buf;
    }
    else
    {
        std::string path = Utils::MakePathString(res_directory, resource);
        ret = hca.LoadFromFile(path, false);
    }

    return ret;
}

bool Xv2QuestCompiler::SetDialogueAudioFromResource(const std::string &resource, const std::string &track)
{
    if (dialogue_acb.size() != 2 || dialogue_awb.size() != 2)
    {
        DPRINTF("%s: BUG, audio files weren't loaded.\n", FUNCNAME);
        return false;
    }

    std::string jp_res;
    std::string en_res;

    HcaFile jp_hca, en_hca;

    for (int i = 0; i < 2; i++)
    {
        std::string hca_resource = resource + ((i == 0) ? "_jp" : "_en");
        hca_resource += ".hca";

        HcaFile *hca = (i == 0) ? &jp_hca : &en_hca;
        if (!LoadHcaResource(*hca, hca_resource))
        {
            // An error is normal here if one of the language is missing (which is allowed)
            //DPRINTF("Fail load %s.\n", hca_resource.c_str());
            continue;
        }

        if (i == 0)
            jp_res = hca_resource;
        else
            en_res = hca_resource;
    }

    if (jp_res.length() == 0 && en_res.length() == 0)
    {
        DPRINTF("Failed to load audio resource \"%s\".\n", resource.c_str());
        return false;
    }
    else if (jp_res.length() == 0)
    {
        if (!LoadHcaResource(jp_hca, en_res)) // fail should not happen, but...
        {
            DPRINTF("Failed to load audio resource \"%s\".\n", resource.c_str());
            return false;
        }
    }
    else if (en_res.length() == 0)
    {
        if (!LoadHcaResource(en_hca, jp_res)) // fail should not happen, but...
        {
            DPRINTF("Failed to load audio resource \"%s\".\n", resource.c_str());
            return false;
        }
    }

    for (int i = 0; i < 2; i++)
    {
        HcaFile *hca = (i == 0) ? &jp_hca : &en_hca;
        AcbFile *acb = &dialogue_acb[i];
        Afs2File *awb = &dialogue_awb[i];

        uint32_t cue_id = Xenoverse2::SetSound(acb, awb, track, *hca, !use_internal_awb[i]);
        if (cue_id == (uint32_t)-1)
        {
            DPRINTF("Failed setting custom sound. (adding to acb/awb failed).\n");
            return false;
        }
    }

    audio_touched = true;
    return true;
}

bool Xv2QuestCompiler::SetMsgEntryCommon(const std::string &entry, const std::string &text, std::vector<MsgFile> &msg_files, int lang, bool escape)
{
    MsgEntry *msg = msg_files[lang].FindEntryByName(entry);
    if (msg)
    {
        msg->lines.resize(1);

        if (escape)
            msg->lines[0] = Xenoverse2::EscapeHtml(text);
        else
            msg->lines[0] = text;
    }
    else
    {
        MsgEntry msg;

        msg.name = entry;

        if (escape)
            msg.lines.push_back(Xenoverse2::EscapeHtml(text));
        else
            msg.lines.push_back(text);

        if (!msg_files[lang].AddEntry(msg, true))
            return false;
    }

    return true;
}

bool Xv2QuestCompiler::GetTitleEntry(const std::string &entry, std::string &ret, int lang)
{
   MsgEntry *msg = title[lang].FindEntryByName(entry);
   if (msg)
   {
       ret = Xenoverse2::UnescapeHtml(msg->lines[0]);
       return true;
   }

   return false;
}

std::string Xv2QuestCompiler::GetTitleEntry(const std::string &entry, int lang)
{
    std::string ret;
    GetTitleEntry(entry, ret, lang);
    return ret;
}

bool Xv2QuestCompiler::SetTitleEntry(const std::string &entry, const std::string &text, int lang)
{
    title_touched = true;
    return SetMsgEntryCommon(entry, text, title, lang, false);
}

bool Xv2QuestCompiler::SetDialogueEntry(const std::string &entry, const std::string &text, int lang)
{
    dialogue_touched = true;
    return SetMsgEntryCommon(entry, text, dialogue, lang, false);
}

bool Xv2QuestCompiler::IsValidIdentifier(ci_string &str)
{
    if (str.length() == 0)
        return false;

    for (size_t i = 0; i < str.length(); i++)
    {
        char ch = str[i];

        if (ch >= 'A' && ch <= 'Z')
        {
        }
        else if (ch >= 'a' && ch <= 'z')
        {
        }
        else if (ch >= '0' && ch <= '9')
        {
            if (i == 0)
                return false;
        }
        else if (ch == '_')
        {
        }
        else
        {
            return false;
        }
    }

    return true;
}

bool Xv2QuestCompiler::IsValidVarName(ci_string &str)
{
    if (!IsValidIdentifier(str))
        return false;

    if (reserved_kw.find(str) != reserved_kw.end())
        return false;

    if (defined_names.find(str) != defined_names.end())
        return false;

    return true;
}

int Xv2QuestCompiler::GetQuestChar(const std::string &code)
{
    auto it = quest_chars.find(Utils::ToUpperCase(code));

    if (it != quest_chars.end())
        return it->second;

    return -1;
}

bool Xv2QuestCompiler::GetLobbyCharName(int pal_id, std::string &name, int lang)
{
    name.clear();

    if (!game_pal)
    {
        DPRINTF("%s: internal bug, LOBBY SHOULD HAVE BEEN INITED.\n", FUNCNAME);
        exit(-1);
    }

    PalEntry *pal = game_pal->FindEntryByID(pal_id);
    if (!pal)
        return false;

    return Xenoverse2::GetLobbyName(pal->name_id, name, lang);
}

bool Xv2QuestCompiler::GetItemName(int item_id, int item_type, std::string &name, int lang)
{
    switch (item_type)
    {
        case QXD_ITEM_TOP:
            name = Xenoverse2::GetCacTopName(item_id, lang);
        break;

        case QXD_ITEM_BOTTOM:
            name = Xenoverse2::GetCacBottomName(item_id, lang);
        break;

        case QXD_ITEM_GLOVES:
            name = Xenoverse2::GetCacGlovesName(item_id, lang);
        break;

        case QXD_ITEM_SHOES:
            name = Xenoverse2::GetCacShoesName(item_id, lang);
        break;

        case QXD_ITEM_ACCESSORY:
            name = Xenoverse2::GetCacAccesoryName(item_id, lang);
        break;

        case QXD_ITEM_SUPERSOUL:
            name = Xenoverse2::GetTalismanNameEx(item_id, lang);
        break;

        case QXD_ITEM_MATERIAL:
            name = Xenoverse2::GetMaterialNameEx(item_id, lang);
        break;

        case QXD_ITEM_EXTRA:
            name = Xenoverse2::GetExtraNameEx(item_id, lang);
        break;

        case QXD_ITEM_BATTLE:
            name = Xenoverse2::GetBattleNameEx(item_id, lang);
        break;

        case QXD_ITEM_PET:
            name = Xenoverse2::GetPetNameEx(item_id, lang);
        break;

        // QXD_ITEM_COLLECTION has its own function

        // TODO: hero colisseum items (objects/skills/figures)
    }

    return (name.length() > 0);
}

std::string Xv2QuestCompiler::GetCollectionName(int id, bool allow_default_name, int lang)
{
    std::string default_name;

    if (allow_default_name)
        default_name = "Collection" + Utils::ToString(id);

    QxdCollection *collection = active_qxd.FindCollection(id);
    if (!collection)
        return default_name;

    std::string top_name;
    std::string bottom_name;
    std::string gloves_name;
    std::string shoes_name;
    std::string acc_name;

    for (const QxdCollectionEntry &entry : collection->entries)
    {
        if (entry.item_type <= QXD_ITEM_ACCESSORY)
        {
            std::string name;
            GetItemName(entry.item_id, entry.item_type, name, lang);

            if (name.length() == 0)
                continue;

            if (entry.item_type == QXD_ITEM_TOP)
            {
                if (top_name.length() > 0) // Two tops
                    return default_name;

                top_name = name;
            }
            else if (entry.item_type == QXD_ITEM_BOTTOM)
            {
                if (bottom_name.length() > 0) // Two bottom
                    return default_name;

                bottom_name = name;
            }
            else if (entry.item_type == QXD_ITEM_GLOVES)
            {
                if (gloves_name.length() > 0) // Two gloves
                    return default_name;

                gloves_name = name;
            }
            else if (entry.item_type == QXD_ITEM_SHOES)
            {
                if (shoes_name.length() > 0) // Two shoes
                    return default_name;

                shoes_name = name;
            }
            else if (entry.item_type == QXD_ITEM_ACCESSORY)
            {
                if (acc_name.length() > 0) // Two accessories
                    return default_name;

                acc_name = name;
            }
        }
        else
        {
            return default_name;
        }
    }

    if (top_name.length() > 0)
        return top_name;

    if (bottom_name.length() > 0)
        return bottom_name;

    if (gloves_name.length() > 0)
        return gloves_name;

    if (shoes_name.length() > 0)
        return shoes_name;

    if (acc_name.length() > 0)
        return acc_name;

    return default_name;
}

XQ_X2mMod *Xv2QuestCompiler::FindStageModById(int16_t stage_id)
{
    if (stage_id < XV2_ORIGINAL_NUM_STAGES)
        return nullptr;

    for (auto &it : mods_table)
    {
        XQ_X2mMod &mod = it.second;

        if (mod.type == X2mType::NEW_STAGE && mod.id == (uint32_t)stage_id)
            return &mod;
    }

    return nullptr;
}

XQ_X2mMod *Xv2QuestCompiler::FindSkillModById(int16_t id1)
{
    for (auto &it : mods_table)
    {
        XQ_X2mMod &mod = it.second;

        if (mod.type == X2mType::NEW_SKILL && mod.id == (uint32_t)id1)
            return &mod;
    }

    return nullptr;
}

XQ_X2mMod *Xv2QuestCompiler::FindCharModById(int16_t cms_id)
{
    if (cms_id < XV2_FREE_ID_SEARCH_START)
        return nullptr;

    for (auto &it : mods_table)
    {
        XQ_X2mMod &mod = it.second;

        if (mod.type == X2mType::NEW_CHARACTER && mod.id == (uint32_t)cms_id)
            return &mod;
    }

    return nullptr;
}

XQ_X2mMod *Xv2QuestCompiler::FindCharModByCode(const std::string &cms_code)
{
    std::string u_cms_code = Utils::ToUpperCase(cms_code);

    if (Xenoverse2::IsOriginalChara(u_cms_code))
        return nullptr;

    for (auto &it : mods_table)
    {
        XQ_X2mMod &mod = it.second;

        if (mod.type == X2mType::NEW_CHARACTER && mod.code == u_cms_code)
            return &mod;
    }

    return nullptr;
}

XQ_X2mMod *Xv2QuestCompiler::FindCostumeModById(int16_t item_id, int item_type)
{
    if (item_type > QXD_ITEM_ACCESSORY)
        return nullptr;

    for (auto &it : mods_table)
    {
        XQ_X2mMod &mod = it.second;

        if (mod.type == X2mType::NEW_COSTUME)
        {
            X2mCostumeEntry &costume = mod.costume;

            for (size_t i = 0; i < costume.idb_entries.size(); i++)
            {
                if (costume.idb_entries[i] != item_id)
                    continue;

                if (item_type == QXD_ITEM_TOP && costume.costume_types[i] == COSTUME_TOP)
                    return &mod;

                else if (item_type == QXD_ITEM_BOTTOM && costume.costume_types[i] == COSTUME_BOTTOM)
                    return &mod;

                else if (item_type == QXD_ITEM_GLOVES && costume.costume_types[i] == COSTUME_GLOVES)
                    return &mod;

                else if (item_type == QXD_ITEM_SHOES && costume.costume_types[i] == COSTUME_SHOES)
                    return &mod;

                else if (item_type == QXD_ITEM_ACCESSORY && costume.costume_types[i] == COSTUME_ACCESSORY)
                    return &mod;
            }
        }
    }

    return nullptr;
}

ci_string Xv2QuestCompiler::LinkMod(XQ_X2mMod &mod)
{
    auto it = linked_mods.find(mod);

    if (it != linked_mods.end())
        return it->var_name;

    ci_string var_name = GetFriendlyName(mod.name, "Mod");
    mod.var_name = var_name;
    linked_mods.insert(mod);
    defined_names.insert(var_name);

    return var_name;
}

ci_string Xv2QuestCompiler::GetFriendlyName(const std::string &str, const std::string &fail_root)
{
    std::u16string u16str = Utils::Utf8ToUcs2(str);
    bool capitalize_next = false;

    for (size_t i = 0; i < u16str.length(); i++)
    {
        bool valid_letter = false;

        if (u16str[i] >= 'A' && u16str[i] <= 'Z')
        {
            valid_letter = true;
        }
        else if (u16str[i] >= 'a' && u16str[i] <= 'z')
        {
            valid_letter = true;
        }
        else if (u16str[i] >= '0' && u16str[i] <= '9')
        {
            valid_letter = true;
        }
        else if (u16str[i] == '_')
        {
            valid_letter = true;
        }

        if (valid_letter)
        {
            if (capitalize_next)
            {
                capitalize_next = false;
                if (u16str[i] >= 'a' && u16str[i] <= 'z')
                {
                    u16str[i] -= ('a' - 'A');
                }
            }
        }
        else
        {
            if (u16str[i] <= ' ')
                capitalize_next = true;

            u16str.erase(u16str.begin()+i);
            i--;
        }
    }

    std::string cv_str = Utils::Ucs2ToUtf8(u16str);
    while (cv_str.length() > 0 && isdigit(cv_str.front()))
        cv_str = cv_str.substr(1);

    bool num_inserted = false;

    if (cv_str.length() == 0)
    {
        cv_str = fail_root + "1";
        num_inserted = true;
    }

    ci_string ret = ci_string(cv_str.c_str());

    for (int i = 2; !IsValidVarName(ret); i++)
    {
        if (!num_inserted)
        {
            ret += Utils::ToString(i);
            num_inserted = true;
        }
        else
        {
            ret = ret.substr(0, ret.length()-Utils::ToString(i-1).length());
            ret += Utils::ToString(i);
        }
    }

    return ret;
}

void Xv2QuestCompiler::WriteIndent(std::ostringstream &oss, bool new_line)
{
    if (!new_line)
        return;

    for (int i = 0; i < indent_level; i++)
    {
        oss << '\t';
    }
}

void Xv2QuestCompiler::WriteQuest(std::ostringstream &oss, int16_t quest_id, std::string &comment)
{
    comment.clear();

    QxdQuest *quest = active_qxd.FindQuestById(quest_id);
    if (quest)
    {
        // TODO: x2m
        comment = GetTitleEntry(quest->msg_entries[0]);
        oss << '"' << quest->name << '"';
    }
    else
    {
        oss << quest_id;
    }
}

void Xv2QuestCompiler::WriteStage(std::ostringstream &oss, int16_t stage_id, std::string &comment)
{
    Xv2Stage *stage = nullptr;
    comment.clear();

    if (game_stage_def && stage_id >= 0 && stage_id < (int16_t)game_stage_def->GetNumStages())
    {
        stage = &(*game_stage_def)[stage_id];
    }

    if (stage)
    {
        XQ_X2mMod *mod = FindStageModById(stage_id);
        if (mod)
        {
            ci_string var_name = LinkMod(*mod);
            oss << var_name.c_str();
        }
        else
        {
            oss << '"' << stage->code << '"';
        }

        Xenoverse2::GetStageName(stage->code, comment);

    }
    else
    {
        oss << stage_id;
    }
}

bool Xv2QuestCompiler::WriteSkill(std::ostringstream &oss, int16_t id2, int type, bool allow_blast, std::string &comment)
{
    int16_t id = id2;
    comment.clear();

    if (id2 < 0)
    {
        oss << id2;
        return true;
    }

    if (type == QXD_SKILL_SUPER)
    {
        Xenoverse2::GetSuperSkillName(id, comment);
        id += CUS_SUPER_START;
    }
    else if (type == QXD_SKILL_ULTIMATE)
    {
        Xenoverse2::GetUltimateSkillName(id, comment);
        id += CUS_ULTIMATE_START;
    }
    else if (type == QXD_SKILL_EVASIVE)
    {
        Xenoverse2::GetEvasiveSkillName(id, comment);
        id += CUS_EVASIVE_START;
    }
    else if (type == QXD_SKILL_BLAST)
    {
        if (!allow_blast)
        {
            DPRINTF("%s: Blast skill not supported here\n", FUNCNAME);
            return false;
        }

        id += CUS_BLAST_START;
    }
    else if (type == QXD_SKILL_AWAKEN)
    {
        Xenoverse2::GetAwakenSkillName(id, comment);
        id += CUS_AWAKEN_START;
    }
    else
    {
        DPRINTF("%s: Unrecognized skill type: %d\n", FUNCNAME, type);
        return false;
    }

    if (comment.length() == 0)
    {
        CusSkill *skill = game_cus->FindSkillAnyByID(id);
        if (skill)
            comment = skill->name;
    }

    XQ_X2mMod *mod = FindSkillModById(id);
    if (mod)
    {
        ci_string var_name = LinkMod(*mod);
        oss << var_name.str;
    }
    else
    {
        oss << id;
    }

    return true;
}

void Xv2QuestCompiler::WriteChar(std::ostringstream &oss, int16_t cms_id, std::string &comment, int costume_for_comment, int model_preset_for_comment)
{
    CmsEntry *cms = nullptr;
    comment.clear();

    if (game_cms && cms_id >= 0)
    {
        cms = game_cms->FindEntryByID(cms_id);
    }

    if (cms)
    {
        XQ_X2mMod *mod = FindCharModById(cms_id);
        if (mod)
        {
            ci_string var_name = LinkMod(*mod);
            oss << var_name.c_str();
        }
        else
        {
            oss << '"' << cms->name << '"';
        }

        if (costume_for_comment < 0)
        {
            Xenoverse2::GetCharaName(cms->name, comment);
        }
        else
        {
            comment = Xenoverse2::GetCharaAndCostumeName(cms->name, costume_for_comment, model_preset_for_comment);
        }
    }
    else
    {
       oss << cms_id;
    }
}

void Xv2QuestCompiler::WriteChar(std::ostringstream &oss, const std::string &cms_code, std::string &comment, int costume_for_comment, int model_preset_for_comment)
{
    comment.clear();

    int qc = GetQuestChar(cms_code);
    if (qc >= 0)
    {
        if (qc == QXD_LPA && costume_for_comment >= 0)
        {
            GetLobbyCharName(costume_for_comment, comment);
        }
        else
        {
            auto it = quest_chars_description.find(qc);
            if (it == quest_chars_description.end())
            {
                DPRINTF("%s: bug, correct me.\n", FUNCNAME);
                exit(-1);
            }

            comment = it->second;
        }
    }
    else
    {
        if (costume_for_comment < 0)
        {
            Xenoverse2::GetCharaName(cms_code, comment);
        }
        else
        {
            comment = Xenoverse2::GetCharaAndCostumeName(cms_code, costume_for_comment, model_preset_for_comment);
        }

        XQ_X2mMod *mod = FindCharModByCode(cms_code);
        if (mod)
        {
            ci_string var_name = LinkMod(*mod);
            oss << var_name.c_str();
            return;
        }
    }

    oss << '"' << cms_code << '"';
}

void Xv2QuestCompiler::WriteTitle(std::ostringstream &oss, const std::string &msg_code, std::string &comment, int lang)
{
    comment.clear();

    MsgEntry *msg = title[lang].FindEntryByName(msg_code);
    if (msg)
    {
        comment = Xenoverse2::UnescapeHtml(msg->lines[0]);
        Utils::Replace(comment, "\n", "\\n");

        if (Utils::BeginsWith(msg_code, "X2Q", false))
        {
            auto it = referenced_title.find(msg_code);
            ci_string var_name;

            if (it == referenced_title.end())
            {
                var_name = GetFriendlyName(msg_code, "Title");
                referenced_title[msg_code] = var_name;
                defined_names.insert(var_name);
            }
            else
            {
                var_name = it->second;
            }

            oss << var_name.str;
            return;
        }
    }

    oss << '"' << msg_code << '"';
}

void Xv2QuestCompiler::WriteDialogueTA(std::ostringstream &oss, const std::string &msg_code, std::string &comment, int lang)
{
    comment.clear();

    MsgEntry *msg = dialogue[lang].FindEntryByName(msg_code);
    if (msg)
    {
        comment = Xenoverse2::UnescapeHtml(msg->lines[0]);
        Utils::Replace(comment, "\n", "\\n");

        if (Utils::BeginsWith(msg_code, "X2Q", false))
        {
            auto it = referenced_dialogue_ta.find(msg_code);
            ci_string var_name;

            if (it == referenced_dialogue_ta.end())
            {
                var_name = GetFriendlyName(msg_code, "DialogueTA");
                referenced_dialogue_ta[msg_code] = var_name;
                defined_names.insert(var_name);
            }
            else
            {
                var_name = it->second;
            }

            oss << var_name.str;
            return;
        }
    }

    oss << '"' << msg_code << '"';
}

bool Xv2QuestCompiler::WriteQChar(std::ostringstream &oss, int qml_id, std::string &comment, bool allow_negative, bool allow_non_existing)
{
    comment.clear();

    if (qml_id < 0)
    {
        if (!allow_negative)
        {
            DPRINTF("%s: I wasn't expecting a negative number as qml_id here.\n", FUNCNAME);
            return false;
        }

        oss << qml_id;
    }
    else
    {
        auto it = referenced_qchars.find(qml_id);
        if (it == referenced_qchars.end())
        {
            if (!allow_non_existing)
            {
                DPRINTF("%s: Invalid qml_id %d.\n", FUNCNAME, qml_id);
                return false;
            }

            oss << qml_id;
            return true;
        }

        oss << it->second.str;

        QmlEntry *qchar = active_qml.FindEntryById(qml_id);

        if (qchar)
        {
            QxdCharacter *chara = active_qxd.FindCharById(qchar->qxd_id);

            if (!chara)
                chara = active_qxd.FindSpecialCharById(qchar->qxd_id);

            if (chara)
            {
                std::ostringstream voidss;
                WriteChar(voidss, chara->cms_name, comment, chara->costume);
            }
        }
    }

    return true;
}

bool Xv2QuestCompiler::WriteDialogue(std::ostringstream &oss, int index, std::string &comment, bool allow_negative, bool allow_non_existing)
{
    comment.clear();

    if (index < 0)
    {
        if (!allow_negative)
        {
            DPRINTF("%s: I wasn't expecting a negative number as dialogue index here.\n", FUNCNAME);
            return false;
        }

        oss << index;
    }
    else
    {
        auto it = referenced_dialogues.find(index);
        if (it == referenced_dialogues.end())
        {
            if (!allow_non_existing)
            {
                DPRINTF("%s: invalid dialogue index: %d.\n", FUNCNAME, index);
                return false;
            }

            oss << index;
        }
        else
        {
            oss << it->second.str;

            if (index < (int)active_qbt.GetNumNormalEntries())
            {
                const QbtEntry &dialogue = active_qbt.GetNormalEntries()[index];

                if (dialogue.parts.size() > 0)
                {
                    std::ostringstream voidss;
                    WriteDialogueTA(voidss, dialogue.parts[0].msg_id, comment);

                    if (dialogue.parts.size() > 1)
                    {
                        comment += " [MORE DIALOGUE PARTS]";
                    }
                }
            }
        }
    }

    return true;
}

bool Xv2QuestCompiler::WriteInteractiveDialogue(std::ostringstream &oss, int index, std::string &comment, bool allow_negative, bool allow_non_existing)
{
    comment.clear();

    if (index < 0)
    {
        if (!allow_negative)
        {
            DPRINTF("%s: I wasn't expecting a negative number as interactive dialogue index here.\n", FUNCNAME);
            return false;
        }

        oss << index;
    }
    else
    {
        auto it = referenced_interactive_dialogues.find(index);
        if (it == referenced_interactive_dialogues.end())
        {
            if (!allow_non_existing)
            {
                DPRINTF("%s: invalid interactive dialogue index: %d.\n", FUNCNAME, index);
                return false;
            }

            oss << index;
        }
        else
        {
            oss << it->second.str;

            if (index < (int)active_qbt.GetNumInteractiveEntries())
            {
                const QbtEntry &idialogue = active_qbt.GetInteractiveEntries()[index];

                if (idialogue.parts.size() > 0)
                {
                    std::ostringstream voidss;
                    WriteDialogueTA(voidss, idialogue.parts[0].msg_id, comment);
                }
            }
        }
    }

    return true;
}

bool Xv2QuestCompiler::WriteFlag(std::ostringstream &oss, int flag)
{
    if (flag < 0 || flag >= QED_MAX_NUM_FLAGS)
    {
        DPRINTF("%s: flag %d is over %d.\n", FUNCNAME, flag, QED_MAX_NUM_FLAGS);
        return false;
    }

    auto it = referenced_flags.find(flag);
    if (it == referenced_flags.end())
    {
        std::string desired_name = "Flag" + Utils::ToString(flag);
        ci_string var_name = GetFriendlyName(desired_name, "Flag");
        defined_names.insert(var_name);
        referenced_flags[flag] = var_name;

        oss << var_name.str;
    }
    else
    {
        oss << it->second.str;
    }

    return true;
}

bool Xv2QuestCompiler::WriteStringVar(std::ostringstream &oss, int string_var)
{
    if (string_var < 0 || string_var >= QED_MAX_NUM_STRING_VARS)
    {
        DPRINTF("%s: position store %d is over %d.\n", FUNCNAME, string_var, QED_MAX_NUM_STRING_VARS);
        return false;
    }

    auto it = referenced_string_vars.find(string_var);
    if (it == referenced_string_vars.end())
    {
        std::string desired_name = "String" + Utils::ToString(string_var);
        ci_string var_name = GetFriendlyName(desired_name, "String");
        defined_names.insert(var_name);
        referenced_string_vars[string_var] = var_name;

        oss << var_name.str;
    }
    else
    {
        oss << it->second.str;
    }

    return true;
}

void Xv2QuestCompiler::WriteIntegerParam(std::ostringstream &oss, const std::string &name, int32_t value, bool new_line, std::string comment, bool hexadecimal)
{
    WriteIndent(oss, new_line);

    oss << name << ": ";

    if (!hexadecimal)
        oss << value;
    else
        oss << Utils::UnsignedToString((uint32_t)value, true);

    if (new_line)
    {
        if (comment.length() > 0)
        {
            Utils::Replace(comment, "\n", "\\n");
            oss << " ; " << comment;
        }

        oss << '\n';
    }
    else
    {
        oss << ' ';
    }
}

void Xv2QuestCompiler::WriteBooleanParam(std::ostringstream &oss, const std::string &name, bool value, bool new_line)
{
    return WriteIdentifierParam(oss, name, (value) ? "true" : "false", new_line);
}

void Xv2QuestCompiler::WriteFloatParam(std::ostringstream &oss, const std::string &name, float value, bool new_line)
{
    WriteIndent(oss, new_line);

    oss << name << ": " << Utils::FloatToString(value); // To avoid precision lose
    if (new_line)
        oss << '\n';
    else
        oss << ' ';
}

void Xv2QuestCompiler::WriteStringParam(std::ostringstream &oss, const std::string &name, const std::string &value, bool new_line, std::string comment)
{
    WriteIndent(oss, new_line);

    std::string str = value;
    Utils::Replace(str, "\"", "\\\"");
    Utils::Replace(str, "\n", "\\n");
    Utils::Replace(str, "\t", "\\\t");

    oss << name << ": \"" << str << "\"";
    if (new_line)
    {
        if (comment.length() > 0)
        {
            Utils::Replace(comment, "\n", "\\n");
            oss << " ; " << comment;
        }

        oss << '\n';
    }
    else
    {
        oss << ' ';
    }
}

void Xv2QuestCompiler::WriteIdentifierParam(std::ostringstream &oss, const std::string &name, const std::string &value, bool new_line, std::string comment)
{
    WriteIndent(oss, new_line);

    oss << name << ": " << value;
    if (new_line)
    {
        if (comment.length() > 0)
        {
            Utils::Replace(comment, "\n", "\\n");
            oss << " ; " << comment;
        }

        oss << '\n';
    }
    else
    {
        oss << ' ';
    }
}

template<typename T>
void Xv2QuestCompiler::WriteIntegerArrayParam(std::ostringstream &oss, const std::string &name, T *values, size_t count, bool new_line)
{
    WriteIndent(oss, new_line);

    oss << name << ": (";

    for (size_t i = 0; i < count; i++)
    {
        if (i != 0)
            oss << ", ";

        oss << values[i];
    }

    oss << ')';

    if (new_line)
        oss << '\n';
    else
        oss << ' ';
}

void Xv2QuestCompiler::WriteQuestParam(std::ostringstream &oss, const std::string &name, int16_t quest_id, bool new_line, std::string *comment_out)
{
    std::string comment;

    WriteIndent(oss, new_line);
    oss << name << ": ";
    WriteQuest(oss, quest_id, comment);

    if (comment_out)
        *comment_out = comment;

    if (new_line)
    {
        if (!comment_out && comment.length() > 0)
            oss << " ; " << comment;

        oss << '\n';
    }
    else
    {
        oss << ' ';
    }
}

void Xv2QuestCompiler::WriteStageParam(std::ostringstream &oss, const std::string &name, int16_t stage_id, bool new_line, std::string *comment_out)
{
    std::string comment;

    WriteIndent(oss, new_line);
    oss << name << ": ";
    WriteStage(oss, stage_id, comment);

    if (comment_out)
        *comment_out = comment;

    if (new_line)
    {
        if (!comment_out && comment.length() > 0)
            oss << " ; " << comment;

        oss << '\n';
    }
    else
    {
        oss << ' ';
    }
}

bool Xv2QuestCompiler::WriteItemParam(std::ostringstream &oss, const std::string &name, const std::string &type_name, int16_t item_id, int type, bool new_line, bool allow_collection, std::string *comment_out, bool write_type)
{
    std::string comment;

    auto it = item_type_to_constant.find(type);
    if (it == item_type_to_constant.end())
    {
        if (type == 999) // Wtf is type 999
        {
            WriteIntegerParam(oss, name, item_id, false);

            if (new_line)
                oss << '\n';

            if (write_type)
                WriteIntegerParam(oss, type_name, type, new_line);

            return true;
        }
        else
        {
            DPRINTF("%s: Unrecognized item type: %d\n", FUNCNAME, type);
            return false;
        }
    }

    if (type == QXD_ITEM_COLLECTION)
    {
        if (allow_collection)
        {
            ci_string var_name;
            auto it = referenced_collections.find(item_id);

            if (it == referenced_collections.end())
            {
                var_name = GetFriendlyName(GetCollectionName(item_id, true), "Collection");
                referenced_collections[item_id] = var_name;
                defined_names.insert(var_name);
            }
            else
            {
                var_name = it->second;
            }

            comment = GetCollectionName(item_id, false);
            WriteIdentifierParam(oss, name, var_name.str, false);
        }
        else
        {
            DPRINTF("%s: Collection not allowed here.\n", FUNCNAME);
            return false;
        }
    }
    else if (type != 999)
    {
        XQ_X2mMod *mod = FindCostumeModById(item_id, type);
        GetItemName(item_id, type, comment);

        if (mod)
        {
            ci_string var_name = LinkMod(*mod);
            WriteIdentifierParam(oss, name, var_name.str, false);
        }
        else
        {
            WriteIntegerParam(oss, name, item_id, false);
        }
    }

    if (comment_out)
        *comment_out = comment;

    if (new_line)
    {
        if (!comment_out && comment.length() > 0)
            oss << "; " << comment;

        oss << '\n';
    }

    if (write_type)
        WriteIdentifierParam(oss, type_name, it->second, new_line);

    return true;
}

bool Xv2QuestCompiler::WriteSkillParam(std::ostringstream &oss, const std::string &name, int16_t id2, int type, bool allow_blast, bool new_line, std::string *comment_out)
{
    std::string comment;

    WriteIndent(oss, new_line);
    oss << name << ": ";

    if (!WriteSkill(oss, id2, type, allow_blast, comment))
        return false;

    if (comment_out)
        *comment_out = comment;

    if (new_line)
    {
        if (!comment_out && comment.length() > 0)
            oss << " ; " << comment;

        oss << '\n';
    }
    else
    {
        oss << ' ';
    }

    return true;
}

bool Xv2QuestCompiler::WriteCharParam(std::ostringstream &oss, const std::string &name, int16_t cms_id, bool new_line, std::string *comment_out, int costume_for_comment, int model_preset_for_comment)
{
    std::string comment;

    WriteIndent(oss, new_line);
    oss << name << ": ";
    WriteChar(oss, cms_id, comment, costume_for_comment, model_preset_for_comment);

    if (comment_out)
        *comment_out = comment;

    if (new_line)
    {
        if (!comment_out && comment.length() > 0)
            oss << " ; " << comment;

        oss << '\n';
    }
    else
    {
        oss << ' ';
    }

    return true;
}

bool Xv2QuestCompiler::WriteCharParam(std::ostringstream &oss, const std::string &name, const std::string &cms_name, bool new_line, std::string *comment_out, int costume_for_comment, int model_preset_for_comment)
{
    std::string comment;

    WriteIndent(oss, new_line);
    oss << name << ": ";
    WriteChar(oss, cms_name, comment, costume_for_comment, model_preset_for_comment);

    if (comment_out)
        *comment_out = comment;

    if (new_line)
    {
        if (!comment_out && comment.length() > 0)
            oss << " ; " << comment;

        oss << '\n';
    }
    else
    {
        oss << ' ';
    }

    return true;
}

void Xv2QuestCompiler::WriteTitleParam(std::ostringstream &oss, const std::string &name, const std::string &msg_code, bool new_line, std::string *comment_out, int lang)
{
    std::string comment;

    WriteIndent(oss, new_line);
    oss << name << ": ";
    WriteTitle(oss, msg_code, comment, lang);

    if (comment_out)
        *comment_out = comment;

    if (new_line)
    {
        if (!comment_out && comment.length() > 0)
            oss << " ; " << comment;

        oss << '\n';
    }
    else
    {
        oss << ' ';
    }
}

void Xv2QuestCompiler::WriteDialogueTAParam(std::ostringstream &oss, const std::string &name, const std::string &msg_code, bool new_line, std::string *comment_out, int lang)
{
    std::string comment;

    WriteIndent(oss, new_line);
    oss << name << ": ";
    WriteDialogueTA(oss, msg_code, comment, lang);

    if (comment_out)
        *comment_out = comment;

    if (new_line)
    {
        if (!comment_out && comment.length() > 0)
            oss << " ; " << comment;

        oss << '\n';
    }
    else
    {
        oss << ' ';
    }
}

bool Xv2QuestCompiler::WriteQCharParam(std::ostringstream &oss, const std::string &name, int qml_id, bool allow_negative, bool allow_non_existing, bool new_line, std::string *comment_out)
{
    std::string comment;

    WriteIndent(oss, new_line);
    oss << name << ": ";
    if (!WriteQChar(oss, qml_id, comment, allow_negative, allow_non_existing))
        return false;

    if (comment_out)
        *comment_out = comment;

    if (new_line)
    {
        if (!comment_out && comment.length() > 0)
            oss << " ; " << comment;

        oss << '\n';
    }
    else
    {
        oss << ' ';
    }

    return true;
}

bool Xv2QuestCompiler::WriteInteractiveDialogueParam(std::ostringstream &oss, const std::string &name, int index, bool allow_negative, bool allow_non_existing, bool new_line, std::string *comment_out)
{
    std::string comment;

    WriteIndent(oss, new_line);
    oss << name << ": ";
    if (!WriteInteractiveDialogue(oss, index, comment, allow_negative, allow_non_existing))
        return false;

    if (comment_out)
        *comment_out = comment;

    if (new_line)
    {
        if (!comment_out && comment.length() > 0)
            oss << " ; " << comment;

        oss << '\n';
    }
    else
    {
        oss << ' ';
    }

    return true;
}

bool Xv2QuestCompiler::FindReferencedQxdChars()
{
    for (const QmlEntry &qchar : active_qml)
    {
        QxdCharacter *chara = active_qxd.FindCharById(qchar.qxd_id);
        bool special = false;

        if (!chara)
        {
            special = true;
            chara = active_qxd.FindSpecialCharById(qchar.qxd_id);

            if (!chara)
            {
                DPRINTF("%s: Failed to find qxd char %d referenced by qml char %d.\n", FUNCNAME, qchar.qxd_id, qchar.id);
                return false;
            }

            if (referenced_special_chars.find(chara->id) != referenced_special_chars.end())
                continue;
        }
        else
        {
            if (referenced_chars.find(chara->id) != referenced_chars.end())
                continue;
        }

        std::string name;

        int qc = GetQuestChar(chara->cms_name);
        if (qc >= 0)
        {
            if (qc == QXD_LPA)
            {
                GetLobbyCharName(chara->costume, name);
            }
            else
            {
                auto it = quest_chars_description.find(qc);
                if (it == quest_chars_description.end())
                {
                    DPRINTF("%s: bug, correct me.\n", FUNCNAME);
                    exit(-1);
                }

                name = it->second;

                if (name == "Player")
                {
                    name = "PlayerBase";
                }
            }
        }
        else
        {
             Xenoverse2::GetCharaName(chara->cms_name, name);
        }

        ci_string var_name = GetFriendlyName(name, "CharBase");
        defined_names.insert(var_name);

        if (special)
            referenced_special_chars[chara->id] = var_name;
        else
            referenced_chars[chara->id] = var_name;
    }

    if (referenced_chars.size() == 0 && referenced_special_chars.size() == 0)
    {
        DPRINTF("%s: No qxd referenced chars were found!\n", FUNCNAME);
        return false;
    }

    return true;
}

void Xv2QuestCompiler::RemoveComments(ci_string &line) const
{
    if (line.find(';') == std::string::npos)
        return;

    bool in_quotes = false;
    ci_string new_line;

    char prev_ch = 0;

    for (size_t i = 0; i < line.length(); i++)
    {
        char ch = line[i];

        if (!in_quotes)
        {
            if (ch == ';')
                break;

            if (ch == '"')
                in_quotes = true;
        }
        else
        {
            if (ch == '"' && prev_ch != '\\')
                in_quotes = false;
        }

        new_line.push_back(ch);
        prev_ch = ch;
    }

    line = new_line;
}

void Xv2QuestCompiler::SaveTokenAndReset(X2QcToken &token, int line_num, const std::string &file)
{
    if (!token.Empty())
    {
        token.line_num = line_num;
        token.file = file;
        token.End();
        tokens.push(token);
    }

    token.Reset();
}

void Xv2QuestCompiler::SaveTokenAndReset(X2QcToken &token, int line_num, const std::string &file, std::vector<X2QcToken> &tokens)
{
    if (!token.Empty())
    {
        token.line_num = line_num;
        token.file = file;
        token.End();
        tokens.push_back(token);
    }

    token.Reset();
}

void Xv2QuestCompiler::GetTokens(const ci_string &line, int line_num, const std::string &file)
{
    bool in_quotes = false;
    bool in_call = false;
    X2QcToken current_token;

    ci_string t_line = line;
    Utils::TrimString(t_line.str);

    if (t_line.length() == 0)
        return;

    char prev_ch = 0;
    char ch = 0;

    for (size_t i = 0; i < t_line.length(); i++, prev_ch = ch)
    {
        char ch = t_line[i];

        if (!in_call)
        {
            if (ch == '(')
            {
                if (!in_quotes)
                {
                    SaveTokenAndReset(current_token, line_num, file);

                    current_token.type = TOKEN_GROUP;
                    current_token.str.push_back(ch);
                    in_call = true;
                    continue;
                }
            }
        }
        else
        {
            current_token.str.push_back(ch);

            if (ch == ')' && !in_quotes)
            {
                SaveTokenAndReset(current_token, line_num, file);
                in_call = false;
            }

            continue;
        }

        if (!in_quotes)
        {
            if (ch == '"')
            {
                if (prev_ch <= ' ' || prev_ch == ':' || prev_ch == '{' || prev_ch == '}')
                {
                    SaveTokenAndReset(current_token, line_num, file);
                    current_token.type = TOKEN_STRING;
                    current_token.str.push_back(ch);
                    in_quotes = true;
                    continue;
                }
            }
        }
        else
        {
            if (i != (t_line.length()-1))
            {
                if (ch == '\\')
                {
                    if (t_line[i+1] == 'n')
                    {
                        current_token.str.push_back('\n');
                        i++;
                        continue;
                    }
                    else if (t_line[i+1] == '"')
                    {
                        current_token.str.push_back('"');
                        i++;
                        continue;
                    }
                    else if (t_line[i+1] == '\t')
                    {
                        current_token.str.push_back('\t');
                        i++;
                        continue;
                    }
                }
            }

            current_token.str.push_back(ch);

            if (ch == '"')
            {
                SaveTokenAndReset(current_token, line_num, file);
                in_quotes = false;
            }

            continue;
        }

        if (ch == '{' || ch == '}' || ch == ':')
        {
            SaveTokenAndReset(current_token, line_num, file);
            current_token.str.push_back(ch);

            if (ch == '{') current_token.type = TOKEN_BRACKET_BEGIN;
            else if (ch == '}') current_token.type = TOKEN_BRACKET_END;
            else current_token.type = TOKEN_COLON;

            SaveTokenAndReset(current_token, line_num, file);
            continue;
        }

        if (ch <= ' ')
        {
            SaveTokenAndReset(current_token, line_num, file);
            continue;
        }

        if (current_token.Empty())
        {
            if ((ch >= '0' && ch <= '9') || ch == '-')
            {
                current_token.type = TOKEN_INTEGER;
            }
        }
        else if (current_token.type == TOKEN_INTEGER)
        {
            if (ch < '0' || ch > '9')
            {
                if ((ch == 'x' || ch == 'X') && current_token.str.length() == 1 && current_token.str[0] == '0')
                {
                    // Hex, allowed
                }
                else if (((ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')) && current_token.str.substr(0, 2) == "0x")
                {
                    // Hex, allowed
                }
                else
                {
                    current_token.type = TOKEN_IDENTIFIER;
                }
            }
        }

        current_token.str.push_back(ch);
    }

    if (!current_token.Empty())
    {
        if (in_quotes || in_call)
            current_token.type = TOKEN_IDENTIFIER; // Degrade to identifer, bad identifier will be found out later

        current_token.End();
        current_token.line_num = line_num;
        current_token.file = file;
        tokens.push(current_token);
    }
}

void Xv2QuestCompiler::Decompose(const std::string &str, const std::string &file)
{
    std::vector<std::string> lines;
    Utils::GetMultipleStrings(str, lines, '\n', false);

    for (size_t i = 0; i < lines.size(); i++)
    {
        ci_string line = lines[i];
        RemoveComments(line);
        GetTokens(line, (int)i+1, file);
    }
}

bool Xv2QuestCompiler::GetGroupTokens(const X2QcToken &group, std::vector<X2QcToken> &tokens)
{
    bool in_quotes = false;
    X2QcToken current_token;
    tokens.clear();

    int line_num = group.line_num;
    const std::string &file = group.file;

    bool token_started = false;
    bool token_ended = false;

    for (size_t i = 0; i < group.str.length(); i++)
    {
        char ch = group.str[i];

        if (!in_quotes)
        {
            if (ch == '"')
            {
                token_started = true;

                if (token_ended)
                {
                    DPRINTF("A comma was expected between tokens (position %Id inside parenthesis). ", i+1);
                    return LineError(group);
                }

                current_token.str.push_back(ch);
                current_token.type = TOKEN_STRING;
                in_quotes = true;
                continue;
            }
        }
        else
        {
            if (i != (group.str.length()-1))
            {
                if (ch == '\\')
                {
                    if (group.str[i+1] == 'n')
                    {
                        current_token.str.push_back('\n');
                        i++;
                        continue;
                    }
                    else if (group.str[i+1] == '"')
                    {
                        current_token.str.push_back('"');
                        i++;
                        continue;
                    }
                    else if (group.str[i+1] == '\t')
                    {
                        current_token.str.push_back('\t');
                        i++;
                        continue;
                    }
                }
            }

            current_token.str.push_back(ch);

            if (ch == '"')
            {
                in_quotes = false;
                token_ended = true;
            }

            continue;
        }

        if (ch == ',')
        {
            SaveTokenAndReset(current_token, line_num, file, tokens);
            token_ended = false;
            token_started = false;
            continue;
        }

        if (ch <= ' ')
        {
            if (token_started)
                token_ended = true;

            continue;
        }

        token_started = true;

        if (token_ended)
        {
            if (ch == '|')
            {
                token_ended = false;
                current_token.str.push_back(ch);
                continue;
            }
            else
            {
                DPRINTF("A comma was expected between tokens (Position %Id inside parenthesis). ", i+1);
                return LineError(group);
            }
        }

        if (current_token.Empty())
        {
            if ((ch >= '0' && ch <= '9') || ch == '-')
            {
                current_token.type = TOKEN_INTEGER;
            }
            else if (ch == '|')
            {
                DPRINTF("Token can't start with '|' ");
                return LineError(group);
            }
        }
        else if (current_token.type == TOKEN_INTEGER)
        {
            if (ch < '0' || ch > '9')
            {
                if ((ch == 'x' || ch == 'X') && current_token.str.length() == 1 && current_token.str[0] == '0')
                {
                    // Hex, allowed
                }
                else if (((ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')) && current_token.str.substr(0, 2) == "0x")
                {
                    // Hex, allowed
                }
                else
                {
                    current_token.type = TOKEN_IDENTIFIER;
                }
            }
        }

        current_token.str.push_back(ch);
    }

    if (in_quotes)
    {
        DPRINTF("Unclosed quotes. ");
        return LineError(group);
    }

    if (current_token.Empty())
    {
        if (tokens.size() != 0)
        {
            DPRINTF("A comma cannot be placed at the end. ");
            return LineError(group);
        }
    }
    else
    {
        SaveTokenAndReset(current_token, line_num, file, tokens);
    }

    return true;
}

bool Xv2QuestCompiler::IsKnownType(const ci_string &id)
{
    return (all_types.find(id) != all_types.end());
}

bool Xv2QuestCompiler::IsTopType(const ci_string &id)
{
    return (top_types.find(id) != top_types.end());
}

bool Xv2QuestCompiler::LineError(const X2QcToken &token)
{
    DPRINTF("At line %d. (file \"%s\")\n", token.line_num, token.file.c_str());
    return false;
}

bool Xv2QuestCompiler::ExpectTokenType(const X2QcToken &token, int expected_type)
{
    if (token.type != expected_type)
    {
        // Allow exception of expected float but got integer
        if (expected_type == TOKEN_FLOAT && token.type == TOKEN_INTEGER)
            return true;

        DPRINTF("Expected %s, but got %s. ", X2QcToken::GTokenName(expected_type).c_str(), token.TokenName(true).c_str());
        return LineError(token);
    }

    if (token.type == TOKEN_IDENTIFIER)
    {
        if (token.str.length() == 0)
        {
            DPRINTF("Empty token? ");
            return LineError(token);
        }

        if (token.str[0] >= '0' && token.str[0] <= '9')
        {
            DPRINTF("Bad identifier \"%s\", cannot start with number. ", token.str.c_str());
            return LineError(token);
        }

        for (char ch : token.str.str)
        {
            if (ch >= '0' && ch <= '9') {}
            else if (ch >= 'a' && ch <= 'z') {}
            else if (ch >= 'A' && ch <= 'Z') {}
            else if (ch == '_') {}
            else
            {
                DPRINTF("Bad identifier \"%s\", identifier can only contain ascii alphanumeric and underscore. ", token.str.c_str());
                return LineError(token);
            }
        }
    }

    return true;
}

bool Xv2QuestCompiler::ExpectTokenType(const X2QcToken &token, const std::vector<int> &expected_types)
{
    std::string expected_string;

    for (size_t i = 0; i < expected_types.size(); i++)
    {
        int expected_type = expected_types[i];

        if (expected_type == token.type)
        {
            if (token.type == TOKEN_IDENTIFIER)
                return ExpectTokenType(token, TOKEN_IDENTIFIER);

            return true;
        }

        // Allow exception of expected float but got integer
        if (expected_type == TOKEN_FLOAT && token.type == TOKEN_INTEGER)
            return true;

        expected_string += X2QcToken::GTokenName(expected_type);

        if (i == expected_types.size()-2)
        {
            expected_string += "or ";
        }
        else if (i != expected_types.size()-1)
        {
            expected_string += ", ";
        }
    }

    DPRINTF("Expected %s, but got %s. ", expected_string.c_str(), token.TokenName(true).c_str());
    return LineError(token);
}

bool Xv2QuestCompiler::GetParam(const X2QcToken &param_token, X2QcToken &value_token, const std::vector<ci_string> &params, const std::vector<int> &types, std::vector<bool> &defined)
{
    if (!ExpectTokenType(param_token, TOKEN_IDENTIFIER))
        return false;

    auto it = std::find(params.begin(), params.end(), param_token.str);
    if (it == params.end())
    {
        DPRINTF("Unrecognized param \"%s\". ", param_token.str.c_str());
        return LineError(param_token);
    }

    size_t i = it - params.begin();
    if (defined[i])
    {
        DPRINTF("Param \"%s\" had already been defined. ", param_token.str.c_str());
        return LineError(param_token);
    }

    defined[i] = true;

    if (tokens.empty())
    {
        DPRINTF("Was expecting \":\" but found end of file.\n");
        return false;
    }

    X2QcToken token = tokens.front();
    tokens.pop();

    if (!ExpectTokenType(token, TOKEN_COLON))
        return false;

    if (tokens.empty())
    {
        DPRINTF("Premature end of file while expecting parameter value\n");
        return false;
    }

    value_token = tokens.front();
    tokens.pop();

    if (types[i] == TOKEN_POLI_INTEGER_IDENTIFIER)
    {
        return ExpectTokenType(value_token, { TOKEN_INTEGER, TOKEN_IDENTIFIER } );
    }
    else if (types[i] == TOKEN_POLI_INTEGER_STRING_IDENTIFIER)
    {
        return ExpectTokenType(value_token, { TOKEN_INTEGER, TOKEN_STRING, TOKEN_IDENTIFIER } );
    }
    else if (types[i] == TOKEN_POLI_INTEGER_STRING)
    {
        return ExpectTokenType(value_token, { TOKEN_INTEGER, TOKEN_STRING } );
    }
    else if (types[i] == TOKEN_POLI_STRING_IDENTIFIER)
    {
        return ExpectTokenType(value_token, { TOKEN_STRING, TOKEN_IDENTIFIER });
    }

    return ExpectTokenType(value_token, types[i]);
}

bool Xv2QuestCompiler::ProcessVarDeclaration(ci_string &ret)
{
    if (tokens.empty())
    {
        DPRINTF("Premature end of file while waiting for variable name.\n");
        return false;
    }

    X2QcToken token = tokens.front();
    tokens.pop();

    if (!ExpectTokenType(token, TOKEN_IDENTIFIER))
        return false;

    if (reserved_kw.find(token.str) != reserved_kw.end())
    {
        DPRINTF("\"%s\" is a reserved keyword and cannot be used as variable name. ", token.str.c_str());
        return LineError(token);
    }

    if (defined_names.find(token.str) != defined_names.end())
    {
        DPRINTF("\"%s\" had already been defined. ", token.str.c_str());
        return LineError(token);
    }

    defined_names.insert(token.str);

    ret = token.str;
    return true;
}

bool Xv2QuestCompiler::ExpectBracketBegin()
{
    if (tokens.empty())
    {
        DPRINTF("Premature end of file while waiting for \"{\"\n");
        return false;
    }

    X2QcToken token = tokens.front();
    tokens.pop();

    return ExpectTokenType(token, TOKEN_BRACKET_BEGIN);
}

bool Xv2QuestCompiler::ExpectColon()
{
    if (tokens.empty())
    {
        DPRINTF("Premature end of file while waiting for \":\"\n");
        return false;
    }

    X2QcToken token = tokens.front();
    tokens.pop();

    return ExpectTokenType(token, TOKEN_COLON);
}

bool Xv2QuestCompiler::GetItemType(X2QcToken &token, uint16_t *item_type)
{
    auto it = constant_to_item_type.find(token.str);
    if (it == constant_to_item_type.end())
    {
        DPRINTF("\"%s\" is not a valid constant for item type. ", token.str.c_str());
        return LineError(token);
    }

    *item_type = it->second;
    return true;
}

XQ_X2mMod *Xv2QuestCompiler::FindModByVar(const ci_string &var_name)
{
    auto it = compiled_mods.find(var_name);
    if (it == compiled_mods.end())
        return nullptr;

    return &compiled_mods[var_name];
}

bool Xv2QuestCompiler::GetModCommon(const X2QcToken &token, XQ_X2mMod *mod)
{
    XQ_X2mMod *vmod = FindModByVar(token.str);
    if (!vmod)
    {
        if (defined_names.find(token.str) == defined_names.end())
        {
            DPRINTF("\"%s\" has not been declared. ", token.str.c_str());
            return LineError(token);
        }
        else
        {
            DPRINTF("\"%s\" is not of X2mMod type. ", token.str.c_str());
            return LineError(token);
        }
    }

    if (test_mode)
    {
        mod->guid = vmod->guid;
        mod->name = vmod->name;

        mod->code = "X2M";
        mod->id = 3000;
        mod->path = "";
        mod->var_name = token.str;

        return true;
    }

    auto it = mods_table.find(vmod->guid);
    if (it == mods_table.end())
    {
        DPRINTF("This quest requires the mod \"%s\" to be installed on the system.\n", vmod->name.c_str());
        return false;
    }

    *mod = it->second;
    return true;
}

bool Xv2QuestCompiler::GetCostumeMod(const X2QcToken &token, int item_type, int *value)
{
    int type;
    std::string component;

    if (item_type == QXD_ITEM_TOP)
    {
        type = COSTUME_TOP;
        component = "top";
    }
    else if (item_type == QXD_ITEM_BOTTOM)
    {
        type = COSTUME_BOTTOM;
        component = "bottom";
    }
    else if (item_type == QXD_ITEM_GLOVES)
    {
        type = COSTUME_GLOVES;
        component = "gloves";
    }
    else if (item_type == QXD_ITEM_SHOES)
    {
        type = COSTUME_SHOES;
        component = "shoes";
    }
    else if (item_type == QXD_ITEM_ACCESSORY)
    {
        type = COSTUME_ACCESSORY;
        component = "accessory";
    }
    else
    {
        DPRINTF("You cannot use a x2m for this typo of item. Only for TOP,BOTTOM,GLOVES,SHOES and ACCESSORY. ");
        return LineError(token);
    }

    XQ_X2mMod mod;

    if (!GetModCommon(token, &mod))
        return false;

    if (test_mode)
    {
        *value = mod.id;
        return true;
    }

    if (mod.type != X2mType::NEW_COSTUME)
    {
        DPRINTF("Mod \"%s\" is not of costume type.\n", mod.name.c_str());
        return LineError(token);
    }

    X2mCostumeEntry &entry = mod.costume;
    for (size_t i = 0; i < entry.idb_entries.size(); i++)
    {
        if (entry.costume_types[i] == type)
        {
            *value = entry.idb_entries[i];
            return true;
        }
    }

    DPRINTF("Costume \"%s\" doesn't have a %s part.\n", mod.name.c_str(), component.c_str());
    return false;
}

bool Xv2QuestCompiler::GetSkillMod(const X2QcToken &token, int16_t *id1)
{
    XQ_X2mMod mod;

    if (!GetModCommon(token, &mod))
        return false;

    if (test_mode)
    {
        *id1 = mod.id;
        return true;
    }

    if (mod.type != X2mType::NEW_SKILL)
    {
        DPRINTF("Mod \"%s\" is not of skill type. ", mod.name.c_str());
        return LineError(token);
    }

    *id1 = (int32_t)mod.id;
    return true;
}

bool Xv2QuestCompiler::GetCharMod(const X2QcToken &token, int16_t *cms_id)
{
    XQ_X2mMod mod;

    if (!GetModCommon(token, &mod))
        return false;

    if (test_mode)
    {
        *cms_id = mod.id;
        return true;
    }

    if (mod.type != X2mType::NEW_CHARACTER)
    {
        DPRINTF("Mod \"%s\" is not of character type. ", mod.name.c_str());
        return LineError(token);
    }

    *cms_id = (int32_t)mod.id;
    return true;
}

bool Xv2QuestCompiler::GetCharMod(const X2QcToken &token, char *cms_code)
{
    XQ_X2mMod mod;

    if (!GetModCommon(token, &mod))
        return false;

    if (test_mode)
    {
        strncpy(cms_code, mod.code.c_str(), 3);
        return true;
    }

    if (mod.type != X2mType::NEW_CHARACTER)
    {
        DPRINTF("Mod \"%s\" is not of character type. ", mod.name.c_str());
        return LineError(token);
    }

    // No need to check bounds, that was already done in x2m load
    strcpy(cms_code, mod.code.c_str());
    return true;
}

bool Xv2QuestCompiler::GetStageMod(const X2QcToken &token, int16_t *stage_id)
{
    XQ_X2mMod mod;

    if (!GetModCommon(token, &mod))
        return false;

    if (test_mode)
    {
        *stage_id = 0x77;
        return true;
    }

    if (mod.type != X2mType::NEW_STAGE)
    {
        DPRINTF("Mod \"%s\" is not of stage type. ", mod.name.c_str());
        return LineError(token);
    }

    *stage_id = (int32_t)mod.id;
    return true;
}

bool Xv2QuestCompiler::GetSkill(const X2QcToken &token, int16_t *id2, uint32_t *type, bool allow_blast)
{
    int16_t id1;

    if (token.type == TOKEN_INTEGER)
    {
        id1 = token.num;
    }
    else if (token.type == TOKEN_STRING)
    {
        if (token.str.length() > 4)
        {
            DPRINTF("Invalid skill code \"%s\" ", token.str.c_str());
            return LineError(token);
        }

        std::vector<CusSkill *> skills;

        if (game_cus->FindAnySkillByName(token.str.str, skills) == 0)
        {
            DPRINTF("Cannot resolve skill \"%s\" -skill not found in system- ", token.str.c_str());
            return LineError(token);
        }
        else if (skills.size() != 1)
        {
            DPRINTF("Cannot resolve skill \"%s\" because there are multiple skills with that code.\n"
                    "Please use an integer identifier for vanilla skills, and a X2mMod object for x2m skills.\n", token.str.c_str());
            return LineError(token);
        }

        id1 = skills[0]->id;
    }
    else
    {
        // Identifier
        if (!GetSkillMod(token, &id1))
            return false;

        //DPRINTF("%d\n", id1);
    }

    if (id1 < 0)
    {
        DPRINTF("Negative skill id \"%d\" not allowed here. ", id1);
        return LineError(token);
    }
    else if (id1 < CUS_ULTIMATE_START)
    {
        *id2 = id1;
        *type = QXD_SKILL_SUPER;
    }
    else if (id1 < CUS_EVASIVE_START)
    {
        *id2 = (id1 - CUS_ULTIMATE_START);
        *type = QXD_SKILL_ULTIMATE;
    }
    else if (id1 < CUS_UNK_START)
    {
        *id2 = (id1 - CUS_EVASIVE_START);
        *type = QXD_SKILL_EVASIVE;
    }
    else if (id1 < CUS_BLAST_START)
    {
        DPRINTF("Invalid skill id \"%d\". ", id1);
        return LineError(token);
    }
    else if (id1 < CUS_AWAKEN_START)
    {
        if (!allow_blast)
        {
            DPRINTF("Blast skills are not allowed here (\"%s\"). ", token.str.c_str());
            return LineError(token);
        }

        *id2 = (id1 - CUS_BLAST_START);
        *type = QXD_SKILL_BLAST;
    }
    else if (id1 < 30000)
    {
        *id2 = (id1 - CUS_AWAKEN_START);
        *type = QXD_SKILL_AWAKEN;
    }

    return true;
}

bool Xv2QuestCompiler::GetSuperSkill(const X2QcToken &token, int16_t *id2)
{
    int16_t id1;

    if (token.type == TOKEN_INTEGER)
    {
        id1 = token.num;
    }
    else if (token.type == TOKEN_STRING)
    {
        if (token.str.length() > 4)
        {
            DPRINTF("Invalid skill code \"%s\" ", token.str.c_str());
            return LineError(token);
        }

        std::vector<CusSkill *> skills;

        if (game_cus->FindSuperSkillByName(token.str.str, skills) == 0)
        {
            DPRINTF("Cannot resolve super skill \"%s\" -skill not found in system or not super- ", token.str.c_str());
            return LineError(token);
        }
        else if (skills.size() != 1)
        {
            DPRINTF("Cannot resolve skill \"%s\" because there are multiple skills with that code.\n"
                    "Please use an integer identifier for vanilla skills, and a X2mMod object for x2m skills.\n", token.str.c_str());
            return LineError(token);
        }

        id1 = skills[0]->id;
    }
    else
    {
        // Identifier
        if (!GetSkillMod(token, &id1))
            return false;

        if (test_mode)
        {
            id1 = 4999;
        }
    }

    if (id1 < 0)
    {
        *id2 = id1;
    }
    else if (id1 < CUS_ULTIMATE_START)
    {
        *id2 = id1;
    }
    else
    {
        DPRINTF("Skill \"%s\" is not of super type. ", token.str.c_str());
        return LineError(token);
    }

    return true;
}

bool Xv2QuestCompiler::GetUltimateSkill(const X2QcToken &token, int16_t *id2)
{
    int16_t id1;

    if (token.type == TOKEN_INTEGER)
    {
        id1 = token.num;
    }
    else if (token.type == TOKEN_STRING)
    {
        if (token.str.length() > 4)
        {
            DPRINTF("Invalid skill code \"%s\" ", token.str.c_str());
            return LineError(token);
        }

        std::vector<CusSkill *> skills;

        if (game_cus->FindUltimateSkillByName(token.str.str, skills) == 0)
        {
            DPRINTF("Cannot resolve super skill \"%s\" -skill not found in system or not ultimate- ", token.str.c_str());
            return LineError(token);
        }
        else if (skills.size() != 1)
        {
            DPRINTF("Cannot resolve skill \"%s\" because there are multiple skills with that code.\n"
                    "Please use an integer identifier for vanilla skills, and a X2mMod object for x2m skills.\n", token.str.c_str());
            return LineError(token);
        }

        id1 = skills[0]->id;
    }
    else
    {
        // Identifier
        if (!GetSkillMod(token, &id1))
            return false;

        if (test_mode)
        {
            id1 = 9999;
        }
    }

    if (id1 < 0)
    {
        *id2 = id1;
    }
    else if (id1 >= CUS_ULTIMATE_START && id1 < CUS_EVASIVE_START)
    {
        *id2 = (id1-CUS_ULTIMATE_START);
    }
    else
    {
        DPRINTF("Skill \"%s\" is not of ultimate type. ", token.str.c_str());
        return LineError(token);
    }

    return true;
}

bool Xv2QuestCompiler::GetEvasiveSkill(const X2QcToken &token, int16_t *id2)
{
    int16_t id1;

    if (token.type == TOKEN_INTEGER)
    {
        id1 = token.num;
    }
    else if (token.type == TOKEN_STRING)
    {
        if (token.str.length() > 4)
        {
            DPRINTF("Invalid skill code \"%s\" ", token.str.c_str());
            return LineError(token);
        }

        std::vector<CusSkill *> skills;

        if (game_cus->FindEvasiveSkillByName(token.str.str, skills) == 0)
        {
            DPRINTF("Cannot resolve evasive skill \"%s\" -skill not found in system or not evasive- ", token.str.c_str());
            return LineError(token);
        }
        else if (skills.size() != 1)
        {
            DPRINTF("Cannot resolve skill \"%s\" because there are multiple skills with that code.\n"
                    "Please use an integer identifier for vanilla skills, and a X2mMod object for x2m skills.\n", token.str.c_str());
            return LineError(token);
        }

        id1 = skills[0]->id;
    }
    else
    {
        // Identifier
        if (!GetSkillMod(token, &id1))
            return false;

        if (test_mode)
        {
            id1 = 14999;
        }
    }

    if (id1 < 0)
    {
        *id2 = id1;
    }
    else if (id1 >= CUS_EVASIVE_START && id1 < CUS_UNK_START)
    {
        *id2 = (id1-CUS_EVASIVE_START);
    }
    else
    {
        DPRINTF("Skill \"%s\" is not of evasive type. ", token.str.c_str());
        return LineError(token);
    }

    return true;
}

bool Xv2QuestCompiler::GetBlastSkill(const X2QcToken &token, int16_t *id2)
{
    int16_t id1;

    if (token.type == TOKEN_INTEGER)
    {
        id1 = token.num;
    }
    else if (token.type == TOKEN_STRING)
    {
        if (token.str.length() > 4)
        {
            DPRINTF("Invalid skill code \"%s\" ", token.str.c_str());
            return LineError(token);
        }

        std::vector<CusSkill *> skills;

        if (game_cus->FindBlastSkillByName(token.str.str, skills) == 0)
        {
            DPRINTF("Cannot resolve blast skill \"%s\" -skill not found in system or not blast- ", token.str.c_str());
            return LineError(token);
        }
        else if (skills.size() != 1)
        {
            DPRINTF("Cannot resolve skill \"%s\" because there are multiple skills with that code.\n"
                    "Please use an integer identifier for vanilla skills, and a X2mMod object for x2m skills.\n", token.str.c_str());
            return LineError(token);
        }

        id1 = skills[0]->id;
    }
    else
    {
        // Identifier
        if (!GetSkillMod(token, &id1))
            return false;

        if (test_mode)
        {
            id1 = 24999;
        }
    }

    if (id1 < 0)
    {
        *id2 = id1;
    }
    else if (id1 >= CUS_BLAST_START && id1 < CUS_AWAKEN_START)
    {
        *id2 = (id1-CUS_BLAST_START);
    }
    else
    {
        DPRINTF("Skill \"%s\" is not of blast type. ", token.str.c_str());
        return LineError(token);
    }

    return true;
}

bool Xv2QuestCompiler::GetAwakenSkill(const X2QcToken &token, int16_t *id2)
{
    int16_t id1;

    if (token.type == TOKEN_INTEGER)
    {
        id1 = token.num;
    }
    else if (token.type == TOKEN_STRING)
    {
        if (token.str.length() > 4)
        {
            DPRINTF("Invalid skill code \"%s\" ", token.str.c_str());
            return LineError(token);
        }

        std::vector<CusSkill *> skills;

        if (game_cus->FindAwakenSkillByName(token.str.str, skills) == 0)
        {
            DPRINTF("Cannot resolve evasive skill \"%s\" -skill not found in system or not awaken- ", token.str.c_str());
            return LineError(token);
        }
        else if (skills.size() != 1)
        {
            DPRINTF("Cannot resolve skill \"%s\" because there are multiple skills with that code.\n"
                    "Please use an integer identifier for vanilla skills, and a X2mMod object for x2m skills.\n", token.str.c_str());
            return LineError(token);
        }

        id1 = skills[0]->id;
    }
    else
    {
        // Identifier
        if (!GetSkillMod(token, &id1))
            return false;

        if (test_mode)
        {
            id1 = 29999;
        }
    }

    if (id1 < 0)
    {
        *id2 = id1;
    }
    else if (id1 >= CUS_AWAKEN_START && id1 < 30000)
    {
        *id2 = (id1-CUS_AWAKEN_START);
    }
    else
    {
        DPRINTF("Skill \"%s\" is not of evasive type. ", token.str.c_str());
        return LineError(token);
    }

    return true;
}

bool Xv2QuestCompiler::GetChar(const X2QcToken &token, int16_t *cms_id)
{
    if (token.type == TOKEN_INTEGER)
    {
        *cms_id = (int16_t)token.num;
    }
    else if (token.type == TOKEN_STRING)
    {
        if (token.str.length() > 3)
        {
            DPRINTF("Invalid chara code \"%s\" ", token.str.c_str());
            return LineError(token);
        }

        CmsEntry *cms = game_cms->FindEntryByName(Utils::ToUpperCase(token.str.str));
        if (!cms)
        {
            DPRINTF("Character \"%s\" is not installed in this system.\n", token.str.c_str());
            return false;
        }

        *cms_id = (int16_t)(int32_t)cms->id;
    }
    else
    {
        // Identifier
        if (!GetCharMod(token, cms_id))
            return false;
    }

    return true;
}

bool Xv2QuestCompiler::GetChar(const X2QcToken &token, char *cms_code)
{
    if (token.type == TOKEN_INTEGER)
    {
        if (token.num < 0)
        {
            DPRINTF("A cms id smaller than 0 is not allowed here. ");
            return LineError(token);
        }

        CmsEntry *cms = game_cms->FindEntryByID(token.num);
        if (!cms)
        {
            DPRINTF("Character \"%s\" is not installed in this system. Please, a use X2mMod object if this is a x2m char.\n", token.str.c_str());
            return false;
        }

        if (cms->name.length() > 3)
        {
            DPRINTF("Didn't expect a cms greater than 3 (%s -> %s).\n", token.str.c_str(), cms->name.c_str());
            return false;
        }

        strcpy(cms_code, Utils::ToUpperCase(cms->name).c_str());
    }
    else if (token.type == TOKEN_STRING)
    {
        if (token.str.length() > 3)
        {
            DPRINTF("Invalid chara code \"%s\" ", token.str.c_str());
            return LineError(token);
        }

        strcpy(cms_code, Utils::ToUpperCase(token.str.str).c_str());
    }
    else
    {
        // Identifier
        if (!GetCharMod(token, cms_code))
            return false;
    }

    return true;
}

bool Xv2QuestCompiler::GetQuest(const X2QcToken &token, int16_t *quest_id, bool allow_self)
{
    // TODO: x2m
    if (compiled_quest.name.length() == 0)
    {
        DPRINTF("%s: Bug, this function shouldn't be called before quest is defined.\n", FUNCNAME);
        return false;
    }

    if (token.type == TOKEN_INTEGER)
    {
        *quest_id = (int16_t)token.num;
    }
    else if (token.type == TOKEN_STRING)
    {
        if (!allow_self)
        {
            if (Utils::ToUpperCase(token.str.str) == compiled_quest.name)
            {
                DPRINTF("You cannot self-reference your own quest here. ");
                return LineError(token);
            }
        }

        QxdQuest *quest = active_qxd.FindQuestByName(token.str.str);
        if (!quest)
        {
            DPRINTF("Cannot resolve quest \"%s\". ", token.str.c_str());

            if (Utils::ToUpperCase(token.str.str.substr(0, 4)) != compiled_quest.name.substr(0, 4))
            {
                DPRINTF("A remainder that you can only reference quest of your own type here.\n");
            }

            DPRINTF("If referenced quest is a x2m quest, please use a X2mMod object for better error output.\n");
            return LineError(token);
        }

        *quest_id = (uint16_t) quest->id;
    }

    return true;
}

bool Xv2QuestCompiler::GetStage(const X2QcToken &token, int16_t *stage_id)
{
    if (token.type == TOKEN_INTEGER)
    {
        *stage_id = token.num;
    }
    else if (token.type == TOKEN_STRING)
    {
        size_t idx;
        Xv2Stage *stage = game_stage_def->GetStageByCode(token.str.str, &idx);
        if (!stage)
        {
            DPRINTF("Stage \"%s\" doesn't exist in this system.\n", token.str.c_str());
            return false;
        }

        // Fix for PRB_0200
        if (stage->code == "SANDBOX" && idx == 0x3F)
        {
            idx = 0x41;
        }

        *stage_id = (int16_t)idx;
    }
    else
    {
        // Identifier
        if (!GetStageMod(token, stage_id))
            return false;
    }

    return true;
}

bool Xv2QuestCompiler::GetTitle(const X2QcToken &token, std::string &msg_code, int num)
{
    if (token.type == TOKEN_STRING)
    {
        if (Utils::BeginsWith(token.str.str, "X2Q", false))
        {
            DPRINTF("A msg entry cannot start with \"X2Q\", since it is reserved for internal usage. For custom text, use TextEntry. ");
            return LineError(token);
        }

        msg_code = token.str.str;
    }
    else
    {
        // Identifier
        auto it = compiled_text_entry.find(token.str);
        if (it == compiled_text_entry.end())
        {
            if (defined_names.find(token.str) == defined_names.end())
            {
                DPRINTF("\"%s\" hasn't been defined. ", token.str.c_str());
            }
            else
            {
                DPRINTF("\"%s\" is not of TextEntry type. ", token.str.c_str());
            }

            return LineError(token);
        }

        std::vector<std::string> &text = it->second;
        std::string entry = "X2Q_" + compiled_quest.name + "_" + Utils::ToString(num);

        for (int lang = 0; lang < XV2_LANG_NUM; lang++)
        {
            if (!SetTitleEntry(entry, text[lang], lang))
            {
                DPRINTF("%s: SetTitleEntry failed with entry \"%s\".\n", FUNCNAME, entry.c_str());
                return false;
            }
        }

        msg_code = entry;
    }

    return true;
}

bool Xv2QuestCompiler::GetDialogueTA(const X2QcToken &token, std::string &msg_code)
{
    if (token.type == TOKEN_STRING)
    {
        if (Utils::BeginsWith(token.str.str, "X2Q", false))
        {
            DPRINTF("A msg entry cannot start with \"X2Q\", since it is reserved for internal usage. For custom text, use TextEntry or TextAudioEntry. ");
            return LineError(token);
        }

        msg_code = token.str.str;
    }
    else
    {
        auto it = compiled_text_entry.find(token.str);
        if (it == compiled_text_entry.end())
        {
            if (defined_names.find(token.str) == defined_names.end())
            {
                DPRINTF("\"%s\" hasn't been defined. ", token.str.c_str());
            }
            else
            {
                DPRINTF("\"%s\" is not of TextEntry or TextAudioEntry type. ", token.str.c_str());
            }

            return LineError(token);
        }

        std::vector<std::string> &text = it->second;
        std::string entry = "X2" + compiled_quest.name + "_" + Utils::ToStringAndPad(global_dialogue_index, 2);

        for (int lang = 0; lang < (int)dialogue.size(); lang++)
        {
            std::string t;

            if (lang >= (int)text.size())
            {
                t = "";
            }
            else
            {
                t = text[lang];
            }

            if (lang == XV2_LANG_NUM)
            {
                if (Utils::BeginsWith(t, "file:", false))
                {
                    if (!SupportsAudio())
                    {
                        DPRINTF("This kind of quest doesn't support audio.\n");
                        return LineError(token);
                    }

                    ci_string hca_file;
                    hca_file.str = t.substr(strlen("file:"));

                    //if (processed_audio_files.find(ci_string(hca_file)) == processed_audio_files.end())
                    if (true)
                    {
                        if (!SetDialogueAudioFromResource(hca_file.str, entry))
                            return false;

                        processed_audio_files.insert(hca_file);
                    }

                    t = entry;
                }
            }

            if (!SetDialogueEntry(entry, t, lang))
            {
                DPRINTF("%s: SetDialogueEntry failed with entry \"%s\".\n", FUNCNAME, entry.c_str());
                return false;
            }
        }

        global_dialogue_index++;
        msg_code = entry;
    }

    return true;
}

bool Xv2QuestCompiler::GetItemCollection(const X2QcToken &token, uint32_t *id)
{
    if (compiled_quest.name.length() == 0)
    {
        DPRINTF("%s: BUG, shouldn't be here, quest undefined.\n", FUNCNAME);
        return false;
    }

    auto it = compiled_item_collections.find(token.str);
    if (it == compiled_item_collections.end())
    {
        if (defined_names.find(token.str) == defined_names.end())
        {
            DPRINTF("\"%s\" hasn't been defined ", token.str.c_str());
        }
        else
        {
            DPRINTF("\"%s\" is not of ItemCollection type. ", token.str.c_str());
        }

        return LineError(token);
    }

    QxdCollection &collection = it->second;
    std::vector<QxdCollection *> existing_collections;

    if (active_qxd.FindSimilarCollections(collection, existing_collections) > 0)
    {
        uint16_t suggested_id = 0xFFFF;
        if (token.str.length() > strlen("Collection") && Utils::BeginsWith(token.str.str, "Collection", false))
        {
            std::string num_str = token.str.str.substr(strlen("Collection"));

            if (Utils::HasOnlyDigits(num_str))
                suggested_id = (uint16_t)Utils::GetUnsigned(num_str);
        }

        size_t index = 0;
        if (suggested_id != 0xFFFF)
        {
            for (size_t i = 0; i < existing_collections.size(); i++)
            {
                if (suggested_id == existing_collections[i]->id)
                {
                    index = i;
                    break;
                }
            }
        }

        *id = existing_collections[index]->id;
        //DPRINTF("Reusing existing collection %d for %s.\n", *id, token.str.c_str());
    }
    else
    {
        //DPRINTF("Adding collection \"%s\"\n", token.str.c_str());

        if (!active_qxd.AddCollection(collection, true, start_new_col_id_search[quest_type]))
        {
            DPRINTF("Failed to add a collection to system, qxd doesn't allow more collections.\n");
            return false;
        }

        *id = collection.id;
    }

    return true;
}

bool Xv2QuestCompiler::GetQxdChar(const X2QcToken token, uint32_t *id)
{
    if (!ExpectTokenType(token, TOKEN_IDENTIFIER))
        return false;

    auto it = compiled_chars.find(token.str);
    if (it == compiled_chars.end())
    {
        auto it = compiled_special_chars.find(token.str);

        if (it == compiled_special_chars.end())
        {
            if (defined_names.find(token.str) == defined_names.end())
            {
                DPRINTF("\"%s\" wasn't defined here. ", token.str.c_str());
            }
            else
            {
                DPRINTF("\"%s\" is not of QxdChar or QxdSpecialChar type. ", token.str.c_str());
            }

            return LineError(token);
        }

        QxdCharacter &chara = it->second;

        if (chara.id == 0xFFFFFFFF)
        {
            QxdCharacter *existing = active_qxd.FindSimilarSpecialChar(chara);
            if (existing)
            {
                chara.id = existing->id;
                *existing = chara;
            }
            else
            {
                active_qxd.AddSpecialCharWithNewId(chara, start_new_qxd_char_id_search[quest_type]);
            }
        }

        *id = chara.id;
    }
    else
    {
        QxdCharacter &chara = it->second;

        if (chara.id == 0xFFFFFFFF)
        {
            QxdCharacter *existing = active_qxd.FindSimilarChar(chara);
            if (existing)
            {
                chara.id = existing->id;
                *existing = chara;
            }
            else
            {
                active_qxd.AddCharWithNewId(chara, start_new_qxd_char_id_search[quest_type]);
            }
        }

        *id = chara.id;
    }

    return true;
}

bool Xv2QuestCompiler::GetQmlChar(const X2QcToken token, uint32_t *id, bool allow_negative)
{
    if (!ExpectTokenType(token, { TOKEN_IDENTIFIER, TOKEN_INTEGER }))
        return false;

    if (token.type == TOKEN_INTEGER)
    {
        if (!allow_negative && token.num < 0)
        {
            DPRINTF("A negative qml id is not allowed here. ");
            return LineError(token);
        }

        *id = token.num;
    }
    else
    {
        auto it = compiled_qml_chars.find(token.str);
        if (it == compiled_qml_chars.end())
        {
            if (defined_names.find(token.str) == defined_names.end())
            {
                DPRINTF("\"%s\" wasn't defined here. ", token.str.c_str());
            }
            else
            {
                DPRINTF("\"%s\" is not of QmlChar type. ", token.str.c_str());
            }

            return LineError(token);
        }

        *id = it->second.id;
    }

    return true;
}

bool Xv2QuestCompiler::GetDialogue(const X2QcToken token, uint32_t *index, bool allow_negative)
{
    if (!ExpectTokenType(token, { TOKEN_IDENTIFIER, TOKEN_INTEGER }))
        return false;

    if (token.type == TOKEN_INTEGER)
    {
        if (!allow_negative && token.num < 0)
        {
            DPRINTF("A negative dialogue index is not allowed here. ");
            return LineError(token);
        }

        *index = token.num;
    }
    else
    {
        auto it = compiled_dialogues.find(token.str);
        if (it == compiled_dialogues.end())
        {
            if (defined_names.find(token.str) == defined_names.end())
            {
                DPRINTF("\"%s\" wasn't defined here. ", token.str.c_str());
            }
            else
            {
                DPRINTF("\"%s\" is not of Dialogue type. ", token.str.c_str());
            }

            return LineError(token);
        }

        *index = it->second;
    }

    return true;
}

bool Xv2QuestCompiler::GetInteractiveDialogue(const X2QcToken token, uint32_t *index, bool allow_negative)
{
    if (!ExpectTokenType(token, { TOKEN_IDENTIFIER, TOKEN_INTEGER }))
        return false;

    if (token.type == TOKEN_INTEGER)
    {
        if (!allow_negative && token.num < 0)
        {
            DPRINTF("A negative interactive dialogue index is not allowed here. ");
            return LineError(token);
        }

        *index = token.num;
    }
    else
    {
        auto it = compiled_interactive_dialogues.find(token.str);
        if (it == compiled_interactive_dialogues.end())
        {
            if (defined_names.find(token.str) == defined_names.end())
            {
                DPRINTF("\"%s\" wasn't defined here. ", token.str.c_str());
            }
            else
            {
                DPRINTF("\"%s\" is not of InteractiveDialogue type. ", token.str.c_str());
            }

            return LineError(token);
        }

        *index = it->second;
    }

    return true;
}

bool Xv2QuestCompiler::GetFlag(const X2QcToken token, uint32_t *flag)
{
    if (!ExpectTokenType(token, TOKEN_IDENTIFIER))
        return false;

    auto it = compiled_flags.find(token.str);
    if (it == compiled_flags.end())
    {
        if (defined_names.find(token.str) == defined_names.end())
        {
            DPRINTF("\"%s\" wasn't defined here. ", token.str.c_str());
        }
        else
        {
            DPRINTF("\"%s\" is not of Flag type. ", token.str.c_str());
        }

        return LineError(token);
    }

    *flag = it->second;
    return true;
}

bool Xv2QuestCompiler::GetStringVar(const X2QcToken token, uint32_t *string_var)
{
    if (!ExpectTokenType(token, TOKEN_IDENTIFIER))
        return false;

    auto it = compiled_string_vars.find(token.str);
    if (it == compiled_string_vars.end())
    {
        if (defined_names.find(token.str) == defined_names.end())
        {
            DPRINTF("\"%s\" wasn't defined here. ", token.str.c_str());
        }
        else
        {
            DPRINTF("\"%s\" is not of StringVar type. ", token.str.c_str());
        }

        return LineError(token);
    }

    *string_var = it->second;
    return true;
}

bool Xv2QuestCompiler::DecompileX2mMod(const XQ_X2mMod &mod, std::ostringstream &oss)
{
    WriteIndent(oss, true);
    oss << "X2mMod " << mod.var_name.c_str() << '\n';
    WriteIndent(oss, true);
    oss << "{\n";
    indent_level++;

    WriteStringParam(oss, "name", mod.name);
    WriteStringParam(oss, "guid", mod.guid);

    indent_level--;
    WriteIndent(oss, true);
    oss << "}\n\n";

    return true;
}

bool Xv2QuestCompiler::DecompileTextEntry(const std::string &msg_code, std::vector<MsgFile> &msg, bool audio, std::ostringstream &oss, bool title)
{
    if (audio)
    {
        if (!msg[XV2_LANG_NUM].FindEntryByName(msg_code))
        {
            audio = false;
        }
    }

    WriteIndent(oss, true);
    oss << ((audio) ? "TextAudioEntry " : "TextEntry ");

    if (title)
        oss << referenced_title[msg_code].str << '\n';
    else
        oss << referenced_dialogue_ta[msg_code].str << '\n';

    WriteIndent(oss, true);
    oss << "{\n";
    indent_level++;

    int num = (audio) ? (XV2_LANG_NUM+1) : XV2_LANG_NUM;

    for (int lang = 0; lang < num; lang++)
    {
        MsgEntry *entry = msg[lang].FindEntryByName(msg_code);
        if (entry)
        {
            WriteStringParam(oss, (lang == XV2_LANG_NUM) ? "voice" : xv2_lang_codes[lang], Xenoverse2::UnescapeHtml(entry->lines[0]));
        }
        else
        {
            if (lang == XV2_LANG_ENGLISH)
            {
                DPRINTF("%s: failed because didn't find english entry despite it was supossed to have been checked before.\n", FUNCNAME);
                return false;
            }
            else if (lang == XV2_LANG_NUM) // Voice
            {
                DPRINTF("%s: failed because couldn't find voice entry %s despite it was previously checked.\n", FUNCNAME, msg_code.c_str());
                return false;
            }
        }
    }

    indent_level--;
    WriteIndent(oss, true);
    oss << "}\n\n";

    return true;
}

bool Xv2QuestCompiler::DecompileItemCollectionEntry(const QxdCollectionEntry &entry, std::ostringstream &oss)
{
    std::string comment;

    WriteIndent(oss, true);
    oss << "ItemCollectionEntry { ";

    if (!WriteItemParam(oss, "item", "type", (int16_t)entry.item_id, entry.item_type, false, false, &comment))
        return false;

    WriteIntegerParam(oss, "i6", entry.unk_06, false);
    WriteIntegerParam(oss, "i8", entry.unk_08, false);
    WriteIntegerParam(oss, "i10", entry.unk_0A, false);

    oss << '}';

    if (comment.length() > 0)
        oss << " ; " << comment;

    oss << '\n';
    return true;
}

bool Xv2QuestCompiler::DecompileItemCollection(uint32_t item_collection, std::ostringstream &oss)
{
    QxdCollection *collection = active_qxd.FindCollection(item_collection);
    if (!collection)
    {
        DPRINTF("%s: Cannot find collection %d\n", FUNCNAME, item_collection);
        return false;
    }
    else if (collection->entries.size() == 0)
    {
        DPRINTF("%s: Collection %d is empty!\n", FUNCNAME, item_collection);
        return false;
    }

    ci_string var_name = referenced_collections[item_collection];

    WriteIndent(oss, true);
    oss << "ItemCollection " << var_name.str << '\n';
    WriteIndent(oss, true);
    oss << "{\n";
    indent_level++;

    for (const QxdCollectionEntry &entry : collection->entries)
    {
        if (!DecompileItemCollectionEntry(entry, oss))
            return false;
    }

    indent_level--;
    WriteIndent(oss, true);
    oss << "}\n\n";

    return true;
}

bool Xv2QuestCompiler::DecompileQxdUnk(const QxdUnk &unk, int type, std::ostringstream &oss)
{
    WriteIndent(oss, true);
    oss << "QxdUnk" << type << "(";

    for (int i = 0; i < 16; i++)
    {
        if (i != 0)
            oss << ", ";

        oss << unk.unk_00[i];
    }

    oss << ")\n";

    return true;
}

bool Xv2QuestCompiler::DecompileItemReward(const QxdItemReward &reward, std::ostringstream &oss)
{
    if (reward.flags != 0 && reward.flags != 1 && reward.flags != 2 && reward.flags != 5 && reward.flags != 6 && reward.flags != 9 && reward.flags != 10 && reward.flags != 11 && reward.flags != 12)
    {
        DPRINTF("%s: Unexpected value for flags (%d)\n", FUNCNAME, reward.flags);
        return false;
    }

    std::string comment;

    WriteIndent(oss, true);
    oss << "ItemReward { ";

    if (!WriteItemParam(oss, "item", "type", (int16_t)reward.id, reward.type, false, true, &comment))
        return false;

    WriteIntegerParam(oss, "condition", reward.condition, false); // TODO: Maybe find out about this?
    WriteIntegerParam(oss, "i12", reward.unk_0C, false);

    if (reward.flags == 1 || reward.flags == 5 || reward.flags == 6 || reward.flags == 9 || reward.flags == 10 || reward.flags == 11 || reward.flags == 12)
        WriteIntegerParam(oss, "flags", reward.flags, false);
    else //0, 2
        WriteIdentifierParam(oss, "flags", (reward.flags == 2) ? "HIDDEN" : "NORMAL", false);

    WriteIntegerParam(oss, "i20", reward.unk_14, false);
    WriteFloatParam(oss, "chance", reward.chance, false);

    oss << '}';

    if (comment.length() > 0)
        oss << " ; " << comment;

    oss << '\n';
    return true;
}

bool Xv2QuestCompiler::DecompileSkillReward(const QxdSkillReward &reward, std::ostringstream &oss)
{
    std::string comment;

    WriteIndent(oss, true);
    oss << "SkillReward { ";

    if (!WriteSkillParam(oss, "skill", reward.id2, reward.type, false, false, &comment))
        return false;

    WriteIntegerParam(oss, "condition", reward.condition, false);  // TODO: Maybe find out about this?
    WriteIntegerParam(oss, "i12", reward.unk_0C, false);
    WriteFloatParam(oss, "chance", reward.chance, false);

    oss << '}';

    if (comment.length() > 0)
        oss << " ; " << comment;

    oss << '\n';
    return true;
}

bool Xv2QuestCompiler::DecompileCharReward(const QxdCharReward &reward, std::ostringstream &oss)
{
    std::string comment;

    WriteIndent(oss, true);
    oss << "CharReward { ";

    if (!WriteCharParam(oss, "char", reward.cms_name, false, &comment, (int16_t)reward.costume_index))
        return false;

    WriteIntegerParam(oss, "costume", (int16_t)reward.costume_index, false);
    WriteIntegerParam(oss, "i6", (int16_t)reward.unk_06, false);

    oss << '}';

    if (comment.length() > 0)
    {
        oss << " ; " << comment;
    }

    oss << '\n';
    return true;
}

bool Xv2QuestCompiler::DecompileCharaPortrait(const QxdCharPortrait &portrait, std::ostringstream &oss)
{
    std::string comment;

    WriteIndent(oss, true);
    oss << "CharPortrait { ";

    if (!WriteCharParam(oss, "char", portrait.cms_id, false, &comment, (int16_t)portrait.costume_index))
        return false;

    WriteIntegerParam(oss, "costume", (int16_t)portrait.costume_index, false);
    WriteIntegerParam(oss, "trans", (int16_t)portrait.trans, false);

    oss << '}';

    if (comment.length() > 0)
    {
        oss << " ; " << comment;
    }

    oss << '\n';
    return true;
}

bool Xv2QuestCompiler::DecompileQuestStruct(const QxdQuest &quest, std::ostringstream &oss)
{
    if (quest.msg_entries.size() != 4 && quest.msg_entries.size() != 6)
    {
        DPRINTF("%s: Was expecting quest to have either 4 or 6 msg_entries, but found %Id\n", FUNCNAME, quest.msg_entries.size());
        return false;
    }

    WriteIndent(oss, true);
    oss << "Quest " << quest.name << '\n';
    WriteIndent(oss, true);
    oss << "{\n";
    defined_names.insert(ci_string(quest.name.c_str()));
    indent_level++;

    WriteIndent(oss, true);
    WriteIntegerParam(oss, "episode", quest.episode, false);

    const std::string dialogue_file = GetDialogueFile(quest.name, quest.episode);
    const std::string audio_file = GetAudioFile(quest.name, quest.episode, quest.flags, false);

    if (dialogue_file.length() > 0)
    {
        oss << "; Dialogue will be loaded from " << dialogue_file;

        if (audio_file.length() > 0)
        {
            oss << ". Audio will be loaded from " << audio_file;
        }

        oss << '.';
    }
    oss << '\n';

    WriteIntegerParam(oss, "sub_type", quest.sub_type);
    WriteIntegerParam(oss, "num_players", quest.num_players);
    oss << '\n';

    WriteTitleParam(oss, "title", quest.msg_entries[0]);
    WriteTitleParam(oss, "success", quest.msg_entries[1]);
    WriteTitleParam(oss, "failure", quest.msg_entries[2]);
    WriteTitleParam(oss, "outline", quest.msg_entries[3]);

    if (quest.msg_entries.size() > 4)
    {
        WriteTitleParam(oss, "warning", quest.msg_entries[4]);
        WriteTitleParam(oss, "ex_success", quest.msg_entries[5]);
    }

    oss << '\n';

    WriteIntegerParam(oss, "i40", (int16_t)quest.unk_28);
    WriteQuestParam(oss, "parent_quest", quest.parent_quest);
    WriteIntegerParam(oss, "i44", (int16_t)quest.unk_2C);

    // i48-i60
    for (int i = 0; i < 4; i++)
    {
        std::string param_name = "i" + Utils::ToString(4*i+48);
        WriteIntegerParam(oss, param_name, quest.unk_30[i]);
    }

    WriteQuestParam(oss, "unlock_requirement", quest.unlock_requirement);

    // i68-i84
    for (int i = 0; i < 5; i++)
    {
        std::string param_name = "i" + Utils::ToString(4*i+68);
        WriteIntegerParam(oss, param_name, quest.unk_44[i]);
    }

    oss << '\n';

    for (const QxdUnk &unk : quest.unk1s)
    {
        if (!DecompileQxdUnk(unk, 1, oss))
            return false;
    }

    for (const QxdUnk &unk : quest.unk2s)
    {
        if (!DecompileQxdUnk(unk, 2, oss))
            return false;
    }

    if (quest.unk1s.size() > 0 || quest.unk2s.size() > 0)
        oss << '\n';

    WriteIntegerParam(oss, "time_limit", (int16_t)quest.time_limit);
    WriteIntegerParam(oss, "difficulty", (int16_t)quest.difficulty);
    WriteStageParam(oss, "start_stage", quest.start_stage);
    WriteIntegerParam(oss, "start_demo", (int16_t)quest.start_demo);

    oss << '\n';

    WriteIntegerParam(oss, "xp_reward", quest.xp_reward);
    WriteIntegerParam(oss, "ult_xp_reward", quest.ult_xp_reward);
    WriteIntegerParam(oss, "fail_xp_reward", quest.fail_xp_reward);
    WriteIntegerParam(oss, "zeni_reward", quest.zeni_reward);
    WriteIntegerParam(oss, "ult_zeni_reward", quest.ult_zeni_reward);
    WriteIntegerParam(oss, "fail_zeni_reward", quest.fail_zeni_reward);
    WriteIntegerParam(oss, "tp_medals_once", quest.tp_medals_once);
    WriteIntegerParam(oss, "tp_medals", quest.tp_medals);
    WriteIntegerParam(oss, "tp_medals_special", quest.tp_medals_special);
    WriteIntegerParam(oss, "resistance_points", quest.resistance_points);

    oss << '\n';

    for (const QxdItemReward &reward : quest.item_rewards)
    {
        if (!DecompileItemReward(reward, oss))
            return false;
    }

    if (quest.item_rewards.size() > 0)
        oss << '\n';

    for (const QxdSkillReward &reward : quest.skill_rewards)
    {
        if (!DecompileSkillReward(reward, oss))
            return false;
    }

    if (quest.skill_rewards.size() > 0)
        oss << '\n';

    for (const QxdCharReward &reward : quest.char_rewards)
    {
        if (!DecompileCharReward(reward, oss))
            return false;
    }

    if (quest.char_rewards.size() > 0)
        oss << '\n';

    {
        std::ostringstream tss;
        std::string comment;

        WriteIndent(tss, true);
        tss << "stages: (";
        for (int i = 0; i < 16; i++)
        {
            std::string stage_name;

            if (i != 0)
               tss << ", ";

           WriteStage(tss, quest.stages[i], stage_name);

           if (stage_name.length() > 0)
           {
               if (comment.length() > 0)
                   comment += ", ";

               comment += stage_name;
           }
        }
        tss << ")\n";
        if (comment.length() > 0)
        {
            WriteIndent(oss, true);
            oss << "; " << comment << '\n';
        }
        oss << tss.str();
    }

    WriteIntegerParam(oss, "i192", quest.unk_C0);

    oss << '\n';

    for (int i = 0; i < 6; i++)
    {
        if (!DecompileCharaPortrait(quest.enemy_portraits[i], oss))
            return false;
    }

    oss << '\n';

    // i232,i234,,..., i250
    for (int i = 0; i < 10; i++)
    {
        std::string param_name = "i" + Utils::ToString(2*i+232);
        WriteIntegerParam(oss, param_name, (int16_t)quest.unk_E8[i]);
    }

    WriteIntegerParam(oss, "flags", quest.flags, true, "", (quest.flags != 0));
    oss << '\n';

    const std::string update = UpdateToConstant(quest.update_requirement);
    if (update.length() == 0)
    {
        DPRINTF("%s: Unrecognized update_requirement 0x%x\n", FUNCNAME, quest.update_requirement);
        return false;
    }
    WriteIdentifierParam(oss, "update_requirement", update);

    const std::string dlc = DlcToConstant(quest.dlc_requirement);
    if (dlc.length() == 0)
    {
        DPRINTF("%s: Unrecognized dlc_requirement 0x%x\n", FUNCNAME, quest.dlc_requirement);
        return false;
    }
    WriteIdentifierParam(oss, "dlc_requirement", dlc);

    oss << '\n';

    WriteIntegerParam(oss, "i264", quest.unk_108);
    WriteIntegerParam(oss, "no_enemy_bgm", (int16_t)quest.no_enemy_music);
    WriteIntegerParam(oss, "enemy_near_bgm", (int16_t)quest.enemy_near_music);
    WriteIntegerParam(oss, "battle_bgm", (int16_t)quest.battle_music);
    WriteIntegerParam(oss, "ultimate_finish_bgm", (int16_t)quest.ult_finish_music);
    WriteFloatParam(oss, "f276", quest.unk_114);
    WriteIntegerParam(oss, "i280", quest.unk_118);

    indent_level--;
    WriteIndent(oss, true);
    oss << "}\n\n";

    return true;
}

bool Xv2QuestCompiler::DecompileQxdChar(const QxdCharacter &chara, bool special, std::ostringstream &oss)
{
    WriteIndent(oss, true);

    bool error;

    if (special)
    {
        oss << "QxdSpecialChar ";
        auto it = referenced_special_chars.find(chara.id);
        error = (it == referenced_special_chars.end());

        if (!error)
            oss << it->second.str;
    }
    else
    {
        oss << "QxdChar ";
        auto it = referenced_chars.find(chara.id);
        error = (it == referenced_chars.end());

        if (!error)
            oss << it->second.str;
    }

    if (error)
    {
        DPRINTF("%s: BUG, referenced char not found, should not happen.\n", FUNCNAME);
        return false;
    }

    oss << '\n';
    WriteIndent(oss, true);
    oss << "{\n";
    indent_level++;


    {
        std::ostringstream toss;
        std::string comment;

        WriteCharParam(toss, "char", chara.cms_name, true, &comment, (int16_t)chara.costume);
        WriteIndent(oss, true);
        oss << "; " << comment << '\n' << toss.str();
    }


    WriteIntegerParam(oss, "costume", chara.costume);
    WriteIntegerParam(oss, "transformation", (int16_t)chara.transformation);
    WriteIntegerParam(oss, "special_effect", (int16_t)chara.special_effect);

    oss << '\n';

    WriteIntegerParam(oss, "i12", chara.unk_0C);
    WriteIntegerParam(oss, "level", chara.level);
    WriteFloatParam(oss, "health", chara.health);
    WriteFloatParam(oss, "f24", chara.unk_18);
    WriteFloatParam(oss, "ki", chara.ki);
    WriteFloatParam(oss, "stamina", chara.stamina);
    WriteFloatParam(oss, "atk", chara.basic_melee);
    WriteFloatParam(oss, "ki_atk", chara.ki_blast);
    WriteFloatParam(oss, "super_atk", chara.strike_super);
    WriteFloatParam(oss, "super_ki", chara.ki_super);
    WriteFloatParam(oss, "atk_damage", chara.basic_melee_damage);
    WriteFloatParam(oss, "ki_damage", chara.ki_blast_damage);
    WriteFloatParam(oss, "super_atk_damage", chara.strike_super_damage);
    WriteFloatParam(oss, "super_ki_damage", chara.ki_super_damage);
    WriteFloatParam(oss, "guard_atk", chara.unk_44);
    WriteFloatParam(oss, "guard_damage", chara.unk_48);
    WriteFloatParam(oss, "move_speed", chara.air_speed);
    WriteFloatParam(oss, "boost_speed", chara.boost_speed);
    WriteIntegerParam(oss, "ait_table_entry", chara.ait_table_entry);

    oss << '\n';

    WriteSkillParam(oss, "super1", chara.skills[0], QXD_SKILL_SUPER, false);
    WriteSkillParam(oss, "super2", chara.skills[1], 0, false);
    WriteSkillParam(oss, "super3", chara.skills[2], 0, false);
    WriteSkillParam(oss, "super4", chara.skills[3], 0, false);
    oss << '\n';

    WriteSkillParam(oss, "ultimate1", chara.skills[4], QXD_SKILL_ULTIMATE, false);
    WriteSkillParam(oss, "ultimate2", chara.skills[5], QXD_SKILL_ULTIMATE, false);
    oss << '\n';

    WriteSkillParam(oss, "evasive", chara.skills[6], QXD_SKILL_EVASIVE, false);
    WriteSkillParam(oss, "blast", chara.skills[7], QXD_SKILL_BLAST, true);
    WriteSkillParam(oss, "awaken", chara.skills[8], QXD_SKILL_AWAKEN, false);
    oss << '\n';

    WriteIntegerParam(oss, "i106", (int16_t)chara.unk_6A[0]);
    WriteIntegerParam(oss, "i108", (int16_t)chara.unk_6A[1]);
    WriteIntegerParam(oss, "i112", (int16_t)chara.unk_6A[3]);

    indent_level--;
    WriteIndent(oss, true);
    oss << "}\n\n";

    return true;
}

bool Xv2QuestCompiler::DecompileQmlChar(const QmlEntry &qchar, std::ostringstream &oss)
{
    ci_string base_name;
    ci_string var_name;

    WriteIndent(oss, true);
    oss << "QmlChar ";

    QxdCharacter *chara = active_qxd.FindCharById(qchar.qxd_id);
    if (!chara)
    {
        chara = active_qxd.FindSpecialCharById(qchar.qxd_id);

        if (!chara)
        {
            DPRINTF("%s: BUG, chara shouldn'tbe null here.\n", FUNCNAME);
            return false;
        }

        auto it = referenced_special_chars.find(chara->id);
        base_name = it->second;
    }
    else
    {
        auto it = referenced_chars.find(chara->id);
        base_name = it->second;
    }

    if (base_name == "PlayerBase")
    {
        var_name.str = "Player";
    }
    else
    {
        if (qchar.team == QML_TEAM_A)
        {
            var_name.str = base_name.str + "Ally";
        }
        else if (qchar.team == QML_TEAM_B)
        {
            var_name.str = base_name.str + "Enemy";
        }
        else
        {
            var_name.str = base_name.str + "Neutral";
        }
    }

    var_name = GetFriendlyName(var_name.str, "Char");
    defined_names.insert(var_name);
    referenced_qchars[qchar.id] = var_name;

    oss << var_name.str << " : " << base_name.str << '\n';

    WriteIndent(oss, true);
    oss << "{\n";
    indent_level++;

    WriteIntegerParam(oss, "battle_index", qchar.battle_index);
    WriteIntegerParam(oss, "i12", qchar.unk_0C);
    WriteStageParam(oss, "stage", qchar.stage_id);
    WriteBooleanParam(oss, "spawn_at_start", (qchar.spawn_at_start != 0));
    oss << '\n';

    std::string ai = AiToConstant(qchar.ai);
    if (ai.length() == 0)
    {
        DPRINTF("%s: Unrecognized ai constant %d\n", FUNCNAME, qchar.ai);
        return false;
    }
    else
    {
        WriteIdentifierParam(oss, "ai", ai);
    }

    std::string team = TeamToConstant(qchar.team);
    if (team.length() == 0)
    {
        DPRINTF("%s: Unrecognized team constant %d\n", FUNCNAME, qchar.team);
        return false;
    }
    WriteIdentifierParam(oss, "team", team);
    oss << '\n';

    WriteIntegerParam(oss, "i36", qchar.unk_24);
    WriteIntegerParam(oss, "i40", qchar.unk_28);
    WriteIntegerParam(oss, "i44", qchar.unk_2C);
    WriteIntegerParam(oss, "i48", (int16_t)qchar.unk_30[0]);
    WriteIntegerParam(oss, "i50", (int16_t)qchar.unk_30[1]);
    WriteIntegerParam(oss, "i52", (int16_t)qchar.unk_30[2]);
    WriteIntegerParam(oss, "i56", (int16_t)qchar.unk_30[4]);
    oss << '\n';

    WriteSkillParam(oss, "super1", qchar.skills[0], QXD_SKILL_SUPER, false);
    WriteSkillParam(oss, "super2", qchar.skills[1], 0, false);
    WriteSkillParam(oss, "super3", qchar.skills[2], 0, false);
    WriteSkillParam(oss, "super4", qchar.skills[3], 0, false);
    oss << '\n';

    WriteSkillParam(oss, "ultimate1", qchar.skills[4], QXD_SKILL_ULTIMATE, false);
    WriteSkillParam(oss, "ultimate2", qchar.skills[5], QXD_SKILL_ULTIMATE, false);
    oss << '\n';

    WriteSkillParam(oss, "evasive", qchar.skills[6], QXD_SKILL_EVASIVE, false);
    WriteSkillParam(oss, "blast", qchar.skills[7], QXD_SKILL_BLAST, true);
    WriteSkillParam(oss, "awaken", qchar.skills[8], QXD_SKILL_AWAKEN, false);

    indent_level--;
    WriteIndent(oss, true);
    oss << "}\n\n";

    return true;
}

bool Xv2QuestCompiler::DecompileDialoguePart(const QbtDialoguePart &part, uint16_t index, bool special_case, std::ostringstream &oss, int parent_index, int interactive_type)
{
    WriteIndent(oss, true);
    oss << "DialoguePart\n";
    WriteIndent(oss, true);
    oss << "{\n";
    indent_level++;

    WriteCharParam(oss, "actor", part.portrait_cms, true, nullptr, (int16_t)part.portrait_costume_index);
    WriteIntegerParam(oss, "costume", (int16_t)part.portrait_costume_index);
    WriteIntegerParam(oss, "transformation", (int16_t)part.portrait_transformation);
    oss << '\n';

    WriteDialogueTAParam(oss, "text_audio", part.msg_id);
    WriteIntegerParam(oss, "i8", (int16_t)part.unk_08);

    bool write_debug_actor = true;

    if ((interactive_type == QBT_ITYPE_JOIN || interactive_type == QBT_ITYPE_EFFECT) && index > 0)
    {
        if (part.cms_04 == (uint32_t)parent_index)
        {
            WriteBooleanParam(oss, "dont_change", true);
        }
        else if (part.cms_04 == (uint32_t)parent_index+1)
        {
            WriteBooleanParam(oss, "continue_next", true);
        }

        //write_debug_actor = false;
    }

    //oss << "\t\t; " << Utils::UnsignedToHexString(part.cms_04, false) << "\n";

    if (part.cms_04 != 0 && write_debug_actor)
        WriteCharParam(oss, "debug_actor", part.cms_04);

    if (special_case)
        WriteBooleanParam(oss, "special_case_osq_0301", true);

    if (index != part.sub_id)
    {
        WriteIntegerParam(oss, "out_of_order", true, "This parameter can be safely ignored");
    }

    indent_level--;
    WriteIndent(oss, true);
    oss << "}\n\n";

    return true;
}

bool Xv2QuestCompiler::DecompileDialogue(const QbtEntry &dialogue, uint32_t index, std::ostringstream &oss)
{
    uint32_t qbt_id = (dialogue.parts.size() > 0) ? dialogue.parts[0].qbt_id : 0xFFFFFFFF;
    ci_string var_name;

    if (qbt_id == 0xFFFFFFFF)
    {
        var_name = GetFriendlyName("", "_Dialogue");
    }
    else
    {
        std::string use_name = "Dialogue" + Utils::ToString(qbt_id);
        if (defined_names.find(ci_string(use_name)) != defined_names.end())
        {
            use_name = "_Dialogue" + Utils::ToString(qbt_id);
        }

        var_name = GetFriendlyName(use_name, "_Dialogue");
    }

    defined_names.insert(var_name);
    referenced_dialogues[index] = var_name;

    WriteIndent(oss, true);
    oss << "Dialogue " << var_name.str << '\n';
    WriteIndent(oss, true);
    oss << "{\n";
    indent_level++;

    if (dialogue.unk_02 != 0 && dialogue.unk_02 == qbt_id)
    {
        WriteBooleanParam(oss, "b_use_i2", true);
    }

    WriteIntegerParam(oss, "i28", dialogue.unk_1C);

    if (dialogue.cms_20 != 0)
    {
        WriteCharParam(oss, "debug_actor", dialogue.cms_20);
    }

    oss << '\n';

    for (size_t i = 0; i < dialogue.parts.size(); i++)
    {
        const QbtDialoguePart &part = dialogue.parts[i];

        if (!DecompileDialoguePart(part, (uint16_t)i, (part.qbt_id != qbt_id), oss, index, -1))
            return false;
    }

    indent_level--;
    WriteIndent(oss, true);
    oss << "}\n\n";

    return true;
}

bool Xv2QuestCompiler::DecompileInteractiveDialogue(const QbtEntry &dialogue, uint32_t index, std::ostringstream &oss)
{
    uint32_t qbt_id = (dialogue.parts.size() > 0) ? dialogue.parts[0].qbt_id : 0xFFFFFFFF;
    ci_string var_name;

    if (qbt_id == 0xFFFFFFFF)
    {
        var_name = GetFriendlyName("", "_InteractiveDialogue");
    }
    else
    {
        std::string use_name = "InteractiveDialogue" + Utils::ToString(qbt_id);
        if (defined_names.find(ci_string(use_name)) != defined_names.end())
        {
            use_name = "_InteractiveDialogue" + Utils::ToString(qbt_id);
        }

        var_name = GetFriendlyName(use_name, "_InteractiveDialogue");
    }

    defined_names.insert(var_name);
    referenced_interactive_dialogues[index] = var_name;

    WriteIndent(oss, true);
    oss << "InteractiveDialogue " << var_name.str << '\n';
    WriteIndent(oss, true);
    oss << "{\n";
    indent_level++;

    auto it = interaction_type_to_constant.find(dialogue.interaction_type);
    if (it == interaction_type_to_constant.end())
    {
        WriteIntegerParam(oss, "interaction_type", dialogue.interaction_type);
    }
    else
    {
        WriteIdentifierParam(oss, "interaction_type", it->second);
    }

    if (dialogue.interaction_type == QBT_ITYPE_GIVE_ITEM)
    {
        int item_type = dialogue.interaction_param / 1000;
        int item_id = dialogue.interaction_param % 1000;
        std::string comment;

        WriteIndent(oss, true);
        WriteItemParam(oss, "param", "item_type", item_id, item_type, false, true, &comment);
        oss << " ; " << comment << '\n';
    }
    else
    {
        WriteIntegerParam(oss, "param", dialogue.interaction_param);
    }

    oss << '\n';

    WriteIntegerParam(oss, "i0", (int16_t)dialogue.unk_00);
    if (dialogue.unk_02 != 0 && dialogue.unk_02 == qbt_id)
    {
        WriteBooleanParam(oss, "b_use_i2", true);
    }

    if (dialogue.cms_20 != 0)
    {
        WriteCharParam(oss, "debug_actor", dialogue.cms_20);
    }

    oss << '\n';

    for (size_t i = 0; i < dialogue.parts.size(); i++)
    {
        const QbtDialoguePart &part = dialogue.parts[i];

        if (!DecompileDialoguePart(part, (uint16_t)i, (part.qbt_id != qbt_id), oss, index, dialogue.interaction_type))
            return false;
    }

    indent_level--;
    WriteIndent(oss, true);
    oss << "}\n\n";

    return true;
}

bool Xv2QuestCompiler::DecompileEventDialogue(const QbtEntry &dialogue, uint32_t index, std::ostringstream &oss)
{
    uint32_t qbt_id = (dialogue.parts.size() > 0) ? dialogue.parts[0].qbt_id : 0xFFFFFFFF;
    ci_string var_name;

    if (qbt_id == 0xFFFFFFFF)
    {
        var_name = GetFriendlyName("", "_EventDialogue");
    }
    else
    {
        std::string use_name = "EventDialogue" + Utils::ToString(qbt_id);
        if (defined_names.find(ci_string(use_name)) != defined_names.end())
        {
            use_name = "_EventDialogue" + Utils::ToString(qbt_id);
        }

        var_name = GetFriendlyName(use_name, "_EventDialogue");
    }

    defined_names.insert(var_name);
    referenced_event_dialogues[index] = var_name;

    WriteIndent(oss, true);
    oss << "EventDialogue " << var_name.str << '\n';
    WriteIndent(oss, true);
    oss << "{\n";
    indent_level++;

    std::string constant = DialogueEventToConstant(dialogue.special_event);
    if (constant.length() > 0)
    {
        WriteIdentifierParam(oss, "devent", constant);
    }
    else
    {
         WriteIntegerParam(oss, "devent", dialogue.special_event);
    }

    WriteBooleanParam(oss, "on_event_end", (dialogue.special_on_event_end != 0));
    if (dialogue.cms_20 != 0)
        WriteStageParam(oss, "stage", dialogue.cms_20);

    oss << '\n';

    WriteIntegerParam(oss, "i12", dialogue.interaction_type);
    WriteIntegerParam(oss, "i16", dialogue.interaction_param);
    WriteIntegerParam(oss, "i34", (int16_t)dialogue.unk_22);
    WriteIntegerParam(oss, "i36", dialogue.unk_24);
    WriteIntegerParam(oss, "i44", dialogue.unk_2C);
    WriteIntegerParam(oss, "i48", dialogue.unk_30);
    oss << '\n';


    for (size_t i = 0; i < dialogue.parts.size(); i++)
    {
        const QbtDialoguePart &part = dialogue.parts[i];

        if (!DecompileDialoguePart(part, (uint16_t)i, (part.qbt_id != qbt_id), oss, index, -1))
            return false;
    }

    indent_level--;
    WriteIndent(oss, true);
    oss << "}\n\n";

    return true;
}

bool Xv2QuestCompiler::DecompileCharPosition(const QslEntry &position, uint32_t stage, std::ostringstream &oss)
{
    std::ostringstream voss;
    std::string comment;
    ci_string var_name;
    std::string desired_var_name;

    if (!WriteQChar(voss, (int16_t)position.qml_item_id, comment, false, true))
        return false;

    desired_var_name = voss.str() + "Position";
    var_name = GetFriendlyName(desired_var_name, "CharPosition");

    defined_names.insert(var_name);
    referenced_chars_positions[position.position] = var_name;

    WriteIndent(oss, true);
    oss << "CharPosition " << var_name.str << '\n';
    WriteIndent(oss, true);
    oss << "{\n";
    indent_level++;

    WriteQCharParam(oss, "char", (int16_t)position.qml_item_id, false, true);
    WriteStageParam(oss, "stage", (int16_t)stage);
    WriteStringParam(oss, "position", position.position);
    WriteIntegerParam(oss, "type", (int16_t)position.type);
    WriteIntegerParam(oss, "i40", position.qml_change);
    WriteIntegerParam(oss, "i50", (int16_t)position.unk_32);

    indent_level--;
    WriteIndent(oss, true);
    oss << "}\n\n";

    return true;
}

bool Xv2QuestCompiler::DecompileInteractiveCharPosition(const QslEntry &position, uint32_t stage, std::ostringstream &oss)
{
    std::ostringstream voss;
    std::string comment;
    ci_string var_name;
    std::string desired_var_name;

    if (!WriteQChar(voss, (int16_t)position.qml_item_id, comment, false, true))
        return false;

    desired_var_name = voss.str() + "Position";
    var_name = GetFriendlyName(desired_var_name, "CharPosition");

    defined_names.insert(var_name);
    referenced_ichars_positions[position.position] = var_name;

    WriteIndent(oss, true);
    oss << "InteractiveCharPosition " << var_name.str << '\n';
    WriteIndent(oss, true);
    oss << "{\n";
    indent_level++;

    WriteQCharParam(oss, "char", (int16_t)position.qml_item_id, false, true);
    WriteStageParam(oss, "stage", (int16_t)stage);
    WriteStringParam(oss, "position", position.position);
    oss << '\n';

    if (Utils::BeginsWith(var_name.str, "RandomAvatar", false))
    {
        WriteIntegerParam(oss, "idialogue", (int16_t)position.chance_idialogue);
    }
    else
    {
        if (!WriteInteractiveDialogueParam(oss, "idialogue", (int16_t)position.chance_idialogue, true, true))
            return false;
    }

    if (position.qml_change != position.qml_item_id)
    {
        if (!WriteQCharParam(oss, "change_to", position.qml_change, false, false))
            return false;
    }

    WriteIntegerParam(oss, "i38", (int16_t)position.unk_26);
    oss << '\n';

    WriteIntegerParam(oss, "default_pose", (int16_t)position.default_pose);
    WriteIntegerParam(oss, "talk_pose", (int16_t)position.talking_pose);
    WriteIntegerParam(oss, "effect_pose", (int16_t)position.effect_pose);
    WriteIntegerParam(oss, "i50", (int16_t)position.unk_32);

    indent_level--;
    WriteIndent(oss, true);
    oss << "}\n\n";

    return true;
}

bool Xv2QuestCompiler::DecompileItemPosition(const QslEntry &position, uint32_t stage, std::ostringstream &oss)
{
    ci_string cvar_name;
    auto it = referenced_collections.find(position.qml_item_id);

    if (it == referenced_collections.end())
    {
        cvar_name = GetFriendlyName(GetCollectionName(position.qml_item_id, true), "Collection");
        referenced_collections[position.qml_item_id] = cvar_name;
        defined_names.insert(cvar_name);
    }
    else
    {
        cvar_name = it->second;
    }

    std::string desired_var_name = cvar_name.str + "Position";
    ci_string var_name = GetFriendlyName(desired_var_name, "ItemPosition");
    defined_names.insert(var_name);
    referenced_items_positions[position.position] = var_name;

    WriteIndent(oss, true);
    oss << "ItemPosition " << var_name.str << '\n';
    WriteIndent(oss, true);
    oss << "{\n";
    indent_level++;

    WriteIndent(oss, true);
    if (!WriteItemParam(oss, "item_collection", "", position.qml_item_id, QXD_ITEM_COLLECTION, true, true, nullptr, false))
        return false;

    WriteStageParam(oss, "stage", (int16_t)stage);
    WriteStringParam(oss, "position", position.position);
    WriteIntegerParam(oss, "chance", (int16_t)position.chance_idialogue);

    indent_level--;
    WriteIndent(oss, true);
    oss << "}\n\n";

    return true;
}

bool Xv2QuestCompiler::DecompileInstruction(const InstructionDef &def, const QedInstructionParams &params, std::ostringstream &oss)
{
    std::string stage_comment;
    std::string char_comment;
    std::string dialogue_comment;
    std::string quest_comment;
    std::string skill_comment;
    std::string battle_msg_comment;

    WriteIndent(oss, true);

    if (def.is_action)
        oss << "Action ";
    else
        oss << "Condition ";

    oss << def.name.str;

    if (def.param_types.size() > 0)
        oss << '(';

    for (size_t i = 0, p = 0; p < def.param_types.size(); i++, p++)
    {
        const int32_t *param_int = nullptr, *next_int = nullptr;
        const float *param_float = nullptr;
        const char *param_string = nullptr;

        if (p != 0)
        {
            oss << ", ";
        }

        if (i == 0)
        {
            param_int = &params.group1.group.nums.param1.num.i;
            next_int = &params.group1.group.nums.param2.num.i;
            param_float = &params.group1.group.nums.param1.num.f;
            param_string = params.group1.group.string.str;
        }
        else if (i == 1)
        {
            param_int = &params.group1.group.nums.param2.num.i;
            next_int = &params.group1.group.nums.param3.num.i;
            param_float = &params.group1.group.nums.param2.num.f;
        }
        else if (i == 2)
        {
            param_int = &params.group1.group.nums.param3.num.i;
            next_int = &params.group1.group.nums.param4.num.i;
            param_float = &params.group1.group.nums.param3.num.f;
        }
        else if (i == 3)
        {
            param_int = &params.group1.group.nums.param4.num.i;
            next_int = &params.group1.group.nums.param5.num.i;
            param_float = &params.group1.group.nums.param4.num.f;
        }
        else if (i == 4)
        {
            param_int = &params.group1.group.nums.param5.num.i;
            next_int = &params.group2.param2.num.i;
            param_float = &params.group1.group.nums.param5.num.f;
        }
        else if (i == 5)
        {
            param_int = &params.group2.param2.num.i;
            next_int = &params.group2.param3.num.i;
            param_float = &params.group2.param2.num.f;
        }
        else if (i == 6)
        {
            param_int = &params.group2.param3.num.i;
            next_int = &params.group2.param4.num.i;
            param_float = &params.group2.param3.num.f;
        }
        else if (i == 7)
        {
            param_int = &params.group2.param4.num.i;
            param_float = &params.group2.param4.num.f;
        }

        if (def.param_types[p] == PARAM_INTEGER)
        {
            oss << *param_int;
        }
        else if (def.param_types[p] == PARAM_INTEGER_HEX)
        {
            oss << Utils::UnsignedToString(*param_int, true);
        }
        else if (def.param_types[p] == PARAM_FLOAT)
        {
            oss << Utils::FloatToString(*param_float);
        }
        else if (def.param_types[p] == PARAM_BOOLEAN)
        {
            oss << ((*param_int == 0) ? "false" : "true");
        }
        else if (def.param_types[p] == PARAM_REVERSE_BOOLEAN)
        {
            oss << ((*param_int != 0) ? "false" : "true");
        }
        else if (def.param_types[p] == PARAM_STRING || def.param_types[p] == PARAM_STRING_LONG)
        {
            std::string str = param_string;

            Utils::Replace(str, "\"", "\\\"");
            Utils::Replace(str, "\n", "\\n");
            Utils::Replace(str, "\t", "\\\t");

            oss << "\"" << str << "\"";

            if (def.param_types[p] == PARAM_STRING)
                i += 4;
            else
                i += 5;
        }
        else if (def.param_types[p] == PARAM_OPERATOR)
        {
            oss << ((*param_int == 0) ? ">=" : "<=");
        }
        else if (def.param_types[p] == PARAM_SCMS)
        {
            WriteChar(oss, param_string, char_comment);
        }
        else if (def.param_types[p] == PARAM_SKILL)
        {
            if (!WriteSkill(oss, (int16_t)*next_int, *param_int, true, skill_comment))
                return false;

            i++;
        }
        else if (def.param_types[p] == PARAM_STAGE)
        {
            std::string previous_comment = stage_comment;
            WriteStage(oss, (int16_t)*param_int, stage_comment);

            if (previous_comment.length() > 0 && stage_comment.length() > 0)
            {
                stage_comment = previous_comment + " -> " + stage_comment;
            }
        }
        else if (def.param_types[p] == PARAM_QUEST)
        {
            WriteQuest(oss, (int16_t)*param_int, quest_comment);
        }
        else if (def.param_types[p] == PARAM_QUEST_WITH_TYPE)
        {
            if (*param_int != quest_type)
            {
                DPRINTF("%s: Was expecting quest type %d but got %d.\n", FUNCNAME, quest_type, *param_int);
                return false;
            }

            WriteQuest(oss, (int16_t)*next_int, quest_comment);
            i++;
        }
        else if (def.param_types[p] == PARAM_QCHAR)
        {
            bool char_comment_existed = (char_comment.length() > 0);

            if (!WriteQChar(oss, *param_int, char_comment, true, true))
            {
                DPRINTF("Failed at %s:%s.\n", (def.is_action) ? "Action" : "Condition", def.name.c_str());
                return false;
            }

            if (char_comment_existed)
                char_comment.clear();
        }
        else if (def.param_types[p] == PARAM_QBT)
        {
            if (!WriteDialogue(oss, *param_int, dialogue_comment, true, false))
                return false;
        }
        else if (def.param_types[p] == PARAM_PARTNER)
        {
            auto it = partner_to_constant.find(*param_int);
            if (it == partner_to_constant.end())
            {
                oss << *param_int;
            }
            else
            {
                oss << it->second;
            }
        }
        else if (def.param_types[p] == PARAM_FLAG)
        {
            if (!WriteFlag(oss, *param_int))
                return false;
        }
        else if (def.param_types[p] == PARAM_STRINGVAR)
        {
            if (!WriteStringVar(oss, *param_int))
                return false;
        }
        else if (def.param_types[p] == PARAM_RACE)
        {
            auto it = race_to_constant.find(*param_int);
            if (it == race_to_constant.end())
            {
                DPRINTF("%s: Unrecognized race %d.\n", FUNCNAME, *param_int);
                return false;
            }

            oss << it->second;
        }
        else if (def.param_types[p] == PARAM_SEX)
        {
            auto it = sex_to_constant.find(*param_int);
            if (it == sex_to_constant.end())
            {
                DPRINTF("%s: Unrecognized sex %d.\n", FUNCNAME, *param_int);
                return false;
            }

            oss << it->second;
        }
        else if (def.param_types[p] == PARAM_FADE)
        {
            auto it = fade_to_constant.find(*param_int);
            if (it == fade_to_constant.end())
            {
                DPRINTF("%s: Unrecognized fade %d.\n", FUNCNAME, *param_int);
                return false;
            }

            oss << it->second;
        }
        else if (def.param_types[p] == PARAM_TIME_CTRL_MODE)
        {
            auto it = time_ctrl_to_constant.find(*param_int);
            if (it == time_ctrl_to_constant.end())
            {
                DPRINTF("%s: Unrecognized time control %d.\n", FUNCNAME, *param_int);
                return false;
            }

            oss << it->second;
        }
        else if (def.param_types[p] == PARAM_TEAM_TYPE)
        {
            auto it = team_type_to_constant.find(*param_int);
            if (it == team_type_to_constant.end())
            {
                DPRINTF("%s: Unrecognized team type %d.\n", FUNCNAME, *param_int);
                return false;
            }

            oss << it->second;
        }
        else if (def.param_types[p] == PARAM_TEAM_TYPE2)
        {
            auto it = team_type2_to_constant.find(*param_int);
            if (it == team_type2_to_constant.end())
            {
                DPRINTF("%s: Unrecognized team type (2) %d.\n", FUNCNAME, *param_int);
                return false;
            }

            oss << it->second;
        }
        else if (def.param_types[p] == PARAM_STATS)
        {
            auto it = stats_to_constant.find(*param_int);
            if (it == stats_to_constant.end())
            {
                DPRINTF("%s: Unrecognized stats %d.\n", FUNCNAME, *param_int);
                return false;
            }

            oss << it->second;
        }
        else if (def.param_types[p] == PARAM_SKILL_SLOT)
        {
            auto it = skill_slot_to_constant.find(*param_int);
            if (it == skill_slot_to_constant.end())
            {
                DPRINTF("%s: Unrecognized skill slot %d.\n", FUNCNAME, *param_int);
                return false;
            }

            oss << it->second;
        }
        else if (def.param_types[p] == PARAM_QUEST_FINISH_STATE)
        {
            auto it = quest_fs_to_constant.find(*param_int);
            if (it == quest_fs_to_constant.end())
            {
                DPRINTF("%s: Unrecognized quest finish state %d.\n", FUNCNAME, *param_int);
                return false;
            }

            oss << it->second;
        }
        else if (def.param_types[p] == PARAM_BATTLE_MSG)
        {
            WriteDialogueTA(oss, param_string, battle_msg_comment);
            i += 4;
        }
        else if (def.param_types[p] == PARAM_SKILL_COMBINATION)
        {
            std::string comb;

            if ((*param_int & 1))
            {
                comb = "ALL";
            }
            else
            {
                for (int i = 0; i < NUM_SKILL_SLOTS; i++)
                {
                    if ((*param_int & (1 << (i+1))))
                    {
                        if (comb.length() > 0)
                            comb += '|';

                        auto it = skill_slot_to_constant.find(i);
                        comb += it->second;
                    }
                }
            }

            if (comb.length() == 0)
            {
                DPRINTF("%s: Unrecognized skill combination 0x%x.\n", FUNCNAME, *param_int);
                return false;
            }

            oss << comb;
        }
        else if (def.param_types[p] == PARAM_STATS_COMBINATION)
        {
            std::string comb;

            for (int i = 0; i < 3; i++)
            {
                if (*param_int & (1 << i))
                {
                    if (comb.length() > 0)
                        comb += '|';

                    auto it = stats_to_constant2.find(i);
                    comb += it->second;
                }
            }

            if (comb.length() == 0)
            {
                DPRINTF("%s: Unrecognized stats combination 0x%x.\n", FUNCNAME, *param_int);
                return false;
            }

            oss << comb;
        }
        else
        {
            DPRINTF("%s: BUG, should not be here.\n", FUNCNAME);
            return false;
        }
    }

    if (def.param_types.size() > 0)
        oss << ')';

    if (char_comment.length() > 0 && stage_comment.length() > 0 && dialogue_comment.length() > 0)
    {
        oss << "; Char: " << char_comment << " --- Stage: " << stage_comment << " --- Dialogue: " << dialogue_comment;
    }
    else if (char_comment.length() > 0 && skill_comment.length() > 0)
    {
        oss << "; Char: " << char_comment << " --- Skill: " << skill_comment;
    }
    else if (char_comment.length() > 0 && dialogue_comment.length() > 0)
    {
        oss << "; Char: " << char_comment << " --- Dialogue: " << dialogue_comment;
    }
    else if (char_comment.length() > 0 && stage_comment.length() > 0)
    {
        oss << "; Char: " << char_comment << " --- Stage: " << stage_comment;
    }
    else if (stage_comment.length() > 0 && dialogue_comment.length() > 0)
    {
        oss << "; Dialogue: " << dialogue_comment << " --- Stage: " << stage_comment;
    }
    else if (char_comment.length() > 0)
    {
        oss << "; " << char_comment;
    }
    else if (stage_comment.length() > 0)
    {
        oss << "; " << stage_comment;
    }
    else if (dialogue_comment.length() > 0)
    {
        oss << "; " << dialogue_comment;
    }
    else if (quest_comment.length() > 0)
    {
        oss << "; " << quest_comment;
    }
    else if (skill_comment.length() > 0)
    {
        oss << "; " << skill_comment;
    }
    else if (battle_msg_comment.length() > 0)
    {
        oss << "; " << battle_msg_comment;
    }

    oss << '\n';
    return true;
}

bool Xv2QuestCompiler::DecompileEvent(const QedEvent &event, int16_t event_num, std::ostringstream &oss, bool last)
{
    WriteIndent(oss, true);
    oss << "Event " << event_num << '\n';
    WriteIndent(oss, true);
    oss << "{\n";
    indent_level++;

    for (const QedInstruction &cond : event.conditions)
    {
        bool found = false;

        for (const InstructionDef &def : instructions_defs)
        {
            if (def.opcode == cond.opcode && !def.is_action)
            {
                found = true;

                if (!DecompileInstruction(def, cond.params, oss))
                    return false;

                break;
            }
        }

        if (!found)
        {
            DPRINTF("%s: Unrecognized condition opcode %d.\n", FUNCNAME, cond.opcode);
            return false;
        }
    }

    if (event.conditions.size() > 0 && event.actions.size() > 0)
        oss << '\n';

    for (const QedInstruction &act : event.actions)
    {
        bool found = false;

        for (const InstructionDef &def : instructions_defs)
        {
            if (def.opcode == act.opcode && def.is_action)
            {
                found = true;

                if (!DecompileInstruction(def, act.params, oss))
                    return false;

                break;
            }
        }

        if (!found)
        {
            DPRINTF("%s: Unrecognized action opcode %d.\n", FUNCNAME, act.opcode);
            return false;
        }
    }

    indent_level--;
    WriteIndent(oss, true);
    oss << "}\n";

    if (!last)
        oss << '\n';

    return true;
}

bool Xv2QuestCompiler::DecompileState(const QedState &state, size_t index, std::ostringstream &oss, bool last)
{
    WriteIndent(oss, true);
    oss << "State " << index << '\n';
    WriteIndent(oss, true);
    oss << "{\n";
    indent_level++;

    size_t count = 0;

    for (auto &it : state.events)
    {
        if (!DecompileEvent(it.second, it.first, oss, count == (state.events.size()-1)))
            return false;

        count++;
    }

    indent_level--;
    WriteIndent(oss, true);
    oss << "}\n";

    if (!last)
        oss << '\n';

    return true;
}

bool Xv2QuestCompiler::DecompileScript(const QedFile &script, std::ostringstream &oss)
{
    WriteIndent(oss, true);
    oss << "Script" << '\n';
    WriteIndent(oss, true);
    oss << "{\n";
    indent_level++;

    for (size_t i = 0; i < script.GetNumStates(); i++)
    {
        if (script[i].IsEmpty())
            continue;

        if (!DecompileState(script[i], i, oss, (i==script.GetNumStates()-1)))
            return false;
    }

    indent_level--;
    WriteIndent(oss, true);
    oss << "}\n\n";

    return true;
}

bool Xv2QuestCompiler::CompileX2mMod()
{
    XQ_X2mMod mod;
    ci_string var_name;
    std::vector<bool> defined;

    if (!ProcessVarDeclaration(var_name))
        return false;

    if (!ExpectBracketBegin())
        return false;

    defined.resize(x2m_mod_params.size(), false);

    while (!tokens.empty())
    {
        X2QcToken token = tokens.front();
        tokens.pop();

        if (token.type == TOKEN_BRACKET_END)
        {
            for (size_t i = 0; i < defined.size(); i++)
            {
                if (!defined[i])
                {
                    DPRINTF("Parameter \"%s\" was not defined in X2mMod object ", x2m_mod_params[i].c_str());
                    return LineError(token);
                }
            }

            linked_mods.insert(mod);
            compiled_mods[var_name] = mod;
            return true;
        }
        else
        {
            X2QcToken value;

            if (!GetParam(token, value, x2m_mod_params, x2m_mod_types, defined))
                return false;

            std::string &value_str = value.str.str;

            if (token.str == "name")
            {
                mod.name = value_str;
            }
            else if (token.str == "guid")
            {
                uint8_t b_guid[16];
                if (!Utils::String2GUID(b_guid, value_str))
                {
                    DPRINTF("\"%s\" is not a valid guid. ", value_str.c_str());
                    return LineError(value);
                }

                mod.guid = value_str;

                if (linked_mods.find(mod) != linked_mods.end())
                {
                    DPRINTF("A X2mMod with similar guid had already been defined. ");
                    return LineError(value);
                }
            }
            else
            {
                DPRINTF("%s: BUG, should not be here: %s\n", FUNCNAME, token.str.c_str());
                return false;
            }
        }
    }

    DPRINTF("Premature end of file (in X2mMod object)\n");
    return false;
}

bool Xv2QuestCompiler::CompileTextEntry(bool audio)
{
    std::vector<std::string> entry;
    ci_string var_name;

    std::vector<bool> defined;
    std::vector<ci_string> params;
    std::vector<int> types;

    if (!ProcessVarDeclaration(var_name))
        return false;

    if (!ExpectBracketBegin())
        return false;

    entry.resize(XV2_LANG_NUM);
    defined.resize(XV2_LANG_NUM, false);
    params.resize(XV2_LANG_NUM);
    types.resize(XV2_LANG_NUM, TOKEN_STRING);

    for (int i = 0; i < XV2_LANG_NUM; i++)
        params[i].str = xv2_lang_codes[i];

    if (audio)
    {
        entry.push_back("");
        defined.push_back(false);
        params.push_back("voice");
        types.push_back(TOKEN_STRING);
    }

    while (!tokens.empty())
    {
        X2QcToken token = tokens.front();
        tokens.pop();

        if (token.type == TOKEN_BRACKET_END)
        {
            for (int lang = 0; lang < (int)defined.size(); lang++)
            {
                if (!defined[lang])
                {
                    if (lang == XV2_LANG_ENGLISH)
                    {
                        DPRINTF("\"en\" entry is mandatory in TextEntry/TextAudioEntry, but wasn't found. ");
                        return LineError(token);
                    }
                    else if (lang == XV2_LANG_NUM)
                    {
                        DPRINTF("Voice wasn't defined in TextAudioEntry. ");
                        return LineError(token);
                    }
                    else
                    {
                        entry[lang] = entry[XV2_LANG_ENGLISH];
                    }
                }
            }

            compiled_text_entry[var_name] = entry;
            return true;
        }
        else
        {
            X2QcToken value;

            if (!GetParam(token, value, params, types, defined))
                return false;

            int lang;

            for (lang = 0; lang < (int)entry.size(); lang++)
            {
                if (token.str == params[lang])
                    break;
            }

            if (lang >= (int)entry.size())
            {
                DPRINTF("%s: BUG: Should not be here.\n", FUNCNAME);
                return LineError(token);
            }

            entry[lang] = Xenoverse2::EscapeHtml(value.str.str);
        }
    }

    DPRINTF("Premature end of file (in TextEntry object)\n");
    return false;
}

bool Xv2QuestCompiler::CompileItemCollectionEntry(QxdCollectionEntry &entry)
{
    std::vector<bool> defined;
    X2QcToken mod_var;

    entry = QxdCollectionEntry();

    if (!ExpectBracketBegin())
        return false;

    defined.resize(it_col_entry_params.size(), false);

    while (!tokens.empty())
    {
        X2QcToken token = tokens.front();
        tokens.pop();

        if (token.type == TOKEN_BRACKET_END)
        {
            for (size_t i = 0; i < defined.size(); i++)
            {
                if (!defined[i])
                {
                    DPRINTF("Parameter \"%s\" was not defined in ItemCollectionEntry object ", it_col_entry_params[i].c_str());
                    return LineError(token);
                }
            }

            if (mod_var.str.length() > 0)
            {
                int item_id;

                if (!GetCostumeMod(mod_var, entry.item_type, &item_id))
                    return false;

                entry.item_id = item_id;
            }

            return true;
        }
        else
        {
            X2QcToken value;

            if (!GetParam(token, value, it_col_entry_params, it_col_entry_types, defined))
                return false;

            int value_int = value.num;

            if (token.str == "item")
            {
                if (value.type == TOKEN_INTEGER)
                {
                    entry.item_id = value_int;
                }
                else if (value.type == TOKEN_IDENTIFIER)
                {
                    // Cannot process mod yet, because we need the item type, so just save the var
                    mod_var = value;
                }
                else
                {
                    DPRINTF("%s: BUG, shouldn't be here (%s)\n", FUNCNAME, value.str.c_str());
                    return false;
                }
            }
            else if (token.str == "type")
            {
                if (value.type == TOKEN_INTEGER)
                {
                    entry.item_type = (int16_t)value_int;

                    if (entry.item_type != 999)
                    {
                        DPRINTF("Unrecognized item_type \"%s\". ", value.str.c_str());
                        return LineError(value);
                    }
                }
                else // Identifier
                {
                    if (!GetItemType(value, &entry.item_type))
                        return false;
                }

                if (entry.item_type == QXD_ITEM_COLLECTION)
                {
                    DPRINTF("Type \"%s\" cannot be used in ItemCollectionEntry ", value.str.c_str());
                    return LineError(value);
                }
            }
            else if (token.str == "i6")
            {
                entry.unk_06 = (int16_t)value_int;
            }
            else if (token.str == "i8")
            {
                entry.unk_08 = (int16_t)value_int;
            }
            else if (token.str == "i10")
            {
                entry.unk_0A = (int16_t)value_int;
            }
            else
            {
                DPRINTF("%s: BUG, shouldn't be here (%s)\n", FUNCNAME, value.str.c_str());
                return false;
            }
        }
    }

    DPRINTF("Premature end of file (in ItemCollectionEntry object)\n");
    return false;
}

bool Xv2QuestCompiler::CompileItemCollection()
{
    QxdCollection collection;
    ci_string var_name;

    collection.id = 0xFFFF;

    if (!ProcessVarDeclaration(var_name))
        return false;

    if (!ExpectBracketBegin())
        return false;

    while (!tokens.empty())
    {
        X2QcToken token = tokens.front();
        tokens.pop();

        if (token.type == TOKEN_BRACKET_END)
        {
            if (collection.entries.size() == 0)
            {
                DPRINTF("No ItemCollectrionEntry were defined for this ItemCollection \"%s\" ", var_name.c_str());
                return LineError(token);
            }

            compiled_item_collections[var_name] = collection;
            return true;
        }
        else
        {
            if (!ExpectTokenType(token, TOKEN_IDENTIFIER))
                return false;

            if (token.str == "ItemCollectionEntry")
            {
                QxdCollectionEntry entry;

                if (!CompileItemCollectionEntry(entry))
                    return false;

                collection.entries.push_back(entry);
            }
            else
            {
                if (all_types.find(token.str) == all_types.end())
                {
                    DPRINTF("Unknown type \"%s\". ", token.str.c_str());
                }
                else
                {
                    DPRINTF("An object of type \"%s\" cannot be defined here. ", token.str.c_str());
                }

                return LineError(token);
            }
        }
    }

    DPRINTF("Premature end of file (in ItemCollection object)\n");
    return true;
}

bool Xv2QuestCompiler::CompileQxdUnk(QxdUnk &unk, int type)
{
    std::string my_name = (type == 1) ? "QxdUnk1" : "QxdUnk2";
    unk = QxdUnk();

    if (tokens.empty())
    {
        DPRINTF("Premature end of file (in %s)\n", my_name.c_str());
        return false;
    }

    X2QcToken token = tokens.front();
    tokens.pop();

    if (!ExpectTokenType(token, TOKEN_GROUP))
        return false;

    std::vector<X2QcToken> gtokens;
    if (!GetGroupTokens(token, gtokens))
        return false;

    if (gtokens.size() != 16)
    {
        DPRINTF("%s requires 16 values, but got %Id. ", my_name.c_str(), gtokens.size());
        return LineError(token);
    }

    for (size_t i = 0; i < 16; i++)
    {
        if (!ExpectTokenType(gtokens[i], TOKEN_INTEGER))
            return false;

        unk.unk_00[i] = (int16_t)gtokens[i].num;
    }

    return true;
}

bool Xv2QuestCompiler::CompileItemReward(QxdItemReward &reward)
{
    std::vector<bool> defined;
    X2QcToken var;

    reward = QxdItemReward();

    if (!ExpectBracketBegin())
        return false;

    defined.resize(item_reward_params.size(), false);

    while (!tokens.empty())
    {
        X2QcToken token = tokens.front();
        tokens.pop();

        if (token.type == TOKEN_BRACKET_END)
        {
            for (size_t i = 0; i < defined.size(); i++)
            {
                if (!defined[i])
                {
                    DPRINTF("Parameter \"%s\" was not defined in ItemReward object ", item_reward_params[i].c_str());
                    return LineError(token);
                }
            }

            if (var.str.length() > 0)
            {
                if (reward.type == QXD_ITEM_COLLECTION)
                {
                    if (!GetItemCollection(var, &reward.id))
                        return false;
                }
                else
                {
                    // Mod
                    int item_id;

                    if (!GetCostumeMod(var, reward.type, &item_id))
                        return false;

                    reward.id = (int32_t)item_id;
                }
            }

            return true;
        }
        else
        {
            X2QcToken value;

            if (!GetParam(token, value, item_reward_params, item_reward_types, defined))
                return false;

            int value_int = value.num;
            float value_float = value.fnum;

            if (token.str == "item")
            {
                if (value.type == TOKEN_INTEGER)
                {
                    reward.id = value_int;
                }
                else if (value.type == TOKEN_IDENTIFIER)
                {
                    // Cannot process mod or collection yet, because we need the item type, so just save the var
                    var = value;
                }
                else
                {
                    DPRINTF("%s: BUG, shouldn't be here (%s)\n", FUNCNAME, value.str.c_str());
                    return false;
                }
            }
            else if (token.str == "type")
            {
                uint16_t type;

                if (!GetItemType(value, &type))
                    return false;

                reward.type = type;
            }
            else if (token.str == "condition")
            {
                reward.condition = value_int;
            }
            else if (token.str == "i12")
            {
                reward.unk_0C = value_int;
            }
            else if (token.str == "flags")
            {
                if (value.type == TOKEN_INTEGER)
                {
                    reward.flags = value_int;
                }
                else if (value.str == "NORMAL" || value.str == "HIDDEN")
                {
                    reward.flags = (value.str == "HIDDEN") ? 2 : 0;
                }
                else
                {
                    DPRINTF("Invalid constant \"%s\" for flags. ", value.str.c_str());
                    return LineError(value);
                }
            }
            else if (token.str == "i20")
            {
                reward.unk_14 = value_int;
            }
            else if (token.str == "chance")
            {
                reward.chance = value_float;
            }
            else
            {
                DPRINTF("%s: BUG, shouldn't be here (%s)\n", FUNCNAME, value.str.c_str());
                return false;
            }
        }
    }

    DPRINTF("Premature end of file (in ItemReward object)\n");
    return false;
}

bool Xv2QuestCompiler::CompileSkillReward(QxdSkillReward &reward)
{
    std::vector<bool> defined;
    reward = QxdSkillReward();

    if (!ExpectBracketBegin())
        return false;

    defined.resize(skill_reward_params.size(), false);

    while (!tokens.empty())
    {
        X2QcToken token = tokens.front();
        tokens.pop();

        if (token.type == TOKEN_BRACKET_END)
        {
            for (size_t i = 0; i < defined.size(); i++)
            {
                if (!defined[i])
                {
                    DPRINTF("Parameter \"%s\" was not defined in SkillReward object ", skill_reward_params[i].c_str());
                    return LineError(token);
                }
            }

            return true;
        }
        else
        {
            X2QcToken value;

            if (!GetParam(token, value, skill_reward_params, skill_reward_types, defined))
                return false;

            int value_int = value.num;
            float value_float = value.fnum;

            if (token.str == "skill")
            {
                int16_t id2;

                if (!GetSkill(value, &id2, &reward.type, false))
                    return false;

                reward.id2 = (int32_t)id2;
            }
            else if (token.str == "condition")
            {
                reward.condition = value_int;
            }
            else if (token.str == "i12")
            {
                reward.unk_0C = value_int;
            }
            else if (token.str == "chance")
            {
                reward.chance = value_float;
            }
            else
            {
                DPRINTF("%s: BUG, shouldn't be here (%s)\n", FUNCNAME, value.str.c_str());
                return false;
            }
        }
    }

    DPRINTF("Premature end of file (in SkillReward object)\n");
    return false;
}

bool Xv2QuestCompiler::CompileCharReward(QxdCharReward &reward)
{
    std::vector<bool> defined;
    reward = QxdCharReward();

    if (!ExpectBracketBegin())
        return false;

    defined.resize(char_reward_params.size(), false);

    while (!tokens.empty())
    {
        X2QcToken token = tokens.front();
        tokens.pop();

        if (token.type == TOKEN_BRACKET_END)
        {
            for (size_t i = 0; i < defined.size(); i++)
            {
                if (!defined[i])
                {
                    DPRINTF("Parameter \"%s\" was not defined in CharReward object ", char_reward_params[i].c_str());
                    return LineError(token);
                }
            }

            return true;
        }
        else
        {
            X2QcToken value;

            if (!GetParam(token, value, char_reward_params, char_reward_types, defined))
                return false;

            int value_int = value.num;

            if (token.str == "char")
            {
                if (!GetChar(value, reward.cms_name))
                    return false;
            }
            else if (token.str == "costume")
            {
                reward.costume_index = (int16_t)value_int;
            }
            else if (token.str == "i6")
            {
                reward.unk_06 = (int16_t) value_int;
            }
            else
            {
                DPRINTF("%s: BUG, shouldn't be here (%s)\n", FUNCNAME, value.str.c_str());
                return false;
            }
        }
    }

    DPRINTF("Premature end of file (in CharReward object)\n");
    return false;
}

bool Xv2QuestCompiler::CompileCharPortrait(QxdCharPortrait &portrait)
{
    std::vector<bool> defined;
    portrait = QxdCharPortrait();

    if (!ExpectBracketBegin())
        return false;

    defined.resize(char_portrait_params.size(), false);

    while (!tokens.empty())
    {
        X2QcToken token = tokens.front();
        tokens.pop();

        if (token.type == TOKEN_BRACKET_END)
        {
            for (size_t i = 0; i < defined.size(); i++)
            {
                if (!defined[i])
                {
                    DPRINTF("Parameter \"%s\" was not defined in CharPortrait object ", char_portrait_params[i].c_str());
                    return LineError(token);
                }
            }

            return true;
        }
        else
        {
            X2QcToken value;

            if (!GetParam(token, value, char_portrait_params, char_portrait_types, defined))
                return false;

            int value_int = value.num;

            if (token.str == "char")
            {
                if (!GetChar(value, (int16_t *)&portrait.cms_id))
                    return false;
            }
            else if (token.str == "costume")
            {
                portrait.costume_index = (int16_t)value_int;
            }
            else if (token.str == "trans")
            {
                portrait.trans = (int16_t)value_int;
            }
            else
            {
                DPRINTF("%s: BUG, shouldn't be here (%s)\n", FUNCNAME, value.str.c_str());
                return false;
            }
        }
    }

    DPRINTF("Premature end of file (in CharPortrait object)\n");
    return false;
}

bool Xv2QuestCompiler::CompileQuestStruct()
{
    ci_string var_name;
    std::vector<bool> defined;
    int next_portrait = 0;

    if (compiled_quest.name.length() > 0)
    {
        DPRINTF("You can only define a single quest object. ");
        if (!tokens.empty())
            return LineError(tokens.front());

        DPRINTF("\n");
        return false;
    }

    if (!ProcessVarDeclaration(var_name))
        return false;

    compiled_quest.name = Utils::ToUpperCase(var_name.str);
    quest_type = GetQuestType(compiled_quest.name);

    if (quest_type < 0)
    {
        DPRINTF("Invalid quest name \"%s\". Quest name must start by one of the quest types (HLQ_,TMQ_,etc).\n", var_name.str.c_str());
        return false;
    }

    if (!LoadActiveQxd())
    {
        DPRINTF("%s: Failed to load qxd file from game.\n", FUNCNAME);
        return false;
    }

    if (!LoadTitle())
        return false;

    if (!ExpectBracketBegin())
        return false;

    defined.resize(quest_params.size(), false);
    compiled_quest.msg_entries.resize(4);

    while (!tokens.empty())
    {
        X2QcToken token = tokens.front();
        tokens.pop();

        if (token.type == TOKEN_BRACKET_END)
        {
            bool warning_defined = false;
            bool ex_success_defined = false;

            for (size_t i = 0; i < defined.size(); i++)
            {
                if (!defined[i])
                {
                    if (quest_params[i] != "warning" && quest_params[i] != "ex_success")
                    {
                        DPRINTF("Parameter \"%s\" was not defined in Quest object ", quest_params[i].c_str());
                        return LineError(token);
                    }
                }
                else
                {
                    if (quest_params[i] == "warning")
                        warning_defined = true;

                    else if (quest_params[i] == "ex_success")
                        ex_success_defined = true;
                }
            }

            if (warning_defined && !ex_success_defined)
            {
                DPRINTF("If \"warning\" param is set, \"ex_success\" param must be set too (in Quest object).\n");
                return false;
            }
            else if (!warning_defined && ex_success_defined)
            {
                DPRINTF("If \"ex_success\" param is set, \"warning\" param must be set too (in Quest object).\n");
                return false;
            }

            if (!LoadDialogue(compiled_quest.name, compiled_quest.episode))
            {
                DPRINTF("%s: Failed to load dialogue file for \"%s\" (episode: %d).\n", FUNCNAME, compiled_quest.name.c_str(), compiled_quest.episode);
                return false;
            }

            if (SupportsAudio())
            {
                if (!LoadDialogueAudio(compiled_quest.name, compiled_quest.episode, compiled_quest.flags))
                    return false;
            }

            return true;
        }
        else
        {
            if (!ExpectTokenType(token, TOKEN_IDENTIFIER))
                return false;

            if (token.str == "QxdUnk1")
            {
                QxdUnk unk1;

                if (!CompileQxdUnk(unk1, 1))
                    return false;

                compiled_quest.unk1s.push_back(unk1);
            }
            else if (token.str == "QxdUnk2")
            {
                QxdUnk unk2;

                if (!CompileQxdUnk(unk2, 2))
                    return false;

                compiled_quest.unk2s.push_back(unk2);
            }
            else if (token.str == "ItemReward")
            {
                QxdItemReward reward;

                if (!CompileItemReward(reward))
                    return false;

                compiled_quest.item_rewards.push_back(reward);
            }
            else if (token.str == "SkillReward")
            {
                QxdSkillReward reward;

                if (!CompileSkillReward(reward))
                    return false;

                compiled_quest.skill_rewards.push_back(reward);
            }
            else if (token.str == "CharReward")
            {
                QxdCharReward reward;

                if (!CompileCharReward(reward))
                    return false;

                compiled_quest.char_rewards.push_back(reward);
            }
            else if (token.str == "CharPortrait")
            {
                if (next_portrait == 6)
                {
                    DPRINTF("Cannot have more than 6 CharPortrait. ");
                    return LineError(token);
                }

                QxdCharPortrait portrait;

                if (!CompileCharPortrait(portrait))
                    return false;

                compiled_quest.enemy_portraits[next_portrait++] = portrait;
            }
            else
            {
                if (all_types.find(token.str) != all_types.end())
                {
                    DPRINTF("\"%s\" cannot be defined here. ", token.str.c_str());
                    return LineError(token);
                }

                X2QcToken value;

                if (!GetParam(token, value, quest_params, quest_types, defined))
                    return false;

                int value_int = value.num;
                float value_float = value.fnum;
                std::string &value_str = value.str.str;
                ci_string &value_cistr = value.str;

                if (token.str == "episode")
                {
                    compiled_quest.episode = value_int;
                }
                else if (token.str == "sub_type")
                {
                    compiled_quest.sub_type = value_int;
                }
                else if (token.str == "num_players")
                {
                    compiled_quest.num_players = value_int;
                }
                else if (token.str == "title")
                {
                    if (!GetTitle(value, compiled_quest.msg_entries[0], 0))
                        return false;
                }
                else if (token.str == "success")
                {
                    if (!GetTitle(value, compiled_quest.msg_entries[1], 1))
                        return false;
                }
                else if (token.str == "failure")
                {
                    if (!GetTitle(value, compiled_quest.msg_entries[2], 2))
                        return false;
                }
                else if (token.str == "outline")
                {
                    if (!GetTitle(value, compiled_quest.msg_entries[3], 3))
                        return false;
                }
                else if (token.str == "warning")
                {
                    if (compiled_quest.msg_entries.size() == 4)
                        compiled_quest.msg_entries.resize(6);

                    if (!GetTitle(value, compiled_quest.msg_entries[4], 4))
                        return false;
                }
                else if (token.str == "ex_success")
                {
                    if (compiled_quest.msg_entries.size() == 4)
                        compiled_quest.msg_entries.resize(6);

                    if (!GetTitle(value, compiled_quest.msg_entries[5], 5))
                        return false;
                }
                else if (token.str == "i40")
                {
                    compiled_quest.unk_28 = (int16_t)value_int;
                }
                else if (token.str == "parent_quest")
                {
                    if (value.type == TOKEN_STRING && Utils::ToUpperCase(value_str) == compiled_quest.name)
                    {
                        self_reference_parent = true;
                        compiled_quest.parent_quest = 0xFFFF; // Will be filled later
                    }

                    else if (!GetQuest(value, (int16_t *)&compiled_quest.parent_quest, false))
                        return false;
                }
                else if (token.str == "i44")
                {
                    compiled_quest.unk_2C = (int16_t)value_int;
                }
                else if (token.str == "i48")
                {
                    compiled_quest.unk_30[0] = value_int;
                }
                else if (token.str == "i52")
                {
                    compiled_quest.unk_30[1] = value_int;
                }
                else if (token.str == "i56")
                {
                    compiled_quest.unk_30[2] = value_int;
                }
                else if (token.str == "i60")
                {
                    compiled_quest.unk_30[3] = value_int;
                }
                else if (token.str == "unlock_requirement")
                {                    
                    if (value.type == TOKEN_STRING && Utils::ToUpperCase(value_str) == compiled_quest.name)
                    {
                        self_reference_unlock = true;
                        compiled_quest.unlock_requirement = 0xFFFFFFFF; // Will be filled later
                    }

                    else if (!GetQuest(value, (int16_t *)&compiled_quest.unlock_requirement, false))
                        return false;
                }
                else if (token.str == "i68")
                {
                    compiled_quest.unk_44[0] = value_int;
                }
                else if (token.str == "i72")
                {
                    compiled_quest.unk_44[1] = value_int;
                }
                else if (token.str == "i76")
                {
                    compiled_quest.unk_44[2] = value_int;
                }
                else if (token.str == "i80")
                {
                    compiled_quest.unk_44[3] = value_int;
                }
                else if (token.str == "i84")
                {
                    compiled_quest.unk_44[4] = value_int;
                }
                else if (token.str == "time_limit")
                {
                    compiled_quest.time_limit = (int16_t)value_int;
                }
                else if (token.str == "difficulty")
                {
                    compiled_quest.difficulty = (int16_t)value_int;
                }
                else if (token.str == "start_stage")
                {
                    if (!GetStage(value, (int16_t *)&compiled_quest.start_stage))
                        return false;
                }
                else if (token.str == "start_demo")
                {
                    compiled_quest.start_demo = (int16_t)value_int;
                }
                else if (token.str == "xp_reward")
                {
                    compiled_quest.xp_reward = value_int;
                }
                else if (token.str == "ult_xp_reward")
                {
                    compiled_quest.ult_xp_reward = value_int;
                }
                else if (token.str == "fail_xp_reward")
                {
                    compiled_quest.fail_xp_reward = value_int;
                }
                else if (token.str == "zeni_reward")
                {
                    compiled_quest.zeni_reward = value_int;
                }
                else if (token.str == "ult_zeni_reward")
                {
                    compiled_quest.ult_zeni_reward = value_int;
                }
                else if (token.str == "fail_zeni_reward")
                {
                    compiled_quest.fail_zeni_reward = value_int;
                }
                else if (token.str == "tp_medals_once")
                {
                    compiled_quest.tp_medals_once = value_int;
                }
                else if (token.str == "tp_medals")
                {
                    compiled_quest.tp_medals = value_int;
                }
                else if (token.str == "tp_medals_special")
                {
                    compiled_quest.tp_medals_special = value_int;
                }
                else if (token.str == "resistance_points")
                {
                    compiled_quest.resistance_points = value_int;
                }
                else if (token.str == "stages")
                {
                    std::vector<X2QcToken> gtokens;

                    if (!GetGroupTokens(value, gtokens))
                        return false;

                    if (gtokens.size() == 0)
                    {
                        DPRINTF("There must be at least one stage portrait. ");
                        return LineError(value);
                    }
                    else if (gtokens.size() > 16)
                    {
                        DPRINTF("Only a max of 16 stage portraits allowed, found %Id. ", gtokens.size());
                        return LineError(value);
                    }

                    for (size_t i = 0; i < gtokens.size(); i++)
                    {
                        if (!ExpectTokenType(gtokens[i], { TOKEN_STRING, TOKEN_IDENTIFIER, TOKEN_INTEGER }))
                            return false;

                        if (!GetStage(gtokens[i], (int16_t *)&compiled_quest.stages[i]))
                            return false;
                    }
                }
                else if (token.str == "i192")
                {
                    compiled_quest.unk_C0 = value_int;
                }
                else if (token.str == "i232")
                {
                    compiled_quest.unk_E8[0] = (int16_t)value_int;
                }
                else if (token.str == "i234")
                {
                    compiled_quest.unk_E8[1] = (int16_t)value_int;
                }
                else if (token.str == "i236")
                {
                    compiled_quest.unk_E8[2] = (int16_t)value_int;
                }
                else if (token.str == "i238")
                {
                    compiled_quest.unk_E8[3] = (int16_t)value_int;
                }
                else if (token.str == "i240")
                {
                    compiled_quest.unk_E8[4] = (int16_t)value_int;
                }
                else if (token.str == "i242")
                {
                    compiled_quest.unk_E8[5] = (int16_t)value_int;
                }
                else if (token.str == "i244")
                {
                    compiled_quest.unk_E8[6] = (int16_t)value_int;
                }
                else if (token.str == "i246")
                {
                    compiled_quest.unk_E8[7] = (int16_t)value_int;
                }
                else if (token.str == "i248")
                {
                    compiled_quest.unk_E8[8] = (int16_t)value_int;
                }
                else if (token.str == "i250")
                {
                    compiled_quest.unk_E8[9] = (int16_t)value_int;
                }
                else if (token.str == "flags")
                {
                    compiled_quest.flags = value_int;
                }                
                else if (token.str == "update_requirement")
                {
                    int update = ConstantToUpdate(value_cistr);
                    if (update < 0)
                    {
                        DPRINTF("Unrecognized update constant \"%s\". ", value_cistr.c_str());
                        return LineError(value);
                    }

                    compiled_quest.update_requirement = update;
                }
                else if (token.str == "dlc_requirement")
                {
                    int dlc = ConstantToDlc(value_cistr);
                    if (dlc < 0)
                    {
                        DPRINTF("Unrecognized dlc constant \"%s\". ", value_cistr.c_str());
                        return LineError(value);
                    }

                    compiled_quest.dlc_requirement = dlc;
                }
                else if (token.str == "i264")
                {
                    compiled_quest.unk_108 = value_int;
                }
                else if (token.str == "no_enemy_bgm")
                {
                    compiled_quest.no_enemy_music = (int16_t)value_int;
                }
                else if (token.str == "enemy_near_bgm")
                {
                    compiled_quest.enemy_near_music = (int16_t)value_int;
                }
                else if (token.str == "battle_bgm")
                {
                    compiled_quest.battle_music = (int16_t)value_int;
                }
                else if (token.str == "ultimate_finish_bgm")
                {
                    compiled_quest.ult_finish_music = (int16_t)value_int;
                }
                else if (token.str == "f276")
                {
                    compiled_quest.unk_114 = value_float;
                }
                else if (token.str == "i280")
                {
                    compiled_quest.unk_118 = value_int;
                }
                else
                {
                    DPRINTF("%s: Bug, shouldn't be here (%s)\n", FUNCNAME, token.str.c_str());
                    return false;
                }
            }
        }
    }

    DPRINTF("Premature end of file (in Quest object)\n");
    return false;
}

bool Xv2QuestCompiler::CompileQxdChar(bool special)
{
    QxdCharacter chara;
    ci_string var_name;
    std::vector<bool> defined;

    if (!ProcessVarDeclaration(var_name))
        return false;

    if (!ExpectBracketBegin())
        return false;

    defined.resize(qxd_char_params.size(), false);
    chara.id = 0xFFFFFFFF;

    while (!tokens.empty())
    {
        X2QcToken token = tokens.front();
        tokens.pop();

        if (token.type == TOKEN_BRACKET_END)
        {
            for (size_t i = 0; i < defined.size(); i++)
            {
                if (!defined[i])
                {
                    DPRINTF("Parameter \"%s\" was not defined in QxdChar object ", qxd_char_params[i].c_str());
                    return LineError(token);
                }
            }

            if (special)
                compiled_special_chars[var_name] = chara;
            else
                compiled_chars[var_name] = chara;

            return true;
        }
        else
        {
            X2QcToken value;

            if (!GetParam(token, value, qxd_char_params, qxd_char_types, defined))
                return false;

            int value_int = value.num;
            float value_float = value.fnum;

            if (token.str == "char")
            {
                if (!GetChar(value, chara.cms_name))
                    return false;
            }
            else if (token.str == "costume")
            {
                chara.costume = value_int;
            }
            else if (token.str == "transformation")
            {
                chara.transformation = (int16_t)value_int;
            }
            else if (token.str == "special_effect")
            {
                chara.special_effect = (int16_t)value_int;
            }
            else if (token.str == "i12")
            {
                chara.unk_0C = value_int;
            }
            else if (token.str == "level")
            {
                chara.level = value_int;
            }
            else if (token.str == "health")
            {
                chara.health = value_float;
            }
            else if (token.str == "f24")
            {
                chara.unk_18 = value_float;
            }
            else if (token.str == "ki")
            {
                chara.ki = value_float;
            }
            else if (token.str == "stamina")
            {
                chara.stamina = value_float;
            }
            else if (token.str == "atk")
            {
                chara.basic_melee = value_float;
            }
            else if (token.str == "ki_atk")
            {
                chara.ki_blast = value_float;
            }
            else if (token.str == "super_atk")
            {
                chara.strike_super = value_float;
            }
            else if (token.str == "super_ki")
            {
                chara.ki_super = value_float;
            }
            else if (token.str == "atk_damage")
            {
                chara.basic_melee_damage = value_float;
            }
            else if (token.str == "ki_damage")
            {
                chara.ki_blast_damage = value_float;
            }
            else if (token.str == "super_atk_damage")
            {
                chara.strike_super_damage = value_float;
            }
            else if (token.str == "super_ki_damage")
            {
                chara.ki_super_damage = value_float;
            }
            else if (token.str == "guard_atk")
            {
                chara.unk_44 = value_float;
            }
            else if (token.str == "guard_damage")
            {
                chara.unk_48 = value_float;
            }
            else if (token.str == "move_speed")
            {
                chara.air_speed = value_float;
            }
            else if (token.str == "boost_speed")
            {
                chara.boost_speed = value_float;
            }
            else if (token.str == "ait_table_entry")
            {
                chara.ait_table_entry = value_int;
            }
            else if (token.str == "super1")
            {
                if (!GetSuperSkill(value, (int16_t *)&chara.skills[0]))
                    return false;
            }
            else if (token.str == "super2")
            {
                if (!GetSuperSkill(value, (int16_t *)&chara.skills[1]))
                    return false;
            }
            else if (token.str == "super3")
            {
                if (!GetSuperSkill(value, (int16_t *)&chara.skills[2]))
                    return false;
            }
            else if (token.str == "super4")
            {
                if (!GetSuperSkill(value, (int16_t *)&chara.skills[3]))
                    return false;
            }
            else if (token.str == "ultimate1")
            {
                if (!GetUltimateSkill(value, (int16_t *)&chara.skills[4]))
                    return false;
            }
            else if (token.str == "ultimate2")
            {
                if (!GetUltimateSkill(value, (int16_t *)&chara.skills[5]))
                    return false;
            }
            else if (token.str == "evasive")
            {
                if (!GetEvasiveSkill(value, (int16_t *)&chara.skills[6]))
                    return false;
            }
            else if (token.str == "blast")
            {
                if (!GetBlastSkill(value, (int16_t *)&chara.skills[7]))
                    return false;
            }
            else if (token.str == "awaken")
            {
                if (!GetAwakenSkill(value, (int16_t *)&chara.skills[8]))
                    return false;
            }
            else if (token.str == "i106")
            {
                chara.unk_6A[0] = (int16_t)value_int;
            }
            else if (token.str == "i108")
            {
                chara.unk_6A[1] = (int16_t)value_int;
            }
            else if (token.str == "i112")
            {
                chara.unk_6A[3] = (int16_t)value_int;
            }
            else
            {
                DPRINTF("%s: BUG, shouldn't be here (%s)\n", FUNCNAME, token.str.c_str());
                return false;
            }
        }
    }

    DPRINTF("Premature end of file (in QxdChar object)\n");
    return false;
}

bool Xv2QuestCompiler::CompileQmlChar()
{
    QmlEntry qchar;
    ci_string var_name;
    std::vector<bool> defined;

    if (!ProcessVarDeclaration(var_name))
        return false;

    if (!ExpectColon())
        return false;

    if (tokens.empty())
    {
        DPRINTF("Premature end of file while waiting for base identifier.\n");
        return false;
    }

    X2QcToken base = tokens.front();
    tokens.pop();

    qchar.id = 0xFFFFFFFF;

    if (!GetQxdChar(base, &qchar.qxd_id))
        return false;

    if (!ExpectBracketBegin())
        return false;

    defined.resize(qml_char_params.size(), false);

    while (!tokens.empty())
    {
        X2QcToken token = tokens.front();
        tokens.pop();

        if (token.type == TOKEN_BRACKET_END)
        {
            for (size_t i = 0; i < defined.size(); i++)
            {
                if (!defined[i])
                {
                    DPRINTF("Parameter \"%s\" was not defined in QmlChar object ", qml_char_params[i].c_str());
                    return LineError(token);
                }
            }

            active_qml.AddQmlEntry(qchar);
            compiled_qml_chars[var_name] = qchar; // Must be below the previous line
            return true;
        }
        else
        {
            X2QcToken value;

            if (!GetParam(token, value, qml_char_params, qml_char_types, defined))
                return false;

            int value_int = value.num;
            bool value_bool = value.b;
            const ci_string &value_cistr = value.str;

            if (token.str == "battle_index")
            {
                if (value_int < -1 || value_int > 6)
                {
                    DPRINTF("battle_index out of valid range [-1,6] ");
                    return LineError(value);
                }

                qchar.battle_index = value_int;
            }
            else if (token.str == "i12")
            {
                qchar.unk_0C = value_int;
            }
            else if (token.str == "stage")
            {
                int16_t stage_id;

                if (!GetStage(value, &stage_id))
                    return false;

                qchar.stage_id = (int32_t)stage_id;
            }
            else if (token.str == "spawn_at_start")
            {
                qchar.spawn_at_start = (value_bool != false);
            }
            else if (token.str == "ai")
            {                
                qchar.ai = ConstantToAi(value_cistr);
                if ((int)qchar.ai < 0)
                {
                    DPRINTF("Unrecognized ai constant \"%s\" ", value_cistr.c_str());
                    return LineError(value);
                }
            }
            else if (token.str == "team")
            {
                qchar.team = ConstantToTeam(value_cistr);
                if ((int)qchar.team < 0)
                {
                    DPRINTF("Unrecognized team constant \"%s\" ", value_cistr.c_str());
                    return LineError(value);
                }
            }
            else if (token.str == "i36")
            {
                qchar.unk_24 = value_int;
            }
            else if (token.str == "i40")
            {
                qchar.unk_28 = value_int;
            }
            else if (token.str == "i44")
            {
                qchar.unk_2C = value_int;
            }
            else if (token.str == "i48")
            {
                qchar.unk_30[0] = (int16_t)value_int;
            }
            else if (token.str == "i50")
            {
                qchar.unk_30[1] = (int16_t)value_int;
            }
            else if (token.str == "i52")
            {
                qchar.unk_30[2] = (int16_t)value_int;
            }
            else if (token.str == "i56")
            {
                qchar.unk_30[4] = (int16_t)value_int;
            }
            else if (token.str == "super1")
            {
                if (!GetSuperSkill(value, (int16_t *)&qchar.skills[0]))
                    return false;
            }
            else if (token.str == "super2")
            {
                if (!GetSuperSkill(value, (int16_t *)&qchar.skills[1]))
                    return false;
            }
            else if (token.str == "super3")
            {
                if (!GetSuperSkill(value, (int16_t *)&qchar.skills[2]))
                    return false;
            }
            else if (token.str == "super4")
            {
                if (!GetSuperSkill(value, (int16_t *)&qchar.skills[3]))
                    return false;
            }
            else if (token.str == "ultimate1")
            {
                if (!GetUltimateSkill(value, (int16_t *)&qchar.skills[4]))
                    return false;
            }
            else if (token.str == "ultimate2")
            {
                if (!GetUltimateSkill(value, (int16_t *)&qchar.skills[5]))
                    return false;
            }
            else if (token.str == "evasive")
            {
                if (!GetEvasiveSkill(value, (int16_t *)&qchar.skills[6]))
                    return false;
            }
            else if (token.str == "blast")
            {
                if (!GetBlastSkill(value, (int16_t *)&qchar.skills[7]))
                    return false;
            }
            else if (token.str == "awaken")
            {
                if (!GetAwakenSkill(value, (int16_t *)&qchar.skills[8]))
                    return false;
            }
            else
            {
                DPRINTF("%s: BUG, shouldn't be here (%s).\n", FUNCNAME, token.str.c_str());
                return false;
            }
        }
    }

    DPRINTF("Premature end of file (in QmlChar object)\n");
    return false;
}

bool Xv2QuestCompiler::CompileDialoguePart(QbtDialoguePart &part, bool *special_case_osq_0301, int *out_of_order, bool *dont_change, bool *continue_next)
{   
    part = QbtDialoguePart();
    std::vector<bool> defined;

    if (!ExpectBracketBegin())
        return false;

    *special_case_osq_0301 = false;
    *out_of_order = -1;
    defined.resize(dialogue_part_params.size(), false);

    if (dont_change)
        *dont_change = false;

    if (continue_next)
        *continue_next = false;

    while (!tokens.empty())
    {
        X2QcToken token = tokens.front();
        tokens.pop();

        if (token.type == TOKEN_BRACKET_END)
        {
            for (size_t i = 0; i < defined.size(); i++)
            {
                if (!defined[i] && dialogue_part_params[i] != "debug_actor" &&
                    dialogue_part_params[i] != "special_case_osq_0301" && dialogue_part_params[i] != "out_of_order" &&
                    dialogue_part_params[i] != "dont_change" && dialogue_part_params[i] != "continue_next")
                {
                    DPRINTF("Parameter \"%s\" was not defined in DialoguePart object ", dialogue_part_params[i].c_str());
                    return LineError(token);
                }
            }

            return true;
        }
        else
        {
            X2QcToken value;

            if (!GetParam(token, value, dialogue_part_params, dialogue_part_types, defined))
                return false;

            int value_int = value.num;
            bool value_boolean = value.b;

            if (token.str == "actor")
            {
                if (!GetChar(value, (int16_t *)&part.portrait_cms))
                    return false;
            }
            else if (token.str == "costume")
            {
                part.portrait_costume_index = (int16_t)value_int;
            }
            else if (token.str == "transformation")
            {
                part.portrait_transformation = (int16_t)value_int;
            }
            else if (token.str == "text_audio")
            {
                if (!GetDialogueTA(value, part.msg_id))
                    return false;
            }
            else if (token.str == "i8")
            {
                part.unk_08 = (int16_t)value_int;
            }
            else if (token.str == "debug_actor")
            {
                int16_t cms_id;

                if (!GetChar(value, &cms_id))
                    return false;

                part.cms_04 = (int32_t)cms_id;                
            }
            else if (token.str == "special_case_osq_0301")
            {
                *special_case_osq_0301 = value_boolean;
            }
            else if (token.str == "out_of_order")
            {
                *out_of_order = value_int;
            }
            else if (token.str == "dont_change")
            {
                if (dont_change)
                    *dont_change = value_boolean;
            }
            else if (token.str == "continue_next")
            {
                if (continue_next)
                    *continue_next = value_boolean;
            }
            else
            {
                DPRINTF("%s: BUG, shouldn't be here (%s).\n", FUNCNAME, token.str.c_str());
                return false;
            }
        }
    }

    DPRINTF("Premature end of file (in DialoguePart object)\n");
    return false;
}

bool Xv2QuestCompiler::CompileDialogue()
{
    QbtEntry dialogue;
    ci_string var_name;
    std::vector<bool> defined;
    uint16_t qbt_id;
    bool use_i2 = false;

    if (!ProcessVarDeclaration(var_name))
        return false;

    uint16_t suggested_id = 0xFFFF;
    if (var_name.length() > strlen("Dialogue") && Utils::BeginsWith(var_name.str, "Dialogue", false))
    {
        std::string num_str = var_name.str.substr(strlen("Dialogue"));

        if (Utils::HasOnlyDigits(num_str))
            suggested_id = (uint16_t)Utils::GetUnsigned(num_str);
    }

    if (suggested_id < 0x8000)
    {
        qbt_id = suggested_id;
    }
    else
    {
        qbt_id = (uint16_t) active_qbt.GetTotalNumEntries();
    }

    if (!ExpectBracketBegin())
        return false;

    defined.resize(dialogue_params.size(), false);

    while (!tokens.empty())
    {
        X2QcToken token = tokens.front();
        tokens.pop();

        if (token.type == TOKEN_BRACKET_END)
        {
            for (size_t i = 0; i < defined.size(); i++)
            {
                if (!defined[i] && dialogue_params[i] != "debug_actor" && dialogue_params[i] != "b_use_i2")
                {
                    DPRINTF("Parameter \"%s\" was not defined in Dialogue object ", dialogue_params[i].c_str());
                    return LineError(token);
                }
            }

            if (use_i2 && dialogue.parts.size() > 0)
                dialogue.unk_02 = dialogue.parts[0].qbt_id;

            compiled_dialogues[var_name] = (uint32_t)active_qbt.GetNormalEntries().size();
            active_qbt.GetNormalEntries().push_back(dialogue);

            return true;
        }
        else
        {
            if (!ExpectTokenType(token, TOKEN_IDENTIFIER))
                return false;

            if (token.str == "DialoguePart")
            {
                QbtDialoguePart part;
                bool special_case;
                int out_of_order;

                if (!CompileDialoguePart(part, &special_case, &out_of_order))
                    return false;

                part.qbt_id = qbt_id;

                if (out_of_order >= 0)
                {
                    part.sub_id = (uint16_t)out_of_order;
                }
                else
                {
                    part.sub_id = (uint16_t)dialogue.parts.size();
                }

                if (special_case)
                    part.qbt_id++;

                dialogue.parts.push_back(part);
            }
            else
            {
                X2QcToken value;

                if (!GetParam(token, value, dialogue_params, dialogue_types, defined))
                    return false;

                int value_int = value.num;
                bool value_boolean = value.b;

                if (token.str == "i28")
                {
                    dialogue.unk_1C = value_int;
                }
                else if (token.str == "b_use_i2")
                {
                    use_i2 = value_boolean;
                }
                else if (token.str == "debug_actor")
                {
                    if (!GetChar(value, (int16_t *)&dialogue.cms_20))
                        return false;
                }
                else
                {
                    DPRINTF("%s: BUG, shouldn't be here (%s).\n", FUNCNAME, token.str.c_str());
                    return false;
                }
            }
        }
    }

    DPRINTF("Premature end of file (in Dialogue object)\n");
    return false;
}

bool Xv2QuestCompiler::CompileInteractiveDialogue()
{
    QbtEntry dialogue;
    ci_string var_name;
    std::vector<bool> defined;
    uint16_t qbt_id;
    bool use_i2 = false;

    X2QcToken iparam;
    X2QcToken item_type_token;
    int item_type = -1;

    if (!ProcessVarDeclaration(var_name))
        return false;

    uint16_t suggested_id = 0xFFFF;
    if (var_name.length() > strlen("InteractiveDialogue") && Utils::BeginsWith(var_name.str, "InteractiveDialogue", false))
    {
        std::string num_str = var_name.str.substr(strlen("InteractiveDialogue"));

        if (Utils::HasOnlyDigits(num_str))
            suggested_id = (uint16_t)Utils::GetUnsigned(num_str);
    }

    if (suggested_id < 0x8000)
    {
        qbt_id = suggested_id;
    }
    else
    {
        qbt_id = (uint16_t) active_qbt.GetTotalNumEntries();
    }

    if (!ExpectBracketBegin())
        return false;

    defined.resize(interactive_dialogue_params.size(), false);

    while (!tokens.empty())
    {
        X2QcToken token = tokens.front();
        tokens.pop();

        if (token.type == TOKEN_BRACKET_END)
        {
            for (size_t i = 0; i < defined.size(); i++)
            {
                if (!defined[i] && interactive_dialogue_params[i] != "debug_actor" &&
                    interactive_dialogue_params[i] != "b_use_i2" && interactive_dialogue_params[i] != "item_type")
                {
                    DPRINTF("Parameter \"%s\" was not defined in InteractiveDialogue object ", interactive_dialogue_params[i].c_str());
                    return LineError(token);
                }
            }

            if (dialogue.interaction_type == QBT_ITYPE_GIVE_ITEM)
            {
                if (item_type < 0)
                {
                    DPRINTF("item_type must be defined when interaction_type is GIVE_ITEM. ");
                    return LineError(token);
                }

                uint32_t item_id;

                if (iparam.type == TOKEN_INTEGER)
                {
                    item_id = iparam.num;
                }
                else // Identifier
                {
                    if (item_type == QXD_ITEM_COLLECTION)
                    {
                        if (!GetItemCollection(iparam, &item_id))
                            return false;
                    }
                    else
                    {
                        // Mod
                        if (!GetCostumeMod(iparam, item_type, (int *)&item_id))
                            return false;
                    }
                }

                 dialogue.interaction_param = (item_type*1000) + item_id;
            }
            else
            {
                if (item_type >= 0)
                {
                    DPRINTF("item_type can only be defined when interaction_type is GIVE_ITEM. ");
                    return LineError(item_type_token);
                }

                if (!ExpectTokenType(iparam, TOKEN_INTEGER))
                    return false;

                dialogue.interaction_param = iparam.num;
            }

            if (use_i2 && dialogue.parts.size() > 0)
                dialogue.unk_02 = dialogue.parts[0].qbt_id;

            compiled_interactive_dialogues[var_name] = (uint32_t)active_qbt.GetInteractiveEntries().size();
            active_qbt.GetInteractiveEntries().push_back(dialogue);

            return true;
        }
        else
        {
            if (!ExpectTokenType(token, TOKEN_IDENTIFIER))
                return false;

            if (token.str == "DialoguePart")
            {
                QbtDialoguePart part;
                bool special_case = false;
                int out_of_order;
                bool dont_change;
                bool continue_next;

                if (!CompileDialoguePart(part, &special_case, &out_of_order, &dont_change, &continue_next))
                    return false;

                if (dont_change)
                {
                    part.cms_04 = (uint32_t)active_qbt.GetInteractiveEntries().size();                   
                }
                else if (continue_next)
                {
                    part.cms_04 = (uint32_t)active_qbt.GetInteractiveEntries().size()+1;
                }

                part.qbt_id = qbt_id;

                if (out_of_order >= 0)
                {
                    part.sub_id = (uint16_t)out_of_order;
                }
                else
                {
                    part.sub_id = (uint16_t)dialogue.parts.size();
                }

                if (special_case)
                    part.qbt_id++;

                dialogue.parts.push_back(part);
            }
            else
            {
                X2QcToken value;

                if (!GetParam(token, value, interactive_dialogue_params, interactive_dialogue_types, defined))
                    return false;

                int value_int = value.num;
                bool value_boolean = value.b;
                ci_string &value_cistr = value.str;

                if (token.str == "interaction_type")
                {
                    if (value.type == TOKEN_INTEGER)
                    {
                        dialogue.interaction_type = value_int;
                    }
                    else // Identifier
                    {
                        auto it = constant_to_interaction_type.find(value_cistr);
                        if (it == constant_to_interaction_type.end())
                        {
                            DPRINTF("Unrecognized interaction_type constant \"%s\". ", value_cistr.c_str());
                            return LineError(value);
                        }

                        dialogue.interaction_type = it->second;
                    }
                }
                else if (token.str == "param")
                {
                    iparam = value;
                }
                else if (token.str == "item_type")
                {
                    auto it = constant_to_item_type.find(value_cistr);
                    if (it == constant_to_item_type.end())
                    {
                        DPRINTF("Unrecognized item_type constant \"%s\". ", value_cistr.c_str());
                        return LineError(value);
                    }

                    item_type = it->second;
                    item_type_token = token;
                }
                else if (token.str == "i0")
                {
                    dialogue.unk_00 = (int16_t)value_int;
                }
                else if (token.str == "b_use_i2")
                {
                    use_i2 = value_boolean;
                }
                else if (token.str == "debug_actor")
                {
                    if (!GetChar(value, (int16_t *)&dialogue.cms_20))
                        return false;
                }
                else
                {
                    DPRINTF("%s: BUG, shouldn't be here (%s).\n", FUNCNAME, token.str.c_str());
                    return false;
                }
            }
        }
    }

    DPRINTF("Premature end of file (in InteractiveDialogue object)\n");
    return false;
}

bool Xv2QuestCompiler::CompileEventDialogue()
{
    QbtEntry dialogue;
    ci_string var_name;
    std::vector<bool> defined;
    uint16_t qbt_id;

    if (!ProcessVarDeclaration(var_name))
        return false;

    uint16_t suggested_id = 0xFFFF;
    if (var_name.length() > strlen("EventDialogue") && Utils::BeginsWith(var_name.str, "EventDialogue", false))
    {
        std::string num_str = var_name.str.substr(strlen("EventDialogue"));

        if (Utils::HasOnlyDigits(num_str))
            suggested_id = (uint16_t)Utils::GetUnsigned(num_str);
    }

    if (suggested_id < 0x8000)
    {
        qbt_id = suggested_id;
    }
    else
    {
        qbt_id = (uint16_t) active_qbt.GetTotalNumEntries();
    }

    if (!ExpectBracketBegin())
        return false;

    dialogue.unk_00 = 2;
    defined.resize(event_dialogue_params.size(), false);

    while (!tokens.empty())
    {
        X2QcToken token = tokens.front();
        tokens.pop();

        if (token.type == TOKEN_BRACKET_END)
        {
            for (size_t i = 0; i < defined.size(); i++)
            {
                if (!defined[i] && event_dialogue_params[i] != "stage")
                {
                    DPRINTF("Parameter \"%s\" was not defined in EventDialogue object ", event_dialogue_params[i].c_str());
                    return LineError(token);
                }
            }

            // Since Event Dialogue is not referenced by anything, we don't need to have them in any table

            active_qbt.GetSpecialEntries().push_back(dialogue);
            return true;
        }
        else
        {
            if (!ExpectTokenType(token, TOKEN_IDENTIFIER))
                return false;

            if (token.str == "DialoguePart")
            {
                QbtDialoguePart part;
                bool special_case;
                int out_of_order;                

                if (!CompileDialoguePart(part, &special_case, &out_of_order))
                    return false;

                part.qbt_id = qbt_id;

                if (out_of_order >= 0)
                {
                    part.sub_id = (uint16_t)out_of_order;
                }
                else
                {
                    part.sub_id = (uint16_t)dialogue.parts.size();
                }

                if (special_case)
                    part.qbt_id++;

                dialogue.parts.push_back(part);
            }
            else
            {
                X2QcToken value;

                if (!GetParam(token, value, event_dialogue_params, event_dialogue_types, defined))
                    return false;

                int value_int = value.num;
                bool value_boolean = value.b;
                ci_string &value_cistr = value.str;

                if (token.str == "devent")
                {
                    if (value.type == TOKEN_INTEGER)
                    {
                        dialogue.special_event = value_int;
                    }
                    else // Identifier
                    {
                        dialogue.special_event = ConstantToDialogueEvent(value_cistr);

                        if ((int)dialogue.special_event < 0)
                        {
                            DPRINTF("Unrecognized event constant \"%s\". ", value_cistr.c_str());
                            return LineError(value);
                        }
                    }
                }
                else if (token.str == "on_event_end")
                {
                    dialogue.special_on_event_end = value_boolean;
                }
                else if (token.str == "stage")
                {
                    if (!GetStage(value, (int16_t *)&dialogue.cms_20))
                        return false;
                }
                else if (token.str == "i12")
                {
                    dialogue.interaction_type = value_int;
                }
                else if (token.str == "i16")
                {
                    dialogue.interaction_param = value_int;
                }
                else if (token.str == "i34")
                {
                    dialogue.unk_22 = (int16_t)value_int;
                }
                else if (token.str == "i36")
                {
                    dialogue.unk_24 = value_int;
                }
                else if (token.str == "i44")
                {
                    dialogue.unk_2C = value_int;
                }
                else if (token.str == "i48")
                {
                    dialogue.unk_30 = value_int;
                }
                else
                {
                    DPRINTF("%s: BUG, shouldn't be here (%s).\n", FUNCNAME, token.str.c_str());
                    return false;
                }
            }
        }
    }

    DPRINTF("Premature end of file (in EventDialogue object)\n");
    return false;
}

bool Xv2QuestCompiler::CompileCharPosition()
{
    QslEntry position;
    ci_string var_name;
    std::vector<bool> defined;
    uint32_t stage = 0;

    if (!ProcessVarDeclaration(var_name))
        return false;

    if (!ExpectBracketBegin())
        return false;

    defined.resize(char_position_params.size(), false);

    while (!tokens.empty())
    {
        X2QcToken token = tokens.front();
        tokens.pop();

        if (token.type == TOKEN_BRACKET_END)
        {
            for (size_t i = 0; i < defined.size(); i++)
            {
                if (!defined[i] && char_position_params[i] != "i50")
                {
                    DPRINTF("Parameter \"%s\" was not defined in CharPosition object ", char_position_params[i].c_str());
                    return LineError(token);
                }
            }

            compiled_chars_positions[var_name] = position.position;
            active_qsl.AddEntryToStage(position, stage);
            return true;
        }
        else
        {
            X2QcToken value;

            if (!GetParam(token, value, char_position_params, char_position_types, defined))
                return false;

            std::string &value_str = value.str.str;
            int value_int = value.num;

            if (token.str == "char")
            {
                uint32_t qml_id;

                if (!GetQmlChar(value, &qml_id, true))
                    return false;

                position.qml_item_id = (int16_t)qml_id;
            }
            else if (token.str == "stage")
            {
                int16_t stage_id;

                if (!GetStage(value, &stage_id))
                    return false;

                stage = (int32_t)stage_id;
            }
            else if (token.str == "position")
            {
                position.position = value_str;

                if (position.position.length() >= QSL_POSITION_LENGTH)
                {
                    DPRINTF("Length of position \"%s\" is too high. ", value_str.c_str());
                    return LineError(value);
                }
            }
            else if (token.str == "type")
            {
                if (value_int != 2 && value_int != 3 && value_int != 5)
                {
                    DPRINTF("Only value of 2, 3 and 5 allowed for type. ");
                    return LineError(value);
                }

                position.type = (int16_t)value_int;
            }
            else if (token.str == "i40")
            {
                position.qml_change = value_int;
            }
            else if (token.str == "i50")
            {
                position.unk_32 = (int16_t)value_int;
            }
            else
            {
                DPRINTF("%s: BUG, shouldn't be here (%s).\n", FUNCNAME, token.str.c_str());
                return false;
            }
        }
    }

    DPRINTF("Premature end of file (in CharPosition object)\n");
    return false;
}

bool Xv2QuestCompiler::CompileInteractiveCharPosition()
{
    QslEntry position;
    ci_string var_name;
    std::vector<bool> defined;
    uint32_t stage = 0;

    if (!ProcessVarDeclaration(var_name))
        return false;

    if (!ExpectBracketBegin())
        return false;

    position.type = QSL_POSITION_ICHAR;
    defined.resize(ichar_position_params.size(), false);

    while (!tokens.empty())
    {
        X2QcToken token = tokens.front();
        tokens.pop();

        if (token.type == TOKEN_BRACKET_END)
        {
            for (size_t i = 0; i < defined.size(); i++)
            {
                if (!defined[i])
                {
                    if (ichar_position_params[i] == "change_to")
                    {
                        position.qml_change = position.qml_item_id;
                    }
                    else
                    {
                        DPRINTF("Parameter \"%s\" was not defined in InteractiveCharPosition object ", ichar_position_params[i].c_str());
                        return LineError(token);
                    }
                }
            }

            compiled_ichars_positions[var_name] = position.position;
            active_qsl.AddEntryToStage(position, stage);
            return true;
        }
        else
        {
            X2QcToken value;

            if (!GetParam(token, value, ichar_position_params, ichar_position_types, defined))
                return false;

            std::string &value_str = value.str.str;
            int value_int = value.num;

            if (token.str == "char")
            {
                uint32_t qml_id;

                if (!GetQmlChar(value, &qml_id, true))
                    return false;

                position.qml_item_id = (int16_t)qml_id;
            }
            else if (token.str == "stage")
            {
                int16_t stage_id;

                if (!GetStage(value, &stage_id))
                    return false;

                stage = (int32_t)stage_id;
            }
            else if (token.str == "position")
            {
                position.position = value_str;

                if (position.position.length() >= QSL_POSITION_LENGTH)
                {
                    DPRINTF("Length of position \"%s\" is too high. ", value_str.c_str());
                    return LineError(value);
                }
            }
            else if (token.str == "idialogue")
            {
                uint32_t idialogue;

                if (!GetInteractiveDialogue(value, &idialogue, true))
                    return false;

                position.chance_idialogue = (int16_t)idialogue;
            }
            else if (token.str == "change_to")
            {
                if (!GetQmlChar(value, &position.qml_change, false))
                    return false;
            }
            else if (token.str == "i38")
            {
                position.unk_26 = (int16_t)value_int;
            }
            else if (token.str == "default_pose")
            {
                position.default_pose = (int16_t)value_int;
            }
            else if (token.str == "talk_pose")
            {
                position.talking_pose = (int16_t)value_int;
            }
            else if (token.str == "effect_pose")
            {
                position.effect_pose = (int16_t)value_int;
            }
            else if (token.str == "i50")
            {
                position.unk_32 = (int16_t)value_int;
            }
            else
            {
                DPRINTF("%s: BUG, shouldn't be here (%s).\n", FUNCNAME, token.str.c_str());
                return false;
            }
        }
    }

    DPRINTF("Premature end of file (in InteractiveCharPosition object)\n");
    return false;
}

bool Xv2QuestCompiler::CompileItemPosition()
{
    QslEntry position;
    ci_string var_name;
    std::vector<bool> defined;
    uint32_t stage = 0;

    if (!ProcessVarDeclaration(var_name))
        return false;

    if (!ExpectBracketBegin())
        return false;

    position.type = QSL_POSITION_ITEM;
    defined.resize(item_position_params.size(), false);

    while (!tokens.empty())
    {
        X2QcToken token = tokens.front();
        tokens.pop();

        if (token.type == TOKEN_BRACKET_END)
        {
            for (size_t i = 0; i < defined.size(); i++)
            {
                if (!defined[i])
                {
                    DPRINTF("Parameter \"%s\" was not defined in ItemPosition object ", item_position_params[i].c_str());
                    return LineError(token);
                }
            }

            compiled_items_positions[var_name] = position.position;
            active_qsl.AddEntryToStage(position, stage);
            return true;
        }
        else
        {
            X2QcToken value;

            if (!GetParam(token, value, item_position_params, item_position_types, defined))
                return false;

            std::string &value_str = value.str.str;
            int value_int = value.num;

            if (token.str == "item_collection")
            {
                uint32_t col_id;

                if (!GetItemCollection(value, &col_id))
                    return false;

                position.qml_item_id = (int16_t)col_id;
            }
            else if (token.str == "stage")
            {
                int16_t stage_id;

                if (!GetStage(value, &stage_id))
                    return false;

                stage = (int32_t)stage_id;
            }
            else if (token.str == "position")
            {
                position.position = value_str;

                if (position.position.length() >= QSL_POSITION_LENGTH)
                {
                    DPRINTF("Length of position \"%s\" is too high. ", value_str.c_str());
                    return LineError(value);
                }
            }
            else if (token.str == "chance")
            {
                position.chance_idialogue = (int16_t)value_int;
            }
            else
            {
                DPRINTF("%s: BUG, shouldn't be here (%s).\n", FUNCNAME, token.str.c_str());
                return false;
            }
        }
    }

    DPRINTF("Premature end of file (in ItemPosition object)\n");
    return false;
}

bool Xv2QuestCompiler::CompileInstruction(const InstructionDef &def, QedInstructionParams &params, const X2QcToken &instruction_token)
{
    if (tokens.empty())
    {
        DPRINTF("Premature end of file (while compiling %s).\n", (def.is_action) ? "Action" : "Condition");
        return false;
    }

    X2QcToken group = tokens.front();
    std::vector<X2QcToken> param_tokens;

    if (group.type == TOKEN_GROUP)
    {
        tokens.pop();

        if (!GetGroupTokens(group, param_tokens))
            return false;
    }

    if (param_tokens.size() != def.param_types.size())
    {
        DPRINTF("Unexpected number of parameters: I was expecting %Id, but got %Id. ", def.param_types.size(), param_tokens.size());
        return LineError(instruction_token);
    }

    for (size_t p = 0, o = 0; p < param_tokens.size(); p++, o++)
    {
        int32_t *out_param_int = nullptr, *out_next_int = nullptr;
        float *out_param_float = nullptr;
        char *out_param_string = nullptr;

        X2QcToken &param = param_tokens[p];

        if (o == 0)
        {
            out_param_int = &params.group1.group.nums.param1.num.i;
            out_next_int = &params.group1.group.nums.param2.num.i;
            out_param_float = &params.group1.group.nums.param1.num.f;
            out_param_string = params.group1.group.string.str;
        }
        else if (o == 1)
        {
            out_param_int = &params.group1.group.nums.param2.num.i;
            out_next_int = &params.group1.group.nums.param3.num.i;
            out_param_float = &params.group1.group.nums.param2.num.f;
        }
        else if (o == 2)
        {
            out_param_int = &params.group1.group.nums.param3.num.i;
            out_next_int = &params.group1.group.nums.param4.num.i;
            out_param_float = &params.group1.group.nums.param3.num.f;
        }
        else if (o == 3)
        {
            out_param_int = &params.group1.group.nums.param4.num.i;
            out_next_int = &params.group1.group.nums.param5.num.i;
            out_param_float = &params.group1.group.nums.param4.num.f;
        }
        else if (o == 4)
        {
            out_param_int = &params.group1.group.nums.param5.num.i;
            out_next_int = &params.group2.param2.num.i;
            out_param_float = &params.group1.group.nums.param5.num.f;
        }
        else if (o == 5)
        {
            out_param_int = &params.group2.param2.num.i;
            out_next_int = &params.group2.param3.num.i;
            out_param_float = &params.group2.param2.num.f;
        }
        else if (o == 6)
        {
            out_param_int = &params.group2.param3.num.i;
            out_next_int = &params.group2.param4.num.i;
            out_param_float = &params.group2.param3.num.f;
        }
        else if (o == 7)
        {
            out_param_int = &params.group2.param4.num.i;
            out_param_float = &params.group2.param4.num.f;
        }

        if (def.param_types[p] == PARAM_INTEGER || def.param_types[p] == PARAM_INTEGER_HEX)
        {
            if (!ExpectTokenType(param, TOKEN_INTEGER))
                return false;

            *out_param_int = param.num;
        }
        else if (def.param_types[p] == PARAM_FLOAT)
        {
            if (!ExpectTokenType(param, TOKEN_FLOAT))
                return false;

            *out_param_float = param.fnum;
        }
        else if (def.param_types[p] == PARAM_BOOLEAN)
        {
            if (!ExpectTokenType(param, TOKEN_BOOLEAN))
                return false;

            *out_param_int = (param.b != false);
        }
        else if (def.param_types[p] == PARAM_REVERSE_BOOLEAN)
        {
            if (!ExpectTokenType(param, TOKEN_BOOLEAN))
                return false;

            *out_param_int = (param.b != true);
        }
        else if (def.param_types[p] == PARAM_STRING || def.param_types[p] == PARAM_STRING_LONG)
        {
            if (!ExpectTokenType(param, TOKEN_STRING))
                return false;

            size_t max_size = (def.param_types[p] == PARAM_STRING) ? 20 : 24;

            if (param.str.length() >= max_size)
            {
                DPRINTF("String parameter \"%s\" too long (limit %Id). ", param.str.c_str(), max_size-1);
                return LineError(param);
            }

            strcpy(out_param_string, param.str.c_str());

            if (def.param_types[p] == PARAM_STRING)
                o += 4;
            else
                o += 5;
        }
        else if (def.param_types[p] == PARAM_OPERATOR)
        {
            if (param.type != TOKEN_IDENTIFIER)
            {
                DPRINTF("Was expecting operator, but got \"%s\". ", param.str.c_str());
                return LineError(param);
            }

            if (param.str != ">=" && param.str != "<=")
            {
                if (param.str == ">" || param.str == "<" || param.str == "==" || param.str == "!=")
                {
                    DPRINTF("Only valid operators are >= and <=, but found %s. ", param.str.c_str());
                }
                else
                {
                    DPRINTF("Expected operator >= or <=, but found %s. ", param.str.c_str());
                }

                return LineError(param);
            }

            *out_param_int = (param.str == "<=");
        }
        else if (def.param_types[p] == PARAM_SCMS)
        {
            if (!ExpectTokenType(param, { TOKEN_STRING, TOKEN_IDENTIFIER, TOKEN_INTEGER }))
                return false;

            if (!GetChar(param, out_param_string))
                return false;
        }
        else if (def.param_types[p] == PARAM_SKILL)
        {
            if (!ExpectTokenType(param, { TOKEN_STRING, TOKEN_IDENTIFIER, TOKEN_INTEGER }))
                return false;

            int16_t skill_id;

            if (!GetSkill(param, &skill_id, (uint32_t *)out_param_int, true))
                return false;

            *out_next_int = (int32_t)skill_id;
            o++;
        }
        else if (def.param_types[p] == PARAM_STAGE)
        {
            if (!ExpectTokenType(param, { TOKEN_STRING, TOKEN_IDENTIFIER, TOKEN_INTEGER } ))
                return false;

            int16_t stage_id;

            if (!GetStage(param, &stage_id))
                return false;

            *out_param_int = stage_id;
        }
        else if (def.param_types[p] == PARAM_QUEST)
        {
            // TODO: x2m
            if (!ExpectTokenType(param, { TOKEN_STRING, TOKEN_INTEGER }))
                return false;

            int16_t quest_id;

            if (!GetQuest(param, &quest_id, true))
                return false;

            *out_param_int = quest_id;
        }
        else if (def.param_types[p] == PARAM_QUEST_WITH_TYPE)
        {
            // TODO: x2m
            if (!ExpectTokenType(param, { TOKEN_STRING, TOKEN_INTEGER }))
                return false;

            int16_t quest_id;

            if (!GetQuest(param, &quest_id, true))
                return false;

            *out_param_int = quest_type;
            *out_next_int = quest_id;
        }
        else if (def.param_types[p] == PARAM_QCHAR)
        {
            if (!ExpectTokenType(param, { TOKEN_IDENTIFIER, TOKEN_INTEGER } ))
                return false;

            if (!GetQmlChar(param, (uint32_t *)out_param_int, true))
                return false;
        }
        else if (def.param_types[p] == PARAM_QBT)
        {
            if (!ExpectTokenType(param, { TOKEN_IDENTIFIER, TOKEN_INTEGER } ))
                return false;

            if (!GetDialogue(param, (uint32_t *)out_param_int, true))
                return false;
        }
        else if (def.param_types[p] == PARAM_PARTNER)
        {
            if (!ExpectTokenType(param, { TOKEN_IDENTIFIER, TOKEN_INTEGER } ))
                return false;

            if (param.type == TOKEN_INTEGER)
            {
                *out_param_int = param.num;
            }
            else
            {
                auto it = constant_to_partner.find(param.str);
                if (it == constant_to_partner.end())
                {
                    DPRINTF("Unrecognized partner constant \"%s\". ", param.str.c_str());
                    return LineError(param);
                }

                *out_param_int = it->second;
            }
        }
        else if (def.param_types[p] == PARAM_FLAG)
        {
            if (!ExpectTokenType(param, TOKEN_IDENTIFIER))
                return false;

            if (!GetFlag(param, (uint32_t *)out_param_int))
                return false;
        }
        else if (def.param_types[p] == PARAM_STRINGVAR)
        {
            if (!ExpectTokenType(param, TOKEN_IDENTIFIER))
                return false;

            if (!GetStringVar(param, (uint32_t *)out_param_int))
                return false;
        }
        else if (def.param_types[p] == PARAM_RACE)
        {
            if (!ExpectTokenType(param, TOKEN_IDENTIFIER))
                return false;

            auto it = constant_to_race.find(param.str);
            if (it == constant_to_race.end())
            {
                DPRINTF("Unrecognized race constant \"%s\". ", param.str.c_str());
                return LineError(param);
            }

            *out_param_int = it->second;
        }
        else if (def.param_types[p] == PARAM_SEX)
        {
            if (!ExpectTokenType(param, TOKEN_IDENTIFIER))
                return false;

            auto it = constant_to_sex.find(param.str);
            if (it == constant_to_sex.end())
            {
                DPRINTF("Unrecognized sex constant \"%s\". ", param.str.c_str());
                return LineError(param);
            }

            *out_param_int = it->second;
        }
        else if (def.param_types[p] == PARAM_FADE)
        {
            if (!ExpectTokenType(param, TOKEN_IDENTIFIER))
                return false;

            auto it = constant_to_fade.find(param.str);
            if (it == constant_to_fade.end())
            {
                DPRINTF("Unrecognized fade constant \"%s\". ", param.str.c_str());
                return LineError(param);
            }

            *out_param_int = it->second;
        }
        else if (def.param_types[p] == PARAM_TIME_CTRL_MODE)
        {
            if (!ExpectTokenType(param, TOKEN_IDENTIFIER))
                return false;

            auto it = constant_to_time_ctrl.find(param.str);
            if (it == constant_to_time_ctrl.end())
            {
                DPRINTF("Unrecognized time control mode constant \"%s\". ", param.str.c_str());
                return LineError(param);
            }

            *out_param_int = it->second;
        }
        else if (def.param_types[p] == PARAM_TEAM_TYPE)
        {
            if (!ExpectTokenType(param, TOKEN_IDENTIFIER))
                return false;

            auto it = constant_to_team_type.find(param.str);
            if (it == constant_to_team_type.end())
            {
                DPRINTF("Unrecognized team type constant \"%s\". ", param.str.c_str());
                return LineError(param);
            }

            *out_param_int = it->second;
        }
        else if (def.param_types[p] == PARAM_TEAM_TYPE2)
        {
            if (!ExpectTokenType(param, TOKEN_IDENTIFIER))
                return false;

            auto it = constant_to_team_type2.find(param.str);
            if (it == constant_to_team_type2.end())
            {
                if (param.str == "PLAYER_TEAM2")
                {
                    DPRINTF("PLAYER_TEAM2 can't be used here, only PLAYER_TEAM and ENEMY_TEAM. ");
                }
                else
                {
                    DPRINTF("Unrecognized team type constant \"%s\". ", param.str.c_str());
                }

                return LineError(param);
            }

            *out_param_int = it->second;
        }
        else if (def.param_types[p] == PARAM_STATS)
        {
            if (!ExpectTokenType(param, TOKEN_IDENTIFIER))
                return false;

            auto it = constant_to_stats.find(param.str);
            if (it == constant_to_stats.end())
            {
                DPRINTF("Unrecognized stats constant \"%s\". ", param.str.c_str());
                return LineError(param);
            }

            *out_param_int = it->second;
        }
        else if (def.param_types[p] == PARAM_SKILL_SLOT)
        {
            if (!ExpectTokenType(param, TOKEN_IDENTIFIER))
                return false;

            auto it = constant_to_skill_slot.find(param.str);
            if (it == constant_to_skill_slot.end())
            {
                DPRINTF("Unrecognized skill slot constant \"%s\". ", param.str.c_str());
                return LineError(param);
            }

            *out_param_int = it->second;
        }
        else if (def.param_types[p] == PARAM_QUEST_FINISH_STATE)
        {
            if (!ExpectTokenType(param, TOKEN_IDENTIFIER))
                return false;

            auto it = constant_to_quest_fs.find(param.str);
            if (it == constant_to_quest_fs.end())
            {
                DPRINTF("Unrecognized quest finish state constant \"%s\". ", param.str.c_str());
                return LineError(param);
            }

            *out_param_int = it->second;
        }
        else if (def.param_types[p] == PARAM_BATTLE_MSG)
        {
            if (!ExpectTokenType(param, { TOKEN_STRING, TOKEN_IDENTIFIER } ))
                return false;

            std::string str;

            if (!GetDialogueTA(param, str))
                return false;

            if (str.length() >= 20)
            {
                DPRINTF("\"%s\" is too long. ", str.c_str());
                return LineError(param);
            }

            strcpy(out_param_string, str.c_str());
            o += 4;
        }
        else if (def.param_types[p] == PARAM_SKILL_COMBINATION)
        {
            std::vector<std::string> values;
            Utils::GetMultipleStrings(param.str.str, values, '|', false);

            int32_t value = 0;

            for (const std::string &str : values)
            {
                ci_string val;
                val.str = str;
                Utils::TrimString(val.str);

                if (val == "ALL")
                {
                    value = 1;
                    break;
                }
                else
                {
                   auto it = constant_to_skill_slot.find(val);
                   if (it == constant_to_skill_slot.end())
                   {
                       DPRINTF("Unrecognized skill slot \"%s\". ", val.str.c_str());
                       return LineError(param);
                   }

                   value |= (1 << (it->second+1));
                }
            }

            *out_param_int = value;
        }
        else if (def.param_types[p] == PARAM_STATS_COMBINATION)
        {
            std::vector<std::string> values;
            Utils::GetMultipleStrings(param.str.str, values, '|', false);

            int32_t value = 0;

            for (const std::string &str : values)
            {
                ci_string val;
                val.str = str;
                Utils::TrimString(val.str);

                auto it = constant_to_stats2.find(val);
                if (it == constant_to_stats2.end())
                {
                    DPRINTF("Unrecognized stat constant \"%s\". ", val.str.c_str());
                    return LineError(param);
                }               

                value |= (1 << it->second);
            }

            *out_param_int = value;
        }
        else
        {

            DPRINTF("%s: BUG, shouldn't be here.\n", FUNCNAME);
            return false;
        }
    }

    return true;
}

bool Xv2QuestCompiler::CompileEvent(QedEvent &event)
{
    event = QedEvent();

    if (!ExpectBracketBegin())
        return false;

    while (!tokens.empty())
    {
        X2QcToken token = tokens.front();
        tokens.pop();

        if (token.type == TOKEN_BRACKET_END)
        {
            // This requirement deleted because there are actually vanilla quests like this.
            /*if (event.conditions.size() == 0 && event.actions.size() > 0)
            {
                DPRINTF("An event that has one or more actions must have at least one condition. ");
                return LineError(token);
            }*/

            return true;
        }
        else
        {
            if (!ExpectTokenType(token, TOKEN_IDENTIFIER))
                return false;

            if (token.str != "Condition" && token.str != "Action")
            {
                if (all_types.find(token.str) == all_types.end())
                {
                    DPRINTF("Unknown type \"%s\". ", token.str.c_str());
                }
                else
                {
                    DPRINTF("\"%s\" cannot be defined here. ", token.str.c_str());
                }

                return LineError(token);
            }

            if (tokens.empty())
                break;

            X2QcToken ins_token = tokens.front();
            tokens.pop();

            if (!ExpectTokenType(ins_token, TOKEN_IDENTIFIER))
                return false;

            if (token.str == "Condition")
            {
                if (event.actions.size() > 0)
                {
                    DPRINTF("Condition must always be before all Action. ");
                    return LineError(token);
                }

                bool found = false;

                for (const InstructionDef &def : instructions_defs)
                {
                    if (!def.is_action && def.name == ins_token.str)
                    {
                        found = true;

                        QedInstruction cond;
                        cond.opcode = def.opcode;

                        if (!CompileInstruction(def, cond.params, ins_token))
                            return false;

                        event.conditions.push_back(cond);
                    }
                }

                if (!found)
                {
                    DPRINTF("Unknown condition \"%s\". ", ins_token.str.c_str());
                    return LineError(ins_token);
                }
            }
            else
            {
                bool found = false;

                for (const InstructionDef &def : instructions_defs)
                {
                    if (def.is_action && def.name == ins_token.str)
                    {
                        found = true;

                        QedInstruction act;
                        act.opcode = def.opcode;

                        if (!CompileInstruction(def, act.params, ins_token))
                            return false;                        

                        event.actions.push_back(act);
                    }
                }

                if (!found)
                {
                    DPRINTF("Unknown action \"%s\". ", ins_token.str.c_str());
                    return LineError(ins_token);
                }
            }
        }
    }

    DPRINTF("Premature end of file (in Event object)\n");
    return false;
}

bool Xv2QuestCompiler::CompileState(QedState &state)
{
    state = QedState();

    if (!ExpectBracketBegin())
        return false;

    while (!tokens.empty())
    {
        X2QcToken token = tokens.front();
        tokens.pop();

        if (token.type == TOKEN_BRACKET_END)
        {
            return true;
        }
        else
        {
            if (!ExpectTokenType(token, TOKEN_IDENTIFIER))
                return false;

            if (token.str != "Event")
            {
                if (all_types.find(token.str) == all_types.end())
                {
                    DPRINTF("Unknown type \"%s\". ", token.str.c_str());
                }
                else
                {
                    DPRINTF("\"%s\" cannot be defined here. ", token.str.c_str());
                }

                return LineError(token);
            }

            if (tokens.empty())
                break;

            X2QcToken event_num = tokens.front();
            tokens.pop();

            if (!ExpectTokenType(event_num, TOKEN_INTEGER))
                return false;

            if (state.events.find(event_num.num) != state.events.end())
            {
                DPRINTF("Event %s had already been defined for this state. ", event_num.str.c_str());
                return LineError(event_num);
            }

            QedEvent event;

            if (!CompileEvent(event))
                return false;

            state.events[event_num.num] = event;
        }
    }

    DPRINTF("Premature end of file (in State object)\n");
    return false;
}

bool Xv2QuestCompiler::CompileScript(QedFile &script)
{
    std::vector<bool> defined;

    if (!ExpectBracketBegin())
        return false;

    while (!tokens.empty())
    {
        X2QcToken token = tokens.front();
        tokens.pop();

        if (token.type == TOKEN_BRACKET_END)
        {
            return true;
        }
        else
        {
            if (!ExpectTokenType(token, TOKEN_IDENTIFIER))
                return false;

            if (token.str != "State")
            {
                if (all_types.find(token.str) == all_types.end())
                {
                    DPRINTF("Unknown type \"%s\". ", token.str.c_str());
                }
                else
                {
                    DPRINTF("\"%s\" cannot be defined here. ", token.str.c_str());
                }

                return LineError(token);
            }

            if (tokens.empty())
                break;

            X2QcToken state_num = tokens.front();
            tokens.pop();

            if (!ExpectTokenType(state_num, TOKEN_INTEGER))
                return false;

            if (state_num.num < 0)
            {
                DPRINTF("State number cannot be a negative number. ");
                return LineError(state_num);
            }

            std::vector<QedState> &states = script.GetStates();

            if (state_num.num < (int)defined.size())
            {
                if (defined[state_num.num])
                {
                    DPRINTF("State %s had already been defined. ", state_num.str.c_str());
                    return LineError(state_num);
                }
            }
            else
            {
                states.resize(state_num.num+1);
                defined.resize(state_num.num+1, false);
            }

            QedState state;

            if (!CompileState(state))
                return false;

            states[state_num.num] = state;
            defined[state_num.num] = true;
        }
    }

    DPRINTF("Premature end of file (in Script object)\n");
    return false;
}

bool Xv2QuestCompiler::CompileFlag()
{
    ci_string var_name;
    int flag=-1;

    if (!ProcessVarDeclaration(var_name))
        return false;

    if (Utils::BeginsWith(var_name.str, "Flag", false) && var_name.length() > strlen("Flag"))
    {
        std::string num = var_name.str.substr(strlen("Flag"));

        if (Utils::HasOnlyDigits(num))
        {
            int desired_flag = Utils::GetUnsigned(num);

            if (desired_flag >= 0 && desired_flag < QED_MAX_NUM_FLAGS && !used_flags_slots[desired_flag])
            {
                flag = desired_flag;
            }
        }
    }

    if (flag < 0)
    {
        for (size_t i = 0; i < QED_MAX_NUM_FLAGS; i++)
        {
            if (!used_flags_slots[i])
            {
                flag = (int)i;
                break;
            }
        }

        if (flag < 0)
        {
            DPRINTF("Too many Flags variables declared. Only a max of %d is allowed.\n", QED_MAX_NUM_FLAGS);
            return false;
        }
    }

    used_flags_slots[flag] = true;
    compiled_flags[var_name] = flag;

    return true;
}

bool Xv2QuestCompiler::CompileStringVar()
{
    ci_string var_name;
    int sv=-1;

    if (!ProcessVarDeclaration(var_name))
        return false;

    if (Utils::BeginsWith(var_name.str, "String", false) && var_name.length() > strlen("String"))
    {
        std::string num = var_name.str.substr(strlen("String"));

        if (Utils::HasOnlyDigits(num))
        {
            int desired_ps = Utils::GetUnsigned(num);

            if (desired_ps >= 0 && desired_ps < QED_MAX_NUM_STRING_VARS && !used_string_vars[desired_ps])
            {
                sv = desired_ps;
            }
        }
    }

    if (sv < 0)
    {
        for (size_t i = 0; i < QED_MAX_NUM_STRING_VARS; i++)
        {
            if (!used_string_vars[i])
            {
                sv = (int)i;
                break;
            }
        }

        if (sv < 0)
        {
            DPRINTF("Too many StringVar variables declared. Only a max of %d is allowed.\n", QED_MAX_NUM_STRING_VARS);
            return false;
        }
    }

    used_string_vars[(size_t)sv] = true;
    compiled_string_vars[var_name] = sv;

    return true;
}

bool Xv2QuestCompiler::DecompileQuest(const std::string &quest_name, std::ostringstream &qoss, std::ostringstream &coss, std::ostringstream &doss, std::ostringstream &poss, std::vector<std::string> &svec)
{
    Reset();
    svec.clear();

    quest_type = GetQuestType(quest_name);
    if (quest_type < 0)
    {
        DPRINTF("%s: Unknown quest type for \"%s\"\n", FUNCNAME, quest_name.c_str());
        return false;
    }

    if (!LoadActiveQxd())
    {
        DPRINTF("%s: Failed to load active qxd (quest_type=%d)\n", FUNCNAME, quest_type);
        return false;
    }

    QxdQuest *quest = active_qxd.FindQuestByName(quest_name);
    if (!quest)
    {
        DPRINTF("%s: Cannot find quest \"%s\"\n", FUNCNAME, quest_name.c_str());
        return false;
    }

    if (!LoadTitle())
        return false;

    if (!DecompileQuestStruct(*quest, qoss))
        return false;

    if (!LoadActiveQml(quest_name))
    {
        DPRINTF("Failed to load qml.\n");
        return false;
    }

    if (!FindReferencedQxdChars())
        return false;

    for (auto &it : referenced_special_chars)
    {
        QxdCharacter *chara = active_qxd.FindSpecialCharById(it.first); // No need to check for null, already done previously

        if (!DecompileQxdChar(*chara, true, coss))
            return false;
    }

    for (auto &it : referenced_chars)
    {
        QxdCharacter *chara = active_qxd.FindCharById(it.first); // No need to check for null, already done previously

        if (!DecompileQxdChar(*chara, false, coss))
            return false;
    }

    for (const QmlEntry &qchar : active_qml)
    {
        if (!DecompileQmlChar(qchar, coss))
            return false;
    }

    if (!LoadActiveQbt(quest_name))
    {
        DPRINTF("Failed to load qbt.\n");
        return false;
    }

    if (!LoadDialogue(quest_name, quest->episode))
    {
        DPRINTF("%s: Failed to load dialogue file for \"%s\" (episode: %d).\n", FUNCNAME, quest->name.c_str(), quest->episode);
        return false;
    }

    const std::vector<QbtEntry> &qbt_entries = active_qbt.GetNormalEntries();
    const std::vector<QbtEntry> &qbt_ientries = active_qbt.GetInteractiveEntries();
    const std::vector<QbtEntry> &qbt_eentries = active_qbt.GetSpecialEntries();

    for (uint32_t i = 0; i < (uint32_t)qbt_entries.size(); i++)
    {
        if (!DecompileDialogue(qbt_entries[i], i, doss))
            return false;
    }

    for (uint32_t i = 0; i < (uint32_t)qbt_ientries.size(); i++)
    {
        if (!DecompileInteractiveDialogue(qbt_ientries[i], i, doss))
            return false;
    }

    for (uint32_t i = 0; i < (uint32_t)qbt_eentries.size(); i++)
    {
        if (!DecompileEventDialogue(qbt_eentries[i], i, doss))
            return false;
    }

    if (!LoadActiveQsl(quest->name))
        return false;

    for (const QslStage &stage : active_qsl)
    {
        for (const QslEntry &entry : stage.entries)
        {
            if (entry.type == QSL_POSITION_CHAR2 || entry.type == QSL_POSITION_CHAR3 || entry.type == QSL_POSITION_CHAR5)
            {
                if (!DecompileCharPosition(entry, stage.stage_id, poss))
                    return false;
            }
            else if (entry.type == QSL_POSITION_ICHAR)
            {
                if (!DecompileInteractiveCharPosition(entry, stage.stage_id, poss))
                    return false;
            }
            else if (entry.type == QSL_POSITION_ITEM)
            {
                if (!DecompileItemPosition(entry, stage.stage_id, poss))
                    return false;
            }
            else
            {
                DPRINTF("%s: Unrecognized qsl entry type %d.\n", FUNCNAME, entry.type);
                return false;
            }
        }
    }

    svec.resize(quest->scripts.size());

    for (size_t i = 0; i < quest->scripts.size(); i++)
    {
        std::ostringstream soss;

        if (!LoadActiveQed(quest->name, i))
        {
            DPRINTF("%s: Failed to load qed file, index %Id. Num scripts = %Id\n", FUNCNAME, i, quest->scripts.size());
            return false;
        }       

        if (!DecompileScript(*scripts[i], soss))
            return false;

        svec[i] = soss.str();
    }

    if (referenced_string_vars.size() > 0)
    {
        std::ostringstream foss;

        for (auto &it : referenced_string_vars)
        {
            WriteIndent(foss, true);
            foss << "StringVar " << it.second.str << '\n';
        }
        foss << '\n';

        svec[0] = foss.str() + svec[0]; // No need to check for svec size, if a stringvar was referenced, at least one script exists
    }

    if (referenced_flags.size() > 0)
    {
        std::ostringstream foss;

        for (auto &it : referenced_flags)
        {
            WriteIndent(foss, true);
            foss << "Flag " << it.second.str << '\n';
        }
        foss << '\n';

        svec[0] = foss.str() + svec[0]; // No need to check for svec size, if a flag was referenced, at least one script exists
    }

    if (referenced_collections.size() > 0)
    {
        std::ostringstream closs;

        for (auto &it: referenced_collections)
        {
            if (!DecompileItemCollection(it.first, closs))
                return false;
        }

        closs << qoss.str();
        qoss.str("");
        qoss.clear();

        qoss << closs.str();
    }

    if (referenced_title.size() > 0)
    {
        std::ostringstream toss;

        for (auto &it : referenced_title)
        {
            if (!DecompileTextEntry(it.first, title, false, toss, true))
                return false;
        }

        toss << qoss.str();
        qoss.str("");
        qoss.clear();

        qoss << toss.str();
    }

    if (linked_mods.size() > 0)
    {
        // Linked mods, prepend them in qoss
        std::ostringstream modss;

        for (const XQ_X2mMod &mod : linked_mods)
        {
            if (!DecompileX2mMod(mod, modss))
                return false;
        }

        modss << qoss.str();
        qoss.str("");
        qoss.clear();

        qoss << modss.str();
    }

    if (referenced_dialogue_ta.size() > 0)
    {
        std::ostringstream taoss;

        for (auto &it : referenced_dialogue_ta)
        {
            if (!DecompileTextEntry(it.first, dialogue, true, taoss, false))
                return false;
        }

        taoss << doss.str();
        doss.str("");
        doss.clear();

        doss << taoss.str();
    }

    return true;
}

bool Xv2QuestCompiler::CompileQuest(const std::string &qstr, const std::string &qfile, const std::string &cstr, const std::string &cfile, const std::string &dstr, const std::string &dfile, const std::string &pstr, const std::string &pfile, const std::vector<std::string> &svec, const std::vector<std::string> &sfvec)
{
    Reset();
    Decompose(qstr, qfile);
    Decompose(cstr, cfile);
    Decompose(dstr, dfile);
    Decompose(pstr, pfile);

    for (size_t i = 0; i < svec.size(); i++)
    {
        Decompose(svec[i], sfvec[i]);
    }

    while (!tokens.empty())
    {
        X2QcToken token = tokens.front();
        tokens.pop();

        //DPRINTF("Processing token %s\n", token.str.c_str());

        if (!ExpectTokenType(token, TOKEN_IDENTIFIER))
            return false;

        if (!IsTopType(token.str))
        {
            if (IsKnownType(token.str))
            {
                DPRINTF("%s cannot be defined at top level. ", token.str.c_str());
                return LineError(token);
            }
            else
            {
                DPRINTF("Unrecognized type \"%s\". ", token.str.c_str());
                return LineError(token);
            }
        }

        if (token.str == "X2mMod")
        {
            if (!CompileX2mMod())
                return false;
        }
        else if (token.str == "TextEntry" || token.str == "TextAudioEntry")
        {
            if (!CompileTextEntry(token.str == "TextAudioEntry"))
                return false;
        }
        else if (token.str == "ItemCollection")
        {
            if (!CompileItemCollection())
                return false;
        }
        else if (token.str == "Quest")
        {
            if (!CompileQuestStruct())
                return false;

            QxdQuest *existing_quest = active_qxd.FindQuestByName(compiled_quest.name);
            if (existing_quest)
            {
                compiled_quest.id = existing_quest->id;
                if (self_reference_parent)
                {
                    compiled_quest.parent_quest = compiled_quest.id;
                }

                if (self_reference_unlock)
                {
                    compiled_quest.unlock_requirement = compiled_quest.id;
                }

                *existing_quest = compiled_quest;
            }
            else
            {
                //DPRINTF("adding quest %s\n", compiled_quest.name.c_str());
                active_qxd.AddQuest(compiled_quest, start_new_quest_id_search[quest_type]);

                if (quest_type == QUEST_TYPE_TMQ && compiled_quest.id >= MAX_PQ)
                {
                    DPRINTF("Overflow, only %d parallel quests allowed on the system.\n", MAX_PQ);
                    return false;
                }
                else if (quest_type == QUEST_TYPE_HLQ && compiled_quest.id >= MAX_EQ)
                {
                    DPRINTF("Overflow, only %d expert quests allowed on the system.\n", MAX_EQ);
                    return false;
                }

                if (self_reference_parent)
                {
                    active_qxd.FindQuestByName(compiled_quest.name)->parent_quest = compiled_quest.id;
                }

                if (self_reference_unlock)
                {
                    active_qxd.FindQuestByName(compiled_quest.name)->unlock_requirement = compiled_quest.id;
                }

                if (!LoadQsf())
                {
                    DPRINTF("%s: Failed to load qsf file.\n", FUNCNAME);
                    return false;
                }

                if (!qsf.AddQuest(compiled_quest.name))
                {
                    DPRINTF("%s: Failed to add quest to the qsf file.\n", FUNCNAME);
                    return false;
                }
            }
        }
        else if (token.str == "QxdChar" || token.str == "QxdSpecialChar")
        {
            if (compiled_quest.name.length() == 0)
            {
                DPRINTF("QxdChar/QxdSpecialChar cannot be defined before Quest! ");
                return LineError(token);
            }

            if (!CompileQxdChar((token.str == "QxdSpecialChar")))
                return false;
        }
        else if (token.str == "QmlChar")
        {
            if (!CompileQmlChar())
                return false;
        }
        else if (token.str == "Dialogue")
        {
            if (compiled_quest.name.length() == 0)
            {
                DPRINTF("Dialogue cannot be defined before Quest! ");
                return LineError(token);
            }

            if (!CompileDialogue())
                return false;
        }
        else if (token.str == "InteractiveDialogue")
        {
            if (compiled_quest.name.length() == 0)
            {
                DPRINTF("InteractiveDialogue cannot be defined before Quest! ");
                return LineError(token);
            }

            if (!CompileInteractiveDialogue())
                return false;
        }
        else if (token.str == "EventDialogue")
        {
            if (compiled_quest.name.length() == 0)
            {
                DPRINTF("EventDialogue cannot be defined before Quest! ");
                return LineError(token);
            }

            if (!CompileEventDialogue())
                return false;
        }
        else if (token.str == "CharPosition")
        {
            if (compiled_quest.name.length() == 0)
            {
                DPRINTF("CharPosition cannot be defined before Quest! ");
                return LineError(token);
            }

            if (!CompileCharPosition())
                return false;
        }
        else if (token.str == "InteractiveCharPosition")
        {
            if (compiled_quest.name.length() == 0)
            {
                DPRINTF("InteractiveCharPosition cannot be defined before Quest! ");
                return LineError(token);
            }

            if (!CompileInteractiveCharPosition())
                return false;
        }
        else if (token.str == "ItemPosition")
        {
            if (compiled_quest.name.length() == 0)
            {
                DPRINTF("ItemPosition cannot be defined before Quest! ");
                return LineError(token);
            }

            if (!CompileItemPosition())
                return false;
        }        
        else if (token.str == "Script")
        {
            if (compiled_quest.name.length() == 0)
            {
                DPRINTF("Script cannot be defined before Quest! ");
                return LineError(token);
            }

            if (next_script >= 6)
            {
                DPRINTF("There is a limit of max of 6 scripts. ");
                return LineError(token);
            }

            if (!CompileScript(*scripts[next_script]))
                return false;

            QxdQuest *my_quest = active_qxd.FindQuestByName(compiled_quest.name); // Cannot be null at this point
            std::string script_name = compiled_quest.name;

            if (next_script != 0)
            {
                script_name.push_back('_');
                script_name += Utils::ToString(next_script);
            }

            my_quest->scripts.push_back(script_name);
            compiled_quest.scripts.push_back(script_name);
            next_script++;
        }
        else if (token.str == "Flag")
        {
            if (!CompileFlag())
                return false;
        }
        else if (token.str == "StringVar")
        {
            if (!CompileStringVar())
                return false;
        }
        else
        {
            DPRINTF("%s: BUG: Correct (%s)\n", FUNCNAME, token.str.c_str());
            return false;
        }
    }    

    if (compiled_quest.name.length() == 0)
    {
        DPRINTF("A quest object was not defined!\n");
        return false;
    }

    return true;
}

bool Xv2QuestCompiler::IsVanilla() const
{
    QxdFile qxd;

    if (!LoadQxd(qxd, true))
    {
        DPRINTF("%s: Internal error, cannot load vanilla qxd from cpk.\nProgram will close.\n", FUNCNAME);
        exit(-1);
    }

    return (qxd.FindQuestByName(compiled_quest.name) != nullptr);
}

bool Xv2QuestCompiler::Maintenance()
{
    if (!MaintenanceQxd())
        return false;

    if (!MaintenanceTitle())
        return false;

    if (!MaintenanceDialogue())
        return false;

    if (!MaintenanceAudio())
        return false;

    return true;
}


