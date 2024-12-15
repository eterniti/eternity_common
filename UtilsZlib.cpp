#include "UtilsZlib.h"

#ifndef NO_ZLIB
#include <zlib.h>
#endif

#include "debug.h"

#ifdef NO_ZLIB

bool Utils::UncompressZlib(void *, uint32_t *, const void *, uint32_t, int)
{
    DPRINTF("%s: zlib is not enabled.\n", FUNCNAME);
    return false;
}

#else

static void *zalloc(void *opaque, unsigned int size, unsigned int num)
{
    UNUSED(opaque);
    return malloc(size * num);
}

static void zfree(void *opaque, void *p)
{
    UNUSED(opaque);
    free(p);
}

bool Utils::CompressZlib(void *comp_buf, long unsigned int *comp_size, const void *uncomp_buf, size_t uncomp_size, int level)
{
    return (compress2((Bytef *)comp_buf, comp_size, (const Bytef *)uncomp_buf, (uLong)uncomp_size, level) == Z_OK);
}

uint8_t *Utils::CompressZlib(const void *uncomp_buf, size_t uncomp_size, size_t *ret_size, int level)
{
    long unsigned int comp_size = (long unsigned int)(uncomp_size + (uncomp_size / 10) + 12);
    uint8_t *buf = new uint8_t[comp_size];

    if (!CompressZlib(buf, &comp_size, uncomp_buf, uncomp_size, level))
    {
        delete[] buf;
        return nullptr;
    }

    *ret_size = (size_t)comp_size;
    return buf;
}

bool Utils::UncompressZlib(void *uncomp_buf, uint32_t *uncomp_size, const void *comp_buf, uint32_t comp_size, int window)
{
    z_stream stream;

    stream.zalloc = &zalloc;
    stream.zfree = &zfree;
    stream.opaque = Z_NULL;
    stream.next_in = (uint8_t *)comp_buf;
    stream.avail_in = comp_size;
    stream.next_out = (uint8_t *)uncomp_buf;
    stream.avail_out = *uncomp_size;

    if (inflateInit2(&stream, window) != Z_OK)
        return false;

    int ret = inflate(&stream, Z_FINISH);
    if (true/*ret == Z_STREAM_END*/)
    {
        *uncomp_size = stream.total_out;
    }

    ret = inflateEnd(&stream);
    return (ret == Z_OK);
}

#endif
