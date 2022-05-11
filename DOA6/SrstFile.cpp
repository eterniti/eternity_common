#include "SrstFile.h"
#include "debug.h"

#define READ_BUF_SIZE   (8*1024*1024)

SrstFile::SrstFile()
{
    cont = nullptr;
    big_endian = false;
    is_raw = false;
    progress = nullptr;
    srsa = nullptr;
}

SrstFile::~SrstFile()
{
    if (cont)
        delete cont;
}

void SrstFile::Reset()
{
    if (cont)
    {
        delete cont;
        cont = nullptr;
    }

    entries.clear();
    is_raw = false;
}

bool SrstFile::LoadInternal()
{
    if (!cont)
        return false;

    cont->Seek(0, SEEK_SET);

    SRSTHeader hdr;

    if (!cont->Read(&hdr, sizeof(SRSTHeader)))
        return false;

    if (hdr.srst_signature != SRST_SIGNATURE)
        return false;

    if (hdr.file_size > cont->GetSize())
        return false;

    SRSTEntry native_entry;

    while (cont->Read(&native_entry, sizeof(SRSTEntry)))
    {
        SrstEntry entry;
        KOVSEntry kovs;

        if (native_entry.signature != SRST_ENTRY_SIGNATURE)
            return false;

        entry.id = native_entry.id;
        entry.srst_offset = cont->Tell() - sizeof(SRSTEntry);
        entry.srst_size = native_entry.entry_size;

        if (native_entry.header_size != sizeof(SRSTEntry))
        {
            DPRINTF("Warning: header size not 0x20 around offset %Ix\n", (size_t)cont->Tell());
            cont->Seek(native_entry.entry_size - sizeof(SRSTEntry), SEEK_CUR);
        }

        if (!cont->Read(&kovs, sizeof(KOVSEntry)))
            return false;

        if (kovs.signature != KOVS_SIGNATURE)
            return false;

        entry.data_offset = cont->Tell();
        entry.data_size = kovs.ogg_size;
        entry.name = Utils::UnsignedToHexString(entry.id, true) + ".ogg";

        if (srsa)
        {
            size_t srsa_idx = srsa->FindFileByTypeAndId(SrsaEntry::Type::EXT_AUDIO, entry.id);
            //DPRINTF("Found at %Id\n", srsa_idx);
            if (srsa_idx == (size_t)-1)
            {
                DPRINTF("(SRST) WARNING: Cannot find 0x%08x in the srsa!\n", entry.id);
                //return false;
            }
            else
            {
                entry.name = (*srsa)[srsa_idx].name;
                size_t dot = entry.name.rfind('.');
                if (dot != std::string::npos)
                {
                    entry.name = entry.name.substr(0, dot) + ".ogg";
                }
            }
        }

        cont->Seek(entry.srst_offset + entry.srst_size, SEEK_SET);
        entries.push_back(entry);
    }

    return true;
}

bool SrstFile::SaveInternal(FileStream *out)
{
    SRSTHeader hdr;

    cont->Seek(0, SEEK_SET);
    if (!cont->Read(&hdr, sizeof(SRSTHeader)))
        return false;

    if (!out->Write(&hdr, sizeof(SRSTHeader)))
        return false;

    size_t current = 1;

    for (const SrstEntry &entry : entries)
    {
        cont->Seek(entry.srst_offset, SEEK_SET);

        SRSTEntry native_entry;
        KOVSEntry kovs_entry;
        size_t file_size;
        uint8_t *buf;

        if (!cont->Read(&native_entry, sizeof(SRSTEntry)))
            return false;

        if (!cont->Read(&kovs_entry, sizeof(KOVSEntry)))
            return false;

        file_size = (entry.external) ? entry.external->GetSize() : entry.data_size;
        buf = new uint8_t[file_size];

        if (entry.external)
        {
            entry.external->Seek(0, SEEK_SET);
            if (!entry.external->Read(buf, file_size))
            {
                delete[] buf;
                return false;
            }
        }
        else
        {
            cont->Seek(entry.data_offset, SEEK_SET);
            if (!cont->Read(buf, file_size))
            {
                delete[] buf;
                return false;
            }
        }

        native_entry.kovs_size = file_size + sizeof(KOVSEntry);
        native_entry.entry_size = Utils::Align2(native_entry.kovs_size + sizeof(SRSTEntry), 0x10);

        if (!out->Write(&native_entry, sizeof(SRSTEntry)))
        {
            delete[] buf;
            return false;
        }

        kovs_entry.ogg_size = file_size;

        if (srsa)
        {
            size_t srsa_idx = srsa->FindFileByTypeAndId(SrsaEntry::Type::EXT_AUDIO, entry.id);
            if (srsa_idx == (size_t)-1)
            {
                DPRINTF("(SRST) Cannot find %s in the srsa.\n", entry.name.c_str());
                delete[] buf;
                return false;
            }

            if (!(*srsa)[srsa_idx].UpdateExternalOgg((uint32_t)out->Tell() - 0x10))
            {
                DPRINTF("(SRST) Failed to update srsa external info in %s.\n", entry.name.c_str());
                delete[] buf;
                return false;
            }
        }

        if (!out->Write(&kovs_entry, sizeof(KOVSEntry)))
        {
            delete[] buf;
            return false;
        }

        if (entry.external)
        {
            size_t h_size = (file_size > 0x100) ? 0x100 : file_size;
            for (size_t i = 0; i < h_size; i++)
            {
                buf[i] ^= (uint8_t)i;
            }
        }

        if (!out->Write(buf, file_size))
        {
            delete[] buf;
            return false;
        }

        delete[] buf;

        while (out->GetSize() & 0xF)
            out->Write8(0);

        if (progress)
            progress(current, entries.size());

        current++;
    }

    hdr.file_size = out->Tell();
    hdr.ktsr.size = hdr.ktsr.size2 = (uint32_t)hdr.file_size - 0x10;

    out->Seek(0, SEEK_SET);
    return out->Write(&hdr, sizeof(SRSTHeader));
}

bool SrstFile::LoadRawInternal()
{
    is_raw = true;
    uint8_t *buf = new uint8_t[READ_BUF_SIZE];
    uint64_t remaining = cont->GetSize();

    if (remaining < 0x24)
    {
        delete[] buf;
        return false;
    }

    do
    {
        size_t read_size = (remaining > READ_BUF_SIZE) ? READ_BUF_SIZE : (size_t)remaining;
        uint64_t start_pos = cont->Tell();

        if (!cont->Read(buf, read_size))
            break;

        bool skip_post = false;

        for (size_t i = 0; i < read_size; i++)
        {
            if (*(uint32_t *)&buf[i] == KOVS_SIGNATURE)
            {
                if ((i+0x24) < read_size)
                {
                    if (*(uint32_t *)&buf[i+0x20] == SCRAMBLED_OGG_SIGNATURE)
                    {
                        remaining -= i;
                        cont->Seek(start_pos + i, SEEK_SET);

                        KOVSEntry kovs;
                        if (cont->Read(&kovs, sizeof(kovs)))
                        {
                            SrstEntry entry;
                            remaining -= sizeof(kovs);

                            if (kovs.signature != KOVS_SIGNATURE)
                            {
                                DPRINTF("%s Internal error.\n", FUNCNAME);
                                delete[] buf;
                                return false;
                            }

                            entry.data_offset = cont->Tell();
                            entry.data_size = kovs.ogg_size;
                            entry.name = Utils::ToString(entries.size()) + ".ogg";

                            cont->Seek(start_pos + i + sizeof(kovs) + entry.data_size, SEEK_SET);
                            remaining -= entry.data_size;

                            entries.push_back(entry);
                        }
                        else
                        {
                            //DPRINTF("WTF\n");
                        }

                        skip_post = true;
                        break;
                    }
                }
                else
                {
                    if (read_size < READ_BUF_SIZE)
                        break;

                    remaining -= 0x24;
                    cont->Seek(start_pos + 0x24, SEEK_SET);
                    skip_post = true;
                    break;
                }
            }
        }

        if (skip_post)
            continue;

        remaining -= read_size;

    } while (remaining > 0);

    delete[] buf;
    return (entries.size() > 0);
}

bool SrstFile::IsSameOggAsExternal(SrstEntry &entry)
{
    if (!entry.external)
        return false;

    if (entry.external->GetSize() != entry.data_size)
        return false;

    if (entry.data_size <= 0x100)
        return false;

    uint8_t *raw_data = new uint8_t[entry.data_size];
    cont->Seek(entry.data_offset, SEEK_SET);
    if (!cont->Read(raw_data, entry.data_size))
    {
        delete[] raw_data;
        return false;
    }

    uint8_t *buf = new uint8_t[entry.external->GetSize()];
    entry.external->Seek(0, SEEK_SET);
    bool ret = entry.external->Read(buf, entry.external->GetSize());
    if (ret)
    {
        ret = (memcmp(raw_data+0x100, buf+0x100, entry.data_size-0x100) == 0);
        if (ret)
        {
            for (size_t i = 0; i < 0x100; i++)
            {
                if (buf[i] != (raw_data[i] ^ i))
                {
                    ret = false;
                    break;
                }
            }
        }
    }

    delete[] buf;
    delete[] raw_data;

    entry.external->Seek(0, SEEK_SET);
    return ret;
}

bool SrstFile::LoadFromFile(const std::string &path, bool show_error)
{
    Reset();

    cont = new FileStream("rb");
    if (!cont->LoadFromFile(path, show_error))
    {
        Reset();
        return false;
    }

    bool ret = LoadInternal();
    if (!ret)
        Reset();

    return ret;
}

bool SrstFile::Extract(size_t idx, Stream *stream)
{
    if (idx >= entries.size())
        return false;

    const SrstEntry &entry = entries[idx];
    if (!cont->Seek(entry.data_offset, SEEK_SET))
        return false;

    uint32_t written = 0;

    uint8_t *hdr = new uint8_t[0x100];
    uint16_t h_size = (entry.data_size < 0x100) ? (uint16_t)entry.data_size : 0x100;

    if (!cont->Read(hdr, h_size))
    {
        delete[] hdr;
        return false;
    }

    for (uint16_t i = 0; i < h_size; i++)
    {
        hdr[i] ^= (uint8_t)i;
    }

    if (!stream->Write(hdr, h_size))
    {
        delete[] hdr;
        return false;
    }

    delete[] hdr;

    if (entry.data_size <= 0x100)
        return true;

    written = 0x100;

    uint8_t *buf = new uint8_t[READ_BUF_SIZE];
    uint32_t remaining = entry.data_size - written;

    while (remaining > 0)
    {
        uint32_t to_read = (remaining > READ_BUF_SIZE) ? READ_BUF_SIZE : remaining;
        if (!cont->Read(buf, to_read))
        {
            delete[] buf;
            return false;
        }

        if (!stream->Write(buf, to_read))
        {
            delete[] buf;
            return false;
        }

        remaining -= to_read;
    }

    delete[] buf;
    return true;
}

bool SrstFile::Extract(size_t idx, const std::string &dir_path)
{
    if (idx >= entries.size())
        return false;

    FileStream *stream = new FileStream("wb");
    if (!stream->LoadFromFile(Utils::MakePathString(dir_path, entries[idx].name)))
    {
        delete stream;
        return false;
    }

    bool ret = Extract(idx, stream);
    delete stream;

    return ret;
}

bool SrstFile::SetExternal(size_t idx, const std::string &path)
{
    if (idx >= entries.size())
        return false;

    SrstEntry &entry = entries[idx];
    FileStream *stream = new FileStream("rb");
    uint32_t signature;

    if (!stream->LoadFromFile(path))
        return false;

    if (!stream->Read32(&signature))
        return false;

    if (signature != 0x5367674F)
    {
        DPRINTF("Not valid .ogg file (%s)\n", path.c_str());
        delete stream;
        return false;
    }

    stream->Seek(0, SEEK_SET);
    entry.external = stream;

    if (IsSameOggAsExternal(entry))
    {
        delete entry.external;
        entry.external = nullptr;
    }

    if (srsa)
    {
        size_t srsa_idx = srsa->FindFileByTypeAndId(SrsaEntry::Type::EXT_AUDIO, entry.id);
        if (srsa_idx == (size_t)-1)
        {
            DPRINTF("(SRST) Failed to get the index in the srsa for %s\n", entry.name.c_str());
            return false;
        }

        if (!(*srsa)[srsa_idx].SetExternalOgg(path))
        {
            DPRINTF("(SRST) Failed to set external ogg in the srsa.\n");
            return false;
        }
    }

    return true;
}

bool SrstFile::SaveToFile(const std::string &path, bool show_error, bool build_path)
{
    FileStream out("r+b");

    FILE *f = (build_path) ? Utils::fopen_create_path(path, "wb") : fopen(path.c_str(), "wb");
    if (!f)
        return false;

    fclose(f);

    if (!out.LoadFromFile(path, show_error))
        return false;

    return SaveInternal(&out);
}

bool SrstFile::LoadFromRaw(const std::string &path, bool show_error)
{
    Reset();

    cont = new FileStream("rb");
    if (!cont->LoadFromFile(path, show_error))
    {
        Reset();
        return false;
    }

    bool ret = LoadRawInternal();
    if (!ret)
        Reset();

    return ret;
}

