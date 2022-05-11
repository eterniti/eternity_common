#ifndef __CDCFILE_H__
#define __CDCFILE_H__

// TODO: this class needs a rewrite

#include <stdint.h>

#include <vector>

#include "CmsFile.h"
#include "SlotsFile.h"

// "#CDC"
#define CDC_SIGNATURE	0x43444323

typedef struct
{
	uint32_t signature; // 0
	uint16_t endianess_check; // 4
	uint16_t unk_06; // 6   zero
	uint32_t num_entries; // 8
	uint32_t unk_0C; // 0x0C    zero
	uint32_t data_start; // 0x10
	uint32_t unk_14; // 0x14	
	// 0x18
} __attribute__((packed)) CDCHeader;

typedef struct
{
	uint32_t char_id; // 0
	uint32_t name_id; // 4 same number seen in tdb
	uint32_t short_name_id; // 8
	uint32_t sign_name_id; // 0xC
	uint32_t robes_name_id; // 0x10 same number seen in tdb
	uint32_t unk_14; // 0x14
	uint32_t robes_type; // 18
	uint32_t is_dlc; // 1C
	// 0x20	
} __attribute__((packed)) CDCEntry;

class CdcFile;

class CdcEntry
{
	friend class CdcFile;
	
private:
	
	bool initialized = false;
	
public:
	
	uint32_t char_id;
	uint32_t name_id;
	uint32_t short_name_id;
	uint32_t sign_name_id;
	uint32_t robes_name_id;
	uint32_t unk_14;
	uint32_t robes_type;
	uint32_t is_dlc;
};


class CdcFile : public BaseFile
{
	
private:

	uint8_t *buf;
	unsigned int size;
	
	CDCEntry *entries; // not-allocated	(ptr in buf)	
	uint32_t num_entries;
	
	std::vector<CharacterDef> char_map;
	
	void Reset();	
	
    void WriteEntry(TiXmlElement *root, uint32_t entry) const;
	bool ReadEntry(TiXmlElement *root, CdcEntry *entry);

public:

	CdcFile();
	CdcFile(uint8_t *buf, unsigned int size);
	virtual ~CdcFile();
	
    virtual bool Load(const uint8_t *buf, size_t size) override;
	virtual uint8_t *Save(size_t *size) override;
	
	inline uint32_t GetNumEntries() { return num_entries; }
	
    CdcEntry *GetModelEntry(uint32_t id, uint32_t *cms_entry=nullptr, uint32_t *cms_model_spec_idx=nullptr);
	bool SetModelEntry(uint32_t id, CdcEntry *entry, uint32_t cms_entry=0xFFFFFFFF, uint32_t cms_model_spec_idx=0xFFFFFFFF);
	
	bool CreateNewEntry(); // Create a new entry at the end, with default dummy data
	
	inline bool AppendEntry(CdcEntry *entry, uint32_t cms_entry=0xFFFFFFFF, uint32_t cms_model_spec_idx=0xFFFFFFFF)
	{
		if (!CreateNewEntry())
			return false;
		
		return SetModelEntry(num_entries-1, entry, cms_entry, cms_model_spec_idx);
	}
	
	// Assumes that last entry must be a dummy/empty one, since the game won't read that one anyway
	inline bool AppendBeforeLast(CdcEntry *entry, uint32_t cms_entry=0xFFFFFFFF, uint32_t cms_model_spec_idx=0xFFFFFFFF)
	{
		if (!SetModelEntry(num_entries-1, entry, cms_entry, cms_model_spec_idx))
			return false;
		
		return CreateNewEntry();
	}
	
    bool DeleteEntry(uint32_t id);
	
	bool SetExtraDecompileData(CmsFile *cms);
	
    virtual TiXmlDocument *Decompile() const override;
	virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;
	
};

#endif

