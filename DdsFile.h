#ifndef DDSFILE_H
#define DDSFILE_H

//#define DIRECTX_TEX_SUPPORT
#ifdef DIRECTX_TEX_SUPPORT
#include <DirectXTex.h>
#else
#include "dxgi_int.h"
#endif

#include "BaseFile.h"

#define DDS_SIGNATURE 0x20534444

#define DXT1_FCC    0x31545844
#define DXT3_FCC    0x33545844
#define DXT5_FCC    0x35545844
#define DX10_FCC    0x30315844

#define ATI1_FCC    0x31495441
#define BC4U_FCC    0x55344342

enum DdsFlags
{
    DDS_FLAGS_CAPS = 1,
    DDS_FLAGS_HEIGHT = 2,
    DDS_FLAGS_WIDTH = 4,
    DDS_FLAGS_PITCH = 8,
    DDS_FLAGS_PIXELFORMAT = 0x1000,
    DDS_FLAGS_MIPMAPCOUNT = 0x20000,
    DDS_FLAGS_LINEAR_SIZE = 0x80000,
    DDS_FLAGS_DEPTH = 0x800000,
};

enum DdsPixelFlags
{
    DDS_PF_ALPHAPIXELS = 1,
    DDS_PF_ALPHA = 2,
    DDS_PF_FOURCC = 4,
    DDS_PF_RGB = 0x40,
    DDS_PF_YUV = 0x200,
    DDS_PF_LUMINANCE = 0x20000,
};

enum DdsCaps
{
    DDS_CAPS_COMPLEX = 8,
    DDS_CAPS_TEXTURE = 0x1000,
    DDS_CAPS_MIPMAP = 0x400000,
};

enum DdsCaps2
{
    DDS_CAPS2_CUBEMAP = 0x200,
    DDS_CAPS2_CUBEMAP_POSITIVEX = 0x400,
    DDS_CAPS2_CUBEMAP_NEGATIVEX = 0x800,
    DDS_CAPS2_CUBEMAP_POSITIVEY = 0x1000,
    DDS_CAPS2_CUBEMAP_NEGATIVEY = 0x2000,
    DDS_CAPS2_CUBEMAP_POSITIVEZ = 0x4000,
    DDS_CAPS2_CUBEMAP_NEGATIVEZ = 0x8000,
};

enum Dxt10ResourceDimension
{
    DXT10_RD_UNKNOWN,
    DXT10_RD_BUFFER,
    DXT10_RD_TEXTURE_1D,
    DXT10_RD_TEXTURE_2D,
    DXT10_RD_TEXTURE_3D,
};

enum Dxt10MiscFlags2
{
    DXT10_MF2_UNKNOWN,
    DXT10_MF2_STRAIGHT,
    DXT10_MF2_PREMULTIPLIED,
    DXT10_MF2_OPAQUE,
    DXT10_MF2_CUSTOM,
};

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED DDSPixelFormat
{
    uint32_t size; // 0x4C - Size of this struct
    uint32_t flags; // 0x50
    uint32_t fourCC; // 0x54
    uint32_t rgb_bit_count; // 0x58
    uint32_t r_mask; // 0x5C
    uint32_t g_mask; // 0x60
    uint32_t b_mask; // 0x64
    uint32_t a_mask; // 0x68

    DDSPixelFormat()
    {
        memset(this, 0, sizeof(DDSPixelFormat));
        size = sizeof(DDSPixelFormat);
        flags = DDS_PF_FOURCC;
        fourCC = DXT5_FCC;
    }
};
CHECK_STRUCT_SIZE(DDSPixelFormat, 0x20);

struct PACKED DDSHeader
{
    uint32_t signature; // 0
    uint32_t size; // 4 - Size of header minus the signature
    uint32_t flags; // 8
    uint32_t height; // 0xC
    uint32_t width; // 0x10
    uint32_t pitch_or_linear_size; // 0x14
    uint32_t depth; // 0x18
    uint32_t mip_map_count; // 0x1C
    uint32_t reserved[11]; // 0x20
    DDSPixelFormat pf; // 0x4C
    uint32_t caps; // 0x6C
    uint32_t caps2; // 0x70
    uint32_t caps3; // 0x74
    uint32_t caps4; // 0x78
    uint32_t reserved2; // 0x7C

    DDSHeader()
    {
        memset(this, 0, sizeof(DDSHeader));

        signature = DDS_SIGNATURE;
        size = sizeof(DDSHeader) - 4;
        flags = DDS_FLAGS_LINEAR_SIZE | DDS_FLAGS_PIXELFORMAT | DDS_FLAGS_WIDTH | DDS_FLAGS_HEIGHT | DDS_FLAGS_CAPS;
        pf = DDSPixelFormat();
        caps = DDS_CAPS_TEXTURE;
    }
};
CHECK_STRUCT_SIZE(DDSHeader, 0x80);

struct PACKED DXT10Header
{
    uint32_t dxgi_format; // 0x80
    uint32_t resource_dimension; // 0x84
    uint32_t misc_flags; // 0x88
    uint32_t array_size; // 0x8C
    uint32_t misc_flags2; // 0x90

    DXT10Header()
    {
        memset(this, 0, sizeof(DXT10Header));
        resource_dimension = DXT10_RD_TEXTURE_2D;
        array_size = 1;
        misc_flags2 = DXT10_MF2_STRAIGHT;
    }
};
CHECK_STRUCT_SIZE(DXT10Header, 0x14);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

class DdsFile : public BaseFile
{
private:

    DDSHeader header;
    int format;
    std::vector<uint8_t> raw;

    bool has_dxt10;
    DXT10Header dxt10;

    bool DecodeB8G8R8X8(uint32_t *dec, bool *alpha) const; // also for B8G8R8X8
    bool DecodeR32G32B32A32Float(uint32_t *dec, bool *alpha) const;
    bool DecodeR16G16B16A16Float(uint32_t *dec, bool *alpha) const;
    bool DecodeR32Float(uint32_t *dec, bool *alpha) const;
    bool DecodeR16G16Float(uint32_t *dec, bool *alpha) const;

#ifdef DIRECTX_TEX_SUPPORT
    bool DecodeGeneric(uint32_t *dec, bool *alpha) const;
#endif

protected:

    void Reset();

public:
    DdsFile();
    DdsFile(int format, uint32_t width, uint32_t height, int mip_maps, const uint8_t *buf, size_t buf_size);
    virtual ~DdsFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    uint32_t *Decode(bool *alpha) const;

    inline uint32_t GetWidth() const { return header.width; }
    inline uint32_t GetHeight() const { return header.height; }

    inline int GetFormat() const { return format; }
    inline int GetMips() const { return (int)header.mip_map_count; }

    inline uint8_t *GetRaw(size_t *psize) { *psize = raw.size(); return raw.data(); }
    inline const uint8_t *GetRaw(size_t *psize) const { *psize = raw.size(); return raw.data(); }

    static std::string GetFormatName(int format);
    inline std::string GetFormatName() { return GetFormatName(format); }
};

#endif // DDSFILE_H
