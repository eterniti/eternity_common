#ifndef __BASEFILE_H__
#define __BASEFILE_H__

#include <stdint.h>
#include <string>
#include <vector>

#include "common.h"
#include "Utils.h"
#include "tinyxml/tinyxml.h"

#ifndef PACKED

#ifdef _MSC_VER
#define PACKED
#else
#define PACKED  __attribute__((packed))
#endif

#endif // PACKED

#define STATIC_ASSERT_STRUCT(c, s)	static_assert(sizeof(c) == s, "Incorrect structure size.")
#define CHECK_STRUCT_SIZE(c, s)	static_assert(sizeof(c) == s, "Incorrect structure size.")
#define CHECK_FIELD_OFFSET(c, f, o)	static_assert(offsetof(c, f) == o, "Incorrect field offset.")

class BaseFile
{
protected:
	
	bool big_endian;
	
    uint64_t val64(uint64_t val) const;
    uint32_t val32(uint32_t val) const;
    uint16_t val16(uint16_t val) const;
    float val_float(float val) const;

    void copy_float(void *x, float val) const;

    static inline uint64_t re64(uint64_t val)
    {
#ifdef __BIG_ENDIAN__
        return LE64(val);
#else
        return BE64(val);
#endif
    }

    static inline uint32_t re32(uint32_t val)
    {
#ifdef __BIG_ENDIAN__
        return LE32(val);
#else
        return BE32(val);
#endif
    }

    static inline uint16_t re16(uint16_t val)
    {
#ifdef __BIG_ENDIAN__
        return LE16(val);
#else
        return BE16(val);
#endif
    }

    static inline void re_float(void *x, float val)
    {
        uint32_t *p = (uint32_t *)&val;
        *(uint32_t *)x = re32(*p);
    }

    static inline uint64_t le64(uint64_t val)
    {
        return LE64(val);
    }

    static inline uint32_t le32(uint32_t val)
    {
        return LE32(val);
    }

    static inline uint16_t le16(uint16_t val)
    {
        return LE16(val);
    }

    static inline void le_float(void *x, float val)
    {
        uint32_t *p = (uint32_t *)&val;
        *(uint32_t *)x = le32(*p);
    }

    static inline uint64_t be64(uint64_t val)
    {
        return BE64(val);
    }

    static inline uint32_t be32(uint32_t val)
    {
        return BE32(val);
    }

    static inline uint16_t be16(uint16_t val)
    {
        return BE16(val);
    }

    static inline void be_float(void *x, float val)
    {
        uint32_t *p = (uint32_t *)&val;
        *(uint32_t *)x = be32(*p);
    }
	
    uint8_t *GetOffsetPtr(const void *base, uint32_t offset, bool native=false) const;
    uint8_t *GetOffsetPtr(const void *base, const uint32_t *offsets_table, uint32_t idx, bool native=false) const;

    void ModTable(uint32_t *offsets, unsigned int count, int mod_by);
	unsigned int GetHighestValueIdx(uint32_t *values, unsigned int count, bool native=false);		
    const char *FindString(const char *list, const char *str, size_t num_str);

    std::string GetString(const void *base, uint32_t offset, bool native=false) const;
    std::u16string GetString16(const void *base, uint32_t offset, bool native=false) const;
    std::string GetString16AsUtf8(const void *base, uint32_t offset, bool native=false) const;

    uint32_t GetStringOffset(uint32_t str_base, const std::vector<std::string> &list, const std::string &str);
    void *WriteStringList(void *buf, const std::vector<std::string> &list);
	
public:
	// Don't make any function abstract, instead let's provide an empty default implementation

	virtual ~BaseFile() { }

    inline bool IsBigEndian() { return big_endian; }
    inline void SetEndianess(bool big_endian) { this->big_endian = big_endian; }
	
    virtual bool Load(const uint8_t *buf, size_t size) { UNUSED(buf); UNUSED(size); return false; }
	virtual bool LoadFromFile(const std::string &path, bool show_error=true);
   	
    virtual TiXmlDocument *Decompile() const { return nullptr; }
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) { UNUSED(doc); UNUSED(big_endian); return false; }
	
	virtual bool DecompileToFile(const std::string &path, bool show_error=true, bool build_path=false);		
	virtual bool CompileFromFile(const std::string &path, bool show_error=true, bool big_endian=false);	
	
    virtual uint8_t *Save(size_t *psize) { UNUSED(psize); return nullptr; }
	virtual bool SaveToFile(const std::string &path, bool show_error=true, bool build_path=false);
	
	bool SmartLoad(const std::string &path, bool show_error=true, bool xml_big_endian=false);
    bool SmartSave(const std::string &path, bool show_error=true, bool build_path=false);
};

#endif
