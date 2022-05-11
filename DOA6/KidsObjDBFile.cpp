#include <algorithm>

#include "KidsObjDBFile.h"
#include "debug.h"

#define MIN_BLOB_SIZE   128

static std::unordered_map<std::string, uint32_t> hashes_exceptions =
{
    { "R_GRP［S0101PIR_set－pir－bag－a01］", 0x82508def },
    { "default.g1n", 0xe876b7f0 },
    { "R_G1N［default］", 0xe876b7f0 }
};

static std::unordered_map<uint32_t, std::string> names_map;

static std::string hash_or_name(uint32_t hash)
{
    auto it = names_map.find(hash);
    if (it == names_map.end())
        return Utils::UnsignedToHexString(hash, true);

    return it->second;
}

static uint32_t hash_func(const char *a1, int a2, int a3)
{
  int v3;

  while ( 1 )
  {
    v3 = a3;

    char c = *a1;
    if (!c)
        break;

    a3 *= 31;
    a2 += 31 * v3 * c;
    a1++;
  }
  return (uint32_t)a2;
}

static uint32_t hash_func(const char *a1)
{
    return hash_func(a1+1, a1[0]*31, 31);
}

static uint32_t hash_func(const std::string &s)
{
    return hash_func(s.c_str());
}

static uint32_t hash_or_name(const std::string &str)
{
    if (str.substr(0, 2) == "0x" || Utils::HasOnlyDigits(str))
        return Utils::GetUnsigned(str);

    auto it_ex = hashes_exceptions.find(str);
    if (it_ex != hashes_exceptions.end())
        return it_ex->second;

    return hash_func(str);
}

bool KidsODBColumn::IsString() const
{
    if (type != KIDS_ODB_UINT8 || values8.size() < 2)
        return false;

    if (name == 0x39a84a47) // EvaluatorParameters
        return false;

    if (values8.back().u8 != 0)
        return false;

    for (size_t i = 0; i< values8.size()-1; i++)
    {
        if (values8[i].u8 == 0)
            return false;
    }

    return true;
}

bool KidsODBColumn::IsBlob() const
{
    return (type == KIDS_ODB_UINT8 && values8.size() >= MIN_BLOB_SIZE);
}

bool KidsODBColumn::IsStringArray(uint32_t parent_type) const
{
    if (type != KIDS_ODB_UINT8 || values8.size() < 2)
        return false;

    //name == ByteCode and parent_type == TypeInfo::Object::Animation::Data::FCurve
    if (name == 0x7ae38475 && parent_type == 0xc27bf870)
    {
        // Disable for the moment
        //return true;
    }

    // ColorVariationNameArray, CharacterNameArray, ScreenLayoutNameArray, ObjNames, EnvironmentNameArray, StageNameArray
    return (name == 0x17de9422 || name == 0x4d269345 || name == 0xcb3ec6e8 || name == 0xc69eb271 || name == 0x2881029b || name == 0x2e0c3ed0);
}

size_t KidsODBColumn::GetStringArray(std::vector<std::string> &ret) const
{
    ret.clear();
    std::string str;

    for (const KidsODBValue8 &val : values8)
    {
        char ch = (char)val.u8;

        if (ch == 0)
        {
            ret.push_back(str);
            str.clear();
        }
        else
        {
            str.push_back(ch);
        }
    }

    return ret.size();
}

TiXmlElement *KidsODBColumn::Decompile(TiXmlNode *root, const std::string &parent_name, uint32_t parent_type, const std::string &att_dir) const
{
    TiXmlElement *entry_root = new TiXmlElement("Column");
    std::string my_name = hash_or_name(name);

    entry_root->SetAttribute("name", my_name);

    switch (type)
    {
        case KIDS_ODB_INT8:
            entry_root->SetAttribute("type", "int8");
        break;

        case KIDS_ODB_UINT8:
            entry_root->SetAttribute("type", "uint8");
        break;

        case KIDS_ODB_INT16:
            entry_root->SetAttribute("type", "int16");
        break;

        case KIDS_ODB_UINT16:
            entry_root->SetAttribute("type", "uint16");
        break;

        case KIDS_ODB_INT32:
            entry_root->SetAttribute("type", "int32");
        break;

        case KIDS_ODB_UINT32:
            entry_root->SetAttribute("type", "uint32");
        break;

        case KIDS_ODB_FLOAT:
            entry_root->SetAttribute("type", "float");
        break;

        case KIDS_ODB_VECTOR2:
            entry_root->SetAttribute("type", "vector2");
        break;

        case KIDS_ODB_VECTOR3:
            entry_root->SetAttribute("type", "vector3");
        break;

        case KIDS_ODB_VECTOR4:
            entry_root->SetAttribute("type", "vector4");
        break;

        default:
            // should not be here
            entry_root->SetAttribute("type", "???");
    }

    for (const KidsODBValue8 &val: values8)
    {
        if (IsString())
        {
            TiXmlElement *rows_root = new TiXmlElement("Rows");
            rows_root->SetAttribute("value", (const char *)values8.data());
            entry_root->LinkEndChild(rows_root);
            break;
        }
        else if (IsStringArray(parent_type))
        {
            std::vector<std::string> array;

            TiXmlElement *rows_root = new TiXmlElement("Rows");
            GetStringArray(array);
            rows_root->SetAttribute("multi_str", Utils::ToSingleString(array, ", ", false));
            entry_root->LinkEndChild(rows_root);
            break;
        }
        else if (IsBlob())
        {
            std::string fn = parent_name + "_" + my_name + ".bin";
            std::string path = Utils::MakePathString(att_dir, fn);

            if (!Utils::WriteFileBool(path, (const uint8_t *)values8.data(), values8.size(), true, true))
                return nullptr;

            TiXmlElement *rows_root = new TiXmlElement("Rows");
            rows_root->SetAttribute("binary", fn);
            entry_root->LinkEndChild(rows_root);
            break;
        }

        TiXmlElement *row_root = new TiXmlElement("Row");
        std::string value;

        switch (type)
        {
            case KIDS_ODB_INT8:
                row_root->SetAttribute("value", (int)val.i8);
            break;

            case KIDS_ODB_UINT8:
                row_root->SetAttribute("value", Utils::UnsignedToHexString(val.u8, true));
            break;           
        }

        entry_root->LinkEndChild(row_root);
    }

    for (const KidsODBValue16 &val: values16)
    {
        TiXmlElement *row_root = new TiXmlElement("Row");
        std::string value;

        switch (type)
        {
            case KIDS_ODB_INT16:
                row_root->SetAttribute("value", (int)val.i16);
            break;

            case KIDS_ODB_UINT16:
                row_root->SetAttribute("value", Utils::UnsignedToHexString(val.u16, true));
            break;
        }

        entry_root->LinkEndChild(row_root);
    }

    for (const KidsODBValue32 &val: values32)
    {
        TiXmlElement *row_root = new TiXmlElement("Row");
        std::string value;

        switch (type)
        {
            case KIDS_ODB_INT32:
                row_root->SetAttribute("value", (int)val.i32);
            break;

            case KIDS_ODB_UINT32:
                row_root->SetAttribute("value", hash_or_name(val.u32));
            break;

            case KIDS_ODB_FLOAT:
                row_root->SetAttribute("value", Utils::FloatToString(val.f));
            break;
        }

        entry_root->LinkEndChild(row_root);
    }

    for (const KidsODBValue64 &val: values64)
    {
        TiXmlElement *row_root = new TiXmlElement("Row");
        std::string value;

        switch (type)
        {
            case KIDS_ODB_VECTOR2:

                for (size_t i = 0; i < 2; i++)
                {
                    value += Utils::FloatToString(val.v2[i]);
                    if (i != 1)
                        value += ", ";
                }

                row_root->SetAttribute("value", value);
            break;
        }

        entry_root->LinkEndChild(row_root);
    }

    for (const KidsODBValue96 &val: values96)
    {
        TiXmlElement *row_root = new TiXmlElement("Row");
        std::string value;

        switch (type)
        {
            case KIDS_ODB_VECTOR3:

                for (size_t i = 0; i < 3; i++)
                {
                    value += Utils::FloatToString(val.v3[i]);
                    if (i != 2)
                        value += ", ";
                }

                row_root->SetAttribute("value", value);
            break;
        }

        entry_root->LinkEndChild(row_root);
    }

    for (const KidsODBValue128 &val: values128)
    {
        TiXmlElement *row_root = new TiXmlElement("Row");
        std::string value;

        switch (type)
        {
            case KIDS_ODB_VECTOR4:

                for (size_t i = 0; i < 4; i++)
                {
                    value += Utils::FloatToString(val.v4[i]);
                    if (i != 3)
                        value += ", ";
                }

                row_root->SetAttribute("value", value);
            break;
        }

        entry_root->LinkEndChild(row_root);
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool KidsODBColumn::Compile(const TiXmlElement *root, const std::string &att_dir)
{
    std::string name_str, type_str;

    if (!Utils::ReadAttrString(root, "name", name_str))
    {
        DPRINTF("Missing attribute \"name\" (in Column at line %d).\n", root->Row());
        return false;
    }

    if (!Utils::ReadAttrString(root, "type", type_str))
    {
        DPRINTF("Missing attribute \"type\" (in Column at line %d).\n", root->Row());
        return false;
    }

    name = hash_or_name(name_str);
    type_str = Utils::ToLowerCase(type_str);

    if (type_str == "int8")
        type = KIDS_ODB_INT8;
    else if (type_str == "uint8")
        type = KIDS_ODB_UINT8;
    else if (type_str == "int16")
        type = KIDS_ODB_INT16;
    else if (type_str == "uint16")
        type = KIDS_ODB_UINT16;
    else if (type_str == "int32")
        type = KIDS_ODB_INT32;
    else if (type_str == "uint32")
        type = KIDS_ODB_UINT32;
    else if (type_str == "float")
        type = KIDS_ODB_FLOAT;
    else if (type_str == "vector2")
        type = KIDS_ODB_VECTOR2;
    else if (type_str == "vector3")
        type = KIDS_ODB_VECTOR3;
    else if (type_str == "vector4")
        type = KIDS_ODB_VECTOR4;
    else
    {
        DPRINTF("Unrecognized Column type \"%s\" (at line %d).\n", type_str.c_str(), root->Row());
        return false;
    }

    values8.clear();
    values16.clear();
    values32.clear();
    values64.clear();
    values96.clear();
    values128.clear();

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Row")
        {
            KidsODBValue8 value8;
            KidsODBValue16 value16;
            KidsODBValue32 value32;
            KidsODBValue64 value64;
            KidsODBValue96 value96;
            KidsODBValue128 value128;
            std::string hash_str;
            bool ret = false;

            switch (type)
            {
                case KIDS_ODB_INT8:
                    if (Utils::ReadAttrSigned(elem, "value", &value8.i8))
                    {
                        values8.push_back(value8);
                        ret = true;
                    }
                break;

                case KIDS_ODB_UINT8:
                    if (Utils::ReadAttrUnsigned(elem, "value", &value8.u8))
                    {
                        values8.push_back(value8);
                        ret = true;
                    }
                break;

                case KIDS_ODB_INT16:
                    if (Utils::ReadAttrSigned(elem, "value", &value16.i16))
                    {
                        values16.push_back(value16);
                        ret = true;
                    }
                break;

                case KIDS_ODB_UINT16:
                    if (Utils::ReadAttrUnsigned(elem, "value", &value16.u16))
                    {
                        values16.push_back(value16);
                        ret = true;
                    }
                break;

                case KIDS_ODB_INT32:
                    if (Utils::ReadAttrSigned(elem, "value", &value32.i32))
                    {
                        values32.push_back(value32);
                        ret = true;
                    }
                break;

                case KIDS_ODB_UINT32:

                    if (Utils::ReadAttrString(elem, "value", hash_str))
                    {
                        value32.u32 = hash_or_name(hash_str);
                        values32.push_back(value32);
                        ret = true;
                    }
                break;

                case KIDS_ODB_FLOAT:
                    if (Utils::ReadAttrFloat(elem, "value", &value32.f))
                    {
                        values32.push_back(value32);
                        ret = true;
                    }
                break;

                case KIDS_ODB_VECTOR2:
                    if (Utils::ReadAttrMultipleFloats(elem, "value", value64.v2, 2))
                    {
                        values64.push_back(value64);
                        ret = true;
                    }
                break;

                case KIDS_ODB_VECTOR3:
                    if (Utils::ReadAttrMultipleFloats(elem, "value", value96.v3, 3))
                    {
                        values96.push_back(value96);
                        ret = true;
                    }
                break;

                case KIDS_ODB_VECTOR4:
                    if (Utils::ReadAttrMultipleFloats(elem, "value", value128.v4, 4))
                    {
                        values128.push_back(value128);
                        ret = true;
                    }
                break;
            }

            if (!ret)
            {
                DPRINTF("Failed to read Row at line %d.\n", elem->Row());
                return false;
            }
        }
        else if (elem->ValueStr() == "Rows")
        {
            if (type != KIDS_ODB_UINT8)
            {
                DPRINTF("Element for multiple rows <Rows>, can only be used on type uint8. (error at line %d).\n", elem->Row());
                return false;
            }

            if (values8.size() > 0)
            {
                DPRINTF("<Row> and <Rows> cannot be mixed (error in line %d).\n", elem->Row());
                return false;
            }

            std::string str;

            if (Utils::ReadAttrString(elem, "value", str))
            {
                values8.resize(str.length()+1);

                for (size_t i = 0; i < str.length(); i++)
                    values8[i].i8 = str[i];

                values8[str.length()].i8 = 0;
            }
            else if (Utils::ReadAttrString(elem, "multi_str", str))
            {
                std::vector<std::string> multi_str;
                Utils::GetMultipleStrings(str, multi_str, ',', false, false);

                for (const std::string &str : multi_str)
                {
                    size_t old_size = values8.size();
                    values8.resize(old_size+str.length()+1);

                    for (size_t i = 0; i < str.length(); i++)
                        values8[old_size+i].i8 = str[i];

                    values8[old_size+str.length()].i8 = 0;
                }                
            }
            else if (Utils::ReadAttrString(elem, "binary", str))
            {
                std::string fn = Utils::MakePathString(att_dir, str);
                size_t size;
                uint8_t *buf = Utils::ReadFile(fn, &size);
                if (!buf)
                    return false;

                values8.resize(size);
                memcpy(values8.data(), buf, size);
                delete[] buf;
            }
            else
            {
                DPRINTF("Error in <Rows> at line %d. Expected one of the following attributes \"value\", \"multi_str\" or \"binary\" but none found.", elem->Row());
                return false;
            }

            break;
        }
    }

    return true;
}

TiXmlElement *KidsODBObject::Decompile(TiXmlNode *root, const std::string &att_dir) const
{
    std::string my_name = hash_or_name(name);

    TiXmlElement *entry_root = new TiXmlElement("Object");
    entry_root->SetAttribute("version", version);
    entry_root->SetAttribute("name", my_name);    

    if (is_r)
    {
        entry_root->SetAttribute("parent_object_file", hash_or_name(parent_object_file));
        entry_root->SetAttribute("parent_object", hash_or_name(parent_object));
    }
    else
    {
        entry_root->SetAttribute("type", hash_or_name(type));
    }

    for (const KidsODBColumn &col : columns)
    {
        if (!col.Decompile(entry_root, my_name, type, att_dir))
            return nullptr;
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool KidsODBObject::Compile(const TiXmlElement *root, const std::string &att_dir)
{
    std::string name_str, parent_object_file_str, parent_object_str, type_str;

    if (!Utils::ReadAttrUnsigned(root, "version", &version))
    {
        DPRINTF("Missing attribute \"version\" in Object at line %d.\n", root->Row());
        return false;
    }

    if (!Utils::ReadAttrString(root, "name", name_str))
    {
        DPRINTF("Missing attribute \"name\" in Object at line %d.\n", root->Row());
        return false;
    }

    name = hash_or_name(name_str);

    if (Utils::ReadAttrString(root, "type", type_str))
    {
        is_r = false;
        type = hash_or_name(type_str);
    }
    else if (Utils::ReadAttrString(root, "parent_object_file", parent_object_file_str))
    {
        is_r = true;

        if (!Utils::ReadAttrString(root, "parent_object", parent_object_str))
        {
            DPRINTF("Missing attribute \"parent_object\" in Object at line %d.\n", root->Row());
            return false;
        }

        parent_object_file = hash_or_name(parent_object_file_str);
        parent_object = hash_or_name(parent_object_str);
    }
    else
    {
        DPRINTF("Error in Object at line %d. Was expecting either attribute \"type\" or attribute \"parent_object_file\".", root->Row());
        return false;
    }

    size_t count = Utils::GetElemCount(root, "Column");
    columns.resize(count);

    size_t n = 0;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Column")
        {
            if (!columns[n++].Compile(elem, att_dir))
                return false;
        }
    }

    return true;
}


KidsObjDBFile::KidsObjDBFile()
{
    this->big_endian = false;
}

KidsObjDBFile::~KidsObjDBFile()
{

}

void KidsObjDBFile::Reset()
{
    objects.clear();
    name_file = 0;
    version = 0;
    platform = 0xA;
}

bool KidsObjDBFile::Load(const uint8_t *buf, size_t size)
{
    FixedMemoryStream mem(const_cast<uint8_t *>(buf), size);

    KODHeader *hdr;

    if (!mem.FastRead((uint8_t **)&hdr, sizeof(KODHeader)))
        return false;

    if (hdr->signature != KOD_SIGNATURE)
        return false;

    name_file = hdr->name_file;
    version = Utils::GetShortVersion(hdr->version);
    platform = hdr->platform;

    if (!mem.Seek(hdr->header_size, SEEK_SET))
        return false;

    objects.resize(hdr->num_entries);

    for (KidsODBObject &obj : objects)
    {
        KODIHeader *entry;        

        if (!mem.FastRead((uint8_t **)&entry, sizeof(KODIHeader)))
        {
            DPRINTF("%s: Failed to read entry\n", FUNCNAME);
            return false;
        }

        if (entry->signature == KODI_SIGNATURE)
        {
            obj.name = entry->name;
            obj.type = entry->type;
            obj.version = Utils::GetShortVersion(entry->version);
            obj.columns.resize(entry->num_columns);
            obj.is_r = false;
        }
        else if (entry->signature == KODR_SIGNATURE)
        {
            KODRHeader *rentry;

            mem.Seek(-sizeof(KODIHeader), SEEK_CUR);

            if (!mem.FastRead((uint8_t **)&rentry, sizeof(KODRHeader)))
            {
                DPRINTF("%s: Failed to read entry (2).\n", FUNCNAME);
                return false;
            }

            obj.name = rentry->name;
            obj.version = Utils::GetShortVersion(rentry->version);
            obj.columns.resize(rentry->num_columns);
            obj.parent_object_file = rentry->parent_object_file;
            obj.parent_object = rentry->parent_object;
            obj.is_r = true;
        }
        else
        {
            DPRINTF("%s: Unknown entry signature at 0x%Ix\n", FUNCNAME, (size_t)(mem.Tell() - sizeof(KODIHeader)));
            return false;
        }

        for (KidsODBColumn &col : obj.columns)
        {
            uint64_t current = mem.Tell();

            if (!mem.Read32(&col.type))
            {
                DPRINTF("%s: Failed to read column type (in column at 0x%Ix).\n", FUNCNAME, (size_t)current);
                return false;
            }

            uint32_t num;

            if (!mem.Read32(&num))
            {
                DPRINTF("%s: Failed to read num of rows (in column at 0x%Ix).\n", FUNCNAME, (size_t)current);
                return false;
            }

            if (!mem.Read32(&col.name))
            {
                DPRINTF("%s: Failed to read coluumn name (in column at 0x%Ix).\n", FUNCNAME, (size_t)current);
                return false;
            } 

            switch (col.type)
            {
                case KIDS_ODB_INT8: case KIDS_ODB_UINT8:
                    col.values8.resize(num);
                break;

                case KIDS_ODB_INT16: case KIDS_ODB_UINT16:
                    col.values16.resize(num);
                break;

                case KIDS_ODB_INT32: case KIDS_ODB_UINT32:
                case KIDS_ODB_FLOAT:
                    col.values32.resize(num);
                break;

                case KIDS_ODB_VECTOR2:
                    col.values64.resize(num);
                break;

                case KIDS_ODB_VECTOR3:
                    col.values96.resize(num);
                break;

                case KIDS_ODB_VECTOR4:
                    col.values128.resize(num);
                break;

                default:
                    DPRINTF("%s: Unknown column type 0x%x (in column at 0x%Ix).\n", FUNCNAME, col.type, (size_t)current);
                    return false;
            }
        }

        for (KidsODBColumn &col : obj.columns)
        {
            switch (col.type)
            {
                case KIDS_ODB_INT8: case KIDS_ODB_UINT8:

                  for (KidsODBValue8 &val: col.values8)
                  {
                      if (!mem.Read8(&val.u8))
                          return false;
                  }

                break;

                case KIDS_ODB_INT16: case KIDS_ODB_UINT16:

                  for (KidsODBValue16 &val: col.values16)
                  {
                      if (!mem.Read16(&val.u16))
                          return false;
                  }

                break;

                case KIDS_ODB_INT32: case KIDS_ODB_UINT32: case KIDS_ODB_FLOAT:

                  for (KidsODBValue32 &val: col.values32)
                  {
                      if (!mem.Read32(&val.u32))
                          return false;
                  }

                break;

                case KIDS_ODB_VECTOR2:

                    for (KidsODBValue64 &val : col.values64)
                    {
                        if (!mem.Read(val.v2, sizeof(val.v2)))
                            return false;
                    }

                break;

                case KIDS_ODB_VECTOR3:

                    for (KidsODBValue96 &val : col.values96)
                    {
                        if (!mem.Read(val.v3, sizeof(val.v3)))
                            return false;
                    }

                break;

                case KIDS_ODB_VECTOR4:

                    for (KidsODBValue128 &val : col.values128)
                    {
                        if (!mem.Read(val.v4, sizeof(val.v4)))
                            return false;
                    }

                break;
            }
        }

        // align to 4
        while (mem.Tell() & 3)
            mem.Seek(1, SEEK_CUR);
    }

    UPRINTF("%Id objects read.\n", objects.size());

    return true;
}

uint8_t *KidsObjDBFile::Save(size_t *psize)
{
    MemoryStream mem;
    KODHeader hdr;

    hdr.signature = KOD_SIGNATURE;
    hdr.version = Utils::GetLongVersion(version);
    hdr.header_size = (uint32_t)sizeof(KODHeader);
    hdr.platform = platform;
    hdr.num_entries = (uint32_t)objects.size();
    hdr.name_file = name_file;
    hdr.file_size = 0;

    if (!mem.Write(&hdr, sizeof(KODHeader)))
        return nullptr;

    for (const KidsODBObject &obj : objects)
    {
        KODIHeader entry;
        KODRHeader rentry;

        uint64_t top_ofs = mem.Tell();

        if (!obj.is_r)
        {
            entry.signature = KODI_SIGNATURE;
            entry.version = Utils::GetLongVersion(obj.version);
            entry.entry_size = 0;
            entry.name = obj.name;
            entry.type = obj.type;
            entry.num_columns = (uint32_t)obj.columns.size();

            if (!mem.Write(&entry, sizeof(KODIHeader)))
                return nullptr;
        }
        else
        {
            rentry.signature = KODR_SIGNATURE;
            rentry.version = Utils::GetLongVersion(obj.version);
            rentry.entry_size = 0;
            rentry.name = obj.name;
            rentry.parent_object_file = obj.parent_object_file;
            rentry.parent_object = obj.parent_object;
            rentry.num_columns = (uint32_t)obj.columns.size();

            if (!mem.Write(&rentry, sizeof(KODRHeader)))
                return nullptr;
        }

        for (const KidsODBColumn &col : obj.columns)
        {
            if (!mem.Write32(col.type))
                return nullptr;

            switch (col.type)
            {
                case KIDS_ODB_INT8: case KIDS_ODB_UINT8:
                    if (!mem.Write32((uint32_t)col.values8.size()))
                        return nullptr;
                break;

                case KIDS_ODB_INT16: case KIDS_ODB_UINT16:
                    if (!mem.Write32((uint32_t)col.values16.size()))
                        return nullptr;
                break;

                case KIDS_ODB_INT32: case KIDS_ODB_UINT32: case KIDS_ODB_FLOAT:
                    if (!mem.Write32((uint32_t)col.values32.size()))
                        return nullptr;
                break;

                case KIDS_ODB_VECTOR2:
                    if (!mem.Write32((uint32_t)col.values64.size()))
                        return nullptr;
                break;

                case KIDS_ODB_VECTOR3:
                    if (!mem.Write32((uint32_t)col.values96.size()))
                        return nullptr;
                break;

                case KIDS_ODB_VECTOR4:
                    if (!mem.Write32((uint32_t)col.values128.size()))
                        return nullptr;
                break;

                default:
                    DPRINTF("%s: Internal error, type 0x%x not handled in save code.\n", FUNCNAME, col.type);
                    return nullptr;
            }

            if (!mem.Write32(col.name))
                return nullptr;
        }

        for (const KidsODBColumn &col : obj.columns)
        {
            switch (col.type)
            {
                case KIDS_ODB_INT8: case KIDS_ODB_UINT8:
                    if (col.values8.size() > 0 && !mem.Write(col.values8.data(), col.values8.size()))
                        return nullptr;
                break;

                case KIDS_ODB_INT16: case KIDS_ODB_UINT16:
                    if (col.values16.size() > 0 && !mem.Write(col.values16.data(), col.values16.size()*2))
                        return nullptr;
                break;

                case KIDS_ODB_INT32: case KIDS_ODB_UINT32: case KIDS_ODB_FLOAT:
                    if (col.values32.size() > 0 && !mem.Write(col.values32.data(), col.values32.size()*4))
                        return nullptr;
                break;

                case KIDS_ODB_VECTOR2:
                    if (col.values64.size() > 0 && !mem.Write(col.values64.data(), col.values64.size()*8))
                        return nullptr;
                break;

                case KIDS_ODB_VECTOR3:
                    if (col.values96.size() > 0 && !mem.Write(col.values96.data(), col.values96.size()*12))
                        return nullptr;
                break;

                case KIDS_ODB_VECTOR4:
                    if (col.values128.size() > 0 && !mem.Write(col.values128.data(), col.values128.size()*16))
                        return nullptr;
                break;

                default:
                    DPRINTF("%s: Internal error, type 0x%x not handled in save code (2).\n", FUNCNAME, col.type);
                    return nullptr;
            }
        }

        mem.Seek(top_ofs, SEEK_SET);

        if (!obj.is_r)
        {
            entry.entry_size = (uint32_t) mem.GetSize() - top_ofs;
            if (!mem.Write(&entry, sizeof(KODIHeader)))
                return nullptr;
        }
        else
        {
            rentry.entry_size = (uint32_t) mem.GetSize() - top_ofs;
            if (!mem.Write(&rentry, sizeof(KODRHeader)))
                return nullptr;
        }

        mem.Seek(0, SEEK_END);

        if (!mem.Align(4))
            return nullptr;
    }

    //
    mem.Seek(0, SEEK_SET);
    hdr.file_size = (uint32_t)mem.GetSize();
    if (!mem.Write(&hdr, sizeof(KODHeader)))
        return nullptr;

    *psize = mem.GetSize();
    return mem.GetMemory(true);
}

TiXmlDocument *KidsObjDBFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();
    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("KidsObjDB");
    root->SetAttribute("version", version);
    root->SetAttribute("platform", platform);
    root->SetAttribute("name_file", hash_or_name(name_file));

    for (const KidsODBObject &obj : objects)
    {
        if (!obj.Decompile(root, att_dir))
            return nullptr;
    }

    doc->LinkEndChild(root);
    return doc;
}

bool KidsObjDBFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "KidsObjDB");

    if (!root)
    {
        DPRINTF("Cannot find\"KidsObjDB\" in xml.\n");
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "version", &version))
    {
        DPRINTF("Missing attribute \"version\" at line %d.\n", root->Row());
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "platform", &platform))
    {
        DPRINTF("Missing attribute \"platform\" at line %d.\n", root->Row());
        return false;
    }

    std::string name_file_str;

    if (!Utils::ReadAttrString(root, "name_file", name_file_str))
    {
        DPRINTF("Missing attribute \"name_file\" at line %d.\n", root->Row());
        return false;
    }

    name_file = hash_or_name(name_file_str);

    size_t count = Utils::GetElemCount(root, "Object");
    objects.resize(count);

    size_t n = 0;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Object")
        {
            if (!objects[n++].Compile(elem, att_dir))
                return false;
        }
    }

    return true;
}

bool KidsObjDBFile::DecompileToFile(const std::string &path, bool show_error, bool build_path)
{
    size_t pos = Utils::NormalizePath(path).rfind('/');

    std::string dir = Utils::GetDirNameString(path);
    std::string fn = Utils::GetFileNameString(path);

    if (pos == std::string::npos)
        dir = "./";

    pos = Utils::ToLowerCase(fn).rfind(".kidsobjdb.xml");
    if (pos == std::string::npos)
    {
        fn += "_BIN";
    }
    else
    {
       fn = fn.substr(0, pos);
    }

    att_dir = Utils::MakePathString(dir, fn);
    return BaseFile::DecompileToFile(path, show_error, build_path);
}

bool KidsObjDBFile::CompileFromFile(const std::string &path, bool show_error, bool big_endian)
{
    size_t pos = Utils::NormalizePath(path).rfind('/');

    std::string dir = Utils::GetDirNameString(path);
    std::string fn = Utils::GetFileNameString(path);

    if (pos == std::string::npos)
        dir = "./";

    pos = Utils::ToLowerCase(fn).rfind(".kidsobjdb");
    if (pos == std::string::npos)
    {
        fn += "_BIN";
    }
    else
    {
       fn = fn.substr(0, pos);
    }

    att_dir = Utils::MakePathString(dir, fn);
    return BaseFile::CompileFromFile(path, show_error, big_endian);
}

static bool object_sort_by_name(const KidsODBObject &obj1, const KidsODBObject &obj2)
{
    auto it1 = names_map.find(obj1.name);
    auto it2 = names_map.find(obj2.name);

    if (it1 != names_map.end())
    {
        if (it2 != names_map.end())
            return (it1->second < it2->second);

        return true;
    }
    else
    {
        if (it2 != names_map.end())
            return false;

        return (obj1.name < obj2.name);
    }

    return true;
}

static bool object_sort_by_hash(const KidsODBObject &obj1, const KidsODBObject &obj2)
{
    return (obj1.name < obj2.name);
}

void KidsObjDBFile::SortByName()
{
    std::sort(objects.begin(), objects.end(), object_sort_by_name);
}

void KidsObjDBFile::SortByHash()
{
    std::sort(objects.begin(), objects.end(), object_sort_by_hash);
}

static bool get_hex_name(const std::string &fn, uint32_t *ret)
{
    if (!Utils::BeginsWith(fn, "0x", false))
        return false;

    if (fn.length() == 2)
        return false;

    for (size_t i = 2; i < fn.length(); i++)
    {
        char ch = (char)tolower(fn[i]);

        if (ch == '.')
            break;

        bool ok = false;

        if (ch >= '0' && ch <= '9')
            ok = true;
        else if (ch >= 'a' && ch <= 'f')
            ok = true;

        if (!ok)
            return false;
    }

    *ret = Utils::GetUnsigned(fn);
    return true;
}

bool KidsObjDBFile::LoadNamesFileBuf(const char *buf)
{
    // New implementaton asumes there are no inner spaces in filenames
    char comp1[32], comp2[256];
    bool in_comment = false;
    bool in_c2 = false;

    size_t len = strlen(buf);
    size_t c1 = 0, c2 = 0;
    int line_num = 1;


    for (size_t i = 0; i < len; i++)
    {
        char ch = buf[i];

        if (ch == '\n')
        {
            if (in_comment)
            {
                in_comment = false;
            }
            else if (c1 != 0)
            {
                if (!in_c2)
                    DPRINTF("%s: Faulty line %d.\n", FUNCNAME, line_num);

                comp1[c1] = 0;
                comp2[c2] = 0;

                uint32_t id;

                if (!get_hex_name(comp1, &id))
                {
                    DPRINTF("%s: parse error: \"%s\" is not a valid hash. (line %d)\n", FUNCNAME, comp1, line_num);
                    return false;
                }

                names_map[id] = comp2;
            }

            c1 = c2 = 0;
            in_c2 = false;
            line_num++;
            continue;
        }

        if (in_comment)
            continue;

        if (ch >= 0 && ch <= ' ')
            continue;

        if (c1 == 0 && (ch == ';' || ch == '#'))
        {
            in_comment = true;
            continue;
        }

        if (!in_c2 && ch == ',')
        {
            in_c2 = true;
            continue;
        }

        if (in_c2)
            comp2[c2++] = ch;
        else
            comp1[c1++] = ch;
    }

    UPRINTF("Map loaded with %Id values.\n", names_map.size());
    return true;
}

bool KidsObjDBFile::LoadNamesFile(const std::string &path)
{
    size_t size = Utils::GetFileSize(path);
    if (size == (size_t)-1)
    {
        DPRINTF("Cannot stat file \"%s\"", path.c_str());
        return false;
    }

    uint8_t *buf = new uint8_t[size+1];
    buf[size] = 0;

    FILE *f = fopen(path.c_str(), "rb");
    bool ret = false;

    if (f)
    {
        ret = (fread(buf, 1, size, f) == size);
        if (ret)
        {
            ret = LoadNamesFileBuf((const char *)buf);
        }

        fclose(f);
    }
    else
    {
        DPRINTF("Cannot open file \"%s\"", path.c_str());
    }

    delete[] buf;
    return ret;
}

bool KidsObjDBFile::LoadNamesFileBinBuf(const uint8_t *buf)
{
    const uint8_t *ptr = buf;
    uint32_t num = *(const uint32_t *)ptr;

    names_map.reserve(num);
    ptr += 4;

    for (size_t i = 0; i < (size_t)num; i++)
    {
        std::string str;

        uint32_t hash = *(const uint32_t *)ptr;
        ptr += 4;

        names_map[hash] = (const char *)ptr;
        ptr += strlen((const char *)ptr) + 1;
    }


    UPRINTF("Map loaded with %Id values.\n", names_map.size());

    return true;
}

bool KidsObjDBFile::LoadNamesFileBin(const std::string &path)
{
    size_t size;
    uint8_t *buf = Utils::ReadFile(path, &size);
    if (!buf)
        return false;

    bool ret = LoadNamesFileBinBuf(buf);

    delete[] buf;
    return ret;
}



