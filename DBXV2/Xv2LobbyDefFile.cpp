#include "Xv2LobbyDefFile.h"

TiXmlElement *Xv2LobbyDef::Decompile(TiXmlNode *root, size_t idx) const
{
    TiXmlElement *entry_root = new TiXmlElement("LobbyDef");

    if (debug_name.length() > 0)
        entry_root->SetAttribute("debug_name", debug_name);

    entry_root->SetAttribute("info_id", (int)(idx+1));

    entry_root->SetAttribute("map", map);
    entry_root->SetAttribute("spm", spm);
    entry_root->SetAttribute("spm_dir", spm_dir);
    entry_root->SetAttribute("integer", integer);
    entry_root->SetAttribute("eflag", (eflag) ? "true" : "false");

    if (deleted)
        entry_root->SetAttribute("deleted", true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool Xv2LobbyDef::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrString(root, "debug_name", debug_name))
        debug_name.clear();

    if (!Utils::ReadAttrString(root, "map", map))
    {
        DPRINTF("%s: Attribute \"map\" is mandatory.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrString(root, "spm", spm))
    {
        DPRINTF("%s: Attribute \"spm\" is mandatory.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrString(root, "spm_dir", spm_dir))
    {
        DPRINTF("%s: Attribute \"spm_dir\" is mandatory.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrSigned(root, "integer", &integer))
    {
        DPRINTF("%s: Attribute \"integer\" is mandatory.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrBoolean(root, "eflag", &eflag))
    {
        DPRINTF("%s: Attribute \"eflag\" is mandatory.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrBoolean(root, "deleted", &deleted))
        deleted = false;

    return true;
}

Xv2LobbyDefFile::Xv2LobbyDefFile()
{

}

Xv2LobbyDefFile::~Xv2LobbyDefFile()
{

}

void Xv2LobbyDefFile::Reset()
{
    defs.clear();
}

TiXmlDocument *Xv2LobbyDefFile::Decompile() const
{
    if (defs.size() > MAX_LOBBY_MODDED)
    {
        DPRINTF("%s: Cannot save because file bypassed max number of lobbies.\n", FUNCNAME);
        return nullptr;
    }

    TiXmlDocument *doc = new TiXmlDocument();
    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("LobbyDefFile");
    Utils::WriteComment(root, "Note: debug_name is only for debug purposes. info_id is only for info, ignored by the compiler, and the decompiler will always set it to index+1");

    for (size_t i = 0; i < defs.size(); i++)
        defs[i].Decompile(root, i);

    doc->LinkEndChild(root);
    return doc;
}

bool Xv2LobbyDefFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "LobbyDefFile");

    if (!root)
    {
        DPRINTF("Cannot find\"LobbyDefFile\" in xml.\n");
        return false;
    }    

    size_t n = Utils::GetElemCount(root, "LobbyDef");

    if (n > MAX_LOBBY_MODDED)
    {
        DPRINTF("%s: Error, number of lobbies is bigger than maximum.\n", FUNCNAME);
        return false;
    }

    defs.reserve(n);

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "LobbyDef")
        {
            Xv2LobbyDef def;

            if (!def.Compile(elem))
                return false;

            defs.push_back(def);
        }
    }

    return true;
}

bool Xv2LobbyDefFile::GetFromGame(char **maps, char **spms, char **spm_dirs, int *integers)
{
    const std::vector<std::string> names =
    {
          "Main", "Satan House", "Capsule Corporation", "Elder House", "Freezer ship", "Boo House",
          "Elder House (inside)", "Freezer Ship (inside)", "Boo House (inside)", "Time Nest",
          "Kamehouse", "Orange Star", "PLBY_01", "PLBY_02", "PLBY_03",
    };

    Reset();

    defs.resize(MAX_LOBBY);

    for (size_t i = 0; i < MAX_LOBBY; i++)
    {
        defs[i].debug_name = names[i];
        defs[i].map = maps[i];
        defs[i].spm = spms[i];
        defs[i].spm_dir = spm_dirs[i];
        defs[i].integer = integers[i];
        defs[i].eflag = (i <= 5); // function 0x3F4EC0 in 1.24.1
    }

    return true;
}

