#include "AurFile.h"
#include "Xenoverse2.h"
#include "debug.h"

TiXmlElement *AurEffect::Decompile(TiXmlNode *root, const std::vector<std::string> &types) const
{
    TiXmlElement *entry_root = new TiXmlElement("Effect");

    if (type_index >= types.size())
    {
        entry_root->SetAttribute("indexAuraType", type_index);
    }
    else
    {
        entry_root->SetAttribute("auraType", types[type_index]);
    }

    entry_root->SetAttribute("idEffect", Utils::UnsignedToString(id, true));

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool AurEffect::Compile(const TiXmlElement *root, const std::vector<std::string> &types)
{
    std::string type;
    type_index = -1;

    if (Utils::ReadAttrString(root, "auraType", type))
    {
        for (uint32_t i = 0; i < (uint32_t)types.size(); i++)
        {
            if (types[i] == type)
            {
                type_index = i;
                break;
            }
        }
    }

    if (type_index == (uint32_t)-1)
    {
        if (!Utils::ReadAttrUnsigned(root, "indexAuraType", &type_index))
            return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "idEffect", &id))
        return false;

    return true;
}

TiXmlElement *AurAura::Decompile(TiXmlNode *root, const std::vector<std::string> &types) const
{
    TiXmlElement *entry_root = new TiXmlElement("Aura");

    entry_root->SetAttribute("id", Utils::UnsignedToString(id, true));
    entry_root->SetAttribute("unknow_0", Utils::UnsignedToString(unk_04, true));

    TiXmlElement *effects_root = new TiXmlElement("Effects");
    entry_root->LinkEndChild(effects_root);

    for (const AurEffect &effect : effects)
    {
        effect.Decompile(effects_root, types);
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool AurAura::Compile(const TiXmlElement *root, const std::vector<std::string> &types)
{
    effects.clear();

    if (!Utils::ReadAttrUnsigned(root, "id", &id))
        return false;

    if (!Utils::ReadAttrUnsigned(root, "unknow_0", &unk_04))
        unk_04 = 0;

    const TiXmlElement *effects_root;

    if (Utils::GetElemCount(root, "Effects", &effects_root) == 0)
        return false;

    for (const TiXmlElement *elem = effects_root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Effect")
        {
            AurEffect effect;

            if (!effect.Compile(elem, types))
                return false;

            effects.push_back(effect);
        }
    }

    return true;
}

TiXmlElement *AurCharaLink::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("CharacLink");

    std::string name = Xenoverse2::GetCharaAndCostumeName(char_id, costume_id, 0, true);
    if (name.length() != 0)
    {
        TiXmlComment *comment = new TiXmlComment();
        comment->SetValue(name);

        root->LinkEndChild(comment);
    }

    entry_root->SetAttribute("idCharac", Utils::UnsignedToString(char_id, true));
    entry_root->SetAttribute("idCostume", costume_id);
    entry_root->SetAttribute("idAura", Utils::UnsignedToString(aura_id, true));
    entry_root->SetAttribute("glare", glare ? "true" : "false");

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool AurCharaLink::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "idCharac", &char_id))
        return false;

    if (!Utils::ReadAttrUnsigned(root, "idCostume", &costume_id))
        return false;

    if (!Utils::ReadAttrUnsigned(root, "idAura", &aura_id))
        return false;

    std::string glare_str;

    if (!Utils::ReadAttrString(root, "glare", glare_str))
        return false;

    glare_str = Utils::ToLowerCase(glare_str);
    glare = (glare_str == "true") || (glare_str == "1");

    return true;
}

AurFile::AurFile()
{
    this->big_endian = false;
}

AurFile::~AurFile()
{

}

void AurFile::Reset()
{
    auras.clear();
    types.clear();
    chara_links.clear();
}

bool AurFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(AURHeader))
        return false;

    const AURHeader *hdr = (const AURHeader *)buf;

    if (hdr->signature != AUR_SIGNATURE && memcmp(buf+1, "AUR", 3) != 0)
        return false;

    const AURAura *file_auras = (const AURAura *)GetOffsetPtr(hdr, hdr->auras_offset);
    auras.resize(hdr->auras_count);

    for (size_t i = 0; i < auras.size(); i++)
    {
        AurAura &aura = auras[i];

        aura.id = file_auras[i].id;
        aura.unk_04 = file_auras[i].unk_04;

        if (aura.id != i)
        {
            DPRINTF("%s: aura id 0x%x doesn't match its index (index=0x%Ix). This could cause problems.\n", FUNCNAME, aura.id, i);
            return false;
        }

        const AUREffect *file_effects = (const AUREffect *)GetOffsetPtr(hdr, file_auras[i].effects_address);
        aura.effects.resize(file_auras[i].effects_count);

        for (size_t j = 0; j < aura.effects.size(); j++)
        {
            AurEffect &effect = aura.effects[j];

            effect.type_index = file_effects[j].type_index;
            effect.id = file_effects[j].id;
        }
    }

    const uint32_t *file_types = (const uint32_t *)GetOffsetPtr(hdr, hdr->types_offset);
    types.resize(hdr->types_count);

    for (size_t i = 0; i < types.size(); i++)
    {
        types[i] = GetString(buf, file_types[i]);
    }

    const AURCharaLink *file_links = (const AURCharaLink *)GetOffsetPtr(hdr, hdr->chara_links_offset);
    chara_links.resize(hdr->chara_links_count);

    for (size_t i = 0; i < chara_links.size(); i++)
    {
        AurCharaLink &link = chara_links[i];

        link.char_id = file_links[i].char_id;
        link.costume_id = file_links[i].costume_id;
        link.aura_id = file_links[i].aura_id;
        link.glare = (file_links[i].glare != 0);
    }

    return true;
}

size_t AurFile::CalculateFileSize()
{
    size_t size = sizeof(AURHeader);

    size +=  auras.size()*sizeof(AURAura);

    for (const AurAura &aura : auras)
    {
        size += aura.effects.size()*sizeof(AUREffect);
    }

    size += types.size()*sizeof(uint32_t);

    for (const std::string &type : types)
    {
        if (type.length() > 0)
        {
            size += type.length() + 1;
        }
    }

    size = Utils::Align2(size, 0x10);
    size += chara_links.size() * sizeof(AURCharaLink);
    return size;
}


uint8_t *AurFile::Save(size_t *psize)
{
    size_t size = CalculateFileSize();
    uint8_t *buf = new uint8_t[size];
    memset(buf, 0, size);

    AURHeader *hdr = (AURHeader *)buf;

    hdr->signature = AUR_SIGNATURE;
    hdr->endianess_check = val16(0xFFFE);
    hdr->header_size = sizeof(AURHeader);

    hdr->auras_offset = sizeof(AURHeader);
    hdr->auras_count = (uint32_t)auras.size();

    AURAura *file_auras = (AURAura *)(hdr+1);
    AUREffect *file_effect = (AUREffect *)(file_auras+auras.size());

    for (size_t i = 0; i < auras.size(); i++)
    {
        const AurAura &aura = auras[i];

        file_auras[i].id = aura.id;
        file_auras[i].unk_04 = aura.unk_04;
        file_auras[i].effects_address = (aura.effects.size() == 0) ? 0 : Utils::DifPointer(file_effect, buf);
        file_auras[i].effects_count = (uint32_t)aura.effects.size();

        for (const AurEffect &effect : aura.effects)
        {
            file_effect->type_index = effect.type_index;
            file_effect->id = effect.id;
            file_effect++;
        }
    }

    uint32_t *file_types = (uint32_t*)file_effect;
    char *types_str = (char *)(file_types + types.size());

    hdr->types_offset = Utils::DifPointer(file_types, buf);
    hdr->types_count = (uint32_t)types.size();

    for (size_t i = 0; i < types.size(); i++)
    {
        const std::string &type = types[i];

        if (type.length() != 0)
        {
            file_types[i] = Utils::DifPointer(types_str, buf);
            strcpy(types_str, type.c_str());
            types_str += type.length()+1;
        }
    }

    AURCharaLink *file_links = (AURCharaLink *)(buf + Utils::Align2(Utils::DifPointer(types_str, buf), 0x10));

    hdr->chara_links_offset = Utils::DifPointer(file_links, buf);
    hdr->chara_links_count = (uint32_t)chara_links.size();

    for (size_t i = 0; i < chara_links.size(); i++)
    {
        const AurCharaLink &link = chara_links[i];

        file_links[i].char_id = link.char_id;
        file_links[i].costume_id = link.costume_id;
        file_links[i].aura_id = link.aura_id;
        file_links[i].glare = link.glare;
    }

    *psize = size;
    return buf;
}

TiXmlDocument *AurFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();
    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("AUR");

    TiXmlElement *types_root = new TiXmlElement("AuraTypes");
    root->LinkEndChild(types_root);

    for (const std::string &type : types)
    {
        TiXmlElement *entry_type = new TiXmlElement("AuraType");
        entry_type->SetAttribute("name", type);
        types_root->LinkEndChild(entry_type);
    }

    TiXmlElement *auras_root = new TiXmlElement("Auras");
    root->LinkEndChild(auras_root);

    for (const AurAura &aura : auras)
        aura.Decompile(auras_root, types);

    TiXmlElement *links_root = new TiXmlElement("CharacLinks");
    root->LinkEndChild(links_root);

    for (const AurCharaLink &link : chara_links)
        link.Decompile(links_root);

    doc->LinkEndChild(root);
    return doc;
}

bool AurFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "AUR");

    if (!root)
    {
        DPRINTF("Cannot find\"AUR\" in xml.\n");
        return false;
    }

    const TiXmlElement *types_root;
    if (Utils::GetElemCount(root, "AuraTypes", &types_root) == 0)
        return false;

    for (const TiXmlElement *elem = types_root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "AuraType")
        {
            std::string type;

            if (!Utils::ReadAttrString(elem, "name", type))
                return false;

            types.push_back(type);
        }
    }

    const TiXmlElement *auras_root;
    if (Utils::GetElemCount(root, "Auras", &auras_root) == 0)
        return false;

    for (const TiXmlElement *elem = auras_root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Aura")
        {
            AurAura aura;

            if (!aura.Compile(elem, types))
                return false;

            auras.push_back(aura);
        }
    }

    const TiXmlElement *links_root;
    if (Utils::GetElemCount(root, "CharacLinks", &links_root) == 0)
        return false;

    for (const TiXmlElement *elem = links_root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "CharacLink")
        {
            AurCharaLink link;

            if (!link.Compile(elem))
                return false;

            chara_links.push_back(link);
        }
    }

    return true;
}

size_t AurFile::FindCharaLinks(uint32_t char_id, std::vector<AurCharaLink *> &links)
{
    links.clear();

    for (AurCharaLink &link : chara_links)
    {
        if (link.char_id == char_id)
        {
            links.push_back(&link);
        }
    }

    return links.size();
}

bool AurFile::AddCharaLink(const AurCharaLink &link, bool unique_char_id)
{
    if (unique_char_id)
    {
        std::vector<AurCharaLink *> temp;

        if (FindCharaLinks(link.char_id, temp) != 0)
            return false;
    }

    chara_links.push_back(link);
    return true;
}

size_t AurFile::RemoveCharaLinks(uint32_t char_id)
{
    size_t removed = 0;

    for (size_t i = 0; i < chara_links.size(); i++)
    {
        if (chara_links[i].char_id == char_id)
        {
            chara_links.erase(chara_links.begin()+i);
            i--;
            removed++;
        }
    }

    return removed;
}

AurAura *AurFile::FindAuraByID(uint32_t id)
{
    for (AurAura &aura : auras)
    {
        if (aura.id == id)
            return &aura;
    }

    return nullptr;
}

bool AurFile::AddAura(AurAura &aura)
{
    aura.id = (uint32_t) auras.size();
    auras.push_back(aura);
    return true;
}

/*void AurFile::RemoveAura(uint32_t id)
{
    for (size_t i = 0; i < auras.size(); i++)
    {
        if (auras[i].id == id)
        {
            auras.erase(auras.begin()+i);
            i--;
        }
    }

    for (size_t i = 0; i < chara_links.size(); i++)
    {
        if (chara_links[i].aura_id == id)
        {
            chara_links.erase(chara_links.begin()+i);
            i--;
        }
    }
}*/

void AurFile::RemoveAuraIfLast(uint32_t id)
{
    if (auras.back().id == id)
    {
        auras.pop_back();

        for (size_t i = 0; i < chara_links.size(); i++)
        {
            if (chara_links[i].aura_id == id)
            {
                chara_links.erase(chara_links.begin()+i);
                i--;
            }
        }
    }
}
