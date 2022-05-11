#ifndef GRPFILE_H
#define GRPFILE_H

#include "BaseFile.h"

struct PACKED GRPEntry
{
    uint32_t parts_name_hash; // 0
    uint32_t parts_id; // 4
    uint32_t count_t; // 8 - This may be the number of submeshes
    uint32_t count_T; // C
    uint32_t count_s; // 0x10
    uint32_t count_set_t; // 0x14 - This may be the number of meshes
    uint32_t count_set_T; // 0x18
    uint32_t count_set_s; // 0x1C

    GRPEntry()
    {
        parts_name_hash = 0x3057221F; // "default"
        parts_id = 0;
        count_t = count_T = count_s = 0;
        count_set_t = count_set_T = count_set_s = 0;
    }
};
CHECK_STRUCT_SIZE(GRPEntry, 0x20);

class GrpFile : public BaseFile
{
private:

    std::vector<GRPEntry> entries;

protected:

    void Reset();

public:

    GrpFile();
    virtual ~GrpFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
};

#endif // GRPFILE_H
