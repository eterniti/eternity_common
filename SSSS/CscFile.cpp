#include <stdint.h>

#include "CscFile.h"
#include "debug.h"

#define CHECK_LOADED_BOOL() { if (!this->buf) { return false; } }
#define CHECK_LOADED_U32() { if (!this->buf) { return (uint32_t)-1; } }
#define CHECK_LOADED_PTR() { if (!this->buf) { return NULL; } }

CscFile::CscFile()
{
	buf = NULL;
	big_endian = false;
	Reset();
}

CscFile::CscFile(const uint8_t *buf, size_t size)
{
	this->buf = NULL;
	Load(buf, size);
}

CscFile::~CscFile()
{
	Reset();
}

void CscFile::Reset()
{
	if (buf)
	{	
		delete[] buf;
		buf = NULL;	
	}
	
	size = 0;	
	
	entries = NULL;
	num_entries = (uint32_t)-1;
}

bool CscFile::Load(const uint8_t *buf, size_t size)
{
	Reset();	
	
	if (*(uint32_t *)buf != CSC_SIGNATURE)
		return false;
	
	this->big_endian = (buf[4] != 0xFE);
		
	this->buf = new uint8_t[size];
	if (!this->buf)
		return false;
	
	this->size = size;
	memcpy(this->buf, buf, size);	
	
	CSCHeader *hdr = (CSCHeader *)this->buf;
	
	this->entries = (CSCEntry *)GetOffsetPtr(hdr, hdr->data_start);
	this->num_entries = val32(hdr->num_entries);	
	
	return true;
}

void CscFile::WriteEntry(TiXmlElement *root, uint32_t entry_idx) const
{
	CSCEntry *entry = &entries[entry_idx];		
	TiXmlElement *entry_root = new TiXmlElement("CharEntry");
	
	entry_root->SetAttribute("id", Utils::UnsignedToString(entry_idx, true));
	
	Utils::WriteParamUnsigned(entry_root, "U_00", val32(entry->unk_00), true);
	Utils::WriteParamUnsigned(entry_root, "U_04", val32(entry->unk_04), true);
	Utils::WriteParamUnsigned(entry_root, "U_08", val32(entry->unk_08), true);
	Utils::WriteParamUnsigned(entry_root, "U_0C", val32(entry->unk_0C), true);
	Utils::WriteParamUnsigned(entry_root, "U_10", val32(entry->unk_10), true);
	Utils::WriteParamUnsigned(entry_root, "U_14", val32(entry->unk_14), true);	
	
	root->LinkEndChild(entry_root);
}

#define ENTRY_U(n, f) { if (!Utils::GetParamUnsigned(root, n, &temp)) \
							return false; \
						entry->csc_entry.f = val32(temp); \
						}

bool CscFile::ReadEntry(TiXmlElement *root, CscEntry *entry)
{
	uint32_t temp;
	
	ENTRY_U("U_00", unk_00);
	ENTRY_U("U_04", unk_04);
	ENTRY_U("U_08", unk_08);
	ENTRY_U("U_0C", unk_0C);
	ENTRY_U("U_10", unk_10);
	ENTRY_U("U_14", unk_14);
	
	return true;
}

TiXmlDocument *CscFile::Decompile() const
{
	CHECK_LOADED_PTR();
	
	TiXmlDocument *doc = new TiXmlDocument();
	
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "" );	
	doc->LinkEndChild(decl);
	
	TiXmlElement *root = new TiXmlElement("CSC");
	
	for (uint32_t i = 0; i < num_entries; i++)
	{
		WriteEntry(root, i);
	}
	
	doc->LinkEndChild(root);
	
	return doc;
}

bool CscFile::Compile(TiXmlDocument *doc, bool big_endian)
{
	unsigned int num_entries = 0;
	std::vector<CscEntry> entries;
	
	Reset();
	this->big_endian = big_endian;
	
	TiXmlHandle handle(doc);
	TiXmlElement *root = NULL;;
	
	for (TiXmlElement *elem = handle.FirstChildElement().Element(); elem != NULL; elem = elem->NextSiblingElement())
	{
		std::string name = elem->ValueStr();
		
		if (name == "CSC")
		{
			root = elem;
			break;
		}
	}		
	
	if (!root)
	{
		DPRINTF("Cannot find \"CSC\" in xml.\n");
		return false;
	}
	
	// First pass to know number of entries, so that we can do later the index check.
	for (TiXmlElement *elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{
		std::string name = elem->ValueStr();
		
		if (name == "CharEntry")
		{
			num_entries++;
		}
	}
	
	if (num_entries == 0)
	{
		DPRINTF("No CharEntry found!\n");
		return false;
	}
	
	entries.resize(num_entries);
	
	for (TiXmlElement *elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{
		std::string name = elem->ValueStr();
		
		if (name == "CharEntry")
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
	
	
	this->size = sizeof(CSCHeader) + num_entries * sizeof(CSCEntry);
	this->buf = new uint8_t[this->size];
	if (!this->buf)
	{
		DPRINTF("We are short in memory.\n");
		this->size = 0;
		return false;
	}
	
	memset(this->buf, 0, this->size);
	
	uint8_t *ptr = this->buf + sizeof(CSCHeader);
	
	for (CscEntry &e: entries)
	{
		memcpy(ptr, &e.csc_entry, sizeof(CSCEntry));
		ptr += sizeof(CSCEntry);
	}
	
	// And finally, the header;	
	CSCHeader *hdr = (CSCHeader *)this->buf;
	
	hdr->signature = CSC_SIGNATURE;
	hdr->endianess_check = val32(0xFFFE);
	hdr->num_entries = val32(num_entries);
	hdr->data_start = val32(sizeof(CSCHeader));
	
	this->entries = (CSCEntry *)GetOffsetPtr(hdr, hdr->data_start);
	this->num_entries = val32(hdr->num_entries);	
	
	return true;
}

uint8_t *CscFile::Save(size_t *size)
{
	CHECK_LOADED_PTR();
	
	uint8_t *buf = new uint8_t[this->size];
		
	memcpy(buf, this->buf, this->size);
	*size = this->size;
	
	return buf;
}

