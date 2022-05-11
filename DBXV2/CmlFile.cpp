#include "CmlFile.h"
#include "Xenoverse2.h"
#include "debug.h"

TiXmlElement *CmlEntry::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("CmlEntry");

    entry_root->SetAttribute("char_id", Utils::UnsignedToString(char_id, true));
    entry_root->SetAttribute("costume_id", Utils::UnsignedToString(costume_id, true));

    std::string name = Xenoverse2::GetCharaAndCostumeName(char_id, costume_id, 0, true);
    if (name.length() != 0)
        Utils::WriteComment(entry_root, name);

    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);

    Utils::WriteParamMultipleFloats(entry_root, "CSS_POS", std::vector<float>(css_pos, css_pos+3));
    Utils::WriteParamMultipleFloats(entry_root, "CSS_ROT", std::vector<float>(css_rot, css_rot+2));

    Utils::WriteParamFloat(entry_root, "F_0C", unk_0C);
    Utils::WriteParamFloat(entry_root, "F_10", unk_10);
    Utils::WriteParamFloat(entry_root, "F_14", unk_14);
    Utils::WriteParamFloat(entry_root, "F_18", unk_18);
    Utils::WriteParamFloat(entry_root, "F_1C", unk_1C);
    Utils::WriteParamFloat(entry_root, "F_20", unk_20);
    Utils::WriteParamFloat(entry_root, "F_24", unk_24);
    Utils::WriteParamFloat(entry_root, "F_28", unk_28);
    Utils::WriteParamFloat(entry_root, "F_2C", unk_2C);
    Utils::WriteParamFloat(entry_root, "F_30", unk_30);
    Utils::WriteParamFloat(entry_root, "F_34", unk_34);
    Utils::WriteParamFloat(entry_root, "F_38", unk_38);
    Utils::WriteParamFloat(entry_root, "F_3C", unk_3C);
    Utils::WriteParamFloat(entry_root, "F_40", unk_40);
    Utils::WriteParamFloat(entry_root, "F_44", unk_44);
    Utils::WriteParamFloat(entry_root, "F_48", unk_48);
    Utils::WriteParamFloat(entry_root, "F_5C", unk_5C);
    Utils::WriteParamFloat(entry_root, "F_60", unk_60);
    Utils::WriteParamFloat(entry_root, "F_64", unk_64);
    Utils::WriteParamFloat(entry_root, "F_68", unk_68);
    Utils::WriteParamFloat(entry_root, "NF_10", neo_unk_10);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool CmlEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "char_id", &char_id))
    {
        DPRINTF("%s: char_id attribute is not optional.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "costume_id", &costume_id))
    {
        DPRINTF("%s: costume_id attribute is not optional.\n", FUNCNAME);
        return false;
    }

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamMultipleFloats(root, "CSS_POS", css_pos, 3))
        return false;

    if (!Utils::GetParamMultipleFloats(root, "CSS_ROT", css_rot, 2))
        return false;

    if (!Utils::GetParamFloat(root, "F_0C", &unk_0C))
        return false;

    if (!Utils::GetParamFloat(root, "F_10", &unk_10))
        return false;

    if (!Utils::GetParamFloat(root, "F_14", &unk_14))
        return false;

    if (!Utils::GetParamFloat(root, "F_18", &unk_18))
        return false;

    if (!Utils::GetParamFloat(root, "F_1C", &unk_1C))
        return false;

    if (!Utils::GetParamFloat(root, "F_20", &unk_20))
        return false;

    if (!Utils::GetParamFloat(root, "F_24", &unk_24))
        return false;

    if (!Utils::GetParamFloat(root, "F_28", &unk_28))
        return false;

    if (!Utils::GetParamFloat(root, "F_2C", &unk_2C))
        return false;

    if (!Utils::GetParamFloat(root, "F_30", &unk_30))
        return false;

    if (!Utils::GetParamFloat(root, "F_34", &unk_34))
        return false;

    if (!Utils::GetParamFloat(root, "F_38", &unk_38))
        return false;

    if (!Utils::GetParamFloat(root, "F_3C", &unk_3C))
        return false;

    if (!Utils::GetParamFloat(root, "F_40", &unk_40))
        return false;

    if (!Utils::GetParamFloat(root, "F_44", &unk_44))
        return false;

    if (!Utils::GetParamFloat(root, "F_48", &unk_48))
        return false;

    if (!Utils::GetParamFloat(root, "F_5C", &unk_5C))
        return false;

    if (!Utils::GetParamFloat(root, "F_60", &unk_60))
        return false;

    if (!Utils::GetParamFloat(root, "F_64", &unk_64))
        return false;

    if (!Utils::GetParamFloat(root, "F_68", &unk_68))
        return false;

    if (!Utils::ReadParamFloat(root, "NF_10", &neo_unk_10))
    {
        neo_unk_10 = 0.0f;
    }

    return true;
}

CmlFile::CmlFile()
{
    this->big_endian = false;
}

CmlFile::~CmlFile()
{

}

void CmlFile::Reset()
{
    entries.clear();
}

bool CmlFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(uint32_t))
        return false;

    uint32_t num_entries = *(uint32_t *)&buf[0];

    if (size < (sizeof(uint32_t) + num_entries*sizeof(CMLEntry)))
        return false;

    const CMLEntry *file_entries = (const CMLEntry *)(buf+sizeof(uint32_t));
    entries.resize(num_entries);

    for (uint32_t i = 0; i < num_entries; i++)
    {
        CmlEntry &entry = entries[i];

        entry.char_id = file_entries[i].char_id;
        entry.costume_id = file_entries[i].costume_id;
        entry.unk_04 = file_entries[i].unk_04;

        entry.css_pos[0] = file_entries[i].css_pos_x;
        entry.css_pos[1] = file_entries[i].css_pos_y;
        entry.css_pos[2] = file_entries[i].css_pos_z;

        entry.css_rot[0] = file_entries[i].css_rot_x;
        entry.css_rot[1] = file_entries[i].css_rot_y;

        entry.unk_0C = file_entries[i].unk_0C;

        entry.neo_unk_10 = file_entries[i].neo_unk_10;

        entry.unk_10 = file_entries[i].unk_10;
        entry.unk_14 = file_entries[i].unk_14;
        entry.unk_18 = file_entries[i].unk_18;
        entry.unk_1C = file_entries[i].unk_1C;
        entry.unk_20 = file_entries[i].unk_20;
        entry.unk_24 = file_entries[i].unk_24;
        entry.unk_28 = file_entries[i].unk_28;
        entry.unk_2C = file_entries[i].unk_2C;
        entry.unk_30 = file_entries[i].unk_30;
        entry.unk_34 = file_entries[i].unk_34;
        entry.unk_38 = file_entries[i].unk_38;
        entry.unk_3C = file_entries[i].unk_3C;
        entry.unk_40 = file_entries[i].unk_40;
        entry.unk_44 = file_entries[i].unk_44;
        entry.unk_48 = file_entries[i].unk_48;
        entry.unk_5C = file_entries[i].unk_5C;
        entry.unk_60 = file_entries[i].unk_60;
        entry.unk_64 = file_entries[i].unk_64;
        entry.unk_68 = file_entries[i].unk_68;
    }

    return true;
}

uint8_t *CmlFile::Save(size_t *psize)
{
    size_t size = sizeof(uint32_t) + entries.size() * sizeof(CMLEntry);
    uint8_t *buf = new uint8_t[size];
    memset(buf, 0, size);

    *(uint32_t *)buf = (uint32_t)entries.size();
    CMLEntry *file_entries = (CMLEntry *)(buf+sizeof(uint32_t));

    for (size_t i = 0; i < entries.size(); i++)
    {
        const CmlEntry &entry = entries[i];

        file_entries[i].char_id = entry.char_id;
        file_entries[i].costume_id = entry.costume_id;
        file_entries[i].unk_04 = entry.unk_04;

        file_entries[i].css_pos_x = entry.css_pos[0];
        file_entries[i].css_pos_y = entry.css_pos[1];
        file_entries[i].css_pos_z = entry.css_pos[2];

        file_entries[i].css_rot_x = entry.css_rot[0];
        file_entries[i].css_rot_y = entry.css_rot[1];

        file_entries[i].unk_0C = entry.unk_0C;

        file_entries[i].neo_unk_10 = entry.neo_unk_10;

        file_entries[i].unk_10 = entry.unk_10;
        file_entries[i].unk_14 = entry.unk_14;
        file_entries[i].unk_18 = entry.unk_18;
        file_entries[i].unk_1C = entry.unk_1C;
        file_entries[i].unk_20 = entry.unk_20;
        file_entries[i].unk_24 = entry.unk_24;
        file_entries[i].unk_28 = entry.unk_28;
        file_entries[i].unk_2C = entry.unk_2C;
        file_entries[i].unk_30 = entry.unk_30;
        file_entries[i].unk_34 = entry.unk_34;
        file_entries[i].unk_38 = entry.unk_38;
        file_entries[i].unk_3C = entry.unk_3C;
        file_entries[i].unk_40 = entry.unk_40;
        file_entries[i].unk_44 = entry.unk_44;
        file_entries[i].unk_48 = entry.unk_48;
        file_entries[i].unk_5C = entry.unk_5C;
        file_entries[i].unk_60 = entry.unk_60;
        file_entries[i].unk_64 = entry.unk_64;
        file_entries[i].unk_68 = entry.unk_68;
    }

    *psize = size;
    return buf;
}

TiXmlDocument *CmlFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("CML");

    for (const CmlEntry &entry : entries)
    {
        entry.Decompile(root);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool CmlFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "CML");

    if (!root)
    {
        DPRINTF("Cannot find\"CML\" in xml.\n");
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "CmlEntry")
        {
            CmlEntry entry;

            if (!entry.Compile(elem))
                return false;

            entries.push_back(entry);
        }
    }

    return true;
}

size_t CmlFile::FindEntriesByCharID(uint32_t char_id, std::vector<CmlEntry *> &cml_entries)
{
    cml_entries.clear();

    for (CmlEntry &entry : entries)
    {
        if (entry.char_id == char_id)
        {
            cml_entries.push_back(&entry);
        }
    }

    return cml_entries.size();
}

bool CmlFile::AddEntry(const CmlEntry &entry, bool unique_char_id)
{
    if (unique_char_id)
    {
        std::vector<CmlEntry *> temp;

        if (FindEntriesByCharID(entry.char_id, temp) != 0)
            return false;
    }

    entries.push_back(entry);
    return true;
}

size_t CmlFile::RemoveEntries(uint32_t char_id)
{
    size_t removed = 0;

    for (size_t i = 0; i < entries.size(); i++)
    {
        if (entries[i].char_id == char_id)
        {
            entries.erase(entries.begin()+i);
            i--;
            removed++;
        }
    }

    return removed;
}

CmlEntry *CmlFile::FindEntry(uint32_t char_id, uint32_t costume_id)
{
    for (CmlEntry &entry : entries)
    {
        if (entry.char_id == char_id && entry.costume_id == costume_id)
            return &entry;
    }

    return nullptr;
}




