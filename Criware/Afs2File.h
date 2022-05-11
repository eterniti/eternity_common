#ifndef __AFS2FILE_H__
#define __AFS2FILE_H__

#include "AwbFile.h"
#include "debug.h"

// "AFS2"
#define AFS2_SIGNATURE	0x32534641 

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
	uint32_t signature;  // 0
	uint8_t version[4]; // 4      01 04 02 00 or 01 02 02
	uint32_t num_files; // 8
	uint32_t alignment;	// 0xC
	// size 0x10
} PACKED AFS2Header;

static_assert(sizeof(AFS2Header) == 0x10, "Incorrect structure size.");

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct Afs2Entry
{
	// This struct has 3 modes of operation
	// Internal file: offset points to the position of the file
	// Memory file: offset is -1, file content is in buf
	// External file: offset is -1, buf is nullptr, and path points to the external file.
	
	uint32_t offset;
	uint32_t size;	
	
	uint8_t *buf; // WARNING: allocated
	std::string path;
	
	void Copy(const Afs2Entry &other)
	{
		offset = other.offset;
		size = other.size;
		path = other.path;
				
		if (other.buf)
		{
			buf = new uint8_t[other.size];
			memcpy(buf, other.buf, other.size);					
		}
		else
		{
			buf = nullptr;
		}
	}

	Afs2Entry()
	{
		offset = -1;
		buf = nullptr;
	}
	
	Afs2Entry(const Afs2Entry &other) 
    {
        Copy(other);
    }
	
	~Afs2Entry()
	{
		if (buf)
			delete[] buf;
	}
	
	inline Afs2Entry &operator=(const Afs2Entry &other)
    {
        if (this == &other)
            return *this;

        Copy(other);
        return *this;
    }	
};

class Afs2File : public AwbFile
{
private:

	FILE *r_handle;	
	
	uint8_t version[4];
	uint32_t alignment;
	
	std::vector<Afs2Entry> files;
	
	void Reset();
	
	void GetOffsetsAndSizes(const void *buf, std::vector<uint32_t> &offsets, std::vector<uint32_t> &sizes) const;
	
	uint32_t CalculateOffsetsSize();
	uint32_t CalculateHeaderSize();
	uint32_t CalculateFileSize();
	
	void *CreateOffsetsSection(uint32_t *offsets_size);
	
	static bool GetEntrySize(const Afs2Entry &entry, uint32_t *psize);
	void PadFile(FILE *file);

    std::string ChooseFileName(uint32_t idx, uint32_t file_size) const;
	
    static inline bool DoCopyFile(FILE *src, FILE *dst, size_t size)
    {
        return Utils::DoCopyFile(src, dst, size);
    }
	
    static inline bool DoCopyFile(uint8_t *src, FILE *dst, size_t size)
	{
		if (fwrite(src, 1, size, dst) != size)
			return false;
		
		return true;
	}
	
    static inline bool DoCopyFile(FILE *src, uint8_t *dst, size_t size)
	{
		if (fread(dst, 1, size, src) != size)
			return false;
		
		return true;
	}
		
    static inline bool DoCopyFile(uint8_t *src, uint8_t *dst, size_t size)
	{ 
		memmove(dst, src, size); 
		return true;
	}
	
public:

	Afs2File();
	virtual ~Afs2File();
	
    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;
	
	virtual bool LoadFromFile(const std::string &path, bool show_error=true) override;
	virtual bool SaveToFile(const std::string &path, bool show_error=true, bool build_path=false) override;
	
    virtual uint8_t *CreateHeader(unsigned int *psize, bool extra_word=true) override;
	
    virtual uint32_t GetNumFiles() const override { return (uint32_t)files.size(); }
    virtual bool GetFileSize(uint32_t idx, uint64_t *psize) const override
	{
		if (idx >= files.size())
			return false;

        uint32_t entry_size;
		
        bool ret = GetEntrySize(files[idx], &entry_size);
        if (ret)
            *psize = entry_size;

        return ret;
	}
	
    virtual bool ExtractFile(uint32_t idx, const std::string &path, bool auto_path=false) const override;
    virtual uint8_t *ExtractFile(uint32_t idx, uint64_t *psize) const override;

    virtual bool SetFile(uint32_t idx, void *buf, uint64_t size, bool take_ownership=false) override;
    // TODO: implement this one
    virtual bool SetFile(uint32_t idx, const std::string &path) override
    {
        UNUSED(idx); UNUSED(path);
        return false;
    }
	
    virtual bool AddFile(const std::string &path) override;
    virtual bool AddFile(void *buf, uint64_t size, bool take_ownership=false) override;
};

#endif // __AFS2FILE_H__
