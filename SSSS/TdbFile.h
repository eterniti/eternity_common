#ifndef __TDBFILE_H__
#define __TDBFILE_H__

#include <stdint.h>

#include <string>
#include <vector>

#include "BaseFile.h"

#ifdef CreateFile
#undef CreateFile
#endif

enum
{
	JAPANESE,
	ENGLISH,
	FRENCH,
	ITALIAN,
	SPANISH1,
	PORTUGUESE,
	SPANISH2,
	CHINESE,
	NUM_LANGUAGES,
};

// "#TDB"
#define TDB_SIGNATURE	0x42445423

typedef struct
{
	uint32_t signature; // 0
	uint32_t num_strings; // 4
	uint32_t num_languages; // 8
	uint32_t unk_0C; // 0xC
	// 0x10
} PACKED TDBHeader;

class TdbFile : public BaseFile
{
	
private:

    uint32_t unk_0C;
	
	std::vector<std::vector<std::u16string>> strings;
	
	void Reset();

    static const std::string GetLanguagePrefix(size_t language);
	
    unsigned int CalculateFileSize() const;

public:

	TdbFile();
	TdbFile(uint8_t *buf, unsigned int size);
    virtual ~TdbFile();
	
    inline uint32_t GetNumStrings() const
	{ 
        return strings.size();
	}
	
    inline uint32_t GetNumLanguages() const
	{
        return strings[0].size();
	}
	
    std::string GetString(uint32_t idx, unsigned int language = ENGLISH) const;
	bool SetString(uint32_t idx, const std::string & str, unsigned int language = ENGLISH);	
	
	uint32_t AppendString(const std::vector<std::string> & v);
	
    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

};

#endif
