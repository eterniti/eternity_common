#ifndef APXFILE_H
#define APXFILE_H

#include <unordered_map>
#include "FileStream.h"

#define ENDILTLE_SIGNATURE  "ENDILTLE"
#define PACKHEDR_SIGNATURE  "PACKHEDR"
#define PACKTOC_SIGNATURE   "PACKTOC "
#define PACKFSLS_SIGNATURE  "PACKFSLS"
#define GENESTRT_SIGNATURE  "GENESTRT"
#define PACKFSHD_SIGNATURE  "PACKFSHD"

#define ATTRIBUTE_UNCOMPRESSED  0
#define ATTRIBUTE_DIRECTORY     1
#define ATTRIBUTE_COMPRESSED    0x200

;
#pragma pack(push,1)

struct ENDILTLEHeader
{
    char signature[8]; // 0
    uint8_t unk[8]; // 8

    ENDILTLEHeader()
    {
        memcpy(signature, ENDILTLE_SIGNATURE, sizeof(signature));
        memset(unk, 0, sizeof(unk));
    }
};
CHECK_STRUCT_SIZE(ENDILTLEHeader, 0x10);

struct PACKHEDRHeader
{
    char signature[8]; // 0
    uint32_t size; // 8 - Not counting checksum
    uint32_t unk_0C;
    uint32_t unk_10;
    uint32_t pack_idx; // 14
    uint32_t file_data_offset; // 18
    uint32_t unk_1C; // It may be an endianess check
    uint8_t checksum[0x10]; // 20 - What does it cover?

    PACKHEDRHeader()
    {
        memcpy(signature, PACKHEDR_SIGNATURE, sizeof(signature));
        size = 0x20;
        unk_0C = 0;
        unk_10 = 0x10000;
        pack_idx = 0;
        file_data_offset = 0;
        unk_1C = 1;
        memset(checksum, 0, sizeof(checksum));
    }
};
CHECK_STRUCT_SIZE(PACKHEDRHeader, 0x30);

struct TOCEntry
{
    uint32_t attributes; // 0
    uint32_t name_idx; // 4
    uint32_t pack_idx; // 8
    uint32_t unk_0C;

    union
    {
         uint64_t file_offset; // 10

         // This is for folders
         struct
         {
            uint32_t file_idx_start; // 10
            uint32_t num_files; // 14
         } b;
    } a; // 10

    uint64_t size; // 18
    uint64_t comp_size; // 20

    TOCEntry()
    {
        memset(this, 0, sizeof(TOCEntry));
    }
};
CHECK_STRUCT_SIZE(TOCEntry, 0x28);

struct PACKTOCHeader
{
    char signature[8]; // 0
    uint32_t toc_size; // 8 - Full toc size - 0x10
    uint32_t unk_0C;
    uint32_t entry_size; // 10
    uint32_t num_files; // 14
    uint32_t unk_18;
    uint32_t unk_1C;

    PACKTOCHeader()
    {
        memcpy(signature, PACKTOC_SIGNATURE, sizeof(signature));
        toc_size = 0;
        unk_0C = 0;
        entry_size = sizeof(TOCEntry);
        num_files = 0;
        unk_18 = 0x10;
        unk_1C = 0;
    }
};
CHECK_STRUCT_SIZE(PACKTOCHeader, 0x20);

struct FSLSEntry
{
    uint32_t name_idx; // 0
    uint32_t pack_idx; // 4
    uint64_t offset; // 8
    uint64_t size; // 0x10
    uint8_t md5[0x10];

    FSLSEntry()
    {
        memset(this, 0, sizeof(FSLSEntry));
    }
};
CHECK_STRUCT_SIZE(FSLSEntry, 0x28);

struct PACKFSLSHeader
{
    char signature[8]; // 0
    uint32_t fsls_size; // 8 - fsls size - 0x10
    uint32_t unk_0C;
    uint32_t num_files; // 10
    uint32_t entry_size; // 14
    uint32_t unk_18;
    uint32_t unk_1C;

    PACKFSLSHeader()
    {
        memcpy(signature, PACKFSLS_SIGNATURE, sizeof(signature));
        fsls_size = 0;
        unk_0C = 0;
        num_files = 0;
        entry_size = sizeof(FSLSEntry);
        unk_18 = 0x10;
        unk_1C = 0;
    }

};
CHECK_STRUCT_SIZE(PACKFSLSHeader, 0x20);

struct GENESTRTHeader
{
    char signature[8]; // 0
    uint32_t str_size; // 8 - str size - 0x20
    uint32_t unk_0C;
    uint32_t num_strings; // 10
    uint32_t unk_14;
    uint32_t str_table_offset; // 18
    uint32_t str_size2; // 1C - seems same as str_size

    GENESTRTHeader()
    {
        memcpy(signature, GENESTRT_SIGNATURE, sizeof(signature));
        str_size = str_size2 = 0;
        unk_0C = 0;
        num_strings = 0;
        unk_14 = 0x10;
        str_table_offset = 0;
    }
};
CHECK_STRUCT_SIZE(GENESTRTHeader, 0x20);

struct FSHDEntry
{
    uint32_t name_idx; // 0
    uint32_t flags; // 4
    uint64_t offset; // 8  offset is from beginning of FSHD (the ENDILTLE)
    uint64_t size; // 10
    uint64_t comp_size; // 18

    FSHDEntry()
    {
        name_idx = 0;
        flags = 2;
        offset = 0;
        size = comp_size = 0;
    }
};
CHECK_STRUCT_SIZE(FSHDEntry, 0x20);

struct PACKFSHDHeader
{
    char signature[8]; // 0
    uint32_t size; // 8 - size of fshd - 0x10 (not counting previous ENDILTLE, not counting GENESTRT)
    uint32_t unk_0C;
    uint32_t unk_10;
    uint32_t entry_size; // 0x14
    uint32_t num_files; // 18
    uint32_t entry_size2; // 0x1C - same as entry_size?
    uint64_t full_size; // 0x20 - Full size of fshd including the GENESTRT section, but not counting the ENDILTLE
    uint64_t unk_28;

    PACKFSHDHeader()
    {
        memcpy(signature, PACKFSHD_SIGNATURE, sizeof(signature));
        size = 0;
        unk_0C = 0;
        unk_10 = 0x10000;
        entry_size = entry_size2 = sizeof(FSHDEntry);
        num_files = 0;
        full_size = 0;
        unk_28 = 0;
    }
};
CHECK_STRUCT_SIZE(PACKFSHDHeader, 0x30);

#pragma pack(pop)

struct FileEntry
{
    uint32_t attributes;
    std::string name;
    uint32_t pack_idx; // Not for files in fshsds
    std::string path;
    uint64_t file_offset; // For files only
    uint64_t size; // For files only
    uint64_t comp_size; // For files only
    uint32_t file_idx_start; // For folders only
    uint32_t num_files; // For folders only
    uint8_t *buf; // For replacing

    FileEntry()
    {
        buf = nullptr;
        pack_idx = 0;
    }

    ~FileEntry()
    {
        if (buf)
        {
            delete buf;
            buf = nullptr;
        }
    }
};

struct Fshd
{
    std::string name;
    std::vector<std::string> names;
    std::vector<FileEntry> files;
    uint32_t pack_idx;
};

#include "IdxFile.h"

class ApkFile : BaseFile
{
    friend class IdxFile;

private:

    FileStream *apk;
    uint32_t pack_idx;
    uint8_t checksum[0x10]; // Until what the checksum hashes and which kind of checksum is, let's just copy it here

    std::vector<std::string> names;
    std::vector<FileEntry> files;
    std::vector<Fshd> fshds;

    uint64_t gfsls_offset; // For use by IdxFile

    bool ReadStrings(uint64_t offset, std::vector<std::string> &strings, bool show_error);
    bool BuildPath(size_t idx, const std::string &path, std::vector<bool> &build);
    bool BuildPaths();
    bool ReadFshd(const FSLSEntry &entry, Fshd &fshd, bool show_error);
    bool ExtractFile(const FileEntry &file, const std::string &dir_path, const std::string &fshd_name, const std::string &filter) const;
    bool DoReplaceFile(FileEntry &file, const uint8_t *buf, size_t size);

    bool SaveFailCleanup(FileStream *stream, const std::string &temp_file);
    static void BuildNamesTable(const std::vector<std::string> &names, std::unordered_map<std::string, uint64_t> &table);
    static bool WriteNamesTable(FileStream *out, const std::vector<std::string> &names, const std::unordered_map<std::string,uint64_t> &table);
    bool WriteFshd(FileStream *out, const Fshd& fshd);
    bool CompareFile(const FileEntry &f1, const FileEntry &f2, FileStream *apk1, FileStream *apk2);

protected:
    void Reset();

public:

    ApkFile();
    virtual ~ApkFile() override;

    virtual bool LoadFromFile(const std::string &path, bool show_error=true) override;
    virtual bool SaveToFile(const std::string &path, bool show_error=true, bool build_path=false) override;

    bool Extract(const std::string &dir_path, const std::string &filter="") const;
    uint8_t *Extract(const std::string &file_path, size_t *psize) const;

    bool Replace(const std::string &path, const uint8_t *buf, size_t size);
    bool Replace(const std::string &path, const std::string &src_path);

    bool VisitFiles(bool (* visitor)(const std::string &path, void *custom_param1, void *custom_param2, void *custom_param3), const std::string &filter="", void *custom_param1=nullptr, void *custom_param2=nullptr, void *custom_param3=nullptr) const;

    void DebugDumpOffsets();
    void DebugDumpPaths(const std::string &filter="") const;

    bool operator==(const ApkFile &other);

    inline bool operator!=(const ApkFile &other)
    {
        return !(*this == other);
    }
};

#endif // APXFILE_H
