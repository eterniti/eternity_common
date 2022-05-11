#include <stdexcept>

#include "CharaListFile.h"
#include "debug.h"

#define ARRAY_DECLARATION   "public static var CharaListBase:Array ="

CharaListFile::CharaListFile()
{
    this->big_endian = false;
}

CharaListFile::~CharaListFile()
{
}

void CharaListFile::SimplifyString(std::string &string)
{
    Utils::TrimString(string);

    for (size_t i = 0; i < string.length(); i++)
    {
        if (string[i] <= ' ')
        {
            string[i] = ' ';

            for (size_t j = i+1; j < string.length(); j++)
            {
                if (string[j] <= ' ')
                {
                    string.erase(string.begin()+j);
                    j--;
                    i--;
                }
                else
                {
                    break;
                }
            }
        }
    }
}

bool CharaListFile::ParseSlotEntry(std::string &string, CharaListSlotEntry &entry)
{
    Utils::TrimString(string);
    entry.with_undefined = false;

    if (string.length() == 0 || string[0] != '[' || string[string.length()-1] != ']')
    {
        DPRINTF("%s: Internal parse error.\n", FUNCNAME);
        return false;
    }

    string = string.substr(1);
    string = string.substr(0, string.length()-1);

    std::vector<std::string> elements;

    if (Utils::GetMultipleStrings(string, elements, ',') != 8)
    {
        if (elements.size() == 8 && elements[5] == "undefined")
        {
            entry.with_undefined = true;
            elements.erase(elements.begin()+5);
        }
        else
        {
            DPRINTF("%s: Parse error, not expected number of elements, in %s\n", FUNCNAME, string.c_str());
            return false;
        }
    }

    entry.code = elements[0];

    if (entry.code != "AvatarCode" && entry.code.length() != 5)
    {
        DPRINTF("%s: Parse error, a code name doesn't have length expected: %s\n", FUNCNAME, entry.code.c_str());
        return false;
    }

#ifdef __MINGW32__

    entry.costume_index = strtoll(elements[1].c_str(), nullptr, 0);
    entry.model_preset = strtoll(elements[2].c_str(), nullptr, 0);
    entry.unlock_index = strtoll(elements[3].c_str(), nullptr, 0);

#else

    try
    {
        entry.costume_index = std::stoi(elements[1], nullptr, 0);
    }
    catch (const std::invalid_argument &)
    {
        DPRINTF("%s: Parse error while parsing costume_index.\n", FUNCNAME);
        return false;
    }

    try
    {
        entry.model_preset = std::stoi(elements[2], nullptr, 0);
    }
    catch (const std::invalid_argument &)
    {
        DPRINTF("%s: Parse error while parsing model_preset.\n", FUNCNAME);
        return false;
    }

    try
    {
        entry.unlock_index = std::stoi(elements[3], nullptr, 0);
    }
    catch (const std::invalid_argument &)
    {
        DPRINTF("%s: Parse error while parsing unlock_index.\n", FUNCNAME);
        return false;
    }

#endif

    if (elements[4] == "true")
    {
        entry.flag_gk2 = true;
    }
    else if (elements[4] == "false")
    {
        entry.flag_gk2 = false;
    }
    else
    {
        DPRINTF("%s: Parse error, flag_gk2 is not true or false: %s\n", FUNCNAME, elements[4].c_str());
        return false;
    }

    if (elements[5].front() != '[')
    {
        DPRINTF("%s: Unexpected start for VoiceIdList: %c\n", FUNCNAME, elements[5].front());
        return false;
    }

#ifdef __MINGW32__

    entry.voices_id_list[0] = strtoll(elements[5].substr(1).c_str(), nullptr, 0);

#else
    try
    {
        entry.voices_id_list[0] = std::stoi(elements[5].substr(1), nullptr, 0);
    }
    catch (const std::invalid_argument &)
    {
        DPRINTF("%s: Parse error while parsing voices_id_list[0].\n", FUNCNAME);
        return false;
    }
#endif

    if (elements[6].back() != ']')
    {
        DPRINTF("%s: Unexpected end for VoiceIdList: %c\n", FUNCNAME, elements[6].back());
        return false;
    }

#ifdef __MINGW32__

    entry.voices_id_list[1] = strtoll(elements[6].substr(0, elements[6].length()-1).c_str(), nullptr, 0);

#else

    try
    {
        entry.voices_id_list[1] = std::stoi(elements[6].substr(0, elements[6].length()-1), nullptr, 0);
    }
    catch (const std::invalid_argument &)
    {
        DPRINTF("%s: Parse error while parsing voices_id_list[1].\n", FUNCNAME);
        return false;
    }
#endif

    entry.dlc = elements[7];

    return true;
}

enum
{
    INITIAL,
    ARRAY_FOUND,
    IN_ARRAY,
    ARRAY_END,
    END,
};

bool CharaListFile::Load(const uint8_t *buf, size_t size)
{
    if (!buf || size == 0)
        return false;

    char *temp_buf = new char[size+1];
    temp_buf[size] = 0;

    memcpy(temp_buf, buf, size);
    std::string content = temp_buf;
    delete[] temp_buf;

    size_t pos = 0;
    bool eof = false;

    do
    {
        std::string line;
        size_t old_pos = pos;

        pos = content.find('\n', pos);
        if (pos == std::string::npos)
        {
            line = content.substr(old_pos);
            eof = true;
        }
        else
        {
            bool rn = false;

            if (pos > 0 && content[pos-1] == '\r')
            {
                pos--;
                rn = true;
            }

            line = content.substr(old_pos, pos-old_pos);

            if (rn)
                pos += 2;
            else
                pos++;
        }

        Utils::TrimString(line, false, true);
        lines.push_back(line);

    } while (!eof);

    int state = INITIAL;
    bool in_multi_line_comment = false;
    bool in_single_line_comment = false;
    bool in_single_quotes = false;
    bool in_double_quotes = false;

    int array_level = 0;

    std::string temp;

    for (size_t i = 0; i < lines.size(); i++)
    {
        const std::string &line = lines[i];

        for (size_t j = 0; j < line.length(); j++)
        {
            bool last_char = (j == line.length()-1);
            bool quotes_closing = false;

            if (!in_single_quotes && !in_double_quotes)
            {
                if (!in_multi_line_comment && !in_single_line_comment)
                {
                    if (!last_char)
                    {
                        if (line[j] == '/')
                        {
                            if (line[j+1] == '*')
                            {
                                in_multi_line_comment = true;
                                j++;
                                continue;
                            }
                            else if (line[j+1] == '/')
                            {
                                in_single_line_comment = true;
                                j++;
                                continue;
                            }
                        }
                    }
                }
                else if (in_multi_line_comment)
                {
                    if (!last_char && line[j] == '*' && line[j+1] == '/')
                    {
                        j++;
                        in_multi_line_comment = false;
                    }

                    continue;
                }
                else if (in_single_line_comment)
                {
                    if (j == 0)
                        in_single_line_comment = false;
                    else
                        continue;
                }
            }

            if (!in_single_quotes && !in_double_quotes)
            {
                if (line[j] == '\'')
                {
                    in_single_quotes = true;
                }
                else if (line[j] == '"')
                {
                    in_double_quotes = true;
                }
            }
            else if (in_single_quotes)
            {
                if (line[j] == '\'')
                {
                    in_single_quotes = false;
                    quotes_closing = true;
                }
            }
            else if (in_double_quotes)
            {
                if (line[j] == '"')
                {
                    in_double_quotes = false;
                    quotes_closing = true;
                }
            }

            switch (state)
            {
                case INITIAL:

                    if (in_double_quotes || in_single_quotes || quotes_closing)
                    {
                        temp.clear();
                    }
                    else
                    {
                        temp.push_back(line[j]);

                        if (line[j] > ' ')
                        {
                            SimplifyString(temp);

                            if (temp == ARRAY_DECLARATION)
                            {
                                state = ARRAY_FOUND;
                                temp.clear();
                                continue;
                            }
                        }

                        if (last_char)
                        {
                            temp.clear();
                            continue;
                        }
                    }

                break;

                case ARRAY_FOUND:

                    if (line[j] > ' ')
                    {
                        if (line[j] != '[')
                        {
                            DPRINTF("%s: Parse error, cannot find start of array.\n", FUNCNAME);
                            return false;
                        }
                        else
                        {
                            state = IN_ARRAY;
                            array_level = 1;
                            slots_line_start = i;

                            CharaListSlot cslot;
                            chara_slots.push_back(cslot);
                        }
                    }

                break;

                case IN_ARRAY:
                {
                    bool level2_open_close = false;

                    if (line[j] > ' ')
                    {
                        if (!in_single_quotes && !in_double_quotes && !quotes_closing)
                        {
                            if (line[j] == '[')
                            {
                                array_level++;

                                if (array_level == 2)
                                    level2_open_close = true;
                            }
                            else if (line[j] == ']')
                            {
                                array_level--;

                                if (array_level == 2)
                                {
                                    CharaListSlotEntry entry;

                                    level2_open_close = true;
                                    temp.push_back(']');

                                    if (!ParseSlotEntry(temp, entry))
                                        return false;

                                    if (chara_slots.size() == 0)
                                    {
                                        DPRINTF("%s: Internal parse error, slots at 0.\n", FUNCNAME);
                                        return false;
                                    }

                                    CharaListSlot &cslot = chara_slots.back();

                                    if (cslot.entries.size() == XV2_MAX_SUBSLOTS)
                                    {
                                        DPRINTF("%s: Parse error, number of costumes %Id bigger than %d.\n", FUNCNAME, cslot.entries.size()+1, XV2_MAX_SUBSLOTS);
                                        return false;
                                    }

                                    cslot.entries.push_back(entry);
                                    temp.clear();
                                }
                                else if (array_level == 1)
                                {
                                    CharaListSlot cslot;

                                    chara_slots.push_back(cslot);
                                }
                                else if (array_level == 0)
                                {
                                    state = ARRAY_END;
                                }
                            }
                        }
                    }

                    if (array_level == 2 && line[j] == ',')
                        continue;

                    if (array_level >= 2 && !level2_open_close)
                    {
                        if (line[j] > ' ' || in_single_quotes || in_double_quotes || quotes_closing)
                        {
                            temp.push_back(line[j]);
                        }
                    }
                }
                break;

                case ARRAY_END:

                    if (line[j] > ' ')
                    {
                        if (line[j] == ';')
                        {
                            state = END;
                        }
                        else
                        {
                            DPRINTF("%s: Parse error, expected \";\" after array.\n", FUNCNAME);
                            return false;
                        }
                    }

                break;
            } // switch

        } // for this line

        if (state > INITIAL)
        {
            lines.erase(lines.begin()+i);
            i--;

            if (state == END)
                break;
        }
    }

    if (state != END)
    {
        DPRINTF("%s: Parse error, parser ended at state %d.\n", FUNCNAME, state);
        return false;
    }

    chara_slots.pop_back();

    /*for (size_t i = 0; i < chara_slots.size(); i++)
    {
        const CharaListSlot &cslot = chara_slots[i];

        for (size_t j = 0; j < cslot.entries.size(); j++)
        {
            const CharaListSlotEntry &entry = cslot.entries[j];

            DPRINTF("%Id:%Id -> [%s,%d,%d,%d,%s,[%d,%d]]", i, j, entry.code.c_str(),
                    entry.costume_index, entry.model_preset, entry.unlock_index,
                    (entry.flag_gk2) ? "true" : "false", entry.voices_id_list[0], entry.voices_id_list[1]);
        }
    }*/

    return true;
}

uint8_t *CharaListFile::Save(size_t *psize)
{
    std::string content;

    for (size_t i = 0; i < slots_line_start; i++)
    {
        content += lines[i] + "\r\n";
    }

    content += "      public static var CharaListBase:Array = [\r\n";

    for (size_t i = 0; i < chara_slots.size(); i++)
    {
        const CharaListSlot &cslot = chara_slots[i];

        content += "\t\t\t\t\t\t\t\t\t\t\t\t[";

        for (size_t j = 0; j < cslot.entries.size(); j++)
        {
            const CharaListSlotEntry &entry = cslot.entries[j];
            char temp[256];

            if (entry.with_undefined)
            {
                snprintf(temp, sizeof(temp), "[%s,%d,%d,%d,%s,undefined,[%d,%d],%s]", entry.code.c_str(),
                    entry.costume_index, entry.model_preset, entry.unlock_index,
                    (entry.flag_gk2) ? "true" : "false", entry.voices_id_list[0], entry.voices_id_list[1],
                    entry.dlc.c_str());
            }
            else
            {
                snprintf(temp, sizeof(temp), "[%s,%d,%d,%d,%s,[%d,%d],%s]", entry.code.c_str(),
                    entry.costume_index, entry.model_preset, entry.unlock_index,
                    (entry.flag_gk2) ? "true" : "false", entry.voices_id_list[0], entry.voices_id_list[1],
                    entry.dlc.c_str());
            }

            content += temp;

            if (j != (cslot.entries.size()-1))
            {
                content += ", ";
            }
            else if (i != (chara_slots.size()-1))
            {
                content += "],\r\n";
            }
            else
            {
                content += "]\r\n\t\t\t\t\t\t\t\t\t\t\t];";
            }
        }
    }

    content += '\n';

    for (size_t i = slots_line_start; i < lines.size(); i++)
    {
        content += lines[i];

        if (i != (lines.size()-1))
            content += "\r\n";
    }

    uint8_t *buf = new uint8_t[content.length()];
    memcpy(buf, content.c_str(), content.length());

    *psize = content.length();
    return buf;
}

CharaListSlotEntry *CharaListFile::FindFromAbsolutePos(size_t pos)
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

size_t CharaListFile::FindSlotsByCode(const std::string &code, std::vector<CharaListSlotEntry *> &entries)
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

size_t CharaListFile::RemoveSlots(const std::string &code)
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

bool CharaListFile::HasText(const std::string &text)
{
    for (const std::string &line : lines)
        if (line.find(text) != std::string::npos)
            return true;

    return false;
}
