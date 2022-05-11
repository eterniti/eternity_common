#include "AitFile.h"
#include "debug.h"

#define COPY_IN(n) entry.n = file_entries[i].n
#define COPY_OUT(n) file_entries[i].n = entry.n

TiXmlElement *AitEntry::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("AitEntry");

    entry_root->SetAttribute("id", id);

    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
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
    Utils::WriteParamUnsigned(entry_root, "U_44", unk_44, true);
    Utils::WriteParamUnsigned(entry_root, "U_48", unk_48, true);
    Utils::WriteParamUnsigned(entry_root, "U_4C", unk_4C, true);
    Utils::WriteParamUnsigned(entry_root, "U_50", unk_50, true);
    Utils::WriteParamUnsigned(entry_root, "U_54", unk_54, true);
    Utils::WriteParamUnsigned(entry_root, "U_58", unk_58, true);
    Utils::WriteParamUnsigned(entry_root, "U_5C", unk_5C, true);
    Utils::WriteParamUnsigned(entry_root, "U_60", unk_60, true);
    Utils::WriteParamUnsigned(entry_root, "U_64", unk_64, true);
    Utils::WriteParamUnsigned(entry_root, "U_68", unk_68, true);
    Utils::WriteParamUnsigned(entry_root, "U_6C", unk_6C, true);
    Utils::WriteParamUnsigned(entry_root, "U_70", unk_70, true);
    Utils::WriteParamUnsigned(entry_root, "U_74", unk_74, true);
    Utils::WriteParamUnsigned(entry_root, "U_78", unk_78, true);
    Utils::WriteParamUnsigned(entry_root, "U_7C", unk_7C, true);
    Utils::WriteParamUnsigned(entry_root, "U_80", unk_80, true);
    Utils::WriteParamUnsigned(entry_root, "U_84", unk_84, true);
    Utils::WriteParamUnsigned(entry_root, "U_88", unk_88, true);
    Utils::WriteParamUnsigned(entry_root, "U_8C", unk_8C, true);
    Utils::WriteParamUnsigned(entry_root, "U_90", unk_90, true);
    Utils::WriteParamUnsigned(entry_root, "U_94", unk_94, true);
    Utils::WriteParamUnsigned(entry_root, "U_98", unk_98, true);
    Utils::WriteParamUnsigned(entry_root, "U_9C", unk_9C, true);
    Utils::WriteParamUnsigned(entry_root, "U_A0", unk_A0, true);
    Utils::WriteParamUnsigned(entry_root, "U_A4", unk_A4, true);
    Utils::WriteParamUnsigned(entry_root, "U_A8", unk_A8, true);
    Utils::WriteParamUnsigned(entry_root, "U_AC", unk_AC, true);
    Utils::WriteParamUnsigned(entry_root, "U_B0", unk_B0, true);
    Utils::WriteParamUnsigned(entry_root, "U_B4", unk_B4, true);
    Utils::WriteParamUnsigned(entry_root, "U_B8", unk_B8, true);
    Utils::WriteParamUnsigned(entry_root, "U_BC", unk_BC, true);
    Utils::WriteParamUnsigned(entry_root, "U_C0", unk_C0, true);
    Utils::WriteParamUnsigned(entry_root, "U_C4", unk_C4, true);
    Utils::WriteParamUnsigned(entry_root, "U_C8", unk_C8, true);
    Utils::WriteParamUnsigned(entry_root, "U_CC", unk_CC, true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool AitEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "id", &id))
    {
        DPRINTF("%s: attribute \"id\" is not optional.\n", FUNCNAME);
        return false;
    }

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04)) return false;
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
    if (!Utils::GetParamUnsigned(root, "U_44", &unk_44)) return false;
    if (!Utils::GetParamUnsigned(root, "U_48", &unk_48)) return false;
    if (!Utils::GetParamUnsigned(root, "U_4C", &unk_4C)) return false;
    if (!Utils::GetParamUnsigned(root, "U_50", &unk_50)) return false;
    if (!Utils::GetParamUnsigned(root, "U_54", &unk_54)) return false;
    if (!Utils::GetParamUnsigned(root, "U_58", &unk_58)) return false;
    if (!Utils::GetParamUnsigned(root, "U_5C", &unk_5C)) return false;
    if (!Utils::GetParamUnsigned(root, "U_60", &unk_60)) return false;
    if (!Utils::GetParamUnsigned(root, "U_64", &unk_64)) return false;
    if (!Utils::GetParamUnsigned(root, "U_68", &unk_68)) return false;
    if (!Utils::GetParamUnsigned(root, "U_6C", &unk_6C)) return false;
    if (!Utils::GetParamUnsigned(root, "U_70", &unk_70)) return false;
    if (!Utils::GetParamUnsigned(root, "U_74", &unk_74)) return false;
    if (!Utils::GetParamUnsigned(root, "U_78", &unk_78)) return false;
    if (!Utils::GetParamUnsigned(root, "U_7C", &unk_7C)) return false;
    if (!Utils::GetParamUnsigned(root, "U_80", &unk_80)) return false;
    if (!Utils::GetParamUnsigned(root, "U_84", &unk_84)) return false;
    if (!Utils::GetParamUnsigned(root, "U_88", &unk_88)) return false;
    if (!Utils::GetParamUnsigned(root, "U_8C", &unk_8C)) return false;
    if (!Utils::GetParamUnsigned(root, "U_90", &unk_90)) return false;
    if (!Utils::GetParamUnsigned(root, "U_94", &unk_94)) return false;
    if (!Utils::GetParamUnsigned(root, "U_98", &unk_98)) return false;
    if (!Utils::GetParamUnsigned(root, "U_9C", &unk_9C)) return false;
    if (!Utils::GetParamUnsigned(root, "U_A0", &unk_A0)) return false;
    if (!Utils::GetParamUnsigned(root, "U_A4", &unk_A4)) return false;
    if (!Utils::GetParamUnsigned(root, "U_A8", &unk_A8)) return false;
    if (!Utils::GetParamUnsigned(root, "U_AC", &unk_AC)) return false;
    if (!Utils::GetParamUnsigned(root, "U_B0", &unk_B0)) return false;
    if (!Utils::GetParamUnsigned(root, "U_B4", &unk_B4)) return false;
    if (!Utils::GetParamUnsigned(root, "U_B8", &unk_B8)) return false;
    if (!Utils::GetParamUnsigned(root, "U_BC", &unk_BC)) return false;
    if (!Utils::GetParamUnsigned(root, "U_C0", &unk_C0)) return false;
    if (!Utils::GetParamUnsigned(root, "U_C4", &unk_C4)) return false;
    if (!Utils::GetParamUnsigned(root, "U_C8", &unk_C8)) return false;
    if (!Utils::GetParamUnsigned(root, "U_CC", &unk_CC)) return false;

    return true;
}

AitFile::AitFile()
{
    this->big_endian = false;
}

AitFile::~AitFile()
{

}

void AitFile::Reset()
{
    entries.clear();
    unk_0C = 0;
}

bool AitFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(AITHeader))
        return false;

    const AITHeader *hdr = (const AITHeader *)buf;
    if (hdr->signature != AIT_SIGNATURE)
        return false;

    unk_0C = hdr->unk_0C;

    const AITEntry *file_entries = (const AITEntry *)(buf+hdr->header_size);
    entries.resize(hdr->num_entries);

    for (size_t i = 0; i < entries.size(); i++)
    {
        AitEntry &entry = entries[i];

        COPY_IN(id);
        COPY_IN(unk_04);
        COPY_IN(unk_08);
        COPY_IN(unk_0C);
        COPY_IN(unk_10);
        COPY_IN(unk_14);
        COPY_IN(unk_18);
        COPY_IN(unk_1C);
        COPY_IN(unk_20);
        COPY_IN(unk_24);
        COPY_IN(unk_28);
        COPY_IN(unk_2C);
        COPY_IN(unk_30);
        COPY_IN(unk_34);
        COPY_IN(unk_38);
        COPY_IN(unk_3C);
        COPY_IN(unk_40);
        COPY_IN(unk_44);
        COPY_IN(unk_48);
        COPY_IN(unk_4C);
        COPY_IN(unk_50);
        COPY_IN(unk_54);
        COPY_IN(unk_58);
        COPY_IN(unk_5C);
        COPY_IN(unk_60);
        COPY_IN(unk_64);
        COPY_IN(unk_68);
        COPY_IN(unk_6C);
        COPY_IN(unk_70);
        COPY_IN(unk_74);
        COPY_IN(unk_78);
        COPY_IN(unk_7C);
        COPY_IN(unk_80);
        COPY_IN(unk_84);
        COPY_IN(unk_88);
        COPY_IN(unk_8C);
        COPY_IN(unk_90);
        COPY_IN(unk_94);
        COPY_IN(unk_98);
        COPY_IN(unk_9C);
        COPY_IN(unk_A0);
        COPY_IN(unk_A4);
        COPY_IN(unk_A8);
        COPY_IN(unk_AC);
        COPY_IN(unk_B0);
        COPY_IN(unk_B4);
        COPY_IN(unk_B8);
        COPY_IN(unk_BC);
        COPY_IN(unk_C0);
        COPY_IN(unk_C4);
        COPY_IN(unk_C8);
        COPY_IN(unk_CC);
    }

    return true;
}

uint8_t *AitFile::Save(size_t *psize)
{
    size_t size = sizeof(AITHeader) + entries.size() * sizeof(AITEntry);
    uint8_t *buf = new uint8_t[size];
    memset(buf, 0, size);

    AITHeader *hdr = (AITHeader *)buf;
    hdr->signature = AIT_SIGNATURE;
    hdr->header_size = sizeof(AITHeader);
    hdr->endianess_check = 0xFFFE;
    hdr->num_entries = (uint32_t)entries.size();
    hdr->unk_0C = unk_0C;

    AITEntry *file_entries = (AITEntry *)(hdr+1);

    for (size_t i = 0; i < entries.size(); i++)
    {
        const AitEntry &entry = entries[i];

        COPY_OUT(id);
        COPY_OUT(unk_04);
        COPY_OUT(unk_08);
        COPY_OUT(unk_0C);
        COPY_OUT(unk_10);
        COPY_OUT(unk_14);
        COPY_OUT(unk_18);
        COPY_OUT(unk_1C);
        COPY_OUT(unk_20);
        COPY_OUT(unk_24);
        COPY_OUT(unk_28);
        COPY_OUT(unk_2C);
        COPY_OUT(unk_30);
        COPY_OUT(unk_34);
        COPY_OUT(unk_38);
        COPY_OUT(unk_3C);
        COPY_OUT(unk_40);
        COPY_OUT(unk_44);
        COPY_OUT(unk_48);
        COPY_OUT(unk_4C);
        COPY_OUT(unk_50);
        COPY_OUT(unk_54);
        COPY_OUT(unk_58);
        COPY_OUT(unk_5C);
        COPY_OUT(unk_60);
        COPY_OUT(unk_64);
        COPY_OUT(unk_68);
        COPY_OUT(unk_6C);
        COPY_OUT(unk_70);
        COPY_OUT(unk_74);
        COPY_OUT(unk_78);
        COPY_OUT(unk_7C);
        COPY_OUT(unk_80);
        COPY_OUT(unk_84);
        COPY_OUT(unk_88);
        COPY_OUT(unk_8C);
        COPY_OUT(unk_90);
        COPY_OUT(unk_94);
        COPY_OUT(unk_98);
        COPY_OUT(unk_9C);
        COPY_OUT(unk_A0);
        COPY_OUT(unk_A4);
        COPY_OUT(unk_A8);
        COPY_OUT(unk_AC);
        COPY_OUT(unk_B0);
        COPY_OUT(unk_B4);
        COPY_OUT(unk_B8);
        COPY_OUT(unk_BC);
        COPY_OUT(unk_C0);
        COPY_OUT(unk_C4);
        COPY_OUT(unk_C8);
        COPY_OUT(unk_CC);
    }

    *psize = size;
    return buf;
}

TiXmlDocument *AitFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("AIT");
    Utils::WriteParamUnsigned(root, "U_0C", unk_0C);

    for (const AitEntry &entry : entries)
    {
        entry.Decompile(root);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool AitFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "AIT");

    if (!root)
    {
        DPRINTF("Cannot find\"AIT\" in xml.\n");
        return false;
    }

    if (!Utils::GetParamUnsigned(root, "U_0C", &unk_0C))
        return false;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "AitEntry")
        {
            AitEntry entry;

            if (!entry.Compile(elem))
                return false;

            entries.push_back(entry);
        }
    }

    return true;
}
