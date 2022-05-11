#include <stdio.h>
#include <string.h>
#include <limits>
#include "IniFile.h"


#include <math.h>

#include "debug.h"

IniFile::IniFile()
{
    Reset();
}

void IniFile::Reset()
{
    lines.clear();
}

IniFile::~IniFile()
{
    Reset();
}

std::string IniFile::UsefulString(const std::string &str)
{
    std::string result;
    std::string temp = str;
    Utils::TrimString(temp);

    bool in_single_quote = false;
    bool in_double_quote = false;
    char prev = 0;

    for (char c : temp)
    {
        if (c == '\'' && prev != '\\')
        {
            if (in_single_quote)
            {
                in_single_quote = false;
            }
            else if (!in_double_quote)
            {
                in_single_quote = true;
            }
        }
        else if (c == '"' && prev != '\\')
        {
            if (in_double_quote)
            {
                in_double_quote = false;
            }
            else if (!in_single_quote)
            {
                in_double_quote = true;
            }
        }
        else if (c == '#' || c == ';')
        {
            if (!in_single_quote && !in_double_quote)
            {
                Utils::TrimString(result);
                return result;
            }
        }

        result.push_back(c);
        prev = c;
    }

    Utils::TrimString(result);
    return result;
}

bool IniFile::GetAssignment(const std::string &str, std::string &left, std::string &right)
{
    char prev = 0;
    bool assignment_found = false;
    bool in_single_quote = false;
    bool in_double_quote = false;

    left.clear();
    right.clear();

    std::string temp = UsefulString(str);

    for (char c : temp)
    {
        if (assignment_found)
        {
            right.push_back(c);
        }
        else
        {
            if (c == '\'' && prev != '\\')
            {
                if (in_single_quote)
                {
                    in_single_quote = false;
                }
                else if (!in_double_quote)
                {
                    in_single_quote = true;
                }
            }
            else if (c == '"' && prev != '\\')
            {
                if (in_double_quote)
                {
                    in_double_quote = false;
                }
                else if (!in_single_quote)
                {
                    in_double_quote = true;
                }
            }
            else if (c == '=')
            {
                if (!in_single_quote && !in_double_quote)
                {
                    assignment_found = true;
                }
            }

            if (!assignment_found)
                left.push_back(c);
        }
    }

    if (!assignment_found)
        return false;

    Utils::TrimString(left);
    Utils::TrimString(right);

    return true;
}

bool IniFile::Load(const uint8_t *buf, size_t size)
{
    std::string content, section;
    size_t pos = 0;
    bool eof = false;

    Reset();

    if (!buf || !size)
        return true;

    char *temp = new char[size+1]; 
    temp[size] = 0;
    
	memcpy(temp, buf, size);
    content = temp;
    delete[] temp;

    // UTF8-bom
    // TODO: on save, keep bom if it was set
    if (content.length() >= 3 && content[0] == '\xEF' && content[1] == '\xBB' && content[2] == '\xBF')
        content = content.substr(3);

    do
    {
        Line line;
        size_t old_pos = pos;

        line.is_section = false;

        pos = content.find('\n', pos);
        if (pos == std::string::npos)
        {
            eof = true;
            line.content = content.substr(old_pos);
        }
        else
        {
            bool rn = false;

            if (pos > 0 && content[pos-1] == '\r')
            {
                pos--;
                rn = true;
            }

            line.content = content.substr(old_pos, pos-old_pos);

            if (rn)
                pos += 2;
            else
                pos++;
        }

        std::string str = line.content;

        Utils::TrimString(str);

        if (str.length() > 0 && (str[0] == '#' || str[0] == ';'))
        {
            line.is_comment = true;
        }
        else
        {
            line.is_comment = false;
            str = UsefulString(str);

            if (str.length() >= 2 && str[0] == '[' && str[str.length()-1] == ']')
            {
                line.is_section = true;
                section = str.substr(1,  str.length()-2);
            }
        }

        line.section = section;
        lines.push_back(line);

    } while(!eof);

    return true;
}

uint8_t *IniFile::Save(size_t *psize)
{
    size_t file_size = 0;

    for (size_t i = 0; i < lines.size(); i++)
    {
        file_size += lines[i].content.length();

        if (i != (lines.size()-1))
            file_size += 2;
    }

    if (file_size == 0)
        return nullptr;

    uint8_t *buf = new uint8_t[file_size];  
    uint8_t *ptr = buf;
	
    for (size_t i = 0; i < lines.size(); i++)
    {
        memcpy(ptr, lines[i].content.c_str(), lines[i].content.length());
        ptr += lines[i].content.length();

        if (i != (lines.size()-1))
        {
            *ptr = '\r';
            *(ptr+1) = '\n';
            ptr += 2;
        }
    }

    *psize = file_size;
    return buf;
}

bool IniFile::GetRawValue(const std::string &section, const std::string &name, std::string &value)
{
    for (const Line &line : lines)
    {
        if (line.is_comment || line.is_section || Utils::ToLowerCase(line.section) != Utils::ToLowerCase(section))
            continue;

        std::string left, right;

        if (!GetAssignment(line.content, left, right))
            continue;

        if (Utils::IsEmptyString(left) || Utils::IsEmptyString(right))
            continue;

        if (Utils::ToLowerCase(left) == Utils::ToLowerCase(name))
        {
            value = right;
            return true;
        }
    }

    return false;
}

bool IniFile::SetRawValue(const std::string &section, const std::string &name, const std::string &value, bool must_exist)
{
    size_t last_pos_section = std::string::npos;

    for (size_t i = 0; i < lines.size(); i++)
    {
        Line &line = lines[i];

        if (Utils::ToLowerCase(line.section) == Utils::ToLowerCase(section))
        {
            last_pos_section = i;

            if (line.is_comment || line.is_section)
                continue;

            std::string left, right;

            if (!GetAssignment(line.content, left, right))
                continue;

            if (Utils::IsEmptyString(left))
                continue;

            if (Utils::ToLowerCase(left) == Utils::ToLowerCase(name))
            {
                line.content = left + " = " + value;
                return true;
            }
        }
    }

    if (must_exist)
        return false;

    if (last_pos_section != std::string::npos)
    {
        Line line;

        line.content = name + " = " + value;
        line.section = section;
        line.is_comment = line.is_section = false;

        if (last_pos_section+1 >= lines.size())
        {
            lines.push_back(line);
        }
        else
        {
            lines.insert(lines.begin()+last_pos_section+1, line);
        }
    }
    else
    {
        // Create section too
        Line section_l, line;

        section_l.content = "[" + section + "]";
        section_l.section = section;
        section_l.is_comment = false;
        section_l.is_section= true;

        line.content = name + " = " + value;
        line.section = section;
        line.is_comment = line.is_section = false;

        if (lines.size() > 0)
        {
            // Separate sections with a dummy line
            Line dummy;

            dummy.content = "";
            dummy.section = lines[lines.size()-1].section;
            dummy.is_comment = dummy.is_section = false;

            lines.push_back(dummy);
        }

        lines.push_back(section_l);
        lines.push_back(line);
    }

    return true;
}

bool IniFile::GetIntegerValue(const std::string &section, const std::string &name, int *value, int error_value)
{
    std::string raw;

    if (!GetRawValue(section, name, raw))
    {
        *value = error_value;
        return false;
    }

    for (auto &c : integer_constants)
    {
        if (Utils::ToLowerCase(c.name) == Utils::ToLowerCase(raw))
        {
            if (c.value > 0xFFFFFFFF)
            {
                *value = error_value;
                return false;
            }

            *value = (int)c.value;
            return true;
        }
    }

#ifdef __MINGW32__
	*value = strtoll(raw.c_str(), nullptr, 0);
#else
    try
    {
        *value = std::stoi(raw, nullptr, 0);
    }
    catch (const std::invalid_argument &)
    {
        *value = error_value;
        return false;
    }
#endif

    return true;
}

bool IniFile::GetIntegerValue(const std::string &section, const std::string &name, int64_t *value, int error_value)
{
    std::string raw;

    if (!GetRawValue(section, name, raw))
    {
        *value = error_value;
        return false;
    }

    for (auto &c : integer_constants)
    {
        if (Utils::ToLowerCase(c.name) == Utils::ToLowerCase(raw))
        {
            *value = (int)c.value;
            return true;
        }
    }

#ifdef __MINGW32__
    *value = strtoll(raw.c_str(), nullptr, 0);
#else
    try
    {
        *value = std::stoll(raw, nullptr, 0);
    }
    catch (const std::invalid_argument &)
    {
        *value = error_value;
        return false;
    }
#endif

    return true;
}

bool IniFile::SetIntegerValue(const std::string &section, const std::string &name, int64_t value, bool hexadecimal, bool must_exist)
{
    char raw[32];

    for (auto &c: integer_constants)
    {
        if (c.value == value)
            return SetRawValue(section, name, c.name, must_exist);
    }

    if (!hexadecimal)
        sprintf(raw, "%I64d", value);
    else
        sprintf(raw, "0x%I64x", value);

    return SetRawValue(section, name, raw, must_exist);
}

void IniFile::ParseString(const std::string &raw, std::string &value, const std::string section, const std::string &name)
{
    bool has_quotes = (raw.length() > 0 && (raw[0] == '"' || raw[0] == '\''));
    bool single_quotes = (has_quotes && raw[0] == '\'');
    bool quotes_closed = false;

    if (has_quotes)
    {
        value.clear();
    }
    else
    {
        if (raw.length() > 0)
            value = raw[0];
    }

    for (size_t i = 1; i < raw.length(); i++) // Start in second character
    {
        char c = raw[i];

        if (has_quotes)
        {
            if ((single_quotes && c == '\'') || (!single_quotes && c == '"'))
            {
                if (i != (raw.length()-1))
                {
                    DPRINTF("%s: Warning: possible unintended use of quotes without \"\\\" prepended in %s:%s.\n", FUNCNAME, section.c_str(), name.c_str());
                }

                quotes_closed = true;
                break;
            }
        }

        if (i != (raw.length()-1))
        {
            if (c == '\\' && raw[i+1] == 'n')
            {
                value.push_back('\n');
                i++;
                continue;
            }
            else if (c == '\\' && raw[i+1] == '\"')
            {
                value.push_back('\"');
                i++;
                continue;
            }
            else if (c == '\\' && raw[i+1] == '\'')
            {
                value.push_back('\'');
                i++;
                continue;
            }
            else if (c == '\\' && raw[i+1] == '\\')
            {
                value.push_back('\\');
                i++;
                continue;
            }
        }

        value.push_back(c);
    }

    if (has_quotes && !quotes_closed)
    {
        DPRINTF("%s: Warning: unclosed quotes at %s:%s.\n", FUNCNAME, section.c_str(), name.c_str());
    }
}

bool IniFile::GetStringValue(const std::string &section, const std::string &name, std::string &value, const std::string error_value)
{
    std::string raw;

    if (!GetRawValue(section, name, raw))
    {
        value = error_value;
        return false;
    }

    ParseString(raw, value, section, name);
    return true;
}

bool IniFile::SetStringValue(const std::string &section, const std::string &name, const std::string &value, bool with_quotes, bool must_exist)
{
    std::string raw;

    for (char c : value)
    {
        if (c == '\n')
        {
            raw += "\\n";
        }
        else if (c == '"')
        {
            raw += "\\\"";
        }
        else
        {
            raw.push_back(c);
        }
    }

    if (with_quotes)
    {
        raw.insert(raw.begin(), '"');
        raw.push_back('"');
    }

    return SetRawValue(section, name, raw, must_exist);
}

bool IniFile::GetBooleanValue(const std::string &section, const std::string &name, bool *value, bool error_value)
{
    std::string str;
    int i;

    if (!GetStringValue(section, name, str))
    {
        *value = error_value;
        return false;
    }

    std::string lower_str = Utils::ToLowerCase(str);

    if (lower_str == "true" || lower_str == "on" || lower_str == "yes" || lower_str == "yeah")
    {
        *value = true;
        return true;
    }

    if (lower_str == "false" || lower_str == "off" || lower_str == "no" || lower_str == "nope")
    {
        *value = false;
        return true;
    }

    bool ret = GetIntegerValue(section, name, &i, error_value);

    *value = (i != 0);
    return ret;
}

bool IniFile::SetBooleanValue(const std::string &section, const std::string &name, bool value, bool must_exist)
{
    return SetRawValue(section, name, (value) ? "true" : "false", must_exist);
}

bool IniFile::GetFloatValue(const std::string &section, const std::string &name, float *value, float error_value)
{
    std::string raw, raw_low;
    float ret;

    if (!GetRawValue(section, name, raw))
    {
        *value = error_value;
        return false;
    }

    raw_low = Utils::ToLowerCase(raw);
    if (raw_low == "infinity" || raw_low == "inf" || raw_low == "+infinity" || raw_low == "+inf")
    {
        *value = std::numeric_limits<float>::infinity();
        return true;
    }
    else if (raw_low == "-infinity" || raw_low == "-inf")
    {
        *value = -std::numeric_limits<float>::infinity();
        return true;
    }

    if (sscanf(raw.c_str(), "%f", &ret) != 1)
    {
        *value = error_value;
        return false;
    }

    *value = ret;
    return true;
}

bool IniFile::SetFloatValue(const std::string &section, const std::string &name, float value, bool must_exist)
{
    return SetRawValue(section, name, Utils::FloatToString(value), must_exist);
}

bool IniFile::GetMultipleIntegersValues(const std::string &section, const std::string &name, std::vector<int> &values, bool clear_on_error)
{
    std::vector<std::string> strings;

    if (!GetMultipleStringsValues(section, name, strings))
    {
        if (clear_on_error)
            values.clear();

        return false;
    }

    for (const std::string &str : strings)
    {
        bool constant_found = false;

        for (auto &c : integer_constants)
        {
            if (Utils::ToLowerCase(c.name) == Utils::ToLowerCase(str))
            {
                if (c.value > 0xFFFFFFFF)
                {
                    if (clear_on_error)
                        values.clear();

                    return false;
                }

                values.push_back((int)c.value);
                constant_found = true;
                break;
            }
        }

        if (constant_found)
            continue;

#ifdef __MINGW32__
        values.push_back((int)strtoll(str.c_str(), nullptr, 0));
#else
        try
        {
            values.push_back(std::stoi(str, nullptr, 0));
        }
        catch (const std::invalid_argument &)
        {
            if (clear_on_error)
                values.clear();

            return false;
        }
#endif
    }

    return true;
}

bool IniFile::GetMultipleIntegersValues(const std::string &section, const std::string &name, std::vector<int64_t> &values, bool clear_on_error)
{
    std::vector<std::string> strings;

    if (!GetMultipleStringsValues(section, name, strings))
    {
        if (clear_on_error)
            values.clear();

        return false;
    }

    for (const std::string &str : strings)
    {
        bool constant_found = false;

        for (auto &c : integer_constants)
        {
            if (Utils::ToLowerCase(c.name) == Utils::ToLowerCase(str))
            {
                values.push_back(c.value);
                constant_found = true;
                break;
            }
        }

        if (constant_found)
            continue;

#ifdef __MINGW32__
        values.push_back(strtoll(str.c_str(), nullptr, 0));
#else
        try
        {
            values.push_back(std::stoll(str, nullptr, 0));
        }
        catch (const std::invalid_argument &)
        {
            if (clear_on_error)
                values.clear();

            return false;
        }
#endif
    }

    return true;
}

bool IniFile::SetMultipleIntegersValues(const std::string &section, const std::string &name, const std::vector<int> &values, bool hexadecimal, bool must_exist)
{
    std::string str;

    for (size_t i = 0; i < values.size(); i++)
    {
        bool constant_found = false;

        if (i != 0)
            str += ',';

        for (auto &c: integer_constants)
        {
            if (c.value == values[i])
            {
                str += c.name;
                constant_found = true;
                break;
            }
        }

        if (constant_found)
            continue;

        char raw[32];

        if (!hexadecimal)
            sprintf(raw, "%d", values[i]);
        else
            sprintf(raw, "0x%x", values[i]);

        str += raw;
    }

    return SetStringValue(section, name, str, true, must_exist);
}

bool IniFile::SetMultipleIntegersValues(const std::string &section, const std::string &name, const std::vector<int64_t> &values, bool hexadecimal, bool must_exist)
{
    std::string str;

    for (size_t i = 0; i < values.size(); i++)
    {
        bool constant_found = false;

        if (i != 0)
            str += ',';

        for (auto &c: integer_constants)
        {
            if (c.value == values[i])
            {
                str += c.name;
                constant_found = true;
                break;
            }
        }

        if (constant_found)
            continue;

        char raw[32];

        if (!hexadecimal)
            sprintf(raw, "%I64d", values[i]);
        else
            sprintf(raw, "0x%I64x", values[i]);

        str += raw;
    }

    return SetStringValue(section, name, str, true, must_exist);
}

bool IniFile::GetMultipleStringsValues(const std::string &section, const std::string &name, std::vector<std::string> &values, bool clear_on_error)
{
    std::string str;

    if (!GetStringValue(section, name, str))
    {
        if (clear_on_error)
            values.clear();

        return false;
    }

    Utils::GetMultipleStrings(str, values);
    return true;
}

bool IniFile::SetMultipleStringsValues(const std::string &section, const std::string &name, const std::vector<std::string> &values, bool must_exist)
{
    std::string str = Utils::ToSingleString(values);
    return SetStringValue(section, name, str, true, must_exist);
}

size_t IniFile::GetAllStringsValues(const std::string &section, std::vector<std::string> &names, std::vector<std::string> &values)
{
    names.clear();
    values.clear();

    for (const Line &line : lines)
    {
        if (line.is_comment || line.is_section || Utils::ToLowerCase(line.section) != Utils::ToLowerCase(section))
            continue;

        std::string left, right;

        if (!GetAssignment(line.content, left, right))
            continue;

        if (Utils::IsEmptyString(left) || Utils::IsEmptyString(right))
            continue;

        names.push_back(left);
        values.push_back("");
        ParseString(right, values.back(), section, left);
    }

    return names.size();
}

void IniFile::AddIntegerConstant(const std::string &name, int64_t value)
{
    IntegerConstant c;

    c.name = name;
    c.value = value;

    integer_constants.push_back(c);
}

bool IniFile::SectionExists(const std::string &name) const
{
    std::string lc_name = Utils::ToLowerCase(name);

    for (const Line &line : lines)
    {
        if (line.is_section && Utils::ToLowerCase(line.section) == lc_name)
            return true;
    }

    return false;
}


