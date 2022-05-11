#include <unordered_map>
#include "GxtFile.h"

static std::unordered_map<uint32_t, int> gxt_to_dds_format =
{
    { GXT_UBC1, DXGI_FORMAT_BC1_UNORM },
    { GXT_UBC2, DXGI_FORMAT_BC2_UNORM },
    { GXT_UBC3, DXGI_FORMAT_BC3_UNORM },
    { GXT_ARGB8888, DXGI_FORMAT_R8G8B8A8_UNORM },
};

static std::unordered_map<int, uint32_t> dds_to_gxt_format =
{
    { DXGI_FORMAT_BC1_UNORM, GXT_UBC1 },
    { DXGI_FORMAT_BC2_UNORM, GXT_UBC2 },
    { DXGI_FORMAT_BC3_UNORM, GXT_UBC3 },
    { DXGI_FORMAT_R8G8B8A8_UNORM, GXT_ARGB8888 },
};

static std::unordered_map<uint32_t, uint32_t> bpps =
{
    { GXT_UBC1, 4 },
    { GXT_UBC2, 8 },
    { GXT_UBC3, 8 },
    { GXT_ARGB8888, 32 },
};

static std::unordered_map<uint32_t, uint32_t> min_block_sizes =
{
    { GXT_UBC1, 8 },
    { GXT_UBC2, 16 },
    { GXT_UBC3, 16 },
    { GXT_ARGB8888, 4 },
};

GxtFile::GxtFile()
{
    this->big_endian = false;
    this->version = 0x10000003;
}

GxtFile::~GxtFile()
{

}

void GxtFile::Reset()
{
    textures.clear();
    version = 0x10000003;
}

bool GxtFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    FixedMemoryStream stream(const_cast<uint8_t *>(buf), size);
    GXTHeader *header;

    if (!stream.FastRead((uint8_t **)&header, sizeof(GXTHeader)))
        return false;

    if (header->signature != GXT_SIGNATURE)
    {
        DPRINTF("%s: Incorrect GXT signature.\n", FUNCNAME);
        return false;
    }

    version = header->version;

    if (header->p4_palettes != 0 || header->p8_palettes != 0)
    {
        DPRINTF("%s: this type of fxt currently not supported (palettes).\n", FUNCNAME);
        return false;
    }

    if (header->num_textures == 0)
    {
        DPRINTF("%s: there are no textures, me not understand.\n", FUNCNAME);
        return false;
    }

    textures.resize(header->num_textures);

    for (GxtTexture &tex : textures)
    {
        GXTTexture *entry;

        if (!stream.FastRead((uint8_t **)&entry, sizeof(GXTTexture)))
            return false;

        if (entry->pal_idx >= 0)
        {
            DPRINTF("%s: palette not supported yet.\n", FUNCNAME);
            return false;
        }

        tex.texture_type = entry->texture_type;
        tex.texture_format = entry->texture_format;
        tex.width = entry->width;
        tex.height = entry->height;
        tex.mips = entry->mips;

        if (tex.texture_format != GXT_UBC3 && tex.texture_format != GXT_UBC1 && tex.texture_format != GXT_UBC2 && tex.texture_format != GXT_ARGB8888 && tex.texture_format != GXT_RGB888)
        {
            DPRINTF("%s: BUUU %x\n", FUNCNAME, tex.texture_format);
            return false;
        }

        stream.SavePos();

        if (!stream.Seek((off64_t)entry->offset, SEEK_SET))
            return false;

        tex.data.resize(entry->size);
        if (!stream.Read(tex.data.data(), tex.data.size()))
            return false;

        stream.RestorePos();
    }

    return true;
}

uint8_t *GxtFile::Save(size_t *psize)
{
    MemoryStream stream;
    GXTHeader header;

    header.version = version;
    header.num_textures = (uint32_t)textures.size();
    header.textures_data_offset = (uint32_t)(sizeof(GXTHeader) + header.num_textures*sizeof(GXTTexture));
    for (const GxtTexture &tex : textures)
        header.total_textures_size += (uint32_t)tex.data.size();

    if (!stream.Write(&header, sizeof(header)))
        return nullptr;

    uint32_t offset = header.textures_data_offset;

    for (const GxtTexture &tex : textures)
    {
        GXTTexture entry;

        entry.offset = offset;
        entry.size = (uint32_t)tex.data.size();
        entry.texture_type = tex.texture_type;
        entry.texture_format = tex.texture_format;
        entry.width = tex.width;
        entry.height = tex.height;
        entry.mips = tex.mips;

        if (!stream.Write(&entry, sizeof(entry)))
            return nullptr;

        offset += entry.size;
    }

    for (const GxtTexture &tex : textures)
    {
        if (!stream.Write(tex.data.data(), tex.data.size()))
            return nullptr;
    }

    *psize = (size_t)stream.GetSize();
    return stream.GetMemory(true);
}

bool GxtFile::CanConvertToDds(size_t tex_idx) const
{
    if (tex_idx >= textures.size())
        return false;

    const GxtTexture &tex = textures[tex_idx];
    if (tex.texture_type != GXT_LINEAR)
        return false;

    return (gxt_to_dds_format.find(tex.texture_format) != gxt_to_dds_format.end());
}

// Can only do linear gtx, so pretty useless atm

DdsFile *GxtFile::ToDDS(size_t tex_idx) const
{
    if (tex_idx >= textures.size())
        return nullptr;

    const GxtTexture &tex = textures[tex_idx];
    auto it = gxt_to_dds_format.find(tex.texture_format);
    if (it == gxt_to_dds_format.end())
        return nullptr;

    if (tex.texture_type != GXT_LINEAR)
        return nullptr;

    int mips = (tex.mips == 1) ? 0 : (int)tex.mips;
    return new DdsFile(it->second, tex.width, tex.height, mips, tex.data.data(), tex.data.size());
}
