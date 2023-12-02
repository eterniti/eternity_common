#include "BpeFile.h"
#include "FixedMemoryStream.h"
#include <unordered_set>

BpeFile::BpeFile()
{
    this->big_endian = false;
}

BpeFile::~BpeFile()
{

}

void BpeFile::Reset()
{
    entries.clear();
}

bool BpeFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    FixedMemoryStream stream(const_cast<uint8_t *>(buf), size);
    BPEHeader *header;

    if (!stream.FastRead((uint8_t **)&header, sizeof(BPEHeader)))
    {
        //DPRINTF("%s: Header read failed.\n", FUNCNAME);
        return false;
    }

    if (header->signature != BPE_SIGNATURE)
    {
        //DPRINTF("%s: Signature check failed.\n", FUNCNAME);
        return false;
    }

    if (!stream.Seek(header->data_start, SEEK_SET))
    {
        //DPRINTF("%s: Seek failed.\n", FUNCNAME);
        return false;
    }

    uint32_t *offsets;
    if (!stream.FastRead((uint8_t **)&offsets, header->num_entries*sizeof(uint32_t)))
        return false;

    entries.resize(header->num_entries);

    for (size_t i = 0; i < entries.size(); i++)
    {
        if (offsets[i] == 0)
            continue;

        BpeEntry &entry = entries[i];
        BPEEntry *native_entry;

        if (!stream.Seek(offsets[i], SEEK_SET))
            return false;

        if (!stream.FastRead((uint8_t **)&native_entry, sizeof(BPEEntry)))
            return false;

        entry = BpeEntry(*native_entry);
        entry.sub_entries.resize(native_entry->num_sub_entries);

        if (!stream.Seek(offsets[i] + native_entry->sub_offset, SEEK_SET))
            return false;

        for (size_t j = 0; j < entry.sub_entries.size(); j++)
        {
            BpeSubEntry &se = entry.sub_entries[j];
            BPESubEntry *native_se;

            if (!stream.FastRead((uint8_t **)&native_se, sizeof(BPESubEntry)))
                return false;

            se.type = native_se->type;
        }
    }

    return true;
}

bool BpeFile::HasType(size_t idx, int type) const
{
    if (idx >= entries.size() || !entries[idx].valid)
        return false;

    for (const BpeSubEntry &se : entries[idx].sub_entries)
        if (se.type == type)
            return true;

    return false;
}

std::string BpeFile::GetTypeString(size_t idx) const
{
    std::string ret;

    if (idx >= entries.size() || !entries[idx].valid)
        return ret;

    std::unordered_set<uint32_t> done;

    for (const BpeSubEntry &se : entries[idx].sub_entries)
    {
        if (done.find(se.type) != done.end())
            continue;

        if (ret.length() > 0)
            ret += ",";

        switch (se.type)
        {
            case BPE_ST_BLUR:
                ret += "Blur";
            break;

            case BPE_ST_WHITESHINE:
                ret += "WhiteShine";
            break;

            case BPE_ST_DISORIENTATION:
                ret += "Disorientation";
            break;

            case BPE_ST_ZOOM:
                ret += "Zoom";
            break;

            case BPE_ST_HUE:
                ret += "Hue";
            break;

            case BPE_ST_BODY_OUTLINE:
                ret += "BodyOutline";
            break;

            default:
                ret += "Type" + Utils::ToString(se.type);
        }

        done.insert(se.type);
    }

    return ret;
}
