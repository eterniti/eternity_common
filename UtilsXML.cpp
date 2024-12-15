#include "UtilsXML.h"
#include "debug.h"

TiXmlElement *Utils::FindRoot(TiXmlHandle *handle, const std::string &root_name)
{
    for (TiXmlElement *elem = handle->FirstChildElement().Element(); elem != nullptr; elem = elem->NextSiblingElement())
    {
        std::string name = elem->ValueStr();

        if (name == root_name)
        {
            return elem;
        }
    }

    return nullptr;
}

size_t Utils::GetElemCount(const TiXmlElement *root, const char *name, const TiXmlElement **first)
{
    size_t count = 0;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == name)
        {
            if (first && count == 0)
            {
                *first = elem;
            }

            count++;
        }
    }

    return count;
}

void Utils::WriteParamString(TiXmlElement *root, const char *name, const std::string & value)
{
    TiXmlElement *param = new TiXmlElement(name);
    param->SetAttribute("value", value);
    root->LinkEndChild(param);
}

void Utils::WriteParamMultipleStrings(TiXmlElement *root, const char *name, const std::vector<std::string> & values)
{
    TiXmlElement *param = new TiXmlElement(name);
    param->SetAttribute("value", ToSingleString(values));
    root->LinkEndChild(param);
}

void Utils::WriteParamUnsigned(TiXmlElement *root, const char *name, uint64_t value, bool hexadecimal)
{
    TiXmlElement *param = new TiXmlElement(name);
    param->SetAttribute("value", UnsignedToString(value, hexadecimal));
    root->LinkEndChild(param);
}

void Utils::WriteParamSigned(TiXmlElement *root, const char *name, int64_t value)
{
    TiXmlElement *param = new TiXmlElement(name);
    param->SetAttribute("value", SignedToString(value));
    root->LinkEndChild(param);
}

void Utils::WriteParamMultipleUnsigned(TiXmlElement *root, const char *name, const std::vector<uint32_t> &values, bool hexadecimal)
{
    TiXmlElement *param = new TiXmlElement(name);
    param->SetAttribute("value", ToSingleString(values, hexadecimal));
    root->LinkEndChild(param);
}

void Utils::WriteParamMultipleUnsigned(TiXmlElement *root, const char *name, const std::vector<uint16_t> &values, bool hexadecimal)
{
    TiXmlElement *param = new TiXmlElement(name);
    param->SetAttribute("value", ToSingleString(values, hexadecimal));
    root->LinkEndChild(param);
}

void Utils::WriteParamMultipleUnsigned(TiXmlElement *root, const char *name, const std::vector<uint8_t> &values, bool hexadecimal)
{
    TiXmlElement *param = new TiXmlElement(name);
    param->SetAttribute("value", ToSingleString(values, hexadecimal));
    root->LinkEndChild(param);
}

void Utils::WriteParamFloat(TiXmlElement *root, const char *name, float value)
{
    TiXmlElement *param = new TiXmlElement(name);
    param->SetAttribute("value", FloatToString(value));
    root->LinkEndChild(param);
}

void Utils::WriteParamMultipleFloats(TiXmlElement *root, const char *name, const std::vector<float> &values)
{
    TiXmlElement *param = new TiXmlElement(name);
    param->SetAttribute("value", ToSingleString(values));
    root->LinkEndChild(param);
}

void Utils::WriteParamGUID(TiXmlElement *root, const char *name, const uint8_t *value)
{
    std::string str = Utils::GUID2String(value);
    WriteParamString(root, name, str);
}

void Utils::WriteParamBlob(TiXmlElement *root, const char *name, const uint8_t *value, size_t size)
{
    TiXmlElement *param = new TiXmlElement(name);
    TiXmlText *base64= new TiXmlText(Base64Encode(value, size, true));

    base64->SetCDATA(true);
    param->LinkEndChild(base64);
    root->LinkEndChild(param);
}

void Utils::WriteParamBoolean(TiXmlElement *root, const char *name, bool value)
{
    TiXmlElement *param = new TiXmlElement(name);
    param->SetAttribute("value", (value) ? "true" : "false");
    root->LinkEndChild(param);
}

bool Utils::ReadAttrString(const TiXmlElement *root, const char *name, std::string & value)
{
    if (root->QueryStringAttribute(name, &value) != TIXML_SUCCESS)
        return false;

    return true;
}

bool Utils::ReadAttrMultipleStrings(const TiXmlElement *root, const char *name, std::vector<std::string> &values, char separator, bool omit_empty)
{
    std::string str;

    if (!ReadAttrString(root, name, str))
        return false;

    GetMultipleStrings(str, values, separator, omit_empty);
    return true;
}

bool Utils::ReadAttrUnsigned(const TiXmlElement *root,  const char *name, uint32_t *value)
{
    std::string str;

    if (root->QueryStringAttribute(name, &str) != TIXML_SUCCESS)
        return false;

    *value = GetUnsigned(str.c_str());
    return true;
}

bool Utils::ReadAttrUnsigned(const TiXmlElement *root,  const char *name, uint64_t *value)
{
    std::string str;

    if (root->QueryStringAttribute(name, &str) != TIXML_SUCCESS)
        return false;

    *value = GetUnsigned64(str.c_str());
    return true;
}

bool Utils::ReadAttrUnsigned(const TiXmlElement *root, const char *name, uint16_t *value)
{
    std::string str;

    if (root->QueryStringAttribute(name, &str) != TIXML_SUCCESS)
        return false;

    uint32_t temp = GetUnsigned(str.c_str());
    if (temp > 0xFFFF)
        return false;

    *value = (uint16_t)temp;
    return true;
}

bool Utils::ReadAttrUnsigned(const TiXmlElement *root, const char *name, uint8_t *value)
{
    std::string str;

    if (root->QueryStringAttribute(name, &str) != TIXML_SUCCESS)
        return false;

    uint32_t temp = GetUnsigned(str.c_str());
    if (temp > 0xFF)
        return false;

    *value = (uint8_t)temp;
    return true;
}

bool Utils::ReadAttrSigned(const TiXmlElement *root,  const char *name, int32_t *value)
{
    std::string str;

    if (!ReadAttrString(root, name, str))
        return false;

    *value = GetSigned(str.c_str());
    return true;
}

bool Utils::ReadAttrSigned(const TiXmlElement *root,  const char *name, int16_t *value)
{
    int32_t value32;

    if (!ReadAttrSigned(root, name, &value32))
        return false;

    if (value32 >= 32768 || value32 < -32768)
        return false;

    *value = (int16_t)value32;
    return true;
}

bool Utils::ReadAttrSigned(const TiXmlElement *root,  const char *name, int8_t *value)
{
    int32_t value32;

    if (!ReadAttrSigned(root, name, &value32))
        return false;

    if (value32 >= 128 || value32 < -128)
        return false;

    *value = (int8_t)value32;
    return true;
}

bool Utils::ReadAttrMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint8_t> &values)
{
    std::vector<std::string> values_str;

    values.clear();

    if (!ReadAttrMultipleStrings(root, name, values_str))
        return false;

    values.reserve(values_str.size());

    for (const std::string &s : values_str)
    {
        uint32_t value = GetUnsigned(s.c_str());

        if (value > 0xFF)
            return false;

        values.push_back(value);
    }

    return true;
}

bool Utils::ReadAttrMultipleUnsigned(const TiXmlElement *root, const char *name, uint8_t *values, size_t count)
{
    std::vector<uint8_t> vec;

    if (!ReadAttrMultipleUnsigned(root, name, vec))
        return false;

    if (vec.size() != count)
        return false;

    memcpy(values, vec.data(), count*sizeof(uint8_t));
    return true;
}

bool Utils::ReadAttrMultipleSigned(const TiXmlElement *root, const char *name, std::vector<int32_t> &values)
{
    std::vector<std::string> values_str;

    values.clear();

    if (!ReadAttrMultipleStrings(root, name, values_str))
        return false;

    values.reserve(values_str.size());

    for (const std::string &s : values_str)
    {
        values.push_back(GetSigned(s.c_str()));
    }

    return true;
}

bool Utils::ReadAttrMultipleSigned(const TiXmlElement *root, const char *name, int32_t *values, size_t count)
{
    std::vector<int32_t> vec;

    if (!ReadAttrMultipleSigned(root, name, vec))
        return false;

    if (vec.size() != count)
         return false;

    memcpy(values, vec.data(), count*sizeof(int32_t));
    return true;
}

bool Utils::ReadAttrFloat(const TiXmlElement *root, const char *name, float *value)
{
    std::string str;

    if (root->QueryFloatAttribute(name, value) != TIXML_SUCCESS)
        return false;

    return true;
}

bool Utils::ReadAttrMultipleFloats(const TiXmlElement *root, const char *name, std::vector<float> &values)
{
    std::vector<std::string> values_str;

    values.clear();

    if (!ReadAttrMultipleStrings(root, name, values_str))
        return false;

    values.reserve(values_str.size());

    for (const std::string &s : values_str)
    {
        float value;

        if (sscanf(s.c_str(), "%f", &value) != 1)
            return false;

        values.push_back(value);
    }

    return true;
}

bool Utils::ReadAttrMultipleFloats(const TiXmlElement *root, const char *name, float *values, size_t count)
{
    std::vector<float> vec;

    if (!ReadAttrMultipleFloats(root, name, vec))
        return false;

    if (vec.size() != count)
         return false;

    memcpy(values, vec.data(), count*sizeof(float));
    return true;
}

bool Utils::ReadAttrBoolean(const TiXmlElement *root, const char *name, bool *value)
{
    std::string str;
    if (!ReadAttrString(root, name, str))
        return false;

    str = Utils::ToLowerCase(str);
    if (str == "true")
        *value = true;
    else if (str == "false")
        *value = false;
    else
    {
         *value = (GetUnsigned64(str) != 0);
    }

    return true;
}

bool Utils::ReadParamString(const TiXmlElement *root, const char *name, std::string &value, const TiXmlElement **ret)
{
    bool found = false;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem != nullptr; elem = elem->NextSiblingElement())
    {
        std::string str;

        str = elem->ValueStr();

        if (str == name)
        {
            if (elem->QueryStringAttribute("value", &value) != TIXML_SUCCESS)
                return false;

            found = true;
            if (ret)
                *ret = elem;

            break;
        }
    }

    return found;
}

bool Utils::ReadParamMultipleStrings(const TiXmlElement *root, const char *name, std::vector<std::string> & values, const TiXmlElement **ret)
{
    std::string str;

    if (!ReadParamString(root, name, str, ret))
        return false;

    GetMultipleStrings(str, values);
    return true;
}

bool Utils::ReadParamUnsigned(const TiXmlElement *root, const char *name, uint32_t *value)
{
    std::string str;

    if (!ReadParamString(root, name, str))
        return false;

    *value = GetUnsigned(str.c_str());
    return true;
}

bool Utils::ReadParamUnsigned(const TiXmlElement *root, const char *name, uint64_t *value)
{
    std::string str;

    if (!ReadParamString(root, name, str))
        return false;

    *value = GetUnsigned64(str.c_str());
    return true;
}

bool Utils::ReadParamUnsigned(const TiXmlElement *root, const char *name, uint16_t *value)
{
    uint32_t temp;

    if (!ReadParamUnsigned(root, name, &temp))
        return false;

    if (temp > 0xFFFF)
        return false;

    *value = (uint16_t)temp;
    return true;
}

bool Utils::ReadParamUnsigned(const TiXmlElement *root, const char *name, uint8_t *value)
{
    uint32_t temp;

    if (!ReadParamUnsigned(root, name, &temp))
        return false;

    if (temp > 0xFF)
        return false;

    *value = (uint8_t)temp;
    return true;
}

bool Utils::ReadParamMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint32_t> &values)
{
    std::vector<std::string> values_str;

    values.clear();

    if (!ReadParamMultipleStrings(root, name, values_str))
        return false;

    values.reserve(values_str.size());

    for (const std::string &s : values_str)
    {
        values.push_back(GetUnsigned(s.c_str()));
    }

    return true;
}

bool Utils::ReadParamMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint16_t> &values)
{
    std::vector<std::string> values_str;

    values.clear();

    if (!ReadParamMultipleStrings(root, name, values_str))
        return false;

    values.reserve(values_str.size());

    for (const std::string &s : values_str)
    {
        uint32_t value = GetUnsigned(s.c_str());

        if (value > 0xFFFF)
            return false;

        values.push_back(value);
    }

    return true;
}

bool Utils::ReadParamMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint8_t> &values)
{
    std::vector<std::string> values_str;

    values.clear();

    if (!ReadParamMultipleStrings(root, name, values_str))
        return false;

    values.reserve(values_str.size());

    for (const std::string &s : values_str)
    {
        uint32_t value = GetUnsigned(s.c_str());

        if (value > 0xFF)
            return false;

        values.push_back(value);
    }

    return true;
}

bool Utils::ReadParamMultipleUnsigned(const TiXmlElement *root, const char *name, uint32_t *values, size_t count)
{
    std::vector<uint32_t> vec;

    if (!ReadParamMultipleUnsigned(root, name, vec))
        return false;

    if (vec.size() != count)
        return false;

    memcpy(values, vec.data(), count*sizeof(uint32_t));
    return true;
}

bool Utils::ReadParamMultipleUnsigned(const TiXmlElement *root, const char *name, uint16_t *values, size_t count)
{
    std::vector<uint16_t> vec;

    if (!ReadParamMultipleUnsigned(root, name, vec))
        return false;

    if (vec.size() != count)
        return false;

    memcpy(values, vec.data(), count*sizeof(uint16_t));
    return true;
}

bool Utils::ReadParamMultipleUnsigned(const TiXmlElement *root, const char *name, uint8_t *values, size_t count)
{
    std::vector<uint8_t> vec;

    if (!ReadParamMultipleUnsigned(root, name, vec))
        return false;

    if (vec.size() != count)
        return false;

    memcpy(values, vec.data(), count*sizeof(uint8_t));
    return true;
}

bool Utils::ReadParamSigned(const TiXmlElement *root, const char *name, int32_t *value)
{
    std::string str;

    if (!ReadParamString(root, name, str))
        return false;

    *value = GetSigned(str.c_str());
    return true;
}

bool Utils::ReadParamFloat(const TiXmlElement *root, const char *name, float *value)
{
    bool found = false;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
    {
        std::string str;

        str = elem->ValueStr();

        if (str == name)
        {
            if (elem->QueryFloatAttribute("value", value) != TIXML_SUCCESS)
                return false;

            found = true;
            break;
        }
    }

    return found;
}

bool Utils::ReadParamMultipleFloats(const TiXmlElement *root, const char *name, std::vector<float> &values)
{
    std::vector<std::string> values_str;

    values.clear();

    if (!ReadParamMultipleStrings(root, name, values_str))
        return false;

    values.reserve(values_str.size());

    for (const std::string &s : values_str)
    {
        float value;

        if (sscanf(s.c_str(), "%f", &value) != 1)
            return false;

        values.push_back(value);
    }

    return true;
}

bool Utils::ReadParamMultipleFloats(const TiXmlElement *root, const char *name, float *values, size_t count)
{
    std::vector<float> vec;

    if (!ReadParamMultipleFloats(root, name, vec))
        return false;

    if (vec.size() != count)
        return false;

    memcpy(values, vec.data(), count*sizeof(float));
    return true;
}

bool Utils::ReadParamGUID(const TiXmlElement *root, const char *name, uint8_t *value)
{
    std::string guid;

    if (!ReadParamString(root, name, guid))
        return false;

    TrimString(guid);

    if (guid.length() != 36)
        return false;

    bool high_flag = true;
    uint8_t h = 0;

    for (size_t i = 0, j = 0; i < guid.length(); i++)
    {
        if (i == 8 || i == 13 || i == 18 || i == 23)
        {
            if (guid[i] != '-')
                return false;
        }
        else
        {
            uint8_t d;

            if (guid[i] >= '0' && guid[i] <= '9')
            {
                d = guid[i] - '0';
            }
            else if (guid[i] >= 'a' && guid[i] <= 'f')
            {
                d = guid[i] - 'a' + 10;
            }
            else if (guid[i] >= 'A' && guid[i] <= 'F')
            {
                d = guid[i] - 'A' + 10;
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
                value[j++] = h | d;
            }

            high_flag = !high_flag;
        }
    }

    return true;
}

uint8_t *Utils::ReadParamBlob(const TiXmlElement *root, const char *name, size_t *psize)
{
    for (const TiXmlElement *elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
    {
        std::string str;

        str = elem->ValueStr();

        if (str == name)
        {
            std::string base64_data = elem->GetText();
            return Base64Decode(base64_data, psize);
        }
    }

    return nullptr;
}

bool Utils::ReadParamUnsignedWithMultipleNames(const TiXmlElement *root, uint32_t *value, const char *name1, const char *name2, const char *name3, const char *name4, const char *name5)
{
    if (ReadParamUnsigned(root, name1, value))
        return true;

    if (ReadParamUnsigned(root, name2, value))
        return true;

    if (name3 && ReadParamUnsigned(root, name3, value))
        return true;

    if (name4 && ReadParamUnsigned(root, name4, value))
        return true;

    return (name5 && ReadParamUnsigned(root, name5, value));
}

bool Utils::ReadParamUnsignedWithMultipleNames(const TiXmlElement *root, uint16_t *value, const char *name1, const char *name2, const char *name3, const char *name4, const char *name5)
{
    if (ReadParamUnsigned(root, name1, value))
        return true;

    if (ReadParamUnsigned(root, name2, value))
        return true;

    if (name3 && ReadParamUnsigned(root, name3, value))
        return true;

    if (name4 && ReadParamUnsigned(root, name4, value))
        return true;

    return (name5 && ReadParamUnsigned(root, name5, value));
}

bool Utils::ReadParamFloatWithMultipleNames(const TiXmlElement *root, float *value, const char *name1, const char *name2, const char *name3, const char *name4, const char *name5)
{
    if (ReadParamFloat(root, name1, value))
        return true;

    if (ReadParamFloat(root, name2, value))
        return true;

    if (name3 && ReadParamFloat(root, name3, value))
        return true;

    if (name4 && ReadParamFloat(root, name4, value))
        return true;

    return (name5 && ReadParamFloat(root, name5, value));
}

bool Utils::ReadParamBoolean(const TiXmlElement *root, const char *name, bool *value)
{
    std::string str;

    if (!ReadParamString(root, name, str))
        return false;

    str = Utils::ToLowerCase(str);
    if (str == "true")
        *value = true;
    else if (str == "false")
        *value = false;
    else
    {
         *value = (GetUnsigned64(str.c_str()) != 0);
    }

    return true;
}

bool Utils::GetParamString(const TiXmlElement *root, const char *name, std::string &value, const TiXmlElement **ret)
{
    if (!ReadParamString(root, name, value, ret))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamMultipleStrings(const TiXmlElement *root, const char *name, std::vector<std::string> & values, const TiXmlElement **ret)
{
    if (!ReadParamMultipleStrings(root, name, values, ret))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamUnsigned(const TiXmlElement *root, const char *name, uint32_t *value)
{
    if (!ReadParamUnsigned(root, name, value))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamUnsigned(const TiXmlElement *root, const char *name, uint64_t *value)
{
    if (!ReadParamUnsigned(root, name, value))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamUnsigned(const TiXmlElement *root, const char *name, uint16_t *value)
{
    uint32_t temp;

    if (!GetParamUnsigned(root, name, &temp))
        return false;

    if (temp > 0xFFFF)
    {
        DPRINTF("Parameter \"%s\" is out of 16 bits limit. (object at line %d)\n", name, root->Row());
        return false;
    }

    *value = (uint16_t)temp;
    return true;
}

bool Utils::GetParamUnsigned(const TiXmlElement *root, const char *name, uint8_t *value)
{
    uint32_t temp;

    if (!GetParamUnsigned(root, name, &temp))
        return false;

    if (temp > 0xFF)
    {
        DPRINTF("Parameter \"%s\" is out of 8 bits limit. (object at line %d)\n", name, root->Row());
        return false;
    }

    *value = (uint8_t)temp;
    return true;
}

bool Utils::GetParamMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint32_t> &values)
{
    if (!ReadParamMultipleUnsigned(root, name, values))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint16_t> &values)
{
    if (!ReadParamMultipleUnsigned(root, name, values))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint8_t> &values)
{
    if (!ReadParamMultipleUnsigned(root, name, values))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamMultipleUnsigned(const TiXmlElement *root, const char *name, uint32_t *values, size_t count)
{
    if (!ReadParamMultipleUnsigned(root, name, values, count))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamMultipleUnsigned(const TiXmlElement *root, const char *name, uint16_t *values, size_t count)
{
    if (!ReadParamMultipleUnsigned(root, name, values, count))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamMultipleUnsigned(const TiXmlElement *root, const char *name, uint8_t *values, size_t count)
{
    if (!ReadParamMultipleUnsigned(root, name, values, count))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamSigned(const TiXmlElement *root, const char *name, int32_t *value)
{
    if (!ReadParamSigned(root, name, value))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamFloat(const TiXmlElement *root, const char *name, float *value)
{
    if (!ReadParamFloat(root, name, value))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamMultipleFloats(const TiXmlElement *root, const char *name, std::vector<float> &values)
{
    if (!ReadParamMultipleFloats(root, name, values))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamMultipleFloats(const TiXmlElement *root, const char *name, float *values, size_t count)
{
    if (!ReadParamMultipleFloats(root, name, values, count))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamGUID(const TiXmlElement *root, const char *name, uint8_t *value)
{
    if (!ReadParamGUID(root, name, value))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

uint8_t *Utils::GetParamBlob(const TiXmlElement *root, const char *name, size_t *psize)
{
    uint8_t *ret = ReadParamBlob(root, name, psize);
    if (!ret)
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
    }

    return ret;
}

bool Utils::GetParamUnsignedWithMultipleNames(const TiXmlElement *root, uint32_t *value, const char *name1, const char *name2, const char *name3, const char *name4, const char *name5)
{
    if (!ReadParamUnsignedWithMultipleNames(root, value, name1, name2, name3, name4, name5))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name1, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamUnsignedWithMultipleNames(const TiXmlElement *root, uint16_t *value, const char *name1, const char *name2, const char *name3, const char *name4, const char *name5)
{
    if (!ReadParamUnsignedWithMultipleNames(root, value, name1, name2, name3, name4, name5))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name1, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamFloatWithMultipleNames(const TiXmlElement *root, float *value, const char *name1, const char *name2, const char *name3, const char *name4, const char *name5)
{
    if (!ReadParamFloatWithMultipleNames(root, value, name1, name2, name3, name4, name5))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name1, root->Row());
        return false;
    }

    return true;
}

void Utils::WriteComment(TiXmlElement *root, const std::string & comment)
{
    TiXmlComment *tx_comment = new TiXmlComment();

    tx_comment->SetValue(comment);
    root->LinkEndChild(tx_comment);
}
