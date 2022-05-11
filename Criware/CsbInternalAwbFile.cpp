#include "CsbInternalAwbFile.h"

bool CsbInternalAwbFile::Load(const uint8_t *buf, size_t size)
{
    if (!UtfFile::Load(buf, size))
        return false;

    entries.clear();
    entries.resize(GetNumRows());

    bool at_least_one = false;

    for (unsigned int i = 0; i < GetNumRows(); i++)
    {
        unsigned int size;
        FileEntry &entry = entries[i];

        entry.buf = GetBlob("data", &size, false, i);
        if (!entry.buf)
        {
            // Some files may have a data with 0. Found this in BTL_CMN.csb of USFIV
            entry.buf = new uint8_t[0];
        }
        else
        {
            at_least_one = true;
        }

        entry.size = size;
        entry.allocated = false;
    }

    if (!at_least_one)
        return true;

    return true;
}

uint8_t *CsbInternalAwbFile::Save(size_t *psize)
{
    for (unsigned int i = 0; i < GetNumRows(); i++)
    {
        const FileEntry &entry = entries[i];

        if (entry.allocated)
        {
            if (!SetBlob("data", entry.buf, (unsigned int)entry.size, i))
                return nullptr;
        }
    }

    return UtfFile::Save(psize);
}

bool CsbInternalAwbFile::LoadFromFile(const std::string &path, bool show_error)
{
    return BaseFile::LoadFromFile(path, show_error);
}

bool CsbInternalAwbFile::SaveToFile(const std::string &path, bool show_error, bool build_path)
{
    return BaseFile::SaveToFile(path, show_error, build_path);
}

bool CsbInternalAwbFile::ExtractFile(uint32_t idx, const std::string &path, bool auto_path) const
{
    if (auto_path)
        return false; // Not implemented

    uint64_t size;
    uint8_t *buf = ExtractFile(idx, &size);

    bool ret = Utils::WriteFileBool(path, buf, size, true, true);

    delete[] buf;
    return ret;
}

uint8_t *CsbInternalAwbFile::ExtractFile(uint32_t idx, uint64_t *psize) const
{
    if (idx >= entries.size())
        return false;

    const FileEntry &entry = entries[idx];

    uint8_t *buf = new uint8_t[entry.size];
    memcpy(buf, entry.buf, entry.size);

    *psize = entry.size;
    return buf;
}

bool CsbInternalAwbFile::SetFile(uint32_t idx, void *buf, uint64_t size, bool take_ownership)
{
    if (idx >= entries.size())
        return false;

    FileEntry &entry = entries[idx];

    entry.size = size;
    entry.allocated = true;

    if (take_ownership)
    {
        entry.buf = (uint8_t *)buf;
    }
    else
    {
        entry.buf = new uint8_t[size];
        memcpy(entry.buf, buf, size);
    }

    return true;
}

bool CsbInternalAwbFile::SetFile(uint32_t idx, const std::string &path)
{
    size_t size;
    uint8_t *buf = Utils::ReadFile(path, &size);

    if (!buf)
        return false;

    return SetFile(idx, buf, size, true);
}
