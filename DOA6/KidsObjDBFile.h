#ifndef KIDSOBJDBFILE_H
#define KIDSOBJDBFILE_H

#include <unordered_map>
#include "FixedMemoryStream.h"

#define KOD_SIGNATURE   0x4B4F445F
#define KODI_SIGNATURE  0x4B4F4449
#define KODR_SIGNATURE  0x4B4F4452

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED KODHeader
{
    uint32_t signature; // 0
    uint32_t version; // 4
    uint32_t header_size; // 8 - Or data start
    uint32_t platform; // 0xC - PC or dx11
    uint32_t num_entries; // 0x10
    uint32_t name_file; // 0x14
    uint32_t file_size; // 0x18
};
CHECK_STRUCT_SIZE(KODHeader, 0x1C);

struct PACKED KODIHeader
{
    uint32_t signature; // 0
    uint32_t version; // 4
    uint32_t entry_size; // 8
    uint32_t name; // 0xC
    uint32_t type; // 0x10
    uint32_t num_columns; // 0x14
};
CHECK_STRUCT_SIZE(KODIHeader, 0x18);

struct PACKED KODRHeader
{
    uint32_t signature; // 0
    uint32_t version; // 4
    uint32_t entry_size; // 8
    uint32_t name; // 0xC
    uint32_t parent_object_file; // 0x10
    uint32_t parent_object; // 0x14
    uint32_t num_columns; // 0x18
};
CHECK_STRUCT_SIZE(KODRHeader, 0x1C);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

// Note: the types seem to match those of Criware UTF db files
enum
{
    KIDS_ODB_INT8 = 0,
    KIDS_ODB_UINT8 = 1,

    KIDS_ODB_INT16 = 2,
    KIDS_ODB_UINT16 = 3,

    KIDS_ODB_INT32 = 4,
    KIDS_ODB_UINT32 = 5,

    KIDS_ODB_FLOAT = 8,

    KIDS_ODB_VECTOR4 = 10,
    KIDS_ODB_VECTOR2 = 12,
    KIDS_ODB_VECTOR3 = 13,
};

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED KidsODBValue8
{
    union
    {
        int8_t i8;
        uint8_t u8;
    };
};
CHECK_STRUCT_SIZE(KidsODBValue8, 1);

struct PACKED KidsODBValue16
{
    union
    {
        int16_t i16;
        uint16_t u16;
    };
};
CHECK_STRUCT_SIZE(KidsODBValue16, 2);

struct PACKED KidsODBValue32
{
    union
    {
        int32_t i32;
        uint32_t u32;
        float f;
    };
};
CHECK_STRUCT_SIZE(KidsODBValue32, 4);

struct PACKED KidsODBValue64
{
    union
    {
        float v2[2];
    };
};
CHECK_STRUCT_SIZE(KidsODBValue64, 8);

struct PACKED KidsODBValue96
{
    union
    {
        float v3[3];
    };
};
CHECK_STRUCT_SIZE(KidsODBValue96, 12);

struct PACKED KidsODBValue128
{
    union
    {
        float v4[4];
    };
};
CHECK_STRUCT_SIZE(KidsODBValue128, 16);

#ifdef _MSC_VER
#pragma pack(pop)
#endif


struct KidsODBColumn
{
    uint32_t name;
    int32_t type;

    std::vector<KidsODBValue8> values8;
    std::vector<KidsODBValue16> values16;
    std::vector<KidsODBValue32> values32;
    std::vector<KidsODBValue64> values64;
    std::vector<KidsODBValue96> values96;
    std::vector<KidsODBValue128> values128;

    bool IsString() const;
    bool IsBlob() const;
    bool IsStringArray(uint32_t parent_type) const;
    size_t GetStringArray(std::vector<std::string> &ret) const;

    TiXmlElement *Decompile(TiXmlNode *root, const std::string &parent_name, uint32_t parent_type, const std::string &att_dir) const;
    bool Compile(const TiXmlElement *root, const std::string &att_dir);
};

struct KidsODBObject
{
    uint32_t name;
    uint32_t type;
    uint32_t version;
    uint32_t parent_object_file;
    uint32_t parent_object;

    bool is_r;

    std::vector<KidsODBColumn> columns;

    TiXmlElement *Decompile(TiXmlNode *root, const std::string &att_dir) const;
    bool Compile(const TiXmlElement *root, const std::string &att_dir);
};

class KidsObjDBFile : public BaseFile
{
private:

    std::vector<KidsODBObject> objects;    
    uint32_t name_file;
    uint32_t version;
    uint32_t platform;

    std::string att_dir;    

protected:

    void Reset();

public:
    KidsObjDBFile();
    virtual ~KidsObjDBFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    virtual bool DecompileToFile(const std::string &path, bool show_error=true, bool build_path=false) override;
    virtual bool CompileFromFile(const std::string &path, bool show_error=true, bool big_endian=false) override;

    void SortByName();
    void SortByHash();

    static bool LoadNamesFileBuf(const char *buf);
    static bool LoadNamesFile(const std::string &path);

    static bool LoadNamesFileBinBuf(const uint8_t *buf);
    static bool LoadNamesFileBin(const std::string &path);
};

#endif // KIDSOBJDBFILE_H
