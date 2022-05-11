#include "SwfFile.h"
#include "FixedBitStream.h"
#include "debug.h"

bool SwfRect::Read(MemoryStream *stream)
{
    uint8_t *buf;
    uint64_t pos = stream->Tell();
    size_t capacity = (size_t) stream->GetSize()-pos;

    if (!stream->FastRead(&buf, capacity))
        return false;

    FixedBitStream bs(buf, capacity*8);

    if (!bs.ReadU8(&bits, 5))
        return false;

    if (!bs.ReadS32(&xmin, bits))
        return false;

    if (!bs.ReadS32(&xmax, bits))
        return false;

    if (!bs.ReadS32(&ymin, bits))
        return false;

    if (!bs.ReadS32(&ymax, bits))
        return false;

    return stream->Seek(pos+bs.TellByteCeiling(), SEEK_SET);
}

bool SwfRect::Write(MemoryStream *stream)
{
    BitStream bs;
    uint8_t n;

    n = BitStream::MinBitsS32(xmin);
    if (n > bits)
        bits = n;

    n = BitStream::MinBitsS32(xmax);
    if (n > bits)
        bits = n;

    n = BitStream::MinBitsS32(ymin);
    if (n > bits)
        bits = n;

    n = BitStream::MinBitsS32(ymax);
    if (n > bits)
        bits = n;

    if (!bs.WriteU8(bits, 5))
        return false;

    if (!bs.WriteS32(xmin, bits))
        return false;

    if (!bs.WriteS32(xmax, bits))
        return false;

    if (!bs.WriteS32(ymin, bits))
        return false;

    if (!bs.WriteS32(ymax, bits))
        return false;

    return stream->Write(bs.GetMemory(false), bs.GetSizeBytes());
}

bool SwfFileAttributes::Read(FixedMemoryStream *stream)
{
    uint32_t fa;

    if (!stream->Read32(&fa))
        return false;

    FixedBitStream bs((uint8_t *)&fa, 32);

    if (!bs.Seek(1, SEEK_CUR))
        return false;

    if (!bs.ReadU8(&use_direct_blit, 1))
        return false;

    if (!bs.ReadU8(&use_gpu, 1))
        return false;

    if (!bs.ReadU8(&has_metadata, 1))
        return false;

    if (!bs.ReadU8(&as3, 1))
        return false;

    if (!bs.Seek(2, SEEK_CUR))
        return false;

    if (!bs.ReadU8(&use_network, 1))
        return false;

    return true;
}

bool SwfFileAttributes::Write(MemoryStream *stream)
{
    uint32_t fa = 0;
    FixedBitStream bs((uint8_t *)&fa, 32);

    if (!bs.Seek(1, SEEK_CUR))
        return false;

    if (!bs.WriteU8(use_direct_blit, 1))
        return false;

    if (!bs.WriteU8(use_gpu, 1))
        return false;

    if (!bs.WriteU8(has_metadata, 1))
        return false;

    if (!bs.WriteU8(as3, 1))
        return false;

    if (!bs.Seek(2, SEEK_CUR))
        return false;

    if (!bs.WriteU8(use_network, 1))
        return false;

    return stream->Write32(fa);
}

bool SwfDoABC::Read(FixedMemoryStream *stream)
{
    if (!stream->Read32(&flags))
        return false;

    name.clear();

    while (1)
    {
        int8_t ch;

        if (!stream->Read8(&ch))
            return false;

        if (ch == 0)
            break;

        name.push_back(ch);
    }

    abc.clear();
    abc.resize(stream->GetSize() - stream->Tell());

    if (!stream->Read(abc.data(), abc.size()))
        return false;

    return true;
}

bool SwfDoABC::Write(MemoryStream *stream)
{
    if (!stream->Write32(flags))
        return false;

    if (!stream->Write(name.c_str(), name.length()+1))
        return false;

    return stream->Write(abc.data(), abc.size());
}

SwfFile::SwfFile()
{
    big_endian = false;
    Reset();
}

SwfFile::~SwfFile()
{
    Reset();
}

void SwfFile::Reset()
{
    is_compressed = false;
    is_gfx = false;
    version = SWF_MIN_VERSION_AS3;

    frame_size.bits = 16;
    frame_size.xmin = 0;
    frame_size.xmax = 1280*20;
    frame_size.ymin = 0;
    frame_size.ymax = 720*20;
    frame_rate = 30<<8;
    frame_count = 0;

    for (SwfBlock *block : blocks)
    {
        if (block)
        {
            delete block;
            block = nullptr;
        }
    }

    blocks.clear();
}

bool SwfFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    FixedMemoryStream *stream = nullptr;
    const SWFHeader *hdr = (const SWFHeader *)buf;
    uint8_t *uncomp_buf = nullptr;

    if (size < sizeof(SWFHeader))
        return false;

    if (hdr->signature == SWF_SIGNATURE || hdr->signature == GFX_SIGNATURE)
    {
        if (hdr->file_size > size)
            return false;

        stream = new FixedMemoryStream(const_cast<uint8_t *>(buf+sizeof(SWFHeader)), hdr->file_size-sizeof(SWFHeader));
    }
    else if (hdr->signature == SWF_SIGNATURE_ZLIB || hdr->signature == GFX_SIGNATURE_ZLIB)
    {
        uint32_t uncomp_size = hdr->file_size - sizeof(SWFHeader);
        uncomp_buf = new uint8_t[uncomp_size];

        if (!Utils::UncompressZlib(uncomp_buf, &uncomp_size, buf+sizeof(SWFHeader), (uint32_t)size-sizeof(SWFHeader)))
        {
            delete[] uncomp_buf;
            return false;
        }

        stream = new FixedMemoryStream(uncomp_buf, uncomp_size);
        is_compressed = true;
    }
    else if (hdr->signature == SWF_SIGNATURE_LZMA)
    {
        DPRINTF("%s: LZMA not implemented.\n", FUNCNAME);
        return false;
    }
    else
    {
        DPRINTF("%s: Unrecognized signature.\n", FUNCNAME);
        return false;
    }

    is_gfx = (hdr->signature == GFX_SIGNATURE || hdr->signature == GFX_SIGNATURE_ZLIB);

    bool ret = frame_size.Read(stream) && stream->Read16(&frame_rate) && stream->Read16(&frame_count);
    if (!ret)
    {
        delete stream;

        if (uncomp_buf)
            delete[] uncomp_buf;

        return ret;
    }

    SWFRecordHeader record;

    while (stream->Read(&record, sizeof(SWFRecordHeader)))
    {
        bool end = false;
        uint32_t length = record.length;

        if (length == 0x3F)
        {
            if (!stream->Read32(&length))
            {
                ret = false;
                break;
            }
        }

        uint8_t *block_buf;
        if (!stream->FastRead(&block_buf, length))
        {
            ret = false;
            break;
        }

        FixedMemoryStream block_stream(block_buf, length);
        SwfBlock *block;

        switch (record.code)
        {
            case SWF_TAGCODE_END:
                block = new SwfEnd();
                end = true;
            break;

            case SWF_TAGCODE_FILEATTRIBUTES:
                block = new SwfFileAttributes();
            break;

            case SWF_TAGCODE_DOABC:
                block = new SwfDoABC();
            break;

            default:
                block = new SwfGenericBlock(record.code);
        }

        if (block->GetMinVersion() > version)
        {
            DPRINTF("%s: Warning: this swf uses feature from a future version. Version=%d, tagcode=%d, min_version=%d\n",
                    FUNCNAME, version, record.code, block->GetMinVersion());
        }

        if (!block->Read(&block_stream))
        {
            ret = false;
            break;
        }        

        blocks.push_back(block);

        if (end)
            break;
    }

    delete stream;

    if (uncomp_buf)
        delete[] uncomp_buf;

    if (ret)
    {
        if (blocks.size() > 0)
        {
            if (version >= SWF_MIN_VERSION_FILEATTRIBUTES)
            {
                SwfFileAttributes *attr = dynamic_cast<SwfFileAttributes *>(blocks[0]);

                if (!attr)
                    DPRINTF("%s: Warning: first block is not file attributes.\n", FUNCNAME);
            }
        }
        else
        {
            DPRINTF("%s: Warning, this swf is empty.\n", FUNCNAME);
        }
    }

    return ret;
}

uint8_t *SwfFile::Save(size_t *psize)
{
    // TODO: allow compression. Currently, writing always decompressed

    MemoryStream stream;
    SWFHeader header;

    header.signature = (is_gfx) ? GFX_SIGNATURE : SWF_SIGNATURE;
    header.version = version;

    if (!stream.Write(&header, sizeof(SWFHeader)))
        return nullptr;

    if (!frame_size.Write(&stream))
        return nullptr;

    if (!stream.Write16(frame_rate))
        return nullptr;

    if (!stream.Write16(frame_count))
        return nullptr;

    for (SwfBlock *block : blocks)
    {
        SWFRecordHeader record;
        uint32_t length = block->GetSize();

        record.code = block->GetTagCode();
        record.length = (length >= 0x3F) ? 0x3F : length;

        if (!stream.Write(&record, sizeof(SWFRecordHeader)))
            return nullptr;

        if (length >= 0x3F)
        {
            if (!stream.Write32(length))
                return nullptr;
        }

        if (!block->Write(&stream))
            return nullptr;
    }

    stream.Seek(0, SEEK_SET);
    header.file_size = (uint32_t)stream.GetSize();

    if (!stream.Write(&header, sizeof(SWFHeader)))
        return nullptr;

    *psize = header.file_size;
    return stream.GetMemory(true);
}
