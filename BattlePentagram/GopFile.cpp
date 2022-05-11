#include <map>
#include "GopFile.h"

static const std::unordered_map<std::string, GopType> str_to_gtype =
{
    { "RecId", GTYPE_RECID },
    { "RecID", GTYPE_RECID2 },
    { "String", GTYPE_STRING },
    { "Int32", GTYPE_INT32 },
    { "Int16", GTYPE_INT16 },
    { "Int8", GTYPE_INT8 },
    { "UInt32", GTYPE_UINT32 },
    { "UInt16", GTYPE_UINT16 },
    { "UInt8", GTYPE_UINT8 },
    { "IVector4", GTYPE_IVECTOR4 },
    { "bool", GTYPE_BOOL },
    { "Float32", GTYPE_FLOAT32 },
    { "Vector3", GTYPE_VECTOR3 },
    { "Vector4", GTYPE_VECTOR4 },
    { "Rgba", GTYPE_RGBA },
    { "FRgb", GTYPE_FRGB },
    { "FRgba", GTYPE_FRGBA },
    { "THUMBNAIL_TYPE", GTYPE_THUMBNAIL },
    { "MagicType", GTYPE_MAGIC },
    { "VoiceType", GTYPE_VOICE },
    { "MsAvoidAIType", GTYPE_MSAVOIDAI },
    { "AvoidTargetType", GTYPE_AVOIDTARGET },
    { "Direction", GTYPE_DIRECTION },
    { "AvoidAction", GTYPE_AVOIDACTION },
    { "MsAtkCtrlAIType", GTYPE_MSATKCTRLAI },
    { "MsGrapAtkAIType", GTYPE_MSGRAPATKAI },
    { "MsMainAtkAIType", GTYPE_MSMAINATKAI },
    { "MsMoveAIType", GTYPE_MSMOVEAI },
    { "MsSchemeAIType", GTYPE_MSSCHEMEAI },
    { "MsSubAtkAIType", GTYPE_MSSUBATKAI },
    { "CharacterType", GTYPE_CHARACTERTYPE },
    { "FieldPhysicsMode", GTYPE_FIELDPHYSICSMODE },
    { "BtnType", GTYPE_BTN },
    { "SkillType", GTYPE_SKILL },
    { "FreeCategory", GTYPE_FREECATEGORY },
    { "VsCategory", GTYPE_VSCATEGORY },
    { "TerritoryID", GTYPE_TERRITORYID },
    { "UnitSizeID", GTYPE_UNITSIZEID },
    { "SkillInvokeCheckType", GTYPE_SKILLINVOKECHECK },
    { "SkillApplyType", GTYPE_SKILLAPPLY },
    { "PrizeIf", GTYPE_PRIZEIF },
    { "PrizeOp", GTYPE_PRIZEOP },
    { "PrizeElf", GTYPE_PRIZELF },
    { "SEResourceID", GTYPE_SERESOURCEID },
    { "SECategory", GTYPE_SECATEGORY },
    { "StaffRollLayout", GTYPE_STAFFROLLLAYOUT },
    { "RederType", GTYPE_REDER },
    { "UnitLockPriority", GTYPE_UNITLOCKPRIORITY },
    { "UnitLockCategory", GTYPE_UNITLOCKCATEGORY },
    { "ShadowDrawType", GTYPE_SHADOWDRAW },
    { "GimmickType", GTYPE_GIMMICK },
    { "DropItemType", GTYPE_DROPITEM },
    { "UnitColType", GTYPE_UNITCOL },
    { "TargetType", GTYPE_TARGET },
    { "FormType", GTYPE_FORM },
    { "MsMotionGrappleOffset", GTYPE_MSMOTIONGRAPPLEOFFSET },
    { "MsMotionID", GTYPE_MSMOTIONID },
};

static const std::unordered_map<uint32_t, std::string> gtype_to_str =
{
    { GTYPE_RECID, "RecId" },
    { GTYPE_RECID2, "RecID" },
    { GTYPE_STRING, "String" },
    { GTYPE_INT32, "Int32" },
    { GTYPE_INT16, "Int16" },
    { GTYPE_INT8, "Int8" },
    { GTYPE_UINT32, "UInt32" },
    { GTYPE_UINT16, "UInt16" },
    { GTYPE_UINT8, "UInt8" },
    { GTYPE_IVECTOR4, "IVector4" },
    { GTYPE_BOOL, "bool" },
    { GTYPE_FLOAT32, "Float32" },
    { GTYPE_VECTOR3, "Vector3" },
    { GTYPE_VECTOR4, "Vector4" },
    { GTYPE_RGBA, "Rgba" },
    { GTYPE_FRGB, "FRgb" },
    { GTYPE_FRGBA, "FRgba" },
    { GTYPE_THUMBNAIL, "THUMBNAIL_TYPE" },
    { GTYPE_MAGIC, "MagicType" },
    { GTYPE_VOICE, "VoiceType" },
    { GTYPE_MSAVOIDAI, "MsAvoidAIType" },
    { GTYPE_AVOIDTARGET, "AvoidTargetType" },
    { GTYPE_DIRECTION, "Direction" },
    { GTYPE_AVOIDACTION, "AvoidAction" },
    { GTYPE_MSATKCTRLAI, "MsAtkCtrlAIType" },
    { GTYPE_MSGRAPATKAI, "MsGrapAtkAIType" },
    { GTYPE_MSMAINATKAI, "MsMainAtkAIType" },
    { GTYPE_MSMOVEAI, "MsMoveAIType" },
    { GTYPE_MSSCHEMEAI, "MsSchemeAIType" },
    { GTYPE_MSSUBATKAI, "MsSubAtkAIType" },
    { GTYPE_CHARACTERTYPE, "CharacterType" },
    { GTYPE_FIELDPHYSICSMODE, "FieldPhysicsMode" },
    { GTYPE_BTN, "BtnType" },
    { GTYPE_SKILL, "SkillType" },
    { GTYPE_FREECATEGORY, "FreeCategory" },
    { GTYPE_VSCATEGORY, "VsCategory" },
    { GTYPE_TERRITORYID, "TerritoryID" },
    { GTYPE_UNITSIZEID, "UnitSizeID" },
    { GTYPE_SKILLINVOKECHECK, "SkillInvokeCheckType" },
    { GTYPE_SKILLAPPLY, "SkillApplyType" },
    { GTYPE_PRIZEIF, "PrizeIf" },
    { GTYPE_PRIZEOP, "PrizeOp" },
    { GTYPE_PRIZELF, "PrizeElf" },
    { GTYPE_SERESOURCEID, "SEResourceID" },
    { GTYPE_SECATEGORY, "SECategory" },
    { GTYPE_STAFFROLLLAYOUT, "StaffRollLayout" },
    { GTYPE_REDER, "RederType" },
    { GTYPE_UNITLOCKPRIORITY, "UnitLockPriority" },
    { GTYPE_UNITLOCKCATEGORY, "UnitLockCategory" },
    { GTYPE_SHADOWDRAW, "ShadowDrawType" },
    { GTYPE_GIMMICK, "GimmickType" },
    { GTYPE_DROPITEM, "DropItemType" },
    { GTYPE_UNITCOL, "UnitColType" },
    { GTYPE_TARGET, "TargetType" },
    { GTYPE_FORM, "FormType" },
    { GTYPE_MSMOTIONGRAPPLEOFFSET, "MsMotionGrappleOffset" },
    { GTYPE_MSMOTIONID, "MsMotionID" },
};

GopFile::GopFile()
{
    this->big_endian = false;
}

GopFile::~GopFile()
{

}

void GopFile::Reset()
{
    table.clear();
    grecs.clear();
    gdat_unks.clear();
    gdat_values.clear();
}

bool GopFile::ReadStrings(FixedMemoryStream &stream, uint64_t offset, const GENESTRTHeader &strt, std::vector<std::string> &strings)
{
    FixedMemoryStream in(stream.GetMemory(false) + offset, strt.str_size - 0x10);
    strings.resize(strt.num_strings);

    std::vector<uint32_t> idxs;
    idxs.resize(strt.num_strings);

    if (!in.Read(idxs.data(), strt.num_strings*sizeof(uint32_t)))
        return false;

    if (!in.Seek(strt.str_table_offset - 0x10, SEEK_SET))
        return false;

    for (std::string &str : strings)
    {
        if (!in.ReadCString(str))
            return false;
    }

    return true;
}

bool GopFile::ReadGmet(FixedMemoryStream &stream, uint64_t offset, size_t size, const std::vector<std::string> &strings)
{
    FixedMemoryStream in(stream.GetMemory(false) + offset, size);
    uint16_t column_name;

    while (in.Read16(&column_name))
    {
        if (column_name == 0)
            break;

        if ((size_t)column_name >= strings.size())
        {
            DPRINTF("%s: column_name out of bounds.\n", FUNCNAME);
            return false;
        }

        GopColumn column;
        column.name = strings[column_name];

        uint16_t num_params;
        if (!in.Read16(&num_params))
            return false;

        for (uint16_t i = 0; i < num_params; i++)
        {
            uint16_t type, value;

            if (!in.Read16(&type) || !in.Read16(&value))
                return false;

            if (type > 1)
            {
                DPRINTF("%s: Unrecognized type %d.\n", FUNCNAME, type);
                return false;
            }

            GopRow row;
            row.is_string = (type == 0);

            if (row.is_string)
            {
                if ((size_t)value >= strings.size())
                {
                    DPRINTF("%s: row value string out of bounds.\n", FUNCNAME);
                    return false;
                }

                row.val_str = strings[value];
            }
            else
            {
                row.val_u16 = value;
            }

            column.rows.push_back(row);
        }

        table.push_back(column);
    }

    return true;
}

bool GopFile::ReadGrec(FixedMemoryStream &stream, uint64_t offset, size_t size, const std::vector<std::string> &strings)
{
    FixedMemoryStream in(stream.GetMemory(false) + offset, size);
    uint32_t num_entries;

    if (!in.Read32(&num_entries))
        return false;

    grecs.resize(num_entries);

    for (GrecEntry &grec : grecs)
    {
        GRECEntry entry;

        if (!in.Read(&entry, sizeof(entry)))
            return false;

        grec.unk_00 = entry.unk_00;
        grec.unk_04 = entry.unk_04;
        grec.unk_0A = entry.unk_0A;

        if ((size_t)entry.name_idx >= strings.size())
        {
            DPRINTF("%s: name idx out of bounds.\n", FUNCNAME);
            return false;
        }

        grec.name = strings[entry.name_idx];
    }

    return true;
}

bool GopFile::ReadGdat(FixedMemoryStream &stream, uint64_t offset, size_t size, const std::vector<std::string> &strings, const std::vector<GopType> &types)
{
     FixedMemoryStream in(stream.GetMemory(false) + offset, size);

     uint32_t entry_size, num_entries, val_offset;

     if (!in.Read32(&entry_size) || !in.Read32(&num_entries) || !in.Seek(4, SEEK_CUR) || !in.Read32(&val_offset))
         return false;

     gdat_unks.resize(num_entries);
     if (!in.Read(gdat_unks.data(), num_entries*sizeof(uint32_t)))
         return false;

     if (!in.Seek(val_offset, SEEK_SET))
         return false;

     gdat_values.resize(num_entries);
     for (auto &v : gdat_values)
     {
         for (size_t i = 0; i < types.size(); i++)
         {
             GDatValue value;
             value.type = types[i];

             switch (types[i])
             {
                case GTYPE_RECID: case GTYPE_RECID2: case GTYPE_STRING:
                {
                    if (!in.SkipToAlignment(4))
                        return false;

                    uint32_t str_idx;
                    if (!in.Read32(&str_idx))
                        return false;

                    if ((size_t)str_idx >= strings.size())
                    {
                        DPRINTF("%s: str_idx out of bounds.\n", FUNCNAME);
                        return false;
                    }

                    value.value_str = strings[str_idx];
                }
                break;

                case GTYPE_UINT32:
                    if (!in.SkipToAlignment(4))
                        return false;

                    if (!in.Read32(&value.vals.u32))
                        return false;
                break;

                case GTYPE_RGBA:
                    if (!in.Read32(&value.vals.rgba))
                        return false;
                break;

                case GTYPE_INT16:
                    if (!in.SkipToAlignment(2))
                        return false;

                    if (!in.Read16(&value.vals.i16))
                        return false;
                break;

                case GTYPE_UINT16:
                    if (!in.SkipToAlignment(2))
                        return false;

                    if (!in.Read16(&value.vals.u16))
                        return false;
                break;

                case GTYPE_INT8:
                 if (!in.Read8(&value.vals.i8))
                     return false;
                break;

                case GTYPE_UINT8:
                 if (!in.Read8(&value.vals.u8))
                     return false;
                break;

                case GTYPE_BOOL:
                {
                    uint8_t b;

                    if (!in.Read8(&b))
                        return false;

                    if (b > 1)
                    {
                        DPRINTF("%s: Invalid bool value %d.\n", FUNCNAME, b);
                        return false;
                    }

                    value.vals.bval = (b != 0);
                }
                break;

                case GTYPE_FLOAT32:
                     if (!in.SkipToAlignment(4))
                         return false;

                     if (!in.ReadFloat(&value.vals.fval))
                         return false;
                break;

                case GTYPE_VECTOR3: case GTYPE_FRGB:
                    if (!in.SkipToAlignment(4))
                        return false;

                    if (!in.Read(value.vals.v3, sizeof(value.vals.v3)))
                        return false;
                break;

                case GTYPE_VECTOR4: case GTYPE_FRGBA:
                    if (!in.SkipToAlignment(4))
                        return false;

                    if (!in.Read(value.vals.v4, sizeof(value.vals.v4)))
                        return false;
                break;

                case GTYPE_IVECTOR4:
                    if (!in.SkipToAlignment(4))
                        return false;

                    if (!in.Read(value.vals.iv4, sizeof(value.vals.iv4)))
                        return false;
                break;

                default: // Integer and the gazillions of clones
                {
                    if (!in.SkipToAlignment(4))
                        return false;

                    if (!in.Read32(&value.vals.i32))
                        return false;
                }
             }

             v.push_back(value);
         }
     }

     return true;
}

static inline void AddString(std::unordered_map<std::string, uint32_t> &strings, std::map<uint32_t, std::string> &offsets, const std::string &str)
{
    auto it = strings.find(str);
    if (it == strings.end())
    {
        uint32_t id = (uint32_t)strings.size();
        strings[str] = id;
        //DPRINTF("Added \"%s\"\n", str.c_str());

        if (offsets.size() == 0)
        {
            offsets[0] = str;
        }
        else
        {
            auto it2 = offsets.rbegin();
            offsets[it2->first + (uint32_t)it2->second.length() + 1] = str;
        }
    }
}

#define ADDSTRING(str) AddString(strings, offsets, str)

bool GopFile::WriteStrings(Stream &out, std::unordered_map<std::string, uint32_t> &strings) const
{
    std::map<uint32_t, std::string> offsets;
    strings.clear();

    ADDSTRING("");
    ADDSTRING("SelfId");

    for (const GrecEntry &grec : grecs)
    {
        ADDSTRING(grec.name);
    }

    for (const auto &v : gdat_values)
    {
        for (const GDatValue &val : v)
        {
            if (val.type == GTYPE_STRING || val.type == GTYPE_RECID || val.type == GTYPE_RECID2)
            {
                ADDSTRING(val.value_str);
            }
        }
    }

    for (const GopColumn &col : table)
    {
        ADDSTRING(col.name);

        for (const GopRow &row: col.rows)
        {
            if (row.is_string)
                ADDSTRING(row.val_str);
        }
    }

    std::vector<GopType> types;
    if (!GetTypes(types))
        return false;

    ADDSTRING("ENDSWPSTR");
    ADDSTRING(BuildTypesString(types));

    uint64_t start = out.Tell();

    GENESTRTHeader hdr;
    hdr.num_strings = (uint32_t)strings.size();
    if (!out.Write(&hdr, sizeof(hdr)))
        return false;

    for (const auto &it : offsets)
    {
        if (!out.Write32(it.first))
            return false;
    }

    // Write one last offset
    auto lit = offsets.rbegin();
    if (!out.Write32(lit->first + (uint32_t)lit->second.length() + 1))
        return false;

    if (!out.Align(0x10))
        return false;

    hdr.str_table_offset = (uint32_t)(out.Tell() - start - 0x10);

    for (const auto &it : offsets)
    {
        if (!out.WriteString(it.second, true))
            return false;
    }

    if (!out.Align(0x10))
        return false;

    uint64_t end = out.Tell();
    hdr.str_size = hdr.str_size2 = (uint32_t)(end - start - 0x10);

    if (!out.Seek((off64_t)start, SEEK_SET) || !out.Write(&hdr, sizeof(hdr)))
        return false;

    return out.Seek((off64_t)end, SEEK_SET);
}

bool GopFile::WriteGmet(Stream &out, const std::unordered_map<std::string, uint32_t> &strings) const
{
    uint64_t start = out.Tell();
    GOPGMET gmet;
    if (!out.Write(&gmet, sizeof(gmet)))
        return false;

    for (const GopColumn &col : table)
    {
        auto it = strings.find(col.name);
        if (it == strings.end())
        {
            DPRINTF("%s: column name not found.\n", FUNCNAME);
            return false;
        }

        if (!out.Write16((uint16_t)it->second) || !out.Write16((uint16_t)col.rows.size()))
            return false;

        for (const GopRow &row : col.rows)
        {
            if (row.is_string)
            {
                auto it2 = strings.find(row.val_str);
                if (it2 == strings.end())
                {
                    DPRINTF("%s: row string value not found.\n", FUNCNAME);
                    return false;
                }

                if (!out.Write16(0) || !out.Write16((uint16_t)it2->second))
                    return false;
            }
            else
            {
                if (!out.Write16(1) || !out.Write16(row.val_u16))
                    return false;
            }
        }
    }

    if (!out.Align(0x10))
        return false;

    uint64_t end = out.Tell();
    gmet.size = (uint32_t)(end - start - 0x10);

    if (!out.Seek((off64_t)start, SEEK_SET) || !out.Write(&gmet, sizeof(gmet)))
        return false;

    return out.Seek((off64_t)end, SEEK_SET);
}

bool GopFile::WriteGrec(Stream &out, const std::unordered_map<std::string, uint32_t> &strings) const
{
    uint64_t start = out.Tell();
    GOPGREC grec;
    if (!out.Write(&grec, sizeof(grec)))
        return false;

    if (!out.Write32((uint32_t)grecs.size()))
        return false;

    for (const GrecEntry &grec : grecs)
    {
        GRECEntry entry;

        auto it = strings.find(grec.name);
        if (it == strings.end())
        {
            DPRINTF("%s: cannot find grec name.\n", FUNCNAME);
            return false;
        }

        entry.name_idx = (uint16_t)it->second;
        entry.unk_00 = grec.unk_00;
        entry.unk_04 = grec.unk_04;
        entry.unk_0A = grec.unk_0A;

        if (!out.Write(&entry, sizeof(entry)))
            return false;
    }

    if (!out.Align(0x10))
        return false;

    uint64_t end = out.Tell();
    grec.size = (uint32_t)(end - start - 0x10);

    if (!out.Seek((off64_t)start, SEEK_SET) || !out.Write(&grec, sizeof(grec)))
        return false;

    return out.Seek((off64_t)end, SEEK_SET);
}

bool GopFile::WriteGdat(Stream &out, const std::unordered_map<std::string, uint32_t> &strings) const
{
    uint64_t start = out.Tell();
    GOPGDAT gdat;
    if (!out.Write(&gdat, sizeof(gdat)))
        return false;

    std::vector<GopType> types;
    if (!GetTypes(types))
        return false;

    uint32_t data_offset = (uint32_t)(0x10 + Utils::Align2(gdat_unks.size()*sizeof(uint32_t), 0x10));
    if (!out.Write32(CalculateGDatValueSize(types)) || !out.Write32((uint32_t)gdat_values.size()))
        return false;

    if (!out.Write32(0x10) || !out.Write32(data_offset))
        return false;

    if (!out.Write(gdat_unks.data(), gdat_unks.size()*sizeof(uint32_t)))
        return false;

    if (!out.Align(0x10))
        return false;

    for (const auto &v : gdat_values)
    {
        if (v.size() != types.size())
        {
            DPRINTF("%s: mismatch between types size and gdat values size.\n", FUNCNAME);
            return false;
        }

        for (size_t i = 0; i < types.size(); i++)
        {
            const GDatValue &value = v[i];

            switch (types[i])
            {
               case GTYPE_RECID: case GTYPE_RECID2: case GTYPE_STRING:
               {
                   if (!out.Align(4))
                       return false;

                   auto it = strings.find(value.value_str);
                   if (it == strings.end())
                   {
                       DPRINTF("%s: cannot find gdat value string \"%s\"\n", FUNCNAME, value.value_str.c_str());
                       return false;
                   }

                   if (!out.Write32(it->second))
                       return false;
               }
               break;

               case GTYPE_UINT32:
                   if (!out.Align(4))
                       return false;

                   if (!out.Write32(value.vals.u32))
                       return false;
               break;

               case GTYPE_RGBA:
                   if (!out.Write32(value.vals.rgba))
                       return false;
               break;

               case GTYPE_INT16:
                   if (!out.Align(2))
                       return false;

                   if (!out.Write16((uint16_t)value.vals.i16))
                       return false;
               break;

               case GTYPE_UINT16:
                   if (!out.Align(2))
                       return false;

                   if (!out.Write16(value.vals.u16))
                       return false;
               break;

               case GTYPE_INT8:
                if (!out.Write8((uint8_t)value.vals.i8))
                    return false;
               break;

               case GTYPE_UINT8:
                if (!out.Write8(value.vals.u8))
                    return false;
               break;

               case GTYPE_BOOL:
               {
                   if (!out.Write8((value.vals.bval ? 1 : 0)))
                       return false;
               }
               break;

               case GTYPE_FLOAT32:
                    if (!out.Align(4))
                        return false;

                    if (!out.WriteFloat(value.vals.fval))
                        return false;
               break;

               case GTYPE_VECTOR3: case GTYPE_FRGB:
                   if (!out.Align(4))
                       return false;

                   if (!out.Write(value.vals.v3, sizeof(value.vals.v3)))
                       return false;
               break;

               case GTYPE_VECTOR4: case GTYPE_FRGBA:
                   if (!out.Align(4))
                       return false;

                   if (!out.Write(value.vals.v4, sizeof(value.vals.v4)))
                       return false;
               break;

               case GTYPE_IVECTOR4:
                   if (!out.Align(4))
                       return false;

                   if (!out.Write(value.vals.iv4, sizeof(value.vals.iv4)))
                       return false;
               break;

               default: // Integer and the gazillions of clones
               {
                   if (!out.Align(4))
                       return false;

                   if (!out.Write32((uint32_t)value.vals.i32))
                       return false;
               }
            }
        }

        if (!out.Align(4))
            return false;
    }

    if (!out.Align(0x10))
        return false;

    uint64_t end = out.Tell();
    gdat.size = (uint32_t)(end - start - 0x10);

    if (!out.Seek((off64_t)start, SEEK_SET) || !out.Write(&gdat, sizeof(gdat)))
        return false;

    return out.Seek((off64_t)end, SEEK_SET);
}

TiXmlElement *GopFile::DecompileGmet(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("GMET");

    for (const GopColumn &col : table)
    {
        TiXmlElement *col_root = new TiXmlElement("Column");
        col_root->SetAttribute("name", col.name);

        std::string types, values;
        for (size_t i = 0; i < col.rows.size(); i++)
        {
            const GopRow &row = col.rows[i];

            if (row.is_string)
            {
                types.push_back('s');
                values += (row.val_str.length() == 0) ? "NULL" : row.val_str;
            }
            else
            {
               types.push_back('i') ;
               values += Utils::UnsignedToString(row.val_u16, false);
            }

            if (i != (col.rows.size()-1))
                values += "|";
        }

        col_root->SetAttribute("row_types", types);
        col_root->SetAttribute("rows", values);

        entry_root->LinkEndChild(col_root);
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

TiXmlElement *GopFile::DecompileGrec(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("GREC");

    for (const GrecEntry &rec : grecs)
    {
        TiXmlElement *rec_root = new TiXmlElement("Rec");
        rec_root->SetAttribute("name", rec.name);
        rec_root->SetAttribute("u00", Utils::UnsignedToString(rec.unk_00, false));
        rec_root->SetAttribute("u04", Utils::UnsignedToString(rec.unk_04, false));
        rec_root->SetAttribute("u0a", Utils::UnsignedToString(rec.unk_0A, false));
        entry_root->LinkEndChild(rec_root);
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

TiXmlElement *GopFile::DecompileGdat(TiXmlNode *root) const
{
    std::vector<std::string> names, infos;

    if (!GetNames(names))
    {
        DPRINTF("%s: GetNames failed.\n", FUNCNAME);
        return nullptr;
    }

    if (!GetInfos(infos))
    {
        DPRINTF("%s: GetNames failed.\n", FUNCNAME);
        return nullptr;
    }

    if (names.size() != infos.size())
    {
        DPRINTF("%s names - infos size mismatch.\n", FUNCNAME);
        return nullptr;
    }

    TiXmlElement *entry_root = new TiXmlElement("GDAT");
    Utils::WriteComment(entry_root, "Note: the type, name and info attributes are only informative and will be ignored in compilation. Their values are actually set in the GMET section.");

    for (size_t i = 0; i < gdat_values.size(); i++)
    {
        TiXmlElement *data_root = new TiXmlElement("Data");
        data_root->SetAttribute("unk", Utils::UnsignedToString(gdat_unks[i], false));

        if (gdat_values[i].size() != names.size())
        {
            DPRINTF("%s: gdat size mismatch with names/infos.\n", FUNCNAME);
            delete entry_root; delete data_root;
            return nullptr;
        }

        for (size_t j = 0; j < gdat_values[i].size(); j++)
        {
            const GDatValue &value = gdat_values[i][j];
            TiXmlElement *param_root = new TiXmlElement("Param");
            std::string type_str = TypeToString(value.type);

            if (type_str == "")
            {
                DPRINTF("%s: Internal error.\n", FUNCNAME);
                delete entry_root; delete data_root;
                return nullptr;
            }

            std::string val_str;

            switch (value.type)
            {
                case GTYPE_RECID: case GTYPE_RECID2: case GTYPE_STRING:
                    val_str = value.value_str;
                break;

                case GTYPE_UINT32:
                    val_str = Utils::UnsignedToString(value.vals.u32, false);
                break;

                case GTYPE_RGBA:
                    val_str = Utils::UnsignedToHexString(value.vals.rgba, true);
                break;

                case GTYPE_INT16:
                    val_str = Utils::SignedToString(value.vals.i16);
                break;

                case GTYPE_UINT16:
                    val_str = Utils::UnsignedToString(value.vals.u16, false);
                break;

                case GTYPE_INT8:
                    val_str = Utils::SignedToString(value.vals.i8);
                break;

                case GTYPE_UINT8:
                    val_str = Utils::UnsignedToString(value.vals.u8, false);
                break;

                case GTYPE_BOOL:
                    val_str = (value.vals.bval) ? "true" : "false";
                break;

                case GTYPE_FLOAT32:
                     val_str = Utils::FloatToString(value.vals.fval);
                break;

                case GTYPE_VECTOR3: case GTYPE_FRGB:
                {
                    for (size_t i = 0; i < 3; i++)
                    {
                        val_str += Utils::FloatToString(value.vals.v3[i]);
                        if (i != 2)
                            val_str += ",";
                    }
                }
                break;

                case GTYPE_VECTOR4: case GTYPE_FRGBA:
                {
                    for (size_t i = 0; i < 4; i++)
                    {
                        val_str += Utils::FloatToString(value.vals.v4[i]);
                        if (i != 3)
                            val_str += ",";
                    }
                }
                break;

                case GTYPE_IVECTOR4:
                {
                    for (size_t i = 0; i < 4; i++)
                    {
                        val_str += Utils::SignedToString(value.vals.iv4[i]);
                        if (i != 3)
                            val_str += ",";
                    }
                }
                break;

                default: // Integer and the gazillions of clones
                {
                    val_str = Utils::SignedToString(value.vals.i32);
                }
            }

            param_root->SetAttribute("value", val_str);
            param_root->SetAttribute("type", type_str);
            param_root->SetAttribute("name", names[j]);
            param_root->SetAttribute("info", infos[j]);

            data_root->LinkEndChild(param_root);
        }

        entry_root->LinkEndChild(data_root);
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool GopFile::CompileGmet(const TiXmlElement *root)
{
    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Column")
        {
            GopColumn col;
            std::string types;
            std::vector<std::string> rows;

            if (!Utils::ReadAttrString(elem, "name", col.name))
            {
                DPRINTF("%s: failed to read attribute \"name\", at line %d.\n", FUNCNAME, elem->Row());
                return false;
            }

            if (!Utils::ReadAttrString(elem, "row_types", types))
            {
                DPRINTF("%s: failed to read attribute \"row_types\", at line %d.\n", FUNCNAME, elem->Row());
                return false;
            }

            if (!Utils::ReadAttrMultipleStrings(elem, "rows", rows, '|', false))
            {
                DPRINTF("%s: failed to read attribute \"rows\", at line %d.\n", FUNCNAME, elem->Row());
                return false;
            }

            if (types.length() != rows.size())
            {
                DPRINTF("%s: Error: \"row_types\" needs to have same number of elements than \"rows\". At line %d.\n", FUNCNAME, elem->Row());
                return false;
            }

            col.rows.resize(rows.size());

            for (size_t i = 0; i < rows.size(); i++)
            {
                GopRow &row = col.rows[i];

                if (types[i] == 'i')
                {
                    uint32_t val = Utils::GetUnsigned(rows[i], 0x10000);
                    if (val >= 0x10000)
                    {
                        DPRINTF("%s: a number value is either not a number or too large for 16bit, at line %d.\n", FUNCNAME, elem->Row());
                        return false;
                    }

                    if (col.name == "TYPE")
                    {
                        DPRINTF("%s: the \"row_types\" for column \"TYPE\" must be all 's', at line %d.\n", FUNCNAME, elem->Row());
                        return false;
                    }

                    row.is_string = false;
                    row.val_u16 = (uint16_t)val;
                }
                else if (types[i] == 's')
                {
                    row.is_string = true;
                    row.val_str = rows[i];
                }
                else
                {
                    DPRINTF("%s: Unrecognized type '%c', at line %d.\n", FUNCNAME, types[i], elem->Row());
                    return false;
                }
            }

            table.push_back(col);
        }
    }

    GopColumn col;
    if (!GetColumn("TYPE", col))
    {
        DPRINTF("%s: Error: \"TYPE\" column is mandatory.\n", FUNCNAME);
        return false;
    }

    return true;
}

bool GopFile::CompileGrec(const TiXmlElement *root)
{
    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Rec")
        {
            GrecEntry rec;

            if (!Utils::ReadAttrString(elem, "name", rec.name))
            {
                DPRINTF("%s: Failed to read attribute \"name\", at line %d.\n", FUNCNAME, elem->Row());
                return false;
            }

            if (!Utils::ReadAttrUnsigned(elem, "u00", &rec.unk_00))
            {
                DPRINTF("%s: Failed to read attribute \"u00\", at line %d.\n", FUNCNAME, elem->Row());
                return false;
            }

            if (!Utils::ReadAttrUnsigned(elem, "u04", &rec.unk_04))
            {
                DPRINTF("%s: Failed to read attribute \"u04\", at line %d.\n", FUNCNAME, elem->Row());
                return false;
            }

            if (!Utils::ReadAttrUnsigned(elem, "u0a", &rec.unk_0A))
            {
                DPRINTF("%s: Failed to read attribute \"u0a\", at line %d.\n", FUNCNAME, elem->Row());
                return false;
            }

            grecs.push_back(rec);
        }
    }

    return true;
}

bool GopFile::CompileGdatParams(const TiXmlElement *root, std::vector<GDatValue> &values, const std::vector<GopType> &types)
{
    size_t i = 0;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Param")
        {
            if (i >= values.size())
            {
                DPRINTF("%s: The number of \"Param\" must be the same than the number of \"TYPE\" in \"GMET\", at line %d.\n", FUNCNAME, root->Row());
                return false;
            }

            GDatValue value;
            value.type = types[i];

            switch (types[i])
            {
                case GTYPE_STRING: case GTYPE_RECID: case GTYPE_RECID2:
                    if (!Utils::ReadAttrString(elem, "value", value.value_str))
                    {
                        DPRINTF("%s: Failed to read String value, at line %d.\n", FUNCNAME, elem->Row());
                        return false;
                    }
                break;

                case GTYPE_INT16:
                    if (!Utils::ReadAttrSigned(elem, "value", &value.vals.i16))
                    {
                        DPRINTF("%s: Failed to read Int16 value, at line %d.\n", FUNCNAME, elem->Row());
                        return false;
                    }
                break;

                case GTYPE_INT8:
                    if (!Utils::ReadAttrSigned(elem, "value", &value.vals.i8))
                    {
                        DPRINTF("%s: Failed to read Int8 value, at line %d.\n", FUNCNAME, elem->Row());
                        return false;
                    }
                break;

                case GTYPE_UINT32:
                    if (!Utils::ReadAttrUnsigned(elem, "value", &value.vals.u32))
                    {
                        DPRINTF("%s: Failed to read UInt32 value, at line %d.\n", FUNCNAME, elem->Row());
                        return false;
                    }
                break;

                case GTYPE_UINT16:
                    if (!Utils::ReadAttrUnsigned(elem, "value", &value.vals.u16))
                    {
                        DPRINTF("%s: Failed to read UInt16 value, at line %d.\n", FUNCNAME, elem->Row());
                        return false;
                    }
                break;

                case GTYPE_UINT8:
                    if (!Utils::ReadAttrUnsigned(elem, "value", &value.vals.u8))
                    {
                        DPRINTF("%s: Failed to read UInt8 value, at line %d.\n", FUNCNAME, elem->Row());
                        return false;
                    }
                break;

                case GTYPE_BOOL:
                    if (!Utils::ReadAttrBoolean(elem, "value", &value.vals.bval))
                    {
                        DPRINTF("%s: Failed to read Bool value, at line %d.\n", FUNCNAME, elem->Row());
                        return false;
                    }
                break;

                case GTYPE_FLOAT32:
                    if (!Utils::ReadAttrFloat(elem, "value", &value.vals.fval))
                    {
                        DPRINTF("%s: Failed to read Float32 value, at line %d.\n", FUNCNAME, elem->Row());
                        return false;
                    }
                break;

                case GTYPE_RGBA:
                    if (!Utils::ReadAttrUnsigned(elem, "value", &value.vals.rgba))
                    {
                        DPRINTF("%s: Failed to read Rgba value, at line %d.\n", FUNCNAME, elem->Row());
                        return false;
                    }
                break;

                case GTYPE_VECTOR3: case GTYPE_FRGB:
                    if (!Utils::ReadAttrMultipleFloats(elem, "value", value.vals.v3, 3))
                    {
                        DPRINTF("%s: Failed to read Vector3 or Frgb value, at line %d.\n", FUNCNAME, elem->Row());
                        return false;
                    }
                break;

                case GTYPE_VECTOR4: case GTYPE_FRGBA:
                    if (!Utils::ReadAttrMultipleFloats(elem, "value", value.vals.v4, 4))
                    {
                        DPRINTF("%s: Failed to read Vector4 or Frgba value, at line %d.\n", FUNCNAME, elem->Row());
                        return false;
                    }
                break;

                case GTYPE_IVECTOR4:
                    if (!Utils::ReadAttrMultipleSigned(elem, "value", value.vals.iv4, 4))
                    {
                        DPRINTF("%s: Failed to read IVector4 value, at line %d.\n", FUNCNAME, elem->Row());
                        return false;
                    }
                break;

                default:
                    if (!Utils::ReadAttrSigned(elem, "value", &value.vals.i32))
                    {
                        DPRINTF("%s: Failed to read Int32 (or similar) value, at line %d.\n", FUNCNAME, elem->Row());
                        return false;
                    }
            }

            values[i++] = value;
        }
    }

    if (i < values.size())
    {
        DPRINTF("%s: The number of \"Param\" must be the same than the number of \"TYPE\" in \"GMET\", at line %d.\n", FUNCNAME, root->Row());
        return false;
    }

    return true;
}

bool GopFile::CompileGdat(const TiXmlElement *root)
{
    std::vector<GopType> types;
    if (!GetTypes(types))
        return false;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Data")
        {
            uint32_t unk;
            if (!Utils::ReadAttrUnsigned(elem, "unk", &unk))
            {
                DPRINTF("%s: Failed to read attribute \"unk\", at line %d.\n", FUNCNAME, elem->Row());
                return false;
            }

            gdat_unks.push_back(unk);
            std::vector<GDatValue> values;
            values.resize(types.size());

            if (!CompileGdatParams(elem, values, types))
                return false;

            gdat_values.push_back(values);
        }
    }

    return true;
}

GopType GopFile::StringToType(const std::string &str) const
{
    auto it = str_to_gtype.find(str);
    if (it != str_to_gtype.end())
        return it->second;

    return GTYPE_NONE;
}

std::string GopFile::TypeToString(GopType type) const
{
    auto it = gtype_to_str.find(type);
    if (it != gtype_to_str.end())
        return it->second;

    return "";
}

bool GopFile::GetTypes(std::vector<GopType> &types) const
{
    types.clear();

    GopColumn tcol;
    if (!GetColumn("TYPE", tcol))
        return false;

    for (const GopRow &row: tcol.rows)
    {
        if (!row.is_string)
            return false;

        GopType type = StringToType(row.val_str);
        if (type != GTYPE_NONE)
        {
            types.push_back(type);
        }
        else
        {
            DPRINTF("%s: unrecognized type \"%s\"\n", FUNCNAME, row.val_str.c_str());

            DPRINTF("Signature is ");
            for (size_t i = 0; i < tcol.rows.size(); i++)
            {
                DPRINTF("%s", tcol.rows[i].val_str.c_str());

                if (i == (tcol.rows.size()-1))
                {
                    DPRINTF("\n");
                }
                else
                {
                    DPRINTF(", ");
                }
            }

            return false;
        }
    }

    return true;
}

bool GopFile::GetNames(std::vector<std::string> &names) const
{
    names.clear();

    GopColumn ncol;
    if (!GetColumn("NAME", ncol))
        return false;

    for (const GopRow &row: ncol.rows)
    {
        if (!row.is_string)
            return false;

        names.push_back(row.val_str);
    }

    return true;
}

bool GopFile::GetInfos(std::vector<std::string> &infos) const
{
    infos.clear();

    GopColumn icol;
    if (!GetColumn("INFO", icol))
        return false;

    for (const GopRow &row: icol.rows)
    {
        if (!row.is_string)
            return false;

        infos.push_back(row.val_str);
    }

    return true;
}

std::string GopFile::BuildTypesString(const std::vector<GopType> types) const
{
    std::string str;
    uint32_t size = 0;

    for (const GopType type : types)
    {
        switch (type)
        {
            case GTYPE_RGBA:
                str += "4(c)";
                size += 4;
            break;

            case GTYPE_INT16: case GTYPE_UINT16:
                if ((size % 2) != 0)
                {
                    str.push_back('c');
                    size++;
                }

                str.push_back('s');
                size += 2;
            break;

            case GTYPE_INT8: case GTYPE_UINT8: case GTYPE_BOOL:
                str.push_back('c');
                size++;
            break;

            case GTYPE_VECTOR3: case GTYPE_FRGB:
                switch ((size % 4))
                {
                    case 1:
                        str += "3c";
                        size += 3;
                    break;

                    case 2:
                        str += "2c";
                        size += 2;
                    break;

                    case 3:
                        str.push_back('c');
                        size++;
                    break;
                }

                str += "3(f)";
                size += 12;
            break;

            case GTYPE_VECTOR4: case GTYPE_FRGBA:
                switch ((size % 4))
                {
                    case 1:
                        str += "3c";
                        size += 3;
                    break;

                    case 2:
                        str += "2c";
                        size += 2;
                    break;

                    case 3:
                        str.push_back('c');
                        size++;
                    break;
                }

                str += "4(f)";
                size += 16;
            break;

            case GTYPE_IVECTOR4:
                switch ((size % 4))
                {
                    case 1:
                        str += "3c";
                        size += 3;
                    break;

                    case 2:
                        str += "2c";
                        size += 2;
                    break;

                    case 3:
                        str.push_back('c');
                        size++;
                    break;
                }

                str += "4(i)";
                size += 16;
            break;

            default:
            {
                switch ((size % 4))
                {
                    case 1:
                        str += "3c";
                        size += 3;
                    break;

                    case 2:
                        str += "2c";
                        size += 2;
                    break;

                    case 3:
                        str.push_back('c');
                        size++;
                    break;
                }

                str.push_back('i');
                size += 4;
            }
        }
    }

    switch ((size % 4))
    {
        case 1:
            str += "3c";
        break;

        case 2:
            str += "2c";
        break;

        case 3:
            str.push_back('c');
        break;
    }

    return str;
}

uint32_t GopFile::CalculateGDatValueSize(const std::vector<GopType> types) const
{
    uint32_t size = 0;

    for (const GopType type : types)
    {
        switch (type)
        {
            case GTYPE_RGBA:
                size += 4;
            break;

            case GTYPE_INT16: case GTYPE_UINT16:
                if ((size % 2) != 0)
                {
                    size++;
                }

                size += 2;
            break;

            case GTYPE_INT8: case GTYPE_UINT8: case GTYPE_BOOL:
                size++;
            break;

            case GTYPE_VECTOR3: case GTYPE_FRGB:
                switch ((size % 4))
                {
                    case 1:
                        size += 3;
                    break;

                    case 2:
                        size += 2;
                    break;

                    case 3:
                        size++;
                    break;
                }

                size += 12;
            break;

            case GTYPE_VECTOR4: case GTYPE_FRGBA:
                switch ((size % 4))
                {
                    case 1:
                        size += 3;
                    break;

                    case 2:
                        size += 2;
                    break;

                    case 3:
                        size++;
                    break;
                }

                size += 16;
            break;

            case GTYPE_IVECTOR4:
                switch ((size % 4))
                {
                    case 1:
                        size += 3;
                    break;

                    case 2:
                        size += 2;
                    break;

                    case 3:
                        size++;
                    break;
                }

                size += 16;
            break;

            default:
            {
                switch ((size % 4))
                {
                    case 1:
                        size += 3;
                    break;

                    case 2:
                        size += 2;
                    break;

                    case 3:
                        size++;
                    break;
                }

                size += 4;
            }
        }
    }

    switch ((size % 4))
    {
        case 1:
            size += 3;
        break;

        case 2:
            size += 2;
        break;

        case 3:
            size++;
        break;
    }

    return size;
}

bool GopFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    FixedMemoryStream stream(const_cast<uint8_t *>(buf), size);
    GOPGFIN gfin;

    if (!stream.Read(&gfin, sizeof(gfin)))
        return false;

    if (memcmp(gfin.signature, GOPGFIN_SIGNATURE, 8) != 0)
    {
        DPRINTF("%s: unrecognized GOPGFIN signature.\n", FUNCNAME);
        return false;
    }

    uint64_t gmet_offset = gfin.size + 0x10;
    if (!stream.Seek((off64_t)gmet_offset, SEEK_SET))
        return false;

    GOPGMET gmet;
    if (!stream.Read(&gmet, sizeof(gmet)))
        return false;

    if (memcmp(gmet.signature, GOPGMET_SIGNATURE, 8) != 0)
    {
        DPRINTF("%s: unrecognized GOPGMET signature.\n", FUNCNAME);
        return false;
    }

    uint64_t grec_offset = gmet_offset + gmet.size + 0x10;
    if (!stream.Seek((off64_t)grec_offset, SEEK_SET))
        return false;

    GOPGREC grec;
    if (!stream.Read(&grec, sizeof(grec)))
        return false;

    if (memcmp(grec.signature, GOPGREC_SIGNATURE, 8) != 0)
    {
        DPRINTF("%s: unrecognized GOPGREC signature.\n", FUNCNAME);
        return false;
    }

    uint64_t strt_offset = grec_offset + grec.size + 0x10;
    if (!stream.Seek((off64_t)strt_offset, SEEK_SET))
        return false;

    GENESTRTHeader strt;
    if (!stream.Read(&strt, sizeof(strt)))
        return false;

    if (memcmp(strt.signature, GENESTRT_SIGNATURE, 8) != 0)
    {
        DPRINTF("%s: unrecognized GENESTRT signature.\n", FUNCNAME);
        return false;
    }

    uint64_t gdat_offset = strt_offset + strt.str_size + 0x10;
    if (!stream.Seek((off64_t)gdat_offset, SEEK_SET))
        return false;

    GOPGDAT gdat;
    if (!stream.Read(&gdat, sizeof(gdat)))
        return false;

    if (memcmp(gdat.signature, GOPGDAT_SIGNATURE, 8) != 0)
    {
        DPRINTF("%s: unrecognized GOPGDAT signature.\n", FUNCNAME);
        return false;
    }

    std::vector<std::string> strings;
    if (!ReadStrings(stream, strt_offset + sizeof(GENESTRTHeader), strt, strings))
        return false;

    if (!ReadGmet(stream, gmet_offset + 0x10, (size_t)gmet.size, strings))
        return false;

    if (!ReadGrec(stream, grec_offset + 0x10, (size_t)grec.size, strings))
        return false;

    std::vector<GopType> types;
    if (!GetTypes(types))
        return false;

    if (!ReadGdat(stream, gdat_offset + 0x10, (size_t)gdat.size, strings, types))
        return false;

    return true;
}

uint8_t *GopFile::Save(size_t *psize)
{
    MemoryStream out, str_stream;
    std::unordered_map<std::string, uint32_t> strings;

    GOPGFIN gfin;
    if (!out.Write(&gfin, sizeof(gfin)))
        return nullptr;

    if (!WriteStrings(str_stream, strings))
        return nullptr;

    if (!WriteGmet(out, strings))
        return nullptr;

    if (!WriteGrec(out, strings))
        return nullptr;

    if (!out.Write(str_stream.GetMemory(false), (size_t)str_stream.GetSize()))
        return nullptr;

    if (!WriteGdat(out, strings))
        return nullptr;

    if (!out.WriteString("GENEEOF ") || !out.Align(0x10))
        return nullptr;

    *psize = (size_t)out.GetSize();
    return out.GetMemory(true);
}

TiXmlDocument *GopFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("GOP");

    if (!DecompileGmet(root) || !DecompileGrec(root) || !DecompileGdat(root))
        return nullptr;

    doc->LinkEndChild(root);
    return doc;
}

bool GopFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "GOP");

    if (!root)
    {
        DPRINTF("Cannot find\"GOP\" in xml.\n");
        return false;
    }

    // GMET must be compiled before
    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "GMET")
        {
            if (!CompileGmet(elem))
                return false;

            break;
        }
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "GREC")
        {
            if (!CompileGrec(elem))
                return false;
        }
        else if (elem->ValueStr() == "GDAT")
        {
            if (!CompileGdat(elem))
                return false;
        }
    }

    return true;
}

bool GopFile::GetColumn(const std::string &name, GopColumn &column) const
{
    for (const GopColumn &col : table)
    {
        if (col.name == name)
        {
            column = col;
            return true;
        }
    }

    return false;
}
