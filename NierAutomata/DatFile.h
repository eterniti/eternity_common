// Nier Automata DAT File

#ifndef DATFILE_H
#define DATFILE_H

#include "BaseFile.h"

#define NA_DAT_SIGNATURE    0x544144

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED NADatHeader
{
    uint32_t signature; // 0
    uint32_t num_files; // 4
    uint32_t file_offsets; // 8
    uint32_t file_extensions; // 0xC
    uint32_t file_names; // 0x10
    uint32_t file_sizes; // 0x14
    uint32_t unk_data_offset; // 0x18
    uint32_t unk_1C; // Zero
};
CHECK_STRUCT_SIZE(NADatHeader, 0x20);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct DatFileEntry
{
    std::vector<uint8_t> content;
    std::string name;
};

class DatFile : public BaseFile
{
private:

    std::vector<DatFileEntry> files;
    std::vector<uint8_t> unk_data;

    size_t CalcFileStructure(size_t *file_offsets_offset, size_t *extensions_offset, uint32_t *names_step, size_t *names_offset, size_t *sizes_offset, size_t *unk_data_offset, size_t *content_offset) const;

protected:

    void Reset();

public:
    DatFile();
    virtual ~DatFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    inline size_t GetNumFiles() const { return files.size(); }

    DatFileEntry *FindFile(const std::string &name);

    inline void AddFile(const DatFileEntry &entry) { files.push_back(entry); }

    inline const DatFileEntry &operator[](size_t n) const { return files[n]; }
    inline const DatFileEntry &operator[](size_t n) { return files[n]; }

    inline std::vector<DatFileEntry>::const_iterator begin() const { return files.begin(); }
    inline std::vector<DatFileEntry>::const_iterator end() const { return files.end(); }

    inline std::vector<DatFileEntry>::iterator begin() { return files.begin(); }
    inline std::vector<DatFileEntry>::iterator end() { return files.end(); }
};

#endif // DATFILE_H
