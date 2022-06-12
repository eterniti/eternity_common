#include "DdsFile.h"
#include "Utils.h"
#include "debug.h"

#include "dxgi_str.h"


DdsFile::DdsFile()
{
    this->big_endian = false;
}

DdsFile::DdsFile(int format, uint32_t width, uint32_t height, int mip_maps, const uint8_t *buf, size_t buf_size)
{
    Reset();

    header.width = width;
    header.height = height;

    header.flags = DDS_FLAGS_LINEAR_SIZE | DDS_FLAGS_PIXELFORMAT | DDS_FLAGS_WIDTH | DDS_FLAGS_HEIGHT | DDS_FLAGS_CAPS;
    header.caps = DDS_CAPS_TEXTURE | DDS_CAPS_COMPLEX;

    if (mip_maps > 0)
    {
        header.flags |= DDS_FLAGS_MIPMAPCOUNT;
        header.caps |= DDS_CAPS_MIPMAP;
        header.mip_map_count = (uint32_t)mip_maps;
    }

    switch (format)
    {
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            header.pf.fourCC = 0;
            header.pf.rgb_bit_count = 32;
            header.pf.flags = DDS_PF_RGB | DDS_PF_ALPHAPIXELS;
            header.pf.r_mask = 0x00FF0000;
            header.pf.g_mask = 0x0000FF00;
            header.pf.b_mask = 0x000000FF;
            header.pf.a_mask = 0xFF000000;
        break;

        case DXGI_FORMAT_B8G8R8X8_UNORM:
            header.pf.fourCC = 0;
            header.pf.rgb_bit_count = 32;
            header.pf.flags = DDS_PF_RGB;
            header.pf.r_mask = 0x00FF0000;
            header.pf.g_mask = 0x0000FF00;
            header.pf.b_mask = 0x000000FF;
        break;

        case DXGI_FORMAT_BC1_UNORM:
            header.pf.flags = DDS_PF_FOURCC;
            header.pf.fourCC = DXT1_FCC;
        break;

        case DXGI_FORMAT_BC2_UNORM:
            header.pf.flags = DDS_PF_FOURCC;
            header.pf.fourCC = DXT3_FCC;
        break;

        case DXGI_FORMAT_BC3_UNORM:
            header.pf.flags = DDS_PF_FOURCC;
            header.pf.fourCC = DXT5_FCC;
        break;

        case DXGI_FORMAT_BC4_UNORM:
            header.pf.flags = DDS_PF_FOURCC;
            header.pf.fourCC = BC4U_FCC;
        break;

        default:
            header.pf.flags = DDS_PF_FOURCC;
            header.pf.fourCC = DX10_FCC;
            has_dxt10 = true;
            dxt10.dxgi_format = (uint32_t)format;
        break;
    }

    if (has_dxt10)
    {
        dxt10.resource_dimension = DXT10_RD_TEXTURE_2D;
        dxt10.array_size = 1;
        dxt10.misc_flags2 = DXT10_MF2_STRAIGHT;
    }

    raw = std::vector<uint8_t>(buf, buf+buf_size);
    this->format = format;
}

DdsFile::~DdsFile()
{

}

void DdsFile::Reset()
{
    header = DDSHeader();
    format = DXGI_FORMAT_UNKNOWN;
    raw.clear();

    has_dxt10 = false;
    dxt10 = DXT10Header();
}

bool DdsFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(DDSHeader))
        return false;

    memcpy(&header, buf, sizeof(DDSHeader));

    if (header.signature != DDS_SIGNATURE || header.size != (sizeof(DDSHeader) - 4))
        return false;

    size -= sizeof(DDSHeader);
    buf += sizeof(DDSHeader);

    if ((header.flags & DDS_FLAGS_CAPS) && header.pf.size == sizeof(DDSPixelFormat))
    {
        if ((header.pf.flags & DDS_PF_FOURCC) && header.pf.fourCC == DX10_FCC)
        {
            has_dxt10 = true;

            if (size < sizeof(DXT10Header))
                return false;

            memcpy(&dxt10, buf, sizeof(DXT10Header));
            size -= sizeof(DXT10Header);
            buf += sizeof(DXT10Header);
        }
    }

    raw.resize(size);
    if (size > 0)
        memcpy(raw.data(), buf, size);

    if (!(header.flags & DDS_FLAGS_CAPS) || !(header.flags & DDS_FLAGS_WIDTH) || !(header.flags & DDS_FLAGS_HEIGHT) || !(header.flags & DDS_FLAGS_PIXELFORMAT))
        return true;

    if (header.pf.size != sizeof(DDSPixelFormat))
        return false;

    if (header.pf.flags & DDS_PF_FOURCC)
    {
        if (has_dxt10)
        {
            format = (int)dxt10.dxgi_format;
        }
        else
        {
            switch  (header.pf.fourCC)
            {
                case DXT1_FCC:
                    format = DXGI_FORMAT_BC1_UNORM;
                break;

                case DXT3_FCC:
                    format = DXGI_FORMAT_BC2_UNORM;
                break;

                case DXT5_FCC:
                    format = DXGI_FORMAT_BC3_UNORM;
                break;

                case ATI1_FCC: case BC4U_FCC:
                    format = DXGI_FORMAT_BC4_UNORM;
                break;
            }
        }
    }
    else if (header.pf.flags & DDS_PF_RGB)
    {
        if (header.pf.r_mask == 0x00FF0000 && header.pf.g_mask == 0x0000FF00 && header.pf.b_mask == 0x000000FF)
        {
            if (header.pf.flags & DDS_PF_ALPHAPIXELS)
            {
                if (header.pf.a_mask == 0xFF000000)
                    format = DXGI_FORMAT_B8G8R8A8_UNORM;
            }
            else
            {
               format = DXGI_FORMAT_B8G8R8X8_UNORM;
            }
        }
    }

    return true;
}

uint8_t *DdsFile::Save(size_t *psize)
{
    size_t buf_size = raw.size() + sizeof(DDSHeader);
    if (has_dxt10)
        buf_size += sizeof(DXT10Header);

    uint8_t *buf = new uint8_t[buf_size];
    uint8_t *ptr = buf;

    memcpy(ptr, &header, sizeof(DDSHeader));
    ptr += sizeof(DDSHeader);

    if (has_dxt10)
    {
        memcpy(ptr, &dxt10, sizeof(DXT10Header));
        ptr += sizeof(DXT10Header);
    }

    memcpy(ptr, raw.data(), raw.size());

    *psize = buf_size;
    return buf;
}

bool DdsFile::DecodeB8G8R8X8(uint32_t *dec, bool *alpha) const
{
    uint32_t dim = header.width * header.height;

    if (raw.size() < (dim*4))
        return false;

    memcpy(dec, raw.data(), dim*4);

    *alpha = (format == DXGI_FORMAT_B8G8R8A8_UNORM);
    return true;
}

bool DdsFile::DecodeR32G32B32A32Float(uint32_t *dec, bool *alpha) const
{
    uint32_t dim = header.width * header.height;

    if (raw.size() < (dim*16))
        return false;

    const float *rgba = (const float *)raw.data();

    for (uint32_t i = 0; i < dim; i++)
    {
        uint32_t r = (uint32_t) (rgba[0] * 255.0f);
        uint32_t g = (uint32_t) (rgba[1] * 255.0f);
        uint32_t b = (uint32_t) (rgba[2] * 255.0f);
        uint32_t a = (uint32_t) (rgba[3] * 255.0f);

        uint32_t color = (a << 24) | (r << 16) | (g << 8) | b;
        dec[i] = color;
        rgba += 4;
    }

    *alpha = true;
    return true;
}

bool DdsFile::DecodeR16G16B16A16Float(uint32_t *dec, bool *alpha) const
{
    uint32_t dim = header.width * header.height;

    if (raw.size() < (dim*8))
        return false;

    const uint16_t *rgba = (const uint16_t *)raw.data();

    for (uint32_t i = 0; i < dim; i++)
    {
        uint32_t r = (uint32_t) (Utils::HalfToFloat(rgba[0]) * 255.0f);
        uint32_t g = (uint32_t) (Utils::HalfToFloat(rgba[1]) * 255.0f);
        uint32_t b = (uint32_t) (Utils::HalfToFloat(rgba[2]) * 255.0f);
        uint32_t a = (uint32_t) (Utils::HalfToFloat(rgba[3]) * 255.0f);

        uint32_t color = (a << 24) | (r << 16) | (g << 8) | b;
        dec[i] = color;
        rgba += 4;
    }

    *alpha = true;
    return true;
}

bool DdsFile::DecodeR32Float(uint32_t *dec, bool *alpha) const
{
    uint32_t dim = header.width * header.height;

    if (raw.size() < (dim*4))
        return false;

    const float *rf = (const float *)raw.data();
    for (uint32_t i = 0; i < dim; i++)
    {
        uint32_t r = (uint32_t) (rf[i] * 255.0f);
        dec[i] = (r << 16) | 0xFF000000;
    }

    *alpha = false;
    return true;
}

bool DdsFile::DecodeR16G16Float(uint32_t *dec, bool *alpha) const
{
    uint32_t dim = header.width * header.height;

    if (raw.size() < (dim*4))
        return false;

    const uint16_t *rf = (const uint16_t *)raw.data();
    for (uint32_t i = 0; i < dim; i++)
    {
        uint32_t r = (uint32_t) (Utils::HalfToFloat(rf[0]) * 255.0f);
        uint32_t g = (uint32_t) (Utils::HalfToFloat(rf[1]) * 255.0f);
        dec[i] = (r << 16) | (g << 8) | 0xFF000000;

        rf += 2;
    }

    *alpha = false;
    return true;
}

#ifdef DIRECTX_TEX_SUPPORT
using namespace  DirectX;

bool DdsFile::DecodeGeneric(uint32_t *dec, bool *alpha) const
{
    ScratchImage img, dec_img;

    /*size_t buf_size = raw.size() + sizeof(DDSHeader);
    if (has_dxt10)
        buf_size += sizeof(DXT10Header);

    uint8_t *buf = new uint8_t[buf_size];
    uint8_t *ptr = buf;

    memcpy(buf, &header, sizeof(DDSHeader));
    ptr += sizeof(DDSHeader);

    if (has_dxt10)
    {
        memcpy(ptr, &dxt10, sizeof(DXT10Header));
        ptr += sizeof(DXT10Header);
    }

    memcpy(ptr, raw.data(), raw.size());

    HRESULT ret = LoadFromDDSMemory(buf, buf_size, DDS_FLAGS_NONE, nullptr, img);
    delete[] buf;

    if (ret != 0)
        return false; */

    if (img.Initialize2D((DXGI_FORMAT)format, header.width, header.height,  1, 0) != 0)
    {
        return false;
    }

    memcpy(img.GetPixels(), raw.data(), min(img.GetPixelsSize(), raw.size()));

    const Image *img0 = img.GetImage(0, 0, 0);
    if (!img0)
        return false;

    if (IsCompressed((DXGI_FORMAT)format))
    {
        if (Decompress(*img0, DXGI_FORMAT_B8G8R8A8_UNORM, dec_img) != 0)
            return false;
    }
    else
    {
        if (Convert(*img0, DXGI_FORMAT_B8G8R8A8_UNORM, TEX_FILTER_DEFAULT, TEX_THRESHOLD_DEFAULT, dec_img) != 0)
            return false;
    }

    const uint8_t *src = dec_img.GetPixels();

    for (uint32_t y = 0; y < header.height; y++)
    {
        const uint32_t *line_src = (const uint32_t *)(src + (y*dec_img.GetImage(0, 0, 0)->rowPitch));
        uint32_t *line_dst = (dec + y*header.width);

        for (uint32_t x = 0; x < header.width; x++)
        {
            *line_dst = *line_src;
            line_dst++;
            line_src++;
        }
    }

    *alpha = HasAlpha(img0->format);
    return true;
}

#endif

uint32_t *DdsFile::Decode(bool *alpha) const
{
    if (format == DXGI_FORMAT_UNKNOWN)
        return nullptr;

    uint32_t dim = header.width * header.height;
    uint32_t *dec = new uint32_t[dim];

    memset(dec, 0, dim*4);

    if (format == DXGI_FORMAT_B8G8R8A8_UNORM || format == DXGI_FORMAT_B8G8R8X8_UNORM)
    {
        if (!DecodeB8G8R8X8(dec, alpha))
        {
            delete[] dec;
            return nullptr;
        }
    }
    else if (format == DXGI_FORMAT_R32G32B32A32_FLOAT)
    {
        if (!DecodeR32G32B32A32Float(dec, alpha))
        {
            delete[] dec;
            return nullptr;
        }
    }
    else if (format == DXGI_FORMAT_R16G16B16A16_FLOAT)
    {
        if (!DecodeR16G16B16A16Float(dec, alpha))
        {
            delete[] dec;
            return nullptr;
        }
    }
    else if (format == DXGI_FORMAT_R32_FLOAT)
    {
        if (!DecodeR32Float(dec, alpha))
        {
            delete[] dec;
            return nullptr;
        }
    }
    else if (format == DXGI_FORMAT_R16G16_FLOAT)
    {
        if (!DecodeR16G16Float(dec, alpha))
        {
            delete[] dec;
            return nullptr;
        }
    }
    else
    {
#ifdef DIRECTX_TEX_SUPPORT

        if (!DecodeGeneric(dec, alpha))
        {
            delete[] dec;
            return nullptr;
        }

#else
        delete[] dec;
        return nullptr;
#endif
    }

    return dec;
}

std::string DdsFile::GetFormatName(int format)
{
    std::string ret;

    if ((size_t)format < dxgi_str.size())
    {
        ret = dxgi_str[(size_t)format];

        if (Utils::BeginsWith(ret, "DXGI_FORMAT_"))
        {
            ret = ret.substr(12);
        }
    }

    return ret;
}
