#include "IkdFile.h"
#include "Xenoverse2.h"
#include "debug.h"

TiXmlElement *IKDEntry::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("IkdEntry");

    std::string comment = Xenoverse2::GetCharaAndCostumeName(cms_id, costume_id, 0, true);

    if (comment.length() != 0)
    {
         Utils::WriteComment(entry_root, comment);
    }

    entry_root->SetAttribute("cms_id", Utils::UnsignedToString(cms_id, true));
    entry_root->SetAttribute("costume", costume_id);

    Utils::WriteParamMultipleFloats(entry_root, "F_08", std::vector<float>(unk_08, unk_08+12));
    Utils::WriteParamUnsigned(entry_root, "U_34", unk_34, true);
    Utils::WriteParamUnsigned(entry_root, "U_38", unk_38, true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif

bool IKDEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "cms_id", &cms_id))
    {
        DPRINTF("%s: cms_id attribute is not optional.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "costume", &costume_id))
    {
        DPRINTF("%s: costume attribute is not optional.\n", FUNCNAME);
        return false;
    }

    if (!Utils::GetParamMultipleFloats(root, "F_08", unk_08, 12))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_34", &unk_34))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_38", &unk_38))
        return false;

    return true;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

IkdFile::IkdFile()
{
    this->big_endian = false;
}

IkdFile::~IkdFile()
{

}

void IkdFile::Reset()
{
    entries.clear();
}

bool IkdFile::Load(const uint8_t *buf, size_t size)
{
    if (!buf || size < sizeof(IKDHeader))
        return false;

    const IKDHeader *hdr = (const IKDHeader *)buf;
    buf += sizeof(IKDHeader); size -= sizeof(IKDHeader);

    if ((size / sizeof(IKDEntry)) != (size_t)hdr->num_entries)
    {
        DPRINTF("%s: Error, possible update of file format.\n", FUNCNAME);
        return false;
    }

    entries.resize(hdr->num_entries);
    for (IkdEntry &entry : entries)
    {
        memcpy(&entry, buf, sizeof(IKDEntry));
        buf += sizeof(IKDEntry);
    }

    return true;
}

uint8_t *IkdFile::Save(size_t *psize)
{
    *psize = sizeof(IKDHeader) + entries.size()*sizeof(IKDEntry);
    uint8_t *buf = new uint8_t[*psize];

    IKDHeader *hdr = (IKDHeader *)buf;
    hdr->signature = IKD_SIGNATURE;
    hdr->endianess_check = val16(0xFFFE);
    hdr->header_size = (uint16_t)sizeof(IKDHeader);
    hdr->num_entries = (uint32_t)entries.size();
    hdr->data_start = (uint32_t)sizeof(IKDHeader);

    uint8_t *ptr = buf + sizeof(IKDHeader);
    for (const IkdEntry &entry : entries)
    {
        memcpy(ptr, &entry, sizeof(IKDEntry));
        ptr += sizeof(IKDEntry);
    }

    return buf;
}

TiXmlDocument *IkdFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("IKD");

    for (const IkdEntry &entry : entries)
        entry.Decompile(root);

    doc->LinkEndChild(root);
    return doc;
}

bool IkdFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "IKD");

    if (!root)
    {
        DPRINTF("%s: Cannot find\"IKD\" in xml.\n", FUNCNAME);
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "IkdEntry")
        {
            IkdEntry entry;

            if (!entry.Compile(elem))
                return false;

            entries.push_back(entry);
        }
    }

    return true;
}

IkdEntry *IkdFile::FindEntry(uint32_t char_id, uint32_t costume_id)
{
    for (IkdEntry &entry : entries)
    {
        if (entry.cms_id == char_id && entry.costume_id == costume_id)
            return &entry;
    }

    return nullptr;
}

size_t IkdFile::FindEntries(uint32_t char_id, std::vector<IkdEntry *> &ret)
{
    ret.clear();
    for (IkdEntry &entry : entries)
        if (entry.cms_id == char_id)
            ret.push_back(&entry);

    return ret.size();
}

size_t IkdFile::RemoveAllReferencesToChar(uint32_t char_id)
{
    size_t count = 0;

    for (size_t i = 0; i < entries.size(); i++)
    {
        const IkdEntry &entry = entries[i];

        if (entry.cms_id == char_id)
        {
            entries.erase(entries.begin()+i);
            i--;
            count++;
        }
    }

    return count;
}
