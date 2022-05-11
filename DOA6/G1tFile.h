#ifndef G1TFILE_H
#define G1TFILE_H

#include "DdsFile.h"

#define G1T_SIGNATURE   0x47315447

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED G1THeader
{
    uint32_t signature; // 0
    uint32_t version; // 4
    uint32_t file_size; // 8
    uint32_t table_offset; // 0xC
    uint32_t num_textures; // 0x10
    uint32_t plattform; // 0x14 - 0xA = PC
    uint32_t unk_data_size; // 0x18 If not 0, there are this X bytes after the offset table until the first texture header
    uint32_t unk_1C; // ?
};
CHECK_STRUCT_SIZE(G1THeader, 0x20);

struct PACKED G1TEntryHeader
{
    uint8_t mip_sys; // 0-3: text sys; 4-7: mip maps count. Game check text sys to be between 0-4
    uint8_t format;
    uint8_t dxdy; // 0-3: width; 4-7: height
    uint8_t unk_3[4];
    uint8_t extra_header_version; // 7
};
CHECK_STRUCT_SIZE(G1TEntryHeader, 0x8);

struct PACKED G1TEntryHeader2
{
    uint32_t size; // 0 - Size of this. Usually either 0xC or 0x14
    uint32_t unk_04; // Is always zero?
    uint8_t array_other; // 8 - Bits 5-7 are num of interleaved images. Bits 0-3: unknown.
    uint8_t unk_09[3];

    // Only when size > C
    uint32_t width;
    uint32_t height;
};
CHECK_STRUCT_SIZE(G1TEntryHeader2, 0x14);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct G1tTexture
{
    uint32_t width;
    uint32_t height;
    uint8_t format;
    uint8_t sys;
    uint8_t mips;
    uint8_t unk_3[4];
    uint8_t extra_header_version;

    uint8_t array_size;

    std::vector<uint8_t> image_data;
    std::vector<uint8_t> extra_header;

    int IdealMipsCount() const;
};

class G1tFile : public BaseFile
{
private:

    std::vector<G1tTexture> textures;
    std::vector<uint8_t> extra_header;
    std::vector<uint8_t> unk_data;

    uint32_t version;
    uint32_t plattform;
    uint32_t unk_1C;

    size_t CalculateFileSize() const;

protected:

    void Reset();

public:
    G1tFile();
    virtual ~G1tFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    inline size_t GetNumTextures() const { return textures.size(); }    

    static uint32_t *Decode(const uint8_t *buf, size_t buf_size, uint32_t width, uint32_t height, uint8_t g1t_format, bool *alpha, bool show_error);
    static uint32_t *Decode(const G1tTexture &tex, bool *alpha, bool show_error);
    uint32_t *Decode(size_t idx, bool *alpha, bool show_error) const;

    static DdsFile *ToDDS(const G1tTexture &tex);
    DdsFile *ToDDS(size_t idx) const;

    static bool FromDDS(G1tTexture &tex, const DdsFile &dds, uint8_t *fmt=nullptr, uint8_t *prev_fmt=nullptr);
    bool FromDDS(size_t idx, const DdsFile &dds, uint8_t *fmt=nullptr, uint8_t *prev_fmt=nullptr);

    static size_t CalculateTextureSize(const G1tTexture &tex, int override_levels=-1);
    size_t CalculateTextureSize(size_t idx, int override_levels=-1) const;

    bool IsArrayTexture(size_t idx) const;
    bool DecomposeArrayTexture(size_t idx, std::vector<G1tTexture> &ret, bool only_firstlevel, bool show_error) const;
    bool DecomposeArrayTextureFast(size_t idx, std::vector<uint8_t *> &ret, bool show_error);

    static bool ComposeArrayTexture(G1tTexture &ret, const std::vector<G1tTexture> &textures, bool show_error);
    bool ComposeArrayTexture(size_t idx, const std::vector<G1tTexture> &textures, bool show_error);

    static bool ReduceMipsLevel(G1tTexture &tex, uint8_t levels);
    bool ReduceMipsLevel(size_t idx, uint8_t levels);

    inline std::vector<G1tTexture> &GetTextures() { return textures; }
    inline const std::vector<G1tTexture> &GetTextures() const { return textures; }

    inline G1tTexture &operator[](size_t n) { return textures[n]; }
    inline const G1tTexture &operator[](size_t n) const { return textures[n]; }

    inline std::vector<G1tTexture>::iterator begin() { return textures.begin(); }
    inline std::vector<G1tTexture>::iterator end() { return textures.end(); }

    inline std::vector<G1tTexture>::const_iterator begin() const { return textures.begin(); }
    inline std::vector<G1tTexture>::const_iterator end() const { return textures.end(); }

    static int G1tToDdsFormat(uint8_t g1t_fmt);
    static int DdsToG1tFormat(int dds_fmt);

    static int IdealMipsCount(int width, int height);
};

#endif // G1TFILE_H
