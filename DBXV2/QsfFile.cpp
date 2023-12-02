#include "QsfFile.h"
#include "debug.h"

QsfFile::QsfFile()
{
    this->big_endian = false;
}

QsfFile::~QsfFile()
{

}

void QsfFile::Reset()
{
    entries.clear();
}

bool QsfFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(QSFHeader))
        return false;

    const QSFHeader *hdr = (const QSFHeader *)buf;
    entries.resize(hdr->num_entries);

    const QSFEntry *file_entries = (const QSFEntry *)(hdr + 1);

    for (size_t i = 0; i < entries.size(); i++)
    {
        QsfEntry &entry = entries[i];

        entry.type = (const char *)GetOffsetPtr(&file_entries[i].quest_type_offset, file_entries[i].quest_type_offset);
        entry.quest_entries.resize(file_entries[i].num_quest_entries);
        //DPRINTF("Type: %s\n", entry.type.c_str());

        const QSFQuestEntry *file_quest_entries = (const QSFQuestEntry *)GetOffsetPtr(&file_entries[i].quest_entries_offset, file_entries[i].quest_entries_offset);

        for (size_t j = 0; j < entry.quest_entries.size(); j++)
        {
            QsfQuestEntry &quest_entry = entry.quest_entries[j];

            quest_entry.quests.resize(file_quest_entries[j].num_quests);
            const uint32_t *names_table = (const uint32_t *)GetOffsetPtr(&file_quest_entries[j].quest_names_table_offset, file_quest_entries[j].quest_names_table_offset);

            for (size_t s = 0; s < quest_entry.quests.size(); s++)
            {
                quest_entry.quests[s] = (const char *)GetOffsetPtr(&names_table[s], names_table[s]);
            }
        }
    }

    return true;
}

size_t QsfFile::CalculateFileSize(size_t *strings_size, size_t *total_quest_entries) const
{
    size_t size = sizeof(QSFHeader) + entries.size() * sizeof(QSFEntry);
    *strings_size = 0;
    *total_quest_entries = 0;

    for (const QsfEntry &entry : entries)
    {
        size += entry.type.length() + 1;
        *strings_size += entry.type.length() + 1;
        size += entry.quest_entries.size() * sizeof(QSFQuestEntry);

        *total_quest_entries += entry.quest_entries.size();

        for (const QsfQuestEntry &quest_entry : entry.quest_entries)
        {
            size += quest_entry.quests.size() * sizeof(uint32_t);

            for (const std::string &str : quest_entry.quests)
            {
                size += str.length() + 1;
                *strings_size += str.length() + 1;
            }
        }
    }

    return size;
}

uint8_t *QsfFile::Save(size_t *psize)
{
    size_t strings_size, total_quest_entries;

    *psize = CalculateFileSize(&strings_size, &total_quest_entries);
    uint8_t *buf = new uint8_t[*psize];
    memset(buf, 0, *psize);

    QSFHeader *hdr = (QSFHeader *)buf;

    hdr->signature = QSF_SIGNATURE;
    hdr->file_size = (uint32_t)*psize;
    hdr->num_entries = (uint32_t)entries.size();
    hdr->unk_0C = 4;

    QSFEntry *file_entries = (QSFEntry *)(hdr + 1);
    QSFQuestEntry *file_quest_entry = (QSFQuestEntry *)(file_entries + entries.size());
    uint32_t *names_table = (uint32_t *)(file_quest_entry + total_quest_entries);
    char* file_str = (char *) buf + *psize - strings_size;

    for (size_t i = 0; i < entries.size(); i++)
    {
        const QsfEntry &entry = entries[i];

        strcpy(file_str, entry.type.c_str());
        file_entries[i].quest_type_offset = Utils::DifPointer(file_str, &file_entries[i].quest_type_offset);
        file_str += entry.type.length() + 1;

        file_entries[i].num_quest_entries = (uint32_t)entry.quest_entries.size();
        file_entries[i].quest_entries_offset = Utils::DifPointer(file_quest_entry, &file_entries[i].quest_entries_offset);

        for (size_t j = 0; j < entry.quest_entries.size(); j++)
        {
            const QsfQuestEntry &quest_entry = entry.quest_entries[j];

            file_quest_entry->num_quests = (uint32_t)quest_entry.quests.size();
            file_quest_entry->quest_names_table_offset = Utils::DifPointer(names_table, &file_quest_entry->quest_names_table_offset);

            for (const std::string &quest : quest_entry.quests)
            {
                *names_table = Utils::DifPointer(file_str, names_table);
                names_table++;

                strcpy(file_str, quest.c_str());
                file_str += quest.length()+1;
            }

            file_quest_entry++;
        }
    }

    return buf;
}

bool QsfFile::AddQuest(const std::string &quest_name)
{
    if (quest_name.length() < 3)
        return false;

    const std::string upper_qn = Utils::ToUpperCase(quest_name);
    const std::string prefix = upper_qn.substr(0, 3);

    for (QsfEntry &entry : entries)
    {
        if (entry.quest_entries.size() == 0)
            continue;

        if (prefix == entry.type)
        {
            bool exists = false;

            for (const std::string &str : entry.quest_entries[0].quests)
            {
                if (Utils::ToUpperCase(str) == upper_qn)
                {
                    exists = true;
                    break;
                }
            }

            if (!exists)
                entry.quest_entries[0].quests.push_back(upper_qn);

            return true;
        }
    }

    return false;
}

void QsfFile::RemoveQuest(const std::string &quest_name)
{
    if (quest_name.length() < 3)
        return;

    const std::string upper_qn = Utils::ToUpperCase(quest_name);
    const std::string prefix = upper_qn.substr(0, 3);

    for (QsfEntry &entry : entries)
    {
        if (entry.quest_entries.size() == 0)
            continue;

        if (prefix == entry.type)
        {
           for (size_t i = 0; i < entry.quest_entries[0].quests.size(); i++)
           {
               if (Utils::ToUpperCase(entry.quest_entries[0].quests[i]) == upper_qn)
               {
                   entry.quest_entries[0].quests.erase(entry.quest_entries[0].quests.begin()+i);
                   i--;
               }
           }
        }
    }
}

