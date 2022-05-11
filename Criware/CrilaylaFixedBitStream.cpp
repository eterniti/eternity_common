#include "CrilaylaFixedBitStream.h"

bool CrilaylaFixedBitStream::Read(void *buf, size_t size)
{
    if (!mem)
        return false;

    if (pos_bits+size > size_bits)
        return false;

    uint8_t *out = (uint8_t *)buf;
    uint8_t *in = mem+size_bytes-pos_bytes-1;

    size_t bit_out = 7;
    size_t bit_in = 7-(pos_bits&7);

    for (size_t i = 0; i < size; i++)
    {
        if (*in & (1<<bit_in))
            *out |= (1<<bit_out);
        else
            *out &= ~(1<<bit_out);

        if (bit_in == 0)
        {
            bit_in = 7;
            in--;
            pos_bytes++;
        }
        else
        {
            bit_in--;
        }

        if (bit_out == 0)
        {
            bit_out = 7;
            out++;
        }
        else
        {
            bit_out--;
        }
    }

    pos_bits += size;
    return true;
}

bool CrilaylaFixedBitStream::Write(const void *buf, size_t size)
{
    if (!mem)
    {
        if (!Grow(size))
            return false;
    }

    else if (pos_bits+size > size_bits)
    {
        if (!Grow(pos_bits+size-size_bits))
            return false;
    }

    const uint8_t *in = (const uint8_t *)buf;
    uint8_t *out = mem+size_bytes-pos_bytes-1;;

    size_t bit_in = 7;
    size_t bit_out = 7-(pos_bits&7);

    for (size_t i = 0; i < size; i++)
    {
        if (*in & (1<<bit_in))
            *out |= (1<<bit_out);
        else
            *out &= ~(1<<bit_out);

        if (bit_in == 0)
        {
            bit_in = 7;
            in++;
        }
        else
        {
            bit_in--;
        }

        if (bit_out == 0)
        {
            bit_out = 7;
            out--;
            pos_bytes++;
        }
        else
        {
            bit_out--;
        }
    }

    pos_bits += size;
    return true;
}
