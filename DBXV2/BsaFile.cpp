#include "BsaFile.h"

BsaFile::BsaFile()
{
    this->big_endian = false;
}

BsaFile::~BsaFile()
{

}

void BsaFile::Reset()
{
    data.clear();
    unk6s.clear();
}

bool BsaFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(BSAHeader) || *(uint32_t *)buf != BSA_SIGNATURE)
        return false;

    data.resize(size);
    memcpy(data.data(), buf, size);

    BSAHeader *hdr = (BSAHeader *)data.data();
    uint32_t *entries_table = (uint32_t *)GetOffsetPtr(hdr, hdr->data_start);

    for (uint16_t i = 0; i < hdr->num_entries; i++)
    {
        if (entries_table[i] == 0)
            continue;

        BSAEntry *entry = (BSAEntry *)GetOffsetPtr(hdr, entries_table, i);

        if (entry->subentries_offset == 0)
            continue;

        BSASubEntry *subentries = (BSASubEntry *)GetOffsetPtr(entry, entry->subentries_offset);

        for (uint16_t j = 0; j < entry->num_subentries; j++)
        {
            if (subentries[j].type == 6)
            {
                BSAUnk6 *file_unk6s = (BSAUnk6 *)GetOffsetPtr(&subentries[j], subentries[j].data_offset);

                for (uint16_t k = 0; k < subentries[j].count; k++)
                {
                    unk6s.push_back(&file_unk6s[k]);
                }
            }
        }
    }

    return true;
}

uint8_t *BsaFile::Save(size_t *psize)
{
    if (data.size() == 0)
        return nullptr;

    uint8_t *buf = new uint8_t[data.size()];
    memcpy(buf, data.data(), data.size());

    *psize = data.size();
    return buf;
}

size_t BsaFile::ChangeReferencesToSkill(uint16_t old_skill, uint16_t new_skill)
{
    size_t count = 0;

    for (BSAUnk6 *unk6 : unk6s)
    {
        if (unk6->skill_id == old_skill)
        {
            unk6->skill_id = new_skill;
            count++;
        }
    }

    return count;
}
