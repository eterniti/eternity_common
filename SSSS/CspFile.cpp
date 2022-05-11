#include <stdint.h>

#include "CspFile.h"
#include "SsssData.h"
#include "debug.h"

#define CHECK_LOADED_BOOL() { if (!this->buf) { return false; } }
#define CHECK_LOADED_U32() { if (!this->buf) { return (uint32_t)-1; } }
#define CHECK_LOADED_PTR() { if (!this->buf) { return NULL; } }

CspFile::CspFile()
{
	buf = NULL;
	Reset();
}

CspFile::CspFile(const uint8_t *buf, size_t size)
{
	this->buf = NULL;
	Load(buf, size);
}

CspFile::~CspFile()
{
	Reset();
}

void CspFile::Reset()
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
}

#define TN_U32(f) { native->f = val32(foreign->f); } 
#define TN_FLOAT(f) { native->f = val_float(foreign->f); }

void CspFile::ToNativeEntry(CSPEntry *foreign, CspEntry *native)
{
	TN_U32(cms_entry);
	TN_U32(cms_model_spec_idx);
    TN_FLOAT(left_pos_x);
    TN_FLOAT(left_pos_y);
    TN_FLOAT(left_pos_z);
    TN_U32(left_rot);
    TN_FLOAT(right_pos_x);
    TN_FLOAT(right_pos_y);
    TN_FLOAT(right_pos_z);
    TN_U32(right_rot);
    TN_U32(animation_id);
	TN_U32(select_phrase_id); 
	TN_U32(unk_30); 
	TN_U32(unk_34); 
	TN_U32(unk_38); 
	TN_U32(unk_3C); 
}

#define TF_U32(f) {foreign->f = val32(native->f); } 
#define TF_FLOAT(f) { foreign->f = val_float(native->f); }

void CspFile::ToForeignEntry(CspEntry *native, CSPEntry *foreign)
{
	TF_U32(cms_entry);
	TF_U32(cms_model_spec_idx);
    TF_FLOAT(left_pos_x);
    TF_FLOAT(left_pos_y);
    TF_FLOAT(left_pos_z);
    TF_U32(left_rot);
    TF_FLOAT(right_pos_x);
    TF_FLOAT(right_pos_y);
    TF_FLOAT(right_pos_z);
    TF_U32(right_rot);
    TF_U32(animation_id);
	TF_U32(select_phrase_id); 
	TF_U32(unk_30); 
	TF_U32(unk_34); 
	TF_U32(unk_38); 
	TF_U32(unk_3C); 
}

bool CspFile::Load(const uint8_t *buf, size_t size)
{
	Reset();	
	
	if (*(uint32_t *)buf != CSP_SIGNATURE)
		return false;
	
	this->big_endian = (buf[4] != 0xFE);
		
	this->buf = new uint8_t[size];
	if (!this->buf)
		return false;
	
	this->size = size;
	memcpy(this->buf, buf, size);	
	
	CSPHeader *hdr = (CSPHeader *)this->buf;
	
	this->entries = (CSPEntry *)GetOffsetPtr(hdr, 0x10, true);
	this->num_entries = val32(hdr->num_entries);	
	
	return true;
}

uint32_t CspFile::FindEntry(uint32_t cms_entry, uint32_t cms_model_spec_idx)
{
	CHECK_LOADED_U32();
	
	for (uint32_t i = 0; i < num_entries; i++)
	{
		if (val32(entries[i].cms_entry) == cms_entry && val32(entries[i].cms_model_spec_idx) == cms_model_spec_idx)
			return i;
	}
	
	return (uint32_t)-1;
}

CspEntry *CspFile::GetEntry(uint32_t id)
{
	CHECK_LOADED_PTR();
	
	if (id >= num_entries)
		return NULL;
	
	CspEntry *entry = new CspEntry();
	ToNativeEntry(&entries[id], entry);
	
	return entry;
}

bool CspFile::SetEntry(uint32_t id, CspEntry *entry)
{
	CHECK_LOADED_BOOL();
	
	if (id >= num_entries)
		return false;
	
	ToForeignEntry(entry, &entries[id]);
	return true;
}

uint32_t CspFile::AppendDefaultEntry()
{
	CSPEntry def_entry;
	
	memset(&def_entry, 0, sizeof(CSPEntry));
	
	uint32_t new_size = size + sizeof(CSPEntry);
	uint8_t *new_buf = new uint8_t[new_size];
	if (!new_buf)
		return false;
	
	memcpy(new_buf, buf, size);
	memcpy(new_buf+size, &def_entry, sizeof(CSPEntry));
	
	delete[] buf;	
	buf = new_buf;
	size = new_size;
	
	uint32_t ret = num_entries;
	num_entries++;
	
	CSPHeader *hdr = (CSPHeader *)buf;	
	entries = (CSPEntry *)GetOffsetPtr(hdr, 0x10, true);
	hdr->num_entries = val32(num_entries);	
	
	return ret;
}

uint32_t CspFile::AppendEntry(CspEntry *entry)
{
	CHECK_LOADED_U32();
	
	uint32_t id = AppendDefaultEntry();
	if (id == (uint32_t)-1)
		return id;
	
	ToForeignEntry(entry, &entries[id]);
	return id;
}

bool CspFile::DeleteEntry(uint32_t id)
{
	CHECK_LOADED_BOOL();
	
	if (id >= num_entries)
		return false;
	
	if (id != (num_entries-1))
	{		
		CSPEntry *src = entries + id + 1;
		CSPEntry *dst = entries + id;		
		size_t size = (num_entries - id - 1) * sizeof(CSPEntry);
		
		memmove(dst, src, size);
	}
	
	num_entries--;
	size = size - sizeof(CSPEntry);
	
	CSPHeader *hdr = (CSPHeader *)buf;
	hdr->num_entries = val32(num_entries);
	
	return true;
}

void CspFile::WriteEntry(TiXmlElement *root, uint32_t entry_idx) const
{
	CSPEntry *entry = &entries[entry_idx];		
	TiXmlElement *entry_root = new TiXmlElement("ModelPosEntry");
	SsssCharInfo *ssss_info = SsssData::FindInfo(val32(entry->cms_entry), val32(entry->cms_model_spec_idx));
	
	if (ssss_info)
	{
		Utils::WriteComment(entry_root, std::string(" ") + ssss_info->model_name + std::string(" / ") + ssss_info->char_name);	
	}
	
	Utils::WriteParamUnsigned(entry_root, "CMS_ENTRY", val32(entry->cms_entry), true);
	Utils::WriteParamUnsigned(entry_root, "CMS_MODEL_SPEC_IDX", val32(entry->cms_model_spec_idx));
    Utils::WriteParamFloat(entry_root, "LEFT_POS_X", val_float(entry->left_pos_x));
    Utils::WriteParamFloat(entry_root, "LEFT_POS_Y", val_float(entry->left_pos_y));
    Utils::WriteParamFloat(entry_root, "LEFT_POS_Z", val_float(entry->left_pos_z));
    Utils::WriteParamSigned(entry_root, "LEFT_ROT", val32(entry->left_rot));
    Utils::WriteParamFloat(entry_root, "RIGHT_POS_X", val_float(entry->right_pos_x));
    Utils::WriteParamFloat(entry_root, "RIGHT_POS_Y", val_float(entry->right_pos_y));
    Utils::WriteParamFloat(entry_root, "RIGHT_POS_Z", val_float(entry->right_pos_z));
    Utils::WriteParamSigned(entry_root, "RIGHT_ROT", val32(entry->right_rot));
    Utils::WriteParamUnsigned(entry_root, "ANIMATION_ID", val32(entry->animation_id), true);
	Utils::WriteParamUnsigned(entry_root, "SELECT_PHRASE_ID", val32(entry->select_phrase_id), true);	
	Utils::WriteParamUnsigned(entry_root, "U_30", val32(entry->unk_30), true);	
	Utils::WriteParamUnsigned(entry_root, "U_34", val32(entry->unk_34), true);	
	Utils::WriteParamUnsigned(entry_root, "U_38", val32(entry->unk_38), true);	
	Utils::WriteParamUnsigned(entry_root, "U_3C", val32(entry->unk_3C), true);	
	
	root->LinkEndChild(entry_root);
}

#define ENTRY_U(n, f) { if (!Utils::GetParamUnsigned(root, n, &entry->f)) \
							return false; \
						}
						
#define ENTRY_F(n, f) { if (!Utils::GetParamFloat(root, n, &entry->f)) \
							return false; \
						}

bool CspFile::ReadEntry(TiXmlElement *root, CspEntry *entry)
{
	ENTRY_U("CMS_ENTRY", cms_entry);
	ENTRY_U("CMS_MODEL_SPEC_IDX", cms_model_spec_idx);

    if (!Utils::GetParamFloatWithMultipleNames(root, &entry->left_pos_x, "LEFT_POS_X", "F_08"))
        return false;

    if (!Utils::GetParamFloatWithMultipleNames(root, &entry->left_pos_y, "LEFT_POS_Y", "F_0C"))
        return false;

    if (!Utils::GetParamFloatWithMultipleNames(root, &entry->left_pos_z, "LEFT_POS_Z", "F_10"))
        return false;

    if (!Utils::ReadParamUnsigned(root, "U_14", &entry->left_rot))
    {
        int32_t signed_rot;

        if (!Utils::GetParamSigned(root, "LEFT_ROT", &signed_rot))
            return false;

        entry->left_rot = signed_rot;
    }

    if (!Utils::GetParamFloatWithMultipleNames(root, &entry->right_pos_x, "RIGHT_POS_X", "F_18"))
        return false;

    if (!Utils::GetParamFloatWithMultipleNames(root, &entry->right_pos_y, "RIGHT_POS_Y", "F_1C"))
        return false;

    if (!Utils::GetParamFloatWithMultipleNames(root, &entry->right_pos_z, "RIGHT_POS_Z", "F_20"))
        return false;

    if (!Utils::ReadParamUnsigned(root, "U_24", &entry->right_rot))
    {
        int32_t signed_rot;

        if (!Utils::GetParamSigned(root, "RIGHT_ROT", &signed_rot))
            return false;

        entry->right_rot = signed_rot;
    }

    if (!Utils::GetParamUnsignedWithMultipleNames(root, &entry->animation_id, "ANIMATION_ID", "U_28"))
        return false;
	
	if (!Utils::GetParamUnsignedWithMultipleNames(root, &entry->select_phrase_id, "SELECT_PHRASE_ID", "U_2C"))
		return false;	
	
	ENTRY_U("U_30", unk_30);
	ENTRY_U("U_34", unk_34);
	ENTRY_U("U_38", unk_38);
	ENTRY_U("U_3C", unk_3C);
	
	return true;
}

TiXmlDocument *CspFile::Decompile() const
{
	CHECK_LOADED_PTR();
	
	TiXmlDocument *doc = new TiXmlDocument();
	
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );	
	doc->LinkEndChild(decl);
	
	TiXmlElement *root = new TiXmlElement("CSP");
	Utils::WriteComment(root, " This file has machine generated comments. Any change to these comments will be lost on next decompilation. ");
		
	for (uint32_t i = 0; i < num_entries; i++)
	{
		WriteEntry(root, i);
	}
	
	doc->LinkEndChild(root);
	
	return doc;
}

bool CspFile::Compile(TiXmlDocument *doc, bool big_endian)
{
	std::vector<CspEntry> entries;
	
	Reset();
	this->big_endian = big_endian;
	
	TiXmlHandle handle(doc);
	TiXmlElement *root = NULL;;
	
	for (TiXmlElement *elem = handle.FirstChildElement().Element(); elem != NULL; elem = elem->NextSiblingElement())
	{
		std::string name = elem->ValueStr();
		
		if (name == "CSP")
		{
			root = elem;
			break;
		}
	}		
	
	if (!root)
	{
		DPRINTF("Cannot find \"CSP\" in xml.\n");
		return false;
	}	
	
	for (TiXmlElement *elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{
		std::string name = elem->ValueStr();
		
		if (name == "ModelPosEntry")
		{
			CspEntry entry;
			
			if (!ReadEntry(elem, &entry))
				return false;

			entries.push_back(entry);
		}
	}
	
	if (entries.size() == 0)
	{
		DPRINTF("There are no ModelPosEntry!\n");
		return false;
	}
	
	
	this->size = sizeof(CSPHeader) + entries.size() * sizeof(CSPEntry);
	this->buf = new uint8_t[this->size];
	if (!this->buf)
	{
        DPRINTF("%s: We are short in memory.\n", FUNCNAME);
		this->size = 0;
		return false;
	}
	
	memset(this->buf, 0, this->size);
	
	uint8_t *ptr = this->buf + sizeof(CSPHeader);
	
	for (CspEntry &e: entries)
	{
		CSPEntry fe;
		
		ToForeignEntry(&e, &fe);
		
		memcpy(ptr, &fe, sizeof(CSPEntry));
		ptr += sizeof(CSPEntry);
	}
	
	// And finally, the header;	
	CSPHeader *hdr = (CSPHeader *)this->buf;
	
	hdr->signature = CSP_SIGNATURE;
	hdr->endianess_check = val32(0xFFFE);
	hdr->num_entries = val32(entries.size());
	
	this->entries = (CSPEntry *)GetOffsetPtr(hdr, 0x10, true);
	this->num_entries = val32(hdr->num_entries);	
	
	return true;
}

uint8_t *CspFile::Save(size_t *size)
{
	CHECK_LOADED_PTR();
	
	uint8_t *buf = new uint8_t[this->size];
		
	memcpy(buf, this->buf, this->size);
	*size = this->size;
	
	return buf;
}
