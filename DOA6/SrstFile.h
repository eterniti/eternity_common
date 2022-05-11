#ifndef SRSTFILE_H
#define SRSTFILE_H

#include "FileStream.h"
#include "SrsaFile.h"

#define SRST_SIGNATURE  0x53525354 // TSRS
#define RSTK_SIGNATURE  0x5253544B // KTSR

#define SRST_ENTRY_SIGNATURE    0x15F4D409

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED SRSTHeader
{
    uint32_t srst_signature; // 0
    uint32_t unk_04; //Zero observed
    uint64_t file_size; // 8 - File size including this header
    KTSRHeader ktsr; // 0x10
};
CHECK_STRUCT_SIZE(SRSTHeader, 0x50);

struct PACKED SRSTEntry
{
    uint32_t signature; // 0
    uint32_t entry_size; // 4 - Entry size including this header
    uint32_t id; // 8 - Hash of filename (matching the one in the srsa)
    uint32_t header_size; // 0x0C - Always 0x20?
    uint32_t kovs_size; // 0x10 - KOVS size (ogg size + sizeof(KOVSEntry), the padded to 0x10 part is not included)
    uint8_t unk_14[0x0C];
};
CHECK_STRUCT_SIZE(SRSTEntry, 0x20);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

typedef void (* SRSTProgressFunction)(size_t current, size_t total);

struct SrstEntry
{
    uint64_t srst_offset;
    uint32_t srst_size;
    uint64_t data_offset;
    uint32_t data_size;

    uint32_t id;
    std::string name;

    Stream *external;
	
    SrstEntry()
    {
        srst_offset = 0;
        srst_size = 0;
        data_offset = 0;
        data_size = 0;
        id = 0;

        external = nullptr;
    }

    ~SrstEntry()
    {
        if (external)
            delete external;
    }
};

class SrstFile : BaseFile
{
protected:

    FileStream *cont;
    std::vector<SrstEntry> entries;
    bool is_raw;
    SRSTProgressFunction progress;

    SrsaFile *srsa;

    void Reset();
    bool LoadInternal();
    bool SaveInternal(FileStream *out);

    bool LoadRawInternal();
    bool IsSameOggAsExternal(SrstEntry &entry);


public:
    SrstFile();
    virtual ~SrstFile() override;

    virtual bool LoadFromFile(const std::string &path, bool show_error=true) override;
    virtual bool SaveToFile(const std::string &path, bool show_error=true, bool build_path=false) override;

    bool LoadFromRaw(const std::string &path, bool show_error=true);

    inline size_t GetNumEntries() const { return entries.size(); }

    bool Extract(size_t idx, Stream *stream);
    bool Extract(size_t idx, const std::string &dir_path);

    bool SetExternal(size_t idx, const std::string &path);
    inline void SetSRSA(SrsaFile *srsa) { this->srsa = srsa; }

    inline void SetSaveProgress(SRSTProgressFunction pf) { progress = pf; }

    inline const std::vector<SrstEntry> &GetEntries() const { return entries; }
    inline std::vector<SrstEntry> &GetEntries() { return entries; }

    inline const SrstEntry &operator[](size_t n) const { return entries[n]; }
    inline SrstEntry &operator[](size_t n) { return entries[n]; }

    inline std::vector<SrstEntry>::const_iterator begin() const { return entries.begin(); }
    inline std::vector<SrstEntry>::const_iterator end() const { return entries.end(); }

    inline std::vector<SrstEntry>::iterator begin() { return entries.begin(); }
    inline std::vector<SrstEntry>::iterator end() { return entries.end(); }
};

#endif // SRSTFILE_H
