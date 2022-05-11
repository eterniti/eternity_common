#ifndef HFSFILE_H
#define HFSFILE_H

#include "BaseFile.h"
#include "Stream.h"

#define HFS_SIGNATUE_CD_HEADER  0x6054648
#define HFS_SIGNATURE_ENTRY    0x2014648

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    uint32_t signature; // 0
    uint16_t disk_number; // 4
    uint16_t start_central_dir_disk; // 6
    uint16_t num_entries; // 8
    uint16_t num_entries_central_dir; // 0xA
    uint32_t central_dir_size; // 0xC
    uint32_t central_dir_offset; // 0x10
    uint16_t comment_size; // 0x14
} PACKED HFSCentralDirHeader;

STATIC_ASSERT_STRUCT(HFSCentralDirHeader, 0x16);

typedef struct
{
    uint32_t signature; // 0
    uint16_t madeby_version; // 4
    uint16_t extract_version; // 6
    uint16_t flags; // 8
    uint16_t comp_method; // 0xA
    uint32_t dostime; // 0xC
    uint32_t crc; // 0x10
    uint32_t comp_size; // 0x14
    uint32_t size; // 0x18
    uint16_t name_len; // 0x1C
    uint16_t extra_len; // 0x1E
    uint16_t comment_len; // 0x20
    uint16_t disk_start_no; // 0x22
    uint16_t internal_attr; // 0x24
    uint32_t external_attr; // 0x26
    uint32_t offset; // 0x2A
} PACKED HFSCentralDirEntry;

STATIC_ASSERT_STRUCT(HFSCentralDirEntry, 0x2E);

typedef struct
{
    uint32_t signature; // 0
    uint16_t version_extract; // 4
    uint16_t flags; // 6
    uint16_t comp_method; // 8
    uint16_t file_time; // 0xA
    uint16_t file_date; // 0xC
    uint32_t crc; // 0xE
    uint32_t comp_size; // 0x12
    uint32_t size; // 0x16
    uint16_t name_len; // 0x1A
    uint16_t extra_len; // 0x1C
} PACKED HFSLocalHeader;

STATIC_ASSERT_STRUCT(HFSLocalHeader, 0x1E);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

class HfsFile;

class HfsEntry
{
private:

    friend class HfsFile;

    std::string name;   
    std::vector<uint8_t> data;

public:

    std::string GetName() const;
    inline size_t GetSize() const { return data.size(); }
    inline const std::vector<uint8_t> &GetData() const { return data; }

    bool Write(Stream *stream) const;

};

class HfsFile : public BaseFile
{
private:

    std::vector<HfsEntry> entries;

    void Reset();

    void Decrypt(std::vector<uint8_t> &data, size_t start_pos);
    void Decrypt(std::string &string, size_t start_pos);

public:

    HfsFile();
    virtual ~HfsFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;

    size_t GetNumEntries() const { return entries.size(); }

    inline const HfsEntry &operator[](size_t n) const { return entries[n]; }
    inline HfsEntry &operator[](size_t n) { return entries[n]; }

    inline std::vector<HfsEntry>::const_iterator begin() const { return entries.begin(); }
    inline std::vector<HfsEntry>::const_iterator end() const { return entries.end(); }

    inline std::vector<HfsEntry>::iterator begin() { return entries.begin(); }
    inline std::vector<HfsEntry>::iterator end() { return entries.end(); }
};

#endif // HFSFILE_H
