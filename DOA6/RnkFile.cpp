#include "RnkFile.h"

RnkFile::RnkFile()
{
    this->big_endian = false;
}

RnkFile::~RnkFile()
{

}

void RnkFile::Reset()
{
    entries.clear();
}

bool RnkFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(RNKHeader))
        return false;

    const RNKHeader *hdr = (const RNKHeader *)buf;
    const uint8_t *p = buf + hdr->header_size;
    const uint8_t *end = p + size;

    while (p < end)
    {
        for (; p < end; p++)
        {
            if (memcmp(p, RNK_ENTRY_SIGNATURE, 8) == 0)
                break;
        }

        if (p >= end)
            break;

        const RNKEntry *entry = (const RNKEntry *)p;
        uint32_t *string_indexes = (uint32_t *)(entry + 1);
        std::vector<std::string> props;

        props.resize(entry->num_strings);

        for (size_t i = 0; i < props.size(); i++)
        {
            props[i] = (const char *)(p + string_indexes[i]);
        }

        entries[entry->file_id] = props;

        p += entry->entry_size;
    }

    return true;
}

