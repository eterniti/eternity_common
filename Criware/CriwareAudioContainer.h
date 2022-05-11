#ifndef __CRIWAREAUDIOCONTAINER_H__
#define __CRIWAREAUDIOCONTAINER_H__

#include "BaseFile.h"

class CriwareAudioContainer : public BaseFile
{
public:

	virtual ~CriwareAudioContainer() {}
	
	virtual bool HasAwb() const = 0;
	virtual uint8_t *GetAwb(uint32_t *awb_size) const = 0;
    virtual bool SetAwb(void *awb, uint32_t awb_size, bool take_ownership=false) = 0;
	
    virtual const uint8_t *GetExternalAwbHash() const = 0;
	virtual bool SetExternalAwbHash(uint8_t *hash) = 0;
	
	virtual bool HasAwbHeader() const = 0;
	virtual uint8_t *GetAwbHeader(uint32_t *header_size) const = 0;		
    virtual bool SetAwbHeader(void *header, uint32_t header_size, bool take_ownership) = 0;

    //
    virtual uint32_t GetNumTracks() const = 0;
    virtual uint32_t GetFilesPerTrack() const = 0;

    virtual bool CanAddTrack() const = 0;

    virtual uint32_t AwbIndexToTrackIndex(uint32_t idx, bool external) const = 0;
    virtual uint32_t TrackIndexToAwbIndex(uint32_t idx, bool *external) const = 0;

    /*virtual uint32_t TrackIndexToCueIndex(uint32_t idx) const = 0;
    virtual size_t TrackIndexToCueIndexes(uint32_t idx, std::vector<uint32_t> &cue_idxs) const = 0;
    virtual uint32_t CueIndexToTrackIndex(uint32_t idx) const = 0;*/
    virtual uint32_t TrackIndexToCueId(uint32_t idx) const = 0;
    virtual size_t TrackIndexToCueIds(uint32_t idx, std::vector<uint32_t> &cue_ids) const = 0;
    virtual uint32_t CueIdToTrackIndex(uint32_t id) const = 0;
    virtual size_t CueIdToTrackIndexes(uint32_t id, std::vector<uint32_t> &idxs) const = 0;

    /*virtual uint32_t AwbIndexToCueIndex(uint32_t idx, bool external) const = 0;
    virtual uint32_t CueIndexToAwbIndex(uint32_t idx, bool *external) const = 0;*/

    virtual uint32_t AwbIndexToCueId(uint32_t idx, bool external) const = 0;
    virtual uint32_t CueIdToAwbIndex(uint32_t id, bool *external) const = 0;

    /*virtual uint32_t CueIndexToCueId(uint32_t idx) const = 0;
    virtual uint32_t CueIdToCueIndex(uint32_t id) const = 0;
    virtual size_t CueIdToCueIndexes(uint32_t id, std::vector<uint32_t> &cue_idxs) const = 0;*/

    virtual bool GetCueName(uint32_t cue_id, std::string *name) const = 0;
};

#endif // __CRIWAREAUDIOCONTAINER_H__
