#ifndef CWAVFILE_H
#define CWAVFILE_H

#include "AudioFile.h"

#define CWAV_SIGNATURE      0x56415743
#define CWAV_INFO_SIGNATURE 0x4F464E49
#define CWAV_DATA_SIGNATURE 0x41544144

enum CwavEncoding
{
    CWAV_ENCODING_PCM8,
    CWAV_ENCODING_PCM16,
    CWAV_ENCODING_DSPADPCM,
    CWAV_ENCODING_IMAADPCM
};

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct CWAVReference
{
    uint16_t id_type; // 0
    uint16_t unk_02; // Reserved/pad
    uint32_t offset; // 4
} PACKED;

STATIC_ASSERT_STRUCT(CWAVReference, 8);

struct CWAVSizeReference : public CWAVReference
{
    uint32_t size; // 8
} PACKED;

STATIC_ASSERT_STRUCT(CWAVSizeReference, 12);

struct CWAVHeader
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t header_size; // 6
    uint32_t version; // 8
    uint32_t file_size; // 0xC
    uint16_t num_blocks; // 0x10
    uint16_t unk_12; // Reserved/pad
    CWAVSizeReference info; // 0x14
    CWAVSizeReference data; // 0x20
} PACKED;

STATIC_ASSERT_STRUCT(CWAVHeader, 0x2C);

struct CWAVInfoHeader
{
    uint32_t signature; // 0
    uint32_t size; // 4
    uint8_t encoding; // 8
    uint8_t loop; // 9
    uint16_t unk_0A; // Reserved/pad
    uint32_t sample_rate; // 0xC
    uint32_t loop_start; // 0x10
    uint32_t loop_end; // 0x14
    uint32_t unk_18; // zero
    uint32_t num_channels; // 0x1C
} PACKED;

STATIC_ASSERT_STRUCT(CWAVInfoHeader, 0x20);

struct CWAVChannelInfo
{
    CWAVReference sample; // 0
    CWAVReference codec; // 8
    uint32_t unk_10;
} PACKED;

STATIC_ASSERT_STRUCT(CWAVChannelInfo, 0x14);

struct CWAVDSPadPcmInfo
{
    int16_t coef[16]; // 0
    uint16_t scale; // 0x20
    int16_t yn1; // 0x22
    int16_t yn2; // 0x24
    uint16_t loopscale; // 0x26
    uint16_t loopyn1; // 0x28
    uint16_t loopyn2; // 0x2A
} PACKED;

STATIC_ASSERT_STRUCT(CWAVDSPadPcmInfo, 0x2C);

struct CWAVDataHeader
{
    uint32_t signature; // 0
    uint32_t size; // 4
} PACKED;

STATIC_ASSERT_STRUCT(CWAVDataHeader, 8);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct CwavDSPadPcmState
{
    int16_t yn1;
    int16_t yn2;
    uint32_t sample_offset;
};

struct CwavChannel
{
    uint32_t start;
    CWAVDSPadPcmInfo dspadpcm_info;
    CwavDSPadPcmState dsapdcm_state;
};

class CwavFile : public AudioFile
{
private:

    bool has_loop;
    uint32_t sample_rate;
    uint32_t loop_start;
    uint32_t loop_end;
    uint32_t num_samples;

    std::vector<CwavChannel> channels;
    std::vector<uint8_t> raw_data;

    void DecodeBlock(CwavChannel &channel, int16_t *out, int max);

protected:

    void Reset();

public:

    CwavFile();
    virtual ~CwavFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;

    virtual uint16_t GetNumChannels() const override { return (uint16_t)channels.size(); }
    virtual bool SetNumChannels(uint16_t num_channels) override{ UNUSED(num_channels); return false; }

    virtual uint32_t GetNumSamples() const override { return 0; }

    virtual uint32_t GetSampleRate() const override { return sample_rate; }
    virtual bool SetSampleRate(uint32_t sample_rate) override { UNUSED(sample_rate); return false; }

    virtual float GetDuration() const override
    {
        return (float)num_samples / (float)sample_rate;
    }

    virtual uint8_t *Decode(int *format, size_t *psize) override;
    virtual bool Encode(uint8_t *buf, size_t size, int format, uint16_t num_channels, uint32_t sample_rate, bool take_ownership=true) override
    {
        UNUSED(buf); UNUSED(size); UNUSED(format);
        UNUSED(num_channels); UNUSED(sample_rate); UNUSED(take_ownership);
        return false;
    }

    virtual bool HasLoop() const override { return has_loop; }

    virtual bool GetLoop(float *start, float *end, int *count) const override
    {
        if (!has_loop)
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
        if (!has_loop)
            return false;

        *sample_start = loop_start;
        *sample_end = loop_end;
        *count = 0;
        return true;
    }

    virtual bool SetLoopSample(uint32_t sample_start, uint32_t sample_end, int count) override
    {
        if (sample_start > sample_end || sample_end > num_samples)
            return false;

        loop_start = sample_start;
        loop_end = sample_end;
        UNUSED(count);

        has_loop = true;
        return true;
    }

    virtual void SetMaxLoop() override { SetLoopSample(0, num_samples, 0); }
    virtual void RemoveLoop() override { has_loop = false; }

    // Unimplemented
    virtual bool FromFiles(const std::vector<AudioFile *> &, int , bool, int) { return false; }
    virtual bool ToFiles(const std::vector<AudioFile *> &, uint16_t, bool, bool, int) const { return false; }

    bool DecodeToWav(const std::string &file, bool preserve_loop=true, int format=AUDIO_FORMAT_16BITS);

    static bool CheckHeader(const uint8_t *buf, size_t size);
};

#endif // CWAVFILE_H
