#include "MsgFile.h"
#include "debug.h"

TiXmlElement *MsgEntry::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("MsgEntry");
    entry_root->SetAttribute("id", Utils::UnsignedToString(id, true));
    entry_root->SetAttribute("name", name);

    for (const std::string &str : lines)
    {
        Utils::WriteParamString(entry_root, "Line", str);
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool MsgEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "id", &id))
    {
        DPRINTF("%s: failed to get id in MsgEntry.\n", FUNCNAME);
        return false;
    }

    if (root->QueryStringAttribute("name", &name) != TIXML_SUCCESS)
    {
        DPRINTF("%s: failed to get name in MsgEntry (id = 0x%x)\n", FUNCNAME, id);
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Line")
        {
            std::string str;

            if (elem->QueryStringAttribute("value", &str) != TIXML_SUCCESS)
            {
                DPRINTF("%s: Format error in a Line entry.\n", FUNCNAME);
                return false;
            }

            lines.push_back(str);
        }
    }

    return true;
}

MsgFile::MsgFile()
{
    this->big_endian = false;
    this->u16_mode = false;
}

MsgFile::~MsgFile()
{
}

void MsgFile::Reset()
{
    entries.clear();
    unicode_names = true;
    unicode_values = true;
}

void MsgFile::ReadMsgStr(const uint8_t *top, const MSGStr *msg_str, std::string &str, bool unicode)
{
    if (!unicode)
    {        
        str = GetString(top, msg_str->string_offset);

    }
    else
    {
        str = GetString16AsUtf8(top, msg_str->string_offset);
    }
}

void MsgFile::ReadMsgStr(const uint8_t *top, const MSGStr *msg_str, std::u16string &str, bool unicode)
{
    if (!unicode)
    {
        std::string str8 = GetString(top, msg_str->string_offset);
        str.reserve(str8.length());

        for (char ch : str8)
        {
            uint16_t ch16 = ch;
            str.push_back(ch16);
        }
    }
    else
    {
        str = GetString16(top, msg_str->string_offset);
    }
}

#define READ_MSG_STR(s, m, u) ReadMsgStr(buf, m, s, u)

bool MsgFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (size < sizeof(MSGHeader))
        return false;

    const MSGHeader *hdr = (const MSGHeader *)buf;

    if (hdr->signature != MSG_SIGNATURE)
        return false;

    if (hdr->type == 0x100)
        unicode_names = true;
    else if (hdr->type == 0)
        unicode_names = false;
    else
    {
        DPRINTF("%s: Unknown type 0x%04X\n", FUNCNAME, hdr->type);
    }

    unicode_values = (hdr->unicode_values != 0);

    entries.resize(hdr->num_entries);

    const MSGStr *names = (const MSGStr *)GetOffsetPtr(buf, hdr->names_section_start);
    const uint32_t *ids = (const uint32_t *)GetOffsetPtr(buf, hdr->id_section_start);
    const MSGLines *lines = (const MSGLines *)GetOffsetPtr(buf, hdr->lines_section_start);

    for (size_t i = 0; i < entries.size(); i++)
    {
        MsgEntry &entry = entries[i];

        READ_MSG_STR(entry.name, &names[i], unicode_names);
        entry.id = ids[i];        

        if (u16_mode)
        {
            entry.u16_lines.resize(lines[i].num_lines);
        }
        else
        {
            entry.lines.resize(lines[i].num_lines);
        }

        const MSGStr *msg_str = (const MSGStr *)GetOffsetPtr(buf, lines[i].top_line);

        for (uint32_t j = 0; j < lines[i].num_lines; j++)
        {
            if (u16_mode)
            {
                READ_MSG_STR(entry.u16_lines[j], msg_str, unicode_values);
            }
            else
            {
                READ_MSG_STR(entry.lines[j], msg_str, unicode_values);
            }

            msg_str++;
        }
    }

    return true;
}

size_t MsgFile::CalculateFileSize(uint32_t *str_data_offset) const
{
    size_t size = sizeof(MSGHeader);
    size += entries.size() * (sizeof(MSGStr)+sizeof(uint32_t)+sizeof(MSGLines));

    size_t num_strings = 0, strings_size = 0;

    for (const MsgEntry &entry : entries)
    {
        std::u16string temp;

        if (!unicode_names)
        {
            strings_size += entry.name.length() + 1;
        }
        else
        {
            temp = Utils::Utf8ToUcs2(entry.name);
            strings_size += (temp.length() * 2) + 2;
        }

        for (const std::string &line : entry.lines)
        {
            if (!unicode_values)
            {
                strings_size += line.length()+1;
            }
            else
            {
                temp = Utils::Utf8ToUcs2(line);
                strings_size += (temp.length() * 2) + 2;
            }

            num_strings++;
        }
    }

    size += num_strings * sizeof(MSGStr);
    *str_data_offset = (uint32_t)size;

    size += strings_size;

    return size;
}

uint8_t *MsgFile::Save(size_t *psize)
{
    if (u16_mode)
    {
        DPRINTF("%s: This function is not yet implemented for u16_mode=true.\n", FUNCNAME);
        return nullptr;
    }

    uint32_t strings_data_offset;
    size_t size = CalculateFileSize(&strings_data_offset);

    uint8_t *buf = new uint8_t[size];
    memset(buf, 0, size);

    MSGHeader *hdr = (MSGHeader *)buf;
    MSGStr *names = (MSGStr *)(hdr+1);
    uint32_t *ids = (uint32_t *)(names+entries.size());
    MSGLines *lines = (MSGLines *)(ids+entries.size());
    MSGStr *lines_str = (MSGStr *)(lines+entries.size());

    uint32_t next_string_offset = strings_data_offset;

    for (size_t i = 0; i < entries.size(); i++)
    {
        const MsgEntry &entry = entries[i];

        names[i].string_offset = next_string_offset;

        if (!unicode_names)
        {
            strcpy((char *)buf+next_string_offset, entry.name.c_str());
            names[i].num_chars = (uint32_t)entry.name.length();
            names[i].num_bytes = names[i].num_chars+1;
        }
        else
        {
            std::u16string u16str = Utils::Utf8ToUcs2(entry.name);
            names[i].num_chars = (uint32_t)u16str.length();
            names[i].num_bytes = (names[i].num_chars*2)+2;

            memcpy(buf+next_string_offset, u16str.c_str(), names[i].num_bytes);
        }

        next_string_offset += names[i].num_bytes;

        ids[i] = entry.id;
    }

    MSGStr *current_line = lines_str;
    uint32_t num_strings = 0;

    for (size_t i = 0; i < entries.size(); i++)
    {
        const MsgEntry &entry = entries[i];

        lines[i].num_lines = (uint32_t)entry.lines.size();
        lines[i].top_line = Utils::DifPointer(current_line, buf);

        for (size_t j = 0; j < entry.lines.size(); j++)
        {
            const std::string &line = entry.lines[j];
            uint32_t num_vars = 0;

            current_line->string_offset = next_string_offset;

            if (!unicode_values)
            {
                size_t pos = std::string::npos;

                do
                {
                    pos = line.find("%ls", pos+1);
                    if (pos != std::string::npos)
                        num_vars++;

                } while (pos != std::string::npos);

                current_line->num_chars = (uint32_t)line.length();
                current_line->num_bytes = current_line->num_chars+1;

                strcpy((char *)buf+next_string_offset, line.c_str());

            }
            else
            {
                std::u16string u16str = Utils::Utf8ToUcs2(line);
                size_t pos = std::string::npos;

                do
                {
                    // Fuck, ms vs2013 doesn't support u16 literals

#ifdef _MSC_VER
                    pos = u16str.find(reinterpret_cast<const char16_t *>(L"%ls"), pos+1);
#else
                    pos = u16str.find(u"%ls", pos+1);
#endif
                    if (pos != std::string::npos)
                        num_vars++;

                } while (pos != std::string::npos);

                current_line->num_chars = (uint32_t)u16str.length();
                current_line->num_bytes = (current_line->num_chars*2)+2;

                memcpy(buf+next_string_offset, u16str.c_str(), current_line->num_bytes);
            }

            current_line->num_vars = num_vars;

            next_string_offset += current_line->num_bytes;
            num_strings++;
            current_line++;
        }
    }

    hdr->signature = MSG_SIGNATURE;
    hdr->type = (unicode_names) ? 0x100 : 0;
    hdr->unicode_values = unicode_values;
    hdr->num_entries = (uint32_t)entries.size();
    hdr->names_section_start = Utils::DifPointer(names, buf);
    hdr->id_section_start = Utils::DifPointer(ids, buf);
    hdr->lines_section_start = Utils::DifPointer(lines, buf);
    hdr->num_strings = num_strings;
    hdr->strings_section_start = Utils::DifPointer(lines_str, buf);

    if (entries.size() == 0)
    {
        // Special case of empty msg (yes, they do exist)

        hdr->names_section_start = 0;
        hdr->id_section_start = 0;
        hdr->lines_section_start = 0;
        hdr->strings_section_start = 0;
    }

    *psize = size;
    return buf;
}

TiXmlDocument *MsgFile::Decompile() const
{
    if (u16_mode)
    {
        DPRINTF("%s: This function is not yet implemented for u16_mode=true.\n", FUNCNAME);
        return nullptr;
    }

    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("MSG");

    root->SetAttribute("unicode_names", unicode_names ? "true" : "false");
    root->SetAttribute("unicode_values", unicode_values ? "true" : "false");

    for (const MsgEntry &entry : entries)
    {
        entry.Decompile(root);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool MsgFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    if (u16_mode)
    {
        DPRINTF("%s: This function is not yet implemented for u16_mode=true.\n", FUNCNAME);
        return false;
    }

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "MSG");

    if (!root)
    {
        DPRINTF("Cannot find\"MSG\" in xml.\n");
        return false;
    }

    std::string str;

    if (Utils::ReadAttrString(root, "unicode_names",  str))
    {
        str = Utils::ToLowerCase(str);
        unicode_names = (str != "false" && Utils::GetUnsigned(str, -1) != 0);
    }
    else
    {
        unicode_names = true;
    }

    if (Utils::ReadAttrString(root, "unicode_values",  str))
    {
        str = Utils::ToLowerCase(str);
        unicode_values = (str != "false" && Utils::GetUnsigned(str, -1) != 0);
    }
    else
    {
        unicode_values = true;
    }

    size_t num = Utils::GetElemCount(root, "MsgEntry");
    entries.resize(num);

    if (num == 0)
        return true;

    size_t i = 0;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "MsgEntry")
        {
            MsgEntry &entry = entries[i++];

            if (!entry.Compile(elem))
                return false;
        }
    }

    return true;
}

MsgEntry *MsgFile::FindEntryByName(const std::string &name)
{
    std::string name_upper = Utils::ToUpperCase(name);

    for (MsgEntry &entry : entries)
    {
        if (Utils::ToUpperCase(entry.name) == name_upper)
            return &entry;
    }

    return nullptr;
}

MsgEntry *MsgFile::FindEntryByID(uint32_t id)
{
    for (MsgEntry &entry : entries)
    {
        if (entry.id == id)
            return &entry;
    }

    return nullptr;
}

uint32_t MsgFile::FindHighestID()
{
    int32_t ret = 0;

    for (MsgEntry &entry : entries)
    {
        int32_t entry_signed = (int32_t)entry.id;

        if (entry_signed > ret)
            ret = entry_signed;
    }

    return (uint32_t)ret;
}

bool MsgFile::AddEntry(MsgEntry &entry, bool auto_id)
{
    if (FindEntryByName(entry.name))
        return false;

    if (!auto_id)
    {
        if (FindEntryByID(entry.id))
            return false;
    }
    else
    {
        entry.id = FindHighestID()+1;

        while (FindEntryByID(entry.id))
            entry.id++;
    }

    entries.push_back(entry);
    return true;
}

bool MsgFile::AddEntryUpdate(MsgEntry &entry)
{
    MsgEntry *existing = FindEntryByName(entry.name);

    if (existing)
    {
        entry.id = existing->id;
        *existing = entry;
    }

    return AddEntry(entry, true);
}

bool MsgFile::RemoveEntry(size_t idx, bool *existed)
{
    if (idx >= entries.size())
    {
        if (existed)
            *existed = false;

        return true;
    }

    if (existed)
        *existed = true;

    entries.erase(entries.begin()+idx);
    return true;
}

bool MsgFile::RemoveEntry(const std::string &name, bool *existed)
{
    std::string name_upper = Utils::ToUpperCase(name);

    for (size_t i = 0; i < entries.size(); i++)
    {
        const MsgEntry &entry = entries[i];

        if (Utils::ToUpperCase(entry.name) == name_upper)
            return RemoveEntry(i, existed);
    }

    if (existed)
        *existed = false;

    return true;
}



