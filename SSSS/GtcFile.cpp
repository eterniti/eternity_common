#include "GtcFile.h"
#include "debug.h"

GtcFile::GtcFile()
{
	this->big_endian = false;
	Reset();	
}

GtcFile::GtcFile(const uint8_t *buf, size_t size)
{
	this->big_endian = false;
	Load(buf, size);
}

GtcFile::~GtcFile()
{
	Reset();
}

void GtcFile::Reset()
{
	banners.clear();
	banner_names.clear();
}

bool GtcFile::Load(const uint8_t *buf, size_t size)
{
	Reset();
	
	if (size < sizeof(GTCHeader))
		return false;
	
	GTCHeader *hdr = (GTCHeader *)buf;
	
	if (hdr->signature != GTC_SIGNATURE || val32(hdr->version) != CURRENT_VERSION)
		return false;
	
	unsigned int should_have_size = sizeof(GTCHeader) + ( (val32(hdr->num_banners) + val32(hdr->num_banner_names)) * sizeof(CharacterDef) );
	
	if (size < should_have_size)
		return false;
	
	CharacterDef *table = (CharacterDef *)GetOffsetPtr(hdr, hdr->banners_offset);
	
	for (uint32_t i = 0; i < val32(hdr->num_banners); i++)
	{
		banners.push_back(table[i]);
	}
	
	table = (CharacterDef *)GetOffsetPtr(hdr, hdr->banner_names_offset);
	
	for (uint32_t i = 0; i < val32(hdr->num_banner_names); i++)
	{
		banner_names.push_back(table[i]);
	}
	
	if (IsEmpty())
		return false;
	
	return true;
}

uint8_t *GtcFile::Save(size_t *psize)
{
	unsigned int file_size = sizeof(GTCHeader) + ( (banners.size() + banner_names.size())  * sizeof(CharacterDef));	
	uint8_t *buf = new uint8_t[file_size];	
	
	GTCHeader *hdr = (GTCHeader *)buf;	
	
	hdr->signature = GTC_SIGNATURE;
	hdr->version = val32(CURRENT_VERSION);
	hdr->num_banners = val32(banners.size());
	hdr->num_banner_names = val32(banner_names.size());
	hdr->banners_offset = val32(sizeof(GTCHeader));
	hdr->banner_names_offset = val32(sizeof(GTCHeader) + (banners.size() * sizeof(CharacterDef)));
	
	
	uint8_t *ptr = GetOffsetPtr(hdr, sizeof(GTCHeader), true);
	
	for (CharacterDef &def : banners)
	{
		memcpy(ptr, &def, sizeof(CharacterDef));
		ptr += sizeof(CharacterDef);
	}
	
	for (CharacterDef &def : banner_names)
	{
		memcpy(ptr, &def, sizeof(CharacterDef));
		ptr += sizeof(CharacterDef);
	}
	
	*psize = file_size;
	return buf;
}

