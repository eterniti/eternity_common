#include "AaxFile.h"
#include "WavFile.h"
#include "FixedMemoryStream.h"

// Until we add support for creation of columns/rows in UtfFile, this is needed to create new files
static const uint8_t dummy_utfadx[96] =
{
    0x40, 0x55, 0x54, 0x46, 0x00, 0x00, 0x00, 0x58, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x2B,
    0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x07, 0x00, 0x02, 0x00, 0x09, 0x00, 0x00, 0x00, 0x01,
    0x5B, 0x00, 0x00, 0x00, 0x0B, 0x50, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x10, 0x01, 0x3C, 0x4E, 0x55, 0x4C, 0x4C, 0x3E, 0x00, 0x41, 0x41, 0x58, 0x00, 0x64, 0x61,
    0x74, 0x61, 0x00, 0x6C, 0x70, 0x66, 0x6C, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33
};

AaxFile::AaxFile() : UtfFile()
{
}

AaxFile::~AaxFile()
{
    ResetAAX(false);
}

void AaxFile::ResetAAX(bool load_dummy)
{
    for (AdxFile *&adx : adxs)
        delete adx;

    adxs.clear();
    loopflg.clear();

    if (!load_dummy)
        return;

    if (!UtfFile::Load(dummy_utfadx, sizeof(dummy_utfadx)))
    {
        DPRINTF("%s: Internal error.\n", FUNCNAME);
    }
}

bool AaxFile::Load(const uint8_t *buf, size_t size)
{
    ResetAAX();

    if (!UtfFile::Load(buf, size))
       return false;

   if (GetTableName() != "AAX")
       return false;

   unsigned int num_files = GetNumRows();
   if (num_files == 0)
       return false;

   adxs.resize(num_files, nullptr);
   loopflg.resize(num_files);

   uint32_t sample_rate;
   uint16_t num_channels;

   for (unsigned int i = 0; i < num_files; i++)
   {
        AdxFile *&adx = adxs[i];
        uint8_t *adx_data;
        unsigned int adx_size;

        if (!GetByte("lpflg", &loopflg[i], i))
        {
            ResetAAX();
            return false;
        }

        adx_data = GetBlob("data", &adx_size, false, i);
        if (!adx_data)
        {
            ResetAAX();
            return false;
        }

        adx = new AdxFile();
        if (!adx->Load(adx_data, adx_size))
        {
            ResetAAX();
            return false;
        }

        if (i == 0)
        {
            sample_rate = adx->GetSampleRate();
            num_channels = adx->GetNumChannels();
        }
        else
        {
            if (sample_rate != adx->GetSampleRate())
            {
                DPRINTF("%s: Not all subtracks have same sample rate. Not supported.\n");
                ResetAAX();
                return false;
            }

            if (num_channels != adx->GetNumChannels())
            {
                DPRINTF("%s: Not all subtracks have same number of channels. Not supported.\n");
                ResetAAX();
                return false;
            }
        }
   }

   return true;
}

uint8_t *AaxFile::Save(size_t *psize)
{
    if (adxs.size() == 0)
        return nullptr;

    assert(adxs.size() == loopflg.size());

    if (GetNumRows() != adxs.size())
    {
        // TODO: set num rows here
    }

    for (unsigned int i = 0; i < GetNumRows(); i++)
    {
        AdxFile *&adx = adxs[i];

        if (!SetByte("lpflg", loopflg[i], i))
            return nullptr;

        size_t adx_size;
        uint8_t *adx_data = adx->Save(&adx_size);
        if (!adx_data)
            return nullptr;

        if (!SetBlob("data", adx_data, (unsigned int)adx_size, i, true))
            return nullptr;
    }

    return UtfFile::Save(psize);
}

bool AaxFile::LoadFromFile(const std::string &path, bool show_error)
{
    return BaseFile::LoadFromFile(path, show_error);
}

bool AaxFile::SaveToFile(const std::string &path, bool show_error, bool build_path)
{
    return BaseFile::SaveToFile(path, show_error, build_path);
}

uint8_t *AaxFile::Decode(int *format, size_t *psize)
{
    if (adxs.size() == 0)
        return nullptr;

    if (adxs.size() == 1)
        return adxs[0]->Decode(format, psize);

    size_t total_samples = GetNumSamples()*GetNumChannels();
    int16_t *samples = new int16_t[total_samples];
    int16_t *ptr = samples;

    for (AdxFile *&adx : adxs)
    {
        int format;
        size_t size;
        uint8_t *buf = adx->Decode(&format, &size);

        if (!buf)
        {
            delete[] samples;
            return false;
        }

        if (format != AUDIO_FORMAT_16BITS || (size&1))
        {
            DPRINTF("%s: Internal error.\n", FUNCNAME);
            delete[] samples;
            delete[] buf;
            return false;
        }

        memcpy(ptr, buf, size);
        ptr += size/2;

        delete[] buf;
    }

    *format = AUDIO_FORMAT_16BITS;
    *psize = total_samples*sizeof(int16_t);
    return (uint8_t *)samples;
}

bool AaxFile::DecodeToWav(const std::string &file, bool preserve_loop, int format)
{
    if (adxs.size() == 0)
        return false;

    if (adxs.size() == 1)
        return adxs[0]->DecodeToWav(file, preserve_loop, format);

    WavFile wav;
    FILE *w_handle;
    bool decode_as_float = (format == AUDIO_FORMAT_FLOAT);

    if (!decode_as_float && format != 8 && format != 16 && format != 24 && format != 32)
        return false;

    w_handle = fopen(file.c_str(), "wb");
    if (!w_handle)
        return false;

    wav.SetFormat((decode_as_float) ? 3 : 1);
    wav.SetNumChannels(GetNumChannels());
    wav.SetSampleRate(GetSampleRate());
    wav.SetBitDepth((decode_as_float) ? 32 : format);

    if (HasLoop() && preserve_loop)
    {
        uint32_t loop_start, loop_end;
        int count;

        GetLoopSample(&loop_start, &loop_end, &count);
        wav.SetLoopSample(loop_start, loop_end, count);
    }

    uint32_t samples_size = GetNumSamples()*GetNumChannels();

    if (decode_as_float)
    {
        samples_size *= sizeof(float);
    }
    else if (format == 32)
    {
        samples_size *= sizeof(uint32_t);
    }
    else if (format == 24)
    {
        samples_size *= 3;
    }
    else if (format == 16)
    {
        samples_size *= sizeof(uint16_t);
    }

    size_t hdr_size;
    uint8_t *hdr = wav.CreateHeader(&hdr_size, samples_size);

    if (!hdr || fwrite(hdr, 1, hdr_size, w_handle) != hdr_size)
    {
        if (hdr)
            delete[] hdr;

        fclose(w_handle);
        return false;
    }

    delete[] hdr;

    bool ret = true;
    for (AdxFile *&adx : adxs)
    {
        int adx_format;
        size_t size;
        uint8_t *buf = adx->Decode(&adx_format, &size);

        if (!buf)
        {
            ret = false;
            break;
        }

        if (adx_format != AUDIO_FORMAT_16BITS || (size&1))
        {
            DPRINTF("%s: Internal error.\n", FUNCNAME);
            delete[] buf;
            ret = false;
            break;
        }

        uint32_t total_samples = adx->GetNumSamples() * adx->GetNumChannels();
        int16_t *samples_in = (int16_t *)buf;

        if (format == 16)
        {
            // Write directly
            if (fwrite(buf, size, 1, w_handle) != 1)
            {
                ret = false;
                delete[] buf;
                break;
            }
        }
        else if (format == 8)
        {
            int8_t *samples_out = new int8_t[total_samples];

            for (uint32_t i = 0; i < total_samples; i++)
            {
                samples_out[i] = samples_in[i]>>8;
            }

            if (fwrite(samples_out, total_samples, 1, w_handle) != 1)
            {
                ret = false;
                delete[] buf;
                delete[] samples_out;
                break;
            }

            delete[] samples_out;
        }
        else if (format == 24)
        {
            // Slow writing...
            for (uint32_t i = 0; i < total_samples; i++)
            {
                int32_t sample_out = samples_in[i]<<8;

                if (fwrite(&sample_out, 3, 1, w_handle) != 1)
                {
                    ret = false;
                    delete[] buf;
                    break;
                }
            }
        }
        else if (format == 32)
        {
            int32_t *samples_out = new int32_t[total_samples];

            for (uint32_t i = 0; i < total_samples; i++)
            {
                samples_out[i] = samples_in[i]<<16;
            }

            if (fwrite(samples_out, total_samples*sizeof(int32_t), 1, w_handle) != 1)
            {
                ret = false;
                delete[] buf;
                delete[] samples_out;
                break;
            }

            delete[] samples_out;
        }
        else // Float
        {
            float *samples_out = new float[total_samples];

            for (uint32_t i = 0; i < total_samples; i++)
            {
                samples_out[i] = ((float)samples_in[i]) / (float)32768;
            }

            if (fwrite(samples_out, total_samples*sizeof(float), 1, w_handle) != 1)
            {
                ret = false;
                delete[] buf;
                delete[] samples_out;
                break;
            }

            delete[] samples_out;
        }

        delete[] buf;
    }

    fclose(w_handle);
    return ret;
}

bool AaxFile::Encode(uint8_t *buf, size_t size, int format, uint16_t num_channels, uint32_t sample_rate, bool take_ownership)
{
    ResetAAX();

    AdxFile *adx = new AdxFile();
    if (!adx->Encode(buf, size, format, num_channels, sample_rate, take_ownership))
    {
        delete adx;
        return false;
    }

    loopflg.push_back(false);
    adxs.push_back(adx);

    return true;
}

bool AaxFile::EncodeFromWav(const std::string &file, bool move_to_memory, bool preserve_loop)
{
    ResetAAX();

    AdxFile *adx = new AdxFile();
    if (!adx->EncodeFromWav(file, move_to_memory, preserve_loop))
    {
        delete adx;
        return false;
    }

    loopflg.push_back(false);

    if (preserve_loop)
    {
        uint32_t sample_start;
        uint32_t sample_end;
        int count;

        if (adx->GetLoopSample(&sample_start, &sample_end, &count))
        {
            if (sample_start == 0 && sample_end == (adx->GetNumSamples()-1))
            {
                loopflg[0] = true;
            }
        }

        adx->RemoveLoop();
    }

    adxs.push_back(adx);
    return true;
}

void AaxFile::JoinAll()
{
    if (adxs.size() < 2)
        return;

    AdxFile *first = adxs[0];

    for (size_t i = 1; i < adxs.size(); i++)
    {
        AdxFile *adx = adxs[i];

        if (!first->Concat(*adx, false))
        {
            DPRINTF("%s: Fatal, Join failed.\n", FUNCNAME);
            exit(-1);
        }

        delete adx;
    }

    adxs.resize(1);
    loopflg.resize(1);
    loopflg[0] = false;
}

bool AaxFile::GetLoop(float *start, float *end, int *count) const
{
    if (adxs.size() == 0)
        return false;

    bool start_set = false;
    float current_time = 0.0f;

    for (size_t i = 0; i < adxs.size(); i++)
    {
        AdxFile *const &adx = adxs[i];

        if (loopflg[i])
        {
            if (!start_set)
            {
                *start = current_time;
                start_set = true;
            }

            current_time += adx->GetDuration();
            *end = current_time;
        }
        else
        {
            if (start_set)
                break;

            current_time += adx->GetDuration();
        }
    }

    *count = 0;
    return start_set;
}

bool AaxFile::GetLoopSample(uint32_t *sample_start, uint32_t *sample_end, int *count) const
{
    if (adxs.size() == 0)
        return false;

    bool start_set = false;
    uint32_t current_sample = 0;

    for (size_t i = 0; i < adxs.size(); i++)
    {
        AdxFile *const &adx = adxs[i];

        if (loopflg[i])
        {
            if (!start_set)
            {
                *sample_start = current_sample;
                start_set = true;
            }

            current_sample += adx->GetNumSamples();
            *sample_end = current_sample;
        }
        else
        {
            if (start_set)
                break;

            current_sample += adx->GetNumSamples();
        }
    }

    *count = 0;
    return start_set;
}

bool AaxFile::SetLoopSample(uint32_t sample_start, uint32_t sample_end, int)
{
    if (sample_end > GetNumSamples())
        return false;

    if (sample_start >= sample_end)
        return false;

    RemoveLoop();

    uint32_t num_samples = GetNumSamples(); // num samples may have changed slightly after join!!

    // align params to block samples
    if (sample_start & 0x1F)
        sample_start += 0x20 - (sample_start & 0x1F);

    if (sample_end & 0x1F)
        sample_end += 0x20 - (sample_end & 0x1F);

    if (sample_end > num_samples)
        sample_end = num_samples;

    if (sample_start >= sample_end)
        return false;

    assert(adxs.size() == 1 && loopflg.size() == 1);
    AdxFile *adx = adxs[0];

    if (sample_start == 0)
    {
        if (sample_end == num_samples || sample_end == (num_samples-1))
        {
            SetMaxLoop();
            return true;
        }

        // File must be split in two, first loops
        AdxFile *adx1 = new AdxFile();
        AdxFile *adx2 = new AdxFile();

        if (!adx->Split(*adx1, *adx2, sample_end))
        {
            delete adx1;
            delete adx2;
            return false;
        }

        delete adx;
        adxs[0] = adx1;
        adxs.push_back(adx2);
        loopflg[0] = true;
        loopflg.push_back(false);
    }
    else
    {
        if (sample_end == num_samples || sample_end == (num_samples-1))
        {
            // File must be split in two, second loops
            AdxFile *adx1 = new AdxFile();
            AdxFile *adx2 = new AdxFile();

            if (!adx->Split(*adx1, *adx2, sample_start))
            {
                delete adx1;
                delete adx2;
                return false;
            }

            delete adx;
            adxs[0] = adx1;
            adxs.push_back(adx2);
            loopflg.push_back(true);
        }
        else
        {
            // File must be split in three, second loops
            AdxFile *adx1 = new AdxFile();
            AdxFile *adx2 = new AdxFile();

            if (!adx->Split(*adx1, *adx2, sample_start))
            {
                delete adx1;
                delete adx2;
                return false;
            }

            delete adx;

            AdxFile *adx22 = new AdxFile();
            AdxFile *adx3 = new AdxFile();

            if (!adx2->Split(*adx22, *adx3, sample_end-adx1->GetNumSamples()))
            {
                delete adx1;
                delete adx2;
                delete adx22;
                delete adx3;
                return false;
            }

            delete adx2;

            adxs[0] = adx1;
            adxs.push_back(adx22);
            adxs.push_back(adx3);
            loopflg.push_back(true);
            loopflg.push_back(false);
        }
    }

    return true;
}

void AaxFile::SetMaxLoop()
{
    JoinAll();
    loopflg[1] = true;
}

void AaxFile::RemoveLoop()
{
    JoinAll();
    loopflg[0] = false;
}

bool AaxFile::CheckHeader(const uint8_t *buf, size_t size)
{
    FixedMemoryStream stream(const_cast<uint8_t *>(buf), size);
    UTFHeader *hdr;
    UTFTableHeader *thdr;
    uint8_t *signature;

    if (!stream.FastRead((uint8_t **)&hdr, sizeof(UTFHeader)))
        return false;

    if (hdr->signature != UTF_SIGNATURE)
        return false;

    if (!stream.FastRead((uint8_t **)&thdr, sizeof(UTFTableHeader)))
        return false;

    if (!stream.Seek(be32(thdr->strings_offset)+be32(thdr->table_name)+sizeof(UTFHeader), SEEK_SET))
        return false;

    if (!stream.FastRead(&signature, 4))
        return false;

    return (memcmp(signature, "AAX", 4) == 0);
}




