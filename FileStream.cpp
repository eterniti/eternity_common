#include "FileStream.h"
#include "debug.h"


FileStream::FileStream(const std::string &mode) : mode(mode)
{
    handle = nullptr;
    Reset();
}

FileStream::~FileStream()
{
    Reset();
}

bool FileStream::SetRegion(uint64_t start, uint64_t size)
{
    if (start + size > file_capacity)
        return false;

    file_start = start;
    stream_size = stream_capacity = size;

    return Seek(0, SEEK_SET);
}

void FileStream::Reset()
{
    big_endian = false;

    if (handle)
    {
        fclose(handle);
        handle = nullptr;
    }

    stream_size = stream_pos = stream_capacity = file_start = file_capacity = 0;
}

bool FileStream::Resize(uint64_t size)
{
    if (size > stream_capacity)
        return false;

    stream_size = size;
    if (stream_pos > size)
        stream_pos = size;

    return true;
}

bool FileStream::Read(void *buf, size_t size)
{
    if (!handle)
        return false;

    off64_t pos = ftello64(handle);
    size_t rd = fread(buf, 1, size, handle);

    if (rd != size)
    {
        fseeko64(handle, pos, SEEK_SET);
        return false;
    }

    stream_pos += rd;
    return true;
}

bool FileStream::Write(const void *buf, size_t size)
{
    if (!handle)
        return false;

    off64_t pos = ftello64(handle);
    size_t rd = fwrite(buf, 1, size, handle);

    if (rd != size)
    {
        fseeko64(handle, pos, SEEK_SET);
        return false;
    }

    if (stream_pos+size > stream_size)
    {
        stream_size = stream_pos+size;
        if (stream_capacity < stream_size)
            stream_capacity = stream_size;
    }

    stream_pos += rd;
    return true;
}

bool FileStream::Reopen(const std::string &mode)
{
    if (!handle)
        return false;

    fclose(handle);
    handle = nullptr;
    this->mode = mode;
    return LoadFromFile(saved_path, false);
}

bool FileStream::Seek(int64_t offset, int whence)
{
    uint64_t new_pos;

    if (whence == SEEK_SET)
    {
        new_pos = (uint64_t)offset;
    }
    else if (whence == SEEK_CUR)
    {
        new_pos = stream_pos + offset;
    }
    else if (whence == SEEK_END)
    {
        new_pos = stream_size + offset;
    }
    else
    {
        return false;
    }

    if (new_pos > stream_size)
        return false;

    if (!handle && !new_pos)
        return false;

    if (fseeko64(handle, new_pos+file_start, SEEK_SET) != 0)
        return false;

    stream_pos = new_pos;
    return true;
}

uint8_t *FileStream::Save(size_t *psize)
{
    if (stream_size == 0 || !handle)
        return nullptr;

    uint8_t *buf = new uint8_t[stream_size];

    uint64_t save_pos = stream_pos;
    Seek(0, SEEK_SET);

    bool ret = Read(buf, stream_size);
    if (!ret)
        delete[] buf;

    Seek(save_pos, SEEK_SET);

    *psize = stream_size;
    return (ret) ? buf : nullptr;
}

bool FileStream::LoadFromFile(const std::string &path, bool show_error)
{
    bool create_path = false;

    if (mode.length() > 0 && mode[0] == 'w')
        create_path = true;

    handle = (create_path) ? Utils::fopen_create_path(path, mode.c_str()) : fopen(path.c_str(), mode.c_str());
    if (!handle)
    {
        if (show_error)
        {
            DPRINTF("%s: Cannot open file \"%s\"\n", FUNCNAME, path.c_str());
        }

        return false;
    }

    fseeko64(handle, 0, SEEK_END);
    stream_size = stream_capacity = file_capacity = ftello64(handle);
    fseeko64(handle, 0, SEEK_SET);

    stream_pos = file_start = 0;
    saved_path = path;
    return true;
}

bool FileStream::SaveToFile(const std::string &path, bool show_error, bool build_path)
{
    if (!handle)
        return false;

    FILE *w = (build_path) ? Utils::fopen_create_path(path, "wb") : fopen(path.c_str(), "wb");
    if (!w)
    {
        if (show_error)
        {
            DPRINTF("%s: Cannot open/create file \"%s\"\n", FUNCNAME, path.c_str());
        }

        return false;
    }

    uint64_t save_pos = stream_pos;
    Seek(0, SEEK_SET);

    bool ret = Utils::DoCopyFile(handle, w, stream_size);
    fclose(w);

    Seek(save_pos, SEEK_SET);
    return ret;
}
