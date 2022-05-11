#include "IggyFile.h"
#include "FixedMemoryStream.h"
#include "debug.h"

// WIP File. We need assert always on
#undef NDEBUG

IggyFlashFile::IggyFlashFile()
{
    this->big_endian = false;
}

IggyFlashFile::~IggyFlashFile()
{
}

size_t IggyFlashFile::GetSize() const
{
    return main_section.size() + as3_names_section.size() + as3_code_section.size() + names_section.size() + last_section.size();
}

bool IggyFlashFile::Load(const uint8_t *buf, size_t size)
{
    FixedMemoryStream stream(const_cast<uint8_t *>(buf), size);
    IGGYFlashHeader hdr;

    if (!stream.Read(&hdr, sizeof(hdr)))
        return false;

    if (hdr.header_size != 0x80)
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
        next_offset = hdr.as3_code_offset + offsetof(IGGYFlashHeader, as3_code_offset);
    }
    else
    {
        if (hdr.names_offset != 1)
        {
            next_offset = hdr.names_offset + offsetof(IGGYFlashHeader, names_offset);
        }
        else
        {
            if (hdr.last_section_offset != 1)
            {
                next_offset = hdr.last_section_offset + offsetof(IGGYFlashHeader, last_section_offset);
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
            next_offset = hdr.names_offset + offsetof(IGGYFlashHeader, names_offset);
        }
        else
        {
            if (hdr.last_section_offset != 1)
            {
                next_offset = hdr.last_section_offset + offsetof(IGGYFlashHeader, last_section_offset);
            }
            else
            {
                next_offset = size;
            }
        }

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
            next_offset = hdr.last_section_offset + offsetof(IGGYFlashHeader, last_section_offset);
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

uint8_t *IggyFlashFile::Save(size_t *psize)
{
    size_t size = GetSize();
    uint8_t *buf = new uint8_t[size];
    IGGYFlashHeader *hdr = (IGGYFlashHeader *)buf;

    memcpy(buf, main_section.data(), main_section.size());
    memcpy(buf+main_section.size(), as3_names_section.data(), as3_names_section.size());
    memcpy(buf+main_section.size()+as3_names_section.size(), as3_code_section.data(), as3_code_section.size());   
    memcpy(buf+main_section.size()+as3_names_section.size()+as3_code_section.size(), names_section.data(), names_section.size());
    memcpy(buf+main_section.size()+as3_names_section.size()+as3_code_section.size()+names_section.size(), last_section.data(), last_section.size());

    hdr->as3_section_offset = (uint32_t)main_section.size();

    if (hdr->names_offset != 1)
        hdr->names_offset = (uint32_t) (main_section.size()+as3_names_section.size()+as3_code_section.size()-offsetof(IGGYFlashHeader, names_offset));

    if (hdr->last_section_offset != 1)
        hdr->last_section_offset = (uint32_t) (size-last_section.size()-offsetof(IGGYFlashHeader, last_section_offset));

    if (hdr->as3_code_offset != 1)
        hdr->as3_code_offset = (uint32_t) (main_section.size()+as3_names_section.size()-offsetof(IGGYFlashHeader, as3_code_offset));

    if (hdr->as3_names_offset != 1)
        hdr->as3_names_offset = (uint32_t) (main_section.size()+8-offsetof(IGGYFlashHeader, as3_names_offset));

    *psize = size;
    return buf;
}

uint8_t *IggyFlashFile::GetAbcBlob(uint32_t *psize) const
{
    if (as3_code_section.size() == 0)
        return nullptr;

    const uint8_t *ptr = as3_code_section.data();
    *psize = *(uint32_t *)&ptr[4];

    uint8_t *buf = new uint8_t[*psize];
    memcpy(buf, ptr+8, *psize);
    return buf;
}

bool IggyFlashFile::SetAbcBlob(void *buf, uint32_t size)
{
    bool hack = false;

    if (as3_code_section.size() == 0)
        return false;

    uint32_t new_size = (uint32_t)Utils::Align2(size, 8) + 8;
    // HACK
    if (names_section.size() == 0 && last_section.size() == 0)
    {
        new_size += 8;
        hack = true;
    }

    as3_code_section.resize(new_size);
    uint8_t *ptr = as3_code_section.data();

    memset(ptr+8, 0, new_size-8);
    memcpy(ptr+8, buf, size);
    *(uint32_t *)&ptr[4] = size;

    if (hack)
        *(uint32_t *)&ptr[new_size-8] = 1;

    return true;
}

IggyFile::IggyFile()
{
    this->big_endian = false;
}

IggyFile::~IggyFile()
{
    Reset();
}

void IggyFile::Reset()
{
    for (IggySubFile *sf : sub_files)
    {
        if (sf)
        {
            delete sf;
            sf = nullptr;
        }
    }

    sub_files.clear();
}

bool IggyFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (size < sizeof(IGGYHeader))
        return false;

    const IGGYHeader *hdr = (const IGGYHeader *)buf;

    if (hdr->signature != IGGY_SIGNATURE)
        return false;

    if (hdr->version != 0x900)
    {
        DPRINTF("%s: Wrong plattform.\n", FUNCNAME);
        return false;
    }

    plattform = hdr->plattform;
    sub_files.resize(hdr->num_subfiles, nullptr);

    const uint8_t *top = buf + size;
    const IGGYSubFileEntry *entry = (const IGGYSubFileEntry *)(hdr+1);
    size -= sizeof(IGGYHeader);

    for (IggySubFile *&sf : sub_files)
    {
        if (size < sizeof(IGGYSubFileEntry))
            return false;

        const uint8_t *ptr = buf + entry->offset;

        if (ptr + entry->size > top)
            return false;

        if (entry->size >= sizeof(IGGYFlashHeader) && *(uint32_t *)ptr == 0x80)
        {
            sf = new IggyFlashFile();
        }
        else
        {
            sf = new IggyGenericSubFile();
        }

        if (!sf->Load(ptr, entry->size))
            return false;

        sf->SetId(entry->id);

        entry++;
        size -= sizeof(IGGYSubFileEntry);
    }

    return true;
}

uint8_t *IggyFile::Save(size_t *psize)
{
    size_t size = sizeof(IGGYHeader) + sub_files.size()*sizeof(IGGYSubFileEntry);
    uint32_t offset = (uint32_t) size;

    for (IggySubFile *sf : sub_files)
    {
        size += sf->GetSize();
    }

    uint8_t *buf = new uint8_t[size];
    memset(buf, 0, size);

    IGGYHeader *hdr = (IGGYHeader *)buf;
    IGGYSubFileEntry *entries = (IGGYSubFileEntry *)(hdr+1);

    hdr->signature = IGGY_SIGNATURE;
    hdr->version = 0x900;
    hdr->plattform = plattform;
    hdr->num_subfiles = (uint32_t)sub_files.size();

    for (size_t i = 0; i < sub_files.size(); i++)
    {
        IggySubFile *sf = sub_files[i];

        size_t sf_size;
        uint8_t *sf_buf = sf->Save(&sf_size);

        if (!sf_buf)
        {
            delete[] buf;
            return nullptr;
        }

        entries[i].id = sf->GetID();
        entries[i].size = entries[i].size2 = sf_size;
        entries[i].offset = offset;

        memcpy(buf+offset, sf_buf, sf_size);

        offset += (uint32_t)sf_size;
        delete[] sf_buf;
    }

    *psize = size;
    return buf;
}
