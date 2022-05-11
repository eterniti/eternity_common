#include "BitStream.h"
#include "debug.h"

// TODO: this class needs optimization

// Grow size in  bits
#define GROW_SIZE	(1024)

BitStream::BitStream()
{
    mem = nullptr;
    size_bytes = size_bits = 0;
    pos_bytes = pos_bits = 0;
    capacity_bytes = capacity_bits = 0;
}

BitStream::~BitStream()
{
    if (mem)
        delete[] mem;
}

uint8_t *BitStream::GetMemory(bool unlink)
{
    uint8_t *ret = mem;

    if (mem && unlink)
    {
        mem = nullptr;
        Resize(0);
    }

    return ret;
}

bool BitStream::Resize(uint64_t size)
{
    assert(size_bytes <= capacity_bytes);

    if (size == 0)
    {
        if (mem)
        {
            delete[] mem;
            mem = nullptr;
        }

        size_bytes = size_bits = 0;
        pos_bytes = pos_bits = 0;
        capacity_bytes = capacity_bits = 0;
        return true;
    }

    if (!mem || size_bits > capacity_bits)
    {
        if (!mem)
        {
            assert(size_bytes == 0 && pos_bytes == 0 && capacity_bytes == 0);
            assert(size_bits == 0 && pos_bits == 0 && capacity_bits == 0);
        }

        size_t alloc_size = (size >= (capacity_bits+GROW_SIZE)) ? size : capacity_bits+GROW_SIZE;
        uint8_t *new_mem;

        capacity_bits = Utils::Align2(alloc_size, 8);
        capacity_bytes = alloc_size/8;

        new_mem = new uint8_t[capacity_bytes];
        memset(new_mem, 0, capacity_bytes);

        if (mem)
        {
            memcpy(new_mem, mem, size_bytes);
            delete[] mem;
        }

        size_bits = size;
        size_bytes = size/8;

        if (size_bits&7)
            size_bytes++;

        mem = new_mem;
        return true;
    }

    // size <= capacity
    size_bits = size;
    size_bytes = size/8;

    if (size_bits&7)
        size_bytes++;

    if (pos_bits > size_bits)
    {
        pos_bits = size_bits;
        pos_bytes = pos_bits/8;
    }

    return true;
}

bool BitStream::Read(void *buf, size_t size)
{
    if (!mem)
        return false;

    if (pos_bits+size > size_bits)
        return false;

    uint8_t *out = (uint8_t *)buf;
    uint8_t *in = mem+pos_bytes;

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
            in++;
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

bool BitStream::Write(const void *buf, size_t size)
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
    uint8_t *out = mem+pos_bytes;

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
            out++;
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

bool BitStream::Seek(off64_t offset, int whence)
{
    size_t new_pos;

    if (whence == SEEK_SET)
    {
        new_pos = offset;
    }
    else if (whence == SEEK_CUR)
    {
        new_pos = pos_bits + offset;
    }
    else if (whence == SEEK_END)
    {
        new_pos = size_bits + offset;
    }
    else
    {
        return false;
    }

    if (new_pos > size_bits)
        return false;

    pos_bits = new_pos;
    pos_bytes = pos_bits/8;

    return true;
}
