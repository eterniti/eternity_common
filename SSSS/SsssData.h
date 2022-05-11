#ifndef __SSSSDATA_H__
#define __SSSSDATA_H__

#include <stdint.h>
#include <string>

struct SsssCharInfo
{
	std::string char_name;
	std::string model_name;
	uint32_t cms_entry;
	uint32_t cms_model_spec_idx;
	uint32_t model_id;
    bool vanilla;
};

struct SsssTournamentInfo
{
	std::string name;
    uint32_t id;
};

struct SsssAssistPhraseInfo
{
    std::string name;
    uint32_t id;
};

namespace SsssData
{
	SsssCharInfo *FindInfo(uint32_t cms_entry, uint32_t cms_model_spec_idx);
	SsssCharInfo *FindInfo(uint32_t model_id);
    void AddInfo(const SsssCharInfo &info);
    void RemoveInfo(uint32_t cms_entry, uint32_t cms_model_spec_idx);
	
    SsssTournamentInfo *FindTournamentInfo(uint32_t id);
    void AddTournamentInfo(const SsssTournamentInfo &info);
    void RemoveTournamentInfo(uint32_t id);

    SsssAssistPhraseInfo *FindAssistPhraseInfo(uint32_t id);
    void AddAssistPhraseInfo(const SsssAssistPhraseInfo &info);
    void RemoveAssistPhraseInfo(uint32_t id);
}

#endif
