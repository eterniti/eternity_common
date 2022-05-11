#include <unordered_set>

#include "G1tFile.h"
#include "debug.h"

static const std::vector<std::pair<int, int>> g1t_to_dxgi_lookup =
{
    { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB }, // 0x0
    { DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB }, // 0x1
    { DXGI_FORMAT_R32_FLOAT, 0 }, // 0x2
    { DXGI_FORMAT_R16G16B16A16_FLOAT, 0 }, // 0x3
    { DXGI_FORMAT_R32G32B32A32_FLOAT, 0 }, // 0x4
    { DXGI_FORMAT_R24_UNORM_X8_TYPELESS, 0 }, // 0x5
    { DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC1_UNORM_SRGB }, // 0x6
    { DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM_SRGB }, // 0x7
    { DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM_SRGB }, // 0x8
    { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB }, // 0x9
    { DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB }, // 0xa
    { DXGI_FORMAT_R32_FLOAT, 0 }, // 0xb
    { DXGI_FORMAT_R16G16B16A16_FLOAT, 0 }, // 0xc
    { DXGI_FORMAT_R32G32B32A32_FLOAT, 0 }, // 0xd
    { 0, 0 }, // 0xe
    { DXGI_FORMAT_A8_UNORM, 0 }, // 0xf
    { DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC1_UNORM_SRGB }, // 0x10
    { DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM_SRGB }, // 0x11
    { DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM_SRGB }, // 0x12
    { DXGI_FORMAT_R24_UNORM_X8_TYPELESS, 0 }, // 0x13
    { DXGI_FORMAT_R16_UNORM, 0 }, // 0x14
    { DXGI_FORMAT_R16_UNORM, 0 }, // 0x15
    { DXGI_FORMAT_R16_UNORM, 0 }, // 0x16
    { DXGI_FORMAT_R16_UNORM, 0 }, // 0x17
    { DXGI_FORMAT_A8_UNORM, 0 }, // 0x18
    { DXGI_FORMAT_B5G6R5_UNORM, 0 }, // 0x19
    { DXGI_FORMAT_B5G5R5A1_UNORM, 0 }, // 0x1a
    { 0, 0 }, // 0x1b
    { DXGI_FORMAT_B5G6R5_UNORM, 0 }, // 0x1c
    { DXGI_FORMAT_B5G5R5A1_UNORM, 0 }, // 0x1d
    { 0, 0 }, // 0x1e
    { 0, 0 }, // 0x1f
    { 0, 0 }, // 0x20
    { DXGI_FORMAT_B8G8R8X8_UNORM, DXGI_FORMAT_B8G8R8X8_UNORM_SRGB }, // 0x21
    { DXGI_FORMAT_B8G8R8X8_UNORM, DXGI_FORMAT_B8G8R8X8_UNORM_SRGB }, // 0x22
    { DXGI_FORMAT_R16G16_UNORM, 0 }, // 0x23
    { DXGI_FORMAT_R16G16_UNORM, 0 }, // 0x24
    { 0, 0 }, // 0x25
    { 0, 0 }, // 0x26
    { 0, 0 }, // 0x27
    { 0, 0 }, // 0x28
    { 0, 0 }, // 0x29
    { DXGI_FORMAT_R8_UNORM, 0 }, // 0x2a
    { 0, 0 }, // 0x2b
    { 0, 0 }, // 0x2c
    { 0, 0 }, // 0x2d
    { 0, 0 }, // 0x2e
    { 0, 0 }, // 0x2f
    { 0, 0 }, // 0x30
    { 0, 0 }, // 0x31
    { 0, 0 }, // 0x32
    { 0, 0 }, // 0x33
    { 0, 0 }, // 0x34
    { 0, 0 }, // 0x35
    { 0, 0 }, // 0x36
    { 0, 0 }, // 0x37
    { 0, 0 }, // 0x38
    { 0, 0 }, // 0x39
    { 0, 0 }, // 0x3a
    { 0, 0 }, // 0x3b
    { 0, 0 }, // 0x3c
    { 0, 0 }, // 0x3d
    { 0, 0 }, // 0x3e
    { 0, 0 }, // 0x3f
    { DXGI_FORMAT_R10G10B10A2_UNORM, 0 }, // 0x40
    { DXGI_FORMAT_R16G16B16A16_UNORM, 0 }, // 0x41
    { DXGI_FORMAT_R10G10B10A2_UNORM, 0 }, // 0x42
    { DXGI_FORMAT_R16G16B16A16_UNORM, 0 }, // 0x43
    { 0, 0 }, // 0x44
    { 0, 0 }, // 0x45
    { DXGI_FORMAT_R8G8_UNORM, 0 }, // 0x46
    { 0, 0 }, // 0x47
    { 0, 0 }, // 0x48
    { 0, 0 }, // 0x49
    { 0, 0 }, // 0x4a
    { 0, 0 }, // 0x4b
    { DXGI_FORMAT_R32G32_FLOAT, 0 }, // 0x4c
    { DXGI_FORMAT_R32G32_FLOAT, 0 }, // 0x4d
    { DXGI_FORMAT_R32_FLOAT, 0 }, // 0x4e
    { DXGI_FORMAT_R32_FLOAT, 0 }, // 0x4f
    { 0, 0 }, // 0x50
    { 0, 0 }, // 0x51
    { 0, 0 }, // 0x52
    { 0, 0 }, // 0x53
    { 0, 0 }, // 0x54
    { 0, 0 }, // 0x55
    { 0, 0 }, // 0x56
    { 0, 0 }, // 0x57
    { 0, 0 }, // 0x58
    { DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC1_UNORM_SRGB }, // 0x59
    { DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM_SRGB }, // 0x5a
    { DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM_SRGB }, // 0x5b
    { DXGI_FORMAT_BC4_UNORM, 0 }, // 0x5c
    { DXGI_FORMAT_BC5_UNORM, 0 }, // 0x5d
    { DXGI_FORMAT_BC6H_UF16, 0 }, // 0x5e
    { DXGI_FORMAT_BC7_UNORM, DXGI_FORMAT_BC7_UNORM_SRGB }, // 0x5f
    { DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC1_UNORM_SRGB }, // 0x60
    { DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM_SRGB }, // 0x61
    { DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM_SRGB }, // 0x62
    { DXGI_FORMAT_BC4_UNORM, 0 }, // 0x63
    { DXGI_FORMAT_BC5_UNORM, 0 }, // 0x64
    { DXGI_FORMAT_BC6H_UF16, 0 }, // 0x65
    { DXGI_FORMAT_BC7_UNORM, DXGI_FORMAT_BC7_UNORM_SRGB }, // 0x66
    { DXGI_FORMAT_R8G8B8A8_UINT, 0 }, // 0x67
    { DXGI_FORMAT_R8G8_UINT, 0 }, // 0x68
    { DXGI_FORMAT_R16G16_FLOAT, 0 }, // 0x69
    { DXGI_FORMAT_R16_FLOAT, 0 }, // 0x6a
    { DXGI_FORMAT_R11G11B10_FLOAT, 0 }, // 0x6b
    { DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS, 0 }, // 0x6c
    { 0, 0 }, // 0x6d
    { 0, 0 }, // 0x6e
    { 0, 0 }, // 0x6f
    { 0, 0 }, // 0x70
    { 0, 0 }, // 0x71
    { DXGI_FORMAT_R8_UNORM, 0 }, // 0x72
    { DXGI_FORMAT_R8G8_UNORM, 0 }, // 0x73
    { DXGI_FORMAT_R8G8B8A8_UINT, 0 }, // 0x74
    { DXGI_FORMAT_R8G8_UINT, 0 }, // 0x75
    { DXGI_FORMAT_R16G16_FLOAT, 0 }, // 0x76
    { DXGI_FORMAT_R16_FLOAT, 0 }, // 0x77
    { DXGI_FORMAT_R11G11B10_FLOAT, 0 }, // 0x78
    { 0, 0 }, // 0x79
    { 0, 0 }, // 0x7a
    { 0, 0 }, // 0x7b
    { 0, 0 }, // 0x7c
};

// Size in bits
static const std::vector<size_t> point_sizes =
{
    32, // 0x0
    32, // 0x1
    32, // 0x2
    64, // 0x3
    128, // 0x4
    32, // 0x5
    4, // 0x6
    8, // 0x7
    8, // 0x8
    32, // 0x9
    32, // 0xa
    32, // 0xb
    64, // 0xc
    128, // 0xd
    0, // 0xe
    8, // 0xf
    4, // 0x10
    8, // 0x11
    8, // 0x12
    32, // 0x13
    16, // 0x14
    16, // 0x15
    16, // 0x16
    16, // 0x17
    8, // 0x18
    16, // 0x19
    16, // 0x1a
    16, // 0x1b
    16, // 0x1c
    16, // 0x1d
    16, // 0x1e
    32, // 0x1f
    32, // 0x20
    32, // 0x21
    32, // 0x22
    32, // 0x23
    0, // 0x24
    0, // 0x25
    0, // 0x26
    0, // 0x27
    0, // 0x28
    0, // 0x29
    8, // 0x2a
    0, // 0x2b
    0, // 0x2c
    0, // 0x2d
    0, // 0x2e
    0, // 0x2f
    0, // 0x30
    0, // 0x31
    0, // 0x32
    0, // 0x33
    0, // 0x34
    0, // 0x35
    0, // 0x36
    0, // 0x37
    0, // 0x38
    0, // 0x39
    0, // 0x3a
    0, // 0x3b
    0, // 0x3c
    0, // 0x3d
    0, // 0x3e
    0, // 0x3f
    32, // 0x40
    64, // 0x41
    32, // 0x42
    64, // 0x43
    0, // 0x44
    0, // 0x45
    16, // 0x46
    0, // 0x47
    0, // 0x48
    0, // 0x49
    0, // 0x4a
    0, // 0x4b
    64, // 0x4c
    64, // 0x4d
    32, // 0x4e
    32, // 0x4f
    0, // 0x50
    0, // 0x51
    0, // 0x52
    0, // 0x53
    0, // 0x54
    0, // 0x55
    0, // 0x56
    0, // 0x57
    0, // 0x58
    4, // 0x59
    8, // 0x5a
    8, // 0x5b
    4, // 0x5c
    8, // 0x5d
    8, // 0x5e
    8, // 0x5f
    4, // 0x60
    8, // 0x61
    8, // 0x62
    4, // 0x63
    8, // 0x64
    8, // 0x65
    8, // 0x66
    32, // 0x67
    16, // 0x68
    32, // 0x69
    16, // 0x6a
    32, // 0x6b
    64, // 0x6c
    0, // 0x6d
    0, // 0x6e
    0, // 0x6f
    0, // 0x70
    0, // 0x71
    8, // 0x72
    16, // 0x73
    32, // 0x74
    16, // 0x75
    32, // 0x76
    16, // 0x77
    32, // 0x78
    0, // 0x79
    0, // 0x7a
    0, // 0x7b
    0, // 0x7c
};

int G1tFile::G1tToDdsFormat(uint8_t g1t_fmt)
{
    if (g1t_fmt < g1t_to_dxgi_lookup.size())
    {
        return g1t_to_dxgi_lookup[g1t_fmt].first;
    }

    return DXGI_FORMAT_UNKNOWN;
}

int G1tFile::DdsToG1tFormat(int dds_fmt)
{
    if (dds_fmt == DXGI_FORMAT_UNKNOWN)
        return -1;

    // Because some formats have several variants, let's choose first some favorites for some of the common formats, otherwise search in the table

    switch (dds_fmt)
    {
        case DXGI_FORMAT_BC1_UNORM:
            return 0x59;

        case DXGI_FORMAT_BC2_UNORM:
            return 0x5A;

        case DXGI_FORMAT_BC3_UNORM:
            return 0x5B;
    }

    for (size_t i = 0; i < g1t_to_dxgi_lookup.size(); i++)
    {
        auto &it = g1t_to_dxgi_lookup[i];

        if (it.first == dds_fmt || it.second == dds_fmt)
        {
            return (int)i;
        }
    }

    return -1;
}

// This function may not return good info for uncompressed formats with less than 32 bits
static void get_pb_size(int g1t_fmt, size_t *point_size, size_t *block_size)
{
    *point_size = point_sizes[(size_t)g1t_fmt];
    *block_size = (*point_size*16)/8;

    if (*point_size >= 32)
        *block_size = *point_size / 8;
}

G1tFile::G1tFile()
{
    this->big_endian = false;
}

G1tFile::~G1tFile()
{

}

void G1tFile::Reset()
{
    textures.clear();
    extra_header.clear();
    unk_data.clear();

    version = 60;
    plattform = 0xA;
    unk_1C = 0;
}

bool G1tFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(G1THeader))
        return false;

    const G1THeader *hdr = (const G1THeader *)buf;

    version = Utils::GetShortVersion(hdr->version);
    plattform = hdr->plattform;    
    unk_1C = hdr->unk_1C;
    textures.resize(hdr->num_textures);

    if (hdr->table_offset > sizeof(G1THeader))
    {
        extra_header.resize(hdr->table_offset - sizeof(G1THeader));
        memcpy(extra_header.data(), (hdr+1), extra_header.size());
    }

    const uint32_t *table = (const uint32_t *)(buf + hdr->table_offset);

    if (hdr->unk_data_size > 0)
    {
        const uint8_t *extra_data = (const uint8_t *)(table + hdr->num_textures);
        unk_data.resize(hdr->unk_data_size);
        memcpy(unk_data.data(), extra_data, hdr->unk_data_size);
    }

    for (size_t i = 0; i < textures.size(); i++)
    {
        const G1TEntryHeader *ehdr = (const G1TEntryHeader *) GetOffsetPtr(table, table, (uint32_t)i);
        //UPRINTF("Offset 0x%x\n", Utils::DifPointer(ehdr, buf));

        textures[i].mips = ehdr->mip_sys >> 4;
        textures[i].sys = ehdr->mip_sys & 0xF;

        if (textures[i].sys != 0)
        {
            //DPRINTF("Warning: sys not 0.\n");
            // swtm textures have this
        }

        textures[i].format = ehdr->format;
        textures[i].width = (1 << (ehdr->dxdy&0xF));
        textures[i].height = (1 << (ehdr->dxdy >> 4));


        //DPRINTF("Width: %d, height: %d mips = %d\n", textures[i].width, textures[i].height, textures[i].mips);

        memcpy(textures[i].unk_3, ehdr->unk_3, sizeof(textures[i].unk_3));
        textures[i].extra_header_version = ehdr->extra_header_version;

        const uint8_t *tex_buf = (const uint8_t *)(ehdr + 1);
        if (ehdr->extra_header_version > 0)
        {
            const G1TEntryHeader2 *ehdr2 = (const G1TEntryHeader2 *)tex_buf;

            if (ehdr2->size < 0xC || ehdr2->size >= 0x14)
            {
                DPRINTF("%s: Warning, unknown extra header size 0x%x (on texture index %Id)\n\n"
                        "On modded .g1t, this is *usually* an indication of file corrupted by the writing application.\n"
                        "On original .g1t, this could be an indication of a new format version not supported yet.\n\n"
                        "A crash may follow soon.\n", FUNCNAME, ehdr2->size, i);
            }

            textures[i].extra_header.resize(ehdr2->size);
            memcpy(textures[i].extra_header.data(), ehdr2, ehdr2->size);

            if (ehdr2->size >= 0xC)
                textures[i].array_size = (ehdr2->array_other >> 4);
            else
                textures[i].array_size = 0;

            if (ehdr2->size >= 0x10)
                textures[i].width = *(const uint32_t *)&tex_buf[0xC];

            if (ehdr2->size >= 0x14)
                textures[i].height = *(const uint32_t *)&tex_buf[0x10];

            tex_buf += ehdr2->size;
        }

        if (i == textures.size()-1)
        {
            textures[i].image_data.resize(Utils::DifPointer(buf+size, tex_buf));
        }
        else
        {
            textures[i].image_data.resize(Utils::DifPointer(GetOffsetPtr(table, table, (uint32_t)i+1), tex_buf));
        }

        //DPRINTF("Size = 0x%Ix\n", textures[i].image_data.size());

        memcpy(textures[i].image_data.data(), tex_buf, textures[i].image_data.size());
    }

    if (unk_data.size() > 0)
    {
        DPRINTF("Warning: this g1t may be a cubemap, which is not supported.\n"
                "Writing to this .g1t may corrupt it.\n");
    }

    if (textures.size() > 1)
    {
        for (size_t i = 0; i < textures.size(); i++)
        {
            if (IsArrayTexture(i))
            {
                DPRINTF("Warning: this g1t uses multiple textures, where at least one of them is also an array texture, this is currently not supported.\n"
                        "A write operation may corrupt mipmaps or not update them.\n");
                break;
            }
        }
    }

    return true;
}

size_t G1tFile::CalculateFileSize() const
{
    size_t size = sizeof(G1THeader) + extra_header.size() + (textures.size() * 4) + unk_data.size();

    for (const G1tTexture &tex : textures)
    {
        size += sizeof(G1TEntryHeader);

        if (tex.extra_header_version > 0)
        {
            const G1TEntryHeader2 *ehdr2 = (const G1TEntryHeader2 *)tex.extra_header.data();
            size += ehdr2->size;
        }

        size += tex.image_data.size();
    }

    return size;
}

uint8_t *G1tFile::Save(size_t *psize)
{
    *psize = CalculateFileSize();
    uint8_t *buf = new uint8_t[*psize];

    memset(buf, 0, *psize);

    G1THeader *hdr = (G1THeader *)buf;
    uint8_t *extra_header_ptr = (uint8_t *)(hdr + 1);
    uint32_t *table = (uint32_t *)(extra_header_ptr + extra_header.size());
    uint8_t *extra_data = (uint8_t *)(table + textures.size());
    uint8_t *ptr = extra_data + unk_data.size();

    hdr->signature = G1T_SIGNATURE;
    hdr->version = Utils::GetLongVersion(version);
    hdr->file_size = (uint32_t)*psize;
    hdr->table_offset = Utils::DifPointer(table, buf);
    hdr->num_textures = (uint32_t)textures.size();
    hdr->plattform = plattform;
    hdr->unk_data_size = (uint32_t)unk_data.size();
    hdr->unk_1C = unk_1C;

    if (extra_header.size() > 0)
        memcpy(extra_header_ptr, extra_header.data(), extra_header.size());

    if (unk_data.size() > 0)
        memcpy(extra_data, unk_data.data(), unk_data.size());

    for (size_t i = 0; i < textures.size(); i++)
    {
        const G1tTexture &tex = textures[i];

        table[i] = Utils::DifPointer(ptr, table);

        G1TEntryHeader *ehdr = (G1TEntryHeader *)ptr;
        ptr += sizeof(G1TEntryHeader);

        if (tex.sys > 0xF)
        {
            DPRINTF("%s: Invalid out of range sys value (%d).\n", FUNCNAME, tex.sys);
            delete[] buf;
            return nullptr;
        }

        if (tex.mips > 0xF)
        {
            DPRINTF("%s: Invalid out of range mips value (%d).\n", FUNCNAME, tex.mips);
            delete[] buf;
            return nullptr;
        }

        ehdr->mip_sys = (tex.sys) | (uint8_t)(tex.mips << 4);
        ehdr->format = tex.format;

        uint8_t wlog = (uint8_t)log2(tex.width);
        uint8_t hlog = (uint8_t)log2(tex.height);

        if ((wlog > 0xF || hlog > 0xF) && tex.extra_header.size() < 0x14)
        {
            DPRINTF("%s: Invalid out of range width or height log2 (width = %d, wlog = %d, height = %d, hlog = %d).\n", FUNCNAME, tex.width, wlog, tex.height, hlog);
            delete[] buf;
            return nullptr;
        }

        ehdr->dxdy = wlog | (uint8_t)(hlog << 4);
        memcpy(ehdr->unk_3, tex.unk_3, sizeof(tex.unk_3));
        ehdr->extra_header_version = tex.extra_header_version;

        if (tex.extra_header_version > 0)
        {
            const G1TEntryHeader2 *ehdr2_in = (const G1TEntryHeader2 *)tex.extra_header.data();
            G1TEntryHeader2 *ehdr2 = (G1TEntryHeader2 *)ptr;
            ptr += ehdr2_in->size;

            memcpy(ehdr2, ehdr2_in, ehdr2_in->size);

            if (ehdr2->size >= 0xC)
            {
                if (tex.array_size > 0xF)
                {
                    DPRINTF("%s: Invalid out of range value of array size (%d).\n", FUNCNAME, tex.array_size);
                    delete[] buf;
                    return nullptr;
                }

                ehdr2->array_other = (ehdr2->array_other & 0xF) | (uint8_t)(tex.array_size << 4);
            }

            if (ehdr2->size >= 0x10)
                ehdr2->width = tex.width;

            if (ehdr2->size >= 0x14)
                ehdr2->height = tex.height;
        }

        memcpy(ptr, tex.image_data.data(), tex.image_data.size());
        ptr += tex.image_data.size();
    }

    return buf;
}

uint32_t *G1tFile::Decode(const uint8_t *buf, size_t buf_size, uint32_t width, uint32_t height, uint8_t g1t_format, bool *alpha, bool show_error)
{
    int dds_format = G1tToDdsFormat(g1t_format);

    if (dds_format < 0)
    {
        if (show_error)
        {
            DPRINTF("Not supported g1t texture type: 0x%x\n", g1t_format);
        }

        return nullptr;
    }

    DdsFile dds = DdsFile(dds_format, width, height, 0, buf, buf_size);
    return dds.Decode(alpha);
}

uint32_t *G1tFile::Decode(const G1tTexture &tex, bool *alpha, bool show_error)
{
    return Decode(tex.image_data.data(), tex.image_data.size(), tex.width, tex.height, tex.format, alpha, show_error);
}

uint32_t *G1tFile::Decode(size_t idx, bool *alpha, bool show_error) const
{
    if (idx >= textures.size())
        return nullptr;

    return Decode(textures[idx], alpha, show_error);
}

DdsFile *G1tFile::ToDDS(const G1tTexture &tex)
{
    int mips = tex.mips;

    if (mips == 1)
        mips = 0;

    int format = G1tToDdsFormat(tex.format);
    if (format == DXGI_FORMAT_UNKNOWN || format < 0)
        return nullptr;

    return new DdsFile(format, tex.width, tex.height, mips, tex.image_data.data(), tex.image_data.size());
}

DdsFile *G1tFile::ToDDS(size_t idx) const
{
    if (idx >= textures.size())
        return nullptr;

    return ToDDS(textures[idx]);
}

bool G1tFile::FromDDS(G1tTexture &tex, const DdsFile &dds, uint8_t *fmt, uint8_t *prev_fmt)
{
    int dds_fmt = dds.GetFormat();

    if (dds_fmt == DXGI_FORMAT_UNKNOWN)
        return false;

    if (prev_fmt)
        *prev_fmt = tex.format;

    int current_dds_fmt = G1tToDdsFormat(tex.format);

    //DPRINTF("%d %d\n", dds_fmt, current_dds_fmt);

    if (dds_fmt != current_dds_fmt)
    {
        int new_fmt = DdsToG1tFormat(dds_fmt);
        if (new_fmt < 0)
            return false;

        //DPRINTF("New fmt %d.\n", new_fmt);
        tex.format = (uint8_t)new_fmt;
    }

    if (fmt)
        *fmt = tex.format;

    uint32_t w = dds.GetWidth();
    uint32_t h = dds.GetHeight();

    bool ok_w = false, ok_h = false;

    for (uint8_t i = 0; i < 32 && (!ok_w || !ok_h); i++)
    {
        if ((uint32_t)(1 << i) == w)
            ok_w = true;

        if ((uint32_t)(1 << i) == h)
            ok_h = true;
    }

    if (!ok_w || !ok_h)
    {
        if (tex.extra_header_version == 0 || tex.extra_header.size() < 0x14)
        {
            DPRINTF("%s: width or height not supported in this g1t (requeriment of power of 2). Dds width=%d, dds height=%d.\n", FUNCNAME, w, h);
            return false;
        }
    }

    tex.width = w;
    tex.height = h;
    tex.mips = (uint8_t)dds.GetMips();

    if (tex.mips == 0)
        tex.mips = 1;

    size_t raw_size;
    const uint8_t *raw = dds.GetRaw(&raw_size);

    tex.image_data.resize(raw_size);
    memcpy(tex.image_data.data(), raw, raw_size);

    return true;
}

bool G1tFile::FromDDS(size_t idx, const DdsFile &dds, uint8_t *fmt, uint8_t *prev_fmt)
{
    if (idx >= textures.size())
        return false;

    return FromDDS(textures[idx], dds, fmt, prev_fmt);
}

size_t G1tFile::CalculateTextureSize(const G1tTexture &tex, int override_levels)
{
    size_t size = 0;

    uint8_t levels = (override_levels >= 0) ? (uint8_t) override_levels : tex.mips;

    if (levels == 0)
        levels++;

    uint32_t width = tex.width;
    uint32_t height = tex.height;

    if (tex.format >= point_sizes.size())
        return 0;

    for (uint8_t i = 0; i < levels; i++)
    {
        size_t point_size, block_size, this_size;

        get_pb_size(tex.format, &point_size, &block_size);
        this_size = (width*height*point_size) / 8;

        if (this_size < block_size)
            this_size = block_size;

        size += this_size;

        width /= 2;
        height /= 2;

        if (width == 0) width = 1;
        if (height == 0) height = 1;
    }

    return size;
}

// The function may not work with uncompressed textures. Anyway, it is fine for our purposes
size_t G1tFile::CalculateTextureSize(size_t idx, int override_levels) const
{
    if (idx >= textures.size())
        return 0;

    return CalculateTextureSize(textures[idx], override_levels);
}

bool G1tFile::IsArrayTexture(size_t idx) const
{
    if (idx >= textures.size())
        return false;

    const G1tTexture &tex = textures[idx];
    return (tex.sys == 3 && tex.array_size > 1 && unk_data.size() == 0);
}

bool G1tFile::DecomposeArrayTexture(size_t idx, std::vector<G1tTexture> &ret, bool only_firstlevel, bool show_error) const
{
    ret.clear();

    if (!IsArrayTexture(idx))
        return false;

    const G1tTexture &tex = textures[idx];
    size_t tex_size = CalculateTextureSize(idx);

    if (tex.image_data.size() != tex_size*tex.array_size)
    {
        if (show_error)
        {
            DPRINTF("DecomposeArrayTexture: size = 0x%Ix, expected size = 0x%Ix\n", tex.image_data.size(), tex_size*tex.array_size);
        }

        return false;
    }

    ret.resize(tex.array_size);

    for (G1tTexture &r : ret)
    {
        r.width = tex.width;
        r.height = tex.height;
        r.format = tex.format;
        r.sys = 0;
        r.mips = (only_firstlevel) ? 1 : tex.mips;
        memcpy(r.unk_3, tex.unk_3, sizeof(r.unk_3));
        r.extra_header_version = tex.extra_header_version;
        r.array_size = 0;
        r.extra_header = tex.extra_header;
        r.image_data.reserve(tex_size);
    }

    const uint8_t *ptr = tex.image_data.data();
    uint8_t levels = (only_firstlevel) ? 1 : tex.mips;;

    if (levels == 0)
        levels++;

    uint32_t width = tex.width;
    uint32_t height = tex.height;

    if (tex.format >= point_sizes.size())
    {
        ret.clear();
        return false;
    }

    for (uint8_t i = 0; i < levels; i++)
    {
        size_t point_size, block_size, this_size;

        get_pb_size(tex.format, &point_size, &block_size);
        this_size = (width*height*point_size) / 8;

        if (this_size < block_size)
            this_size = block_size;

        for (G1tTexture &r : ret)
        {
            size_t previous_size = r.image_data.size();

            r.image_data.resize(previous_size + this_size);
            memcpy(r.image_data.data()+previous_size, ptr, this_size);

            ptr += this_size;
        }

        width /= 2;
        height /= 2;

        if (width == 0) width = 1;
        if (height == 0) height = 1;
    }

    return true;
}

bool G1tFile::DecomposeArrayTextureFast(size_t idx, std::vector<uint8_t *> &ret, bool show_error)
{
    ret.clear();

    if (!IsArrayTexture(idx))
        return false;

    G1tTexture &tex = textures[idx];
    size_t tex_size = CalculateTextureSize(idx);

    if (tex.image_data.size() != tex_size*tex.array_size)
    {
        if (show_error)
        {
            DPRINTF("DecomposeArrayTexture: size = 0x%Ix, expected size = 0x%Ix\n", tex.image_data.size(), tex_size*tex.array_size);
        }

        return false;
    }

    tex_size = CalculateTextureSize(idx, 1);

    uint8_t *ptr = tex.image_data.data();
    ret.resize(tex.array_size);

    for (uint8_t* &r : ret)
    {
        r = ptr;
        ptr += tex_size;
    }

    return true;
}

bool G1tFile::ComposeArrayTexture(G1tTexture &ret, const std::vector<G1tTexture> &textures, bool show_error)
{
    if (textures.size() == 0 || textures.size() > 0xF)
        return false;

    ret.width = textures.front().width;
    ret.height = textures.front().height;
    ret.format = textures.front().format;
    ret.sys = 3;
    ret.mips = textures.front().mips;
    memcpy(ret.unk_3, textures.front().unk_3, sizeof(ret.unk_3));
    ret.extra_header_version = textures.front().extra_header_version;
    ret.array_size = (uint8_t)textures.size();
    ret.extra_header = textures.front().extra_header;
    ret.image_data.resize(textures.front().image_data.size()*ret.array_size);

    uint8_t levels = ret.mips;

    if (levels == 0)
        levels++;

    uint8_t *ptr = ret.image_data.data();
    size_t capacity = ret.image_data.size();
    size_t single_image_size = textures.front().image_data.size();
    uint32_t width = ret.width;
    uint32_t height = ret.height;

    std::vector<const uint8_t *> raw_ptrs;
    raw_ptrs.reserve(textures.size());

    for (const G1tTexture &tex: textures)
    {
        if (tex.width != ret.width || tex.height != ret.height)
        {
            if (show_error)
            {
                DPRINTF("%s: Mismatch of dimension, all images must have same dimension.\n", FUNCNAME);
            }

            return false;
        }

        if (tex.format != ret.format)
        {
            if (show_error)
            {
                DPRINTF("%s: Mismatch of format, all images must have same format.\n", FUNCNAME);
            }

            return false;
        }

        if (tex.mips != ret.mips)
        {
            if (show_error)
            {
                DPRINTF("%s: Mismatch of mips, all images must have same mips count.\n", FUNCNAME);
            }

            return false;
        }

        if (tex.image_data.size() != single_image_size)
        {
            if (show_error)
            {
                DPRINTF("%s: Mismatch of raw data size, all images must have same raw size.\n", FUNCNAME);
            }

            return false;
        }

        raw_ptrs.push_back(tex.image_data.data());
    }

    for (uint8_t i = 0; i < levels; i++)
    {
        size_t point_size, block_size, this_size;

        get_pb_size(ret.format, &point_size, &block_size);
        this_size = (width*height*point_size) / 8;

        if (this_size < block_size)
            this_size = block_size;

        for (size_t t = 0; t < textures.size(); t++)
        {
            if (this_size > capacity)
            {
                if (show_error)
                {
                    DPRINTF("%s: Internal error, not enough capacity in buffer.\n", FUNCNAME);
                }

                return false;
            }

            memcpy(ptr, raw_ptrs[t], this_size);
            ptr += this_size;
            raw_ptrs[t] += this_size;
            capacity -= this_size;
        }

        width /= 2;
        height /= 2;

        if (width == 0) width = 1;
        if (height == 0) height = 1;
    }

    return true;
}

bool G1tFile::ComposeArrayTexture(size_t idx, const std::vector<G1tTexture> &textures, bool show_error)
{
    if (idx >= textures.size())
        return false;

    return ComposeArrayTexture(this->textures[idx], textures, show_error);
}

bool G1tFile::ReduceMipsLevel(G1tTexture &tex, uint8_t levels)
{
    if (levels == 0)
        levels = 1;

    if (levels > tex.mips)
        return false;

    if (levels == tex.mips)
        return true;

    size_t size = CalculateTextureSize(tex, levels);
    if (size > tex.image_data.size())
        return false;

    tex.mips = levels;
    tex.image_data.resize(size);

    return true;
}

bool G1tFile::ReduceMipsLevel(size_t idx, uint8_t levels)
{
    if (idx >= textures.size())
        return false;

    return ReduceMipsLevel(textures[idx], levels);
}

int G1tFile::IdealMipsCount(int width, int height)
{
    int count = 0;

    while (width > 2 && height > 2)
    {
        count++;
        width /= 2;
        height /= 2;
    }

    if (count <= 0)
        return 1;

    return count;
}
