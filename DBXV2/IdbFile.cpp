#include "IdbFile.h"

#ifndef IDB_SIMPLE
#include "Xenoverse2.h"
#endif

#include "debug.h"

#define COPY_EI(n) effect.n = file_effect->n
#define COPY_EO(n) file_effect->n = effect.n

#define COPY_EIN(n) effect.n = file_effect_new->n
#define COPY_EON(n) file_effect_new->n = effect.n

TiXmlElement *IdbEffect::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Effect");

    Utils::WriteParamUnsigned(entry_root, "TYPE", type, true);
    Utils::WriteParamUnsigned(entry_root, "ACTIVATION_TYPE", activation_type, true);
    Utils::WriteParamUnsigned(entry_root, "NUM_ACT_TIMES", num_act_times, true);
    Utils::WriteParamFloat(entry_root, "TIMER", timer);
    Utils::WriteParamMultipleFloats(entry_root, "ABILITY_VALUES", std::vector<float>(ability_values, ability_values+6));
    Utils::WriteParamUnsigned(entry_root, "U_28", unk_28, true);
    Utils::WriteParamUnsigned(entry_root, "ACTIVATION_CHANCE", activation_chance, true);
    Utils::WriteParamMultipleFloats(entry_root, "MULTIPLIERS", std::vector<float>(multipliers, multipliers+6));
    Utils::WriteParamMultipleUnsigned(entry_root, "U_48", std::vector<uint32_t>(unk_48, unk_48+6), true);
    Utils::WriteParamFloat(entry_root, "HEA", hea);
    Utils::WriteParamFloat(entry_root, "KI", ki);
    Utils::WriteParamFloat(entry_root, "KI_RECOVERY", ki_recovery);
    Utils::WriteParamFloat(entry_root, "STM", stm);
    Utils::WriteParamFloat(entry_root, "STAMINA_RECOVERY", stamina_recovery);
    Utils::WriteParamFloat(entry_root, "ENEMY_STAMINA_ERASER", enemy_stamina_eraser);
    Utils::WriteParamFloat(entry_root, "F_78", unk_78);
    Utils::WriteParamFloat(entry_root, "GROUND_SPEED", ground_speed);
    Utils::WriteParamFloat(entry_root, "AIR_SPEED", air_speed);
    Utils::WriteParamFloat(entry_root, "BOOSTING_SPEED", boosting_speed);
    Utils::WriteParamFloat(entry_root, "DASH_SPEED", dash_speed);
    Utils::WriteParamFloat(entry_root, "ATK", atk);
    Utils::WriteParamFloat(entry_root, "BASIC_KI_ATTACK", basic_ki_attack);
    Utils::WriteParamFloat(entry_root, "STR", str);
    Utils::WriteParamFloat(entry_root, "BLA", bla);   
    Utils::WriteParamFloat(entry_root, "ATK_DAMAGE", atk_damage);
    Utils::WriteParamFloat(entry_root, "KI_DAMAGE", ki_damage);
    Utils::WriteParamFloat(entry_root, "STR_DAMAGE", str_damage);
    Utils::WriteParamFloat(entry_root, "BLA_DAMAGE", bla_damage);
    Utils::WriteParamMultipleFloats(entry_root, "F_AC", std::vector<float>(unk_AC, unk_AC+13));
    Utils::WriteParamMultipleUnsigned(entry_root, "NU_48", std::vector<uint32_t>(new_unk_48, new_unk_48+2), true);
    Utils::WriteParamUnsigned(entry_root, "NU_0C", new_unk_0C, true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool IdbEffect::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "TYPE", &type))
        return false;

    if (!Utils::GetParamUnsigned(root, "ACTIVATION_TYPE", &activation_type))
        return false;

    if (!Utils::GetParamUnsigned(root, "NUM_ACT_TIMES", &num_act_times))
        return false;

    if (!Utils::GetParamFloat(root, "TIMER", &timer))
        return false;

    if (!Utils::GetParamMultipleFloats(root, "ABILITY_VALUES", ability_values, 6))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_28", &unk_28))
        return false;

    if (!Utils::GetParamUnsigned(root, "ACTIVATION_CHANCE", &activation_chance))
        return false;

    if (!Utils::GetParamMultipleFloats(root, "MULTIPLIERS", multipliers, 6))
        return false;

    if (!Utils::GetParamMultipleUnsigned(root, "U_48", unk_48, 6))
        return false;

    if (!Utils::GetParamFloat(root, "HEA", &hea))
        return false;

    if (!Utils::GetParamFloat(root, "KI", &ki))
        return false;

    if (!Utils::GetParamFloat(root, "KI_RECOVERY", &ki_recovery))
        return false;

    if (!Utils::GetParamFloat(root, "STM", &stm))
        return false;

    if (!Utils::GetParamFloat(root, "STAMINA_RECOVERY", &stamina_recovery))
        return false;

    if (!Utils::GetParamFloat(root, "ENEMY_STAMINA_ERASER", &enemy_stamina_eraser))
        return false;

    if (!Utils::GetParamFloat(root, "F_78", &unk_78))
        return false;

    if (!Utils::GetParamFloat(root, "GROUND_SPEED", &ground_speed))
        return false;

    if (!Utils::GetParamFloat(root, "AIR_SPEED", &air_speed))
        return false;

    if (!Utils::GetParamFloat(root, "BOOSTING_SPEED", &boosting_speed))
        return false;

    if (!Utils::GetParamFloat(root, "DASH_SPEED", &dash_speed))
        return false;

    if (!Utils::GetParamFloat(root, "ATK", &atk))
        return false;

    if (!Utils::GetParamFloat(root, "BASIC_KI_ATTACK", &basic_ki_attack))
        return false;

    if (!Utils::GetParamFloat(root, "STR", &str))
        return false;

    if (!Utils::GetParamFloat(root, "BLA", &bla))
        return false;

    if (!Utils::GetParamFloat(root, "ATK_DAMAGE", &atk_damage))
        return false;

    if (!Utils::GetParamFloat(root, "KI_DAMAGE", &ki_damage))
        return false;

    if (!Utils::GetParamFloat(root, "STR_DAMAGE", &str_damage))
        return false;

    if (!Utils::GetParamFloat(root, "BLA_DAMAGE", &bla_damage))
        return false;

    if (!Utils::GetParamMultipleFloats(root, "F_AC", unk_AC, 13))
        return false;

    if (!Utils::ReadParamMultipleUnsigned(root, "NU_48", new_unk_48, 2))
        new_unk_48[0] = new_unk_48[1] = 0;

    if (!Utils::ReadParamUnsigned(root, "NU_0C", &new_unk_0C))
        new_unk_0C = 0;

    return true;
}

TiXmlElement *IdbEntry::Decompile(TiXmlNode *root, IdbCommentType comm_type) const
{
    TiXmlElement *entry_root = new TiXmlElement("IdbEntry");

    entry_root->SetAttribute("id", Utils::UnsignedToString(id, false));
    entry_root->SetAttribute("type", Utils::UnsignedToString(type, true));

#ifndef IDB_SIMPLE
    std::string name;

    if (comm_type == IdbCommentType::SKILL)
    {
        bool ret = false;

        if (type == CUS_SKILL_TYPE_SUPER)
        {
            ret = Xenoverse2::GetSuperSkillName(name_id, name);
        }
        else if (type == CUS_SKILL_TYPE_ULTIMATE)
        {
            ret = Xenoverse2::GetUltimateSkillName(name_id, name);
        }
        else if (type == CUS_SKILL_TYPE_EVASIVE)
        {
            ret = Xenoverse2::GetEvasiveSkillName(name_id, name);
        }
        else if (type == CUS_SKILL_TYPE_AWAKEN)
        {
            ret = Xenoverse2::GetAwakenSkillName(name_id, name);
        }

        if (ret && name.length() > 0)
        {
            Utils::WriteComment(entry_root, name);
        }
    }
    else if (comm_type == IdbCommentType::COSTUME)
    {
        if (Xenoverse2::GetCacCostumeName(name_id, name))
            Utils::WriteComment(entry_root, name);
    }
    else if (comm_type == IdbCommentType::ACCESORY)
    {
        if (Xenoverse2::GetAccesoryName(name_id, name))
            Utils::WriteComment(entry_root, name);
    }
    else if (comm_type == IdbCommentType::TALISMAN)
    {
        if (Xenoverse2::GetTalismanName(name_id, name))
            Utils::WriteComment(entry_root, name);
    }
    else if (comm_type == IdbCommentType::MATERIAL)
    {
        if (Xenoverse2::GetMaterialName(name_id, name))
            Utils::WriteComment(entry_root, name);
    }
    else if (comm_type == IdbCommentType::BATTLE)
    {
        if (Xenoverse2::GetBattleName(name_id, name))
            Utils::WriteComment(entry_root, name);
    }
    else if (comm_type == IdbCommentType::EXTRA)
    {
        if (Xenoverse2::GetExtraName(name_id, name))
            Utils::WriteComment(entry_root, name);
    }
    else if (comm_type == IdbCommentType::PET)
    {
        if (Xenoverse2::GetPetName(name_id, name))
            Utils::WriteComment(entry_root, name);
    }
#endif

    Utils::WriteParamUnsigned(entry_root, "STARS", stars);
    Utils::WriteParamUnsigned(entry_root, "NAME_ID", name_id, true);
    Utils::WriteParamUnsigned(entry_root, "DESC_ID", desc_id, true);
    Utils::WriteParamUnsigned(entry_root, "HOW_ID", how_id, true);
    Utils::WriteParamUnsigned(entry_root, "DLC_FLAG", dlc_flag, true);
    Utils::WriteParamUnsigned(entry_root, "AVAILABILITY", availability, true);
    Utils::WriteParamUnsigned(entry_root, "U_0E", unk_0E, true);
    Utils::WriteParamUnsigned(entry_root, "BUY", buy);
    Utils::WriteParamUnsigned(entry_root, "SELL", sell);
    Utils::WriteParamUnsigned(entry_root, "RACELOCK", racelock, true);
    Utils::WriteParamUnsigned(entry_root, "TP", tp);
    Utils::WriteParamUnsigned(entry_root, "STP", stp);
    Utils::WriteParamUnsigned(entry_root, "MODEL", model);
    Utils::WriteParamMultipleUnsigned(entry_root, "U_24", std::vector<uint32_t>(unk_24, unk_24+3), true);
    Utils::WriteParamUnsigned(entry_root, "NU_0C", new_unk_0C, true);
    Utils::WriteParamUnsigned(entry_root, "NU_0A", new_unk_0A, true);
    Utils::WriteParamUnsigned(entry_root, "NU_2C", new_unk_2C);

    for (int i = 0; i < 3; i++)
        effects[i].Decompile(entry_root);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool IdbEntry::Compile(const TiXmlElement *root, int *version)
{
    if (version != nullptr)
        *version = 0;

    if (!Utils::ReadAttrUnsigned(root, "id", &id))
    {
        DPRINTF("%s: Cannot get id attribute.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "type", &type))
    {
        DPRINTF("%s: Cannot get type attribute.\n", FUNCNAME);
        return false;
    }

    if (!Utils::GetParamUnsigned(root, "STARS", &stars))
        return false;

    if (!Utils::GetParamUnsigned(root, "NAME_ID", &name_id))
        return false;

    if (!Utils::GetParamUnsigned(root, "DESC_ID", &desc_id))
        return false;

    if (!Utils::GetParamUnsignedWithMultipleNames(root, &dlc_flag, "DLC_FLAG", "U_0A"))
        return false;

    if (!Utils::GetParamUnsignedWithMultipleNames(root, &availability, "AVAILABILITY", "U_0C"))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0E", &unk_0E))
        return false;

    if (!Utils::GetParamUnsigned(root, "BUY", &buy))
        return false;

    if (!Utils::GetParamUnsigned(root, "SELL", &sell))
        return false;

    if (!Utils::GetParamUnsigned(root, "RACELOCK", &racelock))
        return false;

    if (!Utils::GetParamUnsigned(root, "TP", &tp))
        return false;

    if (!Utils::GetParamUnsigned(root, "MODEL", &model))
        return false;

    if (!Utils::GetParamMultipleUnsigned(root, "U_24", unk_24, 3))
        return false;

    if (Utils::ReadParamUnsigned(root, "NU_0C", &new_unk_0C))
    {
        if (version != nullptr)
            *version = 118;
    }
    else
    {
        new_unk_0C = 0xFFFFFFFF;
    }

    if (Utils::ReadParamUnsigned(root, "HOW_ID", &how_id))
    {
        if (version != nullptr)
            *version = 122;

        if (!Utils::GetParamUnsigned(root, "NU_0A", &new_unk_0A))
            return false;

        if (!Utils::GetParamUnsigned(root, "STP", &stp))
            return false;

        if (!Utils::GetParamUnsigned(root, "NU_2C", &new_unk_2C))
            return false;
    }
    else
    {
        how_id = 0;
        new_unk_0A = 0;
        stp = 0;
        new_unk_2C = 0;
    }

    int idx = 0;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Effect")
        {
            if (!effects[idx].Compile(elem))
                return false;

            idx++;
            if (idx == 3)
                break;
        }
    }

    if (idx != 3)
    {
        DPRINTF("%s: I was expecting 3 effects, but got %d\n", FUNCNAME, idx);
        return false;
    }

    return true;
}

IdbFile::IdbFile()
{
    this->big_endian = false;
    this->comm_type = IdbCommentType::NONE;
    this->version = 122;
}

IdbFile::~IdbFile()
{

}

void IdbFile::Reset()
{
    entries.clear();
    this->comm_type = IdbCommentType::NONE;
    this->version = 122;
}

bool IdbFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(IDBHeader))
        return false;

    const IDBHeader *hdr = (const IDBHeader *)buf;

    // Don't do idb signature, for compatibility with that odd program...
    /*if (hdr->signature != IDB_SIGNATURE)
        return false;*/

    if ((size - 0x10) == hdr->num_entries*sizeof(IDBEntry122))
        version = 122;
    else if ((size - 0x10) == hdr->num_entries*sizeof(IDBEntry118))
        version = 118;
    else if ((size - 0x10) == hdr->num_entries*sizeof(IDBEntry))
        version = 0;
    else
    {
        DPRINTF("%s: Possibly new IDB format.\n", FUNCNAME);
        return false;
    }

    const IDBEntry *file_entries = (const IDBEntry *)(buf + hdr->data_start);
    const IDBEntry118 *file_entries_118 = (const IDBEntry118 *)(buf + hdr->data_start);
    const IDBEntry122 *file_entries_122 = (const IDBEntry122 *)(buf + hdr->data_start);
    entries.resize(hdr->num_entries);

    for (size_t i = 0; i < entries.size(); i++)
    {
        IdbEntry &entry = entries[i];

        if (version == 122)
        {
            entry.id = file_entries_122[i].id;
            entry.stars = file_entries_122[i].stars;
            entry.name_id = file_entries_122[i].name_id;
            entry.desc_id = file_entries_122[i].desc_id;
            entry.type = file_entries_122[i].type;
            entry.dlc_flag = file_entries_122[i].unk_0A;
            entry.availability = file_entries_122[i].old_unk_0C;
            entry.unk_0E = file_entries_122[i].old_unk_0E;
            entry.buy = file_entries_122[i].buy;
            entry.sell = file_entries_122[i].sell;
            entry.racelock = file_entries_122[i].racelock;
            entry.tp = file_entries_122[i].tp;
            entry.model = file_entries_122[i].model;
            memcpy(entry.unk_24, file_entries_122[i].old_unk_24, sizeof(entry.unk_24));
            entry.new_unk_0C = file_entries_122[i].new_unk_0C;
            // New
            entry.how_id = file_entries_122[i].how_id;
            entry.new_unk_0A = file_entries_122[i].new_unk_0A;
            entry.stp = file_entries_122[i].stp;
            entry.new_unk_2C = file_entries_122[i].new_unk_2C;
        }
        else if (version == 118)
        {
            entry.id = file_entries_118[i].id;
            entry.stars = file_entries_118[i].stars;
            entry.name_id = file_entries_118[i].name_id;
            entry.desc_id = file_entries_118[i].desc_id;
            entry.type = file_entries_118[i].type;
            entry.dlc_flag = file_entries_118[i].unk_0A;
            entry.availability = file_entries_118[i].old_unk_0C;
            entry.unk_0E = file_entries_118[i].old_unk_0E;
            entry.buy = file_entries_118[i].buy;
            entry.sell = file_entries_118[i].sell;
            entry.racelock = file_entries_118[i].racelock;
            entry.tp = file_entries_118[i].tp;
            entry.model = file_entries_118[i].model;
            memcpy(entry.unk_24, file_entries_118[i].old_unk_24, sizeof(entry.unk_24));
            // New
            entry.new_unk_0C = file_entries_118[i].new_unk_0C;
        }
        else
        {
            entry.id = file_entries[i].id;
            entry.stars = file_entries[i].stars;
            entry.name_id = file_entries[i].name_id;
            entry.desc_id = file_entries[i].desc_id;
            entry.type = file_entries[i].type;
            entry.dlc_flag = file_entries[i].unk_0A;
            entry.availability = file_entries[i].unk_0C;
            entry.unk_0E = file_entries[i].unk_0E;
            entry.buy = file_entries[i].buy;
            entry.sell = file_entries[i].sell;
            entry.racelock = file_entries[i].racelock;
            entry.tp = file_entries[i].tp;
            entry.model = file_entries[i].model;
            memcpy(entry.unk_24, file_entries[i].unk_24, sizeof(entry.unk_24));
        }

        for (int j = 0; j < 3; j++)
        {
            IdbEffect &effect = entry.effects[j];

            if (version == 122)
            {
                const IDBEffect122 *file_effect_new = &file_entries_122[i].effects[j];

                COPY_EIN(type);
                COPY_EIN(activation_type);
                COPY_EIN(num_act_times);
                COPY_EIN(timer);
                memcpy(effect.ability_values, file_effect_new->ability_values, sizeof(effect.ability_values));
                COPY_EIN(unk_28);
                COPY_EIN(activation_chance);
                memcpy(effect.multipliers, file_effect_new->multipliers, sizeof(effect.multipliers));
                memcpy(effect.unk_48, file_effect_new->old_unk_48, sizeof(effect.unk_48));
                COPY_EIN(hea);
                COPY_EIN(ki);
                COPY_EIN(ki_recovery);
                COPY_EIN(stm);
                COPY_EIN(stamina_recovery);
                COPY_EIN(enemy_stamina_eraser);
                effect.unk_78 = file_effect_new->old_unk_78;
                COPY_EIN(ground_speed);
                COPY_EIN(air_speed);
                COPY_EIN(boosting_speed);
                COPY_EIN(dash_speed);
                COPY_EIN(atk);
                COPY_EIN(basic_ki_attack);
                COPY_EIN(str);
                COPY_EIN(bla);
                COPY_EIN(atk_damage);
                COPY_EIN(ki_damage);
                COPY_EIN(str_damage);
                COPY_EIN(bla_damage);
                memcpy(effect.unk_AC, file_effect_new->old_unk_AC, sizeof(effect.unk_AC));
                memcpy(effect.new_unk_48, file_effect_new->new_unk_48, sizeof(effect.new_unk_48));
                // New
                COPY_EIN(new_unk_0C);

            }
            else if (version == 118)
            {
                const IDBEffect118 *file_effect_new = &file_entries_118[i].effects[j];

                COPY_EIN(type);
                COPY_EIN(activation_type);
                COPY_EIN(num_act_times);
                COPY_EIN(timer);
                memcpy(effect.ability_values, file_effect_new->ability_values, sizeof(effect.ability_values));
                COPY_EIN(unk_28);
                COPY_EIN(activation_chance);
                memcpy(effect.multipliers, file_effect_new->multipliers, sizeof(effect.multipliers));
                memcpy(effect.unk_48, file_effect_new->old_unk_48, sizeof(effect.unk_48));
                COPY_EIN(hea);
                COPY_EIN(ki);
                COPY_EIN(ki_recovery);
                COPY_EIN(stm);
                COPY_EIN(stamina_recovery);
                COPY_EIN(enemy_stamina_eraser);
                effect.unk_78 = file_effect_new->old_unk_78;
                COPY_EIN(ground_speed);
                COPY_EIN(air_speed);
                COPY_EIN(boosting_speed);
                COPY_EIN(dash_speed);
                COPY_EIN(atk);
                COPY_EIN(basic_ki_attack);
                COPY_EIN(str);
                COPY_EIN(bla);
                COPY_EIN(atk_damage);
                COPY_EIN(ki_damage);
                COPY_EIN(str_damage);
                COPY_EIN(bla_damage);
                memcpy(effect.unk_AC, file_effect_new->old_unk_AC, sizeof(effect.unk_AC));
                // New
                memcpy(effect.new_unk_48, file_effect_new->new_unk_48, sizeof(effect.new_unk_48));
            }
            else
            {
                const IDBEffect *file_effect = &file_entries[i].effects[j];

                COPY_EI(type);
                COPY_EI(activation_type);
                COPY_EI(num_act_times);
                COPY_EI(timer);
                memcpy(effect.ability_values, file_effect->ability_values, sizeof(effect.ability_values));
                COPY_EI(unk_28);
                COPY_EI(activation_chance);
                memcpy(effect.multipliers, file_effect->multipliers, sizeof(effect.multipliers));
                memcpy(effect.unk_48, file_effect->unk_48, sizeof(effect.unk_48));
                COPY_EI(hea);
                COPY_EI(ki);
                COPY_EI(ki_recovery);
                COPY_EI(stm);
                COPY_EI(stamina_recovery);
                COPY_EI(enemy_stamina_eraser);
                COPY_EI(unk_78);
                COPY_EI(ground_speed);
                COPY_EI(air_speed);
                COPY_EI(boosting_speed);
                COPY_EI(dash_speed);
                COPY_EI(atk);
                COPY_EI(basic_ki_attack);
                COPY_EI(str);
                COPY_EI(bla);
                COPY_EI(atk_damage);
                COPY_EI(ki_damage);
                COPY_EI(str_damage);
                COPY_EI(bla_damage);
                memcpy(effect.unk_AC, file_effect->unk_AC, sizeof(effect.unk_AC));
            }
        }
    }

    return true;
}

uint8_t *IdbFile::Save(size_t *psize)
{
    size_t size;

    if (version == 122)
        size = sizeof(IDBHeader) + entries.size() * sizeof(IDBEntry122);
    else if (version == 118)
        size = sizeof(IDBHeader) + entries.size() * sizeof(IDBEntry118);
    else
        size = sizeof(IDBHeader) + entries.size() * sizeof(IDBEntry);

    uint8_t *buf = new uint8_t[size];
    memset(buf, 0, size);

    IDBHeader *hdr = (IDBHeader *)buf;
    hdr->signature = IDB_SIGNATURE;
    hdr->endianess_check = 0xFFFE;
    hdr->unk_06 = 7;
    hdr->num_entries = (uint32_t)entries.size();
    hdr->data_start = sizeof(IDBHeader);

    IDBEntry *file_entries = (IDBEntry *)(hdr+1);
    IDBEntry118 *file_entries_118 = (IDBEntry118 *)(hdr + 1);
    IDBEntry122 *file_entries_122 = (IDBEntry122 *)(hdr + 1);

    for (size_t i = 0; i < entries.size(); i++)
    {
        const IdbEntry &entry = entries[i];

        if (version == 122)
        {
            file_entries_122[i].id = entry.id;
            file_entries_122[i].stars = entry.stars;
            file_entries_122[i].name_id = entry.name_id;
            file_entries_122[i].desc_id = entry.desc_id;
            file_entries_122[i].type = entry.type;
            file_entries_122[i].unk_0A = entry.dlc_flag;
            file_entries_122[i].old_unk_0C = entry.availability;
            file_entries_122[i].old_unk_0E = entry.unk_0E;
            file_entries_122[i].buy = entry.buy;
            file_entries_122[i].sell = entry.sell;
            file_entries_122[i].racelock = entry.racelock;
            file_entries_122[i].tp = entry.tp;
            file_entries_122[i].model = entry.model;
            memcpy(file_entries_122[i].old_unk_24, entry.unk_24, sizeof(entry.unk_24));
            file_entries_122[i].new_unk_0C = entry.new_unk_0C;
            // New
            file_entries_122[i].how_id = entry.how_id;
            file_entries_122[i].new_unk_0A = entry.new_unk_0A;
            file_entries_122[i].stp = entry.stp;
            file_entries_122[i].new_unk_2C = entry.new_unk_2C;
        }
        else if (version == 118)
        {
            file_entries_118[i].id = entry.id;
            file_entries_118[i].stars = entry.stars;
            file_entries_118[i].name_id = entry.name_id;
            file_entries_118[i].desc_id = entry.desc_id;
            file_entries_118[i].type = entry.type;
            file_entries_118[i].unk_0A = entry.dlc_flag;
            file_entries_118[i].old_unk_0C = entry.availability;
            file_entries_118[i].old_unk_0E = entry.unk_0E;
            file_entries_118[i].buy = entry.buy;
            file_entries_118[i].sell = entry.sell;
            file_entries_118[i].racelock = entry.racelock;
            file_entries_118[i].tp = entry.tp;
            file_entries_118[i].model = entry.model;
            memcpy(file_entries_118[i].old_unk_24, entry.unk_24, sizeof(entry.unk_24));
            // New
            file_entries_118[i].new_unk_0C = entry.new_unk_0C;
        }
        else
        {
            file_entries[i].id = entry.id;
            file_entries[i].stars = entry.stars;
            file_entries[i].name_id = entry.name_id;
            file_entries[i].desc_id = entry.desc_id;
            file_entries[i].type = entry.type;
            file_entries[i].unk_0A = entry.dlc_flag;
            file_entries[i].unk_0C = entry.availability;
            file_entries[i].unk_0E = entry.unk_0E;
            file_entries[i].buy = entry.buy;
            file_entries[i].sell = entry.sell;
            file_entries[i].racelock = entry.racelock;
            file_entries[i].tp = entry.tp;
            file_entries[i].model = entry.model;
            memcpy(file_entries[i].unk_24, entry.unk_24, sizeof(entry.unk_24));
        }

        for (int j = 0; j < 3; j++)
        {
            const IdbEffect &effect = entry.effects[j];

            if (version == 122)
            {
                IDBEffect122 *file_effect_new = &file_entries_122[i].effects[j];

                COPY_EON(type);
                COPY_EON(activation_type);
                COPY_EON(num_act_times);
                COPY_EON(timer);
                memcpy(file_effect_new->ability_values, effect.ability_values, sizeof(effect.ability_values));
                COPY_EON(unk_28);
                COPY_EON(activation_chance);
                memcpy(file_effect_new->multipliers, effect.multipliers, sizeof(effect.multipliers));
                memcpy(file_effect_new->old_unk_48, effect.unk_48, sizeof(effect.unk_48));
                COPY_EON(hea);
                COPY_EON(ki);
                COPY_EON(ki_recovery);
                COPY_EON(stm);
                COPY_EON(stamina_recovery);
                COPY_EON(enemy_stamina_eraser);
                file_effect_new->old_unk_78 = effect.unk_78;
                COPY_EON(ground_speed);
                COPY_EON(air_speed);
                COPY_EON(boosting_speed);
                COPY_EON(dash_speed);
                COPY_EON(atk);
                COPY_EON(basic_ki_attack);
                COPY_EON(str);
                COPY_EON(bla);
                COPY_EON(atk_damage);
                COPY_EON(ki_damage);
                COPY_EON(str_damage);
                COPY_EON(bla_damage);
                memcpy(file_effect_new->old_unk_AC, effect.unk_AC, sizeof(effect.unk_AC));
                memcpy(file_effect_new->new_unk_48, effect.new_unk_48, sizeof(effect.new_unk_48));
                // New
                COPY_EON(new_unk_0C);
            }
            else if (version == 118)
            {
                IDBEffect118 *file_effect_new = &file_entries_118[i].effects[j];

                COPY_EON(type);
                COPY_EON(activation_type);
                COPY_EON(num_act_times);
                COPY_EON(timer);
                memcpy(file_effect_new->ability_values, effect.ability_values, sizeof(effect.ability_values));
                COPY_EON(unk_28);
                COPY_EON(activation_chance);
                memcpy(file_effect_new->multipliers, effect.multipliers, sizeof(effect.multipliers));
                memcpy(file_effect_new->old_unk_48, effect.unk_48, sizeof(effect.unk_48));
                COPY_EON(hea);
                COPY_EON(ki);
                COPY_EON(ki_recovery);
                COPY_EON(stm);
                COPY_EON(stamina_recovery);
                COPY_EON(enemy_stamina_eraser);
                file_effect_new->old_unk_78 = effect.unk_78;
                COPY_EON(ground_speed);
                COPY_EON(air_speed);
                COPY_EON(boosting_speed);
                COPY_EON(dash_speed);
                COPY_EON(atk);
                COPY_EON(basic_ki_attack);
                COPY_EON(str);
                COPY_EON(bla);
                COPY_EON(atk_damage);
                COPY_EON(ki_damage);
                COPY_EON(str_damage);
                COPY_EON(bla_damage);
                memcpy(file_effect_new->old_unk_AC, effect.unk_AC, sizeof(effect.unk_AC));
                // New
                memcpy(file_effect_new->new_unk_48, effect.new_unk_48, sizeof(effect.new_unk_48));
            }
            else
            {
                IDBEffect *file_effect = &file_entries[i].effects[j];

                COPY_EO(type);
                COPY_EO(activation_type);
                COPY_EO(num_act_times);
                COPY_EO(timer);
                memcpy(file_effect->ability_values, effect.ability_values, sizeof(effect.ability_values));
                COPY_EO(unk_28);
                COPY_EO(activation_chance);
                memcpy(file_effect->multipliers, effect.multipliers, sizeof(effect.multipliers));
                memcpy(file_effect->unk_48, effect.unk_48, sizeof(effect.unk_48));
                COPY_EO(hea);
                COPY_EO(ki);
                COPY_EO(ki_recovery);
                COPY_EO(stm);
                COPY_EO(stamina_recovery);
                COPY_EO(enemy_stamina_eraser);
                COPY_EO(unk_78);
                COPY_EO(ground_speed);
                COPY_EO(air_speed);
                COPY_EO(boosting_speed);
                COPY_EO(dash_speed);
                COPY_EO(atk);
                COPY_EO(basic_ki_attack);
                COPY_EO(str);
                COPY_EO(bla);
                COPY_EO(atk_damage);
                COPY_EO(ki_damage);
                COPY_EO(str_damage);
                COPY_EO(bla_damage);
                memcpy(file_effect->unk_AC, effect.unk_AC, sizeof(effect.unk_AC));
            }
        }
    }

    *psize = size;
    return buf;
}

TiXmlDocument *IdbFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("IDB");

    for (const IdbEntry &entry : entries)
    {
        entry.Decompile(root, comm_type);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool IdbFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "IDB");

    if (!root)
    {
        DPRINTF("Cannot find\"IDB\" in xml.\n");
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "IdbEntry")
        {
            IdbEntry entry;

            if (!entry.Compile(elem, &version))
                return false;

            entries.push_back(entry);
        }
    }

    return true;
}

IdbEntry *IdbFile::FindEntryByID(uint32_t id)
{
    for (IdbEntry &entry : entries)
    {
        if (entry.id == id)
            return &entry;
    }

    return nullptr;
}

IdbEntry *IdbFile::FindEntryByIdAndType(uint16_t id, uint16_t type)
{
    for (IdbEntry &entry : entries)
    {
        if (entry.id == id && entry.type == type)
            return &entry;
    }

    return nullptr;
}

bool IdbFile::AddEntry(const IdbEntry &entry, bool overwrite_existing)
{
    IdbEntry *existing = FindEntryByIdAndType(entry.id, entry.type);

    if (existing)
    {
        if (!overwrite_existing)
            return false;

        *existing = entry;
        return true;
    }

    entries.push_back(entry);
    return true;
}

bool IdbFile::AddEntryAuto(IdbEntry &entry, uint16_t id_start)
{
    for (entry.id = id_start; entry.id != 0x8000; entry.id++)
    {
        if (!FindEntryByID(entry.id))
        {
            entries.push_back(entry);
            return true;
        }
    }

    return false;
}

size_t IdbFile::RemoveEntry(uint16_t id, uint16_t type)
{
    size_t count = 0;

    for (size_t i = 0; i < entries.size(); i++)
    {
        if (entries[i].id == id && entries[i].type == type)
        {
            entries.erase(entries.begin()+i);
            i--;
            count++;
        }
    }

    return count;
}

size_t IdbFile::RemoveEntry(uint16_t id)
{
    size_t count = 0;

    for (size_t i = 0; i < entries.size(); i++)
    {
        if (entries[i].id == id)
        {
            entries.erase(entries.begin()+i);
            i--;
            count++;
        }
    }

    return count;
}




