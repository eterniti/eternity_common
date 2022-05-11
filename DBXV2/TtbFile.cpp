#include <algorithm>

#include "TtbFile.h"
#include "Xenoverse2.h"
#include "debug.h"

static bool GetCharaName(uint32_t cms, uint32_t costume, std::string &name)
{
    if (!game_cms)
        return false;

    CmsEntryXV2 *entry = dynamic_cast<CmsEntryXV2 *>(game_cms->FindEntryByID(cms));
    if (!entry)
        return false;

    if (costume == 0xFFFFFFFF)
    {
        return Xenoverse2::GetCharaName(entry->name, name);
    }

    name = Xenoverse2::GetCharaAndCostumeName(cms, costume, 0, true);
    return (name.length() > 0);
}

static bool GetSubtitle(const std::string &event, std::string &subs)
{
    if (!game_ttb)
        return false;

    return Xenoverse2::GetTtbSubtitle(event, subs);
}

TiXmlElement *TtbEvent::Decompile(TiXmlNode *root, uint32_t cms_parent, bool subtitles_comment) const
{
    TiXmlElement *entry_root = new TiXmlElement("TtbEvent");

    std::string comment;
    if (cms_parent != 0xFFFFFFFF && GetCharaName(cms_parent, costume, comment))
    {
        comment = "char1: " + comment;
        Utils::WriteComment(entry_root, comment);
    }

    if (GetCharaName(cms2_id, costume2, comment))
    {
        comment = "char2: " + comment;
        Utils::WriteComment(entry_root, comment);
    }

    if (cms3_id != 0xFFFFFFFF && GetCharaName(cms3_id, costume3, comment))
    {
        comment = "char3: " + comment;
        Utils::WriteComment(entry_root, comment);
    }

    if (cms4_id != 0xFFFFFFFF && GetCharaName(cms4_id, costume4, comment))
    {
        comment = "char4: " + comment;
        Utils::WriteComment(entry_root, comment);
    }

    if (cms5_id != 0xFFFFFFFF && GetCharaName(cms5_id, costume5, comment))
    {
        comment = "char5: " + comment;
        Utils::WriteComment(entry_root, comment);
    }

    entry_root->SetAttribute("id", id);
    entry_root->SetAttribute("costume", Utils::UnsignedToString(costume, costume==0xFFFFFFFF));
    entry_root->SetAttribute("transformation", Utils::UnsignedToString(transformation, transformation==0xFFFFFFFF));

    entry_root->SetAttribute("cms_ch2", Utils::UnsignedToString(cms2_id, true));
    entry_root->SetAttribute("ch2_costume", Utils::UnsignedToString(costume2, costume2==0xFFFFFFFF));
    entry_root->SetAttribute("ch2_transformation", Utils::UnsignedToString(transformation2, transformation2==0xFFFFFFFF));

    if (cms3_id != 0xFFFFFFFF)
    {
        entry_root->SetAttribute("cms_ch3", Utils::UnsignedToString(cms3_id, true));
        entry_root->SetAttribute("ch3_costume", Utils::UnsignedToString(costume3, costume3==0xFFFFFFFF));
        entry_root->SetAttribute("ch3_transformation", Utils::UnsignedToString(transformation3, transformation3==0xFFFFFFFF));
    }

    if (cms4_id != 0xFFFFFFFF)
    {
        entry_root->SetAttribute("cms_ch4", Utils::UnsignedToString(cms4_id, true));
        entry_root->SetAttribute("ch4_costume", Utils::UnsignedToString(costume4, costume4==0xFFFFFFFF));
        entry_root->SetAttribute("ch4_transformation", Utils::UnsignedToString(transformation4, transformation4==0xFFFFFFFF));
    }

    if (cms5_id != 0xFFFFFFFF)
    {
        entry_root->SetAttribute("cms_ch5", Utils::UnsignedToString(cms5_id, true));
        entry_root->SetAttribute("ch5_costume", Utils::UnsignedToString(costume5, costume5==0xFFFFFFFF));
        entry_root->SetAttribute("ch5_transformation", Utils::UnsignedToString(transformation5, transformation5==0xFFFFFFFF));
    }

    std::string type_str;

    if (type == 1)
    {
        type_str = "ally_talk";
    }
    else if (type == 2)
    {
        type_str = "death";
    }
    else
    {
        type_str = "versus";
    }

    Utils::WriteParamString(entry_root, "TYPE", type_str);
    Utils::WriteParamUnsigned(entry_root, "U_44", unk_44, true);
    Utils::WriteParamUnsigned(entry_root, "U_48", unk_48, true);

    Utils::WriteParamBoolean(entry_root, "REVERSE_ORDER", reverse_order);
    Utils::WriteParamBoolean(entry_root, "DUMMY_ORDER1", dummy_order1);
    Utils::WriteParamBoolean(entry_root, "DUMMY_ORDER2", dummy_order2);

    std::string subs;

    if (subtitles_comment && GetSubtitle(voice_name, subs))
        Utils::WriteComment(entry_root, subs);

    Utils::WriteParamString(entry_root, "VOICE_NAME", voice_name);

    if (subtitles_comment && GetSubtitle(voice2_name, subs))
        Utils::WriteComment(entry_root, subs);

    Utils::WriteParamString(entry_root, "VOICE2_NAME", voice2_name);

    if (voice3_name.length() > 0)
    {
        if (subtitles_comment && GetSubtitle(voice3_name, subs))
            Utils::WriteComment(entry_root, subs);

        Utils::WriteParamString(entry_root, "VOICE3_NAME", voice3_name);
    }

    if (voice4_name.length() > 0)
    {
        if (subtitles_comment && GetSubtitle(voice4_name, subs))
            Utils::WriteComment(entry_root, subs);

        Utils::WriteParamString(entry_root, "VOICE4_NAME", voice4_name);
    }

    if (voice5_name.length() > 0)
    {
        if (subtitles_comment && GetSubtitle(voice5_name, subs))
            Utils::WriteComment(entry_root, subs);

        Utils::WriteParamString(entry_root, "VOICE5_NAME", voice5_name);
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool TtbEvent::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "id", &id))
        return false;

    if (!Utils::ReadAttrUnsigned(root, "costume", &costume))
        return false;

    if (!Utils::ReadAttrUnsigned(root, "transformation", &transformation))
        return false;

    if (!Utils::ReadAttrUnsigned(root, "cms_ch2", &cms2_id))
        return false;

    if (!Utils::ReadAttrUnsigned(root, "ch2_costume", &costume2))
        return false;

    if (!Utils::ReadAttrUnsigned(root, "ch2_transformation", &transformation2))
        return false;

    if (Utils::ReadAttrUnsigned(root, "cms_ch3", &cms3_id))
    {
        if (!Utils::ReadAttrUnsigned(root, "ch3_costume", &costume3))
            return false;

        if (!Utils::ReadAttrUnsigned(root, "ch3_transformation", &transformation3))
            return false;
    }
    else
    {
        cms3_id = 0xFFFFFFFF;
        costume3 = 0xFFFFFFFF;
        transformation3 = 0xFFFFFFFF;
    }

    if (Utils::ReadAttrUnsigned(root, "cms_ch4", &cms4_id))
    {
        if (!Utils::ReadAttrUnsigned(root, "ch4_costume", &costume4))
            return false;

        if (!Utils::ReadAttrUnsigned(root, "ch4_transformation", &transformation4))
            return false;
    }
    else
    {
        cms4_id = 0xFFFFFFFF;
        costume4 = 0xFFFFFFFF;
        transformation4 = 0xFFFFFFFF;
    }

    if (Utils::ReadAttrUnsigned(root, "cms_ch5", &cms5_id))
    {
        if (!Utils::ReadAttrUnsigned(root, "ch5_costume", &costume5))
            return false;

        if (!Utils::ReadAttrUnsigned(root, "ch5_transformation", &transformation5))
            return false;
    }
    else
    {
        cms5_id = 0xFFFFFFFF;
        costume5 = 0xFFFFFFFF;
        transformation5 = 0xFFFFFFFF;
    }

    std::string type_str;
    if (!Utils::GetParamString(root, "TYPE", type_str)) return false;

    if (type_str == "versus")
        type = 0;
    else if (type_str == "ally_talk")
        type = 1;
    else if (type_str == "death")
        type = 2;
    else
    {
        DPRINTF("%s: Unrecognized type \"%s\"\n", FUNCNAME, type_str.c_str());
        return false;
    }

    if (!Utils::GetParamUnsigned(root, "U_44", &unk_44)) return false;
    if (!Utils::GetParamUnsigned(root, "U_48", &unk_48)) return false;

    if (!Utils::ReadParamBoolean(root, "REVERSE_ORDER", &reverse_order)) return false;

    if (!Utils::ReadParamBoolean(root, "DUMMY_ORDER1", &dummy_order1))
        dummy_order1 = false;

    if (!Utils::ReadParamBoolean(root, "DUMMY_ORDER2", &dummy_order2))
        dummy_order2 = false;

    if (!Utils::GetParamString(root, "VOICE_NAME", voice_name))  return false;
    if (!Utils::GetParamString(root, "VOICE2_NAME", voice2_name)) return false;

    if (!Utils::ReadParamString(root, "VOICE3_NAME", voice3_name))
        voice3_name.clear();

    if (!Utils::ReadParamString(root, "VOICE4_NAME", voice4_name))
        voice4_name.clear();

    if (!Utils::ReadParamString(root, "VOICE5_NAME", voice5_name))
        voice5_name.clear();

    return true;
}

TiXmlElement *TtbEntry::Decompile(TiXmlNode *root, bool subtitles_comment) const
{
    TiXmlElement *entry_root = new TiXmlElement("TtbEntry");

    std::string comment;
    if (GetCharaName(cms_id, 0xFFFFFFFF, comment))
    {
        Utils::WriteComment(entry_root, comment);
    }

    entry_root->SetAttribute("cms", Utils::UnsignedToString(cms_id, true));

    for (const TtbEvent &event : events)
        event.Decompile(entry_root, cms_id, subtitles_comment);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool TtbEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "cms", &cms_id))
        return false;

    events.clear();

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "TtbEvent")
        {
           TtbEvent event;

           if (!event.Compile(elem))
           {
                DPRINTF("%s: Failed compilation of event %Id\n", FUNCNAME, events.size());
                return false;
           }

           events.push_back(event);
        }
    }

    return true;
}

bool TtbEntry::AddUniqueEvent(const TtbEvent &event)
{
    for (const TtbEvent &existing_event : events)
    {
        if (event.Compare(existing_event, true))
            return false;
    }

    events.push_back(event);
    return true;
}

TtbFile::TtbFile()
{
   this->big_endian = false;
   this->subtitles_comments = false;
}

TtbFile::~TtbFile()
{

}

void TtbFile::Reset()
{
    entries.clear();
}

bool TtbFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(TTBHeader))
        return false;

    const TTBHeader *hdr = (const TTBHeader *)buf;
    if (hdr->signature != TTB_SIGNATURE)
        return false;

    const char *strings = (const char *)(hdr+1);
    const TTBHeader2 *hdr2 = (const TTBHeader2 *)GetOffsetPtr(strings, hdr->h2_start);

    entries.resize(hdr2->num_lists);

    const TTBEventList *file_list = (const TTBEventList *)GetOffsetPtr(hdr2, hdr2->data_start);

    for (size_t i = 0; i < entries.size(); i++)
    {
        TtbEntry &list = entries[i];

        list.events.resize(file_list->num_events);

        const TTBEvent *file_event = (const TTBEvent *)GetOffsetPtr(hdr2, file_list->data_start);
        file_event += file_list->start_index;

        for (size_t j = 0; j < list.events.size(); j++)
        {
            TtbEvent &event = list.events[j];

            if (j == 0)
            {
                list.cms_id = file_event->cms_id;
            }
            else
            {
                if (list.cms_id != file_event->cms_id)
                {
                    DPRINTF("%s: Unexpected cms: 0x%x != expected 0x%x\n", FUNCNAME, file_event->cms_id, list.cms_id);
                    return false;
                }
            }

            if (file_event->type > 2)
            {
                DPRINTF("%s: Unexpected value (0x%x) for type.\n", FUNCNAME, file_event->type);
                return false;
            }

            event.id = file_event->id;
            event.costume = file_event->costume;
            event.transformation = file_event->transformation;
            event.cms2_id = file_event->cms2_id;
            event.costume2 = file_event->costume2;
            event.transformation2 = file_event->transformation2;
            event.cms3_id = file_event->cms3_id;
            event.costume3 = file_event->costume3;
            event.transformation3 = file_event->transformation3;
            event.cms4_id = file_event->cms4_id;
            event.costume4 = file_event->costume4;
            event.transformation4 = file_event->transformation4;
            event.cms5_id = file_event->cms5_id;
            event.costume5 = file_event->costume5;
            event.transformation5 = file_event->transformation5;
            event.type = file_event->type;
            event.unk_44 = file_event->unk_44;
            event.unk_48 = file_event->unk_48;

            if (file_event->unk_4C == 0 && file_event->unk_54 == 1)
            {
                event.reverse_order = false;
            }
            else if (file_event->unk_4C == 1 && file_event->unk_54 == 0)
            {
                event.reverse_order = true;
                //DPRINTF("Reverse order!\n");
            }
            else if (file_event->unk_4C == 0xFFFFFFFF && file_event->unk_54 == 0xFFFFFFFF)
            {
                event.dummy_order1 = true;
            }
            else
            {
                DPRINTF("%s: Incompatible order: 0x%x 0x%x, in entry:event %Id:%Id.\n", FUNCNAME, file_event->unk_4C, file_event->unk_54, i, j);
                return false;
            }

            if (file_event->unk_5C == 0xFFFFFFFF && file_event->unk_64 == 0xFFFFFFFF && file_event->unk_6C == 0xFFFFFFFF)
            {
                event.dummy_order2 = true;
            }
            else if (file_event->unk_5C != 2 || file_event->unk_64 != 3 || file_event->unk_6C != 4)
            {
                DPRINTF("%s: unexpected values in 5C/64/6C: 0x%x 0x%x 0x%x, in entry:event %Id:%Id\n", FUNCNAME, file_event->unk_5C, file_event->unk_64, file_event->unk_6C, i, j);
                return false;
            }

            if (file_event->voice_name != 0xFFFFFFFF)
                event.voice_name = strings + file_event->voice_name;

            if (file_event->voice2_name != 0xFFFFFFFF)
                event.voice2_name = strings + file_event->voice2_name;

            if (file_event->voice3_name != 0xFFFFFFFF)
                event.voice3_name = strings + file_event->voice3_name;

            if (file_event->voice4_name != 0xFFFFFFFF)
                event.voice4_name = strings + file_event->voice4_name;

            if (file_event->voice5_name != 0xFFFFFFFF)
                event.voice5_name = strings + file_event->voice5_name;

            file_event++;
        }

        file_list++;
    }

    return true;
}

std::set<TtbEvent> TtbFile::GetOrderedEvents() const
{
    std::set<TtbEvent> events;

    for (const TtbEntry &list : entries)
    {
        for (const TtbEvent &event : list.events)
        {
            events.insert(event);
        }
    }

    return events;
}

size_t TtbFile::RemoveAllReferencesToChar(uint32_t char_id)
{
    if (char_id == 0xFFFFFFFF)
        return 0;

    size_t count = 0;

    for (size_t i = 0; i < entries.size(); i++)
    {
        TtbEntry &entry = entries[i];

        if (entry.cms_id == char_id)
        {
            entries.erase(entries.begin()+i);
            i--;
            count++;
            continue;
        }

        for (size_t j = 0; j < entry.events.size(); j++)
        {
            TtbEvent &event = entry.events[j];

            if (event.cms2_id == char_id || event.cms3_id == char_id || event.cms4_id == char_id || event.cms5_id == char_id)
            {
                entry.events.erase(entry.events.begin()+j);
                j--;
                count++;
            }
        }
    }

    return count;
}

TtbEntry *TtbFile::FindEntry(uint32_t char_id)
{
    for (TtbEntry &entry : entries)
    {
        if (entry.cms_id == char_id)
            return &entry;
    }

    return nullptr;
}

size_t TtbFile::BuildStrings(std::unordered_map<std::string, size_t> &map, std::vector<std::string> &strings) const
{
    size_t size = 0;    

    std::set<TtbEvent> events = GetOrderedEvents();

    for (const TtbEvent &event : events)
    {       
        if (event.voice_name.length() > 0 && map.find(event.voice_name) == map.end())
        {
            strings.push_back(event.voice_name);
            map[event.voice_name] = size;
            size += event.voice_name.length() + 1;
        }

        if (event.voice2_name.length() > 0 && map.find(event.voice2_name) == map.end())
        {
            strings.push_back(event.voice2_name);
            map[event.voice2_name] = size;
            size += event.voice2_name.length() + 1;
        }

        if (event.voice3_name.length() > 0 && map.find(event.voice3_name) == map.end())
        {
            strings.push_back(event.voice3_name);
            map[event.voice3_name] = size;
            size += event.voice3_name.length() + 1;
        }

        if (event.voice4_name.length() > 0 && map.find(event.voice4_name) == map.end())
        {
            strings.push_back(event.voice4_name);
            map[event.voice4_name] = size;
            size += event.voice4_name.length() + 1;
        }

        if (event.voice5_name.length() > 0 && map.find(event.voice5_name) == map.end())
        {
            strings.push_back(event.voice5_name);
            map[event.voice5_name] = size;
            size += event.voice5_name.length() + 1;
        }
    }

    return size;
}

void TtbFile::FixEventsID(uint32_t from_id)
{
    std::map<uint32_t, TtbEvent *> events_map;
    uint32_t highest_id = 0;

    for (TtbEntry &entry : entries)
    {
        for (TtbEvent &event : entry.events)
        {
            if (event.id >= from_id)
            {
                events_map[event.id] = &event;
            }
            else
            {
                if (event.id > highest_id)
                    highest_id = event.id;
            }
        }
    }

    int n = highest_id+1;

    for (auto &it : events_map)
    {
        it.second->id = n++;
    }
}

size_t TtbFile::GetNumEvents() const
{
    size_t n = 0;

    for (const TtbEntry &list : entries)
    {
        n += list.events.size();
    }

    return n;
}

void TtbFile::Preprocess()
{
    // First, order entries by cms
    std::sort(entries.begin(), entries.end());

    // Now, create necessary empty entries on gaps
    for (size_t i = 0; i < entries.size(); i++)
    {
        if (entries[i].cms_id == 0xFFFFFFFF)
            continue;

        if (entries[i].cms_id > i)
        {
            size_t n = (size_t) (entries[i].cms_id - i);
            TtbEntry dummy_entry;

            //DPRINTF("Inserting %d dummy entries at %Id.\n", n, i);
            entries.insert(entries.begin()+i, n, dummy_entry);
        }
        else if (entries[i].cms_id < i)
        {
            DPRINTF("%s: BUG, cms id shouldn't be below i here. (0x%x < 0x%Ix)\n", FUNCNAME, entries[i].cms_id, i);
            exit(-1);
        }
    }

    // Finally, remove the dummy entries at the end
    while (entries.back().cms_id == 0xFFFFFFFF)
    {
        //DPRINTF("Removing dummy.\n");
        entries.pop_back();
    }
}

uint8_t *TtbFile::Save(size_t *psize)
{
    Preprocess();

    std::unordered_map<std::string, size_t> strings_map;
    std::vector<std::string> strings;
    size_t strings_size = BuildStrings(strings_map, strings);

    *psize = sizeof(TTBHeader) + strings_size + sizeof(TTBHeader2) + entries.size()*sizeof(TTBEventList) + GetNumEvents()*sizeof(TTBEvent);
    uint8_t *buf = new uint8_t[*psize];
    memset(buf, 0, *psize);

    TTBHeader *hdr = (TTBHeader *)buf;
    hdr->signature = TTB_SIGNATURE;
    hdr->endianess_check = 0xFFFE;
    hdr->unk_06 = 0x40;
    hdr->unk_08 = 1;
    hdr->h2_start = (uint32_t) strings_size;

    char *ptr = (char *)(hdr + 1);

    for (const std::string &str : strings)
    {
        strcpy(ptr, str.c_str());
        ptr += str.length() + 1;
    }

    TTBHeader2 *hdr2 = (TTBHeader2 *)GetOffsetPtr(hdr, hdr->h2_start + sizeof(TTBHeader));

    hdr2->num_lists = (uint32_t)entries.size();
    hdr2->data_start = sizeof(TTBHeader2);

    TTBEventList *file_list = (TTBEventList *)GetOffsetPtr(hdr2, hdr2->data_start);
    TTBEvent *file_event = (TTBEvent *)GetOffsetPtr(file_list, (uint32_t)entries.size()*sizeof(TTBEventList));
    uint32_t current_index = 0;

    for (size_t i = 0; i < entries.size(); i++)
    {
        const TtbEntry &list = entries[i];

        file_list->num_events = (uint32_t)list.events.size();
        file_list->data_start = sizeof(TTBHeader2) + (uint32_t)entries.size()*sizeof(TTBEventList);
        file_list->start_index = current_index;
        file_list->list_id = (uint32_t)i;

        for (size_t j = 0; j < list.events.size(); j++)
        {
            const TtbEvent &event = list.events[j];

            file_event->id = event.id;
            file_event->cms_id = list.cms_id;
            file_event->costume = event.costume;
            file_event->transformation = event.transformation;
            file_event->cms2_id = event.cms2_id;
            file_event->costume2 = event.costume2;
            file_event->transformation2 = event.transformation2;
            file_event->cms3_id = event.cms3_id;
            file_event->costume3 = event.costume3;
            file_event->transformation3 = event.transformation3;
            file_event->cms4_id = event.cms4_id;
            file_event->costume4 = event.costume4;
            file_event->transformation4 = event.transformation4;
            file_event->cms5_id = event.cms5_id;
            file_event->costume5 = event.costume5;
            file_event->transformation5 = event.transformation5;
            file_event->type = event.type;
            file_event->unk_44 = event.unk_44;
            file_event->unk_48 = event.unk_48;

            if (event.reverse_order)
            {
                file_event->unk_4C = 1;
                file_event->unk_54 = 0;
            }
            else if (event.dummy_order1)
            {
                file_event->unk_4C = file_event->unk_54 = 0xFFFFFFFF;
            }

            else
            {
                file_event->unk_4C = 0;
                file_event->unk_54 = 1;
            }

            if (event.dummy_order2)
            {
                file_event->unk_5C = file_event->unk_64 = file_event->unk_6C = 0xFFFFFFFF;
            }
            else
            {
                file_event->unk_5C = 2;
                file_event->unk_64 = 3;
                file_event->unk_6C = 4;
            }

            file_event->voice_name = (event.voice_name.length() == 0) ? 0xFFFFFFFF : (uint32_t)strings_map[event.voice_name];
            file_event->voice2_name = (event.voice2_name.length() == 0) ? 0xFFFFFFFF : (uint32_t)strings_map[event.voice2_name];
            file_event->voice3_name = (event.voice3_name.length() == 0) ? 0xFFFFFFFF : (uint32_t)strings_map[event.voice3_name];
            file_event->voice4_name = (event.voice4_name.length() == 0) ? 0xFFFFFFFF : (uint32_t)strings_map[event.voice4_name];
            file_event->voice5_name = (event.voice5_name.length() == 0) ? 0xFFFFFFFF : (uint32_t)strings_map[event.voice5_name];

            file_event++;
        }

        current_index += file_list->num_events;
        file_list++;
    }

    return buf;
}

TiXmlDocument *TtbFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("TTB");

    for (const TtbEntry &list : entries)
    {
        list.Decompile(root, subtitles_comments);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool TtbFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "TTB");

    if (!root)
    {
        DPRINTF("Cannot find\"TTB\" in xml.\n");
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "TtbEntry")
        {
            TtbEntry list;

            if (!list.Compile(elem))
                return false;

            entries.push_back(list);
        }
    }

    return true;
}


