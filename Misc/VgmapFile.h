#ifndef VGMAPFILE_H
#define VGMAPFILE_H

#include <map>
#include "BaseFile.h"

class VgmapFile : public BaseFile
{
private:

    std::map<uint32_t, std::string> entries;
    std::map<std::string, uint32_t> reverse_entries;

    uint32_t greater_vg;

protected:

    void Reset();

public:
    VgmapFile();
    VgmapFile(VgmapFile const&) = default;
    virtual ~VgmapFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    void Append(const VgmapFile &other);

    inline const VgmapFile operator+(const VgmapFile &other) const
    {
        VgmapFile new_vgmap = *this;
        new_vgmap.Append(other);
        return new_vgmap;
    }

    inline VgmapFile &operator+=(const VgmapFile &other)
    {
        this->Append(other);
        return *this;
    }
};

#endif // VGMAPFILE_H
