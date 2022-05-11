#include "VscFile.h"
#include "debug.h"

VscFile::VscFile()
{
	this->big_endian = false;
	Reset();	
}

VscFile::VscFile(const uint8_t *buf, size_t size)
{
	this->big_endian = false;
	Load(buf, size);
}

VscFile::~VscFile()
{
	Reset();
}

void VscFile::Reset()
{
	select_names.clear();	
}

bool VscFile::Load(const uint8_t *buf, size_t size)
{
	Reset();
	
	if (size < sizeof(VSCHeader))
		return false;
	
	VSCHeader *hdr = (VSCHeader *)buf;
	
	if (hdr->signature != VSC_SIGNATURE || val32(hdr->version) != CURRENT_VERSION)
		return false;
	
	unsigned int should_have_size = sizeof(VSCHeader) + ( ( val32(hdr->num_select_names) ) * sizeof(CharacterDef) );
	
	if (size < should_have_size)
		return false;
	
	CharacterDef *table = (CharacterDef *)GetOffsetPtr(hdr, hdr->select_names_offset);
	
	for (uint32_t i = 0; i < val32(hdr->num_select_names); i++)
	{
		select_names.push_back(table[i]);
	}	
	
	if (IsEmpty())
		return false;
	
	return true;
}

uint8_t *VscFile::Save(size_t *psize)
{
	unsigned int file_size = sizeof(VSCHeader) + ( ( select_names.size() )  * sizeof(CharacterDef));
	
	uint8_t *buf = new uint8_t[file_size];
	VSCHeader *hdr = (VSCHeader *)buf;	
	
	hdr->signature = VSC_SIGNATURE;
	hdr->version = val32(CURRENT_VERSION);
	hdr->num_select_names = val32(select_names.size());
	hdr->select_names_offset = val32(sizeof(VSCHeader));	
	
	uint8_t *ptr = GetOffsetPtr(hdr, sizeof(VSCHeader), true);
	
	for (CharacterDef &def : select_names)
	{
		memcpy(ptr, &def, sizeof(CharacterDef));
		ptr += sizeof(CharacterDef);
	}	
	
	*psize = file_size;
	return buf;
}

