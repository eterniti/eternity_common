#ifndef XV2PATCHERSLOTSFILE_H
#define XV2PATCHERSLOTSFILE_H

#include "CharaListFile.h"

#define CST_SIGNATURE   0x54534323

struct PACKED CSTHeader
{
    uint32_t signature; // 0
    uint32_t header_size; // 4
    uint32_t num_slots; // 8
    uint32_t num_costumes; // 0xC
};
CHECK_STRUCT_SIZE(CSTHeader, 0x10);

struct PACKED CSTEntry
{
    uint32_t costume_id; // 0
    char code[4]; // 4
    uint16_t costume_index; // 8
    uint16_t model_preset; // A
    uint16_t unlock_index; // C
    uint16_t flag_gk2; // E
    int16_t voices_id_list[2]; // 0x10
    uint32_t dlc; // 0x14
    uint32_t dlc_key2; // 0x18 - Added in game v 1.16. Theory: this will be used in DLC 14, after dlc cannot gold more bits.
    uint32_t is_custom_costume; // 0x1C
    uint32_t cac_index; // 0x20 - Added in game v 1.10
    uint32_t var_type_after_TU9_order; // 0x24 - Added in game 1.14. Whatever this shit is, is -1 in all chars, except TU0 ant TU1 where it is 0 and 1 respectively.
};
CHECK_STRUCT_SIZE(CSTEntry, 0x28);

class Xv2PatcherSlotsFile : public BaseFile
{
private:

    std::vector<CharaListSlot> chara_slots;

public:

    Xv2PatcherSlotsFile();
    Xv2PatcherSlotsFile(const CharaListFile &file);
    virtual ~Xv2PatcherSlotsFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    bool LoadFromCst(const uint8_t *buf, size_t size, const uint8_t *buf_raid, size_t size_raid);

    inline size_t GetNumSlots() const { return chara_slots.size(); }
    CharaListSlotEntry *FindFromAbsolutePos(size_t pos);

    const std::vector<CharaListSlot> &GetSlots() const { return chara_slots; }
    std::vector<CharaListSlot> &GetSlots() { return chara_slots; }

    size_t FindSlotsByCode(const std::string &code, std::vector<CharaListSlotEntry *> &entries);
    size_t RemoveSlots(const std::string &code);

    inline const CharaListSlot &operator[](size_t n) const { return chara_slots[n]; }
    inline CharaListSlot &operator[](size_t n) { return chara_slots[n]; }

    inline std::vector<CharaListSlot>::const_iterator begin() const { return chara_slots.begin(); }
    inline std::vector<CharaListSlot>::const_iterator end() const { return chara_slots.end(); }

    inline std::vector<CharaListSlot>::iterator begin() { return chara_slots.begin(); }
    inline std::vector<CharaListSlot>::iterator end() { return chara_slots.end(); }
};

#endif // XV2PATCHERSLOTSFILE_H
