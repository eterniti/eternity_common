#ifndef BCMFILE_H
#define BCMFILE_H

#include "BaseFile.h"

#define BCM_SIGNATURE   0x4D434223

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED BCMHeader
{
    uint32_t signature; // 0
    uint16_t endianess_check;  // 4
    uint16_t unk_06;
    uint32_t num_entries; // 8
    uint32_t data_start; // 0xC
};
CHECK_STRUCT_SIZE(BCMHeader, 0x10);

struct PACKED BCMEntry
{
    uint32_t unk_00;
    uint32_t directional_conditions;
    uint16_t unk_08;
    uint16_t unk_0A;
    uint32_t hold_down_conditions;
    uint32_t opponent_size_conditions; // 0x10
    uint32_t unk_14;
    uint32_t primary_activator_conditions; // 0x18
    uint32_t input_activator_conditions; // 0x1C
    uint16_t bac_entry_to_activate; // 0x20
    uint16_t bac_entry_final_part; // 0x22
    uint16_t unk_24;
    uint16_t bac_entry_user_connect; // 0x26
    uint16_t bac_entry_victim_connect;
    uint16_t bac_entry_unknown; // 0x2A
    uint16_t unk_2C;
    uint16_t unk_2E;
    uint32_t sibling;
    uint32_t child;
    uint32_t parent; // 0x38
    uint32_t root;
    uint32_t ki_cost;
    uint32_t unk_44;
    uint32_t unk_48;
    uint32_t receiver_link_id; // 0x4C
    uint32_t unk_50;
    uint32_t stamina_cost;
    uint32_t unk_58;
    uint32_t ki_required; // 0x5C
    float health_required; // 0x60
    uint16_t trans_modifier; // 0x64
    uint16_t cus_aura; // 0x66
    uint32_t unk_68;
    uint32_t character_conditions; // 0x6C
};
CHECK_STRUCT_SIZE(BCMEntry, 0x70);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct BcmEntry
{
    uint32_t unk_00;
    uint32_t directional_conditions;
    uint16_t unk_08;
    uint16_t unk_0A;
    uint32_t hold_down_conditions;
    uint32_t opponent_size_conditions;
    uint32_t unk_14;
    uint32_t primary_activator_conditions;
    uint32_t input_activator_conditions;
    uint16_t bac_entry_to_activate;
    uint16_t bac_entry_final_part;
    uint16_t unk_24;
    uint16_t bac_entry_user_connect;
    uint16_t bac_entry_victim_connect;
    uint16_t bac_entry_unknown;
    uint16_t unk_2C;
    uint16_t unk_2E;
    uint32_t sibling;
    uint32_t child;
    uint32_t parent;
    uint32_t root;
    uint32_t ki_cost;
    uint32_t unk_44;
    uint32_t unk_48;
    uint32_t receiver_link_id;
    uint32_t unk_50;
    uint32_t stamina_cost;
    uint32_t unk_58;
    uint32_t ki_required;
    float health_required;
    uint16_t trans_modifier;
    uint16_t cus_aura;
    uint32_t unk_68;
    // None of the game files are currently using this condition, but the code in the exe is pretty clear.
    // 0=none, 1=is_cac, 2=is_hum, 3=is_huf, 4=is_sym, 5=is_syf 6=is_nmc, 7=is_fri, 8=is_mam, 9=is_maf
    uint32_t character_conditions;

    TiXmlElement *Decompile(TiXmlNode *root, uint32_t idx) const;
    bool Compile(const TiXmlElement *root);
};

class BcmFile : public BaseFile
{
private:

    std::vector<BcmEntry> entries;

protected:

    void Reset();

public:

    BcmFile();
    virtual ~BcmFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    inline size_t GetNumEntries() const { return entries.size(); }

    inline const std::vector<BcmEntry> &GetEntries() const { return entries; }
    inline std::vector<BcmEntry> &GetEntries() { return entries; }

    inline const BcmEntry &operator[](size_t n) const { return entries[n]; }
    inline BcmEntry &operator[](size_t n) { return entries[n]; }

    inline std::vector<BcmEntry>::const_iterator begin() const { return entries.begin(); }
    inline std::vector<BcmEntry>::const_iterator end() const { return entries.end(); }

    inline std::vector<BcmEntry>::iterator begin() { return entries.begin(); }
    inline std::vector<BcmEntry>::iterator end() { return entries.end(); }
};

#endif // BCMFILE_H
