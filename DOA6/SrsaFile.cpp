#include "SrsaFile.h"
#include "FfmpegAudioFile.h"
#include "WavFile.h"
#include "debug.h"

static const uint8_t adpcm_hdr[90] =
{
    0x52, 0x49, 0x46, 0x46, 0x74, 0x54, 0x19, 0x00, 0x57, 0x41, 0x56, 0x45, 0x66, 0x6D, 0x74, 0x20,
    0x32, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 0x5C, 0xBB, 0x00, 0x00, 0x80, 0x3E, 0x00, 0x00,
    0x46, 0x00, 0x04, 0x00, 0x20, 0x00, 0xF4, 0x07, 0x07, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02,
    0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x40, 0x00, 0xF0, 0x00, 0x00, 0x00, 0xCC, 0x01,
    0x30, 0xFF, 0x88, 0x01, 0x18, 0xFF, 0x66, 0x61, 0x63, 0x74, 0x04, 0x00, 0x00, 0x00, 0x04, 0x5C,
    0x32, 0x00, 0x64, 0x61, 0x74, 0x61, 0x00, 0x54, 0x19, 0x00
};

void SrsaEntry::Copy(const SrsaEntry &other)
{
    this->buf = other.buf;
    this->decoded = other.decoded;
    this->id = other.id;
    this->name = other.name;

    if (this->decoded)
    {
        uintptr_t offs = (uintptr_t)other.raw_data - (uintptr_t)other.buf.data();
        this->raw_data = this->buf.data() + offs;
        this->raw_data_size = other.raw_data_size;
        this->type = other.type;

        this->sample_rate = other.sample_rate;
        this->num_samples = other.num_samples;
        this->block_align = other.block_align;

        this->kovs = nullptr;
        this->fmt_ogg1 = nullptr;
        this->fmt_ogg2 = nullptr;

        this->fmt_adpcm1 = nullptr;
        this->fmt_adpcm2 = nullptr;

        if (other.kovs)
        {
            uintptr_t offs = (uintptr_t)other.kovs - (uintptr_t)other.buf.data();
            this->kovs = (KOVSEntry *)(this->buf.data() + offs);
        }

        if (other.fmt_ogg1)
        {
            uintptr_t offs = (uintptr_t)other.fmt_ogg1 - (uintptr_t)other.buf.data();
            this->fmt_ogg1 = (AUDIOFmtOgg1 *)(this->buf.data() + offs);
        }

        if (other.fmt_ogg2)
        {
            uintptr_t offs = (uintptr_t)other.fmt_ogg2 - (uintptr_t)other.buf.data();
            this->fmt_ogg2 = (AUDIOFmtOgg2 *)(this->buf.data() + offs);
        }

        if (other.fmt_adpcm1)
        {
            uintptr_t offs = (uintptr_t)other.fmt_adpcm1 - (uintptr_t)other.buf.data();
            this->fmt_adpcm1 = (AUDIOFmtAdpcm1 *)(this->buf.data() + offs);
        }

        if (other.fmt_adpcm2)
        {
            uintptr_t offs = (uintptr_t)other.fmt_adpcm2 - (uintptr_t)other.buf.data();
            this->fmt_adpcm2= (AUDIOFmtAdpcm2 *)(this->buf.data() + offs);
        }
    }
    else
    {
        type = Type::SUBP;

        raw_data = nullptr;
        raw_data_size = 0;

        sample_rate = num_samples = 0;
        block_align = 0;
    }
}

static uint32_t GetAproximateSR(uint32_t sample_rate)
{
    static const std::vector<uint32_t> freqs =
    {
        8000, 10000, 12000, 16000, 20000, 22050, 24000, 28000, 32000,
        36000, 40000, 44100, 48000
    };

    uint32_t ret = 48000;
    uint32_t min = 0xFFFFFFFF;

    for (uint32_t freq : freqs)
    {
        uint32_t val = abs((int)freq - (int)sample_rate);
        if (val < min)
        {
            ret = freq;
            min = val;
        }
    }

    return ret;
}

bool SrsaEntry::Decode()
{
    if (decoded)
        return true;

    const SRSAEntry *entry = (const SRSAEntry *)buf.data();
    id = entry->id;

    if (entry->type_signature == SUBP_SIGNATURE)
    {
        SUBPEntry *subp_entry = (SUBPEntry *)entry;
        name = subp_entry->GetName() + ".subd";

        type = Type::SUBP;
        raw_data = buf.data();
        raw_data_size = buf.size();
    }
    else if (entry->type_signature == AUDIO_SIGNATURE)
    {
        AUDIOEntry *audio_entry = (AUDIOEntry *)entry;
        name = audio_entry->GetName();

        uint8_t *data = audio_entry->GetDataPtr();
        fmt_adpcm1 = (AUDIOFmtAdpcm1 *)data;
        fmt_adpcm2 = (AUDIOFmtAdpcm2 *)data;
        fmt_ogg1 = (AUDIOFmtOgg1 *)data;
        fmt_ogg2 = (AUDIOFmtOgg2 *)data;

        if (fmt_adpcm1->fmt == AUDIO_FMT_ADPCM1 || fmt_adpcm2->fmt == AUDIO_FMT_ADPCM2)
        {
            name += ".wav";
            type = Type::ADPCM;

            //DPRINTF("%d -> %d / %s\n", fmt_adpcm1->sample_rate, GetAproximateSR(fmt_adpcm1->sample_rate), name.c_str());
            //sample_rate = fmt_adpcm1->sample_rate;
            sample_rate = GetAproximateSR(fmt_adpcm1->sample_rate); //48000; // Let's set to exactly 48 Khz.
            num_samples = fmt_adpcm1->num_blocks;
            block_align = *(uint16_t *)(fmt_adpcm1->GetFormatPtr() + 2);
            raw_data_size = fmt_adpcm1->GetDataSize();
            raw_data = fmt_adpcm1->GetDataPtr();
        }
        else if (fmt_ogg1->fmt == AUDIO_FMT_OGG1 || fmt_ogg2->fmt == AUDIO_FMT_OGG2)
        {
            uint8_t *kovs_start;

            if (fmt_ogg1->fmt == AUDIO_FMT_OGG1)
            {
                kovs_start = data + sizeof(AUDIOFmtOgg1);
            }
            else
            {
                kovs_start = data + fmt_ogg2->GetFullSize();
            }

            uintptr_t hdr_size = Utils::Align2((uintptr_t)kovs_start - (uintptr_t)buf.data(), 0x10);

            kovs = (KOVSEntry *)(buf.data() + hdr_size);
            if (kovs->signature != KOVS_SIGNATURE)
            {
                /*DPRINTF("SRSA: was expecting a KOVS_SIGNATURE, but got 0x%08x (in %s).\n", kovs->signature, name.c_str());
                return false;*/
                name += ".ext_audio";
                type = Type::EXT_AUDIO;
                raw_data = buf.data();
                raw_data_size = buf.size();
                kovs = nullptr;
            }
            else
            {
                name += ".ogg";
                type = Type::OGG;

                uint8_t *ogg_start = (uint8_t *)(kovs+1);
                if (*(uint32_t *)ogg_start != SCRAMBLED_OGG_SIGNATURE)
                {
                    DPRINTF("SRSA: was expecting SCRAMBLED_OGG_SIGNATURE but got 0x%08x (in %s)\n", *(const uint32_t *)ogg_start, name.c_str());
                    return false;
                }

                raw_data = ogg_start;
                raw_data_size = kovs->ogg_size;
            }
        }
        else
        {
            DPRINTF("SRSA: Unrecognized format 0x%08x for %s\n", fmt_adpcm1->fmt, name.c_str());
        }
    }
    else if (entry->type_signature == UNK_SIGNATURE3)
    {
        UNK3Entry *unk3_entry = (UNK3Entry *)entry;
        name = unk3_entry->GetName() + ".unk3";

        type = Type::UNK3;
        raw_data = buf.data();
        raw_data_size = buf.size();
    }
    else
    {
       DPRINTF("Unknown SRSA entry type signature (0x%08x)\n", entry->type_signature);
       return false;
    }

    decoded = true;
    return true;
}

bool SrsaEntry::IsSameAdpcm(Stream &stream) const
{
    if (type != Type::ADPCM || stream.GetSize() != (raw_data_size+sizeof(adpcm_hdr)))
        return false;

    uint8_t *buf = new uint8_t[stream.GetSize()];
    stream.Seek(0, SEEK_SET);
    bool ret = stream.Read(buf, stream.GetSize());
    if (ret)
    {
        ret = (memcmp(raw_data, buf+sizeof(adpcm_hdr), raw_data_size) == 0);
    }

    delete[] buf;
    stream.Seek(0, SEEK_SET);
    return ret;
}

bool SrsaEntry::IsSameOgg(Stream &stream) const
{
    if (type != Type::OGG || stream.GetSize() != raw_data_size)
        return false;

    if (raw_data_size <= 0x100)
        return false;

    uint8_t *buf = new uint8_t[stream.GetSize()];
    stream.Seek(0, SEEK_SET);
    bool ret = stream.Read(buf, stream.GetSize());
    if (ret)
    {
        ret = (memcmp(raw_data+0x100, buf+0x100, raw_data_size-0x100) == 0);
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
    stream.Seek(0, SEEK_SET);
    return ret;
}

bool SrsaEntry::Extract(Stream &stream) const
{
    uint32_t written = 0;

    if (type == Type::ADPCM)
    {
        uint8_t *hdr = new uint8_t[sizeof(adpcm_hdr)];
        memcpy(hdr, adpcm_hdr, sizeof(adpcm_hdr));

        float duration = (float)num_samples / (float)sample_rate;
        uint32_t byte_rate = (uint32_t) ((float)raw_data_size / duration);

        *(uint32_t *)&hdr[4] = sizeof(adpcm_hdr) + raw_data_size - 8;
        *(uint32_t *)&hdr[0x18] = sample_rate;
        *(uint32_t *)&hdr[0x1C] = byte_rate;
        *(uint16_t *)&hdr[0x20] = block_align;
        *(uint16_t *)&hdr[0x26] = block_align * 2 - 12;
        *(uint32_t *)&hdr[0x4E] = num_samples;
        *(uint32_t *)&hdr[0x56] = raw_data_size;

        bool ret = stream.Write(hdr, sizeof(adpcm_hdr));
        delete[] hdr;

        if (!ret)
            return false;
    }
    else if (type == Type::OGG)
    {
        uint8_t *hdr = new uint8_t[0x100];
        uint16_t h_size = (raw_data_size < 0x100) ? (uint16_t)raw_data_size : 0x100;

        for (uint16_t i = 0; i < h_size; i++)
        {
           hdr[i] = raw_data[i] ^ (uint8_t)i;
        }

        if (!stream.Write(hdr, h_size))
        {
           delete[] hdr;
           return false;
        }

        delete[] hdr;

        if (raw_data_size <= 0x100)
           return true;

        written = 0x100;
    }

    if (!stream.Write(raw_data+written, raw_data_size-written))
        return false;

    return true;
}

bool SrsaEntry::Extract(const std::string &dir) const
{
    if (!decoded)
        throw "Called in invalid state";

    FileStream *stream = new FileStream("wb");
    if (!stream->LoadFromFile(Utils::MakePathString(dir, name)))
    {
        delete stream;
        return false;
    }

    bool ret = Extract(*stream);
    delete stream;

    return ret;
}

bool SrsaEntry::Replace(const std::string &file)
{
    uint32_t signature;
    FileStream stream;
    std::string fn = Utils::GetFileNameString(file);

    if (!stream.LoadFromFile(file))
        return false;

    if (!stream.Read32(&signature))
        return false;

    stream.Seek(0, SEEK_SET);

    if (type == Type::SUBP || type == Type::EXT_AUDIO || type == Type::UNK3)
    {
        decoded = false;

        buf.resize(stream.GetSize());
        if (!stream.Read(buf.data(), buf.size()))
            return false;

        if (!Decode())
            return false;
    }
    else if (type == Type::OGG)
    {
        if (signature != 0x5367674F)
        {
            DPRINTF("Invalid .ogg file (%s)\n", fn.c_str());
            return false;
        }

        if (IsSameOgg(stream))
            return true; // Skip replace

        if (stream.GetSize() < 0x100)
        {
            DPRINTF(".ogg file is too small (%s)\n", fn.c_str());
            return false;
        }

        FfmpegAudioFile ogg;

        if (!ogg.LoadFromFile(file))
            return false;        

        SRSAEntry *srsa = (SRSAEntry *)buf.data();
        size_t full_header_size = (uintptr_t)raw_data - (uintptr_t)buf.data();
        size_t entry_header_size = (uintptr_t)fmt_ogg1 - (uintptr_t)buf.data();
        size_t fmt_size = (uintptr_t)kovs - (uintptr_t)fmt_ogg1;
        size_t ogg_aligned_size = Utils::Align2(stream.GetSize(), 0x10);

        kovs->ogg_size = (uint32_t)stream.GetSize();
        fmt_ogg1->kovs_size = ogg_aligned_size + sizeof(KOVSEntry);
        fmt_ogg1->file_size = fmt_ogg1->kovs_size + fmt_size;
        srsa->entry_size = fmt_ogg1->file_size + entry_header_size;

        fmt_ogg1->num_channels = ogg.GetNumChannels();
        fmt_ogg1->sample_rate = ogg.GetSampleRate();
        fmt_ogg1->num_samples = ogg.GetNumSamples(); //Utils::Align2(ogg.GetNumSamples(), 0x10);

        std::vector<uint8_t> new_buf;
        new_buf.resize(srsa->entry_size, 0);
        memcpy(new_buf.data(), buf.data(), full_header_size);

        if (!stream.Read(new_buf.data() + full_header_size, stream.GetSize()))
            return false;

        for (uint16_t i = 0; i < 0x100; i++)
        {
           (new_buf.data())[full_header_size+i] ^= (uint8_t)i;
        }

        decoded = false;
        buf = new_buf;

        if (!Decode())
            return false;
    }
    else if (type == Type::ADPCM)
    {
        if (signature != RIFF_SIGNATURE)
        {
            DPRINTF("Invalid .wav file (%s)\n", fn.c_str());
            return false;
        }

        WavFile wav;

        if (!wav.LoadFromFile(file))
            return false;

        if (wav.GetFormat() != WAV_ADPCM)
        {
            DPRINTF("I expected a wav with MS-ADPCM codec, not with PCM (file \"%s\")\n", fn.c_str());
            return false;
        }

        if (IsSameAdpcm(stream))
            return true; // Skip replace

        if (!wav.MoveToMemory())
        {
            DPRINTF("(SRSA) Internal error.\n");
            return false;
        }

        MemoryStream *samples_stream = dynamic_cast<MemoryStream *>(wav.GetSamplesStream());
        SRSAEntry *srsa = (SRSAEntry *)buf.data();
        size_t full_header_size = (uintptr_t)raw_data - (uintptr_t)buf.data();
        size_t entry_header_size = (uintptr_t)fmt_adpcm1 - (uintptr_t)buf.data();
        size_t fmt_size = (uintptr_t)raw_data - (uintptr_t)fmt_adpcm1;
        size_t aligned_file_size = Utils::Align2(samples_stream->GetSize(), 0x10);

        uint16_t f = *(uint16_t *)(fmt_adpcm1->GetFormatPtr());
        fmt_adpcm1->sample_rate = wav.GetSampleRate();
        fmt_adpcm1->num_blocks = ((uint32_t)samples_stream->GetSize() / (uint32_t)wav.GetBlockAlign()) * f;
        *(uint16_t *)(fmt_adpcm1->GetFormatPtr() + 2) = wav.GetBlockAlign();

        fmt_adpcm1->SetDataSize(samples_stream->GetSize());
        fmt_adpcm1->file_size = fmt_size + aligned_file_size;
        srsa->entry_size = fmt_adpcm1->file_size + entry_header_size;

        std::vector<uint8_t> new_buf;
        new_buf.resize(srsa->entry_size, 0);
        memcpy(new_buf.data(), buf.data(), full_header_size);
        memcpy(new_buf.data()+full_header_size, samples_stream->GetMemory(false), samples_stream->GetSize());

        decoded = false;
        buf = new_buf;

        if (!Decode())
            return false;
    }

    return true;
}

bool SrsaEntry::SetExternalOgg(const std::string &file)
{
    if (!decoded || type != Type::EXT_AUDIO || !fmt_ogg1)
        return false;

    FileStream stream("rb");
    std::string fn = Utils::GetFileNameString(file);
    uint32_t signature;

    if (!stream.LoadFromFile(file))
        return false;

    if (!stream.Read32(&signature))
        return false;

    stream.Seek(0, SEEK_SET);

    if (signature != 0x5367674F)
    {
        DPRINTF("Invalid .ogg file (%s)\n", fn.c_str());
        return false;
    }

    if (stream.GetSize() < 0x100)
    {
        DPRINTF(".ogg file is too small (%s)\n", fn.c_str());
        return false;
    }

    FfmpegAudioFile ogg;

    if (!ogg.LoadFromFile(file))
        return false;

    fmt_ogg1->num_channels = ogg.GetNumChannels();
    fmt_ogg1->sample_rate = ogg.GetSampleRate();
    fmt_ogg1->num_samples = ogg.GetNumSamples();
    fmt_ogg1->kovs_size = stream.GetSize() + sizeof(KOVSEntry);

    return true;
}

bool SrsaEntry::UpdateExternalOgg(uint32_t offset)
{
    if (!decoded || type != Type::EXT_AUDIO || !fmt_ogg1)
        return false;

    //DPRINTF("Update external 0x%x\n", offset);

    fmt_ogg1->kovs_offset = offset;
    return true;
}

SrsaFile::SrsaFile()
{
    this->big_endian = false;
}

SrsaFile::~SrsaFile()
{

}

size_t SrsaFile::CalculateFileSize() const
{
    size_t result = sizeof(SRSAHeader);

    for (const SrsaEntry &entry : entries)
    {
        if (entry.buf.size() & 0xF)
            DPRINTF("Warning: entry size of %s not multiple of 0x10 (0%08Ix)", entry.name.c_str(), entry.buf.size());

        result += entry.buf.size();
    }

    return result;
}

void SrsaFile::Reset()
{
    entries.clear();
    unk_hash1 = 0x1A487B77;
    unk_hash2 = 0x9CC9E1D1;
}


bool SrsaFile::Load(const uint8_t *buf, size_t size)
{
    Reset();
    if (!buf || size < sizeof(SRSAHeader))
        return false;

    FixedMemoryStream mem(const_cast<uint8_t *>(buf), size);
    const SRSAHeader *hdr;

    if (!mem.FastRead((uint8_t **)&hdr, sizeof(SRSAHeader)))
        return false;

    if (hdr->signature != SRSA_SIGNATURE)
    {
        DPRINTF("Invalid SRSA signature.\n");
        return false;
    }

    if (hdr->ktsr.signature != KTSR_SIGNATURE)
    {
        DPRINTF("SRSA: Invalid KTSR signature\n");
        return false;
    }

    unk_hash1 = hdr->ktsr.unk_04;
    unk_hash2 = hdr->ktsr.unk_0C;

    uint32_t entry_sig;

    while (mem.Read32(&entry_sig))
    {
        uint32_t entry_size;

        if (!mem.Read32(&entry_size))
        {
            DPRINTF("SRSA: Was expecting size after signature, but end of stream reached.\n");
            return false;
        }

        //DPRINTF("0x%08x, 0x%08x, 0x%Ix\n", entry_sig, entry_size, (size_t)mem.Tell());

        entries.push_back(SrsaEntry());
        entries.back().buf.resize(entry_size);

        mem.Seek(-8, SEEK_CUR);

        if (!mem.Read(entries.back().buf.data(), entry_size))
        {
            DPRINTF("SRSA: unable to read entry with size 0x%08x (near 0x%Ix)\n", entry_size, (size_t)mem.Tell());
            return false;
        }
    }

    for (SrsaEntry &entry : entries)
    {
        if (!entry.Decode())
            return false;
    }

    return true;
}

uint8_t *SrsaFile::Save(size_t *psize)
{
    *psize = CalculateFileSize();
    uint8_t *buf = new uint8_t[*psize];
    memset(buf, 0, *psize);

    SRSAHeader *hdr = (SRSAHeader *)buf;
    hdr->signature = SRSA_SIGNATURE;
    hdr->file_size = *psize;
    hdr->ktsr.signature = KTSR_SIGNATURE;
    hdr->ktsr.unk_04 = unk_hash1;
    hdr->ktsr.unk_08 = 1;
    hdr->ktsr.unk_0A = 0x100;
    hdr->ktsr.unk_0C = unk_hash2;
    hdr->ktsr.size = hdr->ktsr.size2 = *psize - 0x10;

    uint8_t *ptr = buf + sizeof(SRSAHeader);

    for (const SrsaEntry &entry : entries)
    {
        if (!entry.decoded)
        {
            DPRINTF("Internal error, entry not decoded.\n");
            exit(-1);
        }

        if (entry.type == SrsaEntry::Type::OGG && entry.fmt_ogg1 && entry.kovs)
        {
            uintptr_t rel_ktsr = (uintptr_t)ptr - (uintptr_t)buf - 0x10;
            uintptr_t rel_kovs = (uintptr_t)entry.kovs - (uintptr_t)entry.buf.data();

            entry.fmt_ogg1->kovs_offset = (uint32_t)rel_ktsr + (uint32_t)rel_kovs;
        }

        memcpy(ptr, entry.buf.data(), entry.buf.size());
        ptr += entry.buf.size();
    }

    return buf;
}

size_t SrsaFile::FindFile(const std::string &name) const
{
    std::string l_name = Utils::ToLowerCase(name);

    for (size_t i = 0; i < entries.size(); i++)
        if (Utils::ToLowerCase(entries[i].name) == l_name)
            return i;

    return (size_t)-1;
}

size_t SrsaFile::FindFileById(uint32_t id) const
{
    for (size_t i = 0; i < entries.size(); i++)
        if (entries[i].id == id)
            return i;

    return (size_t)-1;
}

size_t SrsaFile::FindFileByTypeAndId(SrsaEntry::Type type, uint32_t id) const
{
    for (size_t i = 0; i < entries.size(); i++)
        if (entries[i].type == type && entries[i].id == id)
            return i;

    return (size_t)-1;
}



