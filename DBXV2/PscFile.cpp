#include "PscFile.h"
#include "Xenoverse2.h"
#include "MemoryStream.h"
#include "debug.h"

#define COPY_VAL(a, b, f) a.f = b->f
#define COPY_VAL2(a, b, f) a.f = b.f

#define COPY_I(f) COPY_VAL(spec, file_spec, f)
#define COPY_O(f) COPY_VAL2(file_spec, spec, f)

TiXmlElement *PscSpecEntry::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("PscSpecEntry");    

    Utils::WriteParamUnsigned(entry_root, "COSTUME_ID", costume_id, true);
    Utils::WriteParamUnsigned(entry_root, "COSTUME_ID2", costume_id2, true);
    Utils::WriteParamUnsigned(entry_root, "CAMERA_POSITION", camera_position, true);
    Utils::WriteParamUnsigned(entry_root, "U_0C", unk_0C, true);
    Utils::WriteParamUnsigned(entry_root, "U_10", unk_10, true);
    Utils::WriteParamUnsigned(entry_root, "U_14", unk_14, true);

    Utils::WriteParamFloat(entry_root, "HEALTH", health);
    Utils::WriteParamFloat(entry_root, "F_18", unk_18);
    Utils::WriteParamFloat(entry_root, "KI", ki);
    Utils::WriteParamFloat(entry_root, "KI_RECHARGE", ki_recharge);

    Utils::WriteParamUnsigned(entry_root, "U_24", unk_24, true);
    Utils::WriteParamUnsigned(entry_root, "U_28", unk_28, true);
    Utils::WriteParamUnsigned(entry_root, "U_2C", unk_2C, true);

    Utils::WriteParamFloat(entry_root, "STAMINA", stamina);
    Utils::WriteParamFloat(entry_root, "STAMINA_RECHARGE_MOVE", stamina_recharge_move);
    Utils::WriteParamFloat(entry_root, "STAMINA_RECHARGE_AIR", stamina_recharge_air);
    Utils::WriteParamFloat(entry_root, "STAMINA_RECHARGE_GROUND", stamina_recharge_ground);
    Utils::WriteParamFloat(entry_root, "STAMINA_DRAIN_RATE1", stamina_drain_rate1);
    Utils::WriteParamFloat(entry_root, "STAMINA_DRAIN_RATE2", stamina_drain_rate2);
    Utils::WriteParamFloat(entry_root, "F_48", unk_48);
    Utils::WriteParamFloat(entry_root, "BASIC_ATTACK", basic_attack);
    Utils::WriteParamFloat(entry_root, "BASIC_KI_ATTACK", basic_ki_attack);
    Utils::WriteParamFloat(entry_root, "STRIKE_ATTACK", strike_attack);
    Utils::WriteParamFloat(entry_root, "KI_BLAST_SUPER", ki_blast_super);
    Utils::WriteParamFloat(entry_root, "BASIC_PHYS_DEFENSE", basic_phys_defense);
    Utils::WriteParamFloat(entry_root, "BASIC_KI_DEFENSE", basic_ki_defense);
    Utils::WriteParamFloat(entry_root, "STRIKE_ATK_DEFENSE", strike_atk_defense);
    Utils::WriteParamFloat(entry_root, "SUPER_KI_BLAST_DEFENSE", super_ki_blast_defense);
    Utils::WriteParamFloat(entry_root, "GROUND_SPEED", ground_speed);
    Utils::WriteParamFloat(entry_root, "AIR_SPEED", air_speed);
    Utils::WriteParamFloat(entry_root, "BOOSTING_SPEED", boosting_speed);
    Utils::WriteParamFloat(entry_root, "DASH_DISTANCE", dash_distance);
    Utils::WriteParamFloat(entry_root, "F_7C", unk_7C);
    Utils::WriteParamFloat(entry_root, "REINF_SKILL_DURATION", reinf_skill_duration);
    Utils::WriteParamFloat(entry_root, "F_84", unk_84);
    Utils::WriteParamFloat(entry_root, "REVIVAL_HP_AMOUNT", revival_hp_amount);
    Utils::WriteParamFloat(entry_root, "F_8C", unk_8C);
    Utils::WriteParamFloat(entry_root, "REVIVING_SPEED", reviving_speed);

    Utils::WriteParamUnsigned(entry_root, "U_98", unk_98, true);
    Utils::WriteParamUnsigned(entry_root, "TALISMAN", talisman, true);
    Utils::WriteParamUnsigned(entry_root, "U_B8", unk_B8, true);
    Utils::WriteParamUnsigned(entry_root, "U_BC", unk_BC, true);

    Utils::WriteParamFloat(entry_root, "F_C0", unk_C0);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool PscSpecEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "COSTUME_ID", &costume_id))
        return false;

    if (!Utils::GetParamUnsigned(root, "COSTUME_ID2", &costume_id2))
        return false;

    if (!Utils::GetParamUnsigned(root, "CAMERA_POSITION", &camera_position))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0C", &unk_0C))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_10", &unk_10))
        return false;

    if (!Utils::ReadParamUnsigned(root, "U_14", &unk_14))
    {
        // This thing added in 1.20. Seems this is the typical value
        // (This default value will be used when installing old x2m files)
        unk_14 = 0x3D;
    }

    if (!Utils::GetParamFloat(root, "HEALTH", &health))
        return false;

    if (!Utils::GetParamFloat(root, "F_18", &unk_18))
        return false;

    if (!Utils::GetParamFloat(root, "KI", &ki))
        return false;

    if (!Utils::GetParamFloat(root, "KI_RECHARGE", &ki_recharge))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_24", &unk_24))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_28", &unk_28))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_2C", &unk_2C))
        return false;

    if (!Utils::GetParamFloat(root, "STAMINA", &stamina))
        return false;

    if (!Utils::GetParamFloat(root, "STAMINA_RECHARGE_MOVE", &stamina_recharge_move))
        return false;

    if (!Utils::GetParamFloat(root, "STAMINA_RECHARGE_AIR", &stamina_recharge_air))
        return false;

    if (!Utils::GetParamFloat(root, "STAMINA_RECHARGE_GROUND", &stamina_recharge_ground))
        return false;

    if (!Utils::GetParamFloat(root, "STAMINA_DRAIN_RATE1", &stamina_drain_rate1))
        return false;

    if (!Utils::GetParamFloat(root, "STAMINA_DRAIN_RATE2", &stamina_drain_rate2))
        return false;

    if (!Utils::GetParamFloat(root, "F_48", &unk_48))
        return false;

    if (!Utils::GetParamFloat(root, "BASIC_ATTACK", &basic_attack))
        return false;

    if (!Utils::GetParamFloat(root, "BASIC_KI_ATTACK", &basic_ki_attack))
        return false;

    if (!Utils::GetParamFloat(root, "STRIKE_ATTACK", &strike_attack))
        return false;

    if (!Utils::GetParamFloat(root, "KI_BLAST_SUPER", &ki_blast_super))
        return false;

    if (!Utils::GetParamFloat(root, "BASIC_PHYS_DEFENSE", &basic_phys_defense))
        return false;

    if (!Utils::GetParamFloat(root, "BASIC_KI_DEFENSE", &basic_ki_defense))
        return false;

    if (!Utils::GetParamFloat(root, "STRIKE_ATK_DEFENSE", &strike_atk_defense))
        return false;

    if (!Utils::GetParamFloat(root, "SUPER_KI_BLAST_DEFENSE", &super_ki_blast_defense))
        return false;

    if (!Utils::GetParamFloat(root, "GROUND_SPEED", &ground_speed))
        return false;

    if (!Utils::GetParamFloat(root, "AIR_SPEED", &air_speed))
        return false;

    if (!Utils::GetParamFloat(root, "BOOSTING_SPEED", &boosting_speed))
        return false;

    if (!Utils::GetParamFloat(root, "DASH_DISTANCE", &dash_distance))
        return false;

    if (!Utils::GetParamFloat(root, "F_7C", &unk_7C))
        return false;

    if (!Utils::GetParamFloat(root, "REINF_SKILL_DURATION", &reinf_skill_duration))
        return false;

    if (!Utils::GetParamFloat(root, "F_84", &unk_84))
        return false;

    if (!Utils::GetParamFloat(root, "REVIVAL_HP_AMOUNT", &revival_hp_amount))
        return false;

    if (!Utils::GetParamFloat(root, "F_8C", &unk_8C))
        return false;

    if (!Utils::GetParamFloat(root, "REVIVING_SPEED", &reviving_speed))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_98", &unk_98))
        return false;

    if (!Utils::GetParamUnsigned(root, "TALISMAN", &talisman))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_B8", &unk_B8))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_BC", &unk_BC))
        return false;

    if (!Utils::GetParamFloat(root, "F_C0", &unk_C0))
        return false;

    return true;
}

TiXmlElement *PscEntry::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("PscEntry");

    entry_root->SetAttribute("char_id", Utils::UnsignedToString(char_id, true));

    for (const PscSpecEntry &entry :specs)
    {
        TiXmlElement *elem = entry.Decompile(entry_root);

        std::string name = Xenoverse2::GetCharaAndCostumeName(char_id, entry.costume_id, 0, true);

        if (name.length() != 0)
        {
            TiXmlComment comment;
            comment.SetValue(name);

            const TiXmlElement *costume_id;
            if (Utils::GetElemCount(elem, "COSTUME_ID", &costume_id) != 0)
                elem->InsertBeforeChild(const_cast<TiXmlElement *>(costume_id), comment);
        }
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool PscEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "char_id", &char_id))
    {
        DPRINTF("%s: char_id attribute is not optional.\n", FUNCNAME);
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "PscSpecEntry")
        {
            PscSpecEntry spec;

            if (!spec.Compile(elem))
                return false;

            specs.push_back(spec);
        }
    }

    return true;
}

PscFile::PscFile()
{
    this->big_endian = false;
}

PscFile::~PscFile()
{

}

void PscFile::Reset()
{
    configurations.clear();
}

bool PscFile::CheckNumEntries() const
{
    if (configurations.size() == 0)
        return true;

    size_t num = configurations[0].size();

    for (size_t i = 1; i < configurations.size(); i++)
    {
        if (configurations[i].size() != num)
            return false;
    }

    return true;
}

bool PscFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(PSCHeader))
        return false;

    const PSCHeader *hdr = (const PSCHeader *)buf;

    if (hdr->signature != PSC_SIGNATURE && memcmp(buf+1, "PSC", 3) != 0)
        return false;

    if (hdr->header_size < sizeof(PSCHeader))
    {
        DPRINTF("%s: Cannot load old-style psc.\n", FUNCNAME);
        return false;
    }
    else if (hdr->header_size > sizeof(PSCHeader))
    {
        DPRINTF("%s: PSC format changed, cannot understand it.\n", FUNCNAME);
        return false;
    }

    configurations.resize(hdr->num_configs);

    const PSCEntry *file_entry = (const PSCEntry *)(hdr+1);
    const PSCSpecEntry *file_spec = (const PSCSpecEntry *)(file_entry+hdr->num_entries*2);

    for (size_t c = 0; c < configurations.size(); c++)
    {
        std::vector<PscEntry> &entries = configurations[c];

        entries.resize(hdr->num_entries);

        for (size_t i = 0; i < entries.size(); i++)
        {
            PscEntry &entry = entries[i];

            entry.char_id = file_entry->char_id;
            entry.specs.resize(file_entry->num_specs);

            for (size_t j = 0; j < entry.specs.size(); j++)
            {
                PscSpecEntry &spec = entry.specs[j];

                COPY_I(costume_id);
                COPY_I(costume_id2);
                COPY_I(camera_position);
                COPY_I(unk_0C);
                COPY_I(unk_10);
                COPY_I(unk_14);
                COPY_I(health);
                COPY_I(unk_18);
                COPY_I(ki);
                COPY_I(ki_recharge);
                COPY_I(unk_24);
                COPY_I(unk_28);
                COPY_I(unk_2C);
                COPY_I(stamina);
                COPY_I(stamina_recharge_move);
                COPY_I(stamina_recharge_air);
                COPY_I(stamina_recharge_ground);
                COPY_I(stamina_drain_rate1);
                COPY_I(stamina_drain_rate2);
                COPY_I(unk_48);
                COPY_I(basic_attack);
                COPY_I(basic_ki_attack);
                COPY_I(strike_attack);
                COPY_I(ki_blast_super);
                COPY_I(basic_phys_defense);
                COPY_I(basic_ki_defense);
                COPY_I(strike_atk_defense);
                COPY_I(super_ki_blast_defense);
                COPY_I(ground_speed);
                COPY_I(air_speed);
                COPY_I(boosting_speed);
                COPY_I(dash_distance);
                COPY_I(unk_7C);
                COPY_I(reinf_skill_duration);
                COPY_I(unk_84);
                COPY_I(revival_hp_amount);
                COPY_I(unk_8C);
                COPY_I(reviving_speed);
                COPY_I(unk_98);
                COPY_I(talisman);
                COPY_I(unk_B8);
                COPY_I(unk_BC);
                COPY_I(unk_C0);

                file_spec++;
            }

            file_entry++;
        }
    }

    if (!CheckNumEntries())
    {
        DPRINTF("%s: the number of entries of configurations is not identical.\n", FUNCNAME);
        return false;
    }

    return true;
}

uint8_t *PscFile::Save(size_t *psize)
{
    MemoryStream stream;
    PSCHeader hdr;

    if (!CheckNumEntries())
    {
        DPRINTF("%s: the number of entries of configurations is not identical.\n", FUNCNAME);
        return nullptr;
    }

    memset(&hdr, 0, sizeof(hdr));
    hdr.signature = PSC_SIGNATURE;
    hdr.endianess_check = val16(0xFFFE);
    hdr.header_size = (uint16_t)sizeof(PSCHeader);
    hdr.num_entries = (uint32_t)configurations[0].size();
    hdr.num_configs = (uint32_t)configurations.size();

    if (!stream.Write(&hdr, sizeof(hdr)))
        return nullptr;

    for (const std::vector<PscEntry> &entries : configurations)
    {
        for (const PscEntry &entry : entries)
        {
            PSCEntry file_entry;

            memset(&file_entry, 0, sizeof(file_entry));
            file_entry.char_id = entry.char_id;
            file_entry.num_specs = (uint32_t)entry.specs.size();

            if (!stream.Write(&file_entry, sizeof(file_entry)))
                return nullptr;
        }
    }

    for (const std::vector<PscEntry> &entries : configurations)
    {
        for (const PscEntry &entry : entries)
        {
            for (const PscSpecEntry &spec : entry.specs)
            {
                PSCSpecEntry file_spec;

                memset(&file_spec, 0, sizeof(file_spec));

                COPY_O(costume_id);
                COPY_O(costume_id2);
                COPY_O(camera_position);
                COPY_O(unk_0C);
                COPY_O(unk_10);
                COPY_O(unk_14);
                COPY_O(health);
                COPY_O(unk_18);
                COPY_O(ki);
                COPY_O(ki_recharge);
                COPY_O(unk_24);
                COPY_O(unk_28);
                COPY_O(unk_2C);
                COPY_O(stamina);
                COPY_O(stamina_recharge_move);
                COPY_O(stamina_recharge_air);
                COPY_O(stamina_recharge_ground);
                COPY_O(stamina_drain_rate1);
                COPY_O(stamina_drain_rate2);
                COPY_O(unk_48);
                COPY_O(basic_attack);
                COPY_O(basic_ki_attack);
                COPY_O(strike_attack);
                COPY_O(ki_blast_super);
                COPY_O(basic_phys_defense);
                COPY_O(basic_ki_defense);
                COPY_O(strike_atk_defense);
                COPY_O(super_ki_blast_defense);
                COPY_O(ground_speed);
                COPY_O(air_speed);
                COPY_O(boosting_speed);
                COPY_O(dash_distance);
                COPY_O(unk_7C);
                COPY_O(reinf_skill_duration);
                COPY_O(unk_84);
                COPY_O(revival_hp_amount);
                COPY_O(unk_8C);
                COPY_O(reviving_speed);
                COPY_O(unk_98);
                COPY_O(talisman);
                COPY_O(unk_B8);
                COPY_O(unk_BC);
                COPY_O(unk_C0);

                if (!stream.Write(&file_spec, sizeof(file_spec)))
                    return nullptr;
            }
        }
    }

    *psize = stream.GetSize();
    return stream.GetMemory(true);
}

TiXmlElement *PscFile::DecompileConfig(TiXmlNode *root, size_t cfg) const
{
    TiXmlElement *entry_root = new TiXmlElement("Configuration");

    for (const PscEntry &entry : configurations[cfg])
    {
        entry.Decompile(entry_root);
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool PscFile::CompileConfig(const TiXmlElement *root, size_t cfg)
{
    std::vector<PscEntry> &entries = configurations[cfg];

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "PscEntry")
        {
            PscEntry entry;

            if (!entry.Compile(elem))
                return false;

            entries.push_back(entry);
        }
    }

    return true;
}

TiXmlDocument *PscFile::Decompile() const
{
    if (!CheckNumEntries())
    {
        DPRINTF("%s: the number of entries of configurations is not identical.\n", FUNCNAME);
        return nullptr;
    }

    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("PSC");

    for (size_t cfg = 0; cfg < configurations.size(); cfg++)
    {
        DecompileConfig(root, cfg);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool PscFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "PSC");

    if (!root)
    {
        DPRINTF("Cannot find\"PSC\" in xml.\n");
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Configuration")
        {
            configurations.push_back(std::vector<PscEntry>());

            if (!CompileConfig(elem, configurations.size()-1))
                return false;
        }
    }

    if (!CheckNumEntries())
    {
        DPRINTF("%s: the number of entries of configurations is not identical.\n", FUNCNAME);
        return false;
    }

    return true;
}

/*size_t PscFile::GetTotalSpecEntries() const
{
    size_t size = 0;

    for (const PscEntry& entry: entries)
        size += entry.specs.size();

    return size;
}*/

PscSpecEntry *PscFile::FindSpecFromAbsolutePos(size_t pos, size_t cfg)
{
    size_t current_pos = 0;

    if (cfg >= configurations.size())
        return nullptr;

    for (PscEntry &entry : configurations[cfg])
    {
        size_t last_index = current_pos + entry.specs.size() - 1;

        if (pos <= last_index)
        {
            return &entry.specs[pos-current_pos];
        }

        current_pos += entry.specs.size();
    }

    return nullptr;
}

/*PscEntry *PscFile::FindEntry(uint32_t char_id)
{
    for (PscEntry &entry : entries)
    {
        if (entry.char_id == char_id)
            return &entry;
    }

    return nullptr;
}*/

size_t PscFile::FindEntries(uint32_t char_id, std::vector<PscEntry *> &ret)
{
    ret.clear();

    for (auto &entries : configurations)
    {
        for (PscEntry &entry : entries)
        {
            if (entry.char_id == char_id)
            {
                ret.push_back(&entry);
            }
        }
    }

    if (ret.size() != 0 && ret.size() != configurations.size())
    {
        DPRINTF("%s: Warning, number of results is not same as number of configurations.\n", FUNCNAME);
    }

    return ret.size();
}

PscSpecEntry *PscFile::FindSpec(uint32_t char_id, uint32_t costume_id, size_t cfg)
{
    std::vector<PscEntry *> entries;

     if (FindEntries(char_id, entries) == 0)
        return nullptr;

    for (PscSpecEntry &spec : entries[cfg]->specs)
    {
        if (spec.costume_id == costume_id)
            return &spec;
    }

    return nullptr;
}

size_t PscFile::FindAllSpecs(uint32_t char_id, std::vector<PscSpecEntry *> &specs)
{
    specs.clear();
    std::vector<PscEntry *> entries;

     if (FindEntries(char_id, entries) == 0)
        return 0;

     for (PscEntry *entry : entries)
     {
         for (PscSpecEntry &spec : entry->specs)
         {
             specs.push_back(&spec);
         }
     }

    return specs.size();
}

bool PscFile::AddEntry(const PscEntry &entry, int cfg)
{
    if (cfg >= 0)
    {
        if ((size_t)cfg >= configurations.size())
            return false;

        configurations[cfg].push_back(entry);
        return true;
    }

    for (auto &it : configurations)
    {
        it.push_back(entry);
    }

    return true;
}

void PscFile::RemoveEntry(uint32_t char_id)
{
    // There should be only one entry per char_id, but... we'll check for duplicates anyway

    for (auto &entries : configurations)
    {
        for (size_t i = 0; i < entries.size(); i++)
        {
            if (entries[i].char_id == char_id)
            {
                entries.erase(entries.begin()+i);
                i--;
            }
        }
    }
}

size_t PscFile::RemoveTalismanReferences(uint32_t talisman_id)
{
    size_t count = 0;

    for (auto &entries : configurations)
    {
        for (PscEntry &entry : entries)
        {
            for (PscSpecEntry &spec : entry.specs)
            {
                if (spec.talisman == talisman_id)
                    spec.talisman = 0xFFFFFFFF;
            }
        }
    }

    return count;
}
