#include "FixedMemoryStream.h"

FixedMemoryStream::FixedMemoryStream(uint8_t *buf, size_t size) : MemoryStream()
{
    big_endian = false;
    mem = buf;
    file_size = capacity = size;
    file_pos = 0;    
}

FixedMemoryStream::~FixedMemoryStream()
{
    mem = nullptr;
}

bool FixedMemoryStream::Copy(Stream *other, size_t size)
{
    if (file_pos+size > file_size)
        return false;

    bool ret = other->Read(mem+file_pos, size);
    if (ret)
    {
        file_pos += size;
    }

    return ret;
}
