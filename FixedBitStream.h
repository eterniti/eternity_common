#ifndef __FIXEDBITSTREAM_H__
#define __FIXEDBITSTREAM_H__

#include "BitStream.h"

class FixedBitStream : public BitStream
{
public:

    FixedBitStream(uint8_t *buf, size_t size);
    virtual ~FixedBitStream();

    virtual bool Resize(uint64_t) override { return false; }
};

#endif // FIXEDBITSTREAM_H
