#include "CncFile.h"
#include "Xenoverse2.h"
#include "debug.h"

TiXmlElement *CNCEntry::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("CncEntry");
    entry_root->SetAttribute("cms_id", Utils::UnsignedToString(cms_id, true));
    entry_root->SetAttribute("costume", costume_index);
    entry_root->SetAttribute("model_preset", model_preset);

    if (game_cms)
    {
        std::string name = Xenoverse2::GetCharaAndCostumeName(cms_id, costume_index, model_preset, false);

        if (name.length() > 0)
            Utils::WriteComment(entry_root, name);
    }

    Utils::WriteParamMultipleUnsigned(entry_root, "CNS_SKILL_IDS", std::vector<uint16_t>(cns_skills_ids, cns_skills_ids+3), true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool CNCEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "cms_id", &cms_id))
    {
        DPRINTF("%s: cms_id is not optional.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "costume", &costume_index))
    {
        DPRINTF("%s: costume is not optional.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "model_preset", &model_preset))
        model_preset = 0;

    if (!Utils::GetParamMultipleUnsigned(root, "CNS_SKILL_IDS", cns_skills_ids, 3))
        return false;

    return true;
}

CncFile::CncFile()
{
    this->big_endian = false;
}

CncFile::~CncFile()
{

}

void CncFile::Reset()
{
    entries.clear();
}

bool CncFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(CNCHeader))
        return false;

    const CNCHeader *hdr = (const CNCHeader *)buf;
    if (hdr->signature != CNC_SIGNATURE)
        return false;

    entries.resize(hdr->num_entries);

    const CNCEntry *file_entries = (const CNCEntry *)(buf + hdr->data_start);
    for (size_t i = 0; i < entries.size(); i++)
    {
        entries[i] = file_entries[i];

        /*if (entries[i].unk_08 != 0xFFFFFFFF)
        {
            DPRINTF("Unk 8 value of %d\n", entries[i].unk_08);
        }*/
    }

    return true;
}

uint8_t *CncFile::Save(size_t *psize)
{
    *psize = sizeof(CNCHeader) + entries.size() * sizeof(CNCEntry);
    uint8_t *buf = new uint8_t[*psize];

    CNCHeader *hdr = (CNCHeader *)buf;
    hdr->signature = CNC_SIGNATURE;
    hdr->endianess_check = 0xFFFE;
    hdr->data_start = sizeof(CNCHeader);
    hdr->num_entries = (uint32_t) entries.size();

    CNCEntry *file_entries = (CNCEntry *)(hdr + 1);
    for (size_t i = 0; i < entries.size(); i++)
        file_entries[i] = entries[i];

    return buf;
}

TiXmlDocument *CncFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("CNC");

    for (const CNCEntry &entry : entries)
    {
        entry.Decompile(root);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool CncFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "CNC");

    if (!root)
    {
        DPRINTF("Cannot find\"CNC\" in xml.\n");
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "CncEntry")
        {
            CNCEntry entry;

            if (!entry.Compile(elem))
                return false;

            entries.push_back(entry);
        }
    }

    return true;
}

size_t CncFile::FindEntriesByCharID(uint32_t char_id, std::vector<CncEntry *> &cnc_entries)
{
    cnc_entries.clear();

    for (CncEntry &entry : entries)
    {
        if (entry.cms_id == char_id)
            cnc_entries.push_back(&entry);
    }

    return cnc_entries.size();
}

void CncFile::AddEntry(const CncEntry &entry)
{
    entries.push_back(entry);
}

size_t CncFile::RemoveEntries(uint32_t char_id)
{
    size_t count = 0;

    for (size_t i = 0; i < entries.size(); i++)
    {
        if (entries[i].cms_id == char_id)
        {
            entries.erase(entries.begin()+i);
            i--;
            count++;
        }
    }

    return count;
}




