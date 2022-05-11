#ifndef IGGYTEXFILE_H
#define IGGYTEXFILE_H

#include "BaseFile.h"

#ifdef ReplaceFile
#undef ReplaceFile
#endif

// oggytex
#define IGGYTEX_SIGNATURE   0x7865747967676F

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED IGGYTEXHeader
{
    uint64_t signature; // 0
    uint32_t num_files; // 8
    uint32_t table_start; // 0x0C
};
CHECK_STRUCT_SIZE(IGGYTEXHeader, 0x10);

struct PACKED IGGYTEXTable
{
    uint32_t id; // 0
    uint32_t size; // 4
    uint32_t offset; // 8
    uint32_t unk_0C; // 0x0C
};
CHECK_STRUCT_SIZE(IGGYTEXTable, 0x10);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

class IggyTexContainedFile;

class IggyTexFile : public BaseFile
{
private:

    uint8_t *buf = nullptr;
    size_t size;

    std::vector<IggyTexContainedFile> files;

    void Copy(const IggyTexFile &other);
    void Reset();

public:
    IggyTexFile();
    IggyTexFile(const uint8_t *buf, size_t size);
    IggyTexFile(const IggyTexFile &other) : IggyTexFile()
    {
        Copy(other);
    }

    virtual ~IggyTexFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;

    uint32_t FindIndex(uint32_t id);

    inline size_t GetNumFiles() const { return files.size(); }

    bool ReplaceFile(uint32_t idx, const uint8_t *buf, size_t size);
    bool ReplaceFileByID(uint32_t id, const uint8_t *buf, size_t size);

    uint32_t AppendFile(const uint8_t *buf, size_t size, uint32_t id, bool take_ownership=false);
    uint32_t AppendFile(const IggyTexFile &other, uint32_t idx_other);
    uint32_t AppendFile(const IggyTexContainedFile &file);

    void RemoveFile(uint32_t idx);

    //virtual uint8_t *Save(size_t *psize) override;

    /*virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;*/

    inline IggyTexFile &operator=(const IggyTexFile &other)
    {
        if (this == &other)
            return *this;

        Copy(other);
        return *this;
    }

    inline bool operator==(const IggyTexFile &rhs) const
    {
        return (this->files == rhs.files);
    }

    inline bool operator!=(const IggyTexFile &rhs) const
    {
        return !(*this == rhs);
    }

    inline IggyTexContainedFile &operator[](size_t n) { return files[n]; }
    inline const IggyTexContainedFile &operator[](size_t n) const { return files[n]; }

    inline const IggyTexFile operator+(const IggyTexContainedFile &file) const
    {
        IggyTexFile new_it = *this;

        new_it.AppendFile(file);
        return new_it;
    }

    inline IggyTexFile &operator+=(const IggyTexContainedFile &file)
    {
        this->AppendFile(file);
        return *this;
    }

    inline std::vector<IggyTexContainedFile>::iterator begin() { return files.begin(); }
    inline std::vector<IggyTexContainedFile>::iterator end() { return files.end(); }

    inline std::vector<IggyTexContainedFile>::const_iterator begin() const { return files.begin(); }
    inline std::vector<IggyTexContainedFile>::const_iterator end() const { return files.end(); }

};

class IggyTexContainedFile
{
private:
    uint8_t *buf;
    size_t size;
    bool allocated;
    uint32_t id;

    void Copy(const IggyTexContainedFile &other);
    void Reset();

    friend class IggyTexFile;

public:

    IggyTexContainedFile();
    IggyTexContainedFile(const IggyTexContainedFile &other) : IggyTexContainedFile()
    {
        Copy(other);
    }

    ~IggyTexContainedFile();

    inline uint32_t GetID() const { return id; }
    inline void SetID(uint32_t id) { this->id = id; }

    inline size_t GetSize() const { return size; }
    inline void SetSize(size_t size) { this->size = size; }

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

    inline IggyTexContainedFile &operator=(const IggyTexContainedFile &other)
    {
        if (this == &other)
            return *this;

        Copy(other);
        return *this;
    }

    inline bool operator==(const IggyTexContainedFile &rhs) const
    {
        if (this->size != rhs.size)
            return false;

        if (this->id != rhs.id)
            return false;

        return (memcmp(this->buf, rhs.buf, this->size) == 0);
    }

    inline bool operator!=(const IggyTexContainedFile &rhs) const
    {
        return !(*this == rhs);
    }
};

#endif // IGGYTEXFILE_H
