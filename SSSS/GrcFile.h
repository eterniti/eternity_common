#ifndef __GRCFILE_H__
#define __GRCFILE_H__

#include "SlotsFile.h"

#define GRC_SIGNATURE	0x43524725

typedef struct
{
	uint32_t signature;
	uint32_t version;
	uint32_t num_icons;
	uint32_t num_names;
	uint32_t icons_offset;
	uint32_t names_offset;
} __attribute__((packed)) GRCHeader;

class GrcFile : public BaseFile
{
private:

	const unsigned int CURRENT_VERSION = 1;

	std::vector<CharacterDef> icons;
	std::vector<CharacterDef> names;
	
	void Reset();

public:
	
	GrcFile();
	GrcFile(const uint8_t *buf, size_t size);
	virtual ~GrcFile();
	
	inline size_t GetNumIcons() 
	{
		return icons.size();
	}

	inline size_t GetNumNames()
	{
		return names.size();
	}
	
	inline bool IsEmpty()
	{
		return (GetNumIcons() == 0 && GetNumNames() == 0);
	}
	
	inline bool GetIconEntry(uint32_t idx, uint32_t *cms_entry, uint32_t *cms_model_spec_idx)
	{
		if (idx >= icons.size())
			return false;
		
		*cms_entry = icons[idx].cms_entry;
		*cms_model_spec_idx = icons[idx].cms_model_spec_idx;
		return true;
	}
	
	inline bool GetNameEntry(uint32_t idx, uint32_t *cms_entry, uint32_t *cms_model_spec_idx)
	{
		if (idx >= names.size())
			return false;
		
		*cms_entry = names[idx].cms_entry;
		*cms_model_spec_idx = names[idx].cms_model_spec_idx;
		return true;
	}
	
	inline void AppendIconEntry(uint32_t cms_entry, uint32_t cms_model_spec_idx)
	{
		CharacterDef def;
		
		def.cms_entry = cms_entry;
		def.cms_model_spec_idx = cms_model_spec_idx;
		
		icons.push_back(def);
	}
	
	inline void AppendNameEntry(uint32_t cms_entry, uint32_t cms_model_spec_idx)
	{
		CharacterDef def;
		
		def.cms_entry = cms_entry;
		def.cms_model_spec_idx = cms_model_spec_idx;
		
		names.push_back(def);
	}
	
	virtual bool Load(const uint8_t *buf, size_t size) override;
	virtual uint8_t *Save(size_t *psize) override;
};

#endif

