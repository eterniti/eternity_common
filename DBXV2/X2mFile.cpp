#include <algorithm>
#include "X2mFile.h"
#include "IniFile.h"
#include "debug.h"

#define X2M_TTB_ASSIGN_ID_START 0x70000000

static const std::vector<std::string> aura_types =
{
    "BoostStart",
    "BoostLoop",
    "BoostEnd",
    "KiaiCharge",
    "KiryokuMax",
    "HenshinStart",
    "HenshinEnd"
};

const std::vector<std::string> x2m_cr_code =
{
    "HUM",
    "HUF",
    "NMC",
    "FRI",
    "MAM",
    "MAF"
};

std::string X2mFile::restore_path;
std::unordered_set<std::string> X2mFile::restore_cms_set;

void X2mSlotEntry::CopyFrom(const CharaListSlotEntry &entry, bool name)
{
    costume_index = entry.costume_index;
    model_preset = entry.model_preset;
    flag_gk2 = entry.flag_gk2;
    voices_id_list[0] = entry.voices_id_list[0];
    voices_id_list[1] = entry.voices_id_list[1];
    audio_files[0].clear();
    audio_files[1].clear();

    if (name)
    {
        for (int i = 0; i < XV2_LANG_NUM; i++)
        {
            if (entry.code.length() != 5 || entry.code[0] != '"' || entry.code[4] != '"')
            {
                costume_name[i].clear();
            }
            else
            {
                std::string code = entry.code.substr(1, 3);

                if (!Xenoverse2::GetCharaCostumeName(code, costume_index, model_preset, costume_name[i], i))
                    costume_name[i].clear();
            }
        }
    }
}

void X2mSlotEntry::CopyTo(CharaListSlotEntry &entry, const std::string &code) const
{
    if (voices_id_list[0] == (int)X2M_DUMMY_ID || voices_id_list[1] == (int)X2M_DUMMY_ID)
    {
        DPRINTF("%s: PROGRAM BUG, audio should have been assigned before calling this function.\n", FUNCNAME);
        exit(-1);
    }

    entry.with_undefined = false;
    entry.code = std::string("\"") + code + std::string("\"");
    entry.costume_index = costume_index;
    entry.model_preset = model_preset;
    entry.unlock_index = 0;
    entry.flag_gk2 = flag_gk2;
    entry.voices_id_list[0] = voices_id_list[0];
    entry.voices_id_list[1] = voices_id_list[1];
    entry.dlc = "Dlc_Def";
}

TiXmlElement *X2mSlotEntry::Decompile(TiXmlNode *root, bool new_format) const
{
    TiXmlElement *entry_root = new TiXmlElement("SlotEntry");

    entry_root->SetAttribute("costume_index", costume_index);
    Utils::WriteParamUnsigned(entry_root, "MODEL_PRESET", (int64_t)model_preset, true);
    Utils::WriteParamString(entry_root, "FLAG_GK2", (flag_gk2) ? "true" : "false");

    std::vector<uint32_t> voices = { (uint32_t)voices_id_list[0], (uint32_t)voices_id_list[1] };
    Utils::WriteParamMultipleUnsigned(entry_root, "VOICES_ID_LIST", voices, true);

    for (size_t i = 0; i < XV2_LANG_NUM; i++)
    {
        if (costume_name[i].length() == 0)
        {
            if (i == XV2_LANG_ENGLISH)
                return nullptr;
        }
        else
        {
            std::string name = "COSTUME_NAME_" + Utils::ToUpperCase(xv2_lang_codes[i]);
            Utils::WriteParamString(entry_root, name.c_str(), costume_name[i]);
        }
    }

    if (new_format)
    {
        if (audio_files[0].length() > 0)
            Utils::WriteParamString(entry_root, "AUDIO0", audio_files[0]);

        if (audio_files[1].length() > 0)
            Utils::WriteParamString(entry_root, "AUDIO1", audio_files[1]);
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool X2mSlotEntry::Compile(const TiXmlElement *root, bool new_format)
{
    if (!Utils::ReadAttrUnsigned(root, "costume_index", (uint32_t *)&costume_index))
    {
        DPRINTF("%s: costume_index attribute is not optional.\n", FUNCNAME);
        return false;
    }

    if (!Utils::GetParamUnsigned(root, "MODEL_PRESET", (uint32_t *)&model_preset))
        return false;

    std::string flag;

    if (!Utils::GetParamString(root, "FLAG_GK2", flag))
        return false;

    flag = Utils::ToLowerCase(flag);
    if (flag == "true" || flag == "1")
    {
        flag_gk2 = true;
    }
    else if (flag == "false" || flag == "0")
    {
        flag_gk2 = false;
    }
    else
    {
        DPRINTF("%s: Cannot parse FLAG_GK2 param (%s).\n", FUNCNAME, flag.c_str());
        return false;
    }

    if (!Utils::GetParamMultipleUnsigned(root, "VOICES_ID_LIST", (uint32_t *)voices_id_list, 2))
        return false;

    for (size_t i = 0; i < XV2_LANG_NUM; i++)
    {
        std::string name = "COSTUME_NAME_" + Utils::ToUpperCase(xv2_lang_codes[i]);

        if (i == XV2_LANG_ENGLISH)
        {
            if (!Utils::GetParamString(root, name.c_str(), costume_name[i]))
                return false;
        }
        else
        {
            Utils::ReadParamString(root, name.c_str(), costume_name[i]);
        }
    }

    audio_files[0].clear();
    audio_files[1].clear();

    if (new_format)
    {
        if (Utils::ReadParamString(root, "AUDIO0", audio_files[0]))
            voices_id_list[0] = X2M_DUMMY_ID;

        if (Utils::ReadParamString(root, "AUDIO1", audio_files[1]))
            voices_id_list[1] = X2M_DUMMY_ID;
    }

    return true;
}

TiXmlElement *SevEntryHL::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("SevEntryHL");

    entry_root->SetAttribute("costume_id", Utils::UnsignedToString(costume_id, true));
    entry_root->SetAttribute("copy_char", copy_char);
    entry_root->SetAttribute("copy_costume", Utils::UnsignedToString(copy_costume, true));

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool SevEntryHL::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrString(root, "copy_char", copy_char))
    {
        DPRINTF("%s: copy_char attribute is not optional.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "costume_id", &costume_id))
        costume_id = 0xFFFFFFFF;

    if (!Utils::ReadAttrUnsigned(root, "copy_costume", &copy_costume))
        copy_costume = 0xFFFFFFFF;

    return true;
}

TiXmlElement *SevEventsLL::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("SubEntry");

    entry_root->SetAttribute("im_source", (im_source) ? "true" : "false");
    entry_root->SetAttribute("char_code", char_code);
    entry_root->SetAttribute("costume_id", Utils::UnsignedToString(costume_id, true));

    for (const SevEvent &event : events)
        event.Decompile(entry_root);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool SevEventsLL::Compile(const TiXmlElement *root)
{
    std::string temp;

    if (!Utils::ReadAttrString(root, "im_source", temp))
    {
        DPRINTF("%s: im_source is not optional.\n", FUNCNAME);
        return false;
    }

    temp = Utils::ToLowerCase(temp);
    im_source = (temp == "true" || temp == "1");

    if (!Utils::ReadAttrString(root, "char_code", char_code))
    {
        if (!im_source)
        {
            DPRINTF("%s: char_code is not optional when im_source=false.\n", FUNCNAME);
            return false;
        }
    }

    if (!im_source && char_code.length() != 3)
    {
        DPRINTF("%s: char_code must be a 3-letter code when im_source=false.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "costume_id", &costume_id))
    {
        DPRINTF("%s: costume_id is not optional.\n", FUNCNAME);
        return false;
    }

    events.clear();

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Event")
        {
           SevEvent event;

           if (!event.Compile(elem))
               return false;

           events.push_back(event);
        }
    }

    return true;
}

TiXmlElement *SevEntryLL::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("SevEntryLL");

    entry_root->SetAttribute("costume_id", Utils::UnsignedToString(costume_id, true));

    for (const SevEventsLL &events : sub_entries)
        events.Decompile(entry_root);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool SevEntryLL::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "costume_id", &costume_id))
    {
        DPRINTF("%s: costume_id is not optional\n", FUNCNAME);
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "SubEntry")
        {
           SevEventsLL events;

           if (!events.Compile(elem))
               return false;

           sub_entries.push_back(events);
        }       
    }

    return true;
}

TiXmlElement *X2mCharaName::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("X2mCharaName");
    entry_root->SetAttribute("costume_index", costume_index);

    for (size_t i = 0; i < XV2_LANG_NUM; i++)
    {
        const std::string &name = chara_name[i];

        if (name.length() == 0)
        {
            if (i == XV2_LANG_ENGLISH)
            {
                DPRINTF("%s: Name for language english is not optional.\n", FUNCNAME);
                return nullptr;
            }

            continue;
        }

        const std::string temp_str = "CHARA_NAME_" + Utils::ToUpperCase(xv2_lang_codes[i]);
        Utils::WriteParamString(entry_root, temp_str.c_str(), name);
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool X2mCharaName::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "costume_index", &costume_index))
    {
       DPRINTF("%s: attribute costume_index is not optional.\n", FUNCNAME);
       return false;
    }

    for (size_t i = 0; i < XV2_LANG_NUM; i++)
    {
        chara_name[i].clear();
        const std::string temp_str = "CHARA_NAME_" + Utils::ToUpperCase(xv2_lang_codes[i]);

        Utils::ReadParamString(root, temp_str.c_str(), chara_name[i]);

        if (i == XV2_LANG_ENGLISH && chara_name[i].length() == 0)
        {
            DPRINTF("%s: Name for language english is not optional.\n", FUNCNAME);
            return false;
        }
    }

    return true;
}

TiXmlElement *X2mSkillTransName::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("X2mSkillTransName");

    for (size_t i = 0; i < XV2_LANG_NUM; i++)
    {
        const std::string &name = trans_name[i];

        if (name.length() == 0)
        {
            if (i == XV2_LANG_ENGLISH)
            {
                DPRINTF("%s: Name for language english is not optional.\n", FUNCNAME);
                return nullptr;
            }

            continue;
        }

        const std::string temp_str = "TRANS_NAME_" + Utils::ToUpperCase(xv2_lang_codes[i]);
        Utils::WriteParamString(entry_root, temp_str.c_str(), name);
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool X2mSkillTransName::Compile(const TiXmlElement *root)
{
    for (size_t i = 0; i < XV2_LANG_NUM; i++)
    {
        trans_name[i].clear();
        const std::string temp_str = "TRANS_NAME_" + Utils::ToUpperCase(xv2_lang_codes[i]);

        Utils::ReadParamString(root, temp_str.c_str(), trans_name[i]);

        if (i == XV2_LANG_ENGLISH && trans_name[i].length() == 0)
        {
            DPRINTF("%s: Name for language english is not optional.\n", FUNCNAME);
            return false;
        }
    }

    return true;
}

TiXmlElement *X2mDepends::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("X2mDepends");

    entry_root->SetAttribute("id", Utils::UnsignedToString(id, true));
    entry_root->SetAttribute("name", name);

    Utils::WriteParamGUID(entry_root, "GUID", guid);

    if (type == X2mDependsType::SKILL)
    {
        Utils::WriteParamString(entry_root, "TYPE", "SKILL");
    }
    else if (type == X2mDependsType::COSTUME)
    {
        Utils::WriteParamString(entry_root, "TYPE", "COSTUME");
    }
    else if (type == X2mDependsType::SUPERSOUL)
    {
        Utils::WriteParamString(entry_root, "TYPE", "SUPERSOUL");
    }
    else if (type == X2mDependsType::CHARACTER)
    {
        Utils::WriteParamString(entry_root, "TYPE", "CHARACTER");
    }
    else
    {
        delete entry_root;
        return nullptr;
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool X2mDepends::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "id", &id))
    {
        DPRINTF("%s: Attribute \"id\" is not optional.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrString(root, "name", name))
    {
        DPRINTF("%s: Attribute \"name\" is not optional.\n", FUNCNAME);
        return false;
    }

    if (!Utils::GetParamGUID(root, "GUID", guid))
        return false;

    std::string type_str;

    if (!Utils::GetParamString(root, "TYPE", type_str))
        return false;

    if (type_str == "SKILL")
    {
        type = X2mDependsType::SKILL;
    }
    else if (type_str == "COSTUME")
    {
        type = X2mDependsType::COSTUME;
    }
    else if (type_str == "SUPERSOUL")
    {
        type = X2mDependsType::SUPERSOUL;
    }
    else if (type_str == "CHARACTER")
    {
        type = X2mDependsType::CHARACTER;
    }
    else
    {
        DPRINTF("%s: Unrecognized type \"%s\"\n", FUNCNAME, type_str.c_str());
        return false;
    }

    return true;
}

TiXmlElement *X2mSkillAura::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("X2mSkillAura");

    if (!data.Decompile(entry_root))
        return nullptr;

    if (data.aur_aura_id == X2M_INVALID_ID16)
    {
        if (!aura.Decompile(entry_root, aura_types))
            return nullptr;

        if (!extra.Decompile(entry_root))
            return nullptr;
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool X2mSkillAura::Compile(const TiXmlElement *root)
{
    const TiXmlElement *elem;
    if (Utils::GetElemCount(root, "CusAuraData", &elem) == 0)
    {
        DPRINTF("%s: CusAuraData is not optional.\n", FUNCNAME);
        return false;
    }

    if (!data.Compile(elem))
        return false;

    aura = AurAura();
    extra = AuraExtraData();

    if (data.aur_aura_id == X2M_INVALID_ID16)
    {
        if (Utils::GetElemCount(root, "Aura", &elem) == 0)
        {
            DPRINTF("%s: if data.aur_aur_id is X2M_INVALID_ID, aura is not optional.\n", FUNCNAME);
            return false;
        }

        if (!aura.Compile(elem, aura_types))
            return false;

        if (Utils::GetElemCount(root, "AuraExtraData", &elem) > 0) // If newer mod
        {
            if (!extra.Compile(elem))
                return false;
        }

        extra.aur_id = X2M_INVALID_ID; // If there is custom aura, this value is needed.
    }

    return true;
}

TiXmlElement *X2mItem::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("X2mItem");
    std::string temp_str;

    for (size_t i = 0; i < XV2_LANG_NUM; i++)
    {
        const std::string &name = item_name[i];
        const std::string &desc = item_desc[i];
        const std::string &how = item_how[i];

        if (name.length() == 0)
        {
            if (i == XV2_LANG_ENGLISH)
            {
                DPRINTF("%s: Name for language english is not optional.\n", FUNCNAME);
                return nullptr;
            }

            continue;
        }

        if (desc.length() == 0)
        {
            if (i == XV2_LANG_ENGLISH)
            {
                DPRINTF("%s: Desc for language english is not optional.\n", FUNCNAME);
                return nullptr;
            }

            continue;
        }

        temp_str = "ITEM_NAME_" + Utils::ToUpperCase(xv2_lang_codes[i]);
        Utils::WriteParamString(entry_root, temp_str.c_str(), name);

        temp_str = "ITEM_DESC_" + Utils::ToUpperCase(xv2_lang_codes[i]);
        Utils::WriteParamString(entry_root, temp_str.c_str(), desc);

        if (has_how)
        {
            temp_str = "ITEM_HOW_" + Utils::ToUpperCase(xv2_lang_codes[i]);
            Utils::WriteParamString(entry_root, temp_str.c_str(), how);
        }
    }    

    if (item_type == X2mItemType::TOP)
    {
        temp_str = "TOP";
    }
    else if (item_type == X2mItemType::BOTTOM)
    {
        temp_str = "BOTTOM";
    }
    else if (item_type == X2mItemType::GLOVES)
    {
        temp_str = "GLOVES";
    }
    else if (item_type == X2mItemType::SHOES)
    {
        temp_str = "SHOES";
    }
    else if (item_type == X2mItemType::ACCESSORY)
    {
        temp_str = "ACCESSORY";
    }
    else if (item_type == X2mItemType::SUPER_SOUL)
    {
        temp_str = "SUPER_SOUL";
    }
    else
    {
        DPRINTF("%s: Internal error, item type.\n", FUNCNAME);
        return nullptr;
    }

    Utils::WriteParamString(entry_root, "ITEM_TYPE", temp_str);

    if (!idb.Decompile(entry_root, IdbCommentType::NONE))
        return nullptr;

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool X2mItem::Compile(const TiXmlElement *root)
{
    std::string temp_str;

    for (size_t i = 0; i < XV2_LANG_NUM; i++)
    {
        item_name[i].clear();
        temp_str = "ITEM_NAME_" + Utils::ToUpperCase(xv2_lang_codes[i]);

        Utils::ReadParamString(root, temp_str.c_str(), item_name[i]);

        if (i == XV2_LANG_ENGLISH && item_name[i].length() == 0)
        {
            DPRINTF("%s: Name for language english is not optional.\n", FUNCNAME);
            return false;
        }

        item_desc[i].clear();
        temp_str = "ITEM_DESC_" + Utils::ToUpperCase(xv2_lang_codes[i]);

        Utils::ReadParamString(root, temp_str.c_str(), item_desc[i]);

        if (i == XV2_LANG_ENGLISH && item_desc[i].length() == 0)
        {
            DPRINTF("%s: Desc for language english is not optional.\n", FUNCNAME);
            return false;
        }

        item_how[i].clear();
        temp_str = "ITEM_HOW_" + Utils::ToUpperCase(xv2_lang_codes[i]);

        Utils::ReadParamString(root, temp_str.c_str(), item_how[i]);

        if (i == XV2_LANG_ENGLISH)
        {
            has_how = (item_how[i].length() > 0);
        }
    }    

    if (!Utils::GetParamString(root, "ITEM_TYPE", temp_str))
        return false;

    if (temp_str == "TOP")
    {
        item_type = X2mItemType::TOP;
    }
    else if (temp_str == "BOTTOM")
    {
        item_type = X2mItemType::BOTTOM;
    }
    else if (temp_str == "GLOVES")
    {
        item_type = X2mItemType::GLOVES;
    }
    else if (temp_str == "SHOES")
    {
        item_type = X2mItemType::SHOES;
    }
    else if (temp_str == "ACCESSORY")
    {
        item_type = X2mItemType::ACCESSORY;
    }
    else if (temp_str == "SUPER_SOUL")
    {
        item_type = X2mItemType::SUPER_SOUL;
    }
    else
    {
        DPRINTF("%s: Unrecognized item type for ITEM_TYPE\n", FUNCNAME);
        return false;
    }

    const TiXmlElement *elem;
    if (Utils::GetElemCount(root, "IdbEntry", &elem) == 0)
    {
        DPRINTF("%s: Not IdbEntry inside X2mItem.\n", FUNCNAME);
        return false;
    }

    if (!idb.Compile(elem))
        return false;

    idb.id = idb.name_id = idb.desc_id = X2M_DUMMY_ID16;    
    if (has_how)
        idb.how_id = X2M_DUMMY_ID16;
    else
        idb.how_id = 0;

    return true;
}

TiXmlElement *X2mPartSet::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("X2mPartSet");

    Utils::WriteParamUnsigned(entry_root, "RACE", race);

    if (!bcs.Decompile(entry_root, -1))
        return nullptr;

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool X2mPartSet::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "RACE", &race))
        return false;

    if (race >= X2M_CR_NUM)
    {
        DPRINTF("%s: race value %d out of bounds.\n", FUNCNAME, race);
        return false;
    }

    const TiXmlElement *bcs_root;
    if (Utils::GetElemCount(root, "PartSet", &bcs_root) == 0)
    {
        DPRINTF("%s: Was exepecting a partset here.\n", FUNCNAME);
        return false;
    }

    if (!bcs.Compile(bcs_root))
        return false;

    return true;
}

TiXmlElement *X2mCustomAudio::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("X2mCustomAudio");

    entry_root->SetAttribute("id", Utils::UnsignedToString(id, true));
    entry_root->SetAttribute("name", name);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool X2mCustomAudio::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "id", &id))
        return false;

    if (!Utils::ReadAttrString(root, "name", name))
        return false;

    return true;
}

TiXmlElement *TtbEntryHL::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("TtbEntryHL");

    entry_root->SetAttribute("costume_id", Utils::UnsignedToString(costume_id, true));
    entry_root->SetAttribute("copy_char", copy_char);
    entry_root->SetAttribute("copy_costume", Utils::UnsignedToString(copy_costume, true));

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool TtbEntryHL::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrString(root, "copy_char", copy_char))
    {
        DPRINTF("%s: copy_char attribute is not optional.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "costume_id", &costume_id))
        costume_id = 0xFFFFFFFF;

    if (!Utils::ReadAttrUnsigned(root, "copy_costume", &copy_costume))
        copy_costume = 0xFFFFFFFF;

    return true;
}

TiXmlElement *TtbEntryLL::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("TtbEntryLL");

    entry_root->SetAttribute("num_actors", num_actors);
    entry_root->SetAttribute("cms_ch1", cms_code);
    entry_root->SetAttribute("costume", Utils::UnsignedToString(costume, costume==0xFFFFFFFF));
    entry_root->SetAttribute("transformation", Utils::UnsignedToString(transformation, transformation==0xFFFFFFFF));

    entry_root->SetAttribute("cms_ch2", cms2_code);
    entry_root->SetAttribute("ch2_costume", Utils::UnsignedToString(costume2, costume2==0xFFFFFFFF));
    entry_root->SetAttribute("ch2_transformation", Utils::UnsignedToString(transformation2, transformation2==0xFFFFFFFF));

    entry_root->SetAttribute("cms_ch3",cms3_code);
    entry_root->SetAttribute("ch3_costume", Utils::UnsignedToString(costume3, costume3==0xFFFFFFFF));
    entry_root->SetAttribute("ch3_transformation", Utils::UnsignedToString(transformation3, transformation3==0xFFFFFFFF));

    entry_root->SetAttribute("cms_ch4", cms4_code);
    entry_root->SetAttribute("ch4_costume", Utils::UnsignedToString(costume4, costume4==0xFFFFFFFF));
    entry_root->SetAttribute("ch4_transformation", Utils::UnsignedToString(transformation4, transformation4==0xFFFFFFFF));

    entry_root->SetAttribute("cms_ch5", cms5_code);
    entry_root->SetAttribute("ch5_costume", Utils::UnsignedToString(costume5, costume5==0xFFFFFFFF));
    entry_root->SetAttribute("ch5_transformation", Utils::UnsignedToString(transformation5, transformation5==0xFFFFFFFF));

    std::string type_str;

    if (type == 1)
    {
        type_str = "ally_talk";
    }
    else if (type == 2)
    {
        type_str = "death";
    }
    else
    {
        type_str = "versus";
    }

    Utils::WriteParamString(entry_root, "TYPE", type_str);
    Utils::WriteParamUnsigned(entry_root, "U_44", unk_44, true);
    Utils::WriteParamUnsigned(entry_root, "U_48", unk_48, true);

    Utils::WriteParamBoolean(entry_root, "REVERSE_ORDER", reverse_order);
    Utils::WriteParamBoolean(entry_root, "DUMMY_ORDER1", dummy_order1);
    Utils::WriteParamBoolean(entry_root, "DUMMY_ORDER2", dummy_order2);

    Utils::WriteParamString(entry_root, "VOICE_NAME", event_res1.name);
    Utils::WriteParamString(entry_root, "VOICE2_NAME", event_res2.name);

    if (num_actors >= 3)
        Utils::WriteParamString(entry_root, "VOICE3_NAME", event_res3.name);

    if (num_actors >= 4)
        Utils::WriteParamString(entry_root, "VOICE4_NAME", event_res4.name);

    if (num_actors >= 5)
        Utils::WriteParamString(entry_root, "VOICE5_NAME", event_res5.name);

    for (size_t i = 0; i < XV2_LANG_NUM; i++)
    {
        if (Utils::BeginsWith(event_res1.name, "X2T_"))
        {
            if (event_res1.subtitle[i].length() == 0)
            {
                if (i == XV2_LANG_ENGLISH)
                    return nullptr;
            }
            else
            {
                std::string name = "SUBTITLE1_" + Utils::ToUpperCase(xv2_lang_codes[i]);
                Utils::WriteParamString(entry_root, name.c_str(), event_res1.subtitle[i]);
            }
        }

        if (Utils::BeginsWith(event_res2.name, "X2T_"))
        {
            if (event_res2.subtitle[i].length() == 0)
            {
                if (i == XV2_LANG_ENGLISH)
                    return nullptr;
            }
            else
            {
                std::string name = "SUBTITLE2_" + Utils::ToUpperCase(xv2_lang_codes[i]);
                Utils::WriteParamString(entry_root, name.c_str(), event_res2.subtitle[i]);
            }
        }

        if (num_actors >= 3)
        {
            if (Utils::BeginsWith(event_res3.name, "X2T_"))
            {
                if (event_res3.subtitle[i].length() == 0)
                {
                    if (i == XV2_LANG_ENGLISH)
                        return nullptr;
                }
                else
                {
                    std::string name = "SUBTITLE3_" + Utils::ToUpperCase(xv2_lang_codes[i]);
                    Utils::WriteParamString(entry_root, name.c_str(), event_res3.subtitle[i]);
                }
            }
        }

        if (num_actors >= 4)
        {
            if (Utils::BeginsWith(event_res4.name, "X2T_"))
            {
                if (event_res4.subtitle[i].length() == 0)
                {
                    if (i == XV2_LANG_ENGLISH)
                        return nullptr;
                }
                else
                {
                    std::string name = "SUBTITLE4_" + Utils::ToUpperCase(xv2_lang_codes[i]);
                    Utils::WriteParamString(entry_root, name.c_str(), event_res4.subtitle[i]);
                }
            }
        }

        if (num_actors >= 5)
        {
            if (Utils::BeginsWith(event_res5.name, "X2T_"))
            {
                if (event_res5.subtitle[i].length() == 0)
                {
                    if (i == XV2_LANG_ENGLISH)
                        return nullptr;
                }
                else
                {
                    std::string name = "SUBTITLE5_" + Utils::ToUpperCase(xv2_lang_codes[i]);
                    Utils::WriteParamString(entry_root, name.c_str(), event_res5.subtitle[i]);
                }
            }
        }
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool TtbEntryLL::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "num_actors", &num_actors))
        return false;

    if (num_actors < 2 || num_actors > 5)
    {
        DPRINTF("%s: Invalid number of actors (%d)\n", FUNCNAME, num_actors);
        return false;
    }

    if (!Utils::ReadAttrString(root, "cms_ch1", cms_code))
        return false;

    if (!Utils::ReadAttrUnsigned(root, "costume", &costume))
        return false;

    if (!Utils::ReadAttrUnsigned(root, "transformation", &transformation))
        return false;

    if (!Utils::ReadAttrString(root, "cms_ch2", cms2_code))
        return false;

    if (!Utils::ReadAttrUnsigned(root, "ch2_costume", &costume2))
        return false;

    if (!Utils::ReadAttrUnsigned(root, "ch2_transformation", &transformation2))
        return false;

    if (num_actors >= 3)
    {
        if (!Utils::ReadAttrString(root, "cms_ch3", cms3_code))
            return false;

        if (!Utils::ReadAttrUnsigned(root, "ch3_costume", &costume3))
            return false;

        if (!Utils::ReadAttrUnsigned(root, "ch3_transformation", &transformation3))
            return false;
    }
    else
    {
        cms3_code.clear();
        costume3 = transformation3 = 0;
    }

    if (num_actors >= 4)
    {
        if (!Utils::ReadAttrString(root, "cms_ch4", cms4_code))
            return false;

        if (!Utils::ReadAttrUnsigned(root, "ch4_costume", &costume4))
            return false;

        if (!Utils::ReadAttrUnsigned(root, "ch4_transformation", &transformation4))
            return false;
    }
    else
    {
        cms4_code.clear();
        costume4 = transformation4 = 0;
    }

    if (num_actors >= 5)
    {
        if (!Utils::ReadAttrString(root, "cms_ch5", cms5_code))
            return false;

        if (!Utils::ReadAttrUnsigned(root, "ch5_costume", &costume5))
            return false;

        if (!Utils::ReadAttrUnsigned(root, "ch5_transformation", &transformation5))
            return false;
    }
    else
    {
        cms5_code.clear();
        costume5 = transformation5 = 0;
    }

    std::string type_str;
    if (!Utils::GetParamString(root, "TYPE", type_str)) return false;

    if (type_str == "versus")
        type = 0;
    else if (type_str == "ally_talk")
        type = 1;
    else if (type_str == "death")
        type = 2;
    else
    {
        DPRINTF("%s: Unrecognized type \"%s\"\n", FUNCNAME, type_str.c_str());
        return false;
    }

    if (!Utils::GetParamUnsigned(root, "U_44", &unk_44)) return false;
    if (!Utils::GetParamUnsigned(root, "U_48", &unk_48)) return false;

    if (!Utils::ReadParamBoolean(root, "REVERSE_ORDER", &reverse_order)) return false;

    if (!Utils::ReadParamBoolean(root, "DUMMY_ORDER1", &dummy_order1))
        dummy_order1 = false;

    if (!Utils::ReadParamBoolean(root, "DUMMY_ORDER2", &dummy_order2))
        dummy_order2 = false;

    event_res1.Clear();
    event_res2.Clear();
    event_res3.Clear();
    event_res4.Clear();
    event_res5.Clear();

    if (!Utils::GetParamString(root, "VOICE_NAME", event_res1.name))  return false;
    if (!Utils::GetParamString(root, "VOICE2_NAME", event_res2.name)) return false;

    if (num_actors >= 3)
    {
        if (!Utils::ReadParamString(root, "VOICE3_NAME", event_res3.name)) return false;
    }

    if (num_actors >= 4)
    {
        if (!Utils::ReadParamString(root, "VOICE4_NAME", event_res4.name)) return false;
    }

    if (num_actors >= 5)
    {
        if (!Utils::ReadParamString(root, "VOICE5_NAME", event_res5.name)) return false;
    }

    for (size_t i = 0; i < XV2_LANG_NUM; i++)
    {
        std::string name;

        if (Utils::BeginsWith(event_res1.name, "X2T_"))
        {
            name = "SUBTITLE1_" + Utils::ToUpperCase(xv2_lang_codes[i]);

            if (i == XV2_LANG_ENGLISH)
            {
                if (!Utils::GetParamString(root, name.c_str(), event_res1.subtitle[i]))
                    return false;
            }
            else
            {
                Utils::ReadParamString(root, name.c_str(), event_res1.subtitle[i]);
            }
        }

        if (Utils::BeginsWith(event_res2.name, "X2T_"))
        {
            name = "SUBTITLE2_" + Utils::ToUpperCase(xv2_lang_codes[i]);

            if (i == XV2_LANG_ENGLISH)
            {
                if (!Utils::GetParamString(root, name.c_str(), event_res2.subtitle[i]))
                    return false;
            }
            else
            {
                Utils::ReadParamString(root, name.c_str(), event_res2.subtitle[i]);
            }
        }

        if (num_actors >= 3)
        {
            if (Utils::BeginsWith(event_res3.name, "X2T_"))
            {
                name = "SUBTITLE3_" + Utils::ToUpperCase(xv2_lang_codes[i]);

                if (i == XV2_LANG_ENGLISH)
                {
                    if (!Utils::GetParamString(root, name.c_str(), event_res3.subtitle[i]))
                        return false;
                }
                else
                {
                    Utils::ReadParamString(root, name.c_str(), event_res3.subtitle[i]);
                }
            }
        }

        if (num_actors >= 4)
        {
            if (Utils::BeginsWith(event_res4.name, "X2T_"))
            {
                name = "SUBTITLE4_" + Utils::ToUpperCase(xv2_lang_codes[i]);

                if (i == XV2_LANG_ENGLISH)
                {
                    if (!Utils::GetParamString(root, name.c_str(), event_res4.subtitle[i]))
                        return false;
                }
                else
                {
                    Utils::ReadParamString(root, name.c_str(), event_res4.subtitle[i]);
                }
            }
        }

        if (num_actors >= 5)
        {
            if (Utils::BeginsWith(event_res5.name, "X2T_"))
            {
                name = "SUBTITLE5_" + Utils::ToUpperCase(xv2_lang_codes[i]);

                if (i == XV2_LANG_ENGLISH)
                {
                    if (!Utils::GetParamString(root, name.c_str(), event_res5.subtitle[i]))
                        return false;
                }
                else
                {
                    Utils::ReadParamString(root, name.c_str(), event_res5.subtitle[i]);
                }
            }
        }
    }

    if (num_actors >= 3)
        reverse_order = false;

    return true;
}

TiXmlElement *TtcEntryHL::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("TtcEntryHL");

    entry_root->SetAttribute("costume_id", Utils::UnsignedToString(costume_id, true));
    entry_root->SetAttribute("copy_char", copy_char);
    entry_root->SetAttribute("copy_costume", Utils::UnsignedToString(copy_costume, true));

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool TtcEntryHL::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrString(root, "copy_char", copy_char))
    {
        DPRINTF("%s: copy_char attribute is not optional.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "costume_id", &costume_id))
        costume_id = 0xFFFFFFFF;

    if (!Utils::ReadAttrUnsigned(root, "copy_costume", &copy_costume))
        copy_costume = 0xFFFFFFFF;

    return true;
}

TiXmlElement *TtcEntryLL::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("TtcEntryLL");
    std::string type_str;

    if (type == TTC_MINOR_DAMAGE_LIST)
    {
        type_str = "minor_damage";
    }
    else if (type == TTC_MAJOR_DAMAGE_LIST)
    {
        type_str = "major_damage";
    }
    else if (type == TTC_PLAYER_KO_ENEMY_LIST)
    {
        type_str = "player_ko_enemy";
    }
    else if (type == TTC_STRONG_ATTACK_DAMAGED_LIST)
    {
        type_str = "strong_attack_damaged";
    }
    else if (type == TTC_POWER_UP_LIST)
    {
        type_str = "power_up";
    }
    else if (type == TTC_START_TALK_LIST)
    {
        type_str = "start_talk";
    }
    else if (type == TTC_PLAYER_DAMAGED_LIST)
    {
        type_str = "player_damaged";
    }
    else if (type == TTC_LITTLE_TIME_LIST)
    {
        type_str = "little_time";
    }
    else if (type == TTC_PLAYER_ALLY_KILLED_ENEMY_LIST)
    {
        type_str = "player_ally_killed_enemy";
    }
    else if (type == TTC_CHALLENGE_LIST)
    {
        type_str = "challenge";
    }
    else if (type == TTC_KO_LIST)
    {
        type_str = "ko";
    }
    else if (type == TTC_ENTERING_LIST)
    {
        type_str = "entering";
    }
    else if (type == TTC_MASTER_VERSUS_LIST)
    {
        type_str = "master_versus";
    }
    else if (type == TTC_PLAYER_KO_ALLY_LIST)
    {
        type_str = "player_ko_ally";
    }
    else if (type == TTC_FIGHT_SERIOUSLY_LIST)
    {
        type_str = "fight_seriously";
    }
    else
    {
        type_str = "versus";
    }

    entry_root->SetAttribute("type", type_str);

    std::string condition_str;

    if (condition == TTC_TO_HUMAN_CAC_CONDITION)
    {
        condition_str = "to_human_cac";
    }
    else if (condition == TTC_TO_SAIYAN_CAC_CONDITION)
    {
        condition_str = "to_saiyan_cac";
    }
    else if (condition == TTC_TO_NAMEKIAN_CAC_CONDITION)
    {
        condition_str = "to_namekian_cac";
    }
    else if (condition == TTC_TO_FREEZER_CAC_CONDITION)
    {
        condition_str = "to_freezer_cac";
    }
    else if (condition == TTC_TO_MAJIN_CAC_CONDITION)
    {
        condition_str = "to_majin_cac";
    }
    else if (condition == TTC_TEACHER_CONDITION)
    {
        condition_str = "teacher";
    }
    else
    {
        condition_str = "default";
    }

    entry_root->SetAttribute("condition", condition_str);

    Utils::WriteParamUnsigned(entry_root, "COSTUME", costume, (costume==0xFFFFFFFF));
    Utils::WriteParamUnsigned(entry_root, "TRANSFORMATION", transformation, (transformation==0xFFFFFFFF));
    Utils::WriteParamString(entry_root, "NAME", name);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool TtcEntryLL::Compile(const TiXmlElement *root)
{
    std::string type_str;

    if (!Utils::ReadAttrString(root, "type", type_str))
    {
        DPRINTF("%s: Attribute \"type\" is not optional.\n", FUNCNAME);
        return false;
    }

    if (type_str == "versus")
    {
        type = TTC_VERSUS_LIST;
    }
    else if (type_str == "minor_damage" || type_str == "minor_damage_by_player_team")
    {
        type = TTC_MINOR_DAMAGE_LIST;
    }
    else if (type_str == "major_damage" || type_str == "major_damage_by_player_team")
    {
        type = TTC_MAJOR_DAMAGE_LIST;
    }
    else if (type_str == "player_ko_enemy")
    {
        type = TTC_PLAYER_KO_ENEMY_LIST;
    }
    else if (type_str == "strong_attack_damaged" || type_str == "damaged")
    {
        type = TTC_STRONG_ATTACK_DAMAGED_LIST;
    }
    else if (type_str == "power_up")
    {
        type = TTC_POWER_UP_LIST;
    }
    else if (type_str == "start_talk")
    {
        type = TTC_START_TALK_LIST;
    }
    else if (type_str == "player_damaged")
    {
        type = TTC_PLAYER_DAMAGED_LIST;
    }
    else if (type_str == "little_time")
    {
        type = TTC_LITTLE_TIME_LIST;
    }
    else if (type_str == "player_ally_killed_enemy")
    {
        type = TTC_PLAYER_ALLY_KILLED_ENEMY_LIST;
    }
    else if (type_str == "challenge")
    {
        type = TTC_CHALLENGE_LIST;
    }
    else if (type_str == "ko")
    {
        type = TTC_KO_LIST;
    }
    else if (type_str == "entering")
    {
        type = TTC_ENTERING_LIST;
    }
    else if (type_str == "master_versus")
    {
        type = TTC_MASTER_VERSUS_LIST;
    }
    else if (type_str == "player_ko_ally")
    {
        type = TTC_PLAYER_KO_ALLY_LIST;
    }
    else if (type_str == "fight_seriously")
    {
        type = TTC_FIGHT_SERIOUSLY_LIST;
    }
    else
    {
        DPRINTF("%s: Unrecognized type \"%s\"\n", FUNCNAME, type_str.c_str());
        return false;
    }

    std::string condition_str;

    if (Utils::ReadAttrString(root, "condition", condition_str))
    {
        condition_str = Utils::ToLowerCase(condition_str);

        if (condition_str == "default")
        {
            condition = TTC_DEFAULT_CONDITION;
        }
        else if (condition_str == "to_human_cac")
        {
            condition = TTC_TO_HUMAN_CAC_CONDITION;
        }
        else if (condition_str == "to_saiyan_cac")
        {
            condition = TTC_TO_SAIYAN_CAC_CONDITION;
        }
        else if (condition_str == "to_namekian_cac")
        {
            condition = TTC_TO_NAMEKIAN_CAC_CONDITION;
        }
        else if (condition_str == "to_freezer_cac")
        {
            condition = TTC_TO_FREEZER_CAC_CONDITION;
        }
        else if (condition_str == "to_majin_cac")
        {
            condition = TTC_TO_MAJIN_CAC_CONDITION;
        }
        else if (condition_str == "teacher")
        {
            condition = TTC_TEACHER_CONDITION;
        }
        else
        {
            DPRINTF("%s: unknown condition \"%s\"\n", FUNCNAME, condition_str.c_str());
            return false;
        }
    }
    else
    {
        condition = TTC_DEFAULT_CONDITION;
    }

    if (!Utils::GetParamUnsigned(root, "COSTUME", &costume)) return false;
    if (!Utils::GetParamUnsigned(root, "TRANSFORMATION", &transformation)) return false;
    if (!Utils::GetParamString(root, "NAME", name)) return false;

    return true;
}

X2mFile::X2mFile()
{
    Reset();
}

X2mFile::~X2mFile()
{

}

void X2mFile::Reset()
{
    ZipFile::DeleteAll();

    type = X2mType::REPLACER;
    format_version = X2M_CURRENT_VERSION;

    mod_name.clear();
    mod_author.clear();
    mod_version = 1.0f;
    Utils::GetRandomData(mod_guid, sizeof(mod_guid));
    Utils::GetRandomData(udata, sizeof(udata));

    entry_name.clear();
    chara_name.clear();
    chara_name.resize(XV2_LANG_NUM);
    mult_chara_names.clear();
    is_ozaru = false;
    is_cellmax = false;
    body_shape = -1;
    can_use_any_dual_skill = false;

    slot_entries.clear();
    cms_entry.id = X2M_INVALID_ID;    
    skill_sets.clear();
    cso_entries.clear();
    psc_entries.clear();
    aur_entries.clear();
    sev_hl_entries.clear();
    sev_ll_entries.clear();
    cml_entries.clear();
    hci_entries.clear();

    chara_skill_depends.clear();
    chara_auras.clear();
    chara_auras_extra.clear();

    custom_audios.clear();

    ttb_hl_entries.clear();
    ttb_ll_entries.clear();

    ttc_hl_entries.clear();
    ttc_ll_entries.clear();

    cnc_entries.clear();    
    char_eepk.clear();

    chara_ss_depends.clear();

    ikd_entries.clear();
    vlc_entry = VlcEntry();
    vlc_entry.cms_id = X2M_INVALID_ID;
    des_levels.clear();

    invisible = false;

    skill_name.clear();
    skill_name.resize(XV2_LANG_NUM);
    skill_desc.clear();
    skill_desc.resize(XV2_LANG_NUM);
    skill_how.clear();
    skill_how.resize(XV2_LANG_NUM);
    skill_type = X2mSkillType::SUPER;
    skill_trans_names.clear();
    skill_entry.id = X2M_INVALID_ID16;
    skill_idb_entry.id = X2M_INVALID_ID16;
    skill_pup_entries.clear();
    skill_aura_entries.clear();
    skill_costume_depend.id = X2M_INVALID_ID;
    skill_bodies.clear();
    blast_ss_intended = false;
    skill_chara_depend.id = X2M_INVALID_ID;

    costume_items.clear();
    costume_partsets.clear();

    stage_name.clear();
    stage_name.resize(XV2_LANG_NUM);
    add_stage_slot = true;
    add_stage_slot_local = false;
    bg_eepk.clear();
    stage_eepk.clear();

    ss_item = X2mItem();
    ss_item.idb.type = 9;
    ss_blast_depend.id = X2M_INVALID_ID;

    // Temp variables
    temp_pup_in.clear();
    temp_pup_out.clear();
    temp_cus_aur_in.clear();
    temp_cus_aur_out.clear();
    temp_aur_in.clear();
    temp_aur_out.clear();
    update_costume_entry = X2mCostumeEntry();
    bodies_map.clear();
    update_ss_entry = X2mSuperSoul();
}

bool X2mFile::Validate(bool write)
{
    if (format_version > X2M_CURRENT_VERSION)
    {
        DPRINTF("This x2m requires a newer version of the installer/creator.\n");
        return false;
    }

    if (mod_name.length() == 0)
    {
        DPRINTF("%s: MOD_NAME cannot be empty.\n", FUNCNAME);
        return false;
    }

    if (mod_author.length() == 0)
    {
        DPRINTF("%s: MOD_AUTHOR cannot be empty.\n", FUNCNAME);
        return false;
    }

    if (type == X2mType::NEW_CHARACTER)
    {
        if (entry_name.length() != 3)
        {
            DPRINTF("%s: ENTRY_NAME must be 3 characters long!\n", FUNCNAME);
            return false;
        }

        if (!Utils::IsAlphaNumeric(entry_name))
        {
            DPRINTF("%s: ENTRY_NAME must be alpha-numeric.\n", FUNCNAME);
            return false;
        }

        if (write && Xenoverse2::IsOriginalChara(entry_name))
        {
            DPRINTF("%s: ENTRY_NAME cannot be an original character for the game.\n", FUNCNAME);
            return false;
        }

        if (!DirExists(entry_name))
        {
            DPRINTF("%s: Directory \"%s\" must exist in the package.\n", FUNCNAME, entry_name.c_str());
            return false;
        }

        if (chara_name[XV2_LANG_ENGLISH].length() == 0)
        {
            DPRINTF("%s: CHARA_NAME_EN cannot be empty.\n", FUNCNAME);
            return false;
        }

        if (!SelPortraitExists())
        {
            DPRINTF("%s: %s doesn't exist.\n", FUNCNAME, X2M_SEL_PORTRAIT);
            return false;
        }

        if (!BtlPortraitExists())
        {
            // Battle portrait was mandatory
            if (format_version <  X2M_MIN_VERSION_AUTO_BTL_PORTRAITS)
            {
                DPRINTF("%s: %s doesn't exist.\n", FUNCNAME, X2M_BTL_PORTRAIT);
                return false;
            }
        }

        if (slot_entries.size() == 0)
        {
            DPRINTF("%s: There must be at least one SlotEntry.\n", FUNCNAME);
            return false;
        }

        if (slot_entries.size() > XV2_MAX_SUBSLOTS)
        {
            DPRINTF("%s: Number of slots entries cannot be bigger than %d\n", FUNCNAME, XV2_MAX_SUBSLOTS);
            return false;
        }

        for (const X2mSlotEntry &entry: slot_entries)
        {
            if (entry.costume_name[XV2_LANG_ENGLISH].length() == 0)
            {
                DPRINTF("%s: COSTUME_NAME_EN is not optional.\n", FUNCNAME);
                return false;
            }            
        }        

        if (cms_entry.id == X2M_INVALID_ID)
        {
            DPRINTF("%s: There is no cms entry!\n", FUNCNAME);
            return false;
        }

        if (skill_sets.size() == 0)
        {
            DPRINTF("%s: There must be at least one SkillSet.\n", FUNCNAME);
            return false;
        }

        if (skill_sets.size() != slot_entries.size())
        {
            DPRINTF("%s: Number of skill sets must be same as number of slot entries.\n", FUNCNAME);
            return false;
        }

        struct Combination
        {
            int costume_index;
            int model_preset;

            inline bool operator==(const Combination &rhs) const
            {
                return (this->costume_index == rhs.costume_index && this->model_preset == rhs.model_preset);
            }

            inline bool operator!=(const Combination &rhs) const
            {
                return !(*this == rhs);
            }
        };

        if (slot_entries.size() > 1)
        {
            std::vector<Combination> combinations;

            for (size_t i = 0; i < slot_entries.size(); i++)
            {
                Combination comb;

                comb.costume_index = slot_entries[i].costume_index;
                comb.model_preset = slot_entries[i].model_preset;

                if (std::find(combinations.begin(), combinations.end(), comb) == combinations.end())
                {
                    combinations.push_back(comb);
                }
                else
                {
                    DPRINTF("%s: Every combination of costume_index+model_preset must be unique.\n", FUNCNAME);
                    return false;
                }
            }
        }

        size_t num_costumes = GetNumCostumes();

        if (HasCso())
        {
            if (cso_entries.size() != num_costumes)
            {
                DPRINTF("%s: The number of cso entries must be equal to the number of different costume_index.\n", FUNCNAME);
                return false;
            }
        }

        if (HasPsc())
        {
            if (psc_entries.size() != num_costumes)
            {
                DPRINTF("%s: The number of psc entries must be equal to the number of different costume_index.\n", FUNCNAME);
                return false;
            }
        }

        if (HasAur())
        {
            if (aur_entries.size() != num_costumes)
            {
                DPRINTF("%s: The number of aur entries must be equal to the number of different costume_index.\n", FUNCNAME);
                return false;
            }
        }

        if (HasSevHL() && HasSevLL())
        {
            DPRINTF("%s: A mod cannot have SevHL and SevLL at the same time.\n", FUNCNAME);
            return false;
        }

        if (HasSevHL())
        {
            std::unordered_set<uint32_t> costumes;

            for (const SevEntryHL &entry : sev_hl_entries)
            {
                if (entry.copy_char == entry_name)
                {
                    DPRINTF("%s: Recursive prevention unit, copy_char cannot be the entry_name of the mod.\n", FUNCNAME);
                    return false;
                }

                costumes.insert(entry.costume_id);
            }

            if (costumes.size() != sev_hl_entries.size())
            {
                DPRINTF("%s: Each costume_id in each sev_hl entry must be unique.\n", FUNCNAME);
                return false;
            }
        }
        else if (HasSevLL())
        {
            std::unordered_set<uint32_t> costumes;

            for (const SevEntryLL &entry : sev_ll_entries)
            {
                for (const SevEventsLL &sub_entry : entry.sub_entries)
                {
                    if (!sub_entry.im_source && sub_entry.char_code.length() == 0)
                    {
                        DPRINTF("%s: In SevEventsLL: if im_source is false, char_code cannot be empty.\n", FUNCNAME);
                        return false;
                    }
                }

                costumes.insert(entry.costume_id);
            }

            if (costumes.size() != sev_ll_entries.size())
            {
                DPRINTF("%s: Each costume_id in each sev_ll entry must be unique.\n", FUNCNAME);
                return false;
            }
        }

        if (HasCml())
        {
            if (cml_entries.size() != num_costumes)
            {
                DPRINTF("%s: The number of cml entries must be equal to the number of different costume_index.\n", FUNCNAME);
                return false;
            }
        }

        if (format_version >= X2M_MIN_VERSION_CSS_AUDIO)
        {
            for (size_t i = 0; i < slot_entries.size(); i++)
            {
                const X2mSlotEntry &slot = slot_entries[i];

                for (size_t j = 0; j < 2; j++)
                {
                    if (slot.audio_files[j].length() > 0)
                    {
                        if (!AudioFileExists(slot.audio_files[j]))
                        {
                            DPRINTF("%s: The audio file with base name \"%s\" doesn't exist!\n", FUNCNAME, slot.audio_files[j].c_str());
                            return false;
                        }
                    }
                }
            }
        }

        if (HasCharaSkillDepends())
        {
            std::unordered_set<std::string> dep_guids;
            std::unordered_set<uint32_t> dep_ids;

            for (const X2mDepends &dep : chara_skill_depends)
            {
                const std::string guid_str = Utils::GUID2String(dep.guid);

                if (dep_guids.find(guid_str) != dep_guids.end())
                {
                    DPRINTF("%s: Chara Skill Depends with guid %s was previously defined.\n", FUNCNAME, guid_str.c_str());
                    return false;
                }

                if (dep_ids.find(dep.id) != dep_ids.end())
                {
                    DPRINTF("%s: Chara Skill Depends with id 0x%x was previously defined.\n", FUNCNAME, dep.id);
                    return false;
                }

                if (dep.id < X2M_SKILL_DEPENDS_BEGIN || dep.id >= X2M_SKILL_DEPENDS_END)
                {
                    DPRINTF("%s: id 0x%x is outside of the valid range for Chara Skill Depends.\n", FUNCNAME, dep.id);
                    return false;
                }

                if (!IsCharaSkillDependsReferenced(dep))
                {
                    DPRINTF("%s: Chara Skill Depends with id 0x%x is not referenced in the CusSkillSets.\n", FUNCNAME, dep.id);
                    return false;
                }

                dep_guids.insert(guid_str);
                dep_ids.insert(dep.id);
            }
        }

        if (chara_auras.size() != 0 && chara_auras.size() != aur_entries.size())
        {
            DPRINTF("%s: chara_auras size must match aur entries or be 0.\n", FUNCNAME);
            return false;
        }

        if (chara_auras.size() != 0 && chara_auras.size() != chara_auras_extra.size())
        {
            DPRINTF("%s: chara_auras_extra size must match chara_auras size.\n", FUNCNAME);
            return false;
        }

        if (HasCustomAudio())
        {
            for (const X2mCustomAudio &audio : custom_audios)
            {
                if (!AudioFileExists(audio.name))
                {
                    DPRINTF("%s: The audio file with base name \"%s\" doesn't exist!\n", FUNCNAME, audio.name.c_str());
                    return false;
                }
            }
        }

        if (HasTtbHL())
        {
            if (HasTtbLL())
            {
                DPRINTF("%s: A mod cannot have TTB HL and TTB LL at the same time.\n", FUNCNAME);
                return false;
            }

            std::unordered_set<uint32_t> costumes;

            for (const TtbEntryHL &entry : ttb_hl_entries)
            {
                if (entry.copy_char == entry_name)
                {
                    DPRINTF("%s: Recursive prevention unit, copy_char cannot be the entry_name of the mod.\n", FUNCNAME);
                    return false;
                }

                costumes.insert(entry.costume_id);
            }

            if (costumes.size() != ttb_hl_entries.size())
            {
                DPRINTF("%s: Each costume_id in each ttb_hl entry must be unique.\n", FUNCNAME);
                return false;
            }
        }
        else if (HasTtbLL())
        {
            for (const  TtbEntryLL &entry : ttb_ll_entries)
            {
                int num_myself = 0;

                if (entry.cms_code.length() == 0)
                {
                    num_myself++;
                }
                else
                {
                    if (entry.cms_code.length() != 3 || !Utils::IsAlphaNumeric(entry.cms_code))
                    {
                        DPRINTF("%s: Invalid cms (in TTB) %s\n", FUNCNAME, entry.cms_code.c_str());
                        return false;
                    }
                }

                if (entry.event_res1.name.length() == 0)
                {
                    DPRINTF("%s: An empty ttb event was found.\n", FUNCNAME);
                    return false;
                }

                if (entry.cms2_code.length() == 0)
                {
                    num_myself++;
                }
                else
                {
                    if (entry.cms2_code.length() != 3 || !Utils::IsAlphaNumeric(entry.cms2_code))
                    {
                        DPRINTF("%s: Invalid cms (in TTB) %s\n", FUNCNAME, entry.cms2_code.c_str());
                        return false;
                    }
                }

                if (entry.event_res2.name.length() == 0)
                {
                    DPRINTF("%s: An empty ttb event was found.\n", FUNCNAME);
                    return false;
                }

                if (entry.num_actors <= 2)
                    goto check_myself;

                if (entry.cms3_code.length() == 0)
                {
                    num_myself++;
                }
                else
                {
                    if (entry.cms3_code.length() != 3 || !Utils::IsAlphaNumeric(entry.cms3_code))
                    {
                        DPRINTF("%s: Invalid cms (in TTB) %s\n", FUNCNAME, entry.cms3_code.c_str());
                        return false;
                    }
                }

                if (entry.event_res3.name.length() == 0)
                {
                    DPRINTF("%s: An empty ttb event was found.\n", FUNCNAME);
                    return false;
                }

                if (entry.num_actors <= 3)
                    goto check_myself;

                if (entry.cms4_code.length() == 0)
                {
                    num_myself++;
                }
                else
                {
                    if (entry.cms4_code.length() != 3 || !Utils::IsAlphaNumeric(entry.cms4_code))
                    {
                        DPRINTF("%s: Invalid cms (in TTB) %s\n", FUNCNAME, entry.cms4_code.c_str());
                        return false;
                    }
                }

                if (entry.event_res4.name.length() == 0)
                {
                    DPRINTF("%s: An empty ttb event was found.\n", FUNCNAME);
                    return false;
                }

                if (entry.num_actors <= 4)
                    goto check_myself;

                if (entry.cms5_code.length() == 0)
                {
                    num_myself++;
                }
                else
                {
                    if (entry.cms5_code.length() != 3 || !Utils::IsAlphaNumeric(entry.cms5_code))
                    {
                        DPRINTF("%s: Invalid cms (in TTB) %s\n", FUNCNAME, entry.cms5_code.c_str());
                        return false;
                    }
                }

                if (entry.event_res5.name.length() == 0)
                {
                    DPRINTF("%s: An empty ttb event was found.\n", FUNCNAME);
                    return false;
                }

check_myself:

                if (num_myself == 0)
                {
                    DPRINTF("%s: Current char is not referenced in TTB.\n", FUNCNAME);
                    return false;
                }
                else if (num_myself != 1)
                {
                    DPRINTF("%s: Current char is referenced more than once (in TTB).\n", FUNCNAME);
                    return false;
                }

            } // end for
        }

        if (HasTtcHL())
        {
            if (HasTtcLL())
            {
                DPRINTF("%s: A mod cannot have TTC HL and TTC LL at the same time.\n", FUNCNAME);
                return false;
            }

            std::unordered_set<uint32_t> costumes;

            for (const TtcEntryHL &entry : ttc_hl_entries)
            {
                if (entry.copy_char == entry_name)
                {
                    DPRINTF("%s: Recursive prevention unit, copy_char cannot be the entry_name of the mod.\n", FUNCNAME);
                    return false;
                }

                costumes.insert(entry.costume_id);
            }

            if (costumes.size() != ttc_hl_entries.size())
            {
                DPRINTF("%s: Each costume_id in each ttc_hl entry must be unique.\n", FUNCNAME);
                return false;
            }
        }
        else if (HasTtcLL())
        {
            for (const TtcEntryLL &entry : ttc_ll_entries)
            {
                if (Utils::IsEmptyString(entry.name))
                {
                    DPRINTF("%s: TTC LL event name cannot be empty.\n", FUNCNAME);
                    return false;
                }
            }
        }

        if (HasCnc())
        {
            if (can_use_any_dual_skill)
            {
                DPRINTF("%s: CNC can't b e used with CAN_USE_ANY_DUAL_SKILL=TRUE\n", FUNCNAME);
                return false;
            }

            std::vector<Combination> combinations;

            for (size_t i = 0; i < cnc_entries.size(); i++)
            {
                Combination comb;

                comb.costume_index = cnc_entries[i].costume_index;
                comb.model_preset = cnc_entries[i].model_preset;

                if (!FindSlotEntry(comb.costume_index, comb.model_preset))
                {
                    DPRINTF("%s: CNC: costume index %d model_preset %d doesn't exist.\n", FUNCNAME, comb.costume_index, comb.model_preset);
                    return false;
                }

                if (std::find(combinations.begin(), combinations.end(), comb) == combinations.end())
                {
                    combinations.push_back(comb);
                }
                else
                {
                    DPRINTF("%s: Every combination of costume_index+model_preset must be unique (CNC).\n", FUNCNAME);
                    return false;
                }
            }
        }

        if (HasCharaSsDepends())
        {
            std::unordered_set<std::string> dep_guids;
            std::unordered_set<uint32_t> dep_ids;

            for (const X2mDepends &dep : chara_ss_depends)
            {
                const std::string guid_str = Utils::GUID2String(dep.guid);

                if (dep_guids.find(guid_str) != dep_guids.end())
                {
                    DPRINTF("%s: Chara Supersoul Depends with guid %s was previously defined.\n", FUNCNAME, guid_str.c_str());
                    return false;
                }

                if (dep_ids.find(dep.id) != dep_ids.end())
                {
                    DPRINTF("%s: Chara Supersoul Depends with id 0x%x was previously defined.\n", FUNCNAME, dep.id);
                    return false;
                }

                if (dep.id < X2M_SS_DEPENDS_BEGIN || dep.id >= X2M_SS_DEPENDS_END)
                {
                    DPRINTF("%s: id 0x%x is outside of the valid range for Chara Supersoul Depends.\n", FUNCNAME, dep.id);
                    return false;
                }

                if (!IsCharaSsDependsReferenced(dep))
                {
                    DPRINTF("%s: Chara Skill Depends with id 0x%x is not referenced in any psc entry.\n", FUNCNAME, dep.id);
                    return false;
                }

                dep_guids.insert(guid_str);
                dep_ids.insert(dep.id);
            }
        }

        if (HasIkd())
        {
            if (ikd_entries.size() != num_costumes && ikd_entries.size() != num_costumes*2)
            {
                DPRINTF("%s: Number of IKD entries must be either 0, equal to number of costumes, or double of number of costumes (to enable lobby).\n", FUNCNAME);
                return false;
            }
        }
    }
    else if (type == X2mType::NEW_SKILL)
    {
        if (skill_type != X2mSkillType::BLAST || blast_ss_intended)
        {
            if (skill_name[XV2_LANG_ENGLISH].length() == 0)
            {
                DPRINTF("%s: SKILL_NAME_EN cannot be empty.\n", FUNCNAME);
                return false;
            }

            if (skill_type != X2mSkillType::BLAST)
            {
                if (skill_desc[XV2_LANG_ENGLISH].length() == 0)
                {
                    for (int i = 0; i < XV2_LANG_NUM; i++)
                    {
                        if (i == XV2_LANG_ENGLISH)
                            continue;

                        if (skill_desc[XV2_LANG_ENGLISH].length() != 0)
                        {
                            DPRINTF("%s: skill description in language %d, but not in english.\n", FUNCNAME, i);
                            return false;
                        }
                    }
                }

                if (format_version >= X2M_MIN_VERSION_SKILL_HOW && skill_how[XV2_LANG_ENGLISH].length() == 0)
                {
                    for (int i = 0; i < XV2_LANG_NUM; i++)
                    {
                        if (i == XV2_LANG_ENGLISH)
                            continue;

                        if (skill_how[XV2_LANG_ENGLISH].length() != 0)
                        {
                            DPRINTF("%s: skill how in language %d, but not in english.\n", FUNCNAME, i);
                            return false;
                        }
                    }
                }
            }
        }

        if (skill_type == X2mSkillType::AWAKEN)
        {
            if (skill_trans_names.size() == 0)
            {
                DPRINTF("%s: Awaken skill, but no transformation names.\n", FUNCNAME);
                return false;
            }

            for (const X2mSkillTransName &tn : skill_trans_names)
            {
                if (tn.trans_name[XV2_LANG_ENGLISH].length() == 0)
                {
                    DPRINTF("%s: Tansformation name empty for english language.\n", FUNCNAME);
                    return false;
                }
            }
        }

        if (!SkillDirectoryExists())
        {
            DPRINTF("%s: Skill directory doesn't exist.\n", FUNCNAME);
            return false;
        }

        if (skill_entry.id == X2M_INVALID_ID16)
        {
            DPRINTF("%s: There is no skill entry!\n", FUNCNAME);
            return false;
        }

        if (skill_entry.name.length() != 3 && skill_entry.name.length() != 4)
        {
            DPRINTF("%s: Skill name in <Skill> must be 3 or 4 character string.\n", FUNCNAME);
            return false;
        }

        if (HasSkillCostumeDepend())
        {
            if (!IsSkillCostumeDependReferenced())
            {
                DPRINTF("%s: Skill Costume Depends is not referenced in the cus entry.\n", FUNCNAME);
                return false;

            }
        }

        if (HasSkillCharaDepend())
        {
            if (!IsSkillCharaDependReferenced())
            {
                DPRINTF("%s: Skill Chara Depends is not referenced in the cus entry.\n", FUNCNAME);
                return false;

            }
        }

        if (HasSkillBodies())
        {
            std::unordered_set<int> bodies_ids;

            for (const X2mBody &body : skill_bodies)
            {
                if (body.body.IsEmpty())
                {
                    DPRINTF("%s: X2mBody: BcsBody empty found.\n", FUNCNAME);
                    return false;
                }

                if (body.id < X2M_SKILL_BODY_ID_BEGIN)
                {
                    DPRINTF("%s: X2mBody: id cannot be smaller than %d\n", FUNCNAME, X2M_SKILL_BODY_ID_BEGIN);
                    return false;
                }

                bodies_ids.insert(body.id);
            }

            if (bodies_ids.size() != skill_bodies.size())
            {
                DPRINTF("%s: Some X2mBody has a repeated body id.\n", FUNCNAME);
                return false;
            }
        }
    }    
    else if (type == X2mType::NEW_COSTUME)
    {
        if (costume_partsets.size() == 0)
        {
            DPRINTF("%s: There must be at least one <PartSet>.\n", FUNCNAME);
            return false;
        }

        int sets_count = -1;

        for (uint8_t race = 0; race < X2M_CR_NUM; race++)
        {
            size_t count = GetNumCostumePartSets(race);

            if (count != 0)
            {
                if (sets_count == -1)
                {
                    sets_count = (int)sets_count;
                }
                else if (sets_count != (int)count)
                {
                    DPRINTF("%s: The number of sets for all races must be the same.\n", FUNCNAME);
                    return false;
                }
            }
        }

        for (const X2mItem &item : costume_items)
        {
            if (item.idb.model >= GetEffectiveNumCostumePartSets())
            {
                DPRINTF("%s: idb model cannot be >= than number of partsets.\n", FUNCNAME);
                return false;
            }

            if (item.item_type == X2mItemType::SUPER_SOUL)
            {
                DPRINTF("%s: Invalid type for X2mItem of a costume.\n", FUNCNAME);
                return false;
            }
        }
    }
    else if (type == X2mType::NEW_STAGE)
    {
        if (stage_name[XV2_LANG_ENGLISH].length() == 0)
        {
            DPRINTF("%s: STAGE_NAME_EN cannot be empty.\n", FUNCNAME);
            return false;
        }

        if (!StageDirectoryExists())
        {
            DPRINTF("%s: %s doesn't exist.\n", FUNCNAME, X2M_STAGE);
            return false;
        }

        if (!StageLightingExists())
        {
            DPRINTF("%s: %s doesn't exist.\n", FUNCNAME, X2M_STAGE_LIGHTING);
            return false;
        }

        if (!StageSelPortraitExists())
        {
            DPRINTF("%s: %s doesn't exist.\n", FUNCNAME, X2M_STAGE_SEL_PORTRAIT);
            return false;
        }

        if (!StageQstPortraitExists())
        {
            DPRINTF("%s: %s doesn't exist.\n", FUNCNAME, X2M_STAGE_QST_PORTRAIT);
            return false;
        }

        if (!StageSelBackgroundExists())
        {
            DPRINTF("%s: %s doesn't exist.\n", FUNCNAME, X2M_STAGE_SEL_BACKGROUND);
            return false;
        }

        if (stage_def.code.length() != 5)
        {
            DPRINTF("%s: Code of stage def must be 5 characters long.\n", FUNCNAME);
            return false;
        }

        if (!Utils::IsAlphaNumeric(stage_def.code))
        {
            DPRINTF("%s: Code of stage def must be alpha-numeric.\n", FUNCNAME);
            return false;
        }

        if (Utils::BeginsWith(stage_def.code, "ICO", false))
        {
            DPRINTF("%s: Code of stage def cannot start with ICO.\n", FUNCNAME);
            return false;
        }

        if (Utils::BeginsWith(stage_def.code, "X2M", false))
        {
            DPRINTF("%s: Code of stage def cannot start with X2M.\n", FUNCNAME);
            return false;
        }

        if (Xenoverse2::IsForbiddenNewStageName(stage_def.code))
        {
            DPRINTF("%s: Code of stage def collides with original content.\n", FUNCNAME);
            return false;
        }

        const std::string map_file = Utils::MakePathString(X2M_STAGE, stage_def.code + ".map");
        if (!FileExists(map_file))
        {
            DPRINTF("%s: %s doesn't exist.\n", FUNCNAME, map_file.c_str());
            return false;
        }
    }
    else if (type == X2mType::NEW_QUEST)
    {
        if (!QuestDirectoryExists())
        {
            DPRINTF("%s: QUEST directory is mandatory for x2m quests.\n", FUNCNAME);
            return false;
        }
    }
    else if (type == X2mType::NEW_SUPERSOUL)
    {
        if (ss_item.item_name[XV2_LANG_ENGLISH].length() == 0)
        {
            DPRINTF("%s: Name cannot be empty.\n", FUNCNAME);
            return false;
        }

        if (ss_item.item_desc[XV2_LANG_ENGLISH].length() == 0)
        {
            for (int i = 0; i < XV2_LANG_NUM; i++)
            {
                if (i == XV2_LANG_ENGLISH)
                    continue;

                if (ss_item.item_desc[XV2_LANG_ENGLISH].length() != 0)
                {
                    DPRINTF("%s: description in language %d, but not in english.\n", FUNCNAME, i);
                    return false;
                }
            }
        }

        if (ss_item.item_how[XV2_LANG_ENGLISH].length() == 0)
        {
            for (int i = 0; i < XV2_LANG_NUM; i++)
            {
                if (i == XV2_LANG_ENGLISH)
                    continue;

                if (ss_item.item_how[XV2_LANG_ENGLISH].length() != 0)
                {
                    DPRINTF("%s: how in language %d, but not in english.\n", FUNCNAME, i);
                    return false;
                }
            }
        }

    }
    else if (type == X2mType::REPLACER)
    {
        if (!JungleExists())
        {
            DPRINTF("%s: Jungle must exist for a replacer mod.\n", FUNCNAME);
            return false;
        }
    }

    else
    {
        DPRINTF("%s: Unknown mod type.\n", FUNCNAME);
        return false;
    }

    return true;
}

bool X2mFile::Decompile()
{
    if (!Validate(true))
        return false;

    TiXmlDocument doc;

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc.LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("X2M");
    std::string temp_str;

    if (type == X2mType::REPLACER)
        temp_str = "REPLACER";
    else if (type == X2mType::NEW_SKILL)
        temp_str = "NEW_SKILL";
    else if (type == X2mType::NEW_COSTUME)
        temp_str = "NEW_COSTUME";
    else if (type == X2mType::NEW_STAGE)
        temp_str = "NEW_STAGE";
    else if (type == X2mType::NEW_QUEST)
        temp_str = "NEW_QUEST";
    else if (type == X2mType::NEW_SUPERSOUL)
        temp_str = "NEW_SUPERSOUL";
    else
        temp_str = "NEW_CHARACTER";

    root->SetAttribute("type", temp_str);
    Utils::WriteParamFloat(root, "X2M_FORMAT_VERSION", format_version);

    Utils::WriteParamString(root, "MOD_NAME", mod_name);
    Utils::WriteParamString(root, "MOD_AUTHOR", mod_author);
    Utils::WriteParamFloat(root, "MOD_VERSION", mod_version);
    Utils::WriteParamGUID(root, "MOD_GUID", mod_guid);

    if (format_version >= X2M_MIN_VERSION_UDATA)
        Utils::WriteParamString(root, "UDATA", Utils::Base64Encode(udata, sizeof(udata), false));

    if (type == X2mType::NEW_CHARACTER)
    {
        Utils::WriteParamString(root, "ENTRY_NAME", entry_name);

        if (is_ozaru)
        {
            Utils::WriteParamString(root, "IS_OZARU", "true");
        }
        else if (is_cellmax)
        {
            Utils::WriteParamString(root, "IS_CELLMAX", "true");
        }

        if (body_shape >= 0)
        {
            Utils::WriteParamUnsigned(root, "BODY_SHAPE", body_shape);
        }

        if (can_use_any_dual_skill)
        {
            Utils::WriteParamBoolean(root, "CAN_USE_ANY_DUAL_SKILL", true);
        }

        if (invisible)
        {
            Utils::WriteParamBoolean(root, "INVISIBLE", true);
        }

        for (size_t i = 0; i < XV2_LANG_NUM; i++)
        {
            const std::string &name = chara_name[i];

            if (name.length() == 0)
                continue;

            temp_str = "CHARA_NAME_" + Utils::ToUpperCase(xv2_lang_codes[i]);
            Utils::WriteParamString(root, temp_str.c_str(), name);
        }

        if (HasMultNames())
        {
            for (const X2mCharaName &name : mult_chara_names)
            {
                if (!name.Decompile(root))
                    return false;
            }
        }

        for (const X2mSlotEntry &entry : slot_entries)
        {
            if (!entry.Decompile(root, format_version >= X2M_MIN_VERSION_CSS_AUDIO))
                return false;
        }

        cms_entry.id = X2M_DUMMY_ID;

        if (!cms_entry.Decompile(root))
            return false;

        for (CusSkillSet &entry : skill_sets)
        {
            entry.char_id = entry.costume_id = X2M_DUMMY_ID;
            entry.model_preset = X2M_DUMMY_ID16;

            if (!entry.Decompile(root))
                return false;
        }

        for (CsoEntry &entry : cso_entries)
        {
            entry.char_id = entry.costume_id = X2M_DUMMY_ID;

            if (!entry.Decompile(root))
                return false;
        }

        for (PscSpecEntry &entry : psc_entries)
        {
            entry.costume_id = entry.costume_id2 = X2M_DUMMY_ID;

            if (!entry.Decompile(root))
                return false;
        }

        for (AurCharaLink &entry : aur_entries)
        {
            entry.char_id = entry.costume_id = X2M_DUMMY_ID;

            if (!entry.Decompile(root))
                return false;
        }

        for (const SevEntryHL &entry : sev_hl_entries)
        {
            if (!entry.Decompile(root))
                return false;
        }

        for (const SevEntryLL &entry : sev_ll_entries)
        {
            if (!entry.Decompile(root))
                return false;
        }

        for (CmlEntry &entry : cml_entries)
        {
            entry.char_id = entry.costume_id = X2M_DUMMY_ID16;

            if (!entry.Decompile(root))
                return false;
        }

        std::vector<HciEntry> used_hci_entries;

        for (HciEntry &entry : hci_entries)
        {
            entry.char_id = X2M_DUMMY_ID16;

            if (std::find(used_hci_entries.begin(), used_hci_entries.end(), entry) == used_hci_entries.end())
            {
                used_hci_entries.push_back(entry);

                if (!entry.Decompile(root))
                    return false;
            }
        }

        for (const X2mDepends &dep : chara_skill_depends)
        {
            if (!dep.Decompile(root))
                return false;
        }

        for (AurAura &aur : chara_auras)
        {
            aur.id = X2M_DUMMY_ID;

            if (!aur.Decompile(root, aura_types))
                return false;
        }

        for (AuraExtraData &aed : chara_auras_extra)
        {
            aed.aur_id = X2M_DUMMY_ID;

            if (!aed.Decompile(root))
                return false;
        }

        for (const X2mCustomAudio &audio : custom_audios)
        {
            if (!audio.Decompile(root))
                return false;
        }

        for (const TtbEntryHL &entry_hl : ttb_hl_entries)
        {
            if (!entry_hl.Decompile(root))
                return false;
        }

        for (TtbEntryLL &entry_ll : ttb_ll_entries)
        {
            if (entry_ll.num_actors < 5)
            {
                entry_ll.cms5_code.clear();
                entry_ll.event_res5.Clear();
                entry_ll.costume5 = entry_ll.transformation5 = 0;
            }

            if (entry_ll.num_actors < 4)
            {
                entry_ll.cms4_code.clear();
                entry_ll.event_res4.Clear();
                entry_ll.costume4 = entry_ll.transformation4 = 0;
            }

            if (entry_ll.num_actors < 3)
            {
                entry_ll.cms3_code.clear();
                entry_ll.event_res3.Clear();
                entry_ll.costume3 = entry_ll.transformation3 = 0;
            }

            if (!entry_ll.Decompile(root))
                return false;
        }

        for (const TtcEntryHL &entry_hl : ttc_hl_entries)
        {
            if (!entry_hl.Decompile(root))
                return false;
        }

        for (const TtcEntryLL &entry_ll : ttc_ll_entries)
        {
            if (!entry_ll.Decompile(root))
                return false;
        }

        for (CncEntry &entry : cnc_entries)
        {
            entry.cms_id = X2M_DUMMY_ID16;

            if (!entry.Decompile(root))
                return false;
        }

        if (char_eepk.length() > 0)
            Utils::WriteParamString(root, "CHAR_EEPK", char_eepk);

        for (const X2mDepends &dep : chara_ss_depends)
        {
            if (!dep.Decompile(root))
                return false;
        }

        for (IkdEntry &entry : ikd_entries)
        {
            entry.cms_id = X2M_DUMMY_ID16;

            if (!entry.Decompile(root))
                return false;
        }

        if (vlc_entry.cms_id != X2M_INVALID_ID)
        {
            vlc_entry.cms_id = X2M_DUMMY_ID;

            if (!vlc_entry.Decompile(root))
                return false;
        }

        for (const DestructionLevel &dl : des_levels)
        {
            if (!dl.Decompile(root))
                return false;
        }
    }
    else if (type == X2mType::NEW_SKILL)
    {
        if (skill_type == X2mSkillType::SUPER)
            temp_str = "SUPER";
        else if (skill_type == X2mSkillType::ULTIMATE)
            temp_str = "ULTIMATE";
        else if (skill_type == X2mSkillType::EVASIVE)
            temp_str = "EVASIVE";
        else if (skill_type == X2mSkillType::BLAST)
            temp_str = "BLAST";
        else
            temp_str = "AWAKEN";

        Utils::WriteParamString(root, "SKILL_TYPE", temp_str);

        if (skill_type != X2mSkillType::BLAST || blast_ss_intended)
        {
            for (size_t i = 0; i < XV2_LANG_NUM; i++)
            {
                const std::string &name = skill_name[i];

                if (name.length() == 0)
                    continue;

                temp_str = "SKILL_NAME_" + Utils::ToUpperCase(xv2_lang_codes[i]);
                Utils::WriteParamString(root, temp_str.c_str(), name);
            }

            if (skill_type != X2mSkillType::BLAST)
            {
                for (size_t i = 0; i < XV2_LANG_NUM; i++)
                {
                    const std::string &desc = skill_desc[i];

                    if (desc.length() == 0)
                        continue;

                    temp_str = "SKILL_DESC_" + Utils::ToUpperCase(xv2_lang_codes[i]);
                    Utils::WriteParamString(root, temp_str.c_str(), desc);
                }

                for (size_t i = 0; i < XV2_LANG_NUM; i++)
                {
                    const std::string &how = skill_how[i];

                    if (how.length() == 0)
                        continue;

                    temp_str = "SKILL_HOW_" + Utils::ToUpperCase(xv2_lang_codes[i]);
                    Utils::WriteParamString(root, temp_str.c_str(), how);
                }
            }
        }

        if (skill_type == X2mSkillType::AWAKEN)
        {
            for (const X2mSkillTransName &tn : skill_trans_names)
            {
                if (!tn.Decompile(root))
                    return false;
            }
        }

        skill_entry.id = skill_entry.id2 = X2M_DUMMY_ID16;

        if (!skill_entry.Decompile(root))
            return false;

        if (HasSkillIdbEntry())
        {
            skill_idb_entry.id = skill_idb_entry.name_id = skill_idb_entry.desc_id = skill_idb_entry.type = X2M_DUMMY_ID16;

            if (!skill_idb_entry.Decompile(root, IdbCommentType::NONE))
                return false;
        }

        if (HasSkillPup())
        {
            for (PupEntry &entry : skill_pup_entries)
            {
                entry.id = X2M_DUMMY_ID;
                if (!entry.Decompile(root))
                    return false;
            }
        }

        if (HasSkillAura())
        {
            for (X2mSkillAura &aura : skill_aura_entries)
            {
                aura.data.cus_aura_id = X2M_DUMMY_ID16;
                aura.aura.id = X2M_DUMMY_ID;

                // These two values must always be synchronized
                if (aura.data.aur_aura_id == X2M_INVALID_ID16)
                    aura.extra.aur_id = X2M_INVALID_ID;
                else
                    aura.extra.aur_id = -1;

                if (!aura.Decompile(root))
                    return false;
            }
        }

        if (HasSkillCostumeDepend())
        {
            if (!skill_costume_depend.Decompile(root))
                return false;
        }


        if (HasSkillCharaDepend())
        {
            if (!skill_chara_depend.Decompile(root))
                return false;
        }

        if (HasSkillBodies())
        {
            for (const X2mBody &body : skill_bodies)
            {
                if (!body.Decompile(root))
                    return false;
            }
        }

        if (skill_type == X2mSkillType::BLAST)
            Utils::WriteParamBoolean(root, "BLAST_SS_INTENDED", blast_ss_intended);
    }
    else if (type == X2mType::NEW_COSTUME)
    {
        for (X2mItem &item : costume_items)
        {
            item.idb.id = item.idb.name_id = item.idb.desc_id = X2M_DUMMY_ID16;
            if (!item.Decompile(root))
                return false;
        }

        for (const X2mPartSet &set : costume_partsets)
        {
            if (!set.Decompile(root))
                return false;
        }
    }
    else if (type == X2mType::NEW_STAGE)
    {
        for (size_t i = 0; i < XV2_LANG_NUM; i++)
        {
            const std::string &name = stage_name[i];

            if (name.length() == 0)
                continue;

            temp_str = "STAGE_NAME_" + Utils::ToUpperCase(xv2_lang_codes[i]);
            Utils::WriteParamString(root, temp_str.c_str(), name);
        }

        stage_def.ssid = -1;
        if (!stage_def.Decompile(root, X2M_DUMMY_ID))
            return false;

        Utils::WriteParamBoolean(root, "ADD_SLOT", add_stage_slot);

        if (!add_stage_slot)
            add_stage_slot_local = false;

        Utils::WriteParamBoolean(root, "ADD_SLOT_LOCAL", add_stage_slot_local);

        if (bg_eepk.length() > 0)
            Utils::WriteParamString(root, "STAGE_BG_EEPK", bg_eepk);

        if (stage_eepk.length() > 0)
            Utils::WriteParamString(root, "STAGE_EEPK", stage_eepk);
    }
    else if (type == X2mType::NEW_SUPERSOUL)
    {
        ss_item.has_how = true;

        if (!ss_item.Decompile(root))
            return false;

        if (HasSSSkillDepend() && !ss_blast_depend.Decompile(root))
            return false;
    }

    doc.LinkEndChild(root);

    TiXmlPrinter printer;
    doc.Accept(&printer);

    const char *xml_buf = printer.CStr();
    return WriteFile("x2m.xml", xml_buf, strlen(xml_buf));
}

bool X2mFile::Compile()
{
    TiXmlDocument doc;
    char *xml_buf;

    xml_buf = ReadTextFile("x2m.xml");
    if (!xml_buf)
    {
        DPRINTF("%s: x2m.xml not found.\n", FUNCNAME);
        return false;
    }

    doc.Parse(xml_buf);
    delete[] xml_buf;

    if (doc.ErrorId() != 0)
    {
        DPRINTF("%s: cannot parse xml (wrong formatted xml?)\n\n"
                "This is what tinyxml says: %s (%d). Line=%d, col=%d\n", FUNCNAME, doc.ErrorDesc(), doc.ErrorId(), doc.ErrorRow(), doc.ErrorCol());

        return false;
    }

    TiXmlHandle handle(&doc);
    TiXmlElement *root = Utils::FindRoot(&handle, "X2M");
    if (!root)
    {
        DPRINTF("%s: Cannot find \"X2M\" in xml.\n", FUNCNAME);
        return false;
    }

    if (!Utils::GetParamFloat(root, "X2M_FORMAT_VERSION", &format_version))
        return false;

    std::string temp_str;

    if (!Utils::ReadAttrString(root, "type", temp_str))
    {
        DPRINTF("%s: type attribute is not optional.\n", FUNCNAME);
        return false;
    }

    if (temp_str == "REPLACER")
    {
        type = X2mType::REPLACER;
    }
    else if (temp_str == "NEW_CHARACTER")
    {
        type = X2mType::NEW_CHARACTER;
    }
    else
    {
        if (format_version >= X2M_MIN_VERSION_SKILLS && temp_str == "NEW_SKILL")
        {
            type = X2mType::NEW_SKILL;
        }
        else if (format_version >= X2M_MIN_VERSION_COSTUME && temp_str == "NEW_COSTUME")
        {
            type = X2mType::NEW_COSTUME;
        }
        else if (format_version >= X2M_MIN_VERSION_STAGE && temp_str == "NEW_STAGE")
        {
            type = X2mType::NEW_STAGE;
        }
        else if (format_version >= X2M_MIN_VERSION_QUEST && temp_str == "NEW_QUEST")
        {
            type = X2mType::NEW_QUEST;
        }
        else if (format_version >= X2M_MIN_VERSION_SUPERSOUL && temp_str == "NEW_SUPERSOUL")
        {
            type = X2mType::NEW_SUPERSOUL;
        }
        else
        {
            if (temp_str == "NEW_SKILL" || temp_str == "NEW_COSTUME" || temp_str == "NEW_STAGE" || temp_str == "NEW_QUEST" || temp_str == "NEW_SUPERSOUL")
            {
                DPRINTF("This mod type requires a newer version of the installer.\n");
            }
            else
            {
                DPRINTF("%s: Unrecognized type: \"%s\"\n", FUNCNAME, temp_str.c_str());
            }

            return false;
        }
    }

    if (format_version >= X2M_MIN_VERSION_UDATA && type != X2mType::REPLACER)
    {
        if (!Utils::GetParamString(root, "UDATA", temp_str))
            return false;

        size_t size;
        uint8_t *temp_buf = Utils::Base64Decode(temp_str, &size);

        if (!temp_buf || size != sizeof(udata))
        {
            DPRINTF("%s: corrupted file.\n", FUNCNAME);
            if (temp_buf)
                delete[] temp_buf;

            return false;
        }

        memcpy(udata, temp_buf, sizeof(udata));
        delete[] temp_buf;
    }

    if (!Utils::GetParamString(root, "MOD_NAME", mod_name))
        return false;

    if (!Utils::GetParamString(root, "MOD_AUTHOR", mod_author))
        return false;

    if (!Utils::GetParamFloat(root, "MOD_VERSION", &mod_version))
        return false;

    if (!Utils::GetParamGUID(root, "MOD_GUID", mod_guid))
        return false;

    if (type == X2mType::NEW_CHARACTER)
    {
        if (!Utils::GetParamString(root, "ENTRY_NAME", entry_name))
            return false;

        entry_name = Utils::ToUpperCase(entry_name);

        for (int i = 0; i < XV2_LANG_NUM; i++)
        {
            temp_str = "CHARA_NAME_" + Utils::ToUpperCase(xv2_lang_codes[i]);
            Utils::ReadParamString(root, temp_str.c_str(), chara_name[i]);
        }

        std::string ozaru_str, cm_str;

        if (format_version >= X2M_MIN_VERSION_OZARU && Utils::ReadParamString(root, "IS_OZARU", ozaru_str))
        {
            is_ozaru = (Utils::ToLowerCase(ozaru_str) == "true" || ozaru_str == "1");
        }
        else
        {
            is_ozaru = false; // Not really needed, done at Reset
        }

        if (format_version >= X2M_MIN_VERSION_CELLMAX && Utils::ReadParamString(root, "IS_CELLMAX", cm_str) && !is_ozaru)
        {
            is_cellmax = (Utils::ToLowerCase(cm_str) == "true" || cm_str == "1");
        }
        else
        {
            is_cellmax = false; // Not really needed, done at Reset
        }

        if (format_version >= X2M_MIN_VERSION_BODY_SHAPES)
        {
            if (!Utils::ReadParamUnsigned(root, "BODY_SHAPE", (uint32_t *)&body_shape))
                body_shape = -1;
        }

        if (format_version >= X2M_MIN_VERSION_ANY_DUAL_SKILL && Utils::ReadParamBoolean(root, "CAN_USE_ANY_DUAL_SKILL", &can_use_any_dual_skill))
        {
            // Nothing, already read
        }
        else
        {
            can_use_any_dual_skill = false;
        }

        if (format_version >= X2M_MIN_VERSION_INVISIBLE && Utils::ReadParamBoolean(root, "INVISIBLE", &invisible))
        {
            // Nothing, already read
        }
        else
        {
            invisible = false;
        }

        if (format_version >= X2M_MIN_VERSION_VFX_CHAR)
        {
            if (!Utils::ReadParamString(root, "CHAR_EEPK", char_eepk))
                char_eepk.clear();
        }
    }    
    else if (type == X2mType::NEW_SKILL)
    {
        if (!Utils::GetParamString(root, "SKILL_TYPE", temp_str))
            return false;

        if (temp_str == "SUPER")
            skill_type = X2mSkillType::SUPER;
        else if (temp_str == "ULTIMATE")
            skill_type = X2mSkillType::ULTIMATE;
        else if (temp_str == "EVASIVE")
            skill_type= X2mSkillType::EVASIVE;
        else if (temp_str == "BLAST")
            skill_type = X2mSkillType::BLAST;
        else if (temp_str == "AWAKEN")
            skill_type = X2mSkillType::AWAKEN;
        else
        {
            DPRINTF("%s: Unrecognized skill type \"%s\"\n", FUNCNAME, temp_str.c_str());
            return false;
        }

        if (skill_type == X2mSkillType::BLAST)
            if (!Utils::ReadParamBoolean(root, "BLAST_SS_INTENDED", &blast_ss_intended))
                blast_ss_intended = false;

        if (skill_type != X2mSkillType::BLAST || blast_ss_intended)
        {
            for (int i = 0; i < XV2_LANG_NUM; i++)
            {
                temp_str = "SKILL_NAME_" + Utils::ToUpperCase(xv2_lang_codes[i]);
                Utils::ReadParamString(root, temp_str.c_str(), skill_name[i]);

                if (skill_type != X2mSkillType::BLAST)
                {
                    temp_str = "SKILL_DESC_" + Utils::ToUpperCase(xv2_lang_codes[i]);
                    Utils::ReadParamString(root, temp_str.c_str(), skill_desc[i]);

                    if (format_version >= X2M_MIN_VERSION_SKILL_HOW)
                    {
                        temp_str = "SKILL_HOW_" + Utils::ToUpperCase(xv2_lang_codes[i]);
                        Utils::ReadParamString(root, temp_str.c_str(), skill_how[i]);
                    }
                }
            }
        }
    }
    else if (type == X2mType::NEW_STAGE)
    {
        for (int i = 0; i < XV2_LANG_NUM; i++)
        {
            temp_str = "STAGE_NAME_" + Utils::ToUpperCase(xv2_lang_codes[i]);
            Utils::ReadParamString(root, temp_str.c_str(), stage_name[i]);
        }

        if (!Utils::ReadParamBoolean(root, "ADD_SLOT", &add_stage_slot))
            add_stage_slot = true;

        add_stage_slot_local = false;

        if (add_stage_slot)
            Utils::ReadParamBoolean(root, "ADD_SLOT_LOCAL", &add_stage_slot_local);

        if (format_version >= X2M_MIN_VERSION_VFX_STAGE)
        {
            if (!Utils::ReadParamString(root, "STAGE_BG_EEPK", bg_eepk))
                bg_eepk.clear();

            if (!Utils::ReadParamString(root, "STAGE_EEPK", stage_eepk))
                stage_eepk.clear();
        }
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        const std::string &param_name = elem->ValueStr();

        if (type == X2mType::NEW_CHARACTER)
        {
            if (param_name == "SlotEntry")
            {
                X2mSlotEntry entry;

                if (!entry.Compile(elem, format_version >= X2M_MIN_VERSION_CSS_AUDIO))
                    return false;

                slot_entries.push_back(entry);
            }
            else if (param_name == "X2mCharaName" && format_version >= X2M_MIN_VERSION_MULTIPLE_CHAR_NAMES)
            {
                X2mCharaName name;

                if (!name.Compile(elem))
                    return false;

                mult_chara_names.push_back(name);
            }
            else if (param_name == "Entry")
            {
                if (!cms_entry.Compile(elem))
                {
                    cms_entry.id = X2M_INVALID_ID;
                    return false;
                }
            }
            else if (param_name == "SkillSet")
            {
                CusSkillSet entry;

                if (!entry.Compile(elem))
                    return false;

                skill_sets.push_back(entry);
            }
            else if (param_name == "CsoEntry" && format_version >= X2M_MIN_VERSION_CSO)
            {
                CsoEntry entry;

                if (!entry.Compile(elem))
                    return false;

                cso_entries.push_back(entry);
            }
            else if (param_name == "PscSpecEntry" && format_version >= X2M_MIN_VERSION_PSC)
            {
                PscSpecEntry entry;

                if (!entry.Compile(elem))
                    return false;

                psc_entries.push_back(entry);
            }
            else if (param_name == "CharacLink" && format_version >= X2M_MIN_VERSION_AUR)
            {
                AurCharaLink entry;

                if (!entry.Compile(elem))
                    return false;

                aur_entries.push_back(entry);
            }
            else if (param_name == "SevEntryHL" && format_version >= X2M_MIN_VERSION_SEV)
            {
                SevEntryHL entry;

                if (!entry.Compile(elem))
                    return false;

                sev_hl_entries.push_back(entry);
            }
            else if (param_name == "SevEntryLL" && format_version >= X2M_MIN_VERSION_SEV)
            {
                SevEntryLL entry;

                if (!entry.Compile(elem))
                    return false;

                sev_ll_entries.push_back(entry);
            }
            else if (param_name == "CmlEntry" && format_version >= X2M_MIN_VERSION_CML)
            {
                CmlEntry entry;

                if (!entry.Compile(elem))
                    return false;

                cml_entries.push_back(entry);
            }
            else if (param_name == "HciEntry" && format_version >= X2M_MIN_VERSION_HCI)
            {
                HciEntry entry;

                if (!entry.Compile(elem))
                    return false;

                entry.char_id = X2M_DUMMY_ID16;

                if (std::find(hci_entries.begin(), hci_entries.end(), entry) == hci_entries.end())
                {
                    hci_entries.push_back(entry);
                }
            }
            else if (param_name == "X2mDepends" && format_version >= X2M_MIN_VERSION_SKILLS)
            {
                X2mDepends entry;

                if (!entry.Compile(elem))
                    return false;

                if (entry.type == X2mDependsType::SKILL)
                {
                    chara_skill_depends.push_back(entry);
                }
                else if (entry.type == X2mDependsType::SUPERSOUL)
                {
                    chara_ss_depends.push_back(entry);
                }
            }
            else if (param_name == "Aura" && format_version >= X2M_MIN_VERSION_AURAS)
            {
                AurAura aura;

                if (!aura.Compile(elem, aura_types))
                    return false;

                chara_auras.push_back(aura);

                if (format_version < X2M_MIN_VERSION_AURA_EXTRA)
                {
                    chara_auras_extra.push_back(AuraExtraData());
                }
            }
            else if (param_name == "AuraExtraData" && format_version >= X2M_MIN_VERSION_AURA_EXTRA)
            {
                AuraExtraData aed;

                if (!aed.Compile(elem))
                    return false;

                chara_auras_extra.push_back(aed);
            }
            else if (param_name == "X2mCustomAudio" && format_version >= X2M_MIN_VERSION_CUSTOM_AUDIO)
            {
                X2mCustomAudio audio;

                if (!audio.Compile(elem))
                    return false;

                custom_audios.push_back(audio);
            }
            else if (param_name == "TtbEntryHL" && format_version >= X2M_MIN_VERSION_TTB_TTC)
            {
                TtbEntryHL entry_hl;

                if (!entry_hl.Compile(elem))
                    return false;

                ttb_hl_entries.push_back(entry_hl);
            }
            else if (param_name == "TtbEntryLL" && format_version >= X2M_MIN_VERSION_TTB_TTC)
            {
                TtbEntryLL entry_ll;

                if (!entry_ll.Compile(elem))
                    return false;

                if (entry_ll.num_actors < 5)
                {
                    entry_ll.cms5_code.clear();
                    entry_ll.event_res5.Clear();
                    entry_ll.costume5 = entry_ll.transformation5 = 0;
                }

                if (entry_ll.num_actors < 4)
                {
                    entry_ll.cms4_code.clear();
                    entry_ll.event_res4.Clear();
                    entry_ll.costume4 = entry_ll.transformation4 = 0;
                }

                if (entry_ll.num_actors < 3)
                {
                    entry_ll.cms3_code.clear();
                    entry_ll.event_res3.Clear();
                    entry_ll.costume3 = entry_ll.transformation3 = 0;
                }

                ttb_ll_entries.push_back(entry_ll);
            }
            else if (param_name == "TtcEntryHL" && format_version >= X2M_MIN_VERSION_TTB_TTC)
            {
                TtcEntryHL entry_hl;

                if (!entry_hl.Compile(elem))
                    return false;

                ttc_hl_entries.push_back(entry_hl);
            }
            else if (param_name == "TtcEntryLL" && format_version >= X2M_MIN_VERSION_TTB_TTC)
            {
                TtcEntryLL entry_ll;

                if (!entry_ll.Compile(elem))
                    return false;

                ttc_ll_entries.push_back(entry_ll);
            }
            else if (param_name == "CncEntry" && format_version >= X2M_MIN_VERSION_CNC)
            {
                CncEntry entry;

                if (!entry.Compile(elem))
                    return false;

                cnc_entries.push_back(entry);
            }
            else if (param_name == "IkdEntry" && format_version >= X2M_MIN_VERSION_IKD)
            {
                IkdEntry entry;

                if (!entry.Compile(elem))
                    return false;

                ikd_entries.push_back(entry);
            }
            else if (param_name == "VlcEntry" && format_version >= X2M_MIN_VERSION_VLC)
            {
                if (!vlc_entry.Compile(elem))
                    return false;

                vlc_entry.cms_id = X2M_DUMMY_ID;
            }
            else if (param_name == "DestructionLevel" && format_version >= X2M_MIN_VERSION_DESTRUCTION)
            {
                DestructionLevel dl;

                if (!dl.Compile(elem))
                    return false;

                des_levels.push_back(dl);
            }

        } // End NEW_CHARACTER
        else if (type == X2mType::NEW_SKILL)
        {
            if (param_name == "Skill")
            {
                if (!skill_entry.Compile(elem))
                {
                    skill_entry.id = skill_entry.id2 = X2M_INVALID_ID16;
                    return false;
                }

                skill_entry.id = X2M_DUMMY_ID16;
            }
            else if (skill_type != X2mSkillType::BLAST && param_name == "IdbEntry")
            {
                if (!skill_idb_entry.Compile(elem))
                {
                    skill_idb_entry.id = X2M_INVALID_ID16;
                    return false;
                }
            }
            else if (param_name == "PupEntry")
            {
                PupEntry entry;

                if (!entry.Compile(elem))
                    return false;

                skill_pup_entries.push_back(entry);
            }
            else if (skill_type == X2mSkillType::AWAKEN && param_name == "X2mSkillTransName")
            {
                X2mSkillTransName tn;

                if (!tn.Compile(elem))
                    return false;

                skill_trans_names.push_back(tn);
            }
            else if (param_name == "X2mSkillAura" && format_version >= X2M_MIN_VERSION_AURAS)
            {
                X2mSkillAura aura;

                if (!aura.Compile(elem))
                    return false;

                aura.data.cus_aura_id = X2M_DUMMY_ID16;
                aura.aura.id = X2M_DUMMY_ID;

                skill_aura_entries.push_back(aura);
            }
            else if (param_name == "X2mDepends" && format_version >= X2M_MIN_VERSION_COSTUME)
            {
                X2mDepends dep;

                if (!dep.Compile(elem))
                    return false;

                if (dep.type == X2mDependsType::COSTUME)
                    skill_costume_depend = dep;
                else if (dep.type == X2mDependsType::CHARACTER && format_version >= X2M_MIN_VERSION_SKILL_CHARA_DEPEND)
                    skill_chara_depend = dep;
                else
                {
                    return false;
                }
            }
            else if (param_name == "X2mBody" && format_version >= X2M_MIN_VERSION_SKILL_BCS_BODY)
            {
                X2mBody body;

                if (!body.Compile(elem))
                    return false;

                skill_bodies.push_back(body);
            }

        } // END NEW_SKILL
        else if (type == X2mType::NEW_COSTUME)
        {
            if (param_name == "X2mItem")
            {
                X2mItem item;

                if (!item.Compile(elem))
                    return false;

                costume_items.push_back(item);
            }
            else if (param_name == "X2mPartSet")
            {
                X2mPartSet set;

                if (!set.Compile(elem))
                    return false;

                costume_partsets.push_back(set);
            }

        } // END NEW_COSTUME
        else if (type == X2mType::NEW_STAGE)
        {
            if (param_name == "Stage")
            {
                if (!stage_def.Compile(elem))
                    return false;

                stage_def.ssid = -1;
            }

        } // END NEW_STAGE
        else if (type == X2mType::NEW_SUPERSOUL)
        {
            if (param_name == "X2mItem")
            {
                if (!ss_item.Compile(elem))
                    return false;
            }
            else if (param_name == "X2mDepends")
            {
                if (!ss_blast_depend.Compile(elem) || ss_blast_depend.type != X2mDependsType::SKILL)
                {
                    ss_blast_depend.id = X2M_INVALID_ID;
                    return false;
                }
            }
        } // END NEW SUPERSOUL
    }

    return Validate(false);
}

bool X2mFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!ZipFile::Load(buf, size))
        return false;

    return Compile();
}

uint8_t *X2mFile::Save(size_t *psize)
{
    if (!Decompile())
        return nullptr;

    return ZipFile::Save(psize);
}

bool X2mFile::CharaDirectoryExists() const
{
    if (entry_name.length() == 0)
        return false;

    return DirExists(entry_name);
}

void X2mFile::DeleteCharaDirectory()
{
    if (entry_name.length() == 0)
        return;

    DeleteDir(entry_name);
}

X2mSlotEntry *X2mFile::FindSlotEntry(uint32_t costume_index, uint32_t model_preset)
{
    for (X2mSlotEntry &entry : slot_entries)
        if (entry.costume_index == (int32_t)costume_index && entry.model_preset == (int32_t) model_preset)
            return &entry;

    return nullptr;
}

size_t X2mFile::GetNumCostumes() const
{
    std::unordered_set<int32_t> costumes;

    for (const X2mSlotEntry &slot : slot_entries)
    {
        costumes.insert(slot.costume_index);
    }

    return costumes.size();
}

bool X2mFile::CostumeExists(int32_t costume_id) const
{
    std::unordered_set<int32_t> costumes;

    for (const X2mSlotEntry &slot : slot_entries)
    {
        costumes.insert(slot.costume_index);
    }

    return (costumes.find(costume_id) != costumes.end());
}

X2mFile *X2mFile::CreateDummyPackage()
{
    X2mFile *dummy = new X2mFile();

    dummy->type = type;
    dummy->format_version = format_version;

    dummy->mod_name = mod_name;
    dummy->mod_author = mod_author;
    dummy->mod_version = mod_version;
    memcpy(dummy->mod_guid, mod_guid, sizeof(mod_guid));
    memcpy(dummy->udata, udata, sizeof(udata));

    dummy->entry_name = entry_name;
    dummy->chara_name = chara_name;
    dummy->mult_chara_names = mult_chara_names;
    dummy->is_ozaru = is_ozaru;
    dummy->is_cellmax = is_cellmax;
    dummy->body_shape = body_shape;
    dummy->can_use_any_dual_skill = can_use_any_dual_skill;

    dummy->slot_entries = slot_entries;
    dummy->cms_entry = cms_entry;
    dummy->skill_sets = skill_sets;
    dummy->cso_entries = cso_entries;
    dummy->psc_entries = psc_entries;
    dummy->aur_entries = aur_entries;
    dummy->sev_hl_entries = sev_hl_entries;
    dummy->sev_ll_entries = sev_ll_entries;
    dummy->cml_entries = cml_entries;
    dummy->hci_entries = hci_entries;

    dummy->chara_skill_depends = chara_skill_depends;
    dummy->chara_auras = chara_auras;
    dummy->chara_auras_extra = chara_auras_extra;

    dummy->custom_audios = custom_audios;

    dummy->ttb_hl_entries = ttb_hl_entries;
    dummy->ttb_ll_entries = ttb_ll_entries;
    dummy->ttc_hl_entries = ttc_hl_entries;
    dummy->ttc_ll_entries = ttc_ll_entries;

    dummy->cnc_entries = cnc_entries;
    dummy->char_eepk = char_eepk;

    dummy->ikd_entries = ikd_entries;
    dummy->vlc_entry = vlc_entry;
    dummy->des_levels = des_levels;

    dummy->chara_ss_depends = chara_ss_depends;

    dummy->invisible = invisible;

    dummy->skill_name = skill_name;
    dummy->skill_desc = skill_desc;
    dummy->skill_how = skill_how;
    dummy->skill_type = skill_type;
    dummy->skill_trans_names = skill_trans_names;
    dummy->skill_entry = skill_entry;
    dummy->skill_idb_entry = skill_idb_entry;
    dummy->skill_pup_entries = skill_pup_entries;
    dummy->skill_aura_entries = skill_aura_entries;
    dummy->skill_costume_depend = skill_costume_depend;
    dummy->skill_bodies = skill_bodies;
    dummy->blast_ss_intended = blast_ss_intended;
    dummy->skill_chara_depend = skill_chara_depend;

    dummy->costume_items = costume_items;
    dummy->costume_partsets = costume_partsets;

    dummy->stage_name = stage_name;
    dummy->stage_def = stage_def;
    dummy->add_stage_slot = add_stage_slot;
    dummy->add_stage_slot_local = add_stage_slot_local;
    dummy->bg_eepk = bg_eepk;
    dummy->stage_eepk = stage_eepk;

    dummy->ss_item = ss_item;
    dummy->ss_blast_depend = ss_blast_depend;

    size_t num_entries = GetNumEntries();
    const std::string dummy_content = "DUMMY";

    for (size_t i = 0; i < num_entries; i++)
    {
        zip_stat_t zstat;

        if (zip_stat_index(archive, i, 0, &zstat) == -1)
            continue;

        if (!(zstat.valid & ZIP_STAT_SIZE))
            continue;

        if (!(zstat.valid & ZIP_STAT_NAME))
            continue;

        std::string entry_path = Utils::NormalizePath(zstat.name);

        if (entry_path.back() == '/') // Stupid explicit directory entry added by winrar and others
            continue;

        if (type == X2mType::NEW_QUEST && (Utils::BeginsWith(entry_path, "QUEST/", false) || Utils::BeginsWith(entry_path, "AUDIO/", false)))
        {
            size_t size;
            uint8_t *buf = ReadFile(entry_path, &size);

            if (!buf)
            {
                DPRINTF("Couldn't read file inside x2m (weird error)\n");
                delete dummy;
                return nullptr;
            }

            bool ret = dummy->WriteFile(entry_path, buf, size);
            delete[] buf;

            if (!ret)
            {
                delete dummy;
                return nullptr;
            }
        }
        else
        {
            if (!dummy->WriteFile(entry_path, dummy_content.c_str(), dummy_content.length()))
            {
                delete dummy;
                return nullptr;
            }
        }
    }

    return dummy;
}

bool X2mFile::HasDangerousJungle() const
{
    if (!JungleExists())
        return false;

    static const std::vector<std::string> dangerous_paths =
    {
        std::string(X2M_JUNGLE) + "data/system/",
        std::string(X2M_JUNGLE) + "XV2PATCHER/",
        std::string(X2M_JUNGLE) + "bin/"
    };

    static const std::vector<std::string> dangerous_files =
    {
        std::string(X2M_JUNGLE) + "data/ui/iggy/CHARASELE.iggy",
        std::string(X2M_JUNGLE) + "data/ui/texture/CHARA01.emb",
        std::string(X2M_JUNGLE) + "data/ui/CharaImage/chara_image.hci",
        std::string(X2M_JUNGLE) + "data/sound/VOX/Sys/CRT_CS_vox.acb",
        std::string(X2M_JUNGLE) + "data/sound/VOX/Sys/CRT_CS_vox.awb",
        std::string(X2M_JUNGLE) + "data/sound/VOX/Sys/en/CRT_CS_vox.acb",
        std::string(X2M_JUNGLE) + "data/sound/VOX/Sys/en/CRT_CS_vox.awb",
        std::string(X2M_JUNGLE) + "data/chara/HUM/HUM.bcs",
        std::string(X2M_JUNGLE) + "data/chara/HUF/HUF.bcs",
        std::string(X2M_JUNGLE) + "data/chara/NMC/NMC.bcs",
        std::string(X2M_JUNGLE) + "data/chara/FRI/FRI.bcs",
        std::string(X2M_JUNGLE) + "data/chara/MAM/MAM.bcs",
        std::string(X2M_JUNGLE) + "data/chara/MAF/MAF.bcs",
        std::string(X2M_JUNGLE) + "data/XV2P_SLOTS.x2s",
        std::string(X2M_JUNGLE) + "data/XV2P_SLOTS_STAGE.x2s",
        std::string(X2M_JUNGLE) + "data/XV2P_SLOTS_STAGE_LOCAL.x2s",
        std::string(X2M_JUNGLE) + "data/pre-baked.xml",
        std::string(X2M_JUNGLE) + "data/X2M_COSTUME.xml",
        std::string(X2M_JUNGLE) + "data/xv2_stage_def.xml",
        std::string(X2M_JUNGLE) + "data/vfx/vfx_spec.ers"
    };

    static const std::vector<std::string> dangerous_msg =
    {
        "qs_title",
        "qs_ep00",
        "qs_ep01",
        "qs_ep02",
        "qs_ep03",
        "qs_ep04",
        "qs_ep05",
        "qs_ep06",
        "qs_ep07",
        "qs_ep08",
        "qs_ep09",
        "qs_ep10",
        "qs_ep11",
        "qs_ep12",
        "qs_cep00",
        "qs_cep01",
        "qs_cep02",
        "qs_cep03",
        "qs_cep04",
        "qs_cep05",
        "qs_cep06",
        "qs_cep07",
        "qs_cep08",
        "qs_cep09",
        "qs_cep10",
        "qs_cep11",
        "qs_cep12",
        "qs_cep13",
        "qs_cep14",
        "qs_cep15",
        "qs_cep16",
        "qs_cep17",
        "qs_cep99",
        "qe_title",
        "qe_ep01",
        "qe_ep02",
        "qe_ep03",
        "qe_ep04",
        "qe_ep05",
        "qe_ep06",
        "qe_ep07",
        "qe_ep08",
        "qe_ep09",
        "qe_ep10",
        "qe_ep11",
        "qe_ep12",
        "qe_ep13",
        "qe_cep00",
        "qe_cep01",
        "qe_cep02",
        "qe_cep03",
        "qe_cep04",
        "qe_cep05",
        "qe_cep06",
        "qe_cep07",
        "qe_cep08",
        "qe_cep09",
        "qe_cep10",
        "qe_cep11",
        "qe_cep12",
        "qe_cep13",
        "qe_cep14",
        "qe_cep15",
        "qe_cep16",
        "qe_cep17",
        "qe_cep18",
        "qe_cep99",
        "qh_title",
        "qh_battle",
    };

    for (const std::string &path : dangerous_paths)
    {
        if (DirExists(path))
            return true;
    }

    for (const std::string &file : dangerous_files)
    {
        if (FileExists(file))
            return true;
    }

    for (const std::string &msg_file : dangerous_msg)
    {
        for (size_t i = 0; i <= xv2_lang_codes.size(); i++)
        {
            std::string path = std::string(X2M_JUNGLE) + "data/msg/" + msg_file;
            path.push_back('_');

            if (i == xv2_lang_codes.size())
            {
                path += "voice";
            }
            else
            {
                path += xv2_lang_codes[i];
            }

            path += ".msg";

            if (FileExists(path))
                return true;
        }
    }

    return false;
}

bool X2mFile::AudioFileExists(const std::string &name, bool english) const
{
    if (format_version < X2M_MIN_VERSION_CUSTOM_AUDIO)
    {
        if (english)
            return FileExists(std::string(X2M_CSS_AUDIO) + name + "_en.hca");

        return FileExists(std::string(X2M_CSS_AUDIO) + name + "_jp.hca");
    }

    if (english)
        return FileExists(std::string(X2M_AUDIO) + name + "_en.hca");

    return FileExists(std::string(X2M_AUDIO) + name + "_jp.hca");
}

bool X2mFile::AudioFileExists(const std::string &name) const
{
    return (AudioFileExists(name, false) || AudioFileExists(name, true));
}

HcaFile *X2mFile::LoadAudioFile(const std::string &name, bool english, bool fallback)
{
    if (IsDummyMode())
    {
        DPRINTF("%s: I shouldn't be called in dummy mode!\n", FUNCNAME);
        return nullptr;
    }

    std::string jp_path;
    std::string en_path;

    if (format_version < X2M_MIN_VERSION_CUSTOM_AUDIO)
    {
        jp_path = std::string(X2M_CSS_AUDIO) + name + "_jp.hca";
        en_path = std::string(X2M_CSS_AUDIO) + name + "_en.hca";
    }
    else
    {
        jp_path = std::string(X2M_AUDIO) + name + "_jp.hca";
        en_path = std::string(X2M_AUDIO) + name + "_en.hca";
    }

    size_t size;
    uint8_t *buf = ReadFile((english) ? en_path : jp_path, &size);

    if (!buf)
    {
        if (!fallback)
            return nullptr;

        // Load from the other language
        buf = ReadFile((english) ? jp_path : en_path, &size);
        if (!buf)
            return nullptr;
    }

    HcaFile *hca = new HcaFile();
    if (!hca->Load(buf, size))
    {
        delete hca;
        return nullptr;
    }

    hca->SetCiphType(0);
    return hca;
}

size_t X2mFile::GetAudioFiles(std::vector<std::string> &files)
{
    files.clear();

    size_t num_entries = GetNumEntries();

    for (size_t i = 0; i < num_entries; i++)
    {
        zip_stat_t zstat;

        if (zip_stat_index(archive, i, 0, &zstat) == -1)
            continue;

        if (!(zstat.valid & ZIP_STAT_NAME))
            continue;

        std::string entry_path = Utils::NormalizePath(zstat.name);
        const std::string check = (format_version < X2M_MIN_VERSION_CUSTOM_AUDIO) ? X2M_CSS_AUDIO : X2M_AUDIO;

        if (Utils::BeginsWith(entry_path, check, false))
        {
            if (Utils::EndsWith(entry_path, "_jp.hca", false) || Utils::EndsWith(entry_path, "_en.hca", false))
            {
                std::string base = Utils::GetFileNameString(entry_path);
                if (base.length() == strlen("_jp.hca"))
                    continue;

                base = Utils::ToUpperCase(base.substr(0, base.length()-strlen("_jp.hca")));

                if (std::find(files.begin(), files.end(), base) == files.end())
                    files.push_back(base);
            }
        }
    }

    return files.size();
}

static bool audio_visitor(const std::string &path, bool, void *param)
{
    X2mFile *pthis = (X2mFile *)param;

    if (Utils::EndsWith(path, "_jp.hca", false) || Utils::EndsWith(path, "_en.hca", false))
    {
        size_t size;
        uint8_t *buf = Utils::ReadFile(path, &size);
        if (buf)
        {
            std::string base = Utils::GetFileNameString(path);
            if (base.length() <= strlen("_jp.hca"))
            {
                delete[] buf;
                return true;
            }

            if (pthis->GetFormatVersion() < pthis->X2M_MIN_VERSION_CUSTOM_AUDIO)
                pthis->WriteFile(X2M_CSS_AUDIO + base, buf, size);
            else
                pthis->WriteFile(X2M_AUDIO + base, buf, size);

            delete[] buf;
        }
    }

    return true;
}

size_t X2mFile::SetAudioDir(const std::string &dir_path)
{
    Utils::VisitDirectory(dir_path, true, false, false, audio_visitor, this);

    std::vector<std::string> files;
    return GetAudioFiles(files);
}

void X2mFile::DeleteAudio()
{
    if (format_version < X2M_MIN_VERSION_CUSTOM_AUDIO)
        DeleteDir(X2M_CSS_AUDIO);
    else
        DeleteDir(X2M_AUDIO);
}

bool X2mFile::CharaSkillDependsExist(const uint8_t *guid) const
{
    for (const X2mDepends &dep : chara_skill_depends)
    {
        if (memcmp(dep.guid, guid, 16) == 0)
            return true;
    }

    return false;
}

bool X2mFile::CharaSkillDependsExist(const std::string &guid) const
{
    uint8_t guid_bin[16];

    if (!Utils::String2GUID(guid_bin, guid))
        return false;

    return CharaSkillDependsExist(guid_bin);
}

bool X2mFile::CharaSkillDependsExist(uint16_t id) const
{
    for (const X2mDepends &dep : chara_skill_depends)
    {
        if (dep.id == id)
            return true;
    }

    return false;
}

X2mDepends *X2mFile::FindCharaSkillDepends(const uint8_t *guid)
{
    for (X2mDepends &dep : chara_skill_depends)
    {
        if (memcmp(dep.guid, guid, 16) == 0)
            return &dep;
    }

    return nullptr;
}

X2mDepends *X2mFile::FindCharaSkillDepends(const std::string &guid)
{
    uint8_t guid_bin[16];

    if (!Utils::String2GUID(guid_bin, guid))
        return nullptr;

    return FindCharaSkillDepends(guid_bin);
}

X2mDepends *X2mFile::FindCharaSkillDepends(uint16_t id)
{
    for (X2mDepends &dep : chara_skill_depends)
    {
        if (dep.id == id)
            return &dep;
    }

    return nullptr;
}

bool X2mFile::AddCharaSkillDepends(const X2mDepends &depends)
{
    if (CharaSkillDependsExist(depends.guid) || CharaSkillDependsExist(depends.id))
        return false;

    chara_skill_depends.push_back(depends);
    return true;
}

bool X2mFile::AddCharaSkillDepends(X2mFile *skill_x2m, bool update)
{
    if (skill_x2m->GetType() != X2mType::NEW_SKILL)
        return false;

    X2mDepends *existing = FindCharaSkillDepends(skill_x2m->mod_guid);
    if (existing)
    {
        if (!update)
            return false;

        existing->name = skill_x2m->GetModName();
        return true;
    }

    X2mDepends new_dep;

    new_dep.type = X2mDependsType::SKILL;
    memcpy(new_dep.guid, skill_x2m->mod_guid, 16);
    new_dep.name = skill_x2m->GetModName();

    for (uint16_t id = X2M_SKILL_DEPENDS_BEGIN; id < X2M_SKILL_DEPENDS_END; id++)
    {
        if (!CharaSkillDependsExist(id))
        {
            new_dep.id = id;
            chara_skill_depends.push_back(new_dep);
            return true;
        }
    }

    return false;
}

X2mDepends *X2mFile::AddCharaSkillDepends(const uint8_t *guid, const std::string &name)
{
    if (CharaSkillDependsExist(guid))
        return nullptr;

    X2mDepends new_dep;

    new_dep.type = X2mDependsType::SKILL;
    memcpy(new_dep.guid, guid, 16);
    new_dep.name = name;

    for (uint16_t id = X2M_SKILL_DEPENDS_BEGIN; id < X2M_SKILL_DEPENDS_END; id++)
    {
        if (!CharaSkillDependsExist(id))
        {
            new_dep.id = id;
            chara_skill_depends.push_back(new_dep);
            return &chara_skill_depends.back();
        }
    }

    return nullptr;
}

bool X2mFile::RemoveCharaSkillDepends(const uint8_t *guid)
{
    for (size_t i = 0; i < chara_skill_depends.size(); i++)
    {
        if (memcmp(chara_skill_depends[i].guid, guid, 16) == 0)
        {
            chara_skill_depends.erase(chara_skill_depends.begin()+i);
            return true;
        }
    }

    return true; // Yes true
}

bool X2mFile::RemoveCharaSkillDepends(const std::string &guid)
{
    uint8_t guid_bin[16];

    if (!Utils::String2GUID(guid_bin, guid))
        return false;

    return RemoveCharaSkillDepends(guid_bin);
}

bool X2mFile::CharaSkillDependsHasAttachment(size_t idx) const
{
    if (idx >= chara_skill_depends.size())
        return false;

    if (IsDummyMode())
        return false;

    const std::string att_path = X2M_CHARA_SKILLS_ATTACHMENTS + Utils::GUID2String(chara_skill_depends[idx].guid) + ".x2m";
    return FileExists(att_path);
}

bool X2mFile::CharaSkillDependsHasAttachment(const uint8_t *guid) const
{
    for (size_t i = 0; i < chara_skill_depends.size(); i++)
    {
        if (memcmp(chara_skill_depends[i].guid, guid, 16) == 0)
        {
            return CharaSkillDependsHasAttachment(i);
        }
    }

    return false;
}

bool X2mFile::CharaSkillDependsHasAttachment(const std::string &guid) const
{
    uint8_t guid_bin[16];

    if (!Utils::String2GUID(guid_bin, guid))
        return false;

    return CharaSkillDependsHasAttachment(guid_bin);
}

bool X2mFile::SetCharaSkillDependsAttachment(size_t idx, X2mFile *attachment)
{
    if (idx >= chara_skill_depends.size())
        return false;

    if (attachment->GetType() != X2mType::NEW_SKILL)
        return false;

    X2mDepends &dep = chara_skill_depends[idx];

    if (memcmp(dep.guid, attachment->mod_guid, 16) != 0)
        return false;

    size_t size;
    uint8_t *buf = attachment->Save(&size);

    if (!buf)
        return false;

    const std::string att_path = X2M_CHARA_SKILLS_ATTACHMENTS + Utils::GUID2String(dep.guid) + ".x2m";
    bool ret = WriteFile(att_path, buf, size);
    delete[] buf;

    return ret;
}

bool X2mFile::SetCharaSkillDependsAttachment(X2mFile *attachment)
{
    for (size_t i = 0; i < chara_skill_depends.size(); i++)
    {
        if (memcmp(chara_skill_depends[i].guid, attachment->mod_guid, 16) == 0)
        {
            return SetCharaSkillDependsAttachment(i, attachment);
        }
    }

    return false;
}

bool X2mFile::RemoveCharaSkillDependsAttachment(const uint8_t *guid)
{
    for (const X2mDepends &dep : chara_skill_depends)
    {
        if (memcmp(dep.guid, guid, 16) == 0)
        {
            const std::string att_path = X2M_CHARA_SKILLS_ATTACHMENTS + Utils::GUID2String(dep.guid) + ".x2m";
            return RemoveFile(att_path);
        }
    }

    return false;
}

bool X2mFile::RemoveCharaSkillDependsAttachment(const std::string &guid)
{
    uint8_t guid_bin[16];

    if (!Utils::String2GUID(guid_bin, guid))
        return false;

    return RemoveCharaSkillDependsAttachment(guid_bin);
}

X2mFile *X2mFile::LoadCharaSkillDependsAttachment(const uint8_t *guid)
{
    if (!CharaSkillDependsHasAttachment(guid))
        return nullptr;

    const std::string att_path = X2M_CHARA_SKILLS_ATTACHMENTS + Utils::GUID2String(guid) + ".x2m";
    size_t size;
    uint8_t *buf = ReadFile(att_path, &size);
    if (!buf)
        return nullptr;

    X2mFile *ret = new X2mFile();
    if (!ret->Load(buf, size))
    {
        delete[] buf;
        delete ret;
        return nullptr;
    }

    if (ret->type != X2mType::NEW_SKILL)
    {
        DPRINTF("%s: Some retard has specified as skill attachment a mod that is not a skill.\n", FUNCNAME);
        return nullptr;
    }

    return ret;
}

X2mFile *X2mFile::LoadCharaSkillDependsAttachment(const std::string &guid)
{
    uint8_t guid_bin[16];

    if (!Utils::String2GUID(guid_bin, guid))
        return nullptr;

    return LoadCharaSkillDependsAttachment(guid_bin);
}

bool X2mFile::IsCharaSkillDependsReferenced(const X2mDepends &depends) const
{
    for (const CusSkillSet &skill_set : skill_sets)
    {
        for (int i = 0; i < 9; i++)
        {
            if (skill_set.char_skills[i] == depends.id)
                return true;
        }
    }

    return false;
}

size_t X2mFile::GetNumCharaAuras(bool install_mode) const
{
    if (!install_mode)
        return chara_auras.size();

    if (format_version < X2M_MIN_VERSION_AURAS || chara_auras.size() != aur_entries.size())
        return 0;

    size_t num = 0;

    for (size_t i = 0; i < aur_entries.size(); i++)
    {
        if (aur_entries[i].aura_id == X2M_INVALID_ID)
        {
            num++;
        }
    }

    return num;
}

X2mCustomAudio *X2mFile::FindCustomAudio(uint32_t id)
{
    for (X2mCustomAudio &audio : custom_audios)
    {
        if (audio.id == id)
            return &audio;
    }

    return nullptr;
}

X2mCustomAudio *X2mFile::FindCustomAudio(const std::string &name)
{
    for (X2mCustomAudio &audio : custom_audios)
    {
        if (audio.name == name)
            return &audio;
    }

    return nullptr;
}

uint32_t X2mFile::AddCustomAudio(const std::string &name, bool must_new)
{
    for (X2mCustomAudio &audio : custom_audios)
    {
        if (audio.name == name)
        {
            if (must_new)
                return (uint32_t)-1;

            return audio.id;
        }
    }

    X2mCustomAudio audio;

    for (audio.id = X2M_CUSTOM_AUDIO_BEGIN; audio.id < X2M_CUSTOM_AUDIO_END; audio.id++)
    {
        if (!FindCustomAudio(audio.id))
        {
            audio.name = name;
            custom_audios.push_back(audio);
            return audio.id;
        }
    }

    return (uint32_t)-1;
}

void X2mFile::RemoveCustomAudioByID(uint32_t id)
{
    for (size_t i = 0; i < custom_audios.size(); i++)
    {
        if (custom_audios[i].id == id)
        {
            custom_audios.erase(custom_audios.begin()+i);
            i--;
        }
    }
}

void X2mFile::RemoveCustomAudioByName(const std::string &name)
{
    for (size_t i = 0; i < custom_audios.size(); i++)
    {
        if (custom_audios[i].name == name)
        {
            custom_audios.erase(custom_audios.begin()+i);
            i--;
        }
    }
}

size_t X2mFile::GetCustomSevAudio(std::vector<X2mCustomAudio> &list)
{
    list.clear();

    if (!HasSevLL())
        return 0;

    for (const SevEntryLL &entry : sev_ll_entries)
    {
        for (const SevEventsLL &evll : entry.sub_entries)
        {
            for (const SevEvent &ev : evll.events)
            {
                for (const SevEventEntry &entry : ev.entries)
                {
                    if (entry.cue_id >= X2M_CUSTOM_AUDIO_BEGIN && entry.cue_id < X2M_CUSTOM_AUDIO_END)
                    {
                        X2mCustomAudio *audio = FindCustomAudio(entry.cue_id);

                        if (audio && std::find(list.begin(), list.end(), *audio) == list.end())
                        {
                            list.push_back(*audio);
                        }
                    }

                    if (entry.response_cue_id >= X2M_CUSTOM_AUDIO_BEGIN && entry.response_cue_id < X2M_CUSTOM_AUDIO_END)
                    {
                        X2mCustomAudio *audio = FindCustomAudio(entry.response_cue_id);

                        if (audio && std::find(list.begin(), list.end(), *audio) == list.end())
                        {
                            list.push_back(*audio);
                        }
                    }
                }
            }
        }
    }

    return list.size();
}

void X2mFile::RemoveCustomSevAudio(uint32_t id, bool full_remove)
{
    for (SevEntryLL &entry : sev_ll_entries)
    {
        for (SevEventsLL &evll : entry.sub_entries)
        {
            for (SevEvent &ev : evll.events)
            {
                for (SevEventEntry &entry : ev.entries)
                {
                    if (entry.cue_id == id)
                    {
                        if (full_remove)
                            RemoveCustomAudioByID(entry.cue_id);

                        entry.cue_id = 0xFFFFFFFF;
                        entry.file_id = 0;
                    }

                    if (entry.response_cue_id == id)
                    {
                        if (full_remove)
                            RemoveCustomAudioByID(entry.response_cue_id);

                        entry.response_cue_id = 0xFFFFFFFF;
                        entry.response_file_id = 0;
                    }
                }
            }
        }
    }
}

size_t X2mFile::RemoveAllCustomSevAudio(bool full_remove)
{
    size_t count = 0;

    for (SevEntryLL &entry : sev_ll_entries)
    {
        for (SevEventsLL &evll : entry.sub_entries)
        {
            for (SevEvent &ev : evll.events)
            {
                for (SevEventEntry &entry : ev.entries)
                {
                    if (entry.cue_id >= X2M_CUSTOM_AUDIO_BEGIN && entry.cue_id < X2M_CUSTOM_AUDIO_END)
                    {
                        if (full_remove)
                            RemoveCustomAudioByID(entry.cue_id);

                        entry.cue_id = 0xFFFFFFFF;
                        entry.file_id = 0;
                        count++;
                    }

                    if (entry.response_cue_id >= X2M_CUSTOM_AUDIO_BEGIN && entry.response_cue_id < X2M_CUSTOM_AUDIO_END)
                    {
                        if (full_remove)
                            RemoveCustomAudioByID(entry.response_cue_id);

                        entry.response_cue_id = 0xFFFFFFFF;
                        entry.response_file_id = 0;
                        count++;
                    }
                }
            }
        }
    }

    return count;
}

size_t X2mFile::GetCustomTtbAudio(std::vector<X2mCustomAudio> &list)
{
    list.clear();

    if (!HasTtbLL())
        return 0;

    for (const TtbEntryLL &entry : ttb_ll_entries)
    {
        if (Utils::BeginsWith(entry.event_res1.name, "X2T_"))
        {
            std::string audio_name = entry.event_res1.name.substr(4);
            X2mCustomAudio *audio = FindCustomAudio(audio_name);

            if (audio && std::find(list.begin(), list.end(), *audio) == list.end())
            {
                list.push_back(*audio);
            }
        }

        if (Utils::BeginsWith(entry.event_res2.name, "X2T_"))
        {
            std::string audio_name = entry.event_res2.name.substr(4);
            X2mCustomAudio *audio = FindCustomAudio(audio_name);

            if (audio && std::find(list.begin(), list.end(), *audio) == list.end())
            {
                list.push_back(*audio);
            }
        }

        if (entry.num_actors <= 2)
            continue;

        if (Utils::BeginsWith(entry.event_res3.name, "X2T_"))
        {
            std::string audio_name = entry.event_res3.name.substr(4);
            X2mCustomAudio *audio = FindCustomAudio(audio_name);

            if (audio && std::find(list.begin(), list.end(), *audio) == list.end())
            {
                list.push_back(*audio);
            }
        }

        if (entry.num_actors <= 3)
            continue;

        if (Utils::BeginsWith(entry.event_res4.name, "X2T_"))
        {
            std::string audio_name = entry.event_res4.name.substr(4);
            X2mCustomAudio *audio = FindCustomAudio(audio_name);

            if (audio && std::find(list.begin(), list.end(), *audio) == list.end())
            {
                list.push_back(*audio);
            }
        }

        if (entry.num_actors <= 4)
            continue;

        if (Utils::BeginsWith(entry.event_res5.name, "X2T_"))
        {
            std::string audio_name = entry.event_res5.name.substr(4);
            X2mCustomAudio *audio = FindCustomAudio(audio_name);

            if (audio && std::find(list.begin(), list.end(), *audio) == list.end())
            {
                list.push_back(*audio);
            }
        }
    }

    return list.size();
}

void X2mFile::RemoveCustomTtbAudio(const std::string &name, bool full_remove)
{
    if (name.length() == 0)
        return;

    const std::string voice_name = "X2T_" + name;

    for (TtbEntryLL &entry : ttb_ll_entries)
    {
        if (entry.event_res1.name == voice_name)
        {
            if (full_remove)
                RemoveCustomAudioByName(name);

            entry.event_res1.Clear();
        }

        if (entry.event_res2.name == voice_name)
        {
            if (full_remove)
                RemoveCustomAudioByName(name);

            entry.event_res2.Clear();
        }

        if (entry.num_actors <= 2)
            continue;

        if (entry.event_res3.name == voice_name)
        {
            if (full_remove)
                RemoveCustomAudioByName(name);

            entry.event_res3.Clear();
        }

        if (entry.num_actors <= 3)
            continue;

        if (entry.event_res4.name == voice_name)
        {
            if (full_remove)
                RemoveCustomAudioByName(name);

            entry.event_res4.Clear();
        }

        if (entry.num_actors <= 4)
            continue;

        if (entry.event_res5.name == voice_name)
        {
            if (full_remove)
                RemoveCustomAudioByName(name);

            entry.event_res5.Clear();
        }
    }
}

size_t X2mFile::RemoveAllCustomTtbAudio(bool full_remove)
{
    size_t count = 0;

    for (TtbEntryLL &entry : ttb_ll_entries)
    {
        if (Utils::BeginsWith(entry.event_res1.name, "X2T_"))
        {
            std::string audio_name = entry.event_res1.name.substr(4);

            if (full_remove)
                RemoveCustomAudioByName(audio_name);

            entry.event_res1.Clear();
            count++;
        }

        if (Utils::BeginsWith(entry.event_res2.name, "X2T_"))
        {
            std::string audio_name = entry.event_res2.name.substr(4);

            if (full_remove)
                RemoveCustomAudioByName(audio_name);

            entry.event_res2.Clear();
            count++;
        }

        if (entry.num_actors <= 2)
            continue;

        if (Utils::BeginsWith(entry.event_res3.name, "X2T_"))
        {
            std::string audio_name = entry.event_res3.name.substr(4);

            if (full_remove)
                RemoveCustomAudioByName(audio_name);

            entry.event_res3.Clear();
            count++;
        }

        if (entry.num_actors <= 3)
            continue;

        if (Utils::BeginsWith(entry.event_res4.name, "X2T_"))
        {
            std::string audio_name = entry.event_res4.name.substr(4);

            if (full_remove)
                RemoveCustomAudioByName(audio_name);

            entry.event_res4.Clear();
            count++;
        }

        if (entry.num_actors <= 4)
            continue;

        if (Utils::BeginsWith(entry.event_res5.name, "X2T_"))
        {
            std::string audio_name = entry.event_res5.name.substr(4);

            if (full_remove)
                RemoveCustomAudioByName(audio_name);

            entry.event_res5.Clear();
            count++;
        }
    }

    return count;
}

bool X2mFile::CharaSsDependsExist(const uint8_t *guid) const
{
    for (const X2mDepends &dep : chara_ss_depends)
    {
        if (memcmp(dep.guid, guid, 16) == 0)
            return true;
    }

    return false;
}

bool X2mFile::CharaSsDependsExist(const std::string &guid) const
{
    uint8_t guid_bin[16];

    if (!Utils::String2GUID(guid_bin, guid))
        return false;

    return CharaSsDependsExist(guid_bin);
}

bool X2mFile::CharaSsDependsExist(uint16_t id) const
{
    for (const X2mDepends &dep : chara_ss_depends)
    {
        if (dep.id == id)
            return true;
    }

    return false;
}

X2mDepends *X2mFile::FindCharaSsDepends(const uint8_t *guid)
{
    for (X2mDepends &dep : chara_ss_depends)
    {
        if (memcmp(dep.guid, guid, 16) == 0)
            return &dep;
    }

    return nullptr;
}

X2mDepends *X2mFile::FindCharaSsDepends(const std::string &guid)
{
    uint8_t guid_bin[16];

    if (!Utils::String2GUID(guid_bin, guid))
        return nullptr;

    return FindCharaSsDepends(guid_bin);
}

X2mDepends *X2mFile::FindCharaSsDepends(uint16_t id)
{
    for (X2mDepends &dep : chara_ss_depends)
    {
        if (dep.id == id)
            return &dep;
    }

    return nullptr;
}

bool X2mFile::AddCharaSsDepends(const X2mDepends &depends)
{
    if (CharaSsDependsExist(depends.guid) || CharaSsDependsExist(depends.id))
        return false;

    chara_ss_depends.push_back(depends);
    return true;
}

bool X2mFile::AddCharaSsDepends(X2mFile *ss_x2m, bool update)
{
    if (ss_x2m->GetType() != X2mType::NEW_SUPERSOUL)
        return false;

    X2mDepends *existing = FindCharaSsDepends(ss_x2m->mod_guid);
    if (existing)
    {
        if (!update)
            return false;

        existing->name = ss_x2m->GetModName();
        return true;
    }

    X2mDepends new_dep;

    new_dep.type = X2mDependsType::SUPERSOUL;
    memcpy(new_dep.guid, ss_x2m->mod_guid, 16);
    new_dep.name = ss_x2m->GetModName();

    for (uint16_t id = X2M_SS_DEPENDS_BEGIN; id < X2M_SS_DEPENDS_END; id++)
    {
        if (!CharaSsDependsExist(id))
        {
            new_dep.id = id;
            chara_ss_depends.push_back(new_dep);
            return true;
        }
    }

    return false;
}

X2mDepends *X2mFile::AddCharaSsDepends(const uint8_t *guid, const std::string &name)
{
    if (CharaSsDependsExist(guid))
        return nullptr;

    X2mDepends new_dep;

    new_dep.type = X2mDependsType::SUPERSOUL;
    memcpy(new_dep.guid, guid, 16);
    new_dep.name = name;

    for (uint16_t id = X2M_SS_DEPENDS_BEGIN; id < X2M_SS_DEPENDS_END; id++)
    {
        if (!CharaSsDependsExist(id))
        {
            new_dep.id = id;
            chara_ss_depends.push_back(new_dep);
            return &chara_ss_depends.back();
        }
    }

    return nullptr;
}

bool X2mFile::RemoveCharaSsDepends(const uint8_t *guid)
{
    for (size_t i = 0; i < chara_ss_depends.size(); i++)
    {
        if (memcmp(chara_ss_depends[i].guid, guid, 16) == 0)
        {
            chara_ss_depends.erase(chara_ss_depends.begin()+i);
            return true;
        }
    }

    return true; // Yes true
}

bool X2mFile::RemoveCharaSsDepends(const std::string &guid)
{
    uint8_t guid_bin[16];

    if (!Utils::String2GUID(guid_bin, guid))
        return false;

    return RemoveCharaSsDepends(guid_bin);
}

bool X2mFile::CharaSsDependsHasAttachment(size_t idx) const
{
    if (idx >= chara_ss_depends.size())
        return false;

    if (IsDummyMode())
        return false;

    const std::string att_path = X2M_CHARA_SS_ATTACHMENTS + Utils::GUID2String(chara_ss_depends[idx].guid) + ".x2m";
    return FileExists(att_path);
}

bool X2mFile::CharaSsDependsHasAttachment(const uint8_t *guid) const
{
    for (size_t i = 0; i < chara_ss_depends.size(); i++)
    {
        if (memcmp(chara_ss_depends[i].guid, guid, 16) == 0)
        {
            return CharaSsDependsHasAttachment(i);
        }
    }

    return false;
}

bool X2mFile::CharasSsDependsHasAttachment(const std::string &guid) const
{
    uint8_t guid_bin[16];

    if (!Utils::String2GUID(guid_bin, guid))
        return false;

    return CharaSsDependsHasAttachment(guid_bin);
}

bool X2mFile::SetCharaSsDependsAttachment(size_t idx, X2mFile *attachment)
{
    if (idx >= chara_ss_depends.size())
        return false;

    if (attachment->GetType() != X2mType::NEW_SUPERSOUL)
        return false;

    X2mDepends &dep = chara_ss_depends[idx];

    if (memcmp(dep.guid, attachment->mod_guid, 16) != 0)
        return false;

    size_t size;
    uint8_t *buf = attachment->Save(&size);

    if (!buf)
        return false;

    const std::string att_path = X2M_CHARA_SS_ATTACHMENTS + Utils::GUID2String(dep.guid) + ".x2m";
    bool ret = WriteFile(att_path, buf, size);
    delete[] buf;

    return ret;
}

bool X2mFile::SetCharaSsDependsAttachment(X2mFile *attachment)
{
    for (size_t i = 0; i < chara_ss_depends.size(); i++)
    {
        if (memcmp(chara_ss_depends[i].guid, attachment->mod_guid, 16) == 0)
        {
            return SetCharaSsDependsAttachment(i, attachment);
        }
    }

    return false;
}

bool X2mFile::RemoveCharaSsDependsAttachment(const uint8_t *guid)
{
    for (const X2mDepends &dep : chara_ss_depends)
    {
        if (memcmp(dep.guid, guid, 16) == 0)
        {
            const std::string att_path = X2M_CHARA_SS_ATTACHMENTS + Utils::GUID2String(dep.guid) + ".x2m";
            return RemoveFile(att_path);
        }
    }

    return false;
}

bool X2mFile::RemoveCharaSsDependsAttachment(const std::string &guid)
{
    uint8_t guid_bin[16];

    if (!Utils::String2GUID(guid_bin, guid))
        return false;

    return RemoveCharaSsDependsAttachment(guid_bin);
}

X2mFile *X2mFile::LoadCharaSsDependsAttachment(const uint8_t *guid)
{
    if (!CharaSsDependsHasAttachment(guid))
        return nullptr;

    const std::string att_path = X2M_CHARA_SS_ATTACHMENTS + Utils::GUID2String(guid) + ".x2m";
    size_t size;
    uint8_t *buf = ReadFile(att_path, &size);
    if (!buf)
        return nullptr;

    X2mFile *ret = new X2mFile();
    if (!ret->Load(buf, size))
    {
        delete[] buf;
        delete ret;
        return nullptr;
    }

    if (ret->type != X2mType::NEW_SUPERSOUL)
    {
        DPRINTF("%s: Not a supersoul attachment!.\n", FUNCNAME);
        return nullptr;
    }

    return ret;
}

X2mFile *X2mFile::LoadCharaSsDependsAttachment(const std::string &guid)
{
    uint8_t guid_bin[16];

    if (!Utils::String2GUID(guid_bin, guid))
        return nullptr;

    return LoadCharaSsDependsAttachment(guid_bin);
}

bool X2mFile::IsCharaSsDependsReferenced(const X2mDepends &depends) const
{
    for (const PscSpecEntry &entry : psc_entries)
    {
        if (entry.talisman == depends.id)
            return true;
    }

    return false;
}

size_t X2mFile::GetNumSkillCustomAuras() const
{
    if (!HasSkillAura())
        return 0;

    size_t num = 0;

    for (const X2mSkillAura &aura : skill_aura_entries)
    {
        if (aura.data.aur_aura_id == X2M_INVALID_ID16)
            num++;
    }

    return num;
}

CusSkill *X2mFile::FindInstalledSkill(const uint8_t *guid, X2mSkillType type)
{
    std::vector<CmsEntryXV2 *> entries;
    FindX2mSkillCMS(entries);

    for (CmsEntryXV2 *entry : entries)
    {
        for (int i = 0; i < 10; i++)
        {
            uint16_t id = (entry->id*10) + i;
            id = IdFromId2(id, type);

            CusSkill *skill = game_cus->FindSkillAnyByID(id);
            if (!skill)
                continue;

            const std::string skill_path = GetSkillDirectory(*skill, type);

            if (skill_path.length() != 0)
            {
                IniFile ini;

                if (xv2fs->LoadFile(&ini, skill_path + X2M_SKILL_INI))
                {
                    uint8_t this_guid[16];
                    std::string guid_str;

                    if (ini.GetStringValue("General", "GUID", guid_str))
                    {
                        if (Utils::String2GUID(this_guid, guid_str) && memcmp(this_guid, guid, sizeof(this_guid)) == 0)
                        {
                            return skill;
                        }
                    }
                }
            }
        }
    }

    return nullptr;
}

bool X2mFile::FindInstalledSkill()
{
    if (type != X2mType::NEW_SKILL)
        return false;

    if (skill_entry.id != X2M_DUMMY_ID16 || skill_entry.id2 != X2M_DUMMY_ID16)
    {
        DPRINTF("%s: Internal error, this function should not be called after the skill got an id assigned.\n", FUNCNAME);
        return false;
    }

    std::vector<CmsEntryXV2 *> entries;
    FindX2mSkillCMS(entries);

    for (CmsEntryXV2 *entry : entries)
    {
        for (int i = 0; i < 10; i++)
        {
            uint16_t id = (entry->id*10) + i;
            id = IdFromId2(id);

            const CusSkill *skill = game_cus->FindSkillAnyByID(id);
            if (!skill)
                continue;

            if (skill->name == skill_entry.name)
            {
                skill_entry.id2 = skill->id2;
                const std::string skill_path = GetSkillDirectory();

                if (skill_path.length() != 0)
                {
                    IniFile ini;

                    if (xv2fs->LoadFile(&ini, skill_path + X2M_SKILL_INI))
                    {
                        uint8_t guid[16];
                        std::string guid_str;

                        if (ini.GetStringValue("General", "GUID", guid_str))
                        {
                            if (Utils::String2GUID(guid, guid_str) && memcmp(guid, mod_guid, sizeof(mod_guid)) == 0)
                            {
                                // Found
                                skill_entry.id = skill->id;

                                temp_pup_in.clear();
                                temp_cus_aur_in.clear();
                                temp_aur_in.clear();

                                ini.GetMultipleIntegersValues("General", "PUP", temp_pup_in, true);
                                ini.GetMultipleIntegersValues("General", "CUS_AUR", temp_cus_aur_in, true);
                                ini.GetMultipleIntegersValues("General", "AUR", temp_aur_in, true);

                                return true;
                            }
                        }
                    }
                }

                skill_entry.id2 = X2M_DUMMY_ID16; // Revert
            }
        }
    }

    return false;
}

bool X2mFile::IsSkillCostumeDepends(const uint8_t *guid) const
{
    return (memcmp(skill_costume_depend.guid, guid, 16) == 0);
}

bool X2mFile::IsSkillCostumeDepends(const std::string &guid) const
{
    uint8_t guid_bin[16];

    if (!Utils::String2GUID(guid_bin, guid))
        return false;

    return IsSkillCostumeDepends(guid_bin);
}

bool X2mFile::IsSkillCostumeDepends(uint16_t id) const
{
    return (skill_costume_depend.id == id);
}

bool X2mFile::SetSkillCostumeDepend(const X2mDepends &depends)
{
    skill_costume_depend = depends;
    return true;
}

bool X2mFile::SetSkillCostumeDepend(X2mFile *costume_x2m)
{
    if (costume_x2m->GetType() != X2mType::NEW_COSTUME)
        return false;

    skill_costume_depend.type = X2mDependsType::COSTUME;
    memcpy(skill_costume_depend.guid, costume_x2m->mod_guid, 16);
    skill_costume_depend.name = costume_x2m->GetModName();
    skill_costume_depend.id = X2M_COSTUME_DEPENDS_ID;

    return true;
}

void X2mFile::SetSkillCostumeDepend(const uint8_t *guid, const std::string &name)
{
    skill_costume_depend.type = X2mDependsType::COSTUME;
    memcpy(skill_costume_depend.guid, guid, 16);
    skill_costume_depend.name = name;
    skill_costume_depend.id = X2M_COSTUME_DEPENDS_ID;
}

bool X2mFile::SkillCostumeDependHasAttachment() const
{
    if (IsDummyMode())
        return false;

    const std::string att_path = X2M_SKILLS_COSTUME_ATTACHMENTS + Utils::GUID2String(skill_costume_depend.guid) + ".x2m";
    return FileExists(att_path);
}

bool X2mFile::SetSkillCostumeDependAttachment(X2mFile *attachment)
{
    if (attachment->type != X2mType::NEW_COSTUME)
        return false;

    if (memcmp(skill_costume_depend.guid, attachment->mod_guid, 16) != 0)
        return false;

    size_t size;
    uint8_t *buf = attachment->Save(&size);

    if (!buf)
        return false;

    const std::string att_path = X2M_SKILLS_COSTUME_ATTACHMENTS + Utils::GUID2String(skill_costume_depend.guid) + ".x2m";
    bool ret = WriteFile(att_path, buf, size);
    delete[] buf;

    return ret;
}

bool X2mFile::RemoveSkillCostumeDependAttachment()
{
    const std::string att_path = X2M_SKILLS_COSTUME_ATTACHMENTS + Utils::GUID2String(skill_costume_depend.guid) + ".x2m";
    return RemoveFile(att_path);
}

X2mFile *X2mFile::LoadSkillCostumeDependAttachment()
{
    if (!HasSkillCostumeDepend())
        return nullptr;

    const std::string att_path = X2M_SKILLS_COSTUME_ATTACHMENTS + Utils::GUID2String(skill_costume_depend.guid) + ".x2m";
    size_t size;
    uint8_t *buf = ReadFile(att_path, &size);
    if (!buf)
        return nullptr;

    X2mFile *ret = new X2mFile();
    if (!ret->Load(buf, size))
    {
        delete[] buf;
        delete ret;
        return nullptr;
    }

    if (ret->type != X2mType::NEW_COSTUME)
    {
        DPRINTF("%s: Some retard has specified as costume attachment a mod that is not a costume.\n", FUNCNAME);
        return nullptr;
    }

    return ret;
}

bool X2mFile::IsSkillCostumeDependReferenced() const
{
    return (skill_entry.partset == skill_costume_depend.id);
}

int X2mFile::GetFreeSkillBodyId() const
{
    std::unordered_set<int> ids;

    for (const X2mBody &body : skill_bodies)
    {
        ids.insert(body.id);
    }

    if (ids.size() > 0)
    {
        for (int id = X2M_SKILL_BODY_ID_BEGIN; ; id++)
        {
            if (ids.find(id) == ids.end())
                return id;
        }
    }

    return X2M_SKILL_BODY_ID_BEGIN;
}

bool X2mFile::IsSkillCharaDepends(const uint8_t *guid) const
{
    return (memcmp(skill_chara_depend.guid, guid, 16) == 0);
}

bool X2mFile::IsSkillCharaDepends(const std::string &guid) const
{
    uint8_t guid_bin[16];

    if (!Utils::String2GUID(guid_bin, guid))
        return false;

    return IsSkillCharaDepends(guid_bin);
}

bool X2mFile::IsSkillCharaDepends(uint16_t id) const
{
    return (skill_chara_depend.id == id);
}

bool X2mFile::SetSkillCharaDepend(const X2mDepends &depends)
{
    skill_chara_depend = depends;
    return true;
}

bool X2mFile::SetSkillCharaDepend(X2mFile *char_x2m)
{
    if (char_x2m->GetType() != X2mType::NEW_CHARACTER)
        return false;

    skill_chara_depend.type = X2mDependsType::CHARACTER;
    memcpy(skill_chara_depend.guid, char_x2m->mod_guid, 16);
    skill_chara_depend.name = char_x2m->GetModName();
    skill_chara_depend.id = X2M_CHARA_DEPENDS_ID;

    return true;
}

void X2mFile::SetSkillCharaDepend(const uint8_t *guid, const std::string &name)
{
    skill_chara_depend.type = X2mDependsType::CHARACTER;
    memcpy(skill_chara_depend.guid, guid, 16);
    skill_chara_depend.name = name;
    skill_chara_depend.id = X2M_CHARA_DEPENDS_ID;
}

bool X2mFile::SkillCharaDependHasAttachment() const
{
    if (IsDummyMode())
        return false;

    const std::string att_path = X2M_SKILLS_CHARA_ATTACHMENTS + Utils::GUID2String(skill_chara_depend.guid) + ".x2m";
    return FileExists(att_path);
}

bool X2mFile::SetSkillCharaDependAttachment(X2mFile *attachment)
{
    if (attachment->type != X2mType::NEW_CHARACTER)
        return false;

    if (memcmp(skill_chara_depend.guid, attachment->mod_guid, 16) != 0)
        return false;

    size_t size;
    uint8_t *buf = attachment->Save(&size);

    if (!buf)
        return false;

    const std::string att_path = X2M_SKILLS_CHARA_ATTACHMENTS + Utils::GUID2String(skill_chara_depend.guid) + ".x2m";
    bool ret = WriteFile(att_path, buf, size);
    delete[] buf;

    return ret;
}

bool X2mFile::RemoveSkillCharaDependAttachment()
{
    const std::string att_path = X2M_SKILLS_CHARA_ATTACHMENTS + Utils::GUID2String(skill_chara_depend.guid) + ".x2m";
    return RemoveFile(att_path);
}

X2mFile *X2mFile::LoadSkillCharaDependAttachment()
{
    if (!HasSkillCharaDepend())
        return nullptr;

    const std::string att_path = X2M_SKILLS_CHARA_ATTACHMENTS + Utils::GUID2String(skill_chara_depend.guid) + ".x2m";
    size_t size;
    uint8_t *buf = ReadFile(att_path, &size);
    if (!buf)
        return nullptr;

    X2mFile *ret = new X2mFile();
    if (!ret->Load(buf, size))
    {
        delete[] buf;
        delete ret;
        return nullptr;
    }

    if (ret->type != X2mType::NEW_CHARACTER)
    {
        DPRINTF("%s: Some retard has specified as char attachment a mod that is not a char.\n", FUNCNAME);
        return nullptr;
    }

    return ret;
}

bool X2mFile::IsSkillCharaDependReferenced() const
{
    return (skill_entry.model == skill_chara_depend.id);
}

size_t X2mFile::GetNumCostumePartSets(uint8_t race) const
{
    size_t count = 0;

    for (const X2mPartSet &set : costume_partsets)
    {
        if (set.race == race)
            count++;
    }

    return count;
}

size_t X2mFile::GetEffectiveNumCostumePartSets() const
{
    for (int i = 0; i < X2M_CR_NUM; i++)
    {
        size_t count = GetNumCostumePartSets((uint8_t)i);
        if (count != 0)
            return count;
    }

    return 0;
}

size_t X2mFile::GetNumCostumeUsedRaces() const
{
    size_t count = 0;

    for (int i = 0; i < X2M_CR_NUM; i++)
    {
        if (GetNumCostumePartSets(i) != 0)
            count++;
    }

    return count;
}

const X2mPartSet &X2mFile::GetCostumePartSet(uint8_t race, size_t idx) const
{
    size_t i = 0;

    for (const X2mPartSet &set : costume_partsets)
    {
        if (set.race == race)
        {
            if (i == idx)
            {
                return set;
            }

            i++;
        }
    }

    DPRINTF("%s: Fatal error: Index out of bounds. idx=%Id, race=%d\n", FUNCNAME, idx, race);
    exit(-1); // Todo: replace with exception

    // Make the compiler hapy

    X2mPartSet *ret = new X2mPartSet();
    return *ret;
}

X2mPartSet &X2mFile::GetCostumePartSet(uint8_t race, size_t idx)
{
    size_t i = 0;

    for (X2mPartSet &set : costume_partsets)
    {
        if (set.race == race)
        {
            if (i == idx)
            {
                return set;
            }

            i++;
        }
    }

    DPRINTF("%s: Fatal error: Index out of bounds. idx=%Id, race=%d\n", FUNCNAME, idx, race);
    exit(-1); // Todo: replace with exception

    // Make the compiler hapy

    X2mPartSet *ret = new X2mPartSet();
    return *ret;
}

void X2mFile::ClearCostumePartSets(uint8_t race)
{
    for (size_t i = 0; i < costume_partsets.size(); i++)
    {
        if (costume_partsets[i].race == race)
        {
            costume_partsets.erase(costume_partsets.begin()+i);
            i--;
        }
    }
}

X2mCostumeEntry *X2mFile::FindInstalledCostume()
{
    X2mCostumeEntry *costume = game_costume_file->FindCostume(mod_guid);
    if (costume)
    {
        update_costume_entry = *costume; // Copy
        return &update_costume_entry;
    }

    update_costume_entry = X2mCostumeEntry(); // Reset
    return nullptr;
}

X2mCostumeEntry *X2mFile::FindInstalledCostume(const uint8_t *guid)
{
    return game_costume_file->FindCostume(guid);
}

void X2mFile::DeleteStageDirectory()
{
    DeleteDir(X2M_STAGE);
}

HcaFile *X2mFile::GetStageBgm()
{
    if (!StageHasCustomBgm())
        return nullptr;

    if (IsDummyMode())
        return RecoverStageBgm();

    size_t size;
    uint8_t *buf = ReadFile(X2M_STAGE_CUSTOM_BGM, &size);

    if (!buf)
        return nullptr;


    HcaFile *hca = new HcaFile();
    if (!hca->Load(buf, size))
    {
        delete hca;
        return nullptr;
    }

    hca->SetCiphType(0);
    return hca;
}

bool X2mFile::SetStageBgm(const std::string &file)
{
    size_t size;
    uint8_t *buf = Utils::ReadFile(file, &size);
    if (!buf)
        return false;

    bool ret = WriteFile(X2M_STAGE_CUSTOM_BGM, buf, size);
    delete[] buf;

    return ret;
}

void X2mFile::RemoveStageBgm()
{
    if (!StageHasCustomBgm())
        return;

    _DeleteFile(X2M_STAGE_CUSTOM_BGM);
}

bool X2mFile::SetQuestAttachmentsVisitor(const std::string &path, bool, void *param)
{
    X2mFile *pthis = (X2mFile *)param;

    if (Utils::EndsWith(path, ".x2m", false))
    {
        size_t size;
        uint8_t *buf = Utils::ReadFile(path, &size);
        if (buf)
        {
            std::string filename = Utils::GetFileNameString(path);
            pthis->WriteFile(X2M_QUEST_ATTACHMENTS + filename, buf, size);
            delete[] buf;
            pthis->temp_quest_attachments_num++;
        }
    }

    return true;
}

size_t X2mFile::SetQuestAttachments(const std::string &dir)
{
    temp_quest_attachments_num = 0;
    Utils::VisitDirectory(dir, true, false, false, SetQuestAttachmentsVisitor, this);
    return temp_quest_attachments_num;
}

bool X2mFile::GetQuestAttachmentsVisitor(const std::string &path, void *param)
{
    X2mFile *pthis = (X2mFile *)param;

    if (Utils::EndsWith(path, ".x2m", false))
    {
        X2mFile *x2m = new X2mFile();
        size_t size;
        uint8_t *buf = pthis->ReadFile(path, &size);

        if (!buf)
        {
            delete x2m;
            return true;
        }

        bool ret = x2m->Load(buf, size);
        delete[] buf;
        if (!ret)
        {
            delete x2m;
            return true;
        }

        pthis->temp_attachments->push_back(x2m);
    }

    return true;
}

size_t X2mFile::GetQuestAttachments(std::vector<X2mFile *> &x2ms)
{
    x2ms.clear();
    temp_attachments = &x2ms;

    if (IsDummyMode())
        return 0;

    VisitDirectory(X2M_QUEST_ATTACHMENTS, GetQuestAttachmentsVisitor, this);
    return x2ms.size();
}

bool X2mFile::SetSSSkillDepend(const X2mDepends &depends)
{
    ss_blast_depend = depends;
    return true;
}

bool X2mFile::SetSSSkillDepend(X2mFile *skill_x2m)
{
    if (skill_x2m->GetType() != X2mType::NEW_SKILL || skill_x2m->GetSkillType() != X2mSkillType::BLAST || !skill_x2m->BlastSkillSsIntended())
        return false;

    ss_blast_depend.type = X2mDependsType::SKILL;
    memcpy(ss_blast_depend.guid, skill_x2m->mod_guid, 16);
    ss_blast_depend.name = skill_x2m->GetModName();
    ss_blast_depend.id = X2M_SKILL_DEPENDS_BEGIN;

    return true;
}

void X2mFile::SetSSSkillDepend(const uint8_t *guid, const std::string &name)
{
    ss_blast_depend.type = X2mDependsType::SKILL;
    memcpy(ss_blast_depend.guid, guid, 16);
    ss_blast_depend.name = name;
    ss_blast_depend.id = X2M_SKILL_DEPENDS_BEGIN;
}

bool X2mFile::SSSkillDependHasAttachment() const
{
    if (IsDummyMode())
        return false;

    const std::string att_path = X2M_SS_BLAST_ATTACHMENT + Utils::GUID2String(ss_blast_depend.guid) + ".x2m";
    return FileExists(att_path);
}

bool X2mFile::SetSSSkillDependAttachment(X2mFile *attachment)
{
    if (attachment->type != X2mType::NEW_SKILL || attachment->GetSkillType() != X2mSkillType::BLAST || !attachment->BlastSkillSsIntended())
        return false;

    if (memcmp(ss_blast_depend.guid, attachment->mod_guid, 16) != 0)
        return false;

    size_t size;
    uint8_t *buf = attachment->Save(&size);

    if (!buf)
        return false;

    const std::string att_path = X2M_SS_BLAST_ATTACHMENT + Utils::GUID2String(ss_blast_depend.guid) + ".x2m";
    bool ret = WriteFile(att_path, buf, size);
    delete[] buf;

    return ret;
}

bool X2mFile::RemoveSSSkillDependAttachment()
{
    const std::string att_path = X2M_SS_BLAST_ATTACHMENT + Utils::GUID2String(ss_blast_depend.guid) + ".x2m";
    return RemoveFile(att_path);
}

X2mFile *X2mFile::LoadSSSkillDependAttachment()
{
    if (!HasSSSkillDepend())
        return nullptr;

    const std::string att_path = X2M_SS_BLAST_ATTACHMENT + Utils::GUID2String(ss_blast_depend.guid) + ".x2m";
    size_t size;
    uint8_t *buf = ReadFile(att_path, &size);
    if (!buf)
        return nullptr;

    X2mFile *ret = new X2mFile();
    if (!ret->Load(buf, size))
    {
        delete[] buf;
        delete ret;
        return nullptr;
    }

    if (ret->type != X2mType::NEW_SKILL || ret->GetSkillType() != X2mSkillType::BLAST || !ret->BlastSkillSsIntended())
    {
        DPRINTF("%s: this type of attachment is not valid for this x2m.\n", FUNCNAME);
        return nullptr;
    }

    return ret;
}

X2mSuperSoul *X2mFile::FindInstalledSS()
{
    X2mSuperSoul *ss = game_costume_file->FindSuperSoul(mod_guid);
    if (ss)
    {
        update_ss_entry = *ss; // Copy
        return &update_ss_entry;
    }

    update_ss_entry = X2mSuperSoul(); // Reset
    return nullptr;
}

X2mSuperSoul *X2mFile::FindInstalledSS(const uint8_t *guid)
{
    return game_costume_file->FindSuperSoul(guid);
}

bool X2mFile::InstallCharaName()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    int ls = 0, le = XV2_LANG_NUM;
    if (global_lang >= 0)
    {
        ls = global_lang;
        le = global_lang+1;
    }

    for (int i = ls; i < le; i++)
    {
        std::string *name;

        Xenoverse2::RemoveAllCharaName(entry_name, i);

        if (i == XV2_LANG_ENGLISH)
        {
            if (chara_name[i].length() == 0)
                return false;

            name = &chara_name[i];
        }
        else
        {
            if (chara_name[i].length() == 0)
            {
                name = &chara_name[XV2_LANG_ENGLISH];
            }
            else
            {
                name = &chara_name[i];
            }
        }

        if (!Xenoverse2::SetCharaName(entry_name, *name, i))
            return false;

        if (HasMultNames())
        {
            for (size_t j = 0; j < slot_entries.size(); j++)
            {
                std::string specific_name = *name;
                const X2mSlotEntry &slot = slot_entries[j];

                for (size_t k = 0; k < mult_chara_names.size(); k++)
                {
                    const X2mCharaName &m_chara_name = mult_chara_names[k];

                    if (m_chara_name.costume_index == (uint32_t)slot.costume_index)
                    {
                        if (i == XV2_LANG_ENGLISH)
                        {
                            if (m_chara_name.chara_name[i].length() == 0)
                                return false;

                            specific_name = m_chara_name.chara_name[i];
                        }
                        else
                        {
                            if (m_chara_name.chara_name[i].length() == 0)
                            {
                                specific_name = m_chara_name.chara_name[XV2_LANG_ENGLISH];
                            }
                            else
                            {
                                specific_name = m_chara_name.chara_name[i];
                            }
                        }

                        break;
                    }
                } // end k loop

                if (!Xenoverse2::SetCharaName(entry_name, specific_name, i, (int)j))
                    return false;
            }
        }
    }

    return true;
}

bool X2mFile::InstallCostumeNames()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (slot_entries.size() == 0)
        return false;

    for (size_t i = 0; i < slot_entries.size(); i++)
    {
        const X2mSlotEntry &entry = slot_entries[i];

        int ls = 0, le = XV2_LANG_NUM;
        if (global_lang >= 0)
        {
            ls = global_lang;
            le = global_lang+1;
        }

        for (int j = ls; j < le; j++)
        {
            const std::string *name;

            if (j == XV2_LANG_ENGLISH)
            {
                if (entry.costume_name[j].length() == 0)
                    return false;

                name = &entry.costume_name[j];
            }
            else
            {
                if (entry.costume_name[j].length() == 0)
                {
                    name = &entry.costume_name[XV2_LANG_ENGLISH];
                }
                else
                {
                    name = &entry.costume_name[j];
                }
            }

            if (!Xenoverse2::SetCharaCostumeName(entry_name, entry.costume_index, entry.model_preset, *name, j))
                return false;
        }

    }

    return true;
}

void X2mFile::UninstallCssAudio()
{
    for (int i = 0; i < 100; i++)
    {
        const std::string name = std::string("X2M_") + entry_name + "_" + Utils::ToStringAndPad(i, 2);
        Xenoverse2::FreeCssSound(name, false);
        Xenoverse2::FreeCssSound(name, true);
    }
}

bool X2mFile::InstallCssAudio(X2mSlotEntry &entry)
{
    if (format_version < X2M_MIN_VERSION_CSS_AUDIO)
        return true;

    for (int i = 0 ; i < 2; i++)
    {
        if (entry.voices_id_list[i] != (int)X2M_DUMMY_ID)
            continue;

        if (entry.audio_files[i].length() == 0)
        {
            DPRINTF("%s: Internal error, audio file path is empty and voices id list is dummy.\n", FUNCNAME);
            return false;
        }

        auto it = std::find(installed_css_audio.begin(), installed_css_audio.end(), entry.audio_files[i]);
        if (it != installed_css_audio.end())
        {
            size_t index = it - installed_css_audio.begin();
            entry.voices_id_list[i] = installed_css_cue[index];
            continue;
        }

        if (installed_css_audio.size() == 100)
        {
            DPRINTF("%s: Audio overflow, 100 or more audio entries in the mod.\n", FUNCNAME);
            return false;
        }

        if (!AudioFileExists(entry.audio_files[i]))
        {
            DPRINTF("%s: Internal error, audio file with base name \"%s\" doesn't exist in package.\n", FUNCNAME, entry.audio_files[i].c_str());
            return false;
        }

        HcaFile *hca_jp, *hca_en;
        const std::string cue_name = "X2M_" + entry_name + "_" + Utils::ToStringAndPad((int)installed_css_audio.size(), 2);

        if (IsDummyMode())
        {
            hca_jp = RecoverCssSound(cue_name, false);
            if (!hca_jp)
            {
                DPRINTF("%s: couldn't recover CSS sound (JP) \"%s\"", FUNCNAME, cue_name.c_str());
                return false;
            }

            hca_en = RecoverCssSound(cue_name, true);
            if (!hca_en)
            {
                DPRINTF("%s: couldn't recover CSS sound (EN) \"%s\"", FUNCNAME, cue_name.c_str());
                return false;
            }
        }
        else
        {
            hca_jp = LoadAudioFile(entry.audio_files[i], false, true);
            if (!hca_jp)
            {
                DPRINTF("%s: Cannot load css audio sound \"%s\". Most likely reasons is that it is an invalid HCA file.\n", FUNCNAME, entry.audio_files[i].c_str());
                return false;
            }

            hca_en = LoadAudioFile(entry.audio_files[i], true, true);
            if (!hca_en)
            {
                DPRINTF("%s: Cannot load css audio sound \"%s\" (2). Most likely reasons is that it is an invalid HCA file.\n", FUNCNAME, entry.audio_files[i].c_str());
                delete hca_jp;
                return false;
            }
        }

        uint32_t cue_id_jp, cue_id_en;        

        cue_id_jp = Xenoverse2::SetCssSound(cue_name, false, *hca_jp, 3, 4);
        delete hca_jp;

        if (cue_id_jp == (uint32_t)-1)
        {
            DPRINTF("%s: SetCssSound failed when trying to set japanese track.\n", FUNCNAME);
            delete hca_en;
            return false;
        }

        cue_id_en = Xenoverse2::SetCssSound(cue_name, true, *hca_en, 3, 4);
        delete hca_en;

        if (cue_id_en == (uint32_t)-1)
        {
            DPRINTF("%s: SetCssSound failed when trying to set english track.\n", FUNCNAME);
            return false;
        }

        if (cue_id_jp != cue_id_en)
        {
            DPRINTF("%s: English and Japanese CRT_CS_vox.acb are unsynchronized and cannot assign a common cue_id.\n\nPossible reasons:\n"
                    "- One of them was deleted by user\n."
                    "- One of them was edited by the user to add a track and the other one wasn't.\n"
                    "- One of them was set by the user to a previous version and the other wasn't.\n", FUNCNAME);
            return false;
        }

        entry.voices_id_list[i] = cue_id_jp;
        installed_css_audio.push_back(entry.audio_files[i]);
        installed_css_cue.push_back(cue_id_jp);
    }

    return true;
}

size_t X2mFile::FindX2mSkillCMS(std::vector<CmsEntryXV2 *> &entries, CmsFile *another_cms)
{
    entries.clear();

    CmsFile *cms = (another_cms) ? another_cms : game_cms;

    for (CmsEntry *entry : *cms)
    {
        CmsEntryXV2 *entry_xv2 = dynamic_cast<CmsEntryXV2 *>(entry);
        if (!entry_xv2)
            continue;

        if (entry_xv2->unk_10 == X2M_DUMMY_ID)
            entries.push_back(entry_xv2);
    }

    return entries.size();
}

uint16_t X2mFile::IdFromId2(uint16_t id2) const
{
    if (skill_type == X2mSkillType::SUPER)
    {
        return id2 + CUS_SUPER_START;
    }
    else if (skill_type == X2mSkillType::ULTIMATE)
    {
        return id2 + CUS_ULTIMATE_START;
    }
    else if (skill_type == X2mSkillType::EVASIVE)
    {
        return id2 + CUS_EVASIVE_START;
    }
    else if (skill_type == X2mSkillType::BLAST)
    {
        return id2 + CUS_BLAST_START;
    }
    else if (skill_type == X2mSkillType::AWAKEN)
    {
        return id2 + CUS_AWAKEN_START;
    }

    DPRINTF("%s: Internal error.\n", FUNCNAME);
    exit(-1);

    return 0xFFFF;
}

uint16_t X2mFile::IdFromId2(uint16_t id2, X2mSkillType skill_type)
{
    if (skill_type == X2mSkillType::SUPER)
    {
        return id2 + CUS_SUPER_START;
    }
    else if (skill_type == X2mSkillType::ULTIMATE)
    {
        return id2 + CUS_ULTIMATE_START;
    }
    else if (skill_type == X2mSkillType::EVASIVE)
    {
        return id2 + CUS_EVASIVE_START;
    }
    else if (skill_type == X2mSkillType::BLAST)
    {
        return id2 + CUS_BLAST_START;
    }
    else if (skill_type == X2mSkillType::AWAKEN)
    {
        return id2 + CUS_AWAKEN_START;
    }

    DPRINTF("%s: Internal error.\n", FUNCNAME);
    exit(-1);

    return 0xFFFF;
}

bool X2mFile::AssignSkillIds()
{
    if (type != X2mType::NEW_SKILL)
    {
        DPRINTF("%s: Internal error, this function must only be called for skill x2m.\n", FUNCNAME);
        return false;
    }

    std::vector<CmsEntryXV2 *> entries;
    FindX2mSkillCMS(entries);

    int type;

    if (skill_type == X2mSkillType::SUPER)
    {
        type = CUS_SKILL_TYPE_SUPER;
    }
    else if (skill_type == X2mSkillType::ULTIMATE)
    {
        type = CUS_SKILL_TYPE_ULTIMATE;
    }
    else if (skill_type == X2mSkillType::EVASIVE)
    {
        type = CUS_SKILL_TYPE_EVASIVE;
    }
    else if (skill_type == X2mSkillType::BLAST)
    {
        type = CUS_SKILL_TYPE_BLAST;
    }
    else if (skill_type == X2mSkillType::AWAKEN)
    {
        type = CUS_SKILL_TYPE_AWAKEN;
    }
    else
    {
        DPRINTF("%s Internal error.\n", FUNCNAME);
        return false;
    }

    uint16_t desired_lower_limit = 0;
    if (type == CUS_SKILL_TYPE_BLAST && blast_ss_intended)
    {
        desired_lower_limit = 2000;
    }

    for (CmsEntryXV2 *entry : entries)
    {
        for (int i = 0; i < 10; i++)
        {
            if (!game_cus->IsSkillInUse(entry->id, i, type) && ((uint16_t)entry->id*10) >= desired_lower_limit)
            {
                skill_entry.id2 = (entry->id*10)+i;
                skill_entry.id = IdFromId2(skill_entry.id2);
                return true;
            }
        }
    }

    CmsEntryXV2 dummy_char;
    bool found = false;

    for (int i = 0; i < 100; i++)
    {
        dummy_char.name = "X";

        for (int j = 1; j < 3; j++)
        {            
            while (1)
            {
                char b;

                Utils::GetRandomData(&b, 1);

                if ((b >= 'A' && b <= 'Z') || (b >= '0' && b <= '9'))
                {
                    dummy_char.name += b;
                    break;
                }
            }
        }

        if (!game_cms->FindEntryByName(dummy_char.name))
        {
            if (!IsDummyMode() || restore_cms_set.find(dummy_char.name) == restore_cms_set.end())
            {
                found = true;
                break;
            }
        }
    }

    if (!found)
    {
        // If we didn't find an entry that starts by X, let's find any other entry
        while (1)
        {
            dummy_char.name.clear();

            for (int j = 0; j < 3; j++)
            {
                while (1)
                {
                    char b;

                    Utils::GetRandomData(&b, 1);

                    if ((b >= 'A' && b <= 'Z') || (b >= '0' && b <= '9'))
                    {
                        dummy_char.name += b;
                        break;
                    }
                }
            }

            if (!game_cms->FindEntryByName(dummy_char.name))
            {
                if (!IsDummyMode() || restore_cms_set.find(dummy_char.name) == restore_cms_set.end())
                    break;
            }
        }
    }

    dummy_char.unk_10 = dummy_char.unk_1C = X2M_DUMMY_ID;
    dummy_char.unk_16 = dummy_char.load_cam_dist = dummy_char.unk_18 = dummy_char.unk_1A = X2M_DUMMY_ID16;

    if (!game_cms->AddEntryXV2(dummy_char, true))
        return false;

    if (dummy_char.id >= 500)
    {
        DPRINTF("%s: Overflow of characters. Cannot get a proper fake character entry for skill.\n"
                "Uninstall some characters, then install this mod, and then reinstall again those characters.\n", FUNCNAME);
        return false;
    }

    skill_entry.id2 = dummy_char.id*10;
    skill_entry.id = IdFromId2(skill_entry.id2);

    return true;
}

std::string X2mFile::GetSkillDirectory() const
{
    if (type != X2mType::NEW_SKILL)
    {
        DPRINTF("%s: This function can ony be called by skills x2m.\n", FUNCNAME);
        return std::string();
    }

    std::string ret = "data/skill/";

    if (skill_type == X2mSkillType::SUPER)
        ret += "SPA/";
    else if (skill_type == X2mSkillType::ULTIMATE)
        ret += "ULT/";
    else if (skill_type == X2mSkillType::EVASIVE)
        ret += "ESC/";
    else if (skill_type == X2mSkillType::BLAST)
        ret += "BLT/";
    else if (skill_type == X2mSkillType::AWAKEN)
        ret += "MET/";
    else
    {
        DPRINTF("%s: Internal error.\n", FUNCNAME);
        return std::string();
    }

    ret += Utils::ToStringAndPad(skill_entry.id2, 3);
    ret += '_';

    CmsEntry *entry = game_cms->FindEntryByID(skill_entry.id2 / 10);
    if (!entry)
        return std::string();

    ret += entry->name;
    ret += '_';
    ret += skill_entry.name;
    ret += '/';

    return ret;
}

std::string X2mFile::GetSkillDirectory(const CusSkill &skill_entry, X2mSkillType skill_type)
{
    std::string ret = "data/skill/";

    if (skill_type == X2mSkillType::SUPER)
        ret += "SPA/";
    else if (skill_type == X2mSkillType::ULTIMATE)
        ret += "ULT/";
    else if (skill_type == X2mSkillType::EVASIVE)
        ret += "ESC/";
    else if (skill_type == X2mSkillType::BLAST)
        ret += "BLT/";
    else if (skill_type == X2mSkillType::AWAKEN)
        ret += "MET/";
    else
    {
        DPRINTF("%s: Internal error.\n", FUNCNAME);
        return std::string();
    }

    ret += Utils::ToStringAndPad(skill_entry.id2, 3);
    ret += '_';

    CmsEntry *entry = game_cms->FindEntryByID(skill_entry.id2 / 10);
    if (!entry)
        return std::string();

    ret += entry->name;
    ret += '_';
    ret += skill_entry.name;
    ret += '/';

    return ret;
}

uint16_t X2mFile::GetCusSkillType() const
{
    if (skill_type == X2mSkillType::SUPER)
    {
        return CUS_SKILL_TYPE_SUPER;
    }
    else if (skill_type == X2mSkillType::ULTIMATE)
    {
        return CUS_SKILL_TYPE_ULTIMATE;
    }
    else if (skill_type == X2mSkillType::EVASIVE)
    {
        return CUS_SKILL_TYPE_EVASIVE;
    }
    else if (skill_type == X2mSkillType::BLAST)
    {
        return CUS_SKILL_TYPE_BLAST;
    }
    else if (skill_type == X2mSkillType::AWAKEN)
    {
        return CUS_SKILL_TYPE_AWAKEN;
    }

    return 0;
}

void X2mFile::AssignSkillIdbIds()
{
    if (!HasSkillIdbEntry())
        return;

    if (skill_entry.id2 == X2M_DUMMY_ID16 || skill_entry.id2 == X2M_INVALID_ID16)
        return;

    skill_idb_entry.id = skill_idb_entry.name_id = skill_idb_entry.desc_id = skill_idb_entry.how_id = skill_entry.id2;
    skill_idb_entry.type = GetCusSkillType();
}

bool X2mFile::InstallSlots(bool update)
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (invisible)
        return true; // Yes, true

    UninstallCssAudio();
    installed_css_audio.clear();
    installed_css_cue.clear();

    if (update)
    {
        std::vector<CharaListSlotEntry *> l_entries;
        chara_list->FindSlotsByCode(std::string("\"") + entry_name + std::string("\""), l_entries);

        if (l_entries.size() != slot_entries.size())
        {
            DPRINTF("%s: Update mode failed because there are different num of slots in the list than in the mod. Try uninstalling the mod and install it again.\n", FUNCNAME);
            return false;
        }

        for (size_t i = 0; i < l_entries.size(); i++)
        {
            X2mSlotEntry &x_entry = slot_entries[i];

            if (!InstallCssAudio(x_entry))
                return false;

            x_entry.CopyTo(*(l_entries[i]), entry_name);
        }

        return true;
    }

    std::vector<CharaListSlot> &chara_slots = chara_list->GetSlots();

    if (chara_slots.size() >= XV2_MAX_SLOTS)
        return false;

    CharaListSlot slot;

    slot.entries.resize(slot_entries.size());

    for (size_t i = 0; i < slot_entries.size(); i++)
    {
        X2mSlotEntry &x_entry = slot_entries[i];
        CharaListSlotEntry l_entry = slot.entries[i]; // Copy

        if (!InstallCssAudio(x_entry))
            return false;

        x_entry.CopyTo(l_entry, entry_name);
        slot.entries[i] = l_entry;
    }

    chara_slots.push_back(slot);
    return true;
}

bool X2mFile::InstallSlots(const std::vector<size_t> &positions)
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (invisible)
        return true; // Yes, true

    UninstallCssAudio();
    installed_css_audio.clear();
    installed_css_cue.clear();

    if (positions.size() != slot_entries.size())
        return false;

    std::vector<CharaListSlot> &chara_slots = chara_list->GetSlots();

    // Check first
    for (size_t pos : positions)
    {
        if (pos >= chara_slots.size())
            return false;

        if (chara_slots[pos].entries.size() >= XV2_MAX_SUBSLOTS)
            return false;
    }

    for (size_t i = 0; i < slot_entries.size(); i++)
    {
        X2mSlotEntry &x_entry = slot_entries[i];
        CharaListSlotEntry l_entry;

        if (!InstallCssAudio(x_entry))
            return false;

        x_entry.CopyTo(l_entry, entry_name);
        chara_slots[positions[i]].entries.push_back(l_entry);

        if (chara_slots[positions[i]].entries.size() >= XV2_MAX_SUBSLOTS)
            return false;
    }

    return true;
}

bool X2mFile::InstallCms(bool update)
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (cms_entry.id == X2M_INVALID_ID)
        return false;

    cms_entry.name = entry_name;

    if (update)
    {
        CmsEntryXV2 *entry = dynamic_cast<CmsEntryXV2 *>(game_cms->FindEntryByName(cms_entry.name));

        if (!entry)
        {
            DPRINTF("%s: Update failed because cms entry of mod doesn't exist. A fix may be to uninstall mod and install it again.\n", FUNCNAME);
            return false;
        }

        cms_entry.id = entry->id;
        *entry = cms_entry;
        return true;
    }

    return game_cms->AddEntryXV2(cms_entry, true);
}

void X2mFile::AssignCharaCustomSkills(CusSkillSet &x_set)
{
    if (!HasCharaSkillDepends())
        return;

    for (int i = 0; i < 9; i++)
    {
        if (x_set.char_skills[i] >= X2M_SKILL_DEPENDS_BEGIN && x_set.char_skills[i] < X2M_SKILL_DEPENDS_END)
        {
            X2mDepends *dep = FindCharaSkillDepends(x_set.char_skills[i]);

            if (dep)
            {
                X2mSkillType st;

                if (i < 4)
                {
                    st = X2mSkillType::SUPER;
                }
                else if (i < 6)
                {
                    st = X2mSkillType::ULTIMATE;
                }
                else if (i == 6)
                {
                    st = X2mSkillType::EVASIVE;
                }
                else if (i == 7)
                {
                    st = X2mSkillType::BLAST;
                }
                else
                {
                    st = X2mSkillType::AWAKEN;
                }

                CusSkill *skill = FindInstalledSkill(dep->guid,st);
                if (skill)
                {
                    x_set.char_skills[i] = skill->id;
                    continue;
                }
            }

            x_set.char_skills[i] = 0xFFFF; // If not found, unassign skill
        }
    }
}

void X2mFile::AssignCharaCustomSs()
{
    if (!HasCharaSsDepends())
        return;

    for (PscSpecEntry &entry : psc_entries)
    {
        uint32_t ss_id = entry.talisman;

        if (ss_id >= X2M_SS_DEPENDS_BEGIN && ss_id < X2M_SS_DEPENDS_END)
        {
            entry.talisman = 0xFFFFFFFF; // Assign default none in case it's not found

            X2mDepends *dep = FindCharaSsDepends(ss_id);
            if (dep)
            {
                X2mSuperSoul *ss = FindInstalledSS(dep->guid);
                if (ss)
                {
                    entry.talisman = ss->idb_id;
                }
            }
        }
    }
}

bool X2mFile::InstallCus(bool update)
{
    if (cms_entry.id == X2M_DUMMY_ID || cms_entry.id == X2M_INVALID_ID)
    {
        DPRINTF("%s: Internal error. InstallCms must be called before this!!!\n", FUNCNAME);
        return false;
    }

    if (skill_sets.size() != slot_entries.size())
    {
        DPRINTF("%s: Internal error.\n", FUNCNAME);
        return false;
    }

    if (update)
    {
        std::vector<CusSkillSet *> sets;

        game_cus->FindSkillSetsByCharID(cms_entry.id, sets);

        if (sets.size() != skill_sets.size())
        {
            DPRINTF("%s: Update mode failed because there are different num of skillset in the cus than in the mod (for this char). Try uninstalling the mod and install it again.\n", FUNCNAME);
            return false;
        }

        for (size_t i = 0; i < sets.size(); i++)
        {
            CusSkillSet &x_set = skill_sets[i];

            x_set.char_id = cms_entry.id;
            x_set.costume_id = slot_entries[i].costume_index;
            x_set.model_preset = slot_entries[i].model_preset;
            AssignCharaCustomSkills(x_set);

            *(sets[i]) = x_set;
        }

        return true;
    }

    // New install

    for (size_t i = 0; i < skill_sets.size(); i++)
    {
        CusSkillSet &x_set = skill_sets[i];

        x_set.char_id = cms_entry.id;
        x_set.costume_id = slot_entries[i].costume_index;
        x_set.model_preset = (uint16_t)slot_entries[i].model_preset;
        AssignCharaCustomSkills(x_set);

        if (!game_cus->AddSkillSet(x_set, (i == 0)))
        {
            DPRINTF("%s: AddSkillSet %d  failed. You probably had a dead id in the cus file, caused by manual editing or a replacer mod.\n", FUNCNAME, i);
            return false;
        }
    }

    return true;
}

bool X2mFile::FillCsoEntries()
{
    std::unordered_set<int32_t> costumes;
    size_t i = 0;

    for (const X2mSlotEntry &slot : slot_entries)
    {
        if (costumes.find(slot.costume_index) == costumes.end())
        {
            costumes.insert(slot.costume_index);

            if (i >= cso_entries.size())
                return false;

            cso_entries[i].char_id = cms_entry.id;
            cso_entries[i].costume_id = slot.costume_index;
            i++;
        }
    }

    return (costumes.size() == cso_entries.size());
}

bool X2mFile::InstallCso(bool update)
{
    if (cms_entry.id == X2M_DUMMY_ID || cms_entry.id == X2M_INVALID_ID)
    {
        DPRINTF("%s: Internal error. InstallCms must be called before this!!!\n", FUNCNAME);
        return false;
    }

    std::vector<CsoEntry *> entries;
    game_cso->FindEntriesByCharID(cms_entry.id, entries);

    if (!HasCso())
    {
        if (entries.size() != 0)
        {
            DPRINTF("%s: Update mode failed because there were cso entry(es) in the cso, but 0 here. Uninstall the mod and reinstall it will fix this.\n", FUNCNAME);
            return false;
        }

        return true; // Yes, true!
    }

    if (cso_entries.size() != GetNumCostumes())
    {
        DPRINTF("%s: Internal error.\n", FUNCNAME);
        return false;
    }

    if (update)
    {
        if (entries.size() == 0)
        {
            goto regular_install;
        }
        else if (entries.size() != cso_entries.size())
        {
            DPRINTF("%s: Update mode failed because there are different num of cso entries in the cso than in the mod (for this char). Try uninstalling the mod and install it again.\n", FUNCNAME);
            return false;
        }

        if (!FillCsoEntries())
        {
            DPRINTF("%s: FillCsoEntries failed.\n", FUNCNAME);
            return false;
        }

        for (size_t i = 0; i < cso_entries.size(); i++)
        {
            const CsoEntry &entry = cso_entries[i];
            *(entries[i]) = entry;
        }

        return true;
    }

regular_install:

    if (!FillCsoEntries())
    {
        DPRINTF("%s: FillCsoEntries failed.\n", FUNCNAME);
        return false;
    }

    for (size_t i = 0; i < cso_entries.size(); i++)
    {
        const CsoEntry &entry = cso_entries[i];

        if (!game_cso->AddEntry(entry, (i == 0)))
            return false;
    }

    return true;
}

bool X2mFile::FillPscEntries()
{
    std::unordered_set<int32_t> costumes;
    size_t i = 0;

    for (const X2mSlotEntry &slot : slot_entries)
    {
        if (costumes.find(slot.costume_index) == costumes.end())
        {
            costumes.insert(slot.costume_index);

            if (i >= psc_entries.size())
                return false;

            psc_entries[i].costume_id = psc_entries[i].costume_id2 = slot.costume_index;
            i++;
        }
    }

    return (costumes.size() == psc_entries.size());
}

bool X2mFile::InstallPsc()
{
    if (cms_entry.id == X2M_DUMMY_ID || cms_entry.id == X2M_INVALID_ID)
    {
        DPRINTF("%s: Internal error. InstallCms must be called before this!!!\n", FUNCNAME);
        return false;
    }

    PscEntry new_entry;
    std::vector<PscEntry *> existing_entries;

    game_psc->FindEntries(cms_entry.id, existing_entries);

    if (!HasPsc())
    {
        if (existing_entries.size() != 0)
        {
            DPRINTF("%s: Update mode failed because there was a psc entry in the psc, but 0 in the mod. Uninstalling the mod and reinstall it will fix this.\n", FUNCNAME);
            return false;
        }

        return true; // Yes, true!
    }

    if (!FillPscEntries())
    {
        DPRINTF("%s: FillPscEntries failed.\n", FUNCNAME);
        return false;
    }

    AssignCharaCustomSs();

    if (existing_entries.size() == 0)
    {
        new_entry.char_id = cms_entry.id;
        new_entry.specs.clear();

        for (const PscSpecEntry &spec : psc_entries)
        {
            new_entry.specs.push_back(spec);
        }

        game_psc->AddEntry(new_entry);
    }
    else
    {
        for (PscEntry *entry : existing_entries)
        {
            entry->char_id = cms_entry.id;
            entry->specs.clear();

            for (const PscSpecEntry &spec : psc_entries)
            {
                entry->specs.push_back(spec);
            }
        }
    }

    return true;
}

void X2mFile::GetInstalledCustomAuraChara()
{
    temp_aur_in.clear();

    IniFile ini;

    if (!xv2fs->LoadFile(&ini, "data/chara/" + entry_name + "/" + X2M_CHARA_INI))
        return;

    ini.GetMultipleIntegersValues("General", "AUR", temp_aur_in, true);
}

bool X2mFile::InstallAuraExtraChara()
{
    UninstallAuraExtraChara();

    for (size_t i = 0; i < chara_auras_extra.size(); i++)
    {
        if (chara_auras[i].id != X2M_INVALID_ID)
        {
            chara_auras_extra[i].aur_id = chara_auras[i].id;
            game_prebaked->SetAuraExtra(chara_auras_extra[i]);
        }
    }

    return true;
}

bool X2mFile::UninstallAuraExtraChara()
{
    for (int32_t aur_id : temp_aur_in)
    {
        game_prebaked->RemoveAuraExtra(aur_id);
    }

    return true;
}

bool X2mFile::InstallCustomAuraChara()
{
    GetInstalledCustomAuraChara();
    temp_aur_out.clear();

    size_t num_custom = GetNumCharaAuras(true);

    if (temp_aur_in.size() == num_custom)
    {
        if (num_custom == 0)
            return InstallAuraExtraChara();

        // Case of "perfect update"
        std::vector<AurAura *> update_entries;

        for (int32_t aur_id : temp_aur_in)
        {
            AurAura *aura = game_aur->FindAuraByID((uint32_t)aur_id);
            if (!aura)
                break;

            update_entries.push_back(aura);
        }

        if (update_entries.size() != num_custom)
            goto new_install; // Some of the ids were not found, remove and do new install

        for (size_t i = 0, j = 0; i < aur_entries.size(); i++)
        {
            if (aur_entries[i].aura_id != X2M_INVALID_ID)
                continue;

            if (j >= update_entries.size())
            {
                DPRINTF("%s: Internal coding error (%Id >= %Id).\n", FUNCNAME, j, update_entries.size());
                return false;
            }

            AurAura *update_aura = update_entries[j];
            AurAura &aura = chara_auras[i];

            aura.id = update_aura->id;
            aur_entries[i].aura_id = aura.id;
            *update_aura = aura;

            temp_aur_out.push_back((int32_t)aura.id);
            j++;
        }

        goto setup_aura;
    }

new_install:

    // Case of new install or update where old and new have different amount of custom aura entries

    // Remove first existing entries
    for (int i = (int) temp_aur_in.size()-1; i >= 0; i--)
    {
        game_aur->RemoveAuraIfLast((uint32_t)temp_aur_in[i]);
    }

    if (!HasCharaAura(true))
        return InstallAuraExtraChara();

    for (size_t i = 0; i < chara_auras.size(); i++)
    {
        if (aur_entries[i].aura_id != X2M_INVALID_ID)
            continue;

        AurAura &aura = chara_auras[i];

        if (!game_aur->AddAura(aura))
            return false;

        aur_entries[i].aura_id = aura.id;
        temp_aur_out.push_back((int32_t)aura.id);
    }

setup_aura:

    // Nothing to do, setup happened before
    return InstallAuraExtraChara();
}

bool X2mFile::FillAurEntries()
{
    std::unordered_set<int32_t> costumes;
    size_t i = 0;

    for (const X2mSlotEntry &slot : slot_entries)
    {
        if (costumes.find(slot.costume_index) == costumes.end())
        {
            costumes.insert(slot.costume_index);

            if (i >= aur_entries.size())
                return false;

            aur_entries[i].char_id = cms_entry.id;
            aur_entries[i].costume_id = slot.costume_index;
            i++;
        }
    }

    return (costumes.size() == aur_entries.size());
}

bool X2mFile::InstallAur(bool update)
{
    if (cms_entry.id == X2M_DUMMY_ID || cms_entry.id == X2M_INVALID_ID)
    {
        DPRINTF("%s: Internal error. InstallCms must be called before this!!!\n", FUNCNAME);
        return false;
    }

    if (!InstallCustomAuraChara())
        return false;

    std::vector<AurCharaLink *> entries;
    game_aur->FindCharaLinks(cms_entry.id, entries);

    if (!HasAur())
    {
        if (entries.size() != 0)
        {
            DPRINTF("%s: Update mode failed because there were aur entry(es) in the aur, but 0 here. Uninstalling the mod and reinstall it will fix this.\n", FUNCNAME);
            return false;
        }

        return true; // Yes, true!
    }

    if (aur_entries.size() != GetNumCostumes())
    {
        DPRINTF("%s: Internal error.\n", FUNCNAME);
        return false;
    }

    if (update)
    {
        if (entries.size() == 0)
        {
            goto regular_install;
        }
        else if (entries.size() != aur_entries.size())
        {
            DPRINTF("%s: Update mode failed because there are different num of aur entries in the aur than in the mod (for this char). Try uninstalling the mod and install it again.\n", FUNCNAME);
            return false;
        }

        if (!FillAurEntries())
        {
            DPRINTF("%s: FillAurEntries failed.\n", FUNCNAME);
            return false;
        }

        for (size_t i = 0; i < aur_entries.size(); i++)
        {
            const AurCharaLink &entry = aur_entries[i];
            *(entries[i]) = entry;
        }

        return true;
    }

regular_install:

    if (!FillAurEntries())
    {
        DPRINTF("%s: FillAurEntries failed.\n", FUNCNAME);
        return false;
    }

    for (size_t i = 0; i < aur_entries.size(); i++)
    {
        const AurCharaLink &entry = aur_entries[i];

        if (!game_aur->AddCharaLink(entry, (i == 0)))
            return false;
    }

    return true;
}

bool X2mFile::InstallSevAudio()
{
    if (cms_entry.id == X2M_DUMMY_ID || cms_entry.id == X2M_INVALID_ID)
    {
        DPRINTF("%s: Internal error. InstallCms must be called before this!!!\n", FUNCNAME);
        return false;
    }

    UninstallSevAudio(); // For update, remove traces of previous

    std::vector<X2mCustomAudio> list;
    GetCustomSevAudio(list);

    if (list.size() == 0)
        return true; // Nothing to install, just report success

    std::map<uint32_t, uint32_t> map;
    uint32_t i = 0;

    for (const X2mCustomAudio &audio : list)
    {
        std::string name = "X2M_SEV_" + cms_entry.name + "_" + Utils::ToString(i);
        uint32_t cue_id, cue_id_en;
        HcaFile *hca;

        if (IsDummyMode())
        {
            hca = RecoverSevSound(name, false);
            if (!hca)
            {
                DPRINTF("%s: couldn't recover SEV japanese audio \"%s\"", FUNCNAME, name.c_str());
                return false;
            }
        }
        else
        {
            hca = LoadAudioFile(audio.name, false, true);
            if (!hca)
            {
                DPRINTF("%s: Failed to load japanese audio file \"%s\".\n", FUNCNAME, audio.name.c_str());
                return false;
            }
        }

        cue_id = Xenoverse2::SetSevCmnSound(name, false, *hca, true);
        delete hca;

        if (cue_id == (uint32_t)-1)
        {
            DPRINTF("%s: Failed to set sev sound for japanese.\n", FUNCNAME);
            return false;
        }

        if (IsDummyMode())
        {
            hca = RecoverSevSound(name, true);
            if (!hca)
            {
                DPRINTF("%s: couldn't recover SEV english audio \"%s\"", FUNCNAME, name.c_str());
                return false;
            }
        }
        else
        {
            hca = LoadAudioFile(audio.name, true, true);
            if (!hca)
            {
                if (!hca)
                {
                    DPRINTF("%s: Failed to load english audio file \"%s\".\n", FUNCNAME, audio.name.c_str());
                    return false;
                }
            }
        }

        cue_id_en = Xenoverse2::SetSevCmnSound(name, true, *hca, true);
        delete hca;

        if (cue_id_en == (uint32_t)-1)
        {
            DPRINTF("%s: Failed to set sev sound for english.\n", FUNCNAME);
            return false;
        }

        if (cue_id != cue_id_en)
        {
            DPRINTF("%s: There is a synchronization issue between japanese and english CAQD_ALL_VOX.\n", FUNCNAME);
            return false;
        }

        map[audio.id] = cue_id;
        i++;
    }

    for (SevEntryLL &entry : sev_ll_entries)
    {
        for (SevEventsLL &evll : entry.sub_entries)
        {
            for (SevEvent &ev : evll.events)
            {
                for (SevEventEntry &entry : ev.entries)
                {
                    if (entry.cue_id >= X2M_CUSTOM_AUDIO_BEGIN && entry.cue_id < X2M_CUSTOM_AUDIO_END)
                    {
                        X2mCustomAudio *audio = FindCustomAudio(entry.cue_id);

                        if (audio)
                        {
                            entry.cue_id = map[audio->id];
                            entry.file_id = 2;
                        }
                    }

                    if (entry.response_cue_id >= X2M_CUSTOM_AUDIO_BEGIN && entry.response_cue_id < X2M_CUSTOM_AUDIO_END)
                    {
                        X2mCustomAudio *audio = FindCustomAudio(entry.response_cue_id);

                        if (audio)
                        {
                            entry.response_cue_id = map[audio->id];
                            entry.response_file_id = 2;
                        }
                    }
                }
            }
        }
    }

    return true;
}

void X2mFile::RebuildSevHL(const std::vector<SevTableEntryHL> &hl_table)
{
    if (hl_table.size() == 0)
        return;    

    for (const SevTableEntryHL &hl_entry : hl_table)
    {
        CmsEntry *cms_entry = game_cms->FindEntryByName(hl_entry.char_id);
        if (!cms_entry)
            continue;

        game_sev->RemoveAllReferencesToChar(cms_entry->id);
    }

    std::vector<SevEntry> &sev_entries = game_sev->GetEntries();    

    for (size_t i = 0; i < sev_entries.size(); i++)
    {
        const SevEntry &sev_entry = sev_entries[i];
        CmsEntry *copy_cms_entry = game_cms->FindEntryByID(sev_entry.char_id);

        if (!copy_cms_entry)
            continue;

        for (const SevTableEntryHL &hl_entry : hl_table)
        {
            if (hl_entry.copy_char == copy_cms_entry->name && hl_entry.copy_costume == sev_entry.costume_id)
            {
                CmsEntry *cms_entry = game_cms->FindEntryByName(hl_entry.char_id);
                if (!cms_entry)
                    continue;

                SevEntry new_sev_entry = sev_entry;
                new_sev_entry.char_id = cms_entry->id;
                new_sev_entry.costume_id = hl_entry.costume_id;

                sev_entries.push_back(new_sev_entry);
            }
        }
    }

    for (size_t s = 0; s < sev_entries.size(); s++)
    {
        SevEntry sev_entry = sev_entries[s]; // Copy
        bool commit = false;

        for (size_t i = 0; i < sev_entry.chars_events.size(); i++)
        {
            const SevCharEvents cevents = sev_entry.chars_events[i]; // copy
            CmsEntry *copy_cms_entry = game_cms->FindEntryByID(cevents.char_id);

            if (!copy_cms_entry)
                continue;

            for (const SevTableEntryHL &hl_entry : hl_table)
            {
                if (hl_entry.copy_char == copy_cms_entry->name && hl_entry.copy_costume == cevents.costume_id)
                {
                    CmsEntry *cms_entry = game_cms->FindEntryByName(hl_entry.char_id);
                    if (!cms_entry)
                        continue;

                    if (cms_entry->id == copy_cms_entry->id)
                    {
                        //DPRINTF("Infinite case\n");
                        continue;
                    }

                    SevCharEvents new_cevents = cevents;
                    new_cevents.char_id = cms_entry->id;
                    new_cevents.costume_id = hl_entry.costume_id;

                    sev_entry.chars_events.push_back(new_cevents);
                    commit = true;
                }
            }
        }

        if (commit)
        {
            sev_entries[s] = sev_entry; // Commit
            //UPRINTF("Commit 0x%Ix\n", s);
        }
    }
}

void X2mFile::RebuildSevLL(const std::vector<SevTableEntryLL> &ll_table)
{
    if (ll_table.size() == 0)
        return;

    for (const SevTableEntryLL &ll_entry : ll_table)
    {
        CmsEntry *cms_entry = game_cms->FindEntryByName(ll_entry.char_id);
        if (!cms_entry)
            continue;

        game_sev->RemoveAllReferencesToChar(cms_entry->id);
    }

    std::vector<SevEntry> &sev_entries = game_sev->GetEntries();

    // Let's go first for the we are source ones.
    for (const SevTableEntryLL &ll_entry : ll_table)
    {
        CmsEntry *cms_entry = game_cms->FindEntryByName(ll_entry.char_id);
        if (!cms_entry)
            continue;

        SevEntry new_entry;

        new_entry.char_id = cms_entry->id;
        new_entry.costume_id = ll_entry.costume_id;

        for (const SevEventsLL &sub_entry : ll_entry.sub_entries)
        {
            if (!sub_entry.im_source)
                continue;

            CmsEntry *other_cms_entry = nullptr;

            if (sub_entry.char_code.length() != 0)
            {
                other_cms_entry = game_cms->FindEntryByName(sub_entry.char_code);
                if (!other_cms_entry)
                    continue;
            }

            SevCharEvents cevents;

            cevents.char_id = (other_cms_entry) ? other_cms_entry->id : 0xFFFFFFFF;
            cevents.costume_id = sub_entry.costume_id;
            cevents.events = sub_entry.events;

            new_entry.chars_events.push_back(cevents);
        }

        if (new_entry.chars_events.size() != 0)
        {
            sev_entries.push_back(new_entry);
        }
    }

    // Let's go now we the destinations
    for (const SevTableEntryLL &ll_entry : ll_table)
    {
        CmsEntry *cms_entry = game_cms->FindEntryByName(ll_entry.char_id);
        if (!cms_entry)
            continue;

        for (const SevEventsLL &sub_entry : ll_entry.sub_entries)
        {
            if (sub_entry.im_source)
                continue;

            CmsEntry *other_cms_entry = game_cms->FindEntryByName(sub_entry.char_code);
            if (!other_cms_entry)
                continue;

            SevEntry *other_entry = game_sev->FindEntry(other_cms_entry->id, sub_entry.costume_id);
            if (!other_entry)
                continue;

            SevCharEvents new_cevents;

            new_cevents.char_id = cms_entry->id;
            new_cevents.costume_id = ll_entry.costume_id;
            new_cevents.events = sub_entry.events;

            other_entry->chars_events.push_back(new_cevents);
        }
    }
}

void X2mFile::RebuildTtbHL(const std::vector<TtbTableEntryHL> &hl_table)
{
    if (hl_table.size() == 0)
        return;

    uint32_t current_event_id = X2M_TTB_ASSIGN_ID_START;

    for (const TtbTableEntryHL &hl_entry : hl_table)
    {
        CmsEntry *cms_entry = game_cms->FindEntryByName(hl_entry.char_id);
        if (!cms_entry)
            continue;

        game_ttb->RemoveAllReferencesToChar(cms_entry->id);
    }

    std::vector<TtbEntry> &ttb_entries = game_ttb->GetEntries();

    // First, the entries where we are source (actor 1)
    for (size_t i = 0; i < ttb_entries.size(); i++)
    {
        const TtbEntry &ttb_entry = ttb_entries[i];

        CmsEntry *copy_cms_entry = game_cms->FindEntryByID(ttb_entry.cms_id);

        if (!copy_cms_entry)
            continue;

        for (const TtbTableEntryHL &hl_entry :hl_table)
        {
            TtbEntry new_entry;

            for (const TtbEvent &event : ttb_entry.events)
            {
                if (hl_entry.copy_char == copy_cms_entry->name && hl_entry.copy_costume == event.costume)
                {
                    CmsEntry *cms_entry = game_cms->FindEntryByName(hl_entry.char_id);
                    if (!cms_entry)
                        continue;

                    new_entry.cms_id = cms_entry->id;

                    TtbEvent new_event = event;

                    new_event.id = current_event_id++;
                    new_event.costume = hl_entry.costume_id;
                    new_entry.events.push_back(new_event);
                }

            }

            if (new_entry.events.size() > 0)
            {
                TtbEntry *existing = game_ttb->FindEntry(new_entry.cms_id);

                if (existing)
                {
                    for (const TtbEvent &event : new_entry.events)
                    {
                        existing->events.push_back(event);
                    }
                }
                else
                {
                    ttb_entries.push_back(new_entry);
                }
            }
        }
    }

    // Now the entries where we are actor 2
    for (size_t i = 0; i < ttb_entries.size(); i++)
    {
        TtbEntry ttb_entry = ttb_entries[i]; // Copy
        bool commit = false;

        for (size_t j = 0; j < ttb_entry.events.size(); j++)
        {
            const TtbEvent event = ttb_entry.events[j]; // copy
            CmsEntry *copy_cms_entry = game_cms->FindEntryByID(event.cms2_id);

            if (!copy_cms_entry)
                continue;

            for (const TtbTableEntryHL &hl_entry : hl_table)
            {
                if (hl_entry.copy_char == copy_cms_entry->name && hl_entry.copy_costume == event.costume2)
                {
                    CmsEntry *cms_entry = game_cms->FindEntryByName(hl_entry.char_id);
                    if (!cms_entry)
                        continue;

                    if (cms_entry->id == copy_cms_entry->id)
                    {
                        //DPRINTF("Infinite case\n");
                        continue;
                    }

                    TtbEvent new_event = event;

                    new_event.id = current_event_id++;
                    new_event.cms2_id = cms_entry->id;
                    new_event.costume2 = hl_entry.costume_id;

                    ttb_entry.events.push_back(new_event);
                    commit = true;
                }
            }

        } // end for events

        if (commit)
        {
            ttb_entries[i] = ttb_entry; // Commit
        }
    }

    // Now the entries where we are actor 3
    for (size_t i = 0; i < ttb_entries.size(); i++)
    {
        TtbEntry ttb_entry = ttb_entries[i]; // Copy
        bool commit = false;

        for (size_t j = 0; j < ttb_entry.events.size(); j++)
        {
            const TtbEvent event = ttb_entry.events[j]; // copy

            if (event.cms3_id == 0xFFFFFFFF)
                continue;

            CmsEntry *copy_cms_entry = game_cms->FindEntryByID(event.cms3_id);

            if (!copy_cms_entry)
                continue;

            for (const TtbTableEntryHL &hl_entry : hl_table)
            {
                if (hl_entry.copy_char == copy_cms_entry->name && hl_entry.copy_costume == event.costume3)
                {
                    CmsEntry *cms_entry = game_cms->FindEntryByName(hl_entry.char_id);
                    if (!cms_entry)
                        continue;

                    if (cms_entry->id == copy_cms_entry->id)
                    {
                        //DPRINTF("Infinite case\n");
                        continue;
                    }

                    TtbEvent new_event = event;

                    new_event.id = current_event_id++;
                    new_event.cms3_id = cms_entry->id;
                    new_event.costume3 = hl_entry.costume_id;

                    ttb_entry.events.push_back(new_event);
                    commit = true;
                }
            }

        } // end for events

        if (commit)
        {
            ttb_entries[i] = ttb_entry; // Commit
        }
    }

    // Now the entries where we are actor 4
    for (size_t i = 0; i < ttb_entries.size(); i++)
    {
        TtbEntry ttb_entry = ttb_entries[i]; // Copy
        bool commit = false;

        for (size_t j = 0; j < ttb_entry.events.size(); j++)
        {
            const TtbEvent event = ttb_entry.events[j]; // copy

            if (event.cms4_id == 0xFFFFFFFF)
                continue;

            CmsEntry *copy_cms_entry = game_cms->FindEntryByID(event.cms4_id);

            if (!copy_cms_entry)
                continue;

            for (const TtbTableEntryHL &hl_entry : hl_table)
            {
                if (hl_entry.copy_char == copy_cms_entry->name && hl_entry.copy_costume == event.costume4)
                {
                    CmsEntry *cms_entry = game_cms->FindEntryByName(hl_entry.char_id);
                    if (!cms_entry)
                        continue;

                    if (cms_entry->id == copy_cms_entry->id)
                    {
                        //DPRINTF("Infinite case\n");
                        continue;
                    }

                    TtbEvent new_event = event;

                    new_event.id = current_event_id++;
                    new_event.cms4_id = cms_entry->id;
                    new_event.costume4 = hl_entry.costume_id;

                    ttb_entry.events.push_back(new_event);
                    commit = true;
                }
            }

        } // end for events

        if (commit)
        {
            ttb_entries[i] = ttb_entry; // Commit
        }
    }

    // Now the entries where we are actor 5
    for (size_t i = 0; i < ttb_entries.size(); i++)
    {
        TtbEntry ttb_entry = ttb_entries[i]; // Copy
        bool commit = false;

        for (size_t j = 0; j < ttb_entry.events.size(); j++)
        {
            const TtbEvent event = ttb_entry.events[j]; // copy

            if (event.cms5_id == 0xFFFFFFFF)
                continue;

            CmsEntry *copy_cms_entry = game_cms->FindEntryByID(event.cms5_id);

            if (!copy_cms_entry)
                continue;

            for (const TtbTableEntryHL &hl_entry : hl_table)
            {
                if (hl_entry.copy_char == copy_cms_entry->name && hl_entry.copy_costume == event.costume5)
                {
                    CmsEntry *cms_entry = game_cms->FindEntryByName(hl_entry.char_id);
                    if (!cms_entry)
                        continue;

                    if (cms_entry->id == copy_cms_entry->id)
                    {
                        //DPRINTF("Infinite case\n");
                        continue;
                    }

                    TtbEvent new_event = event;

                    new_event.id = current_event_id++;
                    new_event.cms5_id = cms_entry->id;
                    new_event.costume5 = hl_entry.costume_id;

                    ttb_entry.events.push_back(new_event);
                    commit = true;
                }
            }

        } // end for events

        if (commit)
        {
            ttb_entries[i] = ttb_entry; // Commit
        }
    }

    game_ttb->FixEventsID(X2M_TTB_ASSIGN_ID_START);
}

bool X2mFile::TtbEntryLLToTtbEvent(const TtbEntryLL &entry_ll, TtbEvent &event, std::string char2, std::string char3, std::string char4, std::string char5)
{
    event.costume = entry_ll.costume;
    event.transformation = entry_ll.transformation;
    event.voice_name = entry_ll.event_res1.name;

    if (char2.length() == 0)
        char2 = entry_ll.cms2_code;

    CmsEntry *cms_entry = game_cms->FindEntryByName(char2);
    if (!cms_entry)
        return false;

    event.cms2_id = cms_entry->id;
    event.costume2 = entry_ll.costume2;
    event.transformation2 = entry_ll.transformation2;
    event.voice2_name = entry_ll.event_res2.name;

    event.type = entry_ll.type;
    event.unk_44 = entry_ll.unk_44;
    event.unk_48 = entry_ll.unk_48;
    event.reverse_order = entry_ll.reverse_order;
    event.dummy_order1 = entry_ll.dummy_order1;
    event.dummy_order2 = entry_ll.dummy_order2;

    event.cms3_id = event.cms4_id = event.cms5_id = 0xFFFFFFFF;
    event.costume3 = event.costume4 = event.costume5 = 0xFFFFFFFF;
    event.transformation3 = event.transformation4 = event.transformation5 = 0xFFFFFFFF;

    if (entry_ll.num_actors <= 2)
        return true;

    event.reverse_order = false;

    if (char3.length() == 0)
        char3 = entry_ll.cms3_code;

    cms_entry = game_cms->FindEntryByName(char3);
    if (!cms_entry)
        return false;

    event.cms3_id = cms_entry->id;
    event.costume3 = entry_ll.costume3;
    event.transformation3 = entry_ll.transformation3;
    event.voice3_name = entry_ll.event_res3.name;

    if (entry_ll.num_actors <= 3)
        return true;

    if (char4.length() == 0)
        char4 = entry_ll.cms4_code;

    cms_entry = game_cms->FindEntryByName(char4);
    if (!cms_entry)
        return false;

    event.cms4_id = cms_entry->id;
    event.costume4 = entry_ll.costume4;
    event.transformation4 = entry_ll.transformation4;
    event.voice4_name = entry_ll.event_res4.name;

    if (entry_ll.num_actors <= 4)
        return true;

    if (char5.length() == 0)
        char5 = entry_ll.cms5_code;

    cms_entry = game_cms->FindEntryByName(char5);
    if (!cms_entry)
        return false;

    event.cms5_id = cms_entry->id;
    event.costume5 = entry_ll.costume5;
    event.transformation5 = entry_ll.transformation5;
    event.voice5_name = entry_ll.event_res5.name;

    return true;
}

void X2mFile::RebuildTtbLL(const std::vector<TtbTableEntryLL> &ll_table)
{
    if (ll_table.size() == 0)
        return;

    uint32_t current_event_id = X2M_TTB_ASSIGN_ID_START;

    for (const TtbTableEntryLL &ll_entry : ll_table)
    {
        CmsEntry *cms_entry = game_cms->FindEntryByName(ll_entry.char_id);
        if (!cms_entry)
            continue;

        game_ttb->RemoveAllReferencesToChar(cms_entry->id);
    }

    std::vector<TtbEntry> &ttb_entries = game_ttb->GetEntries();

    // Let's go first for the we are source ones (actor 1)
    std::unordered_map<std::string, TtbEntry> new_entries;

    for (const TtbTableEntryLL &ll_entry : ll_table)
    {
        if (ll_entry.entry.cms_code.length() > 0)
            continue;

        CmsEntry *cms_entry = game_cms->FindEntryByName(ll_entry.char_id);
        if (!cms_entry)
            continue;

        TtbEntry new_entry;
        auto it = new_entries.find(ll_entry.char_id);

        if (it != new_entries.end())
        {
            new_entry = it->second;
        }
        else
        {
            new_entry.cms_id = cms_entry->id;
        }

        TtbEvent new_event;
        new_event.id = current_event_id++;

        if (!TtbEntryLLToTtbEvent(ll_entry.entry, new_event))
            continue;

        new_entry.events.push_back(new_event);
        new_entries[ll_entry.char_id] = new_entry;
    }

    // Commit the new entries
    for (auto &it : new_entries)
    {
        ttb_entries.push_back(it.second);
    }

    // Now add events where we are actor 2
    for (const TtbTableEntryLL &ll_entry : ll_table)
    {
        if (ll_entry.entry.cms2_code.length() > 0)
            continue;

        CmsEntry *cms_entry = game_cms->FindEntryByName(ll_entry.char_id);
        if (!cms_entry)
            continue;

        CmsEntry *other_cms_entry = game_cms->FindEntryByName(ll_entry.entry.cms_code);
        if (!other_cms_entry)
            continue;

        TtbEntry *other_entry = game_ttb->FindEntry(other_cms_entry->id);
        if (!other_entry)
            continue;

        TtbEvent new_event;
        new_event.id = current_event_id++;

        if (!TtbEntryLLToTtbEvent(ll_entry.entry, new_event, ll_entry.char_id))
            continue;

        other_entry->AddUniqueEvent(new_event); // New event commited
    }

    // Now add events where we are actor 3
    for (const TtbTableEntryLL &ll_entry : ll_table)
    {
        if (ll_entry.entry.cms3_code.length() > 0 || ll_entry.entry.num_actors < 3)
            continue;

        CmsEntry *cms_entry = game_cms->FindEntryByName(ll_entry.char_id);
        if (!cms_entry)
            continue;

        CmsEntry *other_cms_entry = game_cms->FindEntryByName(ll_entry.entry.cms_code);
        if (!other_cms_entry)
            continue;

        TtbEntry *other_entry = game_ttb->FindEntry(other_cms_entry->id);
        if (!other_entry)
            continue;

        TtbEvent new_event;
        new_event.id = current_event_id++;

        if (!TtbEntryLLToTtbEvent(ll_entry.entry, new_event, "", ll_entry.char_id))
            continue;

        other_entry->AddUniqueEvent(new_event); // New event commited
    }

    // Now add events where we are actor 4
    for (const TtbTableEntryLL &ll_entry : ll_table)
    {
        if (ll_entry.entry.cms4_code.length() > 0 || ll_entry.entry.num_actors < 4)
            continue;

        CmsEntry *cms_entry = game_cms->FindEntryByName(ll_entry.char_id);
        if (!cms_entry)
            continue;

        CmsEntry *other_cms_entry = game_cms->FindEntryByName(ll_entry.entry.cms_code);
        if (!other_cms_entry)
            continue;

        TtbEntry *other_entry = game_ttb->FindEntry(other_cms_entry->id);
        if (!other_entry)
            continue;

        TtbEvent new_event;
        new_event.id = current_event_id++;

        if (!TtbEntryLLToTtbEvent(ll_entry.entry, new_event, "", "", ll_entry.char_id))
            continue;

        other_entry->AddUniqueEvent(new_event); // New event commited
    }

    // Now add events where we are actor 5
    for (const TtbTableEntryLL &ll_entry : ll_table)
    {
        if (ll_entry.entry.cms5_code.length() > 0 || ll_entry.entry.num_actors < 5)
            continue;

        CmsEntry *cms_entry = game_cms->FindEntryByName(ll_entry.char_id);
        if (!cms_entry)
            continue;

        CmsEntry *other_cms_entry = game_cms->FindEntryByName(ll_entry.entry.cms_code);
        if (!other_cms_entry)
            continue;

        TtbEntry *other_entry = game_ttb->FindEntry(other_cms_entry->id);
        if (!other_entry)
            continue;

        TtbEvent new_event;
        new_event.id = current_event_id++;

        if (!TtbEntryLLToTtbEvent(ll_entry.entry, new_event, "", "", "", ll_entry.char_id))
            continue;

        other_entry->AddUniqueEvent(new_event); // New event commited
    }

    game_ttb->FixEventsID(X2M_TTB_ASSIGN_ID_START);
}

bool X2mFile::InstallSev(std::vector<SevTableEntryHL> &hl_table, std::vector<SevTableEntryLL> &ll_table)
{
    if (cms_entry.id == X2M_DUMMY_ID || cms_entry.id == X2M_INVALID_ID)
    {
        DPRINTF("%s: Internal error. InstallCms must be called before this!!!\n", FUNCNAME);
        return false;
    }

    // Remove ourselves from tables if found (for updating, since costumes or copy data may not match)
    for (size_t i = 0; i < hl_table.size(); i++)
    {
        const SevTableEntryHL &entry_hl = hl_table[i];

        if (entry_hl.char_id == entry_name)
        {
            hl_table.erase(hl_table.begin()+i);
            i--;
        }
    }    

    for (size_t i = 0; i < ll_table.size(); i++)
    {
        const SevTableEntryLL &entry_ll = ll_table[i];

        if (entry_ll.char_id == entry_name)
        {
            ll_table.erase(ll_table.begin()+i);
            i--;
        }
    }

    if (HasSevHL())
    {
        for (const SevEntryHL &entry : sev_hl_entries)
        {
            SevTableEntryHL new_hl_entry;

            new_hl_entry.char_id = entry_name;
            new_hl_entry.costume_id = entry.costume_id;
            new_hl_entry.copy_char = entry.copy_char;
            new_hl_entry.copy_costume = entry.copy_costume;

            hl_table.push_back(new_hl_entry);
        }
    }
    else if (HasSevLL())
    {
        for (const SevEntryLL &entry : sev_ll_entries)
        {
            SevTableEntryLL new_ll_entry;

            new_ll_entry.char_id = entry_name;
            new_ll_entry.costume_id = entry.costume_id;
            new_ll_entry.sub_entries = entry.sub_entries;

            ll_table.push_back(new_ll_entry);
        }
    }

    game_sev->RemoveAllReferencesToChar(cms_entry.id); // Needed in case update doesn't have sev

    // Tables must always rebuild, regardless of if the mod has sev or not
    RebuildSevLL(ll_table); // LL table must go first
    RebuildSevHL(hl_table);

    return true;
}

bool X2mFile::FillCmlEntries()
{
    std::unordered_set<int32_t> costumes;
    size_t i = 0;

    for (const X2mSlotEntry &slot : slot_entries)
    {
        if (costumes.find(slot.costume_index) == costumes.end())
        {
            costumes.insert(slot.costume_index);

            if (i >= cml_entries.size())
                return false;

            cml_entries[i].char_id = cms_entry.id;
            cml_entries[i].costume_id = slot.costume_index;
            i++;
        }
    }

    return (costumes.size() == cml_entries.size());
}

bool X2mFile::InstallCml(bool update)
{
    if (cms_entry.id == X2M_DUMMY_ID || cms_entry.id == X2M_INVALID_ID)
    {
        DPRINTF("%s: Internal error. InstallCms must be called before this!!!\n", FUNCNAME);
        return false;
    }

    std::vector<CmlEntry *> entries;
    game_cml->FindEntriesByCharID(cms_entry.id, entries);

    if (!HasCml())
    {
        if (entries.size() != 0)
        {
            DPRINTF("%s: Update mode failed because there were cml entry(es) in the cml, but 0 here. Uninstalling the mod and reinstall it will fix this.\n", FUNCNAME);
            return false;
        }

        return true; // Yes, true!
    }

    if (cml_entries.size() != GetNumCostumes())
    {
        DPRINTF("%s: Internal error.\n", FUNCNAME);
        return false;
    }

    if (update)
    {
        if (entries.size() == 0)
        {
            goto regular_install;
        }
        else if (entries.size() != cml_entries.size())
        {
            DPRINTF("%s: Update mode failed because there are different num of cml entries in the cml than in the mod (for this char). Try uninstalling the mod and install it again.\n", FUNCNAME);
            return false;
        }

        if (!FillCmlEntries())
        {
            DPRINTF("%s: FillCmlEntries failed.\n", FUNCNAME);
            return false;
        }

        for (size_t i = 0; i < cml_entries.size(); i++)
        {
            const CmlEntry &entry = cml_entries[i];
            *(entries[i]) = entry;
        }

        return true;
    }

regular_install:

    if (!FillCmlEntries())
    {
        DPRINTF("%s: FillCmlEntries failed.\n", FUNCNAME);
        return false;
    }

    for (size_t i = 0; i < cml_entries.size(); i++)
    {
        const CmlEntry &entry = cml_entries[i];

        if (!game_cml->AddEntry(entry, (i == 0)))
            return false;
    }

    return true;
}

bool X2mFile::InstallHci()
{
    if (cms_entry.id == X2M_DUMMY_ID || cms_entry.id == X2M_INVALID_ID)
    {
        DPRINTF("%s: Internal error. InstallCms must be called before this!!!\n", FUNCNAME);
        return false;
    }

    game_hci->RemoveEntries(cms_entry.id); // Remove from previous version (if any)

    if (!HasHci())
        return true; // Yes, true

    for (HciEntry &entry : hci_entries)
    {
        entry.char_id = cms_entry.id;

        if (!game_hci->AddEntry(entry, false))
            return false;
    }

    return true;
}

bool X2mFile::InstallTtbAudio()
{
    if (cms_entry.id == X2M_DUMMY_ID || cms_entry.id == X2M_INVALID_ID)
    {
        DPRINTF("%s: Internal error. InstallCms must be called before this!!!\n", FUNCNAME);
        return false;
    }

    UninstallTtbAudio(); // For update, remove traces of previous

    std::vector<X2mCustomAudio> list;
    GetCustomTtbAudio(list);

    if (list.size() == 0)
        return true; // Nothing to install, just report success

    std::unordered_map<std::string, std::string> map; // From internal audio name to cue name
    uint32_t i = 0;

    for (const X2mCustomAudio &audio : list)
    {
        std::string cue_name = "X2M_TTB_" + cms_entry.name + "_" + Utils::ToString(i);
        uint32_t cue_id, cue_id_en;
        HcaFile *hca;

        if (IsDummyMode())
        {
            hca = RecoverSevSound(cue_name, false);
            if (!hca)
            {
                DPRINTF("%s: couldn't recover TTB japanese audio \"%s\"", FUNCNAME, cue_name.c_str());
                return false;
            }
        }
        else
        {
            hca = LoadAudioFile(audio.name, false, true);
            if (!hca)
            {
                DPRINTF("%s: Failed to load japanese audio file \"%s\".\n", FUNCNAME, audio.name.c_str());
                return false;
            }
        }

        cue_id = Xenoverse2::SetSevCmnSound(cue_name, false, *hca, true);
        delete hca;

        if (cue_id == (uint32_t)-1)
        {
            DPRINTF("%s: Failed to set sev sound for japanese.\n", FUNCNAME);
            return false;
        }

        if (IsDummyMode())
        {
            hca = RecoverSevSound(cue_name, true);
            if (!hca)
            {
                DPRINTF("%s: couldn't recover TTB english audio \"%s\"", FUNCNAME, cue_name.c_str());
                return false;
            }
        }
        else
        {
            hca = LoadAudioFile(audio.name, true, true);
            if (!hca)
            {
                if (!hca)
                {
                    DPRINTF("%s: Failed to load english audio file \"%s\".\n", FUNCNAME, audio.name.c_str());
                    return false;
                }
            }
        }

        cue_id_en = Xenoverse2::SetSevCmnSound(cue_name, true, *hca, true);
        delete hca;

        if (cue_id_en == (uint32_t)-1)
        {
            DPRINTF("%s: Failed to set common dialogue sound for english.\n", FUNCNAME);
            return false;
        }

        if (cue_id != cue_id_en)
        {
            DPRINTF("%s: There is a synchronization issue between japanese and english CAQD_ADD_VOX.\n", FUNCNAME);
            return false;
        }

        map[audio.name] = cue_name;
        i++;
    }

    for (TtbEntryLL &entry : ttb_ll_entries)
    {
        if (Utils::BeginsWith(entry.event_res1.name, "X2T_"))
        {
            const std::string audio_name = entry.event_res1.name.substr(4);
            X2mCustomAudio *audio = FindCustomAudio(audio_name);

            if (audio)
            {
                entry.event_res1.name = map[audio->name];
            }
        }

        if (Utils::BeginsWith(entry.event_res2.name, "X2T_"))
        {
            const std::string audio_name = entry.event_res2.name.substr(4);
            X2mCustomAudio *audio = FindCustomAudio(audio_name);

            if (audio)
            {
                entry.event_res2.name = map[audio->name];
            }
        }

        if (entry.num_actors <= 2)
            continue;

        if (Utils::BeginsWith(entry.event_res3.name, "X2T_"))
        {
            const std::string audio_name = entry.event_res3.name.substr(4);
            X2mCustomAudio *audio = FindCustomAudio(audio_name);

            if (audio)
            {
                entry.event_res3.name = map[audio->name];
            }
        }

        if (entry.num_actors <= 3)
            continue;

        if (Utils::BeginsWith(entry.event_res4.name, "X2T_"))
        {
            const std::string audio_name = entry.event_res4.name.substr(4);
            X2mCustomAudio *audio = FindCustomAudio(audio_name);

            if (audio)
            {
                entry.event_res4.name = map[audio->name];
            }
        }

        if (entry.num_actors <= 4)
            continue;

        if (Utils::BeginsWith(entry.event_res5.name, "X2T_"))
        {
            const std::string audio_name = entry.event_res5.name.substr(4);
            X2mCustomAudio *audio = FindCustomAudio(audio_name);

            if (audio)
            {
                entry.event_res5.name = map[audio->name];
            }
        }
    }

    return true;
}

bool X2mFile::InstallTtbSubtitle(const TtbEventResLL &res)
{
    const std::string prefix = "X2M_TTB_" + cms_entry.name + "_";

    if (!Utils::BeginsWith(res.name, prefix))
        return true;

    int ls = 0, le = XV2_LANG_NUM;
    if (global_lang >= 0)
    {
        ls = global_lang;
        le = global_lang+1;
    }

    for (int lang = ls; lang < le; lang++)
    {
        const std::string *sub;

        if (lang == XV2_LANG_ENGLISH)
        {
            if (res.subtitle[lang].length() == 0)
            {
                DPRINTF("%s: English subtitle missing, (event %s)\n", FUNCNAME, res.name.c_str());
                return false;
            }

            sub = &res.subtitle[lang];
        }
        else
        {
            if (res.subtitle[lang].length() == 0)
            {
                sub = &res.subtitle[XV2_LANG_ENGLISH];
            }
            else
            {
                sub = &res.subtitle[lang];
            }
        }

        if (!Xenoverse2::SetTtbSubtitle(res.name, *sub, lang))
        {
            DPRINTF("%s: SetTtbSubtitle failed.\n", FUNCNAME);
            return false;
        }
    }

    if (!Xenoverse2::SetTtbVoice(res.name, "")) // Empty string = entry name matches cue name
    {
        DPRINTF("%s: SetTtbVoice failed.\n", FUNCNAME);
        return false;
    }

    return true;
}

bool X2mFile::InstallTtbSubtitles()
{
    if (cms_entry.id == X2M_DUMMY_ID || cms_entry.id == X2M_INVALID_ID)
    {
        DPRINTF("%s: Internal error. InstallCms must be called before this!!!\n", FUNCNAME);
        return false;
    }

    UninstallTtbSubtitles(); // For update, remove traces of previous

    for (size_t i = 0; i < ttb_ll_entries.size(); i++)
    {
        const TtbEntryLL &entry = ttb_ll_entries[i];

        if (!InstallTtbSubtitle(entry.event_res1))
            return false;

        if (!InstallTtbSubtitle(entry.event_res2))
            return false;

        if (entry.num_actors <= 2)
            continue;

        if (!InstallTtbSubtitle(entry.event_res3))
            return false;

        if (entry.num_actors <= 3)
            continue;

        if (!InstallTtbSubtitle(entry.event_res4))
            return false;

        if (entry.num_actors <= 4)
            continue;

        if (!InstallTtbSubtitle(entry.event_res5))
            return false;
    }

    return true;
}

bool X2mFile::InstallTtb(std::vector<TtbTableEntryHL> &hl_table, std::vector<TtbTableEntryLL> &ll_table)
{
    if (cms_entry.id == X2M_DUMMY_ID || cms_entry.id == X2M_INVALID_ID)
    {
        DPRINTF("%s: Internal error. InstallCms must be called before this!!!\n", FUNCNAME);
        return false;
    }

    if (!game_ttb)
    {
        if (!Xenoverse2::InitCommonDialogue())
        {
            DPRINTF("%s: InitCommonDialogue failed.\n", FUNCNAME);
            return false;
        }
    }

    // Remove ourselves from tables if found (for updating, since costumes or copy data may not match)
    for (size_t i = 0; i < hl_table.size(); i++)
    {
        const TtbTableEntryHL &entry_hl = hl_table[i];

        if (entry_hl.char_id == entry_name)
        {
            hl_table.erase(hl_table.begin()+i);
            i--;
        }
    }

    for (size_t i = 0; i < ll_table.size(); i++)
    {
        const TtbTableEntryLL &entry_ll = ll_table[i];

        if (entry_ll.char_id == entry_name)
        {
            ll_table.erase(ll_table.begin()+i);
            i--;
        }
    }

    if (HasTtbHL())
    {
        for (const TtbEntryHL &entry : ttb_hl_entries)
        {
            TtbTableEntryHL new_hl_entry;

            new_hl_entry.char_id = entry_name;
            new_hl_entry.costume_id = entry.costume_id;
            new_hl_entry.copy_char = entry.copy_char;
            new_hl_entry.copy_costume = entry.copy_costume;

            hl_table.push_back(new_hl_entry);
        }
    }
    else if (HasTtbLL())
    {
        for (const TtbEntryLL &entry : ttb_ll_entries)
        {
            TtbTableEntryLL new_ll_entry;

            new_ll_entry.char_id = entry_name;
            new_ll_entry.entry = entry;

            ll_table.push_back(new_ll_entry);
        }
    }

    game_ttb->RemoveAllReferencesToChar(cms_entry.id); // Needed in case update doesn't have ttb

    // Tables must always rebuild, regardless of if the mod has sev or not
    RebuildTtbLL(ll_table); // LL table must go first
    RebuildTtbHL(hl_table);

    return true;
}

bool X2mFile::InstallTtc()
{
    if (cms_entry.id == X2M_DUMMY_ID || cms_entry.id == X2M_INVALID_ID)
    {
        DPRINTF("%s: Internal error. InstallCms must be called before this!!!\n", FUNCNAME);
        return false;
    }

    if (!game_ttc)
    {
        if (!Xenoverse2::InitCommonDialogue())
        {
            DPRINTF("%s: InitCommonDialogue failed.\n", FUNCNAME);
            return false;
        }
    }

    if (!HasTtc())
    {
        // In case an update doesn't have ttc but previous version had (unlikely, but can happen), remove the ttc data
        game_ttc->RemoveChar(cms_entry.id);
        return true;
    }

    TtcEntry new_entry;

    new_entry.cms_id = cms_entry.id;
    new_entry.lists.resize(TTC_BIGGER_EVENT_P1);

    for (size_t i = 0; i < new_entry.lists.size(); i++)
    {
        new_entry.lists[i].type = (uint32_t)i;
    }

    if (HasTtcHL())
    {
        for (size_t i = 0; i < ttc_hl_entries.size(); i++)
        {
            const TtcEntryHL &entry_hl = ttc_hl_entries[i];
            CmsEntry *cms = game_cms->FindEntryByName(entry_hl.copy_char);

            if (!cms)
                continue;

            TtcEntry *copy_entry = game_ttc->FindEntry(cms->id);
            if (!copy_entry)
                continue;

            for (const TtcEventList &copy_list : copy_entry->lists)
            {
                if (copy_list.type >= new_entry.lists.size())
                    continue;

                for (const TtcEvent &copy_event : copy_list.events)
                {
                    if (copy_event.costume == entry_hl.copy_costume)
                    {
                        TtcEvent event = copy_event;
                        event.costume = entry_hl.costume_id;

                        new_entry.lists[copy_list.type].events.push_back(event);
                    }
                }
            }
        }
    }
    else if (HasTtcLL())
    {
        for (size_t i = 0; i < ttc_ll_entries.size(); i++)
        {
            const TtcEntryLL &entry_ll = ttc_ll_entries[i];
            TtcEvent event;

            event.name = entry_ll.name;
            event.costume = entry_ll.costume;
            event.transformation = entry_ll.transformation;
            event.condition = entry_ll.condition;

            if (entry_ll.type < new_entry.lists.size())
            {
                new_entry.lists[entry_ll.type].events.push_back(event);
            }
        }
    }

    for (size_t i = 0; i < new_entry.lists.size(); i++)
    {
        if (new_entry.lists[i].events.size() == 0)
        {
            new_entry.lists.erase(new_entry.lists.begin()+i);
            i--;
        }
    }

    if (new_entry.lists.size() > 0)
    {
        game_ttc->AddEntry(new_entry);
    }
    else
    {
        game_ttc->RemoveChar(cms_entry.id);
    }

    return true;
}

bool X2mFile::InstallCnc()
{
    if (cms_entry.id == X2M_DUMMY_ID || cms_entry.id == X2M_INVALID_ID)
    {
        DPRINTF("%s: Internal error. InstallCms must be called before this!!!\n", FUNCNAME);
        return false;
    }

    if (!game_cnc)
    {
        if (!Xenoverse2::InitDualSkill(true, false))
        {
            DPRINTF("%s: InitDualSkill failed.\n", FUNCNAME);
            return false;
        }
    }

    if (!HasCnc())
    {
        game_cnc->RemoveEntries(cms_entry.id);
        return true;
    }

    std::vector<CncEntry *> existing;
    if (game_cnc->FindEntriesByCharID(cms_entry.id, existing) == cnc_entries.size())
    {
        // Case of perfect update
        for (size_t i = 0; i < cnc_entries.size(); i++)
        {
            cnc_entries[i].cms_id = cms_entry.id;
            *(existing[i]) = cnc_entries[i];
        }
    }
    else
    {
        // New install or imperfect update
        game_cnc->RemoveEntries(cms_entry.id);

        for (CncEntry &entry : cnc_entries)
        {
            entry.cms_id = cms_entry.id;
            game_cnc->AddEntry(entry);
        }
    }

    return true;
}

bool X2mFile::InstallCharVfx()
{
    if (cms_entry.id == X2M_DUMMY_ID || cms_entry.id == X2M_INVALID_ID)
    {
        DPRINTF("%s: Internal error. InstallCms must be called before this!!!\n", FUNCNAME);
        return false;
    }

    if (!game_ers)
    {
        if (!Xenoverse2::InitVfx())
            return false;
    }

    if (!game_ers->SetCharEepk(cms_entry.id, char_eepk))
        return false;

    return true;
}

bool X2mFile::FillIkdEntries()
{
    std::unordered_set<int32_t> costumes;
    size_t i = 0;
    size_t count = GetNumIkdEntries(false);

    for (const X2mSlotEntry &slot : slot_entries)
    {
        if (costumes.find(slot.costume_index) == costumes.end())
        {
            costumes.insert(slot.costume_index);

            if (i >= count)
                return false;

            ikd_entries[i].cms_id = cms_entry.id;
            ikd_entries[i].costume_id = slot.costume_index;
            i++;
        }
    }

    if (!HasIkdLobby())
        return (costumes.size() == count);

    costumes.clear();
    for (const X2mSlotEntry &slot : slot_entries)
    {
        if (costumes.find(slot.costume_index) == costumes.end())
        {
            costumes.insert(slot.costume_index);

            if (i >= ikd_entries.size())
                return false;

            ikd_entries[i].cms_id = cms_entry.id;
            ikd_entries[i].costume_id = slot.costume_index;
            i++;
        }
    }

    return (costumes.size() == count);
}

bool X2mFile::InstallIkd()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (cms_entry.id == X2M_DUMMY_ID || cms_entry.id == X2M_INVALID_ID)
    {
        DPRINTF("%s: Internal error. InstallCms must be called before this!!!\n", FUNCNAME);
        return false;
    }

    if (!HasIkd())
    {
        game_ikd_battle->RemoveAllReferencesToChar(cms_entry.id);
        game_ikd_lobby->RemoveAllReferencesToChar(cms_entry.id);
        return true;
    }

    if (!FillIkdEntries())
    {
        DPRINTF("%s: Fill ikd entries failed.\n", FUNCNAME);
        return false;
    }

    size_t count = GetNumIkdEntries(false);
    std::vector<IkdEntry *> existing;

    if (game_ikd_battle->FindEntries(cms_entry.id, existing) == count)
    {
        for (size_t i = 0; i < count; i++)
        {
            *(existing)[i] = ikd_entries[i];
        }
    }
    else
    {
        game_ikd_battle->RemoveAllReferencesToChar(cms_entry.id);

        for (size_t i = 0; i < count; i++)
        {
            game_ikd_battle->AddEntry(ikd_entries[i]);
        }
    }

    if (!HasIkdLobby())
    {
        game_ikd_lobby->RemoveAllReferencesToChar(cms_entry.id);
        return true;
    }

    if (game_ikd_lobby->FindEntries(cms_entry.id, existing) == count)
    {
        for (size_t i = 0; i < count; i++)
        {
            *(existing)[i] = ikd_entries[count+i];
        }
    }
    else
    {
        game_ikd_lobby->RemoveAllReferencesToChar(cms_entry.id);

        for (size_t i = 0; i < count; i++)
        {
            game_ikd_lobby->AddEntry(ikd_entries[count+i]);
        }
    }

    return true;
}

bool X2mFile::InstallVlc()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (cms_entry.id == X2M_DUMMY_ID || cms_entry.id == X2M_INVALID_ID)
    {
        DPRINTF("%s: Internal error. InstallCms must be called before this!!!\n", FUNCNAME);
        return false;
    }

    if (!HasVlc())
    {
        game_vlc->RemoveEntry(cms_entry.id);
        return true;
    }

    vlc_entry.cms_id = cms_entry.id;
    game_vlc->SetEntry(vlc_entry);
    return true;
}

bool X2mFile::InstallSelPortrait()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (!SelPortraitExists())
        return false;

    size_t size;
    uint8_t *buf;

    if (IsDummyMode())
    {
        buf = RecoverSelPortrait(&size);
        if (!buf)
        {
            DPRINTF("%s: Failed to recover portrait.", FUNCNAME);
            return false;
        }
    }
    else
    {
        buf = ReadFile(X2M_SEL_PORTRAIT, &size);
        if (!buf)
            return false;
    }

    std::string name = entry_name + "_000.dds";
    bool ret = Xenoverse2::SetSelPortrait(name, buf, size);

    delete[] buf;
    return ret;
}

void X2mFile::InstallColors()
{
    game_prebaked->RemoveColorsMap(cms_entry.id);

    if (!UsesBodyShape())
        return;

    BcsFile bcs;
    std::string bcs_path = entry_name + "/" + entry_name + ".bcs";

    size_t size;
    uint8_t *buf;

    if (IsDummyMode())
    {
        buf = RecoverCharaFile(entry_name + ".bcs", &size, false);
        if (!buf)
            return;
    }
    else
    {
        buf = this->ReadFile(bcs_path, &size);
        if (!buf)
            return;
    }

    bool ret = bcs.Load(buf, size);
    delete[] buf;

    if (!ret)
        return;

    const std::vector<BcsPartSet> &part_sets = bcs.GetPartSets();
    const std::vector<BcsPartColors> &part_colors = bcs.GetPartsColors();

    for (size_t i = 0; i < slot_entries.size(); i++)
    {
        const X2mSlotEntry &slot_entry = slot_entries[i];
        BcsColorsMap map;

        if (slot_entry.costume_index < 0 || slot_entry.costume_index >= (int32_t)part_sets.size())
            continue;

        const BcsPartSet &set = part_sets[slot_entry.costume_index];
        if (!set.valid)
            continue;

        for (size_t j = 0; j < set.parts.size(); j++)
        {
            const BcsPart &part = set.parts[j];

            if (!part.valid)
                continue;

            for (size_t k = 0; k < part.color_selectors.size(); k++)
            {
                const BcsColorSelector &sel = part.color_selectors[k];

                if (sel.part_colors >= part_colors.size())
                    continue;

                const BcsPartColors &cols = part_colors[sel.part_colors];
                if (!cols.valid)
                    continue;

                map.map[cols.name] = sel.color;
            }
        }

        if (map.map.size() > 0)
            game_prebaked->AddColorsMap(cms_entry.id, slot_entry.costume_index, map);
    }
}

bool X2mFile::InstallPreBaked()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (cms_entry.id == X2M_DUMMY_ID || cms_entry.id == X2M_INVALID_ID)
    {
        DPRINTF("%s: Internal error. InstallCms must be called before this!!!\n", FUNCNAME);
        return false;
    }

    if (is_ozaru)
    {
        game_prebaked->AddOzaru(entry_name);
    }
    else
    {
        game_prebaked->RemoveOzaru(entry_name);
    }

    if (is_cellmax)
    {
        game_prebaked->AddCellMax(entry_name);
    }
    else
    {
        game_prebaked->RemoveCellMax(entry_name);
    }

    game_prebaked->RemoveBodyShape(cms_entry.id);
    game_prebaked->RemoveAutoBtlPortrait(cms_entry.id);

    if (UsesBodyShape())
    {
        game_prebaked->AddBodyShape(cms_entry.id, body_shape);
    }

    if (CanUseAnyDualSkill())
    {
        game_prebaked->AddCharToAnyDualSkillList(cms_entry.id);
    }
    else
    {
        game_prebaked->RemoveCharFromAnyDualSkillList(cms_entry.id);
    }

    if (!BtlPortraitExists())
    {
        game_prebaked->AddAutoBtlPortrait(cms_entry.id);
    }

    PreBakedAlias alias;
    bool add_alias = false;

    alias.cms_id = cms_entry.id;
    alias.cms_name = cms_entry.name;

    if (HasTtcHL())
    {
        add_alias = true;
        alias.ttc_files = ttc_hl_entries[0].copy_char;
    }

    if (add_alias)
    {
        game_prebaked->AddAlias(alias);
    }
    else
    {
        game_prebaked->RemoveAlias(cms_entry.id);
    }

    InstallColors();

    if (HasDestruction())
    {
        game_prebaked->SetDestruction(cms_entry.id, des_levels);
    }

    return true;
}

bool X2mFile::InstallBtlPortrait(bool update)
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (!BtlPortraitExists())
    {
        if (update)
        {
            // Previous version "could" have a battle portrait, so if it exists, it should be removed here
            UninstallBtlPortrait(); // ignore errors
        }

        return true;
    }

    size_t size;
    uint8_t *buf;

    if (IsDummyMode())
    {
        buf = RecoverBtlPortrait(&size);
        if (!buf)
        {
            DPRINTF("%s: Failed to recover battle portrait.", FUNCNAME);
            return false;
        }
    }
    else
    {
        buf = ReadFile(X2M_BTL_PORTRAIT, &size);
        if (!buf)
            return false;
    }

    std::string emb_path = std::string("data/ui/texture/") + entry_name + ".emb";
    bool ret = xv2fs->WriteFile(emb_path, buf, size);

    delete[] buf;
    return ret;
}

bool X2mFile::InstallDir(const std::string &in_path, const std::string &out_path)
{
    size_t num_entries = GetNumEntries();

    std::string proper_in_path = in_path;
    std::string proper_out_path = out_path;

    if (proper_in_path.back() != '/')
        proper_in_path += '/';

    if (proper_out_path.length() != 0 && proper_out_path.back() != '/')
        proper_out_path += '/';

    for (size_t i = 0; i < num_entries; i++)
    {
        zip_stat_t zstat;

        if (zip_stat_index(archive, i, 0, &zstat) == -1)
            continue;

        if (!(zstat.valid & ZIP_STAT_NAME))
            continue;

        if (!(zstat.valid & ZIP_STAT_SIZE))
            continue;

        std::string entry_path = Utils::NormalizePath(zstat.name);

        if (entry_path.back() == '/')
            continue;

        if (!Utils::BeginsWith(entry_path, proper_in_path, false))
            continue;

        size_t size;
        uint8_t *buf;
        std::string out_file = proper_out_path + entry_path.substr(proper_in_path.length());

        if (IsDummyMode())
        {
            if (Utils::BeginsWith(out_file, "data2/", false) || Utils::BeginsWith(out_file, "/data2/", false))
            {
                // some mode out there is using data2 even if it doesn't really use it
                // Just ignore
                return true;
            }

            buf = RecoverFile(out_file, &size);
            if (!buf)
                return false;
        }
        else
        {
            buf = ReadFile(zstat.name, &size);
            if (!buf)
                return false;
        }

        bool ret = xv2fs->WriteFile(out_file, buf, size);
        delete[] buf;

        if (!ret)
            return false;
    }

    return true;
}

bool X2mFile::InstallCharaFiles()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (!CharaDirectoryExists())
        return false;

    if (!InstallDir(entry_name, "data/chara/" + entry_name))
        return false;

    if (temp_aur_out.size() != 0)
    {
        IniFile ini;
        const std::string ini_base = "# This ini file is needed by XV2 Mods Installer to track the character custom auras.\n"
                                     "# DON'T DELETE IT OR AURAS WILL BE UNABLE TO BE UNINSTALLED OR PROPERLY UPDATED";

        ini.Load((const uint8_t *)ini_base.c_str(), ini_base.length());
        ini.SetMultipleIntegersValues("General", "AUR", temp_aur_out, true);

        if (!xv2fs->SaveFile(&ini, "data/chara/" + entry_name + "/" + X2M_CHARA_INI))
            return false;
    }

    return true;
}

bool X2mFile::InstallJungle()
{
    if (!JungleExists())
        return false;

    return InstallDir(X2M_JUNGLE, "");
}

bool X2mFile::UninstallCharaName()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    int ls = 0, le = XV2_LANG_NUM;
    if (global_lang >= 0)
    {
        ls = global_lang;
        le = global_lang+1;
    }

    for (int i = ls; i < le; i++)
    {
        if (!Xenoverse2::RemoveAllCharaName(entry_name, i))
            return false;
    }

    return true;
}

bool X2mFile::UninstallCostumeNames()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (slot_entries.size() == 0)
        return false;

    for (size_t i = 0; i < slot_entries.size(); i++)
    {
        const X2mSlotEntry &slot = slot_entries[i];

        int ls = 0, le = XV2_LANG_NUM;
        if (global_lang >= 0)
        {
            ls = global_lang;
            le = global_lang+1;
        }

        for (int j = ls; j < le; j++)
        {
            if (!Xenoverse2::RemoveCharaCostumeName(entry_name, slot.costume_index, slot.model_preset, j))
                return false;
        }
    }

    return true;
}

bool X2mFile::UninstallSlots()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (invisible)
        return true; // Yes, true

    if (slot_entries.size() == 0)
        return false;

    UninstallCssAudio();
    chara_list->RemoveSlots(std::string("\"") + entry_name + std::string("\""));
    return true;
}

bool X2mFile::UninstallCms()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (cms_entry.id == X2M_INVALID_ID)
        return false;

    cms_entry.name = entry_name;

    CmsEntryXV2 *entry = dynamic_cast<CmsEntryXV2 *>(game_cms->FindEntryByName(cms_entry.name));
    if (entry)
    {
        cms_entry.id = entry->id;
    }
    else
    {
        return true;
    }

    return game_cms->RemoveEntry(entry_name);
}

bool X2mFile::UninstallCus()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (cms_entry.id == X2M_INVALID_ID)
        return false;

    if (cms_entry.id == X2M_DUMMY_ID)
        return true; // Yes, true

    game_cus->RemoveSkillSets(cms_entry.id);
    return true;
}

bool X2mFile::UninstallCso()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (cms_entry.id == X2M_INVALID_ID)
        return false;

    if (cms_entry.id == X2M_DUMMY_ID)
        return true; // Yes, true

    game_cso->RemoveEntries(cms_entry.id);
    return true;
}

bool X2mFile::UninstallPsc()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (cms_entry.id == X2M_INVALID_ID)
        return false;

    if (cms_entry.id == X2M_DUMMY_ID)
        return true; // Yes, true

    game_psc->RemoveEntry(cms_entry.id);
    return true;
}

bool X2mFile::UninstallAur()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (cms_entry.id == X2M_INVALID_ID)
        return false;

    if (cms_entry.id == X2M_DUMMY_ID)
        return true; // Yes, true

    GetInstalledCustomAuraChara();

    for (int i = (int) temp_aur_in.size()-1; i >= 0; i--)
    {
        game_aur->RemoveAuraIfLast((uint32_t)temp_aur_in[i]);
    }

    game_aur->RemoveCharaLinks(cms_entry.id);
    return UninstallAuraExtraChara();
}

bool X2mFile::UninstallSevAudio()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (cms_entry.id == X2M_INVALID_ID)
        return false;

    if (cms_entry.id == X2M_DUMMY_ID)
        return true; // Yes, true

    for (uint32_t i = 0; i < 0x10000; i++)
    {
        std::string name = "X2M_SEV_" + cms_entry.name + "_" + Utils::ToString(i);
        bool is_sev2;
        uint32_t cue_id = Xenoverse2::GetSevCmnCueId(name, false, &is_sev2, true);
        uint32_t cue_id_en = Xenoverse2::GetSevCmnCueId(name, true, &is_sev2, true);

        if (cue_id != cue_id_en)
        {
            DPRINTF("%s: There is a synchronization issue between the japanese and english CAQD_ADD_VOX", FUNCNAME);
            return false;
        }

        if (cue_id == (uint32_t)-1)
            break;

        if (!Xenoverse2::FreeSevCmnSound(name, false))
        {
            DPRINTF("%s: Failed to free sev sound for japanese file.\n", FUNCNAME);
            return false;
        }

        if (!Xenoverse2::FreeSevCmnSound(name, true))
        {
            DPRINTF("%s: Failed to free sev sound for english file.\n", FUNCNAME);
            return false;
        }
    }

    return true;
}

bool X2mFile::UninstallSev(std::vector<SevTableEntryHL> &hl_table, std::vector<SevTableEntryLL> &ll_table)
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (cms_entry.id == X2M_INVALID_ID)
        return false;

    if (cms_entry.id == X2M_DUMMY_ID)
        return true; // Yes, true

    // Remove ourselves from tables if found
    for (size_t i = 0; i < hl_table.size(); i++)
    {
        const SevTableEntryHL &entry_hl = hl_table[i];

        if (entry_hl.char_id == entry_name)
        {
            hl_table.erase(hl_table.begin()+i);
            i--;
        }
    }

    for (size_t i = 0; i < ll_table.size(); i++)
    {
        const SevTableEntryLL &entry_ll = ll_table[i];

        if (entry_ll.char_id == entry_name)
        {
            ll_table.erase(ll_table.begin()+i);
            i--;
        }
    }

    game_sev->RemoveAllReferencesToChar(cms_entry.id);
    RebuildSevLL(ll_table); // LL must go first
    RebuildSevHL(hl_table);

    return true;
}

bool X2mFile::UninstallCml()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (cms_entry.id == X2M_INVALID_ID)
        return false;

    if (cms_entry.id == X2M_DUMMY_ID)
        return true; // Yes, true

    game_cml->RemoveEntries(cms_entry.id);
    return true;
}

bool X2mFile::UninstallHci()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (cms_entry.id == X2M_INVALID_ID)
        return false;

    if (cms_entry.id == X2M_DUMMY_ID)
        return true; // Yes, true

    game_hci->RemoveEntries(cms_entry.id);
    return true;
}

bool X2mFile::UninstallTtbAudio()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (cms_entry.id == X2M_INVALID_ID)
        return false;

    if (cms_entry.id == X2M_DUMMY_ID)
        return true; // Yes, true

    for (uint32_t i = 0; i < 0x10000; i++)
    {
        std::string name = "X2M_TTB_" + cms_entry.name + "_" + Utils::ToString(i);
        bool is_sev2;
        uint32_t cue_id = Xenoverse2::GetSevCmnCueId(name, false, &is_sev2, true);
        uint32_t cue_id_en = Xenoverse2::GetSevCmnCueId(name, true, &is_sev2, true);

        if (cue_id != cue_id_en)
        {
            DPRINTF("%s: There is a synchronization issue between the japanese and english CAQD_ADD_VOX", FUNCNAME);
            return false;
        }

        if (cue_id == (uint32_t)-1)
            break;

        if (!Xenoverse2::FreeSevCmnSound(name, false))
        {
            DPRINTF("%s: Failed to free common dialogue sound for japanese file.\n", FUNCNAME);
            return false;
        }

        if (!Xenoverse2::FreeSevCmnSound(name, true))
        {
            DPRINTF("%s: Failed to free common dialogue sound for english file.\n", FUNCNAME);
            return false;
        }
    }

    return true;
}

bool X2mFile::UninstallTtbSubtitles()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (cms_entry.id == X2M_INVALID_ID)
        return false;

    if (cms_entry.id == X2M_DUMMY_ID)
        return true; // Yes, true

    // Remove subtitles
    for (uint32_t i = 0; i < 0x10000; i++)
    {
        std::string name = "X2M_TTB_" + cms_entry.name + "_" + Utils::ToString(i);
        bool existed;

        int ls = 0, le = XV2_LANG_NUM;
        if (global_lang >= 0)
        {
            ls = global_lang;
            le = global_lang+1;
        }

        for (int lang = ls; lang < le; lang++)
        {
            if (!Xenoverse2::RemoveTtbSubtitle(name, lang, (lang == ls) ? &existed : nullptr))
            {
                DPRINTF("%s: Failed to remove subtitle \"%s\"\n", FUNCNAME, name.c_str());
                return false;
            }
        }

        if (!existed)
            break;
    }

    // Remove voice-subtitles map
    for (uint32_t i = 0; i < 0x10000; i++)
    {
        std::string name = "X2M_TTB_" + cms_entry.name + "_" + Utils::ToString(i);
        bool existed;

        if (!Xenoverse2::RemoveTtbVoice(name, &existed))
        {
            DPRINTF("%s: Failed to remove subtitle voice map \"%s\"\n", FUNCNAME, name.c_str());
            return false;
        }

        if (!existed)
            break;
    }

    return true;
}

bool X2mFile::UninstallTtb(std::vector<TtbTableEntryHL> &hl_table, std::vector<TtbTableEntryLL> &ll_table)
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (cms_entry.id == X2M_INVALID_ID)
        return false;

    if (cms_entry.id == X2M_DUMMY_ID)
        return true; // Yes, true

    if (!game_ttb)
    {
        if (!Xenoverse2::InitCommonDialogue())
        {
            DPRINTF("%s: InitCommonDialogue failed.\n", FUNCNAME);
            return false;
        }
    }

    // Remove ourselves from tables if found
    for (size_t i = 0; i < hl_table.size(); i++)
    {
        const TtbTableEntryHL &entry_hl = hl_table[i];

        if (entry_hl.char_id == entry_name)
        {
            hl_table.erase(hl_table.begin()+i);
            i--;
        }
    }

    for (size_t i = 0; i < ll_table.size(); i++)
    {
        const TtbTableEntryLL &entry_ll = ll_table[i];

        if (entry_ll.char_id == entry_name)
        {
            ll_table.erase(ll_table.begin()+i);
            i--;
        }
    }

    game_ttb->RemoveAllReferencesToChar(cms_entry.id);
    RebuildTtbLL(ll_table); // LL must go first
    RebuildTtbHL(hl_table);

    return true;
}

bool X2mFile::UninstallTtc()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (cms_entry.id == X2M_INVALID_ID)
        return false;

    if (cms_entry.id == X2M_DUMMY_ID)
        return true; // Yes, true

    if (!game_ttc)
    {
        if (!Xenoverse2::InitCommonDialogue())
        {
            DPRINTF("%s: InitCommonDialogue failed.\n", FUNCNAME);
            return false;
        }
    }

    game_ttc->RemoveChar(cms_entry.id);
    return true;
}

bool X2mFile::UninstallCnc()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (cms_entry.id == X2M_INVALID_ID)
        return false;

    if (cms_entry.id == X2M_DUMMY_ID)
        return true; // Yes, true

    if (!game_cnc)
    {
        if (!Xenoverse2::InitDualSkill(true, false))
        {
            DPRINTF("%s: InitDualSkill failed.\n", FUNCNAME);
            return false;
        }
    }

    game_cnc->RemoveEntries(cms_entry.id);
    return true;
}

bool X2mFile::UninstallCharVfx()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (cms_entry.id == X2M_INVALID_ID)
        return false;

    if (cms_entry.id == X2M_DUMMY_ID)
        return true; // Yes, true

    if (!game_ers)
    {
        if (!Xenoverse2::InitVfx())
            return false;
    }

    game_ers->RemoveCharEepk(cms_entry.id);

    return true;
}

bool X2mFile::UninstallIkd()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (cms_entry.id == X2M_INVALID_ID)
        return false;

    if (cms_entry.id == X2M_DUMMY_ID)
        return true; // Yes, true

    game_ikd_battle->RemoveAllReferencesToChar(cms_entry.id);
    game_ikd_lobby->RemoveAllReferencesToChar(cms_entry.id);
    return true;
}

bool X2mFile::UninstallVlc()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (cms_entry.id == X2M_INVALID_ID)
        return false;

    if (cms_entry.id == X2M_DUMMY_ID)
        return true; // Yes, true

    game_vlc->RemoveEntry(cms_entry.id);
    return true;
}

bool X2mFile::UninstallSelPortrait()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    std::string name = entry_name + "_000.dds";
    return Xenoverse2::RemoveSelPortrait(name);
}

bool X2mFile::UninstallPreBaked()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    game_prebaked->RemoveOzaru(entry_name);
    game_prebaked->RemoveCellMax(entry_name);
    game_prebaked->RemoveBodyShape(cms_entry.id);
    game_prebaked->RemoveAutoBtlPortrait(cms_entry.id);
    game_prebaked->RemoveAlias(cms_entry.id);
    game_prebaked->RemoveCharFromAnyDualSkillList(cms_entry.id);
    game_prebaked->RemoveColorsMap(cms_entry.id);
    game_prebaked->RemoveDestruction(cms_entry.id);

    return true;
}

bool X2mFile::UninstallBtlPortrait()
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    std::string emb_path = std::string("data/ui/texture/") + entry_name + ".emb";
    xv2fs->RemoveFile(emb_path); // Ignore error

    return true;
}

bool X2mFile::UninstallDir(const std::string &in_path, const std::string &out_path)
{
    size_t num_entries = GetNumEntries();

    std::string proper_in_path = in_path;
    std::string proper_out_path = out_path;

    if (proper_in_path.back() != '/')
        proper_in_path += '/';

    if (proper_out_path.length() != 0 && proper_out_path.back() != '/')
        proper_out_path += '/';

    for (size_t i = 0; i < num_entries; i++)
    {
        zip_stat_t zstat;

        if (zip_stat_index(archive, i, 0, &zstat) == -1)
            continue;

        if (!(zstat.valid & ZIP_STAT_NAME))
            continue;

        if (!(zstat.valid & ZIP_STAT_SIZE))
            continue;

        std::string entry_path = Utils::NormalizePath(zstat.name);

        if (entry_path.back() == '/')
            continue;

        if (!Utils::BeginsWith(entry_path, proper_in_path, false))
            continue;

        std::string out_file = proper_out_path + entry_path.substr(proper_in_path.length());
        bool ret = xv2fs->RemoveFile(out_file);

        if (!ret)
            return false;
    }

    return true;
}

bool X2mFile::UninstallCharaFiles(bool remove_empty)
{
    if (type != X2mType::NEW_CHARACTER)
        return false;

    if (!CharaDirectoryExists())
        return false;

    const std::string path = "data/chara/" + entry_name;

    if (!UninstallDir(entry_name, path))
        return false;

    xv2fs->RemoveFile("data/chara/" + entry_name + "/" + X2M_CHARA_INI);

    if (remove_empty)
        xv2fs->RemoveEmptyDir(path);

    return true;
}

bool X2mFile::UninstallJungle()
{
    if (!JungleExists())
        return false;

    return UninstallDir(X2M_JUNGLE, "");
}

bool X2mFile::InstallPupSkill()
{
    if (type != X2mType::NEW_SKILL)
        return false;

    temp_pup_out.clear();

    if (temp_pup_in.size() == skill_pup_entries.size())
    {
        if (!HasSkillPup())
            return true;

        // Case of update where old and new have same amount of pup entries
        std::vector<PupEntry *> update_entries;

        for (int32_t pup_id : temp_pup_in)
        {
            PupEntry *entry = game_pup->FindEntryByID((uint32_t)pup_id);
            if (!entry)
                break;

            update_entries.push_back(entry);
        }

        if (update_entries.size() != skill_pup_entries.size())
            goto new_install; // Some of the ids were not found, remove and do new install

        for (size_t i = 0; i < skill_pup_entries.size(); i++)
        {
            PupEntry *update_entry = update_entries[i];
            PupEntry &entry = skill_pup_entries[i];

            entry.id = update_entry->id;
            *update_entry = entry;
        }

        goto setup_pup;
    }

new_install:
    // Case of new install or update where old and new have different amount of pup entries

    // Remove first existing entries
    for (int32_t pup_id : temp_pup_in)
    {
        game_pup->RemoveEntry((uint32_t)pup_id);
    }

    if (!HasSkillPup())
        return true;

    if (!game_pup->AddConsecutiveEntries(skill_pup_entries))
        return false;

setup_pup:

    skill_entry.pup_id = skill_pup_entries[0].id;

    for (const PupEntry &entry : skill_pup_entries)
    {
        temp_pup_out.push_back((int32_t)entry.id);
    }

    return true;
}

bool X2mFile::InstallAuraExtraSkill()
{
    UninstallAuraExtraSkill();

    for (X2mSkillAura &aura : skill_aura_entries)
    {
        if ((int16_t)aura.data.aur_aura_id >= 0 && (uint32_t)aura.extra.aur_id == X2M_INVALID_ID)
        {
            aura.extra.aur_id = aura.data.aur_aura_id;
            game_prebaked->SetAuraExtra(aura.extra);
        }
    }

    return true;
}

bool X2mFile::UninstallAuraExtraSkill()
{
    for (int32_t aur_id : temp_aur_in)
    {
        game_prebaked->RemoveAuraExtra(aur_id);
    }

    return true;
}

bool X2mFile::InstallCustomAuraSkill()
{
    temp_aur_out.clear();

    size_t num_custom = GetNumSkillCustomAuras();

    //DPRINTF("%Id  %Id\n", num_custom, temp_aur_in.size());

    if (temp_aur_in.size() == num_custom)
    {
        if (!HasSkillCustomAuras())
            return InstallAuraExtraSkill();

        // Case of "perfect update"
        std::vector<AurAura *> update_entries;

        for (int32_t aur_id : temp_aur_in)
        {
            AurAura *aura = game_aur->FindAuraByID((uint32_t)aur_id);
            if (!aura)
                break;

            update_entries.push_back(aura);
        }

        if (update_entries.size() != num_custom)
            goto new_install; // Some of the ids were not found, remove and do new install

        for (size_t i = 0, j = 0; i < skill_aura_entries.size(); i++)
        {
            if (skill_aura_entries[i].data.aur_aura_id != X2M_INVALID_ID16)
                continue;

            if (j >= update_entries.size())
            {
                DPRINTF("%s: Internal coding error.\n", FUNCNAME);
                return false;
            }

            AurAura *update_aura = update_entries[j];
            AurAura &aura = skill_aura_entries[i].aura;

            aura.id = update_aura->id;
            skill_aura_entries[i].data.aur_aura_id = (uint16_t)aura.id;
            *update_aura = aura;

            temp_aur_out.push_back((int32_t)aura.id);
            j++;
        }

        goto setup_aura;
    }

new_install:

    // Case of new install or update where old and new have different amount of custom aura entries

    // Remove first existing entries
    for (int i = (int) temp_aur_in.size()-1; i >= 0; i--)
    {
        game_aur->RemoveAuraIfLast((uint32_t)temp_aur_in[i]);
    }

    if (!HasSkillCustomAuras())
        return InstallAuraExtraSkill();

    for (size_t i = 0; i < skill_aura_entries.size(); i++)
    {
        if (skill_aura_entries[i].data.aur_aura_id != X2M_INVALID_ID16)
            continue;

        AurAura &aura = skill_aura_entries[i].aura;

        if (!game_aur->AddAura(aura))
            return false;

        skill_aura_entries[i].data.aur_aura_id = (uint16_t)aura.id;
        temp_aur_out.push_back((int32_t)aura.id);
    }

setup_aura:

    // Nothing to do, setup happened before
    return InstallAuraExtraSkill();
}

bool X2mFile::InstallAuraSkill()
{
    std::vector<CusAuraData> temp;

    if (type != X2mType::NEW_SKILL)
        return false;

    if (!InstallCustomAuraSkill())
        return false;

    temp_cus_aur_out.clear();

    if (temp_cus_aur_in.size() == skill_aura_entries.size())
    {
        if (!HasSkillAura())
            return true;

        // Case of update where old and new have same amount of entries
        std::vector<CusAuraData *> update_entries;

        for (int32_t cus_aur_id : temp_cus_aur_in)
        {
            CusAuraData *data = game_prebaked->FindAuraData((uint16_t)cus_aur_id);
            if (!data)
                break;

            update_entries.push_back(data);
        }

        if (update_entries.size() != skill_aura_entries.size())
            goto new_install; // Some of the ids were not found, remove and do new install

        for (size_t i = 0; i < skill_aura_entries.size(); i++)
        {
            CusAuraData *update_data = update_entries[i];
            CusAuraData &data = skill_aura_entries[i].data;

            data.cus_aura_id = update_data->cus_aura_id;
            *update_data = data;
        }

        goto setup_data;
    }

new_install:
    // Case of new install or update where old and new have different amount of pup entries

    // Remove first existing entries
    for (int32_t cus_aur_id : temp_cus_aur_in)
    {
        game_prebaked->RemoveAuraData((uint16_t)cus_aur_id);
    }

    if (!HasSkillAura())
        return true;

    for (size_t i = 0; i < skill_aura_entries.size(); i++)
    {
        temp.push_back(skill_aura_entries[i].data);
    }

    if (!game_prebaked->AddConsecutiveAuraData(temp))
        return false;

    // Update data from temp
    for (size_t i = 0; i < skill_aura_entries.size(); i++)
    {
        skill_aura_entries[i].data = temp[i];
    }

setup_data:

    skill_entry.aura = skill_aura_entries[0].data.cus_aura_id;

    for (const X2mSkillAura &aura : skill_aura_entries)
    {
        temp_cus_aur_out.push_back((int32_t)aura.data.cus_aura_id);
    }

    return true;
}

bool X2mFile::InstallCusSkill(uint32_t depends_cms)
{
    if (type != X2mType::NEW_SKILL)
        return false;

    if (skill_entry.id == X2M_INVALID_ID16)
        return false;

    if (HasSkillCostumeDepend())
    {
        X2mCostumeEntry *costume = FindInstalledCostume(skill_costume_depend.guid);

        if (costume && costume->partsets.size() > 0)
        {
            skill_entry.partset = costume->partsets.front();
        }
        else
        {
            skill_entry.partset = 0xFFFF;
        }
    }

    if (HasSkillCharaDepend())
    {
        skill_entry.model = (uint16_t)depends_cms;
    }

    if (skill_type == X2mSkillType::BLAST && blast_ss_intended)
    {
        skill_entry.race_lock = 0xFF; // required or won't work
    }

    if (skill_entry.id == X2M_DUMMY_ID16)
    {
        // New install
        if (!AssignSkillIds())
        {
            DPRINTF("%s: Failed in assigning an id to the skill.\n", FUNCNAME);
            return false;
        }

        if (!game_cus->AddSkillAuto(skill_entry))
        {
            DPRINTF("%s: AddSkillAuto failed.\n", FUNCNAME);
            return false;
        }
    }
    else
    {
        CusSkill *skill = game_cus->FindSkillAnyByID(skill_entry.id);
        if (!skill)
        {
            DPRINTF("%s: Internal error, cannot find skill for update.\n", FUNCNAME);
            return false;
        }

        *skill = skill_entry; // Update
    }

    return true;
}

bool X2mFile::InstallSkillName()
{
    if (type != X2mType::NEW_SKILL)
        return false;

    if (skill_entry.id == X2M_INVALID_ID16 || skill_entry.id == X2M_DUMMY_ID16)
    {
        DPRINTF("%s: InstallCusSkill must be called before this!\n", FUNCNAME);
        return false;
    }

    if (skill_type == X2mSkillType::BLAST)
    {
        if (blast_ss_intended)
        {
            if (skill_name[XV2_LANG_ENGLISH].length() == 0)
                return false;

            int ls = 0, le = XV2_LANG_NUM;
            if (global_lang >= 0)
            {
                ls = global_lang;
                le = global_lang+1;
            }

            for (int i = ls; i < le; i++)
            {
                const std::string *name;

                if (skill_name[i].length() != 0)
                    name = &skill_name[i];
                else
                    name = &skill_name[XV2_LANG_ENGLISH];

                if (!Xenoverse2::SetModBlastSkillType(skill_entry.id2, *name, i))
                    return false;
            }
        }

        return true;
    }

    if (skill_name[XV2_LANG_ENGLISH].length() == 0)
        return false;

    int ls = 0, le = XV2_LANG_NUM;
    if (global_lang >= 0)
    {
        ls = global_lang;
        le = global_lang+1;
    }

    for (int i = ls; i < le; i++)
    {
        const std::string *name;

        if (skill_name[i].length() != 0)
            name = &skill_name[i];
        else
            name = &skill_name[XV2_LANG_ENGLISH];

        if (skill_type == X2mSkillType::SUPER)
        {
            if (!Xenoverse2::SetSuperSkillName(skill_entry.id2, *name, i))
                return false;
        }
        else if (skill_type == X2mSkillType::ULTIMATE)
        {
            if (!Xenoverse2::SetUltimateSkillName(skill_entry.id2, *name, i))
                return false;
        }
        else if (skill_type == X2mSkillType::EVASIVE)
        {
            if (!Xenoverse2::SetEvasiveSkillName(skill_entry.id2, *name, i))
                return false;
        }
        else if (skill_type == X2mSkillType::AWAKEN)
        {
            if (!Xenoverse2::SetAwakenSkillName(skill_entry.id2, *name, i))
                return false;
        }
        else
        {
            DPRINTF("%s: Internal error.\n", FUNCNAME);
            return false;
        }
    }

    if (skill_type == X2mSkillType::AWAKEN)
    {
        // Clean up first (for updated with different number of entries)
        for (int i = 0; i < 100; i++)
        {
            int ls = 0, le = XV2_LANG_NUM;
            if (global_lang >= 0)
            {
                ls = global_lang;
                le = global_lang+1;
            }

            for (int lang = ls; lang < le; lang++)
                Xenoverse2::RemoveBtlHudAwakenName(skill_entry.id2, i, lang);
        }

        for (size_t i = 0; i < skill_trans_names.size(); i++)
        {
            const X2mSkillTransName &tn = skill_trans_names[i];

            int ls = 0, le = XV2_LANG_NUM;
            if (global_lang >= 0)
            {
                ls = global_lang;
                le = global_lang+1;
            }

            for (int lang = ls; lang < le; lang++)
            {
                const std::string *name;

                if (tn.trans_name[lang].length() != 0)
                    name = &tn.trans_name[lang];
                else
                    name = &tn.trans_name[XV2_LANG_ENGLISH];

                if (!Xenoverse2::SetBtlHudAwakenName(skill_entry.id2, (uint16_t)i, *name, lang))
                    return false;
            }
        }
    }

    return true;
}

bool X2mFile::InstallSkillDesc()
{
    if (type != X2mType::NEW_SKILL)
        return false;

    if (skill_entry.id == X2M_INVALID_ID16 || skill_entry.id == X2M_DUMMY_ID16)
    {
        DPRINTF("%s: InstallCusSkill must be called before this!\n", FUNCNAME);
        return false;
    }

    if (skill_type == X2mSkillType::BLAST)
        return true;

    if (skill_desc[XV2_LANG_ENGLISH].length() == 0)
    {
        // This is for case where previous version had description and current one doesn't.
        return UninstallSkillDesc();
    }

    int ls = 0, le = XV2_LANG_NUM;
    if (global_lang >= 0)
    {
        ls = global_lang;
        le = global_lang+1;
    }

    for (int i = ls; i < le; i++)
    {
        const std::string *desc;

        if (skill_desc[i].length() != 0)
            desc = &skill_desc[i];
        else
            desc = &skill_desc[XV2_LANG_ENGLISH];

        if (skill_type == X2mSkillType::SUPER)
        {
            if (!Xenoverse2::SetSuperSkillDesc(skill_entry.id2, *desc, i))
                return false;
        }
        else if (skill_type == X2mSkillType::ULTIMATE)
        {
            if (!Xenoverse2::SetUltimateSkillDesc(skill_entry.id2, *desc, i))
                return false;
        }
        else if (skill_type == X2mSkillType::EVASIVE)
        {
            if (!Xenoverse2::SetEvasiveSkillDesc(skill_entry.id2, *desc, i))
                return false;
        }
        else if (skill_type == X2mSkillType::AWAKEN)
        {
            if (!Xenoverse2::SetAwakenSkillDesc(skill_entry.id2, *desc, i))
                return false;
        }
        else
        {
            DPRINTF("%s: Internal error.\n", FUNCNAME);
            return false;
        }
    }

    return true;
}

bool X2mFile::InstallSkillHow()
{
    if (type != X2mType::NEW_SKILL)
        return false;

    if (skill_entry.id == X2M_INVALID_ID16 || skill_entry.id == X2M_DUMMY_ID16)
    {
        DPRINTF("%s: InstallCusSkill must be called before this!\n", FUNCNAME);
        return false;
    }

    if (skill_type == X2mSkillType::BLAST)
        return true;

    if (skill_how[XV2_LANG_ENGLISH].length() == 0)
    {
        // This is for case where previous version had description and current one doesn't.
        return UninstallSkillHow();
    }

    int ls = 0, le = XV2_LANG_NUM;
    if (global_lang >= 0)
    {
        ls = global_lang;
        le = global_lang+1;
    }

    for (int i = ls; i < le; i++)
    {
        const std::string *how;

        if (skill_how[i].length() != 0)
            how = &skill_how[i];
        else
            how = &skill_how[XV2_LANG_ENGLISH];

        if (skill_type == X2mSkillType::SUPER)
        {
            if (!Xenoverse2::SetSuperSkillHow(skill_entry.id2, *how, i))
                return false;
        }
        else if (skill_type == X2mSkillType::ULTIMATE)
        {
            if (!Xenoverse2::SetUltimateSkillHow(skill_entry.id2, *how, i))
                return false;
        }
        else if (skill_type == X2mSkillType::EVASIVE)
        {
            if (!Xenoverse2::SetEvasiveSkillHow(skill_entry.id2, *how, i))
                return false;
        }
        else if (skill_type == X2mSkillType::AWAKEN)
        {
            if (!Xenoverse2::SetAwakenSkillHow(skill_entry.id2, *how, i))
                return false;
        }
        else
        {
            DPRINTF("%s: Internal error.\n", FUNCNAME);
            return false;
        }
    }

    return true;
}

bool X2mFile::InstallIdbSkill()
{
    if (type != X2mType::NEW_SKILL)
        return false;

    if (skill_entry.id == X2M_INVALID_ID16 || skill_entry.id == X2M_DUMMY_ID16)
    {
        DPRINTF("%s: InstallCusSkill must be called before this!\n", FUNCNAME);
        return false;
    }

    if (skill_type == X2mSkillType::BLAST)
        return true;

    if (!HasSkillIdbEntry())
    {
        // Uninstall previous idb (if any)
        game_skill_idb->RemoveEntry(skill_entry.id2, GetCusSkillType());
        return true;
    }

    AssignSkillIdbIds();
    return game_skill_idb->AddEntry(skill_idb_entry, true);
}

bool X2mFile::InstallSkillBodies()
{
    if (type != X2mType::NEW_SKILL)
        return false;

    // Update reusing existing bodies, if possible
    std::vector<X2mBody *> existing_bodies;

    if (game_costume_file->FindBody(mod_guid, existing_bodies) > 0)
    {
        bool can_update = false;

        if (existing_bodies.size() == skill_bodies.size())
        {
            can_update = true;

            for (size_t i = 0; i < existing_bodies.size(); i++)
            {
                if (existing_bodies[i]->race != skill_bodies[i].race)
                {
                    can_update = false;
                    break;
                }
            }

            if (can_update)
            {
                for (X2mBody *body : existing_bodies)
                {
                    int custom_start = (body->race == X2M_CR_NMC) ? 13 : 12;

                    BcsFile *bcs = GetBcsForRace(body->race);
                    const std::vector<BcsBody> &bodies = bcs->GetBodies();

                    if (body->id < custom_start || body->id >= (int)bodies.size())
                    {
                        can_update = false;
                        break;
                    }
                }
            }
        }

        if (can_update)
        {
            for (size_t i = 0; i < existing_bodies.size(); i++)
            {
                X2mBody *existing_body = existing_bodies[i];
                X2mBody updated_body = skill_bodies[i]; // Copy

                memcpy(updated_body.guid, mod_guid, sizeof(mod_guid));
                bodies_map[updated_body.id] = existing_body->id;
                updated_body.id = existing_body->id;

                BcsFile *bcs = GetBcsForRace(updated_body.race);
                std::vector<BcsBody> &bodies = bcs->GetBodies();

                // Index check was already done before
                bodies[updated_body.id] = updated_body.body;

                // Update costume entry
                *existing_body = updated_body;
            }

            return true;
        }
        else
        {
            if (!UninstallSkillBodies())
                return false;

            // Control goes to new install logic now
        }
    }

    // New install

    if (!HasSkillBodies())
        return true;

    for (const X2mBody &body : skill_bodies)
    {
        X2mBody new_body = body; // Copy

        memcpy(new_body.guid, mod_guid, sizeof(mod_guid));

        BcsFile *bcs = GetBcsForRace(new_body.race);
        std::vector<BcsBody> &bodies = bcs->GetBodies();
        int custom_start = (new_body.race == X2M_CR_NMC) ? 13 : 12;

        if ((int)bodies.size() < custom_start)
        {
            DPRINTF("%s: Was expecting bcs for race %d to have at least %d bodies.\n", FUNCNAME, new_body.race, custom_start);
            return false;
        }

        int body_id = -1;

        for (int b = custom_start; b < (int)bodies.size(); b++)
        {
            if (bodies[b].IsEmpty() && !game_costume_file->FindBodyById(b))
            {
                body_id = b;
                break;
            }
        }

        if (body_id < 0)
        {
            body_id = (int)bodies.size();
            bodies.push_back(new_body.body);
        }
        else
        {
            bodies[body_id] = new_body.body;
        }

        bodies_map[new_body.id] = body_id;
        new_body.id = body_id;

        if (!game_costume_file->AddBody(new_body))
            return false;
    }

    return true;
}

bool X2mFile::InstallSkillVisitor(const std::string &path, void *param)
{
    X2mFile *pthis = (X2mFile *)param;
    const std::string skill_dir = pthis->GetSkillDirectory();
    const std::string filename = Utils::GetFileNameString(path);

    size_t size;
    uint8_t *buf;

    if (IsDummyMode())
    {
        buf = pthis->RecoverSkillFile(path, &size);
        if (!buf)
        {
            DPRINTF("%s: Failed to recover skill file \"%s\"", FUNCNAME, filename.c_str());
            return false;
        }
    }
    else
    {
        buf = pthis->ReadFile(path, &size);
        if (!buf)
            return false;
    }

    if (path.back() == '/' || path.back() == '\\')
        return true;

    if (Utils::BeginsWith(filename, X2M_SKILL_PREFIX, false))
    {
        CmsEntry *entry = game_cms->FindEntryByID(pthis->skill_entry.id2 / 10);
        if (!entry)
        {
            delete[] buf;
            return false;
        }

        std::string out_path = skill_dir + Utils::ToStringAndPad(pthis->skill_entry.id2, 3) + "_" + entry->name + "_" + pthis->skill_entry.name;
        out_path += filename.substr(strlen(X2M_SKILL_PREFIX));

        if (Utils::EndsWith(path, ".bac", false))
        {
            BacFile bac;

            if (!bac.Load(buf, size))
            {
                DPRINTF("%s: Failed to load internal x2m bac file: \"%s\"\n", FUNCNAME, path.c_str());
                delete[] buf;
                return false;
            }

            delete[] buf;
            bac.ChangeReferencesToSkill(X2M_DUMMY_ID16, pthis->skill_entry.id2);
            bac.ChangeReferencesToSkill(X2M_DUMMY_ID16_2, pthis->skill_entry.id);

            if (pthis->bodies_map.size() > 0)
            {
                for (BacEntry &entry : bac)
                {
                    if (entry.has_type[15])
                    {
                        for (BACTransformControl &tc : entry.type15)
                        {
                            if (tc.type == 0x2a && (int)tc.parameter >= X2M_SKILL_BODY_ID_BEGIN)
                            {
                                auto it = pthis->bodies_map.find((int)tc.parameter);

                                if (it != pthis->bodies_map.end())
                                {
                                    tc.parameter = (float)it->second;
                                }
                            }
                        }
                    }
                }
            }

            if (!xv2fs->SaveFile(&bac, out_path))
                return false;

            return true;
        }
        else if (Utils::EndsWith(path, ".bsa", false))
        {
            BsaFile bsa;

            if (!bsa.Load(buf, size))
            {
                DPRINTF("%s: Failed to load internal x2m bsa file: \"%s\"\n", FUNCNAME, path.c_str());
                delete[] buf;
                return false;
            }

            delete[] buf;
            bsa.ChangeReferencesToSkill(X2M_DUMMY_ID16, pthis->skill_entry.id2);
            bsa.ChangeReferencesToSkill(X2M_DUMMY_ID16_2, pthis->skill_entry.id);

            if (!xv2fs->SaveFile(&bsa, out_path))
                return false;

            return true;
        }
        else if (Utils::EndsWith(path, ".bdm", false))
        {
            BdmFile bdm;
            bool ret;

            if (bdm.Load(buf, size))
            {
                bdm.ChangeReferencesToSkill(X2M_DUMMY_ID16, pthis->skill_entry.id2);
                bdm.ChangeReferencesToSkill(X2M_DUMMY_ID16_2, pthis->skill_entry.id);

                ret = xv2fs->SaveFile(&bdm, out_path);
            }
            else
            {
                 ret = xv2fs->WriteFile(out_path, buf, size);
            }

            delete[] buf;
            return ret;
        }
        else
        {
            bool ret = xv2fs->WriteFile(out_path, buf, size);
            delete[] buf;

            return ret;
        }
    }
    else
    {
        if (Utils::ToUpperCase(filename) == "X2M_SKILL.INI")
        {
            DPRINTF("PLEASE STOP PACKAGING X2M_SKILL.INI INTO THE SKILLS DIRECTORY, THAT'S AN INTERNAL FILE OF THE INSTALLER.\n"
                    "INSTALLATION WILL CONTINUE, BUT THERE MAY BE PROBLEMS IN REINSTALL/UNINSTALL.\n");
            return true;
        }
    }

    std::string out_path = skill_dir + filename;

    bool ret = xv2fs->WriteFile(out_path, buf, size);
    delete[] buf;

    return ret;
}

bool X2mFile::InstallSkillFiles()
{
    if (type != X2mType::NEW_SKILL)
        return false;

    if (!SkillDirectoryExists())
        return false;

    if (skill_entry.id == X2M_INVALID_ID16 || skill_entry.id == X2M_DUMMY_ID16)
    {
        DPRINTF("%s: InstallCusSkill must be called before this!\n", FUNCNAME);
        return false;
    }

    const std::string skill_dir = GetSkillDirectory();
    if (skill_dir.length() == 0)
    {
        DPRINTF("%s: GetSkillDirectory failed.\n", FUNCNAME);
        return false;
    }

    if (!VisitDirectory(X2M_SKILL, InstallSkillVisitor, this))
        return false;

    IniFile ini;
    const std::string ini_base = "# This ini file is needed by XV2 Mods Installer to track the skill.\n"
                                 "# DON'T DELETE IT OR SKILL WILL BE UNABLE TO BE UNINSTALLED OR PROPERLY UPDATED";

    ini.Load((const uint8_t *)ini_base.c_str(), ini_base.length());
    ini.SetStringValue("General", "GUID", Utils::GUID2String(mod_guid));

    if (temp_pup_out.size() != 0)
        ini.SetMultipleIntegersValues("General", "PUP", temp_pup_out, true);

    if (temp_cus_aur_out.size() != 0)
        ini.SetMultipleIntegersValues("General", "CUS_AUR", temp_cus_aur_out, true);

    if (temp_aur_out.size() != 0)
        ini.SetMultipleIntegersValues("General", "AUR", temp_aur_out, true);

    if (!xv2fs->SaveFile(&ini, skill_dir + X2M_SKILL_INI))
        return false;

    return true;
}

bool X2mFile::UninstallPupSkill()
{
    if (type != X2mType::NEW_SKILL)
        return false;

    if (skill_entry.id == X2M_INVALID_ID16)
        return false;

    for (int32_t pup_id : temp_pup_in)
    {
        game_pup->RemoveEntry((uint32_t)pup_id);
    }

    return true;
}

bool X2mFile::UninstallAuraSkill()
{
    if (type != X2mType::NEW_SKILL)
        return false;

    if (skill_entry.id == X2M_INVALID_ID16)
        return false;

    for (int i = (int) temp_aur_in.size()-1; i >= 0; i--)
    {
        game_aur->RemoveAuraIfLast((uint32_t)temp_aur_in[i]);
    }

    for (int32_t cus_aur_id : temp_cus_aur_in)
    {
        game_prebaked->RemoveAuraData((uint16_t)cus_aur_id);
    }

    return UninstallAuraExtraSkill();
}

bool X2mFile::UninstallCusSkill()
{
    if (type != X2mType::NEW_SKILL)
        return false;

    if (skill_entry.id == X2M_INVALID_ID16)
        return false;

    if (skill_entry.id == X2M_DUMMY_ID16)
        return true; // Yes, true    

    game_cus->RemoveAllReferencesToSkill(skill_entry.id);
    game_cus->RemoveSkillAuto(skill_entry.id);
    return true;
}

bool X2mFile::UninstallSkillName()
{
    if (type != X2mType::NEW_SKILL)
        return false;

    if (skill_entry.id == X2M_INVALID_ID16)
        return false;

    if (skill_entry.id == X2M_DUMMY_ID16)
        return true; // Yes, true

    if (skill_type == X2mSkillType::BLAST)
    {
        if (blast_ss_intended)
        {
            int ls = 0, le = XV2_LANG_NUM;
            if (global_lang >= 0)
            {
                ls = global_lang;
                le = global_lang+1;
            }

            for (int i = ls; i < le; i++)
            {
                if (!Xenoverse2::RemoveModBlastSkillType(skill_entry.id2, i))
                    return false;
            }
        }

        return true;
    }

    int ls = 0, le = XV2_LANG_NUM;
    if (global_lang >= 0)
    {
        ls = global_lang;
        le = global_lang+1;
    }

    for (int i = ls; i < le; i++)
    {
        if (skill_type == X2mSkillType::SUPER)
        {
            if (!Xenoverse2::RemoveSuperSkillName(skill_entry.id2, i))
                return false;
        }
        else if (skill_type == X2mSkillType::ULTIMATE)
        {
            if (!Xenoverse2::RemoveUltimateSkillName(skill_entry.id2, i))
                return false;
        }
        else if (skill_type == X2mSkillType::EVASIVE)
        {
            if (!Xenoverse2::RemoveEvasiveSkillName(skill_entry.id2, i))
                return false;
        }
        else if (skill_type == X2mSkillType::AWAKEN)
        {
            if (!Xenoverse2::RemoveAwakenSkillName(skill_entry.id2, i))
                return false;
        }
        else
        {
            DPRINTF("%s: Internal error.\n", FUNCNAME);
            return false;
        }
    }

    if (skill_type == X2mSkillType::AWAKEN)
    {
        for (int i = 0; i < 100; i++)
        {
            int ls = 0, le = XV2_LANG_NUM;
            if (global_lang >= 0)
            {
                ls = global_lang;
                le = global_lang+1;
            }

            for (int lang = ls; lang < le; lang++)
                Xenoverse2::RemoveBtlHudAwakenName(skill_entry.id2, i, lang);
        }
    }

    return true;
}

bool X2mFile::UninstallSkillDesc()
{
    if (type != X2mType::NEW_SKILL)
        return false;

    if (skill_entry.id == X2M_INVALID_ID16)
        return false;

    if (skill_entry.id == X2M_DUMMY_ID16)
        return true; // Yes, true

    if (skill_type == X2mSkillType::BLAST)
        return true;

    int ls = 0, le = XV2_LANG_NUM;
    if (global_lang >= 0)
    {
        ls = global_lang;
        le = global_lang+1;
    }

    for (int i = ls; i < le; i++)
    {
        if (skill_type == X2mSkillType::SUPER)
        {
            if (!Xenoverse2::RemoveSuperSkillDesc(skill_entry.id2, i))
                return false;
        }
        else if (skill_type == X2mSkillType::ULTIMATE)
        {
            if (!Xenoverse2::RemoveUltimateSkillDesc(skill_entry.id2, i))
                return false;
        }
        else if (skill_type == X2mSkillType::EVASIVE)
        {
            if (!Xenoverse2::RemoveEvasiveSkillDesc(skill_entry.id2, i))
                return false;
        }
        else if (skill_type == X2mSkillType::AWAKEN)
        {
            if (!Xenoverse2::RemoveAwakenSkillDesc(skill_entry.id2, i))
                return false;
        }
        else
        {
            DPRINTF("%s: Internal error.\n", FUNCNAME);
            return false;
        }
    }

    return true;
}

bool X2mFile::UninstallSkillHow()
{
    if (type != X2mType::NEW_SKILL)
        return false;

    if (skill_entry.id == X2M_INVALID_ID16)
        return false;

    if (skill_entry.id == X2M_DUMMY_ID16)
        return true; // Yes, true

    if (skill_type == X2mSkillType::BLAST)
        return true;

    int ls = 0, le = XV2_LANG_NUM;
    if (global_lang >= 0)
    {
        ls = global_lang;
        le = global_lang+1;
    }

    for (int i = ls; i < le; i++)
    {
        if (skill_type == X2mSkillType::SUPER)
        {
            if (!Xenoverse2::RemoveSuperSkillHow(skill_entry.id2, i))
                return false;
        }
        else if (skill_type == X2mSkillType::ULTIMATE)
        {
            if (!Xenoverse2::RemoveUltimateSkillHow(skill_entry.id2, i))
                return false;
        }
        else if (skill_type == X2mSkillType::EVASIVE)
        {
            if (!Xenoverse2::RemoveEvasiveSkillHow(skill_entry.id2, i))
                return false;
        }
        else if (skill_type == X2mSkillType::AWAKEN)
        {
            if (!Xenoverse2::RemoveAwakenSkillHow(skill_entry.id2, i))
                return false;
        }
        else
        {
            DPRINTF("%s: Internal error.\n", FUNCNAME);
            return false;
        }
    }

    return true;
}

bool X2mFile::UninstallIdbSkill()
{
    if (type != X2mType::NEW_SKILL)
        return false;

    if (skill_entry.id == X2M_INVALID_ID16)
        return false;

    if (skill_entry.id == X2M_DUMMY_ID16)
        return true; // Yes, true

    if (skill_type == X2mSkillType::BLAST)
        return true;

    game_skill_idb->RemoveEntry(skill_entry.id2, GetCusSkillType());
    return true;
}

bool X2mFile::UninstallSkillBodies()
{
    if (type != X2mType::NEW_SKILL)
        return false;

    std::vector<X2mBody *> bodies;
    game_costume_file->FindBody(mod_guid, bodies);

    std::vector<std::unordered_set<int>> deleted_bodies_bcs;
    deleted_bodies_bcs.resize(X2M_CR_NUM);

    for (X2mBody *body : bodies)
    {
        int custom_start = (body->race == X2M_CR_NMC) ? 13 : 12;

        //DPRINTF("Uninstalling body %d\n", body->id);

        if (body->id < custom_start)
        {
            DPRINTF("%s: A skill body should not use an id smaller than %d for race %d\n", FUNCNAME, custom_start, body->race);
            return false;
        }

        BcsFile *bcs = GetBcsForRace(body->race);
        std::vector<BcsBody> &race_bodies = bcs->GetBodies();

        // Empty the bcs body
        if (body->id < (int)race_bodies.size())
        {
            race_bodies[body->id].valid = false;
            race_bodies[body->id].unk1s.clear();
            deleted_bodies_bcs[body->race].insert(body->id);
        }

        // Maintenance, clear bodies at the end
        for (int b = (int)(race_bodies.size()-1); b >= custom_start; b--)
        {
            if (deleted_bodies_bcs[body->race].find(b) == deleted_bodies_bcs[body->race].end())
                break;

            race_bodies.pop_back();
        }
    }

    // Remove bodies from costume file
    game_costume_file->RemoveBodiesFromMod(mod_guid);

    return true;
}

bool X2mFile::UninstallSkillFiles(bool remove_empty)
{
    if (type != X2mType::NEW_SKILL)
        return false;

    if (!SkillDirectoryExists())
        return false;

    if (skill_entry.id == X2M_INVALID_ID16)
        return false;

    if (skill_entry.id == X2M_DUMMY_ID16)
        return true; // Yes true

    const std::string path = GetSkillDirectory();
    if (path.length() == 0)
        return false;

    xv2fs->RemoveDir(path, remove_empty);
    return true;
}

static bool ResolvePartSetString(std::string &str, uint16_t base, uint16_t num)
{
    if (str.length() == 0)
        return false;

    static const uint16_t lower_limit = 10000;
    uint16_t upper_limit = lower_limit+num;

    for (uint16_t i = lower_limit; i < upper_limit; i++)
    {
        std::string str_num = Utils::ToString(i);
        size_t pos = str.find(str_num);

        if (pos != std::string::npos)
        {
            std::string temp = str.substr(0, pos);
            temp += Utils::ToStringAndPad((i-lower_limit)+base, 3);
            temp += str.substr(pos+str_num.length());

            str = temp;
            return true;
        }
    }

    return false;
}

static void ResolvePartSet(BcsPartSet &set, uint16_t base, uint16_t num)
{
    if (!set.valid)
        return;

    static const uint16_t lower_limit = 10000;
    uint16_t upper_limit = lower_limit+num;

    for (size_t i = 0; i < set.parts.size(); i++)
    {
        BcsPart &part = set.parts[i];
        if (!part.valid)
            continue;

        if (part.model >= lower_limit && part.model < upper_limit)
        {
            part.model = (part.model-lower_limit) + base;
        }

        if (part.model2 >= lower_limit && part.model2 < upper_limit)
        {
            part.model2 = (part.model2-lower_limit) + base;
        }

        for (std::string &str : part.files)
        {
            ResolvePartSetString(str, base, num);
        }

        for (BcsPhysics &ph : part.physics)
        {
            for (std::string &str : ph.unk_28)
            {
                ResolvePartSetString(str, base, num);
            }
        }
    }
}

bool X2mFile::InstallCostumePartSets()
{
    if (type != X2mType::NEW_COSTUME)
        return false;

    static std::vector<uint16_t> candidates_partsets;
    static std::vector<BcsFile *> races_bcs;

    if (candidates_partsets.size() == 0)
    {        
        for (uint16_t i = 2000; i != 0; i++) // 2000-65535
        {
            if (i >= 10000 && i <= 10099) // To avoid any potential conflict with the numbers used internally by the creator/installer
                continue;

            candidates_partsets.push_back(i);
        }

        /*for (uint16_t i = 535; i <= 549; i++) // 15
            candidates_partsets.push_back(i);

        for (uint16_t i = 559; i <= 599; i++) // 41
            candidates_partsets.push_back(i);

        for (uint16_t i = 631; i <= 649; i++) // 19
            candidates_partsets.push_back(i);

        for (uint16_t i = 670; i <= 699; i++) // 30
            candidates_partsets.push_back(i);

        for (uint16_t i = 796; i <= 799; i++) // 4
            candidates_partsets.push_back(i);

        for (uint16_t i = 801; i <= 830; i++) // 30
            candidates_partsets.push_back(i);

        for (uint16_t i = 866; i <= 884; i++) // 19
            candidates_partsets.push_back(i);

        for (uint16_t i = 973; i <= 999; i++) // 27
            candidates_partsets.push_back(i);

        for (uint16_t i = 1000; i <= 1199; i++) // 200
            candidates_partsets.push_back(i);

        for (uint16_t i = 1211; i <= 1299; i++) // 89
            candidates_partsets.push_back(i);

        for (uint16_t i = 1331; i <= 1499; i++) // 169
            candidates_partsets.push_back(i);

        for (uint16_t i = 1521; i <= 1699; i++) // 179
            candidates_partsets.push_back(i);

        for (uint16_t i = 1741; i <= 1770; i++) // 30
            candidates_partsets.push_back(i);*/
    }

    if (races_bcs.size() == 0)
    {
        races_bcs.push_back(game_hum_bcs);
        races_bcs.push_back(game_huf_bcs);
        races_bcs.push_back(game_nmc_bcs);
        races_bcs.push_back(game_fri_bcs);
        races_bcs.push_back(game_mam_bcs);
        races_bcs.push_back(game_maf_bcs);
    }

    bool update = (update_costume_entry.partsets.size() > 0);
    size_t num = GetEffectiveNumCostumePartSets();

    if (!update)
    {
        size_t num_found = 0;
        uint16_t partset = 0;

        for (size_t i = 0; i < num && num_found != num; i++)
        {
            for (size_t j = 0; j < candidates_partsets.size(); j++)
            {
                uint16_t this_partset = candidates_partsets[j];

                if (j > 0 && this_partset != (candidates_partsets[j-1]+1))
                    num_found = 0;

                bool in_use = false;

                for (BcsFile *bcs : races_bcs)
                {
                    const std::vector<BcsPartSet> &sets = bcs->GetPartSets();

                    if (this_partset < sets.size() && sets[this_partset].valid)
                    {
                        in_use = true;
                        break;
                    }
                }

                if (!in_use)
                {
                    if (game_costume_file->FindCostumeByPartSet(this_partset))
                        in_use = true;
                }

                if (in_use)
                {
                    num_found = 0;
                    continue;
                }

                if (num_found == 0)
                    partset = this_partset;

                num_found++;

                if (num_found == num)
                    break;
            }
        }

        if (num_found != num)
        {
            DPRINTF("%s: Overflow, cannot assign suitable(s) partset(s) id(s)\n", FUNCNAME);
            return false;
        }

        for (size_t i = 0; i < num; i++)
        {
            update_costume_entry.partsets.push_back(partset+(uint16_t)i);
        }

        if (update_costume_entry.races.size() != 0)
        {
            DPRINTF("%s: Internal error, races size should be zero in this point.\n", FUNCNAME);
            return false;
        }

        for (uint8_t i = 0; i < X2M_CR_NUM; i++)
        {
            if (GetNumCostumePartSets(i) > 0)
                update_costume_entry.races.push_back(i);
        }
    }
    else
    {
        if (num != update_costume_entry.partsets.size())
        {
            DPRINTF("%s: The function shouldn't have been called in this state.\n", FUNCNAME);
            return false;
        }
    }

    if (update_costume_entry.partsets.size() == 0)
    {
        DPRINTF("%s: num of partsets shouldn't have been 0 zt this point!\n", FUNCNAME);
        return 0;
    }

    for (size_t i = 0; i < update_costume_entry.partsets.size(); i++)
    {
        uint16_t partset = update_costume_entry.partsets[i];

        for (uint8_t race = 0; race < X2M_CR_NUM; race++)
        {
            if (race >= races_bcs.size())
            {
                DPRINTF("%s: Internal error on races array size.\n", FUNCNAME);
                return false;
            }

            if (GetNumCostumePartSets(race) > 0)
            {
                BcsFile *bcs = races_bcs[race];
                std::vector<BcsPartSet> &sets = bcs->GetPartSets();

                if (partset >= sets.size())
                {
                    bcs->GetPartSets().resize(partset+1);
                    sets = bcs->GetPartSets();
                }

                BcsPartSet &new_ps = GetCostumePartSet(race, i).bcs;

                ResolvePartSet(new_ps, update_costume_entry.partsets[0], (uint16_t)num);
                sets[partset] = new_ps;
            }
        }
    }

    return true;
}

static void RemoveCostumeNameReferences(uint16_t name_id)
{
    for (IdbEntry &entry : *game_top_idb)
    {
        if (entry.name_id == name_id)
            entry.name_id = 0xFFFF;
    }

    for (IdbEntry &entry : *game_bottom_idb)
    {
        if (entry.name_id == name_id)
            entry.name_id = 0xFFFF;
    }

    for (IdbEntry &entry : *game_gloves_idb)
    {
        if (entry.name_id == name_id)
            entry.name_id = 0xFFFF;
    }

    for (IdbEntry &entry : *game_shoes_idb)
    {
        if (entry.name_id == name_id)
            entry.name_id = 0xFFFF;
    }
}

static std::string ChooseCostumeEntryName(const IdbEntry &idb)
{
    std::string str = "wear_";

    if (idb.racelock == IDB_RACE_HUM || idb.racelock == IDB_RACE_SYM || idb.racelock == (IDB_RACE_HUM|IDB_RACE_SYM))
    {
        str += "hum_";
    }
    else if (idb.racelock == IDB_RACE_HUF || idb.racelock == IDB_RACE_SYF || idb.racelock == (IDB_RACE_HUF|IDB_RACE_SYF))
    {
        str += "huf_";
    }
    else if (idb.racelock == IDB_RACE_NMC)
    {
        str += "nmc_";
    }
    else if (idb.racelock == IDB_RACE_FRI)
    {
        str += "fri_";
    }
    else if (idb.racelock == IDB_RACE_MAM)
    {
        str += "mam_";
    }
    else if (idb.racelock == IDB_RACE_MAF)
    {
        str += "maf_";
    }
    else if (idb.racelock == (IDB_RACE_MAM|IDB_RACE_MAF))
    {
        str += "mar_";
    }
    else
    {
        str += "cmn_";
    }

    return str;
}

bool X2mFile::InstallCostumeCostumeNames()
{
    if (type != X2mType::NEW_COSTUME)
        return false;

    if (update_costume_entry.idb_entries.size() != 0 && update_costume_entry.idb_entries.size() != costume_items.size())
    {
        DPRINTF("%s: This function shouldn't have been called in this state.\n", FUNCNAME);
        return false;
    }

    if (update_costume_entry.partsets.size() == 0)
    {
        DPRINTF("%s: This function should have been called after InstallCostumePartSet.\n", FUNCNAME);
        return false;
    }

    // Delete any existing name before
    for (size_t i = 0; i < update_costume_entry.idb_entries.size(); i++)
    {
        uint16_t idb_id = update_costume_entry.idb_entries[i];
        uint8_t type = update_costume_entry.costume_types[i];

        if (type == COSTUME_ACCESSORY)
            continue;

        IdbEntry *entry = nullptr;

        if (type == COSTUME_TOP)
        {
            entry = game_top_idb->FindEntryByID(idb_id);
        }
        else if (type == COSTUME_BOTTOM)
        {
            entry = game_bottom_idb->FindEntryByID(idb_id);
        }
        else if (type == COSTUME_GLOVES)
        {
            entry = game_gloves_idb->FindEntryByID(idb_id);
        }
        else if (type == COSTUME_SHOES)
        {
            entry = game_shoes_idb->FindEntryByID(idb_id);
        }

        if (!entry)
        {
            DPRINTF("%s: Internal error, cannot find installed idb entry for update.\n", FUNCNAME);
            return false;
        }

        uint16_t name_id = entry->name_id;
        if (name_id == 0xFFFF)
            continue;

        RemoveCostumeNameReferences(name_id);

        int ls = 0, le = XV2_LANG_NUM;
        if (global_lang >= 0)
        {
            ls = global_lang;
            le = global_lang+1;
        }

        for (int l = ls; l < le; l++)
        {
            if (!Xenoverse2::RemoveCacCostumeName(name_id, l, (l==ls)))
            {
                DPRINTF("%s: Internal error, fail in RemoveCacCostumeName.\n", FUNCNAME);
                return false;
            }
        }
    }

    // Now add new names
    for (size_t i = 0; i < costume_items.size(); i++)
    {
        X2mItem &item = costume_items[i];
        bool name_found = false;

        if (item.item_type == X2mItemType::ACCESSORY)
            continue;

        for (size_t prev = 0; prev < i; prev++)
        {
            const X2mItem &prev_item = costume_items[prev];

            if (prev_item.item_type == X2mItemType::ACCESSORY)
                continue;

            if (prev_item.item_name == item.item_name)
            {
                item.idb.name_id = prev_item.idb.name_id;
                name_found = true;
                break;
            }
        }

        if (name_found)
            continue;

        int ls = 0, le = XV2_LANG_NUM;
        if (global_lang >= 0)
        {
            ls = global_lang;
            le = global_lang+1;
        }

        for (int l = ls; l < le; l++)
        {
            const std::string *name;

            if (item.item_name[l].length() != 0)
                name = &item.item_name[l];
            else
                name = &item.item_name[XV2_LANG_ENGLISH];            

            if (!Xenoverse2::AddCacCostumeName(ChooseCostumeEntryName(item.idb), *name, l, &item.idb.name_id))
            {
                DPRINTF("%s: AddCacCostumeName failed.\n", FUNCNAME);
                return false;
            }
        }
    }

    return true;
}

static void RemoveAccessoryNameReferences(uint16_t name_id)
{
    for (IdbEntry &entry : *game_accesory_idb)
    {
        if (entry.name_id == name_id)
            entry.name_id = 0xFFFF;
    }
}

bool X2mFile::InstallCostumeAccessoryNames()
{
    if (type != X2mType::NEW_COSTUME)
        return false;

    if (update_costume_entry.idb_entries.size() != 0 && update_costume_entry.idb_entries.size() != costume_items.size())
    {
        DPRINTF("%s: This function shouldn't have been called in this state.\n", FUNCNAME);
        return false;
    }

    if (update_costume_entry.partsets.size() == 0)
    {
        DPRINTF("%s: This function should have been called after InstallCostumePartSet.\n", FUNCNAME);
        return false;
    }

    // Delete any existing name before
    for (size_t i = 0; i < update_costume_entry.idb_entries.size(); i++)
    {
        uint16_t idb_id = update_costume_entry.idb_entries[i];
        uint8_t type = update_costume_entry.costume_types[i];

        if (type != COSTUME_ACCESSORY)
            continue;

        IdbEntry *entry = game_accesory_idb->FindEntryByID(idb_id);
        if (!entry)
        {
            DPRINTF("%s: Internal error, cannot find installed idb entry for update.\n", FUNCNAME);
            return false;
        }

        uint16_t name_id = entry->name_id;
        if (name_id == 0xFFFF)
            continue;

        RemoveAccessoryNameReferences(name_id);

        int ls = 0, le = XV2_LANG_NUM;
        if (global_lang >= 0)
        {
            ls = global_lang;
            le = global_lang+1;
        }

        for (int l = ls; l < le; l++)
        {
            if (!Xenoverse2::RemoveAccesoryName(name_id, l, (l==ls)))
            {
                DPRINTF("%s: Internal error, fail in RemoveAccesoryName.\n", FUNCNAME);
                return false;
            }
        }
    }

    // Now add new names
    for (size_t i = 0; i < costume_items.size(); i++)
    {
        X2mItem &item = costume_items[i];
        bool name_found = false;

        if (item.item_type != X2mItemType::ACCESSORY)
            continue;

        for (size_t prev = 0; prev < i; prev++)
        {
            const X2mItem &prev_item = costume_items[prev];

            if (prev_item.item_type != X2mItemType::ACCESSORY)
                continue;

            if (prev_item.item_name == item.item_name)
            {
                item.idb.name_id = prev_item.idb.name_id;
                name_found = true;
                break;
            }
        }

        if (name_found)
            continue;

        int ls = 0, le = XV2_LANG_NUM;
        if (global_lang >= 0)
        {
            ls = global_lang;
            le = global_lang+1;
        }

        for (int l = ls; l < le; l++)
        {
            const std::string *name;

            if (item.item_name[l].length() != 0)
                name = &item.item_name[l];
            else
                name = &item.item_name[XV2_LANG_ENGLISH];

            if (!Xenoverse2::AddAccesoryName(*name, l, &item.idb.name_id))
            {
                DPRINTF("%s: AddAccesoryName failed.\n", FUNCNAME);
                return false;
            }
        }
    }

    return true;
}

static void RemoveCostumeDescReferences(uint16_t desc_id)
{
    for (IdbEntry &entry : *game_top_idb)
    {
        if (entry.desc_id == desc_id)
            entry.desc_id = 0xFFFF;
    }

    for (IdbEntry &entry : *game_bottom_idb)
    {
        if (entry.desc_id == desc_id)
            entry.desc_id = 0xFFFF;
    }

    for (IdbEntry &entry : *game_gloves_idb)
    {
        if (entry.desc_id == desc_id)
            entry.desc_id = 0xFFFF;
    }

    for (IdbEntry &entry : *game_shoes_idb)
    {
        if (entry.desc_id == desc_id)
            entry.desc_id = 0xFFFF;
    }
}

static std::string ChooseCostumeEntryDesc(const IdbEntry &idb)
{
    std::string str = "wear_";

    if (idb.racelock == IDB_RACE_HUM || idb.racelock == IDB_RACE_SYM || idb.racelock == (IDB_RACE_HUM|IDB_RACE_SYM))
    {
        str += "hum_";
    }
    else if (idb.racelock == IDB_RACE_HUF || idb.racelock == IDB_RACE_SYF || idb.racelock == (IDB_RACE_HUF|IDB_RACE_SYF))
    {
        str += "huf_";
    }
    else if (idb.racelock == IDB_RACE_NMC)
    {
        str += "nmc_";
    }
    else if (idb.racelock == IDB_RACE_FRI)
    {
        str += "fri_";
    }
    else if (idb.racelock == IDB_RACE_MAM)
    {
        str += "mam_";
    }
    else if (idb.racelock == IDB_RACE_MAF)
    {
        str += "maf_";
    }
    else if (idb.racelock == (IDB_RACE_MAM|IDB_RACE_MAF))
    {
        str += "mar_";
    }
    else
    {
        str += "cmn_";
    }

    str += "eff_";
    return str;
}

bool X2mFile::InstallCostumeCostumeDescs()
{
    if (type != X2mType::NEW_COSTUME)
        return false;

    if (update_costume_entry.idb_entries.size() != 0 && update_costume_entry.idb_entries.size() != costume_items.size())
    {
        DPRINTF("%s: This function shouldn't have been called in this state.\n", FUNCNAME);
        return false;
    }

    if (update_costume_entry.partsets.size() == 0)
    {
        DPRINTF("%s: This function should have been called after InstallCostumePartSet.\n", FUNCNAME);
        return false;
    }

    // Delete any existing desc before
    for (size_t i = 0; i < update_costume_entry.idb_entries.size(); i++)
    {
        uint16_t idb_id = update_costume_entry.idb_entries[i];
        uint8_t type = update_costume_entry.costume_types[i];

        if (type == COSTUME_ACCESSORY)
            continue;

        IdbEntry *entry = nullptr;

        if (type == COSTUME_TOP)
        {
            entry = game_top_idb->FindEntryByID(idb_id);
        }
        else if (type == COSTUME_BOTTOM)
        {
            entry = game_bottom_idb->FindEntryByID(idb_id);
        }
        else if (type == COSTUME_GLOVES)
        {
            entry = game_gloves_idb->FindEntryByID(idb_id);
        }
        else if (type == COSTUME_SHOES)
        {
            entry = game_shoes_idb->FindEntryByID(idb_id);
        }

        if (!entry)
        {
            DPRINTF("%s: Internal error, cannot find installed idb entry for update.\n", FUNCNAME);
            return false;
        }

        uint16_t desc_id = entry->desc_id;
        if (desc_id == 0xFFFF)
            continue;

        RemoveCostumeDescReferences(desc_id);

        int ls = 0, le = XV2_LANG_NUM;
        if (global_lang >= 0)
        {
            ls = global_lang;
            le = global_lang+1;
        }

        for (int l = ls; l < le; l++)
        {
            if (!Xenoverse2::RemoveCacCostumeDesc(desc_id, l, (l==ls)))
            {
                DPRINTF("%s: Internal error, fail in RemoveCacCostumeDesc.\n", FUNCNAME);
                return false;
            }
        }
    }

    // Now add new descs
    for (size_t i = 0; i < costume_items.size(); i++)
    {
        X2mItem &item = costume_items[i];
        bool desc_found = false;

        if (item.item_type == X2mItemType::ACCESSORY)
            continue;

        for (size_t prev = 0; prev < i; prev++)
        {
            const X2mItem &prev_item = costume_items[prev];

            if (prev_item.item_type == X2mItemType::ACCESSORY)
                continue;

            if (prev_item.item_desc == item.item_desc)
            {
                item.idb.desc_id = prev_item.idb.desc_id;
                desc_found = true;
                break;
            }
        }

        if (desc_found)
            continue;

        int ls = 0, le = XV2_LANG_NUM;
        if (global_lang >= 0)
        {
            ls = global_lang;
            le = global_lang+1;
        }

        for (int l = ls; l < le; l++)
        {
            const std::string *desc;

            if (item.item_desc[l].length() != 0)
                desc = &item.item_desc[l];
            else
                desc = &item.item_desc[XV2_LANG_ENGLISH];

            if (!Xenoverse2::AddCacCostumeDesc(ChooseCostumeEntryDesc(item.idb), *desc, l, &item.idb.desc_id))
            {
                DPRINTF("%s: AddCacCostumeDesc failed.\n", FUNCNAME);
                return false;
            }
        }
    }

    return true;
}

static void RemoveAccessoryDescReferences(uint16_t desc_id)
{
    for (IdbEntry &entry : *game_accesory_idb)
    {
        if (entry.desc_id == desc_id)
            entry.desc_id = 0xFFFF;
    }
}

bool X2mFile::InstallCostumeAccessoryDescs()
{
    if (type != X2mType::NEW_COSTUME)
        return false;

    if (update_costume_entry.idb_entries.size() != 0 && update_costume_entry.idb_entries.size() != costume_items.size())
    {
        DPRINTF("%s: This function shouldn't have been called in this state.\n", FUNCNAME);
        return false;
    }

    if (update_costume_entry.partsets.size() == 0)
    {
        DPRINTF("%s: This function should have been called after InstallCostumePartSet.\n", FUNCNAME);
        return false;
    }

    // Delete any existing desc before
    for (size_t i = 0; i < update_costume_entry.idb_entries.size(); i++)
    {
        uint16_t idb_id = update_costume_entry.idb_entries[i];
        uint8_t type = update_costume_entry.costume_types[i];

        if (type != COSTUME_ACCESSORY)
            continue;

        IdbEntry *entry = game_accesory_idb->FindEntryByID(idb_id);
        if (!entry)
        {
            DPRINTF("%s: Internal error, cannot find installed idb entry for update.\n", FUNCNAME);
            return false;
        }

        uint16_t desc_id = entry->desc_id;
        if (desc_id == 0xFFFF)
            continue;

        RemoveAccessoryDescReferences(desc_id);

        int ls = 0, le = XV2_LANG_NUM;
        if (global_lang >= 0)
        {
            ls = global_lang;
            le = global_lang+1;
        }

        for (int l = ls; l < le; l++)
        {
            if (!Xenoverse2::RemoveAccesoryDesc(desc_id, l, (l==ls)))
            {
                DPRINTF("%s: Internal error, fail in RemoveAccesoryDesc.\n", FUNCNAME);
                return false;
            }
        }
    }

    // Now add new descs
    for (size_t i = 0; i < costume_items.size(); i++)
    {
        X2mItem &item = costume_items[i];
        bool desc_found = false;

        if (item.item_type != X2mItemType::ACCESSORY)
            continue;

        for (size_t prev = 0; prev < i; prev++)
        {
            const X2mItem &prev_item = costume_items[prev];

            if (prev_item.item_type != X2mItemType::ACCESSORY)
                continue;

            if (prev_item.item_desc == item.item_desc)
            {
                item.idb.desc_id = prev_item.idb.desc_id;
                desc_found = true;
                break;
            }
        }

        if (desc_found)
            continue;

        int ls = 0, le = XV2_LANG_NUM;
        if (global_lang >= 0)
        {
            ls = global_lang;
            le = global_lang+1;
        }

        for (int l = ls; l < le; l++)
        {
            const std::string *desc;

            if (item.item_desc[l].length() != 0)
                desc = &item.item_desc[l];
            else
                desc = &item.item_desc[XV2_LANG_ENGLISH];

            if (!Xenoverse2::AddAccesoryDesc(*desc, l, &item.idb.desc_id))
            {
                DPRINTF("%s: AddAccesoryDesc failed.\n", FUNCNAME);
                return false;
            }
        }
    }

    return true;
}

bool X2mFile::AddCostumeIdb(IdbFile *cost_idb, IdbEntry &entry, uint16_t id_start)
{
    static const std::vector<IdbFile *> idb_files =
    {
        game_top_idb, game_bottom_idb, game_gloves_idb, game_shoes_idb
    };

    for (entry.id = id_start; entry.id != 0x8000; entry.id++)
    {
        bool found = true;

        for (IdbFile *idb : idb_files)
        {
            if (idb->FindEntryByID(entry.id))
            {
                found = false;
                break;
            }
        }

        if (found)
        {
            return cost_idb->AddEntry(entry,  false);
        }
    }

    return false;
}

BcsFile *X2mFile::GetBcsForRace(uint32_t race)
{
    if (race == X2M_CR_HUM_SYM)
        return game_hum_bcs;

    else if (race == X2M_CR_HUF_SYF)
        return game_huf_bcs;

    else if (race == X2M_CR_NMC)
        return game_nmc_bcs;

    else if (race == X2M_CR_FRI)
        return game_fri_bcs;

    else if (race == X2M_CR_MAM)
        return game_mam_bcs;

    else if (race == X2M_CR_MAF)
        return game_maf_bcs;

    return game_hum_bcs;
}

bool X2mFile::InstallCostumeIdb()
{
    if (type != X2mType::NEW_COSTUME)
        return false;

    if (update_costume_entry.partsets.size() == 0)
    {
        DPRINTF("%s: This function should have been called after InstallCostumePartSet.\n", FUNCNAME);
        return false;
    }

    // Assign partsets before
    for (X2mItem &item : costume_items)
    {
        item.idb.model += update_costume_entry.partsets[0];
    }

    static const uint16_t idb_start_costume = 0x10C;
    static const uint16_t idb_start_accessory = 0x6C;

    if (update_costume_entry.idb_entries.size() == 0)
    {
        if (costume_items.size() == 0)
            return true;

        for (size_t i = 0; i < costume_items.size(); i++)
        {
            X2mItem &item = costume_items[i];
            X2mItemType type = item.item_type;

            if (item.idb.name_id == 0xFFFF || item.idb.name_id == X2M_INVALID_ID16 || item.idb.name_id == X2M_DUMMY_ID16)
            {
                DPRINTF("%s: Name should have been assigned before.\n", FUNCNAME);
                return false;
            }

            if (item.idb.desc_id == 0xFFFF || item.idb.desc_id == X2M_INVALID_ID16 || item.idb.desc_id == X2M_DUMMY_ID16)
            {
                DPRINTF("%s: Desc should have been assigned before.\n", FUNCNAME);
                return false;
            }

            if (type == X2mItemType::TOP)
            {
                if (!AddCostumeIdb(game_top_idb, item.idb, idb_start_costume))
                {
                    DPRINTF("%s: AddCostumeIdb failed.\n", FUNCNAME);
                    return false;
                }

                update_costume_entry.costume_types.push_back(COSTUME_TOP);
            }
            else if (type == X2mItemType::BOTTOM)
            {
                if (!AddCostumeIdb(game_bottom_idb, item.idb, idb_start_costume))
                {
                    DPRINTF("%s: AddCostumeIdb failed.\n", FUNCNAME);
                    return false;
                }

                update_costume_entry.costume_types.push_back(COSTUME_BOTTOM);
            }
            else if (type == X2mItemType::GLOVES)
            {
                if (!AddCostumeIdb(game_gloves_idb, item.idb, idb_start_costume))
                {
                    DPRINTF("%s: AddCostumeIdb failed.\n", FUNCNAME);
                    return false;
                }
                update_costume_entry.costume_types.push_back(COSTUME_GLOVES);
            }
            else if (type == X2mItemType::SHOES)
            {
                if (!AddCostumeIdb(game_shoes_idb, item.idb, idb_start_costume))
                {
                    DPRINTF("%s: AddCostumeIdb failed.\n", FUNCNAME);
                    return false;
                }

                update_costume_entry.costume_types.push_back(COSTUME_SHOES);
            }
            else
            {
                if (!game_accesory_idb->AddEntryAuto(item.idb, idb_start_accessory))
                {
                    DPRINTF("%s: AddEntryAuto failed.\n", FUNCNAME);
                    return false;
                }

                update_costume_entry.costume_types.push_back(COSTUME_ACCESSORY);
            }

            update_costume_entry.idb_entries.push_back(item.idb.id);
        }
    }
    else
    {
        if (update_costume_entry.idb_entries.size() != costume_items.size())
        {
            DPRINTF("%s: This function shouldn't have been called in this state.\n", FUNCNAME);
            return false;
        }

        for (size_t i = 0; i < costume_items.size(); i++)
        {
            X2mItem &item = costume_items[i];
            X2mItemType type = item.item_type;

            if (item.idb.name_id == 0xFFFF || item.idb.name_id == X2M_INVALID_ID16 || item.idb.name_id == X2M_DUMMY_ID16)
            {
                DPRINTF("%s: Name should have been assigned before (on update).\n", FUNCNAME);
                return false;
            }

            if (item.idb.desc_id == 0xFFFF || item.idb.desc_id == X2M_INVALID_ID16 || item.idb.desc_id == X2M_DUMMY_ID16)
            {
                DPRINTF("%s: Desc should have been assigned before (on update).\n", FUNCNAME);
                return false;
            }

            if (type == X2mItemType::TOP)
            {
                if (update_costume_entry.costume_types[i] != COSTUME_TOP)
                {
                    DPRINTF("%s: Mismatch on update costume type, this should have been caught before.\n", FUNCNAME);
                    return false;
                }

                item.idb.id = update_costume_entry.idb_entries[i];
                IdbEntry *entry = game_top_idb->FindEntryByID(item.idb.id);
                if (!entry)
                {
                    DPRINTF("%s: Failed to get existing idb entry, this should have been caught before.\n", FUNCNAME);
                    return false;
                }

                *entry = item.idb; // Update
            }
            else if (type == X2mItemType::BOTTOM)
            {
                if (update_costume_entry.costume_types[i] != COSTUME_BOTTOM)
                {
                    DPRINTF("%s: Mismatch on update costume type, this should have been caught before.\n", FUNCNAME);
                    return false;
                }

                item.idb.id = update_costume_entry.idb_entries[i];
                IdbEntry *entry = game_bottom_idb->FindEntryByID(item.idb.id);
                if (!entry)
                {
                    DPRINTF("%s: Failed to get existing idb entry, this should have been caught before.\n", FUNCNAME);
                    return false;
                }

                *entry = item.idb; // Update
            }
            else if (type == X2mItemType::GLOVES)
            {
                if (update_costume_entry.costume_types[i] != COSTUME_GLOVES)
                {
                    DPRINTF("%s: Mismatch on update costume type, this should have been caught before.\n", FUNCNAME);
                    return false;
                }

                item.idb.id = update_costume_entry.idb_entries[i];
                IdbEntry *entry = game_gloves_idb->FindEntryByID(item.idb.id);
                if (!entry)
                {
                    DPRINTF("%s: Failed to get existing idb entry, this should have been caught before.\n", FUNCNAME);
                    return false;
                }

                *entry = item.idb; // Update
            }
            else if (type == X2mItemType::SHOES)
            {
                if (update_costume_entry.costume_types[i] != COSTUME_SHOES)
                {
                    DPRINTF("%s: Mismatch on update costume type, this should have been caught before.\n", FUNCNAME);
                    return false;
                }

                item.idb.id = update_costume_entry.idb_entries[i];
                IdbEntry *entry = game_shoes_idb->FindEntryByID(item.idb.id);
                if (!entry)
                {
                    DPRINTF("%s: Failed to get existing idb entry, this should have been caught before.\n", FUNCNAME);
                    return false;
                }

                *entry = item.idb; // Update
            }
            else
            {
                if (update_costume_entry.costume_types[i] != COSTUME_ACCESSORY)
                {
                    DPRINTF("%s: Mismatch on update costume type, this should have been caught before.\n", FUNCNAME);
                    return false;
                }

                item.idb.id = update_costume_entry.idb_entries[i];
                IdbEntry *entry = game_accesory_idb->FindEntryByID(item.idb.id);
                if (!entry)
                {
                    DPRINTF("%s: Failed to get existing idb entry, this should have been caught before.\n", FUNCNAME);
                    return false;
                }

                *entry = item.idb; // Update
            }
        }
    }

    return true;
}

bool X2mFile::InstallCostumeFile()
{
    if (type != X2mType::NEW_COSTUME)
        return false;

    if (update_costume_entry.partsets.size() == 0)
    {
        DPRINTF("%s: This function should have been called after InstallCostumePartSet.\n", FUNCNAME);
        return false;
    }

    memcpy(update_costume_entry.guid, mod_guid, sizeof(mod_guid));
    game_costume_file->AddCostume(update_costume_entry);

    return true;
}

bool X2mFile::InstallCostumeVisitor(const std::string &path, void *param)
{
    std::string name = Utils::GetFileNameString(path);
    std::string output_path = "data/chara/" + path.substr(0, 3) + "/";
    X2mFile *pthis = (X2mFile *)param;

    if (!ResolvePartSetString(name, pthis->update_costume_entry.partsets[0], (uint16_t)pthis->update_costume_entry.partsets.size()))
        return true; // Skip this file

    output_path += name;

    size_t size;
    uint8_t *buf;

    if (IsDummyMode())
    {
        buf = pthis->RecoverCostumeFile(path.substr(0, 3), Utils::GetFileNameString(path), (uint16_t)pthis->update_costume_entry.partsets.size(), &size);
        if (!buf)
        {
            DPRINTF("%s: Cannot recover costume file (local path=%s)", FUNCNAME, path.c_str());
            return false;
        }
    }
    else
    {
        buf = pthis->ReadFile(path, &size);
        if (!buf)
            return false;
    }

    if (!xv2fs->WriteFile(output_path, buf, size))
        return false;

    return true;
}

bool X2mFile::InstallCostumeFiles()
{
    if (type != X2mType::NEW_COSTUME)
        return false;

    if (update_costume_entry.partsets.size() == 0)
    {
        DPRINTF("%s: This function should have been called after InstallCostumePartSet.\n", FUNCNAME);
        return false;
    }

    for (uint8_t race = 0; race < X2M_CR_NUM; race++)
    {
        if (GetNumCostumePartSets(race) > 0)
        {
            if (DirExists(x2m_cr_code[race]))
            {
                if (!VisitDirectory(x2m_cr_code[race], InstallCostumeVisitor, this))
                    return false;
            }
        }
    }

    return true;
}

bool X2mFile::UninstallCostumePartSets()
{
    if (type != X2mType::NEW_COSTUME)
        return false;

    for (uint16_t partset : update_costume_entry.partsets)
    {
        for (uint8_t race : update_costume_entry.races)
        {
            BcsFile *bcs;

            if (race == X2M_CR_HUM_SYM)
                bcs = game_hum_bcs;
            else if (race == X2M_CR_HUF_SYF)
                bcs = game_huf_bcs;
            else if (race == X2M_CR_NMC)
                bcs = game_nmc_bcs;
            else if (race == X2M_CR_FRI)
                bcs = game_fri_bcs;
            else if (race == X2M_CR_MAM)
                bcs = game_mam_bcs;
            else
                bcs = game_maf_bcs;

            std::vector<BcsPartSet> &partsets = bcs->GetPartSets();

            if (partset < partsets.size())
            {
                partsets[partset].valid = false;
            }
        }
    }

    return true;
}

bool X2mFile::UninstallCostumeCostumeNames()
{
    if (type != X2mType::NEW_COSTUME)
        return false;

    if (update_costume_entry.idb_entries.size() != update_costume_entry.costume_types.size())
        return false;

    for (size_t i = 0; i < update_costume_entry.idb_entries.size(); i++)
    {
        uint16_t id = update_costume_entry.idb_entries[i];
        uint8_t type = update_costume_entry.costume_types[i];

        if (type == COSTUME_ACCESSORY)
            continue;

        IdbFile *idb;

        if (type == COSTUME_TOP)
            idb = game_top_idb;
        else if (type == COSTUME_BOTTOM)
            idb = game_bottom_idb;
        else if (type == COSTUME_GLOVES)
            idb = game_gloves_idb;
        else
            idb = game_shoes_idb;

        IdbEntry *entry = idb->FindEntryByID(id);
        if (entry)
        {
            uint16_t name_id = entry->name_id;
            if (name_id == 0xFFFF)
                continue;

            RemoveCostumeNameReferences(name_id);

            int ls = 0, le = XV2_LANG_NUM;
            if (global_lang >= 0)
            {
                ls = global_lang;
                le = global_lang+1;
            }

            for (int l = ls; l < le; l++)
            {
                if (!Xenoverse2::RemoveCacCostumeName(name_id, l, (l==ls)))
                {
                    DPRINTF("%s: RemoveCacCostumeName failed.\n", FUNCNAME);
                    return false;
                }
            }

        }
    }

    return true;
}

bool X2mFile::UninstallCostumeAccessoryNames()
{
    if (type != X2mType::NEW_COSTUME)
        return false;

    if (update_costume_entry.idb_entries.size() != update_costume_entry.costume_types.size())
        return false;

    for (size_t i = 0; i < update_costume_entry.idb_entries.size(); i++)
    {
        uint16_t id = update_costume_entry.idb_entries[i];
        uint8_t type = update_costume_entry.costume_types[i];

        if (type != COSTUME_ACCESSORY)
            continue;

        IdbEntry *entry = game_accesory_idb->FindEntryByID(id);
        if (entry)
        {
            uint16_t name_id = entry->name_id;
            if (name_id == 0xFFFF)
                continue;

            RemoveAccessoryNameReferences(name_id);

            int ls = 0, le = XV2_LANG_NUM;
            if (global_lang >= 0)
            {
                ls = global_lang;
                le = global_lang+1;
            }

            for (int l = ls; l < le; l++)
            {
                if (!Xenoverse2::RemoveAccesoryName(name_id, l, (l==ls)))
                {
                    DPRINTF("%s: RemoveAccesoryName failed.\n", FUNCNAME);
                    return false;
                }
            }

        }
    }

    return true;
}

bool X2mFile::UninstallCostumeCostumeDescs()
{
    if (type != X2mType::NEW_COSTUME)
        return false;

    if (update_costume_entry.idb_entries.size() != update_costume_entry.costume_types.size())
        return false;

    for (size_t i = 0; i < update_costume_entry.idb_entries.size(); i++)
    {
        uint16_t id = update_costume_entry.idb_entries[i];
        uint8_t type = update_costume_entry.costume_types[i];

        if (type == COSTUME_ACCESSORY)
            continue;

        IdbFile *idb;

        if (type == COSTUME_TOP)
            idb = game_top_idb;
        else if (type == COSTUME_BOTTOM)
            idb = game_bottom_idb;
        else if (type == COSTUME_GLOVES)
            idb = game_gloves_idb;
        else
            idb = game_shoes_idb;

        IdbEntry *entry = idb->FindEntryByID(id);
        if (entry)
        {
            uint16_t desc_id = entry->desc_id;
            if (desc_id == 0xFFFF)
                continue;

            RemoveCostumeDescReferences(desc_id);

            int ls = 0, le = XV2_LANG_NUM;
            if (global_lang >= 0)
            {
                ls = global_lang;
                le = global_lang+1;
            }

            for (int l = ls; l < le; l++)
            {
                if (!Xenoverse2::RemoveCacCostumeDesc(desc_id, l, (l==ls)))
                {
                    DPRINTF("%s: RemoveCacCostumeDesc failed.\n", FUNCNAME);
                    return false;
                }
            }

        }
    }

    return true;
}

bool X2mFile::UninstallCostumeAccessoryDescs()
{
    if (type != X2mType::NEW_COSTUME)
        return false;

    if (update_costume_entry.idb_entries.size() != update_costume_entry.costume_types.size())
        return false;

    for (size_t i = 0; i < update_costume_entry.idb_entries.size(); i++)
    {
        uint16_t id = update_costume_entry.idb_entries[i];
        uint8_t type = update_costume_entry.costume_types[i];

        if (type != COSTUME_ACCESSORY)
            continue;

        IdbEntry *entry = game_accesory_idb->FindEntryByID(id);
        if (entry)
        {
            uint16_t desc_id = entry->desc_id;
            if (desc_id == 0xFFFF)
                continue;

            RemoveAccessoryDescReferences(desc_id);

            int ls = 0, le = XV2_LANG_NUM;
            if (global_lang >= 0)
            {
                ls = global_lang;
                le = global_lang+1;
            }

            for (int l = ls; l < le; l++)
            {
                if (!Xenoverse2::RemoveAccesoryDesc(desc_id, l, (l==ls)))
                {
                    DPRINTF("%s: RemoveAccesoryDesc failed.\n", FUNCNAME);
                    return false;
                }
            }

        }
    }

    return true;
}

bool X2mFile::UninstallCostumeIdb()
{
    if (type != X2mType::NEW_COSTUME)
        return false;

    if (update_costume_entry.idb_entries.size() != update_costume_entry.costume_types.size())
        return false;

    for (size_t i = 0; i < update_costume_entry.idb_entries.size(); i++)
    {
        uint16_t id = update_costume_entry.idb_entries[i];
        uint8_t type = update_costume_entry.costume_types[i];

        IdbFile *idb;

        if (type == COSTUME_TOP)
            idb = game_top_idb;
        else if (type == COSTUME_BOTTOM)
            idb = game_bottom_idb;
        else if (type == COSTUME_GLOVES)
            idb = game_gloves_idb;
        else if (type == COSTUME_SHOES)
            idb = game_shoes_idb;
        else
            idb = game_accesory_idb;

        idb->RemoveEntry(id);
    }

    return true;
}

bool X2mFile::UninstallCostumeFile()
{
    if (type != X2mType::NEW_COSTUME)
        return false;

    game_costume_file->RemoveCostume(mod_guid);
    return true;
}

bool X2mFile::UninstallCostumeVisitor(const std::string &path, void *param)
{
    std::string name = Utils::GetFileNameString(path);
    std::string delete_path = "data/chara/" + path.substr(0, 3) + "/";
    X2mFile *pthis = (X2mFile *)param;

    if (!ResolvePartSetString(name, pthis->update_costume_entry.partsets[0], (uint16_t)pthis->update_costume_entry.partsets.size()))
        return true; // Skip this file

    delete_path += name;
    xv2fs->RemoveFile(delete_path);

    return true;
}

bool X2mFile::UninstallCostumeFiles()
{
    if (type != X2mType::NEW_COSTUME)
        return false;

    if (update_costume_entry.partsets.size() == 0)
        return true; // Yes, true

    for (uint8_t race : update_costume_entry.races)
    {
        if (DirExists(x2m_cr_code[race]))
        {
            if (!VisitDirectory(x2m_cr_code[race], UninstallCostumeVisitor, this))
                return false;
        }
    }

    return true;
}

bool X2mFile::InstallStageDef()
{
    if (type != X2mType::NEW_STAGE)
        return false;

    size_t ret = game_stage_def->AddStage(stage_def, true);
    if (ret == (size_t)-1)
    {
        DPRINTF("%s: AddStage failed.\n", FUNCNAME);
        return false;
    }

    return true;
}

bool X2mFile::InstallStageName()
{
    if (type != X2mType::NEW_STAGE)
        return false;

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        std::string *name;

        if (i == XV2_LANG_ENGLISH)
        {
            if (stage_name[i].length() == 0)
                return false;

            name = &stage_name[i];
        }
        else
        {
            if (stage_name[i].length() == 0)
            {
                name = &stage_name[XV2_LANG_ENGLISH];
            }
            else
            {
                name = &stage_name[i];
            }
        }

        if (!Xenoverse2::SetStageName(stage_def.code, *name, i))
            return false;
    }

    return true;
}

bool X2mFile::InstallStageSelImages()
{
    if (type != X2mType::NEW_STAGE)
        return false;

    if (!game_stage02_emb)
    {
        if (!Xenoverse2::InitStageEmb())
            return false;
    }

    const std::string portrait_name = std::string("ICO") + stage_def.code + ".dds";

    size_t portrait_size;
    uint8_t *portrait_buf;

    if (IsDummyMode())
    {
        portrait_buf = RecoverStageSelPortrait(&portrait_size);
        if (!portrait_buf)
        {
            DPRINTF("%s: Failed to recover stage sel portrait.", FUNCNAME);
            return false;
        }
    }
    else
    {
        portrait_buf = ReadFile(X2M_STAGE_SEL_PORTRAIT, &portrait_size);
        if (!portrait_buf)
        {
            DPRINTF("%s: Failed because cannot load portrait from package.\n", FUNCNAME);
            return false;
        }
    }

    uint16_t idx = game_stage02_emb->FindIndexByName(portrait_name);
    if (idx == (uint16_t)-1)
    {
        game_stage02_emb->AppendFile(portrait_buf, portrait_size, portrait_name, true);
    }
    else
    {
        EmbContainedFile &file = (*game_stage02_emb)[idx];
        file.SetData(portrait_buf, portrait_size, true);
    }

    const std::string background_name = stage_def.code + ".dds";

    size_t background_size;
    uint8_t *background_buf;

    if (IsDummyMode())
    {
        background_buf = RecoverStageBackground(&background_size);
        if (!background_buf)
        {
            DPRINTF("%s: Failed to recover stage background.", FUNCNAME);
            return false;
        }
    }
    else
    {
        background_buf = ReadFile(X2M_STAGE_SEL_BACKGROUND, &background_size);
        if (!background_buf)
        {
            DPRINTF("%s: Failed because cannot load background from package.\n", FUNCNAME);
            return false;
        }
    }

    idx = game_stage02_emb->FindIndexByName(background_name);
    if (idx == (uint16_t)-1)
    {
        game_stage02_emb->AppendFile(background_buf, background_size, background_name, true);
    }
    else
    {
        EmbContainedFile &file = (*game_stage02_emb)[idx];
        file.SetData(background_buf, background_size, true);
    }

    return true;
}

bool X2mFile::InstallStageQstPortrait()
{
    if (type != X2mType::NEW_STAGE)
        return false;

    if (!game_stage01_emb)
    {
        if (!Xenoverse2::InitStageEmb())
            return false;
    }

    const std::string portrait_name = stage_def.code + ".dds";

    size_t portrait_size;
    uint8_t *portrait_buf;

    if (IsDummyMode())
    {
        portrait_buf = RecoverStageQstPortrait(&portrait_size);
        if (!portrait_buf)
        {
            DPRINTF("%s: Failed to recover stage qst portrait.", FUNCNAME);
            return false;
        }
    }
    else
    {
        portrait_buf = ReadFile(X2M_STAGE_QST_PORTRAIT, &portrait_size);
        if (!portrait_buf)
        {
            DPRINTF("%s: Failed because cannot load qst portrait from package.\n", FUNCNAME);
            return false;
        }
    }

    uint16_t idx = game_stage01_emb->FindIndexByName(portrait_name);
    if (idx == (uint16_t)-1)
    {
        game_stage01_emb->AppendFile(portrait_buf, portrait_size, portrait_name, true);
    }
    else
    {
        EmbContainedFile &file = (*game_stage01_emb)[idx];
        file.SetData(portrait_buf, portrait_size, true);
    }

    return true;
}

bool X2mFile::InstallStageSlot()
{
    if (type != X2mType::NEW_STAGE)
        return false;

    if (stage_def.ssid < 0)
    {
        DPRINTF("%s: You cannot call me before a succesful call to InstallStageDef.\n", FUNCNAME);
        return false;
    }

    if (!add_stage_slot)
    {
        // For the rare cases of an update were the old one had slots and the new doesn't, we must delete the slots (if any)
        game_stage_slots_file->RemoveSlots(stage_def.ssid);
        game_stage_slots_file_local->RemoveSlots(stage_def.ssid);
        return true;
    }

    std::vector<Xv2StageSlot *> entries;
    game_stage_slots_file->FindSlots(stage_def.ssid, entries);

    if (entries.size() == 0)
    {
        std::vector<Xv2StageSlot> &stage_slots = game_stage_slots_file->GetSlots();

        Xv2StageSlot new_slot;
        new_slot.stage = stage_def.ssid;
        new_slot.dlc = 0;

        stage_slots.push_back(new_slot);
    }

    if (!add_stage_slot_local)
    {
        // For the rare cases of an update were the old one had slots and the new doesn't, we must delete the slots (if any)
        game_stage_slots_file_local->RemoveSlots(stage_def.ssid);
        return true;
    }

    game_stage_slots_file_local->FindSlots(stage_def.ssid, entries);

    if (entries.size() == 0)
    {
        std::vector<Xv2StageSlot> &stage_slots = game_stage_slots_file_local->GetSlots();

        Xv2StageSlot new_slot;
        new_slot.stage = stage_def.ssid;
        new_slot.dlc = 0;

        stage_slots.push_back(new_slot);
    }

    return true;
}

bool X2mFile::InstallStageVfx()
{
    if (type != X2mType::NEW_STAGE)
        return false;

    size_t idx;
    if (!game_stage_def->GetStageByCode(stage_def.code, &idx))
        return false;

    if (!game_ers)
    {
        if (!Xenoverse2::InitVfx())
            return false;
    }

    if (bg_eepk.length() == 0)
    {
        game_ers->RemoveStageBgEepk(idx);
    }
    else
    {
        if (!game_ers->SetStageBgEepk(stage_def.code, idx, bg_eepk))
            return false;
    }

    if (stage_eepk.length() == 0)
    {
        game_ers->RemoveStageEepk(idx);
    }
    else
    {
        if (!game_ers->SetStageEepk(stage_def.code, idx, stage_eepk))
            return false;
    }

    return true;
}

bool X2mFile::InstallStageBgm()
{
    if (type != X2mType::NEW_STAGE)
        return false;

    if (!StageHasCustomBgm())
    {
        UninstallStageBgm(); // This is in case of an update where the old one has bgm but new one doesn't.
        return true;
    }

    Xv2Stage *def = game_stage_def->GetStageByCode(stage_def.code);
    if (!def)
        return false;

    HcaFile *hca = GetStageBgm();
    if (!hca)
        return false;

    uint32_t cue_id = Xenoverse2::SetBgmSound("X2M_" + stage_def.code, *hca);
    delete hca;

    if (cue_id == (uint32_t)-1)
        return false;

    def->bgm_cue_id = cue_id;
    return true;
}

bool X2mFile::InstallStageFiles()
{
    if (type != X2mType::NEW_STAGE)
        return false;

    if (!StageDirectoryExists())
        return false;

    if (!InstallDir(X2M_STAGE, "data/stage/"))
        return false;

    return true;
}

bool X2mFile::InstallStageLighting()
{
    if (type != X2mType::NEW_STAGE)
        return false;

    if (!StageLightingExists())
        return false;

    size_t size;
    uint8_t *buf;

    if (IsDummyMode())
    {
        buf = RecoverStageLighting(&size);
        if (!buf)
        {
            DPRINTF("%s: Cannot recover stage lighting.", FUNCNAME);
            return false;
        }
    }
    else
    {
        buf = ReadFile(X2M_STAGE_LIGHTING, &size);
        if (!buf)
            return false;
    }

    std::string emb_path = std::string("data/lighting/environment/") + stage_def.code + ".emb";
    bool ret = xv2fs->WriteFile(emb_path, buf, size);

    delete[] buf;
    return ret;
}

bool X2mFile::UninstallStageDef()
{
    if (type != X2mType::NEW_STAGE)
        return false;

    game_stage_def->RemoveStage(stage_def);
    return true;
}

bool X2mFile::UninstallStageName()
{
    if (type != X2mType::NEW_STAGE)
        return false;

    // Stage names go in def, we don't need to do this
    /*for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        if (!Xenoverse2::RemoveStageName(stage_def.code, i))
            return false;
    }*/

    return true;
}

bool X2mFile::UninstallStageSelImages()
{
    if (type != X2mType::NEW_STAGE)
        return false;

    if (!game_stage02_emb)
    {
        if (!Xenoverse2::InitStageEmb())
            return false;
    }

    const std::string portrait_name = std::string("ICO") + stage_def.code + ".dds";
    uint16_t idx = game_stage02_emb->FindIndexByName(portrait_name);

    if (idx != (uint16_t)-1)
    {
        game_stage02_emb->RemoveFile(idx);
    }

    const std::string background_name = stage_def.code + ".dds";
    idx = game_stage02_emb->FindIndexByName(background_name);

    if (idx != (uint16_t)-1)
    {
        game_stage02_emb->RemoveFile(idx);
    }

    return true;
}

bool X2mFile::UninstallStageQstPortrait()
{
    if (type != X2mType::NEW_STAGE)
        return false;

    if (!game_stage01_emb)
    {
        if (!Xenoverse2::InitStageEmb())
            return false;
    }

    const std::string portrait_name = stage_def.code + ".dds";
    uint16_t idx = game_stage01_emb->FindIndexByName(portrait_name);

    if (idx != (uint16_t)-1)
    {
        game_stage01_emb->RemoveFile(idx);
    }

    return true;
}

bool X2mFile::UninstallStageSlot()
{
    if (type != X2mType::NEW_STAGE)
        return false;

    if (stage_def.ssid < 0)
        return true;

    game_stage_slots_file->RemoveSlots(stage_def.ssid);
    game_stage_slots_file_local->RemoveSlots(stage_def.ssid);

    return true;
}

bool X2mFile::UninstallStageVfx()
{
    if (type != X2mType::NEW_STAGE)
        return false;

    size_t idx;
    if (!game_stage_def->GetStageByCode(stage_def.code, &idx))
        return true; // Yes, true

    if (!game_ers)
    {
        if (!Xenoverse2::InitVfx())
            return false;
    }

    game_ers->RemoveStageBgEepk(idx);
    game_ers->RemoveStageEepk(idx);

    return true;
}

bool X2mFile::UninstallStageBgm()
{
    if (type != X2mType::NEW_STAGE)
        return false;

    std::string cue_name = "X2M_" + stage_def.code;

    if (Xenoverse2::GetBgmCueId(cue_name) == (uint32_t)-1)
        return true; // No custom sound, leave it alone and return success

    return Xenoverse2::FreeBgmSound(cue_name);
}

bool X2mFile::UninstallStageFiles()
{
    if (type != X2mType::NEW_STAGE)
        return false;

    if (!StageDirectoryExists())
        return false;

    if (!UninstallDir(X2M_STAGE, "data/stage/"))
        return false;

    return true;
}

bool X2mFile::UninstallStageLighting()
{
    if (type != X2mType::NEW_STAGE)
        return false;

    std::string emb_path = std::string("data/lighting/environment/") + stage_def.code + ".emb";
    xv2fs->RemoveFile(emb_path); // Ignore error

    return true;
}

static void RemoveSSNameReferences(uint16_t name_id)
{
    for (IdbEntry &entry : *game_talisman_idb)
    {
        if (entry.name_id == name_id)
            entry.name_id = 0xFFFF;
    }
}

static void RemoveSSDescReferences(uint16_t desc_id)
{
    for (IdbEntry &entry : *game_talisman_idb)
    {
        if (entry.desc_id == desc_id)
            entry.desc_id = 0xFFFF;
    }
}

static void RemoveSSHowReferences(uint16_t how_id)
{
    for (IdbEntry &entry : *game_talisman_idb)
    {
        if (entry.how_id == how_id)
            entry.how_id = 0xFFFF;
    }
}

bool X2mFile::InstallSSName()
{
    if (type != X2mType::NEW_SUPERSOUL)
        return false;

    if (update_ss_entry.idb_id == 0xFFFF)
        FindInstalledSS(); // Update update_ss_entry

    if (game_talisman_idb == nullptr && !Xenoverse2::InitIdb(false, false, true, false, false, false, false, false))
        return false;

    // Delete existing name (if any)
    if (update_ss_entry.idb_id != 0xFFFF)
    {
        IdbEntry *entry = game_talisman_idb->FindEntryByID(update_ss_entry.idb_id);
        if (!entry)
        {
            DPRINTF("%s: Internal error, cannot find installed idb entry for update.\n", FUNCNAME);
            return false;
        }

        if (entry->name_id != 0xFFFF)
        {
            uint16_t name_id = entry->name_id;
            RemoveSSNameReferences(name_id);

            int ls = 0, le = XV2_LANG_NUM;
            if (global_lang >= 0)
            {
                ls = global_lang;
                le = global_lang+1;
            }

            for (int l = ls; l < le; l++)
            {
                if (!Xenoverse2::RemoveTalismanName(name_id, l, (l==ls)))
                {
                    DPRINTF("%s: Internal error, fail in RemoveTalismanName.\n", FUNCNAME);
                    return false;
                }
            }
        }
    }

    // Now add new name
    int ls = 0, le = XV2_LANG_NUM;
    if (global_lang >= 0)
    {
        ls = global_lang;
        le = global_lang+1;
    }

    for (int l = ls; l < le; l++)
    {
        const std::string *name;

        if (ss_item.item_name[l].length() != 0)
            name = &ss_item.item_name[l];
        else
            name = &ss_item.item_name[XV2_LANG_ENGLISH];

        if (!Xenoverse2::AddTalismanName(*name, l, &ss_item.idb.name_id))
        {
            DPRINTF("%s: AddTalismanName failed.\n", FUNCNAME);
            return false;
        }
    }

    return true;
}

bool X2mFile::InstallSSDesc()
{
    if (type != X2mType::NEW_SUPERSOUL)
        return false;

    if (update_ss_entry.idb_id == 0xFFFF)
        FindInstalledSS(); // Update update_ss_entry

    if (game_talisman_idb == nullptr && !Xenoverse2::InitIdb(false, false, true, false, false, false, false, false))
        return false;

    // Delete existing desc (if any)
    if (update_ss_entry.idb_id != 0xFFFF)
    {
        IdbEntry *entry = game_talisman_idb->FindEntryByID(update_ss_entry.idb_id);
        if (!entry)
        {
            DPRINTF("%s: Internal error, cannot find installed idb entry for update.\n", FUNCNAME);
            return false;
        }

        if (entry->desc_id != 0xFFFF)
        {
            uint16_t desc_id = entry->desc_id;
            RemoveSSDescReferences(desc_id);

            int ls = 0, le = XV2_LANG_NUM;
            if (global_lang >= 0)
            {
                ls = global_lang;
                le = global_lang+1;
            }

            for (int l = ls; l < le; l++)
            {
                if (!Xenoverse2::RemoveTalismanDesc(desc_id, l, (l==ls)))
                {
                    DPRINTF("%s: Internal error, fail in RemoveTalismanDesc.\n", FUNCNAME);
                    return false;
                }
            }
        }
    }

    // Now add new desc
    int ls = 0, le = XV2_LANG_NUM;
    if (global_lang >= 0)
    {
        ls = global_lang;
        le = global_lang+1;
    }

    for (int l = ls; l < le; l++)
    {
        const std::string *desc;

        if (ss_item.item_desc[l].length() != 0)
            desc = &ss_item.item_desc[l];
        else
            desc = &ss_item.item_desc[XV2_LANG_ENGLISH];

        if (!Xenoverse2::AddTalismanDesc(*desc, l, &ss_item.idb.desc_id))
        {
            DPRINTF("%s: AddTalismanDesc failed.\n", FUNCNAME);
            return false;
        }
    }

    return true;
}

bool X2mFile::InstallSSHow()
{
    if (type != X2mType::NEW_SUPERSOUL)
        return false;

    if (update_ss_entry.idb_id == 0xFFFF)
        FindInstalledSS(); // Update update_ss_entry

    if (game_talisman_idb == nullptr && !Xenoverse2::InitIdb(false, false, true, false, false, false, false, false))
        return false;

    // Delete existing desc (if any)
    if (update_ss_entry.idb_id != 0xFFFF)
    {
        IdbEntry *entry = game_talisman_idb->FindEntryByID(update_ss_entry.idb_id);
        if (!entry)
        {
            DPRINTF("%s: Internal error, cannot find installed idb entry for update.\n", FUNCNAME);
            return false;
        }

        if (entry->how_id != 0xFFFF)
        {
            uint16_t how_id = entry->how_id;
            RemoveSSHowReferences(how_id);

            int ls = 0, le = XV2_LANG_NUM;
            if (global_lang >= 0)
            {
                ls = global_lang;
                le = global_lang+1;
            }

            for (int l = ls; l < le; l++)
            {
                if (!Xenoverse2::RemoveTalismanHow(how_id, l, (l==ls)))
                {
                    DPRINTF("%s: Internal error, fail in RemoveTalismanHow.\n", FUNCNAME);
                    return false;
                }
            }
        }
    }

    // Now add new desc
    int ls = 0, le = XV2_LANG_NUM;
    if (global_lang >= 0)
    {
        ls = global_lang;
        le = global_lang+1;
    }

    for (int l = ls; l < le; l++)
    {
        const std::string *how;

        if (ss_item.item_how[l].length() != 0)
            how = &ss_item.item_how[l];
        else
            how = &ss_item.item_how[XV2_LANG_ENGLISH];

        if (!Xenoverse2::AddTalismanHow(*how, l, &ss_item.idb.how_id))
        {
            DPRINTF("%s: AddTalismanHow failed.\n", FUNCNAME);
            return false;
        }
    }

    return true;
}

bool X2mFile::InstallSSIdb()
{
    if (type != X2mType::NEW_SUPERSOUL)
        return false;

    static const uint16_t idb_start = 1000;

    if (ss_item.idb.name_id == 0xFFFF || ss_item.idb.name_id == X2M_INVALID_ID16 || ss_item.idb.name_id == X2M_DUMMY_ID16)
    {
        DPRINTF("%s: Name should have been assigned before.\n", FUNCNAME);
        return false;
    }

    if (ss_item.idb.desc_id == 0xFFFF || ss_item.idb.desc_id == X2M_INVALID_ID16 || ss_item.idb.desc_id == X2M_DUMMY_ID16)
    {
        DPRINTF("%s: Desc should have been assigned before.\n", FUNCNAME);
        return false;
    }

    if (ss_item.idb.how_id == 0xFFFF || ss_item.idb.how_id == X2M_INVALID_ID16 || ss_item.idb.how_id == X2M_DUMMY_ID16)
    {
        DPRINTF("%s: How should have been assigned before.\n", FUNCNAME);
        return false;
    }

    if (!game_talisman_idb && !Xenoverse2::InitIdb(false, false, true, false, false, false, false, false))
        return false;

    if (HasSSSkillDepend())
    {
        ss_item.idb.model = 0; // Default value if not installed yet

        CusSkill *skill = FindInstalledSkill(ss_blast_depend.guid, X2mSkillType::BLAST);
        if (skill)
        {
            ss_item.idb.model = Xenoverse2::GetModelForTalisman(skill->id2, true);
            if (ss_item.idb.model == 0)
            {
                DPRINTF("%s: GetModelForTalisman failed.\n", FUNCNAME);
                return false;
            }
        }
    }

    if (update_ss_entry.idb_id == 0xFFFF) // New install
    {
        if (!game_talisman_idb->AddEntryAuto(ss_item.idb, idb_start))
        {
            DPRINTF("%s: AddEntryAuto failed.\n", FUNCNAME);
            return false;
        }

        update_ss_entry.idb_id = ss_item.idb.id;
    }
    else // Update
    {
        ss_item.idb.id = update_ss_entry.idb_id;
        IdbEntry *entry = game_talisman_idb->FindEntryByID(ss_item.idb.id);
        if (!entry)
        {
            DPRINTF("%s: Failed to get existing idb entry, this should have been caught before.\n", FUNCNAME);
            return false;
        }

        *entry = ss_item.idb; // Update
    }

    return true;
}

bool X2mFile::InstallSSFile()
{
    if (type != X2mType::NEW_SUPERSOUL)
        return false;

    if (update_ss_entry.idb_id == 0xFFFF)
    {
        DPRINTF("%s: This function should have been called after InstallSSIdb.\n", FUNCNAME);
        return false;
    }

    memcpy(update_ss_entry.guid, mod_guid, sizeof(mod_guid));
    game_costume_file->AddSuperSoul(update_ss_entry);

    return true;
}

bool X2mFile::UninstallSSName()
{
    if (type != X2mType::NEW_SUPERSOUL)
        return false;

    if (update_ss_entry.idb_id == 0xFFFF)
        FindInstalledSS(); // Update update_ss_entry

    if (game_talisman_idb == nullptr && !Xenoverse2::InitIdb(false, false, true, false, false, false, false, false))
        return false;

    IdbEntry *entry = game_talisman_idb->FindEntryByID(update_ss_entry.idb_id);
    if (entry && entry->name_id != 0xFFFF)
    {
        uint16_t name_id = entry->name_id;
        RemoveSSNameReferences(name_id);

        int ls = 0, le = XV2_LANG_NUM;
        if (global_lang >= 0)
        {
            ls = global_lang;
            le = global_lang+1;
        }

        for (int l = ls; l < le; l++)
        {
            if (!Xenoverse2::RemoveTalismanName(name_id, l, (l==ls)))
            {
                DPRINTF("%s: RemoveTalismanName failed.\n", FUNCNAME);
                return false;
            }
        }

    }

    return true;
}

bool X2mFile::UninstallSSDesc()
{
    if (type != X2mType::NEW_SUPERSOUL)
        return false;

    if (update_ss_entry.idb_id == 0xFFFF)
        FindInstalledSS(); // Update update_ss_entry

    if (game_talisman_idb == nullptr && !Xenoverse2::InitIdb(false, false, true, false, false, false, false, false))
        return false;

    IdbEntry *entry = game_talisman_idb->FindEntryByID(update_ss_entry.idb_id);
    if (entry && entry->desc_id != 0xFFFF)
    {
        uint16_t desc_id = entry->desc_id;
        RemoveSSDescReferences(desc_id);

        int ls = 0, le = XV2_LANG_NUM;
        if (global_lang >= 0)
        {
            ls = global_lang;
            le = global_lang+1;
        }

        for (int l = ls; l < le; l++)
        {
            if (!Xenoverse2::RemoveTalismanDesc(desc_id, l, (l==ls)))
            {
                DPRINTF("%s: RemoveTalismanDesc failed.\n", FUNCNAME);
                return false;
            }
        }

    }

    return true;
}

bool X2mFile::UninstallSSHow()
{
    if (type != X2mType::NEW_SUPERSOUL)
        return false;

    if (update_ss_entry.idb_id == 0xFFFF)
        FindInstalledSS(); // Update update_ss_entry

    if (game_talisman_idb == nullptr && !Xenoverse2::InitIdb(false, false, true, false, false, false, false, false))
        return false;

    IdbEntry *entry = game_talisman_idb->FindEntryByID(update_ss_entry.idb_id);
    if (entry && entry->how_id != 0xFFFF)
    {
        uint16_t how_id = entry->how_id;
        RemoveSSHowReferences(how_id);

        int ls = 0, le = XV2_LANG_NUM;
        if (global_lang >= 0)
        {
            ls = global_lang;
            le = global_lang+1;
        }

        for (int l = ls; l < le; l++)
        {
            if (!Xenoverse2::RemoveTalismanHow(how_id, l, (l==ls)))
            {
                DPRINTF("%s: RemoveTalismanHow failed.\n", FUNCNAME);
                return false;
            }
        }

    }

    return true;
}

bool X2mFile::UninstallSSIdb()
{
    if (type != X2mType::NEW_SUPERSOUL)
        return false;

    if (!game_talisman_idb && !Xenoverse2::InitIdb(false, false, true, false, false, false, false, false))
        return false;

    game_talisman_idb->RemoveEntry(update_ss_entry.idb_id);
    game_psc->RemoveTalismanReferences(update_ss_entry.idb_id);
    return true;
}

bool X2mFile::UninstallSSFile()
{
    if (type != X2mType::NEW_SUPERSOUL)
        return false;

    game_costume_file->RemoveSuperSoul(mod_guid);
    return true;
}

bool X2mFile::RecoverInitSound(AcbFile **pacb, AwbFile **pawb, const std::string &base_path)
{
    if (!(*pacb))
    {
        *pacb = new AcbFile();
        if (!(*pacb)->LoadFromFile(base_path + ".acb"))
        {
            delete *pacb; *pacb = nullptr;
            return false;
        }
    }

    if (!(*pawb))
    {
        *pawb = new Afs2File();
        if (!(*pawb)->LoadFromFile(base_path + ".awb"))
        {
            delete *pawb; *pawb = nullptr;
            return false;
        }
    }

    return true;
}

bool X2mFile::RecoverInitEmb(EmbFile **pemb, const std::string &path)
{
    if (!(*pemb))
    {
        *pemb = new EmbFile();
        if (!(*pemb)->LoadFromFile(path))
        {
            delete *pemb; *pemb = nullptr;
            return false;
        }
    }

    return true;
}

HcaFile *X2mFile::RecoverSound(AcbFile *acb, AwbFile *awb, uint32_t cue_id)
{
    bool external;
    uint32_t awb_idx = acb->CueIdToAwbIndex(cue_id, &external);

    if (awb_idx == (uint32_t)-1 || !external)
        return nullptr;

    uint64_t size;
    uint8_t *hca_buf = awb->ExtractFile(awb_idx, &size);

    if (hca_buf)
    {
        HcaFile *hca = new HcaFile();

        bool ret = hca->Load(hca_buf, size);
        delete[] hca_buf;

        if (ret)
            return hca;

        delete hca;
    }

    return nullptr;
}

HcaFile *X2mFile::RecoverSound(AcbFile *acb, AwbFile *awb, const std::string &name)
{
    uint32_t cue_id = acb->FindCueId(name);
    if (cue_id == (uint32_t)-1)
        return nullptr;

    return RecoverSound(acb, awb, cue_id);
}

uint8_t *X2mFile::RecoverTexture(EmbFile *emb, const std::string &name, size_t *psize)
{
    uint16_t idx = emb->FindIndexByName(name);
    if (idx == 0xFFFF)
        return nullptr;

    const EmbContainedFile &file = (*emb)[idx];

    *psize = file.GetSize();
    uint8_t *buf = new uint8_t[*psize];

    memcpy(buf, file.GetData(), *psize);
    return buf;
}

HcaFile *X2mFile::RecoverCssSound(const std::string &name, bool english)
{
    static AcbFile *css_acb_jp = nullptr, *css_acb_en = nullptr;
    static AwbFile *css_awb_jp = nullptr, *css_awb_en = nullptr;

    AcbFile *acb;
    AwbFile *awb;

    // UGLY HACK
    if (name == "INSTALLER_CLOSE_AWB")
    {
        if (css_awb_jp)
        {
            delete css_awb_jp;
            css_awb_jp = nullptr;
        }

        if (css_awb_en)
        {
            delete css_awb_en;
            css_awb_en = nullptr;
        }

        return nullptr;
    }
    //

    if (!english)
    {
        if (!css_acb_jp && !RecoverInitSound(&css_acb_jp, &css_awb_jp, Utils::MakePathString(restore_path, "sound/VOX/Sys/CRT_CS_vox")))
            return nullptr;

        acb = css_acb_jp;
        awb = css_awb_jp;
    }
    else
    {
        if (!css_acb_en && !RecoverInitSound(&css_acb_en, &css_awb_en, Utils::MakePathString(restore_path, "sound/VOX/Sys/en/CRT_CS_vox")))
            return nullptr;

        acb = css_acb_en;
        awb = css_awb_en;
    }

    return RecoverSound(acb, awb, name);
}

HcaFile *X2mFile::RecoverSevSound(const std::string &name, bool english)
{
    static AcbFile *sev_acb_jp = nullptr, *sev_acb_en = nullptr;
    static AcbFile *sev2_acb_jp = nullptr, *sev2_acb_en = nullptr;
    static AwbFile *sev_awb_jp = nullptr, *sev_awb_en = nullptr;
    static AwbFile *sev2_awb_jp = nullptr, *sev2_awb_en = nullptr;

    AcbFile *acb1, *acb2;
    AwbFile *awb1, *awb2;

    // UGLY HACK
    if (name == "INSTALLER_CLOSE_AWB")
    {
        if (sev_awb_jp)
        {
            delete sev_awb_jp;
            sev_awb_jp = nullptr;
        }

        if (sev_awb_en)
        {
            delete sev_awb_en;
            sev_awb_en = nullptr;
        }

        if (sev2_awb_jp)
        {
            delete sev2_awb_jp;
            sev2_awb_jp = nullptr;
        }

        if (sev2_awb_en)
        {
            delete sev2_awb_en;
            sev2_awb_en = nullptr;
        }

        return nullptr;
    }
    //

    if (!english)
    {
        if (!sev_acb_jp)
        {
            if (!RecoverInitSound(&sev_acb_jp, &sev_awb_jp, "data/sound/VOX/Quest/Dialogue/CAQD_ALL_VOX") ||
                !RecoverInitSound(&sev2_acb_jp, &sev2_awb_jp, "data/sound/VOX/Quest/Dialogue/CAQD_ADD_VOX"))
                return nullptr;
        }

        acb1 = sev_acb_jp;
        acb2 = sev2_acb_jp;
        awb1 = sev_awb_jp;
        awb2 = sev2_awb_jp;
    }
    else
    {
        if (!sev_acb_en)
        {
            if (!RecoverInitSound(&sev_acb_en, &sev_awb_en, "data/sound/VOX/Quest/Dialogue/en/CAQD_ALL_VOX") ||
                !RecoverInitSound(&sev2_acb_en, &sev2_awb_en, "data/sound/VOX/Quest/Dialogue/en/CAQD_ADD_VOX"))
                return nullptr;
        }

        acb1 = sev_acb_en;
        acb2 = sev2_acb_en;
        awb1 = sev_awb_en;
        awb2 = sev2_awb_en;
    }

    // Will search in both, but x2m mods should only be in the ADD_VOX
    HcaFile *ret = RecoverSound(acb2, awb2, name);
    if (!ret)
        ret = RecoverSound(acb1, awb1, name);

    return ret;
}

uint8_t *X2mFile::RecoverSelPortrait(size_t *psize)
{
    static EmbFile *sel_port = nullptr;

    if (!RecoverInitEmb(&sel_port, Utils::MakePathString(restore_path, "ui/texture/CHARA01.emb")))
        return nullptr;

    std::string name = entry_name + "_000.dds";    
    return RecoverTexture(sel_port, name, psize);
}

uint8_t *X2mFile::RecoverBtlPortrait(size_t *psize)
{
    std::string emb_path = Utils::MakePathString(restore_path, std::string("ui/texture/") + entry_name + ".emb");
    return Utils::ReadFile(emb_path, psize);
}

uint8_t *X2mFile::RecoverFile(const std::string &path, size_t *psize)
{
    std::string old_path = path;

    if (old_path.front() == '/')
        old_path = old_path.substr(1);

    if (!Utils::BeginsWith(Utils::ToLowerCase(old_path), "data/"))
    {
        DPRINTF("%s: Cannot recover a file outside data (path = %s)", FUNCNAME, old_path.c_str());
        return nullptr;
    }

    old_path = Utils::MakePathString(restore_path, old_path.substr(5));
    return Utils::ReadFile(old_path, psize);
}

uint8_t *X2mFile::RecoverCharaFile(const std::string &fn, size_t *psize, bool show_error)
{
    std::string path = Utils::MakePathString(restore_path, std::string("/chara/") + entry_name + "/" + fn);
    return Utils::ReadFile(path, psize, show_error);
}

uint8_t *X2mFile::RecoverCostumeFile(const std::string &race, const std::string &fn, uint16_t num, size_t *psize)
{
    static X2mCostumeFile *costume_file = nullptr;

    if (!costume_file)
    {
        costume_file = new X2mCostumeFile();
        if (!costume_file->CompileFromFile(Utils::MakePathString(restore_path, "X2M_COSTUME.xml")))
        {
            delete costume_file; costume_file = nullptr;
            return nullptr;
        }
    }

    X2mCostumeEntry *entry = costume_file->FindCostume(mod_guid);
    if (!entry || entry->partsets.size() == 0)
        return nullptr;

    std::string old_fn = fn;

    static const uint16_t lower_limit = 10000;
    uint16_t upper_limit = lower_limit+num;

    bool found = false;

    for (uint16_t i = lower_limit; i < upper_limit; i++)
    {
        std::string str_num = Utils::ToString(i);
        size_t pos = old_fn.find(str_num);

        if (pos != std::string::npos)
        {
            size_t idx = i-lower_limit;
            uint16_t base = entry->partsets[0];
            std::string temp = old_fn.substr(0, pos);
            temp += Utils::ToStringAndPad((int)idx+base, 3);
            temp += old_fn.substr(pos+str_num.length());

            old_fn = temp;
            found = true;
            break;
        }
    }

    if (!found)
        return nullptr;

    std::string old_path = Utils::MakePathString(restore_path, std::string("/chara/") + race);
    old_path = Utils::MakePathString(old_path, old_fn);
    return Utils::ReadFile(old_path, psize);
}

uint8_t *X2mFile::RecoverStageSelPortrait(size_t *psize)
{
    static EmbFile *stage02_emb = nullptr;

    if (!RecoverInitEmb(&stage02_emb, Utils::MakePathString(restore_path, "ui/texture/STAGE02.emb")))
        return nullptr;

    const std::string portrait_name = std::string("ICO") + stage_def.code + ".dds";
    return RecoverTexture(stage02_emb, portrait_name, psize);
}

uint8_t *X2mFile::RecoverStageBackground(size_t *psize)
{
    static EmbFile *stage02_emb = nullptr;

    if (!RecoverInitEmb(&stage02_emb, Utils::MakePathString(restore_path, "ui/texture/STAGE02.emb")))
        return nullptr;

    const std::string background_name = stage_def.code + ".dds";
    return RecoverTexture(stage02_emb, background_name, psize);
}

uint8_t *X2mFile::RecoverStageQstPortrait(size_t *psize)
{
    static EmbFile *stage01_emb = nullptr;

    if (!RecoverInitEmb(&stage01_emb, Utils::MakePathString(restore_path, "ui/texture/STAGE01.emb")))
        return nullptr;

    const std::string portrait_name = stage_def.code + ".dds";
    return RecoverTexture(stage01_emb, portrait_name, psize);
}

HcaFile *X2mFile::RecoverStageBgm(const std::string &n)
{
    static AcbFile *bgm_acb = nullptr;
    static AwbFile *bgm_awb = nullptr;

    // UGLY HACK
    if (n == "INSTALLER_CLOSE_AWB")
    {
        if (bgm_awb)
        {
            delete bgm_awb;
            bgm_awb = nullptr;
        }

        return nullptr;
    }
    //

    if (!RecoverInitSound(&bgm_acb, &bgm_awb, Utils::MakePathString(restore_path, "sound/BGM/CAR_BGM")))
        return nullptr;

    const std::string track_name = "X2M_" + stage_def.code;
    return RecoverSound(bgm_acb, bgm_awb, track_name);
}

uint8_t *X2mFile::RecoverStageLighting(size_t *psize)
{
    std::string emb_path = std::string("lighting/environment/") + stage_def.code + ".emb";
    return Utils::ReadFile(Utils::MakePathString(restore_path, emb_path), psize);
}

bool X2mFile::RecoverSkillVars()
{
    static CmsFile *cms = nullptr;
    static CusFile *cus = nullptr;

    if (restore_skill_path.length() > 0) // Already done
        return true;

    if (!cms)
    {
        cms = new CmsFile();
        if (!cms->LoadFromFile(Utils::MakePathString(restore_path, "system/char_model_spec.cms")))
        {
            delete cms; cms = nullptr;
            return false;
        }
    }

    if (!cus)
    {
        cus = new CusFile();
        if (!cus->LoadFromFile(Utils::MakePathString(restore_path, "system/custom_skill.cus")))
        {
            delete cus; cus = nullptr;
            return false;
        }
    }

    std::vector<CmsEntryXV2 *> entries;
    FindX2mSkillCMS(entries, cms);

    for (CmsEntryXV2 *entry : entries)
    {
        for (int i = 0; i < 10; i++)
        {
            uint16_t id = (entry->id*10) + i;
            id = IdFromId2(id);

            const CusSkill *skill = cus->FindSkillAnyByID(id);
            if (!skill)
                continue;

            if (skill->name == skill_entry.name)
            {
                std::string skill_path = Utils::MakePathString(restore_path, "skill");

                if (skill_type == X2mSkillType::SUPER)
                    skill_path += "/SPA/";
                else if (skill_type == X2mSkillType::ULTIMATE)
                    skill_path += "/ULT/";
                else if (skill_type == X2mSkillType::EVASIVE)
                    skill_path += "/ESC/";
                else if (skill_type == X2mSkillType::BLAST)
                    skill_path += "/BLT/";
                else if (skill_type == X2mSkillType::AWAKEN)
                    skill_path += "/MET/";
                else
                {
                    DPRINTF("%s: Internal error.\n", FUNCNAME);
                    return false;
                }

                skill_path += Utils::ToStringAndPad(skill->id2, 3);
                skill_path += '_';

                CmsEntry *entry = cms->FindEntryByID(skill->id2 / 10);
                if (!entry)
                {
                    DPRINTF("%s: Failed to find old cms skill dummy entry 0x%x\n", FUNCNAME, skill->id2 / 10);
                    return false;
                }

                skill_path += entry->name;
                skill_path += '_';
                skill_path += skill_entry.name;

                IniFile ini;

                if (ini.LoadFromFile(skill_path + std::string("/") + X2M_SKILL_INI, false))
                {
                    uint8_t guid[16];
                    std::string guid_str;

                    if (ini.GetStringValue("General", "GUID", guid_str))
                    {
                        if (Utils::String2GUID(guid, guid_str) && memcmp(guid, mod_guid, sizeof(mod_guid)) == 0)
                        {
                            // Found
                            restore_skill_path = skill_path;
                            restore_skill_id = skill->id;
                            restore_skill_id2 = skill->id2;
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}

void X2mFile::RecoverSkillBodies()
{
    static X2mCostumeFile *costume_file = nullptr;

    if (restore_bodies_map.size() > 0) // Already done
        return;

    if (!costume_file)
    {
        costume_file = new X2mCostumeFile();
        if (!costume_file->CompileFromFile(Utils::MakePathString(restore_path, "X2M_COSTUME.xml"), false))
        {
            delete costume_file; costume_file = nullptr;
            return;
        }
    }

    std::vector<X2mBody *> found_bodies;
    if (costume_file->FindBody(mod_guid, found_bodies) > 0)
    {
        for (size_t i = 0; i < found_bodies.size(); i++)
        {
            X2mBody *body = found_bodies[i];
            restore_bodies_map[body->id] = (uint32_t)(X2M_SKILL_BODY_ID_BEGIN + i);
        }
    }
}

uint8_t *X2mFile::RecoverSkillFile(const std::string &local_path, size_t *psize)
{
    uint8_t *buf = nullptr;

    if (!RecoverSkillVars())
        return buf;

    std::string filename = Utils::GetFileNameString(local_path);

    if (Utils::BeginsWith(filename, X2M_SKILL_PREFIX, false))
    {
        std::string skill_prefix = Utils::GetFileNameString(restore_skill_path);
        std::string restore_fp = Utils::MakePathString(restore_skill_path, skill_prefix + filename.substr(strlen(X2M_SKILL_PREFIX)));

        buf = Utils::ReadFile(restore_fp, psize);
        if (!buf)
            return buf;

        if (Utils::EndsWith(filename, ".bac", false))
        {
            BacFile bac;

            if (!bac.Load(buf, *psize))
            {
                DPRINTF("%s: Failed to load bac file: \"%s\"\n", FUNCNAME, restore_fp.c_str());
                delete[] buf;
                return nullptr;
            }

            delete[] buf; buf = nullptr;

            bac.ChangeReferencesToSkill(restore_skill_id2, X2M_DUMMY_ID16);
            bac.ChangeReferencesToSkill(restore_skill_id, X2M_DUMMY_ID16_2);

            if (bodies_map.size() > 0)
            {
                for (BacEntry &entry : bac)
                {
                    if (entry.has_type[15])
                    {
                        for (BACTransformControl &tc : entry.type15)
                        {
                            if (tc.type != 0x2a)
                                continue;

                            RecoverSkillBodies();
                            if (restore_bodies_map.size() == 0)
                                break;

                            int body = (int)tc.parameter;
                            auto it = restore_bodies_map.find(body);
                            if (it != restore_bodies_map.end())
                            {
                                tc.parameter = (float)it->second;
                                //UPRINTF("--------Body restored to %d\n", it->second);
                            }
                        }
                    }
                }
            }

            buf = bac.Save(psize);
        }
        else if (Utils::EndsWith(filename, ".bsa", false))
        {
            BsaFile bsa;

            if (!bsa.Load(buf, *psize))
            {
                DPRINTF("%s: Failed to load bsa file: \"%s\"\n", FUNCNAME, restore_fp.c_str());
                delete[] buf;
                return nullptr;
            }

            delete[] buf; buf = nullptr;
            bsa.ChangeReferencesToSkill(restore_skill_id2, X2M_DUMMY_ID16);
            bsa.ChangeReferencesToSkill(restore_skill_id, X2M_DUMMY_ID16_2);

            buf = bsa.Save(psize);
        }
        else if (Utils::EndsWith(filename, ".bdm", false))
        {
            BdmFile bdm;

            if (bdm.Load(buf, *psize))
            {
                delete[] buf; buf = nullptr;
                bdm.ChangeReferencesToSkill(restore_skill_id2, X2M_DUMMY_ID16);
                bdm.ChangeReferencesToSkill(restore_skill_id, X2M_DUMMY_ID16_2);

                buf = bdm.Save(psize);
            }
        }
    }
    else
    {
        buf = Utils::ReadFile(Utils::MakePathString(restore_skill_path, filename), psize);
    }

    return buf;
}

void X2mFile::RecoverCloseAwb()
{
    // UGLY HACK
    X2mFile dummy;

    dummy.RecoverCssSound("INSTALLER_CLOSE_AWB", false);
    dummy.RecoverSevSound("INSTALLER_CLOSE_AWB", false);
    dummy.RecoverStageBgm("INSTALLER_CLOSE_AWB");
}
