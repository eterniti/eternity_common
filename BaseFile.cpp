#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BaseFile.h"
#include "Utils.h"
#include "common.h"
#include "debug.h"

uint64_t BaseFile::val64(uint64_t val) const
{
#ifdef __BIG_ENDIAN__
	return (big_endian) ? val : LE64(val)
#else
    return (big_endian) ? BE64(val) : val;
#endif
}

uint32_t BaseFile::val32(uint32_t val) const
{
#ifdef __BIG_ENDIAN__
	return (big_endian) ? val : LE32(val)
#else
	return (big_endian) ? BE32(val) : val;
#endif
}

uint16_t BaseFile::val16(uint16_t val) const
{
#ifdef __BIG_ENDIAN__
	return (big_endian) ? val : LE16(val)
#else
	return (big_endian) ? BE16(val) : val;
#endif
}

float BaseFile::val_float(float val) const
{
    uint32_t *p = (uint32_t *)&val;

    *p = val32(*p);
	
    return val;
}

void BaseFile::copy_float(void *x, float val) const
{
    uint32_t *p = (uint32_t *)&val;
    *(uint32_t *)x = val32(*p);
}

uint8_t *BaseFile::GetOffsetPtr(const void *base, uint32_t offset, bool native) const
{
	if (native)
		return ((uint8_t *)base+offset);

	return ((uint8_t *)base+val32(offset));
}

uint8_t *BaseFile::GetOffsetPtr(const void *base, const uint32_t *offsets_table, uint32_t idx, bool native) const
{
	if (native)
		return ((uint8_t *)base+offsets_table[idx]);
	
	return ((uint8_t *)base+val32(offsets_table[idx]));
}

void BaseFile::ModTable(uint32_t *offsets, unsigned int count, int mod_by)
{
	for (unsigned int i = 0; i < count; i++)
	{
		offsets[i] = val32(val32(offsets[i]) + mod_by);
	}
}

unsigned int BaseFile::GetHighestValueIdx(uint32_t *values, unsigned int count, bool native)
{
	unsigned int max_idx = 0xFFFFFFFF;
	uint32_t max = 0;
	
	for (unsigned int i = 0; i < count; i++)
	{
		uint32_t val = (native) ? values[i] : val32(values[i]);
		
		if (val > max)
		{
			max = val;
			max_idx = i;
		}
	}
	
	return max_idx;	
}

const char *BaseFile::FindString(const char *list, const char *str, size_t num_str)
{
    for (size_t i = 0; i < num_str; i++)
	{
		if (strcmp(list, str) == 0)
			return list;	
		
		list += strlen(list) + 1;
	}
	
    return nullptr;
}

std::string BaseFile::GetString(const void *base, uint32_t offset, bool native) const
{
    if (offset == 0)
        return std::string();

    return (char *)GetOffsetPtr(base, offset, native);
}

std::u16string BaseFile::GetString16(const void *base, uint32_t offset, bool native) const
{
    if (offset == 0)
        return std::u16string();

    return (char16_t *)GetOffsetPtr(base, offset, native);
}

std::string BaseFile::GetString16AsUtf8(const void *base, uint32_t offset, bool native) const
{
    if (offset == 0)
        return std::string();

    std::u16string temp = (char16_t *)GetOffsetPtr(base, offset, native);
    return Utils::Ucs2ToUtf8(temp);
}

uint32_t BaseFile::GetStringOffset(uint32_t str_base, const std::vector<std::string> &list, const std::string &str)
{
    if (str.length() == 0)
        return 0;

    uint32_t offset = str_base;

    for (const std::string &str2 : list)
    {
        if (str == str2)
            return offset;

        offset += (uint32_t) str2.length()+1;
    }

    //DPRINTF("buuu\n");
    return 0xFFFFFFFF;
}

void *BaseFile::WriteStringList(void *buf, const std::vector<std::string> &list)
{
    char *buf8 = (char *)buf;
    uint32_t str_offset = 0;

    for (const std::string &str : list)
    {
        strcpy(buf8+str_offset, str.c_str());
        str_offset += (uint32_t)str.length() + 1;
    }

    return (buf8+str_offset);
}

bool BaseFile::LoadFromFile(const std::string &path, bool show_error)
{
	uint8_t *buf;
	size_t size;
	
	buf = Utils::ReadFile(path, &size, show_error);
	if (!buf)
		return false;
	
	bool ret = Load(buf, size);
	delete[] buf;
	
	return ret;
}

bool BaseFile::SaveToFile(const std::string &path, bool show_error, bool build_path)
{
    size_t size;
	
    uint8_t *buf = Save(&size);
	
	if (!buf)
		return false;
	
	bool ret = Utils::WriteFileBool(path, buf, size, show_error, build_path);
	delete[] buf;
	
	return ret;
}

bool BaseFile::DecompileToFile(const std::string &path, bool show_error, bool build_path)
{
	TiXmlDocument *doc = Decompile();
	
	if (!doc)
    {
        if (show_error)
        {
             DPRINTF("Decompilation of file \"%s\" failed.\n", path.c_str());
        }

        return false;
    }
	
	if (build_path)
	{
		if (!Utils::CreatePath(path))
        {
            if (show_error)
            {
                DPRINTF("Cannot create path for file \"%s\"\n", path.c_str());
            }

            return false;
        }
	}
	
	bool ret = doc->SaveFile(path);
	delete doc;

    if (!ret && show_error)
    {
        DPRINTF("Cannot create/write file \"%s\"\n", path.c_str());
    }
	
	return ret;
}

bool BaseFile::CompileFromFile(const std::string &path, bool show_error, bool big_endian)
{
	TiXmlDocument doc;
	
	if (!doc.LoadFile(path))
	{
		if (show_error)
		{
			if (doc.ErrorId() == TiXmlBase::TIXML_ERROR_OPENING_FILE)
			{
				DPRINTF("Cannot open file \"%s\"\n", path.c_str());				
			}
			else
			{
				DPRINTF("Error parsing file \"%s\". This is what tinyxml has to say: %s. Row=%d, col=%d.\n", path.c_str(), doc.ErrorDesc(), doc.ErrorRow(), doc.ErrorCol());
			}
		}
		
		return false;
	}
	
	bool ret = Compile(&doc, big_endian);
	
	if (!ret && show_error)
	{
		DPRINTF("Compilation of file \"%s\" failed.\n", path.c_str());
	}
	
	return ret;
}

bool BaseFile::SmartLoad(const std::string &path, bool show_error, bool xml_big_endian)
{
	if (Utils::EndsWith(path, ".xml", false))
		return CompileFromFile(path, show_error, xml_big_endian);		
		
	return LoadFromFile(path, show_error);
}

bool BaseFile::SmartSave(const std::string &path, bool show_error, bool build_path)
{
	if (Utils::EndsWith(path, ".xml", false))
		return DecompileToFile(path, show_error, build_path);
	
	return SaveToFile(path, show_error, build_path);
}

