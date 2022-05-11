#ifndef __WAVFILE_H__
#define __WAVFILE_H__

// This is WIP. Currently it is more apropiated for use to write rather than reading, as there are wav features not implemented/tested.

#include "AudioFile.h"
#include "Stream.h"

#define RIFF_SIGNATURE	0x46464952 
#define WAV_SIGNATURE	0x20746D6645564157ULL
#define SMPL_SIGNATURE	0x6C706D73
#define DATA_SIGNATURE	0x61746164

enum WavFormat
{
    WAV_PCM = 1,
    WAV_ADPCM,
    WAV_PCM_FLOAT,
};

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
	uint32_t signature; // 0
	uint32_t chunk_size; // 4
} RIFFHeader;

STATIC_ASSERT_STRUCT(RIFFHeader,  8);

typedef struct
{
	uint64_t signature; // +8  Joined the WAVE + fmt here
	uint32_t fmt_size;  // +0x10
	uint16_t type;  // +0x14
	uint16_t num_channels; // +0x16
	uint32_t sample_rate; // +0x18
	uint32_t byte_rate; // +0x1C
	uint16_t block_align; // +0x20
	uint16_t bit_depth; // 0x22
} PACKED WAVHeader;

STATIC_ASSERT_STRUCT(WAVHeader,  0x1C);

typedef struct
{
    uint64_t signature; // +8  Joined the WAVE + fmt here
    uint32_t fmt_size;  // +0x10
    uint16_t type;  // +0x14
    uint16_t num_channels; // +0x16
    uint32_t sample_rate; // +0x18
    uint32_t byte_rate; // +0x1C
    uint16_t block_align; // +0x20
    uint16_t bit_depth; // +0x22
    uint16_t valid_bits_sample; // +0x24
    uint32_t channel_mask; // +0x26
    uint16_t padd; // +0x2A
    uint8_t sub_format[0x10]; // 0x2C

} PACKED WAVExtensibleHeader;

STATIC_ASSERT_STRUCT(WAVExtensibleHeader, 0x34);

typedef struct
{
	uint32_t signature; // 0
	uint32_t smpl_size; // 4
	uint32_t manufacturer; // 8
	uint32_t product; // 0xC
	uint32_t sample_period; // 0x10
	uint32_t midi_unity_note; // 0x14
	uint32_t midi_pitch_fraction; // 0x18
	uint32_t smpte_format; // 0x1C
	uint32_t smpte_offset; // 0x20
	uint32_t num_sample_loops; // 0x24
	uint32_t sampler_size; // 0x28
} PACKED SMPLHeader;

STATIC_ASSERT_STRUCT(SMPLHeader, 0x2C);

typedef struct
{
	uint32_t cue_point_id; // +0x2C
	uint32_t type; // +0x30
	uint32_t loop_start; // +0x34
	uint32_t loop_end; // +0x38
	uint32_t fraction; // +0x3C
	uint32_t play_count; // +0x40
} PACKED SMPLLoop;

STATIC_ASSERT_STRUCT(SMPLLoop, 0x18);

#ifdef _MSC_VER
#pragma pack(pop)
#endif


class WavFile : public AudioFile
{
private:

    mutable Stream *samples;
	
	uint16_t format;	
	uint16_t num_channels;
	uint32_t sample_rate;
	uint16_t bit_depth;
    uint16_t block_align;
	
	uint32_t loop_start;
	uint32_t loop_end;
	uint32_t loop_play_count;

    uint32_t CalculateFileSize();
	
protected:

	void Reset();
    void CreateHeader(uint8_t *header, uint32_t file_size, uint32_t samples_size=0);
	
public:

	WavFile();
    virtual ~WavFile() override;
	
    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual bool LoadFromFile(const std::string &path, bool show_error=true) override;
    // TODO: implement a custom version of SaveToFile, currently using the memory hungry one from BaseFile

    uint8_t *CreateHeader(size_t *psize, uint32_t samples_size=0);
	
	inline uint16_t GetFormat() const { return format; }
	inline bool SetFormat(uint16_t format)
	{
		if (format != 1 && format != 3)
			return false;
		
		this->format = format;
		return true;
	}
	
	virtual uint16_t GetNumChannels() const override { return num_channels; }
	virtual bool SetNumChannels(uint16_t num_channels) override { this->num_channels = num_channels; return true; }

    virtual uint32_t GetNumSamples() const override
    {
        if (format == WAV_ADPCM)
        {
            return (uint32_t)((samples->GetSize() * 2) / num_channels);
        }

        return ((uint32_t)samples->GetSize() / GetSampleSize());
    }
	
	virtual uint32_t GetSampleRate() const override { return sample_rate; }
	virtual bool SetSampleRate(uint32_t sample_rate) override { this->sample_rate = sample_rate; return true; }

    virtual float GetDuration() const override
    {
        return (samples->GetSize() / GetSampleSize()) / (float)sample_rate;
    }

    virtual uint8_t *Decode(int *format, size_t *psize) override;
    virtual bool Encode(uint8_t *buf, size_t size, int format, uint16_t num_channels, uint32_t sample_rate, bool take_ownership=true) override;

    virtual bool HasLoop() const override { return (loop_start != loop_end); }

    virtual bool GetLoop(float *start, float *end, int *count) const override
    {
        if (loop_start == loop_end)
            return false;

        *start = (float)loop_start/(float)sample_rate;
        *end = (float)loop_end/(float)sample_rate;
        *count = loop_play_count;

        return true;
    }

    virtual bool SetLoop(float start, float end, int count) override;

    virtual bool GetLoopSample(uint32_t *sample_start, uint32_t *sample_end, int *count) const override
    {
        if (loop_start == loop_end)
            return false;

        *sample_start = loop_start;
        *sample_end = loop_end;
        *count = loop_play_count;

        return true;
    }

    virtual bool SetLoopSample(uint32_t sample_start, uint32_t sample_end, int count) override
    {
        if (sample_start >= sample_end)
            return false;

        loop_start = sample_start; loop_end = sample_end; loop_play_count = count;
        return true;
    }

    virtual void SetMaxLoop() override
    {
        loop_start = 0;
        loop_end = GetNumSamples();
        loop_play_count = 0;
    }

    virtual void RemoveLoop() override
    {
        loop_start = loop_end = 0;
        loop_play_count = 0;
    }
	
	inline uint16_t GetBitDepth() const { return bit_depth; }
    inline void SetBitDepth(uint16_t bit_depth) { this->bit_depth = bit_depth; }
	
	inline uint16_t GetSampleSize() const
	{
		return num_channels * (bit_depth/8);
    }

    inline uint16_t GetBlockAlign() const { return block_align; }
	
    //inline bool WriteData(const void *data, size_t size) { return samples->Write((const uint8_t *)data, size); }
    inline Stream *GetSamplesStream() { return samples; }
    bool MoveToMemory() const;

    virtual bool FromFiles(const std::vector<AudioFile *> &files, int format, bool preserve_loop=true, int max_threads=0) override;
    virtual bool ToFiles(const std::vector<AudioFile *> &files, uint16_t split_channels, bool allow_silence, bool preserve_loop=true, int max_threads=0) const override;
};

#endif /* __WAVFILE_H__ */
