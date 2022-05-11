#ifndef __FMPFILE_H__
#define __FMPFILE_H__

#include "BaseFile.h"

#define FMP_SIGNATURE "#FMP"

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    char signature[4]; // 0
    uint8_t unk_04[4]; // 4  00 02 01 00
    uint8_t unk_08[8]; //
    uint32_t unk1_offset; // 0x10
    uint32_t unk2_offset; // 0x14
    uint32_t num_unk1e; // 0x18
    uint32_t unk1e_offset; // 0x1C
    uint32_t num_unk2e; // 0x20
    uint32_t unk2e_offset; // 0x24
    uint32_t todo[0x30/4]; // 0x28
} PACKED FMPHeader;

static_assert(sizeof(FMPHeader) == 0x58, "Incorrect structure size.");

typedef struct
{
    float unk[30];
} PACKED FMPUnk1;

static_assert(sizeof(FMPUnk1) == 0x78, "Incorrect structure size.");

typedef struct
{
    float unk[48];
} PACKED FMPUnk2;

static_assert(sizeof(FMPUnk2) == 0xC0, "Incorrect structure size.");

typedef struct
{
    uint32_t name_offset; // 0 Absolute
    uint32_t num_unsigned; // 4
    uint32_t unsigned_offsets; // 8 absolute points to uint16_t * array
} PACKED FMPUnk1EntryHeader;

static_assert(sizeof(FMPUnk1EntryHeader) == 0xC, "Incorrect structure size.");

typedef struct
{
    uint32_t name_offset; // 0 Absolute
    uint16_t unk_04;  // 4
    uint16_t unk_06; // 6
    uint16_t unk_08; // 8
    uint16_t unk_0A; // 0xA
    uint32_t unk4_offset; // 0xC
    uint32_t unk3_offset; // 0x10
    uint32_t num_unk5; // 0x14
    uint32_t unk5_offset; // 0x18
    uint32_t unk_1C; // 0x1C
    float matrix[16]; // 0x20
} PACKED FMPUnk2EntryHeader;

static_assert(sizeof(FMPUnk2EntryHeader) == 0x60, "Incorrect structure size.");

typedef struct
{
    uint32_t name_offset; // 0 Absolute
    uint32_t unk_04; // 4
    uint32_t num_data; // 8
    uint32_t data_offset; // 0xC Absolute
} PACKED FMPUnk3Header;

static_assert(sizeof(FMPUnk3Header) == 0x10, "Incorrect structure size.");

typedef struct
{
    uint32_t name_offset; // 0 Absolute
    uint32_t unk_04; // 4
    uint32_t num_values; // 0x8
    uint32_t values_offset; // 0xC Absolute
} PACKED FMPUnk3DataHeader;

static_assert(sizeof(FMPUnk3DataHeader) == 0x10, "Incorrect structure size.");

typedef struct
{
    uint32_t name_offset; // 0 absolute
    uint32_t type; // 4  0 -> next points to 32 bits integer (32?) // 6 -> data points to string
    uint32_t data_offset; // 0x8  points to integer or string
} PACKED FMPValue;

static_assert(sizeof(FMPValue) == 0xC, "Incorrect structure size.");

#ifdef _MSC_VER
#pragma pack(pop)
#endif

class FmpFile;
class FmpUnk1;
class FmpUnk2;
class FmpUnk3;

struct FmpValue
{
    uint32_t type; // 0 -> unsigned, 6 -> string

    uint32_t data_unsigned;
    std::string data_string;
};

class FmpUnk1
{
private:
    std::string name;
    std::vector<uint16_t> unsigneds;

    friend class FmpFile;

public:


};

class FmpUnk3Data
{
private:

    std::string name;
    uint32_t unk_04;
    std::vector<FmpValue> values;

    friend class FmpUnk3;
    friend class FmpUnk2;
    friend class FmpFile;

public:
};

class FmpUnk3
{
private:

    std::string name;
    uint32_t unk_04;
    std::vector<FmpUnk3Data> data;

    friend class FmpUnk2;
    friend class FmpFile;

public:
};

class FmpUnk2
{
private:

    std::string name;
    FmpUnk3 *unk3=nullptr;

    uint16_t unk_04;
    uint16_t unk_06;
    uint16_t unk_08;
    uint16_t unk_0A;

    uint32_t unk_1C;

    float matrix[16];

    friend class FmpFile;

public:

    ~FmpUnk2()
    {
        if (unk3)
            delete unk3;
    }
};

class FmpFile : public BaseFile
{
private:

    float unk1[30];
    float unk2[48];

    uint8_t unk_08[8];

    std::vector<FmpUnk1> unk1_entries;
    std::vector<FmpUnk2> unk2_entries;

    void Reset();

public:

    FmpFile();
    virtual ~FmpFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
};

#endif
