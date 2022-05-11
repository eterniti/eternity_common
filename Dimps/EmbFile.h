#ifndef __EMBFILE_H__
#define __EMBFILE_H__

#include <stdint.h>
#include <vector>
#include <string>

#include "BaseFile.h"

#ifdef ReplaceFile
#undef ReplaceFile
#endif

// "#EMB"
#define EMB_SIGNATURE	0x424D4523

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t header_size; // 6
    uint16_t unk_08; // 8
    uint16_t unk_0A; // 0xA
    uint32_t files_count; // 0xC
    uint32_t unk_10; // 0x10
    uint32_t unk_14; // 0x14
    uint32_t table_start; // 0x18
    uint32_t filenames_table; // 0x1C    this is 0 when no filenames
    // size 0x20
} PACKED EMBHeader;

static_assert(sizeof(EMBHeader) == 0x20, "Incorrect structure size.");

typedef struct
{
    uint32_t offset; // 0
    uint32_t file_size; // 4
    // size 8
} PACKED EMBTable;

static_assert(sizeof(EMBTable) == 8, "Incorrect structure size.");

#ifdef _MSC_VER
#pragma pack(pop)
#endif

class EmbContainedFile;

class EmbFile : public BaseFile
{
private:

    uint8_t *buf = nullptr;
    size_t size;

    bool recursive;

    std::vector<EmbContainedFile> files;

    void Copy(const EmbFile &other);
    void Reset();

    unsigned int CalculateFileSize();

public:
    EmbFile(bool recursive);
    EmbFile() : EmbFile(false) { }
    EmbFile(const uint8_t *buf, size_t size, bool recursive=false);
    EmbFile(const EmbFile &other) : EmbFile()
    {
        Copy(other);
    }

    virtual ~EmbFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;

    static std::string CreateFileName(uint16_t idx, const uint8_t *buf, size_t size);

    bool HasFileNames() const;
    uint16_t FindIndexByName(const std::string &name);

    inline size_t GetNumFiles() const { return files.size(); }

    bool ReplaceFile(uint16_t idx, const uint8_t *buf, size_t size);
    bool ReplaceFile(const std::string &name, const uint8_t *buf, size_t size);

    uint16_t AppendFile(const uint8_t *buf, size_t size, const std::string &name, bool take_ownership=false);
    uint16_t AppendFile(const EmbFile &other, uint16_t idx_other);
    uint16_t AppendFile(const EmbContainedFile &file);
    uint16_t AppendFile(const std::string &file_path);

    void RemoveFile(uint16_t idx);

    virtual uint8_t *Save(size_t *psize) override;	

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    inline EmbFile &operator=(const EmbFile &other)
    {
        if (this == &other)
            return *this;

        Copy(other);
        return *this;
    }

    inline bool operator==(const EmbFile &rhs) const
    {
        return (this->files == rhs.files);
    }

    inline bool operator!=(const EmbFile &rhs) const
    {
        return !(*this == rhs);
    }

    inline EmbContainedFile &operator[](size_t n) { return files[n]; }
    inline const EmbContainedFile &operator[](size_t n) const { return files[n]; }

    inline const EmbFile operator+(const EmbContainedFile &file) const
    {
        EmbFile new_emb = *this;

        new_emb.AppendFile(file);
        return new_emb;
    }

    inline EmbFile &operator+=(const EmbContainedFile &file)
    {
        this->AppendFile(file);
        return *this;
    }

    inline std::vector<EmbContainedFile>::iterator begin() { return files.begin(); }
    inline std::vector<EmbContainedFile>::iterator end() { return files.end(); }

    inline std::vector<EmbContainedFile>::const_iterator begin() const { return files.begin(); }
    inline std::vector<EmbContainedFile>::const_iterator end() const { return files.end(); }
};

class EmbContainedFile
{
private:
    uint8_t *buf;
    size_t size;
    bool allocated;
    std::string name;

    // For recursivity, this stores the emb, if this file represents another emb
    EmbFile emb;
    bool is_emb;

    void Copy(const EmbContainedFile &other);
    void Reset();

    friend class EmbFile;

public:

    EmbContainedFile();
    EmbContainedFile(const EmbContainedFile &other) : EmbContainedFile()
    {
        Copy(other);
    }

    ~EmbContainedFile();

    inline std::string GetName() const { return name; }
    inline void SetName(const std::string &name) { this->name = name; }

    inline size_t GetSize() const { return size; }
    inline void SetSize(size_t size) { this->size = size; }

    inline bool IsEmb() const { return is_emb; }

    inline uint8_t *GetData() { return buf; }
    inline const uint8_t *GetData() const { return buf; }

    inline void SetData(uint8_t *buf, size_t size, bool allocated)
    {
        if (this->buf && this->allocated)
        {
            delete[] this->buf;
        }

        this->buf = buf;
        this->size = size;
        this->allocated = allocated;
    }

    void Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);

    inline EmbContainedFile &operator=(const EmbContainedFile &other)
    {
        if (this == &other)
            return *this;

        Copy(other);
        return *this;
    }

    inline bool operator==(const EmbContainedFile &rhs) const
    {
        if (this->size != rhs.size)
            return false;

        if (this->name != rhs.name)
            return false;

        if (this->IsEmb() && rhs.IsEmb())
        {
            const EmbFile &file1 = *this;
            const EmbFile &file2 = rhs;

            return (file1 == file2);
        }

        return (memcmp(this->buf, rhs.buf, this->size) == 0);
    }

    inline bool operator!=(const EmbContainedFile &rhs) const
    {
        return !(*this == rhs);
    }

    inline operator EmbFile&() { return emb; }
    inline operator const EmbFile&() const { return emb; }
};

#endif
