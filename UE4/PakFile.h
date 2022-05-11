#ifndef __PAKFILE_H__
#define __PAKFILE_H__

// TODO:
// ucs2 strings handling
// Compression and encryption

#include "BaseFile.h"
#include "UE4Common.h"
#include "FileStream.h"

#define PAK_SIGNATURE	0x5A6F12E1

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
	uint32_t signature; // 0
	int32_t version; // 4
	uint64_t index_offset; // 8
	uint64_t index_size; // 0x10
	uint8_t index_sha1[20]; // 0x18
} PACKED PAKFooter;

static_assert(sizeof(PAKFooter) == 0x2C, "Incorrect structure size.");

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct PakCompressedBlock
{
    uint64_t comp_start;
    uint64_t comp_end;
};

struct PakEntry
{
	uint64_t offset;
	uint64_t size;
	uint64_t uncompressed_size;	
	int32_t compression_method;
	uint8_t sha1[20];
	
    std::vector<PakCompressedBlock> comp_blocks;
	
	uint8_t encrypted;
	uint32_t compression_block_size;

    bool Read(Stream *stream, int version);
    bool Write(Stream *stream, int version, bool set_offset_to_zero) const;
	
	bool operator == (const PakEntry &rhs) const
	{
		return (//offset == rhs.offset && // Don't compare offsets... redundant file format is redundant...
				size == rhs.size &&
				uncompressed_size == rhs.uncompressed_size &&
				compression_method == rhs.compression_method &&
				memcmp(sha1, rhs.sha1, sizeof(sha1)) == 0 &&
				encrypted == rhs.encrypted &&
				compression_block_size == compression_block_size);
	}
	
	bool operator != (const PakEntry &rhs) const
	{
		return !(*this == rhs);
	}
};

struct PakFileEntry : PakEntry
{
    // 3-modes struct.
    // offset different to -1: internal file
    // offset = -1 and buf not null: memory file
    // offset = -1, buf = null: external file on external_path

    UE4Common::FString path;
	std:: string external_path; 
    uint8_t *buf;

    void Copy(const PakFileEntry &other)
    {
        offset = other.offset;
        size = other.size;

        uncompressed_size = other.uncompressed_size;
        compression_method = other.compression_method;
        memcpy(sha1, other.sha1, sizeof(sha1));

        encrypted = other.encrypted;
        compression_block_size = other.compression_block_size;

        path = other.path;
        external_path = other.external_path;

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

    PakFileEntry()
    {
        offset = -1;
        size = 0;

        uncompressed_size = 0;
        compression_method = 0;
        memset(sha1, 0, sizeof(sha1));

        encrypted = 0;
        compression_block_size = 0;

        buf = nullptr;
    }

    PakFileEntry(const PakFileEntry &other)
    {
        Copy(other);
    }

    ~PakFileEntry()
    {
        if (buf)
            delete[] buf;
    }

    inline PakFileEntry &operator=(const PakFileEntry &other)
    {
        if (this == &other)
            return *this;

        Copy(other);
        return *this;
    }

    bool GetSize(uint64_t *psize) const;
    bool GetRealSize(uint64_t *psize) const;
    bool Read(Stream *stream, int version);
    bool Write(Stream *stream, int version, bool set_offset_to_zero) const;
	
	// TODO: add Read and Write 
	
	bool operator == (const PakFileEntry &rhs) const
	{
        bool ret = (PakEntry::operator==(rhs) == 0 && path == rhs.path && external_path == rhs.external_path);

        if (ret)
        {
            if (!buf)
            {
                if (rhs.buf)
                    return false;

                return true;
            }

            return (memcmp(buf, rhs.buf, size) == 0);
        }

        return false;
	}
	
	bool operator != (const PakFileEntry &rhs) const
	{
		return !(*this == rhs);
	}
};

typedef bool (* SAVE_CALLBACK)(size_t processed_files, size_t num_files, uint64_t write_size, void *param);

class PakFile : public BaseFile
{
private:
		
    FileStream *fstream;
	
	int32_t version;
		
    UE4Common::FString mount_point;
	std::vector<PakFileEntry> files;

    uint8_t encryption_key[32];

    SAVE_CALLBACK save_callback;
    void *pc_param;

protected:

    void Reset();
    bool LoadCommon(Stream *stream);
    bool SaveCommon(Stream *stream);
    bool Uncompress(const PakFileEntry& entry, Stream *in, Stream *out, uint8_t *sha1=nullptr) const;
    bool ExtractCommon(const PakFileEntry &entry, Stream *stream, uint64_t size) const;
			
public:

	PakFile();
	virtual ~PakFile();
	
    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual bool LoadFromFile(const std::string &path, bool show_error=true) override;

    virtual uint8_t *Save(size_t *psize) override;
    virtual bool SaveToFile(const std::string &path, bool show_error=true, bool build_path=false) override;
	
    // TODO: handle ucs2 case...
    inline std::string GetMountPoint() const { return mount_point.str; }
    inline void SetMountPoint(const std::string &mount_point) { this->mount_point.str = mount_point; this->mount_point.is_ucs2 = false; }
	
	inline size_t GetNumFiles() const { return files.size(); }
	inline std::string GetFilePath(size_t idx)
	{
		if (idx >= files.size())
			return "";
		
        return files[idx].path.str;
	}

    inline bool GetFileSize(size_t idx, uint64_t *psize) const
    {
        if (idx >= files.size())
            return false;

        return files[idx].GetSize(psize);
    }

    uint64_t GetContentSize() const
    {
        uint64_t size = 0;

        for (const PakFileEntry &entry : files)
        {
            uint64_t tsize;

            if (entry.GetSize(&tsize))
                size += tsize;
        }

        return size;
    }

    bool UsesEncryption() const
    {
        for (const PakFileEntry &entry : files)
            if (entry.encrypted)
                return true;

        return false;
    }
	
	// extract_path_is_base_directory = false ->, extract_path is the absolute path (including filename) where the file gets extracted
	// extract_path_is_base_directory = true -> extract_path is the path of a base directory that gets the internal path of the file (pak entry) appended.
	bool ExtractFile(size_t idx, const std::string &extract_path, bool extract_path_is_base_directory) const;
	bool ExtractFile(const std::string &internal_path, const std::string &extract_path, bool extract_path_is_base_directory) const;
    uint8_t *ExtractFile(uint32_t idx, uint64_t *psize) const;
	
    bool AddFile(const std::string &file_path, const std::string &internal_path);

    inline void SetSaveCallback(SAVE_CALLBACK callback, void *param)
    {
        save_callback = callback;
        pc_param = param;
    }

    inline void SetEncryptionKey(const uint8_t *key)
    {
        memcpy(encryption_key, key, sizeof(encryption_key));
    }
};



#endif
