#include "QxdFile.h"
#include "debug.h"

#define COPY_VAL(a, b, f) a.f = b->f
#define COPY_VAL2(a, b, f) a->f = b.f

#define COPY_I_Q(f) COPY_VAL(quest, file_quest, f)
#define COPY_O_Q(f) COPY_VAL2(file_quest, quest, f)

QxdFile::QxdFile()
{
   this->big_endian = false;
   Reset();
}

QxdFile::~QxdFile()
{

}

void QxdFile::Reset()
{
    quests.clear();
    chars.clear();
    special_chars.clear();
    collections.clear();

    unk_28 = 0;
    memset(unk_floats, 0, sizeof(unk_floats));
}

void QxdFile::ReadFixedStringArray(const char *start, std::vector<std::string> &array, uint32_t entry_size)
{
    const char *ptr = start;

    for (std::string &str : array)
    {
        str = ptr;
        ptr += entry_size;
    }
}

bool QxdFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(QXDHeader))
        return false;

    const QXDHeader *hdr = (const QXDHeader *)buf;
    if (hdr->signature != QXD_SIGNATURE)
        return false;

    const QXDQuest *file_quests = (const QXDQuest *)GetOffsetPtr(hdr, hdr->quests_start);
    quests.resize(hdr->num_quests);

    for (size_t i = 0; i < quests.size(); i++)
    {
        QxdQuest &quest = quests[i];
        const QXDQuest *file_quest = &file_quests[i];

        COPY_I_Q(name);
        COPY_I_Q(id);
        COPY_I_Q(episode);
        COPY_I_Q(sub_type);
        COPY_I_Q(num_players);
        COPY_I_Q(unk_28);
        COPY_I_Q(parent_quest);
        COPY_I_Q(unk_2C);
        COPY_I_Q(unk_2E);
        COPY_I_Q(unlock_requirement);
        COPY_I_Q(time_limit);
        COPY_I_Q(difficulty);
        COPY_I_Q(start_stage);
        COPY_I_Q(start_demo);
        COPY_I_Q(xp_reward);
        COPY_I_Q(ult_xp_reward);
        COPY_I_Q(fail_xp_reward);
        COPY_I_Q(zeni_reward);
        COPY_I_Q(ult_zeni_reward);
        COPY_I_Q(fail_zeni_reward);
        COPY_I_Q(tp_medals_once);
        COPY_I_Q(tp_medals);
        COPY_I_Q(tp_medals_special);
        COPY_I_Q(resistance_points);
        COPY_I_Q(unk_C0);
        COPY_I_Q(flags);
        COPY_I_Q(update_requirement);
        COPY_I_Q(dlc_requirement);
        COPY_I_Q(unk_108);
        COPY_I_Q(no_enemy_music);
        COPY_I_Q(enemy_near_music);
        COPY_I_Q(battle_music);
        COPY_I_Q(ult_finish_music);
        COPY_I_Q(unk_114);
        COPY_I_Q(unk_118);

        quest.msg_entries.resize(file_quest->num_msg_entries);
        const char *file_msg_entries = (const char *)(buf + file_quest->msg_entries_offset);
        ReadFixedStringArray(file_msg_entries, quest.msg_entries);

        memcpy(quest.unk_30, file_quest->unk_30, sizeof(quest.unk_30));
        memcpy(quest.unk_44, file_quest->unk_44, sizeof(quest.unk_44));

        quest.unk1s.resize(file_quest->num_unk1);
        const QXDUnk *file_unk1s = (const QXDUnk *)(buf + file_quest->unk1_offset);
        if (file_quest->num_unk1 > 0)
            memcpy(quest.unk1s.data(), file_unk1s, file_quest->num_unk1*sizeof(QXDUnk));

        quest.unk2s.resize(file_quest->num_unk2);
        const QXDUnk *file_unk2s = (const QXDUnk *)(buf + file_quest->unk2_offset);
        if (file_quest->num_unk2 > 0)
            memcpy(quest.unk2s.data(), file_unk2s, file_quest->num_unk2*sizeof(QXDUnk));

        quest.scripts.resize(file_quest->num_scripts);
        const char *file_scripts = (const char *)(buf + file_quest->scripts_offset);
        ReadFixedStringArray(file_scripts, quest.scripts);

        quest.item_rewards.resize(file_quest->num_item_rewards);
        const QXDItemReward *file_er = (const QXDItemReward *)(buf + file_quest->item_rewards_offset);
        if (file_quest->num_item_rewards > 0)
            memcpy(quest.item_rewards.data(), file_er, file_quest->num_item_rewards*sizeof(QXDItemReward));

        quest.skill_rewards.resize(file_quest->num_skill_rewards);
        const QXDSkillReward *file_sr = (const QXDSkillReward *)(buf + file_quest->skill_rewards_offset);
        if (file_quest->num_skill_rewards > 0)
            memcpy(quest.skill_rewards.data(), file_sr, file_quest->num_skill_rewards*sizeof(QXDSkillReward));

        quest.char_rewards.resize(file_quest->num_chars_rewards);
        const QXDCharReward *file_cr = (const QXDCharReward *)(buf + file_quest->chars_rewards_offset);
        if (file_quest->num_chars_rewards > 0)
            memcpy(quest.char_rewards.data(), file_cr, file_quest->num_chars_rewards*sizeof(QXDCharReward));

        if (file_quest->num_stages == 1)
        {
            memcpy(quest.stages, buf + file_quest->stages_offset, sizeof(quest.stages));
        }
        else
        {
            DPRINTF("%s: wasn't prepared for stage portrait num different than 1: %d (quest %s)\n", FUNCNAME, file_quest->num_stages, file_quest->name);
            return false;
        }

        memcpy(quest.enemy_portraits, file_quest->enemy_portraits, sizeof(quest.enemy_portraits));
        memcpy(quest.unk_E8, file_quest->unk_E8, sizeof(quest.unk_E8));

        /*static std::unordered_set<uint32_t> knowns;
        for (const QxdItemReward &reward : quest.item_rewards)
        {
            //if(knowns.find(reward.type) == knowns.end() && !Utils::BeginsWith(quest.name, "TNB_"))
            if (reward.type >= 100)
            {
                knowns.insert(reward.type);
                DPRINTF("%s: %d\n", quest.name.c_str(), reward.type);
            }
        }*/
        /*for (const QxdCharReward &reward: quest.char_rewards)
        {
            if (reward.unk_06 != 0)
            {
                DPRINTF("Reward: %s %d\n", reward.cms_name, reward.unk_06);
            }
        }*/       
        /*if (quest.unlock_requirement != (quest.unk_30[4]&0xFFFF))
        {
            DPRINTF("%s %d %d\n", quest.name.c_str(), quest.unlock_requirement, quest.unk_30[4]);
            Sleep(1000);
        }*/
    }

    const QXDCharacter *file_chars = (const QXDCharacter *)GetOffsetPtr(hdr, hdr->chars_start);
    chars.resize(hdr->num_chars);

    for (size_t i = 0; i < chars.size(); i++)
    {
        chars[i] = file_chars[i];
    }

    const QXDCharacter *file_schars = (const QXDCharacter *)GetOffsetPtr(hdr, hdr->special_chars_start);
    special_chars.resize(hdr->num_special_chars);

    for (size_t i = 0; i < special_chars.size(); i++)
    {
        special_chars[i] = file_schars[i];
    }

    const QXDCollectionEntry *file_collections = (const QXDCollectionEntry *)GetOffsetPtr(hdr, hdr->collections_entries_start);

    for (uint32_t i = 0; i < hdr->num_collections_entries; i++)
    {
       const QXDCollectionEntry &file_entry = file_collections[i];
       QxdCollectionEntry entry;

       entry.item_id = file_entry.item_id;
       entry.item_type = file_entry.item_type;
	   entry.unk_06 = file_entry.unk_06;
       entry.unk_08 = file_entry.unk_08;
       entry.unk_0A = file_entry.unk_0A;

       if (i == 0 || file_entry.id != file_collections[i-1].id)
       {
           QxdCollection new_collection;

           new_collection.id = file_entry.id;
           new_collection.entries.push_back(entry);
           collections.push_back(new_collection);
       }
       else
       {
           collections.back().entries.push_back(entry);
       }
    }

    unk_28 = hdr->unk_28;
    memcpy(unk_floats, buf + hdr->floats_offset, sizeof(unk_floats));

    return true;
}

size_t QxdFile::CalculateQuestsSize(size_t *pmsg_size, size_t *punk1_size, size_t *punk2_size, size_t *pscripts_size, size_t *pequip_rewards_size, size_t *pskill_rewards_size, size_t *pchar_rewards_size) const
{
    size_t size = quests.size() * sizeof(QXDQuest);

    size_t msg_size = 0;
    size_t unk1_size = 0;
    size_t unk2_size = 0;
    size_t scripts_size = 0;
    size_t equip_rewards_size = 0;
    size_t skill_rewards_size = 0;
    size_t char_rewards_size = 0;

    for (const QxdQuest &quest : quests)
    {
        msg_size += quest.msg_entries.size()*32;
        unk1_size += quest.unk1s.size()*sizeof(QXDUnk);
        unk2_size += quest.unk2s.size()*sizeof(QXDUnk);
        scripts_size += quest.scripts.size()*32;
        equip_rewards_size += quest.item_rewards.size() * sizeof(QXDItemReward);
        skill_rewards_size += quest.skill_rewards.size() * sizeof(QXDSkillReward);
        char_rewards_size += quest.char_rewards.size() * sizeof(QXDCharReward);

        size += sizeof(quest.stages);
    }

    if (pmsg_size)
        *pmsg_size = msg_size;

    if (punk1_size)
        *punk1_size = unk1_size;

    if (punk2_size)
        *punk2_size = unk2_size;

    if (pscripts_size)
        *pscripts_size = scripts_size;

    if (pequip_rewards_size)
        *pequip_rewards_size = equip_rewards_size;

    if (pskill_rewards_size)
        *pskill_rewards_size = skill_rewards_size;

    if (pchar_rewards_size)
        *pchar_rewards_size = char_rewards_size;

    size += msg_size + unk1_size + unk2_size + scripts_size + equip_rewards_size + skill_rewards_size + char_rewards_size;
    return size;
}

uint32_t QxdFile::GetHighestQuestID() const
{
    uint32_t max_id = 0;

    for (const QxdQuest &quest : quests)
    {
        if (quest.id > max_id)
            max_id = quest.id;
    }

    return max_id;
}

size_t QxdFile::GetNumCollectionEntries() const
{
    size_t num = 0;

    for (const QxdCollection &col : collections)
        num += col.entries.size();

    return num;
}

uint32_t QxdFile::GetHighestCharId() const
{
    uint32_t highest = 0;

    for (const QxdCharacter &ch : chars)
        if (ch.id > highest)
            highest = ch.id;

    for (const QxdCharacter &ch : special_chars)
        if (ch.id > highest)
            highest = ch.id;

    return highest;
}

uint8_t *QxdFile::Save(size_t *psize)
{
    size_t msg_size, unk1_size, unk2_size, scripts_size, equip_rewards_size, skill_rewards_size, char_rewards_size;
    size_t quests_size = CalculateQuestsSize(&msg_size, &unk1_size, &unk2_size, &scripts_size, &equip_rewards_size, &skill_rewards_size, &char_rewards_size);
    size_t chars_size = chars.size()*sizeof(QXDCharacter);
    size_t special_chars_size = special_chars.size()*sizeof(QXDCharacter);
    size_t collections_size = GetNumCollectionEntries()*sizeof(QXDCollectionEntry);
    size_t floats_size = sizeof(unk_floats);

    *psize = sizeof(QXDHeader) + quests_size + chars_size + special_chars_size + collections_size + floats_size;

    uint8_t *buf = new uint8_t[*psize];
    memset(buf, 0, *psize);

    //DPRINTF("Quest_size = %Ix, chars_size = %Ix")

    QXDHeader *hdr = (QXDHeader *)buf;
    hdr->signature = QXD_SIGNATURE;
    hdr->endianess_check = 0xFFFE;
    hdr->header_size = sizeof(QXDHeader);
    hdr->num_quests = (uint32_t)quests.size();
    hdr->quests_start = sizeof(QXDHeader);
    hdr->num_chars = (uint32_t)chars.size();
    hdr->chars_start = hdr->quests_start + (uint32_t)quests_size;
    hdr->num_special_chars = (uint32_t)special_chars.size();
    hdr->special_chars_start = hdr->chars_start + (uint32_t)chars_size;
    hdr->num_collections_entries = (uint32_t)collections_size / sizeof(QXDCollectionEntry);
    hdr->collections_entries_start = hdr->special_chars_start + (uint32_t)special_chars_size;
    hdr->unk_28 = unk_28;
    hdr->floats_offset = hdr->collections_entries_start + (uint32_t)collections_size;

    QXDQuest *file_quest = (QXDQuest *)(hdr + 1);
    char *file_msg = (char *)(file_quest + hdr->num_quests);
    QXDUnk *file_unk1 = (QXDUnk *)GetOffsetPtr(file_msg, (uint32_t)msg_size);
    QXDUnk *file_unk2 = (QXDUnk *)GetOffsetPtr(file_unk1, (uint32_t)unk1_size);
    char *file_scripts = (char *)GetOffsetPtr(file_unk2, (uint32_t)unk2_size);
    QXDItemReward *file_er = (QXDItemReward *)GetOffsetPtr(file_scripts, (uint32_t)scripts_size);
    QXDSkillReward *file_sr = (QXDSkillReward *)GetOffsetPtr(file_er, (uint32_t)equip_rewards_size);
    QXDCharReward *file_cr = (QXDCharReward *)GetOffsetPtr(file_sr, (uint32_t)skill_rewards_size);
    uint16_t *file_stage_portraits = (uint16_t *)GetOffsetPtr(file_cr, (uint32_t)char_rewards_size);

    for (const QxdQuest &quest : quests)
    {
        COPY_O_Q(id);
        COPY_O_Q(episode);
        COPY_O_Q(sub_type);
        COPY_O_Q(num_players);
        COPY_O_Q(unk_28);
        COPY_O_Q(parent_quest);
        COPY_O_Q(unk_2C);
        COPY_O_Q(unk_2E);
        COPY_O_Q(unlock_requirement);
        COPY_O_Q(time_limit);
        COPY_O_Q(difficulty);
        COPY_O_Q(start_stage);
        COPY_O_Q(start_demo);
        COPY_O_Q(xp_reward);
        COPY_O_Q(ult_xp_reward);
        COPY_O_Q(fail_xp_reward);
        COPY_O_Q(zeni_reward);
        COPY_O_Q(ult_zeni_reward);
        COPY_O_Q(fail_zeni_reward);
        COPY_O_Q(tp_medals_once);
        COPY_O_Q(tp_medals);
        COPY_O_Q(tp_medals_special);
        COPY_O_Q(resistance_points);
        COPY_O_Q(unk_C0);
        COPY_O_Q(flags);
        COPY_O_Q(update_requirement);
        COPY_O_Q(dlc_requirement);
        COPY_O_Q(unk_108);
        COPY_O_Q(no_enemy_music);
        COPY_O_Q(enemy_near_music);
        COPY_O_Q(battle_music);
        COPY_O_Q(ult_finish_music);
        COPY_O_Q(unk_114);
        COPY_O_Q(unk_118);

        strncpy(file_quest->name, quest.name.c_str(), 16);
        file_quest->name[15] = 0;

        if (quest.msg_entries.size() > 0)
        {
            file_quest->num_msg_entries = (uint32_t) quest.msg_entries.size();
            file_quest->msg_entries_offset = Utils::DifPointer(file_msg, buf);
            for (const std::string &str : quest.msg_entries)
            {
                strncpy(file_msg, str.c_str(), 32);
                file_msg[31] = 0;
                file_msg += 32;
            }
        }

        memcpy(file_quest->unk_30, quest.unk_30, sizeof(quest.unk_30));
        memcpy(file_quest->unk_44, quest.unk_44, sizeof(quest.unk_44));

        if (quest.unk1s.size() > 0)
        {
            file_quest->num_unk1 = (uint32_t) quest.unk1s.size();
            file_quest->unk1_offset = Utils::DifPointer(file_unk1, buf);
            for (const QXDUnk &unk : quest.unk1s)
            {
                *file_unk1 = unk;
                file_unk1++;
            }
        }

        if (quest.unk2s.size() > 0)
        {
            file_quest->num_unk2 = (uint32_t) quest.unk2s.size();
            file_quest->unk2_offset = Utils::DifPointer(file_unk2, buf);
            for (const QXDUnk &unk : quest.unk2s)
            {
                *file_unk2 = unk;
                file_unk2++;
            }
        }

        if (quest.scripts.size() > 0)
        {
            file_quest->num_scripts = (uint32_t) quest.scripts.size();
            file_quest->scripts_offset = Utils::DifPointer(file_scripts, buf);
            for (const std::string &str : quest.scripts)
            {
                strncpy(file_scripts, str.c_str(), 32);
                file_scripts[31] = 0;
                file_scripts += 32;
            }
        }

        if (quest.item_rewards.size() > 0)
        {
            file_quest->num_item_rewards = (uint32_t) quest.item_rewards.size();
            file_quest->item_rewards_offset = Utils::DifPointer(file_er, buf);
            for (const QXDItemReward &reward : quest.item_rewards)
            {
                *file_er = reward;
                file_er++;
            }
        }

        if (quest.skill_rewards.size() > 0)
        {
            file_quest->num_skill_rewards = (uint32_t) quest.skill_rewards.size();
            file_quest->skill_rewards_offset = Utils::DifPointer(file_sr, buf);
            for (const QXDSkillReward &reward : quest.skill_rewards)
            {
                *file_sr = reward;
                file_sr++;
            }
        }

        if (quest.char_rewards.size() > 0)
        {
            file_quest->num_chars_rewards = (uint32_t) quest.char_rewards.size();
            file_quest->chars_rewards_offset = Utils::DifPointer(file_cr, buf);
            for (const QXDCharReward &reward : quest.char_rewards)
            {
                *file_cr = reward;
                file_cr++;
            }
        }

        file_quest->num_stages = 1;
        file_quest->stages_offset = Utils::DifPointer(file_stage_portraits, buf);
        memcpy(file_stage_portraits, quest.stages, sizeof(quest.stages));
        file_stage_portraits += 16;

        memcpy(file_quest->enemy_portraits, quest.enemy_portraits, sizeof(quest.enemy_portraits));
        memcpy(file_quest->unk_E8, quest.unk_E8, sizeof(quest.unk_E8));

        file_quest++;
    }

    QXDCharacter *file_char = (QXDCharacter *)GetOffsetPtr(buf, hdr->chars_start);
    for (const QxdCharacter &ch : chars)
    {
        *file_char = ch;
        file_char++;
    }

    QXDCharacter *file_schar = (QXDCharacter *)GetOffsetPtr(buf, hdr->special_chars_start);
    for (const QxdCharacter &ch : special_chars)
    {
        *file_schar = ch;
        file_schar++;
    }

    QXDCollectionEntry *file_collection = (QXDCollectionEntry *)GetOffsetPtr(buf, hdr->collections_entries_start);
    for (const QxdCollection &collection : collections)
    {
        for (const QxdCollectionEntry &entry : collection.entries)
        {
            file_collection->id = collection.id;
            file_collection->item_type = entry.item_type;
            file_collection->item_id = entry.item_id;
            file_collection->unk_06 = entry.unk_06;
            file_collection->unk_08 = entry.unk_08;
            file_collection->unk_0A = entry.unk_0A;

            file_collection++;
        }
    }

    memcpy(buf + hdr->floats_offset, unk_floats, sizeof(unk_floats));
    return buf;
}

QxdQuest *QxdFile::FindQuestByName(const std::string &name)
{
    std::string name_upper = Utils::ToUpperCase(name);

    for (QxdQuest &quest : quests)
    {
        if (Utils::ToUpperCase(quest.name) == name_upper)
            return &quest;
    }

    return nullptr;
}

QxdQuest *QxdFile::FindQuestById(uint32_t id)
{
    for (QxdQuest &quest : quests)
    {
        if (quest.id == id)
            return &quest;
    }

    return nullptr;
}

bool QxdFile::AddQuest(QxdQuest &quest, int new_id_search_start)
{
    QxdQuest *existing_quest = FindQuestByName(quest.name);

    if (existing_quest)
    {
        quest.id = existing_quest->id;
        *existing_quest = quest;
        return true;
    }

    if (new_id_search_start < 0)
    {
        quest.id = GetHighestQuestID()+1;
    }
    else
    {
        quest.id = 0x80000000;

        for (int i = new_id_search_start; i >= 0; i++)
        {
            QxdQuest *existing_quest = FindQuestById((uint32_t)i);

            if (!existing_quest)
            {
                quest.id = (uint32_t)i;
                break;
            }
            else
            {
                if (existing_quest->update_requirement == QXD_UPDATE_DEVELOPER && Utils::BeginsWith(existing_quest->name, "empty", false))
                {
                    quest.id = existing_quest->id;
                    *existing_quest = quest;
                    return true;
                }
            }
        }
    }

    if (quest.id >= 0x80000000)
        return false;

    quests.push_back(quest);
    return true;
}

void QxdFile::RemoveQuest(uint32_t id, bool only_erase)
{
    for (size_t i = 0; i < quests.size(); i++)
    {
        if (quests[i].id == id)
        {
            if (!only_erase)
            {
                quests.erase(quests.begin()+i);
                i--;
            }
            else
            {
                quests[i].name = "empty_Q" + Utils::ToString(id);
                quests[i].update_requirement = QXD_UPDATE_DEVELOPER;
            }
        }
    }
}

QxdCharacter *QxdFile::FindCharById(uint32_t id)
{
    for (QxdCharacter &ch : chars)
    {
        if (ch.id == id)
            return &ch;
    }

    return nullptr;
}

bool QxdFile::AddCharWithId(const QxdCharacter &ch, bool overwrite)
{
    QxdCharacter *existing_char = FindCharById(ch.id);

    if (existing_char)
    {
        if (!overwrite)
            return false;

        *existing_char = ch;
    }
    else
    {
        chars.push_back(ch);
    }

    return true;
}

void QxdFile::AddCharWithNewId(QxdCharacter &ch, int new_id_search_start)
{
    if (new_id_search_start < 0)
    {
        ch.id = GetHighestCharId()+1;
    }
    else
    {
        ch.id = 0x80000000;

        for (int i = new_id_search_start; i >= 0; i++)
        {
            if (!FindCharById(i) && !FindSpecialCharById(i))
            {
                ch.id = i;
                break;
            }
        }
    }


    chars.push_back(ch);
}

QxdCharacter *QxdFile::FindSimilarChar(const QxdCharacter &ch)
{
    QxdCharacter temp = ch;

    for (QxdCharacter &chara : chars)
    {
        temp.id = chara.id;
        if (temp == chara)
            return &chara;
    }

    return nullptr;
}

QxdCharacter *QxdFile::FindSpecialCharById(uint32_t id)
{
    for (QxdCharacter &ch : special_chars)
    {
        if (ch.id == id)
            return &ch;
    }

    return nullptr;
}

bool QxdFile::AddSpecialCharWithId(const QxdCharacter &ch, bool overwrite)
{
    QxdCharacter *existing_char = FindSpecialCharById(ch.id);

    if (existing_char)
    {
        if (!overwrite)
            return false;

        *existing_char = ch;
    }
    else
    {
        special_chars.push_back(ch);
    }

    return true;
}

void QxdFile::AddSpecialCharWithNewId(QxdCharacter &ch, int new_id_search_start)
{
    if (new_id_search_start < 0)
    {
        ch.id = GetHighestCharId()+1;
    }
    else
    {
        ch.id = 0x80000000;

        for (int i = new_id_search_start; i >= 0; i++)
        {
            if (!FindCharById(i) && !FindSpecialCharById(i))
            {
                ch.id = i;
                break;
            }
        }
    }


    special_chars.push_back(ch);
}

size_t QxdFile::RemoveCharById(uint32_t id)
{
    size_t count = 0;

    for (size_t i = 0; i < chars.size(); i++)
    {
        QxdCharacter &ch = chars[i];

        if (ch.id == id)
        {
            chars.erase(chars.begin()+i);
            i--;
            count++;
        }
    }

    for (size_t i = 0; i < special_chars.size(); i++)
    {
        QxdCharacter &ch = special_chars[i];

        if (ch.id == id)
        {
            special_chars.erase(special_chars.begin()+i);
            i--;
            count++;
        }
    }

    return count;
}

QxdCharacter *QxdFile::FindSimilarSpecialChar(const QxdCharacter &ch)
{
    QxdCharacter temp = ch;

    for (QxdCharacter &chara : special_chars)
    {
        temp.id = chara.id;
        if (temp == chara)
            return &chara;
    }

    return nullptr;
}

QxdCollection *QxdFile::FindCollection(uint32_t id)
{
    for (QxdCollection &col : collections)
        if (id == col.id)
            return &col;

    return nullptr;
}

size_t QxdFile::FindSimilarCollections(const QxdCollection &collection, std::vector<QxdCollection *> &result)
{
    result.clear();

    QxdCollection temp = collection; // copy

    for (QxdCollection &col: collections)
    {
        temp.id = col.id;

        if (col == temp)
            result.push_back(&col);
    }

    return result.size();
}

bool QxdFile::AddCollection(QxdCollection &collection, bool auto_id, int new_id_search_start)
{
    if (auto_id)
    {
        if (new_id_search_start < 0)
        {
            uint16_t highest = 0;

            for (const QxdCollection &col : collections)
            {
                if (col.id > highest)
                    highest = col.id;
            }

            if (highest == 0xFFFF)
            {
                DPRINTF("%s: Overflow!\n", FUNCNAME);
                return false;
            }

            collection.id = highest+1;
        }
        else
        {
            collection.id = 0x8000;

            for (int i = new_id_search_start; i >= 0; i++)
            {
                if (!FindCollection(i))
                {
                    collection.id = i;
                    break;
                }
            }

            if (collection.id >= 0x8000)
                return false;
        }
    }
    else
    {
        if (FindCollection(collection.id))
            return false;
    }

    collections.push_back(collection);
    return true;
}

size_t QxdFile::RemoveCollectionById(uint32_t id)
{
    size_t count = 0;

    for (size_t i = 0; i < collections.size(); i++)
    {
        if (collections[i].id == id)
        {
            collections.erase(collections.begin()+i);
            i--;
            count++;
        }
    }

    return count;
}

