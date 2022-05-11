#ifndef QBTFILE_H
#define QBTFILE_H

#include "BaseFile.h"

// "#QBT"
#define QBT_SIGNATURE  0x54425123

enum QbtInteractionType
{
    QBT_ITYPE_DEFAULT = 0,
    QBT_ITYPE_EFFECT = 1,
    QBT_ITYPE_GIVE_ITEM = 2,
    QBT_ITYPE_JOIN = 3
    // There is also a 5, whose difference with 0 is unknown
};

enum QbtEvent
{
    QBT_EVENT_HYPNO_ALLY = 3,
    QBT_EVENT_HYPNO_ATTACK = 4,
    QBT_EVENT_GIANT_KI_BLAST = 5,
    QBT_EVENT_AREA_CHANGE = 6,
    QBT_EVENT_GIANT_KI_BLAST_RETURNED = 7,
    QBT_EVENT_TELEPORT_ATTACK_FAILED = 8,
    QBT_EVENT_CONTROLLED_ALLY_DEFEATED = 9,
    QBT_EVENT_TELEPORT_END = 11,
    QBT_EVENT_CRYSTALS_DESTROYED = 12,
    QBT_EVENT_TELEPORT_ATTACK_SUCCEDED = 14
};

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED QBTHeader
{
    uint32_t signature;
    uint16_t endianess_check;
    uint16_t header_size;
    uint32_t unk_08; // Aways 0
    uint16_t num_entries; // 0xC
    uint16_t num_parts; // 0xE
    uint16_t num_normal_dialogue; // 0x10
    uint16_t num_interactive_dialogue; // 0x12
    uint16_t num_special_dialogue; // 0x14
    uint16_t pad; // 0x16
    uint32_t normal_dialogue_start; // 0x18
    uint32_t interactive_dialogue_start; // 0x1C
    uint32_t special_dialogue_start; // 0x20
};
CHECK_STRUCT_SIZE(QBTHeader, 0x24);

struct PACKED QBTEntry
{
    uint16_t unk_00;
    uint16_t unk_02;
    uint32_t start_part_idx; // 4
    uint32_t num_parts; // 8
    uint32_t interaction_type; // C
    uint32_t interaction_param; // 10
    uint32_t special_event; // 14
    uint32_t special_on_event_end; // 18
    uint32_t unk_1C; // Non zero values: found small numbers like 1 or 3 (maybe others)
    uint16_t cms_20;
    uint16_t unk_22; // Non zero values: found 0xffff in HLQ quests
    uint32_t unk_24; // // Non zero values: found in HLQ/RBQ quests, always in special dialogues, small numbers
    uint32_t unk_28; // Always zero
    uint32_t unk_2C; // Non zero values: misc values (mostly good looking decimal values) found in HLQ
    uint32_t unk_30; // Non zero values: found in HLQ/RBQ quests, always in special dialogues, only value of 1 found.
    uint32_t unk_34[3]; // Always zero
};
CHECK_STRUCT_SIZE(QBTEntry, 0x40);

struct PACKED QBTDialoguePart
{
    uint16_t qbt_id; // 0  It is same for all parts of an entry... except in OSQ_0301.qbt
    uint16_t sub_id; // 2
    uint32_t cms_04;
    uint16_t unk_08;
    uint16_t portrait_cms; // 0xA
    uint16_t portrait_costume_index; // 0xC
    uint16_t portrait_transformation; // 0xE
    uint16_t unk_10; // Always 0
    uint16_t total_part_index; // 0x12
};
CHECK_STRUCT_SIZE(QBTDialoguePart, 0x14);

struct PACKED QBTMsgId
{
    char id[32];
};
CHECK_STRUCT_SIZE(QBTMsgId, 0x20);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct QbtDialoguePart
{
    uint16_t qbt_id; // It is same for all parts of an entry... except in OSQ_0301.qbt
    uint16_t sub_id;
    uint32_t cms_04; // Normal dialogue: either 0 or mostly matches portrait_cms (may be unused by exe)
    uint16_t unk_08; // Normal dialogue: 0, 1, 5. Interactive dialogue: 0 or 1.

    uint16_t portrait_cms;
    uint16_t portrait_costume_index;
    uint16_t portrait_transformation;

    std::string msg_id;

    QbtDialoguePart()
    {
        qbt_id = sub_id = unk_08 = 0;
        cms_04 = 0;
        portrait_cms = 0;
        portrait_costume_index = portrait_transformation = 0xFFFF;
    }

    inline bool PartialCompare(const QbtDialoguePart &rhs) const
    {
        if (sub_id != rhs.sub_id)
            return false;

        if (cms_04 != rhs.cms_04)
            return false;

        if (unk_08 != rhs.unk_08)
            return false;

        if (portrait_cms != rhs.portrait_cms)
            return false;

        if (portrait_costume_index != rhs.portrait_costume_index)
            return false;

        if (portrait_transformation != rhs.portrait_transformation)
            return false;

        if (msg_id != rhs.msg_id)
            return false;

        return true;
    }

    inline bool operator==(const QbtDialoguePart &rhs) const
    {
        if (qbt_id != rhs.qbt_id)
            return false;

        return PartialCompare(rhs);
    }

    inline bool operator!=(const QbtDialoguePart &rhs) const
    {
        return !(*this == rhs);
    }
};

struct QbtEntry
{
    uint16_t unk_00; // Always 0 in normal entries. Interactive entries: 0 and 1 found (but apparently unused by exe). Special dialogue: always 2.
    uint16_t unk_02; // In normal/interactive entries: either 0, or matches the qbt id of parts (may be unused by exe). Special dialogue: always 0.

    // Normal entries: next two are always 0 and unused
    // Interactive entries: 0 -> none, 1 -> effect.
    // 2 -> item. Item type (as in QXD) is obtained through param / 1000, and item id through param % 1000.
    // 3 -> help (battle) Id is unused apparently (even if in some case it was found to be 3001, probably a edit leftover)
    // 5 -> unknown, only used in TMQ_1000, what's the difference with 0?
    // Special dialogue: type is range 0-2, i guess it serves a different purpose there; interaction param is 0/1
    uint32_t interaction_type;
    uint32_t interaction_param;

    // 3: hypno ally
    // 4: pre hypno
    // 5: giant ki blast
    // 6: area change
    // 7: giant ki returned
    // 8: teleport failed
    // 9: controlled ally defeated (battle) (todo: check if this works with any enemy...)
    // 11: teleport end
    // 12: crystals destroyed?
    // 14: teleport succeeded
    uint32_t special_event; // Always 0 in normal/interactive entries. Special entries: a number in 0-14 range (checked by exe).
    uint32_t special_on_event_end; // Always 0 in normal/interactive entries. Special entries: boolean

    // Normal dialogue: 0, 1 or 3 (may be unused by exe). Value of 1 is used in TMQ with dragon balls (in Trunks voice).
    // Value of 1 and 3 are used in some TPQ. Seem like 3 is used in non-portrait dialogues, but changing value doesn't change that.
    // Interactive/special dialogue: always 0
    uint32_t unk_1C;

    // Normal: when not 0, it *usually* matches the cms id of first part (may be ignored by game)
    // Interacrive dialogue: when not 0, it always matches the cms id of first part
    // special dialogue: stage
    uint16_t cms_20;

    uint16_t unk_22; // Normal/interactive dialogue: always 0. Special dialogue: 0 or -1
    uint32_t unk_24; // Normal/interactive dialogue: always 0. Special dialogue: Numbers like 0-4 but range is unknown.
    uint32_t unk_2C; // Normal/interactive dialogue always 0. Special dialogue: numbers such as 40/30/60/230/180/64/60/4/184
    uint32_t unk_30; // Normal/interactive dialogue: always 0. Special dialogue: 0-1

    std::vector<QbtDialoguePart> parts;

    QbtEntry()
    {
        unk_00 = unk_02 = cms_20 = unk_22 = 0;
        interaction_type = interaction_param = special_event = special_on_event_end = unk_1C = unk_24 = unk_2C = unk_30 = 0;
    }

    inline bool PartialCompare(const QbtEntry &rhs) const
    {
        if (unk_00 != rhs.unk_00)
            return false;

        if (unk_02 != rhs.unk_02)
        {
            if (unk_02 == 0 || rhs.unk_02 == 0)
                return false;
        }

        if (interaction_type != rhs.interaction_type)
            return false;

        if (interaction_param != rhs.interaction_param)
            return false;

        if (special_event != rhs.special_event)
            return false;

        if (special_on_event_end != rhs.special_on_event_end)
            return false;

        if (unk_1C != rhs.unk_1C)
            return false;

        if (cms_20 != rhs.cms_20)
            return false;

        if (unk_22 != rhs.unk_22)
            return false;

        if (unk_24 != rhs.unk_24)
            return false;

        if (unk_2C != rhs.unk_2C)
            return false;

        if (unk_30 != rhs.unk_30)
            return false;

        if (parts.size() != rhs.parts.size())
            return false;

        for (size_t i = 0; i < parts.size(); i++)
        {
            if (!parts[i].PartialCompare(rhs.parts[i]))
                return false;
        }

        return true;
    }

    inline bool operator==(const QbtEntry &rhs) const
    {
        if (unk_00 != rhs.unk_00)
            return false;

        if (unk_02 != rhs.unk_02)
            return false;

        if (interaction_type != rhs.interaction_type)
            return false;

        if (interaction_param != rhs.interaction_param)
            return false;

        if (special_event != rhs.special_event)
            return false;

        if (special_on_event_end != rhs.special_on_event_end)
            return false;

        if (unk_1C != rhs.unk_1C)
            return false;

        if (cms_20 != rhs.cms_20)
            return false;

        if (unk_22 != rhs.unk_22)
            return false;

        if (unk_24 != rhs.unk_24)
            return false;

        if (unk_2C != rhs.unk_2C)
            return false;

        if (unk_30 != rhs.unk_30)
            return false;

        if (parts != rhs.parts)
            return false;

        return true;
    }

    inline bool operator!=(const QbtEntry &rhs) const
    {
        return !(*this == rhs);
    }
};

class QbtFile : public BaseFile
{
private:

    std::vector<QbtEntry> normal_entries;
    std::vector<QbtEntry> interactive_entries;
    std::vector<QbtEntry> special_entries;
    bool uses_endianess_check;

    void LoadEntry(QbtEntry &entry, const QBTEntry *file_entry, const QBTDialoguePart *file_parts, const QBTMsgId *file_msg);
    bool SaveEntry(const QbtEntry &entry, QBTEntry *file_entry, QBTDialoguePart *file_parts, QBTMsgId *file_msg, int &global_index);

protected:

    void Reset();


public:
    QbtFile();
    virtual ~QbtFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    inline size_t GetTotalNumEntries() const { return normal_entries.size() + interactive_entries.size() + special_entries.size(); }
    size_t GetTotalNumDialogueParts() const;

    inline size_t GetNumNormalEntries() const { return normal_entries.size(); }
    inline size_t GetNumInteractiveEntries() const { return interactive_entries.size(); }
    inline size_t GetNumSpecialEntries() const { return special_entries.size(); }

    inline const std::vector<QbtEntry> &GetNormalEntries() const { return normal_entries; }
    inline std::vector<QbtEntry> &GetNormalEntries() { return normal_entries; }

    inline const std::vector<QbtEntry> &GetInteractiveEntries() const { return interactive_entries; }
    inline std::vector<QbtEntry> &GetInteractiveEntries() { return interactive_entries; }

    inline const std::vector<QbtEntry> &GetSpecialEntries() const { return special_entries; }
    inline std::vector<QbtEntry> &GetSpecialEntries() { return special_entries; }

    inline bool PartialCompare(const QbtFile &rhs) const
    {
        if (normal_entries.size() != rhs.normal_entries.size())
            return false;

        for (size_t i = 0; i < normal_entries.size(); i++)
        {
            if (!normal_entries[i].PartialCompare(rhs.normal_entries[i]))
                return false;
        }

        if (interactive_entries.size() != rhs.interactive_entries.size())
            return false;

        for (size_t i = 0; i < interactive_entries.size(); i++)
        {
            if (!interactive_entries[i].PartialCompare(rhs.interactive_entries[i]))
                return false;
        }

        if (special_entries.size() != rhs.special_entries.size())
            return false;

        for (size_t i = 0; i <special_entries.size(); i++)
        {
            if (!special_entries[i].PartialCompare(rhs.special_entries[i]))
                return false;
        }

        return true;
    }

    inline bool operator==(const QbtFile &rhs) const
    {
        // Don't include uses_endianess_check in comparison
        return (normal_entries == rhs.normal_entries && interactive_entries == rhs.interactive_entries && special_entries == rhs.special_entries);
    }

    inline bool operator!=(const QbtFile &rhs) const
    {
        return !(*this == rhs);
    }
};

#endif // QBTFILE_H
