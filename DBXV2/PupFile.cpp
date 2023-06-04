#include "PupFile.h"
#include "debug.h"

#define COPY_IN(n) entry.n = file_entries[i].n
#define COPY_OUT(n) file_entries[i].n = entry.n

TiXmlElement *PupEntry::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("PupEntry");

    entry_root->SetAttribute("id", Utils::UnsignedToString(id, true));

    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "SUPER_SOUL1", super_soul1);
    Utils::WriteParamUnsigned(entry_root, "SUPER_SOUL2", super_soul2);
    Utils::WriteParamFloat(entry_root, "HEA", hea);
    Utils::WriteParamFloat(entry_root, "F_14", unk_14);
    Utils::WriteParamFloat(entry_root, "KI", ki);
    Utils::WriteParamFloat(entry_root, "KI_RECOVERY", ki_recovery);
    Utils::WriteParamFloat(entry_root, "STM", stm);
    Utils::WriteParamFloat(entry_root, "STAMINA_RECOVERY", stamina_recovery);
    Utils::WriteParamFloat(entry_root, "ENEMY_STAMINA_ERASER", enemy_stamina_eraser);
    Utils::WriteParamFloat(entry_root, "STAMINA_ERASER", stamina_eraser);
    Utils::WriteParamFloat(entry_root, "F_30", unk_30);
    Utils::WriteParamFloat(entry_root, "ATK", atk);
    Utils::WriteParamFloat(entry_root, "BASIC_KI_ATTACK", basic_ki_attack);
    Utils::WriteParamFloat(entry_root, "STR", str);
    Utils::WriteParamFloat(entry_root, "BLA", bla);
    Utils::WriteParamFloat(entry_root, "ATK_DAMAGE", atk_damage);
    Utils::WriteParamFloat(entry_root, "KI_DAMAGE", ki_damage);
    Utils::WriteParamFloat(entry_root, "STR_DAMAGE", str_damage);
    Utils::WriteParamFloat(entry_root, "BLA_DAMAGE", bla_damage);
    Utils::WriteParamFloat(entry_root, "GROUND_SPEED", ground_speed);
    Utils::WriteParamFloat(entry_root, "AIR_SPEED", air_speed);
    Utils::WriteParamFloat(entry_root, "BOOSTING_SPEED", boosting_speed);
    Utils::WriteParamFloat(entry_root, "DASH_SPEED", dash_speed);
    Utils::WriteParamFloat(entry_root, "F_64", unk_64);
    Utils::WriteParamFloat(entry_root, "F_68", unk_68);
    Utils::WriteParamFloat(entry_root, "F_6C", unk_6C);
    Utils::WriteParamFloat(entry_root, "F_70", unk_70);
    Utils::WriteParamFloat(entry_root, "F_74", unk_74);
    Utils::WriteParamFloat(entry_root, "F_78", unk_78);
    Utils::WriteParamFloat(entry_root, "F_7C", unk_7C);
    Utils::WriteParamFloat(entry_root, "F_80", unk_80);
    Utils::WriteParamFloat(entry_root, "F_84", unk_84);
    Utils::WriteParamFloat(entry_root, "F_88", unk_88);
    Utils::WriteParamFloat(entry_root, "F_8C", unk_8C);
    Utils::WriteParamFloat(entry_root, "F_90", unk_90);
    Utils::WriteParamFloat(entry_root, "F_94", unk_94);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool PupEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "id", &id))
    {
        DPRINTF("%s: Parameter id is not optional.\n", FUNCNAME);
        return false;
    }

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04)) return false;
    if (!Utils::GetParamUnsignedWithMultipleNames(root, &super_soul1, "SUPER_SOUL1", "U_08")) return false;
    if (!Utils::GetParamUnsignedWithMultipleNames(root, &super_soul2, "SUPER_SOUL2", "U_0C")) return false;
    if (!Utils::GetParamFloat(root, "HEA", &hea)) return false;
    if (!Utils::GetParamFloat(root, "F_14", &unk_14)) return false;
    if (!Utils::GetParamFloat(root, "KI", &ki)) return false;
    if (!Utils::GetParamFloatWithMultipleNames(root, &ki_recovery, "KI_RECOVERY", "F_1C")) return false;
    if (!Utils::GetParamFloat(root, "STM", &stm)) return false;
    if (!Utils::GetParamFloat(root, "STAMINA_RECOVERY", &stamina_recovery)) return false;
    if (!Utils::GetParamFloat(root, "ENEMY_STAMINA_ERASER", &enemy_stamina_eraser)) return false;
    if (!Utils::GetParamFloat(root, "STAMINA_ERASER", &stamina_eraser)) return false;
    if (!Utils::GetParamFloat(root, "F_30", &unk_30)) return false;
    if (!Utils::GetParamFloat(root, "ATK", &atk)) return false;
    if (!Utils::GetParamFloat(root, "BASIC_KI_ATTACK", &basic_ki_attack)) return false;
    if (!Utils::GetParamFloat(root, "STR", &str)) return false;
    if (!Utils::GetParamFloat(root, "BLA", &bla)) return false;
    if (!Utils::GetParamFloat(root, "ATK_DAMAGE", &atk_damage)) return false;
    if (!Utils::GetParamFloat(root, "KI_DAMAGE", &ki_damage)) return false;
    if (!Utils::GetParamFloat(root, "STR_DAMAGE", &str_damage)) return false;
    if (!Utils::GetParamFloat(root, "BLA_DAMAGE", &bla_damage)) return false;
    if (!Utils::GetParamFloat(root, "GROUND_SPEED", &ground_speed)) return false;
    if (!Utils::GetParamFloat(root, "AIR_SPEED", &air_speed)) return false;
    if (!Utils::GetParamFloat(root, "BOOSTING_SPEED", &boosting_speed)) return false;
    if (!Utils::GetParamFloat(root, "DASH_SPEED", &dash_speed)) return false;
    if (!Utils::GetParamFloat(root, "F_64", &unk_64)) return false;
    if (!Utils::GetParamFloat(root, "F_68", &unk_68)) return false;
    if (!Utils::GetParamFloat(root, "F_6C", &unk_6C)) return false;
    if (!Utils::GetParamFloat(root, "F_70", &unk_70)) return false;
    if (!Utils::GetParamFloat(root, "F_74", &unk_74)) return false;
    if (!Utils::GetParamFloat(root, "F_78", &unk_78)) return false;
    if (!Utils::GetParamFloat(root, "F_7C", &unk_7C)) return false;
    if (!Utils::GetParamFloat(root, "F_80", &unk_80)) return false;
    if (!Utils::GetParamFloat(root, "F_84", &unk_84)) return false;
    if (!Utils::GetParamFloat(root, "F_88", &unk_88)) return false;
    if (!Utils::GetParamFloat(root, "F_8C", &unk_8C)) return false;
    if (!Utils::GetParamFloat(root, "F_90", &unk_90)) return false;
    if (!Utils::GetParamFloat(root, "F_94", &unk_94)) return false;

    return true;
}

PupFile::PupFile()
{

}

PupFile::~PupFile()
{

}

void PupFile::Reset()
{
    entries.clear();
}

bool PupFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(PUPHeader))
        return false;

    const PUPHeader *hdr = (const PUPHeader *)buf;
    if (hdr->signature != PUP_SIGNATURE)
        return false;

    const PUPEntry *file_entries = (const PUPEntry *)(buf + hdr->header_size);
    entries.resize(hdr->num_entries);

    for (size_t i = 0; i < entries.size(); i++)
    {
        PupEntry &entry = entries[i];

        COPY_IN(id);
        COPY_IN(unk_04);
        COPY_IN(super_soul1);
        COPY_IN(super_soul2);
        COPY_IN(hea);
        COPY_IN(unk_14);
        COPY_IN(ki);
        COPY_IN(ki_recovery);
        COPY_IN(stm);
        COPY_IN(stamina_recovery);
        COPY_IN(enemy_stamina_eraser);
        COPY_IN(stamina_eraser);
        COPY_IN(unk_30);
        COPY_IN(atk);
        COPY_IN(basic_ki_attack);
        COPY_IN(str);
        COPY_IN(bla);
        COPY_IN(atk_damage);
        COPY_IN(ki_damage);
        COPY_IN(str_damage);
        COPY_IN(bla_damage);
        COPY_IN(ground_speed);
        COPY_IN(air_speed);
        COPY_IN(boosting_speed);
        COPY_IN(dash_speed);
        COPY_IN(unk_64);
        COPY_IN(unk_68);
        COPY_IN(unk_6C);
        COPY_IN(unk_70);
        COPY_IN(unk_74);
        COPY_IN(unk_78);
        COPY_IN(unk_7C);
        COPY_IN(unk_80);
        COPY_IN(unk_84);
        COPY_IN(unk_88);
        COPY_IN(unk_8C);
        COPY_IN(unk_90);
        COPY_IN(unk_94);
    }

    return true;
}

uint8_t *PupFile::Save(size_t *psize)
{
    size_t size = sizeof(PUPHeader) + entries.size() * sizeof(PUPEntry);
    uint8_t *buf = new uint8_t[size];
    memset(buf, 0, size);

    PUPHeader *hdr = (PUPHeader *)buf;
    hdr->signature = PUP_SIGNATURE;
    hdr->endianess_check = 0xFFFE;
    hdr->header_size = sizeof(PUPHeader);
    hdr->num_entries = (uint32_t)entries.size();

    PUPEntry *file_entries = (PUPEntry *)(hdr+1);

    for (size_t i = 0; i < entries.size(); i++)
    {
        const PupEntry &entry = entries[i];

        COPY_OUT(id);
        COPY_OUT(unk_04);
        COPY_OUT(super_soul1);
        COPY_OUT(super_soul2);
        COPY_OUT(hea);
        COPY_OUT(unk_14);
        COPY_OUT(ki);
        COPY_OUT(ki_recovery);
        COPY_OUT(stm);
        COPY_OUT(stamina_recovery);
        COPY_OUT(enemy_stamina_eraser);
        COPY_OUT(stamina_eraser);
        COPY_OUT(unk_30);
        COPY_OUT(atk);
        COPY_OUT(basic_ki_attack);
        COPY_OUT(str);
        COPY_OUT(bla);
        COPY_OUT(atk_damage);
        COPY_OUT(ki_damage);
        COPY_OUT(str_damage);
        COPY_OUT(bla_damage);
        COPY_OUT(ground_speed);
        COPY_OUT(air_speed);
        COPY_OUT(boosting_speed);
        COPY_OUT(dash_speed);
        COPY_OUT(unk_64);
        COPY_OUT(unk_68);
        COPY_OUT(unk_6C);
        COPY_OUT(unk_70);
        COPY_OUT(unk_74);
        COPY_OUT(unk_78);
        COPY_OUT(unk_7C);
        COPY_OUT(unk_80);
        COPY_OUT(unk_84);
        COPY_OUT(unk_88);
        COPY_OUT(unk_8C);
        COPY_OUT(unk_90);
        COPY_OUT(unk_94);
    }

    *psize = size;
    return buf;
}

TiXmlDocument *PupFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("PUP");

    for (const PupEntry &entry : entries)
    {
        entry.Decompile(root);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool PupFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "PUP");

    if (!root)
    {
        DPRINTF("Cannot find\"PUP\" in xml.\n");
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "PupEntry")
        {
            PupEntry entry;

            if (!entry.Compile(elem))
                return false;

            entries.push_back(entry);
        }
    }

    return true;
}

PupEntry *PupFile::FindEntryByID(uint32_t id)
{
    for (PupEntry &entry : entries)
    {
        if (entry.id == id)
            return &entry;
    }

    return nullptr;
}

bool PupFile::AddEntry(PupEntry &entry)
{
    for (entry.id = PUP_CUSTOM_ID_START; FindEntryByID(entry.id); entry.id++)
    {
    }

    entries.push_back(entry);
    return true;
}

size_t PupFile::RemoveEntry(uint32_t id)
{
    size_t count = 0;

    for (size_t i = 0; i < entries.size(); i++)
    {
        if (entries[i].id == id)
        {
            entries.erase(entries.begin()+i);
            i--;
            count++;
        }
    }

    return count;
}

bool PupFile::AddConsecutiveEntries(std::vector<PupEntry> &input_entries)
{
    if (input_entries.size() == 0)
        return true; // Yes, true

    uint32_t id;

    for (id = PUP_CUSTOM_ID_START; ; id++)
    {
        bool found = true;

        for (uint32_t i = 0; i < (uint32_t)input_entries.size(); i++)
        {
            if (FindEntryByID(id+i))
            {
                found = false;
                break;
            }
        }

        if (found)
            break;
    }

    for (PupEntry &entry : input_entries)
    {
        entry.id = id;
        entries.push_back(entry);
        id++;
    }

    return true;
}

