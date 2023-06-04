#ifndef __CUSFILE_H__
#define __CUSFILE_H__

#include <unordered_set>
#include "BaseFile.h"

#define CUS_SIGNATURE 0x53554323 // #CUS

// These are not the same as in idb
#define CUS_RACE_HUM    1
#define CUS_RACE_HUF    0x20
#define CUS_RACE_SYM    2
#define CUS_RACE_SYF    0x40
#define CUS_RACE_NMC    4
#define CUS_RACE_FRI    8
#define CUS_RACE_MAM    0x10
#define CUS_RACE_MAF    0x80

#define CUS_SUPER_START     0
#define CUS_ULTIMATE_START  5000
#define CUS_EVASIVE_START   10000
#define CUS_UNK_START       15000
#define CUS_BLAST_START     20000
#define CUS_AWAKEN_START    25000
#define CUS_ID_END          30000

#define NUM_SKILL_SLOTS 9

enum CusSkillType
{
    CUS_SKILL_TYPE_SUPER,
    CUS_SKILL_TYPE_ULTIMATE,
    CUS_SKILL_TYPE_EVASIVE,
    CUS_SKILL_TYPE_UNK,
    CUS_SKILL_TYPE_BLAST,
    CUS_SKILL_TYPE_AWAKEN
};

#pragma pack(push,1)

typedef struct
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t unk_06; // 6 pad
    uint32_t num_skillsets; // 8
    uint32_t skillset_offset; // 0xC
    uint32_t num_super; // 0x10
    uint32_t num_ultimate; // 0x14;
    uint32_t num_evasive; // 0x18
    uint32_t num_unk; // 0x1C
    uint32_t num_blast; // 0x20
    uint32_t num_awaken; // 0x24
    uint32_t super_offset; // 0x28
    uint32_t ultimate_offset; // 0x2C
    uint32_t evasive_offset; // 0x30
    uint32_t unk_offset; // 0x34
    uint32_t blast_offset; // 0x38
    uint32_t awaken_offset; // 0x3C
    uint64_t unk_40; // 0x40  zero
} PACKED CUSHeader;

STATIC_ASSERT_STRUCT(CUSHeader, 0x48);

typedef struct
{
    uint32_t char_id; // 0
    uint32_t costume_id; // 4
    uint16_t char_skills[NUM_SKILL_SLOTS]; // 8
    uint16_t model_preset; // 0x1A
    uint32_t unk_1C; // 0x1C check for zero
} PACKED CUSSkillSet;

STATIC_ASSERT_STRUCT(CUSSkillSet, 0x20);

struct CUSSkill
{
    char name[4];
    uint32_t unk_04; // Check for 0
    uint16_t id; // 8
    uint16_t id2; // 0xA
    uint8_t race_lock; // 0xC
    uint8_t type; // 0xD
    uint16_t unk_0E;
    uint16_t partset; // 0x10
    uint16_t unk_12; // 0x12
    uint32_t paths_offsets[7]; // 0x14
    uint16_t unk_30;
    uint16_t unk_32;
    uint16_t unk_34;
    uint16_t skill_type; // 0x36
    uint16_t pup_id; // 0x38
    uint16_t aura; // 0x3A
    uint16_t model; // 0x3C
    uint16_t change_skillset; // 0x3E
    uint32_t num_transforms; // 0x40
};
STATIC_ASSERT_STRUCT(CUSSkill, 0x44);

struct CUSSkillNew : CUSSkill
{
    uint32_t unk_44; // New in 1.19
};
STATIC_ASSERT_STRUCT(CUSSkillNew, 0x48);

#pragma pack(pop)

class CusFile;

struct CusSkillSet
{
    uint32_t char_id;
    uint32_t costume_id;
    uint16_t char_skills[9];
    uint16_t model_preset;

    CusSkillSet()
    {
        char_id = 0;
        costume_id = 0;
        memset(char_skills, 0xFF, sizeof(char_skills));
        model_preset = 0;
    }

    TiXmlElement *Decompile(TiXmlNode *root, const CusFile *owner=nullptr) const;
    bool Compile(const TiXmlElement *root);
};

struct CusSkill
{
    std::string name;
    uint16_t id;
    uint16_t id2;
    uint8_t race_lock;
    uint8_t type;
    uint16_t unk_0E;
    uint16_t partset;
    uint16_t unk_12;
    // 0 -> ean
    // 1 -> cam.ean
    // 2 -> eepk
    // 3 -> acb (SE)
    // 4 -> acb (VOX)
    std::string paths[7];
    uint16_t unk_30;
    uint16_t unk_32;
    uint16_t unk_34;
    uint16_t skill_type;
    uint16_t pup_id;
    uint16_t aura;
    uint16_t model;
    uint16_t change_skillset;
    uint32_t num_transforms;
    uint32_t unk_44; // New in 1.19

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root, bool *new_format=nullptr);

    CusSkill()
    {
        unk_44 = 0xFFFFFF00;
    }
};

class CusFile : public BaseFile
{
private:

    std::vector<CusSkillSet> skill_sets;
    std::vector<CusSkill> super_skills;
    std::vector<CusSkill> ultimate_skills;
    std::vector<CusSkill> evasive_skills;
    std::vector<CusSkill> unk_skills;
    std::vector<CusSkill> blast_skills;
    std::vector<CusSkill> awaken_skills;

    bool new_format;

    bool LoadSkills(const uint8_t *top, const CUSSkill *sets_in, std::vector<CusSkill> &sets_out, uint32_t num);
    bool LoadSkillsNew(const uint8_t *top, const CUSSkillNew *sets_in, std::vector<CusSkill> &sets_out, uint32_t num);
    void SaveSkills(uint8_t *top, char *str_top, char **str_current, const std::vector<CusSkill> &sets_in, CUSSkill *sets_out, std::unordered_set<std::string> &strings_list);
    void SaveSkillsNew(uint8_t *top, char *str_top, char **str_current, const std::vector<CusSkill> &sets_in, CUSSkillNew *sets_out, std::unordered_set<std::string> &strings_list);

    TiXmlElement *DecompileSkills(const char *name, TiXmlNode *root, const std::vector<CusSkill> &skills, int type) const;
    bool CompileSkills(const TiXmlElement *root, std::vector<CusSkill> &skills);

protected:

    void Reset();
    size_t CalculateStringsSize(std::unordered_set<std::string> &list, const std::vector<CusSkill> &skills) const;
    size_t CalculateFileSize() const;

public:

    CusFile();
    virtual ~CusFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    inline size_t GetNumeSkillSets() const { return skill_sets.size(); }

    inline const std::vector<CusSkillSet> &GetSkillSets() const { return skill_sets; }
    inline std::vector<CusSkillSet> &GetSkillSets() { return skill_sets; }

    inline const CusSkillSet &GetSkillSet(size_t idx) const { return skill_sets[idx]; }
    inline CusSkillSet &GetSkillSet(size_t idx) { return skill_sets[idx]; }

    inline const std::vector<CusSkill> &GetSuperSkills() const { return super_skills; }
    inline std::vector<CusSkill> &GetSuperSkills() { return super_skills; }

    inline const std::vector<CusSkill> &GetUltimateSkills() const { return ultimate_skills; }
    inline std::vector<CusSkill> &GetUltimateSkills() { return ultimate_skills; }

    inline const std::vector<CusSkill> &GetEvasiveSkills() const { return evasive_skills; }
    inline std::vector<CusSkill> &GetEvasiveSkills() { return evasive_skills; }

    inline const std::vector<CusSkill> &GetBlastSkills() const { return blast_skills; }
    inline std::vector<CusSkill> &GetBlastSkills() { return blast_skills; }

    inline const std::vector<CusSkill> &GetAwakenSkills() const { return awaken_skills; }
    inline std::vector<CusSkill> &GetAwakenSkills() { return awaken_skills; }

    size_t FindSkillSetsByCharID(uint32_t char_id, std::vector<CusSkillSet *> &sets);
    bool AddSkillSet(const CusSkillSet &set, bool unique_char_id);
    size_t RemoveSkillSets(uint32_t char_id);

    CusSkill *FindSuperSkillByID(uint32_t id);
    const CusSkill *FindSuperSkillByID(uint32_t id) const;

    CusSkill *FindUltimateSkillByID(uint32_t id);
    const CusSkill *FindUltimateSkillByID(uint32_t id) const;

    CusSkill *FindEvasiveSkillByID(uint32_t id);
    const CusSkill *FindEvasiveSkillByID(uint32_t id) const;

    CusSkill *FindBlastSkillByID(uint32_t id);
    const CusSkill *FindBlastSkillByID(uint32_t id) const;

    CusSkill *FindAwakenSkillByID(uint32_t id);
    const CusSkill *FindAwakenSkillByID(uint32_t id) const;

    CusSkill *FindSkillAnyByID(uint32_t id);
    const CusSkill *FindSkillAnyByID(uint32_t id) const;

    size_t FindSuperSkillByName(const std::string &name, std::vector<CusSkill *> &skills);
    size_t FindUltimateSkillByName(const std::string &name, std::vector<CusSkill *> &skills);
    size_t FindEvasiveSkillByName(const std::string &name, std::vector<CusSkill *> &skills);
    size_t FindBlastSkillByName(const std::string &name, std::vector<CusSkill *> &skills);
    size_t FindAwakenSkillByName(const std::string &name, std::vector<CusSkill *> &skills);
    size_t FindAnySkillByName(const std::string &name, std::vector<CusSkill *> &skills);

    bool IsSkillInUse(uint32_t char_id, int slot, int type) const;

    bool AddSuperSkill(const CusSkill &skill);
    bool AddUltimateSkill(const CusSkill &skill);
    bool AddEvasiveSkill(const CusSkill &skill);
    bool AddBlastSkill(const CusSkill &skill);
    bool AddAwakenSkill(const CusSkill &skill);

    bool AddSkillAuto(const CusSkill &skill);

    void RemoveSuperSkill(uint16_t id, bool *existed=nullptr);
    void RemoveUltimateSkill(uint16_t id, bool *existed=nullptr);
    void RemoveEvasiveSkill(uint16_t id, bool *existed=nullptr);
    void RemoveBlastSkill(uint16_t id, bool *existed=nullptr);
    void RemoveAwakenSkill(uint16_t id, bool *existed=nullptr);

    void RemoveSkillAuto(uint16_t id, bool *existed=nullptr);
    size_t RemoveAllReferencesToSkill(uint16_t id);

    size_t FindReferencesToPupId(uint16_t pup_id, std::vector<CusSkill *>&skills);

    inline bool IsNewFormat() const { return new_format; }
};

#endif // __CUSFILE_H__
