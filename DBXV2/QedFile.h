#ifndef QEDFILE_H
#define QEDFILE_H

#include <map>
#include "BaseFile.h"

// "#QED"
#define QED_SIGNATURE  0x44455123

#define QED_MAX_NUM_FLAGS           64
#define QED_MAX_NUM_STRING_VARS  10

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED QEDHeader
{
    uint32_t signature;
    uint16_t endianess_check; // 4
    uint16_t pad; // 6 - Always 0
    uint32_t unk_08; // Always 1  (could be number of scripts, but since it is always to 1...)
    uint32_t unk_0C; // Always 0x10  (could be start of scripts header)
    uint32_t unk_10; // Always 0
    uint32_t num_conditions; // 0x14
    uint32_t conditions_start; // 0x18
    uint32_t num_actions; // 0x1C
    uint32_t actions_start; // 0x20
    uint32_t unk_24; // Always 0
};
CHECK_STRUCT_SIZE(QEDHeader, 0x28);

struct PACKED QEDInstruction
{
    uint16_t opcode; // 0 They are different for conditions/actions
    int16_t event_index; // 2 Event index within the "State"  Notice this is signed
    uint16_t state_index; // 4
    uint16_t pad; // 6
    uint32_t params_offset; // 8
};
CHECK_STRUCT_SIZE(QEDInstruction, 0xC);

struct PACKED QEDNumberPrimitive
{
    union
    {
        int32_t i;
        float f;
    } num;
};
CHECK_STRUCT_SIZE(QEDNumberPrimitive, 4);

struct PACKED QEDStringPrimitive
{
    char str[20];
};
CHECK_STRUCT_SIZE(QEDStringPrimitive, 0x14);

struct PACKED QEDParameterGroupNumber1
{
    QEDNumberPrimitive param1;
    QEDNumberPrimitive param2;
    QEDNumberPrimitive param3;
    QEDNumberPrimitive param4;
    QEDNumberPrimitive param5;
};
CHECK_STRUCT_SIZE(QEDParameterGroupNumber1, 0x14);

struct PACKED QEDParameterGroup2
{
    QEDNumberPrimitive param2;
    QEDNumberPrimitive param3;
    QEDNumberPrimitive param4;
};
CHECK_STRUCT_SIZE(QEDParameterGroup2, 0xC);

struct PACKED QEDParameterGroup1
{
    union
    {
        QEDParameterGroupNumber1 nums;
        QEDStringPrimitive string;
    } group;
};
CHECK_STRUCT_SIZE(QEDParameterGroup1, 0x14);

struct PACKED QEDInstructionParams
{
    QEDParameterGroup1 group1;
    QEDParameterGroup2 group2;
};
CHECK_STRUCT_SIZE(QEDInstructionParams, 0x20);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

// Range 0-18
enum QedStatsType
{
    QED_STAT_LEVEL = 0, // "LV"
    QED_STAT_HP,
    QED_STAT_KI, // "KIRYOKU"
    QED_STAT_STAMINA, // "GIRYOKU"
    QED_STAT_ATK,
    QED_STAT_KI_ATK, // 5  "SHOT"
    QED_STAT_SUPER_ATK, // "SP_ATK"
    QED_STAT_SUPER_KI, // "SP_SHOT_ATK"
    QED_STAT_ATK_DAMAGE,
    QED_STAT_KI_DAMAGE, // "SHOT_DAMAGE"
    QED_STAT_SUPER_ATK_DAMAGE, // 10 "SP_ATK_DAMAGE"
    QED_STAT_SUPER_KI_DAMAGE, // "SP_SHOT_DAMAGE"
    QED_STAT_MAX_HP,    
    QED_STAT_MAX_KI, // "MAX KIRYOKU"
    QED_STAT_MAX_STAMINA, // "MAX GIRYOKU"
    QED_STAT_GUARD_ATTACK, // 15
    QED_STAT_GUARD_DAMAGE,
    QED_STAT_MOVE_SPEED,
    QED_STAT_BOOST_SPEED,

    NUM_QED_STATS
};
static_assert(NUM_QED_STATS == 19, "Correct the enumeration!!!");

enum QedFadeMode
{
    QED_FADE_WHITE = 0,
    QED_FADE_BLACK = 1,
    QED_FADE_NONBG = 2,
};

// There are 60 conditions according to the exe (function 0x6ACD10 in 1.09.01)
// We shall put here those that are used in at least one qed, and the alias opcodes

// Conditions currently unused by existing quests in game: 13 (alias for 10), 14, 15, 18, 23, 29 (alias for slways), 31 (alias for always),
// 39 (alias for always), 41 (related with dialogue), 52

// Operators: <= -> true (1), >= -> false (0)

enum QedConditions
{
    QED_COND_NEVER = 0, // Params: none
    QED_COND_ALWAYS = 1, // Params: none
    QED_COND_UNK_2 = 2, // Param 1: stage_id  (Only used in TPQ_06_01, and with a -1...)
    QED_COND_KO = 3, // Param 1: qml_char, Param 2: b2, Param 3: i3 (only -1 -mostly- and 0 found in vanilla files)
    QED_COND_TIME_PASSED_LOCAL = 4, // Returns true if the time passed since this State started is greater than specified amount Param 1: seconds (float)
    QED_COND_IS_SCENE_INIT_COMPLETED = 5, // Params: none
    QED_COND_DEMO_PLAY_ENDED = 6, // (related with demo) Params: none
    QED_COND_FADE_COMPLETED = 7, // Param 1: true (fade in), false (fade out)
    QED_COND_TIME_PASSED = 8, // Param 1: <= / >=, Param 2: seconds (float)
    QED_COND_HEALTH = 9, // Param 1: <= / >= (boolean), Param 2: qml_char, Param 3: percent (float)
    QED_COND_UNK_10 = 10, // Param 1: cms_code (note: code is string one, but implemented in 4-bytes.. Only used by TMQ_0300) Param 2: i2 Param 3: i3 Param 4: i4
    QED_COND_IN_STAGE = 11, // Param 1: qml_char, Param 2: stage_id, Param 3: b3  (an extra unknown check is done if b3 is true)
    QED_COND_RANDOM_INT = 12, //  Evaluates to true if a random int (0-99) is smaller than value specified. A specified value of 100 will always return true. Param 1: value (range 0-100)
    QED_COND_UNK_13 = 13, // This function is an alias of 10
    QED_COND_PLAYER_HEALTH = 16, // Param 1: <= / >=, Param 2: percent
    QED_COND_TEAM_HEALTH = 17, // Param 1: <= / >=, Param 2: team_type (integer, range 1-3), Param 3: percent (float)
    QED_COND_CLEAR_SCREEN_CLOSED = 19, // Params: none
    QED_COND_CHECK_FLAG = 20, // Param 1: flag_id (range 0-63), Param 2: value"
    QED_COND_PLAYER_RACE = 21, // Param 1: race (0: human, 1 : saiyan, 2 : Namekian, 3: Freezer, 4: Majin), Param 2: sex (0: male, 1 : female, 2: any)
    QED_COND_NUM_HUMAN_PLAYERS = 22, // Param 1: <= / >= (boolean), Param 2: num
    QED_COND_USES_SKILL = 24, // Param 1: qml_char, Param 2: skill type, Param 3: skill_id
    QED_COND_HIT_WITH_SKILL = 25, // Param 1: qml_char (the one that was hit), Param 2: skill type, Param 3: skill_id
    QED_COND_KO_WITH_SKILL = 26, // (char can only be defeated by the skill, condition will return true when it happens) Param 1: qml_char, Param 2: skill_type (0 super, 1 ultimate, 2 evasive, 3 blast, 4 awaken), Param 3: skill (id2)
    QED_COND_DIALOGUE_FINISH = 27, // Param 1: qbt_dialogue (normal)
    QED_COND_NUM_CHARS_DEFEATED = 28, // (Numbers of enemies defeated, by player team i guess) Param 1: num (num enemies >= than this will evaluate to true), Param 2: mode (valid values: 0, 1 and 2), Param 3: stage_id
    QED_COND_ALWAYS_ = 29, // Alias for always
    QED_COND_IS_FIGHTING = 30, // Param 1: qml char
    QED_COND_ALWAYS__ = 31, // ALias for always
    QED_COND_IS_ALIVE = 32, // Param 1: qml_char
    QED_COND_REMOVED = 33, // Param 1: qml_char
    QED_COND_BEV_PLAY_COMPLETED = 34, // Params: none
    QED_COND_FIGHT_STARTED_WITH_CHAR = 35, // Param 1: qml_char
    QED_COND_OBJECTS_PICKED = 36, // Param 1: i1, Param 2: i2, Param 3: num of objects
    QED_COND_CARRIES_DRAGON_BALLS = 37, // Param 1: qml_char, Param 2: i2 (seems unused by exe)
    QED_COND_DRAGON_BALLS_SENT = 38, // Param 1: num_balls, Param 2: i2 (seem sunused by exe)
    QED_COND_ALWAYS___ = 39, // Alias for always
    QED_COND_IS_BEV_LOAD_COMPLETED = 40, // Params: none
    QED_COND_EVENT_DONE = 42, // Param 1: i1 (observed values of 0 and 1 only, could be script index, but it is not a boolean), Param 2: State index, Param 3: Event index
    QED_COND_UNK_43 = 43, // (Only used by TMQ_0506) Param 1: mob_index (range 0-13)
    QED_COND_IS_QUEST_FINISHED = 44, // It returns true if the quest has been finished (regardless of if it was ultimated or not) Param 1: quest_type, range 0-11, Param 2: quest_id (as integer, usually own code)
    QED_COND_IS_QUEST_UNFINISHED = 45, // Param 1: quest_type, range 0-11, Param 2: quest_id (as integer, usually own code)
    QED_COND_TIME_IN_DEMO = 46, // (Related with demos, only used by legend patrol quests) Param 1: i1
    QED_COND_DRAGON_BALL_OBTAINED = 47, // Param 1: team_type ( 1,2 -> player team, 3 -> enemy team), Param 2: i2 (ignored by exe)
    QED_COND_REMAINING_TIME = 48, // Param 1: <= / >=, Param 2: seconds (float)
    QED_COND_REVIVE_CHECK = 49, // Param 1: qml_char
    QED_COND_DETECTED_WITH_SCOUTER = 50, // (vanilla only uses this in TMQ_1400) Param 1: qml_char
    QED_COND_COMBO_SUCCESS = 51, // Only used in quests with key combinations display (like CHQ, and the zamasu teacher) It may be related with action 90. Params: none
    QED_COND_ACTION_DONE = 53, // (Only used in chq quests. Note: game acceses a XG::Timer::Timer object. May be related with combo/key combination executed) Param 1: i1, Param 2: i2, Param 3: i3, Param4: i4
    QED_COND_ITEM_USED = 54, // (Only found used in some chq quests) Param 1: i1 (unknown)
    QED_COND_UNK_55 = 55, // Param 1: qml_char, Param 2: i2 (range 0-12, looks like a type/sub-opcode in the exe), Param 3: b3
    QED_COND_TEACHER_DIALOGUE_FINISH = 56, // (Only used in CHQ quests) Param 1: integer, teacher dialogue id, NNN in CAT_XXX_NNN, in files Teacher_XXX_[lang/voice].msg)
    QED_COND_HEALTH_DAMAGE_OVER = 57, // (game uses this in raids) Param 1: qml_char, Param 2: absolute health (integer, not float)
    QED_COND_TTL_GAME_FINISHED = 58, // Related with hero colisseum, used in all? hero colisseum missions (TFB,TNB,TTQ). Params: none
    QED_COND_CURRENT_PARTNER_IS = 59, // (only works in OSQ quests). Param 1: partner_id
    QED_COND_UNK_60 = 60, // Added in version 1.11. Param 1: boolean. Param 2: integer. Param 3: integer.
    QED_COND_UNK_61 = 61, // Added in ? Only used in tournaments? Param 1: integer.
    QED_COND_UNK_62 = 62, // Added in ? Only used in tournaments? Param 1: qml_char Param 2: integer. Param 3: integer. Param 4: integer.
};

// There are 120 actions according to the exe (function 0x6AC380 in 1.09.01)
// We shall put here those that are used in at least one qed, and the alias opcodes

// Actions currently unused by vanilla:
// 24 (unlisted, which makes it act like a nop, it may have been a debug action)
// 53 (qml_char, b2)
// 69 (alias of QED_ACT_GOTO_STATE)
// 72 (qml_char, i2) May be related with dragon balls
// 77 (unlisted, which makes it act like a nop, it may have been a debug action)
// 94 (unlisted, which makes it act like a nop, it may have been a debug action)
// 95 (unlisted, which makes it act like a nop, it may have been a debug action)
// 98 (i1, b2) Game strings suggest something about "Lobby Flag"
// 114 (b1, stat as in QedStatsType, amount (int). Apparently like QED_ACT_STATS, but for player only (game acceses player var)

enum QedActions
{
    QED_ACT_NOP = 0, // No operation, literally does nothing
    QED_ACT_INIT_QUEST = 1, // (Exe just sets a variable to 1 in this action) Params: none
    QED_ACT_FADE_IN = 2, // Does a fade in (game acceses XG::Game::Common::ScrFade singleton). Param 1: speed or degrees or whatever (float, higher makes fade in slower) Param 2: fade_mode, Param 3: keep ui (if false, ui is hidden while the fade happens), Param 4: i4
    QED_ACT_FADE_OUT = 3, // Same params than fade_in
    QED_ACT_PLAY_BGM = 4, // Note: if a custom bgm iset by the user in the menus, this will do nothing, as the debug string of the game suggests   Param 1: i1 (range 0-2), Param 2: cue id, Param 3: fade time (milisecs)
    QED_ACT_STOP_BGM = 5, // Param 1: b1, Param 2: cue_id, Param 3: fade time (milisecs integer)
    QED_ACT_LOAD_DEMO = 6, // Param 1: filename, Param 2: fade_mode  (see fade mode)
    QED_ACT_REGISTER_ACTOR_FOR_DEMO = 7, // (official name is ActorRegist, but this is demo related only) Param 1: qml_id. Note: you can only register up to 7 actors, game holds a count and checks for this!
    QED_ACT_PLAY_DEMO = 8, // Plays loaded demo. Params: none
    QED_ACT_UNK_9 = 9, // Related with demo. Param 1: i1 (unused by game), Param 2: i2, Param 3: b3
    QED_ACT_UNK_10 = 10, // (Used only in few legend patrol quests) Param 1: qml_char, Param 2: i2 (param is logged, but not used otherwise)
    QED_ACT_QUICK_POWERUP = 11, // (Used in some story quests of OSQ and CTP) Param 1: qml_char, Param 2: transformation
    QED_ACT_REMOVE_CHARA_ = 12, // (Alias for REMOVE_CHARA opcode 16, identical function called) Param 1: qml_char, Param 2: boolean (if true, applies a vanish fade)
    QED_ACT_SET_REPLACE_CHARA_LOAD = 13, // (It spawns a char, which takes slot of a previously removed (with RemoveChara) or defeated char. official game name SetReplaceCharaLoad) Param 1: position (can be ""), Param 2: qml_char, Param 3: b3 (if true, char is spawned IF possible, if false char seems to be removed), Param 4: fade_mode (see fade type)
    QED_ACT_SET_CHAR_VISIBLE = 14, // (Identical to SET_CHAR_VISIBLE_ Original function name: enemydraw) Param 1: qml_char, Param 2: boolean (if true, char appears if it wasn't visible, if false, it disappears if is visible)
    QED_ACT_RECOVER_HP = 15, // Param 1: qml_char, Param 2: percent
    QED_ACT_REMOVE_CHARA = 16, // (This and 12 are identical twins)
    QED_ACT_SET_REPLACE_CHARA_LOAD_ = 17, // Alias for SET_REPLACE_CHARA_LOAD. Exactly the same thing.
    QED_ACT_SET_CHAR_VISIBLE_ = 18, // Alias for SET_CHAR_VISIBLE (identical function)
    QED_ACT_QUEST_CLEAR = 19, // (Clears quest, starting the quest result and rewards screen appear, etc) Params: none  (Note, game name for this function is "EventEnd")
    QED_ACT_UNK_20 = 20, // (Game accesses player var in this action. ) Params: none
    QED_ACT_START_BATTLE_MODE = 21, // Params: none
    QED_ACT_UNK_22 = 22, // (Does something to either player or maybe to all loaded chars, only seen used in OSQ quests) Param 1: b1
    QED_ACT_GOTO_STATE = 23, // Param 1: state    
    QED_ACT_PORTAL = 25, // Param 1: departure stage, Param 2: Destination stage, Param 3: gate state (0 -> locked, 1 -> unlocked), Param 4: show_scene (boolean)
    QED_ACT_PLAY_DIALOGUE = 26, // Param 1: qbt_dialogue, Param 2: i2, Param 3: stage_id (-1 -> any stage), Param 4: i4 (unused by the exe)
    QED_ACT_CHARA_SPAWN = 27, // (Official name: Trespass. If char cannot be spawned inmediately because number of players is complete, it will spawn when possible when other chars are removed/defeated) If dialogue is specified, the dialogue portrait will be forced to this char. Param 1: qml_char, Param 2: position (integer), Param 3: qbt_dialogue, Param 4: scene id, Param 5: stage Param 6: i6
    QED_ACT_PLAY_ETOOL_EVENT = 28, // (This is official name of function) Param 1: qml_char, Param 2: scene id, Param 3: qbt_dialogue (can be -1), Param 4: position? Exe code suggests it is the same thing used in "position" in CHARA_LEAVE
    QED_ACT_UNK_29 = 29, // Between other things, if param2 is false, the game implementation constructs a new XG::Game::Menu::Result, which loads RESULT.iggy, otherwise what this function do is unknown.) Param 1: b1 (if b2 is true, this param is ignored), Param 2: b2
    QED_ACT_SHOW_QUEST_SUMMARY = 30, // Shows the quest title and condition/defeat victory. This is only needed in main quests / xv1 quests / osq quests, in all others this is automatically done.  (qtheme.iggy is loaded in this func) Params : none
    QED_ACT_AI_TABLE = 31, // Param 1: qml_char, Param 2: ait_entry in ai_table.ait
    QED_ACT_UNLOCK_QUEST = 32, // (TODO: check if usually called after 29, and check if this indeed loads the quest) Param 1: quest_id  (integer id, not string)
    QED_ACT_QUEST_FINISH_STATE = 33, // Param 1: finish state (0 = Quest Complete, 1 = Ultimate Finish, 2 = Fail)
    QED_ACT_SET_FLAG = 34, // Param 1: flag_id (range 0-63), Param 2: value (boolean)
    QED_ACT_SHOW_WARNING = 35, // Params: none
    QED_ACT_SET_TIME = 36, //  Param 1: mode (0 -> adds time (cannot go over max quest time), 1 -> substracts time (cannot go below 0), 2 -> sets the elapsed time, this is, the max quest time - remaining time), Param 2: seconds (float)
    QED_ACT_HEALTH_CAP = 37, // Param 1: enable (boolean), Param 2: qml_char, Param 3: percent (float)
    QED_ACT_CANCEL_DIALOGUE = 38, // Cancels any ongoing dialogue (probably including those generated outside scripts like common dialogue) Params: none
    QED_ACT_STATS = 39, // (TODO: check if this should be called SetStats or ModStats) Param 1: qml_char, Param 2: type (see Stats type), Param 3: amount (int)
    QED_ACT_UNK_40 = 40, // (Only used in CTP_05_03, on 2 different qml instances of same qxd Cooler) Whatever this does, the boolean param acts a toggle. Param 1: qml_char, Param 2: b2
    QED_ACT_USE_SKILL = 41, // Param 1: qml_char, Param 2: skill (0-3 super, 4-5 Ultimate, 6 Evasive, blast/awaken not supported), Param 3: b3 (TODO: check what this boolean does, only quests that set this to true are CTP_04_02 and CTP_07_04, which happen to be quests where Goku throws the bomb to Frieza and Bu respectively in cutscene)
    QED_ACT_SET_ATTACK_TARGET = 42, // Param 1: qml_char, Param 2: qml_char (-1 is valid input and causes the function to do nothing), Param 3: b3
    QED_ACT_ACTIVATE_INTERACTIVE_CHAR = 43, //  (todo: check what boolean params do) Param 1: qml_char, Param 2: b2, Param 3: b3
    QED_ACT_CHARA_SPAWN2 = 44, // (Official name: Trespass) Param 1: qml_char ("leader"), Param 2: qml_char, Param 3: i3 (probably position), Param 4: qbt_dialogue, Param 5: stage, Param 6: i6, Param 7: i7 (unused, set to -1), Param 8: i8 (unused, set to -1)
    QED_ACT_CHARA_SPAWN3 = 45, // (Official name: Trespass) Param 1: qml_char ("leader"), Param 2: qml_char, Param 3: qml_char, Param 4: i4 (probably position), Param 5: qbt_dialogue, Param 6: stage, Param 7: i7, Param 8: i8 (unused, set to -1)
    QED_ACT_CHARA_LEAVE = 46, // (Official name: ExitDemo) Param 1: qml_char, Param 2: position, Param 3: qbt_dialogue, Param 4: cmn_bev_index, Param 5: b5, Param 6: stage to leave to (can be -1)
    QED_ACT_UNK_47 = 47, // (Only used in TMQ_0801 twice, on two different qml instances (same qxd) of Beerus) Param 1: qml_char, Param 2: i2
    QED_ACT_REVIVE = 48, // (Official name RevivalDemo) Param 1: qml_char, Param 2: qbt_dialogue, Param 3: scene_id, Param 4: health percent (float)
    QED_ACT_POWERUP = 49, // (Official name Powerup demo) Param 1: qml_char, Param 2: qbt_dialogue (can be -1), Param 3: scene_id
    QED_ACT_SET_THERE_ARE_ENEMIES = 50, // Sets if there are enemies (stage portal will show as green when there is. Note that if there are chars loaded in some stage, SetThereAreEnemies(stage, false) won't work. Param 1: stage_id, Param 2: toggle (true -> there are enemies, false -> there is not)
    QED_ACT_UNK_51 = 51, // (It is only used in TMQ_0800 on Beerus, several times with different i2) Param 1: qml_char, Param 2: i2
    QED_ACT_MOVE_TO = 52, // Param 1: position (string), Param 2: i2 (unused by game), Param 3: qml_char
    QED_ACT_TRANSFORM = 54, // (Official name: ModelChangeDemo) Param 1: qml_char (current), Param 2: qml_char to transform, Param 3: position, Param 4: qbt_dialogue, Param 5: scene_id
    QED_ACT_LOAD_CHARA = 55, // (Loads chara on the background, doesn't spawn it) Param 1: qml_char
    QED_ACT_DONT_REMOVE_ON_KO = 56, // It avoids the body from being removed when the char is defeated.  (the body will still be removed when changing stage or when another char replace it) Param 1: qml_char
    QED_ACT_WAIT = 57, // Param 1: seconds (float)
    QED_ACT_CHANGE_STAGE = 58, // Param 1: stage_id, Param 2: fade_mode (see QedFadeMode) On some circunstances, the fade_mode is ignored.
    QED_ACT_PLAY_BEV = 59, // Note: any of the qml chars can be -1 or outside of range) Param 1: i1, Param 2: qml_char, Param 3: qml_char, Param 4: qml_char, Param 5: qml_char, Param 6: qml_char, Param 7: qml_char Param 8: qml_char
    QED_ACT_REMOVE_HP = 60, // Param 1: qml_char, Param 2: percent (float)
    QED_ACT_UNK_61 = 61, // (Used in bunchs at beginning of some PQ scripts) Param 1: qml_char, Param 2: i2 (valid range 0-22), Param 3: i3, Param 4: i4
    QED_ACT_AI_LOCK_SKILL = 62, // Param 1: qml_char, Param 2: skill (0-3 super, 4-5 ultimate, 6 evasive (blast/awaken not supported here), Param 3: lock/unlock (0=lock,1=unlock, TODO: chek if like that)
    QED_ACT_UNK_63 = 63, // (Used in very few quests, some PQ, and only 1 CTP) (f2 is converted into integer and the "i36" of qml is set to this value) Param 1: qml_char, Param 2: f2 (Note, the game will convert the float into integer, so having fractional values is pointless here)
    QED_ACT_UNK_64 = 64, // ???  Param 1: qml_char, Param 2: i2
    QED_ACT_PLACE_DRAGON_BALL = 65, // Param 1: if >= 0, it indicates the dragon ball position in (QUEST_DB_POS_%02d). If it is < 0, the position is gathered with a random number between param3 and param 4. Param 2: the ball to set (0 is 1-star ball, 6 is 7-star ball). Negative number: the ball is selected randomly from the ones not placed yet, Param 3: if param1 was < 0, this param indicates the min random number, otherwise ignored. Param 4: if param1 was < 0, this param indicates the max random number, otherwise ignored
    QED_ACT_SET_CAN_CARRY_DB = 66, // Param 1: qml_char, Param 2: boolean (if true it can carry them, otherwise it disables carry)
    QED_ACT_LOAD_BEV = 67, // Param 1: filename (relative to data/event/battle/, and without extension)
    QED_ACT_SET_INVULNERABLE = 68, // Makes char invulnerable if param is true (it cannot be hit) Note: this function will not work if called before BattleModeStart Param 1: qml_char, Param 2: boolean toggle. The game calls the boolean AtariOFF,AtariON. (true invulnerable).
    QED_ACT_GOTO_STATE_ = 69, // Alias for GOTO_STATE, but no quest use this opcode.
    QED_ACT_DISABLE_COMMON_DIALOGUE = 70, // Param 1: num of chars following probably what to cancel, Param 2: qml_char or set to a number outside qml range for invalid, Param 3: qml_char or set to a number outside qml range, Param 4: qml_char or set to a number outside qml range) Param 5-8: ignored by game
    QED_ACT_TUT_POPUP = 71, // Param 1: tutorial id, Param 2: i2 (ignored by game)
    QED_ACT_UNK_73 = 73, // (This function may only work online) Param 1: qml_char
    QED_ACT_UNK_74 = 74, // Param 1: qml_char
    QED_ACT_PLAY_CLOCK = 75, // (only BAQ_FSS_05 uses this.) Param 1: stop/play (0 = stop, 1 = play) (stop/play are official game names for these values) If false, it stops clock. If true, clock continues to play. Have in mind that while clock is stopped, pause menu will not work either
    QED_ACT_COPY_HEALTH = 76, // TODO: test this in game (Copies health of char 1 into char 2. Only current health is copied, not max health. It is often used to transfer health between different qml instances of same qxd char) Param 1: qml_char, Param 2: qml_char
    QED_ACT_UNK_78 = 78, // (Related with demos, only used by TPQ_09_03, which happen to be end of main xv1 story) Param 1: demo file
    QED_ACT_CREDITS = 79, // Loads and starts the credits screen (only used in CTP_11_01). Params: none
    QED_ACT_UNK_80 = 80, // (It usually precedes PlayBev) Implementation notice: game acceses a XG::Game::Battle::Event object and sets two properties with the params) Param 1: i1, Param 2: i2
    QED_ACT_UNK_81 = 81, // (Only used in TMQ_0403 and TMQ_1200, with opposite boolean values) Param 1: qml_char, Param 2: b2
    QED_ACT_PLAY_DIALOGUE2 = 82, // Used in two TMQ only (At first sight, this looks like a PLAY_DIALOGUE with a qml_char passed, it may be used to force a portrait in the subs over the one in qbt, TODO: test the theory) Param 1: qbt_dialogue, Param 2: i2 (whatever it is, it is same thing that i2 of play_dialogue), Param 3: stage_id, Param 4: i4 (unused by game), Param 5: qml_char
    QED_ACT_TRANSFORM2 = 83, // The difference with the other TRANSFORM is who appears in the subtitle portrait of the dialogue. Param 1: qml_char (current), Param 2: qml_char to transform, Param 3: position, Param 4: qbt_dialogue, Param 5: scene_id
    QED_ACT_UNK_84 = 84, // (Used in most (all?) TMQ, but not anywhere else, around end of script) Param 1: stage_id
    QED_ACT_ADD_TIME = 85, // Param 1: only_refresh (if true, no time is added, clock in the gui is just refreshed. Todo: check this last case), Param 2: seconds (float)
    QED_ACT_SHOW_ENEMY_KO_COUNTER = 86, //  Param 1: b1 (iprobably toggle) Param 2: num
    QED_ACT_SHOW_RADAR = 87, // Param 1: toggle
    QED_ACT_SKILLS_EQUIP = 88, // Param 1: qml_char, Param 2: equip(true)/unequip(false) (boolean), Param 3: Bitmask (e.g. 0x200 = bit 9 = equip/unequip awaken, 0x202 = bit 9 and 2 = equip/unequip super 0 and awaken)
    QED_ACT_SHOW_NAME = 89, // Only used in TMQ_1400, which is the first tutorial-like PQ. Param 1: qml_char, Param 2: true
    QED_ACT_SHOW_COMBO_AND_LISTEN = 90, // (Used in quests with key combinations like CHQ and the Zamasu TCQ.) Param 1: b1, Param 2: i2, Param 3: qml_char
    QED_ACT_SHOW_DEMONSTRATION_TEXT = 91, // Shows "demonstration" in the bottom part of the screen  (used in some chq quests) Param 1: toggle (true show, false hide)
    QED_ACT_ENABLE_MOVEMENT = 92, // (Used in some CHQ quests ) Param 1: if false, it disables movement, if true it enables movement back, Param 2: qml_char
    QED_ACT_LOAD_AIC = 93, // Param 1: filename (relative to the path data/battle/aic/, and without the extension. Worth to mention game only has test.aic there), Param 2: id (official param name, probably an entry in the .aic file), Param 3: b3
    QED_ACT_UNK_94 = 94, // Params: none
    QED_ACT_TIP_MSG = 96, // Param 1: msg_id (as string), Param 2: b2
    QED_ACT_SCATTER_ATTACK = 97, // It causes current enemy to execute the scatter attack (the expert mission attack that sends you to another place). The game randomly choose an stage from the ones defined in the stage param of all QmlChar. i52 of QmlChar must be set to 16, or char may not appear? (internal game string refers to these chars as "RisanPlayers")    Param 1: i1 (game always uses 70), Param 2: i2 (game always uses 71), Param 3: i3 (game always uses 72), Param 4: i4 (game always uses 0 or -1)
    QED_ACT_LISTEN_FOR_ACTION = 99, // (Only used in some chq quests, may or may not be related with condition 53 Note: game acceses a XG::Timer::Timer object, and sets a value of an array indexed by param i1 to 0) Param 1: i1 (range 0-27)
    QED_ACT_TEACHER_DIALOGUE = 100, // (Used only in some chq quests) Param 1: integer, teacher dialogue id, NNN in CAT_XXX_NNN, in files Teacher_XXX_[lang/voice].msg)
    QED_ACT_ENABLE_ITEMS = 101, // (Used only in a few CHQ quests, in the Art of Battle ones) Param 1: b1
    QED_ACT_PLAY_AIC = 102, // (Used in some chq quests) Params: none
    QED_ACT_UNK_103 = 103, // (Used in few CHQ quests, the art of battle ones it seems) Note: the game only sets a 64bit property of this char to -1. They may be two 32 bits ones if optimizations of compiler did their work) Param 1: qml_char
    QED_ACT_RESET_CAMERA = 104, // Restores camera to original position. Params: none
    // Worth to mention that all the functions below didn't exist in the first version released for PC (1.02 or whatever)
    QED_ACT_UNK_105 = 105, // Related with demos (Only used in CTP_15_03  and CTP_15_04) Param 1: string (this gets things like "EFCT_sakeme" and "bnBFtwfDM00_D15", which can be found in some demo files), Param 2: i2 (unused by exe), Param 3: b3 Param 4: i4
    QED_ACT_BODY_CHANGE = 106, // (Official name: SystemBodyChange) Swaps the bodies of the two chars, as the Ginyu skill would do (chars preserve their voice and portrait, it leads to same function called by the bac of ginyu skill). Cannot be used in all kind of quests (?), cannot be used online, cannot be used with gates around (not even if they are closed), function will do nothing in those circumstances.  Param 1: qml_char, Param 2: qml_char
    QED_ACT_HERO_COLOSSEUM = 107, // Note: vanilla game quests don't use the figure params, they set to -1, but from game code, they are definitely qml_id) Param i:"enemy_master_id" (official param name). qml_char of AI enemy master (canNOT be -1),  Param 2: qml_char (this one can be -1, proably figure 1), Param 3: qml_char (probably figure 2), Param 4: qml_char (can be -1, probably figure 3), Param 5: qml_char (can be -1, probably figure 4), Param 6: qml_char (can be -1, probably figure 5), Param 7: i7 (operation?), Param 8: i8 (not used by game)
    QED_ACT_UNK_108 = 108, // (Used in OSQ, and a couple of TPQ) Param 1: i1 (0-2) Param 2: b2
    QED_ACT_STAT_REGENERATION = 109, // Param 1: qml_char, Param 2: enable/disable (true/false), Param 3: bitmask of one or more effects 1,2,4 (TODO: check which one is each. but probably 1 is health, 2 stamina and 4 ki), Param 4: amount (float)
    QED_ACT_HERO_COLOSSEUM_HINT = 110, // Gives a hero colisseum hint, only to be used by hero coliseum quests) Param 1: msg_id (as string)
    QED_ACT_REVIVE_EX = 111, // Like revive, but it can use a postion previously set by SET_POSITION_VAR Param 1: qml_char, Param 2: qbt_id (can be -1), Param 3: scene_id, Param 4: position var (as set by SET_POSITION_VAR), Param 5: health percent,
    QED_ACT_POWERUP_EX = 112, // Like POWERUP, but it can use a position previously set by SET_POSITION_VAR Param 1: qml_char Param 2: qbt_dialogue (can be -1) Param 3: scene Param 4: position_var (as set by SET_POSITION_VAR)
    QED_ACT_SET_STRING_VAR = 113, // Saves a position into a variable (only 10 variable can be used) Param 1: position (string), Param 2: i2 (unused by exe), Param 3: position_var (range 0-9)
    QED_ACT_UNK_114 = 114, // Only seen used in tournament quests. Param 1: boolean, Param 2: integer (range checked in 0-18) Param 3: integer or float
    QED_ACT_PLAY_DIALOGUE3 = 115, // (PLAY_DIALOGUE with the last param not ignored) Param 1: qbt_dialogue, Param 2: i2 (whatever it is, it is the same thing than in PLAY_DIALOGUE), Param 3: stage_id, Param 4: i4 (unlike with PLAY_DIALOGUE, this isn't ignored)
    QED_ACT_UNK_116 = 116, // Bgm related, only used (and widely) in OSQ_0500. Param 1: cue_id (bgm), Param 2: i2 (only 0 was observed, but the game reads the param)
    QED_ACT_UNK_117 = 117, // May be related with audio, only used in OSQ_0500. Params: none
    QED_ACT_CHARA_RESET = 118, // Only used in CHQ_1600 (Dual Ultimate Attack Tutorial) (Official name of function) Param 1: qml_char
    QED_ACT_UNK_119 = 119, // (only used in few OSQ quests, probably added in 1.08+ of game) Param 1: qml_char
    QED_ACT_UNK_120 = 120, // (Added in 1.10) Param 1: integer (range 0-127), Param 2: integer (some kind of subopcode, range of 0-5) Param 3: integer  Param 4: integer
    QED_ACT_UNK_121 = 121, // (Added in 1.10) Param 1: qml char Param 2: integer (range 0-18) Param 3: integer (range 0-127)
    QED_ACT_UNK_122 = 122, // (Added in 1.10) Param 1: qml char Param 2: integer (range 0-127)
};

// Condition extensions of xv2 patcher
enum QedCondExtension
{
    QED_COND_EXT_IS_AVATAR = 1000, // Param 1: qml_char.

    QED_COND_EXT_LIMIT, // Just for limit check, no condition
};

typedef QEDInstructionParams QedInstructionParams;

struct QedInstruction
{
    uint16_t opcode;
    QedInstructionParams params;

    QedInstruction()
    {
        opcode = 0; // Never (cond) / Nop (Action)
        memset(&params, 0, sizeof(params));
    }

    inline bool operator==(const QedInstruction &rhs) const
    {
        return (opcode == rhs.opcode && memcmp(&params, &rhs.params, sizeof(params)) == 0);
    }

    inline bool operator!=(const QedInstruction &rhs) const
    {
        return !(*this == rhs);
    }
};

struct QedEvent
{
    std::vector<QedInstruction> conditions;
    std::vector<QedInstruction> actions;

    inline bool IsEmpty() const
    {
        return (conditions.size() == 0 && actions.size() == 0);
    }

    inline bool operator==(const QedEvent &rhs) const
    {
        return (conditions == rhs.conditions && actions == rhs.actions);
    }

    inline bool operator!=(const QedEvent &rhs) const
    {
        return !(*this == rhs);
    }
};

struct QedState
{
    std::map<int16_t, QedEvent> events;

    inline bool IsEmpty() const
    {
        return (events.size() == 0);
    }

    inline bool operator==(const QedState &rhs) const
    {
        return (events == rhs.events);
    }

    inline bool operator!=(const QedState &rhs) const
    {
        return !(*this == rhs);
    }
};

class QedFile : public BaseFile
{
private:

    std::vector<QedState> states;

protected:

    void Reset();
    bool LoadInstructions(const uint8_t *top, const QEDInstruction *file_instructions, uint32_t num, bool action);
    void SaveInstructions(uint8_t *top, QEDInstruction *file_instructions, bool action, QEDInstructionParams **pcurrent_param) const;

public:
    QedFile();
    virtual ~QedFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    inline size_t GetNumStates() const { return states.size(); }
    size_t GetNumConditions() const;
    size_t GetNumActions() const;

    inline const std::vector<QedState> &GetStates() const { return states; }
    inline std::vector<QedState> &GetStates() { return states; }

    inline const QedState &operator[](size_t n) const { return states[n]; }
    inline QedState &operator[](size_t n) { return states[n]; }

    inline std::vector<QedState>::const_iterator begin() const { return states.begin(); }
    inline std::vector<QedState>::const_iterator end() const { return states.end(); }

    inline std::vector<QedState>::iterator begin() { return states.begin(); }
    inline std::vector<QedState>::iterator end() { return states.end(); }

    inline bool operator==(const QedFile &rhs) const
    {
        return (states == rhs.states);
    }

    inline bool operator!=(const QedFile &rhs) const
    {
        return !(*this == rhs);
    }
};

#endif // QEDFILE_H
