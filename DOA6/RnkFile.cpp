#include "RnkFile.h"
#include "FixedMemoryStream.h"

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

    /*if (memcmp(buf, RNK_SIGNATURE, 8) != 0)
    {
        DPRINTF("%s: Invalid signature.\n", FUNCNAME);
        return false;
    }

    const RNKHeader *hdr = (const RNKHeader *)buf;
    const uint8_t *p = buf + hdr->header_size;
    const uint8_t *end = p + size;

    end = end - 8; // Bugfix for invalid memory access

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
    }*/

    FixedMemoryStream mem(const_cast<uint8_t *>(buf), size);
    RNKHeader *hdr;

    if (!mem.FastRead((uint8_t **)&hdr, sizeof(RNKHeader)))
        return false;

    if (memcmp(hdr->signature, RNK_SIGNATURE, 8) != 0)
    {
        DPRINTF("%s: Invalid signature.\n", FUNCNAME);
        return false;
    }

    if (!mem.Seek(hdr->header_size, SEEK_SET))
        return false;

    while (true)
    {
        while (true)
        {
            uint64_t sig;

            mem.SavePos();

            if (!mem.Read64(&sig))
                return true;

            mem.RestorePos();

            if (memcmp(&sig, RNK_ENTRY_SIGNATURE, 8) == 0)
                break;

            mem.Seek(1, SEEK_CUR);
        }

        RNKEntry *entry;

        uint64_t entry_top = mem.Tell();

        if (!mem.FastRead((uint8_t **)&entry, sizeof(RNKEntry)))
            break;

         std::vector<uint32_t> indexes;
         indexes.resize(entry->num_strings);

         if (!mem.Read(indexes.data(), indexes.size()*sizeof(uint32_t)))
             break;

         std::vector<std::string> props;
         props.resize(entry->num_strings);

         for (size_t i = 0; i < props.size(); i++)
         {
             if (!mem.Seek(entry_top + indexes[i], SEEK_SET))
                 break;

             if (!mem.ReadCString(props[i]))
                 return false;
         }

         entries[entry->file_id] = props;

         if (!mem.Seek(entry_top + entry->entry_size, SEEK_SET))
             break;
    }

    return true;
}

