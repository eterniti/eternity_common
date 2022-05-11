#ifndef CHARALISTFILE_H
#define CHARALISTFILE_H

#include "BaseFile.h"

#define XV2_MAX_SLOTS     850
#define XV2_MAX_SUBSLOTS  31

struct CharaListSlotEntry
{
    bool with_undefined; // Dunno why one of the TRS have that...
    std::string code; // Includes the surroinding quotes (if any)!!!!!
    int costume_index;
    int model_preset;
    int unlock_index;
    bool flag_gk2;
    int voices_id_list[2];
    std::string dlc;

    CharaListSlotEntry()
    {
        with_undefined = flag_gk2 = false;
        costume_index = model_preset = unlock_index = 0;
        voices_id_list[0] = voices_id_list[1] = -1;
        dlc = "Dlc_Def";
    }

    inline bool operator==(const CharaListSlotEntry &rhs) const
    {
        if (rhs.with_undefined != with_undefined)
            return false;

        if (rhs.code != code)
            return false;

        if (rhs.costume_index != costume_index)
            return false;

        if (rhs.model_preset != model_preset)
            return false;

        if (rhs.unlock_index != unlock_index)
            return false;

        if (rhs.flag_gk2 != flag_gk2)
            return false;

        if (memcmp(rhs.voices_id_list, voices_id_list, sizeof(voices_id_list)) != 0)
            return false;

        if (dlc != rhs.dlc)
            return false;

        return true;
    }

    inline bool operator!=(const CharaListSlotEntry &rhs) const
    {
        return !(*this == rhs);
    }
};

struct CharaListSlot
{
    std::vector<CharaListSlotEntry> entries;

    inline bool operator==(const CharaListSlot &rhs) const
    {
        return (rhs.entries == entries);
    }

    inline bool operator!=(const CharaListSlot &rhs) const
    {
        return !(*this == rhs);
    }
};

class CharaListFile : public BaseFile
{
private:

    std::vector<std::string> lines;
    std::vector<CharaListSlot> chara_slots;
    size_t slots_line_start;

    void SimplifyString(std::string &string);
    bool ParseSlotEntry(std::string &string, CharaListSlotEntry &entry);

protected:

    void Reset();

public:

    CharaListFile();
    virtual ~CharaListFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    size_t GetNumSlots() const { return chara_slots.size(); }
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

    bool HasText(const std::string &text);
};

#endif // CHARALISTFILE_H
