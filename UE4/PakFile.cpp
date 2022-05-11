#include "PakFile.h"
#include "FixedMemoryStream.h"
#include "debug.h"

#ifndef NO_CRYPTO
#include "crypto/sha1.h"
#else
#define SHA1_CTX int
#define SHA1_Init(c)
#define SHA1_Update(c, d, l)
#define SHA1_Final(c, d)
#endif

#define STRING_LENGTH_LIMIT		1048576
#define COPY_BUF_SIZE	(128*1024*1024)

#define INVALID_OFFSET	((uint64_t)-1)

#define ENCRYPTION_KEY_SIZE     256
#define ENCRYPTION_BLOCK_SIZE   16

using namespace UE4Common;

bool PakEntry::Read(Stream *stream, int version)
{
    if (!stream->Read64(&offset))
        return false;

    if (!stream->Read64(&size))
        return false;

    if (!stream->Read64(&uncompressed_size))
        return false;

    if (!stream->Read32(&compression_method))
        return false;

    if (version <= 1)
    {
        // Skip time stamp
        stream->Seek(sizeof(uint64_t), SEEK_CUR);
    }

    if (!stream->Read(sha1, sizeof(sha1)))
        return false;

    if (version >= 3)
    {
        if (compression_method != 0)
        {
            uint32_t array_size;

            if (!stream->Read32(&array_size))
                return false;

            comp_blocks.resize(array_size);

            for (PakCompressedBlock &block : comp_blocks)
            {
                if (!stream->Read64(&block.comp_start))
                    return false;

                if (!stream->Read64(&block.comp_end))
                    return false;
            }
        }
        else
        {
            comp_blocks.clear();
        }

        if (!stream->Read8(&encrypted))
            return false;

        if (!stream->Read32(&compression_block_size))
            return false;
    }

    return true;
}

bool PakEntry::Write(Stream *stream, int version, bool set_offset_to_zero) const
{
    uint64_t offset = (set_offset_to_zero) ? 0 : this->offset;

    if (!stream->Write64(offset))
        return false;

    if (!stream->Write64(size))
        return false;

    if (!stream->Write64(uncompressed_size))
        return false;

    if (!stream->Write32(compression_method))
        return false;

    if (version <= 1)
    {
        // Dummy time stamp
        uint64_t timestamp = 0;

        if (!stream->Write64(timestamp))
            return false;
    }

    if (!stream->Write(sha1, sizeof(sha1)))
        return false;

    if (version >= 3)
    {
        if (compression_method != 0)
        {
            uint32_t array_size = (uint32_t)comp_blocks.size();

            if (!stream->Write32(array_size))
                return false;

            for (const PakCompressedBlock &block : comp_blocks)
            {
                if (!stream->Write64(block.comp_start))
                    return false;

                if (!stream->Write64(block.comp_end))
                    return false;
            }
        }

        if (!stream->Write8(encrypted))
            return false;

        if (!stream->Write32(compression_block_size))
            return false;
    }

    return true;
}

bool PakFileEntry::GetSize(uint64_t *psize) const
{
    if (offset != INVALID_OFFSET || buf)
    {
        *psize = uncompressed_size;
        return true;
    }

    size_t size = Utils::GetFileSize(external_path);
    if (size == (size_t)-1)
    {
        DPRINTF("%s: Cannot stat file \"%s\"\n", FUNCNAME, external_path.c_str());
        return false;
    }

    *psize = size;
    return true;
}

bool PakFileEntry::GetRealSize(uint64_t *psize) const
{
    size_t ret;

    if (offset != INVALID_OFFSET || buf)
    {
        ret = size;
    }
    else
    {
        size_t size = Utils::GetFileSize(external_path);
        if (size == (size_t)-1)
        {
            DPRINTF("%s: Cannot stat file \"%s\"\n", FUNCNAME, external_path.c_str());
            return false;
        }

        ret = size;
    }

    if (encrypted)
    {
        if ((ret % ENCRYPTION_BLOCK_SIZE) != 0)
        {
            ret += ENCRYPTION_BLOCK_SIZE - (ret % ENCRYPTION_BLOCK_SIZE);
        }
    }

    *psize = ret;
    return true;
}

bool PakFileEntry::Read(Stream *stream, int version)
{
    if (!path.Read(stream))
        return false;

    return PakEntry::Read(stream, version);
}

bool PakFileEntry::Write(Stream *stream, int version, bool set_offset_to_zero) const
{
    if (!path.Write(stream))
        return false;

    return PakEntry::Write(stream, version, set_offset_to_zero);
}

PakFile::PakFile()
{
	big_endian = false;
    fstream = nullptr;
	version = 3;	
    save_callback = nullptr;
    pc_param = nullptr;
}

PakFile::~PakFile()
{
	Reset();
}

void PakFile::Reset()
{
    if (fstream)
    {
        delete fstream;
        fstream = nullptr;
    }
	
    mount_point.Reset();
	files.clear();
		
    version = 3;
    save_callback = nullptr;
    pc_param = nullptr;
}

bool PakFile::LoadCommon(Stream *stream)
{
    PAKFooter footer;
    uint32_t num_files;

    stream->Seek(-(int64_t)sizeof(PAKFooter), SEEK_END);

    if (!stream->Read(&footer, sizeof(PAKFooter)))
        return false;

    if (val32(footer.signature) != PAK_SIGNATURE)
    {
        DPRINTF("%s: Invalid signature.\n", FUNCNAME);
        return false;
    }

    version = val32(footer.version);

    if (val64(footer.index_size) >= 0xFFFFFFFF)
    {
        DPRINTF("%s: Index too big.\n", FUNCNAME);
        return false;
    }

    uint8_t *index_data = new uint8_t[val64(footer.index_size)];
    stream->Seek(val64(footer.index_offset), SEEK_SET);

    if (!stream->Read(index_data, val64(footer.index_size)))
        return false;

    uint8_t sha1[20];

    Utils::Sha1(index_data, (uint32_t)val64(footer.index_size), sha1);
    delete index_data;

    if (memcmp(sha1, footer.index_sha1, sizeof(sha1)) != 0)
    {
        DPRINTF("%s: warning, sha1 of index data doesn't match.\n", FUNCNAME);
    }

    stream->Seek(val64(footer.index_offset), SEEK_SET);

    if (!mount_point.Read(stream))
        return false;

    if (!stream->Read32(&num_files))
        return false;

    files.resize(num_files);

    for (PakFileEntry &entry : files)
    {
        if (!entry.Read(stream, version))
            return false;       
    }

    MemoryStream *memory = dynamic_cast<MemoryStream *>(stream);
    if (memory)
    {
        for (PakFileEntry &entry : files)
        {
            PakEntry pak_entry;

            stream->Seek(entry.offset, SEEK_SET);

            if (!pak_entry.Read(stream, version))
                return false;

            if (entry.compression_method != 0)
            {
                entry.buf = new uint8_t[entry.uncompressed_size];
                FixedMemoryStream mem(entry.buf, entry.uncompressed_size);

                if (!Uncompress(entry, stream, &mem))
                {
                    delete[] entry.buf;
                    entry.buf = nullptr;
                    return false;
                }

                entry.comp_blocks.clear();
                entry.compression_block_size = 0;
                entry.compression_method = 0;
                entry.encrypted = false;
            }
            else if (!entry.encrypted )
            {
                entry.buf = new uint8_t[entry.size];
                if (!stream->Read(entry.buf, entry.size))
                {
                    delete[] entry.buf;
                    entry.buf = nullptr;
                    return false;
                }
            }
            else
            {
                uint64_t read_size;
                entry.GetRealSize(&read_size);

                entry.buf = new uint8_t[read_size];
                FixedMemoryStream mem(entry.buf, read_size);

                if (!mem.CopyEx(stream, read_size, Stream::Hash::NONE, nullptr, Stream::Cipher::AES_PLAIN, encryption_key, ENCRYPTION_KEY_SIZE))
                {
                    delete[] entry.buf;
                    entry.buf = nullptr;
                    return false;
                }

                entry.encrypted = false;
            }

            entry.offset = INVALID_OFFSET;
        }
    }

    return true;
}

bool PakFile::Load(const uint8_t *buf, size_t size)
{
    FixedMemoryStream stream(const_cast<uint8_t *>(buf), size);

    Reset();
    return LoadCommon(&stream);
}

bool PakFile::LoadFromFile(const std::string &path, bool show_error)
{
    Reset();

    fstream = new FileStream("rb");
    if (!fstream->LoadFromFile(path, show_error))
        return false;

    return LoadCommon(fstream);
}

bool PakFile::SaveCommon(Stream *stream)
{
    PAKFooter footer;
    uint32_t num_files;
    uint64_t written_size = 0;

    // Write files
    for (size_t i = 0; i < files.size(); i++)
    {
        PakFileEntry &entry = files[i];
        PakEntry pak_entry;
        uint64_t size;

        if (!entry.GetSize(&size))
            return false;

        entry.size = size;

        if (entry.offset != INVALID_OFFSET) // Internal file
        {
            uint8_t sha1[20];

            if (!fstream->Seek(val64(entry.offset), SEEK_SET))
                return false;

            if (!pak_entry.Read(fstream, version))
                return false;

            // Update entry
            entry.offset = val64(stream->Tell());

            if (!entry.PakEntry::Write(stream, version, true))
                return false;

            if (entry.compression_method != 0 || entry.encrypted)
            {
                DPRINTF("%s: TODO, handle compression and encryption here.\n", FUNCNAME);
                return false;
            }

            if (!stream->CopyEx(fstream, entry.size, Stream::Hash::SHA1, sha1))
                return false;

            if (memcmp(sha1, entry.sha1, sizeof(sha1)) != 0)
            {
                // If sha1 doesn't match, copy it and write it again
                if (!stream->Seek(val64(entry.offset), SEEK_SET))
                    return false;

                memcpy(entry.sha1, sha1, sizeof(sha1));
                if (!entry.PakEntry::Write(stream, version, true))
                    return false;

                if (!stream->Seek(0, SEEK_END))
                    return false;
            }
        }
        else if (entry.buf) // Memory file
        {
            entry.offset = stream->Tell();

            // Write dummy entry first. It will be rewritten later.
            if (!entry.PakEntry::Write(stream, version, true))
                return false;

            if (!stream->Write(entry.buf, size))
                return false;

            Utils::Sha1(entry.buf, (uint32_t)size, entry.sha1);

            entry.uncompressed_size = entry.size;
            entry.compression_method = 0;
            entry.encrypted = 0;
            entry.compression_block_size = 0;

            if (!stream->Seek(val64(entry.offset), SEEK_SET))
                return false;

            if (!entry.PakEntry::Write(stream, version, true))
            {
                DPRINTF("%s: write real pak entry (external file) failed.\n", FUNCNAME);
                return false;
            }

            if (!stream->Seek(0, SEEK_END))
                return false;
        }
        else // External file
        {
            entry.offset = stream->Tell();

            // Write dummy entry first. It will be rewritten later.
            if (!entry.PakEntry::Write(stream, version, true))
                return false;

            FileStream external("rb");

            if (!external.LoadFromFile(entry.external_path))
                return false;

            if (!stream->CopyEx(&external, size, Stream::Hash::SHA1, entry.sha1))
                return false;

            entry.uncompressed_size = entry.size;
            entry.compression_method = 0;
            entry.encrypted = 0;
            entry.compression_block_size = 0;

            if (!stream->Seek(val64(entry.offset), SEEK_SET))
                return false;

            if (!entry.PakEntry::Write(stream, version, true))
            {
                DPRINTF("%s: write real pak entry (external file) failed.\n", FUNCNAME);
                return false;
            }

            if (!stream->Seek(0, SEEK_END))
                return false;
        }

        if (save_callback)
        {
            written_size += size;
            if (!save_callback(i+1, files.size(), written_size, pc_param))
                return false;
        }
    }

    // Write index
    if (!stream->Seek(0, SEEK_END))
        return false;

    footer.index_offset = val64(stream->Tell());

    if (!mount_point.Write(stream))
        return false;

    num_files = val32((uint32_t)files.size());

    if (!stream->Write32(num_files))
        return false;

    for (PakFileEntry &entry : files)
    {
        if (!entry.Write(stream, version, false))
            return false;
    }

    // Write footer
    if (!stream->Seek(0, SEEK_END))
        return false;

    footer.index_size = val64(stream->Tell() - val64(footer.index_offset));
    footer.signature = val32(PAK_SIGNATURE);
    footer.version = val32(version);

    if (!stream->Seek(val64(footer.index_offset), SEEK_SET))
        return false;

    if (footer.index_size >= 0xFFFFFFFF)
    {
        DPRINTF("%s: Index too big.\n", FUNCNAME);
        return false;
    }

    uint8_t *buf = new uint8_t[val64(footer.index_size)];

    if (!stream->Read(buf, val64(footer.index_size)))
    {
        DPRINTF("%s: failed on reread index\n", FUNCNAME);
        delete[] buf;
        return false;
    }

    Utils::Sha1(buf, (uint32_t)val64(footer.index_size), footer.index_sha1);
    delete[] buf;

    if (!stream->Seek(0, SEEK_END))
        return false;

    if (!stream->Write(&footer, sizeof(PAKFooter)))
    {
        DPRINTF("%s: writing footer failed.\n", FUNCNAME);
        return false;
    }

    return true;
}

bool PakFile::Uncompress(const PakFileEntry &entry, Stream *in, Stream *out, uint8_t *sha1) const
{
#ifdef NO_CRYPTO
    DPRINTF("%s: Warning: crypto is not enabled, sha1 will not be done.\n", FUNCNAME);
#endif

    assert(entry.compression_method != 0);

    if ((entry.compression_method & 1) == 0)
    {
        DPRINTF("%s: Compression method not implemented: 0x%x\n", FUNCNAME, entry.compression_method);
        return false;
    }

    SHA1_CTX ctx;

#ifdef NO_CRYPTO
    UNUSED(ctx);
#endif

    uint64_t remaining = entry.uncompressed_size;
    uint8_t *out_buf = new uint8_t[entry.compression_block_size];

    if (sha1)
        SHA1_Init(&ctx);

    for (const PakCompressedBlock &block : entry.comp_blocks)
    {
        int64_t comp_size = block.comp_end - block.comp_start;
        uint64_t uncomp_size;
        uint32_t avail_out;
        uint8_t *in_buf;

        if (comp_size <= 0 || comp_size > 0xFFFFFFFF)
        {
            delete[] out_buf;
            return false;
        }

        if (!in->Seek(block.comp_start, SEEK_SET))
        {
            delete[] out_buf;
            return false;
        }

        if (!entry.encrypted)
        {
            in_buf = new uint8_t[comp_size];
            if (!in->Read(in_buf, comp_size))
            {
                delete[] in_buf;
                delete[] out_buf;
                return false;
            }

            if (sha1)
                SHA1_Update(&ctx, in_buf, (uint32_t)comp_size);
        }
        else
        {
            size_t read_size = (size_t)comp_size;

            if ((read_size % ENCRYPTION_BLOCK_SIZE) != 0)
            {
                read_size += ENCRYPTION_BLOCK_SIZE - (read_size % ENCRYPTION_BLOCK_SIZE);
            }

            in_buf = new uint8_t[read_size];
            if (!in->Read(in_buf, read_size))
            {
                delete[] in_buf;
                delete[] out_buf;
                return false;
            }

            if (sha1)
                SHA1_Update(&ctx, in_buf, (uint32_t)read_size);

            Utils::AesEcbDecrypt(in_buf, read_size, encryption_key, ENCRYPTION_KEY_SIZE);
        }

        uncomp_size = (remaining < entry.compression_block_size) ? remaining : entry.compression_block_size;
        if (uncomp_size > 0xFFFFFFFF)
        {
            delete[] in_buf;
            delete[] out_buf;
            return false;
        }

        avail_out = (uint32_t)uncomp_size;
        if (!Utils::UncompressZlib(out_buf, &avail_out, in_buf, (uint32_t)comp_size))
        {
            DPRINTF("%s: UncompressZlib failed.\n", FUNCNAME);
            delete[] in_buf;
            delete[] out_buf;
            return false;
        }

        delete[] in_buf;

        if (avail_out != uncomp_size)
        {
            DPRINTF("%s: returned number of uncompressed bytes is not same as expected (0x%x != 0x%x).\n", FUNCNAME, avail_out, (uint32_t)uncomp_size);
            delete[] out_buf;
            return false;
        }

        if (!out->Write(out_buf, avail_out))
        {
            delete[] out_buf;
            return false;
        }

        remaining -= avail_out;
    }

    delete[] out_buf;

    if (remaining != 0)
    {
        DPRINTF("%s: an error happened somewhere (remaining=0x%I64x).\n", FUNCNAME, remaining);
        return false;
    }

    if (sha1)
        SHA1_Final(&ctx, sha1);

    return true;
}

uint8_t *PakFile::Save(size_t *psize)
{
    MemoryStream stream;

    if (!SaveCommon(&stream))
        return nullptr;

    *psize = stream.GetSize();
    return stream.GetMemory(true);
}

bool PakFile::SaveToFile(const std::string &path, bool show_error, bool build_path)
{
    FILE *f = (build_path) ? Utils::fopen_create_path(path, "wb") : fopen(path.c_str(), "wb");
    if (!f)
    {
        if (show_error)
        {
            DPRINTF("%s: Cannot open/create file \"%s\"\n", FUNCNAME, path.c_str());
        }

        return false;
    }

    fclose(f);
    FileStream stream("w+b");

    if (!stream.LoadFromFile(path, show_error))
        return false;

    return SaveCommon(&stream);
}

bool PakFile::ExtractCommon(const PakFileEntry &entry, Stream *stream, uint64_t size) const
{
    if (entry.offset != INVALID_OFFSET)
    {
        // Internal file
        assert(fstream);

        uint8_t sha1[20];
        PakEntry pak_entry;

        if (!fstream->Seek(entry.offset, SEEK_SET))
            return false;

        if (!pak_entry.Read(fstream, version))
            return false;

        if (pak_entry != entry)
        {
            DPRINTF("%s: WARNING: pak_entry doesn't match the one in the index (in file %s)\n", FUNCNAME, entry.path.str.c_str());
        }

        //DPRINTF("Extract %I64x %I64x %x %x\n", entry.size, entry.uncompressed_size, entry.compression_method, entry.encrypted);

        bool ret;
        if (entry.compression_method != 0)
        {
            ret = Uncompress(entry, fstream, stream, sha1);
            if (!ret)
                return false;
        }
        else
        {
            if (!entry.encrypted)
                ret = stream->CopyEx(fstream, size, Stream::Hash::SHA1, sha1);
            else
                ret = stream->CopyEx(fstream, size, Stream::Hash::SHA1, sha1, Stream::Cipher::AES_PLAIN, encryption_key, ENCRYPTION_KEY_SIZE);
        }

        if (!ret)
        {
            //DPRINTF("Failed here. Offset: %I64x size: %x\n", entry.offset, size);
            return false;
        }

        if (memcmp(sha1, entry.sha1, sizeof(sha1)) != 0)
        {
            DPRINTF("%s: Warning, sha1 of \"%s\" doesn't match.\n", FUNCNAME, entry.path.str.c_str());
        }
    }
    else if (entry.buf)
    {
        if (!stream->Write(entry.buf, size))
            return false;
    }
    else
    {
        FileStream external("rb");

        if (!external.LoadFromFile(entry.external_path))
            return false;

        if (!stream->Copy(&external, size))
            return false;
    }

    return true;
}

bool PakFile::ExtractFile(size_t idx, const std::string &extract_path, bool extract_path_is_base_directory) const
{
	if (idx >= GetNumFiles())
		return false;
	
	const PakFileEntry &entry = files[idx];
    uint64_t size;

    if (!entry.GetSize(&size))
        return false;
	
    std::string out_path;

	if (extract_path_is_base_directory)
	{
		out_path = extract_path;
		
        if (out_path.length() != 0 && !Utils::EndsWith(out_path, "/") && !Utils::EndsWith(out_path, "\\"))
		{
			out_path += "/";
		}
		
        out_path += entry.path.str;
	}
	else
	{
		out_path = extract_path;
	}

    FileStream output("w+b");

    if (!output.LoadFromFile(out_path))
        return false;

    return ExtractCommon(entry, &output, size);
}

bool PakFile::ExtractFile(const std::string &internal_path, const std::string &extract_path, bool extract_path_is_base_directory) const
{
	for (size_t i = 0; i < files.size(); i++)
	{
        if (files[i].path == internal_path)
			return ExtractFile(i, extract_path, extract_path_is_base_directory);
	}
	
    return false;
}

uint8_t *PakFile::ExtractFile(uint32_t idx, uint64_t *psize) const
{
    if (idx >= files.size())
        return nullptr;

    const PakFileEntry &entry = files[idx];
    uint64_t size;

    if (!entry.GetSize(&size))
        return nullptr;

    uint8_t *buf = new uint8_t[size];
    FixedMemoryStream output(buf, size);

    if (!ExtractCommon(entry, &output, size))
    {
        delete[] buf;
        return nullptr;
    }

    *psize = size;
    return buf;
}

bool PakFile::AddFile(const std::string &file_path, const std::string &internal_path)
{
	PakFileEntry entry;
	
    entry.path.str = internal_path;
    entry.external_path = file_path;
    entry.buf = nullptr;
	entry.offset = INVALID_OFFSET;
	entry.size = entry.uncompressed_size = 0;
	entry.compression_method = 0;
	entry.encrypted = 0;
	entry.compression_block_size = 0;
	
	files.push_back(entry);
	
	return true;
}

