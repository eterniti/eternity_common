#ifndef AWBCRIWAREAUDIOCONTAINER_H
#define AWBCRIWAREAUDIOCONTAINER_H

#include "CriwareAudioContainer.h"
#include "AwbFile.h"

class AwbCriwareAudioContainer : public CriwareAudioContainer
{
private:

    AwbFile *awb;
    mutable uint8_t *temp_mem;

public:

    AwbCriwareAudioContainer();
    virtual ~AwbCriwareAudioContainer() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual bool HasAwb() const override { return true; }
    virtual uint8_t *GetAwb(uint32_t *awb_size) const override;
    virtual bool SetAwb(void *awb, uint32_t awb_size, bool take_ownership=false) override;

    virtual const uint8_t *GetExternalAwbHash() const override { return nullptr; }
    virtual bool SetExternalAwbHash(uint8_t *) override { return false; }

    virtual bool HasAwbHeader() const override { return false; }
    virtual uint8_t *GetAwbHeader(uint32_t *) const override { return nullptr; }
    virtual bool SetAwbHeader(void *, uint32_t, bool) override { return false; }

    //
    virtual uint32_t GetNumTracks() const override
    {
        return (uint32_t)awb->GetNumFiles();
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
        if (!external)
            return (uint32_t)-1;

        return idx;
    }

    virtual uint32_t TrackIndexToAwbIndex(uint32_t idx, bool *external) const override
    {
        *external = false;
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
        if (!external)
            return (uint32_t)-1;

        return idx;
    }

    virtual uint32_t CueIndexToAwbIndex(uint32_t idx, bool *external) const override
    {
        *external = true;
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

    virtual uint32_t AwbIndexToCueId(uint32_t idx, bool external) const override
    {
        UNUSED(external);
        return idx;
    }

    virtual uint32_t CueIdToAwbIndex(uint32_t id, bool *external) const override
    {
        *external = true;
        return id;
    }

    virtual bool GetCueName(uint32_t cue_idx, std::string *name) const override
    {
        if (!awb || cue_idx >= awb->GetNumFiles())
            return false;

        *name = Utils::ToStringAndPad((int)cue_idx, 4);
        return true;
    }
};

#endif // AWBCRIWAREAUDIOCONTAINER_H
