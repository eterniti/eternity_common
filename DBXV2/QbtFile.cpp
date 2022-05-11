#include "QbtFile.h"
#include "debug.h"

QbtFile::QbtFile()
{
    this->big_endian = false;
    this->uses_endianess_check = true;
}

QbtFile::~QbtFile()
{

}

void QbtFile::Reset()
{
    normal_entries.clear();
    interactive_entries.clear();
    special_entries.clear();
    this->uses_endianess_check = true;
}

void QbtFile::LoadEntry(QbtEntry &entry, const QBTEntry *file_entry, const QBTDialoguePart *file_parts, const QBTMsgId *file_msg)
{
    entry.unk_00 = file_entry->unk_00;
    entry.unk_02 = file_entry->unk_02;
    entry.interaction_type = file_entry->interaction_type;
    entry.interaction_param = file_entry->interaction_param;
    entry.special_event = file_entry->special_event;
    entry.special_on_event_end = file_entry->special_on_event_end;
    entry.unk_1C = file_entry->unk_1C;
    entry.cms_20 = file_entry->cms_20;
    entry.unk_22 = file_entry->unk_22;
    entry.unk_24 = file_entry->unk_24;
    entry.unk_2C = file_entry->unk_2C;
    entry.unk_30 = file_entry->unk_30;

    entry.parts.resize(file_entry->num_parts);

    file_parts = &file_parts[file_entry->start_part_idx];
    file_msg = &file_msg[file_entry->start_part_idx];

    for (size_t i = 0; i < entry.parts.size(); i++)
    {
        QbtDialoguePart &part = entry.parts[i];

        part.qbt_id = file_parts[i].qbt_id;
        part.sub_id = file_parts[i].sub_id;
        part.cms_04 = file_parts[i].cms_04;
        part.unk_08 = file_parts[i].unk_08;

        part.portrait_cms = file_parts[i].portrait_cms;
        part.portrait_costume_index = file_parts[i].portrait_costume_index;
        part.portrait_transformation = file_parts[i].portrait_transformation;

        part.msg_id = file_msg[i].id;
    }
}

bool QbtFile::SaveEntry(const QbtEntry &entry, QBTEntry *file_entry, QBTDialoguePart *file_parts, QBTMsgId *file_msg, int &global_index)
{
    file_entry->unk_00 = entry.unk_00;
    file_entry->unk_02 = entry.unk_02;
    file_entry->interaction_type = entry.interaction_type;
    file_entry->interaction_param = entry.interaction_param;
    file_entry->special_on_event_end = entry.special_on_event_end;
    file_entry->special_event = entry.special_event;
    file_entry->special_on_event_end = entry.special_on_event_end;
    file_entry->unk_1C = entry.unk_1C;
    file_entry->cms_20 = entry.cms_20;
    file_entry->unk_22 = entry.unk_22;
    file_entry->unk_24 = entry.unk_24;
    file_entry->unk_2C = entry.unk_2C;
    file_entry->unk_30 = entry.unk_30;

    file_entry->num_parts = (uint32_t) entry.parts.size();
    file_entry->start_part_idx = global_index;
    file_parts = &file_parts[file_entry->start_part_idx];
    file_msg = &file_msg[file_entry->start_part_idx];

    for (size_t i = 0; i < entry.parts.size(); i++)
    {
        const QbtDialoguePart &part = entry.parts[i];

        file_parts[i].qbt_id = part.qbt_id;
        file_parts[i].sub_id = part.sub_id;
        file_parts[i].cms_04 = part.cms_04;
        file_parts[i].unk_08 = part.unk_08;

        file_parts[i].portrait_cms = part.portrait_cms;
        file_parts[i].portrait_costume_index = part.portrait_costume_index;
        file_parts[i].portrait_transformation = part.portrait_transformation;

        if (part.msg_id.length() > 31)
        {
            DPRINTF("%s: msg_id string longer than expected.\n", FUNCNAME);
            return false;
        }

        strcpy(file_msg[i].id, part.msg_id.c_str());

        file_parts[i].total_part_index = global_index;
        global_index++;
    }

    return true;
}

bool QbtFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(QBTHeader))
        return false;

    const QBTHeader *hdr = (const QBTHeader *)buf;
    if (hdr->signature != QBT_SIGNATURE)
        return false;

    uses_endianess_check = (hdr->endianess_check == 0xFFFE);
    normal_entries.resize(hdr->num_normal_dialogue);
    interactive_entries.resize(hdr->num_interactive_dialogue);
    special_entries.resize(hdr->num_special_dialogue);

    const QBTEntry *file_normal_entry = (const QBTEntry *)GetOffsetPtr(hdr, hdr->normal_dialogue_start);
    const QBTDialoguePart *file_parts = (const QBTDialoguePart *)GetOffsetPtr(file_normal_entry, hdr->num_entries*sizeof(QBTEntry), true);
    const QBTMsgId *file_msg = (const QBTMsgId *)GetOffsetPtr(file_parts, hdr->num_parts*sizeof(QBTDialoguePart), true);

    for (size_t i = 0; i < normal_entries.size(); i++)
    {
        LoadEntry(normal_entries[i], file_normal_entry, file_parts, file_msg);
        file_normal_entry++;
    }

    const QBTEntry *file_interactive_entry = (const QBTEntry *)GetOffsetPtr(hdr, hdr->interactive_dialogue_start);
    for (size_t i = 0; i < interactive_entries.size(); i++)
    {
        LoadEntry(interactive_entries[i], file_interactive_entry, file_parts, file_msg);
        file_interactive_entry++;
    }

    const QBTEntry *file_special_entry = (const QBTEntry *)GetOffsetPtr(hdr, hdr->special_dialogue_start);
    for (size_t i = 0; i < special_entries.size(); i++)
    {
        LoadEntry(special_entries[i], file_special_entry, file_parts, file_msg);
        file_special_entry++;
    }

    /*for (const QbtEntry &entry : interactive_entries)
    {
        for (const QbtDialoguePart &part : entry.parts)
        {
            if (false)
            {
                DPRINTF("%d  %d  %d\n", part.cms_04, entry.cms_20, part.portrait_cms);
                Sleep(2500);
            }
        }

        if (entry.unk_00 != 1)
        {
            DPRINTF("%d\n", entry.unk_00);
            Sleep(2222);
        }
    }*/

    return true;
}

uint8_t *QbtFile::Save(size_t *psize)
{
    uint16_t num_parts = (uint16_t)GetTotalNumDialogueParts();
    *psize = sizeof(QBTHeader) + GetTotalNumEntries()*sizeof(QBTEntry) + num_parts*(sizeof(QBTDialoguePart) + sizeof(QBTMsgId));

    uint8_t *buf = new uint8_t[*psize];
    memset(buf, 0, *psize);

    QBTHeader *hdr = (QBTHeader *)buf;
    hdr->signature = QBT_SIGNATURE;
    hdr->endianess_check = (uses_endianess_check) ? 0xFFFE : 0;
    hdr->header_size = sizeof(QBTHeader);
    hdr->num_entries = (uint16_t)GetTotalNumEntries();
    hdr->num_parts = num_parts;
    hdr->num_normal_dialogue = (uint16_t) normal_entries.size();
    hdr->num_interactive_dialogue = (uint16_t) interactive_entries.size();
    hdr->num_special_dialogue = (uint16_t) special_entries.size();
    hdr->normal_dialogue_start = sizeof(QBTHeader);
    hdr->interactive_dialogue_start = hdr->normal_dialogue_start + hdr->num_normal_dialogue*sizeof(QBTEntry);
    hdr->special_dialogue_start = hdr->interactive_dialogue_start + hdr->num_interactive_dialogue*sizeof(QBTEntry);

    QBTEntry *file_normal_entry = (QBTEntry *)GetOffsetPtr(hdr, hdr->normal_dialogue_start);
    QBTDialoguePart *file_parts = (QBTDialoguePart *)GetOffsetPtr(file_normal_entry, hdr->num_entries*sizeof(QBTEntry), true);
    QBTMsgId *file_msg = (QBTMsgId *)GetOffsetPtr(file_parts, hdr->num_parts*sizeof(QBTDialoguePart), true);
    int global_index = 0;

    for (size_t i = 0; i < normal_entries.size(); i++)
    {
        if (!SaveEntry(normal_entries[i], file_normal_entry, file_parts, file_msg, global_index))
        {
            delete[] buf;
            return nullptr;
        }

        file_normal_entry++;
    }

    QBTEntry *file_interactive_entry = (QBTEntry *)GetOffsetPtr(hdr, hdr->interactive_dialogue_start);
    for (size_t i = 0; i < interactive_entries.size(); i++)
    {
        if (!SaveEntry(interactive_entries[i], file_interactive_entry, file_parts, file_msg, global_index))
        {
            delete[] buf;
            return nullptr;
        }

        file_interactive_entry++;
    }

    QBTEntry *file_special_entry = (QBTEntry *)GetOffsetPtr(hdr, hdr->special_dialogue_start);
    for (size_t i = 0; i < special_entries.size(); i++)
    {
        if (!SaveEntry(special_entries[i], file_special_entry, file_parts, file_msg, global_index))
        {
            delete[] buf;
            return nullptr;
        }

        file_special_entry++;
    }

    return buf;
}

size_t QbtFile::GetTotalNumDialogueParts() const
{
    size_t num = 0;

    for (const QbtEntry &entry : normal_entries)
        num += entry.parts.size();

    for (const QbtEntry &entry : interactive_entries)
        num += entry.parts.size();

    for (const QbtEntry &entry : special_entries)
        num += entry.parts.size();

    return num;
}

