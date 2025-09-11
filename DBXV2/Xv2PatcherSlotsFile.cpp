#include "Xv2PatcherSlotsFile.h"
#include "debug.h"

Xv2PatcherSlotsFile::Xv2PatcherSlotsFile()
{
    this->big_endian = false;
}

Xv2PatcherSlotsFile::Xv2PatcherSlotsFile(const CharaListFile &file)
{
    this->chara_slots = file.GetSlots();
}

Xv2PatcherSlotsFile::~Xv2PatcherSlotsFile()
{

}

static std::string DlcToString(uint64_t dlc)
{
    std::string ret;

    switch (dlc)
    {
        case 1:
            ret = "Dlc_Def";
        break;

        case 2:
            ret = "Dlc_Gkb";
        break;

        case 4:
            ret = "Dlc_1";
        break;

        case 8:
            ret = "Dlc_2";
        break;

        case 16:
            ret = "Dlc_3";
        break;

        case 32:
            ret = "Dlc_4";
        break;

        case 64:
            ret = "Dlc_5";
        break;

        case 128:
            ret = "Dlc_6";
        break;

        case 256:
            ret = "Dlc_7";
        break;

        case 512:
            ret = "Dlc_8";
        break;

        case 1024:
            ret = "Dlc_9";
        break;

        case 2048:
            ret = "Dlc_10";
        break;

        case 4096:
            ret = "Ver_Day1";
        break;

        case 65536:
            ret = "Ver_TU4";
        break;

        case 524288:
            ret = "UD7";
        break;

        case 0x10000000:
            ret = "PRB";
        break;

        case 0x20000000:
            ret = "EL0";
        break;

        case 0x40000000:
            ret = "Dlc_12";
        break;

        case 0x80000000:
            ret = "Dlc_13";
        break;

        case 0x100000000ULL:
            ret = "Dlc_14";
        break;

        case 0x1000000000ULL:
            ret = "Dlc_15";
        break;

        case 0x4000000000ULL:
            ret = "Dlc_16";
        break;

        case 0x20000000000ULL:
            ret = "Dlc_17";
        break;

        case 0x80000000000ULL:
            ret = "Dlc_18";
        break;
		
		case 0x100000000000ULL:
			ret = "Dlc_19";
		break;

        default:
            DPRINTF("%s: Unknown dlc 0x%016I64X.\n", FUNCNAME, dlc);
    }

    return ret;
}

static uint64_t StringToDlc(const std::string &dlc)
{
    if (dlc == "Dlc_Def")
        return 1;
    else if (dlc == "Dlc_Gkb")
        return 2;
    else if (dlc == "Dlc_1")
        return 4;
    else if (dlc == "Dlc_2")
        return 8;
    else if (dlc == "Dlc_3")
        return 16;
    else if (dlc == "Dlc_4")
        return 32;
    else if (dlc == "Dlc_5")
        return 64;
    else if (dlc == "Dlc_6")
        return 128;
    else if (dlc == "Dlc_7")
        return 256;
    else if (dlc == "Dlc_8")
        return 512;
    else if (dlc == "Dlc_9")
        return 1024;
    else if (dlc == "Dlc_10")
        return 2048;
    else if (dlc == "Ver_Day1")
        return 4096;
    else if (dlc == "Ver_TU4")
        return 65536;
    else if (dlc == "UD7")
        return 524288;
    else if (dlc == "PRB")
        return 0x10000000;
    else if (dlc == "EL0")
        return 0x20000000;
    else if (dlc == "Dlc_12")
        return 0x40000000;
    else if (dlc == "Dlc_13")
        return 0x80000000;
    else if (dlc == "Dlc_14")
        return 0x100000000ULL;
    else if (dlc == "Dlc_15")
        return 0x1000000000ULL;
    else if (dlc == "Dlc_16")
        return 0x4000000000ULL;
    else if (dlc == "Dlc_17")
        return 0x20000000000ULL;
    else if (dlc == "Dlc_18")
        return 0x80000000000ULL;
	else if (dlc == "Dlc_19")
		return 0x100000000000ULL;
    else
    {
        DPRINTF("%s: Unknown DLC: %s\n", FUNCNAME, dlc.c_str());
    }

    return 0xDEADBEEF;
}

bool Xv2PatcherSlotsFile::Load(const uint8_t *buf, size_t size)
{
    chara_slots.clear();

    if (!buf)
        return false;

    std::string raw_string;

    raw_string.resize(size);
    memcpy((void *)raw_string.data(), buf, size);

    size_t i = 0;

    while (i < raw_string.length())
    {
        CharaListSlot this_slot;

        if (raw_string[i] != '{')
        {
            DPRINTF("%s: Format error 1\n", FUNCNAME);
            return false;
        }

        i++;

        while (raw_string[i] == '[')
        {
            CharaListSlotEntry entry;

            i++;

            size_t pos = raw_string.find(']', i);
            if (pos == std::string::npos)
            {
                DPRINTF("%s: Format error 2\n", FUNCNAME);
                return false;
            }

            std::string costume = raw_string.substr(i, pos-i);
            std::vector<std::string> fields;

            Utils::GetMultipleStrings(costume, fields);
            if (fields.size() != 8 && fields.size() != 9 && fields.size() != 10)
            {
                DPRINTF("Invalid number of elements: %Id\n", fields.size());
                return false;
            }

            entry.with_undefined = false;
            entry.code = fields[0];

            if (entry.code.length() != 3)
            {
                DPRINTF("%s: Unexpected length for code: %s\n", FUNCNAME, entry.code.c_str());
                return false;
            }

            entry.code = "\"" + entry.code + "\"";
            entry.costume_index = Utils::GetSigned(fields[1]);
            entry.model_preset = Utils::GetSigned(fields[2]);
            entry.unlock_index = Utils::GetSigned(fields[3]);
            entry.flag_gk2 = (Utils::GetSigned(fields[4]) != 0);
            entry.voices_id_list[0] = Utils::GetSigned(fields[5]);
            entry.voices_id_list[1] = Utils::GetSigned(fields[6]);

            uint32_t dlc_low = Utils::GetSigned(fields[7]);
            uint32_t dlc_high = 0;

            if (fields.size() >= 9)
            {
                dlc_high = Utils::GetSigned(fields[8]);
            }

            entry.flag_cgk2 = (fields.size() >= 10) ? (Utils::GetSigned(fields[9]) != 0) : false;

            uint64_t dlc = dlc_low | ((uint64_t)dlc_high << 32);
            entry.dlc = DlcToString(dlc);

            if (entry.dlc.length() == 0)
                return false;

            i = pos + 1;
            this_slot.entries.push_back(entry);
        }

        if (raw_string[i] != '}')
        {
            DPRINTF("%s: Format error 3\n", FUNCNAME);
            return false;
        }

        i++;
        chara_slots.push_back(this_slot);
    }

    return true;
}

uint8_t *Xv2PatcherSlotsFile::Save(size_t *psize)
{
    std::string raw_string;

    for (const CharaListSlot &this_slot : chara_slots)
    {
        raw_string.push_back('{');

        for (const CharaListSlotEntry &entry : this_slot.entries)
        {
            raw_string.push_back('[');

            if (entry.code.length() != 5 || entry.code[0] != '"' || entry.code[4] != '"')
            {
                if (entry.code == "AvatarCode")
                {
                    raw_string += "HUM";
                }
                else
                {
                    DPRINTF("%s: Invalid format for code (%s)\n", FUNCNAME, entry.code.c_str());
                    return nullptr;
                }
            }
            else
            {
                raw_string += entry.code.substr(1, 3);
            }

            raw_string.push_back(',');
            raw_string += Utils::ToString(entry.costume_index);
            raw_string.push_back(',');
            raw_string += Utils::ToString(entry.model_preset);
            raw_string.push_back(',');
            raw_string += Utils::ToString(entry.unlock_index);
            raw_string.push_back(',');
            raw_string += ((entry.flag_gk2) ? "1" : "0");
            raw_string.push_back(',');
            raw_string += Utils::ToString(entry.voices_id_list[0]);
            raw_string.push_back(',');
            raw_string += Utils::ToString(entry.voices_id_list[1]);
            raw_string.push_back(',');

            uint64_t dlc = StringToDlc(entry.dlc);
            if (dlc == 0xDEADBEEF)
                return nullptr;

            raw_string += Utils::ToString(dlc&0xFFFFFFFF);
            raw_string.push_back(',');
            raw_string += Utils::ToString(dlc >> 32);

            raw_string.push_back(',');
            raw_string += ((entry.flag_cgk2) ? "1" : "0");
            raw_string.push_back(']');
        }

        raw_string.push_back('}');
    }

    uint8_t *buf = new uint8_t[raw_string.length()];
    memcpy(buf, raw_string.data(), raw_string.length());

    *psize = raw_string.length();
    return buf;
}

bool Xv2PatcherSlotsFile::LoadFromCst(const uint8_t *buf, size_t size, const uint8_t *buf_raid, size_t size_raid)
{
    chara_slots.clear();

    if (!buf || size < sizeof(CSTHeader) || !buf_raid || size_raid < sizeof(CSTHeader))
        return false;

    const CSTHeader *hdr = (const CSTHeader *)buf;
    if (hdr->signature != CST_SIGNATURE)
        return false;

    chara_slots.resize(hdr->num_slots);
    const CSTEntry *entry = (const CSTEntry *)(buf + hdr->header_size);

    size_t pos = (size_t)-1;

    for (uint32_t i = 0; i < hdr->num_costumes; i++, entry++)
    {
        if (entry->costume_id == 0)
            pos++;

        if (pos >= chara_slots.size())
        {
            DPRINTF("%s: Out of bounds.\n", FUNCNAME);
            return false;
        }

        if (entry->is_custom_costume)
            continue;

        /*if (entry->unk_28 != 0)
        {
           DPRINTF("%s_%d_%d:%d\n", entry->code, entry->costume_index, entry->model_preset, entry->unk_28);
        }*/

        // Some GOK entries at end of 1.10, but before the "HUM" for the other cacs
        // Update 1.14: the dummy GOK don't longer exit, so now we actually leave when detecting ZMD
        // Update 1.17, comment this now, as dlc 12 (or 13, whatever) uses this.
        /*if (entry->dlc == 0x80000000)
        {
            break; // Stop now
        }*/

        if (memcmp(entry->code, "ZMD", 4) == 0)
        {
            break;
        }

        CharaListSlotEntry s_entry;

        s_entry.costume_index = entry->costume_index;
        s_entry.code = entry->code;
        s_entry.code = "\"" + s_entry.code + "\"";
        s_entry.model_preset = entry->model_preset;
        s_entry.flag_gk2 = (entry->flag_gk2 != 0);
        s_entry.unlock_index = entry->unlock_index;
        s_entry.voices_id_list[0] = entry->voices_id_list[0];
        s_entry.voices_id_list[1] = entry->voices_id_list[1];

        s_entry.dlc = DlcToString(entry->dlc_key);
        if (s_entry.dlc.length() == 0)
        {
            DPRINTF("Failed on char %s\n", s_entry.code.c_str());
            return false;
        }

        s_entry.flag_cgk2 = (entry->flag_cgk2 != 0);

        chara_slots[pos].entries.push_back(s_entry);
    }

    chara_slots.resize(pos); // To remove dummy entries

    // Now add the PRB ones

    hdr = (const CSTHeader *)buf_raid;
    if (hdr->signature != CST_SIGNATURE)
        return false;

    entry = (const CSTEntry *)(buf_raid + hdr->header_size);
    chara_slots.resize(chara_slots.size() + hdr->num_slots);

    pos = pos-1;

    for (uint32_t i = 0; i < hdr->num_costumes; i++, entry++)
    {
        if (entry->dlc_key != 0x10000000) // if not PRB flag, ignore
            continue;

        if (entry->costume_id == 0)
            pos++;

        if (pos >= chara_slots.size())
        {
            DPRINTF("%s: Out of bounds.\n", FUNCNAME);
            return false;
        }       

        if (entry->is_custom_costume)
            continue;

        CharaListSlotEntry s_entry;

        s_entry.costume_index = entry->costume_index;
        s_entry.code = entry->code;
        s_entry.code = "\"" + s_entry.code + "\"";
        s_entry.model_preset = entry->model_preset;
        s_entry.flag_gk2 = (entry->flag_gk2 != 0);
        s_entry.unlock_index = entry->unlock_index;
        s_entry.voices_id_list[0] = entry->voices_id_list[0];
        s_entry.voices_id_list[1] = entry->voices_id_list[1];
        s_entry.dlc = DlcToString(entry->dlc_key);
        if (s_entry.dlc.length() == 0)
        {
            DPRINTF("Failed on char %s\n", s_entry.code.c_str());
            return false;
        }

        chara_slots[pos].entries.push_back(s_entry);
    }

    chara_slots.resize(pos+1); // To remove dummy entries
    return true;
}

CharaListSlotEntry *Xv2PatcherSlotsFile::FindFromAbsolutePos(size_t pos)
{
    size_t current_pos = 0;

    for (CharaListSlot &cslot : chara_slots)
    {
        size_t last_index = current_pos + cslot.entries.size() - 1;

        if (pos <= last_index)
        {
            return &cslot.entries[pos-current_pos];
        }

        current_pos += cslot.entries.size();
    }

    return nullptr;
}

size_t Xv2PatcherSlotsFile::FindSlotsByCode(const std::string &code, std::vector<CharaListSlotEntry *> &entries)
{
    entries.clear();

    for (CharaListSlot &cslot : chara_slots)
    {
        for (CharaListSlotEntry &entry : cslot.entries)
        {
            if (entry.code == code)
            {
                entries.push_back(&entry);
            }
        }
    }

    return entries.size();
}

size_t Xv2PatcherSlotsFile::RemoveSlots(const std::string &code)
{
    size_t removed = 0;

    for (size_t i = 0; i < chara_slots.size(); i++)
    {
        CharaListSlot &cslot = chara_slots[i];

        for (size_t j = 0; j < cslot.entries.size(); j++)
        {
            CharaListSlotEntry &entry = cslot.entries[j];

            if (entry.code == code)
            {
                cslot.entries.erase(cslot.entries.begin()+j);
                removed++;

                if (cslot.entries.size() == 0)
                    break;

                j--;
            }
        }

        if (cslot.entries.size() == 0)
        {
            chara_slots.erase(chara_slots.begin()+i);
            i--;
        }
    }

    return removed;
}

bool Xv2PatcherSlotsFile::FindFirstMatch(const std::string &code, size_t *pentry_idx) const
{
    for (size_t i = 0; i < chara_slots.size(); i++)
    {
        const CharaListSlot &slot = chara_slots[i];
        if (slot.entries.size() == 0)
            continue;

        const CharaListSlotEntry &entry = slot.entries[0];
        if (entry.code == code)
        {
            *pentry_idx = i;
            return true;
        }
    }

    return false;
}

bool Xv2PatcherSlotsFile::FindFirstMatch(const std::string &code, int costume_index, int model_preset, size_t *pentry_idx, size_t *subentry_idx) const
{
    for (size_t i = 0; i < chara_slots.size(); i++)
    {
        const CharaListSlot &slot = chara_slots[i];
        if (slot.entries.size() == 0)
            continue;

        for (size_t j = 0; j < slot.entries.size(); j++)
        {
            const CharaListSlotEntry &entry = slot.entries[j];
            if (entry.code == code && entry.costume_index == costume_index && entry.model_preset == model_preset)
            {
                *pentry_idx = i;
                *subentry_idx = j;
                return true;
            }
        }
    }

    return false;
}

bool Xv2PatcherSlotsFile::PlaceAtPos(size_t idx, const CharaListSlot &slot)
{
    if (idx > chara_slots.size())
        return false;

    if (idx == chara_slots.size())
    {
        chara_slots.push_back(slot);
    }
    else
    {
        chara_slots.insert(chara_slots.begin()+idx, slot);
    }

    return true;
}

size_t Xv2PatcherSlotsFile::ChangeVoiceIds(const std::string &code, int costume_index, int model_preset, int voice1, int voice2)
{
    size_t count = 0;

    for (CharaListSlot &slot : chara_slots)
    {
        for (CharaListSlotEntry &entry : slot.entries)
        {
            if (entry.code == code && entry.costume_index == costume_index && entry.model_preset == model_preset)
            {
                entry.voices_id_list[0] = voice1;
                entry.voices_id_list[1] = voice2;
                count++;
            }
        }
    }

    return count;
}
