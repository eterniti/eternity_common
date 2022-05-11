#include "PalFile.h"
#include "Xenoverse2.h"
#include "debug.h"

#define COPY_I(d, s, n) d.n = s->n
#define COPY_II(n) COPY_I(entry, file_info, n)
#define COPY_EI(n) COPY_I(equipment, file_equipment, n)
#define COPY_SI(n) COPY_I(stats, file_stats, n)

#define COPY_O(d, s, n) d->n = s.n
#define COPY_IO(n) COPY_O(file_info, entry, n)
#define COPY_EO(n) COPY_O(file_equipment, equipment, n)
#define COPY_SO(n) COPY_O(file_stats, stats, n)

#define DEC(n, v) Utils::WriteParamUnsigned(entry_root, n, v, true)
#define DECD(n, v) Utils::WriteParamUnsigned(entry_root, n, v, false)
#define COM(n, v) { if (!Utils::GetParamUnsigned(root, n, &v)) return false; }

TiXmlElement *PalEquipment::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Equipment");
    std::string comment;

    DEC("FACE_BASE", face_base);
    DEC("FACE_FOREHEAD", face_forehead);
    DEC("EYES", eyes);
    DEC("NOSE", nose);
    DEC("EARS", ears);
    DEC("HAIR", hair);

    comment = Xenoverse2::GetCacTopName(top);
    if (comment.length() != 0)
        Utils::WriteComment(entry_root, comment);

    DEC("TOP", top);

    comment = Xenoverse2::GetCacBottomName(bottom);
    if (comment.length() != 0)
        Utils::WriteComment(entry_root, comment);

    DEC("BOTTOM", bottom);

    comment = Xenoverse2::GetCacGlovesName(gloves);
    if (comment.length() != 0)
        Utils::WriteComment(entry_root, comment);

    DEC("GLOVES", gloves);

    comment = Xenoverse2::GetCacShoesName(shoes);
    if (comment.length() != 0)
        Utils::WriteComment(entry_root, comment);

    DEC("SHOES", shoes);

    DEC("BODY_SHAPE", body_shape);
    DEC("U_16", unk_16);
    DEC("SKIN_COLOR1", skin_color1);
    DEC("SKIN_COLOR2", skin_color2);
    DEC("SKIN_COLOR3", skin_color3);
    DEC("SKIN_COLOR4", skin_color4);
    DEC("HAIR_COLOR", hair_color);
    DEC("EYE_COLOR", eye_color);
    DEC("TOP_COLOR1", top_color1);
    DEC("TOP_COLOR2", top_color2);
    DEC("TOP_COLOR3", top_color3);
    DEC("TOP_COLOR4", top_color4);
    DEC("BOTTOM_COLOR1", bottom_color1);
    DEC("BOTTOM_COLOR2", bottom_color2);
    DEC("BOTTOM_COLOR3", bottom_color3);
    DEC("BOTTOM_COLOR4", bottom_color4);
    DEC("GLOVES_COLOR1", gloves_color1);
    DEC("GLOVES_COLOR2", gloves_color2);
    DEC("GLOVES_COLOR3", gloves_color3);
    DEC("GLOVES_COLOR4", gloves_color4);
    DEC("SHOES_COLOR1", shoes_color1);
    DEC("SHOES_COLOR2", shoes_color2);
    DEC("SHOES_COLOR3", shoes_color3);
    DEC("SHOES_COLOR4", shoes_color4);
    DEC("MAKEUP_COLOR1", makeup_color1);
    DEC("MAKEUP_COLOR2", makeup_color2);
    DEC("MAKEUP_COLOR3", makeup_color3);

    comment = Xenoverse2::GetCacAccesoryName(accesory);
    if (comment.length() != 0)
        Utils::WriteComment(entry_root, comment);

    DEC("ACCESORY", accesory);

    comment = Xenoverse2::GetTalismanNameEx(talisman);
    if (comment.length() != 0)
        Utils::WriteComment(entry_root, comment);

    DEC("TALISMAN", talisman);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool PalEquipment::Compile(const TiXmlElement *root)
{
    COM("FACE_BASE", face_base);
    COM("FACE_FOREHEAD", face_forehead);
    COM("EYES", eyes);
    COM("NOSE", nose);
    COM("EARS", ears);
    COM("HAIR", hair);
    COM("TOP", top);
    COM("BOTTOM", bottom);
    COM("GLOVES", gloves);
    COM("SHOES", shoes);
    COM("BODY_SHAPE", body_shape);
    COM("U_16", unk_16);
    COM("SKIN_COLOR1", skin_color1);
    COM("SKIN_COLOR2", skin_color2);
    COM("SKIN_COLOR3", skin_color3);
    COM("SKIN_COLOR4", skin_color4);
    COM("HAIR_COLOR", hair_color);
    COM("EYE_COLOR", eye_color);
    COM("TOP_COLOR1", top_color1);
    COM("TOP_COLOR2", top_color2);
    COM("TOP_COLOR3", top_color3);
    COM("TOP_COLOR4", top_color4);
    COM("BOTTOM_COLOR1", bottom_color1);
    COM("BOTTOM_COLOR2", bottom_color2);
    COM("BOTTOM_COLOR3", bottom_color3);
    COM("BOTTOM_COLOR4", bottom_color4);
    COM("GLOVES_COLOR1", gloves_color1);
    COM("GLOVES_COLOR2", gloves_color2);
    COM("GLOVES_COLOR3", gloves_color3);
    COM("GLOVES_COLOR4", gloves_color4);
    COM("SHOES_COLOR1", shoes_color1);
    COM("SHOES_COLOR2", shoes_color2);
    COM("SHOES_COLOR3", shoes_color3);
    COM("SHOES_COLOR4", shoes_color4);
    COM("MAKEUP_COLOR1", makeup_color1);
    COM("MAKEUP_COLOR2", makeup_color2);
    COM("MAKEUP_COLOR3", makeup_color3);
    COM("ACCESORY", accesory);
    COM("TALISMAN", talisman);

    return true;
}

TiXmlElement *PalStats::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Stats");
    std::string comment;

    DECD("LEVEL", level);
    DECD("HEA", hea);
    DECD("KI", ki);
    DECD("STM", stm);
    DECD("ATK", atk);
    DECD("STR", str);
    DECD("BLA", bla);
    DECD("U_0E", unk_0E);

    for (int i = 0; i < 4; i++)
    {
        char name[32];

        if (Xenoverse2::GetSuperSkillName(super_skills[i], comment))
            Utils::WriteComment(entry_root, comment);

        snprintf(name, sizeof(name), "SUPER_SKILL%d", i+1);

        DEC(name, super_skills[i]);
    }

    for (int i = 0; i < 2; i++)
    {
        char name[32];

        if (Xenoverse2::GetUltimateSkillName(ult_skills[i], comment))
            Utils::WriteComment(entry_root, comment);

        snprintf(name, sizeof(name), "ULTIMATE_SKILL%d", i+1);

        DEC(name, ult_skills[i]);
    }

    if (Xenoverse2::GetEvasiveSkillName(evasive_skill, comment))
        Utils::WriteComment(entry_root, comment);

    DEC("EVASIVE_SKILL", evasive_skill);
    DEC("BLAST_SKILL", blast_skill);

    if (Xenoverse2::GetAwakenSkillName(awaken_skill, comment))
        Utils::WriteComment(entry_root, comment);

    DEC("AWAKEN_SKILL", awaken_skill);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool PalStats::Compile(const TiXmlElement *root)
{
    COM("LEVEL", level);
    COM("HEA", hea);
    COM("KI", ki);
    COM("STM", stm);
    COM("ATK", atk);
    COM("STR", str);
    COM("BLA", bla);
    COM("U_0E", unk_0E);

    for (int i = 0; i < 4; i++)
    {
        char name[32];
        snprintf(name, sizeof(name), "SUPER_SKILL%d", i+1);

        COM(name, super_skills[i]);
    }

    for (int i = 0; i < 2; i++)
    {
        char name[32];
        snprintf(name, sizeof(name), "ULTIMATE_SKILL%d", i+1);

        COM(name, ult_skills[i]);
    }

    COM("EVASIVE_SKILL", evasive_skill);
    COM("BLAST_SKILL", blast_skill);
    COM("AWAKEN_SKILL", awaken_skill);

    return true;
}

TiXmlElement *PalEntry::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("PalEntry");
    std::string comment;

    entry_root->SetAttribute("id", Utils::UnsignedToString(id, true));
    entry_root->SetAttribute("name_id", Utils::UnsignedToString(name_id, true));

    if (Xenoverse2::GetLobbyName(this->name_id, comment))
        Utils::WriteComment(entry_root, comment);

    CmsEntry *cms;

    if (game_cms && (cms = game_cms->FindEntryByID(cms_entry)))
    {
        Utils::WriteComment(entry_root, cms->name);
    }

    DEC("CMS_ENTRY", cms_entry);
    DEC("VOICE", voice);
    DEC("TEAM_MATE", team_mate);

    equipment.Decompile(entry_root);
    stats.Decompile(entry_root);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool PalEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "id", &id))
    {
        DPRINTF("%s: Attribute id is not optional.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "name_id", &name_id))
    {
        DPRINTF("%s: Attribute id is not optional.\n", FUNCNAME);
        return false;
    }

    COM("CMS_ENTRY", cms_entry);
    COM("VOICE", voice);
    COM("TEAM_MATE", team_mate);

    const TiXmlElement *equip_root;
    const TiXmlElement *stats_root;

    if (Utils::GetElemCount(root, "Equipment", &equip_root) == 0)
    {
        DPRINTF("%s: Equipment not found.\n", FUNCNAME);
        return false;
    }

    if (Utils::GetElemCount(root, "Stats", &stats_root) == 0)
    {
        DPRINTF("%s: Stats not found.\n", FUNCNAME);
        return false;
    }

    return (equipment.Compile(equip_root) && stats.Compile(stats_root));
}

PalFile::PalFile()
{
    this->big_endian = false;
}

PalFile::~PalFile()
{
}

void PalFile::Reset()
{
    entries.clear();
}

bool PalFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(PALHeader))
        return false;

    const PALHeader *hdr = (const PALHeader *)buf;

    if (hdr->signature != PAL_SIGNATURE)
        return false;

    entries.resize(hdr->num_entries);
    const PALInfo *file_info = (const PALInfo *)(buf + hdr->data_start);

    for (size_t i = 0; i < entries.size(); i++, file_info++)
    {
        PalEntry &entry = entries[i];

        COPY_II(id);
        COPY_II(name_id);
        COPY_II(cms_entry);
        COPY_II(voice);
        COPY_II(team_mate);
    }

    const PALEquipment *file_equipment = (const PALEquipment *)file_info;

    for (size_t i = 0; i < entries.size(); i++, file_equipment++)
    {
        PalEquipment &equipment = entries[i].equipment;

        COPY_EI(face_base);
        COPY_EI(face_forehead);
        COPY_EI(eyes);
        COPY_EI(nose);
        COPY_EI(ears);
        COPY_EI(hair);
        COPY_EI(top);
        COPY_EI(bottom);
        COPY_EI(gloves);
        COPY_EI(shoes);
        COPY_EI(body_shape);
        COPY_EI(unk_16);
        COPY_EI(skin_color1);
        COPY_EI(skin_color2);
        COPY_EI(skin_color3);
        COPY_EI(skin_color4);
        COPY_EI(hair_color);
        COPY_EI(eye_color);
        COPY_EI(top_color1);
        COPY_EI(top_color2);
        COPY_EI(top_color3);
        COPY_EI(top_color4);
        COPY_EI(bottom_color1);
        COPY_EI(bottom_color2);
        COPY_EI(bottom_color3);
        COPY_EI(bottom_color4);
        COPY_EI(gloves_color1);
        COPY_EI(gloves_color2);
        COPY_EI(gloves_color3);
        COPY_EI(gloves_color4);
        COPY_EI(shoes_color1);
        COPY_EI(shoes_color2);
        COPY_EI(shoes_color3);
        COPY_EI(shoes_color4);
        COPY_EI(makeup_color1);
        COPY_EI(makeup_color2);
        COPY_EI(makeup_color3);
        COPY_EI(accesory);
        COPY_EI(talisman);
    }

    const PALStats *file_stats = (const PALStats *)file_equipment;
    const uint8_t *file_limit = buf + size;

    for (size_t i = 0; i < entries.size(); i++, file_stats++)
    {
        PalStats &stats = entries[i].stats;

        if ((const uint8_t *)file_stats < file_limit)
        {
            COPY_SI(level);
            COPY_SI(hea);
            COPY_SI(ki);
            COPY_SI(stm);
            COPY_SI(atk);
            COPY_SI(str);
            COPY_SI(bla);
            COPY_SI(unk_0E);

            memcpy(stats.super_skills, file_stats->super_skills, sizeof(stats.super_skills));
            memcpy(stats.ult_skills, file_stats->ult_skills, sizeof(stats.ult_skills));

            COPY_SI(evasive_skill);
            COPY_SI(blast_skill);
            COPY_SI(awaken_skill);
        }
        else
        {
            stats.nostats = true;
        }
    }

    return true;
}

size_t PalFile::GetNumValidStats() const
{
    size_t count = 0;

    for (const PalEntry &entry : entries)
    {
        if (!entry.stats.nostats)
            count++;
        else
            break;
    }

    return count;
}

uint8_t *PalFile::Save(size_t *psize)
{
    size_t size = sizeof(PALHeader);

    /*size_t num_stats = GetNumValidStats();
    size += entries.size() * (sizeof(PALInfo)+sizeof(PALEquipment)) + num_stats * sizeof(PALStats); */
    size_t num_stats = entries.size();
    size += entries.size() * (sizeof(PALInfo)+sizeof(PALEquipment)+sizeof(PALStats));

    uint8_t *buf = new uint8_t[size];
    memset(buf, 0, size);

    PALHeader *hdr = (PALHeader *)buf;

    hdr->signature = PAL_SIGNATURE;
    hdr->endianess_check = val16(0xFFFE);
    hdr->unk_06 = 1;
    hdr->num_entries = (uint32_t) entries.size();
    hdr->data_start = sizeof(PALHeader);

    PALInfo *file_info = (PALInfo *)(hdr+1);

    for (size_t i = 0; i < entries.size(); i++, file_info++)
    {
        const PalEntry &entry = entries[i];

        COPY_IO(id);
        COPY_IO(name_id);
        COPY_IO(cms_entry);
        COPY_IO(voice);
        COPY_IO(team_mate);
    }

    PALEquipment *file_equipment = (PALEquipment *)file_info;

    for (size_t i = 0; i < entries.size(); i++, file_equipment++)
    {
        const PalEquipment &equipment = entries[i].equipment;

        COPY_EO(face_base);
        COPY_EO(face_forehead);
        COPY_EO(eyes);
        COPY_EO(nose);
        COPY_EO(ears);
        COPY_EO(hair);
        COPY_EO(top);
        COPY_EO(bottom);
        COPY_EO(gloves);
        COPY_EO(shoes);
        COPY_EO(body_shape);
        COPY_EO(unk_16);
        COPY_EO(skin_color1);
        COPY_EO(skin_color2);
        COPY_EO(skin_color3);
        COPY_EO(skin_color4);
        COPY_EO(hair_color);
        COPY_EO(eye_color);
        COPY_EO(top_color1);
        COPY_EO(top_color2);
        COPY_EO(top_color3);
        COPY_EO(top_color4);
        COPY_EO(bottom_color1);
        COPY_EO(bottom_color2);
        COPY_EO(bottom_color3);
        COPY_EO(bottom_color4);
        COPY_EO(gloves_color1);
        COPY_EO(gloves_color2);
        COPY_EO(gloves_color3);
        COPY_EO(gloves_color4);
        COPY_EO(shoes_color1);
        COPY_EO(shoes_color2);
        COPY_EO(shoes_color3);
        COPY_EO(shoes_color4);
        COPY_EO(makeup_color1);
        COPY_EO(makeup_color2);
        COPY_EO(makeup_color3);
        COPY_EO(accesory);
        COPY_EO(talisman);
    }

    PALStats *file_stats = (PALStats *)file_equipment;

    for (size_t i = 0; i < num_stats; i++, file_stats++)
    {
        const PalStats &stats = entries[i].stats;

        COPY_SO(level);
        COPY_SO(hea);
        COPY_SO(ki);
        COPY_SO(stm);
        COPY_SO(atk);
        COPY_SO(str);
        COPY_SO(bla);
        COPY_SO(unk_0E);

        memcpy(file_stats->super_skills, stats.super_skills, sizeof(stats.super_skills));
        memcpy(file_stats->ult_skills, stats.ult_skills, sizeof(stats.ult_skills));

        COPY_SO(evasive_skill);
        COPY_SO(blast_skill);
        COPY_SO(awaken_skill);
    }

    *psize = size;
    return buf;
}

TiXmlDocument *PalFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("PAL");

    /*size_t num_stats = GetNumValidStats();
    if (num_stats != entries.size())
        root->SetAttribute("num_valid_stats_entries", num_stats);*/

    for (const PalEntry &entry : entries)
    {
        entry.Decompile(root);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool PalFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "PAL");

    if (!root)
    {
        DPRINTF("Cannot find\"PAL\" in xml.\n");
        return false;
    }

    /*int num_valid_stats;
    if (!root->Attribute("num_valid_stats_entries", &num_valid_stats))
        num_valid_stats = 0;*/

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "PalEntry")
        {
            PalEntry entry;

            if (!entry.Compile(elem))
                return false;

            /*if (num_valid_stats > 0 && entries.size() >= (size_t)num_valid_stats)
               entry.stats.nostats = true;*/

            entries.push_back(entry);
        }
    }

    return true;
}

PalEntry *PalFile::FindEntryByID(uint32_t id)
{
    for (PalEntry &entry : entries)
    {
        if (entry.id == id)
            return &entry;
    }

    return nullptr;
}
