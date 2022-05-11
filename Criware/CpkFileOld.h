#include <stdint.h>
#include <vector>
#include <map>
#include "BaseFile.h"

// "CPK "
#define CPK_SIGNATURE	0x204B5043

// "@UTF"
#define UTF_SIGNATURE	0x46545540

// "TOC "
#define TOC_SIGNATURE	0x20434F54

enum COLUMN_FLAGS
{
	STORAGE_MASK = 0xf0,
	STORAGE_NONE = 0x00,
	STORAGE_ZERO = 0x10,
	STORAGE_CONSTANT = 0x30,
	STORAGE_PERROW = 0x50,


	TYPE_MASK = 0x0f,
	TYPE_DATA = 0x0b,
	TYPE_STRING = 0x0a,
	TYPE_FLOAT = 0x08,
	TYPE_8BYTE2 = 0x07,
	TYPE_8BYTE = 0x06,
	TYPE_4BYTE2 = 0x05,
	TYPE_4BYTE = 0x04,
	TYPE_2BYTE2 = 0x03,
	TYPE_2BYTE = 0x02,
	TYPE_1BYTE2 = 0x01,
	TYPE_1BYTE = 0x00,
};

typedef struct
{
	uint32_t signature; // 0
	uint32_t unk_04; // 4
	// size 0x8
} PACKED CPKHeader;

typedef struct
{
	uint32_t signature; // 0
	uint32_t table_size; // 4
	// size 8
} PACKED UTFHeader;

typedef struct
{
	uint32_t rows_offset; // 0
	uint32_t strings_offset; // 4
	uint32_t data_offset;	// 8
	uint32_t table_name; // C
	uint16_t num_columns; // 0x10
	uint16_t row_length; // 0x12
	uint32_t num_rows; // 0x14
	// size 0x18
} PACKED UTFTableHeader;

class Column
{
public:
	uint8_t flags;
	char *name;
};

class Row
{
public:

	int type;
	
	uint32_t data_size; // only for type data
	
	uint32_t position;
	
	union
	{
		uint8_t *_u8;
		uint16_t *_u16;
		uint32_t *_u32;
		uint64_t *_u64;
		float *_float;
		char *str;
		uint8_t *data;
	} u1;
	
	Row()
	{
		type = -1;
	}
	
	void *GetValue();	
	
}; 

class Rows
{
public:

	std::vector<Row> rows;
	
	Rows()
	{
		
	}
	
	~Rows()
	{
		
	}
};

class CpkUTF : public BaseFile
{
private:

	uint8_t *top;
	UTFTableHeader *table_hdr; 
	char *strings;
	uint8_t *data;
	uint32_t table_size;
	
	bool is_encrypted;	
	bool encrypt_status;
	
	void ToggleEncryption();

public:

	std::vector<Column> columns;
	std::vector<Rows> rows;

	CpkUTF();
	~CpkUTF();
	
	bool Load(uint8_t *buf);
	
	inline bool IsEncrypted() { return this->is_encrypted; }
	
	void RevertEncryption();
	
	inline uint8_t *GetPtrTable() { return (uint8_t *)table_hdr; }

};

class FileEntry 
{
public:

	char *dir_name;
	char *file_name;
	
	uint64_t file_size;
	uint64_t extract_size;
	uint64_t file_offset;
	
	uint32_t file_name_pos;
	uint32_t dir_name_pos;
};

class CpkFile : public BaseFile
{
private:	

	CpkUTF utf;
	CpkUTF toc;
	
	std::vector<FileEntry> files_table;
		
	uint64_t toc_offset;
	uint64_t etoc_offset;
	uint64_t itoc_offset;
	uint64_t gtoc_offset;
	uint64_t content_offset;
	
	uint64_t toc_size;
	uint64_t etoc_size;
	uint64_t itoc_size;
	uint64_t gtoc_size;
	uint64_t content_size;
	
	uint64_t sorted; // really a bool....
	
	std::map<char *, void *> cpk_data;
	
	void *GetColumnData(CpkUTF & utf, int row, const char *name);	
	uint64_t GetColumnDataAsInteger(CpkUTF & utf, int row, const char *name);
	uint32_t GetColumnPosition(CpkUTF & utf, int row, const char *name);
	
	bool SetColumnDataInteger(CpkUTF & utf, int row, const char *name, uint64_t value);	
	void SetUnsorted();
	
	void CorrectValueForOffset(uint64_t *offset);
	void CorrectValueForSize(uint64_t *size);		

public:

	CpkFile();
	virtual ~CpkFile();
	
	// usually a 0x800 bytes here will suffix, that's what the engine does
	bool ParseHeaderData(uint8_t *buf);
	// pass a buf of toc_size
	bool ParseTocData(uint8_t *buf);
	
	inline uint64_t GetTocOffset() { return toc_offset; }
	inline uint64_t GetTocSize() { return toc_size; }
	
	inline size_t GetNumFiles() { return files_table.size(); }
	inline FileEntry *GetFileAt(int index) { return &files_table[index]; }
	
	void RevertEncryption(bool only_toc);
	
	bool UnlinkFilename(int file_index);
	bool UnlinkFileFromDirectory(int file_index);
};

