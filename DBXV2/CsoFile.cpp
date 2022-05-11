#include <algorithm>

#include "CsoFile.h"
#include "Xenoverse2.h"
#include "debug.h"

TiXmlElement *CsoEntry::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("CsoEntry");

    std::string name = Xenoverse2::GetCharaAndCostumeName(char_id, costume_id, 0, true);
    if (name.length() != 0)
        Utils::WriteComment(entry_root, name);

    Utils::WriteParamUnsigned(entry_root, "CHAR_ID", char_id, true);
    Utils::WriteParamUnsigned(entry_root, "COSTUME_ID", costume_id, true);
    Utils::WriteParamString(entry_root, "SE", se);
    Utils::WriteParamString(entry_root, "VOX", vox);
    Utils::WriteParamString(entry_root, "AMK", amk);
    Utils::WriteParamString(entry_root, "SKILLS", skills);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool CsoEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "CHAR_ID", &char_id))
        return false;

    if (!Utils::GetParamUnsigned(root, "COSTUME_ID", &costume_id))
        return false;

    if (!Utils::GetParamString(root, "SE", se))
        return false;

    if (!Utils::GetParamString(root, "VOX", vox))
        return false;

    if (!Utils::GetParamString(root, "AMK", amk))
        return false;

    if (!Utils::GetParamString(root, "SKILLS", skills))
        return false;

    return true;
}

CsoFile::CsoFile()
{
    this->big_endian = false;
}

CsoFile::~CsoFile()
{

}

void CsoFile::Reset()
{
    entries.clear();
}

bool CsoFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (size < sizeof(CSOHeader))
        return false;

    const CSOHeader *hdr = (const CSOHeader *)buf;

    if (hdr->signature != CSO_SIGNATURE)
        return false;

    /*if (hdr->header_size != sizeof(CSOHeader))
    {
        DPRINTF("%s: Unknown header size 0x%04X\n", FUNCNAME, hdr->header_size);
        return false;
    }*/

    entries.resize(val32(hdr->num_entries));
    const CSOEntry *f_entries = (const CSOEntry *)GetOffsetPtr(hdr, hdr->entries_start);

    for (size_t i = 0; i < entries.size(); i++)
    {
        CsoEntry &entry = entries[i];

        if (f_entries[i].unk_18 != 0)
        {
            DPRINTF("%s: unk_18 not zero as expected (0x%I64x, at entry 0x%Ix)\n", FUNCNAME, f_entries[i].unk_18, i);
            return false;
        }

        entry.char_id = f_entries[i].char_id;
        entry.costume_id = f_entries[i].costume_id;
        entry.se = GetString(buf, f_entries[i].se_offset);
        entry.vox = GetString(buf, f_entries[i].vox_offset);
        entry.amk = GetString(buf, f_entries[i].amk_offset);
        entry.skills = GetString(buf, f_entries[i].skills_offset);
    }

    return true;
}

#define ADD_STR(s) { if (s.length() > 0 && std::find(str_list.begin(), str_list.end(), s) == str_list.end()) {\
    str_list.push_back(s); *str_size += s.length()+1; } }

void CsoFile::GenerateStringsList(std::vector<std::string> &str_list, size_t *str_size) const
{
    str_list.clear();
    *str_size = 0;

    for (const CsoEntry &entry : entries)
    {
        ADD_STR(entry.se);
        ADD_STR(entry.vox);
        ADD_STR(entry.amk);
        ADD_STR(entry.skills);
    }
}

size_t CsoFile::CalculateFileSize(size_t str_size)
{
    return sizeof(CSOHeader) + entries.size()*sizeof(CSOEntry) + str_size;
}

uint8_t *CsoFile::Save(size_t *psize)
{
    std::vector<std::string> str_list;
    size_t str_size, size;
    uint32_t str_base;

    GenerateStringsList(str_list, &str_size);
    size = CalculateFileSize(str_size);

    uint8_t *buf = new uint8_t[size];
    memset(buf, 0, size);

    CSOHeader *hdr = (CSOHeader *)buf;

    hdr->signature = CSO_SIGNATURE;
    hdr->endianess_check = val16(0xFFFE);
    hdr->entries_start = hdr->header_size = sizeof(CSOHeader);
    hdr->num_entries = (uint32_t)entries.size();

    CSOEntry *f_entries = (CSOEntry *)GetOffsetPtr(hdr, hdr->entries_start);
    str_base = Utils::DifPointer(&f_entries[entries.size()], buf);

    for (size_t i = 0; i < entries.size(); i++)
    {
        const CsoEntry &entry = entries[i];

        f_entries[i].char_id = entry.char_id;
        f_entries[i].costume_id = entry.costume_id;
        f_entries[i].se_offset = GetStringOffset(str_base, str_list, entry.se);
        f_entries[i].vox_offset = GetStringOffset(str_base, str_list, entry.vox);
        f_entries[i].amk_offset = GetStringOffset(str_base, str_list, entry.amk);
        f_entries[i].skills_offset = GetStringOffset(str_base, str_list, entry.skills);
    }

    WriteStringList(buf + str_base, str_list);

    *psize = size;
    return buf;
}

TiXmlDocument *CsoFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("CSO");

    for (const CsoEntry &entry : entries)
    {
        entry.Decompile(root);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool CsoFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "CSO");

    if (!root)
    {
        DPRINTF("Cannot find\"CSO\" in xml.\n");
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "CsoEntry")
        {
            CsoEntry entry;

            if (!entry.Compile(elem))
                return false;

            entries.push_back(entry);
        }
    }

    return true;
}

size_t CsoFile::FindEntriesByCharID(uint32_t char_id, std::vector<CsoEntry *> &cso_entries)
{
    cso_entries.clear();

    for (CsoEntry &entry : entries)
    {
        if (entry.char_id == char_id)
        {
            cso_entries.push_back(&entry);
        }
    }

    return cso_entries.size();
}

bool CsoFile::AddEntry(const CsoEntry &entry, bool unique_char_id)
{
    if (unique_char_id)
    {
        std::vector<CsoEntry *> temp;

        if (FindEntriesByCharID(entry.char_id, temp) != 0)
            return false;
    }

    entries.push_back(entry);
    return true;
}

size_t CsoFile::RemoveEntries(uint32_t char_id)
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


