#ifndef VLCFILE_H
#define VLCFILE_H

#include "BaseFile.h"

;
#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED VLCHeader
{
    uint32_t num_entries;
    uint32_t unk_04[3]; // ZERO
};
CHECK_STRUCT_SIZE(VLCHeader, 0x10);

struct PACKED VLCEntry
{
    float x;
    float y;
    float z;
    float cms_id;
};
CHECK_STRUCT_SIZE(VLCEntry, 0x10);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct VlcEntry
{
    float x, y, z;
    float x2, y2, z2;
    uint32_t cms_id;

    VlcEntry()
    {
        cms_id = 0xFFFFFFFF;
        x = y = z = 0.0f;
        x2 = y2 = z2 = 0.0f;
    }

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

class VlcFile : public BaseFile
{
private:

    std::vector<VlcEntry> entries;

protected:
    void Reset();

public:

    VlcFile();
    virtual ~VlcFile();

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    VlcEntry *FindEntry(uint32_t cms_id);
    bool SetEntry(const VlcEntry &entry, bool add_if_needed=true);
    void RemoveEntry(uint32_t cms_id);

    inline size_t GetNumEntries() const { return entries.size(); }

    const std::vector<VlcEntry> &GetEntries() const { return entries; }
    std::vector<VlcEntry> &GetEntries() { return entries; }

    inline const VlcEntry &operator[](size_t n) const { return entries[n]; }
    inline const VlcEntry &operator[](size_t n) { return entries[n]; }

    inline std::vector<VlcEntry>::const_iterator begin() const { return entries.begin(); }
    inline std::vector<VlcEntry>::const_iterator end() const { return entries.end(); }

    inline std::vector<VlcEntry>::iterator begin() { return entries.begin(); }
    inline std::vector<VlcEntry>::iterator end() { return entries.end(); }
};

#endif // VLCFILE_H
