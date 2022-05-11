#include "SevFile.h"
#include "Xenoverse2.h"
#include "debug.h"

TiXmlElement *SevEventEntry::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("EventEntry");

    Utils::WriteParamUnsigned(entry_root, "U_00", unk_00, true);
    Utils::WriteParamUnsigned(entry_root, "CUE_ID", cue_id, (cue_id>=0x80000000));
    Utils::WriteParamUnsigned(entry_root, "FILE_ID", file_id, (file_id==0xFFFFFFFF));
    Utils::WriteParamUnsigned(entry_root, "NU_0C", new_unk_0C, true);
    Utils::WriteParamUnsigned(entry_root, "RESPONSE_CUE_ID", response_cue_id, (response_cue_id>=0x80000000));
    Utils::WriteParamUnsigned(entry_root, "RESPONSE_FILE_ID", response_file_id, (response_file_id==0xFFFFFFFF));
    Utils::WriteParamUnsigned(entry_root, "U_18", unk_18, true);
    Utils::WriteParamUnsigned(entry_root, "OU_0C", old_unk_0C, true);
    Utils::WriteParamUnsigned(entry_root, "OU_10", old_unk_10, true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool SevEventEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "U_00", &unk_00))
        return false;

    if (!Utils::GetParamUnsignedWithMultipleNames(root, &cue_id, "CUE_ID", "U_04"))
        return false;

    if (!Utils::GetParamUnsignedWithMultipleNames(root, &file_id, "FILE_ID", "U_08"))
        return false;

    if (Utils::ReadParamUnsigned(root, "U_0C", &old_unk_0C))
    {
        // Old format
        if (!Utils::GetParamUnsigned(root, "U_10", &old_unk_10))
            return false;

        new_unk_0C = 0;
        response_cue_id = 0xFFFFFFFF;
        response_file_id = 0;
        unk_18 = 0;
    }
    else
    {
        // New format
        if (!Utils::GetParamUnsigned(root, "NU_0C", &new_unk_0C))
            return false;

        if (!Utils::GetParamUnsignedWithMultipleNames(root, &response_cue_id, "NU_10", "RESPONSE_CUE_ID"))
            return false;

        if (!Utils::GetParamUnsignedWithMultipleNames(root, &response_file_id, "RESPONSE_FILE_ID", "U_14"))
            return false;

        if (!Utils::GetParamUnsigned(root, "U_18", &unk_18))
            return false;

        if (!Utils::GetParamUnsigned(root, "OU_0C", &old_unk_0C))
            return false;

        if (!Utils::GetParamUnsigned(root, "OU_10", &old_unk_10))
            return false;
    }



    return true;
}

TiXmlElement *SevEvent::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Event");

    entry_root->SetAttribute("type", type);

    for (const SevEventEntry &entry :entries)
        entry.Decompile(entry_root);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool SevEvent::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "type", &type))
    {
        if (!Utils::ReadAttrUnsigned(root, "u_00", &type))
            return false;
    }

    entries.clear();

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "EventEntry")
        {
           SevEventEntry entry;

           if (!entry.Compile(elem))
               return false;

           entries.push_back(entry);
        }
    }

    return true;
}

TiXmlElement *SevCharEvents::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("CharEvents");

    entry_root->SetAttribute("char_id", Utils::UnsignedToString(char_id, true));
    entry_root->SetAttribute("costume_id", Utils::UnsignedToString(costume_id, true));

    std::string name = Xenoverse2::GetCharaAndCostumeName(char_id, costume_id, 0, true);
    if (name.length() != 0)
        Utils::WriteComment(entry_root, name);

    for (const SevEvent &event : events)
        event.Decompile(entry_root);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool SevCharEvents::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "char_id", &char_id))
        return false;

    if (!Utils::ReadAttrUnsigned(root, "costume_id", &costume_id))
        return false;

    events.clear();

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Event")
        {
           SevEvent event;

           if (!event.Compile(elem))
               return false;

           events.push_back(event);
        }
    }

    return true;
}

TiXmlElement *SevEntry::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("SevEntry");

    entry_root->SetAttribute("char_id", Utils::UnsignedToString(char_id, true));
    entry_root->SetAttribute("costume_id", Utils::UnsignedToString(costume_id, true));
    entry_root->SetAttribute("u_08", unk_08);

    std::string name = Xenoverse2::GetCharaAndCostumeName(char_id, costume_id, 0, true);
    if (name.length() != 0)
        Utils::WriteComment(entry_root, name);

    for (const SevCharEvents &cevents : chars_events)
        cevents.Decompile(entry_root);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool SevEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "char_id", &char_id))
        return false;

    if (!Utils::ReadAttrUnsigned(root, "costume_id", &costume_id))
        return false;

    if (!Utils::ReadAttrUnsigned(root, "u_08", &unk_08))
        unk_08 = 0;

    chars_events.clear();

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "CharEvents")
        {
           SevCharEvents cevents;

           if (!cevents.Compile(elem))
               return false;

           chars_events.push_back(cevents);
        }
    }

    return true;
}

SevFile::SevFile()
{
    this->big_endian = false;
}

SevFile::~SevFile()
{

}

void SevFile::Reset()
{
    entries.clear();
}

bool SevFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(SEVHeader))
        return false;

    const SEVHeader *hdr = (const SEVHeader *)buf;

    if (hdr->signature != SEV_SIGNATURE)
        return false;

    const SEVEntry *file_entries = (const SEVEntry *)(buf+hdr->header_size);
    const SEVCharEvents *file_cees_top = (const SEVCharEvents *)(file_entries+hdr->num_entries);
    const SEVCharEvents *file_cees = file_cees_top;

    uint32_t num_events = 0;

    for (uint32_t i = 0; i < hdr->num_entries; i++)
    {
        if (file_entries[i].unk_08 != 0 && file_entries[i].unk_08 != 1)
        {
            // Probably a pre 1.15 file
            return false;
        }

        for (uint32_t j = 0; j < file_entries[i].num_cev_entries; j++)
        {
            num_events += file_cees->num_events;
            file_cees++;
        }        
    }

    const SEVEvent *file_event = (const SEVEvent *)file_cees;
    const SEVEventEntry *file_ev_entry = (const SEVEventEntry *)(file_event+num_events);

    entries.resize(hdr->num_entries);
    file_cees = file_cees_top;

    for (size_t i = 0; i < entries.size(); i++)
    {
        SevEntry &entry = entries[i];

        entry.char_id = file_entries[i].char_id;
        entry.costume_id = file_entries[i].costume_id;
        entry.unk_08 = file_entries[i].unk_08;

        entry.chars_events.resize(file_entries[i].num_cev_entries);

        for (size_t j = 0; j < entry.chars_events.size(); j++)
        {
            SevCharEvents &cevents = entry.chars_events[j];

            cevents.char_id = file_cees->char_id;
            cevents.costume_id = file_cees->costume_id;
            cevents.events.resize(file_cees->num_events);

            for (size_t k = 0; k < cevents.events.size(); k++)
            {
                SevEvent &event = cevents.events[k];

                event.type = file_event->type;
                event.entries.resize(file_event->num_entries);

                for (size_t l = 0; l < event.entries.size(); l++)
                {
                    SevEventEntry &ev_entry = event.entries[l];

                    ev_entry.unk_00 = file_ev_entry->unk_00;
                    ev_entry.cue_id = file_ev_entry->cue_id;
                    ev_entry.file_id = file_ev_entry->file_id;
                    ev_entry.new_unk_0C = file_ev_entry->new_unk_0C;
                    ev_entry.response_cue_id = file_ev_entry->response_cue_id;
                    ev_entry.response_file_id = file_ev_entry->response_file_id;
                    ev_entry.unk_18 = file_ev_entry->unk_18;
                    ev_entry.old_unk_0C = file_ev_entry->old_unk_0C;
                    ev_entry.old_unk_10 = file_ev_entry->old_unk_10;

                    file_ev_entry++;
                }

                file_event++;
            }

            file_cees++;
        }
    }

    return true;
}

size_t SevFile::CalculateFileLayout(uint32_t *events_offset, uint32_t *ev_entries_offset) const
{
    size_t size = sizeof(SEVHeader);

    size += entries.size()*sizeof(SEVEntry);

    for (const SevEntry &entry : entries)
    {
        size += entry.chars_events.size() * sizeof(SEVCharEvents);
    }

    *events_offset = (uint32_t)size;

    for (const SevEntry &entry : entries)
    {
        for (const SevCharEvents &cevents : entry.chars_events)
        {
            size += cevents.events.size() * sizeof(SEVEvent);
        }
    }

    *ev_entries_offset = (uint32_t)size;

    for (const SevEntry &entry : entries)
    {
        for (const SevCharEvents &cevents : entry.chars_events)
        {
            for (const SevEvent &event : cevents.events)
            {
                size += event.entries.size() * sizeof(SEVEventEntry);
            }
        }
    }

    return size;
}

uint8_t *SevFile::Save(size_t *psize)
{
    uint32_t events_offset, ev_entries_offset;
    size_t size = CalculateFileLayout(&events_offset, &ev_entries_offset);

    uint8_t *buf = new uint8_t[size];
    memset(buf, 0, size);

    SEVHeader *hdr = (SEVHeader *)buf;
    SEVEntry *file_entries = (SEVEntry *)(hdr+1);
    SEVCharEvents *file_cees = (SEVCharEvents *)(file_entries+entries.size());
    SEVEvent *file_event = (SEVEvent *)(buf+events_offset);
    SEVEventEntry *file_ev_entry = (SEVEventEntry *)(buf+ev_entries_offset);

    hdr->signature = SEV_SIGNATURE;
    hdr->endianess_check = val16(0xFFFE);
    hdr->header_size = sizeof(SEVHeader);
    hdr->num_entries = (uint32_t)entries.size();

    for (size_t i = 0; i < entries.size(); i++)
    {
        const SevEntry &entry = entries[i];

        file_entries[i].char_id = entry.char_id;
        file_entries[i].costume_id = entry.costume_id;
        file_entries[i].unk_08 = entry.unk_08;
        file_entries[i].num_cev_entries = (uint32_t)entry.chars_events.size();

        for (const SevCharEvents &cevents : entry.chars_events)
        {
            file_cees->char_id = cevents.char_id;
            file_cees->costume_id = cevents.costume_id;
            file_cees->num_events = (uint32_t)cevents.events.size();

            for (const SevEvent &event : cevents.events)
            {
                file_event->type = event.type;
                file_event->num_entries = (uint32_t)event.entries.size();

                for (const SevEventEntry &ev_entry : event.entries)
                {
                    file_ev_entry->unk_00 = ev_entry.unk_00;
                    file_ev_entry->cue_id = ev_entry.cue_id;
                    file_ev_entry->file_id = ev_entry.file_id;
                    file_ev_entry->new_unk_0C = ev_entry.new_unk_0C;
                    file_ev_entry->response_cue_id = ev_entry.response_cue_id;
                    file_ev_entry->response_file_id = ev_entry.response_file_id;
                    file_ev_entry->unk_18 = ev_entry.unk_18;
                    file_ev_entry->old_unk_0C = ev_entry.old_unk_0C;
                    file_ev_entry->old_unk_10 = ev_entry.old_unk_10;

                    file_ev_entry++;
                }

                file_event++;
            }

            file_cees++;
        }
    }

    *psize = size;
    return buf;
}

TiXmlDocument *SevFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("SEV");

    for (const SevEntry &entry : entries)
    {
        entry.Decompile(root);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool SevFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "SEV");

    if (!root)
    {
        DPRINTF("Cannot find\"SEV\" in xml.\n");
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "SevEntry")
        {
            SevEntry entry;

            if (!entry.Compile(elem))
                return false;

            entries.push_back(entry);
        }
    }

    return true;
}

SevEntry *SevFile::FindEntry(uint32_t char_id, uint32_t costume_id)
{
    for (SevEntry &entry : entries)
    {
        if (entry.char_id == char_id && entry.costume_id == costume_id)
            return &entry;
    }

    return nullptr;
}

size_t SevFile::RemoveAllReferencesToChar(uint32_t char_id)
{
    size_t count = 0;

    for (size_t i = 0; i < entries.size(); i++)
    {
        const SevEntry &entry = entries[i];

        if (entry.char_id == char_id)
        {
            entries.erase(entries.begin()+i);
            i--;
            count++;
        }
    }

    for (size_t i = 0; i < entries.size(); i++)
    {
        SevEntry &entry = entries[i];

        for (size_t j = 0; j < entry.chars_events.size(); j++)
        {
            const SevCharEvents &cevents = entry.chars_events[j];

            if (cevents.char_id == char_id)
            {
                entry.chars_events.erase(entry.chars_events.begin()+j);
                j--;
                count++;
            }
        }

        // If entry became empty of chars_events, delete it.
        if (entry.chars_events.size() == 0)
        {
            entries.erase(entries.begin()+i);
            i--;
        }
    }

    return count;
}

