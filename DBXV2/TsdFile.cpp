#include "TsdFile.h"
#include "debug.h"

#include "Xenoverse2.h"

enum
{
    TSD_TRIGGER = 1,
    TSD_EVENT,
    TSD_GLOBAL,
    TSD_CONSTANT,
    TSD_ZONE
};

#define END_OF_SECTION  0x76543210
#define END_OF_SECTION2 0x89ABCDEF

static std::string GetCharacterName(uint32_t tnl_id)
{
    std::string ret;

    if (!game_tnl)
        return ret;

    TnlCharacter *ch = game_tnl->FindCharOrTeacherByID(tnl_id);
    if (!ch)
        return ret;


    if (ch->lobby_name.length() > 0 && game_lobby_texts.size() > XV2_LANG_ENGLISH)
    {
        MsgEntry *msg = game_lobby_texts[XV2_LANG_ENGLISH]->FindEntryByName(ch->lobby_name);

        if (msg)
        {
            ret = msg->lines[0];
        }
    }

    return ret;
}

TiXmlElement *TsdTrigger::Decompile(TiXmlNode *root, const std::string &event_name) const
{
    TiXmlElement *entry_root = new TiXmlElement("Trigger");
    entry_root->SetAttribute("id", Utils::UnsignedToString(id, true));

    Utils::WriteParamUnsigned(entry_root, "DLC", dlc, true);
    Utils::WriteParamUnsigned(entry_root, "U32_2", u32_2, true);
    Utils::WriteParamUnsigned(entry_root, "TYPE", type);

    std::string comment = GetCharacterName(npc_tnl_id);
    if (comment.length() > 0)
        Utils::WriteComment(entry_root, comment);

    Utils::WriteParamUnsigned(entry_root, "NPC_TNL_ID", npc_tnl_id, true);

    Utils::WriteParamUnsigned(entry_root, "U32_4", u32_4, true);
    Utils::WriteParamString(entry_root, "EVENT", event_name);
    Utils::WriteParamUnsigned(entry_root, "U32_5", u32_5, true);

    Utils::WriteParamString(entry_root, "CONDITION", condition);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool TsdTrigger::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "id", &id))
        return false;

    if (!Utils::GetParamUnsigned(root, "DLC", &dlc))
        return false;

    if (!Utils::GetParamUnsigned(root, "U32_2", &u32_2))
        return false;

    if (!Utils::GetParamUnsigned(root, "TYPE", &type))
        return false;

    if (type != 1 && type != 2)
    {
        DPRINTF("%s: Invalid value for type (%d)\n", FUNCNAME, type);
        return false;
    }

    if (!Utils::GetParamUnsigned(root, "NPC_TNL_ID", &npc_tnl_id))
        return false;

    if (!Utils::GetParamUnsigned(root, "U32_4", &u32_4))
        return false;

    if (!Utils::GetParamString(root, "EVENT", temp_event_name))
        return false;

    if (!Utils::GetParamUnsigned(root, "U32_5", &u32_5))
        return false;

    if (!Utils::GetParamString(root, "CONDITION", condition))
        return false;

    return true;
}

TiXmlElement *TsdEvent::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Event");
    entry_root->SetAttribute("id", Utils::UnsignedToString(id, true));
    entry_root->SetAttribute("name", name);

    Utils::WriteParamUnsigned(entry_root, "DLC", dlc, true);

    Utils::WriteParamString(entry_root, "PATH", path);
    Utils::WriteParamString(entry_root, "SCRIPT", script);
    Utils::WriteParamString(entry_root, "FUNCTION", function);    

    if (npc_tnl_ids.size() > 0)
    {
        bool write_comment = false;
        std::string comment;

        for (uint32_t id : npc_tnl_ids)
        {
            if (comment.length() != 0)
                comment += ", ";

            std::string ch_name = GetCharacterName(id);
            if (ch_name.length() > 0)
            {
                comment += ch_name;
                write_comment = true;
            }
            else
            {
                comment += "???";
            }
        }

        if (write_comment)
            Utils::WriteComment(entry_root, comment);

        Utils::WriteParamMultipleUnsigned(entry_root, "NPC_TNL_IDS", npc_tnl_ids, true);
    }

    TiXmlElement *args_root = new TiXmlElement("Arguments");
    for (size_t i = 0; i < args.size(); i++)
    {
       const std::string arg_name = "v" + Utils::ToString(i);
       Utils::WriteParamString(args_root, arg_name.c_str(), args[i]);
    }
    entry_root->LinkEndChild(args_root);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool TsdEvent::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "id", &id))
        return false;

    if (!Utils::ReadAttrString(root, "name", name))
        return false;

    if (!Utils::GetParamUnsigned(root, "DLC", &dlc))
        return false;

    if (!Utils::GetParamString(root, "PATH", path))
        return false;

    if (!Utils::GetParamString(root, "SCRIPT", script))
        return false;

    if (!Utils::GetParamString(root, "FUNCTION", function))
        return false;

    npc_tnl_ids.clear();
    Utils::ReadParamMultipleUnsigned(root, "NPC_TNL_IDS", npc_tnl_ids);

    const TiXmlElement *args_root;
    if (Utils::GetElemCount(root, "Arguments", &args_root) != 1)
    {
        DPRINTF("%s: Cannot read Arguments\n", FUNCNAME);
        return false;
    }

    args.clear();

    for (size_t i = 0; i < 0x10000; i++)
    {
        const std::string arg_name = "v" + Utils::ToString(i);
        std::string str;

        if (!Utils::ReadParamString(args_root, arg_name.c_str(), str))
            break;

        args.push_back(str);
    }

    return true;
}

TiXmlElement *TsdGlobal::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Global");
    entry_root->SetAttribute("name", name);
    entry_root->SetAttribute("type", type);
    entry_root->SetAttribute("initial_value", initial_value);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool TsdGlobal::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrString(root, "name", name))
        return false;

    if (!Utils::ReadAttrUnsigned(root, "type", &type))
        return false;

    if (!Utils::ReadAttrString(root, "initial_value", initial_value))
        return false;

    return true;
}

TiXmlElement *TsdConstant::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Constant");
    entry_root->SetAttribute("name", name);
    entry_root->SetAttribute("type", type);
    entry_root->SetAttribute("value", value);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool TsdConstant::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrString(root, "name", name))
        return false;

    if (!Utils::ReadAttrUnsigned(root, "type", &type))
        return false;

    if (!Utils::ReadAttrString(root, "value", value))
        return false;

    return true;
}

TiXmlElement *TsdZone::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Zone");

    entry_root->SetAttribute("id", Utils::UnsignedToString(id, true));
    entry_root->SetAttribute("type", type);
    entry_root->SetAttribute("name", name);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool TsdZone::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "id", &id))
        return false;

    if (!Utils::ReadAttrUnsigned(root, "type", &type))
        return false;

    if (!Utils::ReadAttrString(root, "name", name))
        return false;

    return true;
}

TsdFile::TsdFile()
{

}

TsdFile::~TsdFile()
{

}

void TsdFile::Reset()
{
    triggers.clear();
    events.clear();
    globals.clear();
    constants.clear();
    zones.clear();
}

bool TsdFile::ReadString(Stream &stream, std::string &str)
{
    str.clear();

    uint32_t len;

    if (!stream.Read32(&len))
        return false;

    if (len == 0)
        return true;

    if (len >= 0x2000)
    {
        DPRINTF("%s: Warning, string seem to be unusually big: %u (pos=0x%Ix)\n", FUNCNAME, len, (size_t)stream.Tell());
    }

    char *mem = new char[len+1];
    mem[len] = 0;

    if (!stream.Read(mem, len))
        return false;

    str = mem;
    delete[] mem;
    return true;
}

bool TsdFile::WriteString(Stream &stream, const std::string &str)
{
    if (!stream.Write32(str.length()))
        return false;

    if (str.length() == 0)
        return true;

    if (!stream.Write(str.c_str(), str.length()))
        return false;

    return true;
}

size_t TsdFile::FindEvent(uint32_t id) const
{
    for (size_t i = 0; i < events.size(); i++)
    {
        if (events[i].id == id)
            return i;
    }

    return (size_t)-1;
}

size_t TsdFile::FindEvent(const std::string &name) const
{
    for (size_t i = 0; i < events.size(); i++)
    {
        if (events[i].name == name)
            return i;
    }

    return (size_t)-1;
}

uint32_t TsdFile::FindHighestTriggerId() const
{
    uint32_t highest = 0;

    for (const TsdTrigger &trigger : triggers)
        if (trigger.id > highest)
            highest = trigger.id;

    return highest;
}

uint32_t TsdFile::FindHighestEventId() const
{
    uint32_t highest = 0;

    for (const TsdEvent &event : events)
        if (event.id > highest)
            highest = event.id;

    return highest;
}

bool TsdFile::Load(const uint8_t *buf, size_t size)
{
    Reset();    

    FixedMemoryStream mem(const_cast<uint8_t *>(buf), size);
    uint8_t type;

    while (mem.Read8(&type))
    {
        if (type == TSD_TRIGGER)
        {
            while (1)
            {
                TsdTrigger trigger;

                if (!mem.Read32(&trigger.id))
                    return true;

                if (trigger.id == END_OF_SECTION)
                    break;

                if (!mem.Read32(&trigger.dlc) || !mem.Read32(&trigger.u32_2) || !mem.Read32(&trigger.type))
                    return false;

                if (trigger.type != 1 && trigger.type != 2)
                {
                    DPRINTF("%s: Unknown trigger type 0x%x, near offset 0x%I64x\n", FUNCNAME, trigger.type, mem.Tell());
                    return false;
                }

                if (!mem.Read32(&trigger.npc_tnl_id) || !mem.Read32(&trigger.u32_4))
                    return false;

                if (!mem.Read32(&trigger.event_id) || !mem.Read32(&trigger.u32_5) || !ReadString(mem, trigger.condition))
                    return false;

                triggers.push_back(trigger);
            }
        }
        else if (type == TSD_EVENT)
        {
            while (1)
            {
                TsdEvent event;

                if (!mem.Read32(&event.id))
                    return true;

                if (event.id == END_OF_SECTION)
                    break;

                if (!mem.Read32(&event.dlc))
                    return false;

                if (!ReadString(mem, event.name) || !ReadString(mem, event.path) || !ReadString(mem, event.script))
                    return false;

                std::string args;

                if (!ReadString(mem, event.function) || !ReadString(mem, args))
                    return false;

                Utils::GetMultipleStrings(args, event.args, ",,", false);

                uint32_t length;

                if (!mem.Read32(&length))
                    return false;

                if (length > 0x1000)
                {
                    DPRINTF("%s: Array length bigger than expected, near offset 0x%I64x\n", FUNCNAME, mem.Tell());
                    return false;
                }

                event.npc_tnl_ids.resize(length);

                for (uint32_t &u : event.npc_tnl_ids)
                {
                    if (!mem.Read32(&u))
                        return false;
                }

                events.push_back(event);                
            }
        }
        else if (type == TSD_GLOBAL)
        {
            while (1)
            {
                TsdGlobal global;
                uint32_t len;

                if (!mem.Read32(&len))
                    return true;

                if (len == END_OF_SECTION)
                    break;

                mem.Seek(-4, SEEK_CUR);

                if (!ReadString(mem, global.name))
                    return false;

                if (!mem.Read32(&global.type))
                    return false;

                if (!ReadString(mem, global.initial_value))
                    return false;

                globals.push_back(global);
            }
        }
        else if (type == TSD_CONSTANT)
        {
            while (1)
            {
                TsdConstant constant;
                uint32_t len;

                if (!mem.Read32(&len))
                    return true;

                if (len == END_OF_SECTION)
                    break;

                mem.Seek(-4, SEEK_CUR);

                if (!ReadString(mem, constant.name))
                    return false;

                if (!mem.Read32(&constant.type))
                    return false;

                if (!ReadString(mem, constant.value))
                    return false;

                constants.push_back(constant);
            }
        }
        else if (type == TSD_ZONE)
        {
            while (1)
            {
                TsdZone zone;

                if (!mem.Read32(&zone.id))
                    return true;

                if (zone.id == END_OF_SECTION)
                    break;

                if (!mem.Read32(&zone.type))
                    return false;

                if (!ReadString(mem, zone.name))
                    return false;

                zones.push_back(zone);
            }
        }
        else
        {
            DPRINTF("%s: Unknown type 0x%x\n", FUNCNAME, type);
            return false;
        }

        uint32_t bos;

        if (!mem.Read32(&bos))
            return false;

        if (bos != END_OF_SECTION2)
        {
            DPRINTF("%s: Was expecting END_OF_SECTION2, got 0x%x instead.\n", FUNCNAME, bos);
            return false;
        }
    }

    return true;
}

uint8_t *TsdFile::Save(size_t *psize)
{
    MemoryStream mem;

    if (triggers.size() > 0)
    {
        mem.Write8(TSD_TRIGGER);

        for (const TsdTrigger &trigger : triggers)
        {
            if (!mem.Write32(trigger.id))
                return nullptr;

            if (!mem.Write32(trigger.dlc) || !mem.Write32(trigger.u32_2) || !mem.Write32(trigger.type))
                return nullptr;

            if (!mem.Write32(trigger.npc_tnl_id) || !mem.Write32(trigger.u32_4))
                return nullptr;

            if (!mem.Write32(trigger.event_id) || !mem.Write32(trigger.u32_5))
                return nullptr;

            if (!WriteString(mem, trigger.condition))
                return nullptr;
        }

        mem.Write32(END_OF_SECTION);
        mem.Write32(END_OF_SECTION2);
    }

    if (events.size() > 0)
    {
        mem.Write8(TSD_EVENT);

        for (const TsdEvent &event : events)
        {
            if (!mem.Write32(event.id))
                return nullptr;

            if (!mem.Write32(event.dlc))
                return nullptr;

            if (!WriteString(mem, event.name) || !WriteString(mem, event.path) || !WriteString(mem, event.script))
                return nullptr;

            std::string args = Utils::ToSingleString(event.args, ",,", false);

            if (!WriteString(mem, event.function) || !WriteString(mem, args))
                return nullptr;

            if (!mem.Write32((uint32_t)event.npc_tnl_ids.size()))
                return nullptr;

            if (event.npc_tnl_ids.size() > 0 && !mem.Write(event.npc_tnl_ids.data(), event.npc_tnl_ids.size()*4))
                return nullptr;
        }

        mem.Write32(END_OF_SECTION);
        mem.Write32(END_OF_SECTION2);
    }

    if (globals.size() > 0)
    {
        mem.Write8(TSD_GLOBAL);

        for (const TsdGlobal &global : globals)
        {
            if (!WriteString(mem, global.name))
                return nullptr;

            if (!mem.Write32(global.type))
                return nullptr;

            if (!WriteString(mem, global.initial_value))
                return nullptr;
        }

        mem.Write32(END_OF_SECTION);
        mem.Write32(END_OF_SECTION2);
    }

    if (constants.size() > 0)
    {
        mem.Write8(TSD_CONSTANT);

        for (const TsdConstant &constant : constants)
        {
            if (!WriteString(mem, constant.name))
                return nullptr;

            if (!mem.Write32(constant.type))
                return nullptr;

            if (!WriteString(mem, constant.value))
                return nullptr;
        }

        mem.Write32(END_OF_SECTION);
        mem.Write32(END_OF_SECTION2);
    }

    if (zones.size() > 0)
    {
        mem.Write8(TSD_ZONE);

        for (const TsdZone &zone : zones)
        {
            if (!mem.Write32(zone.id))
                return nullptr;

            if (!mem.Write32(zone.type))
                return nullptr;

            if (!WriteString(mem, zone.name))
                return nullptr;
        }
    }

    *psize = (size_t) mem.GetSize();
    return mem.GetMemory(true);
}

TiXmlDocument *TsdFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("TsdFile");

    for (const TsdTrigger &trigger : triggers)
    {
        size_t pos = FindEvent(trigger.event_id);

        if (pos == (size_t)-1)
        {
            DPRINTF("%s: Cannot find event with id 0x%x\n", FUNCNAME, trigger.event_id);
            return nullptr;
        }

        trigger.Decompile(root, events[pos].name);
    }

    for (const TsdEvent &event : events)
    {
        event.Decompile(root);
    }

    for (const TsdGlobal &global : globals)
    {
        global.Decompile(root);
    }

    for (const TsdConstant &constant : constants)
    {
        constant.Decompile(root);
    }

    for (const TsdZone &zone : zones)
    {
        zone.Decompile(root);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool TsdFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "TsdFile");

    if (!root)
    {
        DPRINTF("Cannot find\"TsdFile\" in xml.\n");
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Trigger")
        {
            TsdTrigger trigger;

            if (!trigger.Compile(elem))
                return false;

            triggers.push_back(trigger);
        }
        else if (elem->ValueStr() == "Event")
        {
            TsdEvent event;

            if (!event.Compile(elem))
                return false;

            events.push_back(event);
        }
        else if (elem->ValueStr() == "Global")
        {
            TsdGlobal global;

            if (!global.Compile(elem))
                return false;

            globals.push_back(global);
        }
        else if (elem->ValueStr() == "Constant")
        {
            TsdConstant constant;

            if (!constant.Compile(elem))
                return false;

            constants.push_back(constant);
        }
        else if (elem->ValueStr() == "Zone")
        {
            TsdZone zone;

            if (!zone.Compile(elem))
                return false;

            zones.push_back(zone);
        }
    }

    for (TsdTrigger &trigger : triggers)
    {
        size_t pos = FindEvent(trigger.temp_event_name);

        if (pos == (size_t)-1)
        {
            DPRINTF("%s: Cannot find event \"%s\" (at trigger with id 0x%x)\n", FUNCNAME, trigger.temp_event_name.c_str(), trigger.id);
            return false;
        }

        trigger.event_id = events[pos].id;
    }

    return true;
}

size_t TsdFile::FindTriggersWithEvent(const std::string &event_name, std::vector<TsdTrigger *> &triggers)
{
    triggers.clear();

    TsdEvent *event = FindEventByName(event_name);
    if (!event)
        return 0;

    for (TsdTrigger &trigger : this->triggers)
        if (trigger.event_id == event->id)
            triggers.push_back(&trigger);

    return triggers.size();
}

void TsdFile::AddTrigger(TsdTrigger &trigger)
{
    trigger.id = FindHighestTriggerId()+1;
    triggers.push_back(trigger);
}

TsdEvent *TsdFile::FindEventByName(const std::string &name)
{
    for (TsdEvent &event : events)
        if (event.name == name)
            return &event;

    return nullptr;
}

void TsdFile::AddEvent(TsdEvent &event)
{
    TsdEvent *existing = FindEventByName(event.name);

    if (existing)
    {
        event.id = existing->id;
        *existing = event;
    }

    else
    {
        event.id = FindHighestEventId()+1;
        events.push_back(event);
    }
}



