#include <algorithm>
#include "Xenoverse2.h"

#include "CusFile.h"
#include "debug.h"

TiXmlElement *CusSkillSet::Decompile(TiXmlNode *root, const CusFile *owner) const
{
    TiXmlElement *entry_root = new TiXmlElement("SkillSet");

    std::string name = Xenoverse2::GetCharaAndCostumeName(char_id, costume_id, model_preset, true);
    if (name.length() != 0)
        Utils::WriteComment(entry_root, name);

    Utils::WriteParamUnsigned(entry_root, "CHAR_ID", char_id, true);
    Utils::WriteParamUnsigned(entry_root, "COSTUME_ID", costume_id, true);

    if (owner)
    {
        std::string skills_comment;

        for (int i = 0; i < 4; i++)
        {
            std::string skill_name = "???";

            const CusSkill *skill = owner->FindSuperSkillByID(char_skills[i]);
            if (skill)
                Xenoverse2::GetSuperSkillName(skill->id2, skill_name);

            if (i != 0)
                skills_comment += ", ";

            skills_comment += skill_name;
        }

        for (int i = 0; i < 2; i++)
        {
            std::string skill_name = "???";

            const CusSkill *skill = owner->FindUltimateSkillByID(char_skills[i+4]);
            if (skill)
                Xenoverse2::GetUltimateSkillName(skill->id2, skill_name);

            skills_comment += ", ";
            skills_comment += skill_name;
        }

        {
            std::string skill_name = "???";

            const CusSkill *skill = owner->FindEvasiveSkillByID(char_skills[6]);
            if (skill)
                Xenoverse2::GetEvasiveSkillName(skill->id2, skill_name);

            skills_comment += ", ";
            skills_comment += skill_name;
        }

        {
            std::string skill_name = "???";

            const CusSkill *skill = owner->FindBlastSkillByID(char_skills[7]);
            if (skill)
                skill_name = skill->name;

            skills_comment += ", ";
            skills_comment += skill_name;
        }

        {
            std::string skill_name = "???";

            const CusSkill *skill = owner->FindAwakenSkillByID(char_skills[6]);
            if (skill)
            {
                Xenoverse2::GetAwakenSkillName(skill->id2, skill_name);
                skill_name = skill_name + "(" + skill->name + ")";
            }

            skills_comment += ", ";
            skills_comment += skill_name;
        }

        Utils::WriteComment(entry_root, skills_comment);
    }

    Utils::WriteParamMultipleUnsigned(entry_root, "SKILLS", std::vector<uint16_t>(char_skills, char_skills+9));
    Utils::WriteParamUnsigned(entry_root, "MODEL_PRESET", model_preset);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool CusSkillSet::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "CHAR_ID", &char_id))
        return false;

    if (!Utils::GetParamUnsigned(root, "COSTUME_ID", &costume_id))
        return false;

    std::vector<uint16_t> temp;

    if (!Utils::GetParamMultipleUnsigned(root, "SKILLS", temp))
        return false;

    if (temp.size() != 9 && temp.size() != 10)
    {
        DPRINTF("%s: Incorrect size for SKILLS param.\n", FUNCNAME);
        return false;
    }

    memcpy(char_skills, temp.data(), sizeof(char_skills));

    if (temp.size() == 9)
    {
        if (!Utils::GetParamUnsigned(root, "MODEL_PRESET", &model_preset))
            return false;
    }
    else
    {
        // Compatibility with old format
        model_preset = temp[9];
    }

    return true;
}

TiXmlElement *CusSkill::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Skill");

    entry_root->SetAttribute("name", name);
    entry_root->SetAttribute("id", Utils::UnsignedToString(id, false));
    entry_root->SetAttribute("id2", Utils::UnsignedToString(id2, false));

    Utils::WriteParamUnsigned(entry_root, "RACE_LOCK", race_lock, true);
    Utils::WriteParamUnsigned(entry_root, "TYPE", type, true);
    Utils::WriteParamUnsigned(entry_root, "U_0E", unk_0E, true);
    Utils::WriteParamUnsigned(entry_root, "PARTSET", partset);
    Utils::WriteParamUnsigned(entry_root, "U_12", unk_12, true);
    Utils::WriteParamMultipleStrings(entry_root, "PATHS", std::vector<std::string>(paths, paths+7));
    Utils::WriteParamUnsigned(entry_root, "U_30", unk_30, true);
    Utils::WriteParamUnsigned(entry_root, "U_32", unk_32, true);
    Utils::WriteParamUnsigned(entry_root, "U_34", unk_34, true);
    Utils::WriteParamUnsigned(entry_root, "SKILL_TYPE", skill_type, true);
    Utils::WriteParamUnsigned(entry_root, "PUP_ID", pup_id, true);
    Utils::WriteParamUnsigned(entry_root, "AURA", aura, true);
    Utils::WriteParamUnsigned(entry_root, "MODEL", model, true);
    Utils::WriteParamUnsigned(entry_root, "CHANGE_SKILLSET", change_skillset, true);
    Utils::WriteParamUnsigned(entry_root, "NUM_TRANSFORMS", num_transforms, true);
    Utils::WriteParamUnsigned(entry_root, "U_44", unk_44, true);
    Utils::WriteParamUnsigned(entry_root, "U_48", unk_48, true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool CusSkill::Compile(const TiXmlElement *root, int *version)
{
    if (version)
        *version = 0;

    if (!Utils::ReadAttrString(root, "name", name))
        return false;

    if (name.length() != 3 && name.length() != 4)
    {
        DPRINTF("%s: Name attribute must be exactly 3 or 4 chars: %s\n", FUNCNAME, name.c_str());
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "id", &id))
        return false;

    if (!Utils::ReadAttrUnsigned(root, "id2", &id2))
    {
        if (!Utils::ReadAttrUnsigned(root, "name_id", &id2))
            return false;
    }

    if (!Utils::GetParamUnsigned(root, "RACE_LOCK", &race_lock))
        return false;

    if (!Utils::GetParamUnsigned(root, "TYPE", &type))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0E", &unk_0E))
        return false;

    if (!Utils::GetParamUnsignedWithMultipleNames(root, &partset, "PARTSET", "HAIR"))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_12", &unk_12))
        return false;

    std::vector<std::string> paths_temp;

    if (!Utils::GetParamMultipleStrings(root, "PATHS", paths_temp))
        return false;

    if (paths_temp.size() != 7)
    {
        DPRINTF("%s: Number of strings in PATHS field must be 7 (found %Id).\n", FUNCNAME, paths_temp.size());
        return false;
    }

    for (size_t i = 0; i < 7; i++)
        paths[i] = paths_temp[i];

    if (!Utils::GetParamUnsigned(root, "U_30", &unk_30))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_32", &unk_32))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_34", &unk_34))
        return false;

    if (!Utils::GetParamUnsignedWithMultipleNames(root, &skill_type, "SKILL_TYPE", "U_36"))
        return false;

    if (!Utils::GetParamUnsignedWithMultipleNames(root, &pup_id, "PUP_ID", "U_38"))
        return false;

    if (!Utils::GetParamUnsignedWithMultipleNames(root, &aura, "AURA", "U_3A"))
        return false;

    if (!Utils::GetParamUnsignedWithMultipleNames(root, &model, "MODEL", "U_3C"))
        return false;

    if (!Utils::GetParamUnsignedWithMultipleNames(root, &change_skillset, "CHANGE_SKILLSET", "U_3E"))
        return false;

    if (!Utils::GetParamUnsignedWithMultipleNames(root, &num_transforms, "NUM_TRANSFORMS", "U_40"))
        return false;

    if (Utils::ReadParamUnsigned(root, "U_44", &unk_44))
    {
        if (version)
            *version = 119;
    }
    else
    {
        // For old mods, init to default value
        unk_44 = 0xFFFFFF00;
    }

    if (Utils::ReadParamUnsigned(root, "U_48", &unk_48))
    {
        if (version)
            *version = 121;
    }
    else
    {
        // For old mods, init to default value
        unk_48 = 0;
    }

    return true;
}

CusFile::CusFile()
{
    this->big_endian = false;
    this->version = 0;
}

CusFile::~CusFile()
{
}

void CusFile::Reset()
{
    version = 0;
    skill_sets.clear();
    super_skills.clear();
    ultimate_skills.clear();
    evasive_skills.clear();
    unk_skills.clear();
    blast_skills.clear();
    awaken_skills.clear();
}

bool CusFile::LoadSkills(const uint8_t *top, const CUSSkill *sets_in, std::vector<CusSkill> &sets_out, uint32_t num)
{
    sets_out.resize(num);

    for (size_t i = 0; i < sets_out.size(); i++)
    {
        CusSkill &skill = sets_out[i];

        if (sets_in[i].unk_04 != 0)
        {
            DPRINTF("%s: a value is not zero as expected (entry 0x%Ix): 0x%x\n", FUNCNAME, i, sets_in[i].unk_04);
            return false;
        }

        skill.name = sets_in[i].name;
        skill.id = sets_in[i].id;
        skill.id2 = sets_in[i].id2;
        skill.race_lock = sets_in[i].race_lock;
        skill.type = sets_in[i].type;
        skill.unk_0E = sets_in[i].unk_0E;
        skill.partset = sets_in[i].partset;
        skill.unk_12 = sets_in[i].unk_12;

        for (int j = 0; j < 7; j++)
        {
            skill.paths[j] = GetString(top, sets_in[i].paths_offsets[j]);
        }

        skill.unk_30 = sets_in[i].unk_30;
        skill.unk_32 = sets_in[i].unk_32;
        skill.unk_34 = sets_in[i].unk_34;
        skill.skill_type = sets_in[i].skill_type;
        skill.pup_id = sets_in[i].pup_id;
        skill.aura = sets_in[i].aura;
        skill.model = sets_in[i].model;
        skill.change_skillset = sets_in[i].change_skillset;
        skill.num_transforms = sets_in[i].num_transforms;
    }

    return true;
}

bool CusFile::LoadSkills119(const uint8_t *top, const CUSSkill119 *sets_in, std::vector<CusSkill> &sets_out, uint32_t num)
{
    sets_out.resize(num);

    for (size_t i = 0; i < sets_out.size(); i++)
    {
        CusSkill &skill = sets_out[i];

        if (sets_in[i].unk_04 != 0)
        {
            DPRINTF("%s: a value is not zero as expected (entry 0x%Ix): 0x%x\n", FUNCNAME, i, sets_in[i].unk_04);
            return false;
        }

        skill.name = sets_in[i].name;
        skill.id = sets_in[i].id;
        skill.id2 = sets_in[i].id2;
        skill.race_lock = sets_in[i].race_lock;
        skill.type = sets_in[i].type;
        skill.unk_0E = sets_in[i].unk_0E;
        skill.partset = sets_in[i].partset;
        skill.unk_12 = sets_in[i].unk_12;

        for (int j = 0; j < 7; j++)
        {
            skill.paths[j] = GetString(top, sets_in[i].paths_offsets[j]);
        }

        skill.unk_30 = sets_in[i].unk_30;
        skill.unk_32 = sets_in[i].unk_32;
        skill.unk_34 = sets_in[i].unk_34;
        skill.skill_type = sets_in[i].skill_type;
        skill.pup_id = sets_in[i].pup_id;
        skill.aura = sets_in[i].aura;
        skill.model = sets_in[i].model;
        skill.change_skillset = sets_in[i].change_skillset;
        skill.num_transforms = sets_in[i].num_transforms;
        skill.unk_44 = sets_in[i].unk_44;
    }

    return true;
}

bool CusFile::LoadSkills121(const uint8_t *top, const CUSSkill121 *sets_in, std::vector<CusSkill> &sets_out, uint32_t num)
{
    sets_out.resize(num);

    for (size_t i = 0; i < sets_out.size(); i++)
    {
        CusSkill &skill = sets_out[i];

        if (sets_in[i].unk_04 != 0)
        {
            DPRINTF("%s: a value is not zero as expected (entry 0x%Ix): 0x%x\n", FUNCNAME, i, sets_in[i].unk_04);
            return false;
        }

        skill.name = sets_in[i].name;
        skill.id = sets_in[i].id;
        skill.id2 = sets_in[i].id2;
        skill.race_lock = sets_in[i].race_lock;
        skill.type = sets_in[i].type;
        skill.unk_0E = sets_in[i].unk_0E;
        skill.partset = sets_in[i].partset;
        skill.unk_12 = sets_in[i].unk_12;

        for (int j = 0; j < 7; j++)
        {
            skill.paths[j] = GetString(top, sets_in[i].paths_offsets[j]);
        }

        skill.unk_30 = sets_in[i].unk_30;
        skill.unk_32 = sets_in[i].unk_32;
        skill.unk_34 = sets_in[i].unk_34;
        skill.skill_type = sets_in[i].skill_type;
        skill.pup_id = sets_in[i].pup_id;
        skill.aura = sets_in[i].aura;
        skill.model = sets_in[i].model;
        skill.change_skillset = sets_in[i].change_skillset;
        skill.num_transforms = sets_in[i].num_transforms;
        skill.unk_44 = sets_in[i].unk_44;
        skill.unk_48 = sets_in[i].unk_48;
    }

    return true;
}

bool CusFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (size < sizeof(CUSHeader))
        return false;

    const CUSHeader *hdr = (const CUSHeader *)buf;

    if (hdr->signature != CUS_SIGNATURE && memcmp(buf+1, "CUS", 3) != 0)
        return false;

    if (hdr->endianess_check != 0xFFFE)
        return false;

    const CUSSkillSet *skill_sets_f = (const CUSSkillSet *)GetOffsetPtr(buf, hdr->skillset_offset);
    skill_sets.resize(hdr->num_skillsets);

    for (size_t i = 0; i < skill_sets.size(); i++)
    {
        CusSkillSet &skill_set = skill_sets[i];

        if (skill_sets_f[i].unk_1C != 0)
        {
            DPRINTF("%s: Warning, unk_1C not zero (0x%x) as expected (in CUSSKillSet, near 0x%x).\n", FUNCNAME, skill_sets_f[i].unk_1C, Utils::DifPointer(skill_sets_f+i, buf));
            return false;
        }

        skill_set.char_id = skill_sets_f[i].char_id;
        skill_set.costume_id = skill_sets_f[i].costume_id;
        memcpy(skill_set.char_skills, skill_sets_f[i].char_skills, sizeof(skill_set.char_skills));
        skill_set.model_preset = skill_sets_f[i].model_preset;
    }


    if (((hdr->ultimate_offset - hdr->super_offset) % sizeof(CUSSkill121)) == 0)
    {
        version = 121;
    }

    else if (((hdr->ultimate_offset - hdr->super_offset) % sizeof(CUSSkill119)) == 0)
    {
        version = 119;
    }
    else
    {
        version = 0;
    }

    //DPRINTF("Version = %d\n", version);

    if (version == 121)
    {
        if (!LoadSkills121(buf, (const CUSSkill121 *)GetOffsetPtr(buf, hdr->super_offset), super_skills, hdr->num_super))
            return false;

        if (!LoadSkills121(buf, (const CUSSkill121 *)GetOffsetPtr(buf, hdr->ultimate_offset), ultimate_skills, hdr->num_ultimate))
            return false;

        if (!LoadSkills121(buf, (const CUSSkill121 *)GetOffsetPtr(buf, hdr->evasive_offset), evasive_skills, hdr->num_evasive))
            return false;

        if (!LoadSkills121(buf, (const CUSSkill121 *)GetOffsetPtr(buf, hdr->unk_offset), unk_skills, hdr->num_unk))
            return false;

        if (!LoadSkills121(buf, (const CUSSkill121 *)GetOffsetPtr(buf, hdr->blast_offset), blast_skills, hdr->num_blast))
            return false;

        if (!LoadSkills121(buf, (const CUSSkill121 *)GetOffsetPtr(buf, hdr->awaken_offset), awaken_skills, hdr->num_awaken))
            return false;
    }

    else if (version == 119)
    {
        if (!LoadSkills119(buf, (const CUSSkill119 *)GetOffsetPtr(buf, hdr->super_offset), super_skills, hdr->num_super))
            return false;

        if (!LoadSkills119(buf, (const CUSSkill119 *)GetOffsetPtr(buf, hdr->ultimate_offset), ultimate_skills, hdr->num_ultimate))
            return false;

        if (!LoadSkills119(buf, (const CUSSkill119 *)GetOffsetPtr(buf, hdr->evasive_offset), evasive_skills, hdr->num_evasive))
            return false;

        if (!LoadSkills119(buf, (const CUSSkill119 *)GetOffsetPtr(buf, hdr->unk_offset), unk_skills, hdr->num_unk))
            return false;

        if (!LoadSkills119(buf, (const CUSSkill119 *)GetOffsetPtr(buf, hdr->blast_offset), blast_skills, hdr->num_blast))
            return false;

        if (!LoadSkills119(buf, (const CUSSkill119 *)GetOffsetPtr(buf, hdr->awaken_offset), awaken_skills, hdr->num_awaken))
            return false;
    }
    else
    {
        if (!LoadSkills(buf, (const CUSSkill *)GetOffsetPtr(buf, hdr->super_offset), super_skills, hdr->num_super))
            return false;

        if (!LoadSkills(buf, (const CUSSkill *)GetOffsetPtr(buf, hdr->ultimate_offset), ultimate_skills, hdr->num_ultimate))
            return false;

        if (!LoadSkills(buf, (const CUSSkill *)GetOffsetPtr(buf, hdr->evasive_offset), evasive_skills, hdr->num_evasive))
            return false;

        if (!LoadSkills(buf, (const CUSSkill *)GetOffsetPtr(buf, hdr->unk_offset), unk_skills, hdr->num_unk))
            return false;

        if (!LoadSkills(buf, (const CUSSkill *)GetOffsetPtr(buf, hdr->blast_offset), blast_skills, hdr->num_blast))
            return false;

        if (!LoadSkills(buf, (const CUSSkill *)GetOffsetPtr(buf, hdr->awaken_offset), awaken_skills, hdr->num_awaken))
            return false;
    }

    return true;
}

size_t CusFile::CalculateStringsSize(std::unordered_set<std::string> &list, const std::vector<CusSkill> &skills) const
{
    size_t size = 0;

    for (const CusSkill &skill : skills)
    {
        for (size_t i = 0; i < 7; i++)
        {
            size_t len = skill.paths[i].length();

            if (len == 0)
                continue;

            if (list.find(skill.paths[i]) != list.end())
                continue;

            size += len+1;
            list.insert(skill.paths[i]);
        }
    }

    return size;
}

size_t CusFile::CalculateFileSize() const
{
    std::unordered_set<std::string> strings_list;

    size_t size = sizeof(CUSHeader);
    size += skill_sets.size()*sizeof(CUSSkillSet);

    if (version == 121)
        size += (super_skills.size()+ultimate_skills.size()+evasive_skills.size()+unk_skills.size()+blast_skills.size()+awaken_skills.size()) *sizeof(CUSSkill121);
    else if (version == 119)
        size += (super_skills.size()+ultimate_skills.size()+evasive_skills.size()+unk_skills.size()+blast_skills.size()+awaken_skills.size()) *sizeof(CUSSkill119);
    else
        size += (super_skills.size()+ultimate_skills.size()+evasive_skills.size()+unk_skills.size()+blast_skills.size()+awaken_skills.size()) *sizeof(CUSSkill);

    size += CalculateStringsSize(strings_list, super_skills);
    size += CalculateStringsSize(strings_list, ultimate_skills);
    size += CalculateStringsSize(strings_list, evasive_skills);
    size += CalculateStringsSize(strings_list, unk_skills);
    size += CalculateStringsSize(strings_list, blast_skills);
    size += CalculateStringsSize(strings_list, awaken_skills);

    //DPRINTF("size = %x\n", size);

    return size;
}

void CusFile::SaveSkills(uint8_t *top, char *str_top, char **str_current, const std::vector<CusSkill> &sets_in, CUSSkill *sets_out, std::unordered_set<std::string> &strings_list)
{
    for (size_t i = 0; i < sets_in.size(); i++)
    {
        const CusSkill &skill = sets_in[i];

        strcpy(sets_out[i].name, skill.name.c_str());
        sets_out[i].id = skill.id;
        sets_out[i].id2 = skill.id2;
        sets_out[i].race_lock = skill.race_lock;
        sets_out[i].type = skill.type;
        sets_out[i].unk_0E = skill.unk_0E;
        sets_out[i].partset = skill.partset;
        sets_out[i].unk_12 = skill.unk_12;

        for (size_t j = 0; j < 7; j++)
        {
            if (skill.paths[j].length() == 0)
            {
                sets_out[i].paths_offsets[j] = 0;
                continue;
            }

            if (strings_list.find(skill.paths[j]) == strings_list.end())
            {
                sets_out[i].paths_offsets[j] = Utils::DifPointer(*str_current, top);

                strings_list.insert(skill.paths[j]);
                strcpy(*str_current, skill.paths[j].c_str());
                *str_current += skill.paths[j].length() + 1;
            }
            else
            {
                const char *str = FindString(str_top, skill.paths[j].c_str(), strings_list.size());
                if (!str)
                {
                    DPRINTF("%s: Internal coding error.\n", FUNCNAME);
                    exit(-1);
                }

                sets_out[i].paths_offsets[j] = Utils::DifPointer(str, top);
            }
        }

        sets_out[i].unk_30 = skill.unk_30;
        sets_out[i].unk_32 = skill.unk_32;
        sets_out[i].unk_34 = skill.unk_34;
        sets_out[i].skill_type = skill.skill_type;
        sets_out[i].pup_id = skill.pup_id;
        sets_out[i].aura = skill.aura;
        sets_out[i].model = skill.model;
        sets_out[i].change_skillset = skill.change_skillset;
        sets_out[i].num_transforms = skill.num_transforms;
    }
}

void CusFile::SaveSkills119(uint8_t *top, char *str_top, char **str_current, const std::vector<CusSkill> &sets_in, CUSSkill119 *sets_out, std::unordered_set<std::string> &strings_list)
{
    for (size_t i = 0; i < sets_in.size(); i++)
    {
        const CusSkill &skill = sets_in[i];

        strcpy(sets_out[i].name, skill.name.c_str());
        sets_out[i].id = skill.id;
        sets_out[i].id2 = skill.id2;
        sets_out[i].race_lock = skill.race_lock;
        sets_out[i].type = skill.type;
        sets_out[i].unk_0E = skill.unk_0E;
        sets_out[i].partset = skill.partset;
        sets_out[i].unk_12 = skill.unk_12;

        for (size_t j = 0; j < 7; j++)
        {
            if (skill.paths[j].length() == 0)
            {
                sets_out[i].paths_offsets[j] = 0;
                continue;
            }

            if (strings_list.find(skill.paths[j]) == strings_list.end())
            {
                sets_out[i].paths_offsets[j] = Utils::DifPointer(*str_current, top);

                strings_list.insert(skill.paths[j]);
                strcpy(*str_current, skill.paths[j].c_str());
                *str_current += skill.paths[j].length() + 1;
            }
            else
            {
                const char *str = FindString(str_top, skill.paths[j].c_str(), strings_list.size());
                if (!str)
                {
                    DPRINTF("%s: Internal coding error.\n", FUNCNAME);
                    exit(-1);
                }

                sets_out[i].paths_offsets[j] = Utils::DifPointer(str, top);
            }
        }

        sets_out[i].unk_30 = skill.unk_30;
        sets_out[i].unk_32 = skill.unk_32;
        sets_out[i].unk_34 = skill.unk_34;
        sets_out[i].skill_type = skill.skill_type;
        sets_out[i].pup_id = skill.pup_id;
        sets_out[i].aura = skill.aura;
        sets_out[i].model = skill.model;
        sets_out[i].change_skillset = skill.change_skillset;
        sets_out[i].num_transforms = skill.num_transforms;
        sets_out[i].unk_44 = skill.unk_44;
    }
}

void CusFile::SaveSkills121(uint8_t *top, char *str_top, char **str_current, const std::vector<CusSkill> &sets_in, CUSSkill121 *sets_out, std::unordered_set<std::string> &strings_list)
{
    for (size_t i = 0; i < sets_in.size(); i++)
    {
        const CusSkill &skill = sets_in[i];

        strcpy(sets_out[i].name, skill.name.c_str());
        sets_out[i].id = skill.id;
        sets_out[i].id2 = skill.id2;
        sets_out[i].race_lock = skill.race_lock;
        sets_out[i].type = skill.type;
        sets_out[i].unk_0E = skill.unk_0E;
        sets_out[i].partset = skill.partset;
        sets_out[i].unk_12 = skill.unk_12;

        for (size_t j = 0; j < 7; j++)
        {
            if (skill.paths[j].length() == 0)
            {
                sets_out[i].paths_offsets[j] = 0;
                continue;
            }

            if (strings_list.find(skill.paths[j]) == strings_list.end())
            {
                sets_out[i].paths_offsets[j] = Utils::DifPointer(*str_current, top);

                strings_list.insert(skill.paths[j]);
                strcpy(*str_current, skill.paths[j].c_str());
                *str_current += skill.paths[j].length() + 1;
            }
            else
            {
                const char *str = FindString(str_top, skill.paths[j].c_str(), strings_list.size());
                if (!str)
                {
                    DPRINTF("%s: Internal coding error.\n", FUNCNAME);
                    exit(-1);
                }

                sets_out[i].paths_offsets[j] = Utils::DifPointer(str, top);
            }
        }

        sets_out[i].unk_30 = skill.unk_30;
        sets_out[i].unk_32 = skill.unk_32;
        sets_out[i].unk_34 = skill.unk_34;
        sets_out[i].skill_type = skill.skill_type;
        sets_out[i].pup_id = skill.pup_id;
        sets_out[i].aura = skill.aura;
        sets_out[i].model = skill.model;
        sets_out[i].change_skillset = skill.change_skillset;
        sets_out[i].num_transforms = skill.num_transforms;
        sets_out[i].unk_44 = skill.unk_44;
        sets_out[i].unk_48 = skill.unk_48;
    }
}

static bool skills_sorter(const CusSkill &a, const CusSkill &b)
{
    return (a.id < b.id);
}

uint8_t *CusFile::Save(size_t *psize)
{
    std::sort(super_skills.begin(), super_skills.end(), skills_sorter);
    std::sort(ultimate_skills.begin(), ultimate_skills.end(), skills_sorter);
    std::sort(evasive_skills.begin(), evasive_skills.end(), skills_sorter);
    std::sort(blast_skills.begin(), blast_skills.end(), skills_sorter);
    std::sort(awaken_skills.begin(), awaken_skills.end(), skills_sorter);

    std::unordered_set<std::string> strings_list;
    size_t size = CalculateFileSize();

    uint8_t *buf = new uint8_t[size];
    memset(buf, 0, size);

    CUSHeader *hdr = (CUSHeader *)buf;
    CUSSkillSet *skill_set_f = (CUSSkillSet *)(hdr+1);


    for (size_t i = 0; i < skill_sets.size(); i++)
    {
        const CusSkillSet &skill_set = skill_sets[i];

        skill_set_f[i].char_id = skill_set.char_id;
        skill_set_f[i].costume_id = skill_set.costume_id;
        memcpy(skill_set_f[i].char_skills, skill_set.char_skills, sizeof(skill_set.char_skills));
        skill_set_f[i].model_preset = skill_set.model_preset;
    }

    if (version == 121)
    {
        CUSSkill121 *super_skills_f = (CUSSkill121 *)(skill_set_f+skill_sets.size());
        CUSSkill121 *ultimate_skills_f = (CUSSkill121 *)(super_skills_f+super_skills.size());
        CUSSkill121 *evasive_skills_f = (CUSSkill121 *)(ultimate_skills_f+ultimate_skills.size());
        CUSSkill121 *unk_skills_f = (CUSSkill121 *)(evasive_skills_f+evasive_skills.size());
        CUSSkill121 *blast_skills_f = (CUSSkill121 *)(unk_skills_f+unk_skills.size());
        CUSSkill121 *awaken_skills_f = (CUSSkill121 *)(blast_skills_f+blast_skills.size());
        char *strings = (char *)(awaken_skills_f+awaken_skills.size());
        char *current_str = strings;

        SaveSkills121(buf, strings, &current_str, super_skills, super_skills_f, strings_list);
        SaveSkills121(buf, strings, &current_str, ultimate_skills, ultimate_skills_f, strings_list);
        SaveSkills121(buf, strings, &current_str, evasive_skills, evasive_skills_f, strings_list);
        SaveSkills121(buf, strings, &current_str, unk_skills, unk_skills_f, strings_list);
        SaveSkills121(buf, strings, &current_str, blast_skills, blast_skills_f, strings_list);
        SaveSkills121(buf, strings, &current_str, awaken_skills, awaken_skills_f, strings_list);

        hdr->signature = CUS_SIGNATURE;
        hdr->endianess_check = 0xFFFE;
        hdr->num_skillsets = (uint32_t)skill_sets.size();
        hdr->skillset_offset = Utils::DifPointer(skill_set_f, buf);

        hdr->num_super = (uint32_t)super_skills.size();
        hdr->num_ultimate = (uint32_t)ultimate_skills.size();
        hdr->num_evasive = (uint32_t)evasive_skills.size();
        hdr->num_unk = (uint32_t)unk_skills.size();
        hdr->num_blast = (uint32_t)blast_skills.size();
        hdr->num_awaken = (uint32_t)awaken_skills.size();

        hdr->super_offset = Utils::DifPointer(super_skills_f, buf);
        hdr->ultimate_offset = Utils::DifPointer(ultimate_skills_f, buf);
        hdr->evasive_offset = Utils::DifPointer(evasive_skills_f, buf);
        hdr->unk_offset = Utils::DifPointer(unk_skills_f, buf);
        hdr->blast_offset = Utils::DifPointer(blast_skills_f, buf);
        hdr->awaken_offset = Utils::DifPointer(awaken_skills_f, buf);
    }

    else if (version == 119)
    {
        CUSSkill119 *super_skills_f = (CUSSkill119 *)(skill_set_f+skill_sets.size());
        CUSSkill119 *ultimate_skills_f = (CUSSkill119 *)(super_skills_f+super_skills.size());
        CUSSkill119 *evasive_skills_f = (CUSSkill119 *)(ultimate_skills_f+ultimate_skills.size());
        CUSSkill119 *unk_skills_f = (CUSSkill119 *)(evasive_skills_f+evasive_skills.size());
        CUSSkill119 *blast_skills_f = (CUSSkill119 *)(unk_skills_f+unk_skills.size());
        CUSSkill119 *awaken_skills_f = (CUSSkill119 *)(blast_skills_f+blast_skills.size());
        char *strings = (char *)(awaken_skills_f+awaken_skills.size());
        char *current_str = strings;

        SaveSkills119(buf, strings, &current_str, super_skills, super_skills_f, strings_list);
        SaveSkills119(buf, strings, &current_str, ultimate_skills, ultimate_skills_f, strings_list);
        SaveSkills119(buf, strings, &current_str, evasive_skills, evasive_skills_f, strings_list);
        SaveSkills119(buf, strings, &current_str, unk_skills, unk_skills_f, strings_list);
        SaveSkills119(buf, strings, &current_str, blast_skills, blast_skills_f, strings_list);
        SaveSkills119(buf, strings, &current_str, awaken_skills, awaken_skills_f, strings_list);

        hdr->signature = CUS_SIGNATURE;
        hdr->endianess_check = 0xFFFE;
        hdr->num_skillsets = (uint32_t)skill_sets.size();
        hdr->skillset_offset = Utils::DifPointer(skill_set_f, buf);

        hdr->num_super = (uint32_t)super_skills.size();
        hdr->num_ultimate = (uint32_t)ultimate_skills.size();
        hdr->num_evasive = (uint32_t)evasive_skills.size();
        hdr->num_unk = (uint32_t)unk_skills.size();
        hdr->num_blast = (uint32_t)blast_skills.size();
        hdr->num_awaken = (uint32_t)awaken_skills.size();

        hdr->super_offset = Utils::DifPointer(super_skills_f, buf);
        hdr->ultimate_offset = Utils::DifPointer(ultimate_skills_f, buf);
        hdr->evasive_offset = Utils::DifPointer(evasive_skills_f, buf);
        hdr->unk_offset = Utils::DifPointer(unk_skills_f, buf);
        hdr->blast_offset = Utils::DifPointer(blast_skills_f, buf);
        hdr->awaken_offset = Utils::DifPointer(awaken_skills_f, buf);
    }
    else
    {
        CUSSkill *super_skills_f = (CUSSkill *)(skill_set_f+skill_sets.size());
        CUSSkill *ultimate_skills_f = (CUSSkill *)(super_skills_f+super_skills.size());
        CUSSkill *evasive_skills_f = (CUSSkill *)(ultimate_skills_f+ultimate_skills.size());
        CUSSkill *unk_skills_f = (CUSSkill *)(evasive_skills_f+evasive_skills.size());
        CUSSkill *blast_skills_f = (CUSSkill *)(unk_skills_f+unk_skills.size());
        CUSSkill *awaken_skills_f = (CUSSkill *)(blast_skills_f+blast_skills.size());
        char *strings = (char *)(awaken_skills_f+awaken_skills.size());
        char *current_str = strings;

        SaveSkills(buf, strings, &current_str, super_skills, super_skills_f, strings_list);
        SaveSkills(buf, strings, &current_str, ultimate_skills, ultimate_skills_f, strings_list);
        SaveSkills(buf, strings, &current_str, evasive_skills, evasive_skills_f, strings_list);
        SaveSkills(buf, strings, &current_str, unk_skills, unk_skills_f, strings_list);
        SaveSkills(buf, strings, &current_str, blast_skills, blast_skills_f, strings_list);
        SaveSkills(buf, strings, &current_str, awaken_skills, awaken_skills_f, strings_list);

        hdr->signature = CUS_SIGNATURE;
        hdr->endianess_check = 0xFFFE;
        hdr->num_skillsets = (uint32_t)skill_sets.size();
        hdr->skillset_offset = Utils::DifPointer(skill_set_f, buf);

        hdr->num_super = (uint32_t)super_skills.size();
        hdr->num_ultimate = (uint32_t)ultimate_skills.size();
        hdr->num_evasive = (uint32_t)evasive_skills.size();
        hdr->num_unk = (uint32_t)unk_skills.size();
        hdr->num_blast = (uint32_t)blast_skills.size();
        hdr->num_awaken = (uint32_t)awaken_skills.size();

        hdr->super_offset = Utils::DifPointer(super_skills_f, buf);
        hdr->ultimate_offset = Utils::DifPointer(ultimate_skills_f, buf);
        hdr->evasive_offset = Utils::DifPointer(evasive_skills_f, buf);
        hdr->unk_offset = Utils::DifPointer(unk_skills_f, buf);
        hdr->blast_offset = Utils::DifPointer(blast_skills_f, buf);
        hdr->awaken_offset = Utils::DifPointer(awaken_skills_f, buf);
    }

    *psize = size;
    return buf;
}

TiXmlElement *CusFile::DecompileSkills(const char *name, TiXmlNode *root, const std::vector<CusSkill> &skills, int type) const
{
    TiXmlElement *entry_root = new TiXmlElement(name);

    for (const CusSkill &skill : skills)
    {
        TiXmlElement *elem = skill.Decompile(entry_root);
        if (!elem)
            return nullptr;

        std::string skill_name;

        if (type == 0)
            Xenoverse2::GetSuperSkillName(skill.id2, skill_name);
        else if (type == 1)
            Xenoverse2::GetUltimateSkillName(skill.id2, skill_name);
        else if (type == 2)
            Xenoverse2::GetEvasiveSkillName(skill.id2, skill_name);
        else if (type == 5)
            Xenoverse2::GetAwakenSkillName(skill.id2, skill_name);

        if (skill_name.length() != 0)
        {
            TiXmlComment comment;
            comment.SetValue(skill_name);

            const TiXmlElement *race_lock;
            if (Utils::GetElemCount(elem, "RACE_LOCK", &race_lock) != 0)
                elem->InsertBeforeChild(const_cast<TiXmlElement *>(race_lock), comment);
        }
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool CusFile::CompileSkills(const TiXmlElement *root, std::vector<CusSkill> &skills)
{
    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Skill")
        {
           CusSkill skill;

           if (!skill.Compile(elem, &version))
               return false;

           skills.push_back(skill);
        }
    }

    return true;
}

TiXmlDocument *CusFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("CUS");

    for (const CusSkillSet &ss : skill_sets)
    {
        ss.Decompile(root, this);
    }

    DecompileSkills("SuperSkills", root, super_skills, 0);
    DecompileSkills("UltimateSkills", root, ultimate_skills, 1);
    DecompileSkills("EvasiveSkills", root, evasive_skills, 2);
    DecompileSkills("UnkSkills", root, unk_skills, 3);
    DecompileSkills("BlastSkills", root, blast_skills, 4);
    DecompileSkills("AwakenSkills", root, awaken_skills, 5);

    doc->LinkEndChild(root);
    return doc;
}

bool CusFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "CUS");

    if (!root)
    {
        DPRINTF("Cannot find\"CUS\" in xml.\n");
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        const std::string &str = elem->ValueStr();

        if (str == "SkillSet")
        {
            CusSkillSet ss;

            if (!ss.Compile(elem))
                return false;

            skill_sets.push_back(ss);
        }
        else if (str == "SuperSkills")
        {
            if (!CompileSkills(elem, super_skills))
                return false;
        }
        else if (str == "UltimateSkills")
        {
            if (!CompileSkills(elem, ultimate_skills))
                return false;
        }
        else if (str == "EvasiveSkills")
        {
            if (!CompileSkills(elem, evasive_skills))
                return false;
        }
        else if (str == "UnkSkills")
        {
            if (!CompileSkills(elem, unk_skills))
                return false;
        }
        else if (str == "BlastSkills")
        {
            if (!CompileSkills(elem, blast_skills))
                return false;
        }
        else if (str == "AwakenSkills")
        {
            if (!CompileSkills(elem, awaken_skills))
                return false;
        }
    }

    return true;
}

size_t CusFile::FindSkillSetsByCharID(uint32_t char_id, std::vector<CusSkillSet *> &sets)
{
    sets.clear();

    for (CusSkillSet &set : skill_sets)
    {
        if (set.char_id == char_id)
        {
            sets.push_back(&set);
        }
    }

    return sets.size();
}

bool CusFile::AddSkillSet(const CusSkillSet &set, bool unique_char_id)
{
    if (unique_char_id)
    {
        std::vector<CusSkillSet *> temp;

        if (FindSkillSetsByCharID(set.char_id, temp) != 0)
            return false;
    }

    skill_sets.push_back(set);
    return true;
}

size_t CusFile::RemoveSkillSets(uint32_t char_id)
{
    size_t removed = 0;

    for (size_t i = 0; i < skill_sets.size(); i++)
    {
        if (skill_sets[i].char_id == char_id)
        {
            skill_sets.erase(skill_sets.begin()+i);
            i--;
            removed++;
        }
    }

    return removed;
}

CusSkill *CusFile::FindSuperSkillByID(uint32_t id)
{
    for (CusSkill &skill : super_skills)
    {
        if (skill.id == id)
            return &skill;
    }

    return nullptr;
}

const CusSkill *CusFile::FindSuperSkillByID(uint32_t id) const
{
    for (const CusSkill &skill : super_skills)
    {
        if (skill.id == id)
            return &skill;
    }

    return nullptr;
}

CusSkill *CusFile::FindUltimateSkillByID(uint32_t id)
{
    for (CusSkill &skill : ultimate_skills)
    {
        if (skill.id == id)
            return &skill;
    }

    return nullptr;
}

const CusSkill *CusFile::FindUltimateSkillByID(uint32_t id) const
{
    for (const CusSkill &skill : ultimate_skills)
    {
        if (skill.id == id)
            return &skill;
    }

    return nullptr;
}

CusSkill *CusFile::FindEvasiveSkillByID(uint32_t id)
{
    for (CusSkill &skill : evasive_skills)
    {
        if (skill.id == id)
            return &skill;
    }

    return nullptr;
}

const CusSkill *CusFile::FindEvasiveSkillByID(uint32_t id) const
{
    for (const CusSkill &skill : evasive_skills)
    {
        if (skill.id == id)
            return &skill;
    }

    return nullptr;
}

CusSkill *CusFile::FindBlastSkillByID(uint32_t id)
{
    for (CusSkill &skill : blast_skills)
    {
        if (skill.id == id)
            return &skill;
    }

    return nullptr;
}

const CusSkill *CusFile::FindBlastSkillByID(uint32_t id) const
{
    for (const CusSkill &skill : blast_skills)
    {
        if (skill.id == id)
            return &skill;
    }

    return nullptr;
}

CusSkill *CusFile::FindAwakenSkillByID(uint32_t id)
{
    for (CusSkill &skill : awaken_skills)
    {
        if (skill.id == id)
            return &skill;
    }

    return nullptr;
}

const CusSkill *CusFile::FindAwakenSkillByID(uint32_t id) const
{
    for (const CusSkill &skill : awaken_skills)
    {
        if (skill.id == id)
            return &skill;
    }

    return nullptr;
}

CusSkill *CusFile::FindSkillAnyByID(uint32_t id)
{
    CusSkill *skill;

    skill = FindSuperSkillByID(id);
    if (skill)
        return skill;

    skill = FindUltimateSkillByID(id);
    if (skill)
        return skill;

    skill = FindEvasiveSkillByID(id);
    if (skill)
        return skill;

    skill = FindBlastSkillByID(id);
    if (skill)
        return skill;

    return FindAwakenSkillByID(id);
}

const CusSkill *CusFile::FindSkillAnyByID(uint32_t id) const
{
    const CusSkill *skill;

    skill = FindSuperSkillByID(id);
    if (skill)
        return skill;

    skill = FindUltimateSkillByID(id);
    if (skill)
        return skill;

    skill = FindEvasiveSkillByID(id);
    if (skill)
        return skill;

    skill = FindBlastSkillByID(id);
    if (skill)
        return skill;

    return FindAwakenSkillByID(id);
}

size_t CusFile::FindSuperSkillByName(const std::string &name, std::vector<CusSkill *> &skills)
{
    skills.clear();

    for (CusSkill &skill : super_skills)
    {
        if (skill.name == name)
            skills.push_back(&skill);
    }

    return skills.size();
}

size_t CusFile::FindUltimateSkillByName(const std::string &name, std::vector<CusSkill *> &skills)
{
    skills.clear();

    for (CusSkill &skill : ultimate_skills)
    {
        if (skill.name == name)
            skills.push_back(&skill);
    }

    return skills.size();
}

size_t CusFile::FindEvasiveSkillByName(const std::string &name, std::vector<CusSkill *> &skills)
{
    skills.clear();

    for (CusSkill &skill : evasive_skills)
    {
        if (skill.name == name)
            skills.push_back(&skill);
    }

    return skills.size();
}

size_t CusFile::FindBlastSkillByName(const std::string &name, std::vector<CusSkill *> &skills)
{
    skills.clear();

    for (CusSkill &skill : blast_skills)
    {
        if (skill.name == name)
            skills.push_back(&skill);
    }

    return skills.size();
}

size_t CusFile::FindAwakenSkillByName(const std::string &name, std::vector<CusSkill *> &skills)
{
    skills.clear();

    for (CusSkill &skill : awaken_skills)
    {
        if (skill.name == name)
            skills.push_back(&skill);
    }

    return skills.size();
}

size_t CusFile::FindAnySkillByName(const std::string &name, std::vector<CusSkill *> &skills)
{
    skills.clear();

    for (CusSkill &skill : super_skills)
    {
        if (skill.name == name)
            skills.push_back(&skill);
    }

    for (CusSkill &skill : ultimate_skills)
    {
        if (skill.name == name)
            skills.push_back(&skill);
    }

    for (CusSkill &skill : evasive_skills)
    {
        if (skill.name == name)
            skills.push_back(&skill);
    }

    for (CusSkill &skill : blast_skills)
    {
        if (skill.name == name)
            skills.push_back(&skill);
    }

    for (CusSkill &skill : awaken_skills)
    {
        if (skill.name == name)
            skills.push_back(&skill);
    }

    return skills.size();
}

bool CusFile::IsSkillInUse(uint32_t char_id, int slot, int type) const
{
    if (slot >= 10 || char_id >= 500)
        return true; // Yes, true

    if (type == CUS_SKILL_TYPE_SUPER)
    {
        return (FindSuperSkillByID((char_id*10)+slot+CUS_SUPER_START) != nullptr);
    }
    else if (type == CUS_SKILL_TYPE_ULTIMATE)
    {
        return (FindUltimateSkillByID((char_id*10)+slot+CUS_ULTIMATE_START) != nullptr);
    }
    else if (type == CUS_SKILL_TYPE_EVASIVE)
    {
        return (FindEvasiveSkillByID((char_id*10)+slot+CUS_EVASIVE_START) != nullptr);
    }
    else if (type == CUS_SKILL_TYPE_BLAST)
    {
        return (FindBlastSkillByID((char_id*10)+slot+CUS_BLAST_START) != nullptr);
    }
    else if (type == CUS_SKILL_TYPE_AWAKEN)
    {
        return (FindAwakenSkillByID((char_id*10)+slot+CUS_AWAKEN_START) != nullptr);
    }

    return true; // Dummy return
}

bool CusFile::AddSuperSkill(const CusSkill &skill)
{
    if (skill.id >= (CUS_SUPER_START+5000) || skill.id2 >= 5000)
        return false;

    if (FindSuperSkillByID(skill.id))
        return false;

    super_skills.push_back(skill);
    return true;
}

bool CusFile::AddUltimateSkill(const CusSkill &skill)
{
    if (skill.id >= (CUS_ULTIMATE_START+5000) || skill.id2 >= 5000)
        return false;

    if (FindUltimateSkillByID(skill.id))
        return false;

    ultimate_skills.push_back(skill);
    return true;
}

bool CusFile::AddEvasiveSkill(const CusSkill &skill)
{
    if (skill.id >= (CUS_EVASIVE_START+5000) || skill.id2 >= 5000)
        return false;

    if (FindEvasiveSkillByID(skill.id))
        return false;

    evasive_skills.push_back(skill);
    return true;
}

bool CusFile::AddBlastSkill(const CusSkill &skill)
{
    if (skill.id >= (CUS_BLAST_START+5000) || skill.id2 >= 5000)
        return false;

    if (FindBlastSkillByID(skill.id))
        return false;

    blast_skills.push_back(skill);
    return true;
}

bool CusFile::AddAwakenSkill(const CusSkill &skill)
{
    if (skill.id >= (CUS_AWAKEN_START+5000) || skill.id2 >= 5000)
        return false;

    if (FindAwakenSkillByID(skill.id))
        return false;

    awaken_skills.push_back(skill);
    return true;
}

bool CusFile::AddSkillAuto(const CusSkill &skill)
{
    if (/*skill.id >= 0 && */skill.id < CUS_ULTIMATE_START)
        return AddSuperSkill(skill);

    if (skill.id >= CUS_ULTIMATE_START && skill.id < CUS_EVASIVE_START)
        return AddUltimateSkill(skill);

    if (skill.id >= CUS_EVASIVE_START && skill.id < CUS_UNK_START)
        return AddEvasiveSkill(skill);

    if (skill.id >= CUS_BLAST_START && skill.id < CUS_AWAKEN_START)
        return AddBlastSkill(skill);

    if (skill.id >= CUS_AWAKEN_START && skill.id < CUS_ID_END)
        return AddAwakenSkill(skill);

    return false;
}

void CusFile::RemoveSuperSkill(uint16_t id, bool *existed)
{
    if (existed)
        *existed = false;

    for (size_t i = 0; i < super_skills.size(); i++)
    {
        if (super_skills[i].id == id)
        {
            super_skills.erase(super_skills.begin()+i);
            i--;

            if (existed)
                *existed = true;
        }
    }
}

void CusFile::RemoveUltimateSkill(uint16_t id, bool *existed)
{
    if (existed)
        *existed = false;

    for (size_t i = 0; i < ultimate_skills.size(); i++)
    {
        if (ultimate_skills[i].id == id)
        {
            ultimate_skills.erase(ultimate_skills.begin()+i);
            i--;

            if (existed)
                *existed = true;
        }
    }
}

void CusFile::RemoveEvasiveSkill(uint16_t id, bool *existed)
{
    if (existed)
        *existed = false;

    for (size_t i = 0; i < evasive_skills.size(); i++)
    {
        if (evasive_skills[i].id == id)
        {
            evasive_skills.erase(evasive_skills.begin()+i);
            i--;

            if (existed)
                *existed = true;
        }
    }
}

void CusFile::RemoveBlastSkill(uint16_t id, bool *existed)
{
    if (existed)
        *existed = false;

    for (size_t i = 0; i < blast_skills.size(); i++)
    {
        if (blast_skills[i].id == id)
        {
            blast_skills.erase(blast_skills.begin()+i);
            i--;

            if (existed)
                *existed = true;
        }
    }
}

void CusFile::RemoveAwakenSkill(uint16_t id, bool *existed)
{
    if (existed)
        *existed = false;

    for (size_t i = 0; i < awaken_skills.size(); i++)
    {
        if (awaken_skills[i].id == id)
        {
            awaken_skills.erase(awaken_skills.begin()+i);
            i--;

            if (existed)
                *existed = true;
        }
    }
}

void CusFile::RemoveSkillAuto(uint16_t id, bool *existed)
{
    if (/*id >= 0 && */id < CUS_ULTIMATE_START)
    {
        RemoveSuperSkill(id, existed);
        return;
    }

    else if (id >= CUS_ULTIMATE_START && id < CUS_EVASIVE_START)
    {
        RemoveUltimateSkill(id, existed);
        return;
    }

    else if (id >= CUS_EVASIVE_START && id < CUS_UNK_START)
    {
        RemoveEvasiveSkill(id, existed);
        return;
    }

    else if (id >= CUS_BLAST_START && id < CUS_AWAKEN_START)
    {
        RemoveBlastSkill(id, existed);
        return;
    }

    else if (id >= CUS_AWAKEN_START && id < CUS_ID_END)
    {
        RemoveAwakenSkill(id, existed);
        return;
    }

    if (existed)
        *existed = false;
}

size_t CusFile::RemoveAllReferencesToSkill(uint16_t id)
{
    size_t count = 0;

    for (CusSkillSet &set : skill_sets)
    {
        for (int i = 0; i < 9; i++)
        {
            if (set.char_skills[i] == id)
            {
                set.char_skills[i] = 0xFFFF;
                count++;
            }
        }
    }

    return count;
}

size_t CusFile::FindReferencesToPupId(uint16_t pup_id, std::vector<CusSkill *> &skills)
{
    skills.clear();

    if (pup_id == 0xFFFF)
        return 0;

    for (CusSkill &skill : super_skills)
    {
        if (skill.pup_id == pup_id)
        {
            skills.push_back(&skill);
        }
    }

    for (CusSkill &skill : ultimate_skills)
    {
        if (skill.pup_id == pup_id)
        {
            skills.push_back(&skill);
        }
    }

    for (CusSkill &skill : evasive_skills)
    {
        if (skill.pup_id == pup_id)
        {
            skills.push_back(&skill);
        }
    }

    for (CusSkill &skill : blast_skills)
    {
        if (skill.pup_id == pup_id)
        {
            skills.push_back(&skill);
        }
    }

    for (CusSkill &skill : awaken_skills)
    {
        if (skill.pup_id == pup_id)
        {
            skills.push_back(&skill);
        }
    }

    return skills.size();
}
