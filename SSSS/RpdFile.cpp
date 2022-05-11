#include <stdint.h>

#include <algorithm>

#include "RpdFile.h"
#include "SsssData.h"
#include "debug.h"

#define CHECK_LOADED_BOOL() { if (!this->buf) { return false; } }
#define CHECK_LOADED_U32() { if (!this->buf) { return (uint32_t)-1; } }
#define CHECK_LOADED_PTR() { if (!this->buf) { return nullptr; } }

RpdFile::RpdFile()
{
	buf = NULL;
	Reset();
}

RpdFile::RpdFile(const uint8_t *buf, size_t size)
{
	this->buf = NULL;
	Load(buf, size);
}

RpdFile::~RpdFile()
{
	Reset();
}

void RpdFile::Reset()
{
	if (buf)
	{	
		delete[] buf;
		buf = NULL;	
	}
	
	big_endian = false;	
	size = 0;	
	
	entries = NULL;
	num_entries = (uint32_t)-1;
	char_map.clear();
}

void RpdFile::GetString(std::string & str, uint64_t offset) const
{
	if (offset >= (1ULL << 32))
	{
		// TODO, replace this with exception 
		DPRINTF("No support for >= 4GB files. (offending offset = %I64x\n", offset);
		exit(-1);
	}
	
	if (offset == 0)
	{
		str = "";
		return;
	}
	
	char *cstr = (char *)GetOffsetPtr(buf, offset);	
	str = cstr;
}

char *RpdFile::FindString(const char *str, bool show_warning)
{
	char *string_section = (char *)buf + sizeof(RPDHeader) + ( num_entries * sizeof(RPDEntry) );
	char *bottom = (char *)buf + size;
	
	char *p = string_section;
	
	while (p < bottom)
	{
		size_t max_compare_len = (size_t)(bottom-p);
		
		if (strncmp(p, str, max_compare_len) == 0)
			return p;
		
		p += strlen(p) + 1;
	}
	
	if (show_warning)
	{
		DPRINTF("%s: cannot find string %s.\n", __PRETTY_FUNCTION__, str);
	}
	
	return NULL;
}

#define TN_U32(f) { native->f = val32(foreign->f); }
#define TN_F(f) { native->f = val_float(foreign->f); }
#define TN_STR(f) { GetString(native->f, val32(foreign->f##_offset)); }

void RpdFile::ToNativeEntry(RPDEntry *foreign, RpdEntry *native)
{
	TN_STR(name);
	TN_U32(unk_04);
	TN_F(unk_08);
	TN_U32(unk_0C);
	TN_F(unk_10);
	TN_U32(unk_14);
	TN_F(unk_18);
	TN_F(unk_1C);
	TN_U32(unk_20);
	TN_U32(unk_24);
	TN_F(unk_28);
	TN_U32(unk_2C);
	TN_F(unk_30);
	TN_U32(unk_34);
	TN_U32(unk_38);
	TN_U32(unk_3C);
	TN_U32(unk_40);
	TN_U32(unk_44);
	TN_U32(unk_48);
	TN_U32(unk_4C);
	TN_U32(unk_50);
	TN_U32(unk_54);
	TN_U32(unk_58);
	TN_U32(unk_5C);
}

#define TF_U32(f) { foreign->f = val32(native->f); }
#define TF_F(f) { foreign->f = val_float(native->f); }
#define TF_STR(f) { \
				if (native->f.length() == 0) \
					foreign->f##_offset = 0; \
				else { \
					foreign->f##_offset = val32(Utils::DifPointer((void *)FindString(native->f.c_str(), true), this->buf)); \
				} \
				}

void RpdFile::ToForeignEntry(RpdEntry *native, RPDEntry *foreign)
{
	TF_STR(name);
	TF_U32(unk_04);
	TF_F(unk_08);
	TF_U32(unk_0C);
	TF_F(unk_10);
	TF_U32(unk_14);
	TF_F(unk_18);
	TF_F(unk_1C);
	TF_U32(unk_20);
	TF_U32(unk_24);
	TF_F(unk_28);
	TF_U32(unk_2C);
	TF_F(unk_30);
	TF_U32(unk_34);
	TF_U32(unk_38);
	TF_U32(unk_3C);
	TF_U32(unk_40);
	TF_U32(unk_44);
	TF_U32(unk_48);
	TF_U32(unk_4C);
	TF_U32(unk_50);
	TF_U32(unk_54);
	TF_U32(unk_58);
	TF_U32(unk_5C);
}

bool RpdFile::Load(const uint8_t *buf, size_t size)
{
	Reset();	
	
	if (*(uint32_t *)buf != RPD_SIGNATURE)
		return false;
	
	this->big_endian = (buf[4] != 0xFE);
		
	this->buf = new uint8_t[size];
	if (!this->buf)
		return false;
	
	this->size = size;
	memcpy(this->buf, buf, size);	
	
	RPDHeader *hdr = (RPDHeader *)this->buf;
	
	this->entries = (RPDEntry *)GetOffsetPtr(hdr, hdr->data_start);
	this->num_entries = val32(hdr->num_entries);	
	
	return true;
}

uint32_t RpdFile::FindEntry(uint32_t cms_entry, uint32_t cms_model_spec_idx)
{
	CHECK_LOADED_U32();
	
	if (char_map.size() == 0)
	{
		DPRINTF("%s: (BAD USAGE) this function can only be used if extra data is available.\n", __PRETTY_FUNCTION__);
		return (uint32_t)-1;
	}
	
	for (uint32_t i = 0; i < num_entries; i++)
	{
		if (val32(char_map[i].cms_entry) == cms_entry && val32(char_map[i].cms_model_spec_idx) == cms_model_spec_idx)
			return i;		
	}
	
	return (uint32_t)-1;
}

#define G_U32(f) { entry->f = val32(entries[id].f); }
#define G_F(f) { entry->f = val_float(entries[id].f); }
#define G_STR(f) { GetString(entry->f, val32(entries[id].f##_offset)); }

RpdEntry *RpdFile::GetEntry(uint32_t id, uint32_t *cms_entry, uint32_t *cms_model_spec_idx)
{
	CHECK_LOADED_PTR();
	
	if (id >= num_entries)
		return NULL;

    if (cms_entry)
    {
        if (char_map.size() == 0)
            return nullptr;

        *cms_entry = char_map[id].cms_entry;
        *cms_model_spec_idx = char_map[id].cms_model_spec_idx;
    }
	
	RpdEntry *entry = new RpdEntry();
	
	G_STR(name);
	G_U32(unk_04);
	G_F(unk_08);
	G_U32(unk_0C);
	G_F(unk_10);
	G_U32(unk_14);
	G_F(unk_18);
	G_F(unk_1C);
	G_U32(unk_20);
	G_U32(unk_24);
	G_F(unk_28);
	G_U32(unk_2C);
	G_F(unk_30);
	G_U32(unk_34);
	G_U32(unk_38);
	G_U32(unk_3C);
	G_U32(unk_40);
	G_U32(unk_44);
	G_U32(unk_48);
	G_U32(unk_4C);
	G_U32(unk_50);
	G_U32(unk_54);
	G_U32(unk_58);
	G_U32(unk_5C);
	
	return entry;
}

bool RpdFile::AppendStringIfNeeded(const char *str)
{
	if (FindString(str))
		return true;
	
	uint32_t new_size = size + strlen(str) + 1;
	uint8_t *new_buf = new uint8_t[new_size];
	if (!new_buf)
	{
		DPRINTF("%s: Memory allocation error.\n", __PRETTY_FUNCTION__);
		return false;
	}
	
	memcpy(new_buf, buf, size);
	strcpy((char *)new_buf+size, str);
	
	delete[] buf;
	buf = new_buf;
	size = new_size;
	
	RPDHeader *hdr = (RPDHeader *)buf;	
	entries = (RPDEntry *)GetOffsetPtr(hdr, hdr->data_start);
	
	return true;
}

bool RpdFile::SetEntry(uint32_t id, RpdEntry *entry, uint32_t cms_entry, uint32_t cms_model_spec_idx)
{
	CHECK_LOADED_BOOL();
	
	if (id >= num_entries)
		return false;
	
	if (!AppendStringIfNeeded(entry->name.c_str()))
		return false;
	
	ToForeignEntry(entry, &entries[id]);
	
	if (char_map.size() != 0)
	{
		char_map[id].cms_entry = val32(cms_entry);
		char_map[id].cms_model_spec_idx = val32(cms_model_spec_idx);
	}
	
	return true;
}

uint32_t RpdFile::AppendDefaultEntry()
{
	RPDEntry def_entry;
	
	memset(&def_entry, 0, sizeof(RPDEntry));
	
	uint32_t new_size = size + sizeof(RPDEntry);
	uint8_t *new_buf = new uint8_t[new_size];
	if (!new_buf)
	{
		DPRINTF("%s: Memory allocation error.\n", __PRETTY_FUNCTION__);
		return false;
	}
	
	uint32_t data_section_size = sizeof(RPDHeader) + ( num_entries * sizeof(RPDEntry) );
	uint8_t *string_section = buf + data_section_size;
	uint32_t string_section_size = size - Utils::DifPointer(string_section, buf);
	
	memcpy(new_buf, buf, data_section_size);
	memcpy(new_buf+data_section_size, &def_entry, sizeof(RPDEntry));
	memcpy(new_buf+data_section_size+sizeof(RPDEntry), string_section, string_section_size);
	
	delete[] buf;	
	buf = new_buf;
	size = new_size;
	
	uint32_t ret = num_entries;
	num_entries++;
	
	RPDHeader *hdr = (RPDHeader *)buf;	
	entries = (RPDEntry *)GetOffsetPtr(hdr, hdr->data_start);
	hdr->num_entries = val32(num_entries);	
	
	for (uint32_t i = 0; i < (num_entries-1); i++)
	{
		if (entries[i].name_offset != 0)
		{
			ModTable(&entries[i].name_offset, 1, sizeof(RPDEntry));
		}			
	}
	
	if (char_map.size() != 0)
	{
		CharacterDef def;
		
		def.cms_entry = val32(0x6F);
		def.cms_model_spec_idx = val32(0);
		
		char_map.push_back(def);
	}
	
	return ret;
}

uint32_t RpdFile::AppendEntry(RpdEntry *entry, uint32_t cms_entry, uint32_t cms_model_spec_idx)
{
	CHECK_LOADED_U32();
	
	uint32_t id = AppendDefaultEntry();
	if (id == (uint32_t)-1)
		return id;
	
	if (!SetEntry(id, entry, cms_entry, cms_model_spec_idx))
		return (uint32_t)-1;
	
	return id;
}

bool RpdFile::DeleteEntry(uint32_t id)
{
	CHECK_LOADED_BOOL();
	
	if (id >= num_entries)
		return false;
	
	if (id != (num_entries-1))
	{		
		RPDEntry *src = entries + id + 1;
		RPDEntry *dst = entries + id;		
		size_t size = (num_entries - id - 1) * sizeof(RPDEntry);
		
		memmove(dst, src, size);
	}
	
	uint8_t *string_section = buf + sizeof(RPDHeader) + ( num_entries * sizeof(RPDEntry) );
	uint32_t string_section_size = size - Utils::DifPointer(string_section, buf);
	
	memmove(string_section-sizeof(RPDEntry), string_section, string_section_size);
	
	num_entries--;
	size = size - sizeof(RPDEntry);
	
	RPDHeader *hdr = (RPDHeader *)buf;
	hdr->num_entries = val32(num_entries);
	
	for (uint32_t i = 0; i < num_entries; i++)
	{
		if (entries[i].name_offset != 0)
		{
			ModTable(&entries[i].name_offset, 1, -sizeof(RPDEntry));
		}
	}
	
	if (char_map.size() != 0)
	{
		char_map.erase(char_map.begin()+id);
	}
	
	return true;
}

#define W_U32(n, f) { Utils::WriteParamUnsigned(entry_root, n, val32(entry->f), true); }
#define W_F(n, f) { Utils::WriteParamFloat(entry_root, n, val_float(entry->f)); }
#define W_STR(n, f) { GetString(str, entry->f##_offset); Utils::WriteParamString(entry_root, n, str); }

void RpdFile::WriteEntry(TiXmlElement *root, uint32_t entry_idx) const
{
	RPDEntry *entry = &entries[entry_idx];		
	TiXmlElement *entry_root = new TiXmlElement("ModelRpdEntry");
	std::string str;
	
	entry_root->SetAttribute("id", Utils::UnsignedToString(entry_idx, true));
	
	if (char_map.size() != 0)
	{
		SsssCharInfo *ssss_info = SsssData::FindInfo(val32(char_map[entry_idx].cms_entry), val32(char_map[entry_idx].cms_model_spec_idx));
	
		if (ssss_info)
		{
			Utils::WriteComment(entry_root, std::string(" ") + ssss_info->model_name + std::string(" / ") + ssss_info->char_name);	
		}
		
		Utils::WriteParamUnsigned(entry_root, "CMS_ENTRY", val32(char_map[entry_idx].cms_entry), true);
		Utils::WriteParamUnsigned(entry_root, "CMS_MODEL_SPEC_IDX", val32(char_map[entry_idx].cms_model_spec_idx));
	}
	
	W_STR("NAME", name);
	W_U32("U_04", unk_04);
	W_F("F_08", unk_08);
	W_U32("U_0C", unk_0C);
	W_F("F_10", unk_10);
	W_U32("U_14", unk_14);
	W_F("F_18", unk_18);
	W_F("F_1C", unk_1C);
	W_U32("U_20", unk_20);
	W_U32("U_24", unk_24);
	W_F("F_28", unk_28);
	W_U32("U_2C", unk_2C);
	W_F("F_30", unk_30);
	W_U32("U_34", unk_34);
	W_U32("U_38", unk_38);
	W_U32("U_3C", unk_3C);
	W_U32("U_40", unk_40);
	W_U32("U_44", unk_44);
	W_U32("U_48", unk_48);
	W_U32("U_4C", unk_4C);
	W_U32("U_50", unk_50);
	W_U32("U_54", unk_54);
	W_U32("U_58", unk_58);
	W_U32("U_5C", unk_5C);
	
	root->LinkEndChild(entry_root);
}

#define R_U32(n, f) { if (!Utils::GetParamUnsigned(root, n, &entry->f)) \
							return false; }
						
#define R_F(n, f) { if (!Utils::GetParamFloat(root, n, &entry->f)) \
							return false; }
						
#define R_STR(n, f) { if (!Utils::GetParamString(root, n, entry->f)) \
							return false; }

bool RpdFile::ReadEntry(TiXmlElement *root, RpdEntry *entry)
{
	CharacterDef def;
	
	if (Utils::ReadParamUnsigned(root, "CMS_ENTRY", &def.cms_entry))
	{
		if (!Utils::GetParamUnsigned(root, "CMS_MODEL_SPEC_IDX", &def.cms_model_spec_idx))
			return false;
		
		char_map.push_back(def);
	}
	
	R_STR("NAME", name);
	R_U32("U_04", unk_04);
	R_F("F_08", unk_08);
	R_U32("U_0C", unk_0C);
	R_F("F_10", unk_10);
	R_U32("U_14", unk_14);
	R_F("F_18", unk_18);
	R_F("F_1C", unk_1C);
	R_U32("U_20", unk_20);
	R_U32("U_24", unk_24);
	R_F("F_28", unk_28);
	R_U32("U_2C", unk_2C);
	R_F("F_30", unk_30);
	R_U32("U_34", unk_34);
	R_U32("U_38", unk_38);
	R_U32("U_3C", unk_3C);
	R_U32("U_40", unk_40);
	R_U32("U_44", unk_44);
	R_U32("U_48", unk_48);
	R_U32("U_4C", unk_4C);
	R_U32("U_50", unk_50);
	R_U32("U_54", unk_54);
	R_U32("U_58", unk_58);
	R_U32("U_5C", unk_5C);
	
	return true;
}

bool RpdFile::SetExtraDecompileData(uint8_t *global_characters_list)
{
	CHECK_LOADED_BOOL();
	
	char_map.clear();
	
	uint8_t *ptr = (uint8_t *)global_characters_list;
	
	while (*ptr != val32(0x6F))
	{
		CharacterDef def;
		
		memcpy(&def, ptr, sizeof(CharacterDef));
		ptr += sizeof(CharacterDef);
		
		char_map.push_back(def);
	}

	if (char_map.size() != num_entries)
	{
		DPRINTF("global_characters_list doesn't have same amount of elements as this rdp.\n");
		char_map.clear();
		return false;		
	}
	
	return true;
}

TiXmlDocument *RpdFile::Decompile() const
{
	CHECK_LOADED_PTR();
	
	TiXmlDocument *doc = new TiXmlDocument();
	
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );	
	doc->LinkEndChild(decl);
	
	TiXmlElement *root = new TiXmlElement("RPD");
	
	if (char_map.size() != 0)
	{
		Utils::WriteComment(root, " This file has machine generated comments. Any change to these comments will be lost on next decompilation. ");		
	}
	
	for (uint32_t i = 0; i < num_entries; i++)
	{
		WriteEntry(root, i);
	}
	
	doc->LinkEndChild(root);
	
	return doc;
}

#define ADD_STR(f) { if (e.f.length() != 0 && std::find(str_list.begin(), str_list.end(), e.f) == str_list.end()) { str_list.push_back(e.f); section_size += (e.f.length() + 1);} }

char *RpdFile::BuildStringSection(std::vector<RpdEntry> & entry_list, unsigned int *num_strings, unsigned int *size)
{
	std::vector<std::string> str_list;
	size_t section_size = 0;
	char *buf, *ptr;
	
	for (RpdEntry &e : entry_list)
	{
		ADD_STR(name);
	}
	
	buf = new char[section_size];
	if (!buf)
		return NULL;
	
	memset(buf, 0, section_size);
	
	ptr = buf;
	
	for (std::string &s : str_list)
	{
		strcpy(ptr, s.c_str());
		ptr += strlen(ptr) + 1;
	}
	
	*num_strings = str_list.size();
	*size = section_size;	
	return buf;
}

#define C_U32(f) { re.f = val32(e.f); }
#define C_F(f) { re.f = val_float(e.f); }
#define C_STR(f) { \
				if (e.f.length() == 0) \
					re.f##_offset = 0; \
				else { \
					re.f##_offset = val32(Utils::DifPointer((void *)BaseFile::FindString((char *)this->buf+data_section_size, e.f.c_str(), num_strings), this->buf)); \
				} \
				}

bool RpdFile::Compile(TiXmlDocument *doc, bool big_endian)
{
	unsigned int num_entries = 0;
	std::vector<RpdEntry> entries;
	char *str_section;
	unsigned int num_strings;
	unsigned int str_section_size, data_section_size;
	
	Reset();
	this->big_endian = big_endian;
	
	TiXmlHandle handle(doc);
	TiXmlElement *root = NULL;;
	
	for (TiXmlElement *elem = handle.FirstChildElement().Element(); elem != NULL; elem = elem->NextSiblingElement())
	{
		std::string name = elem->ValueStr();
		
		if (name == "RPD")
		{
			root = elem;
			break;
		}
	}		
	
	if (!root)
	{
		DPRINTF("Cannot find \"RPD\" in xml.\n");
		return false;
	}
	
	// First pass to know number of entries, so that we can do later the index check.
	for (TiXmlElement *elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{
		std::string name = elem->ValueStr();
		
		if (name == "ModelRpdEntry")
		{
			num_entries++;
		}
	}
	
	if (num_entries == 0)
	{
		DPRINTF("No ModelRpdEntry found!\n");
		return false;
	}
	
	entries.resize(num_entries);
	
	for (TiXmlElement *elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{
		std::string name = elem->ValueStr();
		
		if (name == "ModelRpdEntry")
		{
			uint32_t idx;
			
			if (!Utils::ReadAttrUnsigned(elem, "id", &idx))
			{
				DPRINTF("Cannot read \"id\" attribute.\n");
				return false;
			}
			
			if (idx >= num_entries)
			{
				DPRINTF("id out of bounds (0x%x, number of entries=0x%x))\nid is a pseudo field (index) that must be between 0 and number of entries-1.\n", idx, num_entries);
				return false;
			}
			
			if (entries[idx].initialized)
			{
				DPRINTF("Element with id=0x%x already existed. id is a pseudo field(index) that must be unique and between 0 and number of entries-1.\n", idx);
				return false;
			}
			
			if (!ReadEntry(elem, &entries[idx]))
				return false;

			entries[idx].initialized = true;
		}
	}
	
	if (char_map.size() != 0 && char_map.size() != num_entries)
	{
		DPRINTF("CMS extra information is incomplete. It is in some characters and not in other!\n");
		char_map.clear();
		return false;
	}
	
	str_section = BuildStringSection(entries, &num_strings, &str_section_size);
	if (!str_section)
	{
		DPRINTF("We are short in memory!\n");
		return false;
	}
	
	data_section_size = sizeof(RPDHeader) + ( num_entries * sizeof(RPDEntry) );
	
	this->size = data_section_size+str_section_size;
	this->buf = new uint8_t[this->size];
	if (!this->buf)
	{
		DPRINTF("We are short in memory.\n");
		this->size = 0;
		return false;
	}
	
	memset(this->buf, 0, this->size);
	memcpy(this->buf+data_section_size, str_section, str_section_size);
	
	uint8_t *ptr = this->buf + sizeof(RPDHeader);
	
	for (RpdEntry &e: entries)
	{
		RPDEntry re;
		
		C_STR(name);
		C_U32(unk_04);
		C_F(unk_08);
		C_U32(unk_0C);
		C_F(unk_10);
		C_U32(unk_14);
		C_F(unk_18);
		C_F(unk_1C);
		C_U32(unk_20);
		C_U32(unk_24);
		C_F(unk_28);
		C_U32(unk_2C);
		C_F(unk_30);
		C_U32(unk_34);
		C_U32(unk_38);
		C_U32(unk_3C);
		C_U32(unk_40);
		C_U32(unk_44);
		C_U32(unk_48);
		C_U32(unk_4C);
		C_U32(unk_50);
		C_U32(unk_54);
		C_U32(unk_58);
		C_U32(unk_5C);
		
		memcpy(ptr, &re, sizeof(RPDEntry));
		ptr += sizeof(RPDEntry);
	}
	
	// And finally, the header;	
	RPDHeader *hdr = (RPDHeader *)this->buf;
	
	hdr->signature = RPD_SIGNATURE;
	hdr->endianess_check = val32(0xFFFE);
	hdr->num_entries = val32(num_entries);
	hdr->data_start = val32(sizeof(RPDHeader));
	
	this->entries = (RPDEntry *)GetOffsetPtr(hdr, hdr->data_start);
	this->num_entries = val32(hdr->num_entries);	
	
	delete[] str_section;
	
	return true;
}

uint8_t *RpdFile::Save(size_t *size)
{
	CHECK_LOADED_PTR();
	
	uint8_t *buf = new uint8_t[this->size];

	memcpy(buf, this->buf, this->size);
	*size = this->size;
	
	return buf;
}

uint8_t *RpdFile::CreateGlobalCharactersList(unsigned int *size, CmsFile *check)
{
	CHECK_LOADED_PTR();
	
	if (char_map.size() == 0)
		return NULL;
	
	*size = (char_map.size()+1) * sizeof(CharacterDef);
	
	uint8_t *buf = new uint8_t[*size];
	if (!buf)
	{
		DPRINTF("%s: Memory allocation error: 0x%x\n", __PRETTY_FUNCTION__, *size);
		return NULL;
	}
	
	uint8_t *ptr = buf;
	
	for (CharacterDef &d : char_map)
	{
		if (check)
		{
			if (!check->ModelExists(d.cms_entry, d.cms_model_spec_idx))
			{
				DPRINTF("%s: Model 0x%x %d doesn't exist in the .cms.\n", __PRETTY_FUNCTION__, d.cms_entry, d.cms_model_spec_idx);
				delete[] buf;
				return NULL;
			}
		}
		
		memcpy(ptr, &d, sizeof(CharacterDef));
		ptr += sizeof(CharacterDef);
	}
	
	CharacterDef eol;
	
	eol.cms_entry = val32(0x6F);
	eol.cms_model_spec_idx = (uint32_t)-1;
	
	memcpy(ptr, &eol, sizeof(CharacterDef));
	
	return buf;
}

