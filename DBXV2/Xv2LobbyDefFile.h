#ifndef XV2LOBBYDEFFILE_H
#define XV2LOBBYDEFFILE_H

#include "BaseFile.h"
#include "Utils.h"

#define MAX_LOBBY 15
#define MAX_LOBBY_MODDED 127

struct Xv2LobbyDef
{
    std::string debug_name;
    std::string map;
    std::string spm;
    std::string spm_dir;
    int32_t integer;
    bool eflag;
    bool deleted;

    Xv2LobbyDef()
    {
        integer = 0;
        eflag = true;
        deleted = false;
    }

    inline std::string GetDebugName() const
    {
        if (debug_name.length() > 0) return debug_name;
        return map;
    }

    TiXmlElement *Decompile(TiXmlNode *root, size_t idx) const;
    bool Compile(const TiXmlElement *root);
};

class Xv2LobbyDefFile : public BaseFile
{
private:
    std::vector<Xv2LobbyDef> defs;

protected:
    void Reset();

public:
    Xv2LobbyDefFile();
    virtual ~Xv2LobbyDefFile() override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    bool GetFromGame(char **maps, char **spms, char **spm_dirs, int *integers);

    inline size_t GetNumDefs() const { return defs.size(); }

    inline const std::vector<Xv2LobbyDef> &GetLobbyDefs() const { return defs; }
    inline std::vector<Xv2LobbyDef> &GetLobbyDefs() { return defs; }

    inline const Xv2LobbyDef &operator[](size_t n) const { return defs[n]; }
    inline Xv2LobbyDef &operator[](size_t n) { return defs[n]; }

    inline std::vector<Xv2LobbyDef>::const_iterator begin() const { return defs.begin(); }
    inline std::vector<Xv2LobbyDef>::const_iterator end() const { return defs.end(); }

    inline std::vector<Xv2LobbyDef>::iterator begin() { return defs.begin(); }
    inline std::vector<Xv2LobbyDef>::iterator end() { return defs.end(); }
};

#endif // XV2LOBBYDEFFILE_H
