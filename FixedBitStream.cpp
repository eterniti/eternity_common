#include "FixedBitStream.h"

FixedBitStream::FixedBitStream(uint8_t *buf, size_t size)
{
    mem = buf;

    size_bits = size;
    capacity_bits = Utils::Align2(size_bits, 8);
    pos_bits = 0;

    size_bytes = size/8;
    if (size_bits&7)
        size_bytes++;

    capacity_bytes = capacity_bits/8;
    pos_bytes = 0;
}

FixedBitStream::~FixedBitStream()
{
    mem = nullptr;
}
