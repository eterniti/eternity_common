#include "Xv2StageDefFile.h"
#include "debug.h"

#ifndef XV2_STAGEDEF_SIMPLE
#include "Xenoverse2.h"
#endif

const std::vector<std::string> xv2_native_lang_codes =
{
    "jp",
    "en",
    "fr",
    "it",
    "de",
    "es",
    "ca",
    "pt",
    "kr",
    "tw",
    "zh",
    "pl",
    "ru"
};

TiXmlElement *Xv2StageGate::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Gate");

    Utils::WriteParamString(entry_root, "NAME", name);
    Utils::WriteParamUnsigned(entry_root, "TARGET_STAGE_IDX", target_stage_idx, (target_stage_idx==0xFFFFFFFF));
    Utils::WriteParamUnsigned(entry_root, "U_0C", unk_0C, true);
    Utils::WriteParamUnsigned(entry_root, "U_10", unk_10, true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool Xv2StageGate::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamString(root, "NAME", name))
        return false;

    if (!Utils::GetParamUnsigned(root, "TARGET_STAGE_IDX", &target_stage_idx))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0C", &unk_0C))
        return true;

    if (!Utils::GetParamUnsigned(root, "U_10", &unk_10))
        return false;

    return true;
}

#ifndef XV2_STAGEDEF_SIMPLE
std::string Xv2Stage::GetName(int lang) const
{
    if (lang < 0 || lang >= XV2_LANG_NUM)
        return "";

    switch (lang)
    {
        case XV2_LANG_ENGLISH:
            return name[XV2_NATIVE_LANG_ENGLISH];

        case XV2_LANG_SPANISH1:
            return name[XV2_NATIVE_LANG_SPANISH1];

        case XV2_LANG_SPANISH2:
            return name[XV2_NATIVE_LANG_SPANISH2];

        case XV2_LANG_FRENCH:
            return name[XV2_NATIVE_LANG_FRENCH];

        case XV2_LANG_GERMAN:
            return name[XV2_NATIVE_LANG_GERMAN];

        case XV2_LANG_ITALIAN:
            return name[XV2_NATIVE_LANG_ITALIAN];

        case XV2_LANG_PORTUGUESE:
            return name[XV2_NATIVE_LANG_PORTUGUESE];

        case XV2_LANG_POLISH:
            return name[XV2_NATIVE_LANG_POLISH];

        case XV2_LANG_RUSSIAN:
            return name[XV2_NATIVE_LANG_RUSSIAN];

        case XV2_LANG_CHINESE1:
            return name[XV2_NATIVE_LANG_CHINESE1];

        case XV2_LANG_CHINESE2:
            return name[XV2_NATIVE_LANG_CHINESE2];

        case XV2_LANG_KOREAN:
            return name[XV2_NATIVE_LANG_KOREAN];
    }

    return "";
}

void Xv2Stage::SetName(const std::string &stage_name, int lang)
{
    if (lang < 0 || lang >= XV2_LANG_NUM)
        return;

    switch (lang)
    {
        case XV2_LANG_ENGLISH:
            name[XV2_NATIVE_LANG_ENGLISH] = stage_name;
        break;

        case XV2_LANG_SPANISH1:
            name[XV2_NATIVE_LANG_SPANISH1] = stage_name;
        break;

        case XV2_LANG_SPANISH2:
            name[XV2_NATIVE_LANG_SPANISH2] = stage_name;
        break;

        case XV2_LANG_FRENCH:
            name[XV2_NATIVE_LANG_FRENCH] = stage_name;
        break;

        case XV2_LANG_GERMAN:
            name[XV2_NATIVE_LANG_GERMAN] = stage_name;
        break;

        case XV2_LANG_ITALIAN:
            name[XV2_NATIVE_LANG_ITALIAN] = stage_name;
        break;

        case XV2_LANG_PORTUGUESE:
            name[XV2_NATIVE_LANG_PORTUGUESE] = stage_name;
        break;

        case XV2_LANG_POLISH:
            name[XV2_NATIVE_LANG_POLISH] = stage_name;
        break;

        case XV2_LANG_RUSSIAN:
            name[XV2_NATIVE_LANG_RUSSIAN] = stage_name;
        break;

        case XV2_LANG_CHINESE1:
            name[XV2_NATIVE_LANG_CHINESE1] = stage_name;
        break;

        case XV2_LANG_CHINESE2:
            name[XV2_NATIVE_LANG_CHINESE2] = stage_name;
        break;

        case XV2_LANG_KOREAN:
            name[XV2_NATIVE_LANG_KOREAN] = stage_name;
        break;
    }
}
#else
std::string Xv2Stage::GetName(int) const
{
    return "";
}

void Xv2Stage::SetName(const std::string &, int)
{

}
#endif

TiXmlElement *Xv2Stage::Decompile(TiXmlNode *root, uint32_t idx) const
{
    TiXmlElement *entry_root = new TiXmlElement("Stage");
    entry_root->SetAttribute("idx", idx);

    if (ssid != 0xFF)
        entry_root->SetAttribute("ssid", ssid);

#ifndef XV2_STAGEDEF_SIMPLE
    std::string name;

    if (Xenoverse2::GetStageName(code, name) && name.length() > 0)
    {
        Utils::WriteComment(entry_root, name);
    }
#endif

    Utils::WriteParamString(entry_root, "BASE_DIR", base_dir);
    Utils::WriteParamString(entry_root, "CODE", code);
    Utils::WriteParamString(entry_root, "DIR", dir);
    Utils::WriteParamString(entry_root, "STR4", str4);
    Utils::WriteParamString(entry_root, "EVE", eve);
    Utils::WriteParamUnsigned(entry_root, "UNK5", unk5);

    // Atm, let's keep old name for compatibility
    Utils::WriteParamFloat(entry_root, "F6", ki_blast_size_limit);
    //Utils::WriteParamFloat(entry_root, "KI_BLAST_SIZE_LIMIT", ki_blast_size_limit);

    Utils::WriteParamString(entry_root, "SE", se);
    Utils::WriteParamUnsigned(entry_root, "BGM_CUE_ID", bgm_cue_id);

    if (idx >= XV2_ORIGINAL_NUM_STAGES && ssid != 0xFF && ssid >= XV2_ORIGINAL_NUM_SS_STAGES)
    {
        for (int i = 0; i < XV2_NATIVE_LANG_NUM; i++)
        {
            if (this->name[i].length() == 0)
                continue;

            const std::string param_name = "NAME_" + Utils::ToUpperCase(xv2_native_lang_codes[i]);
            Utils::WriteParamString(entry_root, param_name.c_str(), this->name[i]);
        }
    }

    if (override_far_clip != 0.0f)
    {
        Utils::WriteParamFloat(entry_root, "OVERRIDE_FAR_CLIP", override_far_clip);
    }

    Utils::WriteParamFloat(entry_root, "LIMIT", limit);

    for (size_t i = 0; i < XV2_STA_NUM_GATES; i++)
        gates[i].Decompile(entry_root);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool Xv2Stage::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "ssid", &ssid))
        ssid = 0xFF;

    if (!Utils::GetParamString(root, "BASE_DIR", base_dir))
        return false;

    if (!Utils::GetParamString(root, "CODE", code))
        return false;

    if (!Utils::GetParamString(root, "DIR", dir))
        return false;

    if (!Utils::GetParamString(root, "STR4", str4))
        return false;

    if (!Utils::GetParamString(root, "EVE", eve))
        return false;

    if (!Utils::ReadParamUnsigned(root, "UNK5", &unk5))
        unk5 = 0;

    if (!Utils::GetParamFloatWithMultipleNames(root, &ki_blast_size_limit, "KI_BLAST_SIZE_LIMIT", "F6"))
        return false;

    if (!Utils::GetParamString(root, "SE", se))
        return false;

    if (!Utils::GetParamUnsigned(root, "BGM_CUE_ID", &bgm_cue_id))
        return false;

    for (int i = 0; i < XV2_NATIVE_LANG_NUM; i++)
    {
        const std::string param_name = "NAME_" + Utils::ToUpperCase(xv2_native_lang_codes[i]);
        Utils::ReadParamString(root, param_name.c_str(), name[i]);
    }

    if (!Utils::ReadParamFloat(root, "OVERRIDE_FAR_CLIP", &override_far_clip))
        override_far_clip = 0.0f;

    if (!Utils::ReadParamFloat(root, "LIMIT", &limit))
        limit = 500.0f;

    size_t idx = 0;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Gate")
        {
            if (idx >= XV2_STA_NUM_GATES)
            {
                DPRINTF("%s: Too many gates in stage %s\n", FUNCNAME, code.c_str());
                return false;
            }

            if (!gates[idx++].Compile(elem))
                return false;
        }
    }

    // commented because of the change of number of gates in 1.11. If there are less gates, the other ones will just be initialized dummy.
    /*if (idx != XV2_STA_NUM_GATES)
    {
        DPRINTF("%s: There must be exactly %d gates.\n", FUNCNAME, XV2_STA_NUM_GATES);
        return false;
    }*/

    return true;
}

Xv2StageDefFile::Xv2StageDefFile()
{
    this->big_endian = false;
}

Xv2StageDefFile::~Xv2StageDefFile()
{
    this->big_endian = false;
}

void Xv2StageDefFile::Reset()
{
    stages.clear();
    ssids_set.clear();
}

TiXmlDocument *Xv2StageDefFile::Decompile() const
{
    if (stages.size() > XV2_MAX_STAGES)
    {
        DPRINTF("%s: Cannot save because file bypassed max number of stages.\n", FUNCNAME);
        return nullptr;
    }

    TiXmlDocument *doc = new TiXmlDocument();
    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("Xv2StageDef");

    for (size_t i = 0; i < stages.size(); i++)
        stages[i].Decompile(root, (int)i);

    doc->LinkEndChild(root);
    return doc;
}

bool Xv2StageDefFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "Xv2StageDef");

    if (!root)
    {
        DPRINTF("Cannot find\"Xv2StageDef\" in xml.\n");
        return false;
    }

    size_t n = Utils::GetElemCount(root, "Stage");

    if (n > XV2_MAX_STAGES)
    {
        DPRINTF("%s: Error, number of stages is bigger than maximum.\n", FUNCNAME);
        return false;
    }

    stages.resize(n);

    std::vector<bool> used;
    used.resize(n, false);

    uint8_t max_ssid = 0;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Stage")
        {
            Xv2Stage stage;
            uint32_t idx;

            if (!stage.Compile(elem))
                return false;

            if (!Utils::ReadAttrUnsigned(elem, "idx", &idx))
            {
                DPRINTF("%s: Attribute idx is not optional.\n", FUNCNAME);
                return false;
            }

            if (idx >= n)
            {
                DPRINTF("%s: Index %d out of bounds.\n", FUNCNAME, idx);
                return false;
            }

            if (used[idx])
            {
                DPRINTF("%s: index %d is used more than once.\n", FUNCNAME, idx);
                return false;
            }

            if (stage.ssid != 0xFF)
            {
                if (ssids_set.find(stage.ssid) != ssids_set.end())
                {
                    DPRINTF("%s: Error, found duplicate ssid %d\n", FUNCNAME, stage.ssid);
                    return false;
                }

                ssids_set.insert(stage.ssid);

                if (stage.ssid > max_ssid)
                    max_ssid = stage.ssid;
            }

            if (stage.ssid == 0xFF || stage.ssid < XV2_ORIGINAL_NUM_SS_STAGES || idx < XV2_ORIGINAL_NUM_STAGES)
            {
                for (std::string &s : stage.name)
                    s.clear();
            }

            used[idx] = true;
            stages[idx] = stage;
        }
    }

    if (max_ssid != (ssids_set.size()-1))
    {
        DPRINTF("%s Ssids must use a consecutive range.\n", FUNCNAME);
        return false;
    }

    return true;
}

static void FixPointers(size_t base_addr, const void *base_ptr, void *pointers, size_t num)
{
    uint64_t *pointers_64 = (uint64_t *)pointers;

    for (size_t i = 0; i < num; i++)
    {
        if (pointers_64[i])
        {
            pointers_64[i] -= base_addr;
            pointers_64[i] += (uint64_t)base_ptr;
        }
    }
}

bool Xv2StageDefFile::LoadFromDump(size_t stage_num, size_t base_addr, const void *base_ptr, void *def1_buf, size_t playable_stage_num, const void *ssid_buf, const void *f6_buf, const void *def2_buf, const void *sounds_buf, const void *music_buf, const char **eve_dump)
{
    Reset();
    stages.resize(stage_num);

    const XV2StageDef1 *defs1 = (XV2StageDef1 *)def1_buf;

    for (size_t i = 0; i < stage_num; i++)
    {
        Xv2Stage &stage = stages[i];

        FixPointers(base_addr, base_ptr, (void *)&defs1[i], 4);

        stage.base_dir = defs1[i].base_dir;
        stage.code = defs1[i].code;
        stage.dir = defs1[i].dir;
        stage.str4 = defs1[i].str4;
        stage.eve = eve_dump[i];
        stage.unk5 = defs1[i].unk5;
    }

    const uint32_t *ssid_to_idx = (const uint32_t *)ssid_buf;

    for (size_t i = 0; i < playable_stage_num; i++)
    {
        uint32_t idx = ssid_to_idx[i];

        if (idx >= stages.size())
        {
            DPRINTF("%s: Index out of bounds (%d)\n", FUNCNAME, idx);
            return false;
        }

        stages[idx].ssid = (uint8_t)i;
    }

    const float *f6s = (const float *)f6_buf;

    for (size_t i = 0; i < stage_num; i++)
    {
        stages[i].ki_blast_size_limit = f6s[i];
    }    

    const XV2StageDef2 *defs2 = (const XV2StageDef2 *)def2_buf;

    for (size_t i = 0; i < stage_num; i++)
    {
        const XV2StageGate *gates = defs2[i].gates;
        Xv2Stage &stage = stages[i];

        for (int j = 0; j < XV2_STA_NUM_GATES; j++)
        {
            FixPointers(base_addr, base_ptr, (void *)&gates[j].name, 1);
            stage.gates[j].name = (gates[j].name) ? gates[j].name : "";
            stage.gates[j].target_stage_idx = gates[j].target_stage_idx;
            stage.gates[j].unk_0C = gates[j].unk_0C;
            stage.gates[j].unk_10 = gates[j].unk_10;
        }
    }

    const char **sounds = (const char **)sounds_buf;

    for (size_t i = 0; i < stage_num; i++)
    {
        if (sounds[i])
        {
            Xv2Stage &stage = stages[i];

            FixPointers(base_addr, base_ptr, (void *)&sounds[i], 1);
            stage.se = sounds[i];
        }
    }

    const uint32_t *music = (const uint32_t *)music_buf;

    for (size_t i = 0; i < stage_num; i++)
    {
        Xv2Stage &stage = stages[i];

        stage.bgm_cue_id = music[i];
    }

    return true;
}

static bool InsertToMap(std::map<std::string, size_t> &map, const std::string &str, size_t &pos)
{
    if (map.find(str) != map.end())
        return false;

    map[str] = pos;
    pos += str.length() + 1;
    return true;
}

size_t Xv2StageDefFile::BuildStrings(std::map<std::string, size_t> &map) const
{
    size_t pos = 0;

    for (const Xv2Stage &stage : stages)
    {
        InsertToMap(map, stage.base_dir, pos);
        InsertToMap(map, stage.code, pos);
        InsertToMap(map, stage.dir, pos);
        InsertToMap(map, stage.str4, pos);
        InsertToMap(map, stage.eve, pos);
        InsertToMap(map, stage.se, pos);

        for (size_t i = 0; i < XV2_STA_NUM_GATES; i++)
        {
            InsertToMap(map, stage.gates[i].name, pos);
        }
    }

    return pos;
}

void Xv2StageDefFile::BuildSsidMap(void *addr) const
{
    uint32_t *ssid_to_idx = (uint32_t *)addr;

    for (size_t i = 0; i < stages.size(); i++)
    {
        const Xv2Stage &stage = stages[i];

        if (stage.ssid != 0xFF)
        {
            ssid_to_idx[stage.ssid] = (uint32_t)i;
        }
    }
}

Xv2Stage *Xv2StageDefFile::GetStageBySsid(uint8_t ssid)
{
    for (Xv2Stage &stage : stages)
    {
        if (stage.ssid == ssid)
            return &stage;
    }

    return nullptr;
}

Xv2Stage *Xv2StageDefFile::GetStageByCode(const std::string &code, size_t *idx)
{
    for (size_t i = 0; i < stages.size(); i++)
    {
        Xv2Stage &stage = stages[i];

        if (stage.code == code)
        {
            if (idx)
                *idx = i;

            return &stage;
        }
    }

    return nullptr;
}

size_t Xv2StageDefFile::AddStage(Xv2Stage &stage, bool add_ssid, bool overwrite)
{
    size_t id;
    Xv2Stage *existing_stage = GetStageByCode(stage.code, &id);

    if (existing_stage)
    {
        if (!overwrite)
            return (size_t)-1;

        uint8_t ssid = existing_stage->ssid;

        if (ssid == 0xFF && add_ssid)
        {
            if (GetNumSsStages() >= XV2_MAX_STAGES)
            {
                DPRINTF("%s: Overflow of ssid.\n", FUNCNAME);
                return (size_t)-1;
            }

            ssid = (uint8_t)GetNumSsStages();
            ssids_set.insert(ssid);
        }

        stage.ssid = ssid;
        *existing_stage = stage;
        return id;
    }

    for (id = 0; id < GetNumStages(); id++)
    {
        Xv2Stage &this_stage = stages[id];

        if (Utils::BeginsWith(this_stage.code, "X2M_FREE", false))
        {
            uint8_t ssid = this_stage.ssid;

            if (ssid == 0xFF && add_ssid)
            {
                if (GetNumSsStages() >= XV2_MAX_STAGES)
                {
                    DPRINTF("%s: Overflow of ssid.\n", FUNCNAME);
                    return (size_t)-1;
                }

                ssid = (uint8_t)GetNumSsStages();
                ssids_set.insert(ssid);
            }

            stage.ssid = ssid;
            this_stage = stage;
            return id;
        }
    }

    if (GetNumStages() >= XV2_MAX_STAGES)
    {
        DPRINTF("%s: Overflow of stages.\n", FUNCNAME);
        return (size_t)-1;
    }

    if (add_ssid)
    {
        if (GetNumSsStages() >= XV2_MAX_STAGES)
        {
            DPRINTF("%s: Overflow of ssid.\n", FUNCNAME);
            return (size_t)-1;
        }

        stage.ssid = (uint8_t)GetNumSsStages();
        ssids_set.insert(stage.ssid);
    }
    else
    {
        stage.ssid = 0xFF;
    }

    stages.push_back(stage);
    return (stages.size()-1);
}

void Xv2StageDefFile::RemoveStage(Xv2Stage &stage)
{
    size_t id = (size_t)-1;

    for (size_t i = 0; i < stages.size(); i++)
    {
        if (stages[i].code == stage.code)
        {
            id = i;
            break;
        }
    }

    if (id == (size_t)-1)
    {
        stage.ssid = 0xFF;
        return;
    }

    stage.ssid = stages[id].ssid;

    if (id == (stages.size()-1) && stage.ssid == (GetNumSsStages()-1))
    {
        // Can fully remove it

        if (stage.ssid != 0xFF)
        {
            ssids_set.erase(stage.ssid);
        }

        stages.erase(stages.begin()+id);
        return;
    }

    // Pseudo remove it by marking the entry as free
    std::string name;
    uint32_t i = 0;

    do
    {
        name = "X2M_FREE" + Utils::ToString(i);
        i++;
    } while (GetStageByCode(name));

    Xv2Stage free_entry;

    free_entry.code = free_entry.dir = free_entry.str4 = free_entry.eve = name;
    free_entry.ki_blast_size_limit = 123.456f;
    free_entry.ssid = stage.ssid;
    free_entry.se = "SE/Battle/Field/CAR_BTL_BKG_pln";

    stages[id] = free_entry;
}




