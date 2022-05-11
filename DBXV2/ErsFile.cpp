#include "ErsFile.h"
#include "debug.h"

ErsFile::ErsFile()
{
    this->big_endian = false;
}

ErsFile::~ErsFile()
{

}

void ErsFile::Reset()
{
    entries.clear();
}

bool ErsFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(ERSHeader))
        return false;

    const ERSHeader *hdr = (const ERSHeader *)buf;
    entries.resize(hdr->num_entries);

    const uint32_t *entries_table = (const uint32_t *)(buf + hdr->table_start);

    for (size_t i = 0; i < entries.size(); i++)
    {
        ErsEntry &entry = entries[i];

        if (entries_table[i] == 0)
            continue;

        const ERSEntry *file_entry = (const ERSEntry *)(buf + entries_table[i]);
        entry.effects.resize(file_entry->num_effects);

        const uint32_t *effects_table = (const uint32_t *)GetOffsetPtr(file_entry, file_entry->effect_table_offset);

        for (size_t j = 0; j < entry.effects.size(); j++)
        {
            ErsEffect &effect = entry.effects[j];

            if (effects_table[j] == 0)
                continue;

            const ERSEffect *file_effect = (const ERSEffect *)GetOffsetPtr(file_entry, effects_table[j]);

            effect.name = file_effect->name;
            effect.eepk_path = (const char *)GetOffsetPtr(file_effect, file_effect->eepk_path_offset);
        }
    }

    return true;
}

size_t ErsFile::GetNumNonEmptyEntries() const
{
    size_t count = 0;

    for (const ErsEntry &entry : entries)
        if (!entry.IsEmpty())
            count++;

    return count;
}

size_t ErsFile::CalculateFileSize(size_t *effect_start, size_t *strings_size) const
{
    size_t size = sizeof(ERSHeader) + entries.size()*sizeof(uint32_t);
    size_t non_empty = GetNumNonEmptyEntries();

    size += non_empty*sizeof(ERSEntry);
    *effect_start = size;
    *strings_size = 0;

    for (const ErsEntry &entry : entries)
    {
        if (entry.IsEmpty())
            continue;

        size += entry.effects.size()*sizeof(uint32_t);
        *effect_start += entry.effects.size()*sizeof(uint32_t);

        for (const ErsEffect &effect : entry.effects)
        {
            if (effect.IsEmpty())
                continue;

            size += sizeof(ERSEffect);
            size += effect.eepk_path.length() + 1;
            *strings_size += effect.eepk_path.length() + 1;
        }
    }

    return size;
}

uint8_t *ErsFile::Save(size_t *psize)
{
    size_t effect_start, strings_size;

    *psize = CalculateFileSize(&effect_start, &strings_size);
    uint8_t *buf = new uint8_t[*psize];
    memset(buf, 0, *psize);

    ERSHeader *hdr = (ERSHeader *)buf;
    hdr->signature = ERS_SIGNATURE;
    hdr->endianess_check = 0xFFFE;
    hdr->header_size = sizeof(ERSHeader);
    hdr->num_entries = (uint32_t)entries.size();
    hdr->table_start = sizeof(ERSHeader);

    uint32_t *entries_table = (uint32_t *)(buf + hdr->table_start);

    std::vector<size_t> non_empty_entries;
    size_t half = entries.size() / 2;

    for (size_t i = 0; i < half; i++)
    {
        if (!entries[i].IsEmpty())
            non_empty_entries.push_back(i);
    }

    for (size_t i = entries.size()-1; i >= half; i--)
    {
        if (!entries[i].IsEmpty())
            non_empty_entries.push_back(i);
    }

    ERSEntry *file_entry = (ERSEntry *)(entries_table + entries.size());
    uint32_t *effects_table = (uint32_t *)GetOffsetPtr(file_entry, (uint32_t)non_empty_entries.size()*sizeof(ERSEntry));
    ERSEffect *file_effect = (ERSEffect *)(buf + effect_start);
    char *file_string = (char *)(buf + *psize - strings_size);

    for (size_t i = 0; i < non_empty_entries.size(); i++)
    {
        size_t idx = non_empty_entries[i];
        const ErsEntry &entry = entries[idx];

        entries_table[idx] = Utils::DifPointer(file_entry, buf);
        file_entry->type = (uint32_t)idx;
        file_entry->num_effects = (uint32_t)entry.effects.size();
        file_entry->effect_table_offset = Utils::DifPointer(effects_table, file_entry);

        for (size_t j = 0; j < entry.effects.size(); j++)
        {
            const ErsEffect &effect = entry.effects[j];

            if (effect.IsEmpty())
                continue;

            effects_table[j] = Utils::DifPointer(file_effect, file_entry);
            file_effect->index = (uint32_t)j;
            strcpy(file_effect->name, effect.name.substr(0, 7).c_str());
            file_effect->eepk_path_offset = Utils::DifPointer(file_string, file_effect);

            strcpy(file_string, effect.eepk_path.c_str());
            file_string += effect.eepk_path.length()+1;

            file_effect++;
        }

        effects_table += entry.effects.size();
        file_entry++;
    }

    return buf;
}

std::string ErsFile::GetStageBgEepk(size_t stage_id) const
{
    if (entries.size() < ERS_EFFECT_STAGE_BG)
        return std::string();

    const ErsEntry &stage_bg_entry = entries[ERS_EFFECT_STAGE_BG];
    if (stage_id >= stage_bg_entry.effects.size())
        return std::string();

    return stage_bg_entry.effects[stage_id].eepk_path;
}

std::string ErsFile::GetStageEepk(size_t stage_id) const
{
    if (entries.size() < ERS_EFFECT_STAGE)
        return std::string();

    const ErsEntry &stage_entry = entries[ERS_EFFECT_STAGE];
    if (stage_id >= stage_entry.effects.size())
        return std::string();

    return stage_entry.effects[stage_id].eepk_path;
}

std::string ErsFile::GetCharEepk(size_t cms_id) const
{
    if (entries.size() < ERS_EFFECT_CHAR)
        return std::string();

    const ErsEntry &char_entry = entries[ERS_EFFECT_CHAR];
    if (cms_id >= char_entry.effects.size())
        return std::string();

    return char_entry.effects[cms_id].eepk_path;
}

bool ErsFile::SetStageBgEepk(const std::string &stage, size_t stage_id, const std::string &eepk_path)
{
    if (entries.size() < ERS_EFFECT_STAGE_BG)
        return false;

    ErsEntry &stage_bg_entry = entries[ERS_EFFECT_STAGE_BG];
    if (stage_id >= stage_bg_entry.effects.size())
        stage_bg_entry.effects.resize(stage_id+1);

    ErsEffect &effect = stage_bg_entry.effects[stage_id];
    effect.name = stage;
    effect.eepk_path = eepk_path;

    return true;
}

bool ErsFile::SetStageEepk(const std::string &stage, size_t stage_id, const std::string &eepk_path)
{
    if (entries.size() < ERS_EFFECT_STAGE)
        return false;

    ErsEntry &stage_entry = entries[ERS_EFFECT_STAGE];
    if (stage_id >= stage_entry.effects.size())
        stage_entry.effects.resize(stage_id+1);

    ErsEffect &effect = stage_entry.effects[stage_id];
    effect.name = stage;
    effect.eepk_path = eepk_path;

    return true;
}

bool ErsFile::SetCharEepk(size_t cms_id, const std::string &eepk_path)
{
    if (entries.size() < ERS_EFFECT_CHAR)
        return false;

    ErsEntry &char_entry = entries[ERS_EFFECT_CHAR];
    if (cms_id >= char_entry.effects.size())
        char_entry.effects.resize(cms_id+1);

    ErsEffect &effect = char_entry.effects[cms_id];
    effect.name.clear();
    effect.eepk_path = eepk_path;

    return true;
}

void ErsFile::RemoveStageBgEepk(size_t stage_id)
{
    if (entries.size() < ERS_EFFECT_STAGE_BG)
        return;

    ErsEntry &stage_bg_entry = entries[ERS_EFFECT_STAGE_BG];
    if (stage_id >= stage_bg_entry.effects.size())
        return;

    if (stage_id == (stage_bg_entry.effects.size()-1))
    {
        stage_bg_entry.effects.pop_back();
    }
    else
    {
        stage_bg_entry.effects[stage_id] = ErsEffect();
    }
}

void ErsFile::RemoveStageEepk(size_t stage_id)
{
    if (entries.size() < ERS_EFFECT_STAGE)
        return;

    ErsEntry &stage_entry = entries[ERS_EFFECT_STAGE];
    if (stage_id >= stage_entry.effects.size())
        return;

    if (stage_id == (stage_entry.effects.size()-1))
    {
        stage_entry.effects.pop_back();
    }
    else
    {
        stage_entry.effects[stage_id] = ErsEffect();
    }
}

void ErsFile::RemoveCharEepk(size_t cms_id)
{
    if (entries.size() < ERS_EFFECT_CHAR)
        return;

    ErsEntry &char_entry = entries[ERS_EFFECT_CHAR];
    if (cms_id >= char_entry.effects.size())
        return;

    if (cms_id == (char_entry.effects.size()-1))
    {
        char_entry.effects.pop_back();
    }
    else
    {
        char_entry.effects[cms_id] = ErsEffect();
    }
}
