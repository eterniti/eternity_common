#ifndef SRSAFILE_H
#define SRSAFILE_H

#include "FixedMemoryStream.h"
#include "FileStream.h"
#include "SrsCommon.h"

#define SRSA_SIGNATURE  0x53525341
#define KTSR_SIGNATURE	0x5253544B

// Types signatures
#define SUBP_SIGNATURE  0x368C88BD
#define AUDIO_SIGNATURE 0x70CBCCC5
#define UNK_SIGNATURE	0xC18EC4C9 /* Seen in exe */
#define UNK_SIGNATURE2	0xCABE6F83 /* Seen in exe */ 	
#define UNK_SIGNATURE3	0xF13BD2A9 /* For some dummy files (not all, some dummy file use AUDIO_SIGNATURE + AUDIO_FMT_OGGX */

// Audio fmt
#define AUDIO_FMT_ADPCM1	0xE96FD86A
#define AUDIO_FMT_ADPCM2    0x27052510
#define AUDIO_FMT_OGG1      0x7D43D038
#define AUDIO_FMT_OGG2      0x7C002264

#define KWM_SIGNATURE   0x004D574B

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED SRSAHeader
{
    uint32_t signature;
    uint32_t unk_04; // 0? unused?
	uint32_t file_size; // 8
	uint32_t unk_0C; // 0? unused? (not part of file size, game only reads 32 bits, srsa files are "small" and meant to be fully loaded in RAM)
	
	KTSRHeader ktsr; // 0x10
};
CHECK_STRUCT_SIZE(SRSAHeader, 0x50);

struct PACKED SRSAEntry
{
	uint32_t type_signature; 
	uint32_t entry_size;
	uint32_t id; // (Hash of filename -no extension-, following same hash function used everywhere else). The SUBP and its audio counterpart have same hash.
	uint32_t type; // 0x0C	
};	
CHECK_STRUCT_SIZE(SRSAEntry, 0x10);

struct PACKED SUBPEntry : SRSAEntry
{
	uint8_t unk_10[0x18];
	uint32_t ptr_name; // 0x28
	// Other data here, don't care about them yet
	
	std::string GetName() const
	{
        return (const char *)(((const uint8_t *)this) + ptr_name);
	}
};
CHECK_STRUCT_SIZE(SUBPEntry, 0x2C);

struct PACKED AUDIOEntry : SRSAEntry
{
	uint32_t unk_10;
	uint32_t ptrptr_data_header; // 0x14
	uint32_t ptr_name; // 0x18
	
	std::string GetName() const
	{
        return (const char *)(((const uint8_t *)this) + ptr_name);
	}

    const uint8_t *GetDataPtr() const
    {
        uint32_t ofs = *(const uint32_t *)(((const uint8_t *)this) + ptrptr_data_header);
        return ((const uint8_t *)this) + ofs;
    }

    uint8_t *GetDataPtr()
    {
        const uint8_t *ptr  = const_cast<const AUDIOEntry *>(this)->GetDataPtr();
        return const_cast<uint8_t *>(ptr);
    }
};
CHECK_STRUCT_SIZE(AUDIOEntry, 0x1C);

struct PACKED UNK3Entry : SRSAEntry
{
    uint8_t unk_10[0x24];
    uint32_t ptr_name; // 0x34
    uint32_t unk_ptr;
    uint32_t unk_ptr2;

    std::string GetName() const
    {
        return (const char *)(((const uint8_t *)this) + ptr_name);
    }
};
CHECK_STRUCT_SIZE(UNK3Entry, 0x40);

struct PACKED AUDIOFmtAdpcm1
{
	uint32_t fmt;
    uint32_t file_size; // 4 -  File size from start of this header
    uint32_t unk_08; // Some hash
    uint32_t num_channels; // 0xC 
    uint32_t unk_10;
    uint32_t sample_rate; // 0x14 -
    uint32_t num_blocks; // 0x18
    uint32_t unk_1C; // 0x1C - Maybe plattform
    uint32_t unk_20; // 0xFFFFFFFF
    uint32_t unk_24;
    uint32_t ptr_format; // 0x28
    uint32_t unk_2C;
    uint32_t ptrptr_data; // 0x30
    uint32_t ptr_datasize; // 0x34

    const uint8_t *GetFormatPtr() const
    {
        return (((const uint8_t *)this) + ptr_format);
    }

    uint32_t GetDataSize() const
    {
        return *(const uint32_t *)(((const uint8_t *)this) + ptr_datasize);
    }

    void SetDataSize(uint32_t size)
    {
        *(uint32_t *)(((uint8_t *)this) + ptr_datasize) = size;
    }

    const uint8_t *GetDataPtr() const
    {
        uint32_t ofs = *(const uint32_t *)(((const uint8_t *)this) + ptrptr_data);
        return ((const uint8_t *)this) + ofs;
    }

    uint8_t *GetDataPtr()
    {
        const uint8_t *ptr  = const_cast<const AUDIOFmtAdpcm1 *>(this)->GetDataPtr();
        return const_cast<uint8_t *>(ptr);
    }
};
CHECK_STRUCT_SIZE(AUDIOFmtAdpcm1, 0x38);

struct PACKED AUDIOFmtAdpcm2 : AUDIOFmtAdpcm1
{
    // Some other shit here, but we don't care atm
};

struct PACKED AUDIOFmtOgg1
{
	uint32_t fmt;
	uint32_t file_size; // File size from start of this header
	uint32_t unk_08; // Some hash
	uint32_t num_channels; // 0xC
	uint32_t unk_10; // seen value 0x2706B8 in several files
	uint32_t unk_14; // Seen value 5 in several files
	uint32_t sample_rate; // 0x18
    uint32_t num_samples; // 0x1C - rounded to 16?
	uint32_t unk_20; // Seen 0 in several files. Maybe plattform.
    uint32_t external_kovs_unk08; // If external ogg, it matches the srst kovs.unk_08. Otherwise, seen 0 and -1
	uint32_t unk_28; // ?
	uint32_t unk_2C; // Seen 0 in several files
	uint32_t unk_30; // Seen 0 in several files
    uint32_t kovs_offset; // Offset since KTSR (= file offset - 0x10)
	uint32_t kovs_size; // 0x38
	uint32_t unk_3C; // Seen value 0x200 in several files
};
CHECK_STRUCT_SIZE(AUDIOFmtOgg1, 0x40);

struct PACKED AUDIOFmtOgg2 : AUDIOFmtOgg1
{
    uint32_t unk_40;
    uint32_t unk_44;
    uint32_t ptr_num; //0x48
    uint32_t unk_4C;

    size_t GetFullSize() const
    {
        uint32_t num = *(const uint32_t *)(((const uint8_t *)this) + ptr_num);
        return sizeof(AUDIOFmtOgg2) + sizeof(uint32_t) + num*8;
    }
};
CHECK_STRUCT_SIZE(AUDIOFmtOgg2, 0x50);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct SrsaEntry
{
    enum class Type
    {
        SUBP,
        ADPCM,
        OGG,
        EXT_AUDIO,
        UNK3
    };

    std::vector<uint8_t> buf;
    uint32_t id;
    std::string name;
    bool decoded;
    Type type;

    // Only available after decoding
    uint8_t *raw_data; // For ogg, points at full ogg; for ADPCM, it points at start of sample data. For subp, it just point at start of entry.
    size_t raw_data_size;

    // Only for ADPCM
    uint32_t sample_rate;
    uint32_t num_samples;
    uint16_t block_align;
    AUDIOFmtAdpcm1 *fmt_adpcm1;
    AUDIOFmtAdpcm2 *fmt_adpcm2;

    // Only for OGG
    KOVSEntry *kovs;
    AUDIOFmtOgg1 *fmt_ogg1;
    AUDIOFmtOgg2 *fmt_ogg2;

    SrsaEntry()
    {
        id = 0;
        decoded = false;
        type = Type::SUBP;

        raw_data = nullptr;
        raw_data_size = 0;

        sample_rate = num_samples = 0;
        block_align = 0;
        fmt_adpcm1 = nullptr;
        fmt_adpcm2 = nullptr;

        kovs = nullptr;
        fmt_ogg1 = nullptr;
        fmt_ogg2 = nullptr;
    }

    SrsaEntry(const SrsaEntry &other)
    {
        Copy(other);
    }

    SrsaEntry &operator=(const SrsaEntry &other)
    {
        if (this == &other)
            return *this;

        Copy(other);
        return *this;
    }

    void Copy(const SrsaEntry &other);
    bool Decode();

    bool IsSameAdpcm(Stream &stream) const;
    bool IsSameOgg(Stream &stream) const;

    bool Extract(Stream &stream) const;
    bool Extract(const std::string &dir) const;

    bool Replace(const std::string &file);
    bool SetExternalOgg(const std::string &file);
    bool UpdateExternalOgg(uint32_t offset);
};

class SrsaFile : public BaseFile
{
private:
    std::vector<SrsaEntry> entries;
    uint32_t unk_hash1, unk_hash2;

    size_t CalculateFileSize() const;

protected:
    void Reset();

public:
    SrsaFile();
    virtual ~SrsaFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    inline size_t GetNumEntries() const { return entries.size(); }

    size_t FindFile(const std::string &name) const;
    size_t FindFileById(uint32_t id) const;
    size_t FindFileByTypeAndId(SrsaEntry::Type type, uint32_t id) const;

    inline const std::vector<SrsaEntry> &GetEntries() const { return entries; }
    inline std::vector<SrsaEntry> &GetEntries() { return entries; }

    inline const SrsaEntry &operator[](size_t n) const { return entries[n]; }
    inline SrsaEntry &operator[](size_t n) { return entries[n]; }

    inline std::vector<SrsaEntry>::const_iterator begin() const { return entries.begin(); }
    inline std::vector<SrsaEntry>::const_iterator end() const { return entries.end(); }

    inline std::vector<SrsaEntry>::iterator begin() { return entries.begin(); }
    inline std::vector<SrsaEntry>::iterator end() { return entries.end(); }
};

#endif // SRSAFILE_H
