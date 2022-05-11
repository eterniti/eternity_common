#ifndef ERSFILE_H
#define ERSFILE_H

#include "BaseFile.h"

// "#ERS
#define ERS_SIGNATURE  0x53524523

enum ErsEffectIndex
{
    ERS_EFFECT_CMN = 0,
    ERS_EFFECT_STAGE_BG = 1,
    ERS_EFFECT_CHAR = 2,
    ERS_EFFECT_DEMO = 4,
    ERS_EFFECT_UNK9 = 9,
    ERS_EFFECT_SKILL = 10,
    ERS_EFFECT_STAGE = 11,
};

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED ERSHeader
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t header_size; // 6
    uint32_t unk_08; // Zero
    uint32_t num_entries; // 0xC
    uint32_t unk_10; // Zero
    uint32_t table_start; // 0x14
};
CHECK_STRUCT_SIZE(ERSHeader, 0x18);

struct PACKED ERSEntry
{
    uint32_t type; // matches index
    uint32_t unk_04; // Zero
    uint16_t unk_08; // Zero
    uint16_t num_effects; // 0xA
    uint32_t effect_table_offset; // 0xC - Offset relative to this entry start
};
CHECK_STRUCT_SIZE(ERSEntry, 0x10);

struct PACKED ERSEffect
{
    uint32_t index;
    char name[8];
    uint32_t eepk_path_offset; // Relative to this effect start
};
CHECK_STRUCT_SIZE(ERSEffect, 0x10);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct ErsEffect
{
    std::string name;
    std::string eepk_path;

    inline bool IsEmpty() const
    {
        return (name.length() == 0 && eepk_path.length() == 0);
    }
};

struct ErsEntry
{
    std::vector<ErsEffect> effects;

    inline bool IsEmpty() const
    {
        return (effects.size() == 0);
    }
};

class ErsFile : public BaseFile
{
private:

    std::vector<ErsEntry> entries;

    size_t GetNumNonEmptyEntries() const;

protected:

    void Reset();
    size_t CalculateFileSize(size_t *effect_start, size_t *strings_size) const;

public:

    ErsFile();
    virtual ~ErsFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    std::string GetStageBgEepk(size_t stage_id) const;
    std::string GetStageEepk(size_t stage_id) const;
    std::string GetCharEepk(size_t cms_id) const;

    bool SetStageBgEepk(const std::string &stage, size_t stage_id, const std::string &eepk_path);
    bool SetStageEepk(const std::string &stage, size_t stage_id, const std::string &eepk_path);
    bool SetCharEepk(size_t cms_id, const std::string &eepk_path);

    void RemoveStageBgEepk(size_t stage_id);
    void RemoveStageEepk(size_t stage_id);

    void RemoveCharEepk(size_t cms_id);
};

#endif // ERSFILE_H
