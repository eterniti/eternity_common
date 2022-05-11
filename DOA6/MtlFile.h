#ifndef MTLFILE_H
#define MTLFILE_H

#include "FixedMemoryStream.h"

class MtlFile : public BaseFile
{
private:

    std::vector<uint32_t> names;
    std::vector<std::vector<uint32_t>> mat_ids;

    std::vector<std::pair<uint32_t, uint32_t>> cloths, ponytails;

protected:
    void Reset();

public:
    MtlFile();
    virtual ~MtlFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    bool LoadFromTextFile(const std::string &path);
    bool SaveToTextFile(const std::string &path, bool *crack_success, bool full_crack);
};

#endif // MTLFILE_H
