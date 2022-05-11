#include <stdio.h>
#include <math.h>

#include "SszFile.h"
#include "Utils.h"
#include "debug.h"

#define COPY_BUF_SIZE	(64*1024*1024)

#define BATTLE_STEAM_CACHE  "resource/sssspatcher/avatars/cache.bin"
#define CHR_NAME_CACHE		"resource/sssspatcher/chr_name/cache.bin"
#define CHA_SEL_CACHE		"resource/sssspatcher/cha_sel/cache.bin"
#define GWR_CACHE			"resource/sssspatcher/gwr/cache.bin"
#define GWT_CACHE			"resource/sssspatcher/gwt/cache.bin"
#define VS_CACHE			"resource/sssspatcher/vs/cache.bin"

#define AVATARS_DIRECTORY               "resource/sssspatcher/avatars/"
#define CHR_NAME_DIRECTORY              "resource/sssspatcher/chr_name/"
#define ICONS_DIRECTORY_BIG             "resource/sssspatcher/icons_big/"
#define ICONS_DIRECTORY_SMALL           "resource/sssspatcher/icons_small/"
#define SN_DIRECTORY                    "resource/sssspatcher/cha_sel/select_names/"
#define SN2_DIRECTORY                   "resource/sssspatcher/cha_sel/select2_names/"
#define GWR_NAMES_DIRECTORY             "resource/sssspatcher/gwr/names/"
#define GWT_BANNERS_DIRECTORY			"resource/sssspatcher/gwt/banners/"
#define GWT_BANNER_NAMES_DIRECTORY		"resource/sssspatcher/gwt/banner_names/"

#define CHECK_LOADED_BOOL() { if (!this->loaded) { return false; } }

static std::vector<std::string> languages =
{
    "JP",
    "EN",
    "FR",
    "IT",
    "SP",
    "BR",
    "CH",
    "NS",
};

SszFile::SszFile()
{
	Reset();
}

SszFile::~SszFile()
{
	Reset();
}

void SszFile::Reset()
{
	loaded = false;
	
	big_endian = false;
	type = SszType::DUMMY;
	format_version = NAN;
	mod_name = "";
	mod_author = "";
	mod_version = NAN;
	categories = "";
	zip = nullptr;
}

uint8_t *SszFile::ReadFile(const std::string & path, size_t *psize, bool show_error)
{
	ZipArchiveEntry::Ptr entry;
	std::istream *is;
	uint8_t *buf;
	
	entry = zip->GetEntry(path);
	if (!entry)
	{
		if (show_error)
			DPRINTF("Cannot find file \"%s\" in the archive.\n", path.c_str());
		
        return nullptr;
	}
	
	*psize = entry->GetSize();
	buf = new uint8_t[*psize];

	is = entry->GetDecompressionStream();
	is->read((char *)buf, *psize);
	
	if ((size_t)is->gcount() != *psize)
	{
		if (show_error)
        {
			DPRINTF("%s: The number of bytes we read are not same as requested!\n", __PRETTY_FUNCTION__);
        }
		
		delete[] buf;
        buf = nullptr;
	}
	
	entry->CloseDecompressionStream();	
	return buf;
}

char *SszFile::ReadTextFile(const std::string & path, bool show_error)
{
	ZipArchiveEntry::Ptr entry;
	std::istream *is;
	char *buf;
	size_t size;
	
	entry = zip->GetEntry(path);
	if (!entry)
	{
		if (show_error)
			DPRINTF("Cannot find file \"%s\" in the archive.\n", path.c_str());
		
        return nullptr;
	}
	
	size = entry->GetSize();
    buf = new char[size+1];
	buf[size] = 0;
	
	is = entry->GetDecompressionStream();
	is->read(buf, size);
	
	if ((size_t)is->gcount() != size)
	{
		if (show_error)
			DPRINTF("%s: The number of bytes we read are not same as requested!\n", __PRETTY_FUNCTION__);
		
		delete[] buf;
        buf = nullptr;
	}
	
	entry->CloseDecompressionStream();	
	return buf;
}

bool SszFile::FileExists(const std::string & file)
{
	ZipArchiveEntry::Ptr res;
		
	res = zip->GetEntry(file);
	if (!res)
		return false;
	
    return !res->IsDirectory();
}

size_t SszFile::GetFileSize(const std::string & file)
{
    ZipArchiveEntry::Ptr f;

    f = zip->GetEntry(file);
    if (!f || f->IsDirectory())
        return (size_t)-1;

    return f->GetSize();
}

bool SszFile::HasResourceFolder()
{
	ZipArchiveEntry::Ptr res;
		
	res = zip->GetEntry("resource");
	if (!res)
		res = zip->GetEntry("resource/");
		if (!res)
			res = zip->GetEntry("resource\\");
			if (!res)
				return false;
			
	return res->IsDirectory();		
}

bool SszFile::CopyFile(const std::string & local_file, const std::string & native_file)
{
	ZipArchiveEntry::Ptr input;
	FILE *output;
	std::istream *is;
	
	uint8_t *buf;
	size_t file_size;
	size_t currently_copied;
	size_t request_size;
	
	input = zip->GetEntry(local_file);
	if (!input)
	{
		DPRINTF("Cannot open the file \"%s\" inside the .ssz.\n", local_file.c_str());
		return false;
	}
	
	if (input->IsDirectory())
	{
		DPRINTF("Cannot open the file \"%s\" inside the .ssz because it is a directory.\n", local_file.c_str());
		return false;
	}
	
	file_size = input->GetSize();
	
	if (file_size < COPY_BUF_SIZE)
	{
		buf = new uint8_t[file_size];
	}
	else
	{
		buf = new uint8_t[COPY_BUF_SIZE];
	}
	
    output = Utils::fopen_create_path(native_file.c_str(), "wb");
	if (!output)
	{
		DPRINTF("Cannot create file \"%s\"\n", native_file.c_str());
		delete[] buf;
		
		return false;
	}
	
	is = input->GetDecompressionStream();
	currently_copied = 0;
	
	do
	{
		request_size = file_size - currently_copied;		
		if (request_size > COPY_BUF_SIZE)
			request_size = COPY_BUF_SIZE;
		
		is->read((char *)buf, request_size);
	
		if ((size_t)is->gcount() != request_size)
		{
            DPRINTF("%s: The number of bytes we read are not same as requested! (%d != %d) (on file %s)\n\n"
                    "Try compressing the .ssz with different dictionary size.\n", __PRETTY_FUNCTION__, is->gcount(), request_size, local_file.c_str());
			
			delete[] buf;
			input->CloseDecompressionStream();			
			return false;
		}
		
		if (fwrite(buf, 1, request_size, output) != request_size)
		{
			DPRINTF("Error while copying to file \"%s\"\n", native_file.c_str());
		}
		
		currently_copied += request_size;
	
	} while (currently_copied != file_size);
	
	input->CloseDecompressionStream();
	fclose(output);
	delete[] buf;
	
	return true;
}

size_t SszFile::GetResourceFilesList(std::vector<std::string> & files_list, bool clear)
{
	if (!loaded)
		return (size_t)-1;
	
    if (clear)
        files_list.clear();
	
	for (size_t i = 0; i < zip->GetEntriesCount(); i++)
	{
		ZipArchiveEntry::Ptr entry = zip->GetEntry(i);
		
		if (entry && !entry->IsDirectory())
		{
			std::string name = entry->GetFullName();
			
			if (strncasecmp(name.c_str(), "resource/", 9) == 0 || strncasecmp(name.c_str(), "resource\\", 9) == 0)
			{
				files_list.push_back(name);
			}
		}
	}
	
	return files_list.size();
}

bool SszFile::HasPreview()
{
    ZipArchiveEntry::Ptr prev;

    prev = zip->GetEntry("preview.png");
    return (prev && !prev->IsDirectory());
}

uint8_t *SszFile::GetPreview(size_t *psize)
{
    ZipArchiveEntry::Ptr prev;
    std::istream *is;

    prev = zip->GetEntry("preview.png");
    if (!prev || prev->IsDirectory())
        return nullptr;

    size_t file_size = prev->GetSize();

    uint8_t *ret = new uint8_t[file_size];

    is = prev->GetDecompressionStream();
    is->read((char *)ret, file_size);

    if ((size_t)is->gcount() != file_size)
    {
        DPRINTF("%s: couldn't read same amount of bytes as requested (%d != %d)\n", __PRETTY_FUNCTION__, is->gcount(), file_size);
        delete ret;
        prev->CloseDecompressionStream();
        return nullptr;
    }

    prev->CloseDecompressionStream();
    *psize = file_size;
    return ret;
}

bool SszFile::SavePreview(const std::string &output, bool is_dir)
{
    if (is_dir)
    {
        std::string file = output;

        if (file[file.length()-1] != '/' && file[file.length()-1] != '\\')
        {
            file += '\\';
        }

        file += Utils::GUID2String(guid) + ".png";
        return CopyFile("preview.png", file);
    }

    return CopyFile("preview.png", output);
}

bool SszFile::HasAvatar()
{
    if (format_version < MIN_VERSION_AVATAR)
        return false;

    ZipArchiveEntry::Ptr avatar;

    avatar = zip->GetEntry("avatar.dds");
    return (avatar && !avatar->IsDirectory());
}

bool SszFile::HasBattleName()
{
    if (format_version < MIN_VERSION_BATTLE_NAME)
        return false;

    ZipArchiveEntry::Ptr bn;

    bn = zip->GetEntry("battle_name/left_EN.dds");
    return (bn && !bn->IsDirectory());
}

bool SszFile::HasText()
{
    if (format_version < MIN_VERSION_TEXT)
        return false;

    return (!sst.IsEmpty());
}

bool SszFile::HasIcon()
{
    if (format_version < MIN_VERSION_ICON)
        return false;

    ZipArchiveEntry::Ptr icon;

    icon = zip->GetEntry("icon_big.dds");
    return (icon && !icon->IsDirectory());
}

bool SszFile::HasSelectName()
{
    if (format_version < MIN_VERSION_SELECT_NAME)
        return false;

    ZipArchiveEntry::Ptr sn;

    sn = zip->GetEntry("cha_sel/select_name_EN.dds");
    return (sn && !sn->IsDirectory());
}

bool SszFile::HasSelect2Name()
{
    if (format_version < MIN_VERSION_SELECT2_NAME)
        return false;

    ZipArchiveEntry::Ptr sn2;

    sn2 = zip->GetEntry("cha_sel/select2_name_EN.dds");
    return (sn2 && !sn2->IsDirectory());
}

bool SszFile::HasGwrName()
{
    if (format_version < MIN_VERSION_GWR_NAME)
        return false;

    ZipArchiveEntry::Ptr name;

    name = zip->GetEntry("gw/result_name_EN.dds");
    return (name && !name->IsDirectory());
}

bool SszFile::HasBanner()
{
    if (format_version < MIN_VERSION_BANNER)
        return false;

    ZipArchiveEntry::Ptr banner;

    banner = zip->GetEntry("gw/banner.dds");
    return (banner && !banner->IsDirectory());
}

bool SszFile::HasBannerName()
{
    if (format_version < MIN_VERSION_BANNER_NAME)
        return false;

    ZipArchiveEntry::Ptr bn;

    bn = zip->GetEntry("gw/banner_name_EN.dds");
    return (bn && !bn->IsDirectory());
}

bool SszFile::CopyResourceDirectory(const std::string & install_directory)
{
	if (!HasResourceFolder())
	{
		//DPRINTF("No resource folder.\n");
		return true; // Yes, true
	}
	
	std::vector<std::string> files_list;	
	GetResourceFilesList(files_list);
	
    for (std::string &s : files_list)
	{
		if (!CopyFile(s, install_directory + s))
			return false;
	}
	
    return true;
}

bool SszFile::GetParamUnsigned2(const TiXmlElement *root, const char *name, uint32_t *value)
{
    if (!Utils::ReadParamUnsigned(root, name, value))
    {
        DPRINTF("Cannot read parameter \"%s\"\n"
                "A mod that specifies SSZ_FORMAT_VERSION >= 2.0 must include the new fields of <Slot>", name);

        return false;
    }

    return true;
}

bool SszFile::GetParamFloat2(const TiXmlElement *root, const char *name, float *value)
{
    if (!Utils::ReadParamFloat(root, name, value))
    {
        DPRINTF("Cannot read parameter \"%s\"\n"
                "A mod that specifies SSZ_FORMAT_VERSION >= 2.0 must include the new fields of <Slot>", name);

        return false;
    }

    return true;
}

#define SPEC_STR(n, f) { if (!Utils::GetParamString(root, n, spec.f)) return false; }
#define SPEC_U(n, f) { if (!Utils::GetParamUnsigned(root, n, &spec.f)) return false; }
#define SPEC_F(n, f) { if (!Utils::GetParamFloat(root, n, &spec.f)) return false; }

bool SszFile::ReadSpec(const TiXmlElement *root)
{
	SPEC_STR("BBA0", bba0);	
	SPEC_STR("BBA1", bba1);
	SPEC_STR("BBA2", bba2);
	SPEC_STR("BBA3", bba3);
	SPEC_STR("BBA4", bba4);
	SPEC_STR("BBA5", bba5);
	SPEC_STR("EMA", ema_file);
	SPEC_STR("MENU", menu_file);
	SPEC_STR("FCE", fce_file);
	SPEC_STR("MATBAS", matbas_file);
	SPEC_STR("FMA", fma_file);
	SPEC_STR("FDM", fdm_file);
	SPEC_STR("FCM", fcm_file);
	SPEC_STR("VFX", vfx_file);
	SPEC_STR("TDB", tdb_file);
	SPEC_STR("BPM", bpm_file);
	SPEC_STR("VCN", vc_name);
	SPEC_STR("SE", se_name);
	SPEC_STR("VCF", vc_file);

	SPEC_U("U_D0", unk_D0);
	SPEC_U("U_D4", unk_D4);
	SPEC_F("SCALE", scale);
	SPEC_U("COSMO", cosmo);
	SPEC_F("F_E0", unk_E0);
	SPEC_F("F_E4", unk_E4);
	SPEC_F("F_E8", unk_E8);
	SPEC_F("F_EC", unk_EC);
	SPEC_F("F_F0", unk_F0);
	SPEC_U("AURA", aura);
	
	spec.emo_file = "";
	spec.emb_file = "";
	spec.emm_file = "";
	spec._2ry_file = "";
	spec.idx = spec.model_id = 0xDEADDA1A;
	
	return true;
}

#define CDC_U(n, f) { if (!Utils::GetParamUnsigned(root, n, &cdc_entry.f)) \
						return false;  }

bool SszFile::ReadCdcEntry(const TiXmlElement *root)
{
	CDC_U("CHAR_ID", char_id);
	CDC_U("NAME_ID", name_id);
	CDC_U("SHORT_NAME_ID", short_name_id);
	CDC_U("SIGN_NAME_ID", sign_name_id);
	CDC_U("ROBES_NAME_ID", robes_name_id);
	CDC_U("ROBES_TYPE", robes_type);	
	
	cdc_entry.unk_14 = 0xDEADDA1A;
	cdc_entry.is_dlc = 0;
	
	return true;
}

#define CSP_U(n, f) { if (!Utils::GetParamUnsigned(root, n, &csp_entry.f)) \
							return false; \
						}
						
#define CSP_F(n, f) { if (!Utils::GetParamFloat(root, n, &csp_entry.f)) \
							return false; \
						}

bool SszFile::ReadCspEntry(const TiXmlElement *root)
{
    if (!Utils::GetParamFloatWithMultipleNames(root, &csp_entry.left_pos_x, "LEFT_POS_X", "F_08"))
        return false;

    if (!Utils::GetParamFloatWithMultipleNames(root, &csp_entry.left_pos_y, "LEFT_POS_Y", "F_0C"))
        return false;

    if (!Utils::GetParamFloatWithMultipleNames(root, &csp_entry.left_pos_z, "LEFT_POS_Z", "F_10"))
        return false;

    if (!Utils::ReadParamUnsigned(root, "U_14", &csp_entry.left_rot))
    {
        int32_t signed_rot;

        if (!Utils::GetParamSigned(root, "LEFT_ROT", &signed_rot))
            return false;

        csp_entry.left_rot = signed_rot;
    }

    if (!Utils::GetParamFloatWithMultipleNames(root, &csp_entry.right_pos_x, "RIGHT_POS_X", "F_18"))
        return false;

    if (!Utils::GetParamFloatWithMultipleNames(root, &csp_entry.right_pos_y, "RIGHT_POS_Y", "F_1C"))
        return false;

    if (!Utils::GetParamFloatWithMultipleNames(root, &csp_entry.right_pos_z, "RIGHT_POS_Z", "F_20"))
        return false;

    if (!Utils::ReadParamUnsigned(root, "U_24", &csp_entry.right_rot))
    {
        int32_t signed_rot;

        if (!Utils::GetParamSigned(root, "RIGHT_ROT", &signed_rot))
            return false;

        csp_entry.right_rot = signed_rot;
    }

    if (!Utils::GetParamUnsignedWithMultipleNames(root, &csp_entry.animation_id, "ANIMATION_ID", "U_28"))
        return false;
	
	if (!Utils::GetParamUnsignedWithMultipleNames(root, &csp_entry.select_phrase_id, "SELECT_PHRASE_ID", "U_2C"))
		return false;	
	
	CSP_U("U_30", unk_30);
	CSP_U("U_34", unk_34);
	CSP_U("U_38", unk_38);
	CSP_U("U_3C", unk_3C);
	
	csp_entry.cms_entry = csp_entry.cms_model_spec_idx = 0xDEADDA1A;
	
	return true;
}

#define RPD_U32(n, f) { if (!Utils::GetParamUnsigned(root, n, &rpd_entry.f)) \
							return false; }
						
#define RPD_F(n, f) { if (!Utils::GetParamFloat(root, n, &rpd_entry.f)) \
							return false; }
						
#define RPD_STR(n, f) { if (!Utils::GetParamString(root, n, rpd_entry.f)) \
							return false; }

bool SszFile::ReadRpdEntry(const TiXmlElement *root)
{
	RPD_STR("NAME", name);
	RPD_U32("U_04", unk_04);
	RPD_F("F_08", unk_08);
	RPD_U32("U_0C", unk_0C);
	RPD_F("F_10", unk_10);
	RPD_U32("U_14", unk_14);
	RPD_F("F_18", unk_18);
	RPD_F("F_1C", unk_1C);
	RPD_U32("U_20", unk_20);
	RPD_U32("U_24", unk_24);
	RPD_F("F_28", unk_28);
	RPD_U32("U_2C", unk_2C);
	RPD_F("F_30", unk_30);
	RPD_U32("U_34", unk_34);
	RPD_U32("U_38", unk_38);
	RPD_U32("U_3C", unk_3C);
	RPD_U32("U_40", unk_40);
	RPD_U32("U_44", unk_44);
	RPD_U32("U_48", unk_48);
	RPD_U32("U_4C", unk_4C);
	RPD_U32("U_50", unk_50);
	RPD_U32("U_54", unk_54);
	RPD_U32("U_58", unk_58);
	RPD_U32("U_5C", unk_5C);
	
	return true;
}

#define SLOT_U(n, f) { if (!Utils::GetParamUnsigned(root, n, &temp)) \
							return false; \
						slot_entry.f = val32(temp); \
						}
						
#define SLOT_F(n, f) { if (!Utils::GetParamFloat(root, n, &ftemp)) \
							return false; \
						slot_entry.f = val_float(ftemp); \
						}
						
#define CI_U(n, f) { if (!Utils::GetParamUnsigned(root, n, &temp)) \
							return false; \
						info_entry.f = val32(temp); \
						}
						
#define CI2_U(n, f) { if (!Utils::GetParamUnsigned(root, n, &temp)) \
							return false; \
						info2_entry.f = val32(temp); \
						}

#define CI3_U(n, f) { if (!GetParamUnsigned2(root, n, &temp)) \
                            return false; \
                        info3_entry.f = val32(temp); \
                        }

#define CI3_F(n, f) { if (!GetParamFloat2(root, n, &ftemp)) \
                            return false; \
                        info3_entry.f = val_float(ftemp); \
                        }

#define CI4_U(n, f) { if (!GetParamUnsigned2(root, n, &temp)) \
                            return false; \
                        info4_entry.f = val32(temp); \
                        }

#define CI4_F(n, f) { if (!GetParamFloat2(root, n, &ftemp)) \
                            return false; \
                        info4_entry.f = val_float(ftemp); \
                        }

bool SszFile::ReadSlotEntry(const TiXmlElement *root)
{
	uint32_t temp;
	float ftemp;
	
    //
    if (!Utils::GetParamFloatWithMultipleNames(root, &ftemp, "ICON_ID", "ICON_DATA"))
        return false;
    slot_entry.icon_id = val_float(ftemp);
    //

    SLOT_U("U_0C", unk_0C);
	
	// CharacterInfo
	CI_U("SIGN_NAME_ID", sign_name_id);
	
	//
	if (!Utils::GetParamUnsignedWithMultipleNames(root, &temp, "SELECT_NAME_ID", "CIU_0C"))
			return false;	
	info_entry.select_name_id = val32(temp);
	//
	
	CI_U("ROBES_NAME_ID", robes_name_id);
	
	//
	if (!Utils::GetParamUnsignedWithMultipleNames(root, &temp, "SELECT2_NAME_ID", "CIU_14"))
		return false;
	info_entry.select2_name_id = val32(temp);
	//

	// Character Info 2
	CI2_U("AVATAR_ID", avatar_id);
	
	//
	if (!Utils::GetParamUnsignedWithMultipleNames(root, &temp, "BATTLE_NAME_ID", "CIU2_0C"))
			return false;
	info2_entry.battle_name_id = val32(temp);
	//
	
	CI2_U("CIU2_10", unk_10);

    if (format_version >= MIN_VERSION_CI3_CI4)
    {
        // Character info 4
        CI4_F("GWT_BANNER_NAME_ID", gwt_banner_name_id);
        CI4_F("GWT_BANNER_ID", gwt_banner_id);
        CI4_U("CIU4_10", unk_10);
        CI4_U("CIU4_14", unk_14);

        // Character Info 3
        CI3_F("GWR_NAME_ID", gwr_name_id);
        CI3_F("GWR_ICON_ID", gwr_icon_id);
        CI3_U("CIU3_10", unk_10);
        CI3_U("CIU3_14", unk_14);
    }
	
	// Lists
	if (!Utils::GetParamString(root, "CATEGORIES", categories))
		return false;
	
	slot_entry.cms_entry = slot_entry.cms_model_spec_idx = 0xDEADDA1A;
	info_entry.cms_entry = info_entry.cms_model_spec_idx = 0xDEADDA1A;
	info2_entry.cms_entry = info2_entry.cms_model_spec_idx = 0xDEADDA1A;
    info3_entry.cms_entry = info3_entry.cms_model_spec_idx = 0xDEADDA1A;
	
    return true;
}

#define GPD_U32(n, f) { if (!Utils::GetParamUnsigned(root, n, &gpd_entry.f)) \
                            return false; }
#define GPD_FLOAT(n, f) { if (!Utils::GetParamFloat(root, n, &gpd_entry.f)) \
                            return false; }

bool SszFile::ReadGpdEntry(const TiXmlElement *root)
{
    //GPD_U32("U_00", unk_00); // Ignore this one for now
    GPD_FLOAT("F_08", unk_08);
    GPD_FLOAT("F_0C", unk_0C);
    GPD_FLOAT("F_10", unk_10);
    GPD_FLOAT("F_14", unk_14);
    GPD_U32("U_18", unk_18);
    GPD_U32("U_1C", unk_1C);
    GPD_U32("U_20", unk_20);
    GPD_U32("U_24", unk_24);
    GPD_U32("U_28", unk_28);
    GPD_U32("U_2C", unk_2C);
    GPD_U32("U_30", unk_30);
    GPD_U32("U_34", unk_34);
    GPD_U32("U_38", unk_38);
    GPD_U32("U_3C", unk_3C);

    std::vector<std::string> tours_names;

    if (!Utils::GetParamMultipleStrings(root, "GWD_TOURNAMENTS", tours_names))
        return false;

    if (!GpdFile::GetTournamentsIds(tours_names, gwd_tournaments))
    {
        DPRINTF("GetTournamentsIds failed. Probably a bad written tournament name.\n");
        return false;
    }

    gpd_entry.id = 0xDEADDA1A;
    return true;
}

bool SszFile::ReadBgrEntry(const TiXmlElement *root)
{
    BgrEntry entry;

    if (!Utils::GetParamUnsigned(root, "LEVEL", &entry.level))
        return false;

    if (!Utils::GetParamUnsigned(root, "HOUR", &entry.hour))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_08", &entry.unk_08))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_14", &entry.unk_14))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_18", &entry.unk_18))
        return false;

    if (!Utils::GetParamUnsigned(root, "ASSIST_PHRASE1", &entry.assist_phrase1))
        return false;

    if (!Utils::GetParamUnsigned(root, "ASSIST_PHRASE2", &entry.assist_phrase2))
        return false;

    if (!Utils::GetParamUnsigned(root, "ASSIST_PHRASE3", &entry.assist_phrase3))
        return false;

    if (!Utils::GetParamUnsigned(root, "ASSIST_PHRASE4", &entry.assist_phrase4))
        return false;

    if (!Utils::GetParamUnsigned(root, "ASSIST_PHRASE5", &entry.assist_phrase5))
        return false;

    if (!Utils::GetParamUnsigned(root, "ASSIST_PHRASE6", &entry.assist_phrase6))
        return false;

    if (!Utils::GetParamUnsigned(root, "ASSIST_PHRASE7", &entry.assist_phrase7))
        return false;

    if (!Utils::GetParamUnsigned(root, "ASSIST_PHRASE8", &entry.assist_phrase8))
        return false;

    if (!Utils::GetParamUnsigned(root, "STAGE", &entry.stage))
        return false;

    if (!Utils::GetParamUnsigned(root, "HP", &entry.hp))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_44", &entry.unk_44))
        return false;

    entry.cms_entry = 0xDEADDA1A;
    entry.cms_model_spec_idx = 0xDEADDA1A;

    bgr_entries.push_back(entry);
    return true;
}

bool SszFile::ParseXml(TiXmlDocument *doc)
{
	TiXmlHandle handle(doc);
    std::string temp_str;

    gpd_entry.id = 0xFFFFFFFF;

    TiXmlElement *root = Utils::FindRoot(&handle, "SSZ");
	if (!root)
	{
        DPRINTF("%s: cannot find \"SSZ\" in xml.\n", FUNCNAME);
		return false;
	}
	
	if (Utils::ReadAttrString(root, "type", temp_str))
	{
		if (temp_str == "NEW_CHARACTER")
		{
			type = SszType::NEW_CHARACTER;
		}
		else if (temp_str == "REPLACER")
		{
			type = SszType::REPLACER;
		}
	}
	else
	{
		type = SszType::NEW_CHARACTER;
	}
	
	if (!Utils::GetParamFloat(root, "SSZ_FORMAT_VERSION", &format_version))
		return false;
	
    if (format_version > MAX_SSZ_FORMAT_VERSION)
	{
        DPRINTF("This mod requires a newer version of SS Mod Installer: %f\n", format_version);
		return false;
	}
	
	if (!Utils::GetParamString(root, "MOD_NAME", mod_name))
		return false;
	
	if (mod_name.length() == 0)
	{
		DPRINTF("Mod name can't be empty!\n");
		return false;
	}
	
	Utils::ReadParamString(root, "MOD_AUTHOR", mod_author);
	
	if (!Utils::ReadParamFloat(root, "MOD_VERSION", &mod_version))
		mod_version = 1.0f;
	
	if (!Utils::GetParamGUID(root, "MOD_GUID", guid))
		return false;
	
	if (type == SszType::REPLACER)
	{		
		loaded = HasResourceFolder();
		if (!loaded)
		{
			DPRINTF("resource folder is mandatory for replacer mods!\n");
		}
		
		return loaded;
	}
	
	if (!Utils::GetParamString(root, "CMS_ENTRY", temp_str))
		return false;
	
	if (temp_str.length() != 3)
	{
		DPRINTF("CMS_ENTRY must be 3 characters long.\n");
		return false;
	}
	
	strcpy(entry_name, temp_str.c_str());
	
	emo = temp_str + ".emo";
	emb = temp_str + ".emb";
	emm = temp_str + ".emm";
	_2ry = temp_str + ".2ry";
	
	if (!FileExists(emo))
	{
		DPRINTF("File \"%s\" doesn't exist in the archive!\n", emo.c_str());
		return false;
	}
	
	if (!FileExists(emb))
	{
		DPRINTF("File \"%s\" doesn't exist in the archive!\n", emb.c_str());
		return false;
	}
	
	if (!FileExists(emm))
	{
		DPRINTF("File \"%s\" doesn't exist in the archive!\n", emm.c_str());
		return false;
	}
	
	if (!FileExists(_2ry))
	{
		DPRINTF("File \"%s\" doesn't exist in the archive!\n", _2ry.c_str());
		return false;
	}
	
    bool spec_read, cdc_read, csp_read, rpd_read, slot_read, text_read, gpd_read;
	
    spec_read = cdc_read = csp_read = rpd_read = slot_read = text_read = gpd_read = false;
	
    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
	{
		std::string name = elem->ValueStr();
		
		if (!spec_read && name == "ModelSpec")
		{
			if (!ReadSpec(elem))
				return false;
			
			spec_read = true;
		}
		else if (!cdc_read && name == "ModelEntry")
		{
			if (!ReadCdcEntry(elem))
				return false;
			
			cdc_read = true;
		}
		else if (!csp_read && name == "ModelPosEntry")
		{
			if (!ReadCspEntry(elem))
				return false;
			
			csp_read = true;
		}
		else if (!rpd_read && name == "ModelRpdEntry")
		{
			if (!ReadRpdEntry(elem))
				return false;
			
			rpd_read = true;
		}
		else if (!slot_read && name == "Slot")
		{
			if (!ReadSlotEntry(elem))
				return false;
			
			slot_read = true;
		}
        else if (format_version >= MIN_VERSION_TEXT && !text_read && name == "Text")
        {
            if (!sst.Compile(elem))
            {
                DPRINTF("%s: compilation of Text section failed.\n", FUNCNAME);
                return false;
            }

            text_read = true;
        }
        else if (format_version >= MIN_VERSION_GPD && !gpd_read && name == "GpdEntry")
        {
            if (!ReadGpdEntry(elem))
                return false;

            gpd_read = true;
        }
        else if (format_version >= MIN_VERSION_BGR && name == "BgrEntry")
        {
            // Note: there can be multiple bgr
            if (!ReadBgrEntry(elem))
                return false;
        }
	}	

	if (!spec_read)
	{
		DPRINTF("ModelSpec was not found!\n");
		return false;
	}
	
	if (!cdc_read)
	{
		DPRINTF("ModelEntry was not found!\n");
		return false;
	}
	
	if (!csp_read)
	{
		DPRINTF("ModelPosEntry was not found!\n");
		return false;
	}
	
	if (!rpd_read)
	{
		DPRINTF("ModelRpdEntry was not found!\n");
		return false;
	}
	
	if (!slot_read)
	{
		DPRINTF("Slot was not found!\n");
		return false;
	}

    if (!text_read)
    {
        // Fine, it is not mandatory
    }

    if (!gpd_read)
    {
        // Fine, it is not mandatory
    }
	
    return true;
}

bool SszFile::BuildBattleName()
{
    battle_name.clear();

    if (!HasBattleName())
        return true;

    for (const std::string &s : languages)
    {
        std::string left_file = "battle_name/left_" + s + ".dds";
        std::string right_file = "battle_name/right_" + s + ".dds";

        if (FileExists(left_file))
        {
            if (!FileExists(right_file))
            {
                DPRINTF("%s exists but %s doesn't exist. This is not allowed.\n", left_file.c_str(), right_file.c_str());
                return false;
            }

            /*if (GetFileSize(left_file) != BATTLE_NAME_FILE_SIZE)
            {
                DPRINTF("%s must have a size of %d bytes.\n", left_file.c_str(), BATTLE_NAME_FILE_SIZE);
                return false;
            }

            if (GetFileSize(right_file) != BATTLE_NAME_FILE_SIZE)
            {
                DPRINTF("%s must have a size of %d bytes.\n", right_file.c_str(), BATTLE_NAME_FILE_SIZE);
                return false;
            }*/

            battle_name.push_back(left_file);
            battle_name.push_back(right_file);
        }
    }

    assert(battle_name.size() != 0);
    return true;
}

std::string SszFile::GetBattleNameTarget(const std::string & src, uint32_t cms_entry, uint32_t cms_model_spec_idx)
{
    std::string path = "resource/sssspatcher/battle_names_";

    if (src.find("battle_name/left_") == 0)
    {
        std::string language = src.substr(strlen("battle_name/left_"));
        assert(language.length() == 6);

        language = language.substr(0, 2);
        path += "left/" + language + "/";
    }
    else
    {
        assert(src.find("battle_name/right_") == 0);

        std::string language = src.substr(strlen("battle_name/right_"));
        assert(language.length() == 6);

        language = language.substr(0, 2);
        path += "right/" + language + "/";
    }

    path += Utils::ModelFileName(cms_entry, cms_model_spec_idx, ".dds");

    //DPRINTF("path = %s\n", path.c_str());

    return path;
}

bool SszFile::CheckIcon()
{
    if (!HasIcon())
        return true;

    if (GetFileSize("icon_big.dds") != ICON_BIG_FILE_SIZE)
    {
        DPRINTF("icon_big.dds must have a file size of %d bytes.\n", ICON_BIG_FILE_SIZE);
        return false;
    }

    if (!FileExists("icon_small.dds"))
    {
        DPRINTF("icon_big.dds exists but icon_small.dds doesn't. This is not allowed.\n");
        return false;
    }

    if (GetFileSize("icon_small.dds") != ICON_SMALL_FILE_SIZE)
    {
        DPRINTF("icon_small.dds must have a file size of %d bytes.\n", ICON_SMALL_FILE_SIZE);
        return false;
    }

    return true;
}

bool SszFile::BuildSelectName()
{
    select_name.clear();

    if (!HasSelectName())
        return true;

    for (const std::string &s : languages)
    {
        std::string file = "cha_sel/select_name_" + s + ".dds";

        if (FileExists(file))
        {
            if (GetFileSize(file) != SELECT_NAME_FILE_SIZE)
            {
                DPRINTF("%s must have a size of %d bytes.\n", file.c_str(), SELECT_NAME_FILE_SIZE);
                return false;
            }

            select_name.push_back(file);
        }
    }

    assert(select_name.size() != 0);
    return true;
}

std::string SszFile::GetSelectNameTarget(const std::string &src, uint32_t cms_entry, uint32_t cms_model_spec_idx)
{
    std::string path = SN_DIRECTORY;

    std::string language = src.substr(strlen("cha_sel/select_name_"));
    assert(language.length() == 6);

    language = language.substr(0, 2);
    path += language + "/";
    path += Utils::ModelFileName(cms_entry, cms_model_spec_idx, ".dds");

    return path;
}

bool SszFile::BuildSelect2Name()
{
   select2_name.clear();

   if (!HasSelect2Name())
       return true;

   for (const std::string &s : languages)
   {
       std::string file = "cha_sel/select2_name_" + s + ".dds";

       if (FileExists(file))
       {
           if (GetFileSize(file) != SELECT2_NAME_FILE_SIZE)
           {
               DPRINTF("%s must have a size of %d bytes.\n", file.c_str(), SELECT2_NAME_FILE_SIZE);
               return false;
           }

           select2_name.push_back(file);
       }
   }

   assert(select2_name.size() != 0);
   return true;
}

std::string SszFile::GetSelect2NameTarget(const std::string &src, uint32_t cms_entry, uint32_t cms_model_spec_idx)
{
    std::string path = SN2_DIRECTORY;

    std::string language = src.substr(strlen("cha_sel/select2_name_"));
    assert(language.length() == 6);

    language = language.substr(0, 2);
    path += language + "/";
    path += Utils::ModelFileName(cms_entry, cms_model_spec_idx, ".dds");

    return path;
}

bool SszFile::BuildGwrName()
{
    gwr_name.clear();

    if (!HasGwrName())
        return true;

    for (const std::string &s : languages)
    {
        std::string file = "gw/result_name_" + s + ".dds";

        if (FileExists(file))
        {
            if (GetFileSize(file) != GWR_NAME_FILE_SIZE)
            {
                DPRINTF("%s must have a size of %d bytes.\n", file.c_str(), GWR_NAME_FILE_SIZE);
                return false;
            }

            gwr_name.push_back(file);
        }
    }

    assert(gwr_name.size() != 0);
    return true;
}

std::string SszFile::GetGwrNameTarget(const std::string &src, uint32_t cms_entry, uint32_t cms_model_spec_idx)
{
    std::string path = GWR_NAMES_DIRECTORY;

    std::string language = src.substr(strlen("gw/result_name_"));
    assert(language.length() == 6);

    language = language.substr(0, 2);
    path += language + "/";
    path += Utils::ModelFileName(cms_entry, cms_model_spec_idx, ".dds");

    return path;
}

bool SszFile::CheckBanner()
{
    if (!HasBanner())
        return true;

    if (GetFileSize("gw/banner.dds") != BANNER_FILE_SIZE)
    {
        DPRINTF("banner.dds must have a file size of %d bytes.\n", BANNER_FILE_SIZE);
        return false;
    }

    return true;
}

bool SszFile::BuildBannerName()
{
    banner_name.clear();

    if (!HasBannerName())
        return true;

    for (const std::string &s : languages)
    {
        std::string file = "gw/banner_name_" + s + ".dds";

        if (FileExists(file))
        {
            if (GetFileSize(file) != BANNER_NAME_FILE_SIZE)
            {
                DPRINTF("%s must have a size of %d bytes.\n", file.c_str(), BANNER_NAME_FILE_SIZE);
                return false;
            }

            banner_name.push_back(file);
        }
    }

    assert(banner_name.size() != 0);
    return true;
}

std::string SszFile::GetBannerNameTarget(const std::string &src, uint32_t cms_entry, uint32_t cms_model_spec_idx)
{
    std::string path = GWT_BANNER_NAMES_DIRECTORY;

    std::string language = src.substr(strlen("gw/banner_name_"));
    assert(language.length() == 6);

    language = language.substr(0, 2);
    path += language + "/";
    path += Utils::ModelFileName(cms_entry, cms_model_spec_idx, ".dds");

    return path;
}

bool SszFile::LoadFromFile(const std::string &path, bool show_error)
{
	TiXmlDocument doc;
	char *xml_buf;
		
	Reset();

    if (!Utils::FileExists(path))
    {
        if (show_error)
        {
            DPRINTF("File %s doesn't exist or cannot be accesed.\n", path.c_str());
        }

        return false;
    }
	
	zip = ZipFile::Open(path);
	if (!zip)
		return false;
	
	xml_buf = ReadTextFile("sszmod.xml");
	if (!xml_buf)
		return false;
	
    doc.Parse(xml_buf);
    if (doc.ErrorId() != 0)
	{
        DPRINTF("%s: cannot parse xml (wrong formatted xml?)\n\n"
                "This is what tinyxml says: %s (%d). Row=%d, col=%d\n", FUNCNAME, doc.ErrorDesc(), doc.ErrorId(), doc.ErrorRow(), doc.ErrorCol());

        return false;
	}
	
	if (!ParseXml(&doc))
		return false;

    if (!BuildBattleName())
        return false;

    if (!CheckIcon())
        return false;

    if (!BuildSelectName())
        return false;

    if (!BuildSelect2Name())
        return false;

    if (!BuildGwrName())
        return false;

    if (!CheckBanner())
        return false;

    if (!BuildBannerName())
        return false;

    loaded = true;
	return true;
}

bool SszFile::InstallExtraFiles(InstallData *install_data, const std::string &install_directory)
{
    if (HasAvatar())
    {
        std::string filename;
        std::string avatar;

        filename = Utils::ModelFileName(install_data->cms_entry, install_data->cms_model_spec_idx, ".dds");
        avatar = install_directory + AVATARS_DIRECTORY + filename;

        if (!CopyFile("avatar.dds", avatar))
            return false;

        install_data->installed_files.push_back(AVATARS_DIRECTORY + filename);
        Utils::RemoveFile(install_directory + BATTLE_STEAM_CACHE);
    }

    if (HasBattleName())
    {
        for (const std::string &file : battle_name)
        {
            if (!CopyFile(file, install_directory + GetBattleNameTarget(file, install_data->cms_entry, install_data->cms_model_spec_idx)))
                return false;

            install_data->installed_files.push_back(GetBattleNameTarget(file, install_data->cms_entry, install_data->cms_model_spec_idx));
        }

        Utils::RemoveFile(install_directory + BATTLE_STEAM_CACHE);
    }

    if (HasText())
    {
        std::string filename = Utils::ModelFileName(install_data->cms_entry, install_data->cms_model_spec_idx, ".xml");

        if (!sst.DecompileToFile(install_directory + CHR_NAME_DIRECTORY + filename, true, true))
            return false;

        install_data->installed_files.push_back(CHR_NAME_DIRECTORY + filename);
        Utils::RemoveFile(install_directory + CHR_NAME_CACHE);
    }

    if (HasIcon())
    {
        std::string filename = Utils::ModelFileName(install_data->cms_entry, install_data->cms_model_spec_idx, ".dds");

        if (!CopyFile("icon_big.dds", install_directory + ICONS_DIRECTORY_BIG + filename))
            return false;

        if (!CopyFile("icon_small.dds", install_directory + ICONS_DIRECTORY_SMALL + filename))
            return false;

        install_data->installed_files.push_back(ICONS_DIRECTORY_BIG + filename);
        install_data->installed_files.push_back(ICONS_DIRECTORY_SMALL + filename);
        Utils::RemoveFile(install_directory + CHA_SEL_CACHE);
        Utils::RemoveFile(install_directory + GWR_CACHE);
    }

    if (HasSelectName())
    {
        for (const std::string &file : select_name)
        {
            if (!CopyFile(file, install_directory + GetSelectNameTarget(file, install_data->cms_entry, install_data->cms_model_spec_idx)))
                return false;

            install_data->installed_files.push_back(GetSelectNameTarget(file, install_data->cms_entry, install_data->cms_model_spec_idx));
        }

        Utils::RemoveFile(install_directory + CHA_SEL_CACHE);
        Utils::RemoveFile(install_directory + VS_CACHE);
    }

    if (HasSelect2Name())
    {
        for (const std::string &file : select2_name)
        {
            if (!CopyFile(file, install_directory + GetSelect2NameTarget(file, install_data->cms_entry, install_data->cms_model_spec_idx)))
                return false;

            install_data->installed_files.push_back(GetSelect2NameTarget(file, install_data->cms_entry, install_data->cms_model_spec_idx));
        }

        Utils::RemoveFile(install_directory + CHA_SEL_CACHE);
    }

    if (HasGwrName())
    {
        for (const std::string &file : gwr_name)
        {
            if (!CopyFile(file, install_directory + GetGwrNameTarget(file, install_data->cms_entry, install_data->cms_model_spec_idx)))
                return false;

            install_data->installed_files.push_back(GetGwrNameTarget(file, install_data->cms_entry, install_data->cms_model_spec_idx));
        }

        Utils::RemoveFile(install_directory + GWR_CACHE);
    }

    if (HasBanner())
    {
        std::string filename = Utils::ModelFileName(install_data->cms_entry, install_data->cms_model_spec_idx, ".dds");

        if (!CopyFile("gw/banner.dds", install_directory + GWT_BANNERS_DIRECTORY + filename))
            return false;

        install_data->installed_files.push_back(GWT_BANNERS_DIRECTORY + filename);
        Utils::RemoveFile(install_directory + GWT_CACHE);
    }

    if (HasBannerName())
    {
        for (const std::string &file : banner_name)
        {
            if (!CopyFile(file, install_directory + GetBannerNameTarget(file, install_data->cms_entry, install_data->cms_model_spec_idx)))
                return false;

            install_data->installed_files.push_back(GetBannerNameTarget(file, install_data->cms_entry, install_data->cms_model_spec_idx));
        }

        Utils::RemoveFile(install_directory + GWT_CACHE);
    }

    return true;
}

bool SszFile::Install(InstallData *install_data, const std::string & install_directory)
{
	CHECK_LOADED_BOOL();
	
	uint32_t cms_entry, cms_model_spec_idx;
	uint32_t model_id;
    char file_number[16];
	
	if (install_directory.length() > 0)
	{	
		char ch = install_directory[install_directory.length()-1];
		
		if (ch != '/' && ch != '\\')
		{
			DPRINTF("%s: BAD function usage. install_directory must end with / or \\.\n", __PRETTY_FUNCTION__);
			return false;
		}
	}
	
	if (type == SszType::REPLACER)
    {
        GetResourceFilesList(install_data->installed_files, false);
        return CopyResourceDirectory(install_directory);
    }
	
	// Gather generic data first
	cms_entry = install_data->cms->FindEntryByName(entry_name);
	if (cms_entry == (uint32_t)-1)
	{
		DPRINTF("Cannot find entry \"%s\" in the .cms.\n", entry_name);
		return false;
	}
	
	cms_model_spec_idx = install_data->cms->FindHighestIdx(cms_entry);
	if (cms_model_spec_idx == (uint32_t)-1)
	{
		DPRINTF("%s: WTF, the cms file has inconsistent/weird data.\n", __PRETTY_FUNCTION__);
		return false;
	}
	
	if (cms_model_spec_idx >= 33)
	{
		cms_model_spec_idx++;
	}
	else
	{
		cms_model_spec_idx = 33;
	}
	
	model_id = install_data->cdc->GetNumEntries() - 1;
	
	// CMS time
	sprintf(file_number, "%02d", cms_model_spec_idx);
	
	spec.emo_file = std::string(entry_name) + '_' + std::string(file_number) + ".emo";
	spec.emb_file = std::string(entry_name) + '_' + std::string(file_number) + ".emb";
	spec.emm_file = std::string(entry_name) + '_' + std::string(file_number) + ".emm";
	spec._2ry_file = std::string(entry_name) + '_' + std::string(file_number) + ".2ry";
	
	spec.idx = cms_model_spec_idx;
	spec.model_id = model_id;
	
	if (!install_data->cms->AppendModel(cms_entry, &spec))
	{
		DPRINTF("%s: cms->AppendModel failed.\n", __PRETTY_FUNCTION__);
		return false;
	}
	
	// CDC time
	cdc_entry.unk_14 = model_id;
	
	if (!install_data->cdc->AppendBeforeLast(&cdc_entry, cms_entry, cms_model_spec_idx))
	{
		DPRINTF("%s: cdc->AppendBeforeLast failed.\n", __PRETTY_FUNCTION__);
		return false;
	}
	
	// CSP time
	csp_entry.cms_entry = cms_entry;
	csp_entry.cms_model_spec_idx = cms_model_spec_idx;
	
	if (install_data->csp->AppendEntry(&csp_entry) == (uint32_t)-1)
	{
		DPRINTF("%s: csp->AppendEntry failed.\n", __PRETTY_FUNCTION__);
		return false;
	}
	
	// RPD time
	if (install_data->rpd->AppendEntry(&rpd_entry, cms_entry, cms_model_spec_idx) == (uint32_t)-1)
	{
		DPRINTF("%s: rpd->AppendEntry failed.\n", __PRETTY_FUNCTION__);
		return false;
	}
	
	// SLF time
    CharacterInfo3 *pinfo3 = nullptr;
    CharacterInfo4 *pinfo4 = nullptr;

	slot_entry.cms_entry = info_entry.cms_entry = info2_entry.cms_entry = cms_entry;
	slot_entry.cms_model_spec_idx = info_entry.cms_model_spec_idx = info2_entry.cms_model_spec_idx = cms_model_spec_idx;

    if (format_version >= MIN_VERSION_CI3_CI4)
    {
        info3_entry.cms_entry = cms_entry;
        info3_entry.cms_model_spec_idx = cms_model_spec_idx;
        pinfo3 = &info3_entry;

        info4_entry.cms_entry = cms_entry;
        info4_entry.cms_model_spec_idx = cms_model_spec_idx;
        pinfo4 = &info4_entry;
    }
	
    if (!install_data->slf->SetSlot(install_data->slot, install_data->sub_slot, &slot_entry, &info_entry, &info2_entry, categories, pinfo3, pinfo4))
	{
        DPRINTF("%s: slf->SetSlot failed.\n", FUNCNAME);
		return false;
	}

    // GPD time
    if (gpd_entry.id != 0xFFFFFFFF)
    {
        if (!install_data->gpd->AppendEntry(gpd_entry, cms_entry, cms_model_spec_idx, &gwd_tournaments))
        {
            DPRINTF("%s: gpd->AppendEntry failed.\n", FUNCNAME);
            return false;
        }
    }

    // BGR time
    for (BgrEntry &bgr_entry : bgr_entries)
    {
        bgr_entry.cms_entry = cms_entry;
        bgr_entry.cms_model_spec_idx = cms_model_spec_idx;
        install_data->bgr->AddEntry(bgr_entry);
    }
	
	// Install files now
	std::string character_directory = install_directory + "resource\\chara\\" + std::string(entry_name) + "\\";
	
	if (!CopyFile(emo, character_directory + spec.emo_file))
		return false;
	
	if (!CopyFile(emb, character_directory + spec.emb_file))
		return false;
	
	if (!CopyFile(emm, character_directory + spec.emm_file))
		return false;
	
	if (!CopyFile(_2ry, character_directory + spec._2ry_file))
		return false;
	
	if (!CopyResourceDirectory(install_directory))
		return false;

    install_data->cms_entry = cms_entry;
    install_data->cms_model_spec_idx = cms_model_spec_idx;
    install_data->model_id = model_id;

    if (!InstallExtraFiles(install_data, install_directory))
        return false;

    std::string char_path = "resource/chara/" + std::string(entry_name) + '/';

    install_data->installed_files.push_back(char_path + spec.emo_file);
    install_data->installed_files.push_back(char_path + spec.emb_file);
    install_data->installed_files.push_back(char_path + spec.emm_file);
    install_data->installed_files.push_back(char_path + spec._2ry_file);

    if (HasResourceFolder())
    {
        GetResourceFilesList(install_data->installed_files, false);
    }
	
	return true;
}

bool SszFile::Update(InstallData *install_data, const std::string & install_directory)
{
    CHECK_LOADED_BOOL();

    uint32_t my_entry, my_model_spec_idx;

    if (install_directory.length() > 0)
    {
        char ch = install_directory[install_directory.length()-1];

        if (ch != '/' && ch != '\\')
        {
            DPRINTF("%s: BAD function usage. install_directory must end with / or \\.\n", __PRETTY_FUNCTION__);
            return false;
        }
    }

    if (type == SszType::REPLACER)
    {
        GetResourceFilesList(install_data->installed_files, false);
        return CopyResourceDirectory(install_directory);
    }

    // CMS time
    ModelSpec *old_spec = install_data->cms->GetModelSpec(install_data->cms_entry, install_data->cms_model_spec_idx);
    if (!old_spec)
    {
        DPRINTF("Update/reinstall failed because that mod is not installed in the CMS.\n");
        return false;
    }

    if (install_data->cms->FindEntryByName(entry_name) != install_data->cms_entry)
    {
        DPRINTF("Update/reinstall failed: a mod update cannot specify a different entry_name!\n");
        delete old_spec;
        return false;
    }

    spec.idx = install_data->cms_model_spec_idx; // same as old_spec->idx
    spec.model_id = old_spec->model_id;
    spec.emo_file = old_spec->emo_file;
    spec.emb_file = old_spec->emb_file;
    spec.emm_file = old_spec->emm_file;
    spec._2ry_file = old_spec->_2ry_file;

    delete old_spec;

    if (!install_data->cms->SetModelSpec(install_data->cms_entry, &spec))
    {
        DPRINTF("%s: cms->SetModelSpec failed.\n", __PRETTY_FUNCTION__);
        return false;
    }

    // CDC time
    CdcEntry *old_cdc_entry = install_data->cdc->GetModelEntry(spec.model_id, &my_entry, &my_model_spec_idx);
    if (!old_cdc_entry)
    {
        DPRINTF("Update/reinstall failed because that mod is not installed in the CDC.\n");
        return false;
    }

    if (my_entry != install_data->cms_entry || my_model_spec_idx != install_data->cms_model_spec_idx)
    {
        DPRINTF("Update/reinstall failed because there are incosistencies betwee the CMS and CDC about this mod.\n");
        delete old_cdc_entry;
        return false;
    }

    cdc_entry.unk_14 = old_cdc_entry->unk_14;
    cdc_entry.is_dlc = old_cdc_entry->is_dlc;

    delete old_cdc_entry;

    if (!install_data->cdc->SetModelEntry(spec.model_id, &cdc_entry, install_data->cms_entry, install_data->cms_model_spec_idx))
    {
        DPRINTF("%s: cdc->SetModelEntry failed.\n", __PRETTY_FUNCTION__);
        return false;
    }

    // CSP time
    uint32_t csp_entry_id = install_data->csp->FindEntry(install_data->cms_entry, install_data->cms_model_spec_idx);
    if (csp_entry_id == (uint32_t)-1)
    {
        DPRINTF("Update/reinstall failed becasue the mod is not installed in the CSP.\n");
        return false;
    }

    csp_entry.cms_entry = install_data->cms_entry; // Guaranteed to be same as in old csp_entry
    csp_entry.cms_model_spec_idx = install_data->cms_model_spec_idx; // Guaranteed to be same as in old csp entry

    if (!install_data->csp->SetEntry(csp_entry_id, &csp_entry))
    {
        DPRINTF("%s: csp->SetEntry failed.\n", __PRETTY_FUNCTION__);
        return false;
    }

    // RPD time
    uint32_t rpd_entry_id = install_data->rpd->FindEntry(install_data->cms_entry, install_data->cms_model_spec_idx);
    if (rpd_entry_id == (uint32_t)-1)
    {
        DPRINTF("Update/reinstall failed because the mod is not installed in the RPD.\n");
        return false;
    }

    if (!install_data->rpd->SetEntry(rpd_entry_id, &rpd_entry, install_data->cms_entry, install_data->cms_model_spec_idx))
    {
        DPRINTF("%s: rpd->SetEntry failed.\n", __PRETTY_FUNCTION__);
        return false;
    }

    // SLF time
    if (!install_data->slf->FindSlot(install_data->cms_entry, install_data->cms_model_spec_idx, &install_data->slot, &install_data->sub_slot))
    {
        DPRINTF("Update/reinstall failed because the mod is not installed in the SLF.\n");
        return false;
    }

    CharacterInfo3 *pinfo3 = nullptr;
    CharacterInfo4 *pinfo4 = nullptr;

    slot_entry.cms_entry = info_entry.cms_entry = info2_entry.cms_entry = install_data->cms_entry;
    slot_entry.cms_model_spec_idx = info_entry.cms_model_spec_idx = info2_entry.cms_model_spec_idx = install_data->cms_model_spec_idx;

    if (format_version >= MIN_VERSION_CI3_CI4)
    {
        info3_entry.cms_entry = install_data->cms_entry;
        info3_entry.cms_model_spec_idx = install_data->cms_model_spec_idx;
        pinfo3 = &info3_entry;

        info4_entry.cms_entry = install_data->cms_entry;
        info4_entry.cms_model_spec_idx = install_data->cms_model_spec_idx;
        pinfo4 = &info4_entry;
    }

    if (!install_data->slf->SetSlot(install_data->slot, install_data->sub_slot, &slot_entry, &info_entry, &info2_entry, categories, pinfo3, pinfo4))
    {
        DPRINTF("%s: slf->SetSlot failed.\n", FUNCNAME);
        return false;
    }

    // GPD time
    if (gpd_entry.id != 0xFFFFFFFF)
    {
        uint32_t gpd_id = install_data->gpd->FindEntry(install_data->cms_entry, install_data->cms_model_spec_idx);

        if (gpd_id == (uint32_t)-1)
        {
            if (!install_data->gpd->AppendEntry(gpd_entry, install_data->cms_entry, install_data->cms_model_spec_idx, &gwd_tournaments))
            {
                DPRINTF("%s: gpd->AppendEntry failed.\n", FUNCNAME);
                return false;
            }
        }
        else
        {
            if (!install_data->gpd->SetEntry(gpd_id, gpd_entry, install_data->cms_entry, install_data->cms_model_spec_idx, &gwd_tournaments))
            {
                DPRINTF("%s: gpd->SetEntry failed.\n", FUNCNAME);
                return false;
            }
        }
    }

    // BGR time
    install_data->bgr->RemoveChar(install_data->cms_entry, install_data->cms_model_spec_idx);

    for (BgrEntry &bgr_entry : bgr_entries)
    {
        bgr_entry.cms_entry = install_data->cms_entry;
        bgr_entry.cms_model_spec_idx = install_data->cms_model_spec_idx;
        install_data->bgr->AddEntry(bgr_entry);
    }

    // Install files now
    std::string character_directory = install_directory + "resource\\chara\\" + std::string(entry_name) + "\\";

    if (!CopyFile(emo, character_directory + spec.emo_file))
        return false;

    if (!CopyFile(emb, character_directory + spec.emb_file))
        return false;

    if (!CopyFile(emm, character_directory + spec.emm_file))
        return false;

    if (!CopyFile(_2ry, character_directory + spec._2ry_file))
        return false;

    if (!CopyResourceDirectory(install_directory))
        return false;

    if (!InstallExtraFiles(install_data, install_directory))
        return false;

    std::string char_path = "resource/chara/" + std::string(entry_name) + '/';

    install_data->installed_files.push_back(char_path + spec.emo_file);
    install_data->installed_files.push_back(char_path + spec.emb_file);
    install_data->installed_files.push_back(char_path + spec.emm_file);
    install_data->installed_files.push_back(char_path + spec._2ry_file);

    if (HasResourceFolder())
    {
        GetResourceFilesList(install_data->installed_files, false);
    }

    return true;
}

