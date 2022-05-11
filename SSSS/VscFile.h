#ifndef __VSCFILE_H__
#define __VSCFILE_H__

#include "SlotsFile.h"

#define VSC_SIGNATURE	0x43535625

typedef struct
{
	uint32_t signature;
	uint32_t version;
	uint32_t num_select_names;
	uint32_t select_names_offset;
} __attribute__((packed)) VSCHeader;

class VscFile : public BaseFile
{
private:

	const unsigned int CURRENT_VERSION = 1;

	std::vector<CharacterDef> select_names;
		
	void Reset();

public:
	
	VscFile();
	VscFile(const uint8_t *buf, size_t size);
	virtual ~VscFile();
	
	inline size_t GetNumSelectNames()
	{
		return select_names.size();
	}
	
	inline bool IsEmpty()
	{
		return (GetNumSelectNames() == 0);
	}
	
	inline bool GetSelectNameEntry(uint32_t idx, uint32_t *cms_entry, uint32_t *cms_model_spec_idx)
	{
		if (idx >= select_names.size())
			return false;
		
		*cms_entry = select_names[idx].cms_entry;
		*cms_model_spec_idx = select_names[idx].cms_model_spec_idx;
		return true;
	}
	
	inline void AppendSelectNameEntry(uint32_t cms_entry, uint32_t cms_model_spec_idx)
	{
		CharacterDef def;
		
		def.cms_entry = cms_entry;
		def.cms_model_spec_idx = cms_model_spec_idx;
		
		select_names.push_back(def);
	}	
	
	virtual bool Load(const uint8_t *buf, size_t size) override;
	virtual uint8_t *Save(size_t *psize) override;
};

#endif

