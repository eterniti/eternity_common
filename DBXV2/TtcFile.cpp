#include "TtcFile.h"
#include "Xenoverse2.h"
#include "debug.h"

static bool GetCharaName(uint32_t cms, std::string &name)
{
    if (!game_cms)
        return false;

    CmsEntry *entry = game_cms->FindEntryByID(cms);
    if (!entry)
        return false;

    return Xenoverse2::GetCharaName(entry->name, name);
}

static bool GetCostumeName(uint32_t cms, uint32_t costume, std::string &name)
{
    if (costume == 0xFFFFFFFF)
    {
        name = "Any costume";
        return true;
    }

    if (!game_cms)
        return false;

    CmsEntry *entry = game_cms->FindEntryByID(cms);
    if (!entry)
        return false;

    return Xenoverse2::GetCharaCostumeName(entry->name, costume, 0, name);
}

TiXmlElement *TtcEvent::Decompile(TiXmlNode *root, uint32_t cms_id, bool subtitles_comment) const
{
    TiXmlElement *entry_root = new TiXmlElement("TtcEvent");

    if (cms_id != 0xFFFFFFFF)
    {
        std::string name;

        if (GetCostumeName(cms_id, costume, name))
        {
            Utils::WriteComment(entry_root, name);
        }
    }

    std::string condition_str;

    if (condition == TTC_TO_HUMAN_CAC_CONDITION)
    {
        condition_str = "to_human_cac";
    }
    else if (condition == TTC_TO_SAIYAN_CAC_CONDITION)
    {
        condition_str = "to_saiyan_cac";
    }
    else if (condition == TTC_TO_NAMEKIAN_CAC_CONDITION)
    {
        condition_str = "to_namekian_cac";
    }
    else if (condition == TTC_TO_FREEZER_CAC_CONDITION)
    {
        condition_str = "to_freezer_cac";
    }
    else if (condition == TTC_TO_MAJIN_CAC_CONDITION)
    {
        condition_str = "to_majin_cac";
    }
    else if (condition == TTC_TEACHER_CONDITION)
    {
        condition_str = "teacher";
    }
    else
    {
        condition_str = "default";
    }

    entry_root->SetAttribute("condition", condition_str);

    Utils::WriteParamUnsigned(entry_root, "COSTUME", costume, (costume==0xFFFFFFFF));
    Utils::WriteParamUnsigned(entry_root, "TRANSFORMATION", transformation, (transformation==0xFFFFFFFF));

    if (subtitles_comment && xv2fs)
    {
        CmsEntry *cms = game_cms->FindEntryByID(cms_id);
        if (cms)
        {
            static MsgFile subs;
            static std::string last_loaded;
            bool msg_found = false;

            if (last_loaded == cms->name)
            {
                msg_found = true;
            }

            else
            {
                const std::string path = "data/msg/qc_" + cms->name + "_en.msg";
                last_loaded.clear();

                if (xv2fs->LoadFile(&subs, path))
                {
                    last_loaded = cms->name;
                    msg_found = true;
                }
            }

            if (msg_found)
            {
                MsgEntry *msg_entry = subs.FindEntryByName(name);
                if (msg_entry)
                {
                    Utils::WriteComment(entry_root, msg_entry->lines[0]);
                }
            }
        }
    }

    Utils::WriteParamString(entry_root, "NAME", name);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool TtcEvent::Compile(const TiXmlElement *root)
{
    std::string condition_str;

    if (Utils::ReadAttrString(root, "condition", condition_str))
    {
        condition_str = Utils::ToLowerCase(condition_str);

        if (condition_str == "default")
        {
            condition = TTC_DEFAULT_CONDITION;
        }
        else if (condition_str == "to_human_cac")
        {
            condition = TTC_TO_HUMAN_CAC_CONDITION;
        }
        else if (condition_str == "to_saiyan_cac")
        {
            condition = TTC_TO_SAIYAN_CAC_CONDITION;
        }
        else if (condition_str == "to_namekian_cac")
        {
            condition = TTC_TO_NAMEKIAN_CAC_CONDITION;
        }
        else if (condition_str == "to_freezer_cac")
        {
            condition = TTC_TO_FREEZER_CAC_CONDITION;
        }
        else if (condition_str == "to_majin_cac")
        {
            condition = TTC_TO_MAJIN_CAC_CONDITION;
        }
        else if (condition_str == "teacher")
        {
            condition = TTC_TEACHER_CONDITION;
        }
        else
        {
            DPRINTF("%s: unknown condition \"%s\"\n", FUNCNAME, condition_str.c_str());
            return false;
        }
    }
    else
    {
        condition = TTC_DEFAULT_CONDITION;
    }

    if (!Utils::GetParamUnsigned(root, "COSTUME", &costume)) return false;
    if (!Utils::GetParamUnsigned(root, "TRANSFORMATION", &transformation)) return false;
    if (!Utils::GetParamString(root, "NAME", name)) return false;

    return true;
}

bool TtcEventList::IsKnownType(uint32_t type)
{
    if (type == 3 || type == 4 || type == 5 || type == 9 || type == 0x11 || type == 0x14)
        return false;

    if (type >= 0x16)
        return false;

    return true;
}

bool TtcEventList::EventExists(const std::string &name) const
{
    for (const TtcEvent &event : events)
        if (event.name == name)
            return true;

    return false;
}

TiXmlElement *TtcEventList::Decompile(TiXmlNode *root, uint32_t cms_id, bool subtitles_comment) const
{
    TiXmlElement *entry_root = new TiXmlElement("TtcEventList");
    std::string type_str;

    if (type == TTC_MINOR_DAMAGE_LIST)
    {
        type_str = "minor_damage";
    }
    else if (type == TTC_MAJOR_DAMAGE_LIST)
    {
        type_str = "major_damage";
    }
    else if (type == TTC_PLAYER_KO_ENEMY_LIST)
    {
        type_str = "player_ko_enemy";
    }
    else if (type == TTC_STRONG_ATTACK_DAMAGED_LIST)
    {
        type_str = "strong_attack_damaged";
    }
    else if (type == TTC_POWER_UP_LIST)
    {
        type_str = "power_up";
    }
    else if (type == TTC_START_TALK_LIST)
    {
        type_str = "start_talk";
    }
    else if (type == TTC_PLAYER_DAMAGED_LIST)
    {
        type_str = "player_damaged";
    }
    else if (type == TTC_LITTLE_TIME_LIST)
    {
        type_str = "little_time";
    }
    else if (type == TTC_PLAYER_ALLY_KILLED_ENEMY_LIST)
    {
        type_str = "player_ally_killed_enemy";
    }
    else if (type == TTC_CHALLENGE_LIST)
    {
        type_str = "challenge";
    }
    else if (type == TTC_KO_LIST)
    {
        type_str = "ko";
    }
    else if (type == TTC_ENTERING_LIST)
    {
        type_str = "entering";
    }
    else if (type == TTC_MASTER_VERSUS_LIST)
    {
        type_str = "master_versus";
    }
    else if (type == TTC_PLAYER_KO_ALLY_LIST)
    {
        type_str = "player_ko_ally";
    }
    else if (type == TTC_FIGHT_SERIOUSLY_LIST)
    {
        type_str = "fight_seriously";
    }
    else
    {
        type_str = "versus";
    }

    entry_root->SetAttribute("type", type_str);

    for (const TtcEvent &event : events)
        event.Decompile(entry_root, cms_id, subtitles_comment);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool TtcEventList::Compile(const TiXmlElement *root)
{
    std::string type_str;

    if (!Utils::ReadAttrString(root, "type", type_str))
    {
        DPRINTF("%s: Attribute \"type\" is not optional.\n", FUNCNAME);
        return false;
    }

    if (type_str == "versus")
    {
        type = TTC_VERSUS_LIST;
    }
    else if (type_str == "minor_damage" || type_str == "minor_damage_by_player_team")
    {
        type = TTC_MINOR_DAMAGE_LIST;
    }
    else if (type_str == "major_damage" || type_str == "major_damage_by_player_team")
    {
        type = TTC_MAJOR_DAMAGE_LIST;
    }
    else if (type_str == "player_ko_enemy")
    {
        type = TTC_PLAYER_KO_ENEMY_LIST;
    }
    else if (type_str == "strong_attack_damaged" || type_str == "damaged")
    {
        type = TTC_STRONG_ATTACK_DAMAGED_LIST;
    }
    else if (type_str == "power_up")
    {
        type = TTC_POWER_UP_LIST;
    }
    else if (type_str == "start_talk")
    {
        type = TTC_START_TALK_LIST;
    }
    else if (type_str == "player_damaged")
    {
        type = TTC_PLAYER_DAMAGED_LIST;
    }
    else if (type_str == "little_time")
    {
        type = TTC_LITTLE_TIME_LIST;
    }
    else if (type_str == "player_ally_killed_enemy")
    {
        type = TTC_PLAYER_ALLY_KILLED_ENEMY_LIST;
    }
    else if (type_str == "challenge")
    {
        type = TTC_CHALLENGE_LIST;
    }
    else if (type_str == "ko")
    {
        type = TTC_KO_LIST;
    }
    else if (type_str == "entering")
    {
        type = TTC_ENTERING_LIST;
    }
    else if (type_str == "master_versus")
    {
        type = TTC_MASTER_VERSUS_LIST;
    }
    else if (type_str == "player_ko_ally")
    {
        type = TTC_PLAYER_KO_ALLY_LIST;
    }
    else if (type_str == "fight_seriously")
    {
        type = TTC_FIGHT_SERIOUSLY_LIST;
    }
    else
    {
        DPRINTF("%s: Unrecognized type \"%s\"\n", FUNCNAME, type_str.c_str());
        return false;
    }

    events.clear();

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "TtcEvent")
        {
           TtcEvent event;

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

bool TtcEntry::EventExists(const std::string &name) const
{
    for (const TtcEventList &list : lists)
        if (list.EventExists(name))
            return true;

    return false;
}

TiXmlElement *TtcEntry::Decompile(TiXmlNode *root, bool subtitles_comment) const
{
    TiXmlElement *entry_root = new TiXmlElement("TtcEntry");
    entry_root->SetAttribute("cms_id", Utils::UnsignedToString(cms_id, true));

    if (cms_id != 0xFFFFFFFF)
    {
        std::string name;

        if (GetCharaName(cms_id, name))
        {
            Utils::WriteComment(entry_root, name);
        }
    }

    for (const TtcEventList &list : lists)
        list.Decompile(entry_root, cms_id, subtitles_comment);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool TtcEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "cms_id", &cms_id))
    {
        DPRINTF("%s: attribute \"cms_id\" is not optional.\n", FUNCNAME);
        return false;
    }

    lists.clear();

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "TtcEventList")
        {
           TtcEventList list;

           if (!list.Compile(elem))
           {
                DPRINTF("%s: Failed compilation of list %Id\n", FUNCNAME, lists.size());
                return false;
           }

           lists.push_back(list);
        }
    }

    return true;
}

TtcFile::TtcFile()
{
    this->big_endian = false;
    this->subtitles_comments = false;
}

TtcFile::~TtcFile()
{

}



void TtcFile::Reset()
{
    entries.clear();
}

bool TtcFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(TTCHeader))
        return false;

    const TTCHeader *hdr = (const TTCHeader *)buf;
    if (hdr->signature != TTC_SIGNATURE)
        return false;

    const char *strings = (const char *)(hdr+1);
    entries.resize(hdr->num_entries);

    const TTCEntry *file_entry = (const TTCEntry *) GetOffsetPtr(strings, hdr->strings_size);
    const uint8_t *top_section2 = (const uint8_t *)file_entry;

    for (size_t i = 0; i < entries.size(); i++)
    {
        TtcEntry &entry = entries[i];

        entry.cms_id = file_entry->cms_id;
        entry.lists.resize(file_entry->num_lists);

        const TTCEventList *file_list = (const TTCEventList *) GetOffsetPtr(top_section2, file_entry->data_start + file_entry->start_index*sizeof(TTCEventList));

        for (size_t j = 0; j < entry.lists.size(); j++)
        {
            TtcEventList &list = entry.lists[j];

            if (!TtcEventList::IsKnownType(file_list->type))
            {
                DPRINTF("%s: Unknown list type 0x%x\n", FUNCNAME, file_list->type);
                return false;
            }

            list.type = file_list->type;
            list.events.resize(file_list->num_events);

            const TTCEvent *file_event = (const TTCEvent *) GetOffsetPtr(top_section2, file_list->data_start + file_list->start_index*sizeof(TTCEvent));

            for (size_t k = 0; k < list.events.size(); k++)
            {
                TtcEvent &event = list.events[k];

                if (file_event->cms_id != entry.cms_id)
                {
                    DPRINTF("%s: cms_id mismatch (0x%x != 0x%x)\n", FUNCNAME, file_event->cms_id, entry.cms_id);
                    return false;
                }

                if (file_event->list_type != list.type)
                {
                    DPRINTF("%s: list type mistmatch (0x%x != 0x%x)\n", FUNCNAME, file_event->list_type, list.type);
                    return false;
                }

                if (file_event->condition >= TTC_EVENT_CONDITION_MAX)
                {
                    DPRINTF("%s: unrecognized event condition %d\n", FUNCNAME, file_event->condition);
                    return false;
                }

                if (file_event->unk_18 != 0xFFFFFFFF)
                {
                    DPRINTF("%s: Warning, unk_18 is %d and not -1 (entry cms 0x%x, list type 0x%x, event index %Id\n", FUNCNAME, file_event->unk_18, entry.cms_id, list.type, k);
                }

                if (file_event->unk_1C != 0xFFFFFFFF)
                {
                    DPRINTF("%s: Warning, unk_1C is %d and not -1 (entry cms 0x%x, list type 0x%x, event index %Id\n", FUNCNAME, file_event->unk_1C, entry.cms_id, list.type, k);
                }

                if (file_event->unk_20 != 0xFFFFFFFF)
                {
                    DPRINTF("%s: Warning, unk_20 is %d and not -1 (entry cms 0x%x, list type 0x%x, event index %Id\n", FUNCNAME, file_event->unk_20, entry.cms_id, list.type, k);
                }

                if (file_event->unk_24 != 0xFFFFFFFF)
                {
                    DPRINTF("%s: Warning, unk_24 is %d and not -1 (entry cms 0x%x, list type 0x%x, event index %Id\n", FUNCNAME, file_event->unk_24, entry.cms_id, list.type, k);
                }

                event.costume = file_event->costume;
                event.transformation = file_event->transformation;
                event.condition = file_event->condition;
                event.name = (const char *)GetOffsetPtr(strings, file_event->name);

                file_event++;
            }

            file_list++;
        }

        file_entry++;
    }

    return true;
}

TiXmlDocument *TtcFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("TTC");

    for (const TtcEntry &entry : entries)
    {
        entry.Decompile(root, subtitles_comments);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool TtcFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "TTC");

    if (!root)
    {
        DPRINTF("Cannot find\"TTC\" in xml.\n");
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "TtcEntry")
        {
            TtcEntry entry;

            if (!entry.Compile(elem))
                return false;

            entries.push_back(entry);
        }
    }

    return true;
}

void TtcFile::AddString(std::unordered_map<std::string, size_t> &map, std::vector<std::string> &strings, const std::string &str, size_t &size) const
{
    if (map.find(str) == map.end())
    {
        strings.push_back(str);
        map[str] = size;
        size += str.length() + 1;
    }
}

size_t TtcFile::BuildStrings(std::unordered_map<std::string, size_t> &map, std::vector<std::string> &strings) const
{
    size_t size = 0;
    const std::vector<uint32_t> types = { 0, 1, 2, 6, 7, 8, 0xe, 0xa, 0xb, 0x13, 0xc, 0xd, 0xf, 0x10, 0x12 };
    const std::vector<uint32_t> types2 = { 0, 1, 2, 0x13, 7, 0x15, 0xa, 0xb, 6, 8, 0xc, 0xd, 0xe, 0xf, 0x10, 0x12 };

    for (const TtcEntry &entry : entries)
    {
        for (int caq = 0; caq <= 1; caq++)
        {
            for (int costume = -1; costume <= 20; costume++)
            {
                for (int transformation = -1; transformation <= 6; transformation++)
                {
                    const std::vector<uint32_t> *types_vec;

                    if (entry.cms_id == 0x1B )
                    {
                        types_vec = &types2;
                    }
                    else
                    {
                        types_vec = &types;
                    }

                    for (uint32_t type : *types_vec)
                    {
                        for (const TtcEventList &list : entry.lists)
                        {
                            if (list.type != type)
                                continue;

                            for (const TtcEvent &event : list.events)
                            {
                                bool doit;

                                if ((entry.cms_id == 0x43 || entry.cms_id == 0x55 || entry.cms_id == 0x7a || entry.cms_id == 0x79 ||
                                     entry.cms_id == 0x57 || entry.cms_id == 0x7b || entry.cms_id == 0x53 || entry.cms_id == 0x56 ||
                                     entry.cms_id == 0x7e || entry.cms_id == 0x8a || entry.cms_id == 0x8c) && type >= 0xa)
                                {
                                    doit = (int)event.condition == costume && (int)event.transformation == transformation;
                                }
                                else if (entry.cms_id == 0x54)
                                {
                                    if (type >= 0xa)
                                        doit = (int)event.condition == transformation;
                                    else
                                        doit = (int)event.transformation == transformation;
                                }
                                else
                                {
                                    doit = (int)event.costume == costume && (int)event.transformation == transformation;
                                }

                                if (doit)
                                {
                                    bool has_caq = Utils::BeginsWith(event.name, "CAQ_");

                                    if (caq == 0)
                                    {
                                        if (has_caq)
                                            continue;
                                    }
                                    else
                                    {
                                        if (!has_caq)
                                            continue;
                                    }

                                    if (event.name == "TRS-SS1_DML")
                                    {
                                        if (entry.EventExists("TRS-SS1_PKO_0"))
                                            AddString(map, strings, "TRS-SS1_PKO_0", size);
                                    }

                                    AddString(map, strings, event.name, size);
                                }
                            }
                        }
                    }
                }
            }

            // Now type 0x15
            for (int costume = -1; costume <= 10; costume++)
            {
                for (int transformation = -1; transformation <= 4; transformation++)
                {
                    for (const TtcEventList &list : entry.lists)
                    {
                        if (list.type != 0x15)
                            continue;

                        for (const TtcEvent &event : list.events)
                        {
                            if ((int)event.costume == costume && (int)event.transformation == transformation)
                            {
                                AddString(map, strings, event.name, size);
                            }
                        }
                    }
                }
            }
        } // for caq


        // Any remaining entry now
        for (const TtcEventList &list : entry.lists)
        {
            for (const TtcEvent &event : list.events)
            {
                AddString(map, strings, event.name, size);
            }
        }
    }

    return size;
}

uint8_t *TtcFile::Save(size_t *psize)
{
    std::unordered_map<std::string, size_t> strings_map;
    std::vector<std::string> strings;
    size_t strings_size = BuildStrings(strings_map, strings);

    size_t num_lists = GetNumEventsLists();

    *psize = sizeof(TTCHeader) + strings_size + entries.size()*sizeof(TTCEntry) + num_lists*sizeof(TTCEventList) + GetNumEvents()*sizeof(TTCEvent);
    uint8_t *buf = new uint8_t[*psize];
    memset(buf, 0, *psize);

    TTCHeader *hdr = (TTCHeader *)buf;
    hdr->signature = TTC_SIGNATURE;
    hdr->endianess_check = 0xFFFE;
    hdr->unk_06 = 0x18;
    hdr->num_entries = (uint32_t)entries.size();
    hdr->strings_size= (uint32_t) strings_size;

    char *ptr = (char *)(hdr+1);

    for (const std::string &str : strings)
    {
        strcpy(ptr, str.c_str());
        ptr += str.length() + 1;
    }

    TTCEntry *file_entry = (TTCEntry *)ptr;
    uint8_t *top = (uint8_t *)file_entry;
    TTCEventList *file_list = (TTCEventList *)GetOffsetPtr(file_entry, (uint32_t)entries.size()*sizeof(TTCEntry));
    uint32_t current_list_index = 0;
    uint32_t current_event_index = 0;

    for (size_t i = 0; i < entries.size(); i++)
    {
        const TtcEntry &entry = entries[i];

        file_entry->num_lists = (uint32_t)entry.lists.size();
        file_entry->data_start = (uint32_t)entries.size()*sizeof(TTCEntry);
        file_entry->start_index = current_list_index;
        file_entry->cms_id = entry.cms_id;

        for (size_t j = 0; j < entry.lists.size(); j++)
        {
            const TtcEventList &list = entry.lists[j];

            file_list->num_events = (uint32_t)list.events.size();
            file_list->data_start = (uint32_t)entries.size()*sizeof(TTCEntry) + (uint32_t)num_lists*sizeof(TTCEventList);
            file_list->start_index = current_event_index;
            file_list->type = list.type;

            TTCEvent *file_event = (TTCEvent *)(top + file_list->data_start);
            file_event += file_list->start_index;

            for (size_t k = 0; k < list.events.size(); k++)
            {
                const TtcEvent &event = list.events[k];

                file_event->cms_id = entry.cms_id;
                file_event->costume = event.costume;
                file_event->transformation = event.transformation;
                file_event->list_type = list.type;
                file_event->condition = event.condition;
                file_event->name = (uint32_t)strings_map[event.name];
                file_event->unk_18 = file_event->unk_1C = file_event->unk_20 = file_event->unk_24 = 0xFFFFFFFF;

                file_event++;
            }

            current_event_index += (uint32_t)list.events.size();
            file_list++;
        }

        current_list_index += (uint32_t)entry.lists.size();
        file_entry++;
    }

    return buf;
}

size_t TtcFile::GetNumEventsLists() const
{
    size_t num = 0;

    for (const TtcEntry &entry : entries)
        num += entry.lists.size();

    return num;
}

size_t TtcFile::GetNumEvents() const
{
    size_t num = 0;

    for (const TtcEntry &entry : entries)
        for (const TtcEventList &list : entry.lists)
            num += list.events.size();

    return num;
}

TtcEntry *TtcFile::FindEntry(uint32_t cms_id)
{
    for (TtcEntry &entry : entries)
    {
        if (entry.cms_id == cms_id)
            return &entry;
    }

    return nullptr;
}

void TtcFile::AddEntry(const TtcEntry &entry)
{
    TtcEntry *existing = FindEntry(entry.cms_id);

    if (existing)
    {
        *existing = entry;
        return;
    }

    entries.push_back(entry);
}

size_t TtcFile::RemoveChar(uint32_t cms_id)
{
    size_t count = 0;

    for (size_t i = 0; i < entries.size(); i++)
    {
        if (entries[i].cms_id == cms_id)
        {
            entries.erase(entries.begin()+i);
            i--;
            count++;
        }
    }

    return count;
}


