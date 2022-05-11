#ifndef __ADXFILE_H__
#define __ADXFILE_H__

#include "AudioFile.h"

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    uint16_t signature; // 0
    uint16_t copyright_offset; // 2
    uint8_t encoding_type; // 4
    uint8_t block_size; // 5
    uint8_t bit_depth; // 6
    uint8_t num_channels; // 7
    uint32_t sample_rate; // 8
    uint32_t num_samples; // 0xC
    uint16_t highpass_frequency; // 0x10
    uint8_t version;  // 0x12
    uint8_t flags; // 0x13
} PACKED ADXHeader;

STATIC_ASSERT_STRUCT(ADXHeader, 0x14);

typedef struct
{
    uint32_t unk_00; // 0               +0x14
    uint32_t has_loop; // 4             +0x18
    uint32_t loop_start; // 8           +0x1C
    uint32_t loop_start_offset; // C    +0x20   (absolute offset in file)
    uint32_t loop_end; // 0x10          +0x24
    uint32_t loop_end_offset; // 0x14   +0x28   (absolute offset in file)
} PACKED ADXLoop;

STATIC_ASSERT_STRUCT(ADXLoop, 0x18);

typedef struct
{
    uint32_t unk_00; // 0       +0x14
    uint32_t unk_04; // 4       +0x18
    uint32_t unk_08; // 8       +0x1C
} PACKED ADXV4Unk;

STATIC_ASSERT_STRUCT(ADXV4Unk, 0xC);

typedef struct
{
    uint16_t signature; // 0
    uint16_t pad_count;
} PACKED ADXFooter;

STATIC_ASSERT_STRUCT(ADXFooter, 4);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

class AdxFile : public AudioFile
{
private:

    uint8_t num_channels;
    uint32_t sample_rate;
    uint32_t num_samples;
    uint16_t highpass_frequency;
    uint8_t version;

    uint32_t unk1;
    uint32_t v4_unk2;
    uint32_t v4_unk3;
    uint32_t v4_unk4;

    bool has_extra_header;
    bool has_extra_header_v4;
    bool has_loop_data;

    bool loop;
    uint32_t loop_start;
    uint32_t loop_end;

    uint32_t pad_start;
    uint16_t pad_final;
    uint8_t *raw_data;

    double c1, c2;
    int16_t c1_16, c2_16;

private:

    uint32_t SamplesToBytes(uint32_t samples);
    void CalculateCoeff();
    uint32_t CalculateHeaderSize() const;

    void DecodeBlock(const uint8_t *in, int16_t *out, int &s1, int &s2);
    void EncodeBlock(const int16_t *in, uint8_t *out, int16_t &s1, int16_t &s2);

protected:

    void Reset();

public:

    AdxFile();
    virtual ~AdxFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual uint16_t GetNumChannels() const override { return num_channels; }
    virtual bool SetNumChannels(uint16_t num_channels) override
    {
        if (num_channels > 2 || num_channels == 0)
            return false;

        this->num_channels = (uint8_t)num_channels;
        return true;
    }

    virtual uint32_t GetNumSamples() const override { return num_samples; }

    virtual uint32_t GetSampleRate() const override { return sample_rate; }
    virtual bool SetSampleRate(uint32_t sample_rate) override
    {
        this->sample_rate = sample_rate;
        return true;
    }

    virtual float GetDuration() const override
    {
        return (float)num_samples / (float)sample_rate;
    }

    virtual uint8_t *Decode(int *format, size_t *psize) override;
    virtual bool Encode(uint8_t *buf, size_t size, int format, uint16_t num_channels, uint32_t sample_rate, bool take_ownership=true) override;

    virtual bool HasLoop() const override { return loop; }
    virtual bool GetLoop(float *start, float *end, int *count) const override
    {
        if (!loop)
            return false;

        *start = (float)loop_start/(float)sample_rate;
        *end = (float)loop_end/(float)sample_rate;
        *count = 0;
        return true;
    }

    virtual bool SetLoop(float start, float end, int count) override
    {
        return SetLoopSample((uint32_t)(start*(float)sample_rate), (uint32_t)(end*(float)sample_rate), count);
    }

    virtual bool GetLoopSample(uint32_t *sample_start, uint32_t *sample_end, int *count) const override
    {
        if (!loop)
            return false;

        *sample_start = loop_start;
        *sample_end = loop_end;
        *count = 0;
        return true;
    }

    virtual bool SetLoopSample(uint32_t sample_start, uint32_t sample_end, int count) override;

    virtual void SetMaxLoop() override { SetLoopSample(0, num_samples, 0); }
    virtual void RemoveLoop() override;     

    bool DecodeToWav(const std::string &file, bool preserve_loop=true, int format=AUDIO_FORMAT_16BITS);
    bool EncodeFromWav(const std::string &file, bool move_to_memory=false, bool preserve_loop=true);

    // Unimplemented
    virtual bool FromFiles(const std::vector<AudioFile *> &, int , bool, int) { return false; }
    virtual bool ToFiles(const std::vector<AudioFile *> &, uint16_t, bool, bool, int) const { return false; }

    bool Concat(const AdxFile &other, bool keep_loop);
    bool Split(AdxFile &other1, AdxFile &other2, uint32_t split_sample);

    static bool CheckHeader(const uint8_t *buf, size_t size, uint32_t *num_channels=nullptr, uint32_t *sample_rate=nullptr, uint32_t *num_samples=nullptr);
    static bool CheckHeader(const std::string &file, uint32_t *num_channels=nullptr, uint32_t *sample_rate=nullptr, uint32_t *num_samples=nullptr);

};

#endif // __ADXFILE_H__
