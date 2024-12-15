#ifndef UTILSCRYPTO_H
#define UTILSCRYPTO_H

#include "Utils.h"

namespace Utils
{
    std::string Base64Encode(const uint8_t *buf, size_t size, bool add_new_line);
    uint8_t *Base64Decode(const std::string &data, size_t *ret_size);

    void Sha1(const void *buf, uint32_t size, uint8_t *result);
    void Md5(const void *buf, uint32_t size, uint8_t *result);

    bool FileSha1(const std::string &path, uint8_t *result);
    bool FileMd5(const std::string &path, uint8_t *result);

    void AesEcbDecrypt(void *buf, size_t size, const uint8_t *key, int key_size);
    void AesEcbEncrypt(void *buf, size_t size, const uint8_t *key, int key_size);

    void AesCtrEncrypt(void *buf, size_t size, const uint8_t *key, int key_size, const uint8_t *iv);
    static inline void AesCtrDecrypt(void *buf, size_t size, const uint8_t *key, int key_size, const uint8_t *iv)
    {
        return AesCtrEncrypt(buf, size, key, key_size, iv);
    }

    void AesCbcDecrypt(void *buf, size_t size, const uint8_t *key, int key_size, const uint8_t *iv);
    void AesCbcEncrypt(void *buf, size_t size, const uint8_t *key, int key_size, const uint8_t *iv);

    inline void XorBuf(void *out, const void *in, size_t size)
    {
        uint8_t *out8 = (uint8_t *)out;
        const uint8_t *in8 = (const uint8_t *)in;

        for (size_t i = 0; i < size; i++)
        {
            out8[i] ^= in8[i];
        }
    }
}

#endif // UTILSCRYPTO_H
