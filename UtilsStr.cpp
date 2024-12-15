#include "UtilsStr.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
#endif

std::string Utils::UnsignedToString(uint64_t value, bool hexadecimal)
{
    char temp[16];
    std::string str;

    if (hexadecimal)
    {
        sprintf(temp, "0x%I64x", value);
    }
    else
    {
        sprintf(temp, "%I64u", value);
    }

    str = temp;
    return str;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

std::string Utils::UnsignedToHexString(uint8_t value, bool zeropad, bool prefix)
{
    char temp[16];
    std::string str;

    if (!zeropad)
    {
        if (prefix)
            sprintf(temp, "0x%x", value);
        else
            sprintf(temp, "%x", value);
    }
    else
    {
        if (prefix)
            sprintf(temp, "0x%02x", value);
        else
            sprintf(temp, "%02x", value);
    }

    str = temp;
    return str;
}

std::string Utils::UnsignedToHexString(uint16_t value, bool zeropad, bool prefix)
{
    char temp[16];
    std::string str;

    if (!zeropad)
    {
        if (prefix)
            sprintf(temp, "0x%x", value);
        else
            sprintf(temp, "%x", value);
    }
    else
    {
        if (prefix)
            sprintf(temp, "0x%04x", value);
        else
            sprintf(temp, "%04x", value);
    }

    str = temp;
    return str;
}

std::string Utils::UnsignedToHexString(uint32_t value, bool zeropad, bool prefix)
{
    char temp[16];
    std::string str;

    if (!zeropad)
    {
        if (prefix)
            sprintf(temp, "0x%x", value);
        else
            sprintf(temp, "%x", value);
    }
    else
    {
        if (prefix)
            sprintf(temp, "0x%08x", value);
        else
            sprintf(temp, "%08x", value);
    }

    str = temp;
    return str;
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"
#endif

std::string Utils::UnsignedToHexString(uint64_t value, bool zeropad, bool prefix)
{
    char temp[20];
    std::string str;

    if (!zeropad)
    {
        if (prefix)
            sprintf(temp, "0x%I64x", value);
        else
            sprintf(temp, "%I64x", value);
    }
    else
    {
        if (prefix)
            sprintf(temp, "0x%16I64x", value);
        else
            sprintf(temp, "%16I64x", value);
    }

    str = temp;
    return str;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
#endif

std::string Utils::SignedToString(int64_t value)
{
    char temp[16];
    std::string str;

    sprintf(temp, "%I64d", value);

    str = temp;
    return str;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

std::string Utils::FloatToString(float value)
{
    char temp[32];
    std::string str;

    sprintf(temp, "%.9g", value);
    str = temp;

    if (str.find('.') == std::string::npos && str.find('e') == std::string::npos)
    {
        str = str + ".0";
    }

    return str;
}

std::string Utils::Vectors2DToString(const float *vectors, size_t count)
{
    std::string str;
    const float *ptr = vectors;

    for (size_t i = 0; i < count; i++)
    {
        str += "{ ";
        str += FloatToString(*ptr++);
        str += ", ";
        str += FloatToString(*ptr++);
        str += " }";

        if (i != (count-1))
            str += ", ";
    }

    return str;
}

size_t Utils::GetVectors2DFromString(const std::string &str, std::vector<float> &vectors)
{
    size_t count = 0;
    int state = 0; // 0 -> waiting '{'; 1 -> waiting number; 2 -> in number; 3 -> waiting ',' (between '}' and '{')

    std::string number_str;
    vectors.clear();

    for (char ch : str)
    {
        if (ch <= ' ')
            continue;

        if (state == 0)
        {
            if (ch != '{')
                return (size_t)-1;

            state = 1;
        }
        else if (state == 1)
        {
            if (ch == '{' || ch == ',' || ch == '}')
                return (size_t)-1;

            number_str.clear();
            number_str.push_back(ch);
            state = 2;
        }
        else if (state == 2)
        {
            bool num_finished = false;

            if (ch == ',')
            {
                if (vectors.size() & 1)
                    return (size_t)-1;

                num_finished = true;
            }
            else if (ch == '}')
            {
                if (!(vectors.size() & 1))
                    return (size_t)-1;

                num_finished = true;
            }

            if (num_finished)
            {
                float value;

                if (sscanf(number_str.c_str(), "%f", &value) != 1)
                    return (size_t)-1;

                vectors.push_back(value);

                if (vectors.size() & 1)
                {
                    state = 1;
                }
                else
                {
                    state = 3;
                    count++;
                }
            }
            else
            {
                number_str.push_back(ch);
            }
        }
        else if (state == 3)
        {
            if (ch != ',')
                return (size_t)-1;

            state = 0;
        }
    }

    if (state != 0 && state != 3)
        return (size_t)-1;

    return count;
}

std::string Utils::Vectors3DToString(const float *vectors, size_t count)
{
    std::string str;
    const float *ptr = vectors;

    for (size_t i = 0; i < count; i++)
    {
        str += "{ ";
        str += FloatToString(*ptr++);
        str += ", ";
        str += FloatToString(*ptr++);
        str += ", ";
        str += FloatToString(*ptr++);
        str += " }";

        if (i != (count-1))
            str += ", ";
    }

    return str;
}

size_t Utils::GetVectors3DFromString(const std::string &str, std::vector<float> &vectors)
{
    size_t count = 0;
    int state = 0; // 0 -> waiting '{'; 1 -> waiting number; 2 -> in number; 3 -> waiting ',' (between '}' and '{')

    std::string number_str;
    vectors.clear();

    for (char ch : str)
    {
        if (ch <= ' ')
            continue;

        if (state == 0)
        {
            if (ch != '{')
                return (size_t)-1;

            state = 1;
        }
        else if (state == 1)
        {
            if (ch == '{' || ch == ',' || ch == '}')
                return (size_t)-1;

            number_str.clear();
            number_str.push_back(ch);
            state = 2;
        }
        else if (state == 2)
        {
            bool num_finished = false;

            if (ch == ',')
            {
                if ((vectors.size() % 3) == 2)
                    return (size_t)-1;

                num_finished = true;
            }
            else if (ch == '}')
            {
                if ((vectors.size() % 3) != 2)
                    return (size_t)-1;

                num_finished = true;
            }

            if (num_finished)
            {
                float value;

                if (sscanf(number_str.c_str(), "%f", &value) != 1)
                    return (size_t)-1;

                vectors.push_back(value);

                if ((vectors.size() % 3) != 0)
                {
                    state = 1;
                }
                else
                {
                    state = 3;
                    count++;
                }
            }
            else
            {
                number_str.push_back(ch);
            }
        }
        else if (state == 3)
        {
            if (ch != ',')
                return (size_t)-1;

            state = 0;
        }
    }

    if (state != 0 && state != 3)
        return (size_t)-1;

    return count;
}

std::string Utils::Vectors4DToString(const float *vectors, size_t count)
{
    std::string str;
    const float *ptr = vectors;

    for (size_t i = 0; i < count; i++)
    {
        str += "{ ";
        str += FloatToString(*ptr++);
        str += ", ";
        str += FloatToString(*ptr++);
        str += ", ";
        str += FloatToString(*ptr++);
        str += ", ";
        str += FloatToString(*ptr++);
        str += " }";

        if (i != (count-1))
            str += ", ";
    }

    return str;
}

size_t Utils::GetVectors4DFromString(const std::string &str, std::vector<float> &vectors)
{
    size_t count = 0;
    int state = 0; // 0 -> waiting '{'; 1 -> waiting number; 2 -> in number; 3 -> waiting ',' (between '}' and '{')

    std::string number_str;
    vectors.clear();

    for (char ch : str)
    {
        if (ch <= ' ')
            continue;

        if (state == 0)
        {
            if (ch != '{')
                return (size_t)-1;

            state = 1;
        }
        else if (state == 1)
        {
            if (ch == '{' || ch == ',' || ch == '}')
                return (size_t)-1;

            number_str.clear();
            number_str.push_back(ch);
            state = 2;
        }
        else if (state == 2)
        {
            bool num_finished = false;

            if (ch == ',')
            {
                if ((vectors.size() % 4) == 3)
                    return (size_t)-1;

                num_finished = true;
            }
            else if (ch == '}')
            {
                if ((vectors.size() % 4) != 3)
                    return (size_t)-1;

                num_finished = true;
            }

            if (num_finished)
            {
                float value;

                if (sscanf(number_str.c_str(), "%f", &value) != 1)
                    return (size_t)-1;

                vectors.push_back(value);

                if ((vectors.size() % 4) != 0)
                {
                    state = 1;
                }
                else
                {
                    state = 3;
                    count++;
                }
            }
            else
            {
                number_str.push_back(ch);
            }
        }
        else if (state == 3)
        {
            if (ch != ',')
                return (size_t)-1;

            state = 0;
        }
    }

    if (state != 0 && state != 3)
        return (size_t)-1;

    return count;
}

void Utils::TrimString(std::string & str, bool trim_left, bool trim_right)
{
    size_t pos = 0;

    if (trim_left)
    {
        if (str.length() == 0)
            return;

        for (size_t i = 0; i < str.length(); i++)
        {
            if (str[i] > ' ' || str[i] < 0)
            {
                pos = i;
                break;
            }
        }

        if (pos != 0)
        {
            str = str.substr(pos);
        }
        else if (str.front() >= 0 && str.front() <= ' ')
        {
            // String only contained empty chars
            str.clear();
            return;
        }
    }

    if (trim_right)
    {
        if (str.length() == 0)
            return;

        pos = str.length()-1;

        for (size_t i = str.length()-1; i != std::string::npos; i--)
        {
            if (str[i] > ' ' || str[i] < 0)
            {
                pos = i;
                break;
            }
        }

        if (pos != (str.length()-1))
        {
            str = str.substr(0, pos + 1);
        }
        else if (str.back() >= 0 && str.back() <= ' ')
        {
            // String only contained empty chars
            str.clear();
            return;
        }
    }
}

bool Utils::IsEmptyString(const std::string & str)
{
    for (char c : str)
    {
        if (c > ' ' || c < 0)
            return false;
    }

    return true;
}

std::string Utils::GetFileNameString(const std::string &path)
{
    size_t s1 = path.rfind('/');
    size_t s2 = path.rfind('\\');
    size_t pos = std::string::npos;

    if (s1 == std::string::npos)
    {
        if (s2 != std::string::npos)
            pos = s2;
    }
    else
    {
        if (s2 != std::string::npos)
            pos = (s1 > s2) ? s1 : s2;
        else
            pos = s1;
    }

    if (pos == std::string::npos)
        return path;

    return path.substr(pos+1);
}

std::string Utils::GetDirNameString(const std::string &path)
{
    size_t s1 = path.rfind('/');
    size_t s2 = path.rfind('\\');
    size_t pos = std::string::npos;

    if (s1 == std::string::npos)
    {
        if (s2 != std::string::npos)
            pos = s2;
    }
    else
    {
        if (s2 != std::string::npos)
            pos = (s1 > s2) ? s1 : s2;
        else
            pos = s1;
    }

    if (pos == std::string::npos)
        return path;

    return path.substr(0, pos);
}

std::string Utils::MakePathString(const std::string &comp1, const std::string &comp2)
{
    size_t len = comp1.length();

    if (len == 0)
        return NormalizePath(comp2);

    if (len > 0 && (comp1[len-1] == '/' || comp1[len-1] == '\\'))
        return NormalizePath(comp1+comp2);

    return NormalizePath(comp1 + '/' + comp2);
}

size_t Utils::GetMultipleStrings(const std::string & str_param, std::vector<std::string> & list, char separator, bool omit_empty_strings, bool do_trimr)
{
    size_t pos;
    std::string str = str_param;

    list.clear();

    if (omit_empty_strings)
    {
        if (str.length() == 0 || str == "NULL")
        {
            return 0;
        }
    }

    while ((pos = str.find(separator)) != std::string::npos)
    {
        std::string this_string = str.substr(0, pos);

        if (!IsEmptyString(this_string) || !omit_empty_strings)
        {
            if (do_trimr)
                Utils::TrimString(this_string);
            else
                Utils::TrimString(this_string, true, false);

            if (this_string == "NULL")
                this_string.clear();

            list.push_back(this_string);
        }

        str = str.substr(pos+1);
    }

    // Add the last string,
    if (!IsEmptyString(str) || !omit_empty_strings)
    {
        Utils::TrimString(str);

        if (str == "NULL")
            str.clear();

        list.push_back(str);
    }

    return list.size();
}

size_t Utils::GetMultipleStrings(const std::string & str_param, std::vector<std::string> & list, const std::string &separator, bool omit_empty_strings)
{
    size_t pos;
    std::string str = str_param;

    list.clear();

    if (omit_empty_strings)
    {
        if (str.length() == 0 || str == "NULL")
        {
            return 0;
        }
    }

    while ((pos = str.find(separator)) != std::string::npos)
    {
        std::string this_string = str.substr(0, pos);

        if (!IsEmptyString(this_string) || !omit_empty_strings)
        {
            Utils::TrimString(this_string);

            if (this_string == "NULL")
                this_string.clear();

            list.push_back(this_string);
        }

        str = str.substr(pos+separator.length());
    }

    // Add the last string,
    if (!IsEmptyString(str) || !omit_empty_strings)
    {
        Utils::TrimString(str);

        if (str == "NULL" && omit_empty_strings)
            str.clear();

        list.push_back(str);
    }

    return list.size();
}

size_t Utils::GetMultipleStringsSpaces(const std::string &str_param, std::vector<std::string> &list)
{
    std::string str = str_param;
    list.clear();

    bool in_string = false;
    std::string cur;

    for (const char ch : str)
    {
        if (ch <= ' ')
        {
            if (in_string)
            {
                in_string = false;
                list.push_back(cur);
                cur.clear();
            }

        }
        else
        {
            in_string = true;
            cur.push_back(ch);
        }
    }

    list.push_back(cur);
    return list.size();
}

std::string Utils::ToSingleString(const std::vector<std::string> &list, const std::string &separator, bool omit_empty_strings)
{
    std::string ret;

    for (size_t i = 0; i < list.size(); i++)
    {
        const std::string &s = list[i];

        if (i != 0)
            ret += separator;

        if (s.length() != 0 || !omit_empty_strings)
        {
            ret += s;
        }
        else
        {
            ret += "NULL";
        }
    }

    return ret;
}

std::string Utils::ToSingleString(const std::vector<uint32_t> &list, bool hexadecimal)
{
    std::string ret;

    for (uint32_t u : list)
    {
        if (ret.length() != 0)
            ret += ", ";

        ret += UnsignedToString(u, hexadecimal);
    }

    return ret;
}

std::string Utils::ToSingleString(const std::vector<uint16_t> &list, bool hexadecimal)
{
    std::string ret;

    for (uint16_t u : list)
    {
        if (ret.length() != 0)
            ret += ", ";

        ret += UnsignedToString(u, hexadecimal);
    }

    return ret;
}

std::string Utils::ToSingleString(const std::vector<uint8_t> &list, bool hexadecimal)
{
    std::string ret;

    for (uint8_t u : list)
    {
        if (ret.length() != 0)
            ret += ", ";

        ret += UnsignedToString(u, hexadecimal);
    }

    return ret;
}

std::string Utils::ToSingleString(const std::vector<float> &list)
{
    std::string ret;

    for (float f: list)
    {
        if (ret.length() != 0)
            ret += ", ";

        ret += FloatToString(f);
    }

    return ret;
}

std::string Utils::ToLowerCase(const std::string & str)
{
    std::string ret = str;

    for (char &c : ret)
    {
        if (c >= 'A' && c <= 'Z')
        {
            c = c + ('a' - 'A');
        }
    }

    return ret;
}

std::string Utils::ToUpperCase(const std::string & str)
{
    std::string ret = str;

    for (char &c : ret)
    {
        if (c >= 'a' && c <= 'z')
        {
            c = c - ('a' - 'A');
        }
    }

    return ret;
}

bool Utils::IsAlphaNumeric(const std::string &str)
{
    for (char c: str)
    {
        bool ok = false;

        if (c >= 'A' && c <= 'Z')
        {
            ok = true;
        }
        else if (c >= 'a' && c <= 'z')
        {
            ok = true;
        }
        else if (c >= '0' && c <= '9')
        {
            ok = true;
        }

        if (!ok)
            return false;
    }

    return true;
}

bool Utils::HasOnlyDigits(const std::string &str)
{
    for (char c: str)
    {
        bool ok = false;

        if (c >= '0' && c <= '9')
        {
            ok = true;
        }

        if (!ok)
            return false;
    }

    return true;
}

bool Utils::BeginsWith(const std::string &str, const std::string &substr, bool case_sensitive)
{
    size_t len1 = str.length();
    size_t len2 = substr.length();

    if (len2 > len1)
        return false;

    if (case_sensitive)
        return (str.substr(0, len2) == substr);

    std::string lstr = ToLowerCase(str);
    std::string lsubstr = ToLowerCase(substr);

    return (lstr.substr(0, len2) == lsubstr);
}

bool Utils::EndsWith(const std::string &str, const std::string &substr, bool case_sensitive)
{
    size_t len1 = str.length();
    size_t len2 = substr.length();
    size_t pos;

    if (len2 > len1)
        return false;

    if (case_sensitive)
    {
        pos = str.rfind(substr);
    }
    else
    {
        std::string lstr = ToLowerCase(str);
        std::string lsubstr = ToLowerCase(substr);

        pos = lstr.rfind(lsubstr);
    }

    return (pos == (len1-len2));
}

size_t Utils::CharCount(const std::string &str, char ch)
{
    size_t ret = 0;

    for (const char c : str)
    {
        if (c == ch)
            ret++;
    }

    return ret;
}

std::string Utils::GUID2String(const uint8_t *guid)
{
    std::string ret;

    for (int i = 0; i < 16; i++)
    {
        uint8_t h, l;
        char ch;

        h = guid[i] >> 4;
        l = guid[i] & 0xF;

        if (h <= 9)
            ch = h + '0';
        else
            ch = (h-10) + 'a';

        ret += ch;

        if (l <= 9)
            ch = l + '0';
        else
            ch = (l-10) + 'a';

        ret += ch;

        if (i == 3 || i == 5 || i == 7 || i == 9)
            ret += '-';
    }

    return ret;
}

bool Utils::String2GUID(uint8_t *guid, const std::string &str)
{
    if (str.length() != 36)
        return false;

    bool high_flag = true;
    uint8_t h = 0;

    for (size_t i = 0, j = 0; i < str.length(); i++)
    {
        if (i == 8 || i == 13 || i == 18 || i == 23)
        {
            if (str[i] != '-')
                return false;
        }
        else
        {
            uint8_t d;

            if (str[i] >= '0' && str[i] <= '9')
            {
                d = str[i] - '0';
            }
            else if (str[i] >= 'a' && str[i] <= 'f')
            {
                d = str[i] - 'a' + 10;
            }
            else if (str[i] >= 'A' && str[i] <= 'F')
            {
                d = str[i] - 'A' + 10;
            }
            else
            {
                return false;
            }

            if (high_flag)
            {
                h = d << 4;
            }
            else
            {
                guid[j++] = h | d;
            }

            high_flag = !high_flag;
        }
    }

    return true;
}

std::string Utils::ColorToString(uint32_t color, bool alpha)
{
    char str[9];

    if (alpha)
    {
        snprintf(str, sizeof(str), "#%08X", color);
    }
    else
    {
        snprintf(str, sizeof(str), "#%06X", color&0xFFFFFF);
    }

    return std::string(str);
}

std::string Utils::BinaryString(const uint8_t *buf, size_t size, bool separate, bool prefix)
{
    std::string ret;

    for (size_t i = 0; i < size; i++)
    {
        char tmp[4];

        snprintf(tmp, sizeof(tmp), "%02X", buf[i]);

        if (separate && i != 0)
            ret += " ";

        if (prefix)
            ret += "0x";

        ret += tmp;
    }

    return ret;
}

bool Utils::BinaryStringToBuf(const std::string &str, uint8_t *buf, size_t size)
{
    size_t pos = 0;
    bool high = true;

    uint8_t hnibble;

    for (size_t i = 0; i < str.length(); i++)
    {
        char ch = tolower(str[i]);
        uint8_t out;

        if (ch <= ' ')
            continue;

        if (ch == '0')
        {
            if (i != (str.length()-1) && str[i+1] == 'x')
            {
                i++;
                continue;
            }
        }

        if (ch >= '0' && ch <= '9')
        {
            out = ch - '0';
        }
        else if (ch >= 'a' && ch <= 'f')
        {
            out = (ch - 'a') + 0xa;
        }
        else
        {
            return false;
        }

        if (high)
        {
            hnibble = out;
        }
        else
        {
            if (pos >= size)
                return false;

            buf[pos++] = (hnibble << 4) | out;
        }
    }

    return (pos == size);
}

std::string Utils::NormalizePath(const std::string &path)
{
    std::string new_path = path;

    for (char &c : new_path)
    {
        if (c == '\\')
            c = '/';
    }

    bool last_was_slash = false;

    for (size_t i = 0; i < new_path.length()-1; i++)
    {
        char ch = new_path[i];

        if (ch == '/')
        {
            if (last_was_slash)
            {
                new_path.erase(i, 1);
                i--;
            }
            else
            {
                last_was_slash = true;
            }
        }
        else
        {
            last_was_slash = false;
        }
    }

    return new_path;
}

std::u16string Utils::NormalizePath(const std::u16string &path)
{
    std::u16string new_path = path;

    for (char16_t &c : new_path)
    {
        if (c == '\\')
            c = '/';
    }

    bool last_was_slash = false;

    for (size_t i = 0; i < new_path.length()-1; i++)
    {
        char16_t ch = new_path[i];

        if (ch == '/')
        {
            if (last_was_slash)
            {
                new_path.erase(i, 1);
                i--;
            }
            else
            {
                last_was_slash = true;
            }
        }
        else
        {
            last_was_slash = false;
        }
    }

    return new_path;
}

std::string Utils::WindowsPath(const std::string &path)
{
    std::string new_path = path;

    for (char &c : new_path)
    {
        if (c == '/')
            c = '\\';
    }

    return new_path;
}

std::u16string Utils::WindowsPath(const std::u16string &path)
{
    std::u16string new_path = path;

    for (char16_t &c : new_path)
    {
        if (c == '/')
            c = '\\';
    }

    return new_path;
}

std::string Utils::SamePath(const std::string &file_path, const std::string &file_name)
{
    std::string fp = NormalizePath(file_path);
    size_t rs = fp.rfind('/');

    if (rs == std::string::npos)
        return file_name;

    return fp.substr(0, rs+1) + file_name;
}

std::u16string Utils::SamePath(const std::u16string &file_path, const std::u16string &file_name)
{
    std::u16string fp = NormalizePath(file_path);
    size_t rs = fp.rfind('/');

    if (rs == std::string::npos)
        return file_name;

    return fp.substr(0, rs+1) + file_name;
}

std::string Utils::RandomString(size_t len)
{
    std::string str;
    str.resize(len);

    for (char &c : str)
    {
        if (RandomInt(0, 0x10000) & 1)
        {
            c = (char)Utils::RandomInt('A', 'Z');
        }
        else
        {
            c = (char)Utils::RandomInt('a', 'z');
        }
    }

    return str;
}

size_t Utils::Replace(std::string &str, const std::string &from, const std::string &to)
{
    size_t start_pos = 0;
    size_t num = 0;

    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
        num++;
    }

    return num;
}

size_t FORMAT_PRINTF3 Utils::Sprintf(std::string &buf, bool append, const char *fmt, ...)
{
    size_t max_size = strlen(fmt) + 1024;
    char *cbuf = new char[max_size];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(cbuf, max_size, fmt, ap);
    va_end(ap);

    size_t ret;

    if (append)
    {
        buf = buf + std::string(cbuf);
        ret = strlen(cbuf);
    }
    else
    {
        buf = std::string(cbuf);
        ret = buf.length();
    }

    delete[] cbuf;
    return ret;
}
