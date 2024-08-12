#include "VlcFile.h"
#include "Xenoverse2.h"
#include "debug.h"

TiXmlElement *VlcEntry::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("VlcEntry");

    std::string comment, code;
    Xenoverse2::GetCharaCodeFromId(cms_id, code);

    if (code.length() > 0)
    {
        if (!Xenoverse2::GetCharaName(code, comment))
            comment = code;
    }

    if (comment.length() > 0)
         Utils::WriteComment(entry_root, comment);

    entry_root->SetAttribute("cms_id", Utils::UnsignedToString(cms_id, true));

    Utils::WriteParamFloat(entry_root, "X", x);
    Utils::WriteParamFloat(entry_root, "Y", y);
    Utils::WriteParamFloat(entry_root, "Z", z);

    Utils::WriteParamFloat(entry_root, "X2", x2);
    Utils::WriteParamFloat(entry_root, "Y2", y2);
    Utils::WriteParamFloat(entry_root, "Z2", z2);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool VlcEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "cms_id", &cms_id))
    {
        DPRINTF("%s: cms_id attribute is not optional.\n", FUNCNAME);
        return false;
    }

    if (!Utils::GetParamFloatWithMultipleNames(root, &x, "X", "L_00"))
        return false;

    if (!Utils::GetParamFloatWithMultipleNames(root, &y, "Y", "L_04"))
        return false;

    if (!Utils::GetParamFloatWithMultipleNames(root, &z, "Z", "L_08"))
        return false;

    if (!Utils::GetParamFloatWithMultipleNames(root, &x2, "X2", "R_00"))
        return false;

    if (!Utils::GetParamFloatWithMultipleNames(root, &y2, "Y2", "R_04"))
        return false;

    if (!Utils::GetParamFloatWithMultipleNames(root, &z2, "Z2", "R_08"))
        return false;

    return true;
}

VlcFile::VlcFile()
{
    this->big_endian = false;
}

VlcFile::~VlcFile()
{

}

TiXmlDocument *VlcFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("VLC");

    for (const VlcEntry &entry : entries)
        entry.Decompile(root);

    doc->LinkEndChild(root);
    return doc;
}

bool VlcFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "VLC");

    if (!root)
    {
        DPRINTF("%s: Cannot find\"VLC\" in xml.\n", FUNCNAME);
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "VlcEntry")
        {
            VlcEntry entry;

            if (!entry.Compile(elem))
                return false;

            entries.push_back(entry);
        }
    }

    return true;
}

void VlcFile::Reset()
{
    entries.clear();
}

bool VlcFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (size < sizeof(VLCHeader))
        return false;

    const VLCHeader *hdr = (const VLCHeader *)buf;
    buf += sizeof(VLCHeader); size -= sizeof(VLCHeader);

    if (size != 2*hdr->num_entries*sizeof(VLCEntry))
    {
        DPRINTF("%s: Load error -> possible update of file format.\n", FUNCNAME);
        return false;
    }

    entries.resize(hdr->num_entries);

    // Left side
    for (size_t i = 0; i < entries.size(); i++)
    {
        const VLCEntry *in = (const VLCEntry *)buf;
        entries[i].x = in->x;
        entries[i].y = in->y;
        entries[i].z = in->z;
        entries[i].cms_id = (uint32_t)in->cms_id;

        buf += sizeof(VLCEntry);
    }

    // Right side
    for (size_t i = 0; i < entries.size(); i++)
    {
        const VLCEntry *in = (const VLCEntry *)buf;
        VlcEntry *entry = FindEntry((uint32_t)in->cms_id);
        if (!entry)
        {
            DPRINTF("%s: Error, no rigt side for character cms=0x%x\n", FUNCNAME, (uint32_t)in->cms_id);
            return false;
        }

        entry->x2 = in->x;
        entry->y2 = in->y;
        entry->z2 = in->z;

        buf += sizeof(VLCEntry);
    }

    return true;
}

uint8_t *VlcFile::Save(size_t *psize)
{
    *psize = sizeof(VLCHeader) + 2*entries.size()*sizeof(VLCEntry);
    uint8_t *buf = new uint8_t[*psize];
    memset(buf, 0, *psize);

    VLCHeader *hdr = (VLCHeader *)buf;
    hdr->num_entries = (uint32_t)entries.size();

    uint8_t *ptr = buf + sizeof(VLCHeader);

    // Left side
    for (const VlcEntry &entry : entries)
    {
        VLCEntry *out = (VLCEntry *)ptr;
        out->x = entry.x;
        out->y = entry.y;
        out->z = entry.z;
        out->cms_id = (float)entry.cms_id;

        ptr += sizeof(VLCEntry);
    }

    // Right side
    for (const VlcEntry &entry : entries)
    {
        VLCEntry *out = (VLCEntry *)ptr;
        out->x = entry.x2;
        out->y = entry.y2;
        out->z = entry.z2;
        out->cms_id = (float)entry.cms_id;

        ptr += sizeof(VLCEntry);
    }

    return buf;
}

VlcEntry *VlcFile::FindEntry(uint32_t cms_id)
{
    for (VlcEntry &entry : entries)
        if (entry.cms_id == cms_id)
            return &entry;

    return nullptr;
}

bool VlcFile::SetEntry(const VlcEntry &entry, bool add_if_needed)
{
    VlcEntry *existing = FindEntry(entry.cms_id);
    if (existing)
    {
        *existing = entry;
        return true;
    }

    if (!add_if_needed)
        return false;

    entries.push_back(entry);
    return true;
}

void VlcFile::RemoveEntry(uint32_t cms_id)
{
    for (size_t i = 0; i < entries.size(); i++)
    {
        if (entries[i].cms_id == cms_id)
        {
            entries.erase(entries.begin()+i);
            i--;
        }
    }
}
