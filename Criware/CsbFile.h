#ifndef __CSBFILE_H__
#define __CSBFILE_H__

#include "AcbFile.h"

class CsbFile : public AcbFile
{
private:

    int se_row;
    bool data_modified;

    struct TrackData
    {
        uint8_t nch;
        uint32_t sfreq;
        uint32_t nsmpl;
    };

    std::vector<TrackData> tracks_data;

protected:

    void ResetCSB();

public:

    CsbFile();
    virtual ~CsbFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    bool SetTrackData(size_t track, uint8_t num_channels, uint32_t sample_rate, uint32_t num_samples);

    virtual uint32_t GetNumTracks() const override
    {
        return (uint32_t)tracks_data.size();
    }

    virtual uint32_t GetFilesPerTrack() const override
    {
        return 1;
    }

    virtual bool CanAddTrack() const override
    {
        return false;
    }

    virtual uint32_t AwbIndexToTrackIndex(uint32_t idx, bool external) const override
    {
        UNUSED(external);
        return idx;
    }

    virtual uint32_t TrackIndexToAwbIndex(uint32_t idx, bool *external) const override
    {
        *external = !HasAwb();
        return idx;
    }

    /*virtual uint32_t TrackIndexToCueIndex(uint32_t idx) const override
    {
        return idx;
    }

    virtual size_t TrackIndexToCueIndexes(uint32_t idx, std::vector<uint32_t> &cue_idxs) const override
    {
        cue_idxs.resize(1);
        cue_idxs[0] = idx;
        return 1;
    }

    virtual uint32_t CueIndexToTrackIndex(uint32_t idx) const override
    {
        return idx;
    }

    virtual uint32_t AwbIndexToCueIndex(uint32_t idx, bool external) const override
    {
        UNUSED(external);
        return idx;
    }

    virtual uint32_t CueIndexToAwbIndex(uint32_t idx, bool *external) const override
    {
        *external = !HasAwb();
        return idx;
    }

    virtual uint32_t CueIndexToCueId(uint32_t idx) const override
    {
        return idx;
    }

    virtual uint32_t CueIdToCueIndex(uint32_t id) const override
    {
        return id;
    }

    virtual size_t CueIdToCueIndexes(uint32_t id, std::vector<uint32_t> &cue_idxs) const override
    {
        cue_idxs.resize(1);
        cue_idxs[0] = id;
        return 1;
    }*/
    virtual uint32_t TrackIndexToCueId(uint32_t idx) const override
    {
        return idx;
    }

    virtual size_t TrackIndexToCueIds(uint32_t idx, std::vector<uint32_t> &cue_ids) const override
    {
        cue_ids.resize(1);
        cue_ids[0] = idx;
        return 1;
    }

    virtual uint32_t CueIdToTrackIndex(uint32_t id) const override
    {
        return id;

    }

    virtual size_t CueIdToTrackIndexes(uint32_t id, std::vector<uint32_t> &idxs) const override
    {
        idxs.resize(1);
        idxs[0] = id;
        return 1;
    }

    virtual bool GetCueName(uint32_t cue_idx, std::string *name) const override
    {
        UNUSED(cue_idx); UNUSED(name);
        return false;
    }
};

#endif // __CSBFILE_H__
