#ifndef X2MCOSTUMEFILE_H
#define X2MCOSTUMEFILE_H

#include "BcsFile.h"

enum
{
    COSTUME_TOP,
    COSTUME_BOTTOM,
    COSTUME_GLOVES,
    COSTUME_SHOES,
    COSTUME_ACCESSORY,
    COSTUME_MAX
};

struct X2mCostumeEntry
{
    uint8_t guid[16]; // Costume x2m
    std::vector<uint16_t> partsets;
    std::vector<uint8_t> races;
    std::vector<uint16_t> idb_entries;
    std::vector<uint8_t> costume_types;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);

    X2mCostumeEntry()
    {
        memset(guid, 0, sizeof(guid));
    }
};

struct X2mBody
{
    uint8_t guid[16]; // Skill x2m
    uint8_t race;
    int id;
    BcsBody body;

    X2mBody()
    {
        memset(guid, 0, sizeof(guid));
        race = 0;
        id = -1;
    }

    X2mBody(int id) : X2mBody() { this->id = id; }

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

class X2mCostumeFile : public BaseFile
{
private:

    std::vector<X2mCostumeEntry> costumes;
    std::vector<X2mBody> bodies;

protected:

    void Reset();

public:

    X2mCostumeFile();
    virtual ~X2mCostumeFile();

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    X2mCostumeEntry *FindCostume(const uint8_t *guid);
    inline X2mCostumeEntry *FindCostume(const std::string &guid)
    {
        uint8_t buf[16];

        if (!Utils::String2GUID(buf, guid))
            return nullptr;

        return FindCostume(buf);
    }

    X2mCostumeEntry *FindCostumeByPartSet(uint16_t partset);

    void AddCostume(const X2mCostumeEntry &entry);
    void RemoveCostume(const uint8_t *guid);

    size_t FindBody(const uint8_t *guid, std::vector<X2mBody *> &found_bodies);
    X2mBody *FindBodyById(int id);

    bool AddBody(const X2mBody &body);
    size_t RemoveBodiesFromMod(const uint8_t *guid);

};

#endif // X2MCOSTUMEFILE_H
