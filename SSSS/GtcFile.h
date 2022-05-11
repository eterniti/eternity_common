#ifndef __GTCFILE_H__
#define __GTCFILE_H__

#include "SlotsFile.h"

#define GTC_SIGNATURE	0x43544725

typedef struct
{
	uint32_t signature;
	uint32_t version;
	uint32_t num_banners;
	uint32_t num_banner_names;
	uint32_t banners_offset;
	uint32_t banner_names_offset;
} __attribute__((packed)) GTCHeader;

class GtcFile : public BaseFile
{
private:

	const unsigned int CURRENT_VERSION = 1;

	std::vector<CharacterDef> banners;
	std::vector<CharacterDef> banner_names;
	
	void Reset();

public:
	
	GtcFile();
	GtcFile(const uint8_t *buf, size_t size);
	virtual ~GtcFile();
	
	inline size_t GetNumBanners() 
	{
		return banners.size();
	}

	inline size_t GetNumBannerNames()
	{
		return banner_names.size();
	}
	
	inline bool IsEmpty()
	{
		return (GetNumBanners() == 0 && GetNumBannerNames() == 0);
	}
	
	inline bool GetBannerEntry(uint32_t idx, uint32_t *cms_entry, uint32_t *cms_model_spec_idx)
	{
		if (idx >= banners.size())
			return false;
		
		*cms_entry = banners[idx].cms_entry;
		*cms_model_spec_idx = banners[idx].cms_model_spec_idx;
		return true;
	}
	
	inline bool GetBannerNameEntry(uint32_t idx, uint32_t *cms_entry, uint32_t *cms_model_spec_idx)
	{
		if (idx >= banner_names.size())
			return false;
		
		*cms_entry = banner_names[idx].cms_entry;
		*cms_model_spec_idx = banner_names[idx].cms_model_spec_idx;
		return true;
	}
	
	inline void AppendBannerEntry(uint32_t cms_entry, uint32_t cms_model_spec_idx)
	{
		CharacterDef def;
		
		def.cms_entry = cms_entry;
		def.cms_model_spec_idx = cms_model_spec_idx;
		
		banners.push_back(def);
	}
	
	inline void AppendBannerNameEntry(uint32_t cms_entry, uint32_t cms_model_spec_idx)
	{
		CharacterDef def;
		
		def.cms_entry = cms_entry;
		def.cms_model_spec_idx = cms_model_spec_idx;
		
		banner_names.push_back(def);
	}
	
	virtual bool Load(const uint8_t *buf, size_t size) override;
	virtual uint8_t *Save(size_t *psize) override;
};

#endif

