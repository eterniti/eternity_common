#ifndef XV2PATCHERSLOTSFILE_H
#define XV2PATCHERSLOTSFILE_H

#include "CharaListFile.h"

#define CST_SIGNATURE   0x54534323

;

//#ifdef _MSC_VER
#pragma pack(push,1)
//#endif

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
    uint64_t dlc_key; // 0x14  - In game ver 1.16, the field became 64 bits, in 1.18 the higher part started being used after the overflow.
    uint32_t is_custom_costume; // 0x1C
    uint32_t cac_index; // 0x20 - Added in game v 1.10
    uint32_t var_type_after_TU9_order; // 0x24 - Added in game 1.14. It is -1 until character TE0. Then, beginning from TE0, it's an index that increases in one? (but there is jump from 3->5...)
    uint32_t unk_28; // Added in 1.21. Doesnt't seem it is used by charasele.iggy
    uint32_t unk_2C; // Added in 1.21. Only 0 & 1 values found, probably boolean. Doesnt't seem it is used by charasele.iggy
    uint32_t flag_cgk2; // Added in 1.22, "Chou gokuaku"
    uint32_t unk_34; // Added in 1.23. Not used by CSS=
};
CHECK_STRUCT_SIZE(CSTEntry, 0x38);

//#ifdef _MSC_VER
#pragma pack(pop)
//#endif


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

    bool FindFirstMatch(const std::string &code, size_t *pentry_idx) const;
    bool FindFirstMatch(const std::string &code, int costume_index, int model_preset, size_t *pentry_idx, size_t *subentry_idx) const;

    bool PlaceAtPos(size_t idx, const CharaListSlot &slot);

    size_t ChangeVoiceIds(const std::string &code, int costume_index, int model_preset, int voice1, int voice2);

    inline const CharaListSlot &operator[](size_t n) const { return chara_slots[n]; }
    inline CharaListSlot &operator[](size_t n) { return chara_slots[n]; }

    inline std::vector<CharaListSlot>::const_iterator begin() const { return chara_slots.begin(); }
    inline std::vector<CharaListSlot>::const_iterator end() const { return chara_slots.end(); }

    inline std::vector<CharaListSlot>::iterator begin() { return chara_slots.begin(); }
    inline std::vector<CharaListSlot>::iterator end() { return chara_slots.end(); }
};

#endif // XV2PATCHERSLOTSFILE_H
