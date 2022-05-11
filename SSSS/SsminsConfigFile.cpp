#include "SsminsConfigFile.h"
#include "debug.h"

SsminsConfigFile::SsminsConfigFile()
{
	
}

SsminsConfigFile::~SsminsConfigFile()
{
	
}

void SsminsConfigFile::Reset()
{
	mods.clear();
}

void SsminsConfigFile::WriteMod(TiXmlElement *root, const SsssMod *mod) const
{
	std::string type;
	TiXmlElement *entry_root = new TiXmlElement("MOD");
	
	if (mod->type == SszType::REPLACER)
		type = "REPLACER";
	else
		type = "NEW_CHARACTER";
	
	entry_root->SetAttribute("type", type);
	
    Utils::WriteParamString(entry_root, "MOD_NAME", mod->name);
    Utils::WriteParamString(entry_root, "MOD_AUTHOR", mod->author);
    Utils::WriteParamFloat(entry_root, "MOD_VERSION", mod->version);
    Utils::WriteParamGUID(entry_root, "MOD_GUID", mod->guid);
	
    for (const std::string &file : mod->files)
	{
        Utils::WriteParamString(entry_root, "FILE", file);
	}

    if (mod->type == SszType::NEW_CHARACTER)
    {
        Utils::WriteParamUnsigned(entry_root, "CMS_ENTRY", mod->cms_entry, true);
        Utils::WriteParamUnsigned(entry_root, "CMS_MODEL_SPEC_IDX", mod->cms_model_spec_idx);
        Utils::WriteParamUnsigned(entry_root, "MODEL_ID", mod->model_id, true);
    }
	
	root->LinkEndChild(entry_root);
}

bool SsminsConfigFile::ReadMod(TiXmlElement *root, SsssMod *mod)
{
	std::string type;
	
	if (Utils::ReadAttrString(root, "type", type))
	{
		if (type == "NEW_CHARACTER")
		{
			mod->type = SszType::NEW_CHARACTER;
		}
		else if (type == "REPLACER")
		{
			mod->type = SszType::REPLACER;
		}
		else
		{
			DPRINTF("%s: Unknown value for type.\n", __PRETTY_FUNCTION__);
			return false;
		}
	}
	else
	{
		mod->type = SszType::NEW_CHARACTER;
	}
	
	if (!Utils::GetParamString(root, "MOD_NAME", mod->name))
		return false;
	
	if (!Utils::ReadParamString(root, "MOD_AUTHOR", mod->author))
		mod->author = "";
	
	if (!Utils::ReadParamFloat(root, "MOD_VERSION", &mod->version))
		mod->version = 1.0f;
		
	if (!Utils::GetParamGUID(root, "MOD_GUID", mod->guid))
		return false;
	
	for (TiXmlElement *elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{
		std::string name = elem->ValueStr();
		
		if (name == "FILE")
		{
			std::string file;
			
			if (elem->QueryStringAttribute("value", &file) != TIXML_SUCCESS)
				return false;
			
			mod->files.push_back(file);
		}
	}

    if (mod->type == SszType::NEW_CHARACTER)
    {
        if (!Utils::GetParamUnsigned(root, "CMS_ENTRY", &mod->cms_entry))
            return false;

        if (!Utils::GetParamUnsigned(root, "CMS_MODEL_SPEC_IDX", &mod->cms_model_spec_idx))
            return false;

        if (!Utils::GetParamUnsigned(root, "MODEL_ID", &mod->model_id))
            return false;
    }
	
	return true;
}

TiXmlDocument *SsminsConfigFile::Decompile() const
{
	TiXmlDocument *doc = new TiXmlDocument();
	
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );	
	doc->LinkEndChild(decl);
	
	TiXmlElement *root = new TiXmlElement("SSMINS");    
    Utils::WriteParamString(root, "SSSS_DIR", ssss_dir);
    Utils::WriteParamString(root, "LAST_DIR", last_dir);
		
    for (const SsssMod &mod : mods)
	{
		WriteMod(root, &mod);
	}
	
	doc->LinkEndChild(root);	
	return doc;
}

bool SsminsConfigFile::Compile(TiXmlDocument *doc, bool big_endian)
{
	TiXmlHandle handle(doc);
	TiXmlElement *root = NULL;

    UNUSED(big_endian);
	
	Reset();
	
	for (TiXmlElement *elem = handle.FirstChildElement().Element(); elem != NULL; elem = elem->NextSiblingElement())
	{
		std::string name = elem->ValueStr();
		
		if (name == "SSMINS")
		{
			root = elem;
			break;
		}
	}		
	
	if (!root)
	{
		DPRINTF("%s: cannot find \"SSMINS\" in xml.\n", __PRETTY_FUNCTION__);
		return false;
	}
	
	if (!Utils::ReadParamString(root, "SSSS_DIR", ssss_dir))
		ssss_dir = "";

    if (!Utils::ReadParamString(root, "LAST_DIR", last_dir))
        last_dir = "";
	
	// First pass to know number of entries, so that we can do later the index check.
	for (TiXmlElement *elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{
		std::string name = elem->ValueStr();
		
		if (name == "MOD")
		{
			SsssMod mod;
			
			if (ReadMod(elem, &mod))
			{
				mods.push_back(mod);
			}			
		}
	}
	
	return true;
}


