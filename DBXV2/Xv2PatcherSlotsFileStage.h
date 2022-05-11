#ifndef XV2PATCHERSLOTSFILESTAGE_H
#define XV2PATCHERSLOTSFILESTAGE_H

#include "BaseFile.h"

struct Xv2StageSlot
{
    uint32_t stage;
    int dlc;
};

class Xv2PatcherSlotsFileStage : public BaseFile
{
private:

    std::vector<Xv2StageSlot> stage_slots;

public:

    Xv2PatcherSlotsFileStage();
    virtual ~Xv2PatcherSlotsFileStage();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    inline size_t GetNumSlots() const { return stage_slots.size(); }

    const std::vector<Xv2StageSlot> &GetSlots() const { return stage_slots; }
    std::vector<Xv2StageSlot> &GetSlots() { return stage_slots; }

    size_t FindSlots(uint32_t stage_id, std::vector<Xv2StageSlot *> &entries);
    size_t RemoveSlots(uint32_t stage_id);

    inline const Xv2StageSlot &operator[](size_t n) const { return stage_slots[n]; }
    inline Xv2StageSlot &operator[](size_t n) { return stage_slots[n]; }

    inline std::vector<Xv2StageSlot>::const_iterator begin() const { return stage_slots.begin(); }
    inline std::vector<Xv2StageSlot>::const_iterator end() const { return stage_slots.end(); }

    inline std::vector<Xv2StageSlot>::iterator begin() { return stage_slots.begin(); }
    inline std::vector<Xv2StageSlot>::iterator end() { return stage_slots.end(); }
};

#endif // XV2PATCHERSLOTSFILESTAGE_H
