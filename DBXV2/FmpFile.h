#ifndef FMPFILE_H
#define FMPFILE_H

#include "BaseFile.h"

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

#define FMP_SIGNATURE   0x53434223

// All offsets of file are absolute

typedef struct
{
    uint32_t signature; // 0
    uint32_t version; // 4
    uint32_t unk_08; // must be an even number
    uint32_t unk_0C; // Zero
    uint32_t unk_10; // Offset (always 0x70)
    uint32_t unk_14; // Offset (always 0xFC)
    uint32_t num_unk1; // 0x18
    uint32_t unk1_offset; // 0x1C
    uint32_t num_unk2; // 0x20
    uint32_t unk2_offset; // 0x24
    uint32_t num_unk3; // 0x28
    uint32_t unk3_offset; // 0x2C
    uint32_t num_unk4; // 0x30
    uint32_t unk4_offset; // 0x34;
    uint32_t num_unk5; // 0x38
    uint32_t unk5_offset; // 0x3C
    uint32_t num_dep1; // 0x40
    uint32_t dep1_table_offset; // 0x44
    uint32_t num_dep2; // 0x48
    uint32_t dep2_table_offset; // 0x4C
    uint32_t num_dep3; // 0x50
    uint32_t dep3_table_offset; // 0x54
    uint32_t num_dep4; // 0x58
    uint32_t dep4_table_offset; // 0x5C
    uint32_t unk_60[4]; // Found to be zero  but game code references first two
} PACKED FMPHeader;

STATIC_ASSERT_STRUCT(FMPHeader, 0x70);

typedef struct
{
    uint32_t name_offset;
    uint32_t unk_04;
    float unk_08[4];
} PACKED FMPUnk1Entry;

STATIC_ASSERT_STRUCT(FMPUnk1Entry, 0x18);


typedef struct
{
    uint32_t name_offset;
    float unk_04[6];
} PACKED FMPUnk2Entry;

STATIC_ASSERT_STRUCT(FMPUnk2Entry, 0x1C);

typedef struct
{
    uint32_t name_offset;
    uint32_t num_indexes;
    uint32_t indexes_offset; // Points to table of 16 bits integers
} PACKED FMPUnk3Entry;

STATIC_ASSERT_STRUCT(FMPUnk3Entry, 0xC);

typedef struct
{
    uint32_t name_offset;
    uint32_t unk_04;
    uint16_t unk_08;
    int16_t unk5_index; // Signed, compared against < 0
    uint32_t unk1_offset; // 0xC   0x50*n, n is get somehow from FMPUnk5Entry
    uint32_t unk2_offset; // 0x10
    uint32_t unk_14;
    uint32_t unk3_offset; // 0x18
    uint32_t unk4_offset; // 0x1C
    uint32_t unk_20;
    float unk_24[12];
} PACKED FMPUnk4Entry;

STATIC_ASSERT_STRUCT(FMPUnk4Entry, 0x54);

typedef struct
{
    uint32_t name_offset; // 0
    uint32_t count; // 4
    uint32_t unk1_offset; // 8
} PACKED FMPUnk5Entry;

STATIC_ASSERT_STRUCT(FMPUnk5Entry, 0xC);

#ifdef _MSC_VER
#pragma pack(pop)
#endif


class FmpFile : public BaseFile
{
public:
    FmpFile();
};

#endif // FMPFILE_H
