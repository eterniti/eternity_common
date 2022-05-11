#ifndef __CSOFILE_H__
#define __CSOFILE_H__

#include "BaseFile.h"

#define CSO_SIGNATURE   0x4F534323

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t header_size; // 6
    uint32_t num_entries; // 8
    uint32_t entries_start; // 0xC
} PACKED CSOHeader;

STATIC_ASSERT_STRUCT(CSOHeader, 0x10);

typedef struct
{
    uint32_t char_id; // 0
    uint32_t costume_id; // 4
    uint32_t se_offset; // 8
    uint32_t vox_offset; // 0xC
    uint32_t amk_offset; // Str offset
    uint32_t skills_offset; // Str offset
    uint64_t unk_18; // Zero
} CSOEntry;

STATIC_ASSERT_STRUCT(CSOEntry, 0x20);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct CsoEntry
{
    uint32_t char_id;
    uint32_t costume_id;
    std::string se;
    std::string vox;
    std::string amk;
    std::string skills;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

class CsoFile : public BaseFile
{
private:

    std::vector<CsoEntry> entries;

    void GenerateStringsList(std::vector<std::string> &str_list, size_t *str_size) const;

protected:

    void Reset();
    size_t CalculateFileSize(size_t str_size);

public:

    CsoFile();
    virtual ~CsoFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    inline size_t GetNumEntries() const { return entries.size(); }

    size_t FindEntriesByCharID(uint32_t char_id, std::vector<CsoEntry *> &cso_entries);
    bool AddEntry(const CsoEntry &entry, bool unique_char_id);
    size_t RemoveEntries(uint32_t char_id);

    inline const std::vector<CsoEntry> &GetEntries() const { return entries; }
    inline std::vector<CsoEntry> &GetEntries() { return entries; }

    inline const CsoEntry &operator[](size_t n) const { return entries[n]; }
    inline CsoEntry &operator[](size_t n) { return entries[n]; }

    inline std::vector<CsoEntry>::const_iterator begin() const { return entries.begin(); }
    inline std::vector<CsoEntry>::const_iterator end() const { return entries.end(); }

    inline std::vector<CsoEntry>::iterator begin() { return entries.begin(); }
    inline std::vector<CsoEntry>::iterator end() { return entries.end(); }

};

#endif // __CSOFILE_H__
