#ifndef __CNCFILE_H__
#define __CNCFILE_H__

// This is SSSS cnc format.
// For Xv2 format, which is unrelated, see CncXv2File

#include "SlotsFile.h"

#define CNC_SIGNATURE	0x434E4325

typedef struct
{
	uint32_t signature;
	uint32_t version;
	uint32_t num_sign_names;
	uint32_t num_robes_names;
	uint32_t sign_names_offset;
	uint32_t robes_names_offset;
} __attribute__((packed)) CNCHeader;

class CncFile : public BaseFile
{
private:

	const unsigned int CURRENT_VERSION = 1;

	std::vector<CharacterDef> sign_names;
	std::vector<CharacterDef> robes_names;
	
	void Reset();

public:
	
	CncFile();
	CncFile(const uint8_t *buf, size_t size);
	virtual ~CncFile();
	
	inline size_t GetNumSignNames() 
	{
		return sign_names.size();
	}

	inline size_t GetNumRobesNames()
	{
		return robes_names.size();
	}
	
	inline bool IsEmpty()
	{
		return (GetNumSignNames() == 0 && GetNumRobesNames() == 0);
	}
	
	inline bool GetSignNameEntry(uint32_t idx, uint32_t *cms_entry, uint32_t *cms_model_spec_idx)
	{
		if (idx >= sign_names.size())
			return false;
		
		*cms_entry = sign_names[idx].cms_entry;
		*cms_model_spec_idx = sign_names[idx].cms_model_spec_idx;
		return true;
	}
	
	inline bool GetRobesNameEntry(uint32_t idx, uint32_t *cms_entry, uint32_t *cms_model_spec_idx)
	{
		if (idx >= robes_names.size())
			return false;
		
		*cms_entry = robes_names[idx].cms_entry;
		*cms_model_spec_idx = robes_names[idx].cms_model_spec_idx;
		return true;
	}
	
	inline void AppendSignNameEntry(uint32_t cms_entry, uint32_t cms_model_spec_idx)
	{
		CharacterDef def;
		
		def.cms_entry = cms_entry;
		def.cms_model_spec_idx = cms_model_spec_idx;
		
		sign_names.push_back(def);
	}
	
	inline void AppendRobesNameEntry(uint32_t cms_entry, uint32_t cms_model_spec_idx)
	{
		CharacterDef def;
		
		def.cms_entry = cms_entry;
		def.cms_model_spec_idx = cms_model_spec_idx;
		
		robes_names.push_back(def);
	}
	
	virtual bool Load(const uint8_t *buf, size_t size) override;
	virtual uint8_t *Save(unsigned int *psize) override;
};

#endif

