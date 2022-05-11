#include <algorithm>

#include "GwdFile.h"
#include "GpdFile.h"
#include "SsssData.h"

#include "debug.h"

void GwdParticipant::Decompile(TiXmlNode *root, const GpdFile *gpd) const
{
    TiXmlElement *entry_root = new TiXmlElement("Participant");

    if (gpd)
    {
        uint32_t cms_entry, cms_model_spec_idx;
        const GpdEntry *gpd_entry = gpd->GetEntry(gpd_id, &cms_entry, &cms_model_spec_idx);

        if (gpd_entry)
        {
            SsssCharInfo *ssss_info = SsssData::FindInfo(cms_entry, cms_model_spec_idx);

            if (ssss_info)
            {
                Utils::WriteComment(entry_root, std::string(" ") + ssss_info->model_name + std::string(" / ") + ssss_info->char_name);
            }
        }
    }

    Utils::WriteParamUnsigned(entry_root, "GPD_ID", gpd_id, true);
    Utils::WriteParamUnsigned(entry_root, "UNK1", unk1, true);
    Utils::WriteParamUnsigned(entry_root, "UNK2", unk2, true);
    Utils::WriteParamUnsigned(entry_root, "UNK3", unk3, true);
    Utils::WriteParamUnsigned(entry_root, "UNK4", unk4, true);
    Utils::WriteParamUnsigned(entry_root, "UNK5", unk5, true);

    root->LinkEndChild(entry_root);
}

bool GwdParticipant::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "GPD_ID", &gpd_id))
        return false;

    if (!Utils::GetParamUnsigned(root, "UNK1", &unk1))
        return false;

    if (!Utils::GetParamUnsigned(root, "UNK2", &unk2))
        return false;

    if (!Utils::GetParamUnsigned(root, "UNK3", &unk3))
        return false;

    if (!Utils::GetParamUnsigned(root, "UNK4", &unk4))
        return false;

    if (!Utils::GetParamUnsigned(root, "UNK5", &unk5))
        return false;

    return true;
}

void GwdTournament::Decompile(TiXmlNode *root, uint32_t id, const GpdFile *gpd) const
{
    TiXmlElement *entry_root = new TiXmlElement("Tournament");
	
	SsssTournamentInfo *info = SsssData::FindTournamentInfo(id);
	if (info)
	{
		Utils::WriteComment(entry_root, info->name);
	}

    Utils::WriteParamUnsigned(entry_root, "CATEGORY_ID", category, true);
    Utils::WriteParamUnsigned(entry_root, "CATEGORY_INDEX", category_index);
    Utils::WriteParamUnsigned(entry_root, "NAME_ID", name_id, true);
    Utils::WriteParamUnsigned(entry_root, "U_0C", unk_0C, true);
    Utils::WriteParamUnsigned(entry_root, "U_10", unk_10, true);
    Utils::WriteParamUnsigned(entry_root, "U_14", unk_14, true);

    for (const GwdParticipant &p : participants)
    {
        p.Decompile(entry_root, gpd);
    }

    root->LinkEndChild(entry_root);
}

bool GwdTournament::Compile(const TiXmlElement *root)
{
    uint32_t category_u32;

    if (!Utils::GetParamUnsigned(root, "CATEGORY_ID", &category_u32))
        return false;

    category = (GwdCategory)category_u32;

    if (!Utils::GetParamUnsigned(root, "CATEGORY_INDEX", &category_index))
        return false;

    if (!Utils::GetParamUnsigned(root, "NAME_ID", &name_id))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0C", &unk_0C))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_10", &unk_10))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_14", &unk_14))
        return false;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Participant")
        {
            GwdParticipant participant;

            if (!participant.Compile(elem))
                return false;

            participants.push_back(participant);
        }
    }

    return true;
}

GwdFile::GwdFile()
{
    big_endian = false;
    gpd = nullptr;
    Reset();
}

GwdFile::~GwdFile()
{
    Reset();
}

void GwdFile::Reset()
{
    tournaments.clear();
}

std::vector<uint32_t> GwdFile::GetCharacterTours(uint32_t gpd_id) const
{
    std::vector<uint32_t> tours;

    for (size_t i = 0; i < tournaments.size(); i++)
    {
        const GwdTournament &t = tournaments[i];

        for (const GwdParticipant &p : t.participants)
        {
            if (p.gpd_id == gpd_id)
            {
                tours.push_back(i);
                break;
            }
        }
    }

    return tours;
}

bool GwdFile::SetParticipants(uint32_t id, std::vector<uint32_t> &new_participants)
{
    if (id >= tournaments.size())
        return false;

    GwdTournament &t = tournaments[id];
    if (t.participants.size() == 0)
        return false;

    // Add new entries
    for (size_t i = 0; i < new_participants.size(); i++)
    {
        auto it = std::find_if(t.participants.begin(), t.participants.end(), GwdParticipantFinder(new_participants[i]));

        if (it == t.participants.end())
        {
            // We'll just copy params from first entry and update gpd id
            GwdParticipant p = t.participants[0];

            p.gpd_id = new_participants[i];
            t.participants.push_back(p);
        }
    }

    // remove entries
    for (size_t i = 0; i < t.participants.size(); i++)
    {
        if (std::find(new_participants.begin(), new_participants.end(), t.participants[i].gpd_id) == new_participants.end())
        {
            t.participants.erase(t.participants.begin()+i);
            i--;
        }
    }

    return true;
}

bool GwdFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    GWDHeader *hdr = (GWDHeader *)buf;

    if (size < sizeof(GWDHeader) || memcmp(hdr->signature, GWD_SIGNATURE, 4) != 0)
        return false;

    big_endian = (buf[4] != 0xFE);

    GWDEntry *entries = (GWDEntry *)GetOffsetPtr(buf, hdr->data_start);

    tournaments.resize(val32(hdr->num_tournaments));

    for (size_t i = 0; i < tournaments.size(); i++)
    {
        GwdTournament &tournament = tournaments[i];

        tournament.category = (GwdCategory)val32(entries[i].category);
        if (tournament.category >= NUM_GWD_CATEGORIES)
            return false;

        tournament.category_index = val32(entries[i].category_index);
        tournament.name_id = val32(entries[i].name_id);
        tournament.unk_0C = val32(entries[i].unk_0C);
        tournament.unk_10 = val32(entries[i].unk_10);
        tournament.unk_14 = val32(entries[i].unk_14);

        tournament.participants.resize(val64(entries[i].num_participants));

        uint32_t *gpd_ids = (uint32_t *)GetOffsetPtr(buf, entries[i].gpd_ids_offset);
        for (size_t j = 0; j < tournament.participants.size(); j++)
        {
            tournament.participants[j].gpd_id = val32(gpd_ids[j]);
        }

        uint32_t *unk1 = (uint32_t *)GetOffsetPtr(buf, entries[i].unk_offset);
        for (size_t j = 0; j < tournament.participants.size(); j++)
        {
            tournament.participants[j].unk1 = val32(unk1[j]);
        }

        uint32_t *unk2 = (uint32_t *)GetOffsetPtr(buf, entries[i].unk2_offset);
        for (size_t j = 0; j < tournament.participants.size(); j++)
        {
            tournament.participants[j].unk2 = val32(unk2[j]);
        }

        uint32_t *unk3 = (uint32_t *)GetOffsetPtr(buf, entries[i].unk3_offset);
        for (size_t j = 0; j < tournament.participants.size(); j++)
        {
            tournament.participants[j].unk3 = val32(unk3[j]);
        }

        uint32_t *unk4 = (uint32_t *)GetOffsetPtr(buf, entries[i].unk4_offset);
        for (size_t j = 0; j < tournament.participants.size(); j++)
        {
            tournament.participants[j].unk4 = val32(unk4[j]);
        }

        uint32_t *unk5 = (uint32_t *)GetOffsetPtr(buf, entries[i].unk5_offset);
        for (size_t j = 0; j < tournament.participants.size(); j++)
        {
            tournament.participants[j].unk5 = val32(unk5[j]);
        }
    }

    return true;
}

unsigned int GwdFile::CalculateFileSize()
{
    uint32_t file_size = sizeof(GWDHeader);

    file_size += tournaments.size() * sizeof(GWDEntry);

    for (const GwdTournament &t : tournaments)
    {
        file_size += t.participants.size() * 0x18;
    }

    return file_size;
}

uint8_t *GwdFile::Save(size_t *size)
{
    unsigned int file_size = CalculateFileSize();
    uint32_t offset;

    uint8_t *buf = new uint8_t[file_size];
    memset(buf, 0, file_size);

    GWDHeader *hdr = (GWDHeader *)buf;

    memcpy(hdr->signature, GWD_SIGNATURE, 4);
    hdr->endianess_check = val16(0xFFFE);
    hdr->num_tournaments = val32(tournaments.size());

    offset = sizeof(GWDHeader);
    hdr->data_start = val32(offset);

    GWDEntry *entries = (GWDEntry *)GetOffsetPtr(buf, offset, true);

    for (size_t i = 0; i < tournaments.size(); i++)
    {
        const GwdTournament &tournament = tournaments[i];

        entries[i].category = val32(tournament.category);
        entries[i].category_index = val32(tournament.category_index);
        entries[i].name_id = val32(tournament.name_id);
        entries[i].unk_0C = val32(tournament.unk_0C);
        entries[i].unk_10 = val32(tournament.unk_10);
        entries[i].unk_14 = val32(tournament.unk_14);
        entries[i].num_participants = val64(tournament.participants.size());

        offset += sizeof(GWDEntry);
    }

    for (size_t i = 0; i < tournaments.size(); i++)
    {
        const GwdTournament &tournament = tournaments[i];
        entries[i].gpd_ids_offset = val32(offset);

        for (size_t j = 0; j < tournament.participants.size(); j++)
        {
            uint32_t *gpd_id = (uint32_t *)GetOffsetPtr(buf, offset, true);

            *gpd_id = tournament.participants[j].gpd_id;
            offset += sizeof(uint32_t);
        }
    }

    for (size_t i = 0; i < tournaments.size(); i++)
    {
        const GwdTournament &tournament = tournaments[i];
        entries[i].unk_offset = val32(offset);

        for (size_t j = 0; j < tournament.participants.size(); j++)
        {
            uint32_t *unk1 = (uint32_t *)GetOffsetPtr(buf, offset, true);

            *unk1 = tournament.participants[j].unk1;
            offset += sizeof(uint32_t);
        }
    }

    for (size_t i = 0; i < tournaments.size(); i++)
    {
        const GwdTournament &tournament = tournaments[i];
        entries[i].unk2_offset = val32(offset);

        for (size_t j = 0; j < tournament.participants.size(); j++)
        {
            uint32_t *unk2 = (uint32_t *)GetOffsetPtr(buf, offset, true);

            *unk2 = tournament.participants[j].unk2;
            offset += sizeof(uint32_t);
        }
    }

    for (size_t i = 0; i < tournaments.size(); i++)
    {
        const GwdTournament &tournament = tournaments[i];
        entries[i].unk3_offset = val32(offset);

        for (size_t j = 0; j < tournament.participants.size(); j++)
        {
            uint32_t *unk3 = (uint32_t *)GetOffsetPtr(buf, offset, true);

            *unk3 = tournament.participants[j].unk3;
            offset += sizeof(uint32_t);
        }
    }

    for (size_t i = 0; i < tournaments.size(); i++)
    {
        const GwdTournament &tournament = tournaments[i];
        entries[i].unk4_offset = val32(offset);

        for (size_t j = 0; j < tournament.participants.size(); j++)
        {
            uint32_t *unk4 = (uint32_t *)GetOffsetPtr(buf, offset, true);

            *unk4 = tournament.participants[j].unk4;
            offset += sizeof(uint32_t);
        }
    }

    for (size_t i = 0; i < tournaments.size(); i++)
    {
        const GwdTournament &tournament = tournaments[i];
        entries[i].unk5_offset = val32(offset);

        for (size_t j = 0; j < tournament.participants.size(); j++)
        {
            uint32_t *unk5 = (uint32_t *)GetOffsetPtr(buf, offset, true);

            *unk5 = tournament.participants[j].unk5;
            offset += sizeof(uint32_t);
        }
    }

    assert(offset == file_size);

    *size = file_size;
    return buf;
}

TiXmlDocument *GwdFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("GWD");

    if (gpd)
        Utils::WriteComment(root, " This file has machine generated comments. Any change to these comments will be lost on next decompilation. ");

    for (size_t i = 0; i < tournaments.size(); i++)
    {
        tournaments[i].Decompile(root, i, gpd);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool GwdFile::Compile(TiXmlDocument *doc, bool big_endian)
{
    Reset();
    this->big_endian = big_endian;

    TiXmlHandle handle(doc);
    TiXmlElement *root = Utils::FindRoot(&handle, "GWD");

    if (!root)
    {
        DPRINTF("%s: Cannot find GWD root.\n", FUNCNAME);
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Tournament")
        {
            GwdTournament tournament;

            if (!tournament.Compile(elem))
                return false;

            tournaments.push_back(tournament);
        }
    }

    return true;
}

