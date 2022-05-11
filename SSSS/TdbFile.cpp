#include "TdbFile.h"
#include "debug.h"

TdbFile::TdbFile()
{
    big_endian = false;
    Reset();
}

TdbFile::TdbFile(uint8_t *buf, unsigned int size)
{
    Load(buf, size);
}

TdbFile::~TdbFile()
{
	
}

void TdbFile::Reset()
{
    strings.clear();
}

const std::string TdbFile::GetLanguagePrefix(size_t language)
{
    static const std::vector<std::string> prefixes =
    {
        "JP",
        "EN",
        "FR",
        "IT",
        "SP",
        "BR",
        "NS",
        "CH",
    };

    if (language < prefixes.size())
    {
        return prefixes[language];
    }

    return "LANG" + Utils::ToString(language);
}

bool TdbFile::Load(const uint8_t *buf, size_t size)
{
    TDBHeader *hdr = (TDBHeader *)buf;

    if (size < sizeof(TDBHeader) || hdr->signature != TDB_SIGNATURE)
     return false;
	 
    const uint8_t *ptr = buf + sizeof(TDBHeader);
    uint32_t idx, len;

    strings = std::vector<std::vector<std::u16string>>(val32(hdr->num_strings), std::vector<std::u16string>(val32(hdr->num_languages)));

    uint32_t i = 0;

    while (ptr < (buf+size))
    {
        idx = val32(*(uint32_t *)ptr);
        ptr += sizeof(uint32_t);

        if (idx != i)
        {
            DPRINTF("%s: we were expecting ordered strings.\n", FUNCNAME);
            return false;
        }

        i++;

        if (idx >= strings.size())
        {
            DPRINTF("%s: idx out of range (0x%x)\n", FUNCNAME, idx);
            return false;
        }

        std::vector<std::u16string> &v = strings[idx];

        for (uint32_t i = 0; i < v.size(); i++)
        {
            std::u16string &s = v[i];

            len = val32(*(uint32_t *)ptr);
            ptr += sizeof(uint32_t);

            if (len & 1)
            {
                DPRINTF("%s: odd len!\n", FUNCNAME);
                return false;
            }

            // TODO: have u16 strings and do conversion to utf8...
            for (uint32_t j = 0; j < len/2; j++)
            {
                char16_t c = *(char16_t *)ptr;

                if (c != 0)
                    s += c;

                ptr += 2;
            }
        }
    }

    unk_0C = hdr->unk_0C;    

    /*for (uint32_t i = 0; i < GetNumStrings(); i++)
    {
     UPRINTF("0x%x: %s\n", i, GetString(i).c_str());
    } */

    return true;
}

std::string TdbFile::GetString(uint32_t idx, unsigned int language) const
{
    if (idx >= strings.size())
		return "<NULL>";
	
    const std::vector<std::u16string> &v = strings[idx];
	
	if (language >= v.size())
		return "<NULL>";
	
	return Utils::Ucs2ToUtf8(v[language]);
}

bool TdbFile::SetString(uint32_t idx, const std::string & str, unsigned int language)
{
    if (idx >= strings.size())
		return false;
	
	std::vector<std::u16string> &v = strings[idx];	
	
	v[language] = Utils::Utf8ToUcs2(str);	
	return true;
}

uint32_t TdbFile::AppendString(const std::vector<std::string> & v)
{
    if (v.size() != GetNumLanguages())
		return (uint32_t)-1;
	
	uint32_t idx = strings.size();		
	std::vector<std::u16string> v16(GetNumLanguages()); 
	
	for (size_t i = 0; i < GetNumLanguages(); i++)
	{
		v16[i] = Utils::Utf8ToUcs2(v[i]);
	}
	
	strings.push_back(v16);	
	return idx;
}

unsigned int TdbFile::CalculateFileSize() const
{
	unsigned int file_size = sizeof(TDBHeader);
	
    for (const std::vector<std::u16string> &v : strings)
	{
		file_size += sizeof(uint32_t); // idx
		
        for (const std::u16string &s : v)
		{
			file_size += sizeof(uint32_t); // length in bytes
			file_size += ((s.length()+1) * 2);			
		}
	}
	
	return file_size;
}

uint8_t *TdbFile::Save(size_t *psize)
{
    unsigned int file_size = CalculateFileSize();
	
    uint8_t *buf = new uint8_t[file_size];
	TDBHeader *hdr = (TDBHeader *)buf;
	
	hdr->signature = TDB_SIGNATURE;
	hdr->num_strings = val32(GetNumStrings());
	hdr->num_languages = val32(GetNumLanguages());
	hdr->unk_0C = val32(unk_0C);
	
	uint8_t *ptr = buf + sizeof(TDBHeader);
	
	for (size_t i = 0; i < strings.size(); i++)
	{
		std::vector<std::u16string> &v = strings[i];
		
		*(uint32_t *)ptr = val32(i);
		ptr += sizeof(uint32_t);
		
		for (std::u16string &s : v)
		{
			uint32_t len = (s.length()+1) * 2;
			
			*(uint32_t *)ptr = val32(len);			
			ptr += sizeof(uint32_t);			
			
			memcpy(ptr, s.c_str(), len);
			ptr += len;
		}
	}
	
	*psize = file_size;
    return buf;
}

TiXmlDocument *TdbFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("TDB");
    Utils::WriteComment(root, " This file should be read and written with UTF-8 encoding. ");

    Utils::WriteParamUnsigned(root, "U_0C", unk_0C, true);

    for (size_t i = 0; i < strings.size(); i++)
    {
        TiXmlElement *entry = new TiXmlElement("TdbEntry");

        entry->SetAttribute("id", Utils::UnsignedToString(i, true));

        for (size_t j = 0; j < GetNumLanguages(); j++)
        {
            Utils::WriteParamString(entry, GetLanguagePrefix(j).c_str(), GetString(i, j));
        }

        root->LinkEndChild(entry);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool TdbFile::Compile(TiXmlDocument *doc, bool big_endian)
{
    Reset();
    this->big_endian = big_endian;

    TiXmlHandle handle(doc);
    TiXmlElement *root = Utils::FindRoot(&handle, "TDB");

    if (!root)
    {
        DPRINTF("%s: Cannot find TDB root.\n", FUNCNAME);
        return false;
    }

    if (!Utils::GetParamUnsigned(root, "U_0C", &unk_0C))
        return false;

    size_t count = Utils::GetElemCount(root, "TdbEntry");
    if (count == 0)
        return true;

    std::vector<bool> initialized;

    strings.resize(count);
    initialized.resize(count);

    uint32_t num_languages = 0;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "TdbEntry")
        {
            uint32_t id;

            if (!Utils::ReadAttrUnsigned(elem, "id", &id))
            {
                DPRINTF("%s: cannot get attribute id.\n", FUNCNAME);
                return false;
            }

            if (id >= strings.size())
            {
                DPRINTF("%s: id 0x%x is out of range. Id must be between 0 and number of entries-1.\n", FUNCNAME, id);
                return false;
            }

            if (initialized[id])
            {
                DPRINTF("%s: Element with id 0x%x already exists!\n", FUNCNAME, id);
                return false;
            }

            uint32_t count;

            if (num_languages == 0)
            {
                count = 32; // This is the max number of languages we will support;
            }
            else
            {
                count = num_languages;
            }

            for (size_t lang = 0; lang < count; lang++)
            {
                std::string text;

                if (!Utils::ReadParamString(elem, GetLanguagePrefix(lang).c_str(), text))
                    break;

                strings[id].push_back(Utils::Utf8ToUcs2(text));
            }

            if (num_languages == 0)
            {
                num_languages = strings[id].size();
            }
            else if (strings[id].size() != num_languages)
            {
                DPRINTF("%s: number of languages for entry 0x%x doesn't match what was found in a previous entry.\n", FUNCNAME, id);
                return false;
            }

            initialized[id] = true;
        }
    }

    return true;
}
