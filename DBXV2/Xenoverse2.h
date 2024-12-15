#ifndef __XENOVERSE2_H__
#define __XENOVERSE2_H__

#include "Xv2Fs.h"
#include "CharaListFile.h"
#include "Xv2PatcherSlotsFile.h"
#include "MsgFile.h"
#include "CmsFile.h"
#include "CusFile.h"
#include "CsoFile.h"
#include "PscFile.h"
#include "AurFile.h"
#include "SevFile.h"
#include "CmlFile.h"
#include "HciFile.h"
#include "Xv2PreBakedFile.h"
#include "IdbFile.h"
#include "PalFile.h"
#include "TnlFile.h"
#include "IkdFile.h"
#include "VlcFile.h"
#include "Xv2SavFile.h"

#include "PsaFile.h"
#include "PupFile.h"
#include "BcsFile.h"
#include "Xv2StageDefFile.h"
#include "Xv2PatcherSlotsFileStage.h"
#include "TtbFile.h"
#include "TtcFile.h"
#include "CncFile.h"
#include "CnsFile.h"
#include "ErsFile.h"

#include "Criware/AcbFile.h"
#include "Criware/Afs2File.h"
#include "Criware/HcaFile.h"

#include "Dimps/EmbFile.h"

#include "Misc/IggyFile.h"

// Order is not the one the game uses
enum
{
    XV2_LANG_ENGLISH,
    XV2_LANG_SPANISH1,
    XV2_LANG_SPANISH2,
    XV2_LANG_FRENCH,
    XV2_LANG_GERMAN,
    XV2_LANG_ITALIAN,
    XV2_LANG_PORTUGUESE,
    XV2_LANG_POLISH,
    XV2_LANG_RUSSIAN,
    XV2_LANG_CHINESE1,
    XV2_LANG_CHINESE2,
    XV2_LANG_KOREAN,
    XV2_LANG_JAPANESE,

    XV2_LANG_NUM
};

// Internal codes of the DLC
// There are the lowest level dlc codes, used by the function that check the dlc ownership
// Some parts of the game may have different codes that ultimately translate to these
enum
{
    XV2_DLC_GKB = 0,
    XV2_DLC1 = 5,
    XV2_DLC2 = 6,
    XV2_DLC3 = 7,
    XV2_DLC4 = 8,
    XV2_DLC_STEVE_AOKI = 0x12,
    XV2_DLC_BGM1 = 0x13, /* this and next... are they released at any region/plattform? */
    XV2_DLC_BGM2 = 0x14,
    XV2_DLC_XV1_STORY = 0x15, /* NX... exclusive? */
    XV2_DLC_EXTRA1 = 0x16,
    XV2_DLC_EXTRA2 = 0x17,
};

#include "X2mCostumeFile.h"

extern const std::vector<std::string> xv2_lang_codes;
extern Xv2PatcherSlotsFile *chara_list;
extern IggyFile *charasele;

extern CmsFile *game_cms;
extern CusFile *game_cus;
extern CsoFile *game_cso;
extern PscFile *game_psc;
extern AurFile *game_aur;
extern SevFile *game_sev;
extern CmlFile *game_cml;
extern HciFile *game_hci;
extern PsaFile *game_psa;
extern PupFile *game_pup;

extern std::vector<MsgFile *> game_chara_names;
extern std::vector<MsgFile *> game_chara_costume_names;

extern std::vector<MsgFile *> sup_sk_names;
extern std::vector<MsgFile *> ult_sk_names;
extern std::vector<MsgFile *> eva_sk_names;
extern std::vector<MsgFile *> awa_sk_names;

extern std::vector<MsgFile *> sup_sk_descs;
extern std::vector<MsgFile *> ult_sk_descs;
extern std::vector<MsgFile *> eva_sk_descs;
extern std::vector<MsgFile *> awa_sk_descs;

extern std::vector<MsgFile *> quest_btlhud_texts;

extern EmbFile *game_sel_port;
extern Xv2PreBakedFile *game_prebaked;

extern std::vector<MsgFile *> game_lobby_texts;
extern std::vector<MsgFile *> game_cac_costume_names;
extern std::vector<MsgFile *> game_accesory_names;
extern std::vector<MsgFile *> game_talisman_names;
extern std::vector<MsgFile *> game_material_names;
extern std::vector<MsgFile *> game_battle_names;
extern std::vector<MsgFile *> game_extra_names;
extern std::vector<MsgFile *> game_pet_names;

extern std::vector<MsgFile *> game_cac_costume_descs;
extern std::vector<MsgFile *> game_accesory_descs;

extern IdbFile *game_bottom_idb;
extern IdbFile *game_gloves_idb;
extern IdbFile *game_shoes_idb;
extern IdbFile *game_top_idb;
extern IdbFile *game_accesory_idb;
extern IdbFile *game_talisman_idb;
extern IdbFile *game_skill_idb;
extern IdbFile *game_material_idb;
extern IdbFile *game_battle_idb;
extern IdbFile *game_extra_idb;
extern IdbFile *game_pet_idb;

extern PalFile *game_pal;
extern TnlFile *game_tnl;

extern AcbFile *game_css_acb_jp;
extern AcbFile *game_css_acb_en;
extern Afs2File *game_css_awb_jp;
extern Afs2File *game_css_awb_en;

extern AcbFile *game_sev_cmn_acb_jp;
extern AcbFile *game_sev_cmn_acb_en;
extern Afs2File *game_sev_cmn_awb_jp;
extern Afs2File *game_sev_cmn_awb_en;

extern AcbFile *game_sev_cmn2_acb_jp;
extern AcbFile *game_sev_cmn2_acb_en;
extern Afs2File *game_sev_cmn2_awb_jp;
extern Afs2File *game_sev_cmn2_awb_en;

extern BcsFile *game_hum_bcs;
extern BcsFile *game_huf_bcs;
extern BcsFile *game_nmc_bcs;
extern BcsFile *game_fri_bcs;
extern BcsFile *game_mam_bcs;
extern BcsFile *game_maf_bcs;

extern X2mCostumeFile *game_costume_file;

extern Xv2PatcherSlotsFileStage *game_stage_slots_file, *game_stage_slots_file_local;

extern Xv2StageDefFile *game_stage_def;
extern std::vector<MsgFile *> game_stage_names;

extern EmbFile *game_stage01_emb, *game_stage02_emb;

extern TtbFile *game_ttb;
extern TtcFile *game_ttc;

extern MsgFile *qc_dialogue_voice;
extern std::vector<MsgFile *> qc_dialogue_subs;

extern CncFile *game_cnc;
extern CnsFile *game_cns;

extern ErsFile *game_ers;

extern IkdFile *game_ikd_battle, *game_ikd_lobby;

extern VlcFile *game_vlc;

extern int global_lang;

namespace Xenoverse2
{
    void InitFs(const std::string &game_path);

    bool LoadMsgs(const std::string &base_path, std::vector<MsgFile *> &msgs, int only_this_lang);
    bool SaveMsgs(const std::string &base_path, const std::vector<MsgFile *> &msgs, bool mandatory_load);
    bool GetMsgTextByIndex(const std::vector<MsgFile *> &msgs, uint32_t idx, std::string &text, int lang);
    bool SetMsgTextByIndex(std::vector<MsgFile *> &msgs, uint32_t idx, const std::string &text, int lang);
    bool GetMsgTextByName(const std::vector<MsgFile *> &msgs, const std::string &entry_name, std::string &text, int lang);
    bool SetMsgTextByName(std::vector<MsgFile *> &msgs, const std::string &entry_name, const std::string &text, int lang);
    bool RemoveMsgTextByIndex(std::vector<MsgFile *> &msgs, uint32_t idx, int lang);
    bool RemoveMsgTextByName(std::vector<MsgFile *> &msgs, const std::string &entry_name, int lang);

    bool InitCharaList();
    bool InitSystemFiles(bool only_cms=false, bool multiple_hci=false);
    bool InitCharaNames(int only_this_lang=-1);
    bool InitCharaCostumeNames(int only_this_lang=-1);
    bool InitSkillNames(int only_this_lang=-1);
    bool InitSkillDescs(int only_this_lang=-1);
    bool InitSkillHows(int only_this_lang=-1);
    bool InitBtlHudText(int only_this_lang=-1);
    bool InitSelPort();
    bool InitPreBaked();
    bool InitLobbyText(int only_this_lang=-1);
    bool InitCacCostumeNames(int only_this_lang=-1);
    bool InitCacCostumeDescs(int only_this_lang=-1);
    bool InitTalismanNames(int only_this_lang=-1);
    bool InitTalismanDescs(int only_this_lang=-1);
    bool InitTalismanHows(int only_this_lang=-1);
    bool InitMaterialNames(int only_this_lang=-1);
    bool InitBattleNames(int only_this_lang=-1);
    bool InitExtraNames(int only_this_lang=-1);
    bool InitPetNames(int only_this_lang=-1);
    // TODO (also for CommitIdb): change infinite params to a single param flags
    bool InitIdb(bool costumes=true, bool accesories=true, bool talisman=true, bool skills=true, bool material=false, bool battle=false, bool extra=false, bool pet=false);
    bool InitLobby(bool tnl=false);
    bool InitSound(bool load_sev_cmn=false);
    bool InitBgm();
    bool InitCac();
    bool InitCostumeFile();
    bool InitStageSlots();
    bool InitStageNames(int only_this_lang=-1);
    bool InitStageEmb();
    bool InitCommonDialogue();
    bool InitDualSkill(bool init_cnc, bool init_cns);
    bool InitVfx();
    bool InitShopText(int only_this_lang=-1);

    bool CommitCharaList(bool commit_slots, bool commit_iggy);
    bool CommitSystemFiles(bool pup, bool ikd, bool vlc);
    bool CommitSelPort();
    bool CommitCharaNames();
    bool CommitCharaCostumeNames();
    bool CommitSkillNames();
    bool CommitSkillDescs();
    bool CommitSkillHows();
    bool CommitBtlHudText();
    bool CommitPreBaked();
    bool CommitLobbyText();
    bool CommitCacCostumeNames();
    bool CommitCacCostumeDescs();
    bool CommitTalismanNames();
    bool CommitTalismanDescs();
    bool CommitTalismanHows();
    bool CommitLobby(bool tnl=false);
    bool CommitIdb(bool costumes, bool accesories, bool talisman, bool skills);
    bool CommitSound(bool css, bool sev, bool sev1, bool sev2);
    bool CommitBgm();
    bool CommitCac();
    bool CommitCostumeFile();
    bool CommitStageSlots();
    bool CommitStageNames();
    bool CommitStageEmb();
    bool CommitCommonDialogue();
    bool CommitDualSkill(bool commit_cnc, bool commit_cns);
    bool CommitVfx();
    bool CommitShopText();

    bool GetCharaCodeFromId(uint32_t id, std::string &code);

    bool IsOriginalChara(const std::string &short_name);
    bool IsForbiddenNewStageName(const std::string &name);
    bool CompileCharaSel(const std::string &chasel_path, const std::string &compiler);

    bool GetCharaName(const std::string &code, std::string &name, int lang=XV2_LANG_ENGLISH, int index=0);
    bool GetCharaCostumeName(const std::string &code, int var, int model_preset, std::string &name, int lang=XV2_LANG_ENGLISH);

    bool SetCharaName(const std::string &code, const std::string &name, int lang, int index=0);
    bool SetCharaCostumeName(const std::string &code, int var, int model_preset, const std::string &name, int lang);

    bool RemoveCharaName(const std::string &code, int lang, int index=0);
    bool RemoveAllCharaName(const std::string &code, int lang);
    bool RemoveCharaCostumeName(const std::string &code, int var, int model_preset, int lang);

    bool GetSuperSkillName(uint16_t name_id, std::string &name, int lang=XV2_LANG_ENGLISH);
    bool GetUltimateSkillName(uint16_t name_id, std::string &name, int lang=XV2_LANG_ENGLISH);
    bool GetEvasiveSkillName(uint16_t name_id, std::string &name, int lang=XV2_LANG_ENGLISH);
    bool GetAwakenSkillName(uint16_t name_id, std::string &name, int lang=XV2_LANG_ENGLISH);

    bool SetSuperSkillName(uint16_t name_id, const std::string &name, int lang);
    bool SetUltimateSkillName(uint16_t name_id, const std::string &name, int lang);
    bool SetEvasiveSkillName(uint16_t name_id, const std::string &name, int lang);
    bool SetAwakenSkillName(uint16_t name_id, const std::string &name, int lang);

    bool RemoveSuperSkillName(uint16_t name_id, int lang);
    bool RemoveUltimateSkillName(uint16_t name_id, int lang);
    bool RemoveEvasiveSkillName(uint16_t name_id, int lang);
    bool RemoveAwakenSkillName(uint16_t name_id, int lang);

    bool GetSuperSkillDesc(uint16_t name_id, std::string &desc, int lang=XV2_LANG_ENGLISH);
    bool GetUltimateSkillDesc(uint16_t name_id, std::string &desc, int lang=XV2_LANG_ENGLISH);
    bool GetEvasiveSkillDesc(uint16_t name_id, std::string &desc, int lang=XV2_LANG_ENGLISH);
    bool GetAwakenSkillDesc(uint16_t name_id, std::string &desc, int lang=XV2_LANG_ENGLISH);

    bool SetSuperSkillDesc(uint16_t name_id, const std::string &desc, int lang);
    bool SetUltimateSkillDesc(uint16_t name_id, const std::string &desc, int lang);
    bool SetEvasiveSkillDesc(uint16_t name_id, const std::string &desc, int lang);
    bool SetAwakenSkillDesc(uint16_t name_id, const std::string &desc, int lang);

    bool RemoveSuperSkillDesc(uint16_t name_id, int lang);
    bool RemoveUltimateSkillDesc(uint16_t name_id, int lang);
    bool RemoveEvasiveSkillDesc(uint16_t name_id, int lang);
    bool RemoveAwakenSkillDesc(uint16_t name_id, int lang);

    bool GetSuperSkillHow(uint16_t name_id, std::string &how, int lang=XV2_LANG_ENGLISH);
    bool GetUltimateSkillHow(uint16_t name_id, std::string &how, int lang=XV2_LANG_ENGLISH);
    bool GetEvasiveSkillHow(uint16_t name_id, std::string &how, int lang=XV2_LANG_ENGLISH);
    bool GetAwakenSkillHow(uint16_t name_id, std::string &how, int lang=XV2_LANG_ENGLISH);

    bool SetSuperSkillHow(uint16_t name_id, const std::string &how, int lang);
    bool SetUltimateSkillHow(uint16_t name_id, const std::string &how, int lang);
    bool SetEvasiveSkillHow(uint16_t name_id, const std::string &how, int lang);
    bool SetAwakenSkillHow(uint16_t name_id, const std::string &how, int lang);

    bool RemoveSuperSkillHow(uint16_t name_id, int lang);
    bool RemoveUltimateSkillHow(uint16_t name_id, int lang);
    bool RemoveEvasiveSkillHow(uint16_t name_id, int lang);
    bool RemoveAwakenSkillHow(uint16_t name_id, int lang);

    bool GetBtlHudAwakenName(uint16_t name_id, uint16_t trans_stage, std::string &name, int lang=XV2_LANG_ENGLISH);
    bool SetBtlHudAwakenName(uint16_t name_id, uint16_t trans_stage, const std::string &name, int lang);
    bool RemoveBtlHudAwakenName(uint16_t name_id, uint16_t trans_stage, int lang);

    uint8_t *GetSelPortrait(const std::string &name, size_t *psize);
    bool SetSelPortrait(const std::string &name, const uint8_t *buf, size_t size);
    bool RemoveSelPortrait(const std::string &name);

    bool GetLobbyName(uint32_t name_id, std::string &name, int lang=XV2_LANG_ENGLISH);
    bool SetLobbyName(uint32_t name_id, const std::string &name, int lang=XV2_LANG_ENGLISH);
    bool RemoveLobbyName(uint32_t name_id, int lang);

    bool GetCacCostumeName(uint32_t name_idx, std::string &name, int lang=XV2_LANG_ENGLISH);
    bool SetCacCostumeName(uint32_t name_idx, const std::string &name, int lang);
    bool AddCacCostumeName(const std::string &entry_name, const std::string &name, int lang, uint16_t *ret_idx);
    bool RemoveCacCostumeName(uint32_t name_idx, int lang, bool update_idb);

    bool GetAccesoryName(uint32_t name_idx, std::string &name, int lang=XV2_LANG_ENGLISH);
    bool SetAccesoryName(uint32_t desc_idx, const std::string &name, int lang);
    bool AddAccesoryName(const std::string &name, int lang, uint16_t *ret_idx);
    bool RemoveAccesoryName(uint32_t name_idx, int lang, bool update_idb);

    bool GetCacCostumeDesc(uint32_t desc_idx, std::string &desc, int lang=XV2_LANG_ENGLISH);
    bool SetCacCostumeDesc(uint32_t desc_idx, const std::string &desc, int lang);
    bool AddCacCostumeDesc(const std::string &entry_name, const std::string &desc, int lang, uint16_t *ret_idx);
    bool RemoveCacCostumeDesc(uint32_t desc_idx, int lang, bool update_idb);

    bool GetAccesoryDesc(uint32_t desc_idx, std::string &desc, int lang=XV2_LANG_ENGLISH);
    bool SetAccesoryDesc(uint32_t desc_idx, const std::string &desc, int lang);
    bool AddAccesoryDesc(const std::string &desc, int lang, uint16_t *ret_idx);
    bool RemoveAccesoryDesc(uint32_t desc_idx, int lang, bool update_idb);

    bool GetTalismanName(uint32_t name_idx, std::string &name, int lang=XV2_LANG_ENGLISH);
    bool SetTalismanName(uint32_t name_idx, const std::string &name, int lang);
    bool AddTalismanName(const std::string &name, int lang, uint16_t *ret_idx);
    bool RemoveTalismanName(uint32_t name_idx, int lang, bool update_idb);

    bool GetTalismanDesc(uint32_t desc_idx, std::string &desc, int lang=XV2_LANG_ENGLISH);
    bool SetTalismanDesc(uint32_t desc_idx, const std::string &desc, int lang);
    bool AddTalismanDesc(const std::string &desc, int lang, uint16_t *ret_idx);
    bool RemoveTalismanDesc(uint32_t desc_idx, int lang, bool update_idb);

    bool GetTalismanHow(uint32_t how_idx, std::string &how, int lang=XV2_LANG_ENGLISH);
    bool SetTalismanHow(uint32_t how_idx, const std::string &how, int lang);
    bool AddTalismanHow(const std::string &how, int lang, uint16_t *ret_idx);
    bool RemoveTalismanHow(uint32_t how_idx, int lang, bool update_idb);

    uint32_t GetBlastFromTalisman(uint32_t talisman_id, bool request_id2);
    bool SetBlastToTalisman(uint32_t talisman_id, uint16_t skill_id, bool is_id2);
    uint32_t GetModelForTalisman(uint16_t skill_id, bool is_id2);

    bool GetMaterialName(uint32_t name_idx, std::string &name, int lang=XV2_LANG_ENGLISH);
    bool GetBattleName(uint32_t name_idx, std::string &name, int lang=XV2_LANG_ENGLISH);
    bool GetExtraName(uint32_t name_idx, std::string &name, int lang=XV2_LANG_ENGLISH);
    bool GetPetName(uint32_t name_idx, std::string &name, int lang=XV2_LANG_ENGLISH);

    bool GetShopText(const std::string &entry_name, std::string &name, int lang=XV2_LANG_ENGLISH);
    bool SetShopText(const std::string &entry_name, const std::string &name, int lang);
    bool RemoveShopText(const std::string &entry_name, int lang);
    bool SetModBlastSkillType(uint16_t id2, const std::string &type, int lang);
    bool RemoveModBlastSkillType(uint16_t id2, int lang);
    bool GetBlastType(uint32_t type, std::string &name, int lang=XV2_LANG_ENGLISH);
    bool GetAllDefaultBlastTypes(std::vector<std::string> &out, int lang=XV2_LANG_ENGLISH);

    /* Generic audio functions */
    bool SetAcbAwbData(AcbFile *acb, AwbFile *awb);
    bool SetSound(AcbFile *acb, Afs2File *awb, uint32_t cue_id, HcaFile &hca, bool is_external_awb=true);
    uint32_t SetSound(AcbFile *acb, Afs2File *awb, const std::string &name, HcaFile &hca, bool is_external_awb=true, int use_cmd1=-1, int use_cmd2=-1, bool loop=false);
    bool FreeSound(AcbFile *acb,  const std::string &name);
    /***************************/

    HcaFile *GetCssSound(uint32_t cue_id, bool english);
    HcaFile *GetCssSound(const std::string &name, bool english);
    bool SetCssSound(uint32_t cue_id, bool english, HcaFile &hca);
    uint32_t SetCssSound(const std::string &name, bool english, HcaFile &hca, int use_cmd1=-1, int use_cmd2=-1);
    bool FreeCssSound(const std::string &name, bool english);

    bool SetSevCmnSound(uint32_t cue_id, bool english, HcaFile &hca, bool is_global_cue_id);
    uint32_t SetSevCmnSound(const std::string &name, bool english, HcaFile &hca, bool return_global_cue_id);
    uint32_t GetSevCmnCueId(const std::string &name, bool english, bool *is_sev2, bool return_global_cue_id);
    bool FreeSevCmnSound(const std::string &name, bool english);

    bool SetBgmSound(uint32_t cue_id, HcaFile &hca);
    uint32_t SetBgmSound(const std::string &name, HcaFile &hca);
    uint32_t GetBgmCueId(const std::string &name);    
    bool FreeBgmSound(const std::string &name);

    uint32_t CusAuraToAurAura(uint32_t id);
    void GetAuraExtra(int32_t id, AuraExtraData &extra);

    bool GetStageName(const std::string &id, std::string &name, int lang=XV2_LANG_ENGLISH);
    bool SetStageName(const std::string &id, const std::string &name, int lang);
    bool RemoveStageName(const std::string &id, int lang);

    bool GetTtbSubtitle(const std::string &event_name, std::string &subtitle, int lang=XV2_LANG_ENGLISH);
    bool SetTtbSubtitle(const std::string &event_name, const std::string &subtitle, int lang);
    bool RemoveTtbSubtitle(const std::string &event_name, int lang, bool *existed);

    bool GetTtbVoice(const std::string &event_name, std::string &voice);
    bool SetTtbVoice(const std::string &event_name, const std::string &voice);
    bool RemoveTtbVoice(const std::string &event_name, bool *existed);

    // These may be moved to utils...
    std::string UnescapeHtml(const std::string &str);
    std::string EscapeHtml(const std::string &str);

    // Higher level api
    std::string GetCharaAndCostumeName(const std::string &code, uint32_t costume_id, uint32_t model_preset, int lang=XV2_LANG_ENGLISH);
    std::string GetCharaAndCostumeName(uint32_t char_id, uint32_t costume_id, uint32_t model_preset, bool ignore_unknown, int lang=XV2_LANG_ENGLISH);

    std::string GetCacTopName(uint16_t item_id, int lang=XV2_LANG_ENGLISH);
    std::string GetCacBottomName(uint16_t item_id, int lang=XV2_LANG_ENGLISH);
    std::string GetCacGlovesName(uint16_t item_id, int lang=XV2_LANG_ENGLISH);
    std::string GetCacShoesName(uint16_t item_id, int lang=XV2_LANG_ENGLISH);
    std::string GetCacAccesoryName(uint16_t item_id, int lang=XV2_LANG_ENGLISH);
    std::string GetTalismanNameEx(uint16_t item_id, int lang=XV2_LANG_ENGLISH);
    std::string GetMaterialNameEx(uint16_t item_id, int lang=XV2_LANG_ENGLISH);
    std::string GetExtraNameEx(uint16_t item_id, int lang=XV2_LANG_ENGLISH);
    std::string GetBattleNameEx(uint16_t item_id, int lang=XV2_LANG_ENGLISH);
    std::string GetPetNameEx(uint16_t item_id, int lang=XV2_LANG_ENGLISH);

    std::string GetSkillDirectory(const CusSkill &skill, std::string *last_part=nullptr);

    std::vector<std::string> GetCostumeNamesForPartSet(uint32_t partset, uint8_t race_lock=0xFF);
    std::string GetCostumeNamesForPartSet2(uint32_t partset, uint8_t race_lock=0xFF);

    // Misc
    bool IsModCms(uint32_t cms_id);
}

extern Xv2Fs *xv2fs;

#endif // __XENOVERSE2_H__
