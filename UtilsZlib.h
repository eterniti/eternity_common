#ifndef UTILSZLIB_H
#define UTILSZLIB_H

#include "Utils.h"

namespace Utils
{
    bool CompressZlib(void *comp_buf, long unsigned int *comp_size, const void *uncomp_buf, size_t uncomp_size, int level=Z_DEFAULT_COMPRESSION);
    uint8_t *CompressZlib(const void *uncomp_buf, size_t uncomp_size, size_t *ret_size, int level=Z_DEFAULT_COMPRESSION);
    bool UncompressZlib(void* uncomp_buf, uint32_t *uncomp_size, const void* comp_buf, uint32_t comp_size, int window=15);
}

#endif // UTILSZLIB_H
