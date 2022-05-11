#include "TsrFile.h"
#include "debug.h"

static uint8_t xor_table[1024] =
{
    0xE4, 0x6B, 0x8E, 0x6D, 0xA5, 0x70, 0xE5, 0x6C, 0xD5, 0x98, 0x83, 0x92, 0xA3, 0xA7, 0x2A, 0x67,
    0x0D, 0xF5, 0xDD, 0xAF, 0x50, 0x18, 0xF4, 0x34, 0xA3, 0xBC, 0x70, 0xC5, 0x25, 0xD5, 0x8B, 0x7F,
    0x13, 0xF9, 0x93, 0x4C, 0x3E, 0x08, 0xF9, 0xA8, 0xBC, 0x0E, 0xA5, 0xAB, 0xDE, 0x69, 0x8D, 0x44,
    0x63, 0x2E, 0x86, 0xC2, 0xEB, 0xC8, 0x5A, 0x9A, 0xAE, 0xE3, 0x1B, 0x64, 0x6E, 0x8A, 0xB5, 0x26,
    0xAA, 0x8C, 0xCA, 0xD5, 0x15, 0x25, 0xBB, 0x19, 0x47, 0xD8, 0x03, 0x51, 0xB1, 0xA2, 0xC9, 0xD7,
    0xD6, 0x4E, 0xB5, 0x85, 0xB8, 0x24, 0x03, 0x78, 0x91, 0xC5, 0xD4, 0x14, 0xC4, 0x5B, 0xCC, 0x48,
    0x14, 0xEC, 0xDA, 0x13, 0x13, 0x0F, 0x54, 0x46, 0xDB, 0xC3, 0x44, 0x8C, 0x05, 0x9B, 0x06, 0xA8,
    0xCE, 0x20, 0x12, 0xFE, 0xA0, 0x70, 0x15, 0x56, 0xB3, 0x2B, 0x47, 0xDC, 0x0F, 0x8F, 0xF8, 0x6B,
    0xB5, 0xE1, 0x6D, 0x0B, 0x1E, 0x10, 0xEA, 0xB7, 0xE4, 0x94, 0x11, 0x65, 0xBF, 0x9D, 0x6A, 0x40,
    0xB4, 0x6A, 0x43, 0x38, 0x89, 0xF6, 0xBA, 0xBC, 0x7C, 0xDA, 0x18, 0xC6, 0x35, 0x6F, 0x61, 0x19,
    0xF8, 0x33, 0x29, 0xC6, 0x1F, 0x6E, 0xA8, 0xF5, 0xC7, 0x15, 0x11, 0xE2, 0xCA, 0xED, 0x20, 0x26,
    0xEE, 0xF4, 0xF2, 0x38, 0x5D, 0xFE, 0x1A, 0x33, 0x54, 0x9D, 0xF1, 0xD9, 0x1F, 0x43, 0x2E, 0xD8,
    0x44, 0xA9, 0xB5, 0x4E, 0xFE, 0x72, 0xB4, 0x87, 0xEE, 0x0C, 0xEC, 0x0E, 0x0E, 0xD6, 0x4E, 0xE2,
    0xE6, 0x89, 0xC7, 0x08, 0x02, 0xD0, 0x5D, 0x43, 0xA3, 0x3A, 0x7A, 0x1F, 0xB5, 0x53, 0x87, 0x34,
    0x02, 0x0D, 0xBC, 0xA8, 0xA4, 0x64, 0x37, 0xF5, 0xC0, 0x41, 0x4C, 0xED, 0x71, 0x9F, 0x1D, 0xFC,
    0x03, 0xED, 0x6A, 0xAE, 0x62, 0xB4, 0xA9, 0x72, 0xD1, 0x7A, 0x5A, 0x9C, 0xDF, 0xE6, 0x95, 0xB0,
    0x97, 0x25, 0xE4, 0xDD, 0xF7, 0x8B, 0x59, 0xC8, 0xA4, 0x7E, 0xD7, 0x8B, 0xDC, 0x90, 0xB4, 0xFD,
    0xAC, 0xEA, 0x82, 0x35, 0x63, 0xF1, 0x29, 0x4A, 0x47, 0x25, 0x3A, 0x5C, 0x86, 0x46, 0x7F, 0xD7,
    0x6F, 0xB9, 0xD6, 0xF6, 0xE1, 0x31, 0x41, 0x87, 0x05, 0x89, 0x36, 0xEE, 0x38, 0xEF, 0x3C, 0x6D,
    0x4B, 0x49, 0xB7, 0xA2, 0xEE, 0xD1, 0x04, 0x52, 0x6B, 0x03, 0xC0, 0x65, 0x90, 0xB8, 0x6E, 0x31,
    0xEE, 0x93, 0x3A, 0xFA, 0x49, 0x9C, 0x17, 0xBA, 0x47, 0x2B, 0x10, 0x1F, 0x6B, 0x08, 0xDB, 0xD2,
    0x47, 0xD0, 0xB2, 0x00, 0xEC, 0x9B, 0x60, 0x12, 0xA6, 0xDB, 0x97, 0xBE, 0xE6, 0x87, 0x8A, 0x45,
    0x80, 0x7A, 0xB7, 0xF2, 0x17, 0x16, 0x04, 0xE9, 0xD4, 0x2C, 0x0E, 0x25, 0x5F, 0x20, 0xBC, 0xB6,
    0x08, 0x48, 0x1C, 0x54, 0x45, 0x96, 0x67, 0x13, 0x60, 0x77, 0x66, 0x72, 0x71, 0xFA, 0xF9, 0x9A,
    0x8A, 0x35, 0xF5, 0xE5, 0x33, 0xE4, 0x30, 0x9E, 0x16, 0x54, 0xD6, 0x08, 0xFA, 0x80, 0x07, 0xA1,
    0xF5, 0x79, 0x9A, 0xA7, 0xDE, 0x0B, 0x41, 0xDB, 0x02, 0x9D, 0xD4, 0x86, 0x18, 0x59, 0xE6, 0xBA,
    0x76, 0x8D, 0x9D, 0xDB, 0x84, 0x51, 0xC1, 0x5E, 0x72, 0x6B, 0x14, 0xCF, 0x26, 0x6F, 0xE0, 0x1A,
    0x79, 0x2B, 0xD5, 0x03, 0xA2, 0x42, 0x16, 0xF5, 0xF2, 0x16, 0x89, 0x05, 0xC2, 0xEB, 0x78, 0x2E,
    0xAA, 0x4A, 0x57, 0xDC, 0xF4, 0xA4, 0xE2, 0xB2, 0x51, 0x38, 0x6C, 0x85, 0xCA, 0x37, 0x73, 0xA8,
    0xF8, 0x25, 0x77, 0x6C, 0x78, 0x82, 0x0D, 0xE6, 0x9A, 0xA9, 0x2F, 0xF3, 0x59, 0xF9, 0xD5, 0x7A,
    0x90, 0x34, 0xCA, 0xF1, 0x6B, 0x25, 0xB9, 0x24, 0x1B, 0x84, 0x87, 0x31, 0xCD, 0x1E, 0xE5, 0xD5,
    0xDD, 0x30, 0x26, 0xED, 0x48, 0x15, 0x4E, 0x39, 0x61, 0x21, 0x6B, 0x5C, 0xC4, 0xCB, 0x28, 0x2A,
    0x8F, 0x13, 0x9F, 0x22, 0xCE, 0x1B, 0x6F, 0x39, 0x38, 0x18, 0x0F, 0xD8, 0x1A, 0x6C, 0x62, 0x29,
    0x90, 0x14, 0x8B, 0x8E, 0xFA, 0x41, 0x02, 0x73, 0xAE, 0x42, 0xE7, 0x46, 0xEB, 0xA9, 0x97, 0xC3,
    0x0F, 0xAD, 0x7E, 0x75, 0x09, 0xCE, 0x2C, 0x7A, 0x11, 0xB9, 0xAA, 0x86, 0x95, 0x6B, 0x0E, 0x2B,
    0x77, 0x99, 0x4D, 0x57, 0x77, 0x4E, 0x51, 0x1E, 0xEB, 0xD6, 0x4B, 0xB9, 0xB6, 0xDB, 0x4B, 0xCF,
    0x77, 0xCE, 0x0E, 0xF3, 0x01, 0x88, 0x17, 0x6F, 0x0C, 0x32, 0x01, 0x41, 0x2A, 0x62, 0x13, 0x63,
    0xFB, 0x86, 0x15, 0x4E, 0xA5, 0x86, 0x62, 0xC0, 0x7F, 0xA5, 0x3F, 0xBD, 0x0D, 0xA8, 0x6B, 0xD5,
    0x31, 0x3B, 0xF6, 0xA6, 0x9F, 0x90, 0x58, 0xA1, 0x92, 0x49, 0xBA, 0x11, 0xBD, 0x98, 0x98, 0x59,
    0x85, 0xA5, 0x8A, 0x7D, 0x6D, 0x30, 0x5E, 0xE3, 0xD1, 0x77, 0x69, 0x5C, 0xD8, 0x5A, 0x20, 0x5E,
    0xA4, 0xBC, 0xE2, 0x93, 0xCB, 0x2E, 0x18, 0x98, 0x0C, 0xC7, 0x80, 0xFE, 0x3A, 0x56, 0xC6, 0x95,
    0x7B, 0xBC, 0x55, 0xEA, 0xB7, 0x93, 0x6C, 0x10, 0x4C, 0x14, 0x74, 0x9B, 0x00, 0x37, 0x90, 0xF0,
    0x38, 0x1C, 0x77, 0xC4, 0x6E, 0xA9, 0x7F, 0xDA, 0xDF, 0x74, 0xF9, 0x12, 0x86, 0xE4, 0xC4, 0xA0,
    0x47, 0x94, 0x1D, 0xA0, 0x6C, 0xF8, 0xB5, 0xCB, 0x55, 0x43, 0x06, 0x83, 0x6A, 0x88, 0xE6, 0x16,
    0x55, 0x1F, 0x5D, 0x40, 0x6E, 0x4A, 0xB3, 0xF1, 0x77, 0x19, 0xCE, 0x51, 0x8A, 0x8A, 0xBB, 0x02,
    0x4F, 0xF4, 0x8B, 0xA5, 0x72, 0xA7, 0x60, 0x9F, 0x55, 0xCD, 0xC7, 0x1D, 0x02, 0x94, 0x49, 0x56,
    0x63, 0x8F, 0x3D, 0x10, 0xB5, 0x59, 0xDF, 0x65, 0x3A, 0x7B, 0xA6, 0xC6, 0x2F, 0x90, 0xD3, 0x42,
    0xFC, 0xA6, 0x47, 0x01, 0xB3, 0xE8, 0x96, 0x14, 0xB4, 0x7A, 0x61, 0x70, 0xAD, 0xA5, 0xDF, 0x37,
    0xCA, 0x34, 0xBD, 0x3A, 0x2A, 0x1E, 0x2A, 0xBC, 0x8F, 0x65, 0x2B, 0x79, 0x5B, 0x3D, 0x34, 0xE7,
    0xB7, 0x71, 0xF7, 0xBC, 0x16, 0x03, 0x7F, 0xB0, 0xDA, 0x13, 0x79, 0x84, 0x54, 0x02, 0xD3, 0x44,
    0xF2, 0xD6, 0x89, 0xC8, 0xB4, 0xE0, 0xBA, 0x80, 0xE0, 0x9D, 0x02, 0x71, 0xF7, 0xDA, 0x04, 0x7C,
    0xE7, 0x1D, 0x46, 0xDE, 0x82, 0x3F, 0x42, 0xFD, 0x30, 0x5D, 0xB9, 0x62, 0xDF, 0xF1, 0x4B, 0x02,
    0x44, 0x3D, 0x45, 0xC1, 0x3D, 0xE8, 0xBA, 0x39, 0x94, 0xEC, 0xD5, 0xB7, 0xEB, 0xAF, 0x6D, 0x86,
    0xF4, 0x70, 0xDA, 0x71, 0xE1, 0xE5, 0x08, 0x83, 0x1C, 0x23, 0xC9, 0x11, 0x38, 0xBD, 0x70, 0xFA,
    0x26, 0x30, 0x9B, 0x2E, 0xAB, 0x7F, 0x50, 0x6D, 0x13, 0x1A, 0x4C, 0x52, 0x20, 0x05, 0x97, 0x8F,
    0x46, 0x36, 0x5C, 0x7A, 0x1A, 0x3E, 0xF7, 0xC8, 0x07, 0x2B, 0x51, 0x99, 0x43, 0xAD, 0x69, 0xB5,
    0x02, 0x79, 0x33, 0x16, 0xE8, 0xEA, 0xA4, 0xA6, 0xC4, 0xEE, 0x0E, 0x4A, 0x7D, 0x21, 0xA9, 0x1F,
    0x45, 0x33, 0x73, 0x03, 0x15, 0x8F, 0x3B, 0x57, 0x5A, 0x3E, 0xF7, 0x03, 0xEA, 0x09, 0x5F, 0xBB,
    0x3E, 0xDD, 0xB3, 0x81, 0xDB, 0x75, 0xDF, 0x6B, 0x12, 0x33, 0xC3, 0xA7, 0xE9, 0x4E, 0xCC, 0xBC,
    0x59, 0x32, 0xC8, 0x13, 0xB9, 0x23, 0xF8, 0xB5, 0x7B, 0x25, 0x66, 0x56, 0x17, 0x19, 0x79, 0x92,
    0x44, 0x28, 0xC6, 0x77, 0x6C, 0x64, 0x2A, 0x45, 0x62, 0xAD, 0x15, 0x71, 0x4E, 0xD2, 0x29, 0xEF,
    0xEA, 0xF8, 0x03, 0xB1, 0xF0, 0x41, 0x58, 0x6C, 0xD3, 0xA5, 0x45, 0x9A, 0xAE, 0x24, 0xDF, 0xC4,
    0x7A, 0x1F, 0x13, 0x02, 0x83, 0x02, 0xA9, 0xBA, 0x1D, 0x26, 0xAA, 0xB1, 0x93, 0xF6, 0xE4, 0x42,
    0x60, 0x51, 0xCA, 0xE8, 0xA2, 0x30, 0x83, 0x03, 0xCB, 0x89, 0x3A, 0xD7, 0x9A, 0x73, 0xBB, 0xD8
};

static const std::vector<std::string> arg_types =
{
    "",
    "int",
    "float",
    "string",
    "tag",
    "variable",
    "expression",
};

static bool DecodeString(FixedMemoryStream &stream, std::string &string, size_t &enc_pos)
{
    uint32_t size;
    uint8_t *buf;

    if (!stream.Read32(&size))
        return false;

    if (size > 0x10000)
    {
        DPRINTF("%s: string size seems too big. Returning false here.\n", FUNCNAME);
        return false;
    }

    string.clear();
    string.reserve(size-1);

    if (!stream.FastRead(&buf, size))
        return false;

    if (enc_pos >= sizeof(xor_table))
        enc_pos = 0;

    for (size_t i = 0; i < size; i++)
    {
        uint8_t b = buf[i] ^ xor_table[enc_pos];

        enc_pos++;
        if (enc_pos == sizeof(xor_table))
            enc_pos = 0;

        if (b == 0 && i == (size-1))
            break;

        string.push_back((char)b);
    }

    return true;
}

static bool EncodeString(FixedMemoryStream &stream, const std::string &string, size_t &enc_pos)
{
    uint32_t size = (uint32_t)string.length()+1;

    if (!stream.Write32(size))
        return false;

    if (enc_pos >= sizeof(xor_table))
        enc_pos = 0;

    for (uint32_t i = 0; i < size; i++)
    {
        uint8_t b = (i == (size-1)) ? 0 : (uint8_t)string[i];
        b ^= xor_table[enc_pos];

        enc_pos++;
        if (enc_pos == sizeof(xor_table))
            enc_pos = 0;

        if (!stream.Write8(b))
            return false;
    }

    return true;
}

bool TsrTag::Load(FixedMemoryStream &stream, size_t &enc_pos)
{
    if (!stream.Read16(&index))
        return false;

    if (!DecodeString(stream, name, enc_pos))
        return false;

    //DPRINTF("Tag: \"%s\"  index:%d\n", name.c_str(), index);
    return true;
}

size_t TsrTag::CalculateSize() const
{
    return sizeof(uint16_t) + sizeof(uint32_t) + name.length() + 1;
}

bool TsrTag::Save(FixedMemoryStream &stream, size_t &enc_pos) const
{
    if (!stream.Write16(index))
        return false;

    if (!EncodeString(stream, name, enc_pos))
        return false;

    return true;
}

TiXmlElement *TsrTag::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Tag");

    entry_root->SetAttribute("name", name);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool TsrTag::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrString(root, "name", name))
        return false;    

    return true;
}

bool TsrSentenceArg::Load(FixedMemoryStream &stream, size_t &enc_pos)
{
    if (!stream.Read16(&type))
        return false;

    if (type == 1)
    {
       if (!stream.Read32(&uint32_value))
           return false;
    }
    else if (type == 2)
    {
        if (!stream.Read(&float_value, sizeof(float)))
            return false;
    }
    else if (type >= 3 && type <= 6)
    {
        if (!DecodeString(stream, string_value, enc_pos))
            return false;
    }
    else
    {
        DPRINTF("%s: Unknown type 0x%x\n", FUNCNAME, type);
        return false;
    }

    /*if (type == 1)
    {
        DPRINTF("Arg: %d\n", uint32_value);
    }
    else if (type == 2)
    {
        DPRINTF("Arg: %f\n", float_value);
    }
    else if (type >= 3 && type <= 6)
    {
        DPRINTF("Arg: %s\n", string_value.c_str());
    }*/

    return true;
}

size_t TsrSentenceArg::CalculateSize() const
{
    size_t size = sizeof(uint16_t);

    if (type == 1 || type == 2)
    {
        size += sizeof(uint32_t);
    }
    else if (type >= 3 && type <= 6)
    {
        size += sizeof(uint32_t) + string_value.length() + 1;
    }
    else
    {
        DPRINTF("%s: Uggh, shouldn't be here.\n", FUNCNAME);
        exit(-1);
    }

    return size;
}

bool TsrSentenceArg::Save(FixedMemoryStream &stream, size_t &enc_pos) const
{
    if (!stream.Write16(type))
        return false;

    if (type == 1)
    {
       if (!stream.Write32(uint32_value))
           return false;
    }
    else if (type == 2)
    {
        if (!stream.Write(&float_value, sizeof(float)))
            return false;
    }
    else if (type >= 3 && type <= 6)
    {
        if (!EncodeString(stream, string_value, enc_pos))
            return false;
    }
    else
    {
        DPRINTF("%s: Shouldn't be here\n", FUNCNAME);
        return false;
    }

    return true;
}

TiXmlElement *TsrSentenceArg::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Argument");

    //Utils::WriteComment(entry_root, "1 -> integer; 2 -> float; 3-6: string");

    if (type >= ARG_TYPE_INTEGER && type < ARG_TYPE_MAX)
        entry_root->SetAttribute("type", arg_types[type]);
    else
        entry_root->SetAttribute("type", type);

    if (type == 1)
    {
        entry_root->SetAttribute("value", uint32_value);
    }
    else if (type == 2)
    {
        entry_root->SetAttribute("value", Utils::FloatToString(float_value));
    }
    else
    {
        entry_root->SetAttribute("value", string_value);
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool TsrSentenceArg::Compile(const TiXmlElement *root)
{
    std::string type_str;

    if (!Utils::ReadAttrString(root, "type", type_str))
        return false;

    bool found = false;
    type_str = Utils::ToLowerCase(type_str);

    for (int i = ARG_TYPE_INTEGER; i < ARG_TYPE_MAX; i++)
    {
        if (type_str == arg_types[i])
        {
            found = true;
            type = i;
            break;
        }
    }

    if (!found)
        type = Utils::GetSigned(type_str);

    if (type == 1)
    {
        if (!Utils::ReadAttrUnsigned(root, "value", &uint32_value))
            return false;
    }
    else if (type == 2)
    {
        if (!Utils::ReadAttrFloat(root, "value", &float_value))
            return false;
    }
    else if (type >= 3 && type <= 6)
    {
        if (!Utils::ReadAttrString(root, "value", string_value))
            return false;
    }
    else
    {
        DPRINTF("%s: Unrecognized type %d\n", FUNCNAME, type);
        return false;
    }

    return true;
}

bool TsrSentence::Load(FixedMemoryStream &stream, size_t &enc_pos)
{
    if (!DecodeString(stream, name, enc_pos))
        return false;

    //DPRINTF("Sentence: %s\n", name.c_str());

    uint16_t num_args;

    if (!stream.Read16(&unk))
        return false;   

    if (!stream.Read16(&num_args))
        return false;

    args.resize(num_args);

    for (uint16_t i = 0; i < num_args; i++)
    {
        TsrSentenceArg &arg = args[i];

        if (!arg.Load(stream, enc_pos))
        {
            DPRINTF("%s: Failed to load sentence arg %d.\n", FUNCNAME, i);
            return false;
        }
    }

    return true;
}

size_t TsrSentence::CalculateSize() const
{
    size_t size = sizeof(uint32_t) + name.length() + 1;
    size += 2*sizeof(uint16_t);

    for (const TsrSentenceArg &arg : args)
    {
        size += arg.CalculateSize();
    }

    return size;
}

bool TsrSentence::Save(FixedMemoryStream &stream, size_t &enc_pos) const
{
    if (!EncodeString(stream, name, enc_pos))
        return false;

    if (!stream.Write16(unk))
        return false;

    if (!stream.Write16((uint16_t)args.size()))
        return false;

    for (const TsrSentenceArg &arg : args)
        if (!arg.Save(stream, enc_pos))
            return false;

    return true;
}

TiXmlElement *TsrSentence::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Sentence");

    entry_root->SetAttribute("value", name);
    entry_root->SetAttribute("unk", unk);

    for (const TsrSentenceArg &arg : args)
        arg.Decompile(entry_root);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool TsrSentence::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrString(root, "value", name))
        return false;

    if (!Utils::ReadAttrUnsigned(root, "unk", &unk))
        return false;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Argument")
        {
           TsrSentenceArg arg;

           if (!arg.Compile(elem))
               return false;

           args.push_back(arg);
        }
    }

    return true;
}

bool TsrFunction::Load(FixedMemoryStream &stream)
{
    uint32_t header;

    if (!stream.Read32(&header))
        return false;

    if (memcmp(&header, xor_table, sizeof(header)) != 0)
    {
        DPRINTF("%s: Failed in header check.\n", FUNCNAME);
        return false;
    }

    size_t enc_pos = 4;

    if (!DecodeString(stream, name, enc_pos))
        return false;

    //DPRINTF("Function name: %s\n", name.c_str());

    uint16_t num_tags;

    if (!stream.Read16(&unk))
        return false;

    if (!stream.Read16(&num_tags))
        return false;

    tags.resize(num_tags);

    for (uint16_t i = 0; i < num_tags; i++)
    {
        TsrTag &tag = tags[i];

        if (!tag.Load(stream, enc_pos))
        {
            DPRINTF("%s: Failed to load tag.\n", FUNCNAME);
            return false;
        }
    }

    uint16_t num_sentences;

    if (!stream.Read16(&num_sentences))
        return false;

    sentences.resize(num_sentences);

    for (uint16_t i = 0; i < num_sentences; i++)
    {
        TsrSentence &sentence = sentences[i];

        if (!sentence.Load(stream, enc_pos))
        {
            DPRINTF("%s: Failed to load sentence %d.\n", FUNCNAME, i);
            return false;
        }
    }

    return true;
}

size_t TsrFunction::CalculateSize() const
{
    size_t size = sizeof(uint32_t);

    size += sizeof(uint32_t) + name.length() + 1;
    size += 2*sizeof(uint16_t);

    for (const TsrTag &tag : tags)
        size += tag.CalculateSize();

    size += sizeof(uint16_t);

    for (const TsrSentence &sentence : sentences)
        size += sentence.CalculateSize();

    return size;
}

bool TsrFunction::Save(FixedMemoryStream &stream) const
{
    if (!stream.Write(xor_table, sizeof(uint32_t)))
        return false;

    size_t enc_pos = 4;

    if (!EncodeString(stream, name, enc_pos))
        return false;

    if (!stream.Write16(unk))
        return false;

    if (!stream.Write16((uint16_t)tags.size()))
        return false;

    for (const TsrTag &tag : tags)
        if (!tag.Save(stream, enc_pos))
            return false;

    if (!stream.Write16((uint16_t)sentences.size()))
        return false;

    for (const TsrSentence &sentence : sentences)
        if (!sentence.Save(stream, enc_pos))
            return false;

    return true;
}

TiXmlElement *TsrFunction::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Function");

    entry_root->SetAttribute("name", name);
    entry_root->SetAttribute("unk", unk);    

    for (size_t i = 0; i < sentences.size(); i++)
    {
        const TsrSentence &sentence = sentences[i];
        Utils::WriteComment(entry_root, Utils::ToString(i));

        for (const TsrTag &tag : tags)
        {
            if (tag.index == i)
                tag.Decompile(entry_root);
        }

        sentence.Decompile(entry_root);
    }

    bool index_comment_written = false;

    for (const TsrTag &tag : tags)
    {
        if (tag.index >= sentences.size())
        {
            if (!index_comment_written)
            {
                Utils::WriteComment(entry_root, Utils::ToString(sentences.size()));
                index_comment_written = true;
            }

            tag.Decompile(entry_root);
        }
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool TsrFunction::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrString(root, "name", name))
        return false;

    if (!Utils::ReadAttrUnsigned(root, "unk", &unk))
        return false;

    uint16_t index = 0;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Tag")
        {
           TsrTag tag;

           if (!tag.Compile(elem))
               return false;

           tag.index = index;
           tags.push_back(tag);
        }
        else if (elem->ValueStr() == "Sentence")
        {
            TsrSentence sentence;

            if (!sentence.Compile(elem))
                return false;

            sentences.push_back(sentence);
            index++;
        }
    }

    return true;
}

TsrFile::TsrFile()
{

}

TsrFile::~TsrFile()
{

}

void TsrFile::Reset()
{
    functions.clear();
}

bool TsrFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size == 0)
        return false;

    FixedMemoryStream stream((uint8_t *)buf, size);
    uint16_t num_functions;

    if (!stream.Read16(&num_functions))
        return false;

    if (num_functions == 0)
        return true;

    functions.resize(num_functions);
    stream.Seek(num_functions*sizeof(uint32_t), SEEK_CUR);

    for (uint16_t i = 0; i < num_functions; i++)
    {
        uint64_t function_offset = stream.Tell();
        uint32_t function_size;
        uint8_t *function_buf;

        if (!stream.Seek(sizeof(uint16_t) + i * sizeof(uint32_t), SEEK_SET))
            return false;

        if (!stream.Read32(&function_size))
            return false;

        if (!stream.Seek(function_offset, SEEK_SET))
            return false;

        if (!stream.FastRead(&function_buf, function_size))
            return false;

        FixedMemoryStream function_stream(function_buf, function_size);
        TsrFunction &function = functions[i];

        if (!function.Load(function_stream))
        {
            DPRINTF("%s: function load failed (index %d).\n", FUNCNAME, i);
            return false;
        }
    }

    return true;
}

uint8_t *TsrFile::Save(size_t *psize)
{
    *psize = CalculateSize();
    uint8_t *buf = new uint8_t[*psize];

    FixedMemoryStream stream(buf, *psize);

    if (!stream.Write16((uint16_t)functions.size()))
    {
        delete[] buf;
        return nullptr;
    }

    stream.Seek(functions.size()*sizeof(uint32_t), SEEK_CUR);

    for (size_t i = 0; i < functions.size(); i++)
    {
        const TsrFunction &function = functions[i];

        uint64_t function_offset = stream.Tell();
        uint32_t function_size = (uint32_t)function.CalculateSize();

        if (!stream.Seek(sizeof(uint16_t) + i * sizeof(uint32_t), SEEK_SET))
        {
            delete[] buf;
            return nullptr;
        }

        if (!stream.Write32(function_size))
        {
            delete[] buf;
            return nullptr;
        }

        if (!stream.Seek(function_offset, SEEK_SET))
        {
            delete[] buf;
            return nullptr;
        }

        if (!function.Save(stream))
        {
            delete[] buf;
            return nullptr;
        }
    }

    return buf;
}

TiXmlDocument *TsrFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("TsrFile");

    for (const TsrFunction &function : functions)
    {
        function.Decompile(root);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool TsrFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "TsrFile");

    if (!root)
    {
        DPRINTF("Cannot find\"TsrFile\" in xml.\n");
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Function")
        {
            TsrFunction function;

            if (!function.Compile(elem))
                return false;

            functions.push_back(function);
        }
    }

    return true;
}

size_t TsrFile::CalculateSize() const
{
    size_t size = sizeof(uint16_t) + functions.size()*sizeof(uint32_t);

    for (const TsrFunction &function : functions)
        size += function.CalculateSize();

    return size;
}





