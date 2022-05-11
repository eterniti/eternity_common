#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <algorithm>

#include "SsssData.h"

static std::vector<SsssCharInfo> infos;
static std::vector<SsssTournamentInfo> tournament_infos;
static std::vector<SsssAssistPhraseInfo> ap_infos;

class SsssCharFinder
{
private:

	uint32_t cms_entry;
	uint32_t cms_model_spec_idx;

public:

	SsssCharFinder(uint32_t cms_entry, uint32_t cms_model_spec_idx) : cms_entry(cms_entry), cms_model_spec_idx(cms_model_spec_idx) { }
	
	bool operator()(const SsssCharInfo & entry)
	{
		return (entry.cms_entry == cms_entry && entry.cms_model_spec_idx == cms_model_spec_idx);
	}
};

class SsssCharFinder2
{
private:

	uint32_t model_id;

public:

	SsssCharFinder2(uint32_t model_id) : model_id(model_id) { }
	
    bool operator()(const SsssCharInfo &entry)
	{
		return (entry.model_id == model_id);
	}
};

class SsssTournamentFinder
{
private:

    uint32_t id;

public:

    SsssTournamentFinder(uint32_t id) : id(id) { }
    bool operator()(const SsssTournamentInfo &entry)
    {
        return (entry.id == id);
    }
};

class SsssAssistPhraseFinder
{
private:

    uint32_t id;

public:

    SsssAssistPhraseFinder(uint32_t id) : id(id) { }
    bool operator()(const SsssAssistPhraseInfo &entry)
    {
        return (entry.id == id);
    }
};

SsssCharInfo *SsssData::FindInfo(uint32_t cms_entry, uint32_t cms_model_spec_idx)
{
	std::vector<SsssCharInfo>::iterator it = std::find_if(infos.begin(), infos.end(), SsssCharFinder(cms_entry, cms_model_spec_idx));
	
	if (it == infos.end())
		return NULL;
	
	return &(*it);
}

SsssCharInfo *SsssData::FindInfo(uint32_t model_id)
{
	std::vector<SsssCharInfo>::iterator it = std::find_if(infos.begin(), infos.end(), SsssCharFinder2(model_id));
	
	if (it == infos.end())
        return nullptr;
	
	return &(*it);
}

void SsssData::AddInfo(const SsssCharInfo & info)
{
    std::vector<SsssCharInfo>::iterator it = std::find_if(infos.begin(), infos.end(), SsssCharFinder(info.cms_entry, info.cms_model_spec_idx));

    if (it != infos.end())
    {
        *it = info;
        return;
    }
	
	infos.push_back(info);
}

void SsssData::RemoveInfo(uint32_t cms_entry, uint32_t cms_model_spec_idx)
{
    std::vector<SsssCharInfo>::iterator it = std::find_if(infos.begin(), infos.end(), SsssCharFinder(cms_entry, cms_model_spec_idx));

    if (it != infos.end())
        infos.erase(it);
}

SsssTournamentInfo *SsssData::FindTournamentInfo(uint32_t id)
{
    auto it = std::find_if(tournament_infos.begin(), tournament_infos.end(), SsssTournamentFinder(id));

    if (it == tournament_infos.end())
        return nullptr;

    return &(*it);
}


void SsssData::AddTournamentInfo(const SsssTournamentInfo &info)
{
    auto it = std::find_if(tournament_infos.begin(), tournament_infos.end(), SsssTournamentFinder(info.id));

    if (it != tournament_infos.end())
    {
        *it = info;
        return;
    }

    tournament_infos.push_back(info);
}

void SsssData::RemoveTournamentInfo(uint32_t id)
{
     auto it = std::find_if(tournament_infos.begin(), tournament_infos.end(), SsssTournamentFinder(id));

     if (it != tournament_infos.end())
         tournament_infos.erase(it);
}


SsssAssistPhraseInfo *SsssData::FindAssistPhraseInfo(uint32_t id)
{
    auto it = std::find_if(ap_infos.begin(), ap_infos.end(), SsssAssistPhraseFinder(id));

    if (it == ap_infos.end())
        return nullptr;

    return &(*it);
}

void SsssData::AddAssistPhraseInfo(const SsssAssistPhraseInfo &info)
{
    auto it = std::find_if(ap_infos.begin(), ap_infos.end(), SsssAssistPhraseFinder(info.id));

    if (it != ap_infos.end())
    {
        *it = info;
        return;
    }

    ap_infos.push_back(info);
}

void SsssData::RemoveAssistPhraseInfo(uint32_t id)
{
    auto it = std::find_if(ap_infos.begin(), ap_infos.end(), SsssAssistPhraseFinder(id));

    if (it != ap_infos.end())
        ap_infos.erase(it);
}
