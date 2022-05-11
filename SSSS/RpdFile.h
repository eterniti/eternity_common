#ifndef __RPDFILE_H__
#define __RPDFILE_H__

// TODO: this class needs a rewrite

#include <stdint.h>

#include <string>
#include <vector>

#include "CmsFile.h"
#include "SlotsFile.h"

// "#RPD"
#define RPD_SIGNATURE	0x44505223

typedef struct
{
	uint32_t signature; // 0
	uint16_t endianess_check; // 4
	uint16_t unk_06; // 6   zero
	uint32_t num_entries; // 8
	uint32_t data_start; // 0x0C
	// 0x10
} __attribute__((packed)) RPDHeader;

typedef struct
{
	uint32_t name_offset; // 0    Absolute offset. (Name doesn't seem to be used by the game...)
	uint32_t unk_04; // 4
	float unk_08; // 8
	uint32_t unk_0C; // C  seems tobe always 0?
	float unk_10; // 0x10 
	uint32_t unk_14; // 0x14  seems to be always 0
	float unk_18; // 0x19
	float unk_1C; // 0x1C
	uint32_t unk_20; // 0x20
	uint32_t unk_24; // 0x24
	float unk_28; // 0x28
	uint32_t unk_2C; // 0x2C
	float unk_30; // 0x30
	uint32_t unk_34; // 0x34
	uint32_t unk_38; // 0x38
	uint32_t unk_3C; // 0x3C
	uint32_t unk_40; // 0x40
	uint32_t unk_44; // 0x44
	uint32_t unk_48; // 0x48
	uint32_t unk_4C; // 0x4C
	uint32_t unk_50; // 0x50
	uint32_t unk_54; // 0x54
	uint32_t unk_58; // 0x58
	uint32_t unk_5C; // 0x5C
} __attribute__((packed)) RPDEntry;

class RpdFile;

class RpdEntry
{
	friend class RpdFile;
	
private:
	
	bool initialized = false;
	
public:
	
	std::string name;
	uint32_t unk_04; 
	float unk_08;
	uint32_t unk_0C;
	float unk_10; 
	uint32_t unk_14;
	float unk_18; 
	float unk_1C; 
	uint32_t unk_20;
	uint32_t unk_24;
	float unk_28; 
	uint32_t unk_2C;
	float unk_30; 
	uint32_t unk_34; 
	uint32_t unk_38; 
	uint32_t unk_3C; 
	uint32_t unk_40; 
	uint32_t unk_44; 
	uint32_t unk_48; 
	uint32_t unk_4C; 
	uint32_t unk_50; 
	uint32_t unk_54; 
	uint32_t unk_58; 
	uint32_t unk_5C; 
};

class RpdFile : public BaseFile
{
	
private:

	uint8_t *buf;
	unsigned int size;
	
	RPDEntry *entries; // not-allocated	(ptr in buf)	
	uint32_t num_entries;
	
	std::vector<CharacterDef> char_map;
	
	void Reset();
	
	char *FindString(const char *str, bool show_warning=false);
	void ToNativeEntry(RPDEntry *foreign, RpdEntry *native);
	void ToForeignEntry(RpdEntry *native, RPDEntry *foreign);
	
    void GetString(std::string & str, uint64_t offset) const;
		
    void WriteEntry(TiXmlElement *root, uint32_t entry) const;
	bool ReadEntry(TiXmlElement *root, RpdEntry *entry);
	
	bool AppendStringIfNeeded(const char *str);
	uint32_t AppendDefaultEntry();
	
	char *BuildStringSection(std::vector<RpdEntry> & entry_list, unsigned int *num_strings, unsigned int *size);

public:

	RpdFile();
    RpdFile(const uint8_t *buf, size_t size);
	virtual ~RpdFile();
	
    virtual bool Load(const uint8_t *buf, size_t size) override;
	
	inline uint32_t GetNumEntries() { return num_entries; }
	
	uint32_t FindEntry(uint32_t cms_entry, uint32_t cms_model_spec_idx);
	
    RpdEntry *GetEntry(uint32_t id, uint32_t *cms_entry=nullptr, uint32_t *cms_model_spec_idx=nullptr);
	bool SetEntry(uint32_t id, RpdEntry *entry, uint32_t cms_entry=0xFFFFFFFF, uint32_t cms_model_spec_idx=0xFFFFFFFF);
	
	uint32_t AppendEntry(RpdEntry *entry, uint32_t cms_entry=0xFFFFFFFF, uint32_t cms_model_spec_idx=0xFFFFFFFF);
	bool DeleteEntry(uint32_t id);
	
	bool SetExtraDecompileData(uint8_t *global_characters_list);
	
    virtual TiXmlDocument *Decompile() const override;
	virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;
	
    virtual uint8_t *Save(size_t *size) override;
	uint8_t *CreateGlobalCharactersList(unsigned int *size, CmsFile *check=NULL);
};

#endif

