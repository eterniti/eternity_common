#include "SstFile.h"
#include "debug.h"

static std::vector<std::string> languages =
{
    "JP",
    "EN",
    "FR",
    "IT",
    "SP",
    "BR",
    "CH",
    "NS",
};

SstFile::SstFile()
{
	Reset();
}

SstFile::~SstFile()
{
	
}

void SstFile::Reset()
{
	sign_name.clear();
	sign_name.resize(NUM_LANGUAGES);
	
	robes_name.clear();
	robes_name.resize(NUM_LANGUAGES);
}

std::string SstFile::GetSignName(unsigned int language, bool revert_to_english)
{
	if (language >= sign_name.size())
		return "<NULL>";
	
	std::string &s = sign_name[language];
	
	if (s.length() == 0 && language != ENGLISH && revert_to_english)
	{
		return sign_name[ENGLISH];
	}
	
	return s;
}

std::string SstFile::GetRobesName(unsigned int language, bool revert_to_english)
{
	if (language >= robes_name.size())
		return "<NULL>";
	
	std::string &s = robes_name[language];
	
	if (s.length() == 0 && language != ENGLISH && revert_to_english)
	{
		return robes_name[ENGLISH];
	}
	
	return s;
}

void SstFile::WriteSignName(TiXmlElement *root) const
{
	for (size_t i = 0; i < sign_name.size(); i++)
	{
        const std::string &s = sign_name[i];
		
		if (s.length() > 0)
		{
			std::string param_name = "SIGN_NAME_" + languages[i];
			
			Utils::WriteParamString(root, param_name.c_str(), s);
		}
	}
}

void SstFile::WriteRobesName(TiXmlElement *root) const
{
	for (size_t i = 0; i < robes_name.size(); i++)
	{
        const std::string &s = robes_name[i];
		
		if (s.length() > 0)
		{
			std::string param_name = "ROBES_NAME_" + languages[i];
			
			Utils::WriteParamString(root, param_name.c_str(), s);
		}
	}
}

size_t SstFile::ReadSignName(const TiXmlElement *root)
{
	size_t count = 0;
	
	for (size_t i = 0; i < sign_name.size(); i++)
	{
		std::string param_name = "SIGN_NAME_" + languages[i];
		
		if (Utils::ReadParamString(root, param_name.c_str(), sign_name[i]))
		{
			count++;
		}
	}
	
	return count;
}

size_t SstFile::ReadRobesName(const TiXmlElement *root)
{
	size_t count = 0;
	
	for (size_t i = 0; i < robes_name.size(); i++)
	{
		std::string param_name = "ROBES_NAME_" + languages[i];
		
		if (Utils::ReadParamString(root, param_name.c_str(), robes_name[i]))
		{
			count++;
		}
	}
	
	return count;
}

TiXmlDocument *SstFile::Decompile() const
{
	TiXmlDocument *doc = new TiXmlDocument();
	
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );	
	doc->LinkEndChild(decl);
	
	TiXmlElement *root = new TiXmlElement("SST");
	Utils::WriteComment(root, " This file should be read and written with UTF-8 encoding. ");
		
	WriteSignName(root);
	WriteRobesName(root);
	
	doc->LinkEndChild(root);
	
	return doc;
}

bool SstFile::Compile(const TiXmlElement *root)
{
	Reset();
	
	ReadSignName(root);
	ReadRobesName(root);
	
	return true;
}

bool SstFile::Compile(TiXmlDocument *doc, bool big_endian)
{
	UNUSED(big_endian);
	
	Reset();
	
	TiXmlHandle handle(doc);
	TiXmlElement *root = NULL;;
	
	for (TiXmlElement *elem = handle.FirstChildElement().Element(); elem != NULL; elem = elem->NextSiblingElement())
	{
		std::string name = elem->ValueStr();
		
		if (name == "SST")
		{
			root = elem;
			break;
		}
	}		
	
	if (!root)
	{
		DPRINTF("%s: Cannot find \"SST\" in xml.\n", __PRETTY_FUNCTION__);
		return false;
	}
	
	return Compile(root);
}
