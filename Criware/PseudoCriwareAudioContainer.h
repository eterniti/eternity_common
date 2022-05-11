#ifndef PSEUDOCRIWAREAUDIOCONTAINER_H
#define PSEUDOCRIWAREAUDIOCONTAINER_H

#include "CriwareAudioContainer.h"

class PseudoCriwareAudioContainer : public CriwareAudioContainer
{
private:

    std::vector<std::string> files_list;

public:
    PseudoCriwareAudioContainer(const std::vector<std::string> &files_list) : files_list(files_list) { }
    virtual ~PseudoCriwareAudioContainer() override { }

    virtual bool HasAwb() const override { return false; }
    virtual uint8_t *GetAwb(uint32_t *) const override { return nullptr; }
    virtual bool SetAwb(void *, uint32_t, bool) override { return false; }

    virtual const uint8_t *GetExternalAwbHash() const override { return nullptr; }
    virtual bool SetExternalAwbHash(uint8_t *) override { return false; }

    virtual bool HasAwbHeader() const override { return false; }
    virtual uint8_t *GetAwbHeader(uint32_t *) const override { return nullptr; }
    virtual bool SetAwbHeader(void *, uint32_t, bool) override { return false; }

    //
    virtual uint32_t GetNumTracks() const override
    {
        return (uint32_t)files_list.size();
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
        *external = true;
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
        if (cue_idx >= files_list.size())
            return false;

        const std::string &str = files_list[cue_idx];
        size_t pos = str.find_last_of("/\\");

        if (pos == std::string::npos)
            *name = str;
        else
            *name = str.substr(pos+1);

        return true;
    }
};

#endif // PSEUDOCRIWAREAUDIOCONTAINER_H
