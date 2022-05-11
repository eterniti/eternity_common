#include "Cv3File.h"

Cv3File::Cv3File()
{
    this->big_endian = false;
    this->samples = nullptr;
    Reset();
}

Cv3File::~Cv3File()
{
    Reset();
}

void Cv3File::Reset()
{
    if (samples)
    {
        delete[] samples;
        samples = nullptr;
    }

    samples_size = 0;

    hdr.type = 1;
    hdr.num_channels = 1;
    hdr.sample_rate = 44100;
    hdr.byte_rate = 88200;
    hdr.block_align = 2;
    hdr.bit_depth = 16;
    hdr.unk_10 = 0;
    hdr.data_size = 0;
}

bool Cv3File::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (size < sizeof(CV3Header))
        return false;

    memcpy(&hdr, buf, sizeof(hdr));

    samples_size = size - sizeof(CV3Header);

    if (hdr.data_size > samples_size)
        return false;

    samples_size = hdr.data_size;

    samples = new uint8_t[samples_size];
    memcpy(samples, buf + sizeof(CV3Header), samples_size);

    return true;
}

uint8_t *Cv3File::Save(size_t *psize)
{
    if (!samples || samples_size == 0)
        return nullptr;

    *psize = samples_size + sizeof(CV3Header);
    uint8_t *ret = new uint8_t[*psize];
    memcpy(ret, &hdr, sizeof(CV3Header));
    memcpy(ret + sizeof(CV3Header), samples, samples_size);

    return ret;
}

uint8_t *Cv3File::Decode(int *format, size_t *psize){

    if (samples_size == 0)
        return nullptr;

    if (hdr.type == WAV_PCM)
    {
        *format = hdr.bit_depth;
    }
    else
    {
        *format = AUDIO_FORMAT_FLOAT;
    }

    uint8_t *buf = new uint8_t[samples_size];
    memcpy(buf, samples, samples_size);

    *psize = samples_size;
    return buf;
}

bool Cv3File::Encode(uint8_t *buf, size_t size, int format, uint16_t num_channels, uint32_t sample_rate, bool take_ownership)
{
    if (take_ownership)
    {
        if (samples)
            delete[] samples;

        samples = buf;
        samples_size = size;
    }
    else
    {
        if (samples && samples_size >= size)
        {
            memcpy(samples, buf, size);
            samples_size = size;
        }
        else
        {
            if (samples)
                delete[] samples;

            samples = new uint8_t[size];
            memcpy(samples, buf, size);
            samples_size = size;
        }
    }

    hdr.type = (format == AUDIO_FORMAT_FLOAT) ? 3 : 1;
    hdr.num_channels = num_channels;
    hdr.sample_rate = sample_rate;
    hdr.bit_depth = (format == AUDIO_FORMAT_FLOAT) ? 32 : (uint16_t)format;
    hdr.block_align = num_channels * (hdr.bit_depth/8);
    hdr.byte_rate = sample_rate * hdr.block_align;
    hdr.unk_10 = 0;
    hdr.data_size = size;

    return true;
}

bool Cv3File::FromFiles(const std::vector<AudioFile *> &files, int format, bool preserve_loop, int max_threads)

{
    UNUSED(files); UNUSED(format); UNUSED(preserve_loop); UNUSED(max_threads);
    // Unimplemented
    return false;
}

bool Cv3File::ToFiles(const std::vector<AudioFile *> &files, uint16_t split_channels, bool allow_silence, bool preserve_loop, int max_threads) const
{
    uint16_t total_channels = (uint16_t)files.size() * split_channels;

    if (total_channels > hdr.num_channels && !allow_silence)
        return false;

    if (total_channels < hdr.num_channels)
        return false;

    if ((hdr.num_channels % split_channels) != 0)
        return false;

    if (files.size() == 0)
        return true;

    if (max_threads <= 0)
    {
        int cores_count = Thread::LogicalCoresCount();
        max_threads = (files.size() > (unsigned)cores_count) ? cores_count : (int)files.size();
    }

    ThreadPool pool(max_threads);
    bool error = false;


    int format = (hdr.type == 1) ? hdr.bit_depth : (int)AUDIO_FORMAT_FLOAT;

    for (uint16_t i = 0; i < (uint16_t)files.size(); i++)
    {
        pool.AddWork(new MultipleAudioEncoder(files[i], i*split_channels >= hdr.num_channels ? nullptr : samples, i, format,
                                         split_channels, hdr.sample_rate, GetNumSamples(), hdr.num_channels, &error));
    }

    pool.Wait();

    if (preserve_loop && HasLoop() && !error)
    {
        uint32_t loop_start, loop_end;
        int loop_count;

        if (GetLoopSample(&loop_start, &loop_end, &loop_count))
        {
            for (AudioFile * const &file : files)
            {
                file->SetLoopSample(loop_start, loop_end, loop_count);
            }
        }
    }

    return !error;
}

