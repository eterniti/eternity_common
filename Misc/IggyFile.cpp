#include "IggyFile.h"
#include "FixedMemoryStream.h"
#include "debug.h"

// WIP File. We need assert always on
#undef NDEBUG

IggyFile::IggyFile()
{
    this->big_endian = false;
}

IggyFile::~IggyFile()
{

}

bool IggyFile::LoadFlashData32(FixedMemoryStream &stream)
{
    IGGYFlashHeader32 hdr;
    size_t size = (size_t)stream.GetSize();

    if (!stream.Read(&hdr, sizeof(hdr)))
        return false;

    if (hdr.main_offset < sizeof(IGGYFlashHeader32))
        return false;

    if (hdr.as3_section_offset >= size || hdr.unk_offset >= size || hdr.unk_offset2 >= size || hdr.unk_offset3 >= size)
    {
        DPRINTF("%s: Failure in some absolute offset.", FUNCNAME);
        return false;
    }

    stream.Seek(0, SEEK_SET);

    size_t prev_offset = 0;
    size_t next_offset = hdr.as3_section_offset;

    // as3_section_offset = main section size
    main_section.resize(next_offset - prev_offset);

    if (!stream.Read(main_section.data(), main_section.size()))
       return false;

    assert(stream.Tell() == next_offset);
    prev_offset = next_offset;

    if (hdr.as3_code_offset != 1)
    {
        next_offset = hdr.as3_code_offset + offsetof(IGGYFlashHeader32, as3_code_offset);
    }
    else
    {
        if (hdr.names_offset != 1)
        {
            next_offset = hdr.names_offset + offsetof(IGGYFlashHeader32, names_offset);
        }
        else
        {
            if (hdr.last_section_offset != 1)
            {
                next_offset = hdr.last_section_offset + offsetof(IGGYFlashHeader32, last_section_offset);
            }
            else
            {
                next_offset = size;
            }
        }
    }

    as3_names_section.resize(next_offset - prev_offset);

    if (!stream.Read(as3_names_section.data(), as3_names_section.size()))
        return false;

    assert(stream.Tell() == next_offset);
    prev_offset = next_offset;

    if (hdr.as3_code_offset != 1)
    {
        if (hdr.names_offset != 1)
        {
            next_offset = hdr.names_offset + offsetof(IGGYFlashHeader32, names_offset);
        }
        else
        {
            if (hdr.last_section_offset != 1)
            {
                next_offset = hdr.last_section_offset + offsetof(IGGYFlashHeader32, last_section_offset);
            }
            else
            {
                next_offset = size;
            }
        }

        as3_offset = (uint32_t)stream.Tell();
        as3_code_section.resize(next_offset - prev_offset);

        if (!stream.Read(as3_code_section.data(), as3_code_section.size()))
            return false;

        assert(stream.Tell() == next_offset);
        prev_offset = next_offset;
    }
    else
    {
        as3_code_section.clear();
    }

    if (hdr.names_offset != 1)
    {
        if (hdr.last_section_offset != 1)
        {
            next_offset = hdr.last_section_offset + offsetof(IGGYFlashHeader32, last_section_offset);
        }
        else
        {
            next_offset = size;
        }

        names_section.resize(next_offset - prev_offset);

        if (!stream.Read(names_section.data(), names_section.size()))
            return false;

        assert(stream.Tell() == next_offset);
        prev_offset = next_offset;
    }
    else
    {
        names_section.clear();
    }

    if (hdr.last_section_offset != 1)
    {
        next_offset = size;
        last_section.resize(next_offset - prev_offset);

        if (!stream.Read(last_section.data(), last_section.size()))
            return false;
    }
    else
    {
        last_section.clear();
    }

    assert(stream.Tell() == size);

    // Check as3 code section
    uint8_t *as3_ptr = as3_code_section.data();
    size_t max_as3_size = as3_code_section.size();

    if (max_as3_size != 0)
    {
        if (max_as3_size < 8 || *(uint32_t *)&as3_ptr[4] > max_as3_size-8)
        {
            DPRINTF("%s: Failed at AS3 size.\n", FUNCNAME);
            return false;
        }
    }

    return true;
}

bool IggyFile::LoadFlashData64(FixedMemoryStream &stream)
{
    IGGYFlashHeader64 hdr;
    size_t size = (size_t)stream.GetSize();

    if (!stream.Read(&hdr, sizeof(hdr)))
        return false;

    if (hdr.main_offset < sizeof(IGGYFlashHeader64))
        return false;

    if (hdr.as3_section_offset >= size || hdr.unk_offset >= size || hdr.unk_offset2 >= size || hdr.unk_offset3 >= size)
    {
        DPRINTF("%s: Failure in some absolute offset.", FUNCNAME);
        return false;
    }

    stream.Seek(0, SEEK_SET);

    size_t prev_offset = 0;
    size_t next_offset = hdr.as3_section_offset;

    // as3_section_offset = main section size
    main_section.resize(next_offset - prev_offset);

    if (!stream.Read(main_section.data(), main_section.size()))
       return false;

    assert(stream.Tell() == next_offset);
    prev_offset = next_offset;

    if (hdr.as3_code_offset != 1)
    {
        next_offset = hdr.as3_code_offset + offsetof(IGGYFlashHeader64, as3_code_offset);
    }
    else
    {
        if (hdr.names_offset != 1)
        {
            next_offset = hdr.names_offset + offsetof(IGGYFlashHeader64, names_offset);
        }
        else
        {
            if (hdr.last_section_offset != 1)
            {
                next_offset = hdr.last_section_offset + offsetof(IGGYFlashHeader64, last_section_offset);
            }
            else
            {
                next_offset = size;
            }
        }
    }

    as3_names_section.resize(next_offset - prev_offset);

    if (!stream.Read(as3_names_section.data(), as3_names_section.size()))
        return false;

    assert(stream.Tell() == next_offset);
    prev_offset = next_offset;

    if (hdr.as3_code_offset != 1)
    {
        if (hdr.names_offset != 1)
        {
            next_offset = hdr.names_offset + offsetof(IGGYFlashHeader64, names_offset);
        }
        else
        {
            if (hdr.last_section_offset != 1)
            {
                next_offset = hdr.last_section_offset + offsetof(IGGYFlashHeader64, last_section_offset);
            }
            else
            {
                next_offset = size;
            }
        }

        as3_offset = (uint32_t)stream.Tell();
        as3_code_section.resize(next_offset - prev_offset);

        if (!stream.Read(as3_code_section.data(), as3_code_section.size()))
            return false;

        assert(stream.Tell() == next_offset);
        prev_offset = next_offset;
    }
    else
    {
        as3_code_section.clear();
    }

    if (hdr.names_offset != 1)
    {
        if (hdr.last_section_offset != 1)
        {
            next_offset = hdr.last_section_offset + offsetof(IGGYFlashHeader64, last_section_offset);
        }
        else
        {
            next_offset = size;
        }

        names_section.resize(next_offset - prev_offset);

        if (!stream.Read(names_section.data(), names_section.size()))
            return false;

        assert(stream.Tell() == next_offset);
        prev_offset = next_offset;
    }
    else
    {
        names_section.clear();
    }

    if (hdr.last_section_offset != 1)
    {
        next_offset = size;
        last_section.resize(next_offset - prev_offset);

        if (!stream.Read(last_section.data(), last_section.size()))
            return false;
    }
    else
    {
        last_section.clear();
    }

    assert(stream.Tell() == size);

    // Check as3 code section
    uint8_t *as3_ptr = as3_code_section.data();
    size_t max_as3_size = as3_code_section.size();

    if (max_as3_size != 0)
    {
        if (max_as3_size < 0x10 || *(uint32_t *)&as3_ptr[8] > max_as3_size-0xC)
        {
            DPRINTF("%s: Failed at AS3 size.\n", FUNCNAME);
            return false;
        }
    }

    return true;
}

bool IggyFile::Load(const uint8_t *buf, size_t size)
{
    if (size < sizeof(IGGYHeader))
        return false;

    const IGGYHeader *hdr = (IGGYHeader *)buf;
    const uint8_t *top = buf + size;

    if (hdr->signature != IGGY_SIGNATURE)
    {
        DPRINTF("%s: Invalid iggy signature.\n", FUNCNAME);
        return false;
    }

    if (hdr->version != 0x900)
    {
        DPRINTF("%s: Unknown file version. Aborting load.\n", FUNCNAME);
        return false;
    }

    if (hdr->plattform[0] != 1 || hdr->plattform[2] != 1)
    {
        DPRINTF("%s: Unknown plattform data. Aborting load.\n", FUNCNAME);
        return false;
    }

    if (hdr->plattform[1] == 32)
    {
        is_64 = false;
    }
    else if (hdr->plattform[1] == 64)
    {
        is_64 = true;
    }
    else
    {
        DPRINTF("%s: file is for unknonwn pointer size %d\n", FUNCNAME, hdr->plattform[1]);
        return false;
    }

    unk_0C = hdr->unk_0C;

    if (hdr->num_subfiles != 2)
    {
        DPRINTF("%s: I can't process a number of subfiles different to 2.\n", FUNCNAME);
        return false;
    }

    IGGYSubFileEntry *entries = (IGGYSubFileEntry *)(hdr+1);

    if (entries[0].id != 1)
    {
        DPRINTF("%s: Cannot process first section id different to 1.\n", FUNCNAME);
        return false;
    }

    if (entries[1].id != 0)
    {
        DPRINTF("%s: Cannot process second section id different to 0.\n", FUNCNAME);
        return false;
    }

    uint8_t *index_section = GetOffsetPtr(buf, entries[1].offset);
    if (index_section + entries[1].size > top)
        return false;

    index_data.resize(entries[1].size);
    memcpy(index_data.data(), index_section, index_data.size());

    uint8_t *flash_section = GetOffsetPtr(buf, entries[0].offset);
    if (flash_section + entries[0].size > top)
        return false;

    FixedMemoryStream stream(flash_section, entries[0].size);

    if (is_64)
        return LoadFlashData64(stream);

    return LoadFlashData32(stream);
}

void IggyFile::SaveFlashData32(uint8_t *buf, size_t size)
{
    IGGYFlashHeader32 *hdr = (IGGYFlashHeader32 *)buf;

    memcpy(buf, main_section.data(), main_section.size());
    memcpy(buf+main_section.size(), as3_names_section.data(), as3_names_section.size());
    memcpy(buf+main_section.size()+as3_names_section.size(), as3_code_section.data(), as3_code_section.size());
    memcpy(buf+main_section.size()+as3_names_section.size()+as3_code_section.size(), names_section.data(), names_section.size());
    memcpy(buf+main_section.size()+as3_names_section.size()+as3_code_section.size()+names_section.size(), last_section.data(), last_section.size());

    hdr->as3_section_offset = (uint32_t)main_section.size();

    if (hdr->names_offset != 1)
        hdr->names_offset = (uint32_t) (main_section.size()+as3_names_section.size()+as3_code_section.size()-offsetof(IGGYFlashHeader32, names_offset));

    if (hdr->last_section_offset != 1)
        hdr->last_section_offset = (uint32_t) (size-last_section.size()-offsetof(IGGYFlashHeader32, last_section_offset));

    if (hdr->as3_code_offset != 1)
        hdr->as3_code_offset = (uint32_t) (main_section.size()+as3_names_section.size()-offsetof(IGGYFlashHeader32, as3_code_offset));

    if (hdr->as3_names_offset != 1)
        hdr->as3_names_offset = (uint32_t) (main_section.size()+8-offsetof(IGGYFlashHeader32, as3_names_offset));
}

void IggyFile::SaveFlashData64(uint8_t *buf, size_t size)
{
    IGGYFlashHeader64 *hdr = (IGGYFlashHeader64 *)buf;

    memcpy(buf, main_section.data(), main_section.size());
    memcpy(buf+main_section.size(), as3_names_section.data(), as3_names_section.size());
    memcpy(buf+main_section.size()+as3_names_section.size(), as3_code_section.data(), as3_code_section.size());
    memcpy(buf+main_section.size()+as3_names_section.size()+as3_code_section.size(), names_section.data(), names_section.size());
    memcpy(buf+main_section.size()+as3_names_section.size()+as3_code_section.size()+names_section.size(), last_section.data(), last_section.size());

    hdr->as3_section_offset = (uint64_t)main_section.size();

    if (hdr->names_offset != 1)
        hdr->names_offset = (uint64_t) (main_section.size()+as3_names_section.size()+as3_code_section.size()-offsetof(IGGYFlashHeader64, names_offset));

    if (hdr->last_section_offset != 1)
        hdr->last_section_offset = (uint64_t) (size-last_section.size()-offsetof(IGGYFlashHeader64, last_section_offset));

    if (hdr->as3_code_offset != 1)
        hdr->as3_code_offset = (uint64_t) (main_section.size()+as3_names_section.size()-offsetof(IGGYFlashHeader64, as3_code_offset));

    if (hdr->as3_names_offset != 1)
        hdr->as3_names_offset = (uint64_t) (main_section.size()+0xC-offsetof(IGGYFlashHeader64, as3_names_offset));
}

size_t IggyFile::GetFlashDataSize()
{
    return main_section.size() + as3_names_section.size() + as3_code_section.size() + names_section.size() + last_section.size();
}

uint8_t *IggyFile::Save(size_t *psize)
{
    size_t size = sizeof(IGGYHeader) + 2*sizeof(IGGYSubFileEntry);
    uint32_t offset = (uint32_t)size;

    size += GetFlashDataSize();
    size += index_data.size();

    uint8_t *buf = new uint8_t[size];
    memset(buf, 0, size);

    IGGYHeader *hdr = (IGGYHeader *)buf;
    IGGYSubFileEntry *entries = (IGGYSubFileEntry *)(hdr+1);

    hdr->signature = IGGY_SIGNATURE;
    hdr->version = 0x900;
    hdr->plattform[0] = hdr->plattform[2] = 1;
    hdr->plattform[1] = (is_64) ? 64 : 32;
    hdr->unk_0C = unk_0C;
    hdr->num_subfiles = 2;

    entries[0].id = 1;
    entries[0].size = entries[0].size2 = (uint32_t)GetFlashDataSize();
    entries[0].offset = offset;

    if (is_64)
        SaveFlashData64(buf+offset, GetFlashDataSize());
    else
        SaveFlashData32(buf+offset, GetFlashDataSize());

    offset += entries[0].size;

    entries[1].id = 0;
    entries[1].size = entries[1].size2 = (uint32_t)index_data.size();
    entries[1].offset = offset;
    memcpy(buf+offset, index_data.data(), index_data.size());
    offset += entries[1].size;

    assert(offset == size);

    *psize = size;
    return buf;
}

uint8_t *IggyFile::GetAbcBlob(uint32_t *psize) const
{
    if (as3_code_section.size() == 0)
        return nullptr;

    const uint8_t *ptr = as3_code_section.data();
    *psize = (is_64) ? *(uint32_t *)&ptr[8] : *(uint32_t *)&ptr[4];

    uint8_t *buf = new uint8_t[*psize];
    memcpy(buf, (is_64) ? ptr+0xC : ptr+8, *psize);
    return buf;
}

bool IggyFile::SetAbcBlob(void *buf, uint32_t size)
{
    bool hack = false;
    uint32_t old_size = (uint32_t)as3_code_section.size();

    if (old_size == 0)
        return false;

    uint32_t new_size = (uint32_t)Utils::Align2(size + ((is_64) ? 0xC : 8), 0x10);
    // HACK
    if (names_section.size() == 0 && last_section.size() == 0)
    {
        new_size += ((is_64) ? 0x10 : 8);
        hack = true;
    }

    as3_code_section.resize(new_size);
    uint8_t *ptr = as3_code_section.data();

    if (is_64)
    {
        memset(ptr+0xC, 0, new_size-0xC);
        memcpy(ptr+0xC, buf, size);
        *(uint32_t *)&ptr[8] = size;
    }
    else
    {
        memset(ptr+8, 0, new_size-8);
        memcpy(ptr+8, buf, size);
        *(uint32_t *)&ptr[4] = size;
    }

    if (hack)
    {
        if (is_64)
            *(uint32_t *)&ptr[new_size-0x10] = 1;
        else
            *(uint32_t *)&ptr[new_size-8] = 1;
    }

    if (new_size == old_size)
        return true;

    return RebuildIndexAfterAbc(new_size-old_size);
}

bool IggyFile::RebuildIndexAfterAbc(int32_t diff)
{
    FixedMemoryStream stream(index_data.data(), index_data.size());
    std::vector<uint8_t> index_table;
    uint8_t index_table_size;

    stream.Read8(&index_table_size);
    index_table.resize(index_table_size);

    for (uint8_t &offset : index_table)
    {
        uint8_t num;

        stream.Read8(&offset);
        //DPRINTF("index_table_entry: %02x\n", offset);

        stream.Read8(&num);
        stream.Seek(num*2, SEEK_CUR);
    }

    int state = 0;
    uint32_t offset = 0;
    uint8_t code;

    while (stream.Read8(&code))
    {
        if (state == 1)
        {
            if (code != 0xFD)
            {
                DPRINTF("%s: We were expecting code 0xFD in state 1.\n", FUNCNAME);
                return false;
            }
        }
        else if (state == 2)
        {
            if (code != 0xFF)
            {
                DPRINTF("%s: We were expecting code 0xFF in state 2.\n", FUNCNAME);
                return false;
            }
        }

        if (code < 0x80)  // 0-0x7F
        {
            // code is directly an index to the index_table
            if (code >= index_table_size)
            {
                DPRINTF("< 0x80: index is greater than index_table_size. %x > %x\n", code, index_table_size);
                return false;
            }

            offset += index_table[code];
        }
        else if (code < 0xC0) // 0x80-BF
        {
            uint8_t index;

            if (!stream.Read8(&index))
            {
                DPRINTF("< 0xC0: Cannot read index.\n");
                return false;
            }

            if (index >= index_table_size)
            {
                DPRINTF("< 0xC0: index is greater than index_table_size. %x > %x\n", index, index_table_size);
                return false;
            }

            int n = code-0x7F;
            offset += index_table[index]*n;
        }
        else if (code < 0xD0) // 0xC0-0xCF
        {
            offset += ((code*2)-0x17E);
        }
        else if (code < 0xE0) // 0xD0-0xDF
        {
            // Code here depends on plattform[0], we are assuming it is 1, as we checked in load function
            uint8_t i = code&0xF;
            uint8_t n8;
            int n;

            if (!stream.Read8(&n8))
            {
                DPRINTF("< 0xE0: Cannot read n.\n");
                return false;
            }

            n = n8+1;            

            if (is_64)
            {
                if (i <= 2)
                {
                    offset += 8*n; // Ptr type
                }
                else if (i <= 4)
                {
                    offset += 2*n;
                }
                else if (i == 5)
                {
                    offset += 4*n;
                }
                else if (i == 6)
                {
                    offset += 8*n; // 64 bits type
                }
                else
                {
                    DPRINTF("< 0xE0: Invalid value for i (%x %x)\n", i, code);
                }
            }
            else
            {
                switch (i)
                {
                    case 2:
                        offset += 4*n;  // Ptr type
                    break;

                    case 4:
                        offset += 2*n;
                    break;

                    case 5:
                        offset += 4*n; // 32 bits type
                    break;

                    case 6:
                        offset += 8*n;
                    break;

                    default:
                        DPRINTF("< 0xE0: invalid value for i (%x %x)\n", i, code);
                }
            }
        }
        else if (code == 0xFC)
        {
            stream.Seek(1, SEEK_CUR);
        }
        else if (code == 0xFD)
        {
            uint8_t n, m;

            if (!stream.Read8(&n))
            {
                DPRINTF("0xFD: Cannot read n.\n");
                return false;
            }

            if (state == 1)
            {
                if (is_64)
                {
                    if (n != 0xF)
                    {
                        DPRINTF("%s: we were expecting an offset of 0xF in state 1.\n", FUNCNAME);
                        return false;
                    }
                }
                else
                {
                    if (n != 0xB)
                    {
                        DPRINTF("%s: we were expecting an offset of 0xB in state 1.\n", FUNCNAME);
                        return false;
                    }
                }

                state = 2;
            }

            if (!stream.Read8(&m))
            {
                DPRINTF("0xFD: Cannot read m.\n");
                return false;
            }

            offset += n;
            stream.Seek(m*2, SEEK_CUR);
        }
        else if (code == 0xFE)
        {
            uint8_t n8;
            int n;

            if (!stream.Read8(&n8))
            {
                DPRINTF("0xFE: Cannot read n.\n");
                return false;
            }

            n = n8+1;
            offset += n;
        }
        else if (code == 0xFF)
        {
            uint32_t n;

            if (!stream.Read32(&n))
            {
                DPRINTF("0xFF: Cannot read n.\n");
                return false;
            }

            if (state == 2)
            {
                n += diff;
                stream.Seek(-4, SEEK_CUR);
                return stream.Write32(n);
            }

            offset += n;
        }
        else
        {
            DPRINTF("Unrecognized code: %x\n", code);
        }

        if (state == 0 && offset == as3_offset)
            state = 1;
    }

    return false;
}

void IggyFile::PrintIndex() const
{
    FixedMemoryStream stream(const_cast<uint8_t *>(index_data.data()), index_data.size());
    std::vector<uint8_t> index_table;
    uint8_t index_table_size;

    stream.Read8(&index_table_size);
    index_table.resize(index_table_size);

    for (uint8_t &offset : index_table)
    {
        uint8_t num;

        stream.Read8(&offset);
        DPRINTF("index_table_entry: %02x\n", offset);

        stream.Read8(&num);
        stream.Seek(num*2, SEEK_CUR);
    }

    uint32_t offset = 0;
    uint8_t code;

    while (stream.Read8(&code))
    {
        //DPRINTF("Code = %x\n", code);

        if (code < 0x80)  // 0-0x7F
        {
            // code is directly an index to the index_table
            if (code >= index_table_size)
            {
                DPRINTF("< 0x80: index is greater than index_table_size. %x > %x\n", code, index_table_size);
                return;
            }

            offset += index_table[code];
        }
        else if (code < 0xC0) // 0x80-BF
        {
            uint8_t index;

            if (!stream.Read8(&index))
            {
                DPRINTF("< 0xC0: Cannot read index.\n");
                return;
            }

            if (index >= index_table_size)
            {
                DPRINTF("< 0xC0: index is greater than index_table_size. %x > %x\n", index, index_table_size);
                return;
            }

            int n = code-0x7F;
            offset += index_table[index]*n;
        }
        else if (code < 0xD0) // 0xC0-0xCF
        {
            offset += ((code*2)-0x17E);
        }
        else if (code < 0xE0) // 0xD0-0xDF
        {
            // Code here depends on plattform[0], we are assuming it is 1, as we checked in load function
            uint8_t i = code&0xF;
            uint8_t n8;
            int n;

            if (!stream.Read8(&n8))
            {
                DPRINTF("< 0xE0: Cannot read n.\n");
                return;
            }

            n = n8+1;

            if (is_64)
            {
                if (i <= 2)
                {
                    offset += 8*n; // Ptr type
                }
                else if (i <= 4)
                {
                    offset += 2*n;
                }
                else if (i == 5)
                {
                    offset += 4*n;
                }
                else if (i == 6)
                {
                    offset += 8*n; // 64 bits type
                }
                else
                {
                    DPRINTF("< 0xE0: Invalid value for i (%x %x)\n", i, code);
                }
            }
            else
            {
                switch (i)
                {
                    case 2:
                        offset += 4*n;  // Ptr type
                    break;

                    case 4:
                        offset += 2*n;
                    break;

                    case 5:
                        offset += 4*n; // 32 bits type
                    break;

                    case 6:
                        offset += 8*n;
                    break;

                    default:
                        DPRINTF("< 0xE0: invalid value for i (%x %x)\n", i, code);
                }
            }
        }
        else if (code == 0xFC)
        {
            stream.Seek(1, SEEK_CUR);
        }
        else if (code == 0xFD)
        {
            uint8_t n, m;

            if (!stream.Read8(&n))
            {
                DPRINTF("0xFD: Cannot read n.\n");
                return;
            }

            if (!stream.Read8(&m))
            {
                DPRINTF("0xFD: Cannot read m.\n");
                return;
            }

            offset += n;
            stream.Seek(m*2, SEEK_CUR);
        }
        else if (code == 0xFE)
        {
            uint8_t n8;
            int n;

            if (!stream.Read8(&n8))
            {
                DPRINTF("0xFE: Cannot read n.\n");
                return;
            }

            n = n8+1;
            offset += n;
        }
        else if (code == 0xFF)
        {
            uint32_t n;

            if (!stream.Read32(&n))
            {
                DPRINTF("0xFF: Cannot read n.\n");
                return;
            }

            offset += n;
        }
        else
        {
            DPRINTF("Unrecognized code: %x\n", code);
        }

        DPRINTF("Offset = %x\n", offset);
    }
}
