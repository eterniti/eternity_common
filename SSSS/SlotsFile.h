#ifndef __SLOTSFILE_H__
#define __SLOTSFILE_H__

#include <stdint.h>

#include <vector>

#include "CmsFile.h"

#define MAX_SLOTS	10

#define NUM_CHARACTERS_LISTS			95
#define NUM_CHARACTERS_LISTS_NOT_EMPTY	89	

#define NUM_CHARACTERS_LISTS2	9

#define RANDOM_SLOT_UPPER_RIGHT		(0x18)
#define RANDOM_SLOT_LOWER_LEFT		(0x32)
#define RANDOM_SLOT_LOWER_RIGHT		(0x4A)

typedef struct
{
	uint32_t cms_entry; // 0 
	uint32_t cms_model_spec_idx; // 4
    float icon_id; // 8  icon_id in float version even if it should be an integer. because the game is like that...
	uint32_t unk_0C; // 0xC, always 0, apparently...
	// size 0x10
} __attribute__((packed)) Slot;

typedef struct
{
    Slot sub_slots[MAX_SLOTS]; // 0
	uint32_t pad[7]; // 0xA0   apparently always 0
	// size 0xBC
} __attribute__((packed)) CharacterSlot;

// This structure related with character on select screen
typedef struct
{
	uint32_t cms_entry; // 0
	uint32_t cms_model_spec_idx; // 4
	uint32_t sign_name_id; // 8   redundant with cdc, but it is sometimes different! (game bug or different value?) The one here is the one that the game seems to pick anyway.
	uint32_t select_name_id; // 0x0C  
	uint32_t robes_name_id; // 0x10   redundant with cdc, but it is sometimes different! (game bug or different value?) The one here is the one that the game seems to pick anyway.
	uint32_t select2_name_id; // 0x14  used in galaxian wars
	// size 0x18
} __attribute__((packed)) CharacterInfo;

// This structure is related with the battle_steam
typedef struct
{
	uint32_t cms_entry; // 0
	uint32_t cms_model_spec_idx; // 4
	uint32_t avatar_id; // 8
	uint32_t battle_name_id; // 0x0C
	uint32_t unk_10; // 0x10
	// size 0x14
} __attribute__((packed)) CharacterInfo2;

// This structure is related with data in after victory of Galaxian Wars
typedef struct
{
	uint32_t cms_entry; // 0
	uint32_t cms_model_spec_idx; // 4
	float gwr_name_id; // 8
	float gwr_icon_id; // 0xC
	uint32_t unk_10; // 0x10
	uint32_t unk_14; // 0x14
	// size 0x18
} __attribute__((packed)) CharacterInfo3;

// This structure is related with data in Galaxian Wars "Tour" (the screen with the banners)
typedef struct
{
	uint32_t cms_entry; // 0
	uint32_t cms_model_spec_idx; // 4
	float gwt_banner_name_id; // 8
	float gwt_banner_id; // 0xC
	uint32_t unk_10; // 0x10
	uint32_t unk_14; // 0x14
} __attribute__((packed)) CharacterInfo4;

typedef struct
{
	uint32_t cms_entry; // 0
	uint32_t cms_model_spec_idx; // 4
} __attribute__((packed)) CharacterDef;

enum CharactersListConstants
{
	PEGASUS_SEIYA, // 0
	SAGITARIUS_SEIYA,
	PEGASUS_SEIYA_GOD,
	ODIN_SEIYA,
	DRAGON_SHIRYU,
	LIBRA_SHIRYU,
	CYGNUS_HYOGA,
	AQUARIUS_HYOGA,
	ANDROMEDA_SHUN,
	VIRGO_SHUN,
	PHOENIX_IKKI, // 10
	LEO_IKKI,
	HIDRA_ICHI,
	UNICORN_JABU,
	EAGLE_MARIN,
	OPHIUCHUS_SHAINA,
	LYRA_ORPHEE,
	ARIES_MUU_SAINT,
	ARIES_SHION_SAINT,
	ARIES_SHION_SPECTRE,
	TAURUS_ALDEBARAN_SAINT, // 20
	GEMINI_MYSTERIOUS_SAINT,
	GEMINI_SAGA_SAINT,
	GEMINI_SAGA_SPECTRE,
	CANCER_DEATHMASK_SAINT,
	CANCER_DEATHMASK_SPECTRE,
	LEO_AIORIA_SAINT,
	VIRGO_SHAKA_SAINT,
	LIBRA_DOHKO_SAINT,
	SCORPIUS_MILO_SAINT,
	SAGITARIUS_AIOROS_SAINT, // 30
	CAPRICORNUS_SHURA_SAINT,
	CAPRICORNUS_SHURA_SPECTRE,
	AQUARIUS_CAMUS_SAINT,
	AQUARIUS_CAMUS_SPECTRE,
	PISCES_APHRODITE_SAINT,
	PISCES_APHRODITE_SPECTRE,
	GEMINI_KANON_SAINT,
	DRAGONSEA_KANON,
	SIREN_SORRENTO,
	SEAHORSE_BAIAN, // 40
	SCYLLA_IO,
	CHRYSAOR_KRISHNA,
	LYUMNADES_KASA,
	KRAKEN_ISAAC,
	POSEIDON,
	JULIAN,
	WYVERN_RHADAMANTHYS,
	GARUDA_AIACOS,
	GRIFFON_MINOS,
	THANATOS, // 50
	HYPNOS,
	HADES,
	ATHENA,
	BRONZE_ARMOR,
	SILVER_ARMOR,
	GOLD_ARMOR,
	POSEIDON_AND_GENERALS,
	HADES_AND_SERVANTS,
	GEMINI_EVIL_SAGA_SAINT,
	DUHBE_SIGFRIED, // 60
	PHECDA_THOR,
	MEGREZ_ALBERICH,
	MERAK_HAGEN,
	ALIOTH_FENRIR,
	MIZAR_SYD,
	ALCOR_BUD,
	BENESTACH_MIME,
	POLARIS_HILDA,
	GOD_WARRIORS,
	ARIES_MUU_GOD, // 70
	TAURUS_ALDEBARAN_GOD,
	GEMINI_SAGA_GOD,
	CANCER_DEATHMASK_GOD,
	LEO_AIORIA_GOD,
	VIRGO_SHAKA_GOD,
	LIBRA_DOHKO_GOD,
	SCORPIUS_MILO_GOD,
	SAGITARIUS_AIOROS_GOD,
	CAPRICORNUS_SHURA_GOD,
	AQUARIUS_CAMUS_GOD, // 80
	PISCES_APHRODITE_GOD,
	GOLD_GOD_ARMOR,
	DRAGON_SHIRYU_GOD,
	CYGNUS_HYOGA_GOD,
	ANDROMEDA_SHUN_GOD,
	PHOENIX_IKKI_GOD,
	SAGITARIUS_SEIYA_GOD,
	BRONZE_GOD_SAINT, // 88
	EMPTY1, // 89
	EMPTY2,
	EMPTY3,
	EMPTY4,
	EMPTY5,
	EMPTY6,
};

enum CharactersList2Constants
{
	BRONZE_SAINT,
	SILVER_SAINT,
	GOLD_SAINT,
	GOLD_GOD_SAINT,
	ASGARD_WARRIOR,
	POSEIDON_GENERAL,
	HADES_SPECTRE,
	GOD,
	BRONZE_GOD_ARMOR,
};

class SlotsFile;

class CIComparer
{
private:

    SlotsFile *slf;
	
public:

    CIComparer(SlotsFile *slf) { this->slf = slf; }
	bool operator()(const CharacterInfo & info1, const CharacterInfo & info2);
};

class CLComparer
{
private:
    SlotsFile *slf;
	
public:

    CLComparer(SlotsFile *slf) { this->slf = slf; }
	bool operator()(const CharacterDef & entry1, const CharacterDef & entry2);
};

class CLFinder
{
private:

	CharacterDef *entry;

public:

	CLFinder(CharacterDef *entry) { this->entry = entry; }
	bool operator()(const CharacterDef & entry)
	{
		return (entry.cms_entry == this->entry->cms_entry && entry.cms_model_spec_idx == this->entry->cms_model_spec_idx);
	}
};

class CI2Comparer
{
private:

    SlotsFile *slf;
	
public:

    CI2Comparer(SlotsFile *slf) { this->slf = slf; }
	bool operator()(const CharacterInfo2 & info1, const CharacterInfo2 & info2);
};


class CI2Finder
{
private:

	CharacterInfo2 *entry;

public:

	CI2Finder(CharacterInfo2 *entry) { this->entry = entry; }
	bool operator()(const CharacterInfo2 & entry)
	{
		return (memcmp(this->entry, &entry, sizeof(CharacterInfo2)) == 0);
	}
};

class CI3Finder
{
private:

	uint32_t cms_entry;
	uint32_t cms_model_spec_idx;

public:

	CI3Finder(uint32_t cms_entry, uint32_t cms_model_spec_idx) : cms_entry(cms_entry), cms_model_spec_idx(cms_model_spec_idx) { }
	bool operator()(const CharacterInfo3 & entry)
	{
		return (entry.cms_entry == cms_entry && entry.cms_model_spec_idx == cms_model_spec_idx);
	}
};

class CI3Comparer
{
private:

	SlotsFile *slf;

public:

	CI3Comparer(SlotsFile *slf) { this->slf = slf; }
	bool operator()(const CharacterInfo3 &info1, const CharacterInfo3 &info2);	
};

class CI4Finder
{
private:

	uint32_t cms_entry;
	uint32_t cms_model_spec_idx;

public:

	CI4Finder(uint32_t cms_entry, uint32_t cms_model_spec_idx) : cms_entry(cms_entry), cms_model_spec_idx(cms_model_spec_idx) { }
	bool operator()(const CharacterInfo4 & entry)
	{
		return (entry.cms_entry == cms_entry && entry.cms_model_spec_idx == cms_model_spec_idx);
	}
};

class CI4Comparer
{
private:

	SlotsFile *slf;

public:

	CI4Comparer(SlotsFile *slf) { this->slf = slf; }
	bool operator()(const CharacterInfo4 &info1, const CharacterInfo4 &info2);	
};


struct SlotsLoadData
{
	uint8_t *slots_buf, *ci_buf, *cl_buf, *ci2_buf, *ci3_buf, *ci4_buf;
	unsigned int slots_size, ci_size, cl_size, ci2_size, ci3_size, ci4_size;
};

struct SlotsSaveData
{
	uint8_t *slots_buf, *ci_buf, *ci2_buf, *ci3_buf, *ci4_buf;
	unsigned int slots_size, ci_size, ci2_size, ci3_size, ci4_size;
};

class SlotsFile : public BaseFile
{
private:

	uint8_t *slots_buf, *ci_buf, *ci2_buf;
	unsigned int slots_size, ci_size, ci2_size;
	
    CharacterSlot *c_slots; //  non-allocated
	CharacterInfo *infos;  // non-allocated
	CharacterInfo2 *infos2;  // non-allocated
	
	uint32_t num_slots;
	uint32_t num_infos;
	uint32_t num_infos2;
	
	std::vector<std::vector<CharacterDef>> characters_lists;
	std::vector<CharacterInfo3> infos3;
	std::vector<CharacterInfo4> infos4;
	
    void Reset();
	
    CharacterInfo *FindCharacterInfo(uint32_t cms_entry, uint32_t cms_model_spec_idx, bool show_error=true);
    inline CharacterInfo *FindCharacterInfo(Slot *slot, bool show_error=true) { return FindCharacterInfo(slot->cms_entry, slot->cms_model_spec_idx, show_error); }
	
    CharacterInfo2 *FindCharacterInfo2(uint32_t cms_entry, uint32_t cms_model_spec_idx, bool show_error=true);
    inline CharacterInfo2 *FindCharacterInfo2(Slot *slot, bool show_error=true) { return FindCharacterInfo2(slot->cms_entry, slot->cms_model_spec_idx, show_error); }
    bool IdenticalInfo2Exists(const CharacterInfo2 *info2);
	
    CharacterInfo3 *FindCharacterInfo3(uint32_t cms_entry, uint32_t cms_model_spec_idx);
    inline CharacterInfo3 *FindCharacterInfo3(Slot *slot) { return FindCharacterInfo3(slot->cms_entry, slot->cms_model_spec_idx); }
	
    CharacterInfo4 *FindCharacterInfo4(uint32_t cms_entry, uint32_t cms_model_spec_idx) ;
    inline CharacterInfo4 *FindCharacterInfo4(Slot *slot) { return FindCharacterInfo4(slot->cms_entry, slot->cms_model_spec_idx); }
	
	unsigned int GetNumberOfLists(uint8_t *buf, unsigned int size);
    bool IsCharacterInList(size_t list, uint32_t cms_entry, uint32_t cms_model_spec_idx) const;
	size_t GetListFromName(const std::string & str);
	
    void GetCategories(Slot *slot, std::string & categories) const;
	void RemoveFromAllCategories(Slot *slot);
	
	void AppendDefaultCharacterInfo();
	void AppendDefaultCharacterInfo2();
	
	void AppendCharacterInfo(CharacterInfo *info);
	void AppendCharacterInfo2(CharacterInfo2 *info);
    inline void AppendCharacterInfo3(CharacterInfo3 *info)
    {
        infos3.push_back(*info);
    }

    inline void AppendCharacterInfo4(CharacterInfo4 *info)
    {
        infos4.push_back(*info);
    }	
	
    void WriteSlot(TiXmlElement *root, Slot *slot, uint32_t parent_index, uint32_t index) const;
    void WriteCharacterSlot(TiXmlElement *root, uint32_t index) const;
	
	bool ReadSlot(TiXmlElement *root, Slot *slot, uint32_t index, bool *add_info, CharacterInfo *info, CharacterInfo2 *info2, CharacterInfo3 **pinfo3, CharacterInfo4 **pinfo4);
    bool ReadCharacterSlot(TiXmlElement *root, CharacterSlot *tslot, uint32_t index, std::vector<CharacterInfo> & info_list, std::vector<CharacterInfo2> & info2_list);
	
	void AddCharactersListsSpecialEntries();
	void AddCharactersInfo2SpecialEntries(std::vector<CharacterInfo2> &info2_list);
	void AddCharactersInfo3SpecialEntries();
	void AddCharactersInfo4SpecialEntries();
	
	void CreateCompatCI3Entries();	
	uint8_t *CreateCI3(unsigned int *psize);	
	
	void CreateCompatCI4Entries();
	uint8_t *CreateCI4(unsigned int *psize);
	
	friend class CIComparer;
	friend class CLComparer;
	friend class CI2Comparer;
	friend class CI3Comparer;
	friend class CI4Comparer;

public:

	SlotsFile();
	virtual ~SlotsFile();	
	
	bool Load(SlotsLoadData *load_data);	
    bool LoadCI3(uint8_t *ci3_buf, uint32_t ci3_size);
	bool LoadCI4(uint8_t *ci4_buf, uint32_t ci4_size);
	
	inline bool HasCI3()
	{
		return (infos3.size() > 0);
	}
	
	inline bool HasCI4()
	{
		return (infos4.size() > 0);
	}
	
    inline CharacterInfo *GetCharacterInfo(uint32_t cms_entry, uint32_t cms_model_spec_idx)
	{ 	
		return FindCharacterInfo(val32(cms_entry), val32(cms_model_spec_idx), false);
	}
	
    inline CharacterInfo2 *GetCharacterInfo2(uint32_t cms_entry, uint32_t cms_model_spec_idx)
	{ 	
		return FindCharacterInfo2(val32(cms_entry), val32(cms_model_spec_idx), false);
	}
	
    inline CharacterInfo3 *GetCharacterInfo3(uint32_t cms_entry, uint32_t cms_model_spec_idx)
	{
		return FindCharacterInfo3(val32(cms_entry), val32(cms_model_spec_idx));
	}
	
    inline CharacterInfo4 *GetCharacterInfo4(uint32_t cms_entry, uint32_t cms_model_spec_idx)
	{
		return FindCharacterInfo4(val32(cms_entry), val32(cms_model_spec_idx));
	}
	
	inline void GetCategories(uint32_t cms_entry, uint32_t cms_model_spec_idx, std::string & categories)
	{
		Slot slot;
		
		slot.cms_entry = val32(cms_entry);
		slot.cms_model_spec_idx = val32(cms_model_spec_idx);
		
		GetCategories(&slot, categories);
	}
	
	inline bool CharacterHasCI3(uint32_t cms_entry, uint32_t cms_model_spec_idx)
	{
		return (FindCharacterInfo3(cms_entry, cms_model_spec_idx) != nullptr);
	}

	inline bool CharacterHasCI4(uint32_t cms_entry, uint32_t cms_model_spec_idx)
	{
		return (FindCharacterInfo4(cms_entry, cms_model_spec_idx) != nullptr);
	}
	
	inline uint32_t GetNumSlots() { return num_slots; } 
	
	bool FindSlot(uint32_t cms_entry, uint32_t cms_model_spec_idx, uint32_t *slot_idx, uint32_t *sub_slot_idx);
	
	bool SlotHasCI3(uint32_t slot_idx, uint32_t subslot_idx);
	bool SlotHasCI4(uint32_t slot_idx, uint32_t subslot_idx);	
	
    static inline bool IsOriginalRandomSlot(uint32_t slot_idx) 
	{ 
		return (slot_idx == RANDOM_SLOT_UPPER_RIGHT || slot_idx == RANDOM_SLOT_LOWER_LEFT || slot_idx == RANDOM_SLOT_LOWER_RIGHT); 
	}
	
    bool IsCurrentRandomSlot(uint32_t slot_idx) const;
	
    bool IsFreeSlot(uint32_t slot_idx, uint32_t sub_slot_idx) const;
	uint32_t GetFreeSubSlot(uint32_t slot_idx);
	inline bool HasFreeSubSlot(uint32_t slot_idx)  { return (GetFreeSubSlot(slot_idx) != (uint32_t)-1);}
	
    bool GetSlot(uint32_t slot_idx, uint32_t sub_slot_idx, Slot *slot, CharacterInfo *info, CharacterInfo2 *info2,
                 std::string &categories, CharacterInfo3 **pinfo3, CharacterInfo4 **pinfo4);
    bool SetSlot(uint32_t slot_idx, uint32_t sub_slot_idx, Slot *slot, CharacterInfo *info, CharacterInfo2 *info2,
                 const std::string &categories, CharacterInfo3 *info3, CharacterInfo4 *info4);
	
	bool SetIconId(uint32_t cms_entry, uint32_t cms_model_spec_idx, float icon_id);
	
	bool SetSignNameId(uint32_t cms_entry, uint32_t cms_model_spec_idx, uint32_t sign_name_id);
	bool SetRobesNameId(uint32_t cms_entry, uint32_t cms_model_spec_idx, uint32_t robes_name_id);
	bool SetSelectNameId(uint32_t cms_entry, uint32_t cms_model_spec_idx, uint32_t select_name_id);
	bool SetSelect2NameId(uint32_t cms_entry, uint32_t cms_model_spec_idx, uint32_t select2_name_id);
	
	bool SetAvatarId(uint32_t cms_entry, uint32_t cms_model_spec_idx, uint32_t avatar_id);
	bool SetBattleNameId(uint32_t cms_entry, uint32_t cms_model_spec_idx, uint32_t battle_name_id);	
	
	bool SetGwrNameId(uint32_t cms_entry, uint32_t cms_model_spec_idx, float gwr_name_id);
	bool SetGwrIconId(uint32_t cms_entry, uint32_t cms_model_spec_idx, float gwr_icon_id);
	
	bool SetGwtBannerNameId(uint32_t cms_entry, uint32_t cms_model_spec_idx, float gwt_banner_name_id);
	bool SetGwtBannerId(uint32_t cms_entry, uint32_t cms_model_spec_idx, float gwt_banner_id);
	
	bool GetCmsEntry(uint32_t slot_idx, uint32_t sub_slot_idx, uint32_t *cms_entry, uint32_t *cms_model_spec_idx);
	
    bool MakeSlotFree(uint32_t slot_idx, uint32_t sub_slot_idx);

    virtual TiXmlDocument *Decompile() const override;
	virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;
	
	bool CreateFile(SlotsSaveData *save_data);		
	uint8_t *CreateLists(unsigned int *size, uint8_t **ptr_lists);	
	
	// Diagnostic function
	bool CompareCharacterInfo(SlotsFile *other);
	bool CompareLists(SlotsFile *other);
	bool CompareCharacterInfo2(SlotsFile *other);
	bool CompareCharacterInfo4(SlotsFile *other);
	
	bool CheckModels(CmsFile *cms);
};

#endif
