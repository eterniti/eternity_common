#include "Xv2PatcherSlotsFileStage.h"
#include "debug.h"

Xv2PatcherSlotsFileStage::Xv2PatcherSlotsFileStage()
{
    this->big_endian = false;
}

Xv2PatcherSlotsFileStage::~Xv2PatcherSlotsFileStage()
{

}

bool Xv2PatcherSlotsFileStage::Load(const uint8_t *buf, size_t size)
{
    stage_slots.clear();

    if (!buf)
        return false;

    std::string raw_string;

    raw_string.resize(size);
    memcpy((void *)raw_string.data(), buf, size);

    size_t i = 0;

    while (i < raw_string.length())
    {
        Xv2StageSlot this_slot;

        if (raw_string[i] != '[')
        {
            DPRINTF("%s: Format error 1\n", FUNCNAME);
            return false;
        }

        i++;

        size_t pos = raw_string.find(']', i);
        if (pos == std::string::npos)
        {
            DPRINTF("%s: Format error 2\n", FUNCNAME);
            return false;
        }

        std::string stage = raw_string.substr(i, pos-i);
        std::vector<std::string> fields;

        Utils::GetMultipleStrings(stage, fields);
        if (fields.size() != 2)
        {
            DPRINTF("Invalid number of elements: %Id\n", fields.size());
            return false;
        }

        this_slot.stage = Utils::GetUnsigned(fields[0]);
        this_slot.dlc = Utils::GetSigned(fields[1]);

        stage_slots.push_back(this_slot);

        i = pos+1;
    }

    //UPRINTF("Num of slots = %Id\n", stage_slots.size());

    return true;
}

uint8_t *Xv2PatcherSlotsFileStage::Save(size_t *psize)
{
    std::string raw_string;

    for (const Xv2StageSlot &this_slot : stage_slots)
    {
        raw_string.push_back('[');
        raw_string += Utils::ToString(this_slot.stage);
        raw_string.push_back(',');
        raw_string += Utils::ToString(this_slot.dlc);
        raw_string.push_back(']');
    }

    uint8_t *buf = new uint8_t[raw_string.length()];
    memcpy(buf, raw_string.data(), raw_string.length());

    *psize = raw_string.length();
    return buf;
}

size_t Xv2PatcherSlotsFileStage::FindSlots(uint32_t stage_id, std::vector<Xv2StageSlot *> &entries)
{
    entries.clear();

    for (Xv2StageSlot &this_slot : stage_slots)
    {
        if (this_slot.stage == stage_id)
        {
            entries.push_back(&this_slot);
        }
    }

    return entries.size();
}

size_t Xv2PatcherSlotsFileStage::RemoveSlots(uint32_t stage_id)
{
    size_t removed = 0;

    for (size_t i = 0; i < stage_slots.size(); i++)
    {
        if (stage_slots[i].stage == stage_id)
        {
            stage_slots.erase(stage_slots.begin()+i);
            i--;
            removed++;
        }
    }

    return removed;
}
