#ifndef CRILAYLAFIXEDBITSTREAM_H
#define CRILAYLAFIXEDBITSTREAM_H

#include "FixedBitStream.h"

class CrilaylaFixedBitStream : public FixedBitStream
{
public:

    CrilaylaFixedBitStream(uint8_t *buf, size_t size) : FixedBitStream(buf, size) { }

    virtual bool Read(void *buf, size_t size);
    virtual bool Write(const void *buf, size_t size);

    // No limit checks on these ones!
    inline uint8_t FastReadU8(size_t size)
    {
        uint8_t byte_in, byte_out = 0;
        size_t bits_left = 8-(pos_bits&7);

        uint8_t *in = mem+size_bytes-pos_bytes-1;
        size_t current = 0;

        byte_in = *in;

        while (current < size)
        {
            size_t r;

            if (bits_left == 0)
            {
                in--;
                byte_in = *in;
                bits_left = 8;
                pos_bytes++;
            }

            if (bits_left > (size - current))
                r = size - current;
            else
                r = bits_left;

            byte_out <<= r;
            byte_out |= (uint8_t)((uint8_t)(byte_in >> (bits_left - r)) & ((1 << r) - 1));

            bits_left -= r;
            current += r;
        }

        if (bits_left == 0)
            pos_bytes++;

        pos_bits += size;
        return byte_out;
    }

    inline uint16_t FastReadU16(size_t size)
    {
        uint8_t byte_in;
        uint16_t word_out = 0;
        size_t bits_left = 8-(pos_bits&7);

        uint8_t *in = mem+size_bytes-pos_bytes-1;
        size_t current = 0;

        byte_in = *in;

        while (current < size)
        {
            size_t r;

            if (bits_left == 0)
            {
                in--;
                byte_in = *in;
                bits_left = 8;
                pos_bytes++;
            }

            if (bits_left > (size - current))
                r = size - current;
            else
                r = bits_left;

            word_out <<= r;
            word_out |= (uint16_t)((uint16_t)(byte_in >> (bits_left - r)) & ((1 << r) - 1));

            bits_left -= r;
            current += r;
        }

        if (bits_left == 0)
            pos_bytes++;

        pos_bits += size;
        return word_out;
    }

};

#endif // CRILAYLAFIXEDBITSTREAM_H
