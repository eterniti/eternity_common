#include <stdint.h>

#include <vector>
#include <algorithm>

#include "SlotsFile.h"
#include "SsssData.h"
#include "debug.h"

#define CHECK_LOADED_BOOL() { if (!this->slots_buf) { return false; } }
#define CHECK_LOADED_U32() { if (!this->slots_buf) { return (uint32_t)-1; } }
#define CHECK_LOADED_PTR() { if (!this->slots_buf) { return NULL; } }

std::vector<std::string> lists_names =
{
	"PEGASUS_SEIYA", // 0
	"SAGITARIUS_SEIYA",
	"PEGASUS_SEIYA_GOD",
	"ODIN_SEIYA",
	"DRAGON_SHIRYU",
	"LIBRA_SHIRYU",
	"CYGNUS_HYOGA",
	"AQUARIUS_HYOGA",
	"ANDROMEDA_SHUN",
	"VIRGO_SHUN",
	"PHOENIX_IKKI", // 10
	"LEO_IKKI",
	"HIDRA_ICHI",
	"UNICORN_JABU",
	"EAGLE_MARIN",
	"OPHIUCHUS_SHAINA",
	"LYRA_ORPHEE",
	"ARIES_MUU_SAINT",
	"ARIES_SHION_SAINT",
	"ARIES_SHION_SPECTRE",
	"TAURUS_ALDEBARAN_SAINT", // 20
	"GEMINI_MYSTERIOUS_SAINT",
	"GEMINI_SAGA_SAINT",
	"GEMINI_SAGA_SPECTRE",
	"CANCER_DEATHMASK_SAINT",
	"CANCER_DEATHMASK_SPECTRE",
	"LEO_AIORIA_SAINT",
	"VIRGO_SHAKA_SAINT",
	"LIBRA_DOHKO_SAINT",
	"SCORPIUS_MILO_SAINT",
	"SAGITARIUS_AIOROS_SAINT", // 30
	"CAPRICORNUS_SHURA_SAINT",
	"CAPRICORNUS_SHURA_SPECTRE",
	"AQUARIUS_CAMUS_SAINT",
	"AQUARIUS_CAMUS_SPECTRE",
	"PISCES_APHRODITE_SAINT",
	"PISCES_APHRODITE_SPECTRE",
	"GEMINI_KANON_SAINT",
	"DRAGONSEA_KANON",
	"SIREN_SORRENTO",
	"SEAHORSE_BAIAN", // 40
	"SCYLLA_IO",
	"CHRYSAOR_KRISHNA",
	"LYUMNADES_KASA",
	"KRAKEN_ISAAC",
	"POSEIDON",
	"JULIAN",
	"WYVERN_RHADAMANTHYS",
	"GARUDA_AIACOS",
	"GRIFFON_MINOS",
	"THANATOS", // 50
	"HYPNOS",
	"HADES",
	"ATHENA",
	"BRONZE_ARMOR",
	"SILVER_ARMOR",
	"GOLD_ARMOR",
	"POSEIDON_AND_GENERALS",
	"HADES_AND_SERVANTS",
	"GEMINI_EVIL_SAGA_SAINT",
	"DUHBE_SIGFRIED", // 60
	"PHECDA_THOR",
	"MEGREZ_ALBERICH",
	"MERAK_HAGEN",
	"ALIOTH_FENRIR",
	"MIZAR_SYD",
	"ALCOR_BUD",
	"BENESTACH_MIME",
	"POLARIS_HILDA",
	"GOD_WARRIORS",
	"ARIES_MUU_GOD", // 70
	"TAURUS_ALDEBARAN_GOD",
	"GEMINI_SAGA_GOD",
	"CANCER_DEATHMASK_GOD",
	"LEO_AIORIA_GOD",
	"VIRGO_SHAKA_GOD",
	"LIBRA_DOHKO_GOD",
	"SCORPIUS_MILO_GOD",
	"SAGITARIUS_AIOROS_GOD",
	"CAPRICORNUS_SHURA_GOD",
	"AQUARIUS_CAMUS_GOD", // 80
	"PISCES_APHRODITE_GOD",
	"GOLD_GOD_ARMOR",
	"DRAGON_SHIRYU_GOD",
	"CYGNUS_HYOGA_GOD",
	"ANDROMEDA_SHUN_GOD",
	"PHOENIX_IKKI_GOD",
	"SAGITARIUS_SEIYA_GOD",
	"BRONZE_GOD_SAINT", // 88
	"EMPTY1", // 89
	"EMPTY2",
	"EMPTY3",
	"EMPTY4",
	"EMPTY5",
	"EMPTY6",
};

SlotsFile::SlotsFile()
{
	this->slots_buf = NULL;
	this->ci_buf = NULL;
	this->ci2_buf = NULL;
	big_endian = false;
	Reset();	
}

SlotsFile::~SlotsFile()
{
	Reset();
}

void SlotsFile::Reset()
{
	if (slots_buf)
	{	
		delete[] slots_buf;
        c_slots = NULL;
	}
	
	slots_size = 0;	
	
	if (ci_buf)
	{
		delete[] ci_buf;
		ci_buf = NULL;
	}
	
	ci_size = 0;
	
	if (ci2_buf)
	{
		delete[] ci2_buf;
		ci2_buf = NULL;
	}
	
    c_slots = NULL, infos = NULL, infos2 = NULL;
	num_slots = num_infos = num_infos2 = (uint32_t)-1;
	
	characters_lists.clear();
	characters_lists.resize(NUM_CHARACTERS_LISTS_NOT_EMPTY);
	
	infos3.clear();
}

bool SlotsFile::Load(SlotsLoadData *load_data)
{
	Reset();
	
	if((load_data->slots_size % sizeof(CharacterSlot)) != 0)
	{
		DPRINTF("slots_size is not multiple of structure size.\n");
		return false;
	}
	
	if ((load_data->ci_size % sizeof(CharacterInfo)) != 0)
	{
		DPRINTF("ci_size is not multiple of structure size.\n");
		return false;
	}
	
	if ((load_data->cl_size % sizeof(CharacterDef)) != 0)
	{
		DPRINTF("cl_size is not multiple of structure size.\n");
		return false;
	}
	
	if ((load_data->ci2_size % sizeof(CharacterInfo2)) != 0)
	{
		DPRINTF("ci_size2 is not multiple of structure size.\n");
		return false;
	}
	
	unsigned int num_lists = GetNumberOfLists(load_data->cl_buf, load_data->cl_size);	
	if (num_lists != NUM_CHARACTERS_LISTS_NOT_EMPTY)
	{
		DPRINTF("Number of lists is not the expected one. Expected: %d, got %d.\n", NUM_CHARACTERS_LISTS_NOT_EMPTY, num_lists);
		return false;
	}
	
	this->slots_buf = new uint8_t[load_data->slots_size];
	if (!this->slots_buf)
	{
		DPRINTF("Memory allocation error.\n");
		return false;
	}	
	
	this->ci_buf = new uint8_t[load_data->ci_size];
	if (!this->ci_buf)
	{
		delete[] this->slots_buf;
		this->slots_buf = NULL;
		return false;
	}
	
	this->ci2_buf = new uint8_t[load_data->ci2_size];
	if (!this->ci2_buf)
	{
		delete[] this->slots_buf;
		delete[] this->ci_buf;
		this->slots_buf = this->ci_buf = NULL;
		return false;
	}
	
	memcpy(this->slots_buf, load_data->slots_buf, load_data->slots_size);
	memcpy(this->ci_buf, load_data->ci_buf, load_data->ci_size);
	memcpy(this->ci2_buf, load_data->ci2_buf, load_data->ci2_size);
		
	this->slots_size = load_data->slots_size;
	this->ci_size = load_data->ci_size;	
	this->ci2_size = load_data->ci2_size;
	
    this->c_slots = (CharacterSlot *)this->slots_buf;
	this->infos = (CharacterInfo *)this->ci_buf;
	this->infos2 = (CharacterInfo2 *)this->ci2_buf;
	
	this->num_slots = this->slots_size / sizeof(CharacterSlot);
	this->num_infos = this->ci_size / sizeof(CharacterInfo);	
	this->num_infos2 = this->ci2_size / sizeof(CharacterInfo2);
	
	CharacterDef *current = (CharacterDef *)load_data->cl_buf;
	for (unsigned int i = 0; i < num_lists; i++, current++)
	{
		std::vector<CharacterDef> list;
		
		while (val32(current->cms_entry) != 0x6F)
		{
			CharacterDef entry;
			
			entry.cms_entry = current->cms_entry;
			entry.cms_model_spec_idx = current->cms_model_spec_idx;
			
			list.push_back(entry);
			current++;
		}
		
		size_t idx = i;
		
		if (idx == (GEMINI_SAGA_SAINT+1))
		{
			idx = GEMINI_EVIL_SAGA_SAINT;
		}
		else if (idx > GEMINI_SAGA_SAINT && idx <= GEMINI_EVIL_SAGA_SAINT)
		{
			idx--;
		}
		
		characters_lists[idx] = list;
	}
	
	if (!LoadCI3(load_data->ci3_buf, load_data->ci3_size))
		return false;
	
	if (!LoadCI4(load_data->ci4_buf, load_data->ci4_size))
		return false;
	
	return true;
}

bool SlotsFile::LoadCI3(uint8_t *ci3_buf, uint32_t ci3_size)
{
	if ((ci3_size % sizeof(CharacterInfo3)) != 0)
	{
		DPRINTF("ci3_size is not multiple of CharacterInfo3.\n");
		return false;
	}
	
	infos3.clear();
	
	uint32_t num_infos3 = ci3_size / sizeof(CharacterInfo3);	
	uint8_t *ptr = ci3_buf;
	
	for (uint32_t i = 0; i < num_infos3; i++)
	{
		CharacterInfo3 info3;
		
		memcpy(&info3, ptr, sizeof(CharacterInfo3));
		ptr += sizeof(CharacterInfo3);
		
		infos3.push_back(info3);
	}
	
	return true;
}

bool SlotsFile::LoadCI4(uint8_t *ci4_buf, uint32_t ci4_size)
{
	if ((ci4_size % sizeof(CharacterInfo4)) != 0)
	{
		DPRINTF("ci3_size is not multiple of CharacterInfo4.\n");
		return false;
	}
	
	infos4.clear();
	
	uint8_t *ptr = ci4_buf;
	
	while (1)
	{
		CharacterInfo4 info4;
		
		memcpy(&info4, ptr, sizeof(CharacterInfo4));
		ptr += sizeof(CharacterInfo4);
		
		if (val32(info4.cms_entry) == 0x6F)
			break;
		
		infos4.push_back(info4);
	}
	
	return true;
}

CharacterInfo *SlotsFile::FindCharacterInfo(uint32_t cms_entry, uint32_t cms_model_spec_idx, bool show_error)
{
	for (uint32_t i = 0; i < num_infos; i++)
	{
		if (infos[i].cms_entry == cms_entry && infos[i].cms_model_spec_idx == cms_model_spec_idx)
		{
			return &infos[i];
		}
	}	
	
	if (show_error)
		DPRINTF("CRITICAL: cannot find characters info (0x%x %d)\n", cms_entry, cms_model_spec_idx);
	
	return nullptr;
}

CharacterInfo2 *SlotsFile::FindCharacterInfo2(uint32_t cms_entry, uint32_t cms_model_spec_idx, bool show_error)
{
	for (uint32_t i = 0; i < num_infos2; i++)
	{
		if (infos2[i].cms_entry == cms_entry && infos2[i].cms_model_spec_idx == cms_model_spec_idx)
		{
			//DPRINTF("Found at %x\n", i);
			return &infos2[i];
		}
	}	
	
	if (show_error)
		DPRINTF("CRITICAL: cannot find characters info2 (0x%x %d)\n", cms_entry, cms_model_spec_idx);
	
	return NULL;
}

bool SlotsFile::IdenticalInfo2Exists(const CharacterInfo2 *info2)
{
	for (uint32_t i = 0; i < num_infos2; i++)
	{
		if (memcmp(infos2+i, info2, sizeof(CharacterInfo2)) == 0)
			return true;
	}

	return false;
}

CharacterInfo3 *SlotsFile::FindCharacterInfo3(uint32_t cms_entry, uint32_t cms_model_spec_idx)
{
	auto it = std::find_if(infos3.begin(), infos3.end(), CI3Finder(cms_entry, cms_model_spec_idx));
	
	if (it == infos3.end())
    {
        return nullptr;
    }
	
    return &*it;
}

CharacterInfo4 *SlotsFile::FindCharacterInfo4(uint32_t cms_entry, uint32_t cms_model_spec_idx)
{
	auto it = std::find_if(infos4.begin(), infos4.end(), CI4Finder(cms_entry, cms_model_spec_idx));
	
	if (it == infos4.end())
    {
        return nullptr;
    }
	
	return &*it;
}

unsigned int SlotsFile::GetNumberOfLists(uint8_t *buf, unsigned int size)
{
	unsigned int count = 0;
	CharacterDef *current = (CharacterDef *)buf;
	
	while ((uint8_t *)current < (buf + size))
	{
		while (val32(current->cms_entry) != 0x6F)
		{
			current++;
		}
		
		current++;
		count++;
	}
	
	return count;
}

bool SlotsFile::IsCharacterInList(size_t list, uint32_t cms_entry, uint32_t cms_model_spec_idx) const
{
	if (cms_entry == 0x6F)
		return false;
	
    for (const CharacterDef &e : characters_lists[list])
	{
		if (e.cms_entry == cms_entry && e.cms_model_spec_idx == cms_model_spec_idx)
			return true;
	}
	
	return false;
}

size_t SlotsFile::GetListFromName(const std::string & str)
{
	for (size_t i = 0; i < lists_names.size(); i++)
	{
		if (str == lists_names[i])
			return i;
	}
	
	return (size_t)-1;
}

void SlotsFile::GetCategories(Slot *slot, std::string & categories) const
{
	categories.clear();
	
	for (size_t i = PEGASUS_SEIYA; i < EMPTY1; i++)
	{
		if (IsCharacterInList(i, slot->cms_entry, slot->cms_model_spec_idx))
		{
			if (categories.length() != 0)
				categories += ", ";
			
			categories += lists_names[i];
		}
	}	
}

bool SlotsFile::FindSlot(uint32_t cms_entry, uint32_t cms_model_spec_idx, uint32_t *slot_idx, uint32_t *sub_slot_idx)
{
	CHECK_LOADED_BOOL();
	
	for (uint32_t i = 0; i < num_slots; i++)
	{
		if (IsCurrentRandomSlot(i))
			continue;
		
		for (uint32_t j = 0; j < MAX_SLOTS; j++)
		{
            if (val32(c_slots[i].sub_slots[j].cms_entry) == cms_entry && val32(c_slots[i].sub_slots[j].cms_model_spec_idx) == cms_model_spec_idx)
			{
				*slot_idx = i;
				*sub_slot_idx = j;
				return true;
			}
		}
	}
	
	return false;
}

bool SlotsFile::SlotHasCI3(uint32_t slot_idx, uint32_t subslot_idx)
{
	CHECK_LOADED_BOOL();	

	if (slot_idx >= num_slots)
		return false;
	
	if (subslot_idx >= MAX_SLOTS)
		return false;
	
	if (IsCurrentRandomSlot(slot_idx))
		return false;
	
	if (IsFreeSlot(slot_idx, subslot_idx))
		return false;
	
	const Slot &slot = c_slots[slot_idx].sub_slots[subslot_idx];
	
	return CharacterHasCI3(slot.cms_entry, slot.cms_model_spec_idx);
}

bool SlotsFile::SlotHasCI4(uint32_t slot_idx, uint32_t subslot_idx)
{
	CHECK_LOADED_BOOL();	

	if (slot_idx >= num_slots)
		return false;
	
	if (subslot_idx >= MAX_SLOTS)
		return false;
	
	if (IsCurrentRandomSlot(slot_idx))
		return false;
	
	if (IsFreeSlot(slot_idx, subslot_idx))
		return false;
	
	const Slot &slot = c_slots[slot_idx].sub_slots[subslot_idx];
	
	return CharacterHasCI4(slot.cms_entry, slot.cms_model_spec_idx);
}


bool SlotsFile::IsCurrentRandomSlot(uint32_t slot_idx) const
{
	CHECK_LOADED_BOOL();
	
	if (!IsOriginalRandomSlot(slot_idx))
		return false;
	
	for (uint32_t i = 0; i < MAX_SLOTS; i++)
	{
		if (!IsFreeSlot(slot_idx, i))
			return false;
	}
	
	return true;
}

bool SlotsFile::IsFreeSlot(uint32_t slot_idx, uint32_t sub_slot_idx) const
{
	CHECK_LOADED_BOOL();
	
	if (slot_idx >= num_slots)
		return false;
	
	if (sub_slot_idx >= MAX_SLOTS)
		return false;
	
	if (IsOriginalRandomSlot(slot_idx) && sub_slot_idx == 0)
	{
		const Slot &slot = c_slots[slot_idx].sub_slots[sub_slot_idx];
		
		if (slot.cms_entry == val32(1) && slot.cms_model_spec_idx == val32(0))
			return true;
	}
	
    return (c_slots[slot_idx].sub_slots[sub_slot_idx].cms_entry == val32(0x6F));
}

uint32_t SlotsFile::GetFreeSubSlot(uint32_t slot_idx)
{
	CHECK_LOADED_U32();
	
	for (uint32_t i = 0; i < MAX_SLOTS; i++)
	{
		if (IsFreeSlot(slot_idx, i))
			return i;
	}
	
	return (uint32_t)-1;
}

bool SlotsFile::GetSlot(uint32_t slot_idx, uint32_t sub_slot_idx, Slot *slot, CharacterInfo *info, CharacterInfo2 *info2,
                        std::string & categories, CharacterInfo3 **pinfo3, CharacterInfo4 **pinfo4)
{
	CHECK_LOADED_BOOL();
	
	if (slot_idx >= num_slots)
		return false;
	
	if (sub_slot_idx >= MAX_SLOTS)
		return false;
	
    memcpy(slot, &c_slots[slot_idx].sub_slots[sub_slot_idx], sizeof(Slot));
	
    const CharacterInfo *cinfo = FindCharacterInfo(slot);
	if (!cinfo)
		return false;
	
    const CharacterInfo2 *cinfo2 = FindCharacterInfo2(slot);
	if (!cinfo2)
		return false;
	
	memcpy(info, cinfo, sizeof(CharacterInfo));
	memcpy(info2, cinfo2, sizeof(CharacterInfo2));

    if (pinfo3)
    {
        const CharacterInfo3 *cinfo3 = FindCharacterInfo3(slot);

        if (cinfo3)
        {
            *pinfo3 = new CharacterInfo3;
            memcpy(*pinfo3, cinfo3, sizeof(CharacterInfo3));
        }
        else
        {
            *pinfo3 = nullptr;
        }
    }

    if (pinfo4)
    {
        const CharacterInfo4 *cinfo4 = FindCharacterInfo4(slot);

        if (cinfo4)
        {
            *pinfo4 = new CharacterInfo4;
            memcpy(*pinfo4, cinfo4, sizeof(CharacterInfo4));
        }
        else
        {
            *pinfo4 = nullptr;
        }
    }
	
	GetCategories(slot, categories);
	return true;
}

bool SlotsFile::GetCmsEntry(uint32_t slot_idx, uint32_t sub_slot_idx, uint32_t *cms_entry, uint32_t *cms_model_spec_idx)
{
	CHECK_LOADED_BOOL();
	
	if (slot_idx >= num_slots)
		return false;
	
	if (sub_slot_idx >= MAX_SLOTS)
		return false;
	
	*cms_entry = val32(c_slots[slot_idx].sub_slots[sub_slot_idx].cms_entry);
	*cms_model_spec_idx = val32(c_slots[slot_idx].sub_slots[sub_slot_idx].cms_model_spec_idx);
	
	return true;
}

void SlotsFile::RemoveFromAllCategories(Slot *slot)
{
	for (size_t i = PEGASUS_SEIYA; i < EMPTY1; i++)
	{
		for (size_t j = 0; j < characters_lists[i].size(); j++)
		{
			CharacterDef &entry = characters_lists[i][j];
			
			if (entry.cms_entry == slot->cms_entry && entry.cms_model_spec_idx == slot->cms_model_spec_idx)
			{
				characters_lists[i].erase(characters_lists[i].begin() + j);
				j--;
			}
		}
	}	
}

void SlotsFile::AppendDefaultCharacterInfo()
{
	uint32_t  new_ci_size = ci_size + sizeof(CharacterInfo);
	uint8_t *new_ci_buf = new uint8_t[new_ci_size];
	
	CharacterInfo default_ci;	
	
	memset(&default_ci, 0, sizeof(CharacterInfo));	
	default_ci.cms_entry = val32(0x6F);
	
	memcpy(new_ci_buf, ci_buf, ci_size);
	memcpy(new_ci_buf+ci_size, &default_ci, sizeof(CharacterInfo));
	
	delete[] ci_buf;
	ci_buf = new_ci_buf;	
	ci_size = new_ci_size;
	
	infos = (CharacterInfo *)ci_buf;
	num_infos++;
}

void SlotsFile::AppendDefaultCharacterInfo2()
{
	uint32_t  new_ci2_size = ci2_size + sizeof(CharacterInfo2);
	uint8_t *new_ci2_buf = new uint8_t[new_ci2_size];
	
	CharacterInfo2 default_ci2;	
	
	memset(&default_ci2, 0, sizeof(CharacterInfo2));	
	default_ci2.cms_entry = val32(0x6F);
	
	memcpy(new_ci2_buf, ci2_buf, ci2_size);
	memcpy(new_ci2_buf+ci2_size, &default_ci2, sizeof(CharacterInfo2));
	
	delete[] ci2_buf;
	ci2_buf = new_ci2_buf;	
	ci2_size = new_ci2_size;
	
	infos2 = (CharacterInfo2 *)ci2_buf;
	num_infos2++;
}

void SlotsFile::AppendCharacterInfo(CharacterInfo *info)
{
	AppendDefaultCharacterInfo();
	
	CharacterInfo *dst = infos + num_infos - 2;
	if (dst->cms_entry != 0x6F)
		dst++;
	
	memcpy(dst, info, sizeof(CharacterInfo));
}

void SlotsFile::AppendCharacterInfo2(CharacterInfo2 *info)
{
	AppendDefaultCharacterInfo2();
	
	CharacterInfo2 *dst = infos2 + num_infos2 - 2;
	if (dst->cms_entry != 0x6F)
		dst++;
	
	memcpy(dst, info, sizeof(CharacterInfo2));
}

bool SlotsFile::SetSlot(uint32_t slot_idx, uint32_t sub_slot_idx, Slot *slot, CharacterInfo *info, CharacterInfo2 *info2,
                        const std::string & categories, CharacterInfo3 *info3, CharacterInfo4 *info4)
{
	CHECK_LOADED_BOOL();
	
	/*if (IsRandomSlot(slot_idx))
		return false;*/
	
	if (slot_idx >= num_slots)
		return false;
	
	if (sub_slot_idx >= MAX_SLOTS)
		return false;
	
    CharacterInfo *cinfo = FindCharacterInfo(slot, false);
    CharacterInfo2 *cinfo2 = FindCharacterInfo2(slot, false);
	
	if ((cinfo && !cinfo2) || (!cinfo && cinfo2))
	{
		DPRINTF("%s: weird unsyncronization between CharacterInfo and CharacterInfo2.\n", __PRETTY_FUNCTION__);
		return false;
	}
	
	std::vector<std::string> categories_list;
	
	RemoveFromAllCategories(slot);
	Utils::GetMultipleStrings(categories, categories_list);
	
	for (std::string &s : categories_list)
	{
		size_t list = GetListFromName(s);
		
		if (list == (size_t)-1 || list >= NUM_CHARACTERS_LISTS_NOT_EMPTY)
		{
			DPRINTF("\"%s\" is not a valid category name.\n", s.c_str());
			return false;
		}
		
		CharacterDef list_entry;
		
		list_entry.cms_entry = slot->cms_entry;
		list_entry.cms_model_spec_idx = slot->cms_model_spec_idx;
		
		characters_lists[list].push_back(list_entry);					
	}
	
	if (cinfo)
	{
		memcpy(cinfo, info, sizeof(CharacterInfo));
		memcpy(cinfo2, info2, sizeof(CharacterInfo2));
	}
	
	else
	{
		AppendCharacterInfo(info);
		AppendCharacterInfo2(info2);
	}

    if (info3)
    {
       auto it = std::find_if(infos3.begin(), infos3.end(), CI3Finder(slot->cms_entry, slot->cms_model_spec_idx));

       if (it == infos3.end())
       {
           AppendCharacterInfo3(info3);
       }
       else
       {
           *it = *info3;
       }
    }

    if (info4)
    {
       auto it = std::find_if(infos4.begin(), infos4.end(), CI4Finder(slot->cms_entry, slot->cms_model_spec_idx));

       if (it == infos4.end())
       {
           AppendCharacterInfo4(info4);
       }
       else
       {
           *it = *info4;
       }
    }
	
    memcpy(&c_slots[slot_idx].sub_slots[sub_slot_idx], slot, sizeof(Slot));
	return true;
}

bool SlotsFile::SetIconId(uint32_t cms_entry, uint32_t cms_model_spec_idx, float icon_id)
{
	uint32_t slot, sub_slot;
	
	if (!FindSlot(cms_entry, cms_model_spec_idx, &slot, &sub_slot))
		return false;
	
	//UPRINTF("seticon = %f\n", icon_id);
	
	c_slots[slot].sub_slots[sub_slot].icon_id = icon_id;
	return true;
}

bool SlotsFile::SetSignNameId(uint32_t cms_entry, uint32_t cms_model_spec_idx, uint32_t sign_name_id)
{
	CharacterInfo *info = FindCharacterInfo(cms_entry, cms_model_spec_idx, false);
	if (!info)
		return false;
	
	info->sign_name_id = sign_name_id;
	return true;
}

bool SlotsFile::SetRobesNameId(uint32_t cms_entry, uint32_t cms_model_spec_idx, uint32_t robes_name_id)
{
	CharacterInfo *info = FindCharacterInfo(cms_entry, cms_model_spec_idx, false);
	if (!info)
		return false;
	
	info->robes_name_id = robes_name_id;
	return true;
}

bool SlotsFile::SetSelectNameId(uint32_t cms_entry, uint32_t cms_model_spec_idx, uint32_t select_name_id)
{
	CharacterInfo *info = FindCharacterInfo(cms_entry, cms_model_spec_idx, false);
	if (!info)
		return false;
	
	info->select_name_id = select_name_id;
	return true;
}

bool SlotsFile::SetSelect2NameId(uint32_t cms_entry, uint32_t cms_model_spec_idx, uint32_t select2_name_id)
{
	CharacterInfo *info = FindCharacterInfo(cms_entry, cms_model_spec_idx, false);
	if (!info)
		return false;
	
	info->select2_name_id = select2_name_id;
	return true;
}

bool SlotsFile::SetAvatarId(uint32_t cms_entry, uint32_t cms_model_spec_idx, uint32_t avatar_id)
{
	CharacterInfo2 *info2 = FindCharacterInfo2(cms_entry, cms_model_spec_idx, false);
	if (!info2)
		return false;
	
	info2->avatar_id = avatar_id;
	return true;
}

bool SlotsFile::SetBattleNameId(uint32_t cms_entry, uint32_t cms_model_spec_idx, uint32_t battle_name_id)
{
	CharacterInfo2 *info2 = FindCharacterInfo2(cms_entry, cms_model_spec_idx, false);
	if (!info2)
		return false;
	
	info2->battle_name_id = battle_name_id;
	return true;
}

bool SlotsFile::SetGwrNameId(uint32_t cms_entry, uint32_t cms_model_spec_idx, float gwr_name_id)
{
	CharacterInfo3 *info3 = FindCharacterInfo3(cms_entry, cms_model_spec_idx);
	if (!info3)
		return false;
	
	info3->gwr_name_id = gwr_name_id;
	return true;
}

bool SlotsFile::SetGwrIconId(uint32_t cms_entry, uint32_t cms_model_spec_idx, float gwr_icon_id)
{
	CharacterInfo3 *info3 = FindCharacterInfo3(cms_entry, cms_model_spec_idx);
	if (!info3)
		return false;
	
	info3->gwr_icon_id = gwr_icon_id;
	return true;
}

bool SlotsFile::SetGwtBannerNameId(uint32_t cms_entry, uint32_t cms_model_spec_idx, float gwt_banner_name_id)
{
	CharacterInfo4 *info4 = FindCharacterInfo4(cms_entry, cms_model_spec_idx);
	if (!info4)
		return false;
	
	info4->gwt_banner_name_id = gwt_banner_name_id;
	return true;
}

bool SlotsFile::SetGwtBannerId(uint32_t cms_entry, uint32_t cms_model_spec_idx, float gwt_banner_id)
{
	CharacterInfo4 *info4 = FindCharacterInfo4(cms_entry, cms_model_spec_idx);
	if (!info4)
		return false;
	
	info4->gwt_banner_id = gwt_banner_id;
	return true;
}

bool SlotsFile::MakeSlotFree(uint32_t slot_idx, uint32_t sub_slot_idx)
{
    CHECK_LOADED_BOOL();

    if (IsCurrentRandomSlot(slot_idx))
        return false;

    if (slot_idx >= num_slots)
        return false;

    if (sub_slot_idx >= MAX_SLOTS)
        return false;

    if (IsFreeSlot(slot_idx, sub_slot_idx))
        return true; // yeah true...
	
	Slot empty_slot;
	
	memset(&empty_slot, 0, sizeof(Slot));
	empty_slot.cms_entry = val32(0x6F);

	if (IsOriginalRandomSlot(slot_idx))
	{
		int free_count = 0;
		
		for (uint32_t j = 0; j < MAX_SLOTS; j++)
		{
			if (j != sub_slot_idx && IsFreeSlot(slot_idx, j))
				free_count++;
		}
		
		if (free_count == (MAX_SLOTS-1))
		{
			// Return to random state
			
			Slot slot = { .cms_entry = val32(1), .cms_model_spec_idx = 0, .icon_id = 149.0, .unk_0C = 0 };
			
			memcpy(&c_slots[slot_idx].sub_slots[0], &slot, sizeof(Slot));
			
			for (uint32_t j = 1; j < MAX_SLOTS; j++)
			{
				memcpy(&c_slots[slot_idx].sub_slots[j], &empty_slot, sizeof(Slot));
			}
			
			return true;
		}
		else if (sub_slot_idx == 0)
		{
			for (uint32_t j = 1; j < MAX_SLOTS; j++)
			{
				if (!IsFreeSlot(slot_idx, j))
				{
					memcpy(&c_slots[slot_idx].sub_slots[0], &c_slots[slot_idx].sub_slots[j], sizeof(Slot));
					memcpy(&c_slots[slot_idx].sub_slots[j], &empty_slot, sizeof(Slot));
					return true;
				}
			}
			
			assert(0);
		}
	}

    memcpy(&c_slots[slot_idx].sub_slots[sub_slot_idx], &empty_slot, sizeof(Slot));
    return true;
}

void SlotsFile::WriteSlot(TiXmlElement *root, Slot *slot, uint32_t parent_index, uint32_t index) const
{
	TiXmlElement *entry_root = new TiXmlElement("Slot");
		
	if (IsCurrentRandomSlot(parent_index))
	{
		Utils::WriteComment(entry_root, "This slot is of a random selected character.");		
	}
	else if (IsFreeSlot(parent_index, index))
	{		
		Utils::WriteComment(entry_root, "This slot is free.");		
	}
	else
	{
		SsssCharInfo *info = SsssData::FindInfo(val32(slot->cms_entry), val32(slot->cms_model_spec_idx));
	
		if (info)
		{
			Utils::WriteComment(entry_root, std::string(" ") + info->model_name + std::string(" / ") + info->char_name);			
		}
	}
		
	Utils::WriteParamUnsigned(entry_root, "CMS_ENTRY", val32(slot->cms_entry), true);
	Utils::WriteParamUnsigned(entry_root, "CMS_MODEL_SPEC_IDX", val32(slot->cms_model_spec_idx));
	
	Utils::WriteComment(entry_root, "Slot information, aka Slot pre-baked data");	
    Utils::WriteParamFloat(entry_root, "ICON_ID", val_float(slot->icon_id));
	Utils::WriteParamUnsigned(entry_root, "U_0C", val32(slot->unk_0C), true);
	
	// Info
    const CharacterInfo *info = const_cast<SlotsFile *>(this)->FindCharacterInfo(slot);
	if (!info)
	{
		delete entry_root;
		return;
	}
	
	Utils::WriteComment(entry_root, "Select information, aka CharacterInfo pre-baked data");	
	Utils::WriteParamUnsigned(entry_root, "SIGN_NAME_ID", val32(info->sign_name_id), true);
	Utils::WriteParamUnsigned(entry_root, "SELECT_NAME_ID", val32(info->select_name_id), true);
	Utils::WriteParamUnsigned(entry_root, "ROBES_NAME_ID", val32(info->robes_name_id), true);
	Utils::WriteParamUnsigned(entry_root, "SELECT2_NAME_ID", val32(info->select2_name_id), true);
	
	// Info2
    const CharacterInfo2 *info2 = const_cast<SlotsFile *>(this)->FindCharacterInfo2(slot);
	if (!info2)
	{
		delete entry_root;
		return;
	}
	
	Utils::WriteComment(entry_root, "Battle information, aka CharacterInfo2 pre-baked data");	
	Utils::WriteParamUnsigned(entry_root, "AVATAR_ID", val32(info2->avatar_id), true);
	Utils::WriteParamUnsigned(entry_root, "BATTLE_NAME_ID", val32(info2->battle_name_id), true);
	Utils::WriteParamUnsigned(entry_root, "CIU2_10", val32(info2->unk_10), true);
	
	// Info4, optional
    const CharacterInfo4 *info4 = const_cast<SlotsFile *>(this)->FindCharacterInfo4(slot);
	if (info4)
	{
		Utils::WriteComment(entry_root, "GW \"Tour\" information, aka CharacterInfo4 pre-baked data");
		Utils::WriteParamFloat(entry_root, "GWT_BANNER_NAME_ID", val_float(info4->gwt_banner_name_id));
		Utils::WriteParamFloat(entry_root, "GWT_BANNER_ID", val_float(info4->gwt_banner_id));
		Utils::WriteParamUnsigned(entry_root, "CIU4_10", val32(info4->unk_10), true);
		Utils::WriteParamUnsigned(entry_root, "CIU4_14", val32(info4->unk_14), true);
	}
	
	// Info3, optional
    const CharacterInfo3 *info3 = const_cast<SlotsFile *>(this)->FindCharacterInfo3(slot);
	if (info3)
	{
		Utils::WriteComment(entry_root, "GW Result information, aka CharacterInfo3 pre-baked data");
		Utils::WriteParamFloat(entry_root, "GWR_NAME_ID", val_float(info3->gwr_name_id));
		Utils::WriteParamFloat(entry_root, "GWR_ICON_ID", val_float(info3->gwr_icon_id));
		Utils::WriteParamUnsigned(entry_root, "CIU3_10", val32(info3->unk_10), true);
		Utils::WriteParamUnsigned(entry_root, "CIU3_14", val32(info3->unk_14), true);
	}	
	
	// Lists	
	std::string categories;	
	
	GetCategories(slot, categories);	
	Utils::WriteComment(entry_root, "Categories, aka Character Lists pre-baked data");
	Utils::WriteParamString(entry_root, "CATEGORIES", categories);
	
	root->LinkEndChild(entry_root);
}

void SlotsFile::WriteCharacterSlot(TiXmlElement *root, uint32_t index) const
{
    CharacterSlot *tslot = &c_slots[index];
	
	TiXmlElement *entry_root = new TiXmlElement("CharacterSlot");
	entry_root->SetAttribute("id", Utils::UnsignedToString(index, true));
	
	for (int i = 0; i < MAX_SLOTS; i++)
	{
        WriteSlot(entry_root, &tslot->sub_slots[i], index, i);
	}
	
	root->LinkEndChild(entry_root);
}

#define SLOT_U(n, f) { if (!Utils::GetParamUnsigned(root, n, &temp)) \
							return false; \
						slot->f = val32(temp); \
						}
						
#define SLOT_F(n, f) { if (!Utils::GetParamFloat(root, n, &ftemp)) \
							return false; \
						slot->f = val_float(ftemp); \
						}
						
#define CI_U(n, f) { if (!Utils::GetParamUnsigned(root, n, &temp)) \
							return false; \
						info->f = val32(temp); \
						}
						
#define CI2_U(n, f) { if (!Utils::GetParamUnsigned(root, n, &temp)) \
							return false; \
						info2->f = val32(temp); \
						}
						
#define CI3_U(n, f) { if (!Utils::GetParamUnsigned(root, n, &temp)) \
							return false; \
						(*pinfo3)->f = val32(temp); \
						}
						
#define CI3_F(n, f) { if (!Utils::GetParamFloat(root, n, &ftemp)) \
							return false; \
						(*pinfo3)->f = val_float(ftemp); \
						}
						
#define CI4_U(n, f) { if (!Utils::GetParamUnsigned(root, n, &temp)) \
							return false; \
						(*pinfo4)->f = val32(temp); \
						}
						
#define CI4_F(n, f) { if (!Utils::GetParamFloat(root, n, &ftemp)) \
							return false; \
						(*pinfo4)->f = val_float(ftemp); \
						}

bool SlotsFile::ReadSlot(TiXmlElement *root, Slot *slot, uint32_t index, bool *add_info, CharacterInfo *info, CharacterInfo2 *info2, CharacterInfo3 **pinfo3, CharacterInfo4 **pinfo4)
{
	uint32_t temp;
	float ftemp;
	
	*pinfo3 = nullptr;
	*pinfo4 = nullptr;
	*add_info = true;
	
	SLOT_U("CMS_ENTRY", cms_entry);
	SLOT_U("CMS_MODEL_SPEC_IDX", cms_model_spec_idx);

    //
    if (!Utils::GetParamFloatWithMultipleNames(root, &ftemp, "ICON_ID", "ICON_DATA"))
        return false;
    slot->icon_id = val_float(ftemp);
    //

    SLOT_U("U_0C", unk_0C);
	
	if (val32(slot->cms_entry) == 0x6F)
	{
		*add_info = false;
	}
	else
	{
		if (IsOriginalRandomSlot(index))
		{
			if (val32(slot->cms_entry) == 1 && slot->cms_model_spec_idx == 0)
			{
				*add_info = false;
			}
		}
	}
	
	if (*add_info)
	{
		// Info
		info->cms_entry = slot->cms_entry;
		info->cms_model_spec_idx = slot->cms_model_spec_idx;
		
		CI_U("SIGN_NAME_ID", sign_name_id);
		
		//
		if (!Utils::GetParamUnsignedWithMultipleNames(root, &temp, "SELECT_NAME_ID", "CIU_0C"))
			return false;	
		info->select_name_id = val32(temp);
		//
		
		CI_U("ROBES_NAME_ID", robes_name_id);
		
		//
		if (!Utils::GetParamUnsignedWithMultipleNames(root, &temp, "SELECT2_NAME_ID", "CIU_14"))
			return false;
		info->select2_name_id = val32(temp);
		//
		
		// Info 2
		info2->cms_entry = slot->cms_entry;
		info2->cms_model_spec_idx = slot->cms_model_spec_idx;
		
		CI2_U("AVATAR_ID", avatar_id);
		
		//
		if (!Utils::GetParamUnsignedWithMultipleNames(root, &temp, "BATTLE_NAME_ID", "CIU2_0C"))
			return false;
		info2->battle_name_id = val32(temp);
		//
	
		CI2_U("CIU2_10", unk_10);
		
		// Info4, optional
		if (Utils::ReadParamFloat(root, "GWT_BANNER_NAME_ID", &ftemp))
		{
			*pinfo4 = new CharacterInfo4;
			(*pinfo4)->gwt_banner_name_id = val_float(ftemp);
			(*pinfo4)->cms_entry = slot->cms_entry;
			(*pinfo4)->cms_model_spec_idx = slot->cms_model_spec_idx;
			
			CI4_F("GWT_BANNER_ID", gwt_banner_id);
			CI4_U("CIU4_10", unk_10);
			CI4_U("CIU4_14", unk_14);
		}
		
		// Info3, optional
		if (Utils::ReadParamFloat(root, "GWR_NAME_ID", &ftemp))
		{
			*pinfo3 = new CharacterInfo3;			
			(*pinfo3)->gwr_name_id = val_float(ftemp);
			(*pinfo3)->cms_entry = slot->cms_entry;
			(*pinfo3)->cms_model_spec_idx = slot->cms_model_spec_idx;
			
			CI3_F("GWR_ICON_ID", gwr_icon_id);
			CI3_U("CIU3_10", unk_10);
			CI3_U("CIU3_14", unk_14);
		}		
		
		// Lists
		std::vector<std::string> categories;
		
		if (!Utils::ReadParamMultipleStrings(root, "CATEGORIES", categories))
		{
			DPRINTF("Error getting param CATEGORIES.\n");
			return false;
		}		
		
		for (std::string &s : categories)
		{
			size_t list = GetListFromName(s);
			
			if (list == (size_t)-1 || list >= NUM_CHARACTERS_LISTS_NOT_EMPTY)
			{
				DPRINTF("\"%s\" is not a valid category name.\n", s.c_str());
				return false;
			}
			
			CharacterDef list_entry;
			
			list_entry.cms_entry = slot->cms_entry;
			list_entry.cms_model_spec_idx = slot->cms_model_spec_idx;
			
			characters_lists[list].push_back(list_entry);					
		}
	}
	
	return true;
}

// This comparison function doesn't leave the data in same state as original, but it is close enough (game doesn't care about order anyway)
bool CIComparer::operator()(const CharacterInfo & info1, const CharacterInfo & info2)
{
	//printf("Comparing %x,%x with %x,%x\n", info1.cms_entry, info1.cms_model_spec_idx, info2.cms_entry, info2.cms_model_spec_idx);
	
	if (info1.cms_entry == info2.cms_entry)
	{
        return (slf->val32(info1.cms_model_spec_idx) < slf->val32(info2.cms_model_spec_idx));
	}
	
    return (slf->val32(info1.cms_entry) < slf->val32(info2.cms_entry));
}

bool CLComparer::operator()(const CharacterDef & entry1, const CharacterDef & entry2)
{
	if (entry1.cms_entry == entry2.cms_entry)
	{
        return (slf->val32(entry1.cms_model_spec_idx) < slf->val32(entry2.cms_model_spec_idx));
	}
	
    return (slf->val32(entry1.cms_entry) < slf->val32(entry2.cms_entry));
}

bool CI2Comparer::operator()(const CharacterInfo2 & info1, const CharacterInfo2 & info2)
{
	if (info1.cms_entry == info2.cms_entry)
	{
        return (slf->val32(info1.cms_model_spec_idx) < slf->val32(info2.cms_model_spec_idx));
	}
	
    return (slf->val32(info1.cms_entry) < slf->val32(info2.cms_entry));
}

bool CI3Comparer::operator()(const CharacterInfo3 &info1, const CharacterInfo3 &info2)
{
   if (slf->val32(info1.cms_entry) == slf->val32(info2.cms_entry))
	   return (slf->val32(info1.cms_model_spec_idx) < slf->val32(info2.cms_model_spec_idx));
   
   return (slf->val32(info1.cms_entry) < slf->val32(info2.cms_entry));
}

bool CI4Comparer::operator()(const CharacterInfo4 &info1, const CharacterInfo4 &info2)
{
   if (slf->val32(info1.cms_entry) == slf->val32(info2.cms_entry))
	   return (slf->val32(info1.cms_model_spec_idx) < slf->val32(info2.cms_model_spec_idx));
   
   return (slf->val32(info1.cms_entry) < slf->val32(info2.cms_entry));
}

bool SlotsFile::ReadCharacterSlot(TiXmlElement *root, CharacterSlot *tslot, uint32_t index, std::vector<CharacterInfo> & info_list, std::vector<CharacterInfo2> & info2_list)
{
	int count = 0;
	
	size_t info_list_current = info_list.size();
	size_t info2_list_current = info2_list.size();
	
	for (TiXmlElement *elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{
		std::string name = elem->ValueStr();
		
		if (name == "Slot")
		{
			bool add_info;
			CharacterInfo info;
			CharacterInfo2 info2;
			CharacterInfo3 *info3;
			CharacterInfo4 *info4;
			
			if (count >= MAX_SLOTS)
			{
				DPRINTF("ReadCharacterSlot: Slots overflow: there must be only %d Slot per CharacterSlot. Not one more, not one less. (failed on entry 0x%x)\n", MAX_SLOTS, index);
				return false;
			}
			
            if (!ReadSlot(elem, &tslot->sub_slots[count], index, &add_info, &info, &info2, &info3, &info4))
				return false;			
			
            if (add_info)
			{
				info_list.push_back(info);		
				info2_list.push_back(info2);
				
				if (info3)
					infos3.push_back(*info3);
				
				if (info4)
					infos4.push_back(*info4);
			}
			
			if (info3)
				delete info3;
			
			if (info4)
				delete info4;
				
			count++;
		}
	}	
	
	if (count != MAX_SLOTS)
	{
		DPRINTF("ReadCharacterSlot: there must be exactly %d Slot per CharacterSlot. Not one more, not one less (you had %d) (failed on entry 0x%x)\n", MAX_SLOTS, count, index);
		return false;
	}
	
	std::sort(info_list.begin()+info_list_current, info_list.end(), CIComparer(this));
	std::sort(info2_list.begin()+info2_list_current, info2_list.end(), CI2Comparer(this));
	
	return true;
}

TiXmlDocument *SlotsFile::Decompile() const
{
	CHECK_LOADED_PTR();
	
	TiXmlDocument *doc = new TiXmlDocument();
	
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );	
	doc->LinkEndChild(decl);
	
	TiXmlElement *root = new TiXmlElement("SLF");	
	Utils::WriteComment(root, " This file has machine generated comments. Any change to these comments will be lost on next decompilation. ");	
	Utils::WriteComment(root, " This file is specially fragile. Human modification is NOT recommended. ");	
	
	for (uint32_t i = 0; i < num_slots; i++)
	{
		WriteCharacterSlot(root, i);
	}
	
	doc->LinkEndChild(root);
	
	return doc;
}

void SlotsFile::AddCharactersListsSpecialEntries()
{
	// Add special entries to some lists
	// We want to replicate the original lists with their original bugs :)
	CharacterDef sp_entry;
		
	if (!IsCharacterInList(PEGASUS_SEIYA, val32(1), val32(6)))
	{
		sp_entry = { .cms_entry = val32(1), .cms_model_spec_idx = val32(6) };
		characters_lists[PEGASUS_SEIYA].push_back(sp_entry);
	}
	
	if (!IsCharacterInList(PEGASUS_SEIYA, val32(1), val32(8)))
	{
		sp_entry = { .cms_entry = val32(1), .cms_model_spec_idx = val32(8) };
		characters_lists[PEGASUS_SEIYA].push_back(sp_entry);
	}
	
	if (!IsCharacterInList(DRAGON_SHIRYU, val32(2), val32(4)))
	{
		sp_entry = { .cms_entry = val32(2), .cms_model_spec_idx = val32(4) };
		characters_lists[DRAGON_SHIRYU].push_back(sp_entry);
	}
	
	if (!IsCharacterInList(DRAGON_SHIRYU, val32(2), val32(6)))
	{
		sp_entry = { .cms_entry = val32(2), .cms_model_spec_idx = val32(6) };
		characters_lists[DRAGON_SHIRYU].push_back(sp_entry);
	}
	
	if (!IsCharacterInList(DRAGON_SHIRYU, val32(2), val32(8)))
	{
		sp_entry = { .cms_entry = val32(2), .cms_model_spec_idx = val32(8) };
		characters_lists[DRAGON_SHIRYU].push_back(sp_entry);
	}
	
	if (!IsCharacterInList(LIBRA_SHIRYU, val32(7), val32(1)))
	{
		sp_entry = { .cms_entry = val32(7), .cms_model_spec_idx = val32(1) };
		characters_lists[LIBRA_SHIRYU].push_back(sp_entry);
	}
	
	if (!IsCharacterInList(CYGNUS_HYOGA, val32(3), val32(4)))
	{
		sp_entry = { .cms_entry = val32(3), .cms_model_spec_idx = val32(4) };
		characters_lists[CYGNUS_HYOGA].push_back(sp_entry);
	}
	
	if (!IsCharacterInList(ANDROMEDA_SHUN, val32(4), val32(6)))
	{
		sp_entry = { .cms_entry = val32(4), .cms_model_spec_idx = val32(6) };
		characters_lists[ANDROMEDA_SHUN].push_back(sp_entry);
	}
	
	if (!IsCharacterInList(CAPRICORNUS_SHURA_SAINT, val32(0x1C), val32(1)))
	{
		sp_entry = { .cms_entry = val32(0x1C), .cms_model_spec_idx = val32(1) };
		characters_lists[CAPRICORNUS_SHURA_SAINT].push_back(sp_entry);
	}
	
	if (!IsCharacterInList(BRONZE_ARMOR, val32(1), val32(6)))
	{
		sp_entry = { .cms_entry = val32(1), .cms_model_spec_idx = val32(6) };
		characters_lists[BRONZE_ARMOR].push_back(sp_entry);
	}
	
	// Add another 1, 7...
	sp_entry = { .cms_entry = val32(1), .cms_model_spec_idx = val32(7) };
	characters_lists[BRONZE_ARMOR].push_back(sp_entry);
	
	if (!IsCharacterInList(BRONZE_ARMOR, val32(2), val32(4)))
	{
		sp_entry = { .cms_entry = val32(2), .cms_model_spec_idx = val32(4) };
		characters_lists[BRONZE_ARMOR].push_back(sp_entry);
	}
	
	if (!IsCharacterInList(BRONZE_ARMOR, val32(2), val32(6)))
	{
		sp_entry = { .cms_entry = val32(2), .cms_model_spec_idx = val32(6) };
		characters_lists[BRONZE_ARMOR].push_back(sp_entry);
	}
	
	if (!IsCharacterInList(BRONZE_ARMOR, val32(2), val32(8)))
	{
		sp_entry = { .cms_entry = val32(2), .cms_model_spec_idx = val32(8) };
		characters_lists[BRONZE_ARMOR].push_back(sp_entry);
	}
	
	if (!IsCharacterInList(BRONZE_ARMOR, val32(3), val32(4)))
	{
		sp_entry = { .cms_entry = val32(3), .cms_model_spec_idx = val32(4) };
		characters_lists[BRONZE_ARMOR].push_back(sp_entry);
	}
	
	if (!IsCharacterInList(BRONZE_ARMOR, val32(4), val32(6)))
	{
		sp_entry = { .cms_entry = val32(4), .cms_model_spec_idx = val32(6) };
		characters_lists[BRONZE_ARMOR].push_back(sp_entry);
	}
	
	if (!IsCharacterInList(GOLD_ARMOR, val32(0x1C), val32(1)))
	{
		sp_entry = { .cms_entry = val32(0x1C), .cms_model_spec_idx = val32(1) };
		characters_lists[GOLD_ARMOR].push_back(sp_entry);
	}
	
	if (!IsCharacterInList(SAGITARIUS_SEIYA_GOD, val32(0x5A), 0))
	{
		sp_entry = { .cms_entry = val32(0x5A), .cms_model_spec_idx = 0 };
		characters_lists[SAGITARIUS_SEIYA_GOD].push_back(sp_entry);
	}
	
	if (!IsCharacterInList(BRONZE_GOD_SAINT, val32(0x5A), 0))
	{
		sp_entry = { .cms_entry = val32(0x5A), .cms_model_spec_idx = 0 };
		characters_lists[BRONZE_GOD_SAINT].push_back(sp_entry);
	}
}

#define ADD_IF_NOT_EXISTS() { if (std::find_if(info2_list.begin(), info2_list.end(), CI2Finder(&info2)) == info2_list.end()) {\
									info2_list.push_back(info2); } }

void SlotsFile::AddCharactersInfo2SpecialEntries(std::vector<CharacterInfo2> &info2_list)
{
	CharacterInfo2 info2;
	
	// We will mimic original list, for good or bad.
	
	info2 = { .cms_entry = val32(1), .cms_model_spec_idx = val32(6), .avatar_id = val32(3), .battle_name_id = val32(1), .unk_10 = 0 };	
	ADD_IF_NOT_EXISTS();
	
	info2 = { .cms_entry = val32(1), .cms_model_spec_idx = val32(8), .avatar_id = val32(2), .battle_name_id = val32(1), .unk_10 = 0 };	
	ADD_IF_NOT_EXISTS();
	
	info2 = { .cms_entry = val32(2), .cms_model_spec_idx = val32(3), .avatar_id = val32(7), .battle_name_id = val32(2), .unk_10 = val32(2) };	
	ADD_IF_NOT_EXISTS();
	
	info2 = { .cms_entry = val32(2), .cms_model_spec_idx = val32(4), .avatar_id = val32(7), .battle_name_id = val32(2), .unk_10 = 0 };	
	ADD_IF_NOT_EXISTS();
	
	info2 = { .cms_entry = val32(2), .cms_model_spec_idx = val32(4), .avatar_id = val32(7), .battle_name_id = val32(2), .unk_10 = val32(2) };	
	ADD_IF_NOT_EXISTS();
	
	info2 = { .cms_entry = val32(2), .cms_model_spec_idx = val32(6), .avatar_id = val32(6), .battle_name_id = val32(2), .unk_10 = 0 };	
	ADD_IF_NOT_EXISTS();
	
	info2 = { .cms_entry = val32(2), .cms_model_spec_idx = val32(8), .avatar_id = val32(6), .battle_name_id = val32(2), .unk_10 = 0 };	
	ADD_IF_NOT_EXISTS();
	
	info2 = { .cms_entry = val32(3), .cms_model_spec_idx = val32(4), .avatar_id = val32(0xA), .battle_name_id = val32(3), .unk_10 = 0 };	
	ADD_IF_NOT_EXISTS();
	
	info2 = { .cms_entry = val32(4), .cms_model_spec_idx = val32(6), .avatar_id = val32(0x10), .battle_name_id = val32(4), .unk_10 = 0 };	
	ADD_IF_NOT_EXISTS();
	
	info2 = { .cms_entry = val32(7), .cms_model_spec_idx = val32(0), .avatar_id = val32(0x15), .battle_name_id = val32(2), .unk_10 = val32(2) };	
	ADD_IF_NOT_EXISTS();
	
	info2 = { .cms_entry = val32(7), .cms_model_spec_idx = val32(1), .avatar_id = val32(0x15), .battle_name_id = val32(2), .unk_10 = 0 };	
	ADD_IF_NOT_EXISTS();
	
	info2 = { .cms_entry = val32(7), .cms_model_spec_idx = val32(1), .avatar_id = val32(0x15), .battle_name_id = val32(2), .unk_10 = val32(2) };	
	ADD_IF_NOT_EXISTS();
	
	info2 = { .cms_entry = val32(0x17), .cms_model_spec_idx = val32(0), .avatar_id = val32(0x93), .battle_name_id = val32(0x10), .unk_10 = val32(4) };	
	ADD_IF_NOT_EXISTS();
	
	info2 = { .cms_entry = val32(0x18), .cms_model_spec_idx = val32(0), .avatar_id = val32(0x94), .battle_name_id = val32(0x11), .unk_10 = val32(3) };	
	ADD_IF_NOT_EXISTS();
	
	info2 = { .cms_entry = val32(0x5A), .cms_model_spec_idx = val32(0), .avatar_id = val32(0x8D), .battle_name_id = val32(1), .unk_10 = 0 };	
	ADD_IF_NOT_EXISTS();
}

void SlotsFile::AddCharactersInfo3SpecialEntries()
{
	if (!FindCharacterInfo3(0x5A, 0))
	{
		CharacterInfo3 info3 = { .cms_entry = val32(0x5A), .cms_model_spec_idx = 0, .gwr_name_id = 0.0, .gwr_icon_id = val_float(141.0), .unk_10 = val32(0x7D), .unk_14 = val32(0x102) };
		infos3.push_back(info3);
	}
}

void SlotsFile::AddCharactersInfo4SpecialEntries()
{
	if (!FindCharacterInfo4(0x5A, 0))
	{
		CharacterInfo4 info4 = { .cms_entry = val32(0x5A), .cms_model_spec_idx = 0, .gwt_banner_name_id = 0.0, .gwt_banner_id = val_float(10.0), .unk_10 = val32(0x62), .unk_14 = val32(0xA) };
		infos4.push_back(info4);
	}
}

bool SlotsFile::Compile(TiXmlDocument *doc, bool big_endian)
{
	unsigned int num_slots = 0;
	std::vector<CharacterSlot> slots;
	std::vector<CharacterInfo> infos;
	std::vector<CharacterInfo2> infos2;
	std::vector<bool> initialized;
	
	Reset();
	this->big_endian = big_endian;
	
	TiXmlHandle handle(doc);
	TiXmlElement *root = NULL;;
	
	for (TiXmlElement *elem = handle.FirstChildElement().Element(); elem != NULL; elem = elem->NextSiblingElement())
	{
		std::string name = elem->ValueStr();
		
		if (name == "SLF")
		{
			root = elem;
			break;
		}
	}		
	
	if (!root)
	{
		DPRINTF("Cannot find \"SLF\" in xml.\n");
		return false;
	}
	
	// First pass to know number of character slots, so that we can do later the index check.
	for (TiXmlElement *elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{
		std::string name = elem->ValueStr();
		
		if (name == "CharacterSlot")
		{
			num_slots++;
		}
	}
	
	if (num_slots == 0)
	{
		DPRINTF("No CharacterSlot found!\n");
		return false;
	}
	
	slots.resize(num_slots);
	initialized.resize(num_slots);
	
	for (TiXmlElement *elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{
		std::string name = elem->ValueStr();
		
		if (name == "CharacterSlot")
		{
			uint32_t idx;
			
			if (!Utils::ReadAttrUnsigned(elem, "id", &idx))
			{
				DPRINTF("Cannot read \"id\" attribute.\n");
				return false;
			}
			
			if (idx >= num_slots)
			{
				DPRINTF("id out of bounds (0x%x, number of character slots=0x%x))\nid is a pseudo field (index) that must be between 0 and number of character slots -1.\n", idx, num_slots);
				return false;
			}
			
			if (initialized[idx])
			{
				DPRINTF("Element with id=0x%x already existed. id is a pseudo field(index) that must be unique and between 0 and number of character slots-1.\n", idx);
				return false;
			}
			
			if (!ReadCharacterSlot(elem, &slots[idx], idx, infos, infos2))
				return false;

			initialized[idx] = true;
		}
	}
	
	// Add special entries of characters that don't exist to infos2...
	AddCharactersInfo2SpecialEntries(infos2);
	
	// Add a final 0x6F entry to infos list
	CharacterInfo last_info;
	
	memset(&last_info, 0, sizeof(CharacterInfo));
	last_info.cms_entry = val32(0x6F);		
	infos.push_back(last_info);
	
	// Idem for infos2
	CharacterInfo2 last_info2;
	
	memset(&last_info2, 0, sizeof(CharacterInfo2));
	last_info2.cms_entry = val32(0x6F);		
	infos2.push_back(last_info2);	
	
	this->slots_size = num_slots * sizeof(CharacterSlot);
	this->slots_buf = new uint8_t[this->slots_size];
	if (!this->slots_buf)
	{
		DPRINTF("We are short in memory.\n");
		this->slots_size = 0;		
		return false;
	}
	
	memset(this->slots_buf, 0, this->slots_size);
	
	this->ci_size = infos.size() * sizeof(CharacterInfo);
	this->ci_buf = new uint8_t[this->ci_size];
	if (!this->ci_buf)
	{
		DPRINTF("We are short in memory.\n");
		this->ci_size = 0;
		this->slots_size = 0;
		delete[] this->slots_buf;
		this->slots_buf = NULL;
		
		return false;
	}	
	
	this->ci2_size = infos2.size() * sizeof(CharacterInfo2);
	this->ci2_buf = new uint8_t[this->ci2_size];
	if (!this->ci2_buf)
	{
		DPRINTF("We are short in memory.\n");
		this->slots_size = 0;
		this->ci_size = 0;
		this->ci2_size = 0;
		delete[] this->slots_buf;
		delete[] this->ci_buf;		
		this->slots_buf = this->ci_buf = NULL;
		
		return false;
	}
	
	uint8_t *ptr = this->slots_buf;
	
	for (CharacterSlot &s : slots)
	{
		memcpy(ptr, &s, sizeof(CharacterSlot));
		ptr += sizeof(CharacterSlot);
	}
	
	ptr = this->ci_buf;
	
	for (CharacterInfo &i : infos)
	{
		memcpy(ptr, &i, sizeof(CharacterInfo));
		ptr += sizeof(CharacterInfo);
	}
	
	ptr = this->ci2_buf;
	
	for (CharacterInfo2 &i : infos2)
	{
		memcpy(ptr, &i, sizeof(CharacterInfo2));
		ptr += sizeof(CharacterInfo2);
	}
	
    this->c_slots = (CharacterSlot *)this->slots_buf;
	this->infos = (CharacterInfo *)this->ci_buf;
	this->infos2 = (CharacterInfo2 *)this->ci2_buf;
	
	this->num_slots = this->slots_size / sizeof(CharacterSlot);
	this->num_infos = this->ci_size / sizeof(CharacterInfo);
	this->num_infos2 = this->ci2_size / sizeof(CharacterInfo2);
	
	AddCharactersListsSpecialEntries();	
	
	// Order lists
	for (auto &l : characters_lists)
	{
		std::sort(l.begin(), l.end(), CLComparer(this));
	}	
	
	// Order character info 3
	if (infos3.size() != 0)
	{
		AddCharactersInfo3SpecialEntries();
		std::sort(infos3.begin(), infos3.end(), CI3Comparer(this));
	}
	
	// Order character info 4
	if (infos4.size() != 0)
	{
		AddCharactersInfo4SpecialEntries();
		std::sort(infos4.begin(), infos4.end(), CI4Comparer(this));
	}
	
	return true;
}

bool SlotsFile::CreateFile(SlotsSaveData *save_data)
{
	CHECK_LOADED_BOOL();
	
	save_data->slots_buf = new uint8_t[this->slots_size];
	if (!save_data->slots_buf)
	{
		DPRINTF("%s: Memory allocation error: %x\n", __PRETTY_FUNCTION__, this->slots_size);
		return false;
	}
	
	save_data->ci_buf = new uint8_t[this->ci_size];
	if (!save_data->ci_buf)
	{
		DPRINTF("%s: Memory allocation error: %x\n", __PRETTY_FUNCTION__, this->ci_size);
		delete[] save_data->slots_buf;
		return false;
	}
	
	save_data->ci2_buf = new uint8_t[this->ci2_size];
	if (!save_data->ci2_buf)
	{
		DPRINTF("%s: Memory allocation error: %x\n", __PRETTY_FUNCTION__, this->ci2_size);
		delete[] save_data->slots_buf;
		delete[] save_data->ci_buf;
		return false;
	}
	
	memcpy(save_data->slots_buf, this->slots_buf, this->slots_size);
	memcpy(save_data->ci_buf, this->ci_buf, this->ci_size);
	memcpy(save_data->ci2_buf, this->ci2_buf, this->ci2_size);
	
	save_data->slots_size = this->slots_size;
	save_data->ci_size = this->ci_size;
	save_data->ci2_size = this->ci2_size;
	
	save_data->ci3_buf = CreateCI3(&save_data->ci3_size);
	if (!save_data->ci3_buf)
	{
		delete[] save_data->slots_buf;
		delete[] save_data->ci_buf;
		delete[] save_data->ci2_buf;
		return false;
	}
	
	save_data->ci4_buf = CreateCI4(&save_data->ci4_size);
	if (!save_data->ci4_buf)
	{
		delete[] save_data->slots_buf;
		delete[] save_data->ci_buf;
		delete[] save_data->ci2_buf;
		delete[] save_data->ci3_buf;
		return false;
	}
	
	return true;
}

void SlotsFile::CreateCompatCI3Entries()
{
	for (uint32_t i = 0; i < num_slots; i++)
	{
		for (uint32_t j = 0; j < MAX_SLOTS; j++)
		{
			Slot *slot = &c_slots[i].sub_slots[j];
			
			if (!IsFreeSlot(i, j) && !IsCurrentRandomSlot(i))
			{
				if (std::find_if(infos3.begin(), infos3.end(), CI3Finder(slot->cms_entry, slot->cms_model_spec_idx)) == infos3.end())
				{
					bool found = false;
					
					for (unsigned int k = 0; k < 10; k++)
					{
						std::vector<CharacterInfo3>::iterator it = std::find_if(infos3.begin(), infos3.end(), CI3Finder(slot->cms_entry, k));
						
						if (it != infos3.end())
						{
							CharacterInfo3 ci3 = CharacterInfo3(*it);
							
							ci3.cms_model_spec_idx = slot->cms_model_spec_idx;
							infos3.push_back(ci3);
							found = true;
							//DPRINTF("Information3 of character 0x%x %d copied from character 0x%x %d\n", slot->cms_entry, slot->cms_model_spec_idx, slot->cms_entry, k);
							break;
						}
					}
					
					if (!found)
					{
						CharacterInfo3 ci3 = CharacterInfo3(infos3[0]);
						
						ci3.cms_entry = slot->cms_entry;
						ci3.cms_model_spec_idx = slot->cms_model_spec_idx;
						
						//DPRINTF("Information of character 0x%x %d copied from first character.\n", slot->cms_entry, slot->cms_model_spec_idx);
						infos3.push_back(ci3);
					}
				}				
			}
		}
	}
}

uint8_t *SlotsFile::CreateCI3(unsigned int *psize)
{
	if (infos3.size() == 0)
		return nullptr;
	
	CreateCompatCI3Entries();
	
	*psize = infos3.size() * sizeof(CharacterInfo3);
	uint8_t *buf = new uint8_t[*psize];	
	if (!buf)
	{
		DPRINTF("%s: Memory allocation error 0x%x", __PRETTY_FUNCTION__, *psize);
		return nullptr;
	}
	
	uint8_t *ptr = buf;
	
	for (CharacterInfo3 &info3 : infos3)
	{
		memcpy(ptr, &info3, sizeof(CharacterInfo3));
		ptr += sizeof(CharacterInfo3);
	}
	
	return buf;
}

void SlotsFile::CreateCompatCI4Entries()
{
	for (uint32_t i = 0; i < num_slots; i++)
	{
		for (uint32_t j = 0; j < MAX_SLOTS; j++)
		{
			Slot *slot = &c_slots[i].sub_slots[j];
			
			if (!IsFreeSlot(i, j) && !IsCurrentRandomSlot(i))
			{
				if (std::find_if(infos4.begin(), infos4.end(), CI4Finder(slot->cms_entry, slot->cms_model_spec_idx)) == infos4.end())
				{
					bool found = false;
					
					for (unsigned int k = 0; k < 10; k++)
					{
						std::vector<CharacterInfo4>::iterator it = std::find_if(infos4.begin(), infos4.end(), CI4Finder(slot->cms_entry, k));
						
						if (it != infos4.end())
						{
							CharacterInfo4 ci4 = CharacterInfo4(*it);
							
							ci4.cms_model_spec_idx = slot->cms_model_spec_idx;
							infos4.push_back(ci4);
							found = true;
							//DPRINTF("Information4 of character 0x%x %d copied from character 0x%x %d\n", slot->cms_entry, slot->cms_model_spec_idx, slot->cms_entry, k);
							break;
						}
					}
					
					if (!found)
					{
						CharacterInfo4 ci4 = CharacterInfo4(infos4[0]);
						
						ci4.cms_entry = slot->cms_entry;
						ci4.cms_model_spec_idx = slot->cms_model_spec_idx;
						
						//DPRINTF("Information4 of character 0x%x %d copied from first character.\n", slot->cms_entry, slot->cms_model_spec_idx);
						infos4.push_back(ci4);
					}
				}				
			}
		}
	}
}

uint8_t *SlotsFile::CreateCI4(unsigned int *psize)
{
	if (infos4.size() == 0)
		return nullptr;
	
	CreateCompatCI4Entries();
	
	*psize = (infos4.size()+1) * sizeof(CharacterInfo4);
	uint8_t *buf = new uint8_t[*psize];	
	if (!buf)
	{
		DPRINTF("%s: Memory allocation error 0x%x", __PRETTY_FUNCTION__, *psize);
		return nullptr;
	}
	
	uint8_t *ptr = buf;
	
	for (const CharacterInfo4 &info4 : infos4)
	{		
		memcpy(ptr, &info4, sizeof(CharacterInfo4));
		ptr += sizeof(CharacterInfo4);
	}
	
	CharacterInfo4 *last = (CharacterInfo4 *)ptr;
	
	memset(last, 0, sizeof(CharacterInfo4));
	last->cms_entry = 0x6F;	
	
	return buf;
}

#define ADD_ENTRY(le) { memcpy(ptr, &le, sizeof(CharacterDef)); ptr += sizeof(CharacterDef); }
#define ADD_EMPTY_LIST() ADD_ENTRY(empty_list)

uint8_t *SlotsFile::CreateLists(unsigned int *size, uint8_t **ptr_lists)
{
	CHECK_LOADED_PTR();
	
	unsigned int alloc_size = 0;
	CharacterDef empty_list;
	
	empty_list.cms_entry = 0x6F;
	empty_list.cms_model_spec_idx = 0;
	
	for (auto &l : characters_lists)
	{
		alloc_size += (l.size()+1) * sizeof(CharacterDef);
	}
	
	//DPRINTF("alloc_size = %x.\n", alloc_size);
	
	uint8_t *buf = new uint8_t[alloc_size];
	if (!buf)
	{
		DPRINTF("Memory allocation error.\n");
		return NULL;
	}
	
	uint8_t *ptr = buf;
	
	for (size_t i = 0; i <= GEMINI_SAGA_SAINT; i++)
	{
		ptr_lists[i] = ptr;
		
		for (CharacterDef &e : characters_lists[i])
		{
			ADD_ENTRY(e);
		}
		
		ADD_EMPTY_LIST();
	}
	
	// Special case EVIL_SAGA...
	ptr_lists[GEMINI_EVIL_SAGA_SAINT] = ptr;
	
	for (CharacterDef &e: characters_lists[GEMINI_EVIL_SAGA_SAINT])
	{
		ADD_ENTRY(e);
	}
	
	ADD_EMPTY_LIST();
	
	for (size_t i = GEMINI_SAGA_SPECTRE; i < EMPTY1; i++)
	{
		if (i == GEMINI_EVIL_SAGA_SAINT)
			continue;
		
		ptr_lists[i] = ptr;
		
		for (CharacterDef &e : characters_lists[i])
		{
			ADD_ENTRY(e);
		}
		
		ADD_EMPTY_LIST();
	}
	
	*size = alloc_size;
	return buf;
}

bool SlotsFile::CompareCharacterInfo(SlotsFile *other)
{
	CHECK_LOADED_BOOL();
	
	if (num_slots != other->num_slots)
	{
		DPRINTF("Mismatch on number of slots.\n");
		return false;
	}
	
	if (num_infos != other->num_infos)
	{
		DPRINTF("Mismatch on number of infos.\n");
		return false;
	}
	
	for (uint32_t i = 0; i < num_slots; i++)
	{
		if (!IsCurrentRandomSlot(i))
		{
			for (uint32_t j = 0; j < MAX_SLOTS; j++)
			{
				CharacterInfo *info1, *info2;
				
                info1 = FindCharacterInfo(&c_slots[i].sub_slots[j]);
				if (!info1)
				{
                    DPRINTF("Cannnot find character 0x%x 0x%x in \"this\".\n", val32(c_slots[i].sub_slots[j].cms_entry), val32(c_slots[i].sub_slots[j].cms_model_spec_idx));
					return false;
				}
				
                info2 = other->FindCharacterInfo(&c_slots[i].sub_slots[j]); // slots[i].slots[j] must be of this!!!
				if (!info2)
				{
                    DPRINTF("Cannnot find character 0x%x 0x%x in \"other\".\n", val32(c_slots[i].sub_slots[j].cms_entry), val32(c_slots[i].sub_slots[j].cms_model_spec_idx));
					return false;
				}
				
				if (memcmp(info1, info2, sizeof(CharacterInfo)) != 0)
				{
                    DPRINTF("Mismatch on character info for 0x%x 0x%x\n", val32(c_slots[i].sub_slots[j].cms_entry), val32(c_slots[i].sub_slots[j].cms_model_spec_idx));
					return false;
				}
			}
		}
	}
	
	return true;
}

bool SlotsFile::CompareLists(SlotsFile *other)
{
	CHECK_LOADED_BOOL();
	
	for (size_t i = 0; i < characters_lists.size(); i++)
	{
		if (characters_lists[i].size() != other->characters_lists[i].size())
		{
			DPRINTF("List \"%s\" has different number of elements.\n", lists_names[i].c_str());
			return false;
		}
		
		for (size_t j = 0; j < characters_lists[i].size(); j++)
		{
			uint32_t cms_entry;
			uint32_t cms_model_spec_idx;
			
			cms_entry = characters_lists[i][j].cms_entry;
			cms_model_spec_idx = characters_lists[i][j].cms_model_spec_idx;
			
			if (!other->IsCharacterInList(i, cms_entry, cms_model_spec_idx))
			{
				DPRINTF("\"other\" doesn't contain element 0x%x 0x%x\n", cms_entry, cms_model_spec_idx);
				return false;
			}
			
			cms_entry = other->characters_lists[i][j].cms_entry;
			cms_model_spec_idx = other->characters_lists[i][j].cms_model_spec_idx;
			
			if (!IsCharacterInList(i, cms_entry, cms_model_spec_idx))
			{
				DPRINTF("\"this\" doesn't contain element 0x%x 0x%x\n", cms_entry, cms_model_spec_idx);
				return false;
			}
		}
	}
	
	return true;
}

bool SlotsFile::CheckModels(CmsFile *cms)
{
	for (uint32_t i = 0; i < num_slots; i++)
	{
		for (uint32_t j = 0; j < MAX_SLOTS; j++)
		{
            uint32_t entry = val32(c_slots[i].sub_slots[j].cms_entry);
            uint32_t idx = val32(c_slots[i].sub_slots[j].cms_model_spec_idx);
			
			if (entry != 0x6F)
			{
				if (!cms->ModelExists(entry, idx))
				{
					DPRINTF("Model 0x%x %d doesn't exist in the .cms.\n", entry, idx);
					return false;
				}
			}
		}
	}
	
	return true;
}

bool SlotsFile::CompareCharacterInfo2(SlotsFile *other)
{
	CHECK_LOADED_BOOL();
	
	if (num_infos2 != other->num_infos2)
	{
		DPRINTF("Mismatch on number of infos2 (0x%x != 0x%x)\n", num_infos2, other->num_infos2);
		return false;
	}
	
	for (uint32_t i = 0; i < num_infos2; i++)
	{
		if (!other->IdenticalInfo2Exists(infos2+i))
		{
			DPRINTF("0x%x %d doesn't exist or is not similar in \"other\"\n", infos2[i].cms_entry, infos2[i].cms_model_spec_idx);
			return false;
		}
		
		if (!IdenticalInfo2Exists(other->infos2+i))
		{
			DPRINTF("0x%x %d doesn't exist or is not similar in \"this\"\n", other->infos2[i].cms_entry, other->infos2[i].cms_model_spec_idx);
			return false;
		}
	}
	
	return true;
}

bool SlotsFile::CompareCharacterInfo4(SlotsFile *other)
{
	CHECK_LOADED_BOOL();
	
	if (infos4.size() != other->infos4.size())
	{
		DPRINTF("Mismatch on number of infos4 (0x%x != 0x%x)\n", infos4.size(), other->infos4.size());
		return false;
	}
	
	for (const CharacterInfo4 &info4 : infos4)
	{
		CharacterInfo4 *info4_other = other->FindCharacterInfo4(info4.cms_entry, info4.cms_model_spec_idx);
		
		if (!info4_other)
			return false;
		
		if (memcmp(&info4, info4_other, sizeof(CharacterInfo4)) != 0)
			return false;
	}
	
	for (const CharacterInfo4 &info4_other : other->infos4)
	{
		CharacterInfo4 *info4 = FindCharacterInfo4(info4_other.cms_entry, info4_other.cms_model_spec_idx);
		
		if (!info4)
			return false;
		
		if (memcmp(info4, &info4_other, sizeof(CharacterInfo4)) != 0)
			return false;
	}
	
	return true;
}
