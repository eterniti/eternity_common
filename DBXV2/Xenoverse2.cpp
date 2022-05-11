#include <algorithm>
#include <unordered_set>

#include "Xenoverse2.h"
#include "SwfFile.h"
#include "xv2stagedef_default.inc"
#include "xv2_default_stage_slots.inc"
#include "debug.h"

#undef ReplaceFile

//#define CHARALIST_PATH  "/action_script/CharaList.as"
#define TIMELINE_PATH   "/CHARASELE_fla/MainTimeline.as"
#define COMPRESULT_PATH "/CHARASELE_fla/MainTimeline.swf"

#define GAME_CHARASELE_PATH     "data/ui/iggy/CHARASELE.iggy"
#define GAME_SELPORT_PATH       "data/ui/texture/CHARA01.emb"
#define GAME_CHARANAMES_PATH    "data/msg/proper_noun_character_name_"
#define GAME_CHARA_COSTUMENAMES_PATH  "data/msg/proper_noun_variation_name_"

#define GAME_SUPER_SKILLS_NAME_PATH      "data/msg/proper_noun_skill_spa_name_"
#define GAME_ULTIMATE_SKILLS_NAME_PATH   "data/msg/proper_noun_skill_ult_name_"
#define GAME_EVASIVE_SKILLS_NAME_PATH   "data/msg/proper_noun_skill_esc_name_"
#define GAME_AWAKEN_SKILLS_NAME_PATH    "data/msg/proper_noun_skill_met_name_"

#define GAME_SUPER_SKILLS_DESC_PATH      "data/msg/proper_noun_skill_spa_info_"
#define GAME_ULTIMATE_SKILLS_DESC_PATH   "data/msg/proper_noun_skill_ult_info_"
#define GAME_EVASIVE_SKILLS_DESC_PATH   "data/msg/proper_noun_skill_esc_info_"
#define GAME_AWAKEN_SKILLS_DESC_PATH    "data/msg/proper_noun_skill_met_info_"

#define GAME_QUEST_BTLHUD_TEXT_PATH    "data/msg/quest_btlhud_"

#define GAME_CMS_PATH   "data/system/char_model_spec.cms"
#define GAME_CUS_PATH   "data/system/custom_skill.cus"
#define GAME_CSO_PATH   "data/system/chara_sound.cso"
#define GAME_PSC_PATH   "data/system/parameter_spec_char.psc"
#define GAME_AUR_PATH   "data/system/aura_setting.aur"
#define GAME_SEV_PATH   "data/system/special_event_voice.sev"
#define GAME_CML_PATH   "data/system/CameraLimitValue.cml"
#define GAME_HCI_PATH   "data/ui/CharaImage/chara_image.hci"
#define GAME_HCI2_PATH  "data/ui/CharaImage/chara_image_day1.hci"
#define GAME_HCI3_PATH  "data/ui/CharaImage/chara_image_TU1.hci"
#define GAME_HCI4_PATH  "data/ui/CharaImage/chara_image_TU2.hci"
#define GAME_HCI5_PATH  "data/ui/CharaImage/chara_image_TU3.hci"
#define GAME_HCI6_PATH  "data/ui/CharaImage/chara_image_TU4.hci"
#define GAME_HCI7_PATH  "data/ui/CharaImage/chara_image_TU4_5.hci"
#define GAME_HCI8_PATH  "data/ui/CharaImage/chara_image_TU5.hci"
#define GAME_HCI9_PATH  "data/ui/CharaImage/chara_image_TU6.hci"
#define GAME_HCI10_PATH  "data/ui/CharaImage/chara_image_TU7.hci"
#define GAME_HCI11_PATH  "data/ui/CharaImage/chara_image_TU8.hci"
#define GAME_HCI12_PATH  "data/ui/CharaImage/chara_image_TU9.hci"
#define GAME_HCI13_PATH  "data/ui/CharaImage/chara_image_TU10.hci"
#define GAME_HCI14_PATH  "data/ui/CharaImage/chara_image_TU11.hci"
#define GAME_HCI15_PATH  "data/ui/CharaImage/chara_image_TU12.hci"
#define GAME_HCI16_PATH  "data/ui/CharaImage/chara_image_TU13.hci"
#define GAME_HCI17_PATH  "data/ui/CharaImage/chara_image_TU13_5.hci"
#define GAME_PSA_PATH   "data/system/parameter_spec_avater.psa"
#define GAME_PUP_PATH   "data/system/powerup_parameter.pup"

#define GAME_PREBAKED_PATH  "data/pre-baked.xml"

#define GAME_LOBBY_TEXT_PATH    "data/msg/lobby_text_"
#define GAME_CAC_COSTUME_NAMES_PATH  "data/msg/proper_noun_costume_name_"
#define GAME_ACCESORY_NAMES_PATH    "data/msg/proper_noun_accessory_name_"
#define GAME_TALISMAN_NAMES_PATH    "data/msg/proper_noun_talisman_name_"
#define GAME_MATERIAL_NAMES_PATH    "data/msg/proper_noun_material_name_"
#define GAME_BATTLE_NAMES_PATH    "data/msg/proper_noun_battle_name_"
#define GAME_EXTRA_NAMES_PATH    "data/msg/proper_noun_extra_name_"
#define GAME_PET_NAMES_PATH      "data/msg/proper_noun_float_pet_name_"

#define GAME_CAC_COSTUME_DESCS_PATH  "data/msg/proper_noun_costume_info_"
#define GAME_ACCESORY_DESCS_PATH    "data/msg/proper_noun_accessory_info_"

#define GAME_BOTTOM_IDB_PATH "data/system/item/costume_bottom_item.idb"
#define GAME_GLOVES_IDB_PATH "data/system/item/costume_gloves_item.idb"
#define GAME_SHOES_IDB_PATH "data/system/item/costume_shoes_item.idb"
#define GAME_TOP_IDB_PATH "data/system/item/costume_top_item.idb"
#define GAME_ACCESORY_IDB_PATH "data/system/item/accessory_item.idb"
#define GAME_TALISMAN_IDB_PATH "data/system/item/talisman_item.idb"
#define GAME_SKILL_IDB_PATH "data/system/item/skill_item.idb"
#define GAME_MATERIAL_IDB_PATH  "data/system/item/material_item.idb"
#define GAME_BATTLE_IDB_PATH  "data/system/item/battle_item.idb"
#define GAME_EXTRA_IDB_PATH  "data/system/item/extra_item.idb"
#define GAME_PET_IDB_PATH "data/system/item/pet_item.idb"

#define GAME_PAL_PATH   "data/event/lobby/lobby_preset_avatar_list.pal"
#define GAME_TNL_PATH   "data/event/lobby/lobby_npc_list.tnl"

#define GAME_CSS_SOUND_JP_PATH  "data/sound/VOX/Sys/CRT_CS_vox"
#define GAME_CSS_SOUND_EN_PATH  "data/sound/VOX/Sys/en/CRT_CS_vox"

#define GAME_SEV_CMN_SOUND_JP_PATH  "data/sound/VOX/Quest/Dialogue/CAQD_ALL_VOX"
#define GAME_SEV_CMN_SOUND_EN_PATH  "data/sound/VOX/Quest/Dialogue/en/CAQD_ALL_VOX"

#define GAME_SEV_CMN_SOUND2_JP_PATH  "data/sound/VOX/Quest/Dialogue/CAQD_ADD_VOX"
#define GAME_SEV_CMN_SOUND2_EN_PATH  "data/sound/VOX/Quest/Dialogue/en/CAQD_ADD_VOX"

#define GAME_BGM_PATH "data/sound/BGM/CAR_BGM"

#define GAME_HUM_BCS_PATH   "data/chara/HUM/HUM.bcs"
#define GAME_HUF_BCS_PATH   "data/chara/HUF/HUF.bcs"
#define GAME_NMC_BCS_PATH   "data/chara/NMC/NMC.bcs"
#define GAME_FRI_BCS_PATH   "data/chara/FRI/FRI.bcs"
#define GAME_MAM_BCS_PATH   "data/chara/MAM/MAM.bcs"
#define GAME_MAF_BCS_PATH   "data/chara/MAF/MAF.bcs"

#define GAME_COSTUME_FILE_PATH   "data/X2M_COSTUME.xml"

#define GAME_STAGE_DEF_PATH         "data/xv2_stage_def.xml"
#define GAME_STAGE_SLOTS_FILE       "data/XV2P_SLOTS_STAGE.x2s"
#define GAME_STAGE_SLOTS_FILE_LOCAL "data/XV2P_SLOTS_STAGE_LOCAL.x2s"
#define GAME_STAGENAMES_PATH        "data/msg/proper_noun_stage_name_"

#define GAME_STAGE01_PATH   "data/ui/texture/STAGE01.emb"
#define GAME_STAGE02_PATH   "data/ui/texture/STAGE02.emb"

#define GAME_SLOTS_FILE "data/XV2P_SLOTS.x2s"
#define GAME_CST_FILE   "data/system/chara_select_table.cst"
#define GAME_CST_PRB_FILE   "data/system/chara_select_table_prb.cst"

#define GAME_TTB_PATH   "data/quest/XTALK/CommonDialogue.ttb"
#define GAME_TTC_PATH   "data/quest/XTALK/Common.ttc"

#define GAME_QC_DIALOGUE_VOICE  "data/msg/qc_Dialog_voice.msg"
#define GAME_QC_DIALOGUE_SUBS   "data/msg/qc_Dialog_"

#define GAME_CNC_PATH   "data/system/combination_skill.cnc"
#define GAME_CNS_PATH   "data/system/combination_skill.cns"

#define GAME_ERS_PATH   "data/vfx/vfx_spec.ers"

const std::vector<std::string> xv2_lang_codes =
{
    "en",
    "es",
    "ca",
    "fr",
    "de",
    "it",
    "pt",
    "pl",
    "ru",
    "tw",
    "zh",
    "kr"
};

static const std::vector<std::string> original_chars =
{
    "CMN", // Not a char, but we want it in this list. Keep it at index 0!
	"APL",
    "AVB",
    "AVC",
    "BAT",
    "BDK",
    "BJK",
	"BLF",
    "BLM",
    "BLS",
    "BRL",
    "BUL",
    "BUM",
    "BUS",
    "BUU",
    "BUZ",
    "CAB",
    "CL1",
    "CL3",
    "CL4",
    "CLJ",
    "CMP",
    "CO2",
    "COL",
    "CPY",
    "CRC",
    "DBR",
    "DDR",
	"DM2",
	"DMG",
    "DND",
    "El0",
    "FOF",
    "FR4",
    "FR5",
    "FRG",
    "FRI",
    "FRS",
    "FRZ",
	"G13",
    "G16",
    "G17",
    "G18",
    "GBR",
    "GFR",
    "GG1",
    "GGK",
    "GGT",
    "GHF",
    "GHL",
    "GHM",
	"GHP",
	"GIL", 
    "GHS",
    "GK4",
    "GKB",
    "GKG",
    "GKS",
    "GNY",
    "GOD",
    "GOK",
    "GRD",
    "GTG",
    "GTX",
    "GVG",
    "HIK",
    "HIT",
    "HST",
    "HUF",
    "HUM",
    "JCO",
    "JES",
    "JNB",
    "JNG",
    "KFL",
    "KLL",
    "LPA",
    "MAF",
    "MAM",
    "MAP",
    "MIR",
    "MRN",
    "MST",
    "NAP",
    "NI0",
    "NI1",
    "NI2",
    "NIL",
    "NMC",
    "NP1",
    "NP2",
    "NP3",
    "NP4",
    "OBB",
    "OSB",
    "OSN",
    "OSV",
    "OWN",
    "PAN",
    "PIC",
    "POD",
    "PTN",
	"R17",
    "RAD",
    "RCM",
    "ROK",
    "RSB",
    "S17",
    "SB2",
    "SBM",
    "SCR",
    "SD1",
    "SD3",
    "SD4",
    "SIN",
    "SLG",
    "STD",
    "STN",
    "SYF",
    "SYM",
    "TE0",
    "TE1",
    "TG1",
    "TG2",
    "TG3",
    "TG4",
    "TG5",
    "THC",
    "THG",
    "THJ",
    "THK",
    "TKT",
    "TLS",
    "TM0",
    "TM1",
    "TOK",
    "TOW",
	"TPO",
    "TRF",
    "TRS",
    "TRX",
    "TSH",
    "TWP",
    "TWT",
    "VDL",
    "VDS",
    "VG4",
    "VGT",
    "VTB",
    "VTO",
    "WIS",
    "XEG",
    "XEN",
    "YMC",
    "ZBN",
    "ZMD",
    "ZMG",
    "ZMS",    
};

static const std::vector<std::string> forbidden_names_new_stage =
{
    "LBY",
    "BFLBB",
    "BFLBY",
    "BFPLN",
    "BFRCK",
    "BFRCN",
    "BFNMC",
    "BFNMS",
    "BFNMB",
    "BFTWC",
    "BFTWN",
    "BFTWH",
    "BFCEL",
    "BFCEB",
    "BFKOH",
    "BFSIN",
    "BFSKY",
    "BFSEI",
    "BFKAI",
    "BFSPE",
    "BFSPV",
    "BFICE",
    "BFTFL",
    "BFGEN",
    "BFTEN",
    "BFUND",
    "BFTOK",
    "BFNMT",
    "DMKOK",
    "DMNMC",
    "DMNMS",
    "DMTWH",
    "DMDOM",
    "LBY01",
    "LBY02",
    "LBY03",
    "LBY04",
    "DMTWH3",
    "BFLND",
    "BFLNC",
    "BFHEL",
    "LBY05",
    "NLBY",
    "NLBY02",
    "NLBY03",
    "FLBY01",
    "FLBY02",
    "FLBY03",
    "FLBY04",
    "FLBY05",
    "DMTWH2",
    "BFTRE",
    "BFBRW",
    "FLBY03IN",
    "FLBY04IN",
    "FLBY05IN",
    "BFSMT",
    "LBY06",
    "BFTWF",
    "ERR",
    "TRN",
    "CHR",
    "SANDBOX",
    "NMC",
    "LBY01_EVE",
    "BFUND_EVE",
    "BFKOH_QST",
    "BFHEL_QST",
    "DMDOM_EVE",
    "BFUND_QST",
    "BFTWN_QST",
    "BFICE_QST",
    "BFLBB_EVE",
    "BFLBB_QST",
    "BFSPV_QST",
    "BFSPE_QST",
    "BFNMB_QST",
    "BFTWC_EVE",
    "BFLND_QST",
    "TEMPLATE",
    "BFPLN_QST",
    "DMNMS_EVE",
    "BFRCK_QST",
    "BFRCN_QST",
    "FLBY05IN_EVE",
    "FLBY03IN_EVE",
    "DMNMC_QST",
    "BFSEI_EVE",
    "NLBYSKY",
    "BFSMT_EVE",
    "BFSMT_QST",
    "FLBY02_EVE",
    "DMNMC_EVE",
    "BFTFL_EVE",
    "BFTWF_EVE",
    "BFTWF_QST",
    "FLBY01_EVE",
    "BFTWH_QST",
    "BFTWH_EVE",
    "DMTWH_QST",
    "FLBY03_EVE",
    "BFNMC_QST",
    "FLBY04IN_EVE",
    "FLBY04_EVE",
    "FLBY05_EVE",
    "LBY03_EVE",
    "LBY04_EVE",
    "LBY05_EVE",
    "LBY06_EVE",
    "NLBY_EVE",
    "BFBRW_EVE",
    "BFBRW_QST",
    "BFCEB_EVE",
    "BFCEB_QST",
    "BFCEL_EVE",
    "BFCEL_QST",
    "BFGEN_EVE",
    "BFGEN_QST",
    "BFHEL_EVE",
    "BFICE_EVE",
    "BFKAI_EVE",
    "BFKAI_QST",
    "BFKOH_EVE",
    "BFLBY_QST",
    "BFLBY_EVE",
    "BFLNC_EVE",
    "BFLNC_QST",
    "BFLND_EVE",
    "BFNMB_EVE",
    "LBY_EVE",
    "BFNMC_EVE",
    "BFNMS_EVE",
    "BFNMS_QST",
    "BFPLN_",
    "BFPLN_EVE",
    "BFRCK_EVE",
    "BFRCN_EVE",
    "BFSEI_QST",
    "BFSIN_EVE",
    "BFSIN_QST",
    "BFSKY_EVE",
    "BFTWN_EVE",
    "BFSKY_QST",
    "BFSPE_EVE",
    "BFSPV_EVE",
    "DMTWH3_QST",
    "BFTEN_EVE",
    "BFTEN_QST",
    "BFTFL_QST",
    "BFTOK_EVE",
    "BFTOK_QST",
    "BFTRE_EVE",
    "BFTRE_QST",
    "BFTWC_QST",
    "CHR_EVE",
    "DBL",
    "DMKOK_EVE",
    "DMTWH03",
    "DMTWH2_EVE",
    "DMTWH3_EVE",
    "DMTWH_EVE",
    "BFvol",
};

Xv2Fs *xv2fs;
Xv2PatcherSlotsFile *chara_list;
IggyFile *charasele;

CmsFile *game_cms;
CusFile *game_cus;
CsoFile *game_cso;
PscFile *game_psc;
PsaFile *game_psa;
AurFile *game_aur;
SevFile *game_sev;
CmlFile *game_cml;
HciFile *game_hci;
PupFile *game_pup;

std::vector<MsgFile *> game_chara_names;
std::vector<MsgFile *> game_chara_costume_names;

std::vector<MsgFile *> sup_sk_names;
std::vector<MsgFile *> ult_sk_names;
std::vector<MsgFile *> eva_sk_names;
std::vector<MsgFile *> awa_sk_names;

std::vector<MsgFile *> sup_sk_descs;
std::vector<MsgFile *> ult_sk_descs;
std::vector<MsgFile *> eva_sk_descs;
std::vector<MsgFile *> awa_sk_descs;

std::vector<MsgFile *> quest_btlhud_texts;

EmbFile *game_sel_port;
Xv2PreBakedFile *game_prebaked;

std::vector<MsgFile *> game_lobby_texts;
std::vector<MsgFile *> game_cac_costume_names;
std::vector<MsgFile *> game_accesory_names;
std::vector<MsgFile *> game_talisman_names;
std::vector<MsgFile *> game_material_names;
std::vector<MsgFile *> game_battle_names;
std::vector<MsgFile *> game_extra_names;
std::vector<MsgFile *> game_pet_names;

std::vector<MsgFile *> game_cac_costume_descs;
std::vector<MsgFile *> game_accesory_descs;

IdbFile *game_bottom_idb;
IdbFile *game_gloves_idb;
IdbFile *game_shoes_idb;
IdbFile *game_top_idb;
IdbFile *game_accesory_idb;
IdbFile *game_talisman_idb;
IdbFile *game_skill_idb;
IdbFile *game_material_idb;
IdbFile *game_battle_idb;
IdbFile *game_extra_idb;
IdbFile *game_pet_idb;

PalFile *game_pal;
TnlFile *game_tnl;

AcbFile *game_css_acb_jp;
AcbFile *game_css_acb_en;
Afs2File *game_css_awb_jp;
Afs2File *game_css_awb_en;

AcbFile *game_sev_cmn_acb_jp;
AcbFile *game_sev_cmn_acb_en;
Afs2File *game_sev_cmn_awb_jp;
Afs2File *game_sev_cmn_awb_en;

AcbFile *game_sev_cmn2_acb_jp;
AcbFile *game_sev_cmn2_acb_en;
Afs2File *game_sev_cmn2_awb_jp;
Afs2File *game_sev_cmn2_awb_en;

AcbFile *game_bgm_acb;
Afs2File *game_bgm_awb;

BcsFile *game_hum_bcs;
BcsFile *game_huf_bcs;
BcsFile *game_nmc_bcs;
BcsFile *game_fri_bcs;
BcsFile *game_mam_bcs;
BcsFile *game_maf_bcs;

X2mCostumeFile *game_costume_file;

Xv2PatcherSlotsFileStage *game_stage_slots_file, *game_stage_slots_file_local;

Xv2StageDefFile *game_stage_def;
std::vector<MsgFile *> game_stage_names;

EmbFile *game_stage01_emb, *game_stage02_emb;

TtbFile *game_ttb;
TtcFile *game_ttc;

MsgFile *qc_dialogue_voice;
std::vector<MsgFile *> qc_dialogue_subs;

CncFile *game_cnc;
CnsFile *game_cns;

ErsFile *game_ers;

//static std::string chasel_path, chalist_path;
static bool multiple_hci_loaded = false;

static bool sev_audio_changed = false;
static bool bgm_changed = false;

struct HtmlEscape
{
    std::u16string sequence;
    char16_t c;
};

// These were all sequences found in all msg files of the game
static const std::vector<HtmlEscape> xv2_html_sequences =
{
    // HACK: VS2013 doesn't support u16 literals
    { (const char16_t *)L"&quot;", '"' },
    { (const char16_t *)L"&apos;", '\'' },
    { (const char16_t *)L"&gt;", '>' },
    { (const char16_t *)L"&lt;", '<' },
    { (const char16_t *)L"&amp;", '&' },
};

std::string Xenoverse2::UnescapeHtml(const std::string &str)
{
    if (str.find('&') == std::string::npos || str.find(';') == std::string::npos)
        return str;

    std::u16string str16 = Utils::Utf8ToUcs2(str);

    for (size_t i = 0; i < str16.length()-3; i++)
    {
        if (str16[i] == '&')
        {
            for (const HtmlEscape &esc : xv2_html_sequences)
            {
                if (str16.find(esc.sequence, i) == i)
                {
                    str16.replace(i, esc.sequence.length(), std::u16string { esc.c });
                }
            }
        }
    }

    return Utils::Ucs2ToUtf8(str16);
}

std::string Xenoverse2::EscapeHtml(const std::string &str)
{
    bool found = false;

    for (const HtmlEscape &esc : xv2_html_sequences)
    {
        if (str.find((char)esc.c) != std::string::npos)
        {
            found = true;
            break;
        }
    }

    if (!found)
        return str;

    std::u16string str16 = Utils::Utf8ToUcs2(str);

    for (size_t i = 0; i < str16.length(); i++)
    {
        for (const HtmlEscape &esc : xv2_html_sequences)
        {
            if (str16[i] == esc.c)
            {
                str16.replace(i, 1, esc.sequence);
                i += esc.sequence.length()-1;
            }
        }
    }

    return Utils::Ucs2ToUtf8(str16);
}

void Xenoverse2::InitFs(const std::string &game_path)
{
    if (xv2fs)
        delete xv2fs;

    xv2fs = new Xv2Fs(game_path);
}

/*bool Xenoverse2::InitCharaList(const std::string &path, const std::string &list_path)
{
    chasel_path = path;
    chalist_path = list_path;

    if (chara_list)
        delete chara_list;

    chara_list = new CharaListFile();
    if (!chara_list->LoadFromFile(chalist_path))
    {
        delete chara_list;
        chara_list = nullptr;
        return false;
    }

    if (charasele)
        delete charasele;

    charasele = new IggyFile();

    if (!xv2fs->LoadFile(charasele, GAME_CHARASELE_PATH))
    {
        delete chara_list; chara_list = nullptr;
        delete charasele; charasele = nullptr;
        return false;
    }

    return true;
}

bool Xenoverse2::CommitCharaList()
{
    if (!xv2fs || !chara_list || !charasele)
        return false;

    if (!xv2fs->SaveFile(charasele, GAME_CHARASELE_PATH))
        return false;

    return chara_list->SaveToFile(chalist_path, true, true);
}*/

bool Xenoverse2::InitCharaList()
{
    if (chara_list)
        delete chara_list;

    chara_list = new Xv2PatcherSlotsFile();
    if (!xv2fs->LoadFile(chara_list, GAME_SLOTS_FILE))
    {
        size_t cst_size, cst_prb_size;
        uint8_t *cst_buf = xv2fs->ReadFile(GAME_CST_FILE, &cst_size);
        uint8_t *cst_prb_buf = xv2fs->ReadFile(GAME_CST_PRB_FILE, &cst_prb_size);
        bool error = true;

        if (cst_buf && cst_prb_buf)
        {
            error = !chara_list->LoadFromCst(cst_buf, cst_size, cst_prb_buf, cst_prb_size);
            delete[] cst_buf;
            delete[] cst_prb_buf;

            if (!error)
            {
                if (!CommitCharaList(true, false))
                    error = true;
            }
        }
        else
        {
            if (cst_buf)
                delete[] cst_buf;

            if (cst_prb_buf)
                delete[] cst_prb_buf;
        }

        if (error)
        {
            delete chara_list;
            chara_list = nullptr;
            return false;
        }
    }

    if (charasele)
        delete charasele;

    charasele = new IggyFile();

    if (!xv2fs->LoadFile(charasele, GAME_CHARASELE_PATH))
    {
        delete chara_list; chara_list = nullptr;
        delete charasele; charasele = nullptr;
        return false;
    }

    return true;
}

bool Xenoverse2::CommitCharaList(bool commit_slots, bool commit_iggy)
{
    if (!xv2fs || !chara_list)
        return false;

    if (commit_iggy)
    {
        if (!charasele || !xv2fs->SaveFile(charasele, GAME_CHARASELE_PATH))
            return false;
    }

    if (commit_slots && !xv2fs->SaveFile(chara_list, GAME_SLOTS_FILE))
        return false;

    return true;
}

bool Xenoverse2::InitSystemFiles(bool only_cms, bool multiple_hci)
{
    if (!xv2fs)
        return false;

    if (game_cms)
        delete game_cms;

    game_cms = new CmsFile();
    if (!xv2fs->LoadFile(game_cms, GAME_CMS_PATH) || !game_cms->IsXV2())
    {
        DPRINTF("%s: cannot load cms.\n", FUNCNAME);
        delete game_cms; game_cms = nullptr;
        return false;
    }

    if (only_cms)
        return true;

    if (game_cus)
        delete game_cus;

    game_cus = new CusFile();
    if (!xv2fs->LoadFile(game_cus, GAME_CUS_PATH))
    {
        DPRINTF("%s: cannot load cus.\n", FUNCNAME);
        delete game_cus; game_cus = nullptr;
        return false;
    }

    if (game_cso)
        delete game_cso;

    game_cso = new CsoFile();
    if (!xv2fs->LoadFile(game_cso, GAME_CSO_PATH))
    {
        DPRINTF("%s: cannot load cso.\n", FUNCNAME);
        delete game_cso; game_cso = nullptr;
        return false;
    }

    if (game_psc)
        delete game_psc;

    game_psc = new PscFile();
    if (!xv2fs->LoadFile(game_psc, GAME_PSC_PATH))
    {
        DPRINTF("%s: cannot load psc.\n", FUNCNAME);
        delete game_psc; game_psc = nullptr;
        return false;
    }

    if (game_aur)
        delete game_aur;

    game_aur = new AurFile();
    if (!xv2fs->LoadFile(game_aur, GAME_AUR_PATH))
    {
        DPRINTF("%s: cannot load aur.\n", FUNCNAME);
        delete game_aur; game_aur = nullptr;
        return false;
    }    

    if (game_sev)
        delete game_sev;

    game_sev = new SevFile();
    if (!xv2fs->LoadFile(game_sev, GAME_SEV_PATH))
    {
        DPRINTF("%s: cannot load sev.\n", FUNCNAME);
        delete game_sev; game_sev = nullptr;
        return false;
    }

    if (game_cml)
        delete game_cml;

    game_cml = new CmlFile();
    if (!xv2fs->LoadFile(game_cml, GAME_CML_PATH))
    {
        DPRINTF("%s: cannot load cml.\n", FUNCNAME);
        delete game_cml; game_cml = nullptr;
        return false;
    }

    if (game_hci)
        delete game_hci;

    game_hci = new HciFile();

    if (!multiple_hci)
    {
        if (!xv2fs->LoadFile(game_hci, GAME_HCI_PATH))
        {
            DPRINTF("%s: cannot load hci.\n", FUNCNAME);
            delete game_hci; game_hci = nullptr;
            return false;
        }

        multiple_hci_loaded = false;
    }
    else
    {
        std::vector<const uint8_t *> bufs;
        std::vector<size_t> sizes;

        size_t size1;
        uint8_t *buf1 = xv2fs->ReadFile(GAME_HCI_PATH, &size1);

        if (!buf1)
        {
            DPRINTF("%s: failed to read 1st hci.\n", FUNCNAME);
            delete game_hci; game_hci = nullptr;
            return false;
        }

        size_t size2;
        uint8_t *buf2 = xv2fs->ReadFile(GAME_HCI2_PATH, &size2);

        if (!buf2)
        {
            DPRINTF("%s: failed to read 2nd hci.\n", FUNCNAME);
            delete[] buf1;
            delete game_hci; game_hci = nullptr;
            return false;
        }

        size_t size3;
        uint8_t *buf3 = xv2fs->ReadFile(GAME_HCI3_PATH, &size3);

        if (!buf3)
        {
            DPRINTF("%s: failed to read 3rd hci.\n", FUNCNAME);
            delete[] buf1; delete[] buf2;
            delete game_hci; game_hci = nullptr;
            return false;
        }

        size_t size4;
        uint8_t *buf4 = xv2fs->ReadFile(GAME_HCI4_PATH, &size4);

        if (!buf4)
        {
            DPRINTF("%s: failed to read 4th hci.\n", FUNCNAME);
            delete[] buf1; delete[] buf2; delete[] buf3;
            delete game_hci; game_hci = nullptr;
            return false;
        }

        size_t size5;
        uint8_t *buf5 = xv2fs->ReadFile(GAME_HCI5_PATH, &size5);

        if (!buf5)
        {
            DPRINTF("%s: failed to read 5th hci.\n", FUNCNAME);
            delete[] buf1; delete[] buf2; delete[] buf3; delete[] buf4;
            delete game_hci; game_hci = nullptr;
            return false;
        }

        size_t size6;
        uint8_t *buf6 = xv2fs->ReadFile(GAME_HCI6_PATH, &size6);

        if (!buf6)
        {
            DPRINTF("%s: failed to read 6th hci.\n", FUNCNAME);
            delete[] buf1; delete[] buf2; delete[] buf3; delete[] buf4; delete[] buf5;
            delete game_hci; game_hci = nullptr;
            return false;
        }

        size_t size7;
        uint8_t *buf7 = xv2fs->ReadFile(GAME_HCI7_PATH, &size7);

        if (!buf7)
        {
            DPRINTF("%s: failed to read 7th hci.\n", FUNCNAME);
            delete[] buf1; delete[] buf2; delete[] buf3; delete[] buf4; delete[] buf5; delete[] buf6;
            delete game_hci; game_hci = nullptr;
            return false;
        }

        size_t size8;
        uint8_t *buf8 = xv2fs->ReadFile(GAME_HCI8_PATH, &size8);

        if (!buf8)
        {
            DPRINTF("%s: failed to read 8th hci.\n", FUNCNAME);
            delete[] buf1; delete[] buf2; delete[] buf3; delete[] buf4; delete[] buf5; delete[] buf6; delete[] buf7;
            delete game_hci; game_hci = nullptr;
            return false;
        }

        size_t size9;
        uint8_t *buf9 = xv2fs->ReadFile(GAME_HCI9_PATH, &size9);

        if (!buf9)
        {
            DPRINTF("%s: failed to read 9th hci.\n", FUNCNAME);
            delete[] buf1; delete[] buf2; delete[] buf3; delete[] buf4; delete[] buf5;
            delete[] buf6; delete[] buf7; delete[] buf8;
            delete game_hci; game_hci = nullptr;
            return false;
        }

        size_t size10;
        uint8_t *buf10 = xv2fs->ReadFile(GAME_HCI10_PATH, &size10);

        if (!buf10)
        {
            DPRINTF("%s: failed to read 10th hci.\n", FUNCNAME);
            delete[] buf1; delete[] buf2; delete[] buf3; delete[] buf4; delete[] buf5;
            delete[] buf6; delete[] buf7; delete[] buf8; delete[] buf9;
            delete game_hci; game_hci = nullptr;
            return false;
        }

        size_t size11;
        uint8_t *buf11 = xv2fs->ReadFile(GAME_HCI11_PATH, &size11);

        if (!buf11)
        {
            DPRINTF("%s: failed to read 11th hci.\n", FUNCNAME);
            delete[] buf1; delete[] buf2; delete[] buf3; delete[] buf4; delete[] buf5;
            delete[] buf6; delete[] buf7; delete[] buf8; delete[] buf9; delete[] buf10;
            delete game_hci; game_hci = nullptr;
            return false;
        }

        size_t size12;
        uint8_t *buf12 = xv2fs->ReadFile(GAME_HCI12_PATH, &size12);

        if (!buf12)
        {
            DPRINTF("%s: failed to read 12th hci.\n", FUNCNAME);
            delete[] buf1; delete[] buf2; delete[] buf3; delete[] buf4; delete[] buf5;
            delete[] buf6; delete[] buf7; delete[] buf8; delete[] buf9; delete[] buf10;
            delete[] buf11;
            delete game_hci; game_hci = nullptr;
            return false;
        }

        size_t size13;
        uint8_t *buf13 = xv2fs->ReadFile(GAME_HCI13_PATH, &size13);

        if (!buf13)
        {
            DPRINTF("%s: failed to read 13th hci.\n", FUNCNAME);
            delete[] buf1; delete[] buf2; delete[] buf3; delete[] buf4; delete[] buf5;
            delete[] buf6; delete[] buf7; delete[] buf8; delete[] buf9; delete[] buf10;
            delete[] buf11; delete[] buf12;
            delete game_hci; game_hci = nullptr;
            return false;
        }

        size_t size14;
        uint8_t *buf14 = xv2fs->ReadFile(GAME_HCI14_PATH, &size14);

        if (!buf14)
        {
            DPRINTF("%s: failed to read 14th hci.\n", FUNCNAME);
            delete[] buf1; delete[] buf2; delete[] buf3; delete[] buf4; delete[] buf5;
            delete[] buf6; delete[] buf7; delete[] buf8; delete[] buf9; delete[] buf10;
            delete[] buf11; delete[] buf12; delete[] buf13;
            delete game_hci; game_hci = nullptr;
            return false;
        }

        size_t size15;
        uint8_t *buf15 = xv2fs->ReadFile(GAME_HCI15_PATH, &size15);

        if (!buf15)
        {
            DPRINTF("%s: failed to read 15th hci.\n", FUNCNAME);
            delete[] buf1; delete[] buf2; delete[] buf3; delete[] buf4; delete[] buf5;
            delete[] buf6; delete[] buf7; delete[] buf8; delete[] buf9; delete[] buf10;
            delete[] buf11; delete[] buf12; delete[] buf13; delete[] buf14;
            delete game_hci; game_hci = nullptr;
            return false;
        }

        size_t size16;
        uint8_t *buf16 = xv2fs->ReadFile(GAME_HCI16_PATH, &size16);

        if (!buf16)
        {
            DPRINTF("%s: failed to read 16th hci.\n", FUNCNAME);
            delete[] buf1; delete[] buf2; delete[] buf3; delete[] buf4; delete[] buf5;
            delete[] buf6; delete[] buf7; delete[] buf8; delete[] buf9; delete[] buf10;
            delete[] buf11; delete[] buf12; delete[] buf13; delete[] buf14; delete[] buf15;
            delete game_hci; game_hci = nullptr;
            return false;
        }

        size_t size17;
        uint8_t *buf17 = xv2fs->ReadFile(GAME_HCI17_PATH, &size17);

        if (!buf17)
        {
            DPRINTF("%s: failed to read 16th hci.\n", FUNCNAME);
            delete[] buf1; delete[] buf2; delete[] buf3; delete[] buf4; delete[] buf5;
            delete[] buf6; delete[] buf7; delete[] buf8; delete[] buf9; delete[] buf10;
            delete[] buf11; delete[] buf12; delete[] buf13; delete[] buf14; delete[] buf15;
            delete[] buf16;
            delete game_hci; game_hci = nullptr;
            return false;
        }

        bufs.push_back(buf1); bufs.push_back(buf2); bufs.push_back(buf3); bufs.push_back(buf4); bufs.push_back(buf5);
        bufs.push_back(buf6); bufs.push_back(buf7); bufs.push_back(buf8); bufs.push_back(buf9); bufs.push_back(buf10);
        bufs.push_back(buf11); bufs.push_back(buf12); bufs.push_back(buf13); bufs.push_back(buf14); bufs.push_back(buf15);
        bufs.push_back(buf16); bufs.push_back(buf17);
        sizes.push_back(size1); sizes.push_back(size2); sizes.push_back(size3); sizes.push_back(size4); sizes.push_back(size5);
        sizes.push_back(size6); sizes.push_back(size7); sizes.push_back(size8); sizes.push_back(size9); sizes.push_back(size10);
        sizes.push_back(size11); sizes.push_back(size12); sizes.push_back(size13); sizes.push_back(size14); sizes.push_back(size15);
        sizes.push_back(size16); sizes.push_back(size17);

        bool ret = game_hci->LoadMultiple(bufs, sizes);
        delete[] buf1; delete[] buf2; delete[] buf3; delete[] buf4; delete[] buf5;
        delete[] buf6; delete[] buf7; delete[] buf8; delete[] buf9; delete[] buf10;
        delete[] buf11; delete[] buf12; delete[] buf13; delete[] buf14;

        if (!ret)
        {
            DPRINTF("%s: Failed loading multiple hci.\n", FUNCNAME);
            delete game_hci; game_hci = nullptr;
            return false;
        }

        multiple_hci_loaded = true;
    }    

    if (game_psa)
        delete game_psa;

    game_psa = new PsaFile();
    if (!xv2fs->LoadFile(game_psa, GAME_PSA_PATH))
    {
        DPRINTF("%s: cannot load psa.\n", FUNCNAME);
        delete game_psa; game_psa = nullptr;
        return false;
    }

    if (game_pup)
        delete game_pup;

    game_pup = new PupFile();
    if (!xv2fs->LoadFile(game_pup, GAME_PUP_PATH))
    {
        DPRINTF("%s: cannot load pup.\n", FUNCNAME);
        delete game_pup; game_pup = nullptr;
        return false;
    }

    if (game_stage_def)
        delete game_stage_def;

    game_stage_def = new Xv2StageDefFile();
    if (xv2fs->FileExists(GAME_STAGE_DEF_PATH, false))
    {
        if (!xv2fs->CompileFile(game_stage_def, GAME_STAGE_DEF_PATH))
        {
            DPRINTF("%s: cannot load stage def.\n", FUNCNAME);
            delete game_stage_def; game_stage_def = nullptr;
            return false;
        }
    }
    else
    {
        TiXmlDocument doc;
        doc.Parse(default_stagedef);

        if (doc.ErrorId() != 0)
        {
            DPRINTF("%s: Internal error parsing default stage_def (this should never happen)\n", FUNCNAME);
            delete game_stage_def; game_stage_def = nullptr;
            return false;
        }

        if (!game_stage_def->Compile(&doc))
        {
            DPRINTF("%s: Internal error compiling default stage_def (this should never happen)\n", FUNCNAME);
            delete game_stage_def; game_stage_def = nullptr;
            return false;
        }

        xv2fs->DecompileFile(game_stage_def, GAME_STAGE_DEF_PATH);
    }

    return true;
}

bool Xenoverse2::CommitSystemFiles(bool pup)
{
    if (!xv2fs || !game_cms || !game_cus || !game_cso || !game_psc || !game_aur || !game_sev || !game_cml || !game_hci || !game_stage_def)
        return false;

    if (pup && !game_pup)
        return false;

    if (multiple_hci_loaded)
    {
        DPRINTF("%s: Cannot save HCI file when multiple hci were loaded!\n", FUNCNAME);
        return false;
    }

    if (!xv2fs->SaveFile(game_cms, GAME_CMS_PATH))
    {
        DPRINTF("Save of cms failed.\n");
        return false;
    }

    if (!xv2fs->SaveFile(game_cus, GAME_CUS_PATH))
    {
        DPRINTF("Save of cus failed.\n");
        return false;
    }

    if (!xv2fs->SaveFile(game_cso, GAME_CSO_PATH))
    {
        DPRINTF("Save of cso failed.\n");
        return false;
    }

    if (!xv2fs->SaveFile(game_psc, GAME_PSC_PATH))
    {
        DPRINTF("Save of psc failed.\n");
        return false;
    }

    if (!xv2fs->SaveFile(game_aur, GAME_AUR_PATH))
    {
        DPRINTF("Save of aur failed.\n");
        return false;
    }

    if (!xv2fs->SaveFile(game_sev, GAME_SEV_PATH))
    {
        DPRINTF("Save of sev failed.\n");
        return false;
    }

    if (!xv2fs->SaveFile(game_cml, GAME_CML_PATH))
    {
        DPRINTF("Save of cml failed.\n");
        return false;
    }

    if (!xv2fs->SaveFile(game_hci, GAME_HCI_PATH))
    {
        DPRINTF("Save of hci failed.\n");
        return false;
    }

    if (!xv2fs->SaveFile(game_pup, GAME_PUP_PATH))
    {
        DPRINTF("Save of pup failed.\n");
        return false;
    }

    if (!xv2fs->DecompileFile(game_stage_def, GAME_STAGE_DEF_PATH))
    {
        DPRINTF("Save of stage_def failed.\n");
        return false;
    }

    return true;
}

bool Xenoverse2::InitCharaNames(int only_this_lang)
{
    if (!xv2fs)
        return false;

    for (MsgFile *msg : game_chara_names)
    {
        if (msg)
            delete msg;
    }

    game_chara_names.clear();
    game_chara_names.resize(XV2_LANG_NUM);

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        if (only_this_lang >= 0 && i != only_this_lang)
            continue;

        std::string path = GAME_CHARANAMES_PATH + xv2_lang_codes[i] + ".msg";
        game_chara_names[i] = new MsgFile();

        if (!xv2fs->LoadFile(game_chara_names[i], path))
        {
            DPRINTF("Failed loading file \"%s\"\n", path.c_str());

            if (i >= XV2_LANG_CHINESE1)
            {
                DPRINTF("The file that failed loading is one from the new languages added to 1.04.1 -->v2<--.\n\n"
                        "UPDATE YOUR GAME, AND IF YOU ARE PIRATING IT, YOU ARE ON YOUR OWN, DON'T COME TO THE FORUMS ASKING FOR HELP.\n");
            }

            return false;
        }
    }

    return true;
}

bool Xenoverse2::CommitCharaNames()
{
    if (!xv2fs || game_chara_names.size() != XV2_LANG_NUM)
        return false;

    for (size_t i = 0; i < XV2_LANG_NUM; i++)
    {
        if (!game_chara_names[i])
            continue;

        std::string path = GAME_CHARANAMES_PATH + xv2_lang_codes[i] + ".msg";

        if (!xv2fs->SaveFile(game_chara_names[i], path))
        {
            DPRINTF("Failed saving file \"%s\"\n", path.c_str());
            return false;
        }
    }

    return true;
}

bool Xenoverse2::InitCharaCostumeNames(int only_this_lang)
{
    if (!xv2fs)
        return false;

    for (MsgFile *msg : game_chara_costume_names)
    {
        if (msg)
            delete msg;
    }

    game_chara_costume_names.clear();
    game_chara_costume_names.resize(XV2_LANG_NUM);

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        if (only_this_lang >= 0 && i != only_this_lang)
            continue;

        std::string path = GAME_CHARA_COSTUMENAMES_PATH + xv2_lang_codes[i] + ".msg";
        game_chara_costume_names[i] = new MsgFile();

        if (!xv2fs->LoadFile(game_chara_costume_names[i], path))
        {
            DPRINTF("Failed loading file \"%s\"\n", path.c_str());
            return false;
        }
    }

    return true;
}

bool Xenoverse2::CommitCharaCostumeNames()
{
    if (!xv2fs || game_chara_costume_names.size() != XV2_LANG_NUM)
        return false;

    for (size_t i = 0; i < XV2_LANG_NUM; i++)
    {
        if (!game_chara_costume_names[i])
            continue;

        std::string path = GAME_CHARA_COSTUMENAMES_PATH + xv2_lang_codes[i] + ".msg";

        if (!xv2fs->SaveFile(game_chara_costume_names[i], path))
        {
            DPRINTF("Failed saving file \"%s\"\n", path.c_str());
            return false;
        }
    }

    return true;
}

bool Xenoverse2::InitSkillNames(int only_this_lang)
{
    if (!xv2fs)
        return false;

    for (MsgFile *msg : sup_sk_names)
    {
        if (msg)
            delete msg;
    }

    for (MsgFile *msg : ult_sk_names)
    {
        if (msg)
            delete msg;
    }

    for (MsgFile *msg : eva_sk_names)
    {
        if (msg)
            delete msg;
    }

    for (MsgFile *msg : awa_sk_names)
    {
        if (msg)
            delete msg;
    }

    sup_sk_names.clear();
    ult_sk_names.clear();
    eva_sk_names.clear();
    awa_sk_names.clear();

    sup_sk_names.resize(XV2_LANG_NUM);
    ult_sk_names.resize(XV2_LANG_NUM);
    eva_sk_names.resize(XV2_LANG_NUM);
    awa_sk_names.resize(XV2_LANG_NUM);

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        if (only_this_lang >= 0 && i != only_this_lang)
            continue;

        {
            std::string path = GAME_SUPER_SKILLS_NAME_PATH + xv2_lang_codes[i] + ".msg";
            sup_sk_names[i] = new MsgFile();

            if (!xv2fs->LoadFile(sup_sk_names[i], path))
            {
                if (i >= XV2_LANG_CHINESE1)
                {
                    DPRINTF("The file that failed loading is one from the new languages added to 1.04.1 -->v2<--.\n\n"
                            "UPDATE YOUR GAME, AND IF YOU ARE PIRATING IT, YOU ARE ON YOUR OWN, DON'T COME TO THE FORUMS ASKING FOR HELP.\n");
                }

                return false;
            }
        }

        {
            std::string path = GAME_ULTIMATE_SKILLS_NAME_PATH + xv2_lang_codes[i] + ".msg";
            ult_sk_names[i] = new MsgFile();

            if (!xv2fs->LoadFile(ult_sk_names[i], path))
                return false;
        }

        {
            std::string path = GAME_EVASIVE_SKILLS_NAME_PATH + xv2_lang_codes[i] + ".msg";
            eva_sk_names[i] = new MsgFile();

            if (!xv2fs->LoadFile(eva_sk_names[i], path))
                return false;
        }

        {
            std::string path = GAME_AWAKEN_SKILLS_NAME_PATH + xv2_lang_codes[i] + ".msg";
            awa_sk_names[i] = new MsgFile();

            if (!xv2fs->LoadFile(awa_sk_names[i], path))
                return false;
        }
    }

    return true;
}

bool Xenoverse2::CommitSkillNames()
{
    if (!xv2fs || sup_sk_names.size() != XV2_LANG_NUM || ult_sk_names.size() != XV2_LANG_NUM ||
        eva_sk_names.size() != XV2_LANG_NUM || awa_sk_names.size() != XV2_LANG_NUM)
    {
        return false;
    }

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        {
            std::string path = GAME_SUPER_SKILLS_NAME_PATH + xv2_lang_codes[i] + ".msg";

            if (!xv2fs->SaveFile(sup_sk_names[i], path))
                return false;
        }

        {
            std::string path = GAME_ULTIMATE_SKILLS_NAME_PATH + xv2_lang_codes[i] + ".msg";

            if (!xv2fs->SaveFile(ult_sk_names[i], path))
                return false;
        }

        {
            std::string path = GAME_EVASIVE_SKILLS_NAME_PATH + xv2_lang_codes[i] + ".msg";

            if (!xv2fs->SaveFile(eva_sk_names[i], path))
                return false;
        }

        {
            std::string path = GAME_AWAKEN_SKILLS_NAME_PATH + xv2_lang_codes[i] + ".msg";

            if (!xv2fs->SaveFile(awa_sk_names[i], path))
                return false;
        }
    }

    return true;
}

bool Xenoverse2::InitSkillDescs(int only_this_lang)
{
    if (!xv2fs)
        return false;

    for (MsgFile *msg : sup_sk_descs)
    {
        if (msg)
            delete msg;
    }

    for (MsgFile *msg : ult_sk_descs)
    {
        if (msg)
            delete msg;
    }

    for (MsgFile *msg : eva_sk_descs)
    {
        if (msg)
            delete msg;
    }

    for (MsgFile *msg : awa_sk_descs)
    {
        if (msg)
            delete msg;
    }

    sup_sk_descs.clear();
    ult_sk_descs.clear();
    eva_sk_descs.clear();
    awa_sk_descs.clear();

    sup_sk_descs.resize(XV2_LANG_NUM);
    ult_sk_descs.resize(XV2_LANG_NUM);
    eva_sk_descs.resize(XV2_LANG_NUM);
    awa_sk_descs.resize(XV2_LANG_NUM);

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        if (only_this_lang >= 0 && i != only_this_lang)
            continue;

        {
            std::string path = GAME_SUPER_SKILLS_DESC_PATH + xv2_lang_codes[i] + ".msg";
            sup_sk_descs[i] = new MsgFile();

            if (!xv2fs->LoadFile(sup_sk_descs[i], path))
            {
                if (i >= XV2_LANG_CHINESE1)
                {
                    DPRINTF("The file that failed loading is one from the new languages added to 1.04.1 -->v2<--.\n\n"
                            "UPDATE YOUR GAME, AND IF YOU ARE PIRATING IT, YOU ARE ON YOUR OWN, DON'T COME TO THE FORUMS ASKING FOR HELP.\n");
                }

                return false;
            }
        }

        {
            std::string path = GAME_ULTIMATE_SKILLS_DESC_PATH + xv2_lang_codes[i] + ".msg";
            ult_sk_descs[i] = new MsgFile();

            if (!xv2fs->LoadFile(ult_sk_descs[i], path))
                return false;
        }

        {
            std::string path = GAME_EVASIVE_SKILLS_DESC_PATH + xv2_lang_codes[i] + ".msg";
            eva_sk_descs[i] = new MsgFile();

            if (!xv2fs->LoadFile(eva_sk_descs[i], path))
                return false;
        }

        {
            std::string path = GAME_AWAKEN_SKILLS_DESC_PATH + xv2_lang_codes[i] + ".msg";
            awa_sk_descs[i] = new MsgFile();

            if (!xv2fs->LoadFile(awa_sk_descs[i], path))
                return false;
        }
    }

    return true;
}

bool Xenoverse2::CommitSkillDescs()
{
    if (!xv2fs || sup_sk_descs.size() != XV2_LANG_NUM || ult_sk_descs.size() != XV2_LANG_NUM ||
        eva_sk_descs.size() != XV2_LANG_NUM || awa_sk_descs.size() != XV2_LANG_NUM)
    {
        return false;
    }

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        {
            std::string path = GAME_SUPER_SKILLS_DESC_PATH + xv2_lang_codes[i] + ".msg";

            if (!xv2fs->SaveFile(sup_sk_descs[i], path))
                return false;
        }

        {
            std::string path = GAME_ULTIMATE_SKILLS_DESC_PATH + xv2_lang_codes[i] + ".msg";

            if (!xv2fs->SaveFile(ult_sk_descs[i], path))
                return false;
        }

        {
            std::string path = GAME_EVASIVE_SKILLS_DESC_PATH + xv2_lang_codes[i] + ".msg";

            if (!xv2fs->SaveFile(eva_sk_descs[i], path))
                return false;
        }

        {
            std::string path = GAME_AWAKEN_SKILLS_DESC_PATH + xv2_lang_codes[i] + ".msg";

            if (!xv2fs->SaveFile(awa_sk_descs[i], path))
                return false;
        }
    }

    return true;
}

bool Xenoverse2::InitBtlHudText(int only_this_lang)
{
    if (!xv2fs)
        return false;

    for (MsgFile *msg : quest_btlhud_texts)
    {
        if (msg)
            delete msg;
    }

    quest_btlhud_texts.clear();
    quest_btlhud_texts.resize(XV2_LANG_NUM);

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        if (only_this_lang >= 0 && i != only_this_lang)
            continue;

        std::string path = GAME_QUEST_BTLHUD_TEXT_PATH + xv2_lang_codes[i] + ".msg";
        quest_btlhud_texts[i] = new MsgFile();

        if (!xv2fs->LoadFile(quest_btlhud_texts[i], path))
            return false;

    }

    return true;
}

bool Xenoverse2::CommitBtlHudText()
{
    if (!xv2fs || quest_btlhud_texts.size() != XV2_LANG_NUM)
    {
        return false;
    }

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        std::string path = GAME_QUEST_BTLHUD_TEXT_PATH + xv2_lang_codes[i] + ".msg";

        if (!xv2fs->SaveFile(quest_btlhud_texts[i], path))
            return false;

    }

    return true;
}

bool Xenoverse2::InitSelPort()
{
    if (!xv2fs)
        return false;

    if (game_sel_port)
        delete game_sel_port;

    game_sel_port = new EmbFile();
    if (!xv2fs->LoadFile(game_sel_port, GAME_SELPORT_PATH))
    {
        delete game_sel_port;
        game_sel_port = nullptr;
        return false;
    }

    return true;
}

bool Xenoverse2::CommitSelPort()
{
    if (!xv2fs || !game_sel_port)
        return false;

    return xv2fs->SaveFile(game_sel_port, GAME_SELPORT_PATH);
}

bool Xenoverse2::InitPreBaked()
{
    if (!xv2fs)
        return false;

    if (game_prebaked)
        delete game_prebaked;

    game_prebaked = new Xv2PreBakedFile();
    xv2fs->CompileFile(game_prebaked, GAME_PREBAKED_PATH); // Ignore the error, as file may not exist

    return true;
}

bool Xenoverse2::CommitPreBaked()
{
    if (!xv2fs || !game_prebaked)
        return false;

    return xv2fs->DecompileFile(game_prebaked, GAME_PREBAKED_PATH);
}

bool Xenoverse2::InitLobbyText(int only_this_lang)
{
    if (!xv2fs)
        return false;

    for (MsgFile *msg : game_lobby_texts)
    {
        if (msg)
            delete msg;
    }

    game_lobby_texts.clear();
    game_lobby_texts.resize(XV2_LANG_NUM);

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        if (only_this_lang >= 0 && i != only_this_lang)
            continue;

        std::string path = GAME_LOBBY_TEXT_PATH + xv2_lang_codes[i] + ".msg";
        game_lobby_texts[i] = new MsgFile();

        if (!xv2fs->LoadFile(game_lobby_texts[i], path))
        {
            DPRINTF("Failed loading file \"%s\"\n", path.c_str());
            return false;
        }
    }

    return true;
}

bool Xenoverse2::CommitLobbyText()
{
    if (!xv2fs || game_lobby_texts.size() != XV2_LANG_NUM)
        return false;

    for (size_t i = 0; i < XV2_LANG_NUM; i++)
    {
        if (!game_lobby_texts[i])
            continue;

        std::string path = GAME_LOBBY_TEXT_PATH + xv2_lang_codes[i] + ".msg";

        if (!xv2fs->SaveFile(game_lobby_texts[i], path))
        {
            DPRINTF("Failed saving file \"%s\"\n", path.c_str());
            return false;
        }
    }

    return true;
}

bool Xenoverse2::InitCacCostumeNames(int only_this_lang)
{
    if (!xv2fs)
        return false;

    for (MsgFile *msg : game_cac_costume_names)
    {
        if (msg)
            delete msg;
    }

    for (MsgFile *msg : game_accesory_names)
    {
        if (msg)
            delete msg;
    }

    game_cac_costume_names.clear();
    game_accesory_names.clear();

    game_cac_costume_names.resize(XV2_LANG_NUM);
    game_accesory_names.resize(XV2_LANG_NUM);

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        if (only_this_lang >= 0 && i != only_this_lang)
            continue;

        {
            std::string path = GAME_CAC_COSTUME_NAMES_PATH + xv2_lang_codes[i] + ".msg";
            game_cac_costume_names[i] = new MsgFile();

            if (!xv2fs->LoadFile(game_cac_costume_names[i], path))
                return false;
        }

        {
            std::string path = GAME_ACCESORY_NAMES_PATH + xv2_lang_codes[i] + ".msg";
            game_accesory_names[i] = new MsgFile();            

            if (!xv2fs->LoadFile(game_accesory_names[i], path))
                return false;            
        }
    }

    return true;
}

bool Xenoverse2::CommitCacCostumeNames()
{
    if (!xv2fs || game_cac_costume_names.size() != XV2_LANG_NUM || game_accesory_names.size() != XV2_LANG_NUM)
        return false;

    size_t num_entries = game_cac_costume_names[0]->GetNumEntries();

    for (size_t i = 0; i < XV2_LANG_NUM; i++)
    {
        if (i != 0 && game_cac_costume_names[i]->GetNumEntries() != num_entries)
        {
            DPRINTF("%s: Failure in saving proper_noun_costume_name_* files."
                    "This program requires the files for all languages to be synchronized.\n", FUNCNAME);

            return false;
        }

        std::string path = GAME_CAC_COSTUME_NAMES_PATH + xv2_lang_codes[i] + ".msg";

        if (!xv2fs->SaveFile(game_cac_costume_names[i], path))
        {
            DPRINTF("Failed saving file \"%s\"\n", path.c_str());
            return false;
        }
    }

    num_entries = game_accesory_names[0]->GetNumEntries();

    for (size_t i = 0; i < XV2_LANG_NUM; i++)
    {
        if (i != 0 && game_accesory_names[i]->GetNumEntries() != num_entries)
        {
            DPRINTF("%s: Failure in saving proper_noun_accessory_name_* files."
                    "This program requires the files for all languages to be synchronized.\n", FUNCNAME);

            return false;
        }

        std::string path = GAME_ACCESORY_NAMES_PATH + xv2_lang_codes[i] + ".msg";

        if (!xv2fs->SaveFile(game_accesory_names[i], path))
        {
            DPRINTF("Failed saving file \"%s\"\n", path.c_str());
            return false;
        }
    }

    return true;
}

bool Xenoverse2::InitCacCostumeDescs(int only_this_lang)
{
    if (!xv2fs)
        return false;

    for (MsgFile *msg : game_cac_costume_descs)
    {
        if (msg)
            delete msg;
    }

    for (MsgFile *msg : game_accesory_descs)
    {
        if (msg)
            delete msg;
    }

    game_cac_costume_descs.clear();
    game_accesory_descs.clear();

    game_cac_costume_descs.resize(XV2_LANG_NUM);
    game_accesory_descs.resize(XV2_LANG_NUM);

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        if (only_this_lang >= 0 && i != only_this_lang)
            continue;

        {
            std::string path = GAME_CAC_COSTUME_DESCS_PATH + xv2_lang_codes[i] + ".msg";
            game_cac_costume_descs[i] = new MsgFile();

            if (!xv2fs->LoadFile(game_cac_costume_descs[i], path))
                return false;
        }

        {
            std::string path = GAME_ACCESORY_DESCS_PATH + xv2_lang_codes[i] + ".msg";
            game_accesory_descs[i] = new MsgFile();

            if (!xv2fs->LoadFile(game_accesory_descs[i], path))
                return false;
        }
    }

    return true;
}

bool Xenoverse2::CommitCacCostumeDescs()
{
    if (!xv2fs || game_cac_costume_descs.size() != XV2_LANG_NUM)
        return false;

    size_t num_entries = game_cac_costume_descs[0]->GetNumEntries();

    for (size_t i = 0; i < XV2_LANG_NUM; i++)
    {
        if (i != 0 && game_cac_costume_descs[i]->GetNumEntries() != num_entries)
        {
            DPRINTF("%s: Failure in saving proper_noun_costume_info_* files."
                    "This program requires the files for all languages to be synchronized.\n", FUNCNAME);

            return false;
        }

        std::string path = GAME_CAC_COSTUME_DESCS_PATH + xv2_lang_codes[i] + ".msg";

        if (!xv2fs->SaveFile(game_cac_costume_descs[i], path))
        {
            DPRINTF("Failed saving file \"%s\"\n", path.c_str());
            return false;
        }
    }    

    num_entries = game_accesory_descs[0]->GetNumEntries();

    for (size_t i = 0; i < XV2_LANG_NUM; i++)
    {
        if (i != 0 && game_accesory_descs[i]->GetNumEntries() != num_entries)
        {
            DPRINTF("%s: Failure in saving proper_noun_accessory_info_* files."
                    "This program requires the files for all languages to be synchronized.\n", FUNCNAME);

            return false;
        }

        std::string path = GAME_ACCESORY_DESCS_PATH + xv2_lang_codes[i] + ".msg";

        if (!xv2fs->SaveFile(game_accesory_descs[i], path))
        {
            DPRINTF("Failed saving file \"%s\"\n", path.c_str());
            return false;
        }
    }

    return true;
}

bool Xenoverse2::InitTalismanNames(int only_this_lang)
{
    if (!xv2fs)
        return false;

    for (MsgFile *msg : game_talisman_names)
    {
        if (msg)
            delete msg;
    }

    game_talisman_names.clear();
    game_talisman_names.resize(XV2_LANG_NUM);

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        if (only_this_lang >= 0 && i != only_this_lang)
            continue;

        std::string path = GAME_TALISMAN_NAMES_PATH + xv2_lang_codes[i] + ".msg";
        game_talisman_names[i] = new MsgFile();

        if (!xv2fs->LoadFile(game_talisman_names[i], path))
        {
            DPRINTF("Failed loading file \"%s\"\n", path.c_str());
            return false;
        }
    }

    return true;
}

bool Xenoverse2::InitMaterialNames(int only_this_lang)
{
    if (!xv2fs)
        return false;

    for (MsgFile *msg : game_material_names)
    {
        if (msg)
            delete msg;
    }

    game_material_names.clear();
    game_material_names.resize(XV2_LANG_NUM);

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        if (only_this_lang >= 0 && i != only_this_lang)
            continue;

        std::string path = GAME_MATERIAL_NAMES_PATH + xv2_lang_codes[i] + ".msg";
        game_material_names[i] = new MsgFile();

        if (!xv2fs->LoadFile(game_material_names[i], path))
        {
            DPRINTF("Failed loading file \"%s\"\n", path.c_str());
            return false;
        }
    }

    return true;
}

bool Xenoverse2::InitBattleNames(int only_this_lang)
{
    if (!xv2fs)
        return false;

    for (MsgFile *msg : game_battle_names)
    {
        if (msg)
            delete msg;
    }

    game_battle_names.clear();
    game_battle_names.resize(XV2_LANG_NUM);

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        if (only_this_lang >= 0 && i != only_this_lang)
            continue;

        std::string path = GAME_BATTLE_NAMES_PATH + xv2_lang_codes[i] + ".msg";
        game_battle_names[i] = new MsgFile();

        if (!xv2fs->LoadFile(game_battle_names[i], path))
        {
            DPRINTF("Failed loading file \"%s\"\n", path.c_str());
            return false;
        }
    }

    return true;
}

bool Xenoverse2::InitExtraNames(int only_this_lang)
{
    if (!xv2fs)
        return false;

    for (MsgFile *msg : game_extra_names)
    {
        if (msg)
            delete msg;
    }

    game_extra_names.clear();
    game_extra_names.resize(XV2_LANG_NUM);

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        if (only_this_lang >= 0 && i != only_this_lang)
            continue;

        std::string path = GAME_EXTRA_NAMES_PATH + xv2_lang_codes[i] + ".msg";
        game_extra_names[i] = new MsgFile();

        if (!xv2fs->LoadFile(game_extra_names[i], path))
        {
            DPRINTF("Failed loading file \"%s\"\n", path.c_str());
            return false;
        }
    }

    return true;
}

bool Xenoverse2::InitPetNames(int only_this_lang)
{
    if (!xv2fs)
        return false;

    for (MsgFile *msg : game_pet_names)
    {
        if (msg)
            delete msg;
    }

    game_pet_names.clear();
    game_pet_names.resize(XV2_LANG_NUM);

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        if (only_this_lang >= 0 && i != only_this_lang)
            continue;

        std::string path = GAME_PET_NAMES_PATH + xv2_lang_codes[i] + ".msg";
        game_pet_names[i] = new MsgFile();

        if (!xv2fs->LoadFile(game_pet_names[i], path))
        {
            DPRINTF("Failed loading file \"%s\"\n", path.c_str());
            return false;
        }
    }

    return true;
}

bool Xenoverse2::InitIdb(bool costumes, bool accesories, bool talisman, bool skills, bool material, bool battle, bool extra, bool pet)
{
    if (!xv2fs)
        return false;

    if (costumes)
    {
        if (game_top_idb)
            delete game_top_idb;        

        game_top_idb = new IdbFile();
        if (!xv2fs->LoadFile(game_top_idb, GAME_TOP_IDB_PATH))
        {
            delete game_top_idb; game_top_idb = nullptr;
            return false;
        }

        if (game_bottom_idb)
            delete game_bottom_idb;        

        game_bottom_idb = new IdbFile();
        if (!xv2fs->LoadFile(game_bottom_idb, GAME_BOTTOM_IDB_PATH))
        {
            delete game_bottom_idb; game_bottom_idb = nullptr;
            return false;
        }

        if (game_gloves_idb)
            delete game_gloves_idb;

        game_gloves_idb = new IdbFile();
        if (!xv2fs->LoadFile(game_gloves_idb, GAME_GLOVES_IDB_PATH))
        {
            delete game_gloves_idb; game_gloves_idb = nullptr;
            return false;
        }        

        if (game_shoes_idb)
            delete game_shoes_idb;

        game_shoes_idb = new IdbFile();
        if (!xv2fs->LoadFile(game_shoes_idb, GAME_SHOES_IDB_PATH))
        {
            delete game_shoes_idb; game_shoes_idb = nullptr;
            return false;
        }
    }

    if (accesories)
    {
        if (game_accesory_idb)
            delete game_accesory_idb;

        game_accesory_idb = new IdbFile();

        if (!xv2fs->LoadFile(game_accesory_idb, GAME_ACCESORY_IDB_PATH))
        {
            delete game_accesory_idb; game_accesory_idb = nullptr;
            return false;
        }
    }

    if (talisman)
    {
        if (game_talisman_idb)
            delete game_talisman_idb;

        game_talisman_idb = new IdbFile();

        if (!xv2fs->LoadFile(game_talisman_idb, GAME_TALISMAN_IDB_PATH))
        {
            delete game_talisman_idb; game_talisman_idb = nullptr;
            return false;
        }
    }

    if (skills)
    {
        if (game_skill_idb)
            delete game_skill_idb;

        game_skill_idb = new IdbFile();

        if (!xv2fs->LoadFile(game_skill_idb, GAME_SKILL_IDB_PATH))
        {
            delete game_skill_idb; game_skill_idb = nullptr;
            return false;
        }
    }

    if (material)
    {
        if (game_material_idb)
            delete game_material_idb;

        game_material_idb = new IdbFile();

        if (!xv2fs->LoadFile(game_material_idb, GAME_MATERIAL_IDB_PATH))
        {
            delete game_material_idb; game_material_idb = nullptr;
            return false;
        }
    }

    if (battle)
    {
        if (game_battle_idb)
            delete game_battle_idb;

        game_battle_idb = new IdbFile();

        if (!xv2fs->LoadFile(game_battle_idb, GAME_BATTLE_IDB_PATH))
        {
            delete game_battle_idb; game_battle_idb = nullptr;
            return false;
        }
    }

    if (extra)
    {
        if (game_extra_idb)
            delete game_extra_idb;

        game_extra_idb = new IdbFile();

        if (!xv2fs->LoadFile(game_extra_idb, GAME_EXTRA_IDB_PATH))
        {
            delete game_extra_idb; game_extra_idb = nullptr;
            return false;
        }
    }

    if (pet)
    {
        if (game_pet_idb)
            delete game_pet_idb;

        game_pet_idb = new IdbFile();

        if (!xv2fs->LoadFile(game_pet_idb, GAME_PET_IDB_PATH))
        {
            delete game_pet_idb; game_pet_idb = nullptr;
            return false;
        }
    }

    return true;
}

bool Xenoverse2::CommitIdb(bool costumes, bool accesories, bool talisman, bool skills)
{
    if (!xv2fs)
        return false;

    if (costumes)
    {
        if (!game_top_idb || !game_bottom_idb || !game_gloves_idb || !game_shoes_idb)
            return false;

        if (!xv2fs->SaveFile(game_top_idb, GAME_TOP_IDB_PATH))
        {
            DPRINTF("Saving of costume_top_item.idb failed.\n");
            return false;
        }

        if (!xv2fs->SaveFile(game_bottom_idb, GAME_BOTTOM_IDB_PATH))
        {
            DPRINTF("Saving of costume_bottom_item.idb failed.\n");
            return false;
        }

        if (!xv2fs->SaveFile(game_gloves_idb, GAME_GLOVES_IDB_PATH))
        {
            DPRINTF("Saving of costume_gloves_item.idb failed.\n");
            return false;
        }

        if (!xv2fs->SaveFile(game_shoes_idb, GAME_SHOES_IDB_PATH))
        {
            DPRINTF("Saving of costume_shoes_item.idb failed.\n");
            return false;
        }
    }

    if (accesories)
    {
        if (!game_accesory_idb)
            return false;

        if (!xv2fs->SaveFile(game_accesory_idb, GAME_ACCESORY_IDB_PATH))
        {
            DPRINTF("Saving of accesory_item.idb failed.\n");
            return false;
        }
    }

    if (talisman)
    {
        if (!game_talisman_idb)
            return false;

        if (!xv2fs->SaveFile(game_talisman_idb, GAME_TALISMAN_IDB_PATH))
        {
            DPRINTF("Saving of talisman_item.idb failed.\n");
            return false;
        }
    }

    if (skills)
    {
        if (!game_skill_idb)
            return false;

        if (!xv2fs->SaveFile(game_skill_idb, GAME_SKILL_IDB_PATH))
        {
            DPRINTF("Saving of skill_item.idb failed.\n");
            return false;
        }
    }

    return true;
}

bool Xenoverse2::InitLobby(bool tnl)
{
    if (!xv2fs)
        return false;

    if (game_pal)
        delete game_pal;

    game_pal = new PalFile();
    if (!xv2fs->LoadFile(game_pal, GAME_PAL_PATH))
        return false;

    if (tnl)
    {
        if (game_tnl)
            delete game_tnl;

        game_tnl = new TnlFile();
        if (!xv2fs->LoadFile(game_tnl, GAME_TNL_PATH))
            return false;
    }

    return true;
}

bool Xenoverse2::CommitLobby(bool tnl)
{
    if (!xv2fs || !game_pal)
        return false;

    if (!xv2fs->SaveFile(game_pal, GAME_PAL_PATH))
        return false;

    if (tnl)
    {
        if (!game_tnl)
            return false;

        if (!xv2fs->SaveFile(game_tnl, GAME_TNL_PATH))
            return false;
    }

    return true;
}

bool Xenoverse2::InitSound(bool load_sev_cmn)
{
    if (!xv2fs)
        return false;

    if (game_css_acb_jp)
        delete game_css_acb_jp;

    std::string acb_path;
    std::string awb_path;

    game_css_acb_jp = new AcbFile();
    acb_path = std::string(GAME_CSS_SOUND_JP_PATH) + ".acb";
    if (!xv2fs->LoadFile(dynamic_cast<CriwareAudioContainer *>(game_css_acb_jp), acb_path))
    {
        DPRINTF("%s: Failed to load %s\n", FUNCNAME, acb_path.c_str());
        delete game_css_acb_jp; game_css_acb_jp = nullptr;
        return false;
    }

    if (game_css_acb_en)
        delete game_css_acb_en;

    game_css_acb_en = new AcbFile();
    acb_path = std::string(GAME_CSS_SOUND_EN_PATH) + ".acb";
    if (!xv2fs->LoadFile(dynamic_cast<CriwareAudioContainer *>(game_css_acb_en), acb_path))
    {
        DPRINTF("%s: Failed to load %s\n", FUNCNAME, acb_path.c_str());
        delete game_css_acb_en; game_css_acb_en = nullptr;
        return false;
    }

    if (game_css_awb_jp)
        delete game_css_awb_jp;

    game_css_awb_jp = new Afs2File();
    awb_path = std::string(GAME_CSS_SOUND_JP_PATH) + ".awb";
    if (!xv2fs->LoadFile(game_css_awb_jp, awb_path))
    {
        DPRINTF("%s: Failed to load %s\n", FUNCNAME, awb_path.c_str());
        delete game_css_awb_jp; game_css_awb_jp = nullptr;
        return false;
    }

    if (game_css_awb_en)
        delete game_css_awb_en;

    game_css_awb_en = new Afs2File();
    awb_path = std::string(GAME_CSS_SOUND_EN_PATH) + ".awb";
    if (!xv2fs->LoadFile(game_css_awb_en, awb_path))
    {
        DPRINTF("%s: Failed to load %s\n", FUNCNAME, awb_path.c_str());
        delete game_css_awb_en; game_css_awb_en = nullptr;
        return false;
    }

    if (!load_sev_cmn)
        return true;

    if (game_sev_cmn_acb_jp)
        delete game_sev_cmn_acb_jp;

    game_sev_cmn_acb_jp = new AcbFile();
    acb_path = std::string(GAME_SEV_CMN_SOUND_JP_PATH) + ".acb";
    if (!xv2fs->LoadFile(dynamic_cast<CriwareAudioContainer *>(game_sev_cmn_acb_jp), acb_path))
    {
        DPRINTF("%s: Failed to load %s\n", FUNCNAME, acb_path.c_str());
        delete game_sev_cmn_acb_jp; game_sev_cmn_acb_jp = nullptr;
        return false;
    }

    if (game_sev_cmn_acb_en)
        delete game_sev_cmn_acb_en;

    game_sev_cmn_acb_en = new AcbFile();
    acb_path = std::string(GAME_SEV_CMN_SOUND_EN_PATH) + ".acb";
    if (!xv2fs->LoadFile(dynamic_cast<CriwareAudioContainer *>(game_sev_cmn_acb_en), acb_path))
    {
        DPRINTF("%s: Failed to load %s\n", FUNCNAME, acb_path.c_str());
        delete game_sev_cmn_acb_en; game_sev_cmn_acb_en = nullptr;
        return false;
    }

    if (game_sev_cmn_awb_jp)
        delete game_sev_cmn_awb_jp;

    game_sev_cmn_awb_jp = new Afs2File();
    awb_path = std::string(GAME_SEV_CMN_SOUND_JP_PATH) + ".awb";
    if (!xv2fs->LoadFile(game_sev_cmn_awb_jp, awb_path))
    {
        DPRINTF("%s: Failed to load %s\n", FUNCNAME, awb_path.c_str());
        delete game_sev_cmn_awb_jp; game_sev_cmn_awb_jp = nullptr;
        return false;
    }

    if (game_sev_cmn_awb_en)
        delete game_sev_cmn_awb_en;

    game_sev_cmn_awb_en = new Afs2File();
    awb_path = std::string(GAME_SEV_CMN_SOUND_EN_PATH) + ".awb";
    if (!xv2fs->LoadFile(game_sev_cmn_awb_en, awb_path))
    {
        DPRINTF("%s: Failed to load %s\n", FUNCNAME, awb_path.c_str());
        delete game_sev_cmn_awb_en; game_sev_cmn_awb_en = nullptr;
        return false;
    }

    if (game_sev_cmn2_acb_jp)
        delete game_sev_cmn2_acb_jp;

    game_sev_cmn2_acb_jp = new AcbFile();
    acb_path =  std::string(GAME_SEV_CMN_SOUND2_JP_PATH) + ".acb";
    if (!xv2fs->LoadFile(dynamic_cast<CriwareAudioContainer *>(game_sev_cmn2_acb_jp), acb_path))
    {
        DPRINTF("%s: Failed to load %s\n", FUNCNAME, acb_path.c_str());
        delete game_sev_cmn2_acb_jp; game_sev_cmn2_acb_jp = nullptr;
        return false;
    }

    if (game_sev_cmn2_acb_en)
        delete game_sev_cmn2_acb_en;

    game_sev_cmn2_acb_en = new AcbFile();
    acb_path = std::string(GAME_SEV_CMN_SOUND2_EN_PATH) + ".acb";
    if (!xv2fs->LoadFile(dynamic_cast<CriwareAudioContainer *>(game_sev_cmn2_acb_en), acb_path))
    {
        DPRINTF("%s: Failed to load %s\n", FUNCNAME, acb_path.c_str());
        delete game_sev_cmn2_acb_en; game_sev_cmn2_acb_en = nullptr;
        return false;
    }

    if (game_sev_cmn2_awb_jp)
        delete game_sev_cmn2_awb_jp;

    game_sev_cmn2_awb_jp = new Afs2File();
    awb_path = std::string(GAME_SEV_CMN_SOUND2_JP_PATH) + ".awb";
    if (!xv2fs->LoadFile(game_sev_cmn2_awb_jp, awb_path))
    {
        DPRINTF("%s: Failed to load %s\n", FUNCNAME, awb_path.c_str());
        delete game_sev_cmn2_awb_jp; game_sev_cmn2_awb_jp = nullptr;
        return false;
    }

    if (game_sev_cmn2_awb_en)
        delete game_sev_cmn2_awb_en;

    game_sev_cmn2_awb_en = new Afs2File();
    awb_path = std::string(GAME_SEV_CMN_SOUND2_EN_PATH) + ".awb";
    if (!xv2fs->LoadFile(game_sev_cmn2_awb_en, awb_path))
    {
        DPRINTF("%s: Failed to load %s\n", FUNCNAME, awb_path.c_str());
        delete game_sev_cmn2_awb_en; game_sev_cmn2_awb_en = nullptr;
        return false;
    }

    return true;
}

bool Xenoverse2::InitBgm()
{
    if (!xv2fs)
        return false;

    if (game_bgm_acb)
        delete game_bgm_acb;

    std::string acb_path;
    std::string awb_path;

    game_bgm_acb = new AcbFile();
    acb_path = std::string(GAME_BGM_PATH) + ".acb";
    if (!xv2fs->LoadFile(dynamic_cast<CriwareAudioContainer *>(game_bgm_acb), acb_path))
    {
        DPRINTF("%s: Failed to load %s\n", FUNCNAME, acb_path.c_str());
        delete game_bgm_acb; game_bgm_acb = nullptr;
        return false;
    }

    if (game_bgm_awb)
        delete game_bgm_awb;

    game_bgm_awb = new Afs2File();
    awb_path = std::string(GAME_BGM_PATH) + ".awb";
    if (!xv2fs->LoadFile(game_bgm_awb, awb_path))
    {
        DPRINTF("%s: Failed to load %s\n", FUNCNAME, awb_path.c_str());
        delete game_bgm_awb; game_bgm_awb = nullptr;
        return false;
    }

    return true;
}


bool Xenoverse2::SetAcbAwbData(AcbFile *acb, AwbFile *awb)
{
    if (acb->HasAwbHeader())
    {
        unsigned int header_size;
        uint8_t *awb_header = awb->CreateHeader(&header_size);
        if (!awb_header)
        {
            DPRINTF("%s: Failed to create awb header.\n", FUNCNAME);
            return false;
        }

        if (!acb->SetAwbHeader(awb_header, header_size, true))
        {
            delete[] awb_header;
            DPRINTF("Failed to set awb header in .acb.\n");
            return false;
        }
    }

    uint8_t md5[16];
    size_t size;
    uint8_t *buf = awb->Save(&size);

    if (!buf)
    {
        DPRINTF("%s: cannot get awb md5 because save file failed.\n", FUNCNAME);
        return false;
    }

    Utils::Md5(buf, (uint32_t)size, md5);
    acb->SetExternalAwbHash(md5);

    return true;
}

bool Xenoverse2::CommitSound(bool css, bool sev, bool sev1, bool sev2)
{
    if (!xv2fs)
        return false;

    if (css)
    {
        if (!game_css_acb_jp || !game_css_acb_en || !game_css_awb_jp || !game_css_awb_en)
            return true; // Now we return true

        if (!SetAcbAwbData(game_css_acb_jp, game_css_awb_jp))
            return false;

        if (!SetAcbAwbData(game_css_acb_en, game_css_awb_en))
            return false;

        if (!xv2fs->SaveFile(dynamic_cast<CriwareAudioContainer *>(game_css_acb_jp), std::string(GAME_CSS_SOUND_JP_PATH) + ".acb"))
            return false;

        if (!xv2fs->SaveFile(dynamic_cast<CriwareAudioContainer *>(game_css_acb_en), std::string(GAME_CSS_SOUND_EN_PATH) + ".acb"))
            return false;

        if (!xv2fs->SaveFile(game_css_awb_jp, std::string(GAME_CSS_SOUND_JP_PATH) + ".awb"))
            return false;

        if (!xv2fs->SaveFile(game_css_awb_en, std::string(GAME_CSS_SOUND_EN_PATH) + ".awb"))
            return false;
    }

    if (sev && sev_audio_changed)
    {
        if (!sev1 && !sev2)
            return true;

        if (!game_sev_cmn_acb_jp || !game_sev_cmn_acb_en || !game_sev_cmn_awb_jp || !game_sev_cmn_awb_en)
            return true; // Now we return true

        if (sev1)
        {
            if (!SetAcbAwbData(game_sev_cmn_acb_jp, game_sev_cmn_awb_jp))
                return false;

            if (!SetAcbAwbData(game_sev_cmn_acb_en, game_sev_cmn_awb_en))
                return false;

            if (!xv2fs->SaveFile(dynamic_cast<CriwareAudioContainer *>(game_sev_cmn_acb_jp), std::string(GAME_SEV_CMN_SOUND_JP_PATH) + ".acb"))
                return false;

            if (!xv2fs->SaveFile(dynamic_cast<CriwareAudioContainer *>(game_sev_cmn_acb_en), std::string(GAME_SEV_CMN_SOUND_EN_PATH) + ".acb"))
                return false;

            if (!xv2fs->SaveFile(game_sev_cmn_awb_jp, std::string(GAME_SEV_CMN_SOUND_JP_PATH) + ".awb"))
                return false;

            if (!xv2fs->SaveFile(game_sev_cmn_awb_en, std::string(GAME_SEV_CMN_SOUND_EN_PATH) + ".awb"))
                return false;
        }

        if (sev2)
        {
            if (!SetAcbAwbData(game_sev_cmn2_acb_jp, game_sev_cmn2_awb_jp))
                return false;

            if (!SetAcbAwbData(game_sev_cmn2_acb_en, game_sev_cmn2_awb_en))
                return false;

            if (!xv2fs->SaveFile(dynamic_cast<CriwareAudioContainer *>(game_sev_cmn2_acb_jp), std::string(GAME_SEV_CMN_SOUND2_JP_PATH) + ".acb"))
                return false;

            if (!xv2fs->SaveFile(dynamic_cast<CriwareAudioContainer *>(game_sev_cmn2_acb_en), std::string(GAME_SEV_CMN_SOUND2_EN_PATH) + ".acb"))
                return false;

            if (!xv2fs->SaveFile(game_sev_cmn2_awb_jp, std::string(GAME_SEV_CMN_SOUND2_JP_PATH) + ".awb"))
                return false;

            if (!xv2fs->SaveFile(game_sev_cmn2_awb_en, std::string(GAME_SEV_CMN_SOUND2_EN_PATH) + ".awb"))
                return false;
        }
    }

    return true;
}

bool Xenoverse2::CommitBgm()
{
    if (!xv2fs)
        return false;

    if (!bgm_changed || !game_bgm_acb)
        return true;

    if (!SetAcbAwbData(game_bgm_acb, game_bgm_awb))
        return false;

    if (!xv2fs->SaveFile(dynamic_cast<CriwareAudioContainer *>(game_bgm_acb), std::string(GAME_BGM_PATH) + ".acb"))
        return false;

    if (!xv2fs->SaveFile(game_bgm_awb, std::string(GAME_BGM_PATH) + ".awb"))
        return false;

    bgm_changed = false;
    return true;
}

bool Xenoverse2::InitCac()
{
    if (!xv2fs)
        return false;

    if (game_hum_bcs)
        delete game_hum_bcs;

    game_hum_bcs = new BcsFile();
    if (!xv2fs->LoadFile(game_hum_bcs, GAME_HUM_BCS_PATH))
        return false;

    if (game_huf_bcs)
        delete game_huf_bcs;

    game_huf_bcs = new BcsFile();
    if (!xv2fs->LoadFile(game_huf_bcs, GAME_HUF_BCS_PATH))
        return false;

    if (game_nmc_bcs)
        delete game_nmc_bcs;

    game_nmc_bcs = new BcsFile();
    if (!xv2fs->LoadFile(game_nmc_bcs, GAME_NMC_BCS_PATH))
        return false;

    if (game_fri_bcs)
        delete game_fri_bcs;

    game_fri_bcs = new BcsFile();
    if (!xv2fs->LoadFile(game_fri_bcs, GAME_FRI_BCS_PATH))
        return false;

    if (game_mam_bcs)
        delete game_mam_bcs;

    game_mam_bcs = new BcsFile();
    if (!xv2fs->LoadFile(game_mam_bcs, GAME_MAM_BCS_PATH))
        return false;

    if (game_maf_bcs)
        delete game_maf_bcs;

    game_maf_bcs = new BcsFile();
    if (!xv2fs->LoadFile(game_maf_bcs, GAME_MAF_BCS_PATH))
        return false;

    return true;
}

bool Xenoverse2::CommitCac()
{
    if (!xv2fs || !game_hum_bcs || !game_huf_bcs || !game_nmc_bcs || !game_fri_bcs || !game_mam_bcs || !game_maf_bcs)
        return false;

    if (!xv2fs->SaveFile(game_hum_bcs, GAME_HUM_BCS_PATH))
        return false;

    if (!xv2fs->SaveFile(game_huf_bcs, GAME_HUF_BCS_PATH))
        return false;

    if (!xv2fs->SaveFile(game_nmc_bcs, GAME_NMC_BCS_PATH))
        return false;

    if (!xv2fs->SaveFile(game_fri_bcs, GAME_FRI_BCS_PATH))
        return false;

    if (!xv2fs->SaveFile(game_mam_bcs, GAME_MAM_BCS_PATH))
        return false;

    if (!xv2fs->SaveFile(game_maf_bcs, GAME_MAF_BCS_PATH))
        return false;

    return true;
}

bool Xenoverse2::InitCostumeFile()
{
    if (!xv2fs)
        return false;

    if (game_costume_file)
        delete game_costume_file;

    game_costume_file = new X2mCostumeFile();
    xv2fs->CompileFile(game_costume_file, GAME_COSTUME_FILE_PATH); // Ignore the error, as file may not exist

    return true;
}

bool Xenoverse2::CommitCostumeFile()
{
    if (!xv2fs || !game_costume_file)
        return false;

    return xv2fs->DecompileFile(game_costume_file, GAME_COSTUME_FILE_PATH);
}

bool Xenoverse2::GetCharaCodeFromId(uint32_t id, std::string &code)
{
    if (!game_cms)
        return false;

    for (CmsEntry *entry : *game_cms)
    {
        CmsEntryXV2 *xv2_entry = dynamic_cast<CmsEntryXV2 *>(entry);
        if (!xv2_entry)
            continue;

        if (xv2_entry->id == id)
        {
            code = xv2_entry->name;
            return true;
        }
    }

    return false;
}

bool Xenoverse2::IsOriginalChara(const std::string &short_name)
{
    //if (original_chars.size() != 154)
    //    DPRINTF("Correct the list %Id!", original_chars.size());

    std::string upper_name = Utils::ToUpperCase(short_name);
    return (std::find(original_chars.begin(), original_chars.end(), upper_name) != original_chars.end());
}

bool Xenoverse2::IsForbiddenNewStageName(const std::string &name)
{
    std::string upper_name = Utils::ToUpperCase(name);
    return (std::find(forbidden_names_new_stage.begin(),forbidden_names_new_stage.end(), upper_name) != forbidden_names_new_stage.end());
}

static bool InjectSwfAndDelete(const std::string &path)
{
    SwfFile swf;

    bool ret = swf.LoadFromFile(path);
    Utils::RemoveFile(path);

    if (!ret)
        return false;

    uint8_t *abc = nullptr;
    uint32_t abc_size = 0;

    for (uint32_t i = 0; i < swf.GetNumBlocks(); i++)
    {
        SwfDoABC *abc_block = dynamic_cast<SwfDoABC *>(swf.GetBlock(i));

        if (abc_block)
        {
            abc_size = (uint32_t)abc_block->abc.size();
            abc = new uint8_t[abc_size];
            memcpy(abc, abc_block->abc.data(), abc_size);
            break;
        }
    }

    if (!abc)
    {
        DPRINTF("That swf file doesn't have ActionSript 3 data.\n");
        return false;
    }

    ret = charasele->SetAbcBlob(abc, abc_size);
    delete[] abc;
    return ret;
}

bool Xenoverse2::CompileCharaSel(const std::string &chasel_path, const std::string &compiler)
{
    std::vector<std::string> args;

    //if (!chara_list || !charasele)
    if (!charasele)
        return false;

    /*if (!chara_list->SaveToFile(chasel_path + CHARALIST_PATH))
        return false;*/

    args.push_back("-compiler.source-path=" + Utils::WindowsPath(chasel_path));
    args.push_back("-omit-trace-statements=false");
    args.push_back("-compress=false");
    args.push_back(Utils::WindowsPath(chasel_path + TIMELINE_PATH));

    int ret = Utils::RunProgram(Utils::WindowsPath(compiler), args);
    if (ret != 0)
    {
        DPRINTF("%s: Compiler failed. Ret=%d\n"
                "A ret of -1 may indicate a non existing compiler.\n"
                "A ret of 1 may indicate an error in the compile process.\n"
                "A ret of 2 may indicate that 32 bits version of Java is not installed.\n"
                "A ret of 6 may indicate problems between the compiler and the java installation.\n"
                "Compiler path=%s\n"
                "chasel_path (relative) = %s\n\n"
                "arg[1]=%s\n"
                "arg[2]=%s\n"
                "arg[3]=%s\n"
                "arg[4]=%s\n"
                "Current directory=%s\n\n"
                "See the troubleshooting guide at installer thread.\n", FUNCNAME, ret,
                Utils::WindowsPath(compiler).c_str(), Utils::WindowsPath(chasel_path).c_str(),
                args[0].c_str(),
                args[1].c_str(),
                args[2].c_str(),
                args[3].c_str(),
                _getcwd(nullptr, MAX_PATH));
        return false;
    }   

    const std::string comp_result = chasel_path + COMPRESULT_PATH;

    if (!Utils::FileExists(comp_result))
    {
        DPRINTF("%s: swf doesn't exist after compile.\n", FUNCNAME);
        return false;
    }

    // Better do this in a separate function, so that installer can undo things easily if something fails
    /*if (!chara_list->SaveToFile(chalist_path, true, true))
        return false;*/

    return InjectSwfAndDelete(comp_result);
}

bool Xenoverse2::GetCharaName(const std::string &code, std::string &name, int lang, int index)
{
    if (!xv2fs || game_chara_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "chara_" + code + "_" + Utils::ToStringAndPad(index, 3);
    MsgEntry *entry = game_chara_names[lang]->FindEntryByName(entry_name);
    if (!entry && index != 0)
    {
        entry_name = "chara_" + code + "_000";
        entry = game_chara_names[lang]->FindEntryByName(entry_name);
    }

    if (!entry)
        return false;

    name = UnescapeHtml(entry->lines[0]);
    return true;
}

bool Xenoverse2::SetCharaName(const std::string &code, const std::string &name, int lang, int index)
{
    if (!xv2fs || game_chara_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "chara_" + code + "_" + Utils::ToStringAndPad(index, 3);
    MsgEntry *entry = game_chara_names[lang]->FindEntryByName(entry_name);

    if (entry)
    {
        entry->lines.resize(1);
        entry->lines[0] = EscapeHtml(name);
    }
    else
    {
        MsgEntry entry;

        entry.name = entry_name;
        entry.lines.push_back(EscapeHtml(name));

        if (!game_chara_names[lang]->AddEntry(entry, true))
            return false;
    }

    return true;
}

bool Xenoverse2::RemoveCharaName(const std::string &code, int lang, int index)
{
    if (!xv2fs || game_chara_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "chara_" + code + "_" + Utils::ToStringAndPad(index, 3);
    return game_chara_names[lang]->RemoveEntry(entry_name);
}

bool Xenoverse2::RemoveAllCharaName(const std::string &code, int lang)
{
    for (int i = 0; i < 1000; i++)
    {
        if (!RemoveCharaName(code, lang, i))
            return false;
    }

    return true;
}

bool Xenoverse2::GetCharaCostumeName(const std::string &code, int var, int model_preset, std::string &name, int lang)
{
    if (!xv2fs || game_chara_costume_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "variation_" + code + "_" + Utils::ToStringAndPad(var, 3) + "_" + Utils::ToStringAndPad(model_preset, 3);
    MsgEntry *entry = game_chara_costume_names[lang]->FindEntryByName(entry_name);

    if (!entry)
        return false;

    name = UnescapeHtml(entry->lines[0]);
    return true;
}

bool Xenoverse2::SetCharaCostumeName(const std::string &code, int var, int model_preset, const std::string &name, int lang)
{
    if (!xv2fs || game_chara_costume_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "variation_" + code + "_" + Utils::ToStringAndPad(var, 3) + "_" + Utils::ToStringAndPad(model_preset, 3);
    MsgEntry *entry = game_chara_costume_names[lang]->FindEntryByName(entry_name);

    if (entry)
    {
        entry->lines.resize(1);
        entry->lines[0] = EscapeHtml(name);
    }
    else
    {
        MsgEntry entry;

        entry.name = entry_name;
        entry.lines.push_back(EscapeHtml(name));

        if (!game_chara_costume_names[lang]->AddEntry(entry, true))
            return false;
    }

    return true;
}

bool Xenoverse2::RemoveCharaCostumeName(const std::string &code, int var, int model_preset, int lang)
{
    if (!xv2fs || game_chara_costume_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "variation_" + code + "_" + Utils::ToStringAndPad(var, 3) + "_" + Utils::ToStringAndPad(model_preset, 3);
    return game_chara_costume_names[lang]->RemoveEntry(entry_name);
}

bool Xenoverse2::GetSuperSkillName(uint16_t name_id, std::string &name, int lang)
{
    if (!xv2fs || sup_sk_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "spe_skill_" + Utils::ToStringAndPad(name_id, 4);
    MsgEntry *entry = sup_sk_names[lang]->FindEntryByName(entry_name);

    if (!entry)
        return false;

    name = UnescapeHtml(entry->lines[0]);
    return true;
}

bool Xenoverse2::GetUltimateSkillName(uint16_t name_id, std::string &name, int lang)
{
    if (!xv2fs || ult_sk_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "ult_" + Utils::ToStringAndPad(name_id, 4);
    MsgEntry *entry = ult_sk_names[lang]->FindEntryByName(entry_name);

    if (!entry)
        return false;

    name = UnescapeHtml(entry->lines[0]);
    return true;
}

bool Xenoverse2::GetEvasiveSkillName(uint16_t name_id, std::string &name, int lang)
{
    if (!xv2fs || eva_sk_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "avoid_skill_" + Utils::ToStringAndPad(name_id, 4);
    MsgEntry *entry = eva_sk_names[lang]->FindEntryByName(entry_name);

    if (!entry)
        return false;

    name = UnescapeHtml(entry->lines[0]);
    return true;
}

bool Xenoverse2::GetAwakenSkillName(uint16_t name_id, std::string &name, int lang)
{
    if (!xv2fs || awa_sk_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "met_skill_" + Utils::ToStringAndPad(name_id, 4);
    MsgEntry *entry = awa_sk_names[lang]->FindEntryByName(entry_name);

    if (!entry)
        return false;

    name = UnescapeHtml(entry->lines[0]);
    return true;
}

bool Xenoverse2::SetSuperSkillName(uint16_t name_id, const std::string &name, int lang)
{
    if (!xv2fs || sup_sk_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "spe_skill_" + Utils::ToStringAndPad(name_id, 4);
    MsgEntry *entry = sup_sk_names[lang]->FindEntryByName(entry_name);

    if (entry)
    {
        entry->lines.resize(1);
        entry->lines[0] = EscapeHtml(name);
    }
    else
    {
        MsgEntry entry;

        entry.name = entry_name;
        entry.lines.push_back(EscapeHtml(name));

        if (!sup_sk_names[lang]->AddEntry(entry, true))
            return false;
    }

    return true;
}

bool Xenoverse2::SetUltimateSkillName(uint16_t name_id, const std::string &name, int lang)
{
    if (!xv2fs || ult_sk_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "ult_" + Utils::ToStringAndPad(name_id, 4);
    MsgEntry *entry = ult_sk_names[lang]->FindEntryByName(entry_name);

    if (entry)
    {
        entry->lines.resize(1);
        entry->lines[0] = EscapeHtml(name);
    }
    else
    {
        MsgEntry entry;

        entry.name = entry_name;
        entry.lines.push_back(EscapeHtml(name));

        if (!ult_sk_names[lang]->AddEntry(entry, true))
            return false;
    }

    return true;
}

bool Xenoverse2::SetEvasiveSkillName(uint16_t name_id, const std::string &name, int lang)
{
    if (!xv2fs || eva_sk_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "avoid_skill_" + Utils::ToStringAndPad(name_id, 4);
    MsgEntry *entry = eva_sk_names[lang]->FindEntryByName(entry_name);

    if (entry)
    {
        entry->lines.resize(1);
        entry->lines[0] = EscapeHtml(name);
    }
    else
    {
        MsgEntry entry;

        entry.name = entry_name;
        entry.lines.push_back(EscapeHtml(name));

        if (!eva_sk_names[lang]->AddEntry(entry, true))
            return false;
    }

    return true;
}

bool Xenoverse2::SetAwakenSkillName(uint16_t name_id, const std::string &name, int lang)
{
    if (!xv2fs || awa_sk_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "met_skill_" + Utils::ToStringAndPad(name_id, 4);
    MsgEntry *entry = awa_sk_names[lang]->FindEntryByName(entry_name);

    if (entry)
    {
        entry->lines.resize(1);
        entry->lines[0] = EscapeHtml(name);
    }
    else
    {
        MsgEntry entry;

        entry.name = entry_name;
        entry.lines.push_back(EscapeHtml(name));

        if (!awa_sk_names[lang]->AddEntry(entry, true))
            return false;
    }

    return true;
}

bool Xenoverse2::RemoveSuperSkillName(uint16_t name_id, int lang)
{
    if (!xv2fs || sup_sk_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "spe_skill_" + Utils::ToStringAndPad(name_id, 4);
    return sup_sk_names[lang]->RemoveEntry(entry_name);
}

bool Xenoverse2::RemoveUltimateSkillName(uint16_t name_id, int lang)
{
    if (!xv2fs || ult_sk_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "ult_" + Utils::ToStringAndPad(name_id, 4);
    return ult_sk_names[lang]->RemoveEntry(entry_name);
}

bool Xenoverse2::RemoveEvasiveSkillName(uint16_t name_id, int lang)
{
    if (!xv2fs || eva_sk_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "avoid_skill_" + Utils::ToStringAndPad(name_id, 4);
    return eva_sk_names[lang]->RemoveEntry(entry_name);
}

bool Xenoverse2::RemoveAwakenSkillName(uint16_t name_id, int lang)
{
    if (!xv2fs || awa_sk_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "met_skill_" + Utils::ToStringAndPad(name_id, 4);
    return awa_sk_names[lang]->RemoveEntry(entry_name);
}

bool Xenoverse2::GetSuperSkillDesc(uint16_t name_id, std::string &desc, int lang)
{
    if (!xv2fs || sup_sk_descs.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "spe_skill_eff_" + Utils::ToStringAndPad(name_id, 4);
    MsgEntry *entry = sup_sk_descs[lang]->FindEntryByName(entry_name);

    if (!entry)
        return false;

    desc = UnescapeHtml(entry->lines[0]);
    return true;
}

bool Xenoverse2::GetUltimateSkillDesc(uint16_t name_id, std::string &desc, int lang)
{
    if (!xv2fs || ult_sk_descs.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "ult_eff_" + Utils::ToStringAndPad(name_id, 4);
    MsgEntry *entry = ult_sk_descs[lang]->FindEntryByName(entry_name);

    if (!entry)
        return false;

    desc = UnescapeHtml(entry->lines[0]);
    return true;
}

bool Xenoverse2::GetEvasiveSkillDesc(uint16_t name_id, std::string &desc, int lang)
{
    if (!xv2fs || eva_sk_descs.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "avoid_skill_eff_" + Utils::ToStringAndPad(name_id, 4);
    MsgEntry *entry = eva_sk_descs[lang]->FindEntryByName(entry_name);

    if (!entry)
        return false;

    desc = UnescapeHtml(entry->lines[0]);
    return true;
}

bool Xenoverse2::GetAwakenSkillDesc(uint16_t name_id, std::string &desc, int lang)
{
    if (!xv2fs || awa_sk_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "met_skill_eff_" + Utils::ToStringAndPad(name_id, 4);
    MsgEntry *entry = awa_sk_descs[lang]->FindEntryByName(entry_name);

    if (!entry)
        return false;

    desc = UnescapeHtml(entry->lines[0]);
    return true;
}

bool Xenoverse2::SetSuperSkillDesc(uint16_t name_id, const std::string &desc, int lang)
{
    if (!xv2fs || sup_sk_descs.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "spe_skill_eff_" + Utils::ToStringAndPad(name_id, 4);
    MsgEntry *entry = sup_sk_descs[lang]->FindEntryByName(entry_name);

    if (entry)
    {
        entry->lines.resize(1);
        entry->lines[0] = EscapeHtml(desc);
    }
    else
    {
        MsgEntry entry;

        entry.name = entry_name;
        entry.lines.push_back(EscapeHtml(desc));

        if (!sup_sk_descs[lang]->AddEntry(entry, true))
            return false;
    }

    return true;
}

bool Xenoverse2::SetUltimateSkillDesc(uint16_t name_id, const std::string &desc, int lang)
{
    if (!xv2fs || ult_sk_descs.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "ult_eff_" + Utils::ToStringAndPad(name_id, 4);
    MsgEntry *entry = ult_sk_descs[lang]->FindEntryByName(entry_name);

    if (entry)
    {
        entry->lines.resize(1);
        entry->lines[0] = EscapeHtml(desc);
    }
    else
    {
        MsgEntry entry;

        entry.name = entry_name;
        entry.lines.push_back(EscapeHtml(desc));

        if (!ult_sk_descs[lang]->AddEntry(entry, true))
            return false;
    }

    return true;
}

bool Xenoverse2::SetEvasiveSkillDesc(uint16_t name_id, const std::string &desc, int lang)
{
    if (!xv2fs || eva_sk_descs.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "avoid_skill_eff_" + Utils::ToStringAndPad(name_id, 4);
    MsgEntry *entry = eva_sk_descs[lang]->FindEntryByName(entry_name);

    if (entry)
    {
        entry->lines.resize(1);
        entry->lines[0] = EscapeHtml(desc);
    }
    else
    {
        MsgEntry entry;

        entry.name = entry_name;
        entry.lines.push_back(EscapeHtml(desc));

        if (!eva_sk_descs[lang]->AddEntry(entry, true))
            return false;
    }

    return true;
}

bool Xenoverse2::SetAwakenSkillDesc(uint16_t name_id, const std::string &desc, int lang)
{
    if (!xv2fs || awa_sk_descs.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "met_skill_eff_" + Utils::ToStringAndPad(name_id, 4);
    MsgEntry *entry = awa_sk_descs[lang]->FindEntryByName(entry_name);

    if (entry)
    {
        entry->lines.resize(1);
        entry->lines[0] = EscapeHtml(desc);
    }
    else
    {
        MsgEntry entry;

        entry.name = entry_name;
        entry.lines.push_back(EscapeHtml(desc));

        if (!awa_sk_descs[lang]->AddEntry(entry, true))
            return false;
    }

    return true;
}

bool Xenoverse2::RemoveSuperSkillDesc(uint16_t name_id, int lang)
{
    if (!xv2fs || sup_sk_descs.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "spe_skill_eff_" + Utils::ToStringAndPad(name_id, 4);
    return sup_sk_descs[lang]->RemoveEntry(entry_name);
}

bool Xenoverse2::RemoveUltimateSkillDesc(uint16_t name_id, int lang)
{
    if (!xv2fs || ult_sk_descs.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "ult_eff_" + Utils::ToStringAndPad(name_id, 4);
    return ult_sk_descs[lang]->RemoveEntry(entry_name);
}

bool Xenoverse2::RemoveEvasiveSkillDesc(uint16_t name_id, int lang)
{
    if (!xv2fs || eva_sk_descs.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "avoid_skill_eff_" + Utils::ToStringAndPad(name_id, 4);
    return eva_sk_descs[lang]->RemoveEntry(entry_name);
}

bool Xenoverse2::RemoveAwakenSkillDesc(uint16_t name_id, int lang)
{
    if (!xv2fs || awa_sk_descs.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "met_skill_eff_" + Utils::ToStringAndPad(name_id, 4);
    return awa_sk_descs[lang]->RemoveEntry(entry_name);
}

bool Xenoverse2::GetBtlHudAwakenName(uint16_t name_id, uint16_t trans_stage, std::string &name, int lang)
{
    if (!xv2fs || quest_btlhud_texts.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "BHD_MET_" + Utils::ToStringAndPad(name_id, 4) + "_" + Utils::ToString(trans_stage);
    MsgEntry *entry = quest_btlhud_texts[lang]->FindEntryByName(entry_name);

    if (!entry)
        return false;

    name = UnescapeHtml(entry->lines[0]);
    return true;
}

bool Xenoverse2::SetBtlHudAwakenName(uint16_t name_id, uint16_t trans_stage, const std::string &name, int lang)
{
    if (!xv2fs || quest_btlhud_texts.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "BHD_MET_" + Utils::ToStringAndPad(name_id, 4) + "_" + Utils::ToString(trans_stage);
    MsgEntry *entry = quest_btlhud_texts[lang]->FindEntryByName(entry_name);

    if (entry)
    {
        entry->lines.resize(1);
        entry->lines[0] = EscapeHtml(name);
    }
    else
    {
        MsgEntry entry;

        entry.name = entry_name;
        entry.lines.push_back(EscapeHtml(name));

        if (!quest_btlhud_texts[lang]->AddEntry(entry, true))
            return false;
    }

    return true;
}

bool Xenoverse2::RemoveBtlHudAwakenName(uint16_t name_id, uint16_t trans_stage, int lang)
{
    if (!xv2fs || quest_btlhud_texts.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "BHD_MET_" + Utils::ToStringAndPad(name_id, 4) + "_" + Utils::ToString(trans_stage);
    return quest_btlhud_texts[lang]->RemoveEntry(entry_name);
}

uint8_t *Xenoverse2::GetSelPortrait(const std::string &name, size_t *psize)
{
    if (!game_sel_port)
        return nullptr;

    uint16_t idx = game_sel_port->FindIndexByName(name);
    if (idx == 0xFFFF)
        return nullptr;

    const EmbContainedFile &file = (*game_sel_port)[idx];

    *psize = file.GetSize();
    uint8_t *buf = new uint8_t[*psize];

    memcpy(buf, file.GetData(), *psize);
    return buf;
}

bool Xenoverse2::SetSelPortrait(const std::string &name, const uint8_t *buf, size_t size)
{
    if (!game_sel_port)
        return false;

    uint16_t idx = game_sel_port->FindIndexByName(name);
    if (idx != 0xFFFF)
        return game_sel_port->ReplaceFile(idx, buf, size);

    game_sel_port->AppendFile(buf, size, name);
    return true;
}

bool Xenoverse2::RemoveSelPortrait(const std::string &name)
{
    if (!game_sel_port)
        return false;

    uint16_t idx = game_sel_port->FindIndexByName(name);
    if (idx == 0xFFFF)
        return true;

    game_sel_port->RemoveFile(idx);
    return true;
}

bool Xenoverse2::GetLobbyName(uint32_t name_id, std::string &name, int lang)
{
    if (!xv2fs || game_lobby_texts.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "name_" + Utils::ToStringAndPad(name_id, 4);
    MsgEntry *entry = game_lobby_texts[lang]->FindEntryByName(entry_name);

    if (!entry)
        return false;

    name = UnescapeHtml(entry->lines[0]);
    return true;
}

bool Xenoverse2::SetLobbyName(uint32_t name_id, const std::string &name, int lang)
{
    if (!xv2fs || game_lobby_texts.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "name_" + Utils::ToStringAndPad(name_id, 4);
    MsgEntry *entry = game_lobby_texts[lang]->FindEntryByName(entry_name);

    if (entry)
    {
        entry->lines.resize(1);
        entry->lines[0] = EscapeHtml(name);
    }
    else
    {
        MsgEntry entry;

        entry.name = entry_name;
        entry.lines.push_back(EscapeHtml(name));

        if (!game_lobby_texts[lang]->AddEntry(entry, true))
            return false;
    }

    return true;
}

bool Xenoverse2::RemoveLobbyName(uint32_t name_id, int lang)
{
    if (!xv2fs || game_lobby_texts.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    std::string entry_name = "name_" + Utils::ToStringAndPad(name_id, 4);
    return game_lobby_texts[lang]->RemoveEntry(entry_name);
}

bool Xenoverse2::GetCacCostumeName(uint32_t name_idx, std::string &name, int lang)
{
    if (!xv2fs || game_cac_costume_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    if (name_idx >= game_cac_costume_names[lang]->GetNumEntries())
        return false;

    const MsgEntry &entry = (*game_cac_costume_names[lang])[name_idx];

    name = UnescapeHtml(entry.lines[0]);
    return true;
}

bool Xenoverse2::SetCacCostumeName(uint32_t name_idx, const std::string &name, int lang)
{
    if (!xv2fs || game_cac_costume_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    if (name_idx >= game_cac_costume_names[lang]->GetNumEntries())
        return false;

    MsgEntry &entry = (*game_cac_costume_names[lang])[name_idx];
    entry.lines.resize(1);
    entry.lines[0] = EscapeHtml(name);

    return true;
}

bool Xenoverse2::AddCacCostumeName(const std::string &entry_name, const std::string &name, int lang, uint16_t *ret_idx)
{
    if (!xv2fs || game_cac_costume_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    size_t idx = game_cac_costume_names[lang]->GetNumEntries();
    if (idx >= 0xFFFF)
    {
        DPRINTF("%s: Overflow of ids.\n", FUNCNAME);
        return false;
    }

    if (ret_idx)
        *ret_idx = (uint16_t)idx;

    MsgEntry entry;
    entry.lines.resize(1);
    entry.lines[0] = EscapeHtml(name);

    for (int i = 300; ; i++)
    {
        std::string final_entry_name = entry_name + Utils::ToString(i);

        if (!game_cac_costume_names[lang]->FindEntryByName(final_entry_name))
        {
            entry.name = final_entry_name;
            return game_cac_costume_names[lang]->AddEntry(entry, true);
        }
    }

    return false; // should never be here
}

static bool UpdateIdbNames(IdbFile *idb, uint32_t name_idx)
{
    for (IdbEntry &entry : *idb)
    {
        if (entry.name_id > name_idx)
            entry.name_id--;

        else if (entry.name_id == name_idx)
        {
            DPRINTF("%s: Error, name reference should have been removed from the idb first.\n", FUNCNAME);
            return false;
        }
    }

    return true;
}

static bool UpdateIdbDescs(IdbFile *idb, uint32_t desc_idx)
{
    for (IdbEntry &entry : *idb)
    {
        if (entry.desc_id > desc_idx)
            entry.desc_id--;

        else if (entry.desc_id == desc_idx)
        {
            DPRINTF("%s: Error, desc reference should have been removed from the idb first.\n", FUNCNAME);
            return false;
        }
    }

    return true;
}

bool Xenoverse2::RemoveCacCostumeName(uint32_t name_idx, int lang, bool update_idb)
{
    if (!xv2fs || game_cac_costume_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    if (name_idx >= game_cac_costume_names[lang]->GetNumEntries())
        return true; // Yes, true

    if (!game_cac_costume_names[lang]->RemoveEntry(name_idx))
        return false;

    if (!update_idb)
        return true;

    if (!UpdateIdbNames(game_top_idb, name_idx))
        return false;

    if (!UpdateIdbNames(game_bottom_idb, name_idx))
        return false;

    if (!UpdateIdbNames(game_gloves_idb, name_idx))
        return false;

    if (!UpdateIdbNames(game_shoes_idb, name_idx))
        return false;

    return true;
}

bool Xenoverse2::GetAccesoryName(uint32_t name_idx, std::string &name, int lang)
{
    if (!xv2fs || game_accesory_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    if (name_idx >= game_accesory_names[lang]->GetNumEntries())
        return false;

    const MsgEntry &entry = (*game_accesory_names[lang])[name_idx];

    name = UnescapeHtml(entry.lines[0]);
    return true;
}

bool Xenoverse2::SetAccesoryName(uint32_t name_idx, const std::string &name, int lang)
{
    if (!xv2fs || game_accesory_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    if (name_idx >= game_accesory_names[lang]->GetNumEntries())
        return false;

    MsgEntry &entry = (*game_accesory_names[lang])[name_idx];
    entry.lines.resize(1);
    entry.lines[0] = EscapeHtml(name);

    return true;
}

bool Xenoverse2::AddAccesoryName(const std::string &name, int lang, uint16_t *ret_idx)
{
    if (!xv2fs || game_accesory_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    size_t idx = game_accesory_names[lang]->GetNumEntries();
    if (idx >= 0xFFFF)
    {
        DPRINTF("%s: Overflow of ids.\n", FUNCNAME);
        return false;
    }

    if (ret_idx)
        *ret_idx = (uint16_t)idx;

    MsgEntry entry;

    for (size_t i = idx; ; i++)
    {
         entry.name = "accessory_" + Utils::ToStringAndPad((int)i, 3);
         if (!game_accesory_names[lang]->FindEntryByName(entry.name))
             break;
    }

    entry.lines.resize(1);
    entry.lines[0] = EscapeHtml(name);

    return game_accesory_names[lang]->AddEntry(entry, true);
}

bool Xenoverse2::RemoveAccesoryName(uint32_t name_idx, int lang, bool update_idb)
{
    if (!xv2fs || game_accesory_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    if (name_idx >= game_accesory_names[lang]->GetNumEntries())
        return true; // Yes, true

    if (!game_accesory_names[lang]->RemoveEntry(name_idx))
        return false;

    if (!update_idb)
        return true;

    if (!UpdateIdbNames(game_accesory_idb, name_idx))
        return false;

    return true;
}

bool Xenoverse2::GetCacCostumeDesc(uint32_t desc_idx, std::string &desc, int lang)
{
    if (!xv2fs || game_cac_costume_descs.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    if (desc_idx >= game_cac_costume_descs[lang]->GetNumEntries())
        return false;

    const MsgEntry &entry = (*game_cac_costume_descs[lang])[desc_idx];

    desc = UnescapeHtml(entry.lines[0]);
    return true;
}

bool Xenoverse2::SetCacCostumeDesc(uint32_t desc_idx, const std::string &desc, int lang)
{
    if (!xv2fs || game_cac_costume_descs.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    if (desc_idx >= game_cac_costume_descs[lang]->GetNumEntries())
        return false;

    MsgEntry &entry = (*game_cac_costume_descs[lang])[desc_idx];
    entry.lines.resize(1);
    entry.lines[0] = EscapeHtml(desc);

    return true;
}

bool Xenoverse2::AddCacCostumeDesc(const std::string &entry_name, const std::string &desc, int lang, uint16_t *ret_idx)
{
    if (!xv2fs || game_cac_costume_descs.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    size_t idx = game_cac_costume_descs[lang]->GetNumEntries();
    if (idx >= 0xFFFF)
    {
        DPRINTF("%s: Overflow of ids.\n", FUNCNAME);
        return false;
    }

    if (ret_idx)
        *ret_idx = (uint16_t)idx;

    MsgEntry entry;
    entry.lines.resize(1);
    entry.lines[0] = EscapeHtml(desc);

    for (int i = 300; ; i++)
    {
        std::string final_entry_name = entry_name + Utils::ToString(i);

        if (!game_cac_costume_descs[lang]->FindEntryByName(final_entry_name))
        {
            entry.name = final_entry_name;
            return game_cac_costume_descs[lang]->AddEntry(entry, true);
        }
    }

    return false; // should not be here never
}

bool Xenoverse2::RemoveCacCostumeDesc(uint32_t desc_idx, int lang, bool update_idb)
{
    if (!xv2fs || game_cac_costume_descs.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    if (desc_idx >= game_cac_costume_descs[lang]->GetNumEntries())
        return true; // Yes, true

    if (!game_cac_costume_descs[lang]->RemoveEntry(desc_idx))
        return false;

    if (!update_idb)
        return true;

    if (!UpdateIdbDescs(game_top_idb, desc_idx))
        return false;

    if (!UpdateIdbDescs(game_bottom_idb, desc_idx))
        return false;

    if (!UpdateIdbDescs(game_gloves_idb, desc_idx))
        return false;

    if (!UpdateIdbDescs(game_shoes_idb, desc_idx))
        return false;

    return true;
}

bool Xenoverse2::GetAccesoryDesc(uint32_t desc_idx, std::string &desc, int lang)
{
    if (!xv2fs || game_accesory_descs.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    if (desc_idx >= game_accesory_descs[lang]->GetNumEntries())
        return false;

    const MsgEntry &entry = (*game_accesory_descs[lang])[desc_idx];

    desc = UnescapeHtml(entry.lines[0]);
    return true;
}

bool Xenoverse2::SetAccesoryDesc(uint32_t desc_idx, const std::string &desc, int lang)
{
    if (!xv2fs || game_accesory_descs.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    if (desc_idx >= game_accesory_descs[lang]->GetNumEntries())
        return false;

    MsgEntry &entry = (*game_accesory_descs[lang])[desc_idx];
    entry.lines.resize(1);
    entry.lines[0] = EscapeHtml(desc);

    return true;
}

bool Xenoverse2::AddAccesoryDesc(const std::string &desc, int lang, uint16_t *ret_idx)
{
    if (!xv2fs || game_accesory_descs.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    size_t idx = game_accesory_descs[lang]->GetNumEntries();
    if (idx >= 0xFFFF)
    {
        DPRINTF("%s: Overflow of ids.\n", FUNCNAME);
        return false;
    }

    if (ret_idx)
        *ret_idx = (uint16_t)idx;

    MsgEntry entry;

    for (size_t i = idx; ; i++)
    {
        entry.name = "accessory_eff_" + Utils::ToStringAndPad((int)i, 3);
        if (!game_accesory_descs[lang]->FindEntryByName(entry.name))
            break;
    }

    entry.lines.resize(1);
    entry.lines[0] = EscapeHtml(desc);

    return game_accesory_descs[lang]->AddEntry(entry, true);
}

bool Xenoverse2::RemoveAccesoryDesc(uint32_t desc_idx, int lang, bool update_idb)
{
    if (!xv2fs || game_accesory_descs.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    if (desc_idx >= game_accesory_descs[lang]->GetNumEntries())
        return true; // Yes, true

    if (!game_accesory_descs[lang]->RemoveEntry(desc_idx))
        return false;

    if (!update_idb)
        return true;

    if (!UpdateIdbDescs(game_accesory_idb, desc_idx))
        return false;

    return true;
}

bool Xenoverse2::GetTalismanName(uint32_t name_idx, std::string &name, int lang)
{
    if (!xv2fs || game_talisman_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    if (name_idx >= game_talisman_names[lang]->GetNumEntries())
        return false;

    const MsgEntry &entry = (*game_talisman_names[lang])[name_idx];

    name = UnescapeHtml(entry.lines[0]);
    return true;
}

uint32_t Xenoverse2::GetBlastFromTalisman(uint32_t talisman_id, bool id2)
{
    if (!xv2fs || !game_talisman_idb)
        return (uint32_t)-1;

    IdbEntry *entry  = game_talisman_idb->FindEntryByID(talisman_id);
    if (entry)
    {
        if (entry->model == 1)
            return (id2) ? 1080 : 21080; // POW

        if (entry->model == 0 || entry->model == 2)
            return (id2) ? 1081 : 21081; // RSH

        if (entry->model == 3)
            return (id2) ? 1082 : 21082; // BOM

        if (entry->model == 4)
            return (id2) ? 1083 : 21083; // HOM

        if (entry->model == 5)
            return (id2) ? 1084 : 21084; // PAL
    }

    return (uint32_t)-1;
}

bool Xenoverse2::GetMaterialName(uint32_t name_idx, std::string &name, int lang)
{
    if (!xv2fs || game_material_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    if (name_idx >= game_material_names[lang]->GetNumEntries())
        return false;

    const MsgEntry &entry = (*game_material_names[lang])[name_idx];

    name = UnescapeHtml(entry.lines[0]);
    return true;
}

bool Xenoverse2::GetBattleName(uint32_t name_idx, std::string &name, int lang)
{
    if (!xv2fs || game_battle_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    if (name_idx >= game_battle_names[lang]->GetNumEntries())
        return false;

    const MsgEntry &entry = (*game_battle_names[lang])[name_idx];

    name = UnescapeHtml(entry.lines[0]);
    return true;
}

bool Xenoverse2::GetExtraName(uint32_t name_idx, std::string &name, int lang)
{
    if (!xv2fs || game_extra_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    if (name_idx >= game_extra_names[lang]->GetNumEntries())
        return false;

    const MsgEntry &entry = (*game_extra_names[lang])[name_idx];

    name = UnescapeHtml(entry.lines[0]);
    return true;
}

bool Xenoverse2::GetPetName(uint32_t name_idx, std::string &name, int lang)
{
    if (!xv2fs || game_pet_names.size() != XV2_LANG_NUM || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    if (name_idx >= game_pet_names[lang]->GetNumEntries())
        return false;

    const MsgEntry &entry = (*game_pet_names[lang])[name_idx];

    name = UnescapeHtml(entry.lines[0]);
    return true;
}

HcaFile *Xenoverse2::GetCssSound(uint32_t cue_id, bool english)
{
    if (!game_css_acb_en)
    {
        if (!InitSound(true))
        {
            DPRINTF("%s: InitSound failed.\n", FUNCNAME);
            return nullptr;
        }
    }

    AcbFile *acb = (english) ? game_css_acb_en : game_css_acb_jp;
    Afs2File *awb = (english) ? game_css_awb_en : game_css_awb_jp;

    if (!acb || !awb)
        return nullptr;

    bool external;
    uint32_t awb_idx = acb->CueIdToAwbIndex(cue_id, &external);

    if (awb_idx == (uint32_t)-1 || !external)
        return nullptr;

    uint64_t size;
    uint8_t *hca_buf = awb->ExtractFile(awb_idx, &size);

    if (hca_buf)
    {
        HcaFile *hca = new HcaFile();

        bool ret = hca->Load(hca_buf, size);
        delete[] hca_buf;

        if (ret)
            return hca;

        delete hca;
    }

    return nullptr;
}

HcaFile *Xenoverse2::GetCssSound(const std::string &name, bool english)
{
    if (!game_css_acb_en)
    {
        if (!InitSound(true))
        {
            DPRINTF("%s: InitSound failed.\n", FUNCNAME);
            return nullptr;
        }
    }

    AcbFile *acb = (english) ? game_css_acb_en : game_css_acb_jp;
    Afs2File *awb = (english) ? game_css_awb_en : game_css_awb_jp;

    if (!acb || !awb)
        return nullptr;

    uint32_t cue_id = acb->FindCueId(name);
    if (cue_id == (uint32_t)-1)
        return nullptr;

    return GetCssSound(cue_id, english);
}

bool Xenoverse2::SetSound(AcbFile *acb, Afs2File *awb, uint32_t cue_id, HcaFile &hca, bool is_external_awb)
{
    if (!acb || !awb)
        return false;    

    uint32_t track_idx = acb->CueIdToTrackIndex(cue_id);
    if (track_idx == (uint32_t)-1)
        return false;

    bool external;
    uint32_t awb_idx = acb->CueIdToAwbIndex(cue_id, &external);

    if (awb_idx == (uint32_t)-1 || external != is_external_awb)
        return false;

    if (!acb->SetCueLength(cue_id, (uint32_t)(hca.GetDuration()*1000.0f)))
        return false;

    if (!acb->SetWaveformNumSamples(track_idx, hca.GetNumSamples()))
        return false;

    size_t size;
    uint8_t *buf = hca.Save(&size);
    if (!buf)
        return false;

    if (!awb->SetFile(awb_idx, buf, size, true))
    {
        delete[] buf;
        return false;
    }

    return true;
}

uint32_t Xenoverse2::SetSound(AcbFile *acb, Afs2File *awb, const std::string &name, HcaFile &hca, bool is_external_awb, int use_cmd1, int use_cmd2, bool loop)
{
    if (!acb || !awb)
        return (uint32_t)-1;

    uint32_t cue_id = acb->FindCueId(name);
    if (cue_id != (uint32_t)-1)
    {
        if (!SetSound(acb, awb, cue_id, hca, is_external_awb))
            return (uint32_t)-1;

        return cue_id;
    }

    cue_id = acb->FindCueId("X2_FREE");
    if (cue_id != (uint32_t)-1)
    {
        if (!SetSound(acb, awb, cue_id, hca, is_external_awb))
            return (uint32_t)-1;

        if (!acb->SetCueName(cue_id, name))
            return (uint32_t)-1;

        return cue_id;
    }

    uint16_t awb_idx = (uint16_t)awb->GetNumFiles();

    if (use_cmd1 < 0)
        use_cmd1 = 2;

    if (use_cmd2 < 0)
        use_cmd2 = 2;

    if (!acb->AddTrack(name, awb_idx, is_external_awb, (uint32_t)(hca.GetDuration()*1000.0f), hca.GetNumSamples(),
                       loop, (uint16_t)hca.GetSampleRate(), (uint8_t)hca.GetNumChannels(), (uint16_t)use_cmd1, (uint16_t)use_cmd2, &cue_id, nullptr, nullptr))
    {
        DPRINTF("%s: AddTrack failed!\n", FUNCNAME);
        return (uint32_t)-1;
    }

    size_t size;
    uint8_t *buf = hca.Save(&size);
    if (!buf)
        return (uint32_t)-1;

    if (!awb->AddFile(buf, size, true))
    {
        delete[] buf;
        return (uint32_t)-1;
    }

    return cue_id;
}

bool Xenoverse2::FreeSound(AcbFile *acb,  const std::string &name)
{
    if (!acb)
        return false;

    uint32_t cue_id = acb->FindCueId(name);
    if (cue_id == (uint32_t)-1)
        return true; // Yes, true

    if (!acb->SetCueName(cue_id, "X2_FREE"))
        return false;

    return true;
}

bool Xenoverse2::SetCssSound(uint32_t cue_id, bool english, HcaFile &hca)
{
    if (!game_css_acb_en)
    {
        if (!InitSound(true))
        {
            DPRINTF("%s: InitSound failed.\n", FUNCNAME);
            return false;
        }
    }

    AcbFile *acb = (english) ? game_css_acb_en : game_css_acb_jp;
    Afs2File *awb = (english) ? game_css_awb_en : game_css_awb_jp;

    return SetSound(acb, awb, cue_id, hca);
}

uint32_t Xenoverse2::SetCssSound(const std::string &name, bool english, HcaFile &hca, int use_cmd1, int use_cmd2)
{
    if (!game_css_acb_en)
    {
        if (!InitSound(true))
        {
            DPRINTF("%s: InitSound failed.\n", FUNCNAME);
            return (uint32_t)-1;
        }
    }

    AcbFile *acb = (english) ? game_css_acb_en : game_css_acb_jp;
    Afs2File *awb = (english) ? game_css_awb_en : game_css_awb_jp;

    return SetSound(acb, awb, name, hca, true, use_cmd1, use_cmd2);
}

bool Xenoverse2::FreeCssSound(const std::string &name, bool english)
{
    if (!game_css_acb_en)
    {
        if (!InitSound(true))
        {
            DPRINTF("%s: InitSound failed.\n", FUNCNAME);
            return false;
        }
    }

    AcbFile *acb = (english) ? game_css_acb_en : game_css_acb_jp;
    return FreeSound(acb, name);
}

bool Xenoverse2::SetSevCmnSound(uint32_t cue_id, bool english, HcaFile &hca, bool is_global_cue_id)
{
    if (!game_sev_cmn_acb_en)
    {
        if (!InitSound(true))
        {
            DPRINTF("%s: InitSound failed.\n", FUNCNAME);
            return false;
        }
    }

    sev_audio_changed = true;

    AcbFile *acb = (english) ? game_sev_cmn_acb_en : game_sev_cmn_acb_jp;
    Afs2File *awb = (english) ? game_sev_cmn2_awb_en : game_sev_cmn2_awb_jp;

    if (is_global_cue_id)
    {
        uint32_t highest = acb->GetHighestCueId();
        if (highest == (uint32_t)-1)
            return false;

        if (cue_id <= highest)
            return false;

        cue_id = cue_id - highest - 1;
    }

    acb = (english) ? game_sev_cmn2_acb_en : game_sev_cmn2_acb_jp;
    return SetSound(acb, awb, cue_id, hca);
}

uint32_t Xenoverse2::SetSevCmnSound(const std::string &name, bool english, HcaFile &hca, bool return_global_cue_id)
{
    if (!game_sev_cmn2_acb_en)
    {
        if (!InitSound(true))
        {
            DPRINTF("%s: InitSound failed.\n", FUNCNAME);
            return (uint32_t)-1;
        }
    }

    sev_audio_changed = true;

    AcbFile *acb = (english) ? game_sev_cmn2_acb_en : game_sev_cmn2_acb_jp;
    Afs2File *awb = (english) ? game_sev_cmn2_awb_en : game_sev_cmn2_awb_jp;

    uint32_t cue_id = SetSound(acb, awb, name, hca);
    if (cue_id == (uint32_t)-1)
        return cue_id;

    if (return_global_cue_id)
    {
        acb = (english) ? game_sev_cmn_acb_en : game_sev_cmn_acb_jp;

        uint32_t highest = acb->GetHighestCueId();
        if (highest == (uint32_t)-1)
            return highest;

        cue_id = cue_id + highest + 1;
    }

    return cue_id;
}

uint32_t Xenoverse2::GetSevCmnCueId(const std::string &name, bool english, bool *is_sev2, bool return_global_cue_id)
{
    if (!game_sev_cmn_acb_en)
    {
        if (!InitSound(true))
        {
            DPRINTF("%s: InitSound failed.\n", FUNCNAME);
            return (uint32_t)-1;
        }
    }

    AcbFile *acb = (english) ? game_sev_cmn2_acb_en : game_sev_cmn2_acb_jp;
    if (!acb)
        return (uint32_t)-1;

    uint32_t cue_id = acb->FindCueId(name);
    acb = (english) ? game_sev_cmn_acb_en : game_sev_cmn_acb_jp;
    if (!acb)
        return (uint32_t)-1;

    if (cue_id != (uint32_t)-1)
    {
        *is_sev2 = true;

        if (return_global_cue_id)
        {
            uint32_t highest = acb->GetHighestCueId();
            if (highest == (uint32_t)-1)
                return highest;

            cue_id = cue_id + highest + 1;
        }

        return cue_id;
    }

    cue_id = acb->FindCueId(name);
    if (cue_id != (uint32_t)-1)
        *is_sev2 = false;

    return cue_id;
}

bool Xenoverse2::FreeSevCmnSound(const std::string &name, bool english)
{
    if (!game_sev_cmn2_acb_en)
    {
        if (!InitSound(true))
        {
            DPRINTF("%s: InitSound failed.\n", FUNCNAME);
            return false;
        }
    }

    sev_audio_changed = true;

    AcbFile *acb = (english) ? game_sev_cmn2_acb_en : game_sev_cmn2_acb_jp;
    return FreeSound(acb, name);
}

bool Xenoverse2::SetBgmSound(uint32_t cue_id, HcaFile &hca)
{
    if (!game_bgm_acb)
    {
        if (!InitBgm())
        {
            DPRINTF("%s: InitBgm failed.\n", FUNCNAME);
            return false;
        }
    }

    bgm_changed = true;
    return SetSound(game_bgm_acb, game_bgm_awb, cue_id, hca);
}

uint32_t Xenoverse2::SetBgmSound(const std::string &name, HcaFile &hca)
{
    if (!game_bgm_acb)
    {
        if (!InitBgm())
        {
            DPRINTF("%s: InitBgm failed.\n", FUNCNAME);
            return (uint32_t)-1;
        }
    }

    bgm_changed = true;
    uint32_t cue_id = SetSound(game_bgm_acb, game_bgm_awb, name, hca, true, 0xFFFF, 48, true);
    if (cue_id == (uint32_t)-1)
        return cue_id;

    return cue_id;
}

uint32_t Xenoverse2::GetBgmCueId(const std::string &name)
{
    if (!game_bgm_acb)
    {
        if (!InitBgm())
        {
            DPRINTF("%s: InitBgm failed.\n", FUNCNAME);
            return (uint32_t)-1;
        }
    }

    return game_bgm_acb->FindCueId(name);
}

bool Xenoverse2::FreeBgmSound(const std::string &name)
{
    if (!game_bgm_acb)
    {
        if (!InitBgm())
        {
            DPRINTF("%s: InitBgm failed.\n", FUNCNAME);
            return false;
        }
    }

    bgm_changed = true;
    return FreeSound(game_bgm_acb, name);
}

uint32_t Xenoverse2::CusAuraToAurAura(uint32_t id)
{
    if (id == 0xFFFF || id == 0xFFFFFFFF)
        return 0xFFFFFFFF;

    if (game_prebaked && id < 0x10000)
    {
        CusAuraData *data = game_prebaked->FindAuraData((uint16_t)id);
        if (data)
            return data->aur_aura_id;
    }

    switch (id)
    {
        case 0:
            return 5;

        case 1:
            return 6;

        case 2:
            return 7;

        case 3: case 10:
            return 15;

        case 4: case 5: case 6:
            return 13;

        case 7:
            return 14;

        case 8:
            return 18;

        case 9:
            return 19;

        case 11:
            return 24;

        case 12:
            return 25;

        case 13:
            return 20;

        case 15: case 17: case 18:
            return 22;

        case 19:
            return 30;

        case 16:
            return 23;

        case 20: case 21:
            return 33;
			
		case 24:
			return 21;
			
		case 25:
			return 39;
			
		case 26:
			return 48;
    }

    return 0xFFFFFFFF;
}

std::string Xenoverse2::GetCharaAndCostumeName(const std::string &code, uint32_t costume_id, uint32_t model_preset, int lang)
{
    std::string result, name, costume;

    if (!Xenoverse2::GetCharaName(code, name, lang, costume_id))
        name = code;

    if (costume_id == 0xFFFFFFFF)
        costume = "[Wildcard]";

    else if (!Xenoverse2::GetCharaCostumeName(code, costume_id, model_preset, costume, lang))
        costume = Utils::UnsignedToString(costume_id, false);

    result = name + " - " + costume;
    return result;
}

static std::string dummy_costume(uint32_t costume_id)
{
    if (costume_id == 0xFFFFFFFF)
        return "[Wildcard]";

    return Utils::UnsignedToString(costume_id, true);
}

std::string Xenoverse2::GetCharaAndCostumeName(uint32_t char_id, uint32_t costume_id, uint32_t model_preset, bool ignore_unknown, int lang)
{
    CmsEntry *cms_entry;

    if (char_id == 0xFFFFFFFF)
    {
        return "[Wildcard] - " + dummy_costume(costume_id);
    }

    if (!game_cms || ((cms_entry = game_cms->FindEntryByID(char_id)) == nullptr))
    {
        if (ignore_unknown)
            return std::string();

        return Utils::UnsignedToString(char_id, true) + " - " + dummy_costume(costume_id);
    }

    return GetCharaAndCostumeName(cms_entry->name, costume_id, model_preset, lang);
}

std::string Xenoverse2::GetCacTopName(uint16_t item_id, int lang)
{
    if (item_id == 0xFFFF)
        return "None";

    if (!game_top_idb)
        return std::string();

    IdbEntry *entry = game_top_idb->FindEntryByID(item_id);
    if (!entry)
        return std::string();

    std::string ret;
    GetCacCostumeName(entry->name_id, ret, lang);

    return ret;
}

std::string Xenoverse2::GetCacBottomName(uint16_t item_id, int lang)
{
    if (item_id == 0xFFFF)
        return "None";

    if (!game_bottom_idb)
        return std::string();

    IdbEntry *entry = game_bottom_idb->FindEntryByID(item_id);
    if (!entry)
        return std::string();

    std::string ret;
    GetCacCostumeName(entry->name_id, ret, lang);

    return ret;
}

std::string Xenoverse2::GetCacGlovesName(uint16_t item_id, int lang)
{
    if (item_id == 0xFFFF)
        return "None";

    if (!game_gloves_idb)
        return std::string();

    IdbEntry *entry = game_gloves_idb->FindEntryByID(item_id);
    if (!entry)
        return std::string();

    std::string ret;
    GetCacCostumeName(entry->name_id, ret, lang);

    return ret;
}

std::string Xenoverse2::GetCacShoesName(uint16_t item_id, int lang)
{
    if (item_id == 0xFFFF)
        return "None";

    if (!game_shoes_idb)
        return std::string();

    IdbEntry *entry = game_shoes_idb->FindEntryByID(item_id);
    if (!entry)
        return std::string();

    std::string ret;
    GetCacCostumeName(entry->name_id, ret, lang);

    return ret;
}

std::string Xenoverse2::GetCacAccesoryName(uint16_t item_id, int lang)
{
    if (item_id == 0xFFFF)
        return "None";

    if (!game_accesory_idb)
        return std::string();

    IdbEntry *entry = game_accesory_idb->FindEntryByID(item_id);
    if (!entry)
        return std::string();

    std::string ret;
    GetAccesoryName(entry->name_id, ret, lang);

    return ret;
}

std::string Xenoverse2::GetTalismanNameEx(uint16_t item_id, int lang)
{
    if (item_id == 0xFFFF)
        return "None";

    if (!game_talisman_idb)
        return std::string();

    IdbEntry *entry = game_talisman_idb->FindEntryByID(item_id);
    if (!entry)
        return std::string();

    std::string ret;
    GetTalismanName(entry->name_id, ret, lang);

    return ret;
}

std::string Xenoverse2::GetMaterialNameEx(uint16_t item_id, int lang)
{
    if (item_id == 0xFFFF)
        return "None";

    if (!game_material_idb)
        return std::string();

    IdbEntry *entry = game_material_idb->FindEntryByID(item_id);
    if (!entry)
        return std::string();

    std::string ret;
    GetMaterialName(entry->name_id, ret, lang);

    return ret;
}

std::string Xenoverse2::GetExtraNameEx(uint16_t item_id, int lang)
{
    if (item_id == 0xFFFF)
        return "None";

    if (!game_extra_idb)
        return std::string();

    IdbEntry *entry = game_extra_idb->FindEntryByID(item_id);
    if (!entry)
        return std::string();

    std::string ret;
    GetExtraName(entry->name_id, ret, lang);

    return ret;
}

std::string Xenoverse2::GetBattleNameEx(uint16_t item_id, int lang)
{
    if (item_id == 0xFFFF)
        return "None";

    if (!game_battle_idb)
        return std::string();

    IdbEntry *entry = game_battle_idb->FindEntryByID(item_id);
    if (!entry)
        return std::string();

    std::string ret;
    GetBattleName(entry->name_id, ret, lang);

    return ret;
}

std::string Xenoverse2::GetPetNameEx(uint16_t item_id, int lang)
{
    if (item_id == 0xFFFF)
        return "None";

    if (!game_pet_idb)
        return std::string();

    IdbEntry *entry = game_pet_idb->FindEntryByID(item_id);
    if (!entry)
        return std::string();

    std::string ret;
    GetPetName(entry->name_id, ret, lang);

    return ret;
}

std::string Xenoverse2::GetSkillDirectory(const CusSkill &skill, std::string *last_part)
{
    std::string ret = "data/skill/";

    if (/*skill.id >= 0 &&*/ skill.id < CUS_ULTIMATE_START)
    {
        ret += "SPA/";
    }
    else if (skill.id >= CUS_ULTIMATE_START && skill.id < CUS_EVASIVE_START)
    {
        ret += "ULT/";
    }
    else if (skill.id >= CUS_EVASIVE_START && skill.id < CUS_UNK_START)
    {
        ret += "ESC/";
    }
    else if (skill.id >= CUS_BLAST_START && skill.id < CUS_AWAKEN_START)
    {
        ret += "BLT/";
    }
    else if (skill.id >= CUS_AWAKEN_START && skill.id < CUS_ID_END)
    {
        ret += "MET/";
    }
    else
    {
        return std::string();
    }

    std::string skill_dir = Utils::ToStringAndPad(skill.id2, 3);
    skill_dir += '_';

    CmsEntry *entry = game_cms->FindEntryByID(skill.id2 / 10);
    if (!entry)
        return std::string();

    skill_dir += entry->name;
    skill_dir += '_';
    skill_dir += skill.name;

    ret += skill_dir;
    ret += '/';

    if (last_part)
        *last_part = skill_dir;

    return ret;
}

std::vector<std::string> Xenoverse2::GetCostumeNamesForPartSet(uint32_t partset, uint8_t race_lock)
{
    std::vector<std::string> ret;

    if (!game_top_idb || !game_bottom_idb || !game_gloves_idb || !game_shoes_idb || !game_accesory_idb)
        return ret;

    for (const IdbEntry &entry : *game_top_idb)
    {
        if (entry.model == partset && (entry.racelock & race_lock))
        {
            std::string name;

            if (GetCacCostumeName(entry.name_id, name) && name.length() > 0)
            {
                if (std::find(ret.begin(), ret.end(), name) == ret.end())
                {
                    ret.push_back(name);
                }
            }
        }
    }

    for (const IdbEntry &entry : *game_bottom_idb)
    {
        if (entry.model == partset && (entry.racelock & race_lock))
        {
            std::string name;

            if (GetCacCostumeName(entry.name_id, name) && name.length() > 0)
            {
                if (std::find(ret.begin(), ret.end(), name) == ret.end())
                {
                    ret.push_back(name);
                }
            }
        }
    }

    for (const IdbEntry &entry : *game_gloves_idb)
    {
        if (entry.model == partset && (entry.racelock & race_lock))
        {
            std::string name;

            if (GetCacCostumeName(entry.name_id, name) && name.length() > 0)
            {
                if (std::find(ret.begin(), ret.end(), name) == ret.end())
                {
                    ret.push_back(name);
                }
            }
        }
    }

    for (const IdbEntry &entry : *game_shoes_idb)
    {
        if (entry.model == partset && (entry.racelock & race_lock))
        {
            std::string name;

            if (GetCacCostumeName(entry.name_id, name) && name.length() > 0)
            {
                if (std::find(ret.begin(), ret.end(), name) == ret.end())
                {
                    ret.push_back(name);                    
                }
            }
        }
    }

    for (const IdbEntry &entry : *game_accesory_idb)
    {
        if (entry.model == partset && (entry.racelock & race_lock))
        {
            std::string name;

            if (GetAccesoryName(entry.name_id, name) && name.length() > 0)
            {
                if (std::find(ret.begin(), ret.end(), name) == ret.end())
                {
                    ret.push_back(name);                    
                }
            }
        }
    }

    return ret;
}

std::string Xenoverse2::GetCostumeNamesForPartSet2(uint32_t partset, uint8_t race_lock)
{
    std::vector<std::string> ret = GetCostumeNamesForPartSet(partset, race_lock);

    if (ret.size() == 0)
        return std::string();

    return Utils::ToSingleString(ret);
}

bool Xenoverse2::InitStageSlots()
{
    bool commit = false;

    if (game_stage_slots_file)
        delete game_stage_slots_file;

    game_stage_slots_file = new Xv2PatcherSlotsFileStage();
    if (xv2fs->FileExists(GAME_STAGE_SLOTS_FILE, false))
    {
        if (!xv2fs->LoadFile(game_stage_slots_file, GAME_STAGE_SLOTS_FILE))
        {
            DPRINTF("%s: Failed to load stage slots.\n", FUNCNAME);
            return false;
        }
    }
    else
    {
        commit = true;

        if (!game_stage_slots_file->Load((const uint8_t *)default_stage_slots.c_str(), default_stage_slots.length()))
        {
            DPRINTF("%s: Failed to load default stage slots (this should never happen)\n", FUNCNAME);
            return false;
        }
    }

    if (game_stage_slots_file_local)
        delete game_stage_slots_file_local;

    game_stage_slots_file_local = new Xv2PatcherSlotsFileStage();
    if (xv2fs->FileExists(GAME_STAGE_SLOTS_FILE_LOCAL, false))
    {

        if (!xv2fs->LoadFile(game_stage_slots_file_local, GAME_STAGE_SLOTS_FILE_LOCAL))
        {
            DPRINTF("%s: Failed to load local stage slots.\n", FUNCNAME);
            return false;
        }
    }
    else
    {
        commit = true;

        if (!game_stage_slots_file_local->Load((const uint8_t *)default_stage_slots_local.c_str(), default_stage_slots_local.length()))
        {
            DPRINTF("%s: Failed to load default stage slots (this should never happen)\n", FUNCNAME);
            return false;
        }
    }

    if (commit)
    {
        if (!CommitStageSlots())
        {
            DPRINTF("%s: Failed to commit stage slots.\n", FUNCNAME);
            return false;
        }
    }

    return true;
}

bool Xenoverse2::CommitStageSlots()
{
    if (!xv2fs || !game_stage_slots_file || !game_stage_slots_file_local)
        return false;

    if (!xv2fs->SaveFile(game_stage_slots_file, GAME_STAGE_SLOTS_FILE))
        return false;

    return xv2fs->SaveFile(game_stage_slots_file_local, GAME_STAGE_SLOTS_FILE_LOCAL);
}

bool Xenoverse2::InitStageNames(int only_this_lang)
{
    if (!xv2fs)
        return false;

    for (MsgFile *msg : game_stage_names)
    {
        if (msg)
            delete msg;
    }

    game_stage_names.clear();
    game_stage_names.resize(XV2_LANG_NUM);

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        if (only_this_lang >= 0 && i != only_this_lang)
            continue;

        std::string path = GAME_STAGENAMES_PATH + xv2_lang_codes[i] + ".msg";
        game_stage_names[i] = new MsgFile();

        if (!xv2fs->LoadFile(game_stage_names[i], path))
        {
            DPRINTF("%s: Failed loading file \"%s\"\n", FUNCNAME, path.c_str());
            return false;
        }
    }

    return true;
}

bool Xenoverse2::CommitStageNames()
{
    if (!xv2fs)
        return false;

    if (game_stage_names.size() != XV2_LANG_NUM)
        return true; // Yes, true

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        std::string path = GAME_STAGENAMES_PATH + xv2_lang_codes[i] + ".msg";

        if (!xv2fs->SaveFile(game_stage_names[i], path))
        {
            DPRINTF("%s: Failed saving file \"%s\"\n", FUNCNAME, path.c_str());
            return false;
        }
    }

    return true;
}

bool Xenoverse2::GetStageName(const std::string &id, std::string &name, int lang)
{
    if (!xv2fs || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    size_t stage_id;
    Xv2Stage *stage = game_stage_def->GetStageByCode(id, &stage_id);
    if (stage && stage_id >= XV2_ORIGINAL_NUM_STAGES && stage->ssid >= XV2_ORIGINAL_NUM_SS_STAGES)
    {
        name = stage->GetName(lang);
        return true;
    }

    if (game_stage_names.size() != XV2_LANG_NUM)
    {
        if (!InitStageNames())
        {
            DPRINTF("%s: InitStageNames failed.\n", FUNCNAME);
            return false;
        }
    }

    std::string entry_name = "stage_" + id;
    MsgEntry *entry = game_stage_names[lang]->FindEntryByName(entry_name);

    if (!entry)
        return false;

    name = UnescapeHtml(entry->lines[0]);
    return true;
}

bool Xenoverse2::SetStageName(const std::string &id, const std::string &name, int lang)
{
    if (!xv2fs || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    size_t stage_id;
    Xv2Stage *stage = game_stage_def->GetStageByCode(id, &stage_id);
    if (stage && stage_id >= XV2_ORIGINAL_NUM_STAGES && stage->ssid >= XV2_ORIGINAL_NUM_SS_STAGES)
    {
        stage->SetName(name, lang);
        return true;
    }

    if (game_stage_names.size() != XV2_LANG_NUM)
    {
        if (!InitStageNames())
        {
            DPRINTF("%s: InitStageNames failed.\n", FUNCNAME);
            return false;
        }
    }

    std::string entry_name = "stage_" + id;
    MsgEntry *entry = game_stage_names[lang]->FindEntryByName(entry_name);

    if (entry)
    {
        entry->lines.resize(1);
        entry->lines[0] = EscapeHtml(name);
    }
    else
    {
        MsgEntry entry;

        entry.name = entry_name;
        entry.lines.push_back(EscapeHtml(name));

        if (!game_stage_names[lang]->AddEntry(entry, true))
            return false;
    }

    return true;
}

bool Xenoverse2::RemoveStageName(const std::string &id, int lang)
{
    if (!xv2fs || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    if (game_stage_names.size() != XV2_LANG_NUM)
    {
        if (!InitStageNames())
        {
            DPRINTF("%s: InitStageNames failed.\n", FUNCNAME);
            return false;
        }
    }

    std::string entry_name = "stage_" + id;
    return game_stage_names[lang]->RemoveEntry(entry_name);
}

bool Xenoverse2::InitStageEmb()
{
    if (game_stage01_emb)
        delete game_stage01_emb;

    game_stage01_emb = new EmbFile();

    if (!xv2fs->LoadFile(game_stage01_emb, GAME_STAGE01_PATH))
    {
        DPRINTF("%s: Failed to load stage01\n", FUNCNAME);
        return false;
    }

    if (game_stage02_emb)
        delete game_stage02_emb;

    game_stage02_emb = new EmbFile();

    if (!xv2fs->LoadFile(game_stage02_emb, GAME_STAGE02_PATH))
    {
        DPRINTF("%s: Failed to load stage02\n", FUNCNAME);
        return false;
    }

    return true;
}

bool Xenoverse2::CommitStageEmb()
{
    if (!xv2fs)
        return false;

    if (!game_stage01_emb || !game_stage02_emb)
        return true; // Yes, true

    if (!xv2fs->SaveFile(game_stage01_emb, GAME_STAGE01_PATH))
    {
        DPRINTF("%s: Failed saving stage01\n", FUNCNAME);
        return false;
    }

    if (!xv2fs->SaveFile(game_stage02_emb, GAME_STAGE02_PATH))
    {
        DPRINTF("%s: Failed saving stage02\n", FUNCNAME);
        return false;
    }

    return true;
}

bool Xenoverse2::InitCommonDialogue()
{
    if (!xv2fs)
        return false;

    if (game_ttb)
        delete game_ttb;

    game_ttb = new TtbFile();
    if (!xv2fs->LoadFile(game_ttb, GAME_TTB_PATH))
    {
        DPRINTF("%s: Failed to load %s\n", FUNCNAME, GAME_TTB_PATH);
        return false;
    }

    if (game_ttc)
        delete game_ttc;

    game_ttc = new TtcFile();
    if (!xv2fs->LoadFile(game_ttc, GAME_TTC_PATH))
    {
        DPRINTF("%s: Failed to load %s\n", FUNCNAME, GAME_TTC_PATH);
        return false;
    }

    for (MsgFile *msg : qc_dialogue_subs)
    {
        if (msg)
            delete msg;
    }

    if (qc_dialogue_voice)
        delete qc_dialogue_voice;

    qc_dialogue_voice = new MsgFile();
    if (!xv2fs->LoadFile(qc_dialogue_voice, GAME_QC_DIALOGUE_VOICE))
    {
        DPRINTF("%s: Failed to load %s\n", FUNCNAME, GAME_QC_DIALOGUE_VOICE);
        return false;
    }

    qc_dialogue_subs.clear();
    qc_dialogue_subs.resize(XV2_LANG_NUM);

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        std::string path = GAME_QC_DIALOGUE_SUBS + xv2_lang_codes[i] + ".msg";
        qc_dialogue_subs[i] = new MsgFile();

        if (!xv2fs->LoadFile(qc_dialogue_subs[i], path))
        {
            DPRINTF("%s: Failed loading file \"%s\"\n", FUNCNAME, path.c_str());
            return false;
        }
    }

    return true;
}

bool Xenoverse2::CommitCommonDialogue()
{
    if (!xv2fs || !game_ttb || !game_ttc || !qc_dialogue_voice || qc_dialogue_subs.size() != XV2_LANG_NUM)
        return true; // Yes true

    if (!xv2fs->SaveFile(game_ttb, GAME_TTB_PATH))
    {
        DPRINTF("%s: Failed to save %s\n", FUNCNAME, GAME_TTB_PATH);
        return false;
    }

    if (!xv2fs->SaveFile(game_ttc, GAME_TTC_PATH))
    {
        DPRINTF("%s: Failed to save %s\n", FUNCNAME, GAME_TTC_PATH);
        return false;
    }

    if (!xv2fs->SaveFile(qc_dialogue_voice, GAME_QC_DIALOGUE_VOICE))
    {
        DPRINTF("%s: Failed to save %s\n", FUNCNAME, GAME_QC_DIALOGUE_VOICE);
        return false;
    }

    for (int i = 0; i < XV2_LANG_NUM; i++)
    {
        std::string path = GAME_QC_DIALOGUE_SUBS + xv2_lang_codes[i] + ".msg";

        if (!xv2fs->SaveFile(qc_dialogue_subs[i], path))
        {
            DPRINTF("%s: Failed to save \"%s\"\n", FUNCNAME, path.c_str());
            return false;
        }
    }

    return true;
}

bool Xenoverse2::GetTtbSubtitle(const std::string &event_name, std::string &subtitle, int lang)
{
    if (!xv2fs || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    if (!qc_dialogue_voice)
    {
        if (!InitCommonDialogue())
            return false;
    }

    MsgEntry *entry = qc_dialogue_subs[lang]->FindEntryByName(event_name);

    if (!entry)
        return false;

    subtitle = UnescapeHtml(entry->lines[0]);
    return true;
}

bool Xenoverse2::SetTtbSubtitle(const std::string &event_name, const std::string &subtitle, int lang)
{
    if (!xv2fs || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    if (!qc_dialogue_voice)
    {
        if (!InitCommonDialogue())
            return false;
    }

    MsgEntry *entry = qc_dialogue_subs[lang]->FindEntryByName(event_name);

    if (entry)
    {
        entry->lines.resize(1);
        entry->lines[0] = EscapeHtml(subtitle);
    }
    else
    {
        MsgEntry entry;

        entry.name = event_name;
        entry.lines.push_back(EscapeHtml(subtitle));

        if (!qc_dialogue_subs[lang]->AddEntry(entry, true))
            return false;
    }

    return true;
}

bool Xenoverse2::RemoveTtbSubtitle(const std::string &event_name, int lang, bool *existed)
{
    if (!xv2fs || lang < 0 || lang >= XV2_LANG_NUM)
        return false;

    if (!qc_dialogue_voice)
    {
        if (!InitCommonDialogue())
            return false;
    }

    return qc_dialogue_subs[lang]->RemoveEntry(event_name, existed);
}

bool Xenoverse2::GetTtbVoice(const std::string &event_name, std::string &voice)
{
    if (!xv2fs)
        return false;

    if (!qc_dialogue_voice)
    {
        if (!InitCommonDialogue())
            return false;
    }

    MsgEntry *entry = qc_dialogue_voice->FindEntryByName(event_name);

    if (!entry)
        return false;

    voice = UnescapeHtml(entry->lines[0]);
    return true;
}

bool Xenoverse2::SetTtbVoice(const std::string &event_name, const std::string &voice)
{
    if (!xv2fs)
        return false;

    if (!qc_dialogue_voice)
    {
        if (!InitCommonDialogue())
            return false;
    }

    MsgEntry *entry = qc_dialogue_voice->FindEntryByName(event_name);

    if (entry)
    {
        entry->lines.resize(1);
        entry->lines[0] = EscapeHtml(voice);
    }
    else
    {
        MsgEntry entry;

        entry.name = event_name;
        entry.lines.push_back(EscapeHtml(voice));

        if (!qc_dialogue_voice->AddEntry(entry, true))
            return false;
    }

    return true;
}

bool Xenoverse2::RemoveTtbVoice(const std::string &event_name, bool *existed)
{
    if (!xv2fs)
        return false;

    if (!qc_dialogue_voice)
    {
        if (!InitCommonDialogue())
            return false;
    }

    return qc_dialogue_voice->RemoveEntry(event_name, existed);
}

bool Xenoverse2::InitDualSkill(bool init_cnc, bool init_cns)
{
    if (!xv2fs)
        return false;

    if (init_cnc)
    {
        if (game_cnc)
            delete game_cnc;

        game_cnc = new CncFile();
        if (!xv2fs->LoadFile(game_cnc, GAME_CNC_PATH))
        {
            DPRINTF("%s: Failed to load \"%s\"\n", FUNCNAME, GAME_CNC_PATH);
            delete game_cnc; game_cnc = nullptr;
            return false;
        }
    }

    if (init_cns)
    {
        if (game_cns)
            delete game_cns;

        game_cns = new CnsFile();
        if (!xv2fs->LoadFile(game_cns, GAME_CNS_PATH))
        {
            DPRINTF("%s: Failed to load \"%s\"\n", FUNCNAME, GAME_CNS_PATH);
            delete game_cns; game_cns = nullptr;
            return false;
        }
    }

    return true;
}

bool Xenoverse2::CommitDualSkill(bool commit_cnc, bool commit_cns)
{
    if (commit_cns)
    {
        DPRINTF("%s: Commit cns is not implemented!!!!\n", FUNCNAME);
        return false;
    }

    if (!xv2fs)
        return false;

    if (commit_cnc)
    {
        if (!game_cnc)
            return true; // Yes, true

        if (!xv2fs->SaveFile(game_cnc, GAME_CNC_PATH))
        {
            DPRINTF("%s: Failed saving \"%s\"\n", FUNCNAME, GAME_CNC_PATH);
            return false;
        }
    }

    return true;
}

bool Xenoverse2::InitVfx()
{
    if (!xv2fs)
        return false;

    if (game_ers)
        delete game_ers;

    game_ers = new ErsFile();

    if (!xv2fs->LoadFile(game_ers, GAME_ERS_PATH))
    {
        DPRINTF("%s: Failed to load %s.\n", FUNCNAME, GAME_ERS_PATH);
        return false;
    }

    return true;
}

bool Xenoverse2::CommitVfx()
{
    if (!xv2fs)
        return false;

    if (!game_ers)
        return true; // Yes, true

    if (!xv2fs->SaveFile(game_ers, GAME_ERS_PATH))
    {
        DPRINTF("%s: Failed to save %s.\n", FUNCNAME, GAME_ERS_PATH);
        return false;
    }

    return true;
}
