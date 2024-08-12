#ifndef __CMSFILE_H__
#define __CMSFILE_H__

#include "BaseFile.h"

#ifdef SSSS
#include "SSSS/SsssData.h"
#endif

// "#CMS"
#define CMS_SIGNATURE	0x534D4323

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

#define XV2_FREE_ID_SEARCH_START 0x9F
//#define XV2_LAST_KNOWN_CMS  0xD0
//#define XV2_FREE_ID_SEARCH_START 0x6D

// Low-level objects

typedef struct
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t unk_06; // 6       zero
    uint32_t num_entries; // 8
    uint32_t unk_0C;  // C      zero
    uint32_t data_start; // 0x10
    // pad until data_start
} PACKED CMSHeaderSSSS;

STATIC_ASSERT_STRUCT(CMSHeaderSSSS, 0x14);

typedef struct
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t unk_06; // 6       zero
    uint32_t num_entries; // 8
    uint32_t data_start; // 0xC
} PACKED CMSHeaderXV;

STATIC_ASSERT_STRUCT(CMSHeaderXV, 0x10);

typedef struct
{
    uint32_t id; // 0
    char name[4]; // 4
    uint32_t type; // 8
    uint32_t num_models; // C
    uint32_t models_spec_offset; // 0x10   offset is absolute!
    uint32_t unk_14[3]; // 0x14   always zero
    // 0x20
} PACKED CMSEntrySSSS;

STATIC_ASSERT_STRUCT(CMSEntrySSSS, 0x20);

typedef struct
{
    // all offsets are absolute within the file!
    uint64_t bba0_name_offset; // 0
    uint64_t bba1_name_offset; // 8
    uint64_t bba2_name_offset; // 0x10
    uint64_t bba3_name_offset; // 0x18
    uint64_t bba4_name_offset; // 0x20
    uint64_t bba5_name_offset; // 0x28
    uint64_t emo_file_offset; // 0x30
    uint64_t emb_file_offset; // 0x38
    uint64_t emm_file_offset; // 0x40
    uint64_t ema_file_offset; // 0x48
    uint64_t menu_file_offset; // 0x50
    uint64_t unk_58; // 0x58 always 0
    uint64_t fce_file_offset; // 0x60
    uint64_t matbas_file_offset; // 0x68
    uint64_t vc_file_offset; // 0x70
    uint64_t _2ry_file_offset; // 0x78
    uint64_t fma_file_offset; // 0x80
    uint64_t fdm_file_offset; // 0x88
    uint64_t fcm_file_offset; // 0x90
    uint64_t unk_98; // 0x98 always 0
    uint64_t vfx_file_offset; // 0xA0
    uint64_t tdb_file_offset; // 0xA8
    uint64_t bpm_file_offset; // 0xB0
    uint64_t vc_name_offset; // 0xB8
    uint64_t se_name_offset; // 0xC0
    uint32_t idx; // 0xC8  seems important
    uint32_t model_id; // 0xCC  this is also important
    uint32_t unk_D0; // 0xD0  not 0
    uint32_t unk_D4; // 0xD4  not 0
    float 	scale; // 0xD8  possibly, scale
    uint32_t cosmo; // 0xDC
    float unk_E0; // 0xE0
    float unk_E4; // 0xE4
    float unk_E8; // 0xE8
    float unk_EC; // 0xEC
    float unk_F0; // 0xF0
    uint32_t aura; // 0xF4
    uint64_t unk_F8; // always 0
    uint64_t unk_100; // always 0
    // size = 0x108
} PACKED CMSModelSpecSSSS;

STATIC_ASSERT_STRUCT(CMSModelSpecSSSS, 0x108);

typedef struct
{
    uint32_t id; // 0
    char name[4]; // 4
    uint64_t unk_08;
    uint32_t unk_10;
    uint16_t load_cam_dist;
    uint16_t unk_16;
    uint16_t unk_18;
    uint16_t unk_1A;
    uint32_t unk_1C; // Zero
    uint32_t character_offset; // 0x20 String ptr
    uint32_t ean_offset; // 0x24 String ptr
    uint32_t unk_28; // Zero
    uint32_t fce_ean_offset; // 0x2C String ptr
    uint32_t fce_offset; // 0x30 String ptr
    uint32_t unk_34; // Zero
    uint32_t cam_ean_offset; // String ptr
    uint32_t bac_offset; // String ptr
    uint32_t bcm_offset; // String ptr
    uint32_t ai_offset; // String ptr
    uint64_t unk_48; // Zero
} PACKED CMSEntryXV;

STATIC_ASSERT_STRUCT(CMSEntryXV, 0x50);

typedef struct
{
    uint32_t id; // 0
    char name[4]; // 4
    uint64_t unk_08; // Zero
    uint32_t unk_10;
    uint16_t load_cam_dist;
    uint16_t unk_16;
    uint16_t unk_18;
    uint16_t unk_1A;
    uint32_t unk_1C; // It used to be always 0. Starting from 1.17.2, the value 1 is observed in several entries.
    uint32_t character_offset; // 0x20 String ptr
    uint32_t ean_offset; // 0x24 String ptr
    uint32_t unk_28; // Zero
    uint32_t fce_ean_offset; // 0x2C String ptr
    uint32_t fce_offset; // 0x30 String ptr
    uint32_t unk_34; // Zero
    uint32_t cam_ean_offset; // String ptr
    uint32_t bac_offset; // 0x3C String ptr
    uint32_t bcm_offset; // String ptr
    uint32_t ai_offset; // String ptr
    uint64_t unk_48; // Zero
    uint32_t bdm_offset; // String ptr
} PACKED CMSEntryXV2;

STATIC_ASSERT_STRUCT(CMSEntryXV2, 0x54);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

// High level objects

struct CmsEntry
{
    uint32_t id;
    std::string name;

    virtual ~CmsEntry() {}

    bool CompileCommon(const TiXmlElement *root);

    virtual TiXmlElement *Decompile(TiXmlNode *root) const;
    virtual bool Compile(const TiXmlElement *root);
};

struct CmsModelSpecSSSS
{
    std::string bba0;
    std::string bba1;
    std::string bba2;
    std::string bba3;
    std::string bba4;
    std::string bba5;
    std::string emo_file;
    std::string emb_file;
    std::string emm_file;
    std::string ema_file;
    std::string menu_file;
    std::string fce_file;
    std::string matbas_file;
    std::string vc_file;
    std::string _2ry_file;
    std::string fma_file;
    std::string fdm_file;
    std::string fcm_file;
    std::string vfx_file;
    std::string tdb_file;
    std::string bpm_file;
    std::string vc_name;
    std::string se_name;
    uint32_t idx;
    uint32_t model_id;
    uint32_t unk_D0;
    uint32_t unk_D4;
    float scale;
    uint32_t cosmo;
    float unk_E0;
    float unk_E4;
    float unk_E8;
    float unk_EC;
    float unk_F0;
    uint32_t aura;

    void Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct CmsEntrySSSS : CmsEntry
{
    uint32_t type;
    std::vector<CmsModelSpecSSSS> specs;

    virtual ~CmsEntrySSSS() {}

    virtual TiXmlElement *Decompile(TiXmlNode *root) const;
    virtual bool Compile(const TiXmlElement *root);
};

struct CmsEntryXV : CmsEntry
{
    uint32_t unk_10;
    uint16_t load_cam_dist;
    uint16_t unk_16;
    uint16_t unk_18;
    uint16_t unk_1A;
    uint32_t unk_1C;
    std::string character;
    std::string ean;
    std::string fce_ean;
    std::string fce;
    std::string cam_ean;
    std::string bac;
    std::string bcm;
    std::string ai;

    virtual ~CmsEntryXV() {}

    virtual TiXmlElement *Decompile(TiXmlNode *root) const;
    virtual bool Compile(const TiXmlElement *root);
};

struct CmsEntryXV2 : CmsEntryXV
{
    std::string bdm;

    virtual ~CmsEntryXV2() {}

    virtual TiXmlElement *Decompile(TiXmlNode *root) const;
    virtual bool Compile(const TiXmlElement *root);
};

enum class CmsType
{
    CMS_SSSS,
    CMS_XV,
    CMS_XV2
};

class CmsFile : public BaseFile
{
private:

    std::vector<CmsEntry *> entries;
    CmsType type;

    void GenerateStrList(std::vector<std::string> &list) const;
    size_t CalculateFileSize(const std::vector<std::string> &list, uint32_t *strings_offset) const;
    uint32_t GetStringOffset(const std::vector<std::string> &list, const std::string &str) const;

protected:

    void Reset();

public:

    CmsFile();

    virtual ~CmsFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;


    inline bool IsSSSS() const { return (type == CmsType::CMS_SSSS); }
    inline bool IsXV() const { return (type == CmsType::CMS_XV); }
    inline bool IsXV2() const { return (type == CmsType::CMS_XV2); }

    size_t GetNumEntries() const { return entries.size(); }
    CmsEntry *FindEntryByName(const std::string &name);
    CmsEntry *FindEntryByID(uint32_t id);

    bool AddEntryXV2(CmsEntryXV2 &entry, bool auto_id);
    // Returns true too if the entry doesn't exist
    bool RemoveEntry(const std::string &name, bool *existed=nullptr);

    inline CmsEntry *const &operator[](size_t n) const { return entries[n]; }
    inline CmsEntry *&operator[](size_t n) { return entries[n]; }

    inline std::vector<CmsEntry *>::const_iterator begin() const { return entries.begin(); }
    inline std::vector<CmsEntry *>::const_iterator end() const { return entries.end(); }

    inline std::vector<CmsEntry *>::iterator begin() { return entries.begin(); }
    inline std::vector<CmsEntry *>::iterator end() { return entries.end(); }
};

#endif // __CMSFILE_H__
