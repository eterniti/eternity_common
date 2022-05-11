#ifndef __UTFFILE_H__
#define __UTFFILE_H__


#include <stdint.h>
#include <vector>
#include <map>
#include "BaseFile.h"
#include "debug.h"

// "@UTF"
#define UTF_SIGNATURE	0x46545540

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

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
	uint32_t signature; // 0
	uint32_t table_size; // 4
	// size 8
} PACKED UTFHeader;

static_assert(sizeof(UTFHeader) == 8, "Incorrect structure size.");

typedef struct
{
	uint16_t unk_00; // 0           8
	uint16_t rows_offset; // 2      0xA
	uint32_t strings_offset; // 4   0xC
	uint32_t data_offset;	// 8    0x10
	uint32_t table_name; // C       0x14
	uint16_t num_columns; // 0x10   0x18
	uint16_t row_length; // 0x12    0x1A
	uint32_t num_rows; // 0x14      0x1C
	// size 0x18
} PACKED UTFTableHeader;

static_assert(sizeof(UTFTableHeader) == 0x18, "Incorrect structure size.");

#ifdef _MSC_VER
#pragma pack(pop)
#endif


struct UtfColumn
{
	uint8_t flags;
	std::string name;

	uint8_t constant_u8;
	uint16_t constant_u16;
	uint32_t constant_u32;
	uint64_t constant_u64;
	float constant_float;
	std::string constant_str;

	uint8_t *constant_data;
	uint32_t constant_data_size;

	void Copy(const UtfColumn &other)
	{
		flags = other.flags;
		name = other.name;
		constant_u8 = other.constant_u8;
		constant_u16 = other.constant_u16;
		constant_u32 = other.constant_u32;
		constant_u64 = other.constant_u64;
		constant_float = other.constant_float;
		constant_str = other.constant_str;

		if (other.constant_data && other.constant_data_size != 0)
		{
			constant_data = new uint8_t[other.constant_data_size];
			if (!constant_data)
			{
				DPRINTF("%s: Memory allocation error (0x%x)\n", FUNCNAME, other.constant_data_size);
				exit(-1);
			}

			memcpy(constant_data, other.constant_data, other.constant_data_size);
			constant_data_size = other.constant_data_size;
		}
		else
		{
			constant_data = nullptr;
			constant_data_size = 0;
		}
	}

	UtfColumn()
	{
		constant_data = nullptr;
	}

	UtfColumn(const UtfColumn &other)
    {
        Copy(other);
    }

	~UtfColumn()
	{
		if (constant_data)
			delete[] constant_data;
	}

	inline UtfColumn &operator=(const UtfColumn &other)
    {
        if (this == &other)
            return *this;

        Copy(other);
        return *this;
    }
};

struct UtfValue
{
	int type;

	uint8_t _u8;
	uint16_t _u16;
	uint32_t _u32;
	uint64_t _u64;
	float _float;
	std::string str;

	// Type data. WARNING: allocated.
	uint8_t *data;
	uint32_t data_size;

	void Copy(const UtfValue &other)
	{
		type = other.type;
		_u8 = other._u8;
		_u16 = other._u16;
		_u32 = other._u32;
		_u64 = other._u64;
		_float = other._float;
		str = other.str;

		if (other.data && other.data_size != 0)
		{
			data = new uint8_t[other.data_size];
			if (!data)
			{
				DPRINTF("%s: Memory allocation error (0x%x)\n", FUNCNAME, other.data_size);
				exit(-1);
			}

			memcpy(data, other.data, other.data_size);
			data_size = other.data_size;
		}
		else
		{
			data = nullptr;
			data_size = 0;
		}
	}

	UtfValue()
	{
		type = -1;
		data = nullptr;
	}

	UtfValue(const UtfValue &other)
    {
        Copy(other);
    }

	~UtfValue()
	{
		if (data)
			delete[] data;
	}

	inline UtfValue &operator=(const UtfValue &other)
    {
        if (this == &other)
            return *this;

        Copy(other);
        return *this;
    }
};

struct UtfRow
{
	std::vector<UtfValue> values;
};

class UtfFile : public BaseFile
{
private:

	bool is_empty;

	std::string table_name;
	bool add_null;
	uint16_t unk_00;

    size_t CalculateColumnsSize() const;
    uint16_t CalculateRowLength() const;
    size_t CalculateStringsSize() const;    

    char *FindString(char *strings_list, const char *str, char *end);

protected:

	std::vector<UtfColumn> columns;
	std::vector<UtfRow> rows;

public:

	UtfFile();
	virtual ~UtfFile();

	virtual void Reset();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

	std::string GetTableName() const { return table_name; }
	bool IsEmpty() const { return is_empty; }

    bool ColumnExists(const std::string &name) const;
    unsigned int ColumnIndex(const std::string &name) const;
    bool IsVariableColumn(unsigned int column) const;
    inline bool IsVariableColumn(const std::string &name) const { return IsVariableColumn(ColumnIndex(name)); }
    unsigned int GetNumColumns() const { return (unsigned int)columns.size(); }
    unsigned int GetNumRows() const { return (unsigned int)rows.size(); }

	bool GetByte(const std::string &name, uint8_t *byte, unsigned int row=0) const;
	bool GetWord(const std::string &name, uint16_t *word, unsigned int row=0) const;
	bool GetDword(const std::string &name, uint32_t *dword, unsigned int row=0) const;
	bool GetQword(const std::string &name, uint64_t *qword, unsigned int row=0) const;
	bool GetFloat(const std::string &name, float *f, unsigned int row=0) const;
	bool GetString(const std::string &name, std::string *str, unsigned int row=0) const;
    uint8_t *GetBlob(const std::string &name, unsigned int *size, bool alloc_new=false, unsigned int row=0) const;
	bool GetFixedBlob(const std::string &name, uint8_t *buf, unsigned int size, unsigned int row=0) const;

    bool SetByte(const std::string &name, uint8_t byte, unsigned int row=0, bool change_constant=true);
    bool SetWord(const std::string &name, uint16_t word, unsigned int row=0, bool change_constant=true);
    bool SetDword(const std::string &name, uint32_t dword, unsigned int row=0, bool change_constant=true);
    bool SetQword(const std::string &name, uint64_t qword, unsigned int row=0, bool change_constant=true);
    bool SetFloat(const std::string &name, float f, unsigned int row=0, bool change_constant=true);
    bool SetString(const std::string &name, const std::string &str, unsigned int row=0, bool change_constant=true);
    bool SetBlob(const std::string &name, uint8_t *blob, unsigned int blob_size, unsigned int row=0, bool take_ownership=false, bool change_constant=true);

    unsigned int GetHighestRow16(const std::string &name, uint16_t *val) const;
    unsigned int GetHighestRow32(const std::string &name, uint32_t *val) const;

    UtfRow *CreateRow();

    size_t CalculateFileSize() const;

    void Debug() const;
    void DebugDump() const;
};

#endif // __UTFFILE_H__
