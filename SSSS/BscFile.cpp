#include "BscFile.h"
#include "debug.h"

BscFile::BscFile()
{
	this->big_endian = false;
	Reset();	
}

BscFile::BscFile(const uint8_t *buf, size_t size)
{
	this->big_endian = false;
	Load(buf, size);
}

BscFile::~BscFile()
{
	Reset();
}

void BscFile::Reset()
{
	avatars.clear();
	battle_names.clear();
}

bool BscFile::Load(const uint8_t *buf, size_t size)
{
	Reset();
	
	if (size < sizeof(BSCHeader))
		return false;
	
	BSCHeader *hdr = (BSCHeader *)buf;
	
	if (hdr->signature != BSC_SIGNATURE || val32(hdr->version) != CURRENT_VERSION)
		return false;
	
	unsigned int should_have_size = sizeof(BSCHeader) + ( (val32(hdr->num_avatars) + val32(hdr->num_battle_names)) * sizeof(CharacterDef) );
	
	if (size < should_have_size)
		return false;
	
	CharacterDef *table = (CharacterDef *)GetOffsetPtr(hdr, hdr->avatars_offset);
	
	for (uint32_t i = 0; i < val32(hdr->num_avatars); i++)
	{
		avatars.push_back(table[i]);
	}
	
	table = (CharacterDef *)GetOffsetPtr(hdr, hdr->battle_names_offset);
	
	for (uint32_t i = 0; i < val32(hdr->num_battle_names); i++)
	{
		battle_names.push_back(table[i]);
	}
	
	if (IsEmpty())
		return false;
	
	return true;
}

uint8_t *BscFile::Save(size_t *psize)
{
	unsigned int file_size = sizeof(BSCHeader) + ( (avatars.size() + battle_names.size())  * sizeof(CharacterDef));
	
	uint8_t *buf = new uint8_t[file_size];
	memset(buf, 0, file_size);
	
	BSCHeader *hdr = (BSCHeader *)buf;	
	
	hdr->signature = BSC_SIGNATURE;
	hdr->version = val32(CURRENT_VERSION);
	hdr->num_avatars = val32(avatars.size());
	hdr->num_battle_names = val32(battle_names.size());
	hdr->avatars_offset = val32(sizeof(BSCHeader));
	hdr->battle_names_offset = val32(sizeof(BSCHeader) + (avatars.size() * sizeof(CharacterDef)));
	
	
	uint8_t *ptr = GetOffsetPtr(hdr, sizeof(BSCHeader), true);
	
	for (CharacterDef &def : avatars)
	{
		memcpy(ptr, &def, sizeof(CharacterDef));
		ptr += sizeof(CharacterDef);
	}
	
	for (CharacterDef &def : battle_names)
	{
		memcpy(ptr, &def, sizeof(CharacterDef));
		ptr += sizeof(CharacterDef);
	}
	
	*psize = file_size;
	return buf;
}

