#ifndef QMLFILE_H
#define QMLFILE_H

#include "CusFile.h"
#include "QxdFile.h"

// "#QML"
#define QML_SIGNATURE  0x4C4D5123

enum QmlTeam
{
    QML_TEAM_NONE,
    QML_TEAM_A,
    QML_TEAM_B,
};

enum QmlAI
{
    QML_AI_HUMAN,
    QML_AI_STILL,
    QML_AI_NORMAL
};

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED QMLHeader
{
    uint32_t signature;
    uint16_t endianess_check;
    uint16_t header_size;
    uint32_t num_entries; // 8
    uint32_t data_start;  // 0xC
};
CHECK_STRUCT_SIZE(QMLHeader, 0x10);

struct PACKED QMLEntry
{
   uint32_t id; // 0
   int32_t battle_index; // 4 between -1 and 6.
   uint8_t unk_08[4]; // Always set to 45,45,45,0  in the 100% of qml files.
   uint32_t unk_0C;
   uint32_t stage_id; // 0x10
   uint32_t spawn_at_start; // 0x14
   uint32_t ai; // 0x18 - 0 = Player, 2 = AI. 1 -> inmobile. Values of 3-5 possible too.
   uint32_t team; // 0x1C - 1 usually ally, 2 usually enemy. 0 -> neither (inmobile)
   uint32_t qxd_id; // 0x20
   uint32_t unk_24; // Mostly 9999, but found other typical decimal values
   uint32_t unk_28; // Mostly 5 except in a few HLQ where it is 0
   uint32_t unk_2C; // Mostly 0, but found values of 1-4 in some TMQ
   uint16_t unk_30[5]; // [0,1] -> mostly -1, found 0 and 1 aswell. [2] -> foudn values of 0, 2, 5, 8, 16, maybe other. [3] -> always 0 in 100% of files. [4] -> found 0 and 2
   uint16_t skills[NUM_SKILL_SLOTS]; // 0x3A

   QMLEntry()
   {
       id = 0;
       battle_index = -1;
       unk_08[0] = unk_08[1] = unk_08[2] = 45;
       unk_08[3] = 0;
       unk_0C = 0;
       stage_id = 0;
       spawn_at_start = 0;
       ai = 0;
       team = 1;
       qxd_id = 0;
       unk_24 = 9999;
       unk_28 = 5;
       unk_2C = 0;
       unk_30[0] = unk_30[1] = 0xFFFF;
       unk_30[2] = unk_30[3] = unk_30[4] = 0;
       memset(skills, 0xFF, sizeof(skills));
   }

   inline bool ComparePartial(const QMLEntry &rhs) const
   {
       if (battle_index != rhs.battle_index)
           return false;

       if (memcmp(unk_08, rhs.unk_08, sizeof(unk_08)) != 0)
           return false;

       if (unk_0C != rhs.unk_0C)
           return false;

       if (stage_id != rhs.stage_id)
           return false;

       if (spawn_at_start != rhs.spawn_at_start)
           return false;

       if (ai != rhs.ai)
           return false;

       if (team != rhs.team)
           return false;       

       if (unk_24 != rhs.unk_24)
           return false;

       if (unk_28 != rhs.unk_28)
           return false;

       if (unk_2C != rhs.unk_2C)
           return false;

       if (memcmp(unk_30, rhs.unk_30, sizeof(unk_30)) != 0)
           return false;

       if (memcmp(skills, rhs.skills, sizeof(skills)) != 0)
           return false;

       return true;
   }

   // Takes into account qml id, but doesn't fail if qxd id are different, instead, it checks if the qxd entries are similar
   bool SpecialCompare(const QMLEntry &rhs, QxdFile &qxd) const;

   inline bool operator==(const QMLEntry &rhs) const
   {
       if (id != rhs.id)
           return false;

       if (qxd_id != rhs.qxd_id)
           return false;

       return ComparePartial(rhs);
   }

   inline bool operator!=(const QMLEntry &rhs) const
   {
       return !(*this == rhs);
   }
};
CHECK_STRUCT_SIZE(QMLEntry, 0x4C);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

typedef QMLEntry QmlEntry;

class QmlFile : public BaseFile
{
private:

    std::vector<QmlEntry> entries;

protected:

    void Reset();

public:
    QmlFile();
    virtual ~QmlFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    inline size_t GetNumEntries() const { return entries.size(); }

    inline void AddQmlEntry(QmlEntry &new_entry)
    {
        new_entry.id = (uint32_t)GetNumEntries();
        entries.push_back(new_entry);
    }

    // Takes into account qml id, but doesn't fail if qxd id are different, instead, it checks if the qxd entries are similar
    bool SpecialCompare(const QmlFile &rhs, QxdFile &qxd) const;

    QmlEntry *FindEntryById(uint32_t id);

    inline const std::vector<QmlEntry> &GetEntries() const { return entries; }
    inline std::vector<QmlEntry> &GetEntries() { return entries; }

    inline const QmlEntry &operator[](size_t n) const { return entries[n]; }
    inline const QmlEntry &operator[](size_t n) { return entries[n]; }

    inline std::vector<QmlEntry>::const_iterator begin() const { return entries.begin(); }
    inline std::vector<QmlEntry>::const_iterator end() const { return entries.end(); }

    inline std::vector<QmlEntry>::iterator begin() { return entries.begin(); }
    inline std::vector<QmlEntry>::iterator end() { return entries.end(); }

    inline bool operator==(const QmlFile &rhs) const
    {
        return (entries == rhs.entries);
    }

    inline bool operator!=(const QmlFile &rhs) const
    {
        return !(*this == rhs);
    }
};

#endif // QMLFILE_H
