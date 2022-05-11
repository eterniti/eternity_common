#include "IggyTexFile.h"

IggyTexContainedFile::IggyTexContainedFile()
{
    buf = nullptr;
    Reset();
}

IggyTexContainedFile::~IggyTexContainedFile()
{
    Reset();
}

void IggyTexContainedFile::Copy(const IggyTexContainedFile &other)
{
    Reset();

    if (other.buf)
    {
        this->buf = new uint8_t[other.size];
        memcpy(this->buf, other.buf, other.size);

        this->size = other.size;
        this->allocated = true;
    }

    this->id = other.id;
}

void IggyTexContainedFile::Reset()
{
    if (buf && allocated)
    {
        delete[] buf;
    }

    buf = nullptr;
    size = 0;
    allocated = false;
    id = 0;
}

IggyTexFile::IggyTexFile()
{
    buf = nullptr;
    size = 0;
    big_endian = false;
}

IggyTexFile::IggyTexFile(const uint8_t *buf, size_t size)
{
    Load(buf, size);
}

IggyTexFile::~IggyTexFile()
{
    Reset();
}

void IggyTexFile::Copy(const IggyTexFile &other)
{
    this->buf = nullptr;
    this->size = 0;
    this->big_endian = other.big_endian;

    this->files = other.files;
}

void IggyTexFile::Reset()
{
    files.clear();

    if (buf)
    {
        delete[] buf;
        buf = nullptr;
    }

    size = 0;
}

bool IggyTexFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    IGGYTEXHeader *hdr = (IGGYTEXHeader *)buf;

    if (size < sizeof(IGGYTEXHeader) || hdr->signature != IGGYTEX_SIGNATURE)
        return false;

    this->buf = new uint8_t[size];
    memcpy(this->buf, buf, size);
    this->size = size;

    hdr = (IGGYTEXHeader *)this->buf;
    IGGYTEXTable *table = (IGGYTEXTable *)GetOffsetPtr(hdr, hdr->table_start);

    files.resize(hdr->num_files);

    for (uint32_t i = 0; i < hdr->num_files; i++)
    {
        files[i].buf = this->buf + table[i].offset;
        files[i].size = table[i].size;
        files[i].id = table[i].id;
        files[i].allocated = false;
    }

    return true;
}

uint32_t IggyTexFile::FindIndex(uint32_t id)
{
    for (size_t i = 0; i < files.size(); i++)
    {
        if (files[i].id == id)
            return (uint32_t)i;
    }

    return (uint32_t)-1;
}

bool IggyTexFile::ReplaceFileByID(uint32_t id, const uint8_t *buf, size_t size)
{
    uint32_t idx = FindIndex(id);
    if (idx == (uint32_t)-1)
        return false;

    return ReplaceFile(idx, buf, size);
}

bool IggyTexFile::ReplaceFile(uint32_t idx, const uint8_t *buf, size_t size)
{
    if (idx >= files.size())
        return false;

    uint8_t *copy = new uint8_t[size];

    if (files[idx].allocated)
    {
        delete[] files[idx].buf;
    }

    memcpy(copy, buf, size);
    files[idx].buf = copy;
    files[idx].size = size;
    files[idx].allocated = true;

    return true;
}

uint32_t IggyTexFile::AppendFile(const uint8_t *buf, size_t size, uint32_t id, bool take_ownership)
{
    IggyTexContainedFile file;
    uint32_t idx = (uint32_t)files.size();

    if (take_ownership)
    {
        file.buf = const_cast<uint8_t *>(buf);
    }
    else
    {

        uint8_t *copy = new uint8_t[size];
        memcpy(copy, buf, size);
        file.buf = copy;
    }

    file.size = size;
    file.allocated = true;
    file.id = id;

    files.push_back(file);
    return idx;
}

uint32_t IggyTexFile::AppendFile(const IggyTexFile &other, uint32_t idx_other)
{
    if (idx_other >= other.files.size())
        return (uint32_t)-1;

    return AppendFile(other.files[idx_other]);
}

uint32_t IggyTexFile::AppendFile(const IggyTexContainedFile &file)
{
    return AppendFile(file.buf, file.size, file.id);
}

void IggyTexFile::RemoveFile(uint32_t idx)
{
    if (idx >= files.size())
        return;

    files.erase(files.begin()+idx);
}
