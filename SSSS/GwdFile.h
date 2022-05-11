#ifndef __GWDFILE_H__
#define __GWDFILE_H__

#include <vector>

#include "BaseFile.h"

#define GWD_SIGNATURE	"#GWD"

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

typedef struct
{
    char signature[4]; // 0
    uint16_t endianess_check; // 4
    uint16_t unk_06; // 6 probably padding
    uint32_t num_tournaments; // 8
    uint32_t unk_0C; // 0xC  zero
    uint32_t data_start; // 0x10
    uint32_t unk_14; // 0x14
    // size 0x18
} PACKED GWDHeader;

static_assert(sizeof(GWDHeader) == 0x18, "Incorrect structure size.");

// offsets are absolute

typedef struct
{
    uint32_t category_index; // 0  index within the category
    uint32_t category; // 4     // Bronze, Silver, Gold, Legendary, and the other one
    uint32_t name_id; // 8
    uint32_t unk_0C; // 0xC
    uint32_t unk_10; // 0x10
    uint32_t unk_14; // 0x14
    uint64_t num_participants; // 0x18
    uint64_t gpd_ids_offset; // 0x20
    uint64_t unk_offset; // 0x28
    uint64_t unk2_offset; // 0x30
    uint64_t unk3_offset; // 0x38
    uint64_t unk4_offset; // 0x40
    uint64_t unk5_offset; // 0x48
    // size 0x50
} PACKED GWDEntry;

static_assert(sizeof(GWDEntry) == 0x50, "Incorrect structure size.");

#ifdef _MSC_VER
#pragma pack(pop)
#endif

enum GwdCategory
{
    GWD_BRONZE,
    GWD_SILVER,
    GWD_GOLD,
    GWD_LEGEND,
    GWD_GOD,
    NUM_GWD_CATEGORIES
};

class GpdFile;

struct GwdParticipant
{
    uint32_t gpd_id;
    uint32_t unk1;
    uint32_t unk2;
    uint32_t unk3;
    uint32_t unk4;
    uint32_t unk5;

    void Decompile(TiXmlNode *root, const GpdFile *gpd) const;
    bool Compile(const TiXmlElement *root);
};

struct GwdTournament
{
    GwdCategory category;
    uint32_t category_index;
    std::vector<GwdParticipant> participants;

    // from GWDEntryHeader
    uint32_t name_id;
    uint32_t unk_0C;
    uint32_t unk_10;
    uint32_t unk_14;

    void Decompile(TiXmlNode *root, uint32_t id, const GpdFile *gpd) const;
    bool Compile(const TiXmlElement *root);
};

class GwdParticipantFinder
{
private:

    uint32_t gpd_id;

public:
    GwdParticipantFinder(const GwdParticipant &p) { gpd_id = p.gpd_id; }
    GwdParticipantFinder(uint32_t gpd_id) : gpd_id(gpd_id) { }
    bool operator()(const GwdParticipant &p){ return (p.gpd_id == gpd_id); }

};

class GwdFile : public BaseFile
{

    std::vector<GwdTournament> tournaments;

    // Extra data
    const GpdFile *gpd = nullptr;

private:

    void Reset();

    unsigned int CalculateFileSize();

public:

    GwdFile();
    virtual ~GwdFile();
	
	inline uint32_t GetNumTournaments() const { return tournaments.size(); }
	
	inline GwdTournament *GetTournament(uint32_t id) 
	{
		if (id >= tournaments.size())
			return nullptr;
		
		return &tournaments[id];
	}
	
	inline const GwdTournament *GetTournament(uint32_t id) const
	{
		if (id >= tournaments.size())
			return nullptr;
		
		return &tournaments[id];
	}

    inline void SetGpd(const GpdFile *gpd)
    {
        this->gpd = gpd;
    }

    virtual std::vector<uint32_t> GetCharacterTours(uint32_t gpd_id) const;
    virtual bool SetParticipants(uint32_t id, std::vector<uint32_t> &new_participants);

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *size) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;
};

#endif
