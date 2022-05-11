#include <stdlib.h>
#include <stdio.h>

#include <algorithm>

#include "CmsFile.h"
#include "SsssData.h"
#include "debug.h"

#define CHECK_LOADED_BOOL() { if (!this->buf) { return false; } }
#define CHECK_LOADED_U32() { if (!this->buf) { return (uint32_t)-1; } }
#define CHECK_LOADED_INT() { if (!this->buf) { return -1; } }
#define CHECK_LOADED_PTR() { if (!this->buf) { return NULL; } }

CmsFile::CmsFile()
{
	buf = NULL;
	big_endian = false;
	Reset();
}

CmsFile::CmsFile(const uint8_t *buf, size_t size)
{
	this->buf = NULL;
	Load(buf, size);
}

CmsFile::~CmsFile()
{
	Reset();
}

void CmsFile::Reset()
{
	if (buf)
	{	
		delete[] buf;
		buf = NULL;	
	}
	
	size = 0;

	entries = NULL;
	num_entries = (uint32_t)-1;
	
	native_entries.clear();
}

bool CmsFile::Load(const uint8_t *buf, size_t size)
{
	Reset();	
	
	if (*(uint32_t *)buf != CMS_SIGNATURE)
		return false;
	
	this->big_endian = (buf[4] != 0xFE);
		
	this->buf = new uint8_t[size];
	if (!this->buf)
		return false;
	
	this->size = size;
	memcpy(this->buf, buf, size);	
	
	CMSHeader *hdr = (CMSHeader *)this->buf;
	
	this->entries = (CMSEntry *)GetOffsetPtr(hdr, hdr->data_start);
	this->num_entries = val32(hdr->num_entries);	
	
	return true;
}

uint32_t CmsFile::GetNumModels(uint32_t entry_id)
{
	CHECK_LOADED_U32();
	
	if (native_entries.size() != 0)
	{
		for (CmsEntry &e : native_entries)
		{
			if (val32(e.cms_entry.id) == entry_id)
				return e.specs.size();
		}

		return (uint32_t)-1;
	}
	
	for (uint32_t i = 0; i < num_entries; i++)
	{
		if (val32(entries[i].id) == entry_id)
			return val32(entries[i].num_models);
	}
	
	return (uint32_t)-1;
}

char *CmsFile::GetEntryNameById(uint32_t entry_id)
{
	CHECK_LOADED_PTR();
	
	if (native_entries.size() != 0)
	{
		for (CmsEntry &e: native_entries)
		{
			if (val32(e.cms_entry.id) == entry_id)
				return e.cms_entry.name;
		}
		
		return NULL;
	}
	
	for (uint32_t i = 0; i < num_entries; i++)
	{
		if (val32(entries[i].id) == entry_id)
				return entries[i].name;
	}
	
	return NULL;
}

uint32_t CmsFile::FindEntryByName(const char *name)
{
	CHECK_LOADED_U32();
	
	if (native_entries.size() != 0)
	{
		for (CmsEntry &e: native_entries)
		{
			if (strcmp(e.cms_entry.name, name) == 0)
				return val32(e.cms_entry.id);
		}
		
		return (uint32_t)-1;
	}
	
	for (uint32_t i = 0; i < num_entries; i++)
	{
		if (strcmp(entries[i].name, name) == 0)
			return val32(entries[i].id);
	}
	
	return (uint32_t)-1;
}

uint32_t CmsFile::FindHighestIdx(uint32_t entry_id)
{
	CHECK_LOADED_U32();
	
	int32_t highest = -1;
	
	if (native_entries.size() != 0)
	{
		for (CmsEntry &e: native_entries)
		{
			if (val32(e.cms_entry.id) == entry_id)
			{
				for (ModelSpec &m : e.specs)
				{
					if ((int32_t)m.idx > highest)
					{
						highest = m.idx;
					}
				}
				
				break;
			}			
		}
	}
	else
	{
		for (uint32_t i = 0; i < num_entries; i++)
		{
			if (val32(entries[i].id) == entry_id)
			{
				CMSModelSpec *specs = (CMSModelSpec *)GetOffsetPtr(buf, entries[i].models_spec_offset);
				
				for (uint32_t j = 0; j < val32(entries[i].num_models); j++)
				{
					int32_t this_idx = (int32_t)val32(specs[j].idx);
					
					if (this_idx > highest)
					{
						highest = this_idx;
					}
				}
			
				break;
			}
		}
	}
	
	return (uint32_t)highest;
}

void CmsFile::GetString(std::string & str, uint64_t offset) const
{
	if (offset >= (1ULL << 32))
	{
		// TODO, replace this with exception 
		DPRINTF("%s: no support for >= 4GB files. (offending offset = %I64x\n", FUNCNAME, offset);
		return;
	}
	
	if (offset == 0)
	{
		str = "";
		return;
	}
	
	char *cstr = (char *)GetOffsetPtr(buf, offset);	
	str = cstr;
}

void CmsFile::GetNativeModelSpec(CMSModelSpec *cms, ModelSpec *ms) const
{
	GetString(ms->bba0, val64(cms->bba0_name_offset));
	GetString(ms->bba1, val64(cms->bba1_name_offset));
	GetString(ms->bba2, val64(cms->bba2_name_offset));
	GetString(ms->bba3, val64(cms->bba3_name_offset));
	GetString(ms->bba4, val64(cms->bba4_name_offset));
	GetString(ms->bba5, val64(cms->bba5_name_offset));
	GetString(ms->emo_file, val64(cms->emo_file_offset));
	GetString(ms->emb_file, val64(cms->emb_file_offset));
	GetString(ms->emm_file, val64(cms->emm_file_offset));
	GetString(ms->ema_file, val64(cms->ema_file_offset));
	GetString(ms->menu_file, val64(cms->menu_file_offset));
	GetString(ms->fce_file, val64(cms->fce_file_offset));
	GetString(ms->matbas_file, val64(cms->matbas_file_offset));
	GetString(ms->vc_file, val64(cms->vc_file_offset));
	GetString(ms->_2ry_file, val64(cms->_2ry_file_offset));
	GetString(ms->fma_file, val64(cms->fma_file_offset));
	GetString(ms->fdm_file, val64(cms->fdm_file_offset));
	GetString(ms->fcm_file, val64(cms->fcm_file_offset));
	GetString(ms->vfx_file, val64(cms->vfx_file_offset));
	GetString(ms->tdb_file, val64(cms->tdb_file_offset));
	GetString(ms->bpm_file, val64(cms->bpm_file_offset));
	GetString(ms->vc_name, val64(cms->vc_name_offset));
	GetString(ms->se_name, val64(cms->se_name_offset));
	
	ms->idx = val32(cms->idx);
	ms->model_id = val32(cms->model_id);
	ms->unk_D0 = val32(cms->unk_D0);
	ms->unk_D4 = val32(cms->unk_D4);
	ms->scale = val_float(cms->scale);
	ms->cosmo = val32(cms->cosmo);
	ms->unk_E0 = val_float(cms->unk_E0);
	ms->unk_E4 = val_float(cms->unk_E4);
	ms->unk_E8 = val_float(cms->unk_E8);
	ms->unk_EC = val_float(cms->unk_EC);
	ms->unk_F0 = val_float(cms->unk_F0);
	ms->aura = val32(cms->aura);
	
	// Check for values that I think are always 0
	
	if (cms->unk_58 != 0)
	{
		DPRINTF("WARNING: unk_58 is not 0 as we expected (model_entry offset = %x)\n", Utils::DifPointer(cms, buf));
	}
	
	if (cms->unk_98 != 0)
	{
		DPRINTF("WARNING: unk_98 is not 0 as we expected (model_entry offset = %x)\n", Utils::DifPointer(cms, buf));
	}
	
	if (cms->unk_F8 != 0)
	{
		DPRINTF("WARNING: unk_F8 is not 0 as we expected (model_entry offset = %x)\n", Utils::DifPointer(cms, buf));
	}
	
	if (cms->unk_100 != 0)
	{
		DPRINTF("WARNING: unk_100 is not 0 as we expected (model_entry offset = %x)\n", Utils::DifPointer(cms, buf));
	}
}

ModelSpec *CmsFile::GetModelSpecInternal(uint32_t entry, uint32_t model) const
{
	CHECK_LOADED_PTR();
	
	size_t native_size = native_entries.size();
	
	if (native_size != 0)
	{
		if (entry >= native_size)
			return NULL;
		
        const CmsEntry &this_entry = native_entries[entry];
		
		if (model >= this_entry.specs.size())
			return NULL;
		
		return new ModelSpec(this_entry.specs[model]);
	}
	
	if (entry >= num_entries)
		return NULL;
	
	if (model >= val32(entries[entry].num_models))
		return NULL;
	
	CMSModelSpec *cms = (CMSModelSpec *)(GetOffsetPtr(buf, entries[entry].models_spec_offset)) + model;	
	ModelSpec *ms = new ModelSpec();
	
	if (!ms)
		return NULL;
	
	GetNativeModelSpec(cms, ms);	
	return ms;
}

ModelSpec *CmsFile::FindNativeModelSpec(uint32_t entry_id, uint32_t model_spec_idx)
{
	for (CmsEntry &e : native_entries)
	{
		if (val32(e.cms_entry.id) == entry_id)
		{
			for (ModelSpec &m : e.specs)
			{
				if (m.idx == model_spec_idx)
					return &m;
			}
		}
	}
	
	return NULL;
}

ModelSpec *CmsFile::GetModelSpec(uint32_t entry_id, uint32_t model_spec_idx)
{
	CHECK_LOADED_PTR();	
	
	if (native_entries.size() != 0)
	{			
		ModelSpec *spec = FindNativeModelSpec(entry_id, model_spec_idx);
		if (!spec)
			return NULL;
		
		return new ModelSpec(*spec);
	}
	
	CMSModelSpec *cms = NULL;
	
	for (uint32_t i = 0; i < num_entries; i++)
	{
		if (val32(entries[i].id) == entry_id)
		{
			CMSModelSpec *specs = (CMSModelSpec *)GetOffsetPtr(buf, entries[i].models_spec_offset);
			
			for (uint32_t j = 0; j < val32(entries[i].num_models); j++)
			{
				if (val32(specs[j].idx) == model_spec_idx)
				{
					cms = &specs[j];
					break;
				}
			}
			
			break;
		}
	}	
	
	if (!cms)
		return NULL;
	
	ModelSpec *ms = new ModelSpec();
	
	if (!ms)
		return NULL;
	
	GetNativeModelSpec(cms, ms);
	
	return ms;
}

ModelSpec *CmsFile::GetModelSpecFromModelId(uint32_t model_id, uint32_t *entry_id)
{
	CHECK_LOADED_PTR();
	
	size_t native_size = native_entries.size();
	
	if (native_size != 0)
	{
		for (CmsEntry &e : native_entries)
		{
			for (ModelSpec &m : e.specs)
			{
				if (m.model_id == model_id)
				{
					if (entry_id)
						*entry_id = val32(e.cms_entry.id);
					
					return new ModelSpec(m);
				}
			}
		}
	}
	
	for (uint32_t i = 0; i < num_entries; i++)
	{
		uint32_t num_models = val32(entries[i].num_models);
		CMSModelSpec *specs = (CMSModelSpec *)GetOffsetPtr(buf, entries[i].models_spec_offset);
		
		for (uint32_t j = 0; j < num_models; j++)
		{
			if (val32(specs[j].model_id) == model_id)
			{
				ModelSpec *ms = new ModelSpec();				
				GetNativeModelSpec(&specs[j], ms);
				
				if (entry_id)
					*entry_id = val32(entries[i].id);
				
				return ms;
			}
		}
	}
	
	return NULL;
}

bool CmsFile::SetModelSpec(uint32_t entry_id, ModelSpec *spec)
{
    CHECK_LOADED_BOOL();

    DecomposeIfNeeded();

    for (CmsEntry &e : native_entries)
    {
        if (val32(e.cms_entry.id) == entry_id)
        {
            for (ModelSpec &m : e.specs)
            {
                if (m.idx == spec->idx)
                {
                    m = ModelSpec(*spec);
                    return true;
                }
            }

            return false;
        }
    }

    return false;
}

bool CmsFile::AppendModel(uint32_t entry_id, ModelSpec *spec)
{
	CHECK_LOADED_BOOL();
	
	DecomposeIfNeeded();
	
	for (CmsEntry &e : native_entries)
	{
		if (val32(e.cms_entry.id) == entry_id)
		{
			for (ModelSpec &m : e.specs)
			{
				if (m.idx == spec->idx)
					return false;
			}
			
			e.specs.push_back(*spec);
			e.cms_entry.num_models = val32(val32(e.cms_entry.num_models)+1);
			return true;
		}
	}
	
	return false;
}

bool CmsFile::DeleteModel(uint32_t entry_id, uint32_t model_spec_idx)
{
	CHECK_LOADED_BOOL();
	
	DecomposeIfNeeded();
	
	for (CmsEntry &e : native_entries)
	{
		if (val32(e.cms_entry.id) == entry_id)
		{
			for (size_t i = 0; i < e.specs.size(); i++)
			{
				if (e.specs[i].idx == model_spec_idx)
				{
					e.specs.erase(e.specs.begin()+i);
                    e.cms_entry.num_models = val32(val32(e.cms_entry.num_models)-1);
					return true;
				}
			}
			
			return false;
		}
	}
	
	return false;
}

int CmsFile::DecreaseAllModelId(uint32_t from)
{
    CHECK_LOADED_INT();

    int count = 0;

    DecomposeIfNeeded();

    for (CmsEntry & e : native_entries)
    {
        for (ModelSpec &m : e.specs)
        {
            if (m.model_id > from)
            {
                m.model_id = m.model_id - 1;
                count++;
            }
        }
    }

    return count;
}

void CmsFile::Decompose(std::vector<CmsEntry> & entry_list)
{
	entry_list.clear();
	entry_list.reserve(num_entries);
	
	for (uint32_t i = 0; i < num_entries; i++)
	{
		CMSEntry *entry_foreign;
		CmsEntry entry_native;
		uint32_t num_models;
		
		entry_foreign = &entries[i];
		memcpy(&entry_native.cms_entry, entry_foreign, sizeof(CMSEntry));
		
		num_models = val32(entry_foreign->num_models);
		for (uint32_t j = 0; j < num_models; j++)
		{
			CMSModelSpec *cms;
			ModelSpec ms;
			
			cms = (CMSModelSpec *)(GetOffsetPtr(buf, entries[i].models_spec_offset)) + j;	
			GetNativeModelSpec(cms, &ms);
			
			entry_native.specs.push_back(ms);
		}
		
		entry_list.push_back(entry_native);
	}
}

void CmsFile::DecomposeIfNeeded()
{
	if (native_entries.size() == 0)
	{
		Decompose(native_entries);
	}
}

void CmsFile::WriteSpec(TiXmlElement *root, ModelSpec *spec) const
{
	TiXmlElement *spec_root = new TiXmlElement("ModelSpec");
	SsssCharInfo *ssss_info = SsssData::FindInfo(spec->model_id);
	
	if (ssss_info)
	{
		Utils::WriteComment(spec_root, std::string(" ") + ssss_info->model_name + std::string(" / ") + ssss_info->char_name);	
	}
	
	Utils::WriteParamString(spec_root, "BBA0", spec->bba0);
	Utils::WriteParamString(spec_root, "BBA1", spec->bba1);
	Utils::WriteParamString(spec_root, "BBA2", spec->bba2);
	Utils::WriteParamString(spec_root, "BBA3", spec->bba3);
	Utils::WriteParamString(spec_root, "BBA4", spec->bba4);
	Utils::WriteParamString(spec_root, "BBA5", spec->bba5);
	Utils::WriteParamString(spec_root, "EMO", spec->emo_file);
	Utils::WriteParamString(spec_root, "EMB", spec->emb_file);
	Utils::WriteParamString(spec_root, "EMM", spec->emm_file);
	Utils::WriteParamString(spec_root, "EMA", spec->ema_file);
	Utils::WriteParamString(spec_root, "MENU", spec->menu_file);
	Utils::WriteParamString(spec_root, "FCE", spec->fce_file);
	Utils::WriteParamString(spec_root, "MATBAS", spec->matbas_file);
	Utils::WriteParamString(spec_root, "TWORY", spec->_2ry_file);
	Utils::WriteParamString(spec_root, "FMA", spec->fma_file);
	Utils::WriteParamString(spec_root, "FDM", spec->fdm_file);
	Utils::WriteParamString(spec_root, "FCM", spec->fcm_file);
	Utils::WriteParamString(spec_root, "VFX", spec->vfx_file);
	Utils::WriteParamString(spec_root, "TDB", spec->tdb_file);
	Utils::WriteParamString(spec_root, "BPM", spec->bpm_file);
	Utils::WriteParamString(spec_root, "VCN", spec->vc_name);
	Utils::WriteParamString(spec_root, "SE", spec->se_name);
	Utils::WriteParamString(spec_root, "VCF", spec->vc_file);
	
	Utils::WriteParamUnsigned(spec_root, "IDX", spec->idx);
	Utils::WriteParamUnsigned(spec_root, "MODEL_ID", spec->model_id, true);
	Utils::WriteParamUnsigned(spec_root, "U_D0", spec->unk_D0, true);
	Utils::WriteParamUnsigned(spec_root, "U_D4", spec->unk_D4, true);
	Utils::WriteParamFloat(spec_root, "SCALE", spec->scale);
	Utils::WriteParamUnsigned(spec_root, "COSMO", spec->cosmo, true);
	Utils::WriteParamFloat(spec_root, "F_E0", spec->unk_E0);
	Utils::WriteParamFloat(spec_root, "F_E4", spec->unk_E4);
	Utils::WriteParamFloat(spec_root, "F_E8", spec->unk_E8);
	Utils::WriteParamFloat(spec_root, "F_EC", spec->unk_EC);
	Utils::WriteParamFloat(spec_root, "F_F0", spec->unk_F0);
	Utils::WriteParamUnsigned(spec_root, "AURA", spec->aura, true);
	
	
	root->LinkEndChild(spec_root);
}

void CmsFile::WriteEntry(TiXmlElement *root, uint32_t entry_idx) const
{
	CMSEntry *entry = &entries[entry_idx];	
	uint32_t num_models = val32(entry->num_models);	
	
	TiXmlElement *entry_root = new TiXmlElement("Entry");
	entry_root->SetAttribute("id", Utils::UnsignedToString(val32(entry->id), true));
	entry_root->SetAttribute("name", entry->name);
	entry_root->SetAttribute("type", Utils::UnsignedToString(val32(entry->type), true));
	
	for (uint32_t i = 0; i < num_models; i++)
	{
		ModelSpec *spec = GetModelSpecInternal(entry_idx, i);		
		WriteSpec(entry_root, spec);
				
		delete spec;
	}
	
	root->LinkEndChild(entry_root);
}

#define SPEC_STR(n, f) { if (!Utils::GetParamString(root, n, spec->f)) return false; }
#define SPEC_U(n, f) { if (!Utils::GetParamUnsigned(root, n, &spec->f)) return false; }
#define SPEC_F(n, f) { if (!Utils::GetParamFloat(root, n, &spec->f)) return false; }

bool CmsFile::ReadSpec(TiXmlElement *root, ModelSpec *spec)
{
	SPEC_STR("BBA0", bba0);	
	SPEC_STR("BBA1", bba1);
	SPEC_STR("BBA2", bba2);
	SPEC_STR("BBA3", bba3);
	SPEC_STR("BBA4", bba4);
	SPEC_STR("BBA5", bba5);
	SPEC_STR("EMO", emo_file);
	SPEC_STR("EMB", emb_file);
	SPEC_STR("EMM", emm_file);
	SPEC_STR("EMA", ema_file);
	SPEC_STR("MENU", menu_file);
	SPEC_STR("FCE", fce_file);
	SPEC_STR("MATBAS", matbas_file);
	SPEC_STR("TWORY", _2ry_file);
	SPEC_STR("FMA", fma_file);
	SPEC_STR("FDM", fdm_file);
	SPEC_STR("FCM", fcm_file);
	SPEC_STR("VFX", vfx_file);
	SPEC_STR("TDB", tdb_file);
	SPEC_STR("BPM", bpm_file);
	SPEC_STR("VCN", vc_name);
	SPEC_STR("SE", se_name);
	SPEC_STR("VCF", vc_file);
	
	SPEC_U("IDX", idx);
	SPEC_U("MODEL_ID", model_id);
	SPEC_U("U_D0", unk_D0);
	SPEC_U("U_D4", unk_D4);
	SPEC_F("SCALE", scale);
	SPEC_U("COSMO", cosmo);
	SPEC_F("F_E0", unk_E0);
	SPEC_F("F_E4", unk_E4);
	SPEC_F("F_E8", unk_E8);
	SPEC_F("F_EC", unk_EC);
	SPEC_F("F_F0", unk_F0);
	SPEC_U("AURA", aura);
	
	return true;
}

bool CmsFile::ReadEntry(TiXmlElement *root, CmsEntry *entry)
{
	std::string str, name;
			
	memset(&entry->cms_entry, 0, sizeof(CMSEntry));
	entry->specs.clear();
	
	if (!Utils::ReadAttrUnsigned(root, "id", &entry->cms_entry.id))
	{
		DPRINTF("%s: failed to get id in Entry.\n", __PRETTY_FUNCTION__);
		return false;
	}
	
	if (root->QueryStringAttribute("name", &str) != TIXML_SUCCESS)
	{
		DPRINTF("%s: failed to get name in Entry (id = 0x%x)\n", __PRETTY_FUNCTION__, entry->cms_entry.id);
		return false;
	}
	
	if (str.length() != 3)
	{
		DPRINTF("%s: name must be 3 character length. Offending string = \"%s\" in entry 0x%x\n", __PRETTY_FUNCTION__, str.c_str(), entry->cms_entry.id);
		return false;
	}
	
	strcpy(entry->cms_entry.name, str.c_str());
	
	if (!Utils::ReadAttrUnsigned(root, "type", &entry->cms_entry.type))
	{
		DPRINTF("%s: failed to get type in Entry \"%s\" (0x%x)\n", __PRETTY_FUNCTION__, entry->cms_entry.name, entry->cms_entry.id);
		return false;
	}
	
	for (TiXmlElement *elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{
		name = elem->ValueStr();
		
		if (name == "ModelSpec")
		{
			ModelSpec spec;
			
			if (!ReadSpec(elem, &spec))
			{
				DPRINTF("%s: ReadSpec failed on entry \"%s\"\n", __PRETTY_FUNCTION__, entry->cms_entry.name);
				return false;
			}
			
			entry->specs.push_back(spec);
		}
	}
	
	if (entry->specs.size() == 0)
	{
		DPRINTF("%s: there must be at least one spec per entry (offending entry = \"%s\")\n", __PRETTY_FUNCTION__, entry->cms_entry.name);
		return false;
	}
	
	entry->cms_entry.num_models = val32(entry->specs.size());	
	return true;
}

TiXmlDocument *CmsFile::Decompile() const
{
	CHECK_LOADED_PTR();
	
    if (!const_cast<CmsFile *>(this)->BuildIfNeeded(true))
	{
		DPRINTF("%s: BuildIfNeeded failed.\n", __PRETTY_FUNCTION__);
		return NULL;
	}
	
	TiXmlDocument *doc = new TiXmlDocument();
	
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );	
	doc->LinkEndChild(decl);
	
	TiXmlElement *root = new TiXmlElement("CMS");
	Utils::WriteComment(root, " This file has machine generated comments. Any change to these comments will be lost on next decompilation. ");	
	
	for (uint32_t i = 0; i < num_entries; i++)
	{
		WriteEntry(root, i);
	}
	
	doc->LinkEndChild(root);
	
	return doc;
}

#define ADD_STR(f) { if (m.f.length() != 0 && std::find(str_list.begin(), str_list.end(), m.f) == str_list.end()) { str_list.push_back(m.f); section_size += (m.f.length() + 1);} }

char *CmsFile::BuildStringSection(std::vector<CmsEntry> & entry_list, unsigned int *num_strings, unsigned int *size)
{
	std::vector<std::string> str_list;
	size_t section_size = 0;
	char *buf, *ptr;
	
	for (CmsEntry &e : entry_list)
	{
		for (ModelSpec &m : e.specs)
		{
			ADD_STR(bba0);
			ADD_STR(bba1);
			ADD_STR(bba2);
			ADD_STR(bba3);
			ADD_STR(bba4);
			ADD_STR(bba5);
			ADD_STR(emo_file);
			ADD_STR(emb_file);
			ADD_STR(emm_file);
			ADD_STR(ema_file);
			ADD_STR(menu_file);
			ADD_STR(fce_file);
			ADD_STR(matbas_file);
			ADD_STR(_2ry_file);
			ADD_STR(fma_file);
			ADD_STR(fdm_file);
			ADD_STR(fcm_file);
			ADD_STR(vfx_file);
			ADD_STR(tdb_file);
			ADD_STR(bpm_file);
			ADD_STR(vc_name);
			ADD_STR(se_name);
			ADD_STR(vc_file);
		}
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

#define DO_STRO(f, fc) { \
				if (m.f.length() == 0) \
					this_model->fc = 0; \
				else { \
					this_model->fc = val64(Utils::DifPointer((void *)FindString((char *)this->buf+data_section_size, m.f.c_str(), num_strings), this->buf)); \
				} \
				}
				
#define DO_U32(f) { this_model->f = val32(m.f); }
#define DO_FLOAT(f) { this_model->f = val_float(m.f); }

bool CmsFile::BuildBinary(std::vector<CmsEntry> & entries)
{
	char *str_section;
	unsigned int num_strings;
	unsigned int str_section_size, data_section_size;
	
	str_section = BuildStringSection(entries, &num_strings, &str_section_size);
	if (!str_section)
	{
		DPRINTF("%s: we are short in memory!\n", __PRETTY_FUNCTION__);
		return false;
	}
	
	data_section_size = 0x20 + ( entries.size() * sizeof(CMSEntry) );
	
	for (CmsEntry &e : entries)
	{
		data_section_size += (e.specs.size() * sizeof(CMSModelSpec));
	}
	
	this->buf = new uint8_t[data_section_size+str_section_size];
	if (!this->buf)
	{
		DPRINTF("%s: we are short in memory!\n", __PRETTY_FUNCTION__);
		return false;
	}
	
	this->size = data_section_size+str_section_size;
	memset(this->buf, 0, this->size);
	memcpy(this->buf+data_section_size, str_section, str_section_size);
	
	uint8_t *p = this->buf + 0x20;
	uint8_t *q = p + (entries.size() * sizeof(CMSEntry));
	
	for (CmsEntry &e : entries)
	{
		uint8_t *top_m = q;		
		
		for (ModelSpec &m : e.specs)
		{
			CMSModelSpec *this_model = (CMSModelSpec *)q;
			
			DO_STRO(bba0, bba0_name_offset);			
			DO_STRO(bba1, bba1_name_offset);
			DO_STRO(bba2, bba2_name_offset);
			DO_STRO(bba3, bba3_name_offset);
			DO_STRO(bba4, bba4_name_offset);
			DO_STRO(bba5, bba5_name_offset);
			DO_STRO(emo_file, emo_file_offset);
			DO_STRO(emb_file, emb_file_offset);
			DO_STRO(emm_file, emm_file_offset);
			DO_STRO(ema_file, ema_file_offset);
			DO_STRO(menu_file, menu_file_offset);
			DO_STRO(fce_file, fce_file_offset);
			DO_STRO(matbas_file, matbas_file_offset);
			DO_STRO(vc_file, vc_file_offset);
			DO_STRO(_2ry_file, _2ry_file_offset);
			DO_STRO(fma_file, fma_file_offset);
			DO_STRO(fdm_file, fdm_file_offset);
			DO_STRO(fcm_file, fcm_file_offset);
			DO_STRO(vfx_file, vfx_file_offset);
			DO_STRO(tdb_file, tdb_file_offset);
			DO_STRO(bpm_file, bpm_file_offset);
			DO_STRO(vc_name, vc_name_offset);
			DO_STRO(se_name, se_name_offset);
			
			DO_U32(idx);
			DO_U32(model_id);
			DO_U32(unk_D0);
			DO_U32(unk_D4);
			DO_FLOAT(scale);
			DO_U32(cosmo);
			DO_FLOAT(unk_E0);
			DO_FLOAT(unk_E4);
			DO_FLOAT(unk_E8);
			DO_FLOAT(unk_EC);
			DO_FLOAT(unk_F0);
			DO_U32(aura);
			
			q += sizeof(CMSModelSpec);
		}
		
		e.cms_entry.models_spec_offset = val32(Utils::DifPointer(top_m, this->buf));
		memcpy(p, &e.cms_entry, sizeof(CMSEntry));
		p += sizeof(CMSEntry);
	}
	
	// And finally, the header;	
	CMSHeader *hdr = (CMSHeader *)this->buf;
	
	hdr->signature = CMS_SIGNATURE;
	hdr->endianess_check = val32(0xFFFE);
	hdr->num_entries = val32(entries.size());
	hdr->data_start = val32(0x20);
	
	this->entries = (CMSEntry *)GetOffsetPtr(hdr, hdr->data_start);
	this->num_entries = val32(hdr->num_entries);	
	
	delete[] str_section;
	return true;
}

bool CmsFile::BuildIfNeeded(bool clear)
{
	if (native_entries.size() == 0)
		return true;
	
	if (!BuildBinary(native_entries))
		return false;
	
	if (clear)
	{
		native_entries.clear();
	}
	
	return true;
}

bool CmsFile::Compile(TiXmlDocument *doc, bool big_endian)
{
	std::vector<CmsEntry> entries;
			
	Reset();
	this->big_endian = big_endian;
	
	TiXmlHandle handle(doc);
	TiXmlElement *root = NULL;;
	
	for (TiXmlElement *elem = handle.FirstChildElement().Element(); elem != NULL; elem = elem->NextSiblingElement())
	{
		std::string name = elem->ValueStr();
		
		if (name == "CMS")
		{
			root = elem;
			break;
		}
	}		
	
	if (!root)
	{
		DPRINTF("%s: cannot find \"CMS\" in xml.\n", __PRETTY_FUNCTION__);
		return false;
	}
	
	for (TiXmlElement *elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{
		std::string name = elem->ValueStr();
		
		if (name == "Entry")
		{
			CmsEntry entry;
			
			if (!ReadEntry(elem, &entry))
				return false;

			entries.push_back(entry);
		}
	}
	
	if (entries.size() == 0)
	{
		DPRINTF("%s: there are no Entry!\n", __PRETTY_FUNCTION__);
		return false;
	}
	
	return BuildBinary(entries);
}

uint8_t *CmsFile::Save(size_t *size)
{
	CHECK_LOADED_PTR();
	
	if (!BuildIfNeeded(true))
	{
		DPRINTF("%s: BuildIfNeeded failed.\n", __PRETTY_FUNCTION__);
		return nullptr;
	}
		
	uint8_t *buf = new uint8_t[this->size];	
	
	memcpy(buf, this->buf, this->size);
	*size = this->size;
	
	return buf;
}
