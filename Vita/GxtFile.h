#ifndef GXTFILE_H
#define GXTFILE_H

#include "FixedMemoryStream.h"
#include "DdsFile.h"

#define GXT_SIGNATURE   0x00545847

;
#pragma pack(push,1)

#define GXT_PVRT2BPP        0x80000000
#define GXT_PVRT4BPP        0x81000000
#define GXT_PVRTII2BPP      0x82000000
#define GXT_PVRTII4BPP      0x83000000
#define GXT_UBC1            0x85000000
#define GXT_UBC2            0x86000000
#define GXT_UBC3            0x87000000
#define GXT_1555            0x00040010
#define GXT_ARGB4444        0x10000000
#define GXT_ARGB8888        0x0C001000
#define GXT_XRGB888         0x0C005000
#define GXT_RGB888          0x98001000
#define GXT_RGB565          0x05001000
#define GXT_RGB555          0x04005000
#define GXT_RGB4444         0x02001000

#define GXT_SWIZZLED    0
#define GXT_CUBE        0x40
#define GXT_LINEAR      0x60
#define GXT_TILED       0x80
#define LINEAR_STRIDED  0x0C

struct GXTHeader
{
    uint32_t signature; // 0
    uint32_t version; // 4
    uint32_t num_textures; // 8
    uint32_t textures_data_offset; // C
    uint32_t total_textures_size; // 0x10
    uint32_t p4_palettes; // 0x14
    uint32_t p8_palettes; // 0x18
    uint32_t pad; // 0x1C

    GXTHeader()
    {
        signature = GXT_SIGNATURE;
        version = 0x10000003;
        num_textures = 0;
        textures_data_offset = 0x40;
        total_textures_size = 0;
        p4_palettes = p8_palettes = 0;
        pad = 0;
    }
};
CHECK_STRUCT_SIZE(GXTHeader, 0x20);

struct GXTTexture
{
    uint32_t offset; // 0
    uint32_t size; // 4
    int32_t pal_idx; // 8
    uint32_t flags; // C
    uint32_t texture_type; // 0x10
    uint32_t texture_format; // 0x14
    uint16_t width; // 0x18
    uint16_t height; // 0x1A
    uint32_t mips; // 0x1C

    GXTTexture()
    {
        memset(this, 0, sizeof(GXTTexture));
        pal_idx = -1;
    }
};

CHECK_STRUCT_SIZE(GXTTexture, 0x20);

#pragma pack(pop)

struct GxtTexture
{
    uint32_t texture_type;
    uint32_t texture_format;
    uint16_t width;
    uint16_t height;
    uint32_t mips;
    std::vector<uint8_t> data;
};

class GxtFile : public BaseFile
{
private:
    uint32_t version;
    std::vector<GxtTexture> textures;

protected:
    void Reset();

public:
    GxtFile();
    virtual ~GxtFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    inline size_t GetNumTextures() const { return textures.size(); }

    bool CanConvertToDds(size_t tex_idx) const;
    DdsFile *ToDDS(size_t tex_idx) const;
};

#endif // GXTFILE_H
