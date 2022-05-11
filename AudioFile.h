#ifndef __AUDIOFILE_H__
#define __AUDIOFILE_H__

#include "BaseFile.h"

enum
{
    AUDIO_FORMAT_FLOAT,
    AUDIO_FORMAT_8BITS = 8,
    AUDIO_FORMAT_16BITS = 16,
    AUDIO_FORMAT_24BITS = 24,
    AUDIO_FORMAT_32BITS = 32
};

class AudioFile : public BaseFile
{
protected:

    enum class State
    {
        STOPPED,
        PLAYING,
        PAUSED
    } state = State::STOPPED;

public:

    virtual ~AudioFile()
    {
    }

    virtual uint16_t GetNumChannels() const = 0;
    virtual bool SetNumChannels(uint16_t num_channels) = 0;

    virtual uint32_t GetNumSamples() const = 0;

    virtual uint32_t GetSampleRate() const = 0;
    virtual bool SetSampleRate(uint32_t sample_rate) = 0;

    virtual float GetDuration() const = 0;

    virtual uint8_t *Decode(int *format, size_t *psize) = 0;
    virtual bool Encode(uint8_t *buf, size_t size, int format, uint16_t num_channels, uint32_t sample_rate, bool take_ownership=true) = 0;

    virtual bool HasLoop() const = 0;

    virtual bool GetLoop(float *start, float *end, int *count) const = 0;
    virtual bool SetLoop(float start, float end, int count) = 0;

    virtual bool GetLoopSample(uint32_t *sample_start, uint32_t *sample_end, int *count) const = 0;
    virtual bool SetLoopSample(uint32_t sample_start, uint32_t sample_end, int count) = 0;

    virtual void SetMaxLoop() = 0;
    virtual void RemoveLoop() = 0;

    virtual bool FromFiles(const std::vector<AudioFile *> &files, int format, bool preserve_loop=true, int max_threads=0) = 0;
    virtual bool ToFiles(const std::vector<AudioFile *> &files, uint16_t split_channels, bool allow_silence, bool preserve_loop=true, int max_threads=0) const = 0;
};

#include "Thread.h"

class MultipleAudioDecoder : public Runnable
{
private:
    AudioFile *file;
    uint8_t *buf;
    int index;
    int total;
    int format;
    bool *error;

public:

    MultipleAudioDecoder(AudioFile *file, uint8_t *buf, int index, int total, int format, bool *error) :
        file(file), buf(buf), index(index), total(total), format(format), error(error)
    {
    }

    ~MultipleAudioDecoder()
    {
    }

    virtual uint32_t Run() override
    {
        if (*error)
            return -1;

        uint32_t num_samples = file->GetNumSamples();
        uint16_t num_channels = file->GetNumChannels();

        size_t total_write_size = num_channels*num_samples*total;
        size_t advance_size = total*num_channels;
        size_t input_size;
        int input_format;

        if (format == 0 || format == 32)
        {
            total_write_size *= 4;
            advance_size *= 4;
        }
        else if (format == 16)
        {
            total_write_size *= 2;
            advance_size *= 2;
        }
        else if (format == 24)
        {
            total_write_size *= 3;
            advance_size *= 3;
        }

        uint8_t *input_buf = file->Decode(&input_format, &input_size);
        if (!input_buf)
        {
            *error = true;
            return -1;
        }

        if (input_format != 0 && input_format != 16)
        {
            //DPRINTF("%s: FIXME, this input_format %d not supported.\n", input_format);
            delete[] input_buf;
            return -1;
        }

        uint8_t *ptr_out = buf;
        uint8_t *ptr_in = input_buf;
        uint8_t *bottom = ptr_out + total_write_size;

        while (ptr_out < bottom && !(*error))
        {
            for (uint16_t ch = 0; ch < num_channels; ch++)
            {
                if (format == 0)
                {
                    float *out = ((float *)ptr_out) + index*num_channels+ch;

                    if (input_format == 0)
                    {
                        float *in = (float *)ptr_in;
                        *out = *in;
                        ptr_in += sizeof(float);
                    }
                    else // 16
                    {
                        int16_t *in = (int16_t *)ptr_in;
                        *out = ((float)*in) / (float)32768;
                        ptr_in += sizeof(int16_t);
                    }
                }
                else if (format == 8)
                {
                    uint8_t *out = ptr_out + index*num_channels + ch;

                    if (input_format == 0)
                    {
                        float *in = (float *)ptr_in;
                        *out = (int)((double)*in * 0x7F) + 0x80;
                        ptr_in += sizeof(float);
                    }
                    else // 16
                    {
                        int16_t *in = (int16_t *)ptr_in;
                        *out = *in>>8;
                        ptr_in += sizeof(int16_t);
                    }
                }
                else if (format == 16)
                {
                    int16_t *out = ((int16_t *)ptr_out) + index*num_channels + ch;

                    if (input_format == 0)
                    {
                        float *in = (float *)ptr_in;
                        *out = (int)((double)*in * 0x7FFF);
                        ptr_in += sizeof(float);
                    }
                    else // 16
                    {
                        int16_t *in = (int16_t *)ptr_in;
                        *out = *in;
                        ptr_in += sizeof(int16_t);
                    }
                }
                else if (format == 24)
                {
                    uint8_t *out = ptr_out + (index*num_channels+ch)*3;
                    int val;

                    if (input_format == 0)
                    {
                        float *in = (float *)ptr_in;
                        val = (int)((double)*in * 0x7FFFFF);
                        ptr_in += sizeof(float);
                    }
                    else // 16
                    {
                        int16_t *in = (int16_t *)ptr_in;
                        val = *in<<8;
                        ptr_in += sizeof(int16_t);
                    }

                    memcpy(out, &val, 3);
                }
                else if (format == 32)
                {
                    int32_t *out = ((int32_t *)ptr_out) + index*num_channels + ch;

                    if (input_format == 0)
                    {
                        float *in = (float *)ptr_in;
                        *out = (int)(*in * 0x7FFFFFFF);
                        ptr_in += sizeof(float);
                    }
                    else // 16
                    {
                        int16_t *in = (int16_t *)ptr_in;
                        *out = *in<<16;
                        ptr_in += sizeof(int16_t);
                    }
                }
            }

            ptr_out += advance_size;
        }

        delete[] input_buf;
        return 0;
    }

};

class MultipleAudioEncoder : public Runnable
{
private:
    AudioFile *file;
    const uint8_t *buf;
    int index;
    int format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t num_samples;
    uint16_t input_channels;
    bool *error;

public:

    MultipleAudioEncoder(AudioFile *file, const uint8_t *buf, int index, int format, uint16_t num_channels, uint32_t sample_rate,
                    uint32_t num_samples, uint16_t input_channels, bool *error) :file(file), buf(buf), index(index), format(format),
                    num_channels(num_channels), sample_rate(sample_rate), num_samples(num_samples), input_channels(input_channels), error(error)
    {
    }

    ~MultipleAudioEncoder()
    {
    }

    virtual uint32_t Run() override
    {
        if (*error)
            return -1;

        size_t in_size = num_samples*num_channels;
        size_t sample_size = num_channels;
        size_t advance_size = input_channels;

        if (format == 0 || format == 32)
        {
            in_size *= 4;
            sample_size *= 4;
            advance_size *= 4;
        }
        else if (format == 16)
        {
            in_size *= 2;
            sample_size *= 2;
            advance_size *= 2;
        }
        else if (format == 24)
        {
            in_size *= 3;
            sample_size *= 3;
            advance_size *= 3;
        }

        uint8_t *in = (sample_size == advance_size && buf) ? nullptr :  new uint8_t[in_size];

        if (!buf)
        {
            memset(in, 0, in_size);
        }
        else if (sample_size == advance_size)
        {
            if (index != 0)
            {
                //DPRINTF("%s: Index should be zero..\n", FUNCNAME);
            }

            if (!file->Encode(const_cast<uint8_t *>(buf), in_size, format, num_channels, sample_rate, false))
            {
                *error = true;
                return -1;
            }

            return 0;
        }
        else
        {
            uint8_t *ptr = in;
            const uint8_t *ptr_wav = buf + index*sample_size;

            for (uint32_t i = 0; i < num_samples; i++)
            {
                if (*error)
                    return -1;

                memcpy(ptr, ptr_wav, sample_size);
                ptr += sample_size;
                ptr_wav += advance_size;
            }
        }

        if (*error)
            return -1;

        if (!file->Encode(in, in_size, format, num_channels, sample_rate))
        {
            *error = true;
            return -1;
        }

        return 0;
    }
};

#endif

