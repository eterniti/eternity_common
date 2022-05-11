#ifndef __CHCFILE_H__
#define __CHCFILE_H__

#include "SlotsFile.h"

#define CHC_SIGNATURE	0x43484325

typedef struct
{
	uint32_t signature;
	uint32_t version;
	uint32_t num_icons;
	uint32_t num_select_names;
	uint32_t num_select2_names;
	uint32_t icons_offset;
	uint32_t select_names_offset;
	uint32_t select2_names_offset;
} __attribute__((packed)) CHCHeader;

class ChcFile : public BaseFile
{
private:

	const unsigned int CURRENT_VERSION = 2;

	std::vector<CharacterDef> icons;
	std::vector<CharacterDef> select_names;
	std::vector<CharacterDef> select2_names;
	
	void Reset();

public:
	
	ChcFile();
	ChcFile(const uint8_t *buf, size_t size);
	virtual ~ChcFile();
	
	inline size_t GetNumIcons() 
	{
		return icons.size();
	}

	inline size_t GetNumSelectNames()
	{
		return select_names.size();
	}
	
	inline size_t GetNumSelect2Names()
	{
		return select2_names.size();
	}
	
	inline bool IsEmpty()
	{
		return (GetNumIcons() == 0 && GetNumSelectNames() == 0 && GetNumSelect2Names() == 0);
	}
	
	inline bool GetIconEntry(uint32_t idx, uint32_t *cms_entry, uint32_t *cms_model_spec_idx)
	{
		if (idx >= icons.size())
			return false;
		
		*cms_entry = icons[idx].cms_entry;
		*cms_model_spec_idx = icons[idx].cms_model_spec_idx;
		return true;
	}
	
	inline bool GetSelectNameEntry(uint32_t idx, uint32_t *cms_entry, uint32_t *cms_model_spec_idx)
	{
		if (idx >= select_names.size())
			return false;
		
		*cms_entry = select_names[idx].cms_entry;
		*cms_model_spec_idx = select_names[idx].cms_model_spec_idx;
		return true;
	}
	
	inline bool GetSelect2NameEntry(uint32_t idx, uint32_t *cms_entry, uint32_t *cms_model_spec_idx)
	{
		if (idx >= select2_names.size())
			return false;
		
		*cms_entry = select2_names[idx].cms_entry;
		*cms_model_spec_idx = select2_names[idx].cms_model_spec_idx;
		return true;
	}
	
	inline void AppendIconEntry(uint32_t cms_entry, uint32_t cms_model_spec_idx)
	{
		CharacterDef def;
		
		def.cms_entry = cms_entry;
		def.cms_model_spec_idx = cms_model_spec_idx;
		
		icons.push_back(def);
	}
	
	inline void AppendSelectNameEntry(uint32_t cms_entry, uint32_t cms_model_spec_idx)
	{
		CharacterDef def;
		
		def.cms_entry = cms_entry;
		def.cms_model_spec_idx = cms_model_spec_idx;
		
		select_names.push_back(def);
	}
	
	inline void AppendSelect2NameEntry(uint32_t cms_entry, uint32_t cms_model_spec_idx)
	{
		CharacterDef def;
		
		def.cms_entry = cms_entry;
		def.cms_model_spec_idx = cms_model_spec_idx;
		
		select2_names.push_back(def);
	}
	
	virtual bool Load(const uint8_t *buf, size_t size) override;
	virtual uint8_t *Save(size_t *psize) override;
};

#endif

