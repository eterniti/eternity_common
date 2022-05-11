#ifndef PSEUDOAWBFILE_H
#define PSEUDOAWBFILE_H

#include "AwbFile.h"
#include "Utils.h"

class PseudoAwbFile : public AwbFile
{
private:

    std::vector<std::string> files;

public:

    PseudoAwbFile(const std::vector<std::string> &files_list) : files(files_list) { }
    virtual ~PseudoAwbFile() { }

    virtual uint8_t *CreateHeader(unsigned int *, bool) override { return nullptr; }

    virtual uint32_t GetNumFiles() const override { return (uint32_t)files.size(); }
    virtual bool GetFileSize(uint32_t idx, uint64_t *psize) const override
    {
        if (idx >= files.size())
            return false;

        *psize = Utils::GetFileSize(files[idx]);
        return (*psize != (size_t)-1);
    }

    // Unimplemented
    virtual bool ExtractFile(uint32_t, const std::string &, bool) const override { return false; }

    virtual uint8_t *ExtractFile(uint32_t idx, uint64_t *psize) const override
    {
        if (idx >= files.size())
            return false;

        size_t ret_size;

        uint8_t *ret = Utils::ReadFile(files[idx], &ret_size, false);
        *psize = ret_size;
        return ret;
    }

    virtual bool SetFile(uint32_t idx, void *buf, uint64_t size, bool take_ownership=false) override
    {
        if (idx >= files.size())
            return false;

        bool ret = Utils::WriteFileBool(files[idx], (const uint8_t *)buf, size, false);

        if (take_ownership)
        {
            uint8_t *ptr = (uint8_t *)buf;
            delete[] ptr;
        }

        return ret;
    }

    virtual bool SetFile(uint32_t idx, const std::string &path) override
    {
        if (idx >= files.size())
            return false;

        files[idx] = path;
        return true;
    }

    virtual bool AddFile(const std::string &path) override
    {
        files.push_back(path);
        return true;
    }

    // Unimplemented
    virtual bool AddFile(void *, uint64_t, bool) override { return false; }

    inline const std::string &GetFilePath(uint32_t idx) const
    {
        return (files[idx]);
    }
};

#endif // PSEUDOAWBFILE_H
