#include "QslFile.h"
#include "debug.h"

QslFile::QslFile()
{
    this->big_endian = false;
}

QslFile::~QslFile()
{

}

void QslFile::Reset()
{
    stages.clear();
}

size_t QslFile::CalculateFileSize() const
{
    size_t size = sizeof(QSLHeader) + stages.size()*4 + stages.size()*sizeof(QSLStage);

    for (const QslStage &stage : stages)
    {
        size += stage.entries.size()*sizeof(QSLEntry);
    }

    return size;
}

bool QslFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(QSLHeader))
        return false;

    const QSLHeader *hdr = (const QSLHeader *)buf;
    if (hdr->signature != QSL_SIGNATURE)
        return false;

    stages.resize(hdr->num_stages);

    const uint32_t *stage_table = (const uint32_t *)GetOffsetPtr(hdr, hdr->data_start);

    for (size_t i = 0; i < stages.size(); i++)
    {
        QslStage &stage = stages[i];
        const QSLStage *file_stage = (const QSLStage *)GetOffsetPtr(hdr, stage_table, (uint32_t)i);

        stage.stage_id = file_stage->stage_id;
        stage.entries.resize(file_stage->num_entries);

        const QSLEntry *file_entry = (const QSLEntry *)GetOffsetPtr(file_stage, file_stage->entries_start);

        for (size_t j = 0; j < stage.entries.size(); j++)
        {
            QslEntry &entry = stage.entries[j];

            entry.position = file_entry->position;
            entry.type = file_entry->type;
            entry.qml_item_id = file_entry->qml_item_id;
            entry.chance_idialogue = file_entry->chance_idialogue;
            entry.unk_26 = file_entry->unk_26;
            entry.qml_change = file_entry->qml_change;
            entry.default_pose = file_entry->default_pose;
            entry.talking_pose = file_entry->talking_pose;
            entry.effect_pose = file_entry->effect_pose;
            entry.unk_32 = file_entry->unk_32;

            /*if (entry.type == 1)
            {
                if (entry.unk_32 != 0 && entry.unk_32 != 1)
                {
                    DPRINTF("%d.\n", entry.unk_32);
                    Sleep(2400);
                }
            }*/

            file_entry++;
        }
    }

    return true;
}

uint8_t *QslFile::Save(size_t *psize)
{
    *psize = CalculateFileSize();
    uint8_t *buf = new uint8_t[*psize];
    memset(buf, 0, *psize);

    QSLHeader *hdr = (QSLHeader *)buf;
    hdr->signature = QSL_SIGNATURE;
    hdr->endianess_check = 0xFFFE;
    hdr->header_size = sizeof(QSLHeader);
    hdr->unk_08 = 0x10000;
    hdr->num_stages = (uint32_t)stages.size();
    hdr->data_start = sizeof(QSLHeader);

    uint32_t *stage_table = (uint32_t *)(hdr + 1);
    for (size_t i = 0; i < stages.size(); i++)
    {
        if (i == 0)
        {
            stage_table[i] = sizeof(QSLHeader) + (uint32_t)stages.size()*4;
        }
        else
        {
            stage_table[i] = stage_table[i-1] + sizeof(QSLStage);
        }
    }

    QSLEntry *file_entry = (QSLEntry *)GetOffsetPtr(hdr, sizeof(QSLHeader) + (uint32_t)stages.size()*(4+sizeof(QSLStage)), true);

    for (size_t i = 0; i < stages.size(); i++)
    {
        const QslStage &stage = stages[i];
        QSLStage *file_stage = (QSLStage *)GetOffsetPtr(hdr, stage_table, (uint32_t)i);

        file_stage->stage_id = stage.stage_id;
        file_stage->num_entries = (uint32_t)stage.entries.size();
        file_stage->entries_start = Utils::DifPointer(file_entry, file_stage);

        for (size_t j = 0; j < stage.entries.size(); j++)
        {
            const QslEntry &entry = stage.entries[j];

            if (entry.position.length() > 31)
            {
                DPRINTF("%s: position string too big.\n", FUNCNAME);;
                delete[] buf;
                return nullptr;
            }

            strcpy(file_entry->position, entry.position.c_str());
            file_entry->type = entry.type;
            file_entry->qml_item_id = entry.qml_item_id;
            file_entry->chance_idialogue = entry.chance_idialogue;
            file_entry->unk_26 = entry.unk_26;
            file_entry->qml_change = entry.qml_change;
            file_entry->default_pose = entry.default_pose;
            file_entry->talking_pose = entry.talking_pose;
            file_entry->effect_pose = entry.effect_pose;
            file_entry->unk_32 = entry.unk_32;

            file_entry++;
        }
    }

    return buf;
}

void QslFile::AddEntryToStage(const QslEntry &entry, uint32_t stage_id)
{
    for (QslStage &stage : stages)
    {
        if (stage.stage_id == stage_id)
        {
            stage.entries.push_back(entry);
            return;
        }
    }

    QslStage new_stage;

    new_stage.stage_id = stage_id;
    new_stage.entries.push_back(entry);
    stages.push_back(new_stage);
}

bool QslFile::HasIdenticalEntry(const QslEntry &entry, uint32_t stage_id) const
{
    for (const QslStage &stage : stages)
    {
        if (stage.stage_id == stage_id)
        {
            for (const QslEntry &this_entry : stage.entries)
            {
                if (this_entry == entry)
                    return true;
            }
        }
    }

    return false;
}

bool QslFile::SpecialCompare(const QslFile &rhs) const
{
    size_t num_entries = 0, rhs_num_entries = 0;

    for (const QslStage &stage : stages)
        num_entries += stage.entries.size();

    for (const QslStage &stage : rhs.stages)
        rhs_num_entries += stage.entries.size();

    if (num_entries != rhs_num_entries)
        return false;

    for (const QslStage &stage : stages)
    {
        for (const QslEntry &entry : stage.entries)
            if (!rhs.HasIdenticalEntry(entry, stage.stage_id))
                return false;
    }

    return true;
}

