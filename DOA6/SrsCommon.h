#ifndef SRSCOMMON_H
#define SRSCOMMON_H

#include "BaseFile.h"

#define KOVS_SIGNATURE 0x53564F4B
#define SCRAMBLED_OGG_SIGNATURE	0x5065664F

struct PACKED KTSRHeader
{
    uint32_t signature;
    uint32_t unk_04; // Unknown hash or whatever. 0x1A487B77 (unused?)
    uint16_t unk_08; // Must be 01
    uint16_t unk_0A; // saw 0x100. (unused ?)
    uint32_t unk_0C; // other hash or whatever. 0x9CC9E1D1 (unused?)
    uint32_t unk_10; // Saw 0 (unused ?)
    uint32_t unk_14; // saw 0 (unused ?)
    uint32_t size; // 0x18 - Size of KTSR, should be equal to SRSA size - 0x10
    uint32_t size2; // 0x1C - Seem to be exactly same as above
    uint8_t unk_20[0x20]; // All zero, seem unused.
};
CHECK_STRUCT_SIZE(KTSRHeader, 0x40);

struct PACKED KOVSEntry
{
    uint32_t signature; // 0
    uint32_t ogg_size; // 4
    uint32_t unk_08;
    uint32_t unk_0C;
    uint8_t unk_10[0x10]; //
};
CHECK_STRUCT_SIZE(KOVSEntry, 0x20);

#endif // SRSCOMMON_H
