#ifndef GOPFILE_H
#define GOPFILE_H

#include <unordered_map>
#include "ApkFile.h"

#define GOPGFIN_SIGNATURE   "GOP GFIN"
#define GOPGMET_SIGNATURE   "GOP GMET"
#define GOPGREC_SIGNATURE   "GOP GREC"
#define GOPGDAT_SIGNATURE   "GOP GDAT"

;
#pragma pack(push,1)

struct GOPGFIN
{
    char signature[8]; // 0
    uint64_t size; // 8 - size minus 0x10
    uint32_t unk_10;
    uint32_t unk_14;
    uint64_t unk_18;

    GOPGFIN()
    {
        memcpy(signature, GOPGFIN_SIGNATURE, 8);
        size = 0x10;
        unk_10 = 2;
        unk_14 = 1;
        unk_18 = 0;
    }
};
CHECK_STRUCT_SIZE(GOPGFIN, 0x20);

struct GOPGMET
{
    char signature[8]; // 0
    uint64_t size; // 8 - size of section minus 0x10

    GOPGMET()
    {
        memcpy(signature, GOPGMET_SIGNATURE, 8);
        size = 0;
    }
};
CHECK_STRUCT_SIZE(GOPGMET, 0x10);

struct GOPGREC
{
    char signature[8]; // 0
    uint64_t size; // 8 - size of section minus 0x10

    GOPGREC()
    {
        memcpy(signature, GOPGREC_SIGNATURE, 8);
        size = 0;
    }
};
CHECK_STRUCT_SIZE(GOPGREC, 0x10);

struct GOPGDAT
{
    char signature[8]; // 0
    uint64_t size; // 8 - size of section minus 0x10

    GOPGDAT()
    {
        memcpy(signature, GOPGDAT_SIGNATURE, 8);
        size = 0;
    }
};
CHECK_STRUCT_SIZE(GOPGDAT, 0x10);

struct GRECEntry
{
    uint32_t unk_00;
    uint32_t unk_04;
    uint16_t name_idx; // 8
    uint16_t unk_0A;
};
CHECK_STRUCT_SIZE(GRECEntry, 0xC);

#pragma pack(pop)

enum GopType
{
    GTYPE_NONE,
    GTYPE_RECID,
    GTYPE_RECID2, // Always zero?
    GTYPE_STRING,
    GTYPE_INT32,
    GTYPE_INT16,
    GTYPE_INT8,
    GTYPE_UINT32,
    GTYPE_UINT16,
    GTYPE_UINT8,
    GTYPE_IVECTOR4,
    GTYPE_BOOL,
    GTYPE_FLOAT32,
    GTYPE_VECTOR3,
    GTYPE_VECTOR4,
    GTYPE_RGBA,
    GTYPE_FRGB,
    GTYPE_FRGBA,
    GTYPE_THUMBNAIL,
    GTYPE_MAGIC,
    GTYPE_VOICE,
    GTYPE_MSAVOIDAI,
    GTYPE_AVOIDTARGET,
    GTYPE_DIRECTION,
    GTYPE_AVOIDACTION,
    GTYPE_MSATKCTRLAI,
    GTYPE_MSGRAPATKAI,
    GTYPE_MSMAINATKAI,
    GTYPE_MSMOVEAI,
    GTYPE_MSSCHEMEAI,
    GTYPE_MSSUBATKAI,
    GTYPE_CHARACTERTYPE,
    GTYPE_FIELDPHYSICSMODE,
    GTYPE_BTN,
    GTYPE_SKILL,
    GTYPE_FREECATEGORY,
    GTYPE_VSCATEGORY,
    GTYPE_TERRITORYID,
    GTYPE_UNITSIZEID,
    GTYPE_SKILLINVOKECHECK,
    GTYPE_SKILLAPPLY,
    GTYPE_PRIZEIF,
    GTYPE_PRIZEOP,
    GTYPE_PRIZELF,
    GTYPE_SERESOURCEID,
    GTYPE_SECATEGORY,
    GTYPE_STAFFROLLLAYOUT,
    GTYPE_REDER,
    GTYPE_UNITLOCKPRIORITY,
    GTYPE_UNITLOCKCATEGORY,
    GTYPE_SHADOWDRAW,
    GTYPE_GIMMICK,
    GTYPE_DROPITEM,
    GTYPE_UNITCOL,
    GTYPE_TARGET,
    GTYPE_FORM,
    GTYPE_MSMOTIONGRAPPLEOFFSET,
    GTYPE_MSMOTIONID
};

struct GopRow
{
    bool is_string;
    uint16_t val_u16;
    std::string val_str;
};

struct GopColumn
{
    std::string name;
    std::vector<GopRow> rows;
};

struct GrecEntry
{
    uint32_t unk_00;
    uint32_t unk_04;
    std::string name;
    uint16_t unk_0A;
};

struct GDatValue
{
    GopType type;
    std::string value_str; // Also for rec

    union
    {
        int32_t i32; // Also for thumbnail, magic, voice, etc
        uint32_t u32;
        uint32_t rgba; // Difference between this and above, is that this one doesn't require alignment
        int16_t i16;
        uint16_t u16;
        int8_t  i8;
        uint8_t u8;
        bool    bval;
        float   fval;
        float   v3[3]; // Also for Fgrb
        float   v4[4]; // Also for Frgba
        int32_t iv4[4];
    } vals;
};

class GopFile : public BaseFile
{
private:

    std::vector<GopColumn> table;
    std::vector<GrecEntry> grecs;
    std::vector<uint32_t> gdat_unks;
    std::vector<std::vector<GDatValue>> gdat_values; // Must be same size as above

    bool ReadStrings(FixedMemoryStream &stream, uint64_t offset, const GENESTRTHeader &strt, std::vector<std::string> &strings);
    bool ReadGmet(FixedMemoryStream &stream, uint64_t offset, size_t size, const std::vector<std::string> &strings);
    bool ReadGrec(FixedMemoryStream &stream, uint64_t offset, size_t size, const std::vector<std::string> &strings);
    bool ReadGdat(FixedMemoryStream &stream, uint64_t offset, size_t size, const std::vector<std::string> &strings, const std::vector<GopType> &types);

    bool WriteStrings(Stream &out, std::unordered_map<std::string, uint32_t> &strings) const;
    bool WriteGmet(Stream &out, const std::unordered_map<std::string, uint32_t> &strings) const;
    bool WriteGrec(Stream &out, const std::unordered_map<std::string, uint32_t> &strings) const;
    bool WriteGdat(Stream &out, const std::unordered_map<std::string, uint32_t> &strings) const;

    TiXmlElement *DecompileGmet(TiXmlNode *root) const;
    TiXmlElement *DecompileGrec(TiXmlNode *root) const;
    TiXmlElement *DecompileGdat(TiXmlNode *root) const;

    bool CompileGmet(const TiXmlElement *root);
    bool CompileGrec(const TiXmlElement *root);
    bool CompileGdatParams(const TiXmlElement *root, std::vector<GDatValue> &values, const std::vector<GopType> &types);
    bool CompileGdat(const TiXmlElement *root);

    GopType StringToType(const std::string &str) const;
    std::string TypeToString(GopType type) const;

    bool GetTypes(std::vector<GopType> &types) const;
    bool GetNames(std::vector<std::string> &names) const;
    bool GetInfos(std::vector<std::string> &infos) const;

    std::string BuildTypesString(const std::vector<GopType> types) const;
    uint32_t CalculateGDatValueSize(const std::vector<GopType> types) const;

protected:
    void Reset();
public:
    GopFile();
    virtual ~GopFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    bool GetColumn(const std::string &name, GopColumn &column) const;
};

#endif // GOPFILE_H
