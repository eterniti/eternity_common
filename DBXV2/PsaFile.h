#ifndef PSAFILE_H
#define PSAFILE_H

#include "BaseFile.h"

#define PSA_SIGNATURE   0x41535023

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t header_size; // 6
    uint32_t num_attributes; // 8
    uint32_t unk_0C; // Always zero
} PACKED PSAHeader;

STATIC_ASSERT_STRUCT(PSAHeader, 0x10);

typedef struct
{
    float hea;
    float ki;
    float sta;
    float atk;
    float str;
    float bla;
} PACKED PSAAttributes;

STATIC_ASSERT_STRUCT(PSAAttributes, 0x18);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct PsaAttributes
{
    float hea;
    float ki;
    float sta;
    float atk;
    float str;
    float bla;
};

class PsaFile : public BaseFile
{
private:

    std::vector<PsaAttributes> attributes;

protected:

    void Reset();
public:

    PsaFile();
    virtual ~PsaFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;

    float GetHea(uint16_t points, bool *error=nullptr) const;
    float GetKi(uint16_t points, bool *error=nullptr) const;
    float GetSta(uint16_t points, bool *error=nullptr) const;
    float GetAtk(uint16_t points, bool *error=nullptr) const;
    float GetStr(uint16_t points, bool *error=nullptr) const;
    float GetBla(uint16_t points, bool *error=nullptr) const;

    uint16_t GetHeaPoints(float hea) const;
    uint16_t GetKiPoints(float ki) const;
    uint16_t GetStaPoints(float sta) const;
    uint16_t GetAtkPoints(float atk) const;
    uint16_t GetStrPoints(float str) const;
    uint16_t GetBlaPoints(float bla) const;
};

#endif // PSAFILE_H
