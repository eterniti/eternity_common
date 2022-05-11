#include "QedFile.h"
#include "debug.h"

QedFile::QedFile()
{
    this->big_endian = false;
}

QedFile::~QedFile()
{

}

void QedFile::Reset()
{
    states.clear();
}

/*static bool IsKnownCondition(uint16_t opcode)
{
    if (opcode >= 60)
        return false;

    if (opcode == 14 || opcode == 15 || opcode == 18 || opcode == 23 || opcode == 41 || opcode == 52)
        return false;

    return true;
}*/

/*static bool IsKnownAction(uint16_t opcode)
{
    if (opcode >= 120)
        return false;

    if (opcode == 24 || opcode == 53 || opcode == 69 || opcode == 72 || opcode == 77 || opcode == 94 || opcode == 95 || opcode == 98 || opcode == 114)
        return false;

    return true;
}*/

bool QedFile::LoadInstructions(const uint8_t *top, const QEDInstruction *file_instructions, uint32_t num, bool action)
{
    for (uint32_t i = 0; i < num; i++)
    {
        if (file_instructions->state_index >= states.size())
        {
            states.resize(file_instructions->state_index+1);
        }

        QedState &state = states[file_instructions->state_index];

        if (state.events.find(file_instructions->event_index) == state.events.end())
        {
            QedEvent new_event;
            state.events[file_instructions->event_index] = new_event;
        }

        QedEvent &event = state.events[file_instructions->event_index];
        QedInstruction instruction;
        const QEDInstructionParams *file_params = (const QEDInstructionParams *)(top + file_instructions->params_offset);

        instruction.opcode = file_instructions->opcode;
        memcpy(&instruction.params, file_params, sizeof(QEDInstructionParams));

        if (action)
        {
            event.actions.push_back(instruction);
        }
        else
        {
            event.conditions.push_back(instruction);
        }

        /*if (action && !IsKnownAction(instruction.opcode))
        {
            DPRINTF("State %d, event %d: %d\n", file_instructions->state_index, file_instructions->event_index, instruction.opcode);
            return false;
        }*/
        /*if (action && instruction.opcode == QED_ACT_POWERUP)
        {
            int32_t value = instruction.params.group2.param2.num.i;

            if (value != 0)
            {
                DPRINTF("0x%x\n", value);
                Sleep(5000);
            }            
        }*/
        /*if (!action && instruction.opcode == QED_COND_UNK_44)
        {
            int32_t param1 = instruction.params.group1.group.nums.param1.num.i;
            int32_t param2 = instruction.params.group1.group.nums.param2.num.i;
            int32_t param3 = instruction.params.group1.group.nums.param3.num.i;
            int32_t param4 = instruction.params.group1.group.nums.param4.num.i;
            int32_t param5 = instruction.params.group1.group.nums.param5.num.i;
            int32_t param6 = instruction.params.group2.param2.num.i;
            int32_t param7 = instruction.params.group2.param3.num.i;
            int32_t param8 = instruction.params.group2.param4.num.i;

            DPRINTF("%d %d %d %d %d %d %d %d\n", param1, param2, param3, param4, param5, param6, param7, param8);
            Sleep(2400);
        }*/
        /*if (action && instruction.opcode == QED_ACT_UNK_78)
        {
            const char *param1 = instruction.params.group1.group.string.str;
            int32_t param2 = instruction.params.group2.param2.num.i;
            int32_t param3 = instruction.params.group2.param3.num.i;
            int32_t param4 = instruction.params.group2.param4.num.i;

            DPRINTF("%s %d %d %d\n", param1, param2, param3, param4);
            Sleep(3000);
        }*/

        file_instructions++;
    }

    return true;
}

void QedFile::SaveInstructions(uint8_t *top, QEDInstruction *file_instructions, bool action, QEDInstructionParams **pcurrent_param) const
{
    size_t n = 0;

    for (size_t i = 0; i < states.size(); i++)
    {
        const QedState &state = states[i];

        if (state.IsEmpty())
            continue;

        for (auto &it : state.events)
        {
            const QedEvent &event = it.second;
            const std::vector<QedInstruction> &instructions = (action) ? event.actions : event.conditions;

            for (const QedInstruction &instruction : instructions)
            {
                file_instructions[n].opcode = instruction.opcode;
                file_instructions[n].event_index = it.first;
                file_instructions[n].state_index = (uint16_t)i;
                file_instructions[n].params_offset = Utils::DifPointer(*pcurrent_param, top);
                memcpy(*pcurrent_param, &instruction.params, sizeof(QEDInstructionParams));

                (*pcurrent_param)++;
                n++;
            }
        }
    }
}

bool QedFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(QEDHeader))
        return false;

    const QEDHeader *hdr = (const QEDHeader *)buf;
    if (hdr->signature != QED_SIGNATURE)
        return false;

    const QEDInstruction *file_conditions = (const QEDInstruction *)GetOffsetPtr(hdr, hdr->conditions_start);
    const QEDInstruction *file_actions = (const QEDInstruction *)GetOffsetPtr(hdr, hdr->actions_start);

    if (!LoadInstructions(buf, file_conditions, hdr->num_conditions, false)) return false;
    if (!LoadInstructions(buf, file_actions, hdr->num_actions, true)) return false;

    /*for (size_t i = 0; i < states.size(); i++)
    {
        const QedState &state = states[i];

        if (!state.IsEmpty())
        {
           for (size_t j = 0; j < state.events.size(); j++)
           {
               //const QedEvent &event = state.events[j];
           }
        }
    }*/

    return true;
}

uint8_t *QedFile::Save(size_t *psize)
{
    size_t num_conditions = GetNumConditions();
    size_t num_actions = GetNumActions();

    *psize = sizeof(QEDHeader) + ((num_actions+num_conditions) *(sizeof(QEDInstruction) + sizeof(QEDInstructionParams)));
    uint8_t *buf = new uint8_t[*psize];
    memset(buf, 0, *psize);

    QEDHeader *hdr = (QEDHeader *)buf;
    hdr->signature = QED_SIGNATURE;
    hdr->endianess_check = 0xFFFE;
    hdr->unk_08 = 1;
    hdr->unk_0C = 0x10;
    hdr->num_conditions = (uint32_t)num_conditions;
    hdr->num_actions = (uint32_t) num_actions;
    hdr->conditions_start = sizeof(QEDHeader);
    hdr->actions_start = hdr->conditions_start + (uint32_t)num_conditions*sizeof(QEDInstruction);

    QEDInstruction *file_conditions = (QEDInstruction *)(buf + hdr->conditions_start);
    QEDInstruction *file_actions = (QEDInstruction *)(buf + hdr->actions_start);

    QEDInstructionParams *current_param = (QEDInstructionParams *)(file_actions + num_actions);

    SaveInstructions(buf, file_conditions, false, &current_param);
    SaveInstructions(buf, file_actions, true, &current_param);

    return buf;
}

size_t QedFile::GetNumConditions() const
{
    size_t num = 0;

    for (const QedState &state : states)
    {
        for (auto &it : state.events)
        {
            num += it.second.conditions.size();
        }
    }

    return num;
}

size_t QedFile::GetNumActions() const
{
    size_t num = 0;

    for (const QedState &state : states)
    {
        for (auto &it : state.events)
        {
            num += it.second.actions.size();
        }
    }

    return num;
}



