#ifndef TSDFILE_H
#define TSDFILE_H

#include "FixedMemoryStream.h"

struct TsdTrigger
{
    uint32_t id;
    uint32_t dlc;
    uint32_t u32_2;
    uint32_t type;

    uint32_t npc_tnl_id;
    uint32_t u32_4;

    uint32_t event_id;
    uint32_t u32_5;

    std::string condition;

    std::string temp_event_name; // Only for temmp usage by the xml parser

    TiXmlElement *Decompile(TiXmlNode *root, const std::string &event_name) const;
    bool Compile(const TiXmlElement *root);
};

struct TsdEvent
{
    uint32_t id;
    uint32_t dlc;

    std::string name;
    std::string path;
    std::string script;
    std::string function;
    std::vector<std::string> args;

    std::vector<uint32_t> npc_tnl_ids;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct TsdGlobal
{
    std::string name;
    uint32_t type;
    std::string initial_value;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct TsdConstant
{
    std::string name;
    uint32_t type;
    std::string value;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct TsdZone
{
    uint32_t id;
    uint32_t type;
    std::string name;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

class TsdFile : public BaseFile
{
private:

    std::vector<TsdTrigger> triggers;
    std::vector<TsdEvent> events;
    std::vector<TsdGlobal> globals;
    std::vector<TsdConstant> constants;
    std::vector<TsdZone> zones;

protected:

    void Reset();

    bool ReadString(Stream &stream, std::string &str);
    bool WriteString(Stream &stream, const std::string &str);

    size_t FindEvent(uint32_t id) const;
    size_t FindEvent(const std::string &name) const;

    uint32_t FindHighestTriggerId() const;
    uint32_t FindHighestEventId() const;

public:
    TsdFile();
    virtual ~TsdFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    size_t FindTriggersWithEvent(const std::string &event_name, std::vector<TsdTrigger *> &triggers);

    void AddTrigger(TsdTrigger &trigger);

    TsdEvent *FindEventByName(const std::string &name);
    void AddEvent(TsdEvent &event);
};

#endif // TSDFILE_H
