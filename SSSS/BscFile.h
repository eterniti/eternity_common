#ifndef __BSCFILE_H__
#define __BSCFILE_H__

#include "SlotsFile.h"

#define BSC_SIGNATURE	0x43534225

typedef struct
{
	uint32_t signature;
	uint32_t version;
	uint32_t num_avatars;
	uint32_t num_battle_names;
	uint32_t avatars_offset;
	uint32_t battle_names_offset;
} __attribute__((packed)) BSCHeader;

class BscFile : public BaseFile
{
private:

	const unsigned int CURRENT_VERSION = 1;

	std::vector<CharacterDef> avatars;
	std::vector<CharacterDef> battle_names;
	
	void Reset();

public:
	
	BscFile();
	BscFile(const uint8_t *buf, size_t size);
	virtual ~BscFile();
	
	inline size_t GetNumAvatars() 
	{
		return avatars.size();
	}

	inline size_t GetNumBattleNames()
	{
		return battle_names.size();
	}
	
	inline bool IsEmpty()
	{
		return (GetNumAvatars() == 0 && GetNumBattleNames() == 0);
	}
	
	inline bool GetAvatarEntry(uint32_t idx, uint32_t *cms_entry, uint32_t *cms_model_spec_idx)
	{
		if (idx >= avatars.size())
			return false;
		
		*cms_entry = avatars[idx].cms_entry;
		*cms_model_spec_idx = avatars[idx].cms_model_spec_idx;
		return true;
	}
	
	inline bool GetBattleNameEntry(uint32_t idx, uint32_t *cms_entry, uint32_t *cms_model_spec_idx)
	{
		if (idx >= battle_names.size())
			return false;
		
		*cms_entry = battle_names[idx].cms_entry;
		*cms_model_spec_idx = battle_names[idx].cms_model_spec_idx;
		return true;
	}
	
	inline void AppendAvatarEntry(uint32_t cms_entry, uint32_t cms_model_spec_idx)
	{
		CharacterDef def;
		
		def.cms_entry = cms_entry;
		def.cms_model_spec_idx = cms_model_spec_idx;
		
		avatars.push_back(def);
	}
	
	inline void AppendBattleNameEntry(uint32_t cms_entry, uint32_t cms_model_spec_idx)
	{
		CharacterDef def;
		
		def.cms_entry = cms_entry;
		def.cms_model_spec_idx = cms_model_spec_idx;
		
		battle_names.push_back(def);
	}
	
	virtual bool Load(const uint8_t *buf, size_t size) override;
	virtual uint8_t *Save(size_t *psize) override;
};

#endif

