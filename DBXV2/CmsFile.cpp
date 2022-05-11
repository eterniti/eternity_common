#include <algorithm>

#include "CmsFile.h"
#include "Xenoverse2.h"
#include "debug.h"

#define XML_READ_STR(n, s) { if (!Utils::GetParamString(root, n, s)) return false; }
#define XML_READ_U(n, u) { if (!Utils::GetParamUnsigned(root, n, &u)) return false; }
#define XML_READ_F(n, f) { if (!Utils::GetParamFloat(root, n, &f)) return false; }
#define XML_READ_U16(n, u) { if (!Utils::GetParamUnsigned(root, n, &temp)) return false; \
                            if (temp > 65535) { DPRINTF("%s: \"%s\" cannot be greater than 65535.\n", FUNCNAME, n); return false; }\
                            u = (uint16_t)temp; }

#define XML_WRITE_STR(n, s) Utils::WriteParamString(entry_root, n, s)
#define XML_WRITE_U(n, u) Utils::WriteParamUnsigned(entry_root, n, u, true)
#define XML_WRITE_UD(n, u) Utils::WriteParamUnsigned(entry_root, n, u)
#define XML_WRITE_F(n, f) Utils::WriteParamFloat(entry_root, n, f)

TiXmlElement *CmsEntry::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Entry");
    entry_root->SetAttribute("id", Utils::UnsignedToString(id, true));
    entry_root->SetAttribute("name", name);

    std::string comment;
    if (Xenoverse2::GetCharaName(name, comment))
    {
        Utils::WriteComment(entry_root, comment);
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool CmsEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "id", &id))
    {
        DPRINTF("%s: failed to get id in Entry.\n", FUNCNAME);
        return false;
    }

    if (root->QueryStringAttribute("name", &name) != TIXML_SUCCESS)
    {
        DPRINTF("%s: failed to get name in Entry (id = 0x%x)\n", FUNCNAME, id);
        return false;
    }

    if (name.length() != 3)
    {
        DPRINTF("%s: name must be 3 character length. Offending string = \"%s\" in entry 0x%x\n", FUNCNAME, name.c_str(), id);
        return false;
    }

    return true;
}

void CmsModelSpecSSSS::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("ModelSpec");

#ifdef SSSS
    SsssCharInfo *ssss_info = SsssData::FindInfo(model_id);

    if (ssss_info)
    {
        Utils::WriteComment(entry_root, std::string(" ") + ssss_info->model_name + std::string(" / ") + ssss_info->char_name);
    }
#endif

    XML_WRITE_STR("BBA0", bba0);
    XML_WRITE_STR("BBA1", bba1);
    XML_WRITE_STR("BBA2", bba2);
    XML_WRITE_STR("BBA3", bba3);
    XML_WRITE_STR("BBA4", bba4);
    XML_WRITE_STR("BBA5", bba5);
    XML_WRITE_STR("EMO", emo_file);
    XML_WRITE_STR("EMB", emb_file);
    XML_WRITE_STR("EMM", emm_file);
    XML_WRITE_STR("EMA", ema_file);
    XML_WRITE_STR("MENU", menu_file);
    XML_WRITE_STR("FCE", fce_file);
    XML_WRITE_STR("MATBAS", matbas_file);
    XML_WRITE_STR("TWORY", _2ry_file);
    XML_WRITE_STR("FMA", fma_file);
    XML_WRITE_STR("FDM", fdm_file);
    XML_WRITE_STR("FCM", fcm_file);
    XML_WRITE_STR("VFX", vfx_file);
    XML_WRITE_STR("TDB", tdb_file);
    XML_WRITE_STR("BPM", bpm_file);
    XML_WRITE_STR("VCN", vc_name);
    XML_WRITE_STR("SE", se_name);
    XML_WRITE_STR("VCF", vc_file);

    XML_WRITE_UD("IDX", idx);
    XML_WRITE_U("MODEL_ID", model_id);
    XML_WRITE_U("U_D0", unk_D0);
    XML_WRITE_U("U_D4", unk_D4);
    XML_WRITE_F("SCALE", scale);
    XML_WRITE_U("COSMO", cosmo);
    XML_WRITE_F("F_E0", unk_E0);
    XML_WRITE_F("F_E4", unk_E4);
    XML_WRITE_F("F_E8", unk_E8);
    XML_WRITE_F("F_EC", unk_EC);
    XML_WRITE_F("F_F0", unk_F0);
    XML_WRITE_U("AURA", aura);

    root->LinkEndChild(entry_root);
}

bool CmsModelSpecSSSS::Compile(const TiXmlElement *root)
{
    XML_READ_STR("BBA0", bba0);
    XML_READ_STR("BBA1", bba1);
    XML_READ_STR("BBA2", bba2);
    XML_READ_STR("BBA3", bba3);
    XML_READ_STR("BBA4", bba4);
    XML_READ_STR("BBA5", bba5);
    XML_READ_STR("EMO", emo_file);
    XML_READ_STR("EMB", emb_file);
    XML_READ_STR("EMM", emm_file);
    XML_READ_STR("EMA", ema_file);
    XML_READ_STR("MENU", menu_file);
    XML_READ_STR("FCE", fce_file);
    XML_READ_STR("MATBAS", matbas_file);
    XML_READ_STR("TWORY", _2ry_file);
    XML_READ_STR("FMA", fma_file);
    XML_READ_STR("FDM", fdm_file);
    XML_READ_STR("FCM", fcm_file);
    XML_READ_STR("VFX", vfx_file);
    XML_READ_STR("TDB", tdb_file);
    XML_READ_STR("BPM", bpm_file);
    XML_READ_STR("VCN", vc_name);
    XML_READ_STR("SE", se_name);
    XML_READ_STR("VCF", vc_file);

    XML_READ_U("IDX", idx);
    XML_READ_U("MODEL_ID", model_id);
    XML_READ_U("U_D0", unk_D0);
    XML_READ_U("U_D4", unk_D4);
    XML_READ_F("SCALE", scale);
    XML_READ_U("COSMO", cosmo);
    XML_READ_F("F_E0", unk_E0);
    XML_READ_F("F_E4", unk_E4);
    XML_READ_F("F_E8", unk_E8);
    XML_READ_F("F_EC", unk_EC);
    XML_READ_F("F_F0", unk_F0);
    XML_READ_U("AURA", aura);

    return true;
}

TiXmlElement *CmsEntrySSSS::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = CmsEntry::Decompile(root);
    entry_root->SetAttribute("type", Utils::UnsignedToString(type, true));

    for (const CmsModelSpecSSSS &spec : specs)
    {
        spec.Decompile(entry_root);
    }

    return entry_root;
}

bool CmsEntrySSSS::Compile(const TiXmlElement *root)
{
    if (!CmsEntry::Compile(root))
        return false;

    if (!Utils::ReadAttrUnsigned(root, "type", &type))
    {
        DPRINTF("%s: failed to get type in Entry \"%s\" (0x%x)\n", FUNCNAME, name.c_str(), id);
        return false;
    }

    size_t num_specs = Utils::GetElemCount(root, "ModelSpec");

    if (num_specs == 0)
    {
        DPRINTF("%s: there must be at least one spec per entry (offending entry = \"%s\")\n", FUNCNAME, name.c_str());
        return false;
    }

    specs.reserve(num_specs);

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "ModelSpec")
        {
            CmsModelSpecSSSS spec;

            if (!spec.Compile(elem))
            {
                DPRINTF("%s: ReadSpec failed on entry \"%s\"\n", FUNCNAME, name.c_str());
                return false;
            }

            specs.push_back(spec);
        }
    }

    return true;
}

TiXmlElement *CmsEntryXV::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = CmsEntry::Decompile(root);

    XML_WRITE_U("U_10", unk_10);
    XML_WRITE_U("LOAD_CAM_DIST", load_cam_dist);
    XML_WRITE_U("U_16", unk_16);
    XML_WRITE_U("U_18", unk_18);
    XML_WRITE_U("U_1A", unk_1A);
    XML_WRITE_U("U_1C", unk_1C);

    XML_WRITE_STR("CHARACTER", character);
    XML_WRITE_STR("EAN", ean);
    XML_WRITE_STR("FCE_EAN", fce_ean);
    XML_WRITE_STR("FCE", fce);
    XML_WRITE_STR("CAM_EAN", cam_ean);
    XML_WRITE_STR("BAC", bac);
    XML_WRITE_STR("BCM", bcm);
    XML_WRITE_STR("AI", ai);

    return entry_root;
}

bool CmsEntryXV::Compile(const TiXmlElement *root)
{
    if (!CmsEntry::Compile(root))
        return false;

    uint32_t temp;

    XML_READ_U("U_10", unk_10);
    XML_READ_U16("LOAD_CAM_DIST", load_cam_dist);
    XML_READ_U16("U_16", unk_16);
    XML_READ_U16("U_18", unk_18);
    XML_READ_U16("U_1A", unk_1A);

    // value that started being not zero in some entries in 1.17.2.
    // To support older .xml, we set it to zero if it doesn't exist
    if (!Utils::ReadParamUnsigned(root, "U_1C", &unk_1C))
        unk_1C = 0;

    XML_READ_STR("CHARACTER", character);
    XML_READ_STR("EAN", ean);
    XML_READ_STR("FCE_EAN", fce_ean);
    XML_READ_STR("FCE", fce);
    XML_READ_STR("CAM_EAN", cam_ean);
    XML_READ_STR("BAC", bac);
    XML_READ_STR("BCM", bcm);
    XML_READ_STR("AI", ai);

    return true;
}

TiXmlElement *CmsEntryXV2::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = CmsEntryXV::Decompile(root);
    XML_WRITE_STR("STR_50", bdm);
    return entry_root;
}

bool CmsEntryXV2::Compile(const TiXmlElement *root)
{
    if (!CmsEntryXV::Compile(root))
        return false;

    XML_READ_STR("STR_50", bdm);
    return true;
}

CmsFile::CmsFile()
{
    this->big_endian = false;
}

CmsFile::~CmsFile()
{
    Reset();
}

void CmsFile::Reset()
{
    for (CmsEntry *&entry : entries)
    {
        delete entry;
        entry = nullptr;
    }

    entries.clear();
    type = CmsType::CMS_SSSS;
}

static inline bool valid_name(const std::string &name)
{
    if (name.length() != 3)
        return false;

    for (char c : name)
    {
        if (c > 'Z' || c < '0')
            return false;

        if (c > '9' && c < 'A')
            return false;
    }

    return true;
}

#define GET_STR(s, o) if (o == 0) s.clear(); else s = (char *)GetOffsetPtr(buf, o, true)

bool CmsFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (size < sizeof(CMSHeaderSSSS))
        return false;

    const CMSHeaderSSSS *hdr_ssss = (const CMSHeaderSSSS *)buf;
    const CMSHeaderXV *hdr_xv = (const CMSHeaderXV *)buf;

    if (hdr_ssss->signature != CMS_SIGNATURE)
        return false;

    if (hdr_ssss->endianess_check != 0xFFFE)
        return false;

    if (hdr_ssss->unk_0C == 0)
    {
        type = CmsType::CMS_SSSS;

        const CMSEntrySSSS *entries_ssss = (const CMSEntrySSSS *)GetOffsetPtr(buf, hdr_ssss->data_start);
        entries.resize(hdr_ssss->num_entries);

        for (size_t i = 0; i < entries.size(); i++)
        {
            CmsEntrySSSS *entry = new CmsEntrySSSS();

            entry->id = entries_ssss[i].id;
            entry->name = entries_ssss[i].name;
            entry->type = entries_ssss[i].type;

            entry->specs.resize(entries_ssss[i].num_models);

            CMSModelSpecSSSS *specs  = (CMSModelSpecSSSS *)GetOffsetPtr(buf, entries_ssss[i].models_spec_offset, true);

            for (size_t j = 0; j < entry->specs.size(); j++)
            {
                CmsModelSpecSSSS &spec = entry->specs[j];

                GET_STR(spec.bba0, (uint32_t)specs[j].bba0_name_offset);
                GET_STR(spec.bba1, (uint32_t)specs[j].bba1_name_offset);
                GET_STR(spec.bba2, (uint32_t)specs[j].bba2_name_offset);
                GET_STR(spec.bba3, (uint32_t)specs[j].bba3_name_offset);
                GET_STR(spec.bba4, (uint32_t)specs[j].bba4_name_offset);
                GET_STR(spec.bba5, (uint32_t)specs[j].bba5_name_offset);

                GET_STR(spec.emo_file, (uint32_t)specs[j].emo_file_offset);
                GET_STR(spec.emb_file, (uint32_t)specs[j].emb_file_offset);
                GET_STR(spec.emm_file, (uint32_t)specs[j].emm_file_offset);
                GET_STR(spec.ema_file, (uint32_t)specs[j].ema_file_offset);

                GET_STR(spec.menu_file, (uint32_t)specs[j].menu_file_offset);
                GET_STR(spec.fce_file, (uint32_t)specs[j].fce_file_offset);
                GET_STR(spec.matbas_file, (uint32_t)specs[j].matbas_file_offset);
                GET_STR(spec.vc_file, (uint32_t)specs[j].vc_file_offset);
                GET_STR(spec._2ry_file, (uint32_t)specs[j]._2ry_file_offset);

                GET_STR(spec.fma_file, (uint32_t)specs[j].fma_file_offset);
                GET_STR(spec.fdm_file, (uint32_t)specs[j].fdm_file_offset);
                GET_STR(spec.fcm_file, (uint32_t)specs[j].fcm_file_offset);

                GET_STR(spec.vfx_file, (uint32_t)specs[j].vfx_file_offset);
                GET_STR(spec.tdb_file, (uint32_t)specs[j].tdb_file_offset);
                GET_STR(spec.bpm_file, (uint32_t)specs[j].bpm_file_offset);
                GET_STR(spec.vc_name, (uint32_t)specs[j].vc_name_offset);
                GET_STR(spec.se_name, (uint32_t)specs[j].se_name_offset);

                spec.idx = specs[j].idx;
                spec.model_id = specs[j].model_id;
                spec.unk_D0 = specs[j].unk_D0;
                spec.unk_D4 = specs[j].unk_D4;
                spec.scale = specs[j].scale;
                spec.cosmo = specs[j].cosmo;
                spec.unk_E0 = specs[j].unk_E0;
                spec.unk_E4 = specs[j].unk_E4;
                spec.unk_E8 = specs[j].unk_E8;
                spec.unk_EC = specs[j].unk_EC;
                spec.unk_F0 = specs[j].unk_F0;
                spec.aura = specs[j].aura;
            }

            entries[i] = entry;
        }
    }
    else
    {
        type = CmsType::CMS_XV2;

        const CMSEntryXV *entries_xv = (const CMSEntryXV *)GetOffsetPtr(buf, hdr_xv->data_start);
        const CMSEntryXV2 *entries_xv2 = (const CMSEntryXV2 *)entries_xv;

        if (hdr_xv->num_entries > 1)
        {
            // A lame way of checkign if file is xenoverse or xenoverse 2
            // Sadly it won't work on 1 entry files, so those will assume xenoverse 2
            if (!valid_name(entries_xv2[1].name))
                type = CmsType::CMS_XV;
        }

        if (type == CmsType::CMS_XV)
        {
            DPRINTF("Xenoverse 1 format not supported properly yet.\n");
            return false;
        }

        entries.resize(hdr_xv->num_entries);

        if (type == CmsType::CMS_XV)
        {
            for (size_t i = 0; i < entries.size(); i++)
            {
                CmsEntryXV *entry = new CmsEntryXV();

                if (entries_xv[i].unk_08 != 0 || entries_xv[i].unk_1C != 0 || entries_xv[i].unk_28 != 0
                        || entries_xv[i].unk_34 != 0 || entries_xv[i].unk_48 != 0)
                {
                    DPRINTF("%s: A value that was supossed to always be zero isn't.\n", FUNCNAME);
                    return false;
                }

                entry->id = entries_xv[i].id;
                entry->name = entries_xv[i].name;

                entry->unk_10 = entries_xv[i].unk_10;
                entry->load_cam_dist = entries_xv[i].load_cam_dist;
                entry->unk_16 = entries_xv[i].unk_16;
                entry->unk_18 = entries_xv[i].unk_18;
                entry->unk_1A = entries_xv[i].unk_1A;
                entry->unk_1C = entries_xv[i].unk_1C;

                GET_STR(entry->character, entries_xv[i].character_offset);
                GET_STR(entry->ean, entries_xv[i].ean_offset);
                GET_STR(entry->fce_ean, entries_xv[i].fce_ean_offset);
                GET_STR(entry->fce, entries_xv[i].fce_offset);
                GET_STR(entry->cam_ean, entries_xv[i].cam_ean_offset);
                GET_STR(entry->bac, entries_xv[i].bac_offset);
                GET_STR(entry->bcm, entries_xv[i].bcm_offset);
                GET_STR(entry->ai, entries_xv[i].ai_offset);

                entries[i] = entry;
            }
        }
        else
        {
            for (size_t i = 0; i < entries.size(); i++)
            {
                CmsEntryXV2 *entry = new CmsEntryXV2();

                if (entries_xv2[i].unk_08 != 0 || entries_xv2[i].unk_28 != 0
                        || entries_xv2[i].unk_34 != 0 || entries_xv2[i].unk_48 != 0)
                {
                    DPRINTF("%s: A value that was supossed to always be zero isn't.\n", FUNCNAME);
                    DPRINTF("Entry %x %I64x %x %x %x %I64x\n", (uint32_t)i, entries_xv2[i].unk_08, entries_xv2[i].unk_1C,
                            entries_xv2[i].unk_28, entries_xv2[i].unk_34, entries_xv2[i].unk_48);

                    delete entry;
                    return false;
                }

                entry->id = entries_xv2[i].id;
                entry->name = entries_xv2[i].name;

                entry->unk_10 = entries_xv2[i].unk_10;
                entry->load_cam_dist = entries_xv2[i].load_cam_dist;
                entry->unk_16 = entries_xv2[i].unk_16;
                entry->unk_18 = entries_xv2[i].unk_18;
                entry->unk_1A = entries_xv2[i].unk_1A;
                entry->unk_1C = entries_xv2[i].unk_1C;

                GET_STR(entry->character, entries_xv2[i].character_offset);
                GET_STR(entry->ean, entries_xv2[i].ean_offset);
                GET_STR(entry->fce_ean, entries_xv2[i].fce_ean_offset);
                GET_STR(entry->fce, entries_xv2[i].fce_offset);
                GET_STR(entry->cam_ean, entries_xv2[i].cam_ean_offset);
                GET_STR(entry->bac, entries_xv2[i].bac_offset);
                GET_STR(entry->bcm, entries_xv2[i].bcm_offset);
                GET_STR(entry->ai, entries_xv2[i].ai_offset);
                GET_STR(entry->bdm, entries_xv2[i].bdm_offset);

                entries[i] = entry;
            }
        }
    }

    return true;
}

#define ADD_STR(s) { if (s.length() > 0 && std::find(list.begin(), list.end(), s) == list.end()) list.push_back(s); }

void CmsFile::GenerateStrList(std::vector<std::string> &list) const
{
    for (CmsEntry *const &entry : entries)
    {
        CmsEntrySSSS *const entry_ssss = dynamic_cast<CmsEntrySSSS *const>(entry);
        CmsEntryXV *const entry_xv = dynamic_cast<CmsEntryXV *const>(entry);
        CmsEntryXV2 *const entry_xv2 = dynamic_cast<CmsEntryXV2 *const>(entry);

        if (entry_ssss)
        {
            for (const CmsModelSpecSSSS &spec : entry_ssss->specs)
            {
                ADD_STR(spec.bba0);
                ADD_STR(spec.bba1);
                ADD_STR(spec.bba2);
                ADD_STR(spec.bba3);
                ADD_STR(spec.bba4);
                ADD_STR(spec.bba5);

                ADD_STR(spec.emo_file);
                ADD_STR(spec.emb_file);
                ADD_STR(spec.emm_file);
                ADD_STR(spec.ema_file);

                ADD_STR(spec.menu_file);
                ADD_STR(spec.fce_file);
                ADD_STR(spec.matbas_file);
                ADD_STR(spec._2ry_file);

                ADD_STR(spec.fma_file);
                ADD_STR(spec.fdm_file);
                ADD_STR(spec.fcm_file);

                ADD_STR(spec.vfx_file);
                ADD_STR(spec.tdb_file);
                ADD_STR(spec.bpm_file);
                ADD_STR(spec.vc_name);
                ADD_STR(spec.se_name);
                ADD_STR(spec.vc_file);
            }
        }
        else if (entry_xv2)
        {
            ADD_STR(entry_xv2->character);
            ADD_STR(entry_xv2->ean);
            ADD_STR(entry_xv2->fce_ean);
            ADD_STR(entry_xv2->fce);
            ADD_STR(entry_xv2->cam_ean);
            ADD_STR(entry_xv2->bac);
            ADD_STR(entry_xv2->bcm);
            ADD_STR(entry_xv2->ai);
            ADD_STR(entry_xv2->bdm);
        }
        else
        {
            ADD_STR(entry_xv->character);
            ADD_STR(entry_xv->ean);
            ADD_STR(entry_xv->fce_ean);
            ADD_STR(entry_xv->fce);
            ADD_STR(entry_xv->cam_ean);
            ADD_STR(entry_xv->bac);
            ADD_STR(entry_xv->bcm);
            ADD_STR(entry_xv->ai);
        }
    }
}

size_t CmsFile::CalculateFileSize(const std::vector<std::string> &list, uint32_t *strings_offset) const
{
    size_t size;

    if (type == CmsType::CMS_SSSS)
    {
        size = 0x20 + entries.size() * sizeof(CMSEntrySSSS);

        for (CmsEntry *const &entry : entries)
        {
            CmsEntrySSSS *const entry_ssss = dynamic_cast<CmsEntrySSSS *const>(entry);

            size += entry_ssss->specs.size() * sizeof(CMSModelSpecSSSS);
        }
    }
    else if (type == CmsType::CMS_XV)
    {
        size = sizeof(CMSHeaderXV) + entries.size() * sizeof(CMSEntryXV);
    }
    else
    {
        size = sizeof(CMSHeaderXV) + entries.size() * sizeof(CMSEntryXV2);
    }

    *strings_offset = (uint32_t)size;

    for (const std::string &str : list)
    {
        size += str.length() + 1;
    }

    return size;
}

uint32_t CmsFile::GetStringOffset(const std::vector<std::string> &list, const std::string &str) const
{
    uint32_t offset = 0;

    if (str.length() == 0)
        return 0xFFFFFFFF;

    for (const std::string &str2 : list)
    {
        if (str == str2)
            return offset;

        offset += (uint32_t) str2.length()+1;
    }

    //DPRINTF("buuu\n");
    return 0xFFFFFFFF;
}

#define GET_STRO(o, s) { o = GetStringOffset(strings_list, s); if (o == 0xFFFFFFFF) o = 0; else o += strings_offset; }

uint8_t *CmsFile::Save(size_t *psize)
{
    std::vector<std::string> strings_list;
    size_t size;
    uint32_t strings_offset;

    GenerateStrList(strings_list);
    size = CalculateFileSize(strings_list, &strings_offset);

    uint8_t *buf = new uint8_t[size];
    memset(buf, 0, size);

    if (type == CmsType::CMS_SSSS)
    {
        CMSHeaderSSSS *hdr = (CMSHeaderSSSS *)buf;

        hdr->signature = CMS_SIGNATURE;
        hdr->endianess_check = 0xFFFE;
        hdr->num_entries = (uint32_t)entries.size();
        hdr->data_start = 0x20;

        CMSEntrySSSS *entries_ssss = (CMSEntrySSSS *)(buf+0x20);
        CMSModelSpecSSSS *specs_ssss = (CMSModelSpecSSSS *)(entries_ssss + entries.size());

        for (size_t i = 0; i < entries.size(); i++)
        {
            CmsEntrySSSS *entry = dynamic_cast<CmsEntrySSSS *>(entries[i]);

            entries_ssss[i].id = entry->id;
            strcpy(entries_ssss[i].name, entry->name.c_str());
            entries_ssss[i].type = entry->type;
            entries_ssss[i].num_models = (uint32_t)entry->specs.size();
            entries_ssss[i].models_spec_offset = Utils::DifPointer(specs_ssss, buf);

            for (const CmsModelSpecSSSS &spec : entry->specs)
            {
                GET_STRO(specs_ssss->bba0_name_offset, spec.bba0);
                GET_STRO(specs_ssss->bba1_name_offset, spec.bba1);
                GET_STRO(specs_ssss->bba2_name_offset, spec.bba2);
                GET_STRO(specs_ssss->bba3_name_offset, spec.bba3);
                GET_STRO(specs_ssss->bba4_name_offset, spec.bba4);
                GET_STRO(specs_ssss->bba5_name_offset, spec.bba5);

                GET_STRO(specs_ssss->emo_file_offset, spec.emo_file);
                GET_STRO(specs_ssss->emb_file_offset, spec.emb_file);
                GET_STRO(specs_ssss->emm_file_offset, spec.emm_file);
                GET_STRO(specs_ssss->ema_file_offset, spec.ema_file);

                GET_STRO(specs_ssss->menu_file_offset, spec.menu_file);
                GET_STRO(specs_ssss->fce_file_offset, spec.fce_file);
                GET_STRO(specs_ssss->matbas_file_offset, spec.matbas_file);
                GET_STRO(specs_ssss->vc_file_offset, spec.vc_file);
                GET_STRO(specs_ssss->_2ry_file_offset, spec._2ry_file);

                GET_STRO(specs_ssss->fma_file_offset, spec.fma_file);
                GET_STRO(specs_ssss->fdm_file_offset, spec.fdm_file);
                GET_STRO(specs_ssss->fcm_file_offset, spec.fcm_file);

                GET_STRO(specs_ssss->vfx_file_offset, spec.vfx_file);
                GET_STRO(specs_ssss->tdb_file_offset, spec.tdb_file);
                GET_STRO(specs_ssss->bpm_file_offset, spec.bpm_file);
                GET_STRO(specs_ssss->vc_name_offset, spec.vc_name);
                GET_STRO(specs_ssss->se_name_offset, spec.se_name);

                specs_ssss->idx = spec.idx;
                specs_ssss->model_id = spec.model_id;
                specs_ssss->unk_D0 = spec.unk_D0;
                specs_ssss->unk_D4 = spec.unk_D4;
                specs_ssss->scale = spec.scale;
                specs_ssss->cosmo = spec.cosmo;
                specs_ssss->unk_E0 = spec.unk_E0;
                specs_ssss->unk_E4 = spec.unk_E4;
                specs_ssss->unk_E8 = spec.unk_E8;
                specs_ssss->unk_EC = spec.unk_EC;
                specs_ssss->unk_F0 = spec.unk_F0;
                specs_ssss->aura = spec.aura;

                specs_ssss++;
            }
        }
    }
    else
    {
        CMSHeaderXV *hdr = (CMSHeaderXV *)buf;

        hdr->signature = CMS_SIGNATURE;
        hdr->endianess_check = 0xFFFE;
        hdr->num_entries = (uint32_t)entries.size();
        hdr->data_start = sizeof(CMSHeaderXV);

        if (type == CmsType::CMS_XV)
        {
            CMSEntryXV *entries_xv = (CMSEntryXV *)(hdr+1);

            for (size_t i = 0; i < entries.size(); i++)
            {
                CmsEntryXV *entry = dynamic_cast<CmsEntryXV *>(entries[i]);

                entries_xv[i].id = entry->id;
                strcpy(entries_xv[i].name, entry->name.c_str());

                entries_xv[i].unk_10 = entry->unk_10;
                entries_xv[i].load_cam_dist = entry->load_cam_dist;
                entries_xv[i].unk_16 = entry->unk_16;
                entries_xv[i].unk_18 = entry->unk_18;
                entries_xv[i].unk_1A = entry->unk_1A;
                entries_xv[i].unk_1C = entry->unk_1C;

                GET_STRO(entries_xv[i].character_offset, entry->character);
                GET_STRO(entries_xv[i].ean_offset, entry->ean);
                GET_STRO(entries_xv[i].fce_ean_offset, entry->fce_ean);
                GET_STRO(entries_xv[i].fce_offset, entry->fce);
                GET_STRO(entries_xv[i].cam_ean_offset, entry->cam_ean);
                GET_STRO(entries_xv[i].bac_offset, entry->bac);
                GET_STRO(entries_xv[i].bcm_offset, entry->bcm);
                GET_STRO(entries_xv[i].ai_offset, entry->ai);
            }
        }
        else
        {
            CMSEntryXV2 *entries_xv2 = (CMSEntryXV2 *)(hdr+1);

            for (size_t i = 0; i < entries.size(); i++)
            {
                CmsEntryXV2 *entry = dynamic_cast<CmsEntryXV2 *>(entries[i]);

                entries_xv2[i].id = entry->id;
                strcpy(entries_xv2[i].name, entry->name.c_str());

                entries_xv2[i].unk_10 = entry->unk_10;
                entries_xv2[i].load_cam_dist = entry->load_cam_dist;
                entries_xv2[i].unk_16 = entry->unk_16;
                entries_xv2[i].unk_18 = entry->unk_18;
                entries_xv2[i].unk_1A = entry->unk_1A;
                entries_xv2[i].unk_1C = entry->unk_1C;

                GET_STRO(entries_xv2[i].character_offset, entry->character);
                GET_STRO(entries_xv2[i].ean_offset, entry->ean);
                GET_STRO(entries_xv2[i].fce_ean_offset, entry->fce_ean);
                GET_STRO(entries_xv2[i].fce_offset, entry->fce);
                GET_STRO(entries_xv2[i].cam_ean_offset, entry->cam_ean);
                GET_STRO(entries_xv2[i].bac_offset, entry->bac);
                GET_STRO(entries_xv2[i].bcm_offset, entry->bcm);
                GET_STRO(entries_xv2[i].ai_offset, entry->ai);
                GET_STRO(entries_xv2[i].bdm_offset, entry->bdm);
            }
        }
    }

    for (const std::string &str : strings_list)
    {
        strcpy((char *)buf+strings_offset, str.c_str());
        strings_offset += (uint32_t)str.length() + 1;
    }

    assert(strings_offset == (uint32_t)size);

    *psize = size;
    return buf;
}

TiXmlDocument *CmsFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("CMS");

    if (type == CmsType::CMS_SSSS)
    {
        root->SetAttribute("type", "SSSS");
        Utils::WriteComment(root, " This file has machine generated comments. Any change to these comments will be lost on next decompilation. ");
    }
    else if (type == CmsType::CMS_XV)
    {
        root->SetAttribute("type", "DBXV");
    }
    else
    {
        root->SetAttribute("type", "DBXV2");
    }

    for (CmsEntry *entry : entries)
    {
        entry->Decompile(root);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool CmsFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "CMS");

    if (!root)
    {
        DPRINTF("Cannot find\"CMS\" in xml.\n");
        return false;
    }

    std::string type_str;

    if (Utils::ReadAttrString(root, "type",  type_str))
    {
        type_str = Utils::ToLowerCase(type_str);

        if (type_str == "ssss")
            type = CmsType::CMS_SSSS;
        else if (type_str == "dbxv")
            type = CmsType::CMS_XV;
        else if (type_str == "dbxv2")
            type = CmsType::CMS_XV2;
        else
            DPRINTF("Not recognized cms type: %s\n", type_str.c_str());
    }
    else
    {
        type = CmsType::CMS_SSSS;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
       if (elem->ValueStr() == "Entry")
       {
           CmsEntry *entry;

           if (type == CmsType::CMS_SSSS)
           {
               entry = new CmsEntrySSSS();
           }
           else if (type == CmsType::CMS_XV)
           {
               entry = new CmsEntryXV();
           }
           else
           {
               entry = new CmsEntryXV2();
           }

           if (!entry->Compile(elem))
           {
               DPRINTF("%s: Entry compilation failed.\n", FUNCNAME);
               return false;
           }

           entries.push_back(entry);
       }
    }

    return true;
}

CmsEntry *CmsFile::FindEntryByName(const std::string &name)
{
    std::string name_upper = Utils::ToUpperCase(name);

    for (CmsEntry *entry : entries)
    {
        if (Utils::ToUpperCase(entry->name) == name_upper)
            return entry;
    }

    return nullptr;
}

CmsEntry *CmsFile::FindEntryByID(uint32_t id)
{
    for (CmsEntry *entry : entries)
    {
        if (entry->id == id)
            return entry;
    }

    return nullptr;
}

bool CmsFile::AddEntryXV2(CmsEntryXV2 &entry, bool auto_id)
{
    if (!IsXV2())
        return false;

    if (FindEntryByName(entry.name))
        return false;

    if (!auto_id)
    {
        if (FindEntryByID(entry.id))
            return false;
    }
    else
    {
        entry.id = XV2_FREE_ID_SEARCH_START;

        while (FindEntryByID(entry.id))
            entry.id++;
    }

    CmsEntryXV2 *copy = new CmsEntryXV2();
    *copy = entry;

    entries.push_back(copy);
    std::sort(entries.begin(), entries.end(),
              [](CmsEntry *const &e1, CmsEntry *const &e2) -> bool
              {
                return (e1->id < e2->id);
              }
    );

    return true;
}

bool CmsFile::RemoveEntry(const std::string &name, bool *existed)
{
    std::string name_upper = Utils::ToUpperCase(name);

    for (size_t i = 0; i < entries.size(); i++)
    {
        CmsEntry *entry = entries[i];

        if (Utils::ToUpperCase(entry->name) == name_upper)
        {
            entries.erase(entries.begin()+i);

            if (existed)
                *existed = true;

            return true;
        }
    }

    if (existed)
        *existed = false;

    return true;
}
