#ifndef IKDFILE_H
#define IKDFILE_H

#include "BaseFile.h"

#define IKD_SIGNATURE   0x444B4923

;
#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED IKDHeader
{
    uint32_t signature;
    uint16_t endianess_check; // 4
    uint16_t header_size; // 6
    uint32_t num_entries; // 8
    uint32_t data_start; // 0xC
};
CHECK_STRUCT_SIZE(IKDHeader, 0x10);

struct PACKED IKDEntry
{
    uint16_t cms_id;
    uint16_t costume_id; // 2
    float unk_08[12];
    uint32_t unk_34;
    uint32_t unk_38;

    IKDEntry()
    {
        memset(this, 0, sizeof(IKDEntry));
    }

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(IKDEntry, 0x3C);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

typedef IKDEntry IkdEntry;

class IkdFile : public BaseFile
{
private:

    std::vector<IkdEntry> entries;

protected:

    void Reset();

public:

    IkdFile();
    virtual ~IkdFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    inline size_t GetNumEntries() const { return entries.size(); }

    IkdEntry *FindEntry(uint32_t char_id, uint32_t costume_id);
    size_t FindEntries(uint32_t char_id, std::vector<IkdEntry *> &ret);

    inline void AddEntry(const IkdEntry &entry) { entries.push_back(entry); }
    size_t RemoveAllReferencesToChar(uint32_t char_id);

    const std::vector<IkdEntry> &GetEntries() const { return entries; }
    std::vector<IkdEntry> &GetEntries() { return entries; }

    inline const IkdEntry &operator[](size_t n) const { return entries[n]; }
    inline const IkdEntry &operator[](size_t n) { return entries[n]; }

    inline std::vector<IkdEntry>::const_iterator begin() const { return entries.begin(); }
    inline std::vector<IkdEntry>::const_iterator end() const { return entries.end(); }

    inline std::vector<IkdEntry>::iterator begin() { return entries.begin(); }
    inline std::vector<IkdEntry>::iterator end() { return entries.end(); }
};

#endif // IKDFILE_H
