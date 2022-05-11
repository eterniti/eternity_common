#ifndef __AWBFILE_H__
#define __AWBFILE_H__

#include "BaseFile.h"

class AwbFile : public BaseFile
{
public:

    virtual uint8_t *CreateHeader(unsigned int *psize, bool extra_word=true) = 0;

    virtual uint32_t GetNumFiles() const = 0;
    virtual bool GetFileSize(uint32_t idx, uint64_t *psize) const = 0;

    virtual bool ExtractFile(uint32_t idx, const std::string &path, bool auto_path=false) const = 0;
    virtual uint8_t *ExtractFile(uint32_t idx, uint64_t *psize) const = 0;

    virtual bool SetFile(uint32_t idx, void *buf, uint64_t size, bool take_ownership=false) = 0;
    virtual bool SetFile(uint32_t idx, const std::string &path) = 0;

    virtual bool AddFile(const std::string &path) = 0;
    virtual bool AddFile(void *buf, uint64_t size, bool take_ownership=false) = 0;
};

#endif // __AWBFILE_H__
