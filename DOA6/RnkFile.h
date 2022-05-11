#ifndef RNKFILE_H
#define RNKFILE_H

// should have called this file KnrFile instead but oh, well...

#include <map>
#include "BaseFile.h"

#define RNK_SIGNATURE       "_RNK0000"
#define RNK_ENTRY_SIGNATURE "IRNK0000"

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED RNKHeader
{
    char signature[8];   // 0
    uint32_t header_size; // 8
    uint32_t unk_0C;
    uint32_t num_entries; // 10
    uint32_t file_size; // 14 size of this file
};
CHECK_STRUCT_SIZE(RNKHeader, 0x18);

struct PACKED RNKEntry
{
    char signature[8]; // 0
    uint32_t entry_size; // 8
    uint32_t file_id; // 0C
    uint32_t num_strings;
};
CHECK_STRUCT_SIZE(RNKEntry, 0x14);

#ifdef _MSC_VER
#pragma pack(pop)
#endif


class RnkFile : public BaseFile
{
private:

    std::map<uint32_t, std::vector<std::string>> entries;

protected:

    void Reset();

public:
    RnkFile();
    virtual ~RnkFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;

    inline const std::map<uint32_t, std::vector<std::string>> &GetMap() const { return entries; }
    inline std::map<uint32_t, std::vector<std::string>> &GetMap() { return entries; }
};

#endif // RNKFILE_H
