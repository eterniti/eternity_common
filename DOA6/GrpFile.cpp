#include "GrpFile.h"
#include "FixedMemoryStream.h"

#include "debug.h"

GrpFile::GrpFile()
{
    this->big_endian = false;
}

GrpFile::~GrpFile()
{

}

void GrpFile::Reset()
{
    entries.clear();
}

bool GrpFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if ((size % sizeof(GRPEntry))!= 0)
    {
        DPRINTF("Unknown grp size %Id\n", size);
        return false;
    }

    entries.resize(size / sizeof(GRPEntry));

    FixedMemoryStream mem(const_cast<uint8_t *>(buf), size);

    for (GRPEntry &entry: entries)
    {
        if (!mem.Read32(&entry.parts_name_hash))
            return false;

        if (!mem.Read32(&entry.parts_id))
            return false;

        if (!mem.Read32(&entry.count_t))
            return false;

        if (!mem.Read32(&entry.count_T))
            return false;

        if (!mem.Read32(&entry.count_s))
            return false;

        if (!mem.Read32(&entry.count_set_t))
            return false;

        if (!mem.Read32(&entry.count_set_T))
            return false;

        if (!mem.Read32(&entry.count_set_s))
            return false;

        if (entry.parts_id != 0)
        {
            DPRINTF("parts_id not 0.\n");
            return false;
        }
    }

    return true;
}
