#ifndef __BITSTREAM_H__
#define __BITSTREAM_H__

#include "BaseFile.h"

class BitStream : public BaseFile
{
protected:

    uint8_t *mem;

    uint64_t size_bytes;
    uint64_t size_bits;

    uint64_t pos_bytes;
    uint64_t pos_bits;

    uint64_t capacity_bytes;
    uint64_t capacity_bits;

public:

    BitStream();
    virtual ~BitStream();

    uint8_t *GetMemory(bool unlink);

    inline uint64_t GetSize() const { return size_bits; }
    inline uint64_t GetSizeBytes() const { return size_bytes; }

    virtual bool Resize(uint64_t size);
    inline bool Grow(int64_t size) { return Resize(size_bits+size); }

    virtual bool Read(void *buf, size_t size);
    virtual bool Write(const void *buf, size_t size);

    bool Seek(off64_t offset, int whence);
    inline uint64_t Tell() { return pos_bits; }

    inline uint64_t TellByte() { return pos_bytes; }
    inline uint64_t TellByteCeiling()
    {
        if (pos_bits&7)
            return pos_bytes+1;

        return pos_bytes;
    }

    inline bool ReadU8(uint8_t *ptr, unsigned int bits)
    {
        if (bits > 8)
            return false;

        *ptr = 0;

        bool ret = Read(ptr, bits);
        if (ret)
            *ptr >>= (8-bits);

        return ret;
    }

    inline bool ReadS8(int8_t *ptr, unsigned int bits)
    {
        bool ret = ReadU8((uint8_t *)ptr, bits);
        if (ret)
        {
            size_t highest_bit = bits-1;

            if (*ptr & (1 << highest_bit))
            {
                for (size_t i = highest_bit+1; i < 8; i++)
                    *ptr |= (1 << i);
            }
        }

        return true;
    }

    inline bool ReadU16(uint16_t *ptr, unsigned int bits)
    {
        if (bits > 16)
            return false;

        *ptr = 0;

        bool ret = Read(ptr, bits);
        if (ret)
            *ptr = be16(*ptr) >> (16-bits);

        return ret;
    }

    inline bool ReadS16(int16_t *ptr, unsigned int bits)
    {
        bool ret = ReadU16((uint16_t *)ptr, bits);
        if (ret)
        {
            size_t highest_bit = bits-1;

            if (*ptr & (1 << highest_bit))
            {
                for (size_t i = highest_bit+1; i < 16; i++)
                    *ptr |= (1 << i);
            }
        }

        return true;
    }

    inline bool ReadU32(uint32_t *ptr, unsigned int bits)
    {
        if (bits > 32)
            return false;

        *ptr = 0;

        bool ret = Read(ptr, bits);
        if (ret)
            *ptr = be32(*ptr) >> (32-bits);

        return ret;
    }

    inline bool ReadS32(int32_t *ptr, unsigned int bits)
    {
        bool ret = ReadU32((uint32_t *)ptr, bits);
        if (ret)
        {
            size_t highest_bit = bits-1;

            if (*ptr & (1 << highest_bit))
            {
                for (size_t i = highest_bit+1; i < 32; i++)
                    *ptr |= (1 << i);
            }
        }

        return true;
    }

    inline bool ReadU64(uint64_t *ptr, unsigned int bits)
    {
        if (bits > 64)
            return false;

        *ptr = 0;

        bool ret = Read(ptr, bits);
        if (ret)
            *ptr = be64(*ptr) >> (64-bits);

        return ret;
    }

    inline bool ReadS64(int64_t *ptr, unsigned int bits)
    {
        bool ret = ReadU64((uint64_t *)ptr, bits);
        if (ret)
        {
            size_t highest_bit = bits-1;

#if defined(_MSC_VER) && defined(CPU_X86_64)
            if (*ptr & (1i64 << highest_bit))
#else
            if (*ptr & (1 << highest_bit))
#endif
            {
                for (size_t i = highest_bit+1; i < 64; i++)
                {
#if defined(_MSC_VER) && defined(CPU_X86_64)
                    *ptr |= (1i64 << i);
#else
                    *ptr |= (1 << i);
#endif
                }
            }
        }

        return true;
    }

    inline bool WriteU8(uint8_t value, unsigned int bits)
    {
        if (bits > 8)
            return false;

        value <<= (8-bits);
        return Write(&value, bits);
    }

    inline bool WriteS8(int8_t value, unsigned int bits) { return WriteU8((uint8_t)value, bits); }

    inline bool WriteU16(uint16_t value, unsigned int bits)
    {
        if (bits > 16)
            return false;

        value = be16(value << (16-bits));
        return Write(&value, bits);
    }

    inline bool WriteS16(int16_t value, unsigned int bits) { return WriteU16((uint16_t)value, bits); }

    inline bool WriteU32(uint32_t value, unsigned int bits)
    {
        if (bits > 32)
            return false;

        value = be32(value << (32-bits));
        return Write(&value, bits);
    }

    inline bool WriteS32(int32_t value, unsigned int bits) { return WriteU32((uint32_t)value, bits); }

    inline bool WriteU64(uint64_t value, unsigned int bits)
    {
        if (bits > 64)
            return false;

        value = be64(value << (64-bits));
        return Write(&value, bits);
    }

    inline bool WriteS64(int64_t value, unsigned int bits) { return WriteU64((uint64_t)value, bits); }

    static inline uint8_t MinBitsU8(uint8_t val)
    {
        for (size_t bit = 7; bit > 0; bit--)
        {
            if (val & (1<<bit))
                return (uint8_t)(bit+1);
        }

        return 1;
    }

    static inline uint8_t MinBitsS8(int8_t val)
    {
        if (val > 0)
            return MinBitsU8((uint8_t)val)+1;
        else
            return MinBitsU8((uint8_t)-val);

        // else, is 0
        return 1;
    }

    static inline uint8_t MinBitsU16(uint16_t val)
    {
        for (size_t bit = 15; bit > 0; bit--)
        {
            if (val & (1<<bit))
                return (uint8_t)(bit+1);
        }

        return 1;
    }

    static inline uint8_t MinBitsS16(int16_t val)
    {
        if (val > 0)
            return MinBitsU16((uint16_t)val)+1;
        else
            return MinBitsU16((uint16_t)-val);

        // else, is 0
        return 1;
    }

    static inline uint8_t MinBitsU32(uint32_t val)
    {
        for (size_t bit = 31; bit > 0; bit--)
        {
            if (val & (1<<bit))
                return (uint8_t)(bit+1);
        }

        return 1;
    }

    static inline uint8_t MinBitsS32(int32_t val)
    {
        if (val > 0)
            return MinBitsU32((uint32_t)val)+1;
        else
            return MinBitsU32((uint32_t)-val);

        // else, is 0
        return 1;
    }

    static inline uint8_t MinBitsU64(uint64_t val)
    {
        for (size_t bit = 63; bit > 0; bit--)
        {
#if defined(_MSC_VER) && defined(CPU_X86_64)
            if (val & (1i64<<bit))
#else
            if (val & (1<<bit))
#endif
                return (uint8_t)(bit+1);
        }

        return 1;
    }

    static inline uint8_t MinBitsS64(int64_t val)
    {
        if (val > 0)
            return MinBitsU64((uint64_t)val)+1;
        else
            return MinBitsU64((uint64_t)-val);

        // else, is 0
        return 1;
    }
};

#endif // __BITSTREAM_H__
