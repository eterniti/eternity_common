#ifndef CV3FILE_H
#define CV3FILE_H

#include "WavFile.h"

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED CV3Header
{
    uint16_t type;  // 0 - same as in WAVHeader
    uint16_t num_channels; // 2
    uint32_t sample_rate; // 4
    uint32_t byte_rate; // 8
    uint16_t block_align; // 0xC
    uint16_t bit_depth; // 0xE
    uint16_t unk_10; // Always zero?
    uint32_t data_size;
};
CHECK_STRUCT_SIZE(CV3Header, 0x16);

class Cv3File : public AudioFile
{

private:

    CV3Header hdr;
    uint8_t *samples;
    size_t samples_size;


protected:
    void Reset();

public:
    Cv3File();
    virtual ~Cv3File();

    virtual bool Load(const uint8_t *buf, size_t size);
    virtual uint8_t *Save(size_t *psize);

    virtual uint16_t GetNumChannels() const { return hdr.num_channels; }
    virtual bool SetNumChannels(uint16_t num_channels) { hdr.num_channels = num_channels; return true; }

    virtual uint32_t GetNumSamples() const { return samples_size / GetSampleSize(); }

    virtual uint32_t GetSampleRate() const { return hdr.sample_rate; }
    virtual bool SetSampleRate(uint32_t sample_rate) { hdr.sample_rate = sample_rate; return true; }

    virtual float GetDuration() const
    {
        return (samples_size / GetSampleSize()) / (float)hdr.sample_rate;
    }

    virtual uint8_t *Decode(int *format, size_t *psize);
    virtual bool Encode(uint8_t *buf, size_t size, int format, uint16_t num_channels, uint32_t sample_rate, bool take_ownership=true);

    virtual bool HasLoop() const { return false; }

    virtual bool GetLoop(float *start, float *end, int *count) const { UNUSED(start); UNUSED(end); UNUSED(count); return false; }
    virtual bool SetLoop(float start, float end, int count) { UNUSED(start); UNUSED(end); UNUSED(count); return false; }

    virtual bool GetLoopSample(uint32_t *sample_start, uint32_t *sample_end, int *count) const { UNUSED(sample_start); UNUSED(sample_end); UNUSED(count); return false; }
    virtual bool SetLoopSample(uint32_t sample_start, uint32_t sample_end, int count) { UNUSED(sample_start); UNUSED(sample_end); UNUSED(count); return false; }

    virtual void SetMaxLoop() { }
    virtual void RemoveLoop() { }

    virtual bool FromFiles(const std::vector<AudioFile *> &files, int format, bool preserve_loop=true, int max_threads=0);
    virtual bool ToFiles(const std::vector<AudioFile *> &files, uint16_t split_channels, bool allow_silence, bool preserve_loop=true, int max_threads=0) const;

    inline uint16_t GetSampleSize() const
    {
        return hdr.num_channels * (hdr.bit_depth/8);
    }
};

#endif // CV3FILE_H
