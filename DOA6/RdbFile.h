#ifndef RDBFILE_H
#define RDBFILE_H

#include <unordered_map>
#include <unordered_set>

#include "Utils.h"
#include "FileStream.h"

#define RDB_SIGNATURE       "_DRK"
#define RDB_ENTRY_SIGNATURE "IDRK"

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED RDBHeader
{
    char signature[4]; // 0
    char version[4]; // 4
    uint32_t header_size; // 8
    uint32_t plattform;  // 0xA = windows
    uint32_t num_files; // 0x10
    uint32_t name_db_file; // 0x14  ID of the file containing the resource file name to hash mapping and other info
    char path[8]; // 0x18;
};
CHECK_STRUCT_SIZE(RDBHeader, 0x20);

// Flags may differ in .bin files?
#define RDB_FLAG_EXTERNAL   0x00010000
#define RDB_FLAG_INTERNAL   0x00020000
#define RDB_FLAG_COMPRESSED 0x00100000

struct PACKED RDBEntry
{
    char signature[4];
    char version[4];
    uint32_t entry_size; // 8
    uint32_t unk_0C;
    uint32_t c_size; // 0x10  string size in .rdb, comp size in .bin
    uint32_t unk_14;
    uint64_t file_size; // 0x18
    uint32_t type; // 0x20
    uint32_t file_id; // 0x24
    uint32_t type_id; // 0x28
    uint32_t flags; // 0x2C
    // ... varies by type
};
CHECK_STRUCT_SIZE(RDBEntry, 0x30);

// Fairy tail 2

#define RDB_NEW_TYPE_INTERNAL   0x401
#define RDB_NEW_TYPE_EXTERNAL   0xC01

struct PACKED RDBEntryEx
{
   uint16_t flags; // 0x401 for "internal" files (.fdata), 0xC01 for external files (.file)
   uint32_t fdata_offset; // 2 - Dunno what this is for .file (there is the possibility that is noise)
   uint32_t full_size; // 6 - Full size in the .fdata/.file including the IDRK
   uint16_t file_id; // 0xA - Used to map to the .fdata file (dunno the meaning in external files, probably all files that share this id are related)
   uint8_t unk_0C;

   static RDBEntryEx *GetFromRDBEntry(RDBEntry *entry)
   {
       if (entry->c_size != 0xD)
           return nullptr;

       return (RDBEntryEx *)(((uint8_t *)entry)+entry->entry_size-0xD);
   }
};
CHECK_STRUCT_SIZE(RDBEntryEx, 0xD);

// Fairy Tail 2
struct PACKED RDXEntry
{
    uint16_t file_id;
    int16_t unk_02; // Always -1?
    uint32_t filename_hash; // 4
};
CHECK_STRUCT_SIZE(RDXEntry, 8);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct RdbEntry
{
    std::string version;

    std::string address;
    std::string bin_file;

    uint64_t offset;
    uint64_t size;
    int index1;
    int index2;
    bool external;

    uint64_t file_size;
    uint32_t type;

    uint32_t file_id;
    uint32_t type_id;
    uint32_t flags;

    uint16_t fdata_id; // Fairy Tail 2, etc
    uint32_t fdata_hash;
    bool is_new_format;

    std::vector<uint8_t> unk_data;

    RdbEntry()
    {
        offset = 0;
        size = 0;
        index1 = -1;
        index2 = -1;
        external = false;
        //
        fdata_id = 0xFFFF;
        fdata_hash = 0;
        is_new_format = false;
    }
};

class RdbFile : public BaseFile
{
private:

    std::string rdb_path;
    std::vector<RdbEntry> entries;
    std::unordered_map<std::string, FileStream *> bin_files;
    std::unordered_map<uint32_t, std::string> names_map;
    std::unordered_map<std::string, uint32_t> names_map_rev;
    std::unordered_map<uint32_t, size_t> hash_to_idx;

    FileStream *wb;
    int w_index1, w_index2;

    uint32_t name_db_file;

    std::vector<RDXEntry> rdx_entries;
    std::string base_path;
    std::unordered_map<uint16_t, uint32_t> rdx_map;

    mutable bool fe4_check;
    mutable bool fe4_ret;

    RdbFile();

    bool ExtractFileFData(const RdbEntry &entry, Stream *out, bool omit_external_error=false, bool external_error_is_success=false);

protected:

    void Reset();

public:
    RdbFile(const std::string &rdb_path);
    virtual ~RdbFile() override;

    bool LoadAndBuildMap(const uint8_t *buf, size_t size, std::vector<RDBEntry *> *ret_entries);

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    bool ReloadAsDeadFiles(const std::string &bin1, const std::string &bin2="", const std::string &bin3="", const std::string &bin4="");

    void BuildAdditionalLookup(bool build_name_rev, bool build_hash_to_idx);

    inline size_t GetNumFiles() const { return entries.size(); }

    std::string GetExternalPath(size_t idx) const;

    bool ExtractFile(size_t idx, Stream *out, bool omit_external_error=false, bool external_error_is_success=false);
    bool ExtractFile(size_t idx, const std::string &path, bool omit_external_error=false, bool external_error_is_success=false);

    bool SetWriteBin(const std::string &file, int index1, int index2);

    bool ReimportFile(size_t idx, Stream *in);

    size_t FindFileByID(uint32_t id) const;
    size_t FindFileByName(const std::string &name) const;

    uint32_t GetTypeByExtension(const std::string &ext) const;
    bool MatchesType(size_t idx, uint32_t type) const;

    bool GetFileName(size_t idx, std::string &name) const;
    bool GetFileNameByID(uint32_t id, std::string &name) const;

    inline const RdbEntry &GetEntry(size_t idx) const { return entries[idx]; }
    inline RdbEntry &GetEntry(size_t idx) { return entries[idx]; }

    bool LoadAdditionalNames(const char *content);
    void LoadAdditionalNames(const std::unordered_map<uint32_t, const char *> &map);

    bool SaveNamesToTxt(const std::string &path);

    bool IsCharacterEditor() const;
    bool IsMaterialEditor() const;
    bool IsRRPreview() const;
    bool IsKIDSSystemResource() const;
    bool IsFieldEditor4() const;
    bool IsSystem() const;
    bool IsScreenLayout() const;
    bool IsSequenceEditor() const;


    bool SetRdxMode(const std::string &rdx_path, const std::string &base_path);
    inline bool IsRdxMode() const { return rdx_entries.size() > 0; }

    bool GetNewFormatData(size_t idx, std::string &pkg) const;

    inline std::unordered_map<uint32_t, std::string> &DebugGetNamesMap() { return names_map; }
};

#endif // RDBFILE_H
