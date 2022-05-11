#include <algorithm>
#include "Xv2PreBakedFile.h"
#include "debug.h"

#define CUS_DATA_ID_START   0x30

#define BEHAVIOUR_MAX	0x1A

TiXmlElement *BodyShape::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("BodyShape");

    entry_root->SetAttribute("cms_id", Utils::UnsignedToString(cms_entry, true));
    entry_root->SetAttribute("body_shape", body_shape);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BodyShape::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "cms_id", &cms_entry))
    {
        DPRINTF("%s: cms_id attribute is not optional.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "body_shape", &body_shape))
    {
        DPRINTF("%s: body_shape attribute is not optional.\n", FUNCNAME);
        return false;
    }

    return true;
}

TiXmlElement *CusAuraData::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("CusAuraData");

    entry_root->SetAttribute("cus_aura_id", Utils::UnsignedToString(cus_aura_id, true));
    entry_root->SetAttribute("aur_aura_id", Utils::UnsignedToString(aur_aura_id, true));

    Utils::WriteParamUnsigned(entry_root, "BEHAVIOUR_11", behaviour_11, true);
    Utils::WriteParamUnsigned(entry_root, "INTEGER_2", integer_2);
    Utils::WriteParamUnsigned(entry_root, "BEHAVIOUR_10", behaviour_10, true);
    Utils::WriteParamUnsigned(entry_root, "INTEGER_3", integer_3);
    Utils::WriteParamBoolean(entry_root, "FORCE_TELEPORT", force_teleport);
    Utils::WriteParamUnsigned(entry_root, "BEHAVIOUR_13", behaviour_13, true);    

    if (!force_teleport && behaviour_66 <= BEHAVIOUR_MAX)
        Utils::WriteParamUnsigned(entry_root, "BEHAVIOUR_66", behaviour_66);

    Utils::WriteParamUnsigned(entry_root, "REMOVE_HAIR_ACCESSORIES", remove_hair_accessories, true);
    Utils::WriteParamUnsigned(entry_root, "BCS_HAIR_COLOR", bcs_hair_color, bcs_hair_color==0xFFFFFFFF);
    Utils::WriteParamUnsigned(entry_root, "BCS_EYES_COLOR", bcs_eyes_color, bcs_eyes_color==0xFFFFFFFF);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool CusAuraData::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "cus_aura_id", &cus_aura_id))
    {
        DPRINTF("%s: cus_aura_id attribute is not optional.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "aur_aura_id", &aur_aura_id))
    {
        DPRINTF("%s: aur_aura_id attribute is not optional.\n", FUNCNAME);
        return false;
    }

    if (!Utils::GetParamUnsigned(root, "BEHAVIOUR_11", &behaviour_11))
        return false;

    if (behaviour_11 > BEHAVIOUR_MAX)
    {
        DPRINTF("%s: BEHAVIOUR_11 cannot be greater than 0x%x.\n", FUNCNAME, BEHAVIOUR_MAX);
        return false;
    }

    if (!Utils::GetParamUnsigned(root, "INTEGER_2", &integer_2))
        return false;

    if (Utils::ReadParamUnsigned(root, "BEHAVIOUR_10", &behaviour_10))
    {
        if (behaviour_10 > BEHAVIOUR_MAX)
        {
            DPRINTF("%s: BEHAVIOUR_10 cannot be greater than 0x%x.\n", FUNCNAME, BEHAVIOUR_MAX);
            return false;
        }
    }
    else
    {
        behaviour_10 = behaviour_11;
    }

    if (!Utils::ReadParamUnsigned(root, "INTEGER_3", &integer_3))
    {
        integer_3 = 0;
    }

    if (!Utils::ReadParamBoolean(root, "FORCE_TELEPORT", &force_teleport))
    {
        force_teleport = false;
    }

    if (Utils::ReadParamUnsigned(root, "BEHAVIOUR_13", &behaviour_13))
    {
        if (behaviour_13 > BEHAVIOUR_MAX)
        {
            DPRINTF("%s: BEHAVIOUR_13 cannot be greater than %x.\n", FUNCNAME, BEHAVIOUR_MAX);
            return false;
        }
    }
    else
    {
        behaviour_13 = behaviour_11;
    }

    behaviour_66 = 0xFF;

    if (!force_teleport)
    {
        if (Utils::ReadParamUnsigned(root, "BEHAVIOUR_66", &behaviour_66))
        {
            if (behaviour_66 > BEHAVIOUR_MAX && behaviour_66 != 0xFF)
            {
                DPRINTF("%s: TELEPORT_BEHAVIOUR cannot be greater than %x.\n", FUNCNAME, BEHAVIOUR_MAX);
                return false;
            }
        }
    }

    if (!Utils::ReadParamUnsigned(root, "REMOVE_HAIR_ACCESSORIES", &remove_hair_accessories))
        remove_hair_accessories = 0xFF;

    if (remove_hair_accessories >= 2 && remove_hair_accessories != 0xFF)
        remove_hair_accessories = 0xFF;

    if (!Utils::ReadParamUnsigned(root, "BCS_HAIR_COLOR", &bcs_hair_color))
        bcs_hair_color = 0xFFFFFFFF;

    if (!Utils::ReadParamUnsigned(root, "BCS_EYES_COLOR", &bcs_eyes_color))
        bcs_eyes_color = 0xFFFFFFFF;

    return true;
}

TiXmlElement *PreBakedAlias::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Alias");
    entry_root->SetAttribute("cms_id", Utils::UnsignedToString(cms_id, true));
    entry_root->SetAttribute("cms_name", cms_name);

    if (ttc_files.length() > 0)
        Utils::WriteParamString(entry_root, "TTC_FILES", ttc_files);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool PreBakedAlias::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "cms_id", &cms_id))
        return false;

    if (!Utils::ReadAttrString(root, "cms_name", cms_name))
        return false;

    if (!Utils::ReadParamString(root, "TTC_FILES", ttc_files))
        ttc_files.clear();

    return true;
}

TiXmlElement *BcsColorsMap::Decompile(TiXmlNode *root, uint32_t cms_id, uint32_t costume) const
{
    TiXmlElement *entry_root = new TiXmlElement("ColorsMap");
    entry_root->SetAttribute("cms_id", Utils::UnsignedToString(cms_id, true));
    entry_root->SetAttribute("costume", costume);

    for (auto &it : map)
    {
        Utils::WriteParamUnsigned(entry_root, it.first.c_str(), it.second);
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BcsColorsMap::Compile(const TiXmlElement *root, uint32_t *cms_id, uint32_t *costume)
{
    if (!Utils::ReadAttrUnsigned(root, "cms_id", cms_id))
        return false;

    if (!Utils::ReadAttrUnsigned(root, "costume", costume))
        return false;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        const std::string &name = elem->ValueStr();
        uint32_t value;

        if (!Utils::ReadParamUnsigned(root, name.c_str(), &value))
            return false;

        map[name] = value;
    }

    return true;
}

Xv2PreBakedFile::Xv2PreBakedFile()
{
    this->big_endian = false;
}

Xv2PreBakedFile::~Xv2PreBakedFile()
{

}

void Xv2PreBakedFile::Reset()
{
    ozarus.clear();
    auto_btl_portrait_list.clear();
    body_shapes.clear();
    cus_aura_datas.clear();
    aliases.clear();
    any_dual_skill_list.clear();
}

TiXmlDocument *Xv2PreBakedFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("Xv2PreBaked");
    Utils::WriteComment(root, "This file is used by xv2patcher and XV2 Mods Installer. It defines characters data that only exist in exe at runtime. ");
    Utils::WriteComment(root, "--------------------------------------------");

    Utils::WriteComment(root, "This is the list of additional ozarus to add to the game.");
    Utils::WriteParamMultipleStrings(root, "OZARUS", ozarus);

    Utils::WriteComment(root, "This is the list of characters that will have an auto battle portrait.");
    Utils::WriteParamMultipleUnsigned(root, "AUTO_BTL_PORT", auto_btl_portrait_list, true);

    if (body_shapes.size() != 0)
        Utils::WriteComment(root, "This is the list of body shapes to apply.");

    for (const BodyShape &shape : body_shapes)
    {
        shape.Decompile(root);
    }

    if (cus_aura_datas.size() != 0)
        Utils::WriteComment(root, "This is the data of cus auras.");

    for (const CusAuraData &data : cus_aura_datas)
    {
        data.Decompile(root);
    }

    for (const PreBakedAlias &alias : aliases)
    {
        if ((int32_t)alias.cms_id  < 0)
            continue;

        alias.Decompile(root);
    }

    if (any_dual_skill_list.size() > 0)
    {
        Utils::WriteComment(root, "This is the list of characters that can use any dual skill.");
        Utils::WriteParamMultipleUnsigned(root, "ANY_DUAL_SKILL", any_dual_skill_list, true);
    }

    for (auto &it : colors_map)
    {
        it.second.Decompile(root, it.first >> 16, it.first&0xFFFF);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool Xv2PreBakedFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "Xv2PreBaked");

    if (!root)
    {
        DPRINTF("Cannot find\"Xv2PreBaked\" in xml.\n");
        return false;
    }

    Utils::ReadParamMultipleStrings(root, "OZARUS", ozarus);
    Utils::ReadParamMultipleUnsigned(root, "AUTO_BTL_PORT", auto_btl_portrait_list);
    Utils::ReadParamMultipleUnsigned(root, "ANY_DUAL_SKILL", any_dual_skill_list);

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        const std::string &name = elem->ValueStr();

        if (name == "BodyShape")
        {
            BodyShape shape;

            if (!shape.Compile(elem))
                return false;

            bool add_it = true;

            for (const BodyShape &bs : body_shapes)
            {
                if (bs.cms_entry == shape.cms_entry)
                {
                    add_it = false;
                    break;
                }
            }

            if (add_it)
                body_shapes.push_back(shape);
        }
        else if (name == "CusAuraData")
        {
            CusAuraData new_data;

            if (!new_data.Compile(elem))
                return false;

            bool add_it = true;

            for (const CusAuraData &data : cus_aura_datas)
            {
                if (data.cus_aura_id == new_data.cus_aura_id)
                {
                    add_it = false;
                    break;
                }
            }

            if (add_it)
                cus_aura_datas.push_back(new_data);
        }
        else if (name == "Alias")
        {
            PreBakedAlias new_alias;

            if (!new_alias.Compile(elem))
                return false;

            aliases.push_back(new_alias);
        }
        else if (name == "ColorsMap")
        {
            BcsColorsMap map;
            uint32_t cms_id;
            uint32_t costume;

            if (!map.Compile(elem, &cms_id, &costume))
                return false;

            if (map.map.size() > 0)
                colors_map[(cms_id << 16) | (costume&0xFFFF)] = map;
        }
    }

    return true;
}

void Xv2PreBakedFile::AddOzaru(const std::string &ozaru)
{
    if (std::find(ozarus.begin(), ozarus.end(), ozaru) != ozarus.end())
        return;

    ozarus.push_back(ozaru);
}

void Xv2PreBakedFile::RemoveOzaru(const std::string &ozaru)
{
    for (size_t i = 0; i < ozarus.size(); i++)
    {
        if (ozarus[i] == ozaru)
        {
            ozarus.erase(ozarus.begin()+i);
            i--;
        }
    }
}

void Xv2PreBakedFile::AddAutoBtlPortrait(uint32_t cms_entry)
{
    if (std::find(auto_btl_portrait_list.begin(), auto_btl_portrait_list.end(), cms_entry) != auto_btl_portrait_list.end())
        return;

    auto_btl_portrait_list.push_back(cms_entry);
}

void Xv2PreBakedFile::RemoveAutoBtlPortrait(uint32_t cms_entry)
{
    for (size_t i = 0; i < auto_btl_portrait_list.size(); i++)
    {
        if (auto_btl_portrait_list[i] == cms_entry)
        {
            auto_btl_portrait_list.erase(auto_btl_portrait_list.begin()+i);
            i--;
        }
    }
}

void Xv2PreBakedFile::AddBodyShape(uint32_t cms_entry, uint32_t body_shape)
{
    BodyShape shape;

    shape.cms_entry = cms_entry;
    shape.body_shape = body_shape;

    body_shapes.push_back(shape);
}

void Xv2PreBakedFile::RemoveBodyShape(uint32_t cms_entry)
{
    for (size_t i = 0; i < body_shapes.size(); i++)
    {
        if (body_shapes[i].cms_entry == cms_entry)
        {
            body_shapes.erase(body_shapes.begin()+i);
            i--;
        }
    }
}

void Xv2PreBakedFile::AddCharToAnyDualSkillList(uint32_t cms_entry)
{
    if (std::find(any_dual_skill_list.begin(), any_dual_skill_list.end(), cms_entry) != any_dual_skill_list.end())
        return;

    any_dual_skill_list.push_back(cms_entry);
}

void Xv2PreBakedFile::RemoveCharFromAnyDualSkillList(uint32_t cms_entry)
{
    for (size_t i = 0; i < any_dual_skill_list.size(); i++)
    {
        if (any_dual_skill_list[i] == cms_entry)
        {
            any_dual_skill_list.erase(any_dual_skill_list.begin()+i);
            i--;
        }
    }
}

CusAuraData *Xv2PreBakedFile::FindAuraData(uint16_t cus_aura_id)
{
    for (CusAuraData &data : cus_aura_datas)
    {
        if (data.cus_aura_id == cus_aura_id)
            return &data;
    }

    return nullptr;
}

CusAuraData *Xv2PreBakedFile::FindAuraDataByAurId(uint16_t aur_aura_id)
{
    for (CusAuraData &data : cus_aura_datas)
    {
        if (data.aur_aura_id == aur_aura_id)
            return &data;
    }

    return nullptr;
}

bool Xv2PreBakedFile::AddConsecutiveAuraData(std::vector<CusAuraData> &datas)
{
    if (datas.size() == 0)
        return true; // Yes, true

    uint16_t id;

    for (id = CUS_DATA_ID_START; ; id++)
    {
        bool found = true;

        for (uint32_t i = 0; i < (uint32_t)datas.size(); i++)
        {
            if (FindAuraData(id+i))
            {
                found = false;
                break;
            }
        }

        if (found)
            break;
    }

    for (CusAuraData &data : datas)
    {
        data.cus_aura_id = id;
        cus_aura_datas.push_back(data);
        id++;
    }

    return true;
}

void Xv2PreBakedFile::RemoveAuraData(uint16_t cus_aura_id)
{
    for (size_t i = 0; i < cus_aura_datas.size(); i++)
    {
        if (cus_aura_datas[i].cus_aura_id == cus_aura_id)
        {
            cus_aura_datas.erase(cus_aura_datas.begin()+i);
            i--;
        }
    }
}

PreBakedAlias *Xv2PreBakedFile::FindAlias(uint32_t cms_id)
{
    for (PreBakedAlias &alias : aliases)
    {
        if (alias.cms_id == cms_id)
            return &alias;
    }

    return nullptr;
}

PreBakedAlias *Xv2PreBakedFile::FindAlias(const std::string &cms_name)
{
    for (PreBakedAlias &alias : aliases)
    {
        if (alias.cms_name == cms_name)
            return &alias;
    }

    return nullptr;
}

void Xv2PreBakedFile::AddAlias(const PreBakedAlias &alias)
{
    if ((int32_t)alias.cms_id  < 0)
        return;

    PreBakedAlias *existing_alias = FindAlias(alias.cms_id);
    if (existing_alias)
    {
        *existing_alias = alias;
        return;
    }

    aliases.push_back(alias);
}

size_t Xv2PreBakedFile::RemoveAlias(uint32_t cms_id)
{
    size_t count = 0;

    for (size_t i = 0; i < aliases.size(); i++)
    {
        if (aliases[i].cms_id == cms_id)
        {
            aliases.erase(aliases.begin()+i);
            i--;
            count++;
        }
    }

    return count;
}

size_t Xv2PreBakedFile::RemoveAlias(const std::string &cms_name)
{
    size_t count = 0;

    for (size_t i = 0; i < aliases.size(); i++)
    {
        if (aliases[i].cms_name == cms_name)
        {
            aliases.erase(aliases.begin()+i);
            i--;
            count++;
        }
    }

    return count;
}


