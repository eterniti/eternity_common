#ifndef XV2STAGEDEFFILE_H
#define XV2STAGEDEFFILE_H

// Note: THIS FILE REQUIRES A 64 BIT COMPILER.

#include <map>
#include <unordered_set>
#include "BaseFile.h"
#include "Utils.h"

#define XV2_ORIGINAL_NUM_STAGES         0x43 /* updated in 1.17 */
#define XV2_ORIGINAL_NUM_SS_STAGES      0x24 /* updated in 1.17 */
#define XV2_MAX_STAGES                  0x7E

#define XV2_STA_NUM_GATES   9	/* Updated in 1.11 */


// This should be moved to other file...
// These are the real language codes used by the game, unlike those from Xenoverse2.h
enum
{
    XV2_NATIVE_LANG_JAPANESE,
    XV2_NATIVE_LANG_ENGLISH,
    XV2_NATIVE_LANG_FRENCH,
    XV2_NATIVE_LANG_ITALIAN,
    XV2_NATIVE_LANG_GERMAN,
    XV2_NATIVE_LANG_SPANISH1,
    XV2_NATIVE_LANG_SPANISH2,
    XV2_NATIVE_LANG_PORTUGUESE,
    XV2_NATIVE_LANG_KOREAN,
    XV2_NATIVE_LANG_CHINESE1,
    XV2_NATIVE_LANG_CHINESE2,
    XV2_NATIVE_LANG_POLISH,
    XV2_NATIVE_LANG_RUSSIAN,
    XV2_NATIVE_LANG_NUM
};

extern const std::vector<std::string> xv2_native_lang_codes;

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

// All the native structures are between the pack push/pop

struct XV2StageDef1
{
    char *base_dir;
    char *code;
    char *dir;
    char *str4;
    uint64_t unk5; // Added in 1.11. It is an uint32_t, but due to alignment, let's make it a uint64_t. Known values: 0 (mostly), and 1 (BFLnc, BFlnd)
#ifdef CPU_X86
    uint32_t dummy[4]; // Just to make CHECK_STRUCT_SIZE not complain when being compiled in 32 bits version of genser.
#endif
} PACKED;
CHECK_STRUCT_SIZE(XV2StageDef1, 0x28);

struct XV2StageGate
{
    char *name;
    uint32_t target_stage_idx;
    uint32_t unk_0C;
    uint64_t unk_10; // This is really a 32 bits, the other 4 bytes are unused padding
#ifdef CPU_X86
    uint32_t dummy; // Just to make CHECK_STRUCT_SIZE not complain when being compiled in 32 bits version of genser.
#endif
} PACKED;
CHECK_STRUCT_SIZE(XV2StageGate, 0x18);

struct XV2StageDef2
{
    XV2StageGate gates[XV2_STA_NUM_GATES];
} PACKED;
CHECK_STRUCT_SIZE(XV2StageDef2, 0xD8);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct Xv2StageGate
{
    std::string name;
    uint32_t target_stage_idx;
    uint32_t unk_0C;
    uint64_t unk_10;

    Xv2StageGate()
    {
        target_stage_idx = 0xFFFFFFFF;
        unk_0C = 0;
        unk_10 = 1;
    }

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct Xv2Stage
{
    std::string base_dir;
    std::string code;
    std::string dir; // spm (inside subfolder)
    std::string str4; // unknown
    std::string eve;
    uint64_t unk5; // New in 1.11
    // /////////////////
    uint8_t ssid;
    // /////////////////
    float ki_blast_size_limit;
    // /////////////////
    Xv2StageGate gates[XV2_STA_NUM_GATES];    
    // /////////////////
    std::string se;
    // /////////////////
    uint32_t bgm_cue_id;
    // /////////////////
    std::vector<std::string> name; // Don't add to the build strings thing
    // /////////////////
    float override_far_clip;
    float limit;

    Xv2Stage()
    {
        base_dir = "data/stage/";
        unk5 = 0;
        ssid = 0xFF;
        ki_blast_size_limit = 200.0f;
        bgm_cue_id = 0;        
        name.resize(XV2_NATIVE_LANG_NUM);
        override_far_clip = 0.0f;
        limit = 500.0f;
    }

    inline const std::string &GetNativeName(int lang) const
    {
        return name[lang];
    }

    std::string GetName(int lang) const;
    void SetName(const std::string &stage_name, int lang);

    TiXmlElement *Decompile(TiXmlNode *root, uint32_t idx) const;
    bool Compile(const TiXmlElement *root);
};

class Xv2StageDefFile : public BaseFile
{
private:

    std::vector<Xv2Stage> stages;
    std::unordered_set<uint8_t> ssids_set;

protected:

    void Reset();

public:

    Xv2StageDefFile();
    virtual ~Xv2StageDefFile() override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    bool LoadFromDump(size_t stage_num, size_t base_addr, const void *base_ptr, void *def1_buf,
                      size_t playable_stage_num, const void *ssid_buf,
                      const void *f6_buf, const void *def2_buf, const void *sounds_buf, const void *music_buf,
                      const char **eve_dump);
    size_t BuildStrings(std::map<std::string, size_t> &map) const;
    void BuildSsidMap(void *addr) const;

    inline size_t GetNumStages() const { return stages.size(); }    
    inline size_t GetNumSsStages() const { return ssids_set.size(); }

    Xv2Stage *GetStageBySsid(uint8_t ssid);
    Xv2Stage *GetStageByCode(const std::string &code, size_t *idx=nullptr);

    size_t AddStage(Xv2Stage &stage, bool add_ssid, bool overwrite=true);
    void RemoveStage(Xv2Stage &stage);

    inline const std::vector<Xv2Stage> &GetStages() const { return stages; }
    inline std::vector<Xv2Stage> &GetStages() { return stages; }

    inline const Xv2Stage &operator[](size_t n) const { return stages[n]; }
    inline Xv2Stage &operator[](size_t n) { return stages[n]; }

    inline std::vector<Xv2Stage>::const_iterator begin() const { return stages.begin(); }
    inline std::vector<Xv2Stage>::const_iterator end() const { return stages.end(); }

    inline std::vector<Xv2Stage>::iterator begin() { return stages.begin(); }
    inline std::vector<Xv2Stage>::iterator end() { return stages.end(); }
};

#endif // XV2STAGEDEFFILE_H
