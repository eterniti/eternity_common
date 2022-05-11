#include "BgrFile.h"
#include "SsssData.h"

#include "debug.h"

void BgrEntry::DecompileAssistPhrase(TiXmlElement *root, const char *name, uint32_t assist_phrase) const
{
    SsssAssistPhraseInfo *info = SsssData::FindAssistPhraseInfo(assist_phrase);

    if (info)
    {
        Utils::WriteComment(root, std::string(" ") + info->name + " ");
    }

    Utils::WriteParamUnsigned(root, name, assist_phrase, true);
}


void BgrEntry::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("BgrEntry");

    SsssCharInfo *info = SsssData::FindInfo(cms_entry, cms_model_spec_idx);
    if (info)
    {
        Utils::WriteComment(entry_root, std::string(" ") + info->model_name + std::string(" / ") + info->char_name);
    }

    Utils::WriteParamUnsigned(entry_root, "CMS_ENTRY", cms_entry, true);
    Utils::WriteParamUnsigned(entry_root, "CMS_MODEL_SPEC_IDX", cms_model_spec_idx);

    Utils::WriteComment(entry_root, " 0 = Outer clock (easy), 1 = clock at center (difficult), 2 = inner clock (most difficult, aka bosses)");
    Utils::WriteParamUnsigned(entry_root, "LEVEL", level, true);
    Utils::WriteComment(entry_root, " 0 = Aries hour, 11 = Pisces hour, 12 = Any hour");
    Utils::WriteParamUnsigned(entry_root, "HOUR", hour);

    Utils::WriteParamUnsigned(entry_root, "U_08", unk_08, true);
    Utils::WriteParamUnsigned(entry_root, "U_14", unk_14, true);
    Utils::WriteParamUnsigned(entry_root, "U_18", unk_18, true);

    DecompileAssistPhrase(entry_root, "ASSIST_PHRASE1", assist_phrase1);
    DecompileAssistPhrase(entry_root, "ASSIST_PHRASE2", assist_phrase2);
    DecompileAssistPhrase(entry_root, "ASSIST_PHRASE3", assist_phrase3);
    DecompileAssistPhrase(entry_root, "ASSIST_PHRASE4", assist_phrase4);
    DecompileAssistPhrase(entry_root, "ASSIST_PHRASE5", assist_phrase5);
    DecompileAssistPhrase(entry_root, "ASSIST_PHRASE6", assist_phrase6);
    DecompileAssistPhrase(entry_root, "ASSIST_PHRASE7", assist_phrase7);
    DecompileAssistPhrase(entry_root, "ASSIST_PHRASE8", assist_phrase8);

    Utils::WriteParamUnsigned(entry_root, "STAGE", stage, true);
    Utils::WriteParamUnsigned(entry_root, "HP", hp);
    Utils::WriteParamUnsigned(entry_root, "U_44", unk_44, true);

    root->LinkEndChild(entry_root);
}

bool BgrEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "CMS_ENTRY", &cms_entry))
        return false;

    if (!Utils::GetParamUnsigned(root, "CMS_MODEL_SPEC_IDX", &cms_model_spec_idx))
        return false;

    if (!Utils::GetParamUnsigned(root, "LEVEL", &level))
        return false;

    if (!Utils::GetParamUnsigned(root, "HOUR", &hour))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_08", &unk_08))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_14", &unk_14))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_18", &unk_18))
        return false;

    if (!Utils::GetParamUnsigned(root, "ASSIST_PHRASE1", &assist_phrase1))
        return false;

    if (!Utils::GetParamUnsigned(root, "ASSIST_PHRASE2", &assist_phrase2))
        return false;

    if (!Utils::GetParamUnsigned(root, "ASSIST_PHRASE3", &assist_phrase3))
        return false;

    if (!Utils::GetParamUnsigned(root, "ASSIST_PHRASE4", &assist_phrase4))
        return false;

    if (!Utils::GetParamUnsigned(root, "ASSIST_PHRASE5", &assist_phrase5))
        return false;

    if (!Utils::GetParamUnsigned(root, "ASSIST_PHRASE6", &assist_phrase6))
        return false;

    if (!Utils::GetParamUnsigned(root, "ASSIST_PHRASE7", &assist_phrase7))
        return false;

    if (!Utils::GetParamUnsigned(root, "ASSIST_PHRASE8", &assist_phrase8))
        return false;

    if (!Utils::GetParamUnsigned(root, "STAGE", &stage))
        return false;

    if (!Utils::GetParamUnsigned(root, "HP", &hp))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_44", &unk_44))
        return false;

    return true;
}

BgrFile::BgrFile()
{
    this->big_endian = false;
}

BgrFile::~BgrFile()
{

}

void BgrFile::Reset()
{
    entries.clear();
}

size_t BgrFile::RemoveChar(uint32_t cms_entry, uint32_t cms_model_spec_idx)
{
    size_t count = 0;

    for (size_t i = 0; i < GetNumEntries(); i++)
    {
        const BgrEntry &entry = entries[i];

        if (entry.cms_entry == cms_entry && entry.cms_model_spec_idx == cms_model_spec_idx)
        {
            entries.erase(entries.begin()+i);
            i--;
            count++;
        }
    }

    return count;
}

bool BgrFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    BGRHeader *hdr = (BGRHeader *)buf;

    if (size < sizeof(BGRHeader) || memcmp(hdr->signature, BGR_SIGNATURE, 4) != 0)
        return false;

    entries.resize(val32(hdr->num_entries));

    BGREntry *fentries = (BGREntry *)GetOffsetPtr(hdr, hdr->data_start);

    for (size_t i = 0; i < hdr->num_entries; i++)
    {
        BgrEntry &entry = entries[i];

        entry.cms_entry = val32(fentries[i].cms_entry);
        entry.cms_model_spec_idx = val32(fentries[i].cms_model_spec_idx);
        entry.level = val32(fentries[i].level);
        entry.hour = val32(fentries[i].hour);
        entry.unk_08 = val32(fentries[i].unk_08);
        entry.unk_14 = val32(fentries[i].unk_14);
        entry.unk_18 = val32(fentries[i].unk_18);
        entry.assist_phrase1 = val32(fentries[i].assist_phrase1);
        entry.assist_phrase2 = val32(fentries[i].assist_phrase2);
        entry.assist_phrase3 = val32(fentries[i].assist_phrase3);
        entry.assist_phrase4 = val32(fentries[i].assist_phrase4);
        entry.assist_phrase5 = val32(fentries[i].assist_phrase5);
        entry.assist_phrase6 = val32(fentries[i].assist_phrase6);
        entry.assist_phrase7 = val32(fentries[i].assist_phrase7);
        entry.assist_phrase8 = val32(fentries[i].assist_phrase8);
        entry.stage = val32(fentries[i].stage);
        entry.hp = val32(fentries[i].hp);
        entry.unk_44 = val32(fentries[i].unk_44);
    }

    return true;
}

uint8_t *BgrFile::Save(size_t *size)
{
    unsigned int file_size = sizeof(BGRHeader) + entries.size() * sizeof(BGREntry);
    
	uint8_t *buf = new uint8_t[file_size];
    memset(buf, 0, file_size);

    BGRHeader *hdr = (BGRHeader *)buf;

    memcpy(hdr->signature, BGR_SIGNATURE, 4);
    hdr->endianess_check = val16(0xFFFE);
    hdr->num_entries = val32(entries.size());
    hdr->data_start = val32(sizeof(BGRHeader));

    BGREntry *fentries = (BGREntry *)GetOffsetPtr(hdr, hdr->data_start);

    for (size_t i = 0; i < entries.size(); i++)
    {
        const BgrEntry &entry = entries[i];

        fentries[i].cms_entry = val32(entry.cms_entry);
        fentries[i].cms_model_spec_idx = val32(entry.cms_model_spec_idx);
        fentries[i].level = val32(entry.level);
        fentries[i].hour = val32(entry.hour);
        fentries[i].unk_08 = val32(entry.unk_08);
        fentries[i].unk_14 = val32(entry.unk_14);
        fentries[i].unk_18 = val32(entry.unk_18);
        fentries[i].assist_phrase1 = val32(entry.assist_phrase1);
        fentries[i].assist_phrase2 = val32(entry.assist_phrase2);
        fentries[i].assist_phrase3 = val32(entry.assist_phrase3);
        fentries[i].assist_phrase4 = val32(entry.assist_phrase4);
        fentries[i].assist_phrase5 = val32(entry.assist_phrase5);
        fentries[i].assist_phrase6 = val32(entry.assist_phrase6);
        fentries[i].assist_phrase7 = val32(entry.assist_phrase7);
        fentries[i].assist_phrase8 = val32(entry.assist_phrase8);
        fentries[i].stage = val32(entry.stage);
        fentries[i].hp = val32(entry.hp);
        fentries[i].unk_44 = val32(entry.unk_44);
    }

    *size = file_size;
    return buf;
}

TiXmlDocument *BgrFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("BGR");

    for (const BgrEntry &entry : entries)
    {
        entry.Decompile(root);
    }

    doc->LinkEndChild(root);

    return doc;
}

bool BgrFile::Compile(TiXmlDocument *doc, bool big_endian)
{
    Reset();
    this->big_endian = big_endian;

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "BGR");

    if (!root)
    {
        DPRINTF("Cannot find\"BGR\" in xml.\n");
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
       if (elem->ValueStr() == "BgrEntry")
       {
           BgrEntry entry;

           if (!entry.Compile(elem))
           {
               DPRINTF("%s: BGREntry compilation failed.\n", FUNCNAME);
               return false;
           }

           entries.push_back(entry);
       }
    }

    return true;
}



