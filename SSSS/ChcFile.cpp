#include "ChcFile.h"
#include "debug.h"

ChcFile::ChcFile()
{
	this->big_endian = false;
	Reset();	
}

ChcFile::ChcFile(const uint8_t *buf, size_t size)
{
	this->big_endian = false;
	Load(buf, size);
}

ChcFile::~ChcFile()
{
	Reset();
}

void ChcFile::Reset()
{
	icons.clear();
	select_names.clear();
	select2_names.clear();
}

bool ChcFile::Load(const uint8_t *buf, size_t size)
{
	Reset();
	
	if (size < sizeof(CHCHeader))
		return false;
	
	CHCHeader *hdr = (CHCHeader *)buf;
	
	if (hdr->signature != CHC_SIGNATURE || val32(hdr->version) != CURRENT_VERSION)
		return false;
	
	unsigned int should_have_size = sizeof(CHCHeader) + ( (val32(hdr->num_icons) + val32(hdr->num_select_names) + val32(hdr->num_select2_names) ) * sizeof(CharacterDef) );
	
	if (size < should_have_size)
		return false;
	
	CharacterDef *table = (CharacterDef *)GetOffsetPtr(hdr, hdr->icons_offset);
	
	for (uint32_t i = 0; i < val32(hdr->num_icons); i++)
	{
		icons.push_back(table[i]);
	}
	
	table = (CharacterDef *)GetOffsetPtr(hdr, hdr->select_names_offset);
	
	for (uint32_t i = 0; i < val32(hdr->num_select_names); i++)
	{
		select_names.push_back(table[i]);
	}
	
	table = (CharacterDef *)GetOffsetPtr(hdr, hdr->select2_names_offset);
	
	for (uint32_t i = 0; i < val32(hdr->num_select2_names); i++)
	{
		select2_names.push_back(table[i]);
	}
	
	if (IsEmpty())
		return false;
	
	return true;
}

uint8_t *ChcFile::Save(size_t *psize)
{
	unsigned int file_size = sizeof(CHCHeader) + ( (icons.size() + select_names.size() + select2_names.size() )  * sizeof(CharacterDef));	
	
	uint8_t *buf = new uint8_t[file_size];
	memset(buf, 0, file_size);
	
	CHCHeader *hdr = (CHCHeader *)buf;	
	
	hdr->signature = CHC_SIGNATURE;
	hdr->version = val32(CURRENT_VERSION);
	hdr->num_icons = val32(icons.size());
	hdr->num_select_names = val32(select_names.size());
	hdr->num_select2_names = val32(select2_names.size());
	hdr->icons_offset = val32(sizeof(CHCHeader));
	hdr->select_names_offset = val32(sizeof(CHCHeader) + (icons.size() * sizeof(CharacterDef)));
	hdr->select2_names_offset = val32( val32(hdr->select_names_offset) + (select_names.size() * sizeof(CharacterDef)) );
	
	
	uint8_t *ptr = GetOffsetPtr(hdr, sizeof(CHCHeader), true);
	
	for (CharacterDef &def : icons)
	{
		memcpy(ptr, &def, sizeof(CharacterDef));
		ptr += sizeof(CharacterDef);
	}
	
	for (CharacterDef &def : select_names)
	{
		memcpy(ptr, &def, sizeof(CharacterDef));
		ptr += sizeof(CharacterDef);
	}
	
	for (CharacterDef &def : select2_names)
	{
		memcpy(ptr, &def, sizeof(CharacterDef));
		ptr += sizeof(CharacterDef);
	}
	
	*psize = file_size;
	return buf;
}

