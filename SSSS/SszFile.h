#ifndef __SSZFILE_H__
#define __SSZFILE_H__

#include <stdint.h>
#include <stdlib.h>

#include <string>
#include <vector>

#include <ZipFile.h>

#include "SlotsFile.h"
#include "CmsFile.h"
#include "CdcFile.h"
#include "CspFile.h"
#include "RpdFile.h"
#include "GpdFile.h"
#include "BgrFile.h"
#include "SstFile.h"

//
// File structure
//
//
// sszmod.xml
// XXX.emo (required for NEW_CHARACTER)
// XXX.emb (required for NEW_CHARACTER)
// XXX.emm (required for NEW_CHARACTER)
// XXX.2ry (required for NEW_CHARACTER)
// (where XXX must match entry_name)
// resource (folder, optional for NEW_CHARACTER, NOT optional for REPLACER type)
 
enum class SszType
{
	DUMMY = -1,
	NEW_CHARACTER,
	REPLACER,
};
 
struct InstallData
{
    // Input parameters
    uint32_t slot;
	uint32_t sub_slot;
	SlotsFile *slf;
	CmsFile *cms;
	CdcFile *cdc;
	CspFile *csp;
	RpdFile *rpd;
    GpdFile *gpd;
    BgrFile *bgr;
    // Input on Update, output on Install
    uint32_t cms_entry;
    uint32_t cms_model_spec_idx;
    // Output parameters
    uint32_t model_id; // Only on Install, unused on Update
    std::vector<std::string> installed_files;
};
 
class SszFile : public BaseFile
{
private:

    //const size_t BATTLE_NAME_FILE_SIZE = 23168;
    const size_t ICON_BIG_FILE_SIZE = 4224;
    const size_t ICON_SMALL_FILE_SIZE = 2064;
    const size_t SELECT_NAME_FILE_SIZE = 43136;
    const size_t SELECT2_NAME_FILE_SIZE = 10048;
    const size_t GWR_NAME_FILE_SIZE = 23168;
    const size_t BANNER_FILE_SIZE = 40832;
    const size_t BANNER_NAME_FILE_SIZE = 18560;

    bool loaded;

	/* Required for ALL */
	float format_version;
	std::string mod_name;	
	uint8_t guid[16];  

	/* Optional for ALL */
	SszType type; // Defaults to NEW_CHARACTER when not found.
	std::string mod_author;
	float mod_version;

	/* Required for NEW_CHARACTER */
	char entry_name[4];
	ModelSpec spec;
	CdcEntry cdc_entry;
	CspEntry csp_entry;
	RpdEntry rpd_entry;
	// These 4 go together
	Slot slot_entry;
	CharacterInfo info_entry;
	CharacterInfo2 info2_entry;
    CharacterInfo3 info3_entry;
    CharacterInfo4 info4_entry;
	std::string categories;
    // Gpd, new in 2.3
    GpdEntry gpd_entry;
    std::vector<uint32_t> gwd_tournaments;
    // Bgr, new in 2.4
    std::vector<BgrEntry> bgr_entries;
	
	//
	ZipArchive::Ptr zip;
	
	std::string emo, emb, emm, _2ry;
    std::vector<std::string> battle_name;
    std::vector<std::string> select_name;
    std::vector<std::string> select2_name;
    std::vector<std::string> gwr_name;
    std::vector<std::string> banner_name;

    SstFile sst;

	void Reset();

    uint8_t *ReadFile(const std::string &path, size_t *psize, bool show_error=true);
    char *ReadTextFile(const std::string &path, bool show_error=true);
	
    bool FileExists(const std::string & file);
    size_t GetFileSize(const std::string & file);

	bool HasResourceFolder();
    bool CopyFile(const std::string &local_file, const std::string &native_file);
    bool CopyResourceDirectory(const std::string &install_directory);
	
    bool GetParamUnsigned2(const TiXmlElement *root, const char *name, uint32_t *value);
    bool GetParamFloat2(const TiXmlElement *root, const char *name, float *value);

    bool ReadSpec(const TiXmlElement *root);
    bool ReadCdcEntry(const TiXmlElement *root);
    bool ReadCspEntry(const TiXmlElement *root);
    bool ReadRpdEntry(const TiXmlElement *root);
    bool ReadSlotEntry(const TiXmlElement *root);
    bool ReadGpdEntry(const TiXmlElement *root);
    bool ReadBgrEntry(const TiXmlElement *root);
	
	bool ParseXml(TiXmlDocument *doc);	

    bool BuildBattleName();
    std::string GetBattleNameTarget(const std::string & src, uint32_t cms_entry, uint32_t cms_model_spec_idx);

    bool CheckIcon();

    bool BuildSelectName();
    std::string GetSelectNameTarget(const std::string &src, uint32_t cms_entry, uint32_t cms_model_spec_idx);

    bool BuildSelect2Name();
    std::string GetSelect2NameTarget(const std::string &src, uint32_t cms_entry, uint32_t cms_model_spec_idx);

    bool BuildGwrName();
    std::string GetGwrNameTarget(const std::string &src, uint32_t cms_entry, uint32_t cms_model_spec_idx);

    bool CheckBanner();

    bool BuildBannerName();
    std::string GetBannerNameTarget(const std::string &src, uint32_t cms_entry, uint32_t cms_model_spec_idx);

    bool InstallExtraFiles(InstallData *install_data, const std::string &install_directory);
	   
public:

    const float MAX_SSZ_FORMAT_VERSION = 2.4f;

    const float MIN_VERSION_AVATAR = 1.2f;
    const float MIN_VERSION_BATTLE_NAME = 2.0f;
    const float MIN_VERSION_TEXT = 2.0f;
    const float MIN_VERSION_ICON = 2.0f;
    const float MIN_VERSION_SELECT_NAME = 2.0f;
    const float MIN_VERSION_SELECT2_NAME = 2.0f;
    const float MIN_VERSION_GWR_NAME = 2.0f;
    const float MIN_VERSION_BANNER = 2.0f;
    const float MIN_VERSION_BANNER_NAME = 2.0f;

    const float MIN_VERSION_CI3_CI4 = 2.0f;

    const float MIN_VERSION_GPD = 2.3f;

    const float MIN_VERSION_BGR = 2.4f;
 
	SszFile();
	virtual ~SszFile();

	inline SszType GetType()
	{
		return type;
	}
	
	inline float GetFormatVersion()
	{
		return format_version;
	}

    inline const std::string GetModName()
	{
		return mod_name;
	}

    inline const std::string GetModAuthor()
	{
		return mod_author;
	}
	
	inline float GetModVersion()
	{
        return mod_version;
	}

    inline uint8_t *GetModGuid()
	{
		if (!loaded)
			return NULL;
	   
		return guid;
	}
	
	const char *GetEntryName()
	{
		if (!loaded)
			return NULL;
		
		return entry_name;
	}
	
	inline ModelSpec *GetModelSpec()
	{
		if (!loaded || type != SszType::NEW_CHARACTER)
			return NULL;
		
		return &spec;		
	}	
	
	inline CdcEntry *GetCdcEntry()
	{
		if (!loaded || type != SszType::NEW_CHARACTER)
			return NULL;
		
		return &cdc_entry;
	}
	
	inline CspEntry *GetCspEntry()
	{
		if (!loaded || type != SszType::NEW_CHARACTER)
			return NULL;
		
		return &csp_entry;
	}
	
	inline RpdEntry *GetRpdEntry()
	{
		if (!loaded || type != SszType::NEW_CHARACTER)
			return NULL;
		
		return &rpd_entry;
	}
	
	inline Slot *GetSlotEntry()
	{
		if (!loaded || type != SszType::NEW_CHARACTER)
			return NULL;
		
		return &slot_entry;
	}
	
	inline CharacterInfo *GetCharacterInfoEntry()
	{
		if (!loaded || type != SszType::NEW_CHARACTER)
			return NULL;
		
		return &info_entry;
	}
	
	inline CharacterInfo2 *GetCharacterInfo2Entry()
	{
		if (!loaded || type != SszType::NEW_CHARACTER)
			return NULL;
		
		return &info2_entry;
	}
	
	inline std::string & GetCategories()
	{
		return categories;
	}

	inline bool IsType(SszType type)
	{
		if (!loaded)
			return false;
	   
		return (type == this->type);
	}
	
	inline bool FormatVersionLowerOrEqualThan(float version)
	{
		if (!loaded)
			return false;
	   
		return (this->format_version <= version);
	}
	
	inline bool ModVersionLowerOrEqualThan(float version)
	{
		if (!loaded)
			return false;
	   
		return (this->mod_version <= version);
	}
	
	inline bool IsGuid(uint8_t *guid)
	{
		if (!loaded)
			return false;
	   
		return (memcmp(guid, this->guid, 16) == 0);
	} 

    size_t GetResourceFilesList(std::vector<std::string> & files_list, bool clear=true);

    bool HasPreview();
    uint8_t *GetPreview(size_t *psize);
    bool SavePreview(const std::string &output, bool is_dir);

    bool HasAvatar();
    bool HasBattleName();
    bool HasText();
    bool HasIcon();
    bool HasSelectName();
    bool HasSelect2Name();
    bool HasGwrName();
    bool HasBanner();
    bool HasBannerName();

    virtual bool LoadFromFile(const std::string &path, bool show_error=true) override;   

    bool Install(InstallData *install_data, const std::string &install_directory);
    bool Update(InstallData *install_data, const std::string &install_directory);
};

#endif
