#include "CnsFile.h"
#include "debug.h"

CnsFile::CnsFile()
{
    this->big_endian = false;
}

CnsFile::~CnsFile()
{

}

void CnsFile::Reset()
{
    entries.clear();
}

bool CnsFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(CNSHeader))
        return false;

    const CNSHeader *hdr = (const CNSHeader *)buf;
    if (hdr->signature != CNS_SIGNATURE)
        return false;

    entries.resize(hdr->num_entries);

    const CNSEntry *file_entries = (const CNSEntry *)(buf + hdr->data_start);
    for (size_t i = 0; i < entries.size(); i++)
    {
        entries[i] = file_entries[i];
    }

    return true;
}

CnsEntry *CnsFile::FindEntry(uint16_t skill_id)
{
    if (skill_id >= 0x8000)
        return nullptr;

    for (CnsEntry &entry : entries)
        if (entry.id == skill_id)
            return &entry;

    return nullptr;
}
