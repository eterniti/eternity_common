#ifndef BSAFILE_H
#define BSAFILE_H

// Class for Xenoverse 2 skills bsa files.
// This is not related at all with Bethesda bsa format!

#include "BaseFile.h"

#define BSA_SIGNATURE   0x41534223

#pragma pack(push,1)

struct BSAHeader
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t header_size; // 6
    uint64_t unk_08;
    uint16_t unk_10;
    uint16_t num_entries; // 0x12
    uint32_t data_start; // 0x14
};

STATIC_ASSERT_STRUCT(BSAHeader, 0x18);

struct BSAEntry
{
    uint32_t unk_00;
    uint16_t num_unk1; // 4
    uint16_t num_unk2; // 6
    uint32_t unk1_offset; // 8
    uint32_t unk2_offset; // 0xC
    uint16_t unk_10;
    uint16_t unk_12;
    uint16_t unk_14;
    uint16_t unk_16;
    uint16_t unk_18;
    uint16_t unk_1A;
    uint16_t unk_1C;
    uint16_t unk_1E;
    uint16_t unk_20;
    uint16_t num_subentries; // 0x22
    uint32_t subentries_offset; // 0x24
    uint32_t unk_28[3];
};

STATIC_ASSERT_STRUCT(BSAEntry, 0x34);

struct BSASubEntry
{
    uint16_t type; // 0
    uint16_t unk_02;
    uint16_t unk_04;
    uint16_t count; // 6
    uint32_t hdr_offset; // 8
    uint32_t data_offset; // 0xC
};

STATIC_ASSERT_STRUCT(BSASubEntry, 0x10);

struct BSAUnk6
{
    uint16_t unk_00;
    uint16_t skill_id; // 2
    uint16_t unk_04;
    uint16_t unk_06;
    uint16_t unk_08;
    uint16_t unk_0A;
    uint32_t unk_0C[3];
};

STATIC_ASSERT_STRUCT(BSAUnk6, 0x18);

#pragma pack(pop)

class BsaFile : public BaseFile
{
private:

    std::vector<uint8_t> data;
    std::vector<BSAUnk6 *> unk6s;

protected:

    void Reset();

public:

    BsaFile();
    virtual ~BsaFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    size_t ChangeReferencesToSkill(uint16_t old_skill, uint16_t new_skill);
};

#endif // BSAFILE_H
