#include "HciFile.h"
#include "Xenoverse2.h"
#include "debug.h"

TiXmlElement *HciEntry::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("HciEntry");

    std::string name = Xenoverse2::GetCharaAndCostumeName(char_id, costume_id, 0, true);
    if (name.length() != 0)
    {
        TiXmlComment *comment = new TiXmlComment();
        comment->SetValue(name);

        root->LinkEndChild(comment);
    }

    entry_root->SetAttribute("char_id", Utils::UnsignedToString(char_id, true));
    entry_root->SetAttribute("costume_id", Utils::UnsignedToString(costume_id, true));
    entry_root->SetAttribute("state1", Utils::UnsignedToString(state1, true));
    entry_root->SetAttribute("state2", Utils::UnsignedToString(state2, true));
    entry_root->SetAttribute("emb_index", Utils::UnsignedToString(emb_index, false));

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool HciEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "char_id", &char_id))
    {
        DPRINTF("%s: Cannot find char_id attribute.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "costume_id", &costume_id))
    {
        DPRINTF("%s: Cannot find costume_id attribute.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "state1", &state1))
    {
        DPRINTF("%s: Cannot find state1 attribute.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "state2", &state2))
    {
        DPRINTF("%s: Cannot find state2 attribute.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "emb_index", &emb_index))
    {
        DPRINTF("%s: Cannot find emb_index attribute.\n", FUNCNAME);
        return false;
    }

    return true;
}

HciFile::HciFile()
{
   this->big_endian = false;
}

HciFile::~HciFile()
{

}

void HciFile::Reset()
{
    entries.clear();
}

bool HciFile::LoadCommon(const uint8_t *buf, size_t size)
{
    if (!buf || size < sizeof(HCIHeader))
        return false;

    const HCIHeader *hdr = (const HCIHeader *)buf;

    if (hdr->signature != HCI_SIGNATURE && memcmp(buf+1, "CHI", 3) != 0)
        return false;

    const HCIEntry *file_entries = (const HCIEntry *)GetOffsetPtr(hdr, hdr->entries_offset);
    size_t current_size = entries.size();
    entries.resize(current_size + hdr->num_entries);

    for (size_t i = current_size, j = 0; i < entries.size(); i++, j++)
    {
        HciEntry &entry = entries[i];

        entry.char_id = file_entries[j].char_id;
        entry.costume_id = file_entries[j].costume_id;
        entry.state1 = file_entries[j].state1;
        entry.state2 = file_entries[j].state2;
        entry.emb_index = file_entries[j].emb_index;
    }

    return true;
}

bool HciFile::Load(const uint8_t *buf, size_t size)
{
    Reset();
    return LoadCommon(buf, size);
}

bool HciFile::LoadMultiple(const std::vector<const uint8_t *> &bufs, const std::vector<size_t> &sizes)
{
    Reset();

    if (bufs.size() != sizes.size())
        return false;

    if (bufs.size() == 0)
        return false;

    for (size_t i = 0; i < bufs.size(); i++)
    {
        if (!LoadCommon(bufs[i], sizes[i]))
            return false;
    }

    return true;
}

uint8_t *HciFile::Save(size_t *psize)
{
    size_t size = sizeof(HCIHeader) + entries.size() * sizeof(HCIEntry);

    uint8_t *buf = new uint8_t[size];
    memset(buf, 0, size);

    HCIHeader *hdr = (HCIHeader *)buf;

    hdr->signature = HCI_SIGNATURE;
    hdr->endianess_check = val16(0xFFFE);
    hdr->num_entries = (uint32_t)entries.size();
    hdr->entries_offset = sizeof(HCIHeader);

    HCIEntry *file_entries = (HCIEntry *)(hdr+1);

    for (size_t i = 0; i < entries.size(); i++)
    {
        const HciEntry &entry = entries[i];

        file_entries[i].char_id = entry.char_id;
        file_entries[i].costume_id = entry.costume_id;
        file_entries[i].state1 = entry.state1;
        file_entries[i].state2 = entry.state2;
        file_entries[i].emb_index = entry.emb_index;
    }

    *psize = size;
    return buf;
}

TiXmlDocument *HciFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();
    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("HCI");

    for (const HciEntry &entry : entries)
        entry.Decompile(root);

    doc->LinkEndChild(root);
    return doc;
}

bool HciFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "HCI");

    if (!root)
    {
        DPRINTF("Cannot find\"HCI\" in xml.\n");
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "HciEntry")
        {
            HciEntry entry;

            if (!entry.Compile(elem))
                return false;

            entries.push_back(entry);
        }
    }

    return true;
}

size_t HciFile::FindEntriesByCharID(uint32_t char_id, std::vector<HciEntry *> &hci_entries)
{
    hci_entries.clear();

    for (HciEntry &entry : entries)
    {
        if (entry.char_id == char_id)
        {
            hci_entries.push_back(&entry);
        }
    }

    return hci_entries.size();
}

bool HciFile::AddEntry(const HciEntry &entry, bool unique_char_id)
{
    if (unique_char_id)
    {
        std::vector<HciEntry *> temp;

        if (FindEntriesByCharID(entry.char_id, temp) != 0)
            return false;
    }

    entries.push_back(entry);
    return true;
}

size_t HciFile::RemoveEntries(uint32_t char_id)
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




