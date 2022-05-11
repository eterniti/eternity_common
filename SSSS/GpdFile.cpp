#include <algorithm>

#include "GpdFile.h"
#include "GwdFile.h"
#include "SsssData.h"

#include "debug.h"

static const std::vector<std::string> gwd_tournaments_names =
{
    "BRONZE_THE_BRONZE_RIVALS",
    "BRONZE_NEW_BRONZE_AND_SILVER_POWER",
    "BRONZE_UNDEFEATED_SAINT_I",
    "BRONZE_BATTLE_TO_THE_DEATH_I",
    "BRONZE_12_GOLD_PALACES",
    "BRONZE_12_GOLD_PALACES_CONTINUED",
    "BRONZE_SEVEN_SENSES_AWAKENING",
    "BRONZE_BEST_OF_THE_BEST_I",
    "BRONZE_BEST_OF_THE_BEST_II",

    "SILVER_A_NEW_ERA_OF_BRONZE_MIGHT",
    "SILVER_SILVER_AND_GOLD",
    "SILVER_UNDEFEATED_SAINT_II",
    "SILVER_BATTLE_TO_THE_DEATH_II",
    "SILVER_POWERHOUSES_OF_THE_DEEP",
    "SILVER_UNDEFEATED_SAINT_III",
    "SILVER_BATTLE_TO_THE_DEATH_III",
    "SILVER_BEST_OF_THE_BEST_III",
    "SILVER_MIND_EYE_MASTER_I",

    "GOLD_INVESTED_DREAMS",
    "GOLD_END_OF_THE_GOLD_SAINT",
    "GOLD_UNDEFEATED_SAINT_IV",
    "GOLD_BATTLE_TO_THE_DEATH_IV",
    "GOLD_BETRAYAL_AND_ASSAULT",
    "GOLD_UNDEFEATED_SAINT_V",
    "GOLD_BATTLE_TO_THE_DEATH_V",
    "GOLD_BEST_OF_THE_BEST_IV",
    "GOLD_MIND_EYE_MASTER_II",
    "GOLD_RULES_OF_COMBAT_I",

    "LEGEND_LEGENDARY_BEING",
    "LEGEND_LEGION_OF_SAINTS",
    "LEGEND_LEGION_OF_MARINAS",
    "LEGEND_LEGION_OF_SPECTERS",
    "LEGEND_DIEHARD_SAINT_I",
    "LEGEND_UNDEFEATED_SAINT_VI",
    "LEGEND_BATTLE_TO_THE_DEATH_VI",
    "LEGEND_MOST_POPULAR_FINAL_BATTLE",
    "LEGEND_DECIDE_BY_FACE",
    "LEGEND_PEAK_SHOWDOWN",

    "GOD_BRAWLERS",
    "GOD_LEGION_OF_GOD_WARRIORS",
    "GOD_UNDEFEATED_SAINT_VII",
    "GOD_RESURRECTED_GOLDEN_COSMO",
    "GOD_DIEHARD_SAINT_II",
    "GOD_FIGHT_WITH_WEAPONS",
    "GOD_MIND_EYE_MASTERS_III",
    "GOD_WINGED_FIGHTERS",
    "GOD_BEST_OF_THE_BEST_V",
    "GOD_FINAL_BATTLE"
};

bool GWLComparer::operator()(const GWLEntry &info1, const GWLEntry &info2)
{
    if (gpd->val32(info1.cms_entry) == gpd->val32(info2.cms_entry))
        return (gpd->val32(info1.cms_model_spec_idx) < gpd->val32(info2.cms_model_spec_idx));

    return (gpd->val32(info1.cms_entry) < gpd->val32(info2.cms_entry));
}

#define D_U32(n, f) { Utils::WriteParamUnsigned(entry_root, n, f, true); }
#define D_FLOAT(n, f) { Utils::WriteParamFloat(entry_root, n, f); }

void GpdEntry::Decompile(TiXmlNode *root, const CharacterDef *def, const std::vector<uint32_t> *tours_ids) const
{
    TiXmlElement *entry_root = new TiXmlElement("GpdEntry");

    entry_root->SetAttribute("id", Utils::UnsignedToString(id, true));

    if (def)
    {
        SsssCharInfo *ssss_info = SsssData::FindInfo(def->cms_entry, def->cms_model_spec_idx);

        if (ssss_info)
        {
            Utils::WriteComment(entry_root, std::string(" ") + ssss_info->model_name + std::string(" / ") + ssss_info->char_name);
        }

        Utils::WriteParamUnsigned(entry_root, "CMS_ENTRY", def->cms_entry, true);
        Utils::WriteParamUnsigned(entry_root, "CMS_MODEL_SPEC_IDX", def->cms_model_spec_idx);
    }

    D_U32("U_00", unk_00);
    D_FLOAT("F_08", unk_08);
    D_FLOAT("F_0C", unk_0C);
    D_FLOAT("F_10", unk_10);
    D_FLOAT("F_14", unk_14);
    D_U32("U_18", unk_18);
    D_U32("U_1C", unk_1C);
    D_U32("U_20", unk_20);
    D_U32("U_24", unk_24);
    D_U32("U_28", unk_28);
    D_U32("U_2C", unk_2C);
    D_U32("U_30", unk_30);
    D_U32("U_34", unk_34);
    D_U32("U_38", unk_38);
    D_U32("U_3C", unk_3C);

    if (tours_ids)
    {
        std::vector<std::string> tours_names;

        if (!GpdFile::GetTournamentNames(*tours_ids, tours_names))
        {
            assert(0);
        }

        Utils::WriteParamMultipleStrings(entry_root, "GWD_TOURNAMENTS", tours_names);
    }

    root->LinkEndChild(entry_root);
}

#define C_U32(n, f) { if (!Utils::GetParamUnsigned(root, n, &f)) \
                            return false; }
#define C_FLOAT(n, f) { if (!Utils::GetParamFloat(root, n, &f)) \
                            return false; }

bool GpdEntry::Compile(TiXmlElement *root, CharacterDef **pdef, std::vector<uint32_t> **ptours_ids)
{
    uint32_t cms_entry, cms_model_spec_idx;

    *pdef = nullptr;
    *ptours_ids = nullptr;

    if (!Utils::ReadAttrUnsigned(root, "id", &id))
    {
        DPRINTF("%s: Cannot read atttribute \"id\"\n", __PRETTY_FUNCTION__);
        return false;
    }

    if (Utils::ReadParamUnsigned(root, "CMS_ENTRY", &cms_entry))
    {
        if (!Utils::GetParamUnsigned(root, "CMS_MODEL_SPEC_IDX", &cms_model_spec_idx))
            return false;

        *pdef = new CharacterDef;
        (*pdef)->cms_entry = cms_entry;
        (*pdef)->cms_model_spec_idx = cms_model_spec_idx;
    }

    C_U32("U_00", unk_00);
    C_FLOAT("F_08", unk_08);
    C_FLOAT("F_0C", unk_0C);
    C_FLOAT("F_10", unk_10);
    C_FLOAT("F_14", unk_14);
    C_U32("U_18", unk_18);
    C_U32("U_1C", unk_1C);
    C_U32("U_20", unk_20);
    C_U32("U_24", unk_24);
    C_U32("U_28", unk_28);
    C_U32("U_2C", unk_2C);
    C_U32("U_30", unk_30);
    C_U32("U_34", unk_34);
    C_U32("U_38", unk_38);
    C_U32("U_3C", unk_3C);

    std::vector<std::string> tours_names;

    if (Utils::ReadParamMultipleStrings(root, "GWD_TOURNAMENTS", tours_names))
    {
        *ptours_ids = new std::vector<uint32_t>;
        if (!GpdFile::GetTournamentsIds(tours_names, **ptours_ids))
        {
            delete *ptours_ids;
            return false;
        }
    }

    return true;
}

GpdFile::GpdFile()
{
    assert(gwd_tournaments_names.size() == 0x30);

    big_endian = false;
    Reset();
}

GpdFile::~GpdFile()
{
    Reset();
}

void GpdFile::Reset()
{
    entries.clear();
    char_map.clear();
    gwd_tournaments.clear();
}

bool GpdFile::GetTournamentNames(const std::vector<uint32_t> &ids, std::vector<std::string> &names)
{
    names.resize(ids.size());

    for (size_t i = 0; i < ids.size(); i++)
    {
       if (ids[i] >= gwd_tournaments_names.size())
       {
            DPRINTF("%s: Tournament id 0x%x is out of names range.\n", FUNCNAME, ids[i]);
            return false;
       }

       names[i] = gwd_tournaments_names[ids[i]];
    }

    return true;
}

bool GpdFile::GetTournamentsIds(const std::vector<std::string> &names, std::vector<uint32_t> &ids)
{
    ids.resize(names.size());

    for (size_t i = 0; i < names.size(); i++)
    {
        auto it = std::find(gwd_tournaments_names.begin(), gwd_tournaments_names.end(), names[i]);
        if (it == gwd_tournaments_names.end())
        {
            DPRINTF("%s: Invalid tournament name \"%s\"\n", FUNCNAME, names[i].c_str());
            return false;
        }

        ids[i] = it - gwd_tournaments_names.begin();
    }

    return true;
}

uint32_t GpdFile::FindEntry(uint32_t cms_entry, uint32_t cms_model_spec_idx)
{
    for (size_t i = 0; i < char_map.size(); i++)
    {
        if (char_map[i].cms_entry == cms_entry && char_map[i].cms_model_spec_idx == cms_model_spec_idx)
            return i;
    }

    return (uint32_t)-1;
}

GpdEntry *GpdFile::GetEntry(uint32_t idx, uint32_t *cms_entry, uint32_t *cms_model_spec_idx, std::vector<uint32_t> *tours_ids)
{
    if (idx >= entries.size())
        return nullptr;

    if (cms_entry)
    {
        if (char_map.size() == 0)
            return nullptr;

        *cms_entry = char_map[idx].cms_entry;
        *cms_model_spec_idx = char_map[idx].cms_model_spec_idx;
    }

    if (tours_ids)
    {
        if (gwd_tournaments.size() == 0)
            return nullptr;

        *tours_ids = gwd_tournaments[idx];
    }

    return &entries[idx];
}

const GpdEntry *GpdFile::GetEntry(uint32_t idx, uint32_t *cms_entry, uint32_t *cms_model_spec_idx, std::vector<uint32_t> *tours_ids) const
{
    if (idx >= entries.size())
        return nullptr;

    if (cms_entry)
    {
        if (char_map.size() == 0)
            return nullptr;

        *cms_entry = char_map[idx].cms_entry;
        *cms_model_spec_idx = char_map[idx].cms_model_spec_idx;
    }

    if (tours_ids)
    {
        if (gwd_tournaments.size() == 0)
            return nullptr;

        *tours_ids = gwd_tournaments[idx];
    }

    return &entries[idx];
}

bool GpdFile::SetEntry(uint32_t idx, const GpdEntry &gpd_entry, uint32_t cms_entry, uint32_t cms_model_spec_idx, std::vector<uint32_t> *tours_ids)
{
    if (idx >= entries.size())
        return false;

    if (char_map.size() != 0 && cms_entry == 0xFFFFFFFF)
        return false;

    if (gwd_tournaments.size() != 0 && !tours_ids)
        return false;

    GpdEntry entry_to_set = gpd_entry;

    entry_to_set.id = idx;
    entry_to_set.unk_00 = idx+1;
    entries[idx] = entry_to_set;

    if (char_map.size() != 0)
    {
        char_map[idx].cms_entry = cms_entry;
        char_map[idx].cms_model_spec_idx = cms_model_spec_idx;
    }

    if (gwd_tournaments.size() != 0)
    {
        gwd_tournaments[idx] = *tours_ids;
    }

    return true;
}

bool GpdFile::AppendEntry(const GpdEntry &gpd_entry, uint32_t cms_entry, uint32_t cms_model_spec_idx, std::vector<uint32_t> *tours_ids)
{
    if (char_map.size() != 0 && cms_entry == 0xFFFFFFFF)
        return false;

    if (gwd_tournaments.size() != 0 && !tours_ids)
        return false;

    GpdEntry entry_to_add = gpd_entry;

    entry_to_add.id = entries.size();
    entry_to_add.unk_00 = entry_to_add.id+1;
    entries.push_back(entry_to_add);

    if (char_map.size() != 0)
    {
        CharacterDef def;

        def.cms_entry = cms_entry;
        def.cms_model_spec_idx = cms_model_spec_idx;

        char_map.push_back(def);
    }

    if (gwd_tournaments.size() != 0)
    {
        gwd_tournaments.push_back(*tours_ids);
    }

    return true;
}

bool GpdFile::DeleteEntry(uint32_t idx)
{
    if (idx >= entries.size())
        return false;

    entries.erase(entries.begin()+idx);

    if (char_map.size() != 0)
    {
        char_map.erase(char_map.begin()+idx);
    }

    if (gwd_tournaments.size() != 0)
    {
        gwd_tournaments.erase(gwd_tournaments.begin()+idx);
    }

    for (size_t i = 0; i < entries.size(); i++)
    {
        entries[i].id = i;
        entries[i].unk_00 = i+1;
    }

    return true;
}

#define TON_U32(field) { e.field = val32(f->field); }
#define TON_FLOAT(field) { e.field = val_float(f->field); }

bool GpdFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    GPDHeader *hdr = (GPDHeader *)buf;

    if (size < sizeof(GPDHeader) || hdr->signature != GPD_SIGNATURE)
        return false;

    big_endian = (buf[4] != 0xFE);

    uint8_t *ptr = GetOffsetPtr(hdr, hdr->data_start);
    entries.resize(val32(hdr->num_entries));

    for (size_t i = 0; i < entries.size(); i++)
    {
        GpdEntry &e = entries[i];
        GPDEntry *f = (GPDEntry *)ptr;

        TON_U32(unk_00);
        TON_U32(id);
        TON_FLOAT(unk_08);
        TON_FLOAT(unk_0C);
        TON_FLOAT(unk_10);
        TON_FLOAT(unk_14);
        TON_U32(unk_18);
        TON_U32(unk_1C);
        TON_U32(unk_20);
        TON_U32(unk_24);
        TON_U32(unk_28);
        TON_U32(unk_2C);
        TON_U32(unk_30);
        TON_U32(unk_34);
        TON_U32(unk_38);
        TON_U32(unk_3C);

        if (e.id != i)
        {
            DPRINTF("%s: Invalid id 0x%x for element 0x%x\n", __PRETTY_FUNCTION__, e.id, i);
            return false;
        }

        ptr += sizeof(GPDEntry);
    }

    return true;
}

#define TOF_U32(field) { f->field = val32(e.field); }
#define TOF_FLOAT(field) { f->field = val_float(e.field); }

uint8_t *GpdFile::Save(size_t *size)
{
    uint32_t offset;
    unsigned int file_size = sizeof(GPDHeader) + (entries.size() * sizeof(GPDEntry));

    uint8_t *buf = new uint8_t[file_size];
    
    GPDHeader *hdr = (GPDHeader *)buf;

    memset(hdr, 0, sizeof(GPDHeader));
    hdr->signature = GPD_SIGNATURE;
    hdr->endianess_check = val16(0xFFFE);
    hdr->num_entries = val32(entries.size());
    hdr->data_start = val32(sizeof(GPDHeader));

    offset = sizeof(GPDHeader);

    for (const GpdEntry &e : entries)
    {
        GPDEntry *f = (GPDEntry *)GetOffsetPtr(hdr, offset, true);

        TOF_U32(unk_00);
        TOF_U32(id);
        TOF_FLOAT(unk_08);
        TOF_FLOAT(unk_0C);
        TOF_FLOAT(unk_10);
        TOF_FLOAT(unk_14);
        TOF_U32(unk_18);
        TOF_U32(unk_1C);
        TOF_U32(unk_20);
        TOF_U32(unk_24);
        TOF_U32(unk_28);
        TOF_U32(unk_2C);
        TOF_U32(unk_30);
        TOF_U32(unk_34);
        TOF_U32(unk_38);
        TOF_U32(unk_3C);

        offset += sizeof(GPDEntry);
    }

    assert(offset == file_size);

    *size = file_size;
    return buf;
}

bool GpdFile::SetExtraData(const uint8_t *galaxian_wars_list, unsigned int size)
{
    if ((size % sizeof(GWLEntry)) != 0)
    {
        DPRINTF("%s: size of gwl is not multiple of structure size.\n", __PRETTY_FUNCTION__);
        return false;
    }

    if (entries.size() == 0)
        return true;

    size_t num_entries = size / sizeof(GWLEntry);
    const uint8_t *ptr = galaxian_wars_list;

    std::vector<bool> initialized;

    char_map.resize(entries.size());
    initialized.resize(entries.size());

    for (size_t i = 0; i < num_entries; i++)
    {
        GWLEntry *entry = (GWLEntry *)ptr;
        uint32_t idx = val32(entry->gpd_entry);

        if (idx < entries.size())
        {
            if (initialized[idx])
            {
                DPRINTF("%s: entry 0x%x had already been initialized.\n", __PRETTY_FUNCTION__, idx);
                return false;
            }

            char_map[idx].cms_entry = val32(entry->cms_entry);
            char_map[idx].cms_model_spec_idx = val32(entry->cms_model_spec_idx);
            initialized[idx] = true;
        }

        ptr += sizeof(GWLEntry);
    }

    for (bool b : initialized)
    {
        if (!b)
        {
            DPRINTF("%s: Not all entries were initialized.\n", __PRETTY_FUNCTION__);
            return false;
        }
    }

    return true;
}

uint8_t *GpdFile::CreateGalaxianWarsList(unsigned int *psize, CmsFile *check)
{
    if (char_map.size() == 0)
        return nullptr;

    std::vector<GWLEntry> gwl_entries;

    gwl_entries.resize(entries.size());

    for (size_t i = 0; i < entries.size(); i++)
    {
        gwl_entries[i].cms_entry = val32(char_map[i].cms_entry);
        gwl_entries[i].cms_model_spec_idx = val32(char_map[i].cms_model_spec_idx);

        if (check && !check->ModelExists(gwl_entries[i].cms_entry, gwl_entries[i].cms_model_spec_idx))
        {
            DPRINTF("%s: Model 0x%x %d doesn't exist in cms.\n", __PRETTY_FUNCTION__, gwl_entries[i].cms_entry, gwl_entries[i].cms_model_spec_idx);
            return nullptr;
        }

        gwl_entries[i].gpd_entry = val32(i);
        gwl_entries[i].unk_0C = 0;
    }

    // Add special entries for Hades OCE and Athena OCE if they don't exist
    if (std::find_if(gwl_entries.begin(), gwl_entries.end(), GWLFinder(val32(0x29), val32(1))) == gwl_entries.end())
    {
        GWLEntry hades_oce;

        hades_oce.cms_entry = val32(0x29);
        hades_oce.cms_model_spec_idx = val32(1);
        hades_oce.gpd_entry = val32(gwl_entries.size());
        hades_oce.unk_0C = 0;

        gwl_entries.push_back(hades_oce);
    }

    if (std::find_if(gwl_entries.begin(), gwl_entries.end(), GWLFinder(0, val32(1))) == gwl_entries.end())
    {
        GWLEntry athena_oce;

        athena_oce.cms_entry = 0;
        athena_oce.cms_model_spec_idx = val32(1);
        athena_oce.gpd_entry = val32(gwl_entries.size());
        athena_oce.unk_0C = 0;

        gwl_entries.push_back(athena_oce);
    }

    std::sort(gwl_entries.begin(), gwl_entries.end(), GWLComparer(this));

    *psize = gwl_entries.size() * sizeof(GWLEntry);
    
	uint8_t *buf = new uint8_t[*psize];
	memcpy(buf, gwl_entries.data(), *psize);
	
    return buf;
}

bool GpdFile::LoadGwdTournaments(const GwdFile &gwd)
{
    gwd_tournaments.clear();

    if (entries.size() == 0)
        return true;

    gwd_tournaments.resize(entries.size());

    for (size_t i = 0; i < entries.size(); i++)
    {
        assert(i == entries[i].id);
        gwd_tournaments[i] = gwd.GetCharacterTours(i);
    }

    return true;
}

bool GpdFile::SetGwdTournaments(GwdFile &gwd) const
{
    if (gwd_tournaments.size() == 0)
        return false;

    assert(gwd_tournaments.size() == entries.size());

    for (size_t i = 0; i < gwd.GetNumTournaments(); i++)
    {
        std::vector<uint32_t> participants;

        for (size_t j = 0; j < gwd_tournaments.size(); j++)
        {
            const std::vector<uint32_t> &tours_this_pp = gwd_tournaments[j];

            if (std::find(tours_this_pp.begin(), tours_this_pp.end(), i) != tours_this_pp.end())
            {
                participants.push_back(j);
            }
        }

        if (!gwd.SetParticipants(i, participants))
            return false;
    }

    return true;
}

TiXmlDocument *GpdFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("GPD");

    if (char_map.size() != 0)
        Utils::WriteComment(root, " This file has machine generated comments. Any change to these comments will be lost on next decompilation. ");

    for (size_t i = 0; i < entries.size(); i++)
    {
        const CharacterDef *def = nullptr;
        const std::vector<uint32_t> *tours = nullptr;

        if (char_map.size() != 0)
        {
            def = &char_map[i];
        }

        if (gwd_tournaments.size() != 0)
        {
            tours = &gwd_tournaments[i];
        }

        entries[i].Decompile(root, def, tours);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool GpdFile::Compile(TiXmlDocument *doc, bool big_endian)
{
    std::vector<bool> initialized;

    Reset();
    this->big_endian = big_endian;

    TiXmlHandle handle(doc);
    TiXmlElement *root = Utils::FindRoot(&handle, "GPD");

    if (!root)
    {
        DPRINTF("%s: Cannot find GPD root.\n", FUNCNAME);
        return false;
    }

    size_t i = 0;

    // First pass to know number of entries, and do later id check
    for (TiXmlElement *elem = root->FirstChildElement(); elem != nullptr; elem = elem->NextSiblingElement())
    {
        const std::string &name = elem->ValueStr();

        if (name == "GpdEntry")
           i++;
    }

    if (i == 0)
        return true;

    entries.resize(i);
    initialized.resize(i);

    // Second pass
    for (TiXmlElement *elem = root->FirstChildElement(); elem != nullptr; elem = elem->NextSiblingElement())
    {
        const std::string &name = elem->ValueStr();

        if (name == "GpdEntry")
        {
            GpdEntry entry;
            CharacterDef *def;
            std::vector<uint32_t> *tours_ids;

            if (!entry.Compile(elem, &def, &tours_ids))
                return false;

            if (entry.id >= entries.size())
            {
                DPRINTF("%s: id 0x%x is out of bounds.\n", FUNCNAME, entry.id);
                if (def)
                    delete def;

                if (tours_ids)
                    delete tours_ids;

                return false;
            }

            if (initialized[entry.id])
            {
                DPRINTF("%s: element with id 0x%x exists more than once.\n", FUNCNAME, entry.id);
                if (def)
                    delete def;

                if (tours_ids)
                    delete tours_ids;

                return false;
            }

            entries[entry.id] = entry;
            initialized[entry.id] = true;

            if (def)
            {
                if (char_map.size() == 0)
                {
                    char_map.resize(entries.size());
                }

                char_map[entry.id] = *def;
                delete def;
            }
            else if (char_map.size() != 0)
            {
                DPRINTF("%s: Extra information is incomplete. It is in some characters and not in other!\n", FUNCNAME);
                return false;
            }

            if (tours_ids)
            {
                if (gwd_tournaments.size() == 0)
                {
                    gwd_tournaments.resize(entries.size());
                }

                gwd_tournaments[entry.id] = *tours_ids;
            }
            else if (gwd_tournaments.size() != 0)
            {
                DPRINTF("%s: Tournaments information is incomplete. It is in some characters and not in other!\n", FUNCNAME);
                return false;
            }
        }
    }

    return true;
}


