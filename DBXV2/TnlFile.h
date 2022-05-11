#ifndef __TNLFILE_H__
#define __TNLFILE_H__

#include "Stream.h"

struct TnlCharacter
{
    uint32_t id;
    uint8_t u8_1;
    uint8_t u8_2;
    uint8_t u8_3;
    std::string cms;
    uint16_t costume_id;
    uint16_t cid;
    std::string lobby_name;
    uint32_t u32_1;
    std::string position;
    std::string zone;
    uint32_t action;

    TiXmlElement *Decompile(TiXmlNode *root, const char *entry_name) const;
    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct TnlTeacher : public TnlCharacter
{
    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct TnlObject
{
    uint32_t id;
    std::string lobby_name;
    uint32_t u32_1;
    std::string position1;
    std::string position2;
    std::string unk_string;
    uint32_t u32_2;
    uint32_t u32_3;
    uint32_t u32_4;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct TnlAction
{
    uint32_t id;
    std::string path;
    std::string script;
    std::string function;
    std::vector<std::string> args;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

class TnlFile : public BaseFile
{
private:

    std::vector<TnlCharacter> chars;
    std::vector<TnlTeacher> teachers;
    std::vector<TnlObject> objects;
    std::vector<TnlAction> actions;

    bool ReadString(Stream &stream, std::string &str);
    bool WriteString(Stream &stream, const std::string &str);

protected:

    void Reset();

public:

    TnlFile();
    virtual ~TnlFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    TnlCharacter *FindCharByID(uint32_t id)
    {
        for (TnlCharacter &ch : chars)
            if (ch.id == id)
                return &ch;

        return nullptr;
    }

    TnlTeacher *FindTeacherByID(uint32_t id)
    {
        for (TnlTeacher &teacher : teachers)
            if (teacher.id == id)
                return &teacher;

        return nullptr;
    }

    TnlCharacter *FindCharOrTeacherByID(uint32_t id)
    {
        TnlCharacter *ch = FindCharByID(id);

        if (ch)
            return ch;

        return FindTeacherByID(id);
    }

    inline size_t GetNumObjects() const {return objects.size(); }
    inline const TnlObject &GetLobbyObject(size_t idx) const { return objects[idx]; }
    inline TnlObject &GetLobbyObject(size_t idx) { return objects[idx]; }

    TnlObject *FindObjectById(uint32_t id);
    bool AddObjectWithId(const TnlObject &object, bool overwrite);
};

#endif // __TNLFILE_H__
