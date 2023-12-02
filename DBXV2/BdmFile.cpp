#include "BdmFile.h"
#include "FixedMemoryStream.h"

BdmFile::BdmFile()
{
    this->big_endian = false;
}

BdmFile::~BdmFile()
{

}

void BdmFile::Reset()
{
    entries.clear();
}

bool BdmFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    FixedMemoryStream stream(const_cast<uint8_t *>(buf), size);
    BDMHeader *header;

    if (!stream.FastRead((uint8_t **)&header, sizeof(BDMHeader)))
    {
        //DPRINTF("%s: Header read failed.\n", FUNCNAME);
        return false;
    }

    if (header->signature != BDM_SIGNATURE)
    {
        //DPRINTF("%s: Signature check failed.\n", FUNCNAME);
        return false;
    }

    if (!stream.Seek(header->data_start, SEEK_SET))
    {
        //DPRINTF("%s: Seek failed.\n", FUNCNAME);
        return false;
    }

    size_t data_size = (size_t)stream.GetSize() - (size_t)stream.Tell();
    if ((data_size / header->num_entries) != sizeof(BDMEntry))
    {
        //DPRINTF("%s: Unsupported type of bdm.\n", FUNCNAME);
        return false;
    }

    entries.resize(header->num_entries);
    if (!stream.Read(entries.data(), data_size))
    {
        //DPRINTF("%s: Entries read failed.\n", FUNCNAME);
        return false;
    }

    return true;
}

uint8_t *BdmFile::Save(size_t *psize)
{
    *psize = sizeof(BDMHeader) + entries.size() * sizeof(BDMEntry);
    uint8_t *buf = new uint8_t[*psize];

    BDMHeader *header = (BDMHeader *)buf;
    *header = BDMHeader();
    header->num_entries = (uint32_t)entries.size();

    if (entries.size() > 0)
        memcpy(buf + header->data_start, entries.data(), entries.size()*sizeof(BDMEntry));

    return buf;
}

size_t BdmFile::ChangeReferencesToSkill(uint16_t old_skill, uint16_t new_skill)
{
    size_t count = 0;

    for (BDMEntry &entry : entries)
    {
        for (size_t i = 0; i < NUM_BDM_SUBENTRIES; i++)
        {
            for (size_t j = 0; j < NUM_BDM_EFFECTS; j++)
            {
                if (entry.sub_entries[i].effects[j].skill_id == old_skill)
                {
                    entry.sub_entries[i].effects[j].skill_id = new_skill;
                    count++;
                }
            }
        }
    }

    return count;
}
