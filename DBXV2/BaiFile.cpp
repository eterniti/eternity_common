#include "BaiFile.h"
#include "debug.h"

TiXmlElement *BaiSubEntry::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("BaiSubEntry");

    entry_root->SetAttribute("table_name", table_name);

    Utils::WriteParamUnsigned(entry_root, "U_08", unk_08, true);
    Utils::WriteParamUnsigned(entry_root, "U_0C", unk_0C, true);
    Utils::WriteParamUnsigned(entry_root, "U_10", unk_10, true);
    Utils::WriteParamUnsigned(entry_root, "U_14", unk_14, true);
    Utils::WriteParamUnsigned(entry_root, "U_18", unk_18, true);
    Utils::WriteParamUnsigned(entry_root, "U_1C", unk_1C, true);
    Utils::WriteParamUnsigned(entry_root, "U_20", unk_20, true);
    Utils::WriteParamUnsigned(entry_root, "U_24", unk_24, true);
    Utils::WriteParamUnsigned(entry_root, "U_28", unk_28, true);
    Utils::WriteParamUnsigned(entry_root, "U_2C", unk_2C, true);
    Utils::WriteParamUnsigned(entry_root, "U_30", unk_30, true);
    Utils::WriteParamUnsigned(entry_root, "U_34", unk_34, true);
    Utils::WriteParamUnsigned(entry_root, "U_38", unk_38, true);
    Utils::WriteParamUnsigned(entry_root, "U_3C", unk_3C, true);
    Utils::WriteParamUnsigned(entry_root, "U_40", unk_40, true);
    Utils::WriteParamFloat(entry_root, "F_44", unk_44);
    Utils::WriteParamFloat(entry_root, "F_48", unk_48);
    Utils::WriteParamFloat(entry_root, "F_4C", unk_4C);
    Utils::WriteParamUnsigned(entry_root, "U_50", unk_50, true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BaiSubEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrString(root, "table_name", table_name))
    {
        DPRINTF("%s: attribute \"table_name\" is not optional.\n", FUNCNAME);
        return false;
    }

    if (table_name.length() > 8)
    {
        DPRINTF("%s: table_name cannot be bigger than 8 chars.\n", FUNCNAME);
        return false;
    }

    if (!Utils::GetParamUnsigned(root, "U_08", &unk_08)) return false;
    if (!Utils::GetParamUnsigned(root, "U_0C", &unk_0C)) return false;
    if (!Utils::GetParamUnsigned(root, "U_10", &unk_10)) return false;
    if (!Utils::GetParamUnsigned(root, "U_14", &unk_14)) return false;
    if (!Utils::GetParamUnsigned(root, "U_18", &unk_18)) return false;
    if (!Utils::GetParamUnsigned(root, "U_1C", &unk_1C)) return false;
    if (!Utils::GetParamUnsigned(root, "U_20", &unk_20)) return false;
    if (!Utils::GetParamUnsigned(root, "U_24", &unk_24)) return false;
    if (!Utils::GetParamUnsigned(root, "U_28", &unk_28)) return false;
    if (!Utils::GetParamUnsigned(root, "U_2C", &unk_2C)) return false;
    if (!Utils::GetParamUnsigned(root, "U_30", &unk_30)) return false;
    if (!Utils::GetParamUnsigned(root, "U_34", &unk_34)) return false;
    if (!Utils::GetParamUnsigned(root, "U_38", &unk_38)) return false;
    if (!Utils::GetParamUnsigned(root, "U_3C", &unk_3C)) return false;
    if (!Utils::GetParamUnsigned(root, "U_40", &unk_40)) return false;
    if (!Utils::GetParamFloat(root, "F_44", &unk_44)) return false;
    if (!Utils::GetParamFloat(root, "F_48", &unk_48)) return false;
    if (!Utils::GetParamFloat(root, "F_4C", &unk_4C)) return false;
    if (!Utils::GetParamUnsigned(root, "U_50", &unk_50)) return false;

    return true;
}

TiXmlElement *BaiEntry::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("BaiEntry");

    Utils::WriteParamUnsigned(entry_root, "U_00", unk_00, true);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "U_08", unk_08, true);
    Utils::WriteParamUnsigned(entry_root, "U_0C", unk_0C, true);

    for (const BaiSubEntry &sub : sub_entries)
    {
        sub.Decompile(entry_root);
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BaiEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "U_00", &unk_00)) return false;
    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04)) return false;
    if (!Utils::GetParamUnsigned(root, "U_08", &unk_08)) return false;
    if (!Utils::GetParamUnsigned(root, "U_0C", &unk_0C)) return false;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "BaiSubEntry")
        {
           BaiSubEntry sub;

           if (!sub.Compile(elem))
               return false;

           sub_entries.push_back(sub);
        }
    }

    return true;
}

BaiFile::BaiFile()
{
    this->big_endian = false;
}

BaiFile::~BaiFile()
{

}

void BaiFile::Reset()
{
    entries.clear();
}

bool BaiFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(BAIHeader))
        return false;

    const BAIHeader *hdr = (const BAIHeader *)buf;

    if (hdr->signature != BAI_SIGNATURE)
        return false;

    entries.resize(hdr->num_entries);

    const BAIEntry *file_entries = (const BAIEntry *)(buf+hdr->data_start);

    for (size_t i = 0; i < hdr->num_entries; i++)
    {
        BaiEntry &entry = entries[i];

        entry.unk_00 = file_entries[i].unk_00;
        entry.unk_04 = file_entries[i].unk_04;
        entry.unk_08 = file_entries[i].unk_08;
        entry.unk_0C = file_entries[i].unk_0C;
        entry.sub_entries.resize(file_entries[i].num_subentries);

        const BAISubEntrry *file_subs = (const BAISubEntrry *)(buf+file_entries[i].subentries_offset);

        for (size_t j = 0; j < entry.sub_entries.size(); j++)
        {
            BaiSubEntry &sub = entry.sub_entries[j];
            char temp_str[9];

            memcpy(temp_str, file_subs[j].table_name, 8);
            temp_str[8] = 0;

            sub.table_name = temp_str;
            sub.unk_08 = file_subs[j].unk_08;
            sub.unk_0C = file_subs[j].unk_0C;
            sub.unk_10 = file_subs[j].unk_10;
            sub.unk_14 = file_subs[j].unk_14;
            sub.unk_18 = file_subs[j].unk_18;
            sub.unk_1C = file_subs[j].unk_1C;
            sub.unk_20 = file_subs[j].unk_20;
            sub.unk_24 = file_subs[j].unk_24;
            sub.unk_28 = file_subs[j].unk_28;
            sub.unk_2C = file_subs[j].unk_2C;
            sub.unk_30 = file_subs[j].unk_30;
            sub.unk_34 = file_subs[j].unk_34;
            sub.unk_38 = file_subs[j].unk_38;
            sub.unk_3C = file_subs[j].unk_3C;
            sub.unk_40 = file_subs[j].unk_40;
            sub.unk_44 = file_subs[j].unk_44;
            sub.unk_48 = file_subs[j].unk_48;
            sub.unk_4C = file_subs[j].unk_4C;
            sub.unk_50 = file_subs[j].unk_50;
        }
    }

    return true;
}

size_t BaiFile::CalculateFileSize() const
{
    size_t size = sizeof(BAIHeader) + entries.size() * sizeof(BAIEntry);

    for (const BaiEntry &entry : entries)
        size += entry.sub_entries.size() * sizeof(BAISubEntrry);

    return size;
}

uint8_t *BaiFile::Save(size_t *psize)
{
    size_t size = CalculateFileSize();
    uint8_t *buf = new uint8_t[size];
    memset(buf, 0, size);

    BAIHeader *hdr = (BAIHeader *)buf;
    hdr->signature = BAI_SIGNATURE;
    hdr->endianess_check = val16(0xFFFE);
    hdr->version = 3;
    hdr->num_entries = (uint32_t)entries.size();
    hdr->data_start = sizeof(BAIHeader);

    BAIEntry *file_entries = (BAIEntry *)(hdr+1);
    BAISubEntrry *file_sub = (BAISubEntrry *)(file_entries + hdr->num_entries);

    for (size_t i = 0; i < entries.size(); i++)
    {
        const BaiEntry &entry = entries[i];

        file_entries[i].unk_00 = entry.unk_00;
        file_entries[i].unk_04 = entry.unk_04;
        file_entries[i].unk_08 = entry.unk_08;
        file_entries[i].unk_0C = entry.unk_0C;
        file_entries[i].num_subentries = (uint32_t)entry.sub_entries.size();
        file_entries[i].subentries_offset = Utils::DifPointer(file_sub, buf);

        for (size_t j = 0; j < entry.sub_entries.size(); j++)
        {
            const BaiSubEntry &sub = entry.sub_entries[j];

            if (sub.table_name.length() > 8)
            {
                DPRINTF("%s: Cannot save binary, table_name %s has more than 8 chars.\n", FUNCNAME, sub.table_name.c_str());
                delete[] buf;
                return nullptr;
            }

            memcpy(file_sub->table_name, sub.table_name.c_str(), sub.table_name.length());
            file_sub->unk_08 = sub.unk_08;
            file_sub->unk_0C = sub.unk_0C;
            file_sub->unk_10 = sub.unk_10;
            file_sub->unk_14 = sub.unk_14;
            file_sub->unk_18 = sub.unk_18;
            file_sub->unk_1C = sub.unk_1C;
            file_sub->unk_20 = sub.unk_20;
            file_sub->unk_24 = sub.unk_24;
            file_sub->unk_28 = sub.unk_28;
            file_sub->unk_2C = sub.unk_2C;
            file_sub->unk_30 = sub.unk_30;
            file_sub->unk_34 = sub.unk_34;
            file_sub->unk_38 = sub.unk_38;
            file_sub->unk_3C = sub.unk_3C;
            file_sub->unk_40 = sub.unk_40;
            file_sub->unk_44 = sub.unk_44;
            file_sub->unk_48 = sub.unk_48;
            file_sub->unk_4C = sub.unk_4C;
            file_sub->unk_50 = sub.unk_50;

            file_sub++;
        }
    }

    *psize = size;
    return buf;
}

TiXmlDocument *BaiFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("BAI");

    for (const BaiEntry &entry : entries)
    {
        entry.Decompile(root);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool BaiFile::Compile(TiXmlDocument *doc, bool )
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "BAI");

    if (!root)
    {
        DPRINTF("Cannot find\"BAI\" in xml.\n");
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "BaiEntry")
        {
            BaiEntry entry;

            if (!entry.Compile(elem))
                return false;

            entries.push_back(entry);
        }
    }

    return true;
}
