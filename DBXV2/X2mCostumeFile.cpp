#include "X2mCostumeFile.h"
#include "X2mFile.h"
#include "debug.h"

TiXmlElement *X2mCostumeEntry::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("X2mCostumeEntry");

    Utils::WriteParamGUID(entry_root, "GUID", guid);
    Utils::WriteParamMultipleUnsigned(entry_root, "PARTSETS", partsets);
    Utils::WriteParamMultipleUnsigned(entry_root, "RACES", races);

    if (idb_entries.size() > 0)
        Utils::WriteParamMultipleUnsigned(entry_root, "IDB_ENTRIES", idb_entries, true);

    if (costume_types.size() > 0)
        Utils::WriteParamMultipleUnsigned(entry_root, "COSTUME_TYPES", costume_types);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool X2mCostumeEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamGUID(root, "GUID", guid))
        return false;

    if (!Utils::GetParamMultipleUnsigned(root, "PARTSETS", partsets))
        return false;

    if (partsets.size() == 0)
    {
        DPRINTF("%s: There must be at least one partset in PARTSETS.\n", FUNCNAME);
        return false;
    }

    if (!Utils::GetParamMultipleUnsigned(root, "RACES", races))
        return false;

    if (races.size() == 0)
    {
        DPRINTF("%s: There must be at least one race in RACES.\n", FUNCNAME);
        return false;
    }

    for (uint8_t race : races)
    {
        if (race >= X2M_CR_NUM)
        {
            DPRINTF("%s: Invalid race code %d\n", FUNCNAME, race);
            return false;
        }
    }

    if (!Utils::ReadParamMultipleUnsigned(root, "IDB_ENTRIES", idb_entries))
        idb_entries.clear();

    if (!Utils::ReadParamMultipleUnsigned(root, "COSTUME_TYPES", costume_types))
        costume_types.clear();

    if (costume_types.size() != idb_entries.size())
    {
        DPRINTF("%s: Number of costume types must be same than number of idb entries.\n", FUNCNAME);
        return false;
    }

    for (uint8_t type : costume_types)
    {
        if (type >= COSTUME_MAX)
        {
            DPRINTF("%s: Invalid value %d for costume_type\n", FUNCNAME, type);
            return false;
        }
    }

    return true;
}

TiXmlElement *X2mBody::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("X2mBody");

    Utils::WriteParamGUID(entry_root, "GUID", guid);
    Utils::WriteParamUnsigned(entry_root, "RACE", race);
    Utils::WriteParamSigned(entry_root, "BODY_ID", id);

    if (!body.IsEmpty())
        body.Decompile(entry_root, id);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool X2mBody::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamGUID(root, "GUID", guid))
        return false;

    if (!Utils::GetParamUnsigned(root, "RACE", &race))
        return false;

    if (race >= X2M_CR_NUM)
    {
        DPRINTF("%s: Invalid race code %d\n", FUNCNAME, race);
        return false;
    }

    if (!Utils::GetParamSigned(root, "BODY_ID", &id))
        return false;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        const std::string &name = elem->ValueStr();

        if (name == "Body")
        {
            if (!body.Compile(elem))
                return false;
        }
    }

    return true;
}

TiXmlElement *X2mSuperSoul::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("X2mSuperSoul");

    entry_root->SetAttribute("id", idb_id);
    Utils::WriteParamGUID(entry_root, "GUID", guid);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool X2mSuperSoul::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "id", &idb_id))
        return false;

    if (!Utils::GetParamGUID(root, "GUID", guid))
        return false;

    return true;
}

X2mCostumeFile::X2mCostumeFile()
{
    this->big_endian = false;
}

X2mCostumeFile::~X2mCostumeFile()
{

}

void X2mCostumeFile::Reset()
{
    costumes.clear();
    bodies.clear();
    super_souls.clear();
}

TiXmlDocument *X2mCostumeFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("X2mCostumeFile");
    Utils::WriteComment(root, "This file is used by XV2 Mods Installer to track installed costumes and super soul. DON'T DELETE THIS FILE OR Xv2 Mods Installer won't be able to update/uninstall costumes/supersouls properly.");

    for (const X2mCostumeEntry &costume : costumes)
    {
        costume.Decompile(root);
    }

    for (const X2mBody &body : bodies)
    {
        body.Decompile(root);
    }

    for (const X2mSuperSoul &ss : super_souls)
    {
        ss.Decompile(root);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool X2mCostumeFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "X2mCostumeFile");

    if (!root)
    {
        DPRINTF("Cannot find\"X2mCostumeFile\" in xml.\n");
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        const std::string &name = elem->ValueStr();

        if (name == "X2mCostumeEntry")
        {
            X2mCostumeEntry costume;

            if (!costume.Compile(elem))
            {
                Reset();
                return false;
            }

            costumes.push_back(costume);
        }
        else if (name == "X2mBody")
        {
            X2mBody body;

            if (!body.Compile(elem))
            {
                Reset();
                return false;
            }

            bodies.push_back(body);
        }
        else if (name == "X2mSuperSoul")
        {
            X2mSuperSoul ss;

            if (!ss.Compile(elem))
            {
                Reset();
                return false;
            }

            super_souls.push_back(ss);
        }
    }

    return true;
}

X2mCostumeEntry *X2mCostumeFile::FindCostume(const uint8_t *guid)
{
    for (X2mCostumeEntry &costume : costumes)
    {
        if (memcmp(costume.guid, guid, sizeof(costume.guid)) == 0)
            return &costume;
    }

    return nullptr;
}

X2mCostumeEntry *X2mCostumeFile::FindCostumeByPartSet(uint16_t partset)
{
    for (X2mCostumeEntry &costume : costumes)
    {
        for (uint16_t ps : costume.partsets)
        {
            if (ps == partset)
                return &costume;
        }
    }

    return nullptr;
}

void X2mCostumeFile::AddCostume(const X2mCostumeEntry &entry)
{
    X2mCostumeEntry *existing = FindCostume(entry.guid);

    if (existing)
    {
        *existing = entry;
    }
    else
    {
        costumes.push_back(entry);
    }
}

void X2mCostumeFile::RemoveCostume(const uint8_t *guid)
{
    for (size_t i = 0; i < costumes.size(); i++)
    {
        X2mCostumeEntry &costume = costumes[i];

        if (memcmp(costume.guid, guid, sizeof(costume.guid)) == 0)
        {
            costumes.erase(costumes.begin()+i);
            i--;
        }
    }
}

size_t X2mCostumeFile::FindBody(const uint8_t *guid, std::vector<X2mBody *> &found_bodies)
{
    found_bodies.clear();

    //DPRINTF("In FindBody.\n");

    for (X2mBody &body : bodies)
    {
        //DPRINTF("Compare %s to %s\n", Utils::GUID2String(guid).c_str(), Utils::GUID2String(body.guid).c_str());

        if (memcmp(body.guid, guid, sizeof(body.guid)) == 0)
        {
            found_bodies.push_back(&body);
        }
    }

    return found_bodies.size();
}

X2mBody *X2mCostumeFile::FindBodyById(int id)
{
    for (X2mBody &body : bodies)
    {
        if (body.id == id)
            return &body;
    }

    return nullptr;
}

bool X2mCostumeFile::AddBody(const X2mBody &body)
{
    X2mBody *existing_body = FindBodyById(body.id);

    if (existing_body)
    {
        if (memcmp(body.guid, existing_body, sizeof(body.guid)) == 0)
        {
            *existing_body = body;
            return true;
        }
        else
        {
            DPRINTF("%s: Body id %d is already in use!\n", FUNCNAME, body.id);
            return false;
        }
    }

    bodies.push_back(body);
    return true;
}

size_t X2mCostumeFile::RemoveBodiesFromMod(const uint8_t *guid)
{
    size_t count = 0;

    for (size_t i = 0; i < bodies.size(); i++)
    {
        X2mBody &body = bodies[i];

        if (memcmp(body.guid, guid, sizeof(body.guid)) == 0)
        {
            bodies.erase(bodies.begin()+i);
            i--;
            count++;
        }
    }

    return count;
}

X2mSuperSoul *X2mCostumeFile::FindSuperSoul(const uint8_t *guid)
{
    for (X2mSuperSoul &ss : super_souls)
    {
        if (memcmp(ss.guid, guid, sizeof(ss.guid)) == 0)
            return &ss;
    }

    return nullptr;
}

void X2mCostumeFile::AddSuperSoul(const X2mSuperSoul &ss)
{
    X2mSuperSoul *existing = FindSuperSoul(ss.guid);

    if (existing)
    {
        *existing = ss;
    }
    else
    {
        super_souls.push_back(ss);
    }
}

void X2mCostumeFile::RemoveSuperSoul(const uint8_t *guid)
{
    for (size_t i = 0; i < super_souls.size(); i++)
    {
        X2mSuperSoul &ss = super_souls[i];

        if (memcmp(ss.guid, guid, sizeof(ss.guid)) == 0)
        {
            super_souls.erase(super_souls.begin()+i);
            i--;
        }
    }
}
