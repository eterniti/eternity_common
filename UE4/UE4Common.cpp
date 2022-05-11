#include "UE4Common.h"
#include "debug.h"

bool UE4Common::FString::Read(FILE *file)
{
    int32_t len;

    str.clear();
    ucs2_str.clear();
    is_ucs2 = false;

    if (fread(&len, 1, sizeof(int32_t), file) != sizeof(int32_t))
        return false;

    if (len == 0)
    {
        return true;
    }

    if (len < 0)
    {
        len = -len;
        is_ucs2 = true;
    }

    if (len >= 0x1000)
    {
        DPRINTF("%s: Warning, string seems to be very big (0x%x bytes)\n", FUNCNAME, len);
    }

    if (!is_ucs2)
    {
        char *buf = new char[len+1];
        buf[len] = 0; // Although the string already includes the zero, lets add one after the read size for security

        if (fread(buf, 1, len, file) != (size_t)len)
        {
            delete[] buf;
            return false;
        }

        str = buf;
        delete[] buf;
    }
    else
    {
        char16_t *buf = new char16_t[len+1];
        buf[len] = 0; // Although the string already includes the zero, lets add one after the read size for security

        if (fread(buf, 1, len*2, file) != (size_t)(len*2))
        {
            delete[] buf;
            return false;
        }

        ucs2_str = buf;
        delete[] buf;
    }

    return true;
}

bool UE4Common::FString::Read(Stream *stream)
{
    int32_t len;

    str.clear();
    ucs2_str.clear();
    is_ucs2 = false;

    if (!stream->Read32(&len))
        return false;

    if (len == 0)
    {
        return true;
    }

    if (len < 0)
    {
        len = -len;
        is_ucs2 = true;
    }

    if (len >= 0x1000)
    {
        DPRINTF("%s: Warning, string seems to be very big (0x%x bytes)\n", FUNCNAME, len);
    }

    if (!is_ucs2)
    {
        char *buf = new char[len+1];
        buf[len] = 0; // Although the string already includes the zero, lets add one after the read size for security

        if (!stream->Read(buf, len))
        {
            delete[] buf;
            return false;
        }

        str = buf;
        delete[] buf;
    }
    else
    {
        char16_t *buf = new char16_t[len+1];
        buf[len] = 0; // Although the string already includes the zero, lets add one after the read size for security

        if (!stream->Read(buf, len*2))
        {
            delete[] buf;
            return false;
        }

        ucs2_str = buf;
        delete[] buf;
    }

    return true;
}

bool UE4Common::FString::Write(FILE *file) const
{
    int32_t length = (int32_t)Length();

    if (length != 0)
    {
        length += 1;
    }

    if (is_ucs2)
        length = -length;

    if (fwrite(&length, 1, sizeof(int32_t), file) != sizeof(int32_t))
        return false;

    if (length != 0)
    {
        uint32_t size = Size();

        if (!is_ucs2)
        {
            if (fwrite(str.c_str(), 1, size, file) != size)
                return false;
        }
        else
        {
            if (fwrite(ucs2_str.c_str(), 1, size, file) != size)
                return false;
        }
    }

    return true;
}

bool UE4Common::FString::Write(Stream *stream) const
{
    int32_t length = (int32_t)Length();

    if (length != 0)
    {
        length += 1;
    }

    if (is_ucs2)
        length = -length;

    if (!stream->Write32(length))
        return false;

    if (length != 0)
    {
        uint32_t size = Size();

        if (!is_ucs2)
        {
            if (!stream->Write(str.c_str(), size))
                return false;
        }
        else
        {
            if (!stream->Write(ucs2_str.c_str(), size))
                return false;
        }
    }

    return true;
}
