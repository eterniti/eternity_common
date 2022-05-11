#ifndef CSBINTERNALAWBFILE_H
#define CSBINTERNALAWBFILE_H

#include "AwbFile.h"
#include "UtfFile.h"

class CsbInternalAwbFile : public AwbFile, protected UtfFile
{
private:

    struct FileEntry
    {
        uint8_t *buf;
        uint64_t size;
        bool allocated;

        void Copy(const FileEntry &other)
        {
            size = other.size;
            allocated = other.allocated;

            if (allocated)
            {
                buf = new uint8_t[size];
                memcpy(buf, other.buf, size);
            }
            else
            {
                buf = other.buf;
            }
        }

        FileEntry() : buf(nullptr), size(0), allocated(false) { }
        FileEntry(const FileEntry &other)
        {
            Copy(other);
        }

        ~FileEntry()
        {
            if (allocated && buf)
                delete[] buf;
        }

        inline FileEntry &operator=(const FileEntry &other)
        {
            if (this == &other)
                return *this;

            Copy(other);
            return *this;
        }
    };

    std::vector<FileEntry> entries;

public:
    CsbInternalAwbFile() : UtfFile() {}
    virtual ~CsbInternalAwbFile() { }

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual bool LoadFromFile(const std::string &path, bool show_error=true) override;
    virtual bool SaveToFile(const std::string &path, bool show_error=true, bool build_path=false) override;

    virtual uint8_t *CreateHeader(unsigned int *psize, bool extra_word=true) override
    {
        UNUSED(psize);
        UNUSED(extra_word);
        return nullptr;
    }

    virtual uint32_t GetNumFiles() const override { return (uint32_t)entries.size(); }
    virtual bool GetFileSize(uint32_t idx, uint64_t *psize) const override
    {
        if (idx >= entries.size())
            return false;

        *psize = entries[idx].size;
        return true;
    }

    virtual bool ExtractFile(uint32_t idx, const std::string &path, bool auto_path=false) const override;
    virtual uint8_t *ExtractFile(uint32_t idx, uint64_t *psize) const override;

    virtual bool SetFile(uint32_t idx, void *buf, uint64_t size, bool take_ownership=false) override;
    virtual bool SetFile(uint32_t idx, const std::string &path) override;

    // Currently unimplemented;
    virtual bool AddFile(const std::string &path) override
    {
        UNUSED(path);
        return false;
    }

    virtual bool AddFile(void *buf, uint64_t size, bool take_ownership=false) override
    {
        UNUSED(buf); UNUSED(size); UNUSED(take_ownership);
        return false;
    }
};

#endif // CSBINTERNALAWBFILE_H
