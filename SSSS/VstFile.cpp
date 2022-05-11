#include "VstFile.h"

#include "debug.h"

void VstEntry::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("VstEntry");

    entry_root->SetAttribute("name", name);

    Utils::WriteParamUnsigned(entry_root, "U_10", unk_10, true);
    Utils::WriteParamMultipleUnsigned(entry_root, "U_14", std::vector<uint16_t>(unk_14, unk_14+4), true);
    Utils::WriteParamMultipleFloats(entry_root, "F_1C", std::vector<float>(unk_1C, unk_1C+10));
    Utils::WriteParamUnsigned(entry_root, "U_44", unk_44, true);
    Utils::WriteParamUnsigned(entry_root, "U_48", unk_48, true);
    Utils::WriteParamMultipleFloats(entry_root, "F_4C", std::vector<float>(unk_4C, unk_4C+13));
    Utils::WriteParamUnsigned(entry_root, "U_80", unk_80, true);
    Utils::WriteParamMultipleFloats(entry_root, "F_84", std::vector<float>(unk_84, unk_84+9));
    Utils::WriteParamMultipleUnsigned(entry_root, "U_A8", std::vector<uint16_t>(unk_A8, unk_A8+78), true);

    root->LinkEndChild(entry_root);
}

bool VstEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrString(root, "name", name))
    {
        DPRINTF("%s: Cannot get name attribute.\n", FUNCNAME);
        return false;
    }

    if (!Utils::GetParamUnsigned(root, "U_10", &unk_10))
        return false;

    if (!Utils::GetParamMultipleUnsigned(root, "U_14", unk_14, 4))
        return false;

    if (!Utils::GetParamMultipleFloats(root, "F_1C", unk_1C, 10))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_44", &unk_44))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_48", &unk_48))
        return false;

    if (!Utils::GetParamMultipleFloats(root, "F_4C", unk_4C, 13))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_80", &unk_80))
        return false;

    if (!Utils::GetParamMultipleFloats(root, "F_84", unk_84, 9))
        return false;

    if (!Utils::GetParamMultipleUnsigned(root, "U_A8", unk_A8, 78))
        return false;

    return true;
}

VstFile::VstFile()
{
    this->big_endian = false;
}

VstFile::~VstFile()
{
    Reset();
}

void VstFile::Reset()
{
    entries.clear();
}

bool VstFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    VSTHeader *hdr = (VSTHeader *)buf;

    if (size < sizeof(VSTHeader) || memcmp(hdr->signature, VST_SIGNATURE, 4) != 0)
        return false;

    entries.resize(val32(hdr->num_entries));

    VSTEntry *f_entries = (VSTEntry *)GetOffsetPtr(buf, hdr->data_start);

    for (size_t i = 0; i < entries.size(); i++)
    {
        VstEntry &entry = entries[i];
        char temp[17];

        memcpy(temp, f_entries[i].name, 16);
        temp[16] = 0;

        entry.name = temp;

        if (entry.name.length() < 15 && temp[entry.name.length()+1] != 0)
        {
            entry.name += std::string(":") + &temp[entry.name.length()+1];
        }

        entry.unk_10 = val32(f_entries[i].unk_10);
        entry.unk_44 = val32(f_entries[i].unk_44);
        entry.unk_48 = val32(f_entries[i].unk_48);
        entry.unk_80 = val32(f_entries[i].unk_80);

        for (int j = 0; j < 78; j++)
        {
            entry.unk_A8[j] = val16(f_entries[i].unk_A8[j]);

            if (j < 13)
            {
                entry.unk_4C[j] = val_float(f_entries[i].unk_4C[j]);

                if (j < 10)
                {
                    entry.unk_1C[j] = val_float(f_entries[i].unk_1C[j]);

                    if (j < 9)
                    {
                        entry.unk_84[j] = val_float(f_entries[i].unk_84[j]);

                        if (j < 4)
                        {
                            entry.unk_14[j] = val16(f_entries[i].unk_14[j]);
                        }
                    }
                }
            }
        }
    }

    return true;
}

uint8_t *VstFile::Save(size_t *psize)
{
    *psize = sizeof(VSTHeader) + entries.size() * sizeof(VSTEntry);

    uint8_t *buf = new uint8_t[*psize];
    memset(buf, 0, *psize);

    VSTHeader *hdr = (VSTHeader *)buf;
    memcpy(hdr->signature, VST_SIGNATURE, 4);
    hdr->num_entries = val32(entries.size());
    hdr->data_start = val32(sizeof(VSTHeader));

    VSTEntry *f_entries = (VSTEntry *)GetOffsetPtr(buf, hdr->data_start);

    for (size_t i = 0; i < entries.size(); i++)
    {
        const VstEntry &entry = entries[i];

        assert(entry.name.length() <= 16);
        memcpy(f_entries[i].name, entry.name.c_str(), entry.name.length());

        size_t pos = entry.name.find(':');
        if (pos != std::string::npos)
        {
            f_entries[i].name[pos] = 0;
        }

        f_entries[i].unk_10 = val32(entry.unk_10);
        f_entries[i].unk_44 = val32(entry.unk_44);
        f_entries[i].unk_48 = val32(entry.unk_48);
        f_entries[i].unk_80 = val32(entry.unk_80);

        for (int j = 0; j < 78; j++)
        {
            f_entries[i].unk_A8[j] = val16(entry.unk_A8[j]);

            if (j < 13)
            {
                copy_float(f_entries[i].unk_4C + j, entry.unk_4C[j]);

                if (j < 10)
                {
                    copy_float(f_entries[i].unk_1C + j, entry.unk_1C[j]);

                    if (j < 9)
                    {
                        copy_float(f_entries[i].unk_84+j, entry.unk_84[j]);

                        if (j < 4)
                        {
                            f_entries[i].unk_14[j] = val16(entry.unk_14[j]);
                        }
                    }
                }
            }
        }
    }

    return buf;
}

TiXmlDocument *VstFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("VST");

    for (size_t i = 0; i < entries.size(); i++)
    {
        entries[i].Decompile(root);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool VstFile::Compile(TiXmlDocument *doc, bool big_endian)
{
    Reset();
    this->big_endian = big_endian;

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "VST");

    if (!root)
    {
        DPRINTF("Cannot find\"VST\" in xml.\n");
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "VstEntry")
        {
            VstEntry entry;

            if (!entry.Compile(elem))
                return false;

            entries.push_back(entry);
        }
    }

    return true;
}
