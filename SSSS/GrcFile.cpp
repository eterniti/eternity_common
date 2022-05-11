#include "GrcFile.h"
#include "debug.h"

GrcFile::GrcFile()
{
	this->big_endian = false;
	Reset();	
}

GrcFile::GrcFile(const uint8_t *buf, size_t size)
{
	this->big_endian = false;
	Load(buf, size);
}

GrcFile::~GrcFile()
{
	Reset();
}

void GrcFile::Reset()
{
	icons.clear();
	names.clear();
}

bool GrcFile::Load(const uint8_t *buf, size_t size)
{
	Reset();
	
	if (size < sizeof(GRCHeader))
		return false;
	
	GRCHeader *hdr = (GRCHeader *)buf;
	
	if (hdr->signature != GRC_SIGNATURE || val32(hdr->version) != CURRENT_VERSION)
		return false;
	
	unsigned int should_have_size = sizeof(GRCHeader) + ( (val32(hdr->num_icons) + val32(hdr->num_names)) * sizeof(CharacterDef) );
	
	if (size < should_have_size)
		return false;
	
	CharacterDef *table = (CharacterDef *)GetOffsetPtr(hdr, hdr->icons_offset);
	
	for (uint32_t i = 0; i < val32(hdr->num_icons); i++)
	{
		icons.push_back(table[i]);
	}
	
	table = (CharacterDef *)GetOffsetPtr(hdr, hdr->names_offset);
	
	for (uint32_t i = 0; i < val32(hdr->num_names); i++)
	{
		names.push_back(table[i]);
	}
	
	if (IsEmpty())
		return false;
	
	return true;
}

uint8_t *GrcFile::Save(size_t *psize)
{
	unsigned int file_size = sizeof(GRCHeader) + ( (icons.size() + names.size())  * sizeof(CharacterDef));
	
	uint8_t *buf = new uint8_t[file_size];
		
	GRCHeader *hdr = (GRCHeader *)buf;	
	
	hdr->signature = GRC_SIGNATURE;
	hdr->version = val32(CURRENT_VERSION);
	hdr->num_icons = val32(icons.size());
	hdr->num_names = val32(names.size());
	hdr->icons_offset = val32(sizeof(GRCHeader));
	hdr->names_offset = val32(sizeof(GRCHeader) + (icons.size() * sizeof(CharacterDef)));
	
	
	uint8_t *ptr = GetOffsetPtr(hdr, sizeof(GRCHeader), true);
	
	for (CharacterDef &def : icons)
	{
		memcpy(ptr, &def, sizeof(CharacterDef));
		ptr += sizeof(CharacterDef);
	}
	
	for (CharacterDef &def : names)
	{
		memcpy(ptr, &def, sizeof(CharacterDef));
		ptr += sizeof(CharacterDef);
	}
	
	*psize = file_size;
	return buf;
}

