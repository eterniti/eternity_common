#include "BcmFile.h"
#include "debug.h"

#define COPY_IN(n) entry.n = file_entries[i].n
#define COPY_OUT(n) file_entries[i].n = entry.n

TiXmlElement *BcmEntry::Decompile(TiXmlNode *root, uint32_t idx) const
{
    TiXmlElement *entry_root = new TiXmlElement("BcmEntry");

    entry_root->SetAttribute("idx", idx);

    if (sibling != 0)
    {
        entry_root->SetAttribute("sibling", ((sibling-sizeof(BCMHeader)) / sizeof(BCMEntry)));
    }

    if (child != 0)
    {
        entry_root->SetAttribute("child", ((child-sizeof(BCMHeader)) / sizeof(BCMEntry)));
    }

    if (parent != 0)
    {
        entry_root->SetAttribute("parent", ((parent-sizeof(BCMHeader)) / sizeof(BCMEntry)));
    }

    if (this->root != 0)
    {
        entry_root->SetAttribute("root", ((this->root-sizeof(BCMHeader)) / sizeof(BCMEntry)));
    }

    Utils::WriteParamUnsigned(entry_root, "U_00", unk_00, true);

    /*Utils::WriteComment(entry_root, "Known flags 0xAB");
    Utils::WriteComment(entry_root, "B:");
    Utils::WriteComment(entry_root, "0 -> No direction required");
    Utils::WriteComment(entry_root, "1 -> Left only");
    Utils::WriteComment(entry_root, "2 -> Any direction");
    Utils::WriteComment(entry_root, "3 -> Forwards and backwards only");
    Utils::WriteComment(entry_root, "4 -> Left and backwards only");
    Utils::WriteComment(entry_root, "5 -> Left and forwards only");
    Utils::WriteComment(entry_root, "0xa -> Backwards and right only");*/
    Utils::WriteParamUnsigned(entry_root, "DIRECTIONAL_CONDITIONS", directional_conditions, true);

    Utils::WriteParamUnsigned(entry_root, "U_08", unk_08, true);
    Utils::WriteParamUnsigned(entry_root, "U_0A", unk_0A, true);

    /*Utils::WriteComment(entry_root, "Known flags 0xABCDE");
    Utils::WriteComment(entry_root, "A: Charge type. Links with TransformControl TYPE 0x0 in BAC Entry");
    Utils::WriteComment(entry_root, "0 -> Automatic");
    Utils::WriteComment(entry_root, "2 -> Manual");
    Utils::WriteComment(entry_root, "E: Hold-down action");
    Utils::WriteComment(entry_root, "0 -> Continue until released");
    Utils::WriteComment(entry_root, "1 -> Delay skill activation until release");
    Utils::WriteComment(entry_root, "2 -> Unknown difference to default");
    Utils::WriteComment(entry_root, "4 -> Stop skill from activating");*/
    Utils::WriteParamUnsigned(entry_root, "HOLD_DOWN_CONDITIONS", hold_down_conditions, true);

    /*Utils::WriteComment(entry_root, "Known flags 0xABCDE");
    Utils::WriteComment(entry_root, "A: CMS size. 0 -> all sizes");*/
    Utils::WriteParamUnsigned(entry_root, "OPPONENT_SIZE_CONDITIONS", opponent_size_conditions, true);

    Utils::WriteParamUnsigned(entry_root, "U_14", unk_14, true);

    /*Utils::WriteComment(entry_root, "Known flags 0xABCDEFGH");
    Utils::WriteComment(entry_root, "A: Health condition");
    Utils::WriteComment(entry_root, "0 -> (default) No health condition");
    Utils::WriteComment(entry_root, "1 -> User’s health. One-use-only. Health amount is determined by HEALTH_REQUIRED");
    Utils::WriteComment(entry_root, "3 -> Target’s health below 25%. One-use-only");
    Utils::WriteComment(entry_root, "4 -> User’s health. Health amount is determined by HEALTH_REQUIRED");
    Utils::WriteComment(entry_root, "5 -> User’s health. One-use-only. Health amount is determined by HEALTH_REQUIRED");
    Utils::WriteComment(entry_root, "6 -> Target’s health below 25%");
    Utils::WriteComment(entry_root, "8 -> User’s health. Health amount is determined by HEALTH_REQUIRED");
    Utils::WriteComment(entry_root, "9 -> User’s health. One-use-only. Health amount is determined by HEALTH_REQUIRED");
    Utils::WriteComment(entry_root, "E: Additional condition");
    Utils::WriteComment(entry_root, "1 -> Opponent attacks (counter activator)");
    Utils::WriteComment(entry_root, "4 -> Ki Less than 100%");
    Utils::WriteComment(entry_root, "8 -> Ki Greater than 0%");
    Utils::WriteComment(entry_root, "F:  Primary activator condition. This is vital");
    Utils::WriteComment(entry_root, "2 -> Repeatedly flashes on/off unless targeting opponent");
    Utils::WriteComment(entry_root, "3 -> When not moving and in transformed state only");
    Utils::WriteComment(entry_root, "4 -> Repeatedly flashes on/off unless targeting opponent, when not moving, and in transformed state only");
    Utils::WriteComment(entry_root, "6 -> Repeatedly flashes on/off unless targeting opponent, and when not moving only");
    Utils::WriteComment(entry_root, "a -> Only when not moving");
    Utils::WriteComment(entry_root, "c -> Default. Any movement, any state");
    Utils::WriteComment(entry_root, "d -> When in transformed state only");
    Utils::WriteComment(entry_root, "G: Distance and transformation condition");
    Utils::WriteComment(entry_root, "0 -> Default. Any distance, any form");
    Utils::WriteComment(entry_root, "2 -> Close-up to opponent only.");
    Utils::WriteComment(entry_root, "4 -> Any distance except close-up");
    Utils::WriteComment(entry_root, "8 -> Any distance, base-form only");
    Utils::WriteComment(entry_root, "a -> Close-up to opponent and base-form only");
    Utils::WriteComment(entry_root, "c -> Any distance except close-up and base-form only");
    Utils::WriteComment(entry_root, "H: Position condition");
    Utils::WriteComment(entry_root, "0 -> None");
    Utils::WriteComment(entry_root, "1 -> Standing on ground");
    Utils::WriteComment(entry_root, "2 -> Floating in air");
    Utils::WriteComment(entry_root, "8 -> When attack hits");*/
    Utils::WriteParamUnsigned(entry_root, "PRIMARY_ACTIVATOR_CONDITIONS", primary_activator_conditions, true);

    /*Utils::WriteComment(entry_root, "Known flags 0xAB");
    Utils::WriteComment(entry_root, "A: Skill type?");
    Utils::WriteComment(entry_root, "0 -> Everything except Evasive");
    Utils::WriteComment(entry_root, "1 -> Only Evasive");
    Utils::WriteComment(entry_root, "B: Input type");
    Utils::WriteComment(entry_root, "0 -> None");
    Utils::WriteComment(entry_root, "2 -> Reactivation (multiple activation skills)");
    Utils::WriteComment(entry_root, "3 -> Initial activation");*/
    Utils::WriteParamUnsigned(entry_root, "INPUT_ACTIVATOR_CONDITIONS", input_activator_conditions, true);

    Utils::WriteParamUnsigned(entry_root, "BAC_ENTRY_TO_ACTIVATE", bac_entry_to_activate, bac_entry_to_activate==0xFFFF);

    //Utils::WriteComment(entry_root, "BAC Entry ID of the final part of a ChainAttackParameters charging chain");
    Utils::WriteParamUnsigned(entry_root, "BAC_ENTRY_FINAL_PART", bac_entry_final_part, bac_entry_final_part==0xFFFF);

    Utils::WriteParamUnsigned(entry_root, "U_24", unk_24, true);

    //Utils::WriteComment(entry_root, "BAC Entry ID for the user to connect to post-grab");
    Utils::WriteParamUnsigned(entry_root, "BAC_ENTRY_USER_CONNECT", bac_entry_user_connect, bac_entry_user_connect==0xFFFF);

    //Utils::WriteComment(entry_root, "BAC Entry ID for the victim of a grab to connect to");
    Utils::WriteParamUnsigned(entry_root, "BAC_ENTRY_VICTIM_CONNECT", bac_entry_victim_connect, bac_entry_victim_connect==0xFFFF);

    Utils::WriteParamUnsigned(entry_root, "BAC_ENTRY_UNKNOWN", bac_entry_unknown, bac_entry_unknown==0xFFFF);
    Utils::WriteParamUnsigned(entry_root, "U_2C", unk_2C, true);
    Utils::WriteParamUnsigned(entry_root, "U_2E", unk_2E, true);

    Utils::WriteParamUnsigned(entry_root, "KI_COST", ki_cost);
    Utils::WriteParamUnsigned(entry_root, "U_44", unk_44, true);
    Utils::WriteParamUnsigned(entry_root, "U_48", unk_48, true);
    Utils::WriteParamUnsigned(entry_root, "RECEIVER_LINK_ID", receiver_link_id, true);
    Utils::WriteParamUnsigned(entry_root, "U_50", unk_50, true);
    Utils::WriteParamUnsigned(entry_root, "STAMINA_COST", stamina_cost);
    Utils::WriteParamUnsigned(entry_root, "U_58", unk_58, true);
    Utils::WriteParamUnsigned(entry_root, "KI_REQUIRED", ki_required);
    Utils::WriteParamFloat(entry_root, "HEALTH_REQUIRED", health_required);
    Utils::WriteParamUnsigned(entry_root, "TRANS_MODIFIER", trans_modifier);

    //Utils::WriteComment(entry_root, "For transformation skills, if this matches the \"cus aura\" of current transformation stage, it will make some techniques use teleport");
    Utils::WriteParamUnsigned(entry_root, "CUS_AURA", cus_aura, true);

    Utils::WriteParamUnsigned(entry_root, "U_68", unk_68, true);
    Utils::WriteParamUnsigned(entry_root, "U_6C", unk_6C, true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BcmEntry::Compile(const TiXmlElement *root)
{
    if (Utils::ReadAttrUnsigned(root, "sibling", &sibling))
    {
        sibling = sizeof(BCMHeader) + sibling * sizeof(BCMEntry);
    }
    else
    {
        sibling = 0;
    }

    if (Utils::ReadAttrUnsigned(root, "child", &child))
    {
        child = sizeof(BCMHeader) + child * sizeof(BCMEntry);
    }
    else
    {
        child = 0;
    }

    if (Utils::ReadAttrUnsigned(root, "parent", &parent))
    {
        parent = sizeof(BCMHeader) + parent * sizeof(BCMEntry);
    }
    else
    {
        parent = 0;
    }

    if (Utils::ReadAttrUnsigned(root, "root", &this->root))
    {
        this->root = sizeof(BCMHeader) + this->root * sizeof(BCMEntry);
    }
    else
    {
        this->root = 0;
    }

    if (!Utils::GetParamUnsigned(root, "U_00", &unk_00)) return false;
    if (!Utils::GetParamUnsigned(root, "DIRECTIONAL_CONDITIONS", &directional_conditions)) return false;
    if (!Utils::GetParamUnsigned(root, "U_08", &unk_08)) return false;
    if (!Utils::GetParamUnsigned(root, "U_0A", &unk_0A)) return false;
    if (!Utils::GetParamUnsigned(root, "HOLD_DOWN_CONDITIONS", &hold_down_conditions)) return false;
    if (!Utils::GetParamUnsigned(root, "OPPONENT_SIZE_CONDITIONS", &opponent_size_conditions)) return false;
    if (!Utils::GetParamUnsigned(root, "U_14", &unk_14)) return false;
    if (!Utils::GetParamUnsigned(root, "PRIMARY_ACTIVATOR_CONDITIONS", &primary_activator_conditions)) return false;
    if (!Utils::GetParamUnsigned(root, "INPUT_ACTIVATOR_CONDITIONS", &input_activator_conditions)) return false;
    if (!Utils::GetParamUnsigned(root, "BAC_ENTRY_TO_ACTIVATE", &bac_entry_to_activate)) return false;
    if (!Utils::GetParamUnsigned(root, "BAC_ENTRY_FINAL_PART", &bac_entry_final_part)) return false;
    if (!Utils::GetParamUnsigned(root, "U_24", &unk_24)) return false;
    if (!Utils::GetParamUnsigned(root, "BAC_ENTRY_USER_CONNECT", &bac_entry_user_connect)) return false;
    if (!Utils::GetParamUnsigned(root, "BAC_ENTRY_VICTIM_CONNECT", &bac_entry_victim_connect)) return false;
    if (!Utils::GetParamUnsigned(root, "BAC_ENTRY_UNKNOWN", &bac_entry_unknown)) return false;
    if (!Utils::GetParamUnsigned(root, "U_2C", &unk_2C)) return false;
    if (!Utils::GetParamUnsigned(root, "U_2E", &unk_2E)) return false;    
    if (!Utils::GetParamUnsigned(root, "KI_COST", &ki_cost)) return false;
    if (!Utils::GetParamUnsigned(root, "U_44", &unk_44)) return false;
    if (!Utils::GetParamUnsigned(root, "U_48", &unk_48)) return false;
    if (!Utils::GetParamUnsigned(root, "RECEIVER_LINK_ID", &receiver_link_id)) return false;
    if (!Utils::GetParamUnsigned(root, "U_50", &unk_50)) return false;
    if (!Utils::GetParamUnsigned(root, "STAMINA_COST", &stamina_cost)) return false;
    if (!Utils::GetParamUnsigned(root, "U_58", &unk_58)) return false;
    if (!Utils::GetParamUnsigned(root, "KI_REQUIRED", &ki_required)) return false;
    if (!Utils::GetParamFloat(root, "HEALTH_REQUIRED", &health_required)) return false;
    if (!Utils::GetParamUnsigned(root, "TRANS_MODIFIER", &trans_modifier)) return false;
    if (!Utils::GetParamUnsigned(root, "CUS_AURA", &cus_aura)) return false;
    if (!Utils::GetParamUnsigned(root, "U_68", &unk_68)) return false;
    if (!Utils::GetParamUnsigned(root, "U_6C", &unk_6C)) return false;

    return true;
}

BcmFile::BcmFile()
{
    this->big_endian = false;
}

BcmFile::~BcmFile()
{

}

void BcmFile::Reset()
{
    entries.clear();
}

bool BcmFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(BCMHeader))
        return false;

    const BCMHeader *hdr = (const BCMHeader *)buf;
    if (hdr->signature != BCM_SIGNATURE)
        return false;

    const BCMEntry *file_entries = (const BCMEntry *)(buf+hdr->data_start);
    entries.resize(hdr->num_entries);

    for (size_t i = 0; i < entries.size(); i++)
    {
        BcmEntry &entry = entries[i];

        COPY_IN(unk_00);
        COPY_IN(directional_conditions);
        COPY_IN(unk_08);
        COPY_IN(unk_0A);
        COPY_IN(hold_down_conditions);
        COPY_IN(opponent_size_conditions);
        COPY_IN(unk_14);
        COPY_IN(primary_activator_conditions);
        COPY_IN(input_activator_conditions);
        COPY_IN(bac_entry_to_activate);
        COPY_IN(bac_entry_final_part);
        COPY_IN(unk_24);
        COPY_IN(bac_entry_user_connect);
        COPY_IN(bac_entry_victim_connect);
        COPY_IN(bac_entry_unknown);
        COPY_IN(unk_2C);
        COPY_IN(unk_2E);
        COPY_IN(sibling);
        COPY_IN(child);
        COPY_IN(parent);
        COPY_IN(root);
        COPY_IN(ki_cost);
        COPY_IN(unk_44);
        COPY_IN(unk_48);
        COPY_IN(receiver_link_id);
        COPY_IN(unk_50);
        COPY_IN(stamina_cost);
        COPY_IN(unk_58);
        COPY_IN(ki_required);
        COPY_IN(health_required);
        COPY_IN(trans_modifier);
        COPY_IN(cus_aura);
        COPY_IN(unk_68);
        COPY_IN(unk_6C);
    }

    return true;
}

uint8_t *BcmFile::Save(size_t *psize)
{
    size_t size = sizeof(BCMHeader) + entries.size()*sizeof(BCMEntry);
    uint8_t *buf = new uint8_t[size];
    memset(buf, 0, size);

    BCMHeader *hdr = (BCMHeader *)buf;
    hdr->signature = BCM_SIGNATURE;
    hdr->endianess_check = 0xFFFE;
    hdr->num_entries = (uint32_t)entries.size();
    hdr->data_start = sizeof(BCMHeader);

    BCMEntry *file_entries = (BCMEntry *)(hdr+1);

    for (size_t i = 0; i < entries.size(); i++)
    {
        const BcmEntry &entry = entries[i];

        COPY_OUT(unk_00);
        COPY_OUT(directional_conditions);
        COPY_OUT(unk_08);
        COPY_OUT(unk_0A);
        COPY_OUT(hold_down_conditions);
        COPY_OUT(opponent_size_conditions);
        COPY_OUT(unk_14);
        COPY_OUT(primary_activator_conditions);
        COPY_OUT(input_activator_conditions);
        COPY_OUT(bac_entry_to_activate);
        COPY_OUT(bac_entry_final_part);
        COPY_OUT(unk_24);
        COPY_OUT(bac_entry_user_connect);
        COPY_OUT(bac_entry_victim_connect);
        COPY_OUT(bac_entry_unknown);
        COPY_OUT(unk_2C);
        COPY_OUT(unk_2E);
        COPY_OUT(sibling);
        COPY_OUT(child);
        COPY_OUT(parent);
        COPY_OUT(root);
        COPY_OUT(ki_cost);
        COPY_OUT(unk_44);
        COPY_OUT(unk_48);
        COPY_OUT(receiver_link_id);
        COPY_OUT(unk_50);
        COPY_OUT(stamina_cost);
        COPY_OUT(unk_58);
        COPY_OUT(ki_required);
        COPY_OUT(health_required);
        COPY_OUT(trans_modifier);
        COPY_OUT(cus_aura);
        COPY_OUT(unk_68);
        COPY_OUT(unk_6C);
    }

    *psize = size;
    return buf;
}

TiXmlDocument *BcmFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("BCM");

    for (size_t i = 0; i < entries.size(); i++)
    {
        entries[i].Decompile(root, (uint32_t)i);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool BcmFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "BCM");

    if (!root)
    {
        DPRINTF("Cannot find\"BCM\" in xml.\n");
        return false;
    }

    entries.resize(Utils::GetElemCount(root, "BcmEntry"));
    if (entries.size() == 0)
        return true;

    std::vector<bool> processed;
    processed.resize(entries.size(), false);

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "BcmEntry")
        {
            uint32_t idx;
            if (!Utils::ReadAttrUnsigned(elem, "idx", &idx))
            {
                DPRINTF("%s: Attribute \"idx\" is not optional.\n", FUNCNAME);
                return false;
            }

            if (idx >= entries.size())
            {
                DPRINTF("%s: idx = %d is out of bounds\n", FUNCNAME, idx);
                return false;
            }

            if (processed[idx])
            {
                DPRINTF("%s: idx=%d is duplicated.\n", FUNCNAME, idx);
                return false;
            }

            BcmEntry &entry = entries[idx];

            if (!entry.Compile(elem))
                return false;

            processed[idx] = true;
        }
    }

    return true;
}


