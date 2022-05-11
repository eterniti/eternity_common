#include "FmpFile.h"

#include "debug.h"

FmpFile::FmpFile()
{
    this->big_endian = false;
}

FmpFile::~FmpFile()
{
    Reset();
}

bool FmpFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    FMPHeader *hdr = (FMPHeader *)buf;

    if (size < sizeof(FMPHeader) || memcmp(hdr->signature, FMP_SIGNATURE, 4) != 0)
        return false;

    if (memcmp(hdr->unk_04, "\x00\x02\x01\x00", 4) != 0)
    {
        DPRINTF("%s: unk_04 not as expected.\n", FUNCNAME);
        return false;
    }

    for (int i = 0; i < 8; i++)
    {
        unk_08[i] = hdr->unk_08[i];
    }

    FMPUnk1 *unk1 = (FMPUnk1 *)GetOffsetPtr(buf, hdr->unk1_offset);
    for (int i = 0; i < 30; i++)
    {
        this->unk1[i] = val_float(unk1->unk[i]);
    }

    FMPUnk2 *unk2 = (FMPUnk2 *)GetOffsetPtr(buf, hdr->unk2_offset);
    for (int i = 0; i < 48; i++)
    {
        this->unk2[i] = val_float(unk2->unk[i]);
    }

    FMPUnk1EntryHeader *unk1_entries = (FMPUnk1EntryHeader *)GetOffsetPtr(buf, hdr->unk1e_offset);
    this->unk1_entries.resize(val32(hdr->num_unk1e));

    for (size_t i = 0; i < this->unk1_entries.size(); i++)
    {
        FmpUnk1 &unk1 = this->unk1_entries[i];

        unk1.name = (char *)GetOffsetPtr(buf, unk1_entries[i].name_offset);
        unk1.unsigneds.resize(val32(unk1_entries->num_unsigned));

        for (size_t j = 0; j < unk1.unsigneds.size(); j++)
        {
            uint16_t *p = (uint16_t *)GetOffsetPtr(buf, unk1_entries[i].unsigned_offsets);
            unk1.unsigneds[j] = val16(p[j]);
        }
    }

    FMPUnk2EntryHeader *unk2_entries = (FMPUnk2EntryHeader *)GetOffsetPtr(buf, hdr->unk2e_offset);
    this->unk2_entries.resize(val32(hdr->num_unk2e));

    for (size_t i = 0; i < this->unk2_entries.size(); i++)
    {
        FmpUnk2 &unk2 = this->unk2_entries[i];

        unk2.name = (char *)GetOffsetPtr(buf, unk2_entries[i].name_offset);
        unk2.unk_04 = val16(unk2_entries[i].unk_04);
        unk2.unk_06 = val16(unk2_entries[i].unk_06);
        unk2.unk_08 = val16(unk2_entries[i].unk_08);
        unk2.unk_0A = val16(unk2_entries[i].unk_0A);
        unk2.unk_1C = val32(unk2_entries[i].unk_1C);

        for (int j = 0; j < 16; j++)
        {
            unk2.matrix[j] = val_float(unk2_entries[i].matrix[j]);
        }
    }

    return true;
}

void FmpFile::Reset()
{
    unk1_entries.clear();
    unk2_entries.clear();
}
