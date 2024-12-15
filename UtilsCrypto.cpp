#include "UtilsCrypto.h"

#ifndef NO_CRYPTO
#include "crypto/sha1.h"
#include "crypto/md5.h"
#include "crypto/rijndael.h"
#endif

#include "debug.h"

#define FILE_BUFFER_SIZE	(16*1024*1024)

#ifdef NO_CRYPTO

void Utils::Sha1(const void *, uint32_t, uint8_t *)
{
    DPRINTF("%s: Crypto is not enabled.\n", FUNCNAME);
}

void Utils::Md5(const void *, uint32_t, uint8_t *)
{
    DPRINTF("%s: Crypto is not enabled.\n", FUNCNAME);
}

bool Utils::FileSha1(const std::string &, uint8_t *)
{
    DPRINTF("%s: Crypto is not enabled.\n", FUNCNAME);
    return false;
}

bool Utils::FileMd5(const std::string &, uint8_t *)
{
    DPRINTF("%s: Crypto is not enabled.\n", FUNCNAME);
    return false;
}

void Utils::AesEcbDecrypt(void *, size_t, const uint8_t *, int)
{
    DPRINTF("%s: Crypto is not enabled.\n", FUNCNAME);
}

void Utils::AesEcbEncrypt(void *, size_t, const uint8_t *, int)
{
    DPRINTF("%s: Crypto is not enabled.\n", FUNCNAME);
}

void Utils::AesCtrEncrypt(void *, size_t, const uint8_t *, int, const uint8_t *)
{
    DPRINTF("%s: Crypto is not enabled.\n", FUNCNAME);
}

#else

void Utils::Sha1(const void *buf, uint32_t size, uint8_t *result)
{
    SHA1_CTX ctx;

    __SHA1_Init(&ctx);
    __SHA1_Update(&ctx, (const uint8_t *)buf, size);
    __SHA1_Final(&ctx, result);
}

void Utils::Md5(const void *buf, uint32_t size, uint8_t *result)
{
    MD5_CTX ctx;

    __MD5_Init(&ctx);
    __MD5_Update(&ctx, buf, size);
    __MD5_Final(result, &ctx);
}

bool Utils::FileSha1(const std::string &path, uint8_t *result)
{
    size_t remaining;
    FILE *in;
    SHA1_CTX ctx;

    remaining = GetFileSize(path);
    if (remaining == (size_t)-1)
        return false;

    __SHA1_Init(&ctx);

    if (remaining == 0) // Special case, 0 bytes file
    {
        __SHA1_Final(&ctx, result);
        return true;
    }

    in = fopen(path.c_str(), "rb");
    if (!in)
        return false;

    size_t buf_size = (remaining < FILE_BUFFER_SIZE) ? remaining : FILE_BUFFER_SIZE;
    uint8_t *buf;

    buf = new uint8_t[buf_size];

    while (remaining > 0)
    {
        size_t r = (remaining < buf_size) ? remaining : buf_size;

        if (fread(buf, 1, r, in) != r)
        {
            fclose(in);
            delete[] buf;
            return false;
        }

        __SHA1_Update(&ctx, buf, (uint32_t)r);
        remaining -= r;
    }

    fclose(in);
    __SHA1_Final(&ctx, result);

    return true;
}

bool Utils::FileMd5(const std::string &path, uint8_t *result)
{
    size_t remaining;
    FILE *in;
    MD5_CTX ctx;

    remaining = GetFileSize(path);
    if (remaining == (size_t)-1)
        return false;

    __MD5_Init(&ctx);

    if (remaining == 0) // Special case, 0 bytes file
    {
        __MD5_Final(result, &ctx);
        return true;
    }

    in = fopen(path.c_str(), "rb");
    if (!in)
        return false;

    size_t buf_size = (remaining < FILE_BUFFER_SIZE) ? remaining : FILE_BUFFER_SIZE;
    uint8_t *buf;

    buf = new uint8_t[buf_size];

    while (remaining > 0)
    {
        size_t r = (remaining < buf_size) ? remaining : buf_size;

        if (fread(buf, 1, r, in) != r)
        {
            fclose(in);
            delete[] buf;
            return false;
        }

        __MD5_Update(&ctx, buf, (unsigned long)r);
        remaining -= r;
    }

    fclose(in);
    __MD5_Final(result, &ctx);

    return true;
}

void Utils::AesEcbDecrypt(void *buf, size_t size, const uint8_t *key, int key_size)
{
    if (key_size != 256 && key_size != 128 && key_size != 192)
        return;

    uint32_t rk[RKLENGTH(256)]; // Max size
    static const size_t block_size = 16;

    int nrounds = rijndaelSetupDecrypt(rk, key, key_size);
    size_t num_blocks = size / block_size;
    uint8_t *ptr = (uint8_t *)buf;
    bool last_pass = false;

    if ((size % block_size) != 0)
    {
        num_blocks++;
        last_pass = true;
    }

    for (size_t i = 0; i < num_blocks; i++)
    {
        if ( (i == (num_blocks-1)) && last_pass )
        {
            uint8_t temp[block_size];
            rijndaelDecrypt(rk, nrounds, ptr, temp);

            memcpy(ptr, temp, size % block_size);
        }
        else
        {
            rijndaelDecrypt(rk, nrounds, ptr, ptr);
        }

        ptr += block_size;
    }
}

void Utils::AesEcbEncrypt(void *buf, size_t size, const uint8_t *key, int key_size)
{
    if (key_size != 256 && key_size != 128 && key_size != 192)
        return;

    uint32_t rk[RKLENGTH(256)]; // Max size
    static const size_t block_size = 16;

    int nrounds = rijndaelSetupEncrypt(rk, key, key_size);
    size_t num_blocks = size / block_size;
    uint8_t *ptr = (uint8_t *)buf;

    if ((size % block_size) != 0)
    {
        DPRINTF("%s: Bad function usage. Encryption function requires a size multiple of block_size", FUNCNAME);
        return;
    }

    for (size_t i = 0; i < num_blocks; i++)
    {
        rijndaelEncrypt(rk, nrounds, ptr, ptr);
        ptr += block_size;
    }
}

void Utils::AesCtrEncrypt(void *buf, size_t size, const uint8_t *key, int key_size, const uint8_t *iv)
{
    static const size_t block_size = 16;

    uint8_t ctr[block_size];
    uint8_t *inout = (uint8_t *)buf;
    size_t nblocks = size / block_size;

    memcpy(ctr, iv, block_size);

    for (size_t i = 0; i < nblocks; i++)
    {
        uint8_t temp[block_size];
        size_t xor_size;

        memcpy(temp, ctr, block_size);
        Utils::AesEcbEncrypt(temp, block_size, key, key_size);

        if (i == (nblocks-1) && (size&(block_size-1)))
        {
            xor_size = size&(block_size-1);
        }
        else
        {
            xor_size = block_size;
        }

        for (size_t j = 0; j < xor_size; j++)
        {
            inout[i*block_size+j] ^= temp[j];
        }

        if (i != (nblocks-1))
        {
            for (int k = block_size-1, carry = 1; k >= 0 && carry; k--)
                carry = !++ctr[k];
        }
    }
}

void Utils::AesCbcDecrypt(void *buf, size_t size, const uint8_t *key, int key_size, const uint8_t *iv)
{
    static const size_t block_size = 16;

    uint8_t xblock[block_size];
    uint8_t *inout = (uint8_t *)buf;
    size_t nblocks = size / block_size;

    memcpy(xblock, iv, block_size);

    for (size_t i = 0; i < nblocks; i++)
    {
        uint8_t temp[block_size];
        uint8_t *cb = inout + i*block_size;

        memcpy(temp, cb, block_size);
        Utils::AesEcbDecrypt(cb, block_size, key, key_size);

        for (size_t j = 0; j < block_size; j++)
        {
            cb[j] ^= xblock[j];
        }

        memcpy(xblock, temp, block_size);
    }
}

void Utils::AesCbcEncrypt(void *buf, size_t size, const uint8_t *key, int key_size, const uint8_t *iv)
{
    static const size_t block_size = 16;

    uint8_t xblock[block_size];
    uint8_t *inout = (uint8_t *)buf;
    size_t nblocks = size / block_size;

    memcpy(xblock, iv, block_size);

    for (size_t i = 0; i < nblocks; i++)
    {
        uint8_t *cb = inout + i*block_size;

        for (size_t j = 0; j < block_size; j++)
        {
            cb[j] ^= xblock[j];
        }

        Utils::AesEcbEncrypt(cb, block_size, key, key_size);
        memcpy(xblock, cb, block_size);
    }
}

#endif // NO_CRYPTO
