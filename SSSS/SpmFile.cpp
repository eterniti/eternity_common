#include "SpmFile.h"

#include "debug.h"

void SpmEntry::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("SpmEntry");

    entry_root->SetAttribute("name", name);

    Utils::WriteParamMultipleUnsigned(entry_root, "U_10", std::vector<uint16_t>(unk_10, unk_10+8), true);
    Utils::WriteParamMultipleFloats(entry_root, "F_20", std::vector<float>(unk_20, unk_20+128));
    Utils::WriteParamMultipleUnsigned(entry_root, "U_220", std::vector<uint16_t>(unk_220, unk_220+8), true);
    Utils::WriteParamMultipleFloats(entry_root, "F_230", std::vector<float>(unk_230, unk_230+32));

    root->LinkEndChild(entry_root);
}

bool SpmEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrString(root, "name", name))
    {
        DPRINTF("%s: Cannot get name attribute.\n", FUNCNAME);
        return false;
    }

    if (!Utils::GetParamMultipleUnsigned(root, "U_10", unk_10, 8))
        return false;

    if (!Utils::GetParamMultipleFloats(root, "F_20", unk_20, 128))
        return false;

    if (!Utils::GetParamMultipleUnsigned(root, "U_220", unk_220, 8))
        return false;

    if (!Utils::GetParamMultipleFloats(root, "F_230", unk_230, 32))
        return false;

    return true;
}

SpmFile::SpmFile()
{
    this->big_endian = false;
}

SpmFile::~SpmFile()
{

}

void SpmFile::Reset()
{
    name = "";
    entries.clear();
}

bool SpmFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    SPMHeader *hdr = (SPMHeader *)buf;

    if (size < sizeof(SPMHeader) || memcmp(hdr->signature, SPM_SIGNATURE, 4) != 0)
        return false;

    name = hdr->name;
    entries.resize(val32(hdr->num_entries));

    SPMEntry *f_entries = (SPMEntry *)GetOffsetPtr(buf, hdr->data_start);

    for (size_t i = 0; i < entries.size(); i++)
    {
        SpmEntry &entry = entries[i];
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
            if (f_entries[i].unk_2B0[j] != 0)
            {
                DPRINTF("%s: unk_2B0 not 0 as expected, near offset 0x%x\n", FUNCNAME, Utils::DifPointer(f_entries + i, buf));
                return false;
            }           
        }

        for (int j = 0; j < 128; j++)
        {
            entry.unk_20[j] = val_float(f_entries[i].unk_20[j]);

            if (j < 32)
            {
                entry.unk_230[j] = val_float(f_entries[i].unk_230[j]);

                if (j < 8)
                {
                    entry.unk_10[j] = val16(f_entries[i].unk_10[j]);
                    entry.unk_220[j] = val16(f_entries[i].unk_220[j]);
                }
            }
        }
    }

    return true;
}

uint8_t *SpmFile::Save(size_t *psize)
{
    *psize = sizeof(SPMHeader) + entries.size() * sizeof(SPMEntry);

    uint8_t *buf = new uint8_t[*psize];
    memset(buf, 0, *psize);

    SPMHeader *hdr = (SPMHeader *)buf;
    memcpy(hdr->signature, SPM_SIGNATURE, 4);

    assert(name.length() < 4);
    strcpy(hdr->name, name.c_str());

    hdr->num_entries = val32(entries.size());
    hdr->data_start = val32(sizeof(SPMHeader));

    SPMEntry *f_entries = (SPMEntry *)GetOffsetPtr(buf, hdr->data_start);

    for (size_t i = 0; i < entries.size(); i++)
    {
        const SpmEntry &entry = entries[i];

        assert(entry.name.length() <= 16);
        memcpy(f_entries[i].name, entry.name.c_str(), entry.name.length());

        size_t pos = entry.name.find(':');
        if (pos != std::string::npos)
        {
            f_entries[i].name[pos] = 0;
        }

        for (int j = 0; j < 128; j++)
        {
            copy_float(f_entries[i].unk_20+j, entry.unk_20[j]);

            if (j < 32)
            {
                copy_float(f_entries[i].unk_230+j, entry.unk_230[j]);

                if (j < 8)
                {
                    f_entries[i].unk_10[j] = val16(entry.unk_10[j]);
                    f_entries[i].unk_220[j] = val16(entry.unk_220[j]);
                }
            }
        }
    }

    return buf;
}

TiXmlDocument *SpmFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("SPM");
    root->SetAttribute("name", name);

    for (size_t i = 0; i < entries.size(); i++)
    {
        entries[i].Decompile(root);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool SpmFile::Compile(TiXmlDocument *doc, bool big_endian)
{
    Reset();
    this->big_endian = big_endian;

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "SPM");

    if (!root)
    {
        DPRINTF("Cannot find\"SPM\" in xml.\n");
        return false;
    }

    if (!Utils::ReadAttrString(root, "name", name))
    {
        DPRINTF("%s: Cannot get attribute name.\n", FUNCNAME);
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "SpmEntry")
        {
            SpmEntry entry;

            if (!entry.Compile(elem))
                return false;

            entries.push_back(entry);
        }
    }

    return true;
}
