#ifndef __CPKFILE_H__
#define __CPKFILE_H__

#include <unordered_set>
#include "AwbFile.h"
#include "UtfFile.h"
#include "FileStream.h"

// "CPK "
#define CPK_SIGNATURE	0x204B5043

// "TOC "
#define TOC_SIGNATURE	0x20434F54

// "ITOC"
#define ITOC_SIGNATURE  0x434F5449

// "ETOC"
#define ETOC_SIGNATURE  0x434F5445

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
	uint32_t signature; // 0
	uint32_t unk_04; // 4    
} PACKED CPKHeader;

STATIC_ASSERT_STRUCT(CPKHeader, 8);

typedef struct
{
    uint32_t signature; // 0
    uint32_t unk_04;
} PACKED TOCHeader;

STATIC_ASSERT_STRUCT(TOCHeader, 8);

typedef struct
{
    uint32_t signature; // 0
    uint32_t unk_04;
} PACKED ITOCHeader;

STATIC_ASSERT_STRUCT(ITOCHeader, 8);

typedef struct
{
    uint32_t signature; // 0
    uint32_t unk_04;
} PACKED ETOCHeader;

STATIC_ASSERT_STRUCT(ETOCHeader, 8);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct CpkEntry
{
    // This struct has 3 modes of operation
    // Internal file: offset points to the position of the file
    // Memory file: offset is -1, file content is in buf
    // External file: offset is -1, buf is nullptr, and external_path points to the external file.

    uint64_t offset;
    uint32_t size;
    uint32_t compressed_size;

    uint8_t *buf; // WARNING: allocated
    std::string external_path;

    std::string file_name;
    std::string dir_name;

    uint64_t update_date_time;
    uint32_t id;

    uint32_t toc_index; // Only used in reordering entries

    bool has_name;
    bool has_date;
    bool has_id;

    void Copy(const CpkEntry &other)
    {
        offset = other.offset;
		size = other.size;

        external_path = other.external_path;
        file_name = other.file_name;
        dir_name = other.dir_name;

        update_date_time = other.update_date_time;
        id = other.id;

        toc_index = other.toc_index;

        has_name = other.has_date;
        has_date = other.has_date;
        has_id = other.has_id;

        if (other.buf)
        {
            buf = new uint8_t[other.size];
            memcpy(buf, other.buf, other.size);
            size = other.size;
        }
        else
        {
            buf = nullptr;           
        }
    }

    CpkEntry()
    {
        offset = -1;
        size = compressed_size = 0;

        buf = nullptr;
        update_date_time = -1;
        id = -1;

        has_name = false;
        has_date = false;
        has_id = false;
    }

    CpkEntry(const CpkEntry &other)
    {
        Copy(other);
    }

    ~CpkEntry()
    {
        if (buf)
            delete[] buf;
    }

    inline CpkEntry &operator=(const CpkEntry &other)
    {
        if (this == &other)
            return *this;

        Copy(other);
        return *this;
    }

    bool GetSize(uint32_t *psize) const;
};

class CpkFile : public AwbFile
{
private:

    FileStream *fstream;

    UtfFile cpk_header;
    UtfFile toc;
    UtfFile itoc;
    UtfFile etoc;
    UtfFile datah, datal;

    bool use_encryption;
    bool has_toc;
    bool has_itoc;
    bool has_etoc;
    bool is_itoc_extend;
    uint32_t files_h, files_l;

    uint64_t content_offset;
    uint16_t align;

    // From the serveral headers
    uint32_t unk_04;
    uint32_t toc_unk_04;
    uint32_t itoc_unk_04;
    uint32_t etoc_unk_04;

    std::vector<CpkEntry> entries;

    // For fileexists cache
    mutable bool cache_built = false;
    mutable std::unordered_set<std::string> cache;

    void ToggleEncryption(uint8_t *buf, uint64_t size);
    bool LoadTable(Stream *stream, UtfFile *table, const std::string &name, const std::string &alt_name="");
    bool SaveTable(Stream *stream, UtfFile *table, const std::string &name);

    bool ReadItocEntry(uint32_t id, CpkEntry &entry);
    bool WriteItocEntry(uint32_t id, const CpkEntry &entry);

protected:

	void Reset();
    bool LoadCommon(Stream *stream);

    uint64_t GetContentSize(bool final_align) const;
    uint64_t GetEnabledDatasize() const;

    void UpdateToc();
    void UpdateItoc();
    void UpdateEtoc();
    void UpdateHeader();

    bool SaveCommon(Stream *stream);

    bool ExtractCrylaila(Stream *input, Stream *output, uint32_t compressed_size, uint32_t uncompressed_size) const;
    bool ExtractCommon(const CpkEntry &entry, Stream *stream, uint32_t size) const;
    std::string GetPath(uint32_t idx, uint32_t file_size) const;
	
public:

	CpkFile();
	virtual ~CpkFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual bool LoadFromFile(const std::string &path, bool show_error=true) override;
    virtual bool SaveToFile(const std::string &path, bool show_error=true, bool build_path=false) override;

    // Not implemented
    virtual uint8_t *CreateHeader(unsigned int *psize, bool extra_word=true)
    {
        UNUSED(psize); UNUSED(extra_word);
        return nullptr;
    }

    virtual uint32_t GetNumFiles() const { return (uint32_t)entries.size(); }
    virtual bool GetFileSize(uint32_t idx, uint64_t *psize) const
    {
        if (idx >= entries.size())
            return false;

        uint32_t entry_size;

        bool ret = entries[idx].GetSize(&entry_size);
        if (ret)
            *psize = entry_size;

        return ret;
    }

    bool GetFileSize(const std::string &path_in_cpk, uint64_t *psize) const;

    virtual bool ExtractFile(uint32_t idx, const std::string &path, bool auto_path=false) const;
    virtual uint8_t *ExtractFile(uint32_t idx, uint64_t *psize) const;

    uint32_t FindEntryByPath(const std::string &path) const;
    bool FileExists(const std::string &path) const;
    uint8_t *ExtractFile(const std::string &path_in_cpk, uint64_t *psize) const;

    virtual bool SetFile(uint32_t idx, void *buf, uint64_t size, bool take_ownership=false);
    virtual bool SetFile(uint32_t idx, const std::string &path);

    // TODO: implement these two (needed also to support creation of new cpk files)
    virtual bool AddFile(const std::string &path)
    {
        UNUSED(path);
        return false;
    }

    virtual bool AddFile(void *buf, uint64_t size, bool take_ownership=false)
    {
        UNUSED(buf); UNUSED(size); UNUSED(take_ownership);
        return false;
    }

    inline uint64_t GetContentSize() const
    {
        uint64_t total_size = 0;

        for (const CpkEntry &entry: entries)
            total_size += entry.size;

        return total_size;
    }

    bool GetFilePath(uint32_t idx, std::string &path) const;
    bool GetParentDirectory(uint32_t idx, std::string &path) const;
};

#endif // __CPKFILE_H__
