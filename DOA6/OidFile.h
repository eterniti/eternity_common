#ifndef OIDFILE_H
#define OIDFILE_H

#include <unordered_map>
#include "FixedMemoryStream.h"

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED OIDEntry
{
    uint32_t bone_id;
    uint32_t hash;
    uint32_t unk;
};
CHECK_STRUCT_SIZE(OIDEntry, 0xC);
#ifdef _MSC_VER
#pragma pack(pop)
#endif


class OidFile : public BaseFile
{
private:

    uint32_t max_bone_id;
    std::vector<OIDEntry> entries;

    std::unordered_map<uint32_t, std::string> names_map;

    void CreateNamesMap();

protected:
    void Reset();

public:
    OidFile();
    virtual ~OidFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    //virtual uint8_t *Save(size_t *psize) override;

    //bool LoadFromTextFile(const std::string &path);
    bool SaveToTextFile(const std::string &path, bool *crack_success);
};

#endif // OIDFILE_H
