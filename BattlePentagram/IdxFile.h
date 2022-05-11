#ifndef IDXFILE_H
#define IDXFILE_H

#include "FixedMemoryStream.h"
#include "ApkFile.h"

struct Fsls
{
    std::string name;
    uint32_t pack_idx;
    uint64_t offset;
    uint64_t size;
    uint8_t md5[0x10];
};

class ApkFile;

class IdxFile : public BaseFile
{
private:
    std::vector<PACKHEDRHeader> packs;
    std::vector<std::string> names;
    std::vector<FileEntry> files;
    std::vector<Fsls> fslss;

    bool ReadStrings(Stream *stream, uint64_t offset, std::vector<std::string> &strings);
    bool BuildPath(size_t idx, const std::string &path, std::vector<bool> &build);
    bool BuildPaths();
    void BuildNamesTable(const std::vector<std::string> &names, std::unordered_map<std::string, uint64_t> &table);
    bool WriteNamesTable(Stream *out, const std::vector<std::string> &names, const std::unordered_map<std::string, uint64_t> &table);

protected:
    void Reset();

public:
    IdxFile();
    virtual ~IdxFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    bool UpdateFrom(ApkFile *apk, uint32_t pack_idx);
};

#endif // IDXFILE_H
