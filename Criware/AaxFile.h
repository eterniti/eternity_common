#ifndef __AAXFILE_H__
#define __AAXFILE_H__

#include "AdxFile.h"
#include "UtfFile.h"

class AaxFile : public AudioFile, protected UtfFile
{
private:

    std::vector<AdxFile *> adxs;
    std::vector<uint8_t> loopflg;

    void JoinAll();

protected:

    void ResetAAX(bool load_dummy=true);

public:

    AaxFile();
    virtual ~AaxFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual bool LoadFromFile(const std::string &path, bool show_error=true) override;
    virtual bool SaveToFile(const std::string &path, bool show_error=true, bool build_path=false) override;

    virtual uint16_t GetNumChannels() const override
    {
        return (adxs.size() == 0) ? 0 : adxs[0]->GetNumChannels();
    }
    virtual bool SetNumChannels(uint16_t num_channels) override
    {
        if (adxs.size() == 0)
            return false;

        for (AdxFile *&adx : adxs)
            if (!adx->SetNumChannels(num_channels))
                return false;

        return true;
    }

    virtual uint32_t GetNumSamples() const override
    {
        uint32_t num_samples = 0;

        for (auto &adx : adxs)
            num_samples += adx->GetNumSamples();

        return num_samples;
    }

    virtual uint32_t GetSampleRate() const override
    {
        return (adxs.size() == 0) ? 1 : adxs[0]->GetSampleRate();
    }

    virtual bool SetSampleRate(uint32_t sample_rate) override
    {
        for (AdxFile *&adx : adxs)
            if (!adx->SetSampleRate(sample_rate))
                return false;

        return true;
    }

    virtual float GetDuration() const override
    {
        return (float)GetNumSamples() / (float)GetSampleRate();
    }

    virtual uint8_t *Decode(int *format, size_t *psize) override;
    virtual bool Encode(uint8_t *buf, size_t size, int format, uint16_t num_channels, uint32_t sample_rate, bool take_ownership=true) override;

    virtual bool HasLoop() const override
    {
        if (adxs.size() == 0)
            return false;

        for (uint8_t flg : loopflg)
            if (flg)
                return true;

        return false;
    }
    virtual bool GetLoop(float *start, float *end, int *count) const override;
    virtual bool SetLoop(float start, float end, int count) override
    {
        return SetLoopSample((uint32_t)(start*(float)GetSampleRate()), (uint32_t)(end*(float)GetSampleRate()), count);
    }

    virtual bool GetLoopSample(uint32_t *sample_start, uint32_t *sample_end, int *count) const override;
    virtual bool SetLoopSample(uint32_t sample_start, uint32_t sample_end, int count) override;

    virtual void SetMaxLoop() override;
    virtual void RemoveLoop() override;

    // Unimplemented
    virtual bool FromFiles(const std::vector<AudioFile *> &, int , bool, int) { return false; }
    virtual bool ToFiles(const std::vector<AudioFile *> &, uint16_t, bool, bool, int) const { return false; }

    bool DecodeToWav(const std::string &file, bool preserve_loop=true, int format=AUDIO_FORMAT_16BITS);
    bool EncodeFromWav(const std::string &file, bool move_to_memory=false, bool preserve_loop=true);

    static bool CheckHeader(const uint8_t *buf, size_t size);
};

#endif // __AAXFILE_H__
