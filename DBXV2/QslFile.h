#ifndef QSLFILE_H
#define QSLFILE_H

#include "BaseFile.h"

// "#QSL"
#define QSL_SIGNATURE  0x4C535123

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

#define QSL_POSITION_LENGTH 32

enum QslPositionType
{
    QSL_POSITION_ITEM = 0,
    QSL_POSITION_ICHAR = 1,
    QSL_POSITION_CHAR2 = 2,
    QSL_POSITION_CHAR3 = 3,
    QSL_POSITION_CHAR5 = 5
};

struct PACKED QSLHeader
{
    uint32_t signature;
    uint16_t endianess_check;
    uint16_t header_size;
    uint32_t unk_08; // Always 0x10000
    uint32_t num_stages; // 0xC
    uint32_t unk_14; // Always 0
    uint32_t data_start;  // 0x14
};
CHECK_STRUCT_SIZE(QSLHeader, 0x18);

struct PACKED QSLStage
{
    uint32_t stage_id; // 0
    uint16_t unk_04[3]; // Always 0
    uint16_t num_entries; // 0xA
    uint32_t entries_start; // 0xC - value is relative to this stage
};
CHECK_STRUCT_SIZE(QSLStage, 0x10);

struct PACKED QSLEntry
{
    char position[QSL_POSITION_LENGTH]; // 0
    uint16_t type; // 0x20 Known values: 0-3, 5 (only seen in a couple of legend patrol quests)
    uint16_t qml_item_id; // 0x22
    uint16_t chance_idialogue; // 0x24  Known values: 0, 20, 30, 50, 60, -1
    uint16_t unk_26; // 0 or 0xffff
    uint32_t qml_change; // 28
    uint16_t default_pose; // 2C
    uint16_t talking_pose; // 2E
    uint16_t effect_pose; // 30
    uint16_t unk_32;
    uint16_t unk_34[6]; // Always 0
};
CHECK_STRUCT_SIZE(QSLEntry, 0x40);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct QslEntry
{
    std::string position;
    uint16_t type; // 3 is used for chars positions. 2 also chars positions (what is the difference?)  1 -> interactive chars. 0 -> items. 5 -> char position too
    uint16_t qml_item_id;    
    uint16_t chance_idialogue; // Type 0: exact numbers <= 100. Type 1: interactive dialogue index. Others: always 0
    uint16_t unk_26; // Type 1: -1 or 0. Others: always 0
    uint32_t qml_change; // Type 1: qml change. Type 2: 0 or 1 (1 only in CTP_15_00) Others: always 0.
    uint16_t default_pose; // Type 1: ean index from CMN.mis.ean. Others: always 0.
    uint16_t talking_pose; // Type 1: ean index from CMN.mis.ean. Others: always 0.
    uint16_t effect_pose; // Type 1: ean index from CMN.mis.ean. Others: always 0.
    uint16_t unk_32; // Type 1: 0 or 1. Others: always 0. Update: in one of the quests added in 1.11, the value 1 was seen in type 3

    QslEntry()
    {
        type = 0;
        qml_item_id = 0;
        chance_idialogue = 0;
        unk_26 = default_pose = talking_pose = effect_pose = unk_32 = 0;
        qml_change = 0;
    }

    inline bool operator==(const QslEntry &rhs) const
    {
        if (position != rhs.position)
            return false;

        if (type != rhs.type)
            return false;

        if (qml_item_id != rhs.qml_item_id)
            return false;

        if (chance_idialogue != rhs.chance_idialogue)
            return false;

        if (unk_26 != rhs.unk_26)
            return false;

        if (qml_change != rhs.qml_change)
            return false;

        if (default_pose != rhs.default_pose)
            return false;

        if (talking_pose != rhs.talking_pose)
            return false;

        if (effect_pose != rhs.effect_pose)
            return false;

        if (unk_32 != rhs.unk_32)
            return false;

        return true;
    }

    inline bool operator!=(const QslEntry &rhs) const
    {
        return !(*this == rhs);
    }
};

struct QslStage
{
    uint32_t stage_id;
    std::vector<QslEntry> entries;

    QslStage()
    {
        stage_id = 0;
    }

    inline bool operator==(const QslStage &rhs) const
    {
        return (stage_id == rhs.stage_id && entries == rhs.entries);
    }

    inline bool operator!=(const QslStage &rhs) const
    {
        return !(*this == rhs);
    }
};

class QslFile : public BaseFile
{
private:

    std::vector<QslStage> stages;

protected:

    void Reset();
    size_t CalculateFileSize() const;

public:
    QslFile();
    virtual ~QslFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    inline size_t GetNumStages() const { return stages.size(); }

    void AddEntryToStage(const QslEntry &entry, uint32_t stage_id);

    bool HasIdenticalEntry(const QslEntry &entry, uint32_t stage_id) const;

    inline const std::vector<QslStage> &GetStages() const { return stages; }
    inline std::vector<QslStage> &GetStages() { return stages; }

    inline const QslStage &operator[](size_t n) const { return stages[n]; }
    inline const QslStage &operator[](size_t n) { return stages[n]; }

    inline std::vector<QslStage>::const_iterator begin() const { return stages.begin(); }
    inline std::vector<QslStage>::const_iterator end() const { return stages.end(); }

    inline std::vector<QslStage>::iterator begin() { return stages.begin(); }
    inline std::vector<QslStage>::iterator end() { return stages.end(); }

    // Compares ignoring stage order, and ignoring empty stages
    // Also, having in consideration repeated stages
    bool SpecialCompare(const QslFile &rhs) const;

    inline bool operator==(const QslFile &rhs) const
    {
        return (stages == rhs.stages);
    }

    inline bool operator!=(const QslFile &rhs) const
    {
        return !(*this == rhs);
    }
};

#endif // QSLFILE_H
