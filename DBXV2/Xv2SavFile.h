#ifndef XV2SAVFILE_H
#define XV2SAVFILE_H

#include "BaseFile.h"

#define XV2_NUM_CAC 8
#define XV2_NUM_PRESETS 8

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

// XV1 preset hero (not imported from xenoverse 1): 46

// B430: first super skill item
// D430: first ultimate skill item

enum XV2CacRace
{
    CAC_HUM,
    CAC_HUF,
    CAC_SYM,
    CAC_SYF,
    CAC_NMC,
    CAC_FRI,
    CAC_MAM,
    CAC_MAF,
    CAC_NUM_RACES
};

typedef struct
{
    uint32_t top; // 0
    uint32_t bottom; // 4
    uint32_t gloves; // 8
    uint32_t shoes; // 0xC
    uint32_t accesory; // 0x10
    uint32_t talisman; // 0x14
    uint32_t qq; // 0x18
    uint16_t top_color1; // 0x1C
    uint16_t top_color2; // 0x1E
    uint16_t top_color3; // 0x20
    uint16_t top_color4; // 0x22
    uint16_t bottom_color1; // 0x24
    uint16_t bottom_color2; // 0x26
    uint16_t bottom_color3; // 0x28
    uint16_t bottom_color4; // 0x2A
    uint16_t gloves_color1; // 0x2C
    uint16_t gloves_color2; // 0x2E
    uint16_t gloves_color3; // 0x30
    uint16_t gloves_color4; // 0x32
    uint16_t shoes_color1; // 0x34
    uint16_t shoes_color2; // 0x36
    uint16_t shoes_color3; // 0x38
    uint16_t shoes_color4; // 0x3A
    // (Note: skill matches the id1 in cus)
    uint32_t super_skills[4]; // 0x3C
    uint32_t ult_skills[2]; // 0x4C
    uint32_t evasive_skill; // 0x54
    uint32_t blast_skill; // 0x58 set to 0xFFFFFFFF (cac uses blast from talisman)
    uint32_t awaken_skill; // 0x5C
} XV2CacSet;

STATIC_ASSERT_STRUCT(XV2CacSet, 0x60);

typedef struct
{
    uint8_t unk_00[0x14]; // 0 -> starts at 0 in a new chara
    uint32_t race; // 0x14
    uint32_t voice; // 0x18
    uint32_t body_shape; // 0x1C
    uint32_t unk_20;
    uint16_t skin_color1; // 0x24
    uint16_t skin_color2; // 0x26
    uint16_t skin_color3; // 0x28
    uint16_t skin_color4; // 0x2A
    uint16_t hair_color; // 0x2C
    uint16_t eye_color; // 0x2E
    uint16_t makeup_color1; // 0x30
    uint16_t makeup_color2; // 0x32
    uint16_t makeup_color3; // 0x34
    uint8_t unk_36[0xE];
    char name[64]; // 0x44 Ascii... or utf8, we don't know for sure
    uint32_t face_base; // 0x84
    uint32_t face_forehead; // 0x88
    uint32_t eyes; // 0x8C (
    uint32_t nose; // 0x90
    uint32_t ears; // 0x94
    uint32_t hair; // 0x98
    uint32_t unk_9C[4]; // these maybe regen rates or something like that
    uint32_t level; // 0xAC
    uint32_t experience; // 0xB0
    uint32_t attribute_points; // 0xB4
    uint32_t hea; // 0xB8
    uint32_t ki; // 0xBC
    uint32_t atk; // 0xC0
    uint32_t str; // 0xC4
    uint32_t bla; // 0xC8
    uint32_t stm; // 0xCC
    XV2CacSet sets[XV2_NUM_PRESETS]; // 0xD0   0 is current clothing, 1-7 are presets
} PACKED XV2Cac;

// Size of current struct, this is much higher actually
STATIC_ASSERT_STRUCT(XV2Cac, 0x3D0);

#ifdef _MSC_VER
#pragma pack(pop)
#endif


class Xv2SavFile : public BaseFile
{
private:

    std::vector<uint8_t> data;
    bool is_encrypted;
    int version;

    uint32_t GetEncryptedSize() const;
    uint32_t GetDecryptedSize() const;

    static int EncryptedSizeToVersion(size_t size);
    static int DecryptedSizeToVersion(size_t size);

public:

    Xv2SavFile();
    virtual ~Xv2SavFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    inline bool IsEncrypted() const { return is_encrypted; }
    inline void SetEncrypted(bool encrypted) { this->is_encrypted = encrypted; }

    uint64_t GetSteamID64() const;
    void SetSteamID64(uint64_t id);

    XV2Cac *GetCac(uint32_t idx);
    bool IsValidCac(uint32_t idx) const;
};

#endif // XV2SAVFILE_H
