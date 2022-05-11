#ifndef FFMPEGAUDIOFILE_H
#define FFMPEGAUDIOFILE_H

#include "AudioFile.h"
#include "MemoryStream.h"

extern "C"
{
#include <libavutil/avutil.h>
#include <libavutil/frame.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

class FfmpegAudioFile : public AudioFile
{
private:

    uint16_t num_channels;
    uint32_t num_samples;
    uint32_t sample_rate;
    int format;

    MemoryStream *decoded;

protected:

    void Reset();

public:

    FfmpegAudioFile();
    virtual ~FfmpegAudioFile() override;

    virtual bool LoadFromFile(const std::string &path, bool show_error=true) override;

    virtual uint16_t GetNumChannels() const override { return num_channels; }
    // Unimplemented
    virtual bool SetNumChannels(uint16_t) override { return false; }

    virtual uint32_t GetNumSamples() const override { return num_samples; }

    virtual uint32_t GetSampleRate() const override { return sample_rate; }
    //Unimplemented
    virtual bool SetSampleRate(uint32_t) override { return false; }

    virtual float GetDuration() const override
    {
        return (float)num_samples / (float)sample_rate;
    }

    virtual uint8_t *Decode(int *format, size_t *psize) override;

    virtual bool Encode(uint8_t *buf, size_t size, int format, uint16_t num_channels, uint32_t sample_rate, bool take_ownership=true) override;

    // Loop unimplemented
    virtual bool HasLoop() const override { return false; }

    virtual bool GetLoop(float *, float *, int *) const override { return false; }
    virtual bool SetLoop(float, float, int) override { return false; }

    virtual bool GetLoopSample(uint32_t *, uint32_t *, int *) const override { return false; }
    virtual bool SetLoopSample(uint32_t, uint32_t, int) override { return false; }

    virtual void SetMaxLoop() override { }
    virtual void RemoveLoop() override { }

    virtual bool FromFiles(const std::vector<AudioFile *> &, int , bool, int) { return false; }
    virtual bool ToFiles(const std::vector<AudioFile *> &files, uint16_t split_channels, bool allow_silence, bool preserve_loop=true, int max_threads=0) const;

    static bool CheckHeader(const std::string &path, uint16_t *num_channels=nullptr, uint32_t *sample_rate=nullptr);
};

#endif // FFMPEGAUDIOFILE_H
