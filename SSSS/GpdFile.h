#ifndef __GPDFILE_H__
#define __GPDFILE_H__

#include <vector>

#include "CmsFile.h"
#include "SlotsFile.h"

// "#GPD"
#define GPD_SIGNATURE   0x44504723

typedef struct
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t unk_06; // 6
    uint32_t num_entries; // 8
    uint32_t unk_0C; // 0xC
    uint32_t data_start; // 0x10
    uint32_t unk_14; // 0x14
    // size 0z18
} __attribute__((packed)) GPDHeader;

typedef struct
{
    uint32_t unk_00; // 0   Seems to be always id+1
    uint32_t id; // 4  Always matches the position in the file
    float unk_08; // 8
    float unk_0C; // 0xC
    float unk_10; // 0x10
    float unk_14; // 0x14
    uint32_t unk_18; // 0x18
    uint32_t unk_1C; // 0x1C
    uint32_t unk_20; // 0x20
    uint32_t unk_24; // 0x24
    uint32_t unk_28; // 0x28
    uint32_t unk_2C; // 0x2C
    uint32_t unk_30; // 0x30
    uint32_t unk_34; // 0x34
    uint32_t unk_38; // 0x38
    uint32_t unk_3C; // 0x3C
    // size 0x40
} __attribute__((packed)) GPDEntry;

typedef struct
{
    uint32_t cms_entry; // 0
    uint32_t cms_model_spec_idx; // 4
    uint32_t gpd_entry; // 8
    uint32_t unk_0C; // 0xC seems to be pad...
    // size 0x10
} __attribute__((packed)) GWLEntry;

struct GpdEntry
{
    uint32_t unk_00;
    uint32_t id;
    float unk_08;
    float unk_0C;
    float unk_10;
    float unk_14;
    uint32_t unk_18;
    uint32_t unk_1C;
    uint32_t unk_20;
    uint32_t unk_24;
    uint32_t unk_28;
    uint32_t unk_2C;
    uint32_t unk_30;
    uint32_t unk_34;
    uint32_t unk_38;
    uint32_t unk_3C;   

    void Decompile(TiXmlNode *root, const CharacterDef *def, const std::vector<uint32_t> *tours_ids) const;
    bool Compile(TiXmlElement *root, CharacterDef **pdef, std::vector<uint32_t> **ptours_ids);
};

class GpdFile;
class GwdFile;

class GWLFinder
{
private:

    uint32_t cms_entry;
    uint32_t cms_model_spec_idx;

public:

    GWLFinder(uint32_t cms_entry, uint32_t cms_model_spec_idx) : cms_entry(cms_entry), cms_model_spec_idx(cms_model_spec_idx) { }
    bool operator()(const GWLEntry & entry)
    {
        return (entry.cms_entry == cms_entry && entry.cms_model_spec_idx == cms_model_spec_idx);
    }
};

class GWLComparer
{
private:

    const GpdFile *gpd;

public:

    GWLComparer(const GpdFile *gpd) : gpd(gpd) { }
    bool operator()(const GWLEntry &info1, const GWLEntry &info2);
};

class GpdFile : public BaseFile
{
private:

    std::vector<GpdEntry> entries;
    std::vector<CharacterDef> char_map;
    std::vector<std::vector<uint32_t>> gwd_tournaments;

    void Reset();

    friend class GWLComparer;

public:

    GpdFile();
    virtual ~GpdFile();

    static bool GetTournamentNames(const std::vector<uint32_t> &ids, std::vector<std::string> &names);
    static bool GetTournamentsIds(const std::vector<std::string> &names, std::vector<std::uint32_t> &ids);

    inline uint32_t GetNumEntries() const
    {
        return entries.size();
    }

    uint32_t FindEntry(uint32_t cms_entry, uint32_t cms_model_spec_idx);

    GpdEntry *GetEntry(uint32_t idx, uint32_t *cms_entry=nullptr, uint32_t *cms_model_spec_idx=nullptr, std::vector<uint32_t> *tours_ids=nullptr);
    const GpdEntry *GetEntry(uint32_t idx, uint32_t *cms_entry=nullptr, uint32_t *cms_model_spec_idx=nullptr, std::vector<uint32_t> *tours_ids=nullptr) const;

    bool SetEntry(uint32_t idx, const GpdEntry &gpd_entry, uint32_t cms_entry = 0xFFFFFFFF, uint32_t cms_model_spec_idx=0xFFFFFFFF, std::vector<uint32_t> *tours_ids=nullptr);
    bool AppendEntry(const GpdEntry &gpd_entry, uint32_t cms_entry = 0xFFFFFFFF, uint32_t cms_model_spec_idx = 0xFFFFFFFF, std::vector<uint32_t> *tours_ids=nullptr);

    bool DeleteEntry(uint32_t idx);

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *size) override;

    bool SetExtraData(const uint8_t *galaxian_wars_list, unsigned int size);
    uint8_t *CreateGalaxianWarsList(unsigned int *psize, CmsFile *check=nullptr);

    bool LoadGwdTournaments(const GwdFile &gwd);
    bool SetGwdTournaments(GwdFile &gwd) const;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;
};

#endif
