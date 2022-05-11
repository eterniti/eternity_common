#include "EmmFile.h"
#include "debug.h"

void EmmMaterialParameter::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("EMMParameter");

    entry_root->SetAttribute("name", name);
    entry_root->SetAttribute("type", type);

    if (type == 0 || type == 0x10000)
    {
        entry_root->SetAttribute("value", Utils::FloatToString(value.value_float));
    }
    else
    {
        entry_root->SetAttribute("value", value.value_u32);
    }

    root->LinkEndChild(entry_root);
}

bool EmmMaterialParameter::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrString(root, "name", name))
    {
        DPRINTF("%s: Cannot get \"name\" attribute.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "type", &type))
    {
        DPRINTF("%s: Cannot get \"type\" attribute.\n", FUNCNAME);
        return false;
    }

    if (type == 0 || type == 0x10000)
    {
        if (!Utils::ReadAttrFloat(root, "value", &value.value_float))
        {
            DPRINTF("%s: Cannot get \"value\" attribute.\n", FUNCNAME);
            return false;
        }
    }
    else
    {
        if (!Utils::ReadAttrUnsigned(root, "value", &value.value_u32))
        {
            DPRINTF("%s: Cannot get \"value\" attribute.\n", FUNCNAME);
            return false;
        }
    }

    return true;
}

void EmmMaterial::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("EMMMaterial");

    entry_root->SetAttribute("name", name);
    entry_root->SetAttribute("shader_name", shader_name);

    for (const EmmMaterialParameter &p : parameters)
    {
        p.Decompile(entry_root);
    }

    root->LinkEndChild(entry_root);
}

bool EmmMaterial::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrString(root, "name", name))
    {
        DPRINTF("%s: Cannot get \"name\" attribute.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrString(root, "shader_name", shader_name))
    {
        DPRINTF("%s: Cannot get \"shader_name\" attribute.\n", FUNCNAME);
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        std::string name = elem->ValueStr();

        if (name == "EMMParameter")
        {
            EmmMaterialParameter param;

            if (!param.Compile(elem))
                return false;

            parameters.push_back(param);
        }
    }

    return true;
}

EmmFile::EmmFile()
{
    this->big_endian = false;
    Reset();
}

EmmFile::EmmFile(uint8_t *buf, size_t size)
{
    Load(buf, size);
}

EmmFile::~EmmFile()
{
    Reset();
}

void EmmFile::Reset()
{
    materials.clear();
}

bool EmmFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    EMMHeader *hdr = (EMMHeader *)buf;

    if (size < sizeof(EMMHeader) || hdr->signature != EMM_SIGNATURE)
        return false;

    this->big_endian = (buf[4] != 0xFE);
    assert(hdr->unk_08 == 0);

    EMMSubHeader *hdr2 = (EMMSubHeader *)GetOffsetPtr(hdr, hdr->material_base_address);
    assert(hdr2->unk_02 == 0);

    for (uint16_t i = 0; i < val16(hdr2->mat_count); i++)
    {
        EmmMaterial material;

        EMMMaterial *mn = (EMMMaterial *)GetOffsetPtr(hdr2, hdr2->offsets, i);
        assert(mn->unk_42 == 0);

        if (strlen(mn->name) > 31 || strlen(mn->shader_name) > 31)
            return false;

        material.name = mn->name;
        material.shader_name = mn->shader_name;

        EMMMaterialParameter *parameters = (EMMMaterialParameter *)GetOffsetPtr(mn, sizeof(EMMMaterial), true);

        for (uint16_t j = 0; j < val16(mn->param_count); j++)
        {
            EmmMaterialParameter parameter;

            if (strlen(parameters[j].name) > 31)
                return false;

            parameter.name = parameters[j].name;
            parameter.type = val32(parameters[j].type);

            if (big_endian)
            {
                parameter.type = ((parameter.type&0xFFFF) << 16) | ((parameter.type & 0xFFFF0000) >> 16);
            }

            parameter.value.value_u32 = val32(parameters[j].value);

            material.parameters.push_back(parameter);
        }

        materials.push_back(material);
    }

    return true;
}

unsigned int EmmFile::CalculateFileSize()
{
    unsigned int file_size = sizeof(EMMHeader) + sizeof(EMMSubHeader);

    if (materials.size() > 0)
    {
        file_size += (materials.size() - 1) * sizeof(uint32_t);
    }

    for (const EmmMaterial &m : materials)
    {
        file_size += sizeof(EMMMaterial);
        file_size += m.parameters.size() * sizeof(EMMMaterialParameter);
    }

    return file_size;
}

uint8_t *EmmFile::Save(size_t *psize)
{
    unsigned int file_size = CalculateFileSize();
    uint32_t offset;

    uint8_t *buf = new uint8_t[file_size];
    memset(buf, 0, file_size);

    EMMHeader *hdr = (EMMHeader *)buf;

    hdr->signature = EMM_SIGNATURE;
    hdr->endianess_check = val16(0xFFFE);
    hdr->header_size = val16(sizeof(EMMHeader));
    hdr->material_base_address = val32(sizeof(EMMHeader));

    offset = sizeof(EMMHeader);

    EMMSubHeader *hdr2 = (EMMSubHeader *)GetOffsetPtr(buf, offset, true);

    assert(materials.size() < 65536);
    hdr2->mat_count = val16(materials.size());

    offset += sizeof(EMMSubHeader);

    if (materials.size() > 0)
    {
        offset += (materials.size() - 1) * sizeof(uint32_t);
    }

    for (size_t i = 0; i < materials.size(); i++)
    {
        const EmmMaterial &material = materials[i];
        EMMMaterial *mn = (EMMMaterial *)GetOffsetPtr(buf, offset, true);

        hdr2->offsets[i] = val32(offset - sizeof(EMMHeader));
        strcpy(mn->name, material.name.c_str());
        strcpy(mn->shader_name, material.shader_name.c_str());

        assert(material.parameters.size() < 65536);
        mn->param_count = val16(material.parameters.size());

        offset += sizeof(EMMMaterial);

        for (size_t j = 0; j < material.parameters.size(); j++)
        {
            const EmmMaterialParameter &parameter = material.parameters[j];
            EMMMaterialParameter *pn = (EMMMaterialParameter *)GetOffsetPtr(buf, offset, true);

            strcpy(pn->name, parameter.name.c_str());
            pn->type = val32(parameter.type);

            if (big_endian)
            {
                pn->type = ((pn->type&0xFFFF) << 16) | ((pn->type & 0xFFFF0000) >> 16);
            }

            pn->value = val32(parameter.value.value_u32);

            offset += sizeof(EMMMaterialParameter);
        }
    }

    assert(offset == file_size);

    *psize = file_size;
    return buf;
}

uint16_t EmmFile::AppendMaterial(const EmmFile &other, uint16_t idx)
{
    if (idx >= other.materials.size())
        return (uint16_t)-1;

    return AppendMaterial(other.materials[idx]);
}

TiXmlDocument *EmmFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("EMM");

    for (const EmmMaterial &m : materials)
    {
        m.Decompile(root);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool EmmFile::Compile(TiXmlDocument *doc, bool big_endian)
{
    Reset();
    this->big_endian = big_endian;

    TiXmlHandle handle(doc);
    TiXmlElement *root = NULL;;

    for (TiXmlElement *elem = handle.FirstChildElement().Element(); elem != NULL; elem = elem->NextSiblingElement())
    {
        std::string name = elem->ValueStr();

        if (name == "EMM")
        {
            root = elem;
            break;
        }
    }

    if (!root)
    {
        DPRINTF("Cannot find\"EMM\" in xml.\n");
        return false;
    }

    for (TiXmlElement *elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
    {
        std::string name = elem->ValueStr();

        if (name == "EMMMaterial")
        {
            EmmMaterial material;

            if (!material.Compile(elem))
                return false;

            materials.push_back(material);
        }
    }

    return true;
}
