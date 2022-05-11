#ifndef TTCFILE_H
#define TTCFILE_H

#include <unordered_map>
#include "BaseFile.h"

// "#TTC"
#define TTC_SIGNATURE  0x43545423

enum TtcEventListType
{
    TTC_VERSUS_LIST = 0, /* Player faces this char */
    TTC_MINOR_DAMAGE_LIST = 1, /* This char has been damaged (minor damage) */
    TTC_MAJOR_DAMAGE_LIST = 2, /* This char has been damaged (major damage) */
    TTC_PLAYER_KO_ENEMY_LIST = 6, /* This char has ko'd the player */
    TTC_STRONG_ATTACK_DAMAGED_LIST = 7, /* This char has taken damage */
    TTC_POWER_UP_LIST = 8, /* This char is about to power up */
    TTC_START_TALK_LIST = 0xa, /* Start talk (ally) */
    TTC_PLAYER_DAMAGED_LIST = 0xb, /* The player is an ally of this character, and has been damaged by an enemy*/
    TTC_LITTLE_TIME_LIST = 0xc, /* The player is an ally and there is little time */
    TTC_PLAYER_ALLY_KILLED_ENEMY_LIST = 0xd, /* The player is an ally and has killed an anemy */
    TTC_CHALLENGE_LIST = 0xe, /* The player challenges this char. ? */
    TTC_KO_LIST = 0xf, /* This char has been ko'd */
    TTC_ENTERING_LIST = 0x10, /* This char (an enemy?) enters the quest */
    TTC_MASTER_VERSUS_LIST = 0x12, /* This char is the master of the player, and is going to face him */
    TTC_PLAYER_KO_ALLY_LIST = 0x13, /* This char is an ally of the player, and the player has been ko'd by an enemy */
    TTC_FIGHT_SERIOUSLY_LIST = 0x15, /* This char is going to fight seriously from now on */

    TTC_BIGGER_EVENT_P1 = 0x16
};

enum TtcEventCondition
{
    TTC_DEFAULT_CONDITION = 0,
    TTC_TO_HUMAN_CAC_CONDITION = 1,
    TTC_TO_SAIYAN_CAC_CONDITION = 2,
    TTC_TO_NAMEKIAN_CAC_CONDITION = 3,
    TTC_TO_FREEZER_CAC_CONDITION = 4,
    TTC_TO_MAJIN_CAC_CONDITION = 5,
    TTC_TEACHER_CONDITION = 6,
    TTC_EVENT_CONDITION_MAX = 7
};

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED TTCHeader
{
    uint32_t signature;
    uint16_t endianess_check;
    uint16_t unk_06;
    uint32_t num_entries;
    uint32_t strings_size;
};
CHECK_STRUCT_SIZE(TTCHeader, 0x10);

struct PACKED TTCEntry
{
    uint32_t num_lists;
    uint32_t data_start;
    uint32_t start_index;
    uint32_t cms_id;
};
CHECK_STRUCT_SIZE(TTCEntry, 0x10);

struct PACKED TTCEventList
{
    uint32_t num_events;
    uint32_t data_start;
    uint32_t start_index;
    uint32_t type;
};
CHECK_STRUCT_SIZE(TTCEventList, 0x10);

struct PACKED TTCEvent
{
    uint32_t cms_id; // Always matches the TTCEntry cms_id
    uint32_t costume;
    uint32_t transformation;
    uint32_t list_type; // Always matches the TTCEventsList type
    uint32_t condition;
    uint32_t name;
    // The following are always 0xFFFFFFFF
    uint32_t unk_18;
    uint32_t unk_1C;
    uint32_t unk_20;
    uint32_t unk_24;
};
CHECK_STRUCT_SIZE(TTCEvent, 0x28);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct TtcEvent
{
    uint32_t costume;
    uint32_t transformation;
    uint32_t condition;
    std::string name;

    TtcEvent()
    {
        costume = 0xFFFFFFFF;
        transformation = 0;
        condition = 0;
    }

    TiXmlElement *Decompile(TiXmlNode *root, uint32_t cms_id=0xFFFFFFFF, bool subtitles_comment=false) const;
    bool Compile(const TiXmlElement *root);
};

struct TtcEventList
{
    uint32_t type;
    std::vector<TtcEvent> events;

    TtcEventList()
    {
        type = 0;
    }

    static bool IsKnownType(uint32_t type);
    bool EventExists(const std::string &name) const;

    TiXmlElement *Decompile(TiXmlNode *root, uint32_t cms_id=0xFFFFFFFF, bool subtitles_comment=false) const;
    bool Compile(const TiXmlElement *root);
};

struct TtcEntry
{
    uint32_t cms_id;
    std::vector<TtcEventList> lists;

    bool EventExists(const std::string &name) const;

    TiXmlElement *Decompile(TiXmlNode *root, bool subtitles_comment=false) const;
    bool Compile(const TiXmlElement *root);
};

class TtcFile : public BaseFile
{
private:

    std::vector<TtcEntry> entries;    
    bool subtitles_comments; // for xml

    void AddString(std::unordered_map<std::string, size_t> &map, std::vector<std::string> &strings, const std::string &str, size_t &size) const;
    size_t BuildStrings(std::unordered_map<std::string, size_t> &map, std::vector<std::string> &strings) const;

protected:

    void Reset();

public:

    TtcFile();
    virtual ~TtcFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    size_t GetNumEventsLists() const;
    size_t GetNumEvents() const;

    TtcEntry *FindEntry(uint32_t cms_id);
    void AddEntry(const TtcEntry &entry);
    size_t RemoveChar(uint32_t cms_id);

    inline void SetSubtitleComments(bool value) { subtitles_comments = value; }

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    inline const std::vector<TtcEntry> &GetEntries() const { return entries; }
    inline std::vector<TtcEntry> &GetEntries() { return entries; }

    inline const TtcEntry &operator[](size_t n) const { return entries[n]; }
    inline const TtcEntry &operator[](size_t n) { return entries[n]; }

    inline std::vector<TtcEntry>::const_iterator begin() const { return entries.begin(); }
    inline std::vector<TtcEntry>::const_iterator end() const { return entries.end(); }

    inline std::vector<TtcEntry>::iterator begin() { return entries.begin(); }
    inline std::vector<TtcEntry>::iterator end() { return entries.end(); }
};

#endif // TTCFILE_H
