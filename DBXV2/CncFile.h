#ifndef CNCFILE_H
#define CNCFILE_H

#include "BaseFile.h"

#define CNC_SIGNATURE   0x434E4323

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED CNCHeader
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t num_entries; // 6
    uint32_t data_start; // 8
};
CHECK_STRUCT_SIZE(CNCHeader, 0xC);

struct PACKED CNCEntry
{
    uint16_t cms_id; // 0
    uint16_t costume_index; // 2
    uint16_t model_preset; // 4
    uint16_t cns_skills_ids[3]; // 6

    CNCEntry()
    {
        cms_id = 0xFFFF;
        costume_index = model_preset = 0;
        cns_skills_ids[0] = cns_skills_ids[1] = cns_skills_ids[2] = 0xFFFF;
    }

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(CNCEntry, 0xC);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

typedef CNCEntry CncEntry;

class CncFile : public BaseFile
{
private:

    std::vector<CncEntry> entries;

protected:

    void Reset();

public:
    CncFile();
    virtual ~CncFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    inline size_t GetNumEntries() const { return entries.size(); }

    size_t FindEntriesByCharID(uint32_t char_id, std::vector<CncEntry *> &cnc_entries);
    void AddEntry(const CncEntry &entry);
    size_t RemoveEntries(uint32_t char_id);

    inline const std::vector<CncEntry> &GetEntries() const { return entries; }
    inline std::vector<CncEntry> &GetEntries() { return entries; }

    inline const CncEntry &operator[](size_t n) const { return entries[n]; }
    inline CncEntry &operator[](size_t n) { return entries[n]; }

    inline std::vector<CncEntry>::const_iterator begin() const { return entries.begin(); }
    inline std::vector<CncEntry>::const_iterator end() const { return entries.end(); }

    inline std::vector<CncEntry>::iterator begin() { return entries.begin(); }
    inline std::vector<CncEntry>::iterator end() { return entries.end(); }
};

#endif // CNCFILE_H
