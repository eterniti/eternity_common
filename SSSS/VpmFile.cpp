#include "VpmFile.h"

#include "debug.h"

void VpmEntry::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("VpmEntry");

    entry_root->SetAttribute("name", name);

    Utils::WriteParamMultipleFloats(entry_root, "F_10", std::vector<float>(unk_10, unk_10+12));
    Utils::WriteParamMultipleUnsigned(entry_root, "U_40", std::vector<uint16_t>(unk_40, unk_40+10), true);
    Utils::WriteParamMultipleUnsigned(entry_root, "U_54", std::vector<uint32_t>(unk_54, unk_54+5), true);
    Utils::WriteParamMultipleFloats(entry_root, "F_68", std::vector<float>(unk_68, unk_68+6));

    root->LinkEndChild(entry_root);
}

bool VpmEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrString(root, "name", name))
    {
        DPRINTF("%s: Cannot get name attribute.\n", FUNCNAME);
        return false;
    }

    if (!Utils::GetParamMultipleFloats(root, "F_10", unk_10, 12))
        return false;

    if (!Utils::GetParamMultipleUnsigned(root, "U_40", unk_40, 10))
        return false;

    if (!Utils::GetParamMultipleUnsigned(root, "U_54", unk_54, 5))
        return false;

    if (!Utils::GetParamMultipleFloats(root, "F_68", unk_68, 6))
        return false;

    return true;
}

VpmFile::VpmFile()
{
    this->big_endian = false;
}

VpmFile::~VpmFile()
{
    Reset();
}

void VpmFile::Reset()
{
    entries.clear();
}

bool VpmFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    VPMHeader *hdr = (VPMHeader *)buf;

    if (size < sizeof(VPMHeader) || memcmp(hdr->signature, VPM_SIGNATURE, 4) != 0)
        return false;

    entries.resize(val32(hdr->num_entries));

    VPMEntry *f_entries = (VPMEntry *)GetOffsetPtr(buf, hdr->data_start);

    for (size_t i = 0; i < entries.size(); i++)
    {
        VpmEntry &entry = entries[i];
        char temp[17];

        memcpy(temp, f_entries[i].name, 16);
        temp[16] = 0;

        entry.name = temp;

        if (entry.name.length() < 15 && temp[entry.name.length()+1] != 0)
        {
            entry.name += std::string(":") + &temp[entry.name.length()+1];
        }

        for (int j = 0; j < 12; j++)
        {
            entry.unk_10[j] = val_float(f_entries[i].unk_10[j]);

            if (j < 10)
            {
                entry.unk_40[j] = val16(f_entries[i].unk_40[j]);

                if (j < 6)
                {
                    entry.unk_68[j] = val_float(f_entries[i].unk_68[j]);

                    if (j < 5)
                    {
                        entry.unk_54[j] = val32(f_entries[i].unk_54[j]);
                    }
                }
            }
        }
    }

    return true;
}

uint8_t *VpmFile::Save(size_t *psize)
{
    *psize = sizeof(VPMHeader) + entries.size() * sizeof(VPMEntry);

    uint8_t *buf = new uint8_t[*psize];
    memset(buf, 0, *psize);

    VPMHeader *hdr = (VPMHeader *)buf;
    memcpy(hdr->signature, VPM_SIGNATURE, 4);
    hdr->num_entries = val32(entries.size());
    hdr->data_start = val32(sizeof(VPMHeader));

    VPMEntry *f_entries = (VPMEntry *)GetOffsetPtr(buf, hdr->data_start);

    for (size_t i = 0; i < entries.size(); i++)
    {
        const VpmEntry &entry = entries[i];

        assert(entry.name.length() <= 16);
        memcpy(f_entries[i].name, entry.name.c_str(), entry.name.length());

        size_t pos = entry.name.find(':');
        if (pos != std::string::npos)
        {
            f_entries[i].name[pos] = 0;
        }

        for (int j = 0; j < 12; j++)
        {
            copy_float(f_entries[i].unk_10+j, entry.unk_10[j]);

            if (j < 10)
            {
                f_entries[i].unk_40[j] = val16(entry.unk_40[j]);

                if (j < 6)
                {
                    copy_float(f_entries[i].unk_68+j, entry.unk_68[j]);

                    if (j < 5)
                    {
                        f_entries[i].unk_54[j] = val32(entry.unk_54[j]);
                    }
                }
            }
        }
    }

    return buf;
}

TiXmlDocument *VpmFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("VPM");

    for (size_t i = 0; i < entries.size(); i++)
    {
        entries[i].Decompile(root);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool VpmFile::Compile(TiXmlDocument *doc, bool big_endian)
{
    Reset();
    this->big_endian = big_endian;

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "VPM");

    if (!root)
    {
        DPRINTF("Cannot find\"VPM\" in xml.\n");
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "VpmEntry")
        {
            VpmEntry entry;

            if (!entry.Compile(elem))
                return false;

            entries.push_back(entry);
        }
    }

    return true;
}
