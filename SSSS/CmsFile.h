#ifndef __CMSFILE_H__
#define __CMSFILE_H__

// TODO: This class needs a revamp.

#include <stdint.h>
#include <string>
#include <vector>

#include "BaseFile.h"

// "#CMS"
#define CMS_SIGNATURE	0x534D4323

typedef struct
{
	uint32_t signature; // 0
	uint16_t endianess_check; // 4
	uint16_t unk_06; // 6       zero
	uint32_t num_entries; // 8
	uint32_t unk_0C;  // C      zero 
	uint32_t data_start; // 0x10
	// pad until data_start
} __attribute__((packed)) CMSHeader;

typedef struct
{
	uint32_t id; // 0
	char name[4]; // 4
	uint32_t type; // 8
	uint32_t num_models; // C
	uint32_t models_spec_offset; // 0x10   offset is absolute!
	uint32_t unk_14[3]; // 0x14   always zero
	// 0x20
} __attribute__((packed)) CMSEntry;

typedef struct
{
	// all offsets are absolute within the file!
	uint64_t bba0_name_offset; // 0 
	uint64_t bba1_name_offset; // 8
	uint64_t bba2_name_offset; // 0x10
	uint64_t bba3_name_offset; // 0x18
	uint64_t bba4_name_offset; // 0x20
	uint64_t bba5_name_offset; // 0x28
	uint64_t emo_file_offset; // 0x30
	uint64_t emb_file_offset; // 0x38
	uint64_t emm_file_offset; // 0x40
	uint64_t ema_file_offset; // 0x48
	uint64_t menu_file_offset; // 0x50
	uint64_t unk_58; // 0x58 always 0
	uint64_t fce_file_offset; // 0x60
	uint64_t matbas_file_offset; // 0x68
	uint64_t vc_file_offset; // 0x70
	uint64_t _2ry_file_offset; // 0x78
	uint64_t fma_file_offset; // 0x80
	uint64_t fdm_file_offset; // 0x88
	uint64_t fcm_file_offset; // 0x90
	uint64_t unk_98; // 0x98 always 0
	uint64_t vfx_file_offset; // 0xA0
	uint64_t tdb_file_offset; // 0xA8
	uint64_t bpm_file_offset; // 0xB0
	uint64_t vc_name_offset; // 0xB8
	uint64_t se_name_offset; // 0xC0
	uint32_t idx; // 0xC8  seems important
	uint32_t model_id; // 0xCC  this is also important 
	uint32_t unk_D0; // 0xD0  not 0
	uint32_t unk_D4; // 0xD4  not 0
	float 	scale; // 0xD8  possibly, scale
	uint32_t cosmo; // 0xDC  
	float unk_E0; // 0xE0 
	float unk_E4; // 0xE4
	float unk_E8; // 0xE8
	float unk_EC; // 0xEC
	float unk_F0; // 0xF0
	uint32_t aura; // 0xF4 
	uint64_t unk_F8; // always 0
	uint64_t unk_100; // always 0
	// size = 0x108
} __attribute__((packed)) CMSModelSpec;

class ModelSpec
{
public:
	std::string bba0;
	std::string bba1;
	std::string bba2;
	std::string bba3;
	std::string bba4;
	std::string bba5;
	std::string emo_file;
	std::string emb_file;
	std::string emm_file;
	std::string ema_file;
	std::string menu_file;
	std::string fce_file;
	std::string matbas_file;
	std::string vc_file;
	std::string _2ry_file;
	std::string fma_file;
	std::string fdm_file;
	std::string fcm_file;
	std::string vfx_file;
	std::string tdb_file;
	std::string bpm_file;
	std::string vc_name;
	std::string se_name;
	uint32_t idx;
	uint32_t model_id;
	uint32_t unk_D0;
	uint32_t unk_D4;
	float scale;
	uint32_t cosmo;
	float unk_E0;
	float unk_E4;
	float unk_E8;
	float unk_EC;
	float unk_F0;
	uint32_t aura; 	
};

class CmsEntry
{
public:
	CMSEntry cms_entry;
	std::vector<ModelSpec> specs;
};

class CmsFile : public BaseFile
{
private:

	uint8_t *buf;
	unsigned int size;
	
	CMSEntry *entries; // not-allocated	(ptr in buf)	
	uint32_t num_entries;
	
	std::vector<CmsEntry> native_entries;
		
	void Reset();	
			
    void GetString(std::string & str, uint64_t offset) const;
	
    void GetNativeModelSpec(CMSModelSpec *cms, ModelSpec *ms) const;
	ModelSpec *FindNativeModelSpec(uint32_t entry_id, uint32_t model_spec_idx);
	// This uses indexes, not entry_id + model_spec_idx 
    ModelSpec *GetModelSpecInternal(uint32_t entry, uint32_t model) const;
	
	void Decompose(std::vector<CmsEntry> & entry_list);
	void DecomposeIfNeeded();
	
    void WriteEntry(TiXmlElement *root, uint32_t entry) const;
    void WriteSpec(TiXmlElement *root, ModelSpec *spec) const;
	
	bool ReadEntry(TiXmlElement *root, CmsEntry *entry);
	bool ReadSpec(TiXmlElement *root, ModelSpec *spec);
	
	char *BuildStringSection(std::vector<CmsEntry> & entry_list, unsigned int *num_strings, unsigned int *size);	
	bool BuildBinary(std::vector<CmsEntry> & entry_list);
    bool BuildIfNeeded(bool clear);
	
public:

	CmsFile();
	CmsFile(const uint8_t *buf, size_t size);
	virtual ~CmsFile();
	
    virtual bool Load(const uint8_t *buf, size_t size) override;
	
	inline uint32_t GetNumEntries() { return (native_entries.size() == 0) ? num_entries : native_entries.size(); }	
	// Uses entry_id, not index
	uint32_t GetNumModels(uint32_t entry_id);	
	
	char *GetEntryNameById(uint32_t entry_id);
	uint32_t FindEntryByName(const char *name);	
	uint32_t FindHighestIdx(uint32_t entry_id);
	
	// This one uses entry_id and model_spec_idx, not indexes 
	ModelSpec *GetModelSpec(uint32_t entry_id, uint32_t model_spec_idx);	
	ModelSpec *GetModelSpecFromModelId(uint32_t model_id, uint32_t *entry_id=NULL);
	
	// TODO: make a more efficient implementation
	inline bool ModelExists(uint32_t entry_id, uint32_t model_idx) 
	{
		ModelSpec *spec = GetModelSpec(entry_id, model_idx); 
		if (!spec)
			return false;
		
		delete spec;
		return true;
	}
	
    bool SetModelSpec(uint32_t entry_id, ModelSpec *spec);
    bool AppendModel(uint32_t entry_id, ModelSpec *spec);
	bool DeleteModel(uint32_t entry_id, uint32_t model_spec_idx);

    int DecreaseAllModelId(uint32_t from);
	
    virtual TiXmlDocument *Decompile() const override;
	virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;
	
	virtual uint8_t *Save(size_t *size) override;
};

#endif
