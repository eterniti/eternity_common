#ifndef __SWFFILE_H__
#define __SWFFILE_H__

#include "BaseFile.h"
#include "FixedMemoryStream.h"

#define SWF_SIGNATURE       0x535746
#define SWF_SIGNATURE_ZLIB  0x535743
#define SWF_SIGNATURE_LZMA  0x53575A

#define GFX_SIGNATURE       0x584647
#define GFX_SIGNATURE_ZLIB  0x584643
// Does gfx lzma exist?

enum
{
    SWF_MIN_VERSION_ZLIB = 6,
    SWF_MIN_VERSION_FILEATTRIBUTES = 8,
    SWF_MIN_VERSION_AS3 = 9,
};

enum
{
    SWF_TAGCODE_END = 0,
    SWF_TAGCODE_SHOWFRAME = 1,
    SWF_TAGCODE_DEFINESHAPE = 2,
    SWF_TAGCODE_PLACEOBJECT = 4,
    SWF_TAGCODE_REMOVEOBJECT = 5,
    SWF_TAGCODE_DEFINEBITS = 6,
    SWF_TAGCODE_DEFINEBUTTON = 7,
    SWF_TAGCODE_JPEGTABLES = 8,
    SWF_TAGCODE_SETBACKGROUNDCOLOR = 9,
    SWF_TAGCODE_DEFINEFONT = 10,
    SWF_TAGCODE_DEFINETEXT = 11,
    SWF_TAGCODE_DOACTION = 12,
    SWF_TAGCODE_DEFINEFONTINFO = 13,
    SWF_TAGCODE_DEFINESOUND = 14,
    SWF_TAGCODE_STARTSOUND = 15,
    SWF_TAGCODE_DEFINEBUTTONSOUND = 17,
    SWF_TAGCODE_SOUNDSTREAMHEAD = 18,
    SWF_TAGCODE_STREAMBLOCK = 19,
    SWF_TAGCODE_DEFINEBITSLOSSLESS = 20,
    SWF_TAGCODE_DEFINEBITSJPEG2 = 21,
    SWF_TAGCODE_DEFINESHAPE2 = 22,
    SWF_TAGCODE_DEFINEBUTTONCXFORM = 23,
    SWF_TAGCODE_PROTECT = 24,
    SWF_TAGCODE_PLACEOBJECT2 = 26,
    SWF_TAGCODE_REMOVEOBJECT2 = 28,
    SWF_TAGCODE_DEFINESHAPE3 = 32,
    SWF_TAGCODE_DEFINETEXT2 = 33,
    SWF_TAGCODE_DEFINEBUTTON2 = 34,
    SWF_TAGCODE_DEFINEBITSJPEG3 = 35,
    SWF_TAGCODE_DEFINEBITSLOSSLESS2 = 36,
    SWF_TAGCODE_DEFINEEDITTEXT = 37,
    SWF_TAGCODE_SPRITE = 39,
    SWF_TAGCODE_PRODUCTID = 41,
    SWF_TAGCODE_FRAMELABEL = 43,
    SWF_TAGCODE_SOUNDSTREAMHEAD2 = 45,
    SWF_TAGCODE_DEFINEMORPHSHAPE2 = 46,
    SWF_TAGCODE_DEFINEFONT2 = 48,
    SWF_TAGCODE_EXPORTASSETS = 56,
    SWF_TAGCODE_IMPORTASSETS = 57,
    SWF_TAGCODE_ENABLEDEBUGGER = 58,
    SWF_TAGCODE_DOINITACTION = 59,
    SWF_TAGCODE_DEFINEVIDEOSTREAM = 60,
    SWF_TAGCODE_VIDEOFRAME = 61,
    SWF_TAGCODE_DEFINEFONTINFO2 = 62,
    SWF_TAGCODE_DEBUGID = 63,
    SWF_TAGCODE_ENABLEDEBUGGER2 = 64,
    SWF_TAGCODE_SCRIPTLIMITS = 65,
    SWF_TAGCODE_SETTABINDEX = 66,
    SWF_TAGCODE_FILEATTRIBUTES = 69,
    SWF_TAGCODE_PLACEOBJECT3 = 70,
    SWF_TAGCODE_IMPORTASSETS2 = 71,
    SWF_TAGCODE_DEFINEFONTALIGNZONES = 73,
    SWF_TAGCODE_CSMTEXTSETTINGS = 74,
    SWF_TAGCODE_DEFINEFONT3 = 75,
    SWF_TAGCODE_SYMBOLCLASS = 76,
    SWF_TAGCODE_METADATA = 77,
    SWF_TAGCODE_DEFINESCALINGGRID = 78,
    SWF_TAGCODE_DOABC = 82,
    SWF_TAGCODE_DEFINESHAPE4 = 84,
    SWF_TAGCODE_DEFINESCENEANDFRAMELABELDATA = 86,
    SWF_TAGCODE_DEFINEBINARY_DATA = 87,
    SWF_TAGCODE_DEFINEFONTNAME = 88,
    SWF_TAGCODE_STARTSOUND2 = 89,
    SWF_TAGCODE_DEFINEBITSJPEG4 = 90,
    SWF_TAGCODE_DEFINEFONT4 = 91,
};

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    uint32_t signature : 24; // 0
    uint32_t version : 8; // 3
    uint32_t file_size; // 4
} PACKED SWFHeader;

STATIC_ASSERT_STRUCT(SWFHeader, 8);

typedef struct
{
    uint16_t length : 6;
    uint16_t code : 10;
} PACKED SWFRecordHeader;

STATIC_ASSERT_STRUCT(SWFRecordHeader, 2);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct SwfRect
{
    uint8_t bits;
    int32_t xmin, xmax;
    int32_t ymin, ymax;

    SwfRect() : bits(1), xmin(0), xmax(0), ymin(0), ymax(0) { }

    bool Read(MemoryStream *stream);
    bool Write(MemoryStream *stream);
};

struct SwfBlock
{
    virtual ~SwfBlock() {}

    virtual uint16_t GetTagCode() const = 0;
    virtual uint8_t GetMinVersion() const = 0;
    virtual uint32_t GetSize() const = 0;

    virtual bool Read(FixedMemoryStream *stream) = 0;
    virtual bool Write(MemoryStream *stream) = 0;
};

struct SwfGenericBlock : SwfBlock
{
    uint16_t code;
    std::vector<uint8_t> data;

    SwfGenericBlock(uint16_t code) : code(code) {}
    virtual ~SwfGenericBlock() { }

    virtual uint16_t GetTagCode() const override { return code; }
    virtual uint8_t GetMinVersion() const override { return 1; }
    virtual uint32_t GetSize() const override { return (uint32_t)data.size(); }

    virtual bool Read(FixedMemoryStream *stream) override
    {
        data.resize(stream->GetSize());
        return stream->Read(data.data(), data.size());
    }

    virtual bool Write(MemoryStream *stream) override
    {
        return stream->Write(data.data(), data.size());
    }
};

struct SwfEnd : SwfBlock
{
    virtual ~SwfEnd() {}

    virtual uint16_t GetTagCode() const override { return SWF_TAGCODE_END; }
    virtual uint8_t GetMinVersion() const override { return 1; }
    virtual uint32_t GetSize() const override { return 0; }

    virtual bool Read(FixedMemoryStream *) override { return true; }

    virtual bool Write(MemoryStream *) override { return true; }
};

struct SwfFileAttributes : SwfBlock
{
    uint8_t use_direct_blit;
    uint8_t use_gpu;
    uint8_t has_metadata;
    uint8_t as3;
    uint8_t use_network;

    SwfFileAttributes() : use_direct_blit(0), use_gpu(0), has_metadata(0), as3(0), use_network(0) { }
    virtual ~SwfFileAttributes() { }

    virtual uint16_t GetTagCode() const override { return SWF_TAGCODE_FILEATTRIBUTES; }
    virtual uint8_t GetMinVersion() const override { return SWF_MIN_VERSION_FILEATTRIBUTES; }
    virtual uint32_t GetSize() const override { return 4; }

    virtual bool Read(FixedMemoryStream *stream) override;
    virtual bool Write(MemoryStream *stream) override;
};

struct SwfDoABC : SwfBlock
{
    uint32_t flags;
    std::string name;
    std::vector<uint8_t> abc;

    SwfDoABC() : flags(0) { }
    SwfDoABC(const uint8_t *abc_buf, uint32_t size, uint32_t flags, const std::string &name="") : flags(flags), name(name)
    {
        abc.resize(size);
        memcpy(abc.data(), abc_buf, size);
    }
    virtual ~SwfDoABC() { }

    virtual uint16_t GetTagCode() const override { return SWF_TAGCODE_DOABC; }
    virtual uint8_t GetMinVersion() const override { return SWF_MIN_VERSION_AS3; }
    virtual uint32_t GetSize() const override { return sizeof(flags) + (uint32_t)name.length() + 1 + (uint32_t)abc.size(); }

    virtual bool Read(FixedMemoryStream *stream) override;
    virtual bool Write(MemoryStream *stream) override;
};

class SwfFile : public BaseFile
{
private:

    uint8_t version;
    bool is_compressed;
    bool is_gfx;

    SwfRect frame_size;
    uint16_t frame_rate;
    uint16_t frame_count;

    std::vector<SwfBlock *> blocks;

protected:

    void Reset();

public:

    SwfFile();
    virtual ~SwfFile() override;

    inline uint8_t GetVersion() const { return version; }
    inline bool IsCompressed() const { return is_compressed; }
    inline bool IsGfx() const { return is_gfx; }
    inline const SwfRect &GetFrameSize() const { return frame_size; }
    inline uint16_t GetFrameRate() const { return frame_rate; }
    inline uint16_t GetFrameCount() const { return frame_count; }

    inline uint32_t GetNumBlocks() { return (uint32_t)blocks.size(); }
    inline SwfBlock *GetBlock(uint32_t idx)
    {
        if (idx >= blocks.size())
            return nullptr;

        return blocks[idx];
    }

    inline void SetGfx(bool gfx) { is_gfx = gfx; }

    // The object gets the ownership of block. Don't delete from outside!
    inline void AddBlock(SwfBlock *block) { blocks.push_back(block); }

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;
};

#endif // __SWFFILE_H__
