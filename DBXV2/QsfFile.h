#ifndef QSFFILE_H
#define QSFFILE_H

#include "BaseFile.h"

// "FSQ#"
#define QSF_SIGNATURE  0x23515346

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED QSFHeader
{
    uint32_t signature; // 0
    uint32_t file_size; // 4
    uint32_t num_entries; // 8
    uint32_t unk_0C; // What's the 4 here for? (maybe it is relative offset to start, starting from this field)
};
CHECK_STRUCT_SIZE(QSFHeader, 0x10);

struct PACKED QSFEntry
{
    uint32_t quest_type_offset; // 0  - offset relative to start of entry
    uint32_t num_quest_entries; // 4
    uint32_t quest_entries_offset; // 8 - offset relative to this field offset
    uint32_t unk_0C; // always zero
};
CHECK_STRUCT_SIZE(QSFEntry, 0x10);

struct PACKED QSFQuestEntry
{
    uint32_t num_quests;
    uint32_t quest_names_table_offset; // Offset relative to this field offset. Also every entry in the names table behave the same.
};
CHECK_STRUCT_SIZE(QSFQuestEntry, 8);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct QsfQuestEntry
{
    std::vector<std::string> quests;
};

struct QsfEntry
{
    std::string type;
    std::vector<QsfQuestEntry> quest_entries;
};

class QsfFile : public BaseFile
{
private:

    std::vector<QsfEntry> entries;

protected:

    void Reset();
    size_t CalculateFileSize(size_t *strings_size, size_t *total_quest_entries) const;

public:

    QsfFile();
    virtual ~QsfFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;
	
	inline size_t GetNumEntries() const { return entries.size(); }

    // Not suitable for BAQ atm
    bool AddQuest(const std::string &quest_name);
    void RemoveQuest(const std::string &quest_name);
	
	// Lower level
	inline void AddEntry(const QsfEntry &entry)
	{
		entries.push_back(entry);
	}
};

#endif // QSFFILE_H
