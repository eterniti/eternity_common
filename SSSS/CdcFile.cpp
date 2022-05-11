#include <stdint.h>

#include "CdcFile.h"
#include "SsssData.h"
#include "debug.h"

#define CHECK_LOADED_BOOL() { if (!this->buf) { return false; } }
#define CHECK_LOADED_U32() { if (!this->buf) { return (uint32_t)-1; } }
#define CHECK_LOADED_PTR() { if (!this->buf) { return NULL; } }

CdcFile::CdcFile()
{
	buf = NULL;
	big_endian = false;
	Reset();
}

CdcFile::CdcFile(uint8_t *buf, unsigned int size)
{
	this->buf = NULL;
	Load(buf, size);
}

CdcFile::~CdcFile()
{
	Reset();
}

void CdcFile::Reset()
{
	if (buf)
	{	
		delete[] buf;
		buf = NULL;	
	}
	
	size = 0;	
	
	entries = NULL;
	num_entries = (uint32_t)-1;
	char_map.clear();
}

bool CdcFile::Load(const uint8_t *buf, size_t size)
{
	Reset();	
	
	if (*(uint32_t *)buf != CDC_SIGNATURE)
		return false;
	
	this->big_endian = (buf[4] != 0xFE);
		
	this->buf = new uint8_t[size];
	if (!this->buf)
		return false;
	
	this->size = size;
	memcpy(this->buf, buf, size);	
	
	CDCHeader *hdr = (CDCHeader *)this->buf;
	
	this->entries = (CDCEntry *)GetOffsetPtr(hdr, hdr->data_start);
	this->num_entries = val32(hdr->num_entries);	
	
	return true;
}

CdcEntry *CdcFile::GetModelEntry(uint32_t id, uint32_t *cms_entry, uint32_t *cms_model_spec_idx)
{
	CHECK_LOADED_PTR();
	
	if (id >= num_entries)
		return NULL;
	
	if (cms_entry)
	{
		if (char_map.size() == 0)
			return NULL;
		
		*cms_entry = val32(char_map[id].cms_entry);
		*cms_model_spec_idx = val32(char_map[id].cms_model_spec_idx);
	}
	
	CdcEntry *entry = new CdcEntry();
	
	entry->char_id = val32(entries[id].char_id);
	entry->name_id = val32(entries[id].name_id);
	entry->short_name_id = val32(entries[id].short_name_id);
	entry->sign_name_id = val32(entries[id].sign_name_id);
	entry->robes_name_id = val32(entries[id].robes_name_id);
	entry->unk_14 = val32(entries[id].unk_14);
	entry->robes_type = val32(entries[id].robes_type);
	entry->is_dlc = val32(entries[id].is_dlc);
	
	return entry;
}

bool CdcFile::SetModelEntry(uint32_t id, CdcEntry *entry, uint32_t cms_entry, uint32_t cms_model_spec_idx)
{
	CHECK_LOADED_BOOL();
	
	if (id >= num_entries)
		return false;
	
	if (char_map.size() > 0)
	{
		char_map[id].cms_entry = val32(cms_entry);
		char_map[id].cms_model_spec_idx = val32(cms_model_spec_idx);
	}
	
	entries[id].char_id = val32(entry->char_id);
	entries[id].name_id = val32(entry->name_id);
	entries[id].short_name_id = val32(entry->short_name_id);
	entries[id].sign_name_id = val32(entry->sign_name_id);
	entries[id].robes_name_id = val32(entry->robes_name_id);
	entries[id].unk_14 = val32(entry->unk_14);
	entries[id].robes_type = val32(entry->robes_type);
	entries[id].is_dlc = val32(entry->is_dlc);
	
	return true;
}

bool CdcFile::CreateNewEntry()
{
	CHECK_LOADED_BOOL();
	
	uint32_t new_size = size+sizeof(CDCEntry);
	uint8_t *new_buf = new uint8_t[new_size];
	
	if (!new_buf)
	{
		DPRINTF("%s: Memory allocation error.\n", FUNCNAME);
		return false;
	}
	
	memcpy(new_buf, buf, size);	
	delete[] buf;
	
	buf = new_buf;
	size = new_size;
	
	CDCHeader *hdr = (CDCHeader *)buf;
	
	entries = (CDCEntry *)GetOffsetPtr(hdr, hdr->data_start);
	num_entries++;	
	hdr->num_entries = val32(num_entries);
	
	CDCEntry *entry = entries + num_entries - 1;
	
	memset(entry, 0xFF, sizeof(CDCEntry));
	entry->name_id = entry->short_name_id = val32(0x1B9);
	entry->sign_name_id = entry->robes_name_id = val32(0x1B8);
	entry->is_dlc = 0;
	
	if (char_map.size() != 0)
	{
		CharacterDef def;
		
		memset(&def, 0xFF, sizeof(CharacterDef));
		char_map.push_back(def);
	}
	
	return true;
}

bool CdcFile::DeleteEntry(uint32_t id)
{
	CHECK_LOADED_BOOL();
	
	if (id >= num_entries)
		return false;
	
	if (id != (num_entries-1))
	{		
		CDCEntry *src = entries + id + 1;
		CDCEntry *dst = entries + id;		
		size_t size = (num_entries - id - 1) * sizeof(CDCEntry);
		
		memmove(dst, src, size);
	}
	
	num_entries--;
	size = size - sizeof(CDCEntry);
	
	CDCHeader *hdr = (CDCHeader *)buf;
	hdr->num_entries = val32(num_entries);
	
	if (char_map.size() != 0)
	{
		char_map.erase(char_map.begin() + id);
	}

    for (; id < num_entries; id++)
    {
        if (entries[id].unk_14 != 0xFFFFFFFF)
            entries[id].unk_14 = val32(val32(entries[id].unk_14)-1);
    }
	
	return true;
}

void CdcFile::WriteEntry(TiXmlElement *root, uint32_t entry_idx) const
{
	CDCEntry *entry = &entries[entry_idx];		
	TiXmlElement *entry_root = new TiXmlElement("ModelEntry");
	SsssCharInfo *ssss_info = SsssData::FindInfo(entry_idx);	
	
	entry_root->SetAttribute("id", Utils::UnsignedToString(entry_idx, true));
	
	if (ssss_info)
	{
		Utils::WriteComment(entry_root, std::string(" ") + ssss_info->model_name + std::string(" / ") + ssss_info->char_name);	
	}
	
	
	if (char_map.size() != 0)
	{
		Utils::WriteParamUnsigned(entry_root, "CMS_ENTRY", val32(char_map[entry_idx].cms_entry), true);
		Utils::WriteParamUnsigned(entry_root, "CMS_MODEL_SPEC_IDX", val32(char_map[entry_idx].cms_model_spec_idx));
	}
	
	Utils::WriteParamUnsigned(entry_root, "CHAR_ID", val32(entry->char_id), true);
	Utils::WriteParamUnsigned(entry_root, "NAME_ID", val32(entry->name_id), true);
	Utils::WriteParamUnsigned(entry_root, "SHORT_NAME_ID", val32(entry->short_name_id), true);
	Utils::WriteParamUnsigned(entry_root, "SIGN_NAME_ID", val32(entry->sign_name_id), true);
	Utils::WriteParamUnsigned(entry_root, "ROBES_NAME_ID", val32(entry->robes_name_id), true);
	Utils::WriteParamUnsigned(entry_root, "U_14", val32(entry->unk_14), true);
	Utils::WriteParamUnsigned(entry_root, "ROBES_TYPE", val32(entry->robes_type));
	Utils::WriteParamUnsigned(entry_root, "IS_DLC", val32(entry->is_dlc));
	
	root->LinkEndChild(entry_root);
}

#define ENTRY_U(n, f) { if (!Utils::GetParamUnsigned(root, n, &temp)) \
							return false; \
						entry->f = temp; \
						}

bool CdcFile::ReadEntry(TiXmlElement *root, CdcEntry *entry)
{
	uint32_t temp;
	CharacterDef def;
	
	if (Utils::ReadParamUnsigned(root, "CMS_ENTRY", &def.cms_entry))
	{
		if (!Utils::GetParamUnsigned(root, "CMS_MODEL_SPEC_IDX", &def.cms_model_spec_idx))
			return false;
		
		char_map.push_back(def);
	}
	
	ENTRY_U("CHAR_ID", char_id);
	ENTRY_U("NAME_ID", name_id);
	ENTRY_U("SHORT_NAME_ID", short_name_id);
	ENTRY_U("SIGN_NAME_ID", sign_name_id);
	ENTRY_U("ROBES_NAME_ID", robes_name_id);
	ENTRY_U("U_14", unk_14);
	ENTRY_U("ROBES_TYPE", robes_type);
	ENTRY_U("IS_DLC", is_dlc);
	
	return true;
}

bool CdcFile::SetExtraDecompileData(CmsFile *cms)
{
	CHECK_LOADED_BOOL();
	
	char_map.clear();
	char_map.reserve(num_entries);
	
	for (uint32_t i = 0; i < (num_entries-1); i++)
	{
		uint32_t entry_id;
		ModelSpec *spec = cms->GetModelSpecFromModelId(i, &entry_id);
		CharacterDef def;
		
		if (!spec)
		{
			DPRINTF("%s: cannot find model_id %d\n", FUNCNAME, i);
			char_map.clear();
			return false;
		}
		
		def.cms_entry = val32(entry_id);
		def.cms_model_spec_idx = val32(spec->idx);
		
		delete spec;
		char_map.push_back(def);
	}
	
	// Special last entry
	
	CharacterDef last_def;
	last_def.cms_entry = 0xFFFFFFFF;
	last_def.cms_model_spec_idx = 0xFFFFFFFF;
	
	char_map.push_back(last_def);	
	return true;
}

TiXmlDocument *CdcFile::Decompile() const
{
	CHECK_LOADED_PTR();
	
	TiXmlDocument *doc = new TiXmlDocument();
	
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );	
	doc->LinkEndChild(decl);
	
	TiXmlElement *root = new TiXmlElement("CDC");
	Utils::WriteComment(root, " This file has machine generated comments. Any change to these comments will be lost on next decompilation. ");
		
	for (uint32_t i = 0; i < num_entries; i++)
	{
		WriteEntry(root, i);
	}
	
	doc->LinkEndChild(root);
	
	return doc;
}

#define DO_U32(f) { ce.f = val32(e.f); }

bool CdcFile::Compile(TiXmlDocument *doc, bool big_endian)
{
	unsigned int num_entries = 0;
	std::vector<CdcEntry> entries;
	
	Reset();
	this->big_endian = big_endian;
	
	TiXmlHandle handle(doc);
	TiXmlElement *root = NULL;;
	
	for (TiXmlElement *elem = handle.FirstChildElement().Element(); elem != NULL; elem = elem->NextSiblingElement())
	{
		std::string name = elem->ValueStr();
		
		if (name == "CDC")
		{
			root = elem;
			break;
		}
	}		
	
	if (!root)
	{
		DPRINTF("%s: cannot find \"CDC\" in xml.\n", FUNCNAME);
		return false;
	}
	
	// First pass to know number of entries, so that we can do later the index check.
	for (TiXmlElement *elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{
		std::string name = elem->ValueStr();
		
		if (name == "ModelEntry")
		{
			num_entries++;
		}
	}
	
	if (num_entries == 0)
	{
		DPRINTF("%s: No ModelEntry found!\n", FUNCNAME);
		return false;
	}
	
	entries.resize(num_entries);
	
	for (TiXmlElement *elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{
		std::string name = elem->ValueStr();
		
		if (name == "ModelEntry")
		{
			uint32_t idx;
			
			if (!Utils::ReadAttrUnsigned(elem, "id", &idx))
			{
				DPRINTF("%s: cannot read \"id\" attribute.\n", FUNCNAME);
				return false;
			}
			
			if (idx >= num_entries)
			{
				DPRINTF("%s: id out of bounds (0x%x, number of entries=0x%x))\nid is a pseudo field (index) that must be between 0 and number of entries-1.\n", FUNCNAME, idx, num_entries);
				return false;
			}
			
			if (entries[idx].initialized)
			{
				DPRINTF("%s: element with id=0x%x already existed. id is a pseudo field(index) that must be unique and between 0 and number of entries-1.\n", FUNCNAME, idx);
				return false;
			}
			
			if (!ReadEntry(elem, &entries[idx]))
				return false;

			entries[idx].initialized = true;
		}
	}
	
	if (char_map.size() != 0 && char_map.size() != num_entries)
	{
		DPRINTF("%s: CMS extra information is incomplete. It is in some characters and not in other!\n", FUNCNAME);
		char_map.clear();
		return false;
	}
	
	this->size = sizeof(CDCHeader) + num_entries * sizeof(CDCEntry);
	this->buf = new uint8_t[this->size];
	if (!this->buf)
	{
		DPRINTF("%s: We are short in memory.\n", FUNCNAME);
		this->size = 0;
		return false;
	}
	
	memset(this->buf, 0, this->size);
	
	uint8_t *ptr = this->buf + sizeof(CDCHeader);
	
	for (CdcEntry &e: entries)
	{
		CDCEntry ce;
		
		DO_U32(char_id);
		DO_U32(name_id);
		DO_U32(short_name_id);
		DO_U32(sign_name_id);
		DO_U32(robes_name_id);
		DO_U32(unk_14);
		DO_U32(robes_type);
		DO_U32(is_dlc);
		
		memcpy(ptr, &ce, sizeof(CDCEntry));
		ptr += sizeof(CDCEntry);
	}
	
	// And finally, the header;	
	CDCHeader *hdr = (CDCHeader *)this->buf;
	
	hdr->signature = CDC_SIGNATURE;
	hdr->endianess_check = val32(0xFFFE);
	hdr->num_entries = val32(num_entries);
	hdr->data_start = val32(sizeof(CDCHeader));
	
	this->entries = (CDCEntry *)GetOffsetPtr(hdr, hdr->data_start);
	this->num_entries = val32(hdr->num_entries);	
	
	return true;
}

uint8_t *CdcFile::Save(size_t *size)
{
	CHECK_LOADED_PTR();
	
	uint8_t *buf = new uint8_t[this->size];
		
	memcpy(buf, this->buf, this->size);
	*size = this->size;
	
	return buf;
}

