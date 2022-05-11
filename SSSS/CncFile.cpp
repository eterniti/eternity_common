#include "CncFile.h"
#include "debug.h"

CncFile::CncFile()
{
	this->big_endian = false;
	Reset();	
}

CncFile::CncFile(const uint8_t *buf, size_t size)
{
	this->big_endian = false;
	Load(buf, size);
}

CncFile::~CncFile()
{
	Reset();
}

void CncFile::Reset()
{
	sign_names.clear();
	robes_names.clear();
}

bool CncFile::Load(const uint8_t *buf, size_t size)
{
	Reset();
	
	if (size < sizeof(CNCHeader))
		return false;
	
	CNCHeader *hdr = (CNCHeader *)buf;
	
	if (hdr->signature != CNC_SIGNATURE || val32(hdr->version) != CURRENT_VERSION)
		return false;
	
	unsigned int should_have_size = sizeof(CNCHeader) + ( (val32(hdr->num_sign_names) + val32(hdr->num_robes_names)) * sizeof(CharacterDef) );
	
	if (size < should_have_size)
		return false;
	
	CharacterDef *table = (CharacterDef *)GetOffsetPtr(hdr, hdr->sign_names_offset);
	
	for (uint32_t i = 0; i < val32(hdr->num_sign_names); i++)
	{
		sign_names.push_back(table[i]);
	}
	
	table = (CharacterDef *)GetOffsetPtr(hdr, hdr->robes_names_offset);
	
	for (uint32_t i = 0; i < val32(hdr->num_robes_names); i++)
	{
		robes_names.push_back(table[i]);
	}
	
	if (IsEmpty())
		return false;
	
	return true;
}

uint8_t *CncFile::Save(size_t *psize)
{
	unsigned int file_size = sizeof(CNCHeader) + ( (sign_names.size() + robes_names.size())  * sizeof(CharacterDef));	
	
	uint8_t *buf = new uint8_t[file_size];
	memset(buf, 0, file_size);
	
	CNCHeader *hdr = (CNCHeader *)buf;	
	
	hdr->signature = CNC_SIGNATURE;
	hdr->version = val32(CURRENT_VERSION);
	hdr->num_sign_names = val32(sign_names.size());
	hdr->num_robes_names = val32(robes_names.size());
	hdr->sign_names_offset = val32(sizeof(CNCHeader));
	hdr->robes_names_offset = val32(sizeof(CNCHeader) + (sign_names.size() * sizeof(CharacterDef)));
	
	
	uint8_t *ptr = GetOffsetPtr(hdr, sizeof(CNCHeader), true);
	
	for (CharacterDef &def : sign_names)
	{
		memcpy(ptr, &def, sizeof(CharacterDef));
		ptr += sizeof(CharacterDef);
	}
	
	for (CharacterDef &def : robes_names)
	{
		memcpy(ptr, &def, sizeof(CharacterDef));
		ptr += sizeof(CharacterDef);
	}
	
	*psize = file_size;
	return buf;
}

