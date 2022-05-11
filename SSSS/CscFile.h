#ifndef __CSCFILE_H__
#define __CSCFILE_H__

// TODO: this class needs a rewrite

#include <stdint.h>

#include <vector>

#include "BaseFile.h"

// "#CSC"
#define CSC_SIGNATURE	0x43534323

typedef struct
{
	uint32_t signature; // 0
	uint16_t endianess_check; // 4
	uint16_t unk_06; // 6   zero
	uint32_t num_entries; // 8
	uint32_t unk_0C; // 0x0C    zero
	uint32_t data_start; // 0x10
	uint32_t unk_14; // 0x14	
	// 0x18
} PACKED CSCHeader;

typedef struct
{
	uint32_t unk_00; // 0
	uint32_t unk_04; // 4
	uint32_t unk_08; // 8
	uint32_t unk_0C; // 0xC
	uint32_t unk_10; // 0x10
	uint32_t unk_14; // 0x14
	// 0x18	
} PACKED CSCEntry;

class CscEntry
{
public:
	CSCEntry csc_entry;
	bool initialized = false;
};


class CscFile : public BaseFile
{
	
private:

	uint8_t *buf;
	unsigned int size;
	
	CSCEntry *entries; // not-allocated	(ptr in buf)	
	uint32_t num_entries;
	
	void Reset();	
	
	void WriteEntry(TiXmlElement *root, uint32_t entry) const;
	bool ReadEntry(TiXmlElement *root, CscEntry *entry);

public:

	CscFile();
	CscFile(const uint8_t *buf, size_t size);
	virtual ~CscFile();
	
	virtual bool Load(const uint8_t *buf, size_t size) override;
	
	inline uint32_t GetNumEntries() { return num_entries; }
	
	virtual TiXmlDocument *Decompile() const override;
	virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;
	
	virtual uint8_t *Save(size_t *size) override;
};

#endif

