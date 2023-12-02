#ifndef XV2QUESTCOMPILER_H
#define XV2QUESTCOMPILER_H

#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <queue>
#include <bitset>

#include "Utils.h"
#include "Xenoverse2.h"
#include "X2mFile.h"
#include "QxdFile.h"
#include "QmlFile.h"
#include "QbtFile.h"
#include "QslFile.h"
#include "QedFile.h"
#include "QsfFile.h"
#include "CusFile.h"

// This is a limit imposed to new quest names.
// This is mainly to avoid overflow in the msg string in case they are referenced by scripts, which have very limited space for a string.
#define MAX_NEW_QUEST_NAME  12

enum TokenType
{
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_INTEGER,
    TOKEN_BOOLEAN,
    TOKEN_FLOAT,
    TOKEN_COLON,
    TOKEN_BRACKET_BEGIN,
    TOKEN_BRACKET_END,
    TOKEN_GROUP,

    TOKEN_POLI_INTEGER_IDENTIFIER,
    TOKEN_POLI_INTEGER_STRING_IDENTIFIER,
    TOKEN_POLI_INTEGER_STRING,
    TOKEN_POLI_STRING_IDENTIFIER,
};

struct ci_string
{
    std::string str;

    ci_string() { }

    ci_string (const ci_string &other)
    {
        str = other.str;
    }

    ci_string (const std::string &other)
    {
        str = other;
    }

    ci_string(const char *cstr)
    {
        str = cstr;
    }

    inline void clear()
    {
        str.clear();
    }

    inline size_t length() const { return str.length(); }

    inline ci_string substr(size_t pos, size_t count=std::string::npos) const
    {
        ci_string cs;
        cs.str = str.substr(pos, count);
        return cs;
    }

    inline size_t find (const ci_string &str, size_t pos = 0) const // noexcept
    {
        return this->str.find(str.str, pos);
    }

    inline size_t find (const char* str, size_t pos = 0) const
    {
        return this->str.find(str, pos);
    }

    inline size_t find(const char* str, size_t pos, size_t n) const
    {
        return this->str.find(str, pos, n);
    }

    inline size_t find(char c, size_t pos = 0) const // noexcept
    {
        return str.find(c, pos);
    }

    inline void push_back(char ch)
    {
        str.push_back(ch);
    }

    inline const char *c_str() const{ return str.c_str(); }

    inline ci_string &operator=(const ci_string &other)
    {
        str = other.str;
        return *this;
    }

    inline ci_string &operator=(const std::string &other)
    {
        str = other;
        return *this;
    }

    /*inline ci_string &operator=(const char *cstr)
    {
        str = cstr;
        return *this;
    }*/

    inline ci_string &operator+=(const ci_string &other)
    {
        str += other.str;
        return *this;
    }

    inline ci_string &operator+=(const std::string &other)
    {
        str += other;
        return *this;
    }

    inline bool operator==(const ci_string &other) const
    {
        return (Utils::ToLowerCase(this->str) == Utils::ToLowerCase(other.str));
    }

    inline bool operator!=(const ci_string &other) const
    {
        return !(*this == other);
    }

    inline char &operator[](size_t n)
    {
        return str[n];
    }

    inline const char &operator[](size_t n) const
    {
        return str[n];
    }
};

struct X2QcToken
{
    int type;
    int num;
    float fnum;
    bool b;
    int line_num;
    ci_string str;
    std::string file;

    X2QcToken()
    {
        Reset();
    }

    void Reset()
    {
        type = TOKEN_IDENTIFIER;
        num = 0;
        fnum = 0.0;
        b = false;
        line_num = -1;
        str.clear();
    }

    void End()
    {
        if (type == TOKEN_INTEGER)
        {
            num = Utils::GetSigned(str.str);
            fnum = (float)num;
        }
        else if (type == TOKEN_STRING || type == TOKEN_GROUP)
        {
           if (str.length() >= 2)
           {
               str = str.substr(1, str.length()-2);
           }
        }
        else if (type == TOKEN_IDENTIFIER)
        {
            bool dot = false;
            bool is_float = true;
            bool first_pos = true;

            for (char ch : str.str)
            {
                if (ch >= '0' && ch <= '9')
                {
                }
                else if (ch == '-' && first_pos)
                {

                }
                else if (ch == '.')
                {
                    if (dot)
                    {
                        is_float = false;
                        break;
                    }

                    dot = true;
                }
                else
                {
                    is_float = false;
                }

                first_pos = false;
            }

            if (is_float)
            {
                sscanf(str.c_str(), "%f", &fnum);
                type = TOKEN_FLOAT;
            }
            else
            {
                if (str == "true" || str == "false")
                {
                    type = TOKEN_BOOLEAN;
                    b = (str == "true");
                }
            }
        }
    }

    bool Empty() const
    {
        return (str.length() == 0);
    }

    static std::string GTokenName(int type)
    {
        std::string ret;

        if (type == TOKEN_IDENTIFIER)
            ret = "identifier";

        else if (type == TOKEN_STRING)
            ret = "string";

        else if (type == TOKEN_INTEGER)
            ret = "integer";

        else if (type == TOKEN_FLOAT)
            ret = "float";

        else if (type == TOKEN_COLON)
            ret = ":";

        else if (type == TOKEN_BRACKET_BEGIN)
            ret = "{";

        else if (type == TOKEN_BRACKET_END)
            ret = "}";

        else if (type == TOKEN_GROUP)
            ret = "(group)";

        else if (type == TOKEN_BOOLEAN)
            ret = "boolean";

        return ret;
    }

    std::string TokenName(bool add_str) const
    {
        std::string ret = GTokenName(type);

        if (add_str && type != TOKEN_BRACKET_BEGIN && type != TOKEN_BRACKET_END && type != TOKEN_COLON)
            ret += "(" + str.str +")";

        return ret;
    }
};

struct CIStrHash
{
    size_t operator()(const ci_string &str) const
    {
        return std::hash<std::string>()(Utils::ToLowerCase(str.str));
    }
};

struct XQ_X2mMod
{
    X2mType type;
    std::string path; // Path to x2d
    std::string code; // For chars, skills, stages and quests
    uint32_t id; // For chars, skills (id1), quests and stages
    X2mCostumeEntry costume; // Only for costumes
    std::string name; // Only to be used for error messages or to create var names
    std::string guid;

    // To be used by the compiler/decompiler, not outside
    ci_string var_name;

    XQ_X2mMod()
    {
        type = X2mType::REPLACER;
        id = 0xFFFFFFFF;
    }

    inline bool operator==(const XQ_X2mMod &rhs) const
    {
        return (guid == rhs.guid);
    }

    inline bool operator!=(const XQ_X2mMod &rhs) const
    {
        return !(*this == rhs);
    }
};

struct XQModHash
{
    size_t operator()(const XQ_X2mMod &mod) const
    {
        return std::hash<std::string>()(Utils::ToLowerCase(mod.guid));
    }
};

enum InstructionParamsTypes
{
    PARAM_INTEGER,
    PARAM_INTEGER_HEX, // For the compiler, there is no difference with above, but for decompiler it makes it to write it in hex
    PARAM_FLOAT,
    PARAM_BOOLEAN,
    PARAM_REVERSE_BOOLEAN,
    PARAM_STRING, // 20 chars
    PARAM_STRING_LONG, // 24 chars
    PARAM_STRING_SHORT, // 12 chars
    PARAM_OPERATOR,
    PARAM_SCMS,
    PARAM_SKILL,
    PARAM_STAGE,
    PARAM_QUEST,
    PARAM_QUEST_WITH_TYPE,
    PARAM_QCHAR,
    PARAM_QBT,
    PARAM_PARTNER,
    PARAM_FLAG,
    PARAM_STRINGVAR,
    PARAM_RACE,
    PARAM_SEX,
    PARAM_FADE,
    PARAM_TIME_CTRL_MODE,
    PARAM_TEAM_TYPE,
    PARAM_TEAM_TYPE2,
    PARAM_STATS,
    PARAM_SKILL_SLOT,
    PARAM_BATTLE_MSG,
    PARAM_SKILL_COMBINATION,
    PARAM_STATS_COMBINATION,
    PARAM_QUEST_FINISH_STATE
};

struct InstructionDef
{
    uint16_t opcode;
    ci_string name;
    bool is_action;
    std::vector<int> param_types;
};

class Xv2QuestCompiler
{
private:

    // Non reset vars
    std::unordered_map<std::string, XQ_X2mMod> mods_table; // (GUID -> mod) (Optimized for compilation, but not for decompilation...)
    std::vector<QedFile *> scripts;
    bool test_mode; // Test mode on: don't resolve x2m
    std::string res_directory;
    X2mFile *res_x2m;

    // Common vars
    int quest_type;
    QxdFile active_qxd;
    QmlFile active_qml;
    QbtFile active_qbt;
    QslFile active_qsl;
    QedFile active_qed0;
    QedFile active_qed1;
    QedFile active_qed2;
    QedFile active_qed3;
    QedFile active_qed4;
    QedFile active_qed5;
    QsfFile qsf;
    std::vector<MsgFile> title;
    std::vector<MsgFile> dialogue;
    std::vector<AcbFile> dialogue_acb;
    std::vector<Afs2File> dialogue_awb;
    std::vector<bool> use_internal_awb;
    std::unordered_set<ci_string, CIStrHash> defined_names;
    std::unordered_set<XQ_X2mMod, XQModHash> linked_mods;

    // Decompiler vars
    std::map<uint32_t, ci_string> referenced_collections;
    std::map<uint32_t, ci_string> referenced_chars;
    std::map<uint32_t, ci_string> referenced_special_chars;    
    std::map<uint32_t, ci_string> referenced_qchars;
    std::map<uint32_t, ci_string> referenced_dialogues;
    std::map<uint32_t, ci_string> referenced_interactive_dialogues;
    std::map<uint32_t, ci_string> referenced_event_dialogues;
    std::map<std::string, ci_string> referenced_title;
    std::map<std::string, ci_string> referenced_dialogue_ta;
    std::map<std::string, ci_string> referenced_chars_positions;
    std::map<std::string, ci_string> referenced_ichars_positions;
    std::map<std::string, ci_string> referenced_items_positions;
    std::map<uint32_t, ci_string> referenced_flags;
    std::map<uint32_t, ci_string> referenced_string_vars;
    int indent_level = 0;

    // Compiler vars
    std::queue<X2QcToken> tokens;
    QxdQuest compiled_quest;
    std::unordered_map<ci_string, XQ_X2mMod, CIStrHash> compiled_mods;
    std::unordered_map<ci_string, QxdCollection, CIStrHash> compiled_item_collections;
    std::unordered_map<ci_string, QxdCharacter, CIStrHash> compiled_chars;
    std::unordered_map<ci_string, QxdCharacter, CIStrHash> compiled_special_chars;
    std::unordered_map<ci_string, QmlEntry, CIStrHash> compiled_qml_chars;
    std::unordered_map<ci_string, uint32_t, CIStrHash> compiled_dialogues;
    std::unordered_map<ci_string, uint32_t, CIStrHash> compiled_interactive_dialogues;
    std::unordered_map<ci_string, std::string, CIStrHash> compiled_chars_positions;
    std::unordered_map<ci_string, std::string, CIStrHash> compiled_ichars_positions;
    std::unordered_map<ci_string, std::string, CIStrHash> compiled_items_positions;
    std::unordered_map<ci_string, std::vector<std::string>, CIStrHash> compiled_text_entry; // Text or text audio (size of vector determines that)
    std::unordered_map<ci_string, uint32_t, CIStrHash> compiled_flags;
    std::unordered_map<ci_string, uint32_t, CIStrHash> compiled_string_vars;
    std::unordered_set<ci_string, CIStrHash> processed_audio_files;
    std::vector<bool> used_flags_slots;
    std::vector<bool> used_string_vars;
    bool self_reference_unlock;
    bool self_reference_parent;
    int global_dialogue_index;
    int next_script;
    bool title_touched, dialogue_touched, audio_touched;
    bool qsf_loaded;

    void Reset();

    int GetQuestType(const std::string &name);
    std::string GetDialogueFile(const std::string &name, uint32_t episode);

    bool SupportsAudio();
    std::string GetAudioFile(const std::string &name, uint32_t episode, uint32_t flags, bool english);

    std::string UpdateToConstant(uint32_t update);
    std::string DlcToConstant(uint32_t dlc);

    int ConstantToUpdate(const ci_string &ct);
    int ConstantToDlc(const ci_string &ct);

    std::string AiToConstant(uint32_t ai);
    int ConstantToAi(const ci_string &ct);

    std::string TeamToConstant(uint32_t team);
    int ConstantToTeam(const ci_string &ct);

    std::string DialogueEventToConstant(uint32_t event);
    int ConstantToDialogueEvent(const ci_string &ct);

    std::string GetTitlePath() const;
    std::string GetCommonFilePath(const std::string &quest_name) const;

    bool LoadQxd(QxdFile &qxd, bool vanilla) const;
    bool LoadQsf();

    bool LoadActiveQxd();
    bool LoadActiveQml(const std::string &quest_name);
    bool LoadActiveQbt(const std::string &quest_name);
    bool LoadActiveQsl(const std::string &quest_name);
    bool LoadActiveQed(const std::string &quest_name, size_t index);

    bool CommitCompiledQml();
    bool CommitCompiledQbt();
    bool CommitCompiledQsl();
    bool CommitCompiledQeds();

    bool MaintenanceQxdChar(QxdFile &vanilla_qxd);
    bool MaintenanceQxdCollection(QxdFile &vanilla_qxd);
    bool MaintenanceQxd();
    bool MaintenanceTitle();
    bool MaintenanceDialogue();
    bool MaintenanceAudio();

    bool LoadTitle();
    bool LoadDialogue(const std::string &quest_name, uint32_t episode);
    bool LoadDialogueAudio(const std::string &quest_name, uint32_t episode, uint32_t flags);

    bool LoadHcaResource(HcaFile &hca, const std::string &resource);

    bool SetDialogueAudioFromResource(const std::string &resource, const std::string &track);

    bool SetMsgEntryCommon(const std::string &entry, const std::string &text, std::vector<MsgFile> &msg_files, int lang, bool escape);

    bool GetTitleEntry(const std::string &entry, std::string &ret, int lang=XV2_LANG_ENGLISH);    
    std::string GetTitleEntry(const std::string &entry, int lang=XV2_LANG_ENGLISH);
    bool SetTitleEntry(const std::string &entry, const std::string &text, int lang);

    bool SetDialogueEntry(const std::string &entry, const std::string &text, int lang);

    bool IsValidIdentifier(ci_string &str);
    bool IsValidVarName(ci_string &str);

    int GetQuestChar(const std::string &code);
    bool GetLobbyCharName(int pal_id, std::string &name, int lang=XV2_LANG_ENGLISH);

    bool GetItemName(int item_id, int item_type, std::string &name, int lang=XV2_LANG_ENGLISH);
    std::string GetCollectionName(int id, bool allow_default_name, int lang=XV2_LANG_ENGLISH);

    XQ_X2mMod *FindStageModById(int16_t stage_id);
    XQ_X2mMod *FindSkillModById(int16_t id1);
    XQ_X2mMod *FindCharModById(int16_t cms_id);
    XQ_X2mMod *FindCharModByCode(const std::string &cms_code);
    XQ_X2mMod *FindCostumeModById(int16_t item_id, int item_type);

    ci_string LinkMod(XQ_X2mMod &mod); // Parameter copy

    // Decompiler helper
    ci_string GetFriendlyName(const std::string &str, const std::string &fail_root);

    void WriteIndent(std::ostringstream &oss, bool new_line);

    void WriteQuest(std::ostringstream &oss, int16_t quest_id, std::string &comment);
    void WriteStage(std::ostringstream &oss, int16_t stage_id, std::string &comment);
    bool WriteSkill(std::ostringstream &oss, int16_t id2, int type, bool allow_blast, std::string &comment);
    void WriteChar(std::ostringstream &oss, int16_t cms_id, std::string &comment, int costume_for_comment=-1, int model_preset_for_comment=0);
    void WriteChar(std::ostringstream &oss, const std::string &cms_code, std::string &comment, int costume_for_comment=-1, int model_preset_for_comment=0);
    void WriteTitle(std::ostringstream &oss, const std::string &msg_code, std::string &comment, int lang=XV2_LANG_ENGLISH);
    void WriteDialogueTA(std::ostringstream &oss, const std::string &msg_code, std::string &comment, int lang=XV2_LANG_ENGLISH);
    bool WriteQChar(std::ostringstream &oss, int qml_id, std::string &comment, bool allow_negative, bool allow_non_existing);
    bool WriteDialogue(std::ostringstream &oss, int index, std::string &comment, bool allow_negative, bool allow_non_existing);
    bool WriteInteractiveDialogue(std::ostringstream &oss, int index, std::string &comment, bool allow_negative, bool allow_non_existing);
    bool WriteFlag(std::ostringstream &oss, int flag);
    bool WriteStringVar(std::ostringstream &oss, int string_var);

    void WriteIntegerParam(std::ostringstream &oss, const std::string &name, int32_t value, bool new_line=true, std::string comment="", bool hexadecimal=false);
    void WriteBooleanParam(std::ostringstream &oss, const std::string &name, bool value, bool new_line=true);
    void WriteFloatParam(std::ostringstream &oss, const std::string &name, float value, bool new_line=true);
    void WriteStringParam(std::ostringstream &oss, const std::string &name, const std::string &value, bool new_line=true, std::string comment="");
    void WriteIdentifierParam(std::ostringstream &oss, const std::string &name, const std::string &value, bool new_line=true, std::string comment="");

    template<typename T>
    void WriteIntegerArrayParam(std::ostringstream &oss, const std::string &name, T *values, size_t count, bool new_line=true);

    void WriteQuestParam(std::ostringstream &oss, const std::string &name, int16_t quest_id, bool new_line=true, std::string *comment_out=nullptr);
    void WriteStageParam(std::ostringstream &oss, const std::string &name, int16_t stage_id, bool new_line=true, std::string *comment_out=nullptr);
    bool WriteItemParam(std::ostringstream &oss, const std::string &name, const std::string &type_name, int16_t item_id, int type, bool new_line=true, bool allow_collection=true, std::string *comment_out=nullptr, bool write_type=true);
    bool WriteSkillParam(std::ostringstream &oss, const std::string &name, int16_t id2, int type, bool allow_blast, bool new_line=true, std::string *comment_out=nullptr);
    bool WriteCharParam(std::ostringstream &oss, const std::string &name, int16_t cms_id, bool new_line=true, std::string *comment_out=nullptr, int costume_for_comment=-1, int model_preset_for_comment=0);
    bool WriteCharParam(std::ostringstream &oss, const std::string &name, const std::string &cms_name, bool new_line=true, std::string *comment_out=nullptr, int costume_for_comment=-1, int model_preset_for_comment=0);
    void WriteTitleParam(std::ostringstream &oss, const std::string &name, const std::string &msg_code, bool new_line=true, std::string *comment_out=nullptr, int lang=XV2_LANG_ENGLISH);
    void WriteDialogueTAParam(std::ostringstream &oss, const std::string &name, const std::string &msg_code, bool new_line=true, std::string *comment_out=nullptr, int lang=XV2_LANG_ENGLISH);
    bool WriteQCharParam(std::ostringstream &oss, const std::string &name, int qml_id, bool allow_negative, bool allow_non_existing, bool new_line=true, std::string *comment_out=nullptr);
    bool WriteInteractiveDialogueParam(std::ostringstream &oss, const std::string &name, int index, bool allow_negative, bool allow_non_existing, bool new_line=true, std::string *comment_out=nullptr);

    bool FindReferencedQxdChars();

    // Compiler helper
    void RemoveComments(ci_string &line) const;
    void SaveTokenAndReset(X2QcToken &token, int line_num, const std::string &file);
    void SaveTokenAndReset(X2QcToken &token, int line_num, const std::string &file, std::vector<X2QcToken> &tokens);
    void GetTokens(const ci_string &line, int line_num, const std::string &file);
    void Decompose(const std::string &str, const std::string &file);

    bool GetGroupTokens(const X2QcToken &group, std::vector<X2QcToken> &tokens);

    bool IsKnownType(const ci_string &id);
    bool IsTopType(const ci_string &id);

    bool LineError(const X2QcToken &token);
    bool ExpectTokenType(const X2QcToken &token, int expected_type);
    bool ExpectTokenType(const X2QcToken &token, const std::vector<int> &expected_types);

    bool GetParam(const X2QcToken &param_token, X2QcToken &value_token, const std::vector<ci_string> &params, const std::vector<int> &types, std::vector<bool> &defined);
    bool ProcessVarDeclaration(ci_string &ret);
    bool ExpectBracketBegin();
    bool ExpectColon();

    bool GetItemType(X2QcToken &token, uint16_t *item_type);

    XQ_X2mMod *FindModByVar(const ci_string &var_name);

    bool GetModCommon(const X2QcToken &token, XQ_X2mMod *mod);
    bool GetCostumeMod(const X2QcToken &token, int item_type, int *value);
    bool GetSkillMod(const X2QcToken &token, int16_t *id1);
    bool GetCharMod(const X2QcToken &token, int16_t *cms_id);
    bool GetCharMod(const X2QcToken &token, char *cms_code);
    bool GetStageMod(const X2QcToken &token, int16_t *stage_id);

    bool GetSkill(const X2QcToken &token, int16_t *id2, uint32_t *type, bool allow_blast);
    bool GetSuperSkill(const X2QcToken &token, int16_t *id2);
    bool GetUltimateSkill(const X2QcToken &token, int16_t *id2);
    bool GetEvasiveSkill(const X2QcToken &token, int16_t *id2);
    bool GetBlastSkill(const X2QcToken &token, int16_t *id2);
    bool GetAwakenSkill(const X2QcToken &token, int16_t *id2);
    bool GetChar(const X2QcToken &token, int16_t *cms_id);
    bool GetChar(const X2QcToken &token, char *cms_code);
    bool GetQuest(const X2QcToken &token, int16_t *quest_id, bool allow_self);
    bool GetStage(const X2QcToken &token, int16_t *stage_id);

    bool GetTitle(const X2QcToken &token, std::string &msg_code, int num);
    bool GetDialogueTA(const X2QcToken &token, std::string &msg_code);

    bool GetItemCollection(const X2QcToken &token, uint32_t *id);
    bool GetQxdChar(const X2QcToken token, uint32_t *id);
    bool GetQmlChar(const X2QcToken token, uint32_t *id, bool allow_negative);
    bool GetDialogue(const X2QcToken token, uint32_t *index, bool allow_negative);
    bool GetInteractiveDialogue(const X2QcToken token, uint32_t *index, bool allow_negative);
    bool GetFlag(const X2QcToken token, uint32_t *flag);
    bool GetStringVar(const X2QcToken token, uint32_t *string_var);

    // Decompile types
    bool DecompileX2mMod(const XQ_X2mMod &mod, std::ostringstream &oss);
    bool DecompileTextEntry(const std::string &msg_code, std::vector<MsgFile> &msg, bool audio, std::ostringstream &oss, bool title);

    bool DecompileItemCollectionEntry(const QxdCollectionEntry &entry, std::ostringstream &oss);
    bool DecompileItemCollection(uint32_t item_collection, std::ostringstream &oss);

    bool DecompileQxdUnk(const QxdUnk &unk, int type, std::ostringstream &oss);
    bool DecompileItemReward(const QxdItemReward &reward, std::ostringstream &oss);
    bool DecompileSkillReward(const QxdSkillReward &reward, std::ostringstream &oss);
    bool DecompileCharReward(const QxdCharReward &reward, std::ostringstream &oss);
    bool DecompileCharaPortrait(const QxdCharPortrait &portrait, std::ostringstream &oss);
    bool DecompileQuestStruct(const QxdQuest &quest, std::ostringstream &oss);

    bool DecompileQxdChar(const QxdCharacter &chara, bool special, std::ostringstream &oss);
    bool DecompileQmlChar(const QmlEntry &qchar, std::ostringstream &oss);

    bool DecompileDialoguePart(const QbtDialoguePart &part, uint16_t index, bool special_case, std::ostringstream &oss, int parent_index, int interactive_type);
    bool DecompileDialogue(const QbtEntry &dialogue, uint32_t index, std::ostringstream &oss);
    bool DecompileInteractiveDialogue(const QbtEntry &dialogue, uint32_t index, std::ostringstream &oss);
    bool DecompileEventDialogue(const QbtEntry &dialogue, uint32_t index, std::ostringstream &oss);

    bool DecompileCharPosition(const QslEntry &position, uint32_t stage, std::ostringstream &oss);
    bool DecompileInteractiveCharPosition(const QslEntry &position, uint32_t stage, std::ostringstream &oss);
    bool DecompileItemPosition(const QslEntry &position, uint32_t stage, std::ostringstream &oss);

    bool DecompileInstruction(const InstructionDef &def, const QedInstructionParams &params, std::ostringstream &oss);
    bool DecompileEvent(const QedEvent &event, int16_t event_num, std::ostringstream &oss, bool last);
    bool DecompileState(const QedState &state, size_t index, std::ostringstream &oss, bool last);
    bool DecompileScript(const QedFile &script, std::ostringstream &oss);

    // Compile types
    bool CompileX2mMod();
    bool CompileTextEntry(bool audio);

    bool CompileItemCollectionEntry(QxdCollectionEntry &entry);
    bool CompileItemCollection();

    bool CompileQxdUnk(QxdUnk &unk, int type);
    bool CompileItemReward(QxdItemReward &reward);
    bool CompileSkillReward(QxdSkillReward &reward);
    bool CompileCharReward(QxdCharReward &reward);
    bool CompileCharPortrait(QxdCharPortrait &portrait);
    bool CompileQuestStruct();

    bool CompileQxdChar(bool special);
    bool CompileQmlChar();

    bool CompileDialoguePart(QbtDialoguePart &part, bool *special_case_osq_0301, int *out_of_order, bool *dont_change=nullptr, bool *continue_next=nullptr);
    bool CompileDialogue();
    bool CompileInteractiveDialogue();
    bool CompileEventDialogue();

    bool CompileCharPosition();
    bool CompileInteractiveCharPosition();
    bool CompileItemPosition();

    bool CompileInstruction(const InstructionDef &def, QedInstructionParams &params, const X2QcToken &instruction_token);
    bool CompileEvent(QedEvent &event);
    bool CompileState(QedState &state);
    bool CompileScript(QedFile &script);

    bool CompileFlag();
    bool CompileStringVar();

public:

    Xv2QuestCompiler();

    inline void PushMod(const std::string &guid, const XQ_X2mMod &mod)
    {
        mods_table[guid] = mod;
        mods_table[guid].guid = guid;
    }

    inline void RemoveMod(const std::string &guid)
    {
        auto it = mods_table.find(guid);
        if (it != mods_table.end())
            mods_table.erase(guid);
    }

    inline void RemoveAllMods() { mods_table.clear(); }

    inline void SetTestMode(bool value) { test_mode = value; }

    bool DecompileQuest(const std::string &quest_name, std::ostringstream &qoss, std::ostringstream &coss, std::ostringstream &doss,
                        std::ostringstream &poss, std::vector<std::string> &svec);

    bool CompileQuest(const std::string &qstr, const std::string &qfile, const std::string &cstr, const std::string &cfile,
                      const std::string &dstr, const std::string &dfile, const std::string &pstr, const std::string &pfile,
                      const std::vector<std::string> &svec, const std::vector<std::string> &sfvec);

    inline std::string GetCompiledQuestName()
    {
        return compiled_quest.name;
    }

    bool IsVanilla() const; // Chek if last compiled quest is vanilla

    bool Maintenance();

    bool CommitCompiledFiles();
    bool CommitActiveQxd();
    bool CommitQsf();
    bool CommitTitle();
    bool CommitDialogue();
    bool CommitDialogueAudio();

    inline void SetResourceLoad(const std::string &path, X2mFile *x2m)
    {
        res_directory = path;
        res_x2m = x2m;
    }

    bool UninstallCompiledQuest(bool remove_empty_dir);

    //  Only for debug, don't use it otherwise
    QxdFile &DebugGetActiveQxd() { return active_qxd; }
    QmlFile &DebugGetActiveQml() { return active_qml; }
    QbtFile &DebugGetActiveQbt() { return active_qbt; }
    QslFile &DebugGetActiveQsl() { return active_qsl; }
    QedFile *DebugGetActiveQed(size_t idx) { return scripts[idx]; }

    void DebugDumpTitle(const std::string &path); // Requires a compiled quest
    void DebugDumpDialogue(const std::string &path); // Requires a compiled quest
};

#endif // XV2QUESTCOMPILER_H
