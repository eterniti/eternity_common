#include "TnlFile.h"
#include "FixedMemoryStream.h"
#include "debug.h"

#include "Xenoverse2.h"

enum
{
    TNL_CHARACTER = 1,
    TNL_TEACHER,
    TNL_ACTION,
    TNL_OBJECT
};

#define END_OF_SECTION  0x76543210
#define END_OF_SECTION2 0x89ABCDEF

TiXmlElement *TnlCharacter::Decompile(TiXmlNode *root, const char *entry_name) const
{
    TiXmlElement *entry_root = new TiXmlElement(entry_name);
    entry_root->SetAttribute("id", Utils::UnsignedToString(id, true));

    if (lobby_name.length() > 0 && game_lobby_texts.size() > XV2_LANG_ENGLISH)
    {
        MsgEntry *msg = game_lobby_texts[XV2_LANG_ENGLISH]->FindEntryByName(lobby_name);

        if (msg)
        {
            Utils::WriteComment(entry_root, msg->lines[0]);
        }
    }

    Utils::WriteParamUnsigned(entry_root, "U8_1", u8_1, true);
    Utils::WriteParamUnsigned(entry_root, "U8_2", u8_2, true);
    Utils::WriteParamUnsigned(entry_root, "U8_3", u8_3, true);
    Utils::WriteParamString(entry_root, "CMS", cms);
    Utils::WriteParamUnsigned(entry_root, "COSTUME_ID", costume_id, (costume_id==0xFFFF));

    Utils::WriteComment(entry_root, "If CMS is \"PRISET\" (preset), cid is the char id in lobby_preset_avatar_list.pal");
    Utils::WriteParamUnsigned(entry_root, "CID", cid, true);

    Utils::WriteComment(entry_root, "For \"PRISET\" chars, the name info in the .pal has precedence over the lobby_name here.");
    Utils::WriteParamString(entry_root, "LOBBY_NAME", lobby_name);

    Utils::WriteParamUnsigned(entry_root, "U32_1", u32_1, true);
    Utils::WriteParamString(entry_root, "POSITION", position);
    Utils::WriteParamString(entry_root, "ZONE", zone);
    Utils::WriteParamUnsigned(entry_root, "ACTION", action, true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

TiXmlElement *TnlCharacter::Decompile(TiXmlNode *root) const
{
    return Decompile(root, "Character");
}

bool TnlCharacter::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "id", &id))
    {
        DPRINTF("%s: Attribute \"id\" is not optional.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadParamUnsigned(root, "U8_1", &u8_1)) return false;
    if (!Utils::ReadParamUnsigned(root, "U8_2", &u8_2)) return false;
    if (!Utils::ReadParamUnsigned(root, "U8_3", &u8_3)) return false;
    if (!Utils::ReadParamString(root, "CMS", cms)) return false;
    if (!Utils::ReadParamUnsigned(root, "COSTUME_ID", &costume_id)) return false;
    if (!Utils::ReadParamUnsigned(root, "CID", &cid)) return false;
    if (!Utils::ReadParamString(root, "LOBBY_NAME", lobby_name)) return false;
    if (!Utils::ReadParamUnsigned(root, "U32_1", &u32_1)) return false;
    if (!Utils::ReadParamString(root, "POSITION", position)) return false;
    if (!Utils::ReadParamString(root, "ZONE", zone)) return false;
    if (!Utils::ReadParamUnsigned(root, "ACTION", &action)) return false;

    return true;
}

TiXmlElement *TnlTeacher::Decompile(TiXmlNode *root) const
{
    return TnlCharacter::Decompile(root, "Teacher");
}

bool TnlTeacher::Compile(const TiXmlElement *root)
{
    return TnlCharacter::Compile(root);
}

TiXmlElement *TnlObject::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Object");
    entry_root->SetAttribute("id", Utils::UnsignedToString(id, true));

    Utils::WriteParamString(entry_root, "LOBBY_NAME", lobby_name);
    Utils::WriteParamUnsigned(entry_root, "U32_1", u32_1, true);
    Utils::WriteParamString(entry_root, "POSITION1", position1);
    Utils::WriteParamString(entry_root, "POSITION2", position2);
    Utils::WriteParamString(entry_root, "UNK_STRING", unk_string);
    Utils::WriteParamUnsigned(entry_root, "U32_2", u32_2, true);
    Utils::WriteParamUnsigned(entry_root, "U32_3", u32_3, true);
    Utils::WriteParamUnsigned(entry_root, "U32_4", u32_4, true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool TnlObject::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "id", &id))
    {
        DPRINTF("%s: Attribute \"id\" is not optional.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadParamString(root, "LOBBY_NAME", lobby_name)) return false;
    if (!Utils::ReadParamUnsigned(root, "U32_1", &u32_1)) return false;
    if (!Utils::ReadParamString(root, "POSITION1", position1)) return false;
    if (!Utils::ReadParamString(root, "POSITION2", position2)) return false;
    if (!Utils::ReadParamString(root, "UNK_STRING", unk_string)) return false;
    if (!Utils::ReadParamUnsigned(root, "U32_2", &u32_2)) return false;
    if (!Utils::ReadParamUnsigned(root, "U32_3", &u32_3)) return false;
    if (!Utils::ReadParamUnsigned(root, "U32_4", &u32_4)) return false;

    return true;
}


TiXmlElement *TnlAction::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Action");
    entry_root->SetAttribute("id", Utils::UnsignedToString(id, true));

    Utils::WriteParamString(entry_root, "PATH", path);
    Utils::WriteParamString(entry_root, "SCRIPT", script);
    Utils::WriteParamString(entry_root, "FUNCTION", function);

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

bool TnlAction::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "id", &id))
    {
        DPRINTF("%s: Attribute \"id\" is not optional.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadParamString(root, "PATH", path)) return false;
    if (!Utils::ReadParamString(root, "SCRIPT", script)) return false;
    if (!Utils::ReadParamString(root, "FUNCTION", function)) return false;

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

TnlFile::TnlFile()
{
    this->big_endian = false;
}

TnlFile::~TnlFile()
{

}

bool TnlFile::ReadString(Stream &stream, std::string &str)
{
    str.clear();

    uint32_t len;

    if (!stream.Read32(&len))
        return false;

    if (len == 0)
        return true;

    if (len >= 0x200)
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

bool TnlFile::WriteString(Stream &stream, const std::string &str)
{
    if (!stream.Write32((uint32_t)str.length()))
        return false;

    if (str.length() == 0)
        return true;

    if (!stream.Write(str.c_str(), str.length()))
        return false;

    return true;
}

void TnlFile::Reset()
{
    chars.clear();
    teachers.clear();
    objects.clear();
    actions.clear();
}

bool TnlFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf)
        return false;

    FixedMemoryStream mem(const_cast<uint8_t *>(buf), size);
    uint8_t type;

    while (mem.Read8(&type))
    {
        if (type == TNL_CHARACTER)
        {
            while (1)
            {
                TnlCharacter ch;

                if (!mem.Read32(&ch.id))
                    return false;

                if (ch.id == END_OF_SECTION)
                    break;

                if (!mem.Read8(&ch.u8_1) || !mem.Read8(&ch.u8_2) || !mem.Read8(&ch.u8_3))
                    return false;

                if (!ReadString(mem, ch.cms))
                    return false;

                if (!mem.Read16(&ch.costume_id) || !mem.Read16(&ch.cid))
                    return false;

                if (!ReadString(mem, ch.lobby_name))
                    return false;

                if (!mem.Read32(&ch.u32_1))
                    return false;

                if (!ReadString(mem, ch.position) || !ReadString(mem, ch.zone))
                    return false;

                if (!mem.Read32(&ch.action))
                    return false;

                chars.push_back(ch);
            }
        }
        else if (type == TNL_TEACHER)
        {
            while (1)
            {
                TnlTeacher teacher;

                if (!mem.Read32(&teacher.id))
                    return false;

                if (teacher.id == END_OF_SECTION)
                    break;

                if (!mem.Read8(&teacher.u8_1) || !mem.Read8(&teacher.u8_2) || !mem.Read8(&teacher.u8_3))
                    return false;

                if (!ReadString(mem, teacher.cms))
                    return false;

                if (!mem.Read16(&teacher.costume_id) || !mem.Read16(&teacher.cid))
                    return false;

                if (!ReadString(mem, teacher.lobby_name))
                    return false;

                if (!mem.Read32(&teacher.u32_1))
                    return false;

                if (!ReadString(mem, teacher.position) || !ReadString(mem, teacher.zone))
                    return false;

                if (!mem.Read32(&teacher.action))
                    return false;

                teachers.push_back(teacher);
            }
        }
        else if (type == TNL_OBJECT)
        {
            while (1)
            {
                TnlObject object;

                if (!mem.Read32(&object.id))
                    return false;

                if (object.id == END_OF_SECTION)
                    break;

                if (!ReadString(mem, object.lobby_name))
                    return false;

                if (!mem.Read32(&object.u32_1))
                    return false;

                if (!ReadString(mem, object.position1) || !ReadString(mem, object.position2) || !ReadString(mem, object.unk_string))
                    return false;

                if (!mem.Read32(&object.u32_2) || !mem.Read32(&object.u32_3) || !mem.Read32(&object.u32_4))
                    return false;

                /*if (object.u32_4 != 0)
                {
                    DPRINTF("Warning, u32_4 not zero near 0x%Ix\n", (size_t)mem.Tell());
                }*/

                objects.push_back(object);
            }
        }
        else if (type == TNL_ACTION)
        {
            while (1)
            {
                TnlAction action;

                if (!mem.Read32(&action.id))
                    return false;

                if (action.id == END_OF_SECTION)
                    break;

                if (!ReadString(mem, action.path) || !ReadString(mem, action.script))
                    return false;

                std::string args;

                if (!ReadString(mem, action.function) || !ReadString(mem, args))
                    break;

                Utils::GetMultipleStrings(args, action.args, ',', false);

                actions.push_back(action);
            }
        }
        else
        {
            DPRINTF("%s: Unknown type 0x%x\n", FUNCNAME, type);
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

uint8_t *TnlFile::Save(size_t *psize)
{
    MemoryStream mem;

    if (chars.size() > 0)
    {
        mem.Write8(TNL_CHARACTER);

        for (const TnlCharacter &ch : chars)
        {
            if (!mem.Write32(ch.id))
                return nullptr;

            if (!mem.Write8(ch.u8_1) || !mem.Write8(ch.u8_2) || !mem.Write8(ch.u8_3))
                return nullptr;

            if (!WriteString(mem, ch.cms))
                return nullptr;

            if (!mem.Write16(ch.costume_id) || !mem.Write16(ch.cid))
                return nullptr;

            if (!WriteString(mem, ch.lobby_name))
                return nullptr;

            if (!mem.Write32(ch.u32_1))
                return nullptr;

            if (!WriteString(mem, ch.position) || !WriteString(mem, ch.zone))
                return nullptr;

            if (!mem.Write32(ch.action))
                return nullptr;
        }

        mem.Write32(END_OF_SECTION);
        mem.Write32(END_OF_SECTION2);
    }

    if (teachers.size() > 0)
    {
        mem.Write8(TNL_TEACHER);

        for (const TnlTeacher &teacher : teachers)
        {
            if (!mem.Write32(teacher.id))
                return nullptr;

            if (!mem.Write8(teacher.u8_1) || !mem.Write8(teacher.u8_2) || !mem.Write8(teacher.u8_3))
                return nullptr;

            if (!WriteString(mem, teacher.cms))
                return nullptr;

            if (!mem.Write16(teacher.costume_id) || !mem.Write16(teacher.cid))
                return nullptr;

            if (!WriteString(mem, teacher.lobby_name))
                return nullptr;

            if (!mem.Write32(teacher.u32_1))
                return nullptr;

            if (!WriteString(mem, teacher.position) || !WriteString(mem, teacher.zone))
                return nullptr;

            if (!mem.Write32(teacher.action))
                return nullptr;
        }

        mem.Write32(END_OF_SECTION);
        mem.Write32(END_OF_SECTION2);
    }

    if (objects.size() > 0)
    {
        mem.Write8(TNL_OBJECT);

        for (const TnlObject &object : objects)
        {
            if (!mem.Write32(object.id))
                return nullptr;

            if (!WriteString(mem, object.lobby_name))
                return nullptr;

            if (!mem.Write32(object.u32_1))
                return nullptr;

            if (!WriteString(mem, object.position1) || !WriteString(mem, object.position2) || !WriteString(mem, object.unk_string))
                return nullptr;

            if (!mem.Write32(object.u32_2) || !mem.Write32(object.u32_3) || !mem.Write32(object.u32_4))
                return nullptr;
        }

        mem.Write32(END_OF_SECTION);
        mem.Write32(END_OF_SECTION2);
    }

    if (actions.size() > 0)
    {
        mem.Write8(TNL_ACTION);

        for (const TnlAction &action : actions)
        {
            if (!mem.Write32(action.id))
                return nullptr;

            if (!WriteString(mem, action.path) || !WriteString(mem, action.script))
                return nullptr;

            std::string args = Utils::ToSingleString(action.args, ",", false);

            if (!WriteString(mem, action.function) || !WriteString(mem, args))
                break;
        }

        mem.Write32(END_OF_SECTION);
        mem.Write32(END_OF_SECTION2);
    }

    *psize = (size_t) mem.GetSize();
    return mem.GetMemory(true);
}

TiXmlDocument *TnlFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("TnlFile");

    for (const TnlCharacter &ch :chars)
    {
        ch.Decompile(root);
    }

    for (const TnlTeacher &teacher : teachers)
    {
        teacher.Decompile(root);
    }

    for (const TnlObject &object : objects)
    {
        object.Decompile(root);
    }

    for (const TnlAction &action :actions)
    {
        action.Decompile(root);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool TnlFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "TnlFile");

    if (!root)
    {
        DPRINTF("Cannot find\"TnlFile\" in xml.\n");
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Character")
        {
            TnlCharacter ch;

            if (!ch.Compile(elem))
                return false;

            chars.push_back(ch);
        }
        else if (elem->ValueStr() == "Teacher")
        {
            TnlTeacher teacher;

            if (!teacher.Compile(elem))
                return false;

            teachers.push_back(teacher);
        }
        else if (elem->ValueStr() == "Object")
        {
            TnlObject object;

            if (!object.Compile(elem))
                return false;

            objects.push_back(object);
        }
        else if (elem->ValueStr() == "Action")
        {
            TnlAction action;

            if (!action.Compile(elem))
                return false;

            actions.push_back(action);
        }
    }

    return true;
}

TnlObject *TnlFile::FindObjectById(uint32_t id)
{
    for (TnlObject &object : objects)
    {
        if (object.id == id)
            return &object;
    }

    return nullptr;
}

bool TnlFile::AddObjectWithId(const TnlObject &object, bool overwrite)
{
    TnlObject *existing = FindObjectById(object.id);

    if (existing)
    {
        if (!overwrite)
            return false;

        *existing = object;
    }
    else
    {
        //DPRINTF("Added new.\n");
        objects.push_back(object);
    }

    return true;
}

