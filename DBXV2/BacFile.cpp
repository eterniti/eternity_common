#include "BacFile.h"
#include "debug.h"

TiXmlElement *BACMatrix3x3::Decompile(TiXmlNode *root, const std::string &comment) const
{
    TiXmlElement *entry_root = new TiXmlElement("Matrix3x3");

    if (comment.length() > 0)
        Utils::WriteComment(entry_root, comment);

    for (int i = 0; i < 3; i++)
    {
        const std::string name = "L" + Utils::ToString(i);
        Utils::WriteParamMultipleFloats(entry_root, name.c_str(), std::vector<float>(floats+i*3, floats+i*3+3))  ;
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACMatrix3x3::Compile(const TiXmlElement *root)
{
    for (int i = 0; i < 3; i++)
    {
        const std::string name = "L" + Utils::ToString(i);

        if (!Utils::GetParamMultipleFloats(root, name.c_str(), floats+i*3, 3))
            return false;
    }

    return true;
}

TiXmlElement *BACAnimation::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Animation");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);

    Utils::WriteComment(entry_root, "0 -> CMN.ean; 0xfffe -> skill ean; 0x5: char ean");
    Utils::WriteComment(entry_root, "Other possible values: 0x9 -> unknown, frequently appears in victim throw entries; 0xa -> fce.ean?; "
                        "0xb -> fce.lips.ean");
    Utils::WriteParamUnsigned(entry_root, "EAN_TO_USE", ean_to_use, true);
    Utils::WriteParamUnsigned(entry_root, "EAN_INDEX", ean_index);

    Utils::WriteParamUnsigned(entry_root, "U_0C", unk_0C, true);    
    Utils::WriteParamUnsigned(entry_root, "U_0E", unk_0E, true);
    Utils::WriteParamUnsigned(entry_root, "FRAME_START", frame_start);
    Utils::WriteParamUnsigned(entry_root, "FRAME_END", frame_end, (frame_end==0xFFFF));

    //Utils::WriteComment(entry_root, "0xffff: no loop");
    Utils::WriteParamUnsigned(entry_root, "FRAME_LOOP_START", frame_loop_start, (frame_loop_start==0xFFFF));

    Utils::WriteParamUnsigned(entry_root, "U_16", unk_16, true);
    Utils::WriteParamFloat(entry_root, "SPEED", speed);
    Utils::WriteParamFloat(entry_root, "TRANSITORY_ANIMATION_CONNECTION_TYPE", transitory_animation_connection_type);
    Utils::WriteParamFloat(entry_root, "TRANSITORY_ANIMATION_COMPRESSION", transitory_animation_compression);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACAnimation::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "EAN_TO_USE", &ean_to_use))
        return false;

    if (!Utils::GetParamUnsigned(root, "EAN_INDEX", &ean_index))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0C", &unk_0C))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0E", &unk_0E))
        return false;

    if (!Utils::GetParamUnsigned(root, "FRAME_START", &frame_start))
        return false;

    if (!Utils::GetParamUnsigned(root, "FRAME_END", &frame_end))
        return false;

    if (!Utils::GetParamUnsigned(root, "FRAME_LOOP_START", &frame_loop_start))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_16", &unk_16))
        return false;

    if (!Utils::GetParamFloat(root, "SPEED", &speed))
        return false;

    if (!Utils::GetParamFloat(root, "TRANSITORY_ANIMATION_CONNECTION_TYPE", &transitory_animation_connection_type))
        return false;

    if (!Utils::GetParamFloat(root, "TRANSITORY_ANIMATION_COMPRESSION", &transitory_animation_compression))
        return false;

    return true;
}

TiXmlElement *BACHitbox::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Hitbox");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);

    /*Utils::WriteComment(entry_root, "Known values for CMN.bdm:  0xd -> Light hit with no delay or stun (Cell’s rapid kicks, etcetera)");
    Utils::WriteComment(entry_root, "0x2b -> Light stamina break; 0x2c -> Stamina-broken light stamina break");
    Utils::WriteComment(entry_root, "0x2d -> Heavy stamina break; 0x2e -> Stamina-broken heavy stamina break");
    Utils::WriteComment(entry_root, "0x4e -> Stamina-broken heavy stamina break; 0x6e -> Standard diagonal knockdown");
    Utils::WriteComment(entry_root, "0x8e -> Guard break; 0xa0 -> Grab A");
    Utils::WriteComment(entry_root, "0xa2 -> Grab B (has impact effect); 0xa4 -> Grab C (pierces guards)");*/
    Utils::WriteParamUnsigned(entry_root, "BDM_ENTRY", bdm_entry);

    /*Utils::WriteComment(entry_root, "Known values (0xABCD)");
    Utils::WriteComment(entry_root, "A: 0 -> (Default) None; 2 -> The hitbox will only register a single impact.");
    Utils::WriteComment(entry_root, "C (Hitbox spawn source): 0 -> (Default) User; 8 -> Target");*/
    Utils::WriteParamUnsigned(entry_root, "HITBOX_FLAGS", hitbox_flags, true);

    /*Utils::WriteComment(entry_root, "This value is only 1/10th of the in-game damage, before all multipliers are even applied.");
    Utils::WriteComment(entry_root, "This damage is added on top of BDM damage but is not used when an opponent is guarding, even for piercing attacks.");*/
    Utils::WriteParamUnsigned(entry_root, "DAMAGE", damage);
    Utils::WriteParamUnsigned(entry_root, "DAMAGE_WHEN_BLOCKED", damage_when_blocked);

    /*Utils::WriteComment(entry_root, "Amount of stamina taken from victim when blocked.");
    Utils::WriteComment(entry_root, "Any usage of BDM Damage Type 1 is considered “blocking”. This is how God of Destruction’s Roar works.");*/
    Utils::WriteParamUnsigned(entry_root, "STAMINA_TAKEN_WHEN_BLOCKED", stamina_taken_when_blocked);

    Utils::WriteParamUnsigned(entry_root, "U_12", unk_12, true);
    /*Utils::WriteComment(entry_root, "Known flags 0xAB");
    Utils::WriteComment(entry_root, "A: 0 -> 0x0: CMN.bdm; 1 -> Moveset’s BDM; 2 -> Skill’s BDM");*/    
    Utils::WriteParamUnsigned(entry_root, "BDM_FLAGS", bdm_flags, true);
    Utils::WriteParamMultipleUnsigned(entry_root, "U_14", std::vector<uint32_t>(unk_14, unk_14+2), true);    

    //matrix.Decompile(entry_root, "Position+rotation+scale");
    matrix.Decompile(entry_root, "");


    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACHitbox::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "BDM_ENTRY", &bdm_entry))
        return false;

    if (!Utils::GetParamUnsigned(root, "HITBOX_FLAGS", &hitbox_flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "DAMAGE", &damage))
        return false;

    if (!Utils::GetParamUnsigned(root, "DAMAGE_WHEN_BLOCKED", &damage_when_blocked))
        return false;

    if (!Utils::GetParamUnsigned(root, "STAMINA_TAKEN_WHEN_BLOCKED", &stamina_taken_when_blocked))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_12", &unk_12))
        return false;

    if (!Utils::GetParamUnsigned(root, "BDM_FLAGS", &bdm_flags))
        return false;

    if (!Utils::GetParamMultipleUnsigned(root, "U_14", unk_14, 2))
        return false;

    const TiXmlElement *matrix_entry;
    if (Utils::GetElemCount(root, "Matrix3x3", &matrix_entry) == 0)
        return false;

    if (!matrix.Compile(matrix_entry))
        return false;

    return true;
}

TiXmlElement *BACAccelerationMovement::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("AccelerationMovement");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);

    /*Utils::WriteComment(entry_root, "Known flags 0xABCD");
    Utils::WriteComment(entry_root, "A: 0 -> None(default); 1 -> Automatic persistent movement in relative direction;");
    Utils::WriteComment(entry_root, "2 -> Automatic persistent  movement relative to opponent’s position.; ");
    Utils::WriteComment(entry_root, "4 -> Manual persistent “forwards/back/left/right” overrides z-axis, with automatic persistent movement in x/y axes. Affected by B flags.");
    Utils::WriteComment(entry_root, "8 -> Teleport to opponent’s position.");
    Utils::WriteComment(entry_root, "B: 0 -> default; 4 -> Movement only takes place when “up” is used. Triggers y-axis and disables x/z axes on applicable D flags.");
    Utils::WriteComment(entry_root, "8 -> Movement only takes place when “down” is used. Triggers y-axis and disables x/z axes on applicable D flags.");*/
    Utils::WriteParamUnsigned(entry_root, "MOVEMENT_FLAGS", movement_flags, true);

    Utils::WriteParamUnsigned(entry_root, "U_0A", unk_0A, true);
    Utils::WriteParamFloat(entry_root, "X_AXIS_MOVEMENT", x_axis_movement);
    Utils::WriteParamFloat(entry_root, "Y_AXIS_MOVEMENT", y_axis_movement);
    Utils::WriteParamFloat(entry_root, "Z_AXIS_MOVEMENT", z_axis_movement);
    Utils::WriteParamFloat(entry_root, "X_AXIS_DRAG", x_axis_drag);
    Utils::WriteParamFloat(entry_root, "Y_AXIS_DRAG", y_axis_drag);
    Utils::WriteParamFloat(entry_root, "Z_AXIS_DRAG", z_axis_drag);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACAccelerationMovement::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "MOVEMENT_FLAGS", &movement_flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0A", &unk_0A))
        return false;

    if (!Utils::GetParamFloat(root, "X_AXIS_MOVEMENT", &x_axis_movement))
        return false;

    if (!Utils::GetParamFloat(root, "Y_AXIS_MOVEMENT", &y_axis_movement))
        return false;

    if (!Utils::GetParamFloat(root, "Z_AXIS_MOVEMENT", &z_axis_movement))
        return false;

    if (!Utils::GetParamFloat(root, "X_AXIS_DRAG", &x_axis_drag))
        return false;

    if (!Utils::GetParamFloat(root, "Y_AXIS_DRAG", &y_axis_drag))
        return false;

    if (!Utils::GetParamFloat(root, "Z_AXIS_DRAG", &z_axis_drag))
        return false;

    return true;
}

TiXmlElement *BACInvulnerability::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Invulnerability");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);

    /*Utils::WriteComment(entry_root, "0x0 -> all attacks miss, complete invulnerability; 0x2 -> take damage, but stun-proof (super armor)");
    Utils::WriteComment(entry_root, "0x3, 0x4, 0x8 -> no damage, stun proof; 5 -> attacks do no damage but can take stamina");
    Utils::WriteComment(entry_root, "0x6, 0x7 -> all attacks are absorbed, complete invulnerability; ");
    Utils::WriteComment(entry_root, "0x9, 0xa, 0xb, 0xc, 0xf -> stun-proof, all attacks do the amount of damage that the first attack to land did, throws are exempt from this");
    Utils::WriteComment(entry_root, "0xd -> automatic guard; 0xe -> ki blasts fly through");*/
    Utils::WriteParamUnsigned(entry_root, "TYPE", type, true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACInvulnerability::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "TYPE", &type))
        return false;

    return true;
}

TiXmlElement *BACMotionAdjust::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("MotionAdjust");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);
    Utils::WriteParamFloat(entry_root, "TIME_SCALE", time_scale);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACMotionAdjust::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (!Utils::GetParamFloat(root, "TIME_SCALE", &time_scale))
        return false;

    return true;
}

TiXmlElement *BACOpponentKnockback::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("OpponentKnockback");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);

    Utils::WriteComment(entry_root, "Works as prcentage (0.0-1.0)");
    Utils::WriteParamFloat(entry_root, "TRACKING", tracking);

    Utils::WriteParamUnsigned(entry_root, "U_0C", unk_0C, true);
    Utils::WriteParamUnsigned(entry_root, "U_0E", unk_0E, true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACOpponentKnockback::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (!Utils::GetParamFloat(root, "TRACKING", &tracking))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0C", &unk_0C))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0E", &unk_0E))
        return false;

    return true;
}

TiXmlElement *BACChainAttackParameters::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("ChainAttackParameters");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);
    Utils::WriteParamUnsigned(entry_root, "U_08", unk_08, true);

    //Utils::WriteComment(entry_root, "The entire BAC Entry covered by DURATION will be time-dilated to match the chain time.");
    Utils::WriteParamUnsigned(entry_root, "TOTAL_CHAIN_TIME", total_chain_time);

    Utils::WriteParamUnsigned(entry_root, "U_0E", unk_0E, true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACChainAttackParameters::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_08", &unk_08))
        return false;

    if (!Utils::GetParamUnsigned(root, "TOTAL_CHAIN_TIME", &total_chain_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0E", &unk_0E))
        return false;

    return true;
}

TiXmlElement *BACBcmCallback::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("BcmCallback");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);

    /*Utils::WriteComment(entry_root, "Known values (0xABCD");
    Utils::WriteComment(entry_root, "3 -> 1+2; 5 -> 4+1; 6 -> 4+2; 7 -> 4+2+1; 9 -> 8+1;");
    Utils::WriteComment(entry_root, "0xa -> 8+2; 0xb -> 8+2+1; 0xc -> 8+4; 0xd -> 8+4+1; 0xe -> 8+4+2; 0xf -> 8+4+2+1");
    Utils::WriteComment(entry_root, "A: 0 -> no links; 1 -> links to ki blasts; 2 -> links to jump; 4 -> links to guard; 8 -> links to step dash and fly");
    Utils::WriteComment(entry_root, "B (Connects to BCM Entries with RECEIVER_LINK_ID of the same value): 0 -> no links; 1 -> links to combo; 2 -> links to supers; 4 -> links to ultimates; 8 -> links to z-vanish");
    Utils::WriteComment(entry_root, "C: 0 -> no links; 8/0xf -> links to back-hit");
    Utils::WriteComment(entry_root, "D: 0 -> no links (frequent in movesets); 0x2, 0x4 -> unknown (frequent in skills)");*/
    Utils::WriteParamUnsigned(entry_root, "BCM_LINK_FLAGS", bcm_link_flags, true);

    Utils::WriteParamUnsigned(entry_root, "U_0A", unk_0A, true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACBcmCallback::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "BCM_LINK_FLAGS", &bcm_link_flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0A", &unk_0A))
        return false;

    return true;
}

TiXmlElement *BACEffect::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Effect");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);    

    /*Utils::WriteComment(entry_root, "Known values: 0x0 -> Global EEPK’s; 0x1 -> Dust effects; 0x2 -> Player’s EEPK");
    Utils::WriteComment(entry_root, "0x3 -> Awoken EEPK’s; 0x5 -> Super EEPK’s; 0x6 -> Ultimate EEPK’s");
    Utils::WriteComment(entry_root, "0x7 -> Evasive EEPK’s; 0x9 -> Ki Blast EEPK’s");*/
    Utils::WriteParamUnsigned(entry_root, "TYPE", type, true);

    /*Utils::WriteComment(entry_root, "0x0 -> base; 0x1 -> pelvis; 0x2 -> head; 0x7 -> right hand; 0x8 -> left hand");
    Utils::WriteComment(entry_root, "0x13 -> left foot; 0x14 -> left wrist; 0x15 -> right foot; 0x16 -> right wrist");*/
    Utils::WriteParamUnsigned(entry_root, "BONE_LINK", bone_link, true);

    bool hex = (skill_id == 0xFFFF || skill_id == 0xBACA || skill_id == 0xCACA);
    //Utils::WriteComment(entry_root, "For Global EEPK’s: 0 -> CMN.eepk (misc effects); 1 -> BTL_AURA.eepk; 2 -> BTL_KDN.eepk");
    Utils::WriteParamUnsigned(entry_root, "SKILL_ID", skill_id, hex);

    Utils::WriteComment(entry_root, "0: true; 0xffff: false");
    Utils::WriteParamUnsigned(entry_root, "USE_SKILL_ID", use_skill_id, true);
    Utils::WriteParamUnsigned(entry_root, "EFFECT_ID", effect_id);

    Utils::WriteParamMultipleUnsigned(entry_root, "U_14", std::vector<uint32_t>(unk_14, unk_14+6), true);

    Utils::WriteComment(entry_root, "0: on; 1: off");
    Utils::WriteParamUnsigned(entry_root, "ON_OFF_SWITCH", on_off_switch);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACEffect::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "TYPE", &type))
        return false;

    if (!Utils::GetParamUnsigned(root, "BONE_LINK", &bone_link))
        return false;

    if (!Utils::GetParamUnsigned(root, "SKILL_ID", &skill_id))
        return false;

    if (!Utils::GetParamUnsigned(root, "USE_SKILL_ID", &use_skill_id))
        return false;

    if (!Utils::GetParamUnsigned(root, "EFFECT_ID", &effect_id))
        return false;

    if (!Utils::GetParamMultipleUnsigned(root, "U_14", unk_14, 6))
        return false;

    if (!Utils::GetParamUnsigned(root, "ON_OFF_SWITCH", &on_off_switch))
        return false;

    return true;
}

TiXmlElement *BACProjectile::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Projectile");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);

    bool hex = (skill_id == 0xFFFF || skill_id == 0xBACA || skill_id == 0xCACA);
    Utils::WriteParamUnsigned(entry_root, "SKILL_ID", skill_id, hex);

    Utils::WriteComment(entry_root, "0x0: Yes; 0xffff: No");;
    Utils::WriteParamUnsigned(entry_root, "CAN_USE_CMN_BSA", can_use_cmn_bsa, true);

    Utils::WriteParamUnsigned(entry_root, "PROJECTILE_ID", projectile_id);

    /*Utils::WriteComment(entry_root, "0x0 -> base; 0x7 -> right hand; 0x8 -> left hand");
    Utils::WriteComment(entry_root, "0x13 -> left foot; 0x14 -> left wrist; 0x15 -> right foot; 0x16 -> right wrist");*/
    Utils::WriteParamUnsigned(entry_root, "BONE_TO_SPAWN_FROM", bone_to_spawn_from, true);

    /*Utils::WriteComment(entry_root, "Known values 0xABC");
    Utils::WriteComment(entry_root, "C: 0 -> User; 1 -> Target; 5 -> Target; 7 -> Map");*/
    Utils::WriteParamUnsigned(entry_root, "SPAWN_SOURCE", spawn_source, true);

    //Utils::WriteComment(entry_root, "Positioning of projectile spawn-point relative to its source.");
    Utils::WriteParamMultipleFloats(entry_root, "POSITION", std::vector<float>(position, position+3));

    /*Utils::WriteComment(entry_root, " Known values: 0x0 -> CMN; 0x3 -> Awoken");
    Utils::WriteComment(entry_root, "0x5 -> Super; 0x6 -> Ultimate; 0x7 -> Evasive; 0x8 -> Blast");*/
    Utils::WriteParamUnsigned(entry_root, "SKILL_TYPE", skill_type, true);

    Utils::WriteParamUnsigned(entry_root, "U_24", unk_24, true);
    Utils::WriteParamFloat(entry_root, "F_28", unk_28);
    Utils::WriteParamUnsigned(entry_root, "U_2C", unk_2C, true);
    Utils::WriteParamUnsigned(entry_root, "U_2E", unk_2E, true);

    //Utils::WriteComment(entry_root, "Used in Gigantic Ki Blast to determine how much damage the projectile can take before being knocked back.");
    Utils::WriteParamUnsigned(entry_root, "PROJECTILE_HEALTH", projectile_health);

    Utils::WriteParamMultipleUnsigned(entry_root, "U_34", std::vector<uint32_t>(unk_34, unk_34+3), true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACProjectile::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "SKILL_ID", &skill_id))
        return false;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "CAN_USE_CMN_BSA", &can_use_cmn_bsa))
        return false;

    if (!Utils::GetParamUnsigned(root, "PROJECTILE_ID", &projectile_id))
        return false;

    if (!Utils::GetParamUnsigned(root, "BONE_TO_SPAWN_FROM", &bone_to_spawn_from))
        return false;

    if (!Utils::GetParamUnsigned(root, "SPAWN_SOURCE", &spawn_source))
        return false;

    if (!Utils::GetParamMultipleFloats(root, "POSITION", position, 3))
        return false;

    if (!Utils::GetParamUnsigned(root, "SKILL_TYPE", &skill_type))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_24", &unk_24))
        return false;

    if (!Utils::GetParamFloat(root, "F_28", &unk_28))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_2C", &unk_2C))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_2E", &unk_2E))
        return false;

    if (!Utils::GetParamUnsigned(root, "PROJECTILE_HEALTH", &projectile_health))
        return false;

    if (!Utils::GetParamMultipleUnsigned(root, "U_34", unk_34, 3))
        return false;

    return true;
}

TiXmlElement *BACCamera::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Camera");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);

    /*Utils::WriteComment(entry_root, "0x0 -> Basic Lock Camera; 0x1 -> Heavy Rumble (Linked to BDM value, likely same effect)");
    Utils::WriteComment(entry_root, "0x2 -> Extreme Rumble (Linked to BDM value, likely same effect); 0x3 -> CMN.cam.ean");
    Utils::WriteComment(entry_root, "0x4 -> Character’s cam.ean; 0x5 -> skill ean");
    Utils::WriteComment(entry_root, "0x7 -> Static Camera (Linked to BDM value, likely same effect)");
    Utils::WriteComment(entry_root, "0x8 -> Camera Focuses on Victim Temporarily (Linked to BDM value, likely same effect)");
    Utils::WriteComment(entry_root, "0xa -> Camera zooms into player, Generates speed-line effects (Likely linked to heavy hit effect camera)");
    Utils::WriteComment(entry_root, "0xb -> Generic Cinematic Boosting Camera");
    Utils::WriteComment(entry_root, "0xc -> same as 0xb; 0xe -> same as 0x1; 0xf -> same as 2; 0x11 -> Zooms into Player");*/
    Utils::WriteParamUnsigned(entry_root, "EAN_TO_USE", ean_to_use, true);

    /*Utils::WriteComment(entry_root, "0x0 -> base; 0x7 -> right hand; 0x8 -> left hand");
    Utils::WriteComment(entry_root, "0x13 -> left foot; 0x14 -> left wrist; 0x15 -> right foot; 0x16 -> right wrist");*/
    Utils::WriteParamUnsigned(entry_root, "BONE_TO_FOCUS_ON", bone_to_focus_on, true);

    Utils::WriteParamUnsigned(entry_root, "EAN_INDEX", ean_index);
    Utils::WriteParamUnsigned(entry_root, "CAMERA_FRAME_START", camera_frame_start);
    Utils::WriteParamUnsigned(entry_root, "U_10", unk_10, true);
    Utils::WriteParamUnsigned(entry_root, "U_12", unk_12, true);

    //Utils::WriteComment(entry_root, "Z-axis camera position, relative to source. Positive values backwards, negative values forwards");
    Utils::WriteParamFloat(entry_root, "Z_AXIS_CAMERA_POSITION", z_axis_camera_position);

    //Utils::WriteComment(entry_root, "Camera displacement over a circle across the X/Z axis, in degrees");
    Utils::WriteParamFloat(entry_root, "CAMERA_DISP_XZ", camera_disp_xz);

    //Utils::WriteComment(entry_root, "Camera displacement over a circle across the Y/Z axis, in degrees");
    Utils::WriteParamFloat(entry_root, "CAMERA_DISP_YZ", camera_disp_yz);

    //Utils::WriteComment(entry_root, "Camera Y-axis rotation, in degrees. Camera remains in same place. Positive values anti-clockwise, negative values clockwise");
    Utils::WriteParamFloat(entry_root, "CAMERA_Y_ROT", camera_y_rot);

    //Utils::WriteComment(entry_root, "Camera X-axis rotation, in degrees. Camera remains in same place. Positive values anti-clockwise, negative values clockwise");
    Utils::WriteParamFloat(entry_root, "CAMERA_X_ROT", camera_x_rot);

    //Utils::WriteComment(entry_root, "X-axis camera position, relative to source. Positive values left, negative values right");
    Utils::WriteParamFloat(entry_root, "CAMERA_X_POS", camera_x_pos);

    //Utils::WriteComment(entry_root, "Y-axis camera position, relative to source. Positive values left, negative values right");
    Utils::WriteParamFloat(entry_root, "CAMERA_Y_POS", camera_y_pos);

    //Utils::WriteComment(entry_root, "Camera zoom/FOV. Positive values zoom outwards, negative values zoom inwards");
    Utils::WriteParamFloat(entry_root, "CAMERA_ZOOM", camera_zoom);

    //Utils::WriteComment(entry_root, "Camera Z-axis rotation, in degrees. Camera remains in same place. Positive values anti-clockwise, negative values clockwise");
    Utils::WriteParamFloat(entry_root, "CAMERA_Z_ROT", camera_z_rot);

    Utils::WriteParamMultipleUnsigned(entry_root, "U_38", std::vector<uint32_t>(unk_38, unk_38+4), true);
    Utils::WriteParamUnsigned(entry_root, "U_48", unk_48, true);

    /*Utils::WriteComment(entry_root, "Known values 0xAB");
    Utils::WriteComment(entry_root, "A: Position/rotation/zoom enabler. 0 -> (Default) position/rotation/zoom disabled. 0x8-> Position/rotation/zoom enabled");
    Utils::WriteComment(entry_root, "B: Unknown. Clearly tied to skill EANs. Something to do with ESK data? 0 -> CACs. 8 -> Roster Characters");*/
    Utils::WriteParamUnsigned(entry_root, "CAMERA_FLAGS", camera_flags, true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACCamera::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "EAN_TO_USE", &ean_to_use))
        return false;

    if (!Utils::GetParamUnsigned(root, "BONE_TO_FOCUS_ON", &bone_to_focus_on))
        return false;

    if (!Utils::GetParamUnsigned(root, "EAN_INDEX", &ean_index))
        return false;

    if (!Utils::GetParamUnsigned(root, "CAMERA_FRAME_START", &camera_frame_start))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_10", &unk_10))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_12", &unk_12))
        return false;

    if (!Utils::GetParamFloat(root, "Z_AXIS_CAMERA_POSITION", &z_axis_camera_position))
        return false;

    if (!Utils::GetParamFloat(root, "CAMERA_DISP_XZ", &camera_disp_xz))
        return false;

    if (!Utils::GetParamFloat(root, "CAMERA_DISP_YZ", &camera_disp_yz))
        return false;

    if (!Utils::GetParamFloat(root, "CAMERA_Y_ROT", &camera_y_rot))
        return false;

    if (!Utils::GetParamFloat(root, "CAMERA_X_ROT", &camera_x_rot))
        return false;

    if (!Utils::GetParamFloat(root, "CAMERA_X_POS", &camera_x_pos))
        return false;

    if (!Utils::GetParamFloat(root, "CAMERA_Y_POS", &camera_y_pos))
        return false;

    if (!Utils::GetParamFloat(root, "CAMERA_ZOOM", &camera_zoom))
        return false;

    if (!Utils::GetParamFloat(root, "CAMERA_Z_ROT", &camera_z_rot))
        return false;

    if (!Utils::GetParamMultipleUnsigned(root, "U_38", unk_38, 4))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_48", &unk_48))
        return false;

    if (!Utils::GetParamUnsigned(root, "CAMERA_FLAGS", &camera_flags))
        return false;

    return true;
}

TiXmlElement *BACSound::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Sound");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);

    Utils::WriteComment(entry_root, "0x0 -> SE/Battle/Common/CAR_BTL_CMN; 0x2 -> char SE; 0x3 -> char VOX; 0xa -> skill SE; 0xb -> skill VOX");
    Utils::WriteParamUnsigned(entry_root, "ACB_TO_USE", acb_to_use, true);

    Utils::WriteParamUnsigned(entry_root, "U_0A", unk_0A, true);
    Utils::WriteParamUnsigned(entry_root, "CUE_ID", cue_id);
    Utils::WriteParamUnsigned(entry_root, "U_0E", unk_0E, true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACSound::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "ACB_TO_USE", &acb_to_use))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0A", &unk_0A))
        return false;

    if (!Utils::GetParamUnsigned(root, "CUE_ID", &cue_id))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0E", &unk_0E))
        return false;

    return true;
}

TiXmlElement *BACType12::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("BACType12");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);
    Utils::WriteParamUnsigned(entry_root, "U_08", unk_08, true);
    Utils::WriteParamUnsigned(entry_root, "U_0A", unk_0A, true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACType12::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_08", &unk_08))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0A", &unk_0A))
        return false;

    return true;
}

TiXmlElement *BACPartInvisibility::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("PartInvisibility");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);
    Utils::WriteParamUnsigned(entry_root, "BCS_PART_ID", bcs_part_id, true);

    Utils::WriteComment(entry_root, "0: on; 1: off");
    Utils::WriteParamUnsigned(entry_root, "ON_OFF_SWITCH", on_off_switch, true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACPartInvisibility::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "BCS_PART_ID", &bcs_part_id))
        return false;

    if (!Utils::GetParamUnsigned(root, "ON_OFF_SWITCH", &on_off_switch))
        return false;

    return true;
}

TiXmlElement *BACAnimationModification::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("AnimationModification");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);

    /*Utils::WriteComment(entry_root, "Known values: 0x0 -> none;");
    Utils::WriteComment(entry_root, "0x1: head, horizontal and vertical, only when opponent is not behind");
    Utils::WriteComment(entry_root, "0x2 -> spine, horizontal, only when grounded, only when opponent is above");
    Utils::WriteComment(entry_root, "0x3 -> 0x3: spinal vertical rotation (only if you're grounded and the target is above you), head horizontal and vertical rotation (only if your opponent isn't both behind and to the right)");
    Utils::WriteComment(entry_root, "0x5 -> spine and head, horizontal and vertical (frequently used in stances)");
    Utils::WriteComment(entry_root, "0x6 -> spine, horizontal and vertical");
    Utils::WriteComment(entry_root, "0x1f -> head, horizontal and vertical");*/
    Utils::WriteParamUnsigned(entry_root, "MODIFICATION", modification, true);

    Utils::WriteParamUnsigned(entry_root, "U_0A", unk_0A, true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACAnimationModification::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "MODIFICATION", &modification))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0A", &unk_0A))
        return false;

    return true;
}

TiXmlElement *BACTransformControl::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("TransformControl");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);

    /*Utils::WriteComment(entry_root, "Known values:");
    Utils::WriteComment(entry_root, "0x0: BAC Entry looping, while the attack is being held down. Relies on BCM support. "
                                    "PARAMETER: no function, use 0.0");
    Utils::WriteComment(entry_root, "0x2: Damages user over the DURATION. "
                                    "PARAMETER: mount of damage. Positive values deal damage, negative values deal damage "
                                    "but it is not affected by buffs or defenses. This value is only 1/10th of the in-game damage, "
                                    "before all multipliers are even applied");
    Utils::WriteComment(entry_root, "0x4: Gives/takes Ki over the DURATION. "
                                    "PARAMETER: Ki amount. Positive values drain Ki, negative values give");
    Utils::WriteComment(entry_root, "0x6: Invisibility over the DURATION. "
                                    "PARAMETER: no function, use 0.0");
    Utils::WriteComment(entry_root, "0x7: Rotate animation around the y-axis. "
                                    "PARAMETER: rotation in degrees.");
    Utils::WriteComment(entry_root, "0xc: Screen darkens for user. All in-game operations freeze, players’ screens darken and their cameras focus on user. "
                                    "PARAMETER: no function, use 0.0");
    Utils::WriteComment(entry_root, "0xd: Full transformation activator (requires PUP to have any noticeable effects). "
                                    "PARAMETER: no function, use 0.0");
    Utils::WriteComment(entry_root, "0xe: Transformation deactivator. "
                                    "PARAMETER: no function, use 0.0");
    Utils::WriteComment(entry_root, "0x10: Changes projectile ID (used in manual detonation of projectiles). "
                                    "PARAMETER: Projectile ID");
    Utils::WriteComment(entry_root, "0x11: Swaps bodies with the opponent. Only works in throw moves. "
                                    "PARAMETER: no function, use 0.0");
    Utils::WriteComment(entry_root, "0x12: Targets/untargets opponent "
                                    "PARAMETER: no function, use 0.0");
    Utils::WriteComment(entry_root, "0x13/0x14: Sets BCS PartSet. Does not work on CAC’s. "
                                    "PARAMETER: PartSet ID. -1.0 to deactivate");
    Utils::WriteComment(entry_root, "0x15: Removes the user from the game for the DURATION. "
                                    "PARAMETER: no function, use 0.0");
    Utils::WriteComment(entry_root, "0x16: Gives/takes Stamina over the DURATION. "
                                    "PARAMETER: Stamina amount. Positive values drain Stamina, negative values give");
    Utils::WriteComment(entry_root, "0x1b: Limited transformation activator (has a built-in time limit and does not have full PUP access). "
                                    "PARAMETER: no function, use 0.0");
    Utils::WriteComment(entry_root, "0x1d: Immediately passes on to specified BAC Entry. "
                                    "PARAMETER: BAC Entry ID");
    Utils::WriteComment(entry_root, "0x20: Disables movement and skill usage for the DURATION. "
                                    "PARAMETER: no function, use 0.0");
    Utils::WriteComment(entry_root, "0x22: Creates a BAC Loop across the frames covered by DURATION, similar to FRAME_LOOP_START in Animation. "
                                    "PARAMETER: no function, use 0.0");
    Utils::WriteComment(entry_root, "0x23: Creates floating pebbles over the DURATION. "
                                    "PARAMETER: no function, use 0.0");
    Utils::WriteComment(entry_root, "0x24: Knocks away floating pebbles. "
                                    "PARAMETER: no function, use 0.0");
    Utils::WriteComment(entry_root, "0x28: Makes all opponents invisible to the user for the DURATION. "
                                    "PARAMETER: no function, use 0.0");
    Utils::WriteComment(entry_root, "0x29: Makes user untargetable (but can still take damage) for the DURATION. "
                                    "PARAMETER: no function, use 0.0");
    Utils::WriteComment(entry_root, "0x2a: Sets bcs Body (bone scaling). Scaling is gradual over the DURATION. "
                                    "PARAMETER: The bcs Body id. -1 to deactivate");
    Utils::WriteComment(entry_root, "0x2d: No-clip mode, somewhat glitchy. "
                                    "PARAMETER: -1.0 to deactivate");
    Utils::WriteComment(entry_root, "0x2e: Creates a giant collision box for the user. "
                                    "PARAMETER: -1.0 to deactivate");
    Utils::WriteComment(entry_root, "0x30: Renders a black void instead of the environment (like Molotov), for the DURATION. "
                                    "PARAMETER: no function, use 0.0");
    Utils::WriteComment(entry_root, "0x31: Replenishes health over the DURATION. "
                                    "PARAMETER: Positive values for health gain");*/
    Utils::WriteParamUnsigned(entry_root, "TYPE", type, true);

    Utils::WriteParamUnsigned(entry_root, "U_0A", unk_0A, true);
    Utils::WriteParamFloat(entry_root, "PARAMETER", parameter);

    Utils::WriteParamMultipleFloats(entry_root, "F_10", std::vector<float>(unk_10, unk_10+2));
    Utils::WriteParamMultipleUnsigned(entry_root, "U_18", std::vector<uint32_t>(unk_18, unk_18+2), true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACTransformControl::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "TYPE", &type))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0A", &unk_0A))
        return false;

    if (!Utils::GetParamFloat(root, "PARAMETER", &parameter))
        return false;

    if (!Utils::GetParamMultipleFloats(root, "F_10", unk_10, 2))
        return false;

    if (!Utils::GetParamMultipleUnsigned(root, "U_18", unk_18, 2))
        return false;

    return true;
}

TiXmlElement *BACScreenEffect::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("ScreenEffect");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);
    Utils::WriteParamUnsigned(entry_root, "BPE_EFFECT_ID", bpe_effect_id);
    Utils::WriteParamUnsigned(entry_root, "U_0C", unk_0C, true);
    Utils::WriteParamUnsigned(entry_root, "U_0E", unk_0E, true);
    Utils::WriteParamMultipleUnsigned(entry_root, "U_10", std::vector<uint32_t>(unk_10, unk_10+4), true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACScreenEffect::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "BPE_EFFECT_ID", &bpe_effect_id))
        return false;    

    if (!Utils::GetParamUnsigned(root, "U_0C", &unk_0C))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0E", &unk_0E))
        return false;

    if (!Utils::GetParamMultipleUnsigned(root, "U_10", unk_10, 4))
        return false;

    return true;
}

TiXmlElement *BACThrowHandler::Decompile(TiXmlNode *root, bool _small) const
{
    TiXmlElement *entry_root = new TiXmlElement("ThrowHandler");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);

    /*Utils::WriteComment(entry_root, "Known flags 0xABC");
    Utils::WriteComment(entry_root, "A: Conditions under which next BAC Entry is activated (selected with BAC_ENTRY)");
    Utils::WriteComment(entry_root, "C: Direction orientation and connection");
    Utils::WriteComment(entry_root, "Known values for A:");
    Utils::WriteComment(entry_root, "0 -> None (default); 1 -> When ground reached or DURATION is finished");
    Utils::WriteComment(entry_root, "2 -> (Default) Only when DURATION is finished; 3 -> When ground or wall is reached, or DURATION is finished");
    Utils::WriteComment(entry_root, "Known values for C:");
    Utils::WriteComment(entry_root, "0 -> Dynamic direction throughout entire DURATION. (User Only) Bone connection disabled");
    Utils::WriteComment(entry_root, "1 (default) -> Direction fixed to animation throughout the entire DURATION. (User Only) Bone connection enabled.");
    Utils::WriteComment(entry_root, "2 -> Direction fixed to animation throughout the entire DURATION. (User Only) Bone connection disabled");
    Utils::WriteComment(entry_root, "3 -> Direction fixed to animation throughout the entire DURATION. (User Only) Bone connection enabled");
    Utils::WriteComment(entry_root, "4 -> Dynamic direction throughout entire DURATION. (User Only) Bone connection enabled");
    Utils::WriteComment(entry_root, "5 -> Direction fixed to animation throughout the entire DURATION. (User Only) Bone connection enabled");
    Utils::WriteComment(entry_root, "6 -> Direction fixed to animation throughout the entire DURATION. (User Only) Bone connection disabled");
    Utils::WriteComment(entry_root, "7 -> Direction fixed to animation throughout the entire DURATION. (User Only) Bone connection enabled");
    Utils::WriteComment(entry_root, "8 -> Dynamic direction throughout entire DURATION. (User Only) Bone connection disabled");
    Utils::WriteComment(entry_root, "9 -> Direction fixed to animation throughout the entire DURATION. (User Only) Bone connection enabled");
    Utils::WriteComment(entry_root, "a -> Direction fixed to animation throughout the entire DURATION. (User Only) Bone connection disabled");
    Utils::WriteComment(entry_root, "b -> Direction fixed to animation throughout the entire DURATION. (User Only) Bone connection enabled");
    Utils::WriteComment(entry_root, "c -> Dynamic direction throughout entire DURATION. (User Only) Bone connection enabled");
    Utils::WriteComment(entry_root, "d -> Direction fixed to animation throughout the entire DURATION). (User Only) Bone connection enabled");
    Utils::WriteComment(entry_root, "e -> Direction fixed to animation throughout the entire DURATION. (User Only) Bone connection disabled");
    Utils::WriteComment(entry_root, "f -> Direction fixed to animation throughout the entire DURATION. (User Only) Bone connection enabled");*/
    Utils::WriteParamUnsigned(entry_root, "TH_FLAGS", th_flags, true);

    Utils::WriteParamUnsigned(entry_root, "U_0A", unk_0A, true);

    /*Utils::WriteComment(entry_root, "Known values for bone:");
    Utils::WriteComment(entry_root, "0x0 -> base; 0x7 -> right hand; 0x8 -> left hand");
    Utils::WriteComment(entry_root, "0x13 -> left foot; 0x14 -> left wrist; 0x15 -> right foot; 0x16 -> right wrist");*/
    Utils::WriteParamUnsigned(entry_root, "BONE_USER_CONNECTS_TO_VICTIM_FROM", bone_user_connects_to_victim_from, true);
    Utils::WriteParamUnsigned(entry_root, "BONE_VICTIM_CONNECTS_TO_USER_FROM", bone_victim_connects_to_user_from, true);

    Utils::WriteParamUnsigned(entry_root, "BAC_ENTRY", bac_entry, (bac_entry == 0xFFFF));
    Utils::WriteParamUnsigned(entry_root, "U_12", unk_12, true);

    if (!_small)
    {
        //Utils::WriteComment(entry_root, "Victim  displacement relative to throw source (x, y ,z)");
        Utils::WriteParamMultipleFloats(entry_root, "VICTIM_DISPLACEMENT", std::vector<float>(victim_displacement, victim_displacement+3));
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACThrowHandler::Compile(const TiXmlElement *root, bool _small)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "TH_FLAGS", &th_flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0A", &unk_0A))
        return false;

    if (!Utils::GetParamUnsigned(root, "BONE_USER_CONNECTS_TO_VICTIM_FROM", &bone_user_connects_to_victim_from))
        return false;

    if (!Utils::GetParamUnsigned(root, "BONE_VICTIM_CONNECTS_TO_USER_FROM", &bone_victim_connects_to_user_from))
        return false;

    if (!Utils::GetParamUnsigned(root, "BAC_ENTRY", &bac_entry))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_12", &unk_12))
        return false;

    if (_small)
        return true;

    if (!Utils::GetParamMultipleFloats(root, "VICTIM_DISPLACEMENT", victim_displacement, 3))
        return false;

    return true;
}

TiXmlElement *BACType18::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("BACType18");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);
    Utils::WriteParamMultipleUnsigned(entry_root, "U_08", std::vector<uint32_t>(unk_08, unk_08+3), true);
    Utils::WriteParamMultipleFloats(entry_root, "F_14", std::vector<float>(unk_14, unk_14+2));
    Utils::WriteParamUnsigned(entry_root, "U_1C", unk_1C, true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACType18::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (!Utils::GetParamMultipleUnsigned(root, "U_08", unk_08, 3))
        return false;

    if (!Utils::GetParamMultipleFloats(root, "F_14", unk_14, 2))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_1C", &unk_1C))
        return false;

    return true;
}

TiXmlElement *BACAuraEffect::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("AuraEffect");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);

    /*Utils::WriteComment(entry_root, "Known values:");
    Utils::WriteComment(entry_root, "0 -> Boost Start");
    Utils::WriteComment(entry_root, "1 -> Boost Loop");
    Utils::WriteComment(entry_root, "2 -> Boost End");
    Utils::WriteComment(entry_root, "3 -> Ki Charge Loop");
    Utils::WriteComment(entry_root, "4 -> Ki Charge End");
    Utils::WriteComment(entry_root, "5 -> Transform Aura Loop");
    Utils::WriteComment(entry_root, "6 -> Transform Aura End");*/
    Utils::WriteParamUnsigned(entry_root, "TYPE", type, true);

    /*Utils::WriteComment(entry_root, "Known values:");
    Utils::WriteComment(entry_root, "0x0, 0x8: on");
    Utils::WriteComment(entry_root, "1, 0x9: off");*/
    Utils::WriteParamUnsigned(entry_root, "ON_OFF_SWITCH", on_off_switch, true);

    Utils::WriteParamUnsigned(entry_root, "U_0C", unk_0C, true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACAuraEffect::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "TYPE", &type))
        return false;

    if (!Utils::GetParamUnsigned(root, "ON_OFF_SWITCH", &on_off_switch))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0C", &unk_0C))
        return false;

    return true;
}

TiXmlElement *BACHomingMovement::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("HomingMovement");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);

    /*Utils::WriteComment(entry_root, "Known values:");
    Utils::WriteComment(entry_root, "0x0 -> Horizontal arc");
    Utils::WriteComment(entry_root, "0x1 -> Straight line");
    Utils::WriteComment(entry_root, "0x2 -> Right-left and up-down arc");*/
    Utils::WriteParamUnsigned(entry_root, "TYPE", type, true);

    //Utils::WriteComment(entry_root, "Known Values: 0x0 -> Right-left arc. 0x1 -> Left-right arc");
    Utils::WriteParamUnsigned(entry_root, "HORIZONTAL_HOMING_ARC_DIRECTION", horizontal_homing_arc_direction, true);

    //Utils::WriteComment(entry_root, "The larger the value the slower the speed. 0x0 -> Instantly reach target");
    Utils::WriteParamUnsigned(entry_root, "SPEED_MODIFIER", speed_modifier, true);

    Utils::WriteParamUnsigned(entry_root, "U_10", unk_10, true);
    Utils::WriteParamUnsigned(entry_root, "U_12", unk_12, true);

    //Utils::WriteComment(entry_root, "Positive values for right, negative for left");
    Utils::WriteParamFloat(entry_root, "HORIZONTAL_DIRECTION_MODIFIER", horizontal_direction_modifier);
    //Utils::WriteComment(entry_root, "Positive values for up, negative for down");
    Utils::WriteParamFloat(entry_root, "VERTICAL_DIRECTION_MODIFIER", vertical_direction_modifier);    
    Utils::WriteParamFloat(entry_root, "Z_DIRECTION_MODIFIER", z_direction_modifier);

    Utils::WriteParamMultipleUnsigned(entry_root, "U_20",  std::vector<uint32_t>(unk_20, unk_20+4), true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACHomingMovement::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "TYPE", &type))
        return false;

    if (!Utils::GetParamUnsigned(root, "HORIZONTAL_HOMING_ARC_DIRECTION", &horizontal_homing_arc_direction))
        return false;

    if (!Utils::GetParamUnsigned(root, "SPEED_MODIFIER", &speed_modifier))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_10", &unk_10))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_12", &unk_12))
        return false;

    if (!Utils::GetParamFloat(root, "HORIZONTAL_DIRECTION_MODIFIER", &horizontal_direction_modifier))
        return false;

    if (!Utils::GetParamFloat(root, "VERTICAL_DIRECTION_MODIFIER", &vertical_direction_modifier))
        return false;

    if (!Utils::GetParamFloat(root, "Z_DIRECTION_MODIFIER", &z_direction_modifier))
        return false;

    if (!Utils::GetParamMultipleUnsigned(root, "U_20", unk_20, 4))
        return false;

    return true;
}

TiXmlElement *BACType21::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("BACType21");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);
    Utils::WriteParamUnsigned(entry_root, "U_08", unk_08, true);
    Utils::WriteParamUnsigned(entry_root, "U_0C", unk_0C, true);
    Utils::WriteParamUnsigned(entry_root, "U_0E", unk_0E, true);
    Utils::WriteParamUnsigned(entry_root, "U_10", unk_10, true);
    Utils::WriteParamUnsigned(entry_root, "U_12", unk_12, true);
    Utils::WriteParamUnsigned(entry_root, "U_14", unk_14, true);
    Utils::WriteParamUnsigned(entry_root, "U_16", unk_16, true);
    Utils::WriteParamFloat(entry_root, "F_18", unk_18);
    Utils::WriteParamFloat(entry_root, "F_1C", unk_1C);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACType21::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_08", &unk_08))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0C", &unk_0C))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0E", &unk_0E))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_10", &unk_10))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_12", &unk_12))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_14", &unk_14))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_16", &unk_16))
        return false;

    if (!Utils::GetParamFloat(root, "F_18", &unk_18))
        return false;

    if (!Utils::GetParamFloat(root, "F_1C", &unk_1C))
        return false;

    return true;
}

TiXmlElement *BACType22::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("BACType22");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);
    Utils::WriteParamUnsigned(entry_root, "U_08", unk_08, true);
    Utils::WriteParamUnsigned(entry_root, "U_0A", unk_0A, true);
    Utils::WriteParamFloat(entry_root, "F_0C", unk_0C);
    Utils::WriteParamString(entry_root, "NAME", name);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACType22::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_08", &unk_08))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0A", &unk_0A))
        return false;

    if (!Utils::GetParamFloat(root, "F_0C", &unk_0C))
        return false;

    std::string temp;

    if (!Utils::GetParamString(root, "NAME", temp))
        return false;

    if (temp.length() >= sizeof(name))
    {
        DPRINTF("%s: NAME too long. Must be 31 characters or less. (offending string=%s)\n", FUNCNAME, temp.c_str());
        return false;
    }

    memset(name, 0, sizeof(name));
    strcpy(name, temp.c_str());

    return true;
}

TiXmlElement *BACTransparencyEffect::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("TransparencyEffect");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);

    /*Utils::WriteComment(entry_root, "Known flags 0xABC");
    Utils::WriteComment(entry_root, "A: Vertical gap width");
    Utils::WriteComment(entry_root, "C: Activator. Known value: 2 -> Activate");*/
    Utils::WriteParamUnsigned(entry_root, "TRANSPARENCY_FLAGS", transparency_flags, true);

    /*Utils::WriteComment(entry_root, "Known flags 0xABC");
    Utils::WriteComment(entry_root, "A: Visible pixel width");
    Utils::WriteComment(entry_root, "C: Horizontal gap height");*/
    Utils::WriteParamUnsigned(entry_root, "TRANSPARENCY_FLAGS2", transparency_flags2, true);

    //Utils::WriteComment(entry_root, "Known value: 0x0 -> 100% transparent");
    Utils::WriteParamUnsigned(entry_root, "DILUTION", dilution, true);

    Utils::WriteParamUnsigned(entry_root, "U_0E", unk_0E, true);
    Utils::WriteParamUnsigned(entry_root, "U_10", unk_10, true);
    Utils::WriteParamMultipleFloats(entry_root, "RGB", std::vector<float>(rgb, rgb+3));
    Utils::WriteParamMultipleFloats(entry_root, "F_20", std::vector<float>(unk_20, unk_20+8));

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACTransparencyEffect::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "TRANSPARENCY_FLAGS", &transparency_flags))
        return false;

    if (!Utils::GetParamUnsigned(root, "TRANSPARENCY_FLAGS2", &transparency_flags2))
        return false;

    if (!Utils::GetParamUnsigned(root, "DILUTION", &dilution))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0E", &unk_0E))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_10", &unk_10))
        return false;

    if (!Utils::GetParamMultipleFloats(root, "RGB", rgb, 3))
        return false;

    if (!Utils::GetParamMultipleFloats(root, "F_20", unk_20, 8))
        return false;

    return true;
}

TiXmlElement *BACDualSkillData::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("DualSkillData");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);
    Utils::WriteParamUnsigned(entry_root, "U_08", unk_08, true);
    Utils::WriteParamUnsigned(entry_root, "U_0A", unk_0A, true);
    Utils::WriteParamUnsigned(entry_root, "U_0C", unk_0C, true);
    Utils::WriteParamUnsigned(entry_root, "U_0E", unk_0E, true);
    Utils::WriteParamUnsigned(entry_root, "U_10", unk_10, true);
    Utils::WriteParamFloat(entry_root, "F_14", unk_14);
    Utils::WriteParamFloat(entry_root, "F_18", unk_18);
    Utils::WriteParamFloat(entry_root, "F_1C", unk_1C);
    Utils::WriteParamUnsigned(entry_root, "U_20", unk_20, true);
    Utils::WriteParamUnsigned(entry_root, "U_22", unk_22, true);
    Utils::WriteParamUnsigned(entry_root, "U_24", unk_24, true);
    Utils::WriteParamFloat(entry_root, "F_28", unk_28);
    Utils::WriteParamFloat(entry_root, "F_2C", unk_2C);
    Utils::WriteParamFloat(entry_root, "F_30", unk_30);
    Utils::WriteParamUnsigned(entry_root, "U_34", unk_34, true);
    Utils::WriteParamUnsigned(entry_root, "U_36", unk_36, true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACDualSkillData::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time)) return false;
    if (!Utils::GetParamUnsigned(root, "DURATION", &duration)) return false;
    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04)) return false;
    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags)) return false;
    if (!Utils::GetParamUnsigned(root, "U_08", &unk_08)) return false;
    if (!Utils::GetParamUnsigned(root, "U_0A", &unk_0A)) return false;
    if (!Utils::GetParamUnsigned(root, "U_0C", &unk_0C)) return false;
    if (!Utils::GetParamUnsigned(root, "U_0E", &unk_0E)) return false;
    if (!Utils::GetParamUnsigned(root, "U_10", &unk_10)) return false;
    if (!Utils::GetParamFloat(root, "F_14", &unk_14)) return false;
    if (!Utils::GetParamFloat(root, "F_18", &unk_18)) return false;
    if (!Utils::GetParamFloat(root, "F_1C", &unk_1C)) return false;
    if (!Utils::GetParamUnsigned(root, "U_20", &unk_20)) return false;
    if (!Utils::GetParamUnsigned(root, "U_22", &unk_22)) return false;
    if (!Utils::GetParamUnsigned(root, "U_24", &unk_24)) return false;
    if (!Utils::GetParamFloat(root, "F_28", &unk_28)) return false;
    if (!Utils::GetParamFloat(root, "F_2C", &unk_2C)) return false;
    if (!Utils::GetParamFloat(root, "F_30", &unk_30)) return false;
    if (!Utils::GetParamUnsigned(root, "U_34", &unk_34)) return false;
    if (!Utils::GetParamUnsigned(root, "U_36", &unk_36)) return false;

    return true;
}

TiXmlElement *BACType25::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("BACType25");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);
    Utils::WriteParamUnsigned(entry_root, "U_08", unk_08, true);
    Utils::WriteParamUnsigned(entry_root, "U_0C", unk_0C);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACType25::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time)) return false;
    if (!Utils::GetParamUnsigned(root, "DURATION", &duration)) return false;
    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04)) return false;
    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags)) return false;
    if (!Utils::GetParamUnsigned(root, "U_08", &unk_08)) return false;
    if (!Utils::GetParamUnsigned(root, "U_0C", &unk_0C)) return false;

    return true;
}

TiXmlElement *BACType26::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("BACType26");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "U_08", unk_08);
    Utils::WriteParamUnsigned(entry_root, "U_0C", unk_0C);
    Utils::WriteParamFloat(entry_root, "F_10", unk_10);
    Utils::WriteParamMultipleUnsigned(entry_root, "U_14", std::vector<uint32_t>(unk_14, unk_14+15));

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACType26::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time)) return false;
    if (!Utils::GetParamUnsigned(root, "DURATION", &duration)) return false;
    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04)) return false;
    if (!Utils::GetParamUnsigned(root, "U_08", &unk_08)) return false;
    if (!Utils::GetParamUnsigned(root, "U_0C", &unk_0C)) return false;
    if (!Utils::GetParamFloat(root, "F_10", &unk_10)) return false;
    if (!Utils::GetParamMultipleUnsigned(root, "U_14", unk_14, 15)) return false;

    return true;
}

TiXmlElement *BACType27::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("BACType27");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);
    Utils::WriteParamUnsigned(entry_root, "SKILL_ID", skill_id);
    Utils::WriteParamUnsigned(entry_root, "U_0A", unk_0A);
    Utils::WriteParamUnsigned(entry_root, "U_0C", unk_0C);
    Utils::WriteParamUnsigned(entry_root, "U_0E", unk_0E);
    Utils::WriteParamUnsigned(entry_root, "U_10", unk_10);
    Utils::WriteParamUnsigned(entry_root, "U_14", unk_14);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACType27::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time)) return false;
    if (!Utils::GetParamUnsigned(root, "DURATION", &duration)) return false;
    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags)) return false;
    if (!Utils::GetParamUnsigned(root, "SKILL_ID", &skill_id)) return false;
    if (!Utils::GetParamUnsigned(root, "U_0A", &unk_0A)) return false;
    if (!Utils::GetParamUnsigned(root, "U_0C", &unk_0C)) return false;
    if (!Utils::GetParamUnsigned(root, "U_0E", &unk_0E)) return false;
    if (!Utils::GetParamUnsigned(root, "U_10", &unk_10)) return false;
    if (!Utils::GetParamUnsigned(root, "U_14", &unk_14)) return false;

    return true;
}

TiXmlElement *BACType28::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("BACType28");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04);
    Utils::WriteParamUnsigned(entry_root, "U_08", unk_08);
    Utils::WriteParamUnsigned(entry_root, "U_0A", unk_0A);
    Utils::WriteParamUnsigned(entry_root, "U_0C", unk_0C);
    Utils::WriteParamFloat(entry_root, "F_10", unk_10);
    Utils::WriteParamFloat(entry_root, "F_14", unk_14);
    Utils::WriteParamMultipleUnsigned(entry_root, "U_18", std::vector<uint32_t>(unk_18, unk_18+3));

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACType28::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time)) return false;
    if (!Utils::GetParamUnsigned(root, "DURATION", &duration)) return false;
    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04)) return false;
    if (!Utils::GetParamUnsigned(root, "U_08", &unk_08)) return false;
    if (!Utils::GetParamUnsigned(root, "U_0A", &unk_0A)) return false;
    if (!Utils::GetParamUnsigned(root, "U_0C", &unk_0C)) return false;
    if (!Utils::GetParamFloat(root, "F_10", &unk_10)) return false;
    if (!Utils::GetParamFloat(root, "F_14", &unk_14)) return false;
    if (!Utils::GetParamMultipleUnsigned(root, "U_18", unk_18, 3)) return false;

    return true;
}

TiXmlElement *BACType29::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("BACType29");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04);
    Utils::WriteParamUnsigned(entry_root, "U_08", unk_08);
    Utils::WriteParamUnsigned(entry_root, "U_0A", unk_0A);
    Utils::WriteParamUnsigned(entry_root, "U_0C", unk_0C);
    Utils::WriteParamMultipleFloats(entry_root, "F_10", std::vector<float>(unk_10, unk_10+8));
    Utils::WriteParamMultipleUnsigned(entry_root, "U_30", std::vector<uint32_t>(unk_30, unk_30+3));

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACType29::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time)) return false;
    if (!Utils::GetParamUnsigned(root, "DURATION", &duration)) return false;
    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04)) return false;
    if (!Utils::GetParamUnsigned(root, "U_08", &unk_08)) return false;
    if (!Utils::GetParamUnsigned(root, "U_0A", &unk_0A)) return false;
    if (!Utils::GetParamUnsigned(root, "U_0C", &unk_0C)) return false;
    if (!Utils::GetParamMultipleFloats(root, "F_10", unk_10, 8)) return false;
    if (!Utils::GetParamMultipleUnsigned(root, "U_30", unk_30, 3)) return false;

    return true;
}

TiXmlElement *BACType30::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("BACType30");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04);
    Utils::WriteParamFloat(entry_root, "F_08", unk_08);
    Utils::WriteParamMultipleUnsigned(entry_root, "U_0C", std::vector<uint32_t>(unk_0C, unk_0C+9));

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACType30::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time)) return false;
    if (!Utils::GetParamUnsigned(root, "DURATION", &duration)) return false;
    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04)) return false;
    if (!Utils::GetParamFloat(root, "F_08", &unk_08)) return false;
    if (!Utils::GetParamMultipleUnsigned(root, "U_0C", unk_0C, 9)) return false;

    return true;
}

TiXmlElement *BACType31::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("BACType31");

    Utils::WriteParamUnsigned(entry_root, "START_TIME", start_time);
    Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags);
    Utils::WriteParamUnsigned(entry_root, "U_08", unk_08);
    Utils::WriteParamUnsigned(entry_root, "U_0C", unk_0C);
    Utils::WriteParamMultipleUnsigned(entry_root, "U_10", std::vector<uint16_t>(unk_10, unk_10+4));
    Utils::WriteParamFloat(entry_root, "F_18", unk_18);
    Utils::WriteParamFloat(entry_root, "F_1C", unk_1C);
    Utils::WriteParamMultipleUnsigned(entry_root, "U_20", std::vector<uint32_t>(unk_20, unk_20+8));

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BACType31::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "START_TIME", &start_time)) return false;
    if (!Utils::GetParamUnsigned(root, "DURATION", &duration)) return false;
    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04)) return false;
    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags)) return false;
    if (!Utils::GetParamUnsigned(root, "U_08", &unk_08)) return false;
    if (!Utils::GetParamUnsigned(root, "U_0C", &unk_0C)) return false;
    if (!Utils::GetParamMultipleUnsigned(root, "U_10", unk_10, 4)) return false;
    if (!Utils::GetParamFloat(root, "F_18", &unk_18)) return false;
    if (!Utils::GetParamFloat(root, "F_1C", &unk_1C)) return false;
    if (!Utils::GetParamMultipleUnsigned(root, "U_20", unk_20, 8)) return false;

    return true;
}

TiXmlElement *BacEntry::Decompile(TiXmlNode *root, bool small_17, int idx) const
{
    TiXmlElement *entry_root = new TiXmlElement("BacEntry");
    entry_root->SetAttribute("idx", idx);

    entry_root->SetAttribute("flags", Utils::UnsignedToString(flags, true));
    if (valid)
    {
        if (has_type[0])
        {
            if (type0.size() != 0)
            {
                for (const BACAnimation &t0 : type0)
                    t0.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY0", 1);
            }
        }

        if (has_type[1])
        {
            if (type1.size() != 0)
            {
                for (const BACHitbox &t1 : type1)
                    t1.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY1", 1);
            }
        }

        if (has_type[2])
        {
            if (type2.size() != 0)
            {
                for (const BACAccelerationMovement &t2 : type2)
                    t2.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY2", 1);
            }
        }

        if (has_type[3])
        {
            if (type3.size() != 0)
            {
                for (const BACInvulnerability &t3 : type3)
                    t3.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY3", 1);
            }
        }

        if (has_type[4])
        {
            if (type4.size() != 0)
            {
                for (const BACMotionAdjust &t4 : type4)
                    t4.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY4", 1);
            }
        }

        if (has_type[5])
        {
            if (type5.size() != 0)
            {
                for (const BACOpponentKnockback &t5 : type5)
                    t5.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY5", 1);
            }
        }

        if (has_type[6])
        {
            if (type6.size() != 0)
            {
                for (const BACChainAttackParameters &t6 : type6)
                    t6.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY6", 1);
            }
        }

        if (has_type[7])
        {
            if (type7.size() != 0)
            {
                for (const BACBcmCallback &t7 : type7)
                    t7.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY7", 1);
            }
        }

        if (has_type[8])
        {
            if (type8.size() != 0)
            {
                for (const BACEffect &t8 : type8)
                    t8.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY8", 1);
            }
        }

        if (has_type[9])
        {
            if (type9.size() != 0)
            {
                for (const BACProjectile &t9 : type9)
                    t9.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY9", 1);
            }
        }

        if (has_type[10])
        {
            if (type10.size() != 0)
            {
                for (const BACCamera &t10 : type10)
                    t10.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY10", 1);
            }
        }

        if (has_type[11])
        {
            if (type11.size() != 0)
            {
                for (const BACSound &t11 : type11)
                    t11.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY11", 1);
            }
        }

        if (has_type[12])
        {
            if (type12.size() != 0)
            {
                for (const BACType12 &t12 : type12)
                    t12.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY12", 1);
            }
        }

        if (has_type[13])
        {
            if (type13.size() != 0)
            {
                for (const BACPartInvisibility &t13 : type13)
                    t13.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY13", 1);
            }
        }

        if (has_type[14])
        {
            if (type14.size() != 0)
            {
                for (const BACAnimationModification &t14 : type14)
                    t14.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY14", 1);
            }
        }

        if (has_type[15])
        {
            if (type15.size() != 0)
            {
                for (const BACTransformControl &t15 : type15)
                    t15.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY15", 1);
            }
        }

        if (has_type[16])
        {
            if (type16.size() != 0)
            {
                for (const BACScreenEffect &t16 : type16)
                    t16.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY16", 1);
            }
        }

        if (has_type[17])
        {
            if (type17.size() != 0)
            {
                for (const BACThrowHandler &t17 : type17)
                    t17.Decompile(entry_root, small_17);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY17", 1);
            }
        }

        if (has_type[18])
        {
            if (type18.size() != 0)
            {
                for (const BACType18 &t18 : type18)
                    t18.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY18", 1);
            }
        }

        if (has_type[19])
        {
            if (type19.size() != 0)
            {
                for (const BACAuraEffect &t19 : type19)
                    t19.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY19", 1);
            }
        }

        if (has_type[20])
        {
            if (type20.size() != 0)
            {
                for (const BACHomingMovement &t20 : type20)
                    t20.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY20", 1);
            }
        }

        if (has_type[21])
        {
            if (type21.size() != 0)
            {
                for (const BACType21 &t21 : type21)
                    t21.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY21", 1);
            }
        }

        if (has_type[22])
        {
            if (type22.size() != 0)
            {
                for (const BACType22 &t22 : type22)
                    t22.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY22", 1);
            }
        }

        if (has_type[23])
        {
            if (type23.size() != 0)
            {
                for (const BACTransparencyEffect &t23 : type23)
                    t23.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY23", 1);
            }
        }

        if (has_type[24])
        {
            if (type24.size() != 0)
            {
                for (const BACDualSkillData &t24: type24)
                    t24.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY24", 1);
            }
        }

        if (has_type[25])
        {
            if (type25.size() != 0)
            {
                for (const BACType25 &t25: type25)
                    t25.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY25", 1);
            }
        }

        if (has_type[26])
        {
            if (type26.size() != 0)
            {
                for (const BACType26 &t26: type26)
                    t26.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY26", 1);
            }
        }

        if (has_type[27])
        {
            if (type27.size() != 0)
            {
                for (const BACType27 &t27: type27)
                    t27.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY27", 1);
            }
        }

        if (has_type[28])
        {
            if (type28.size() != 0)
            {
                for (const BACType28 &t28: type28)
                    t28.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY28", 1);
            }
        }

        if (has_type[29])
        {
            if (type29.size() != 0)
            {
                for (const BACType29 &t29: type29)
                    t29.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY29", 1);
            }
        }

        if (has_type[30])
        {
            if (type30.size() != 0)
            {
                for (const BACType30 &t30: type30)
                    t30.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY30", 1);
            }
        }

        if (has_type[31])
        {
            if (type31.size() != 0)
            {
                for (const BACType31 &t31: type31)
                    t31.Decompile(entry_root);
            }
            else
            {
                Utils::WriteParamUnsigned(entry_root, "HAS_DUMMY31", 1);
            }
        }
    }
    else
    {
        Utils::WriteComment(entry_root, "This entry is empty.");
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BacEntry::Compile(const TiXmlElement *root, bool small_17)
{
    if (!Utils::ReadAttrUnsigned(root, "flags", &flags))
    {
        DPRINTF("%s: Attribute flags is not optional.\n", FUNCNAME);
        return false;
    }

    for (int i = 0; i <= MAX_BAC_TYPE; i++)
        has_type[i] = false;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Animation")
        {
            BACAnimation t0;

            if (!t0.Compile(elem))
                return false;

            has_type[0] = true;
            type0.push_back(t0);
        }
        else if (elem->ValueStr() == "Hitbox")
        {
            BACHitbox t1;

            if (!t1.Compile(elem))
                return false;

            has_type[1] = true;
            type1.push_back(t1);
        }
        else if (elem->ValueStr() == "AccelerationMovement")
        {
            BACAccelerationMovement t2;

            if (!t2.Compile(elem))
                return false;

            has_type[2] = true;
            type2.push_back(t2);
        }
        else if (elem->ValueStr() == "Invulnerability")
        {
            BACInvulnerability t3;

            if (!t3.Compile(elem))
                return false;

            has_type[3] = true;
            type3.push_back(t3);
        }
        else if (elem->ValueStr() == "MotionAdjust")
        {
            BACMotionAdjust t4;

            if (!t4.Compile(elem))
                return false;

            has_type[4] = true;
            type4.push_back(t4);
        }
        else if (elem->ValueStr() == "OpponentKnockback")
        {
            BACOpponentKnockback t5;

            if (!t5.Compile(elem))
                return false;

            has_type[5] = true;
            type5.push_back(t5);
        }
        else if (elem->ValueStr() == "ChainAttackParameters")
        {
            BACChainAttackParameters t6;

            if (!t6.Compile(elem))
                return false;

            has_type[6] = true;
            type6.push_back(t6);
        }
        else if (elem->ValueStr() == "BcmCallback")
        {
            BACBcmCallback t7;

            if (!t7.Compile(elem))
                return false;

            has_type[7] = true;
            type7.push_back(t7);
        }
        else if (elem->ValueStr() == "Effect")
        {
            BACEffect t8;

            if (!t8.Compile(elem))
                return false;

            has_type[8] = true;
            type8.push_back(t8);
        }
        else if (elem->ValueStr() == "Projectile")
        {
            BACProjectile t9;

            if (!t9.Compile(elem))
                return false;

            has_type[9] = true;
            type9.push_back(t9);
        }
        else if (elem->ValueStr() == "Camera")
        {
            BACCamera t10;

            if (!t10.Compile(elem))
                return false;

            has_type[10] = true;
            type10.push_back(t10);
        }
        else if (elem->ValueStr() == "Sound")
        {
            BACSound t11;

            if (!t11.Compile(elem))
                return false;

            has_type[11] = true;
            type11.push_back(t11);
        }
        else if (elem->ValueStr() == "BACType12")
        {
            BACType12 t12;

            if (!t12.Compile(elem))
                return false;

            has_type[12] = true;
            type12.push_back(t12);
        }
        else if (elem->ValueStr() == "PartInvisibility")
        {
            BACPartInvisibility t13;

            if (!t13.Compile(elem))
                return false;

            has_type[13] = true;
            type13.push_back(t13);
        }
        else if (elem->ValueStr() == "AnimationModification")
        {
            BACAnimationModification t14;

            if (!t14.Compile(elem))
                return false;

            has_type[14] = true;
            type14.push_back(t14);
        }
        else if (elem->ValueStr() == "TransformControl")
        {
            BACTransformControl t15;

            if (!t15.Compile(elem))
                return false;

            has_type[15] = true;
            type15.push_back(t15);
        }
        else if (elem->ValueStr() == "ScreenEffect")
        {
            BACScreenEffect t16;

            if (!t16.Compile(elem))
                return false;

            has_type[16] = true;
            type16.push_back(t16);
        }
        else if (elem->ValueStr() == "ThrowHandler")
        {
            BACThrowHandler t17;

            if (!t17.Compile(elem, small_17))
                return false;

            has_type[17] = true;
            type17.push_back(t17);
        }
        else if (elem->ValueStr() == "BACType18")
        {
            BACType18 t18;

            if (!t18.Compile(elem))
                return false;

            has_type[18] = true;
            type18.push_back(t18);
        }
        else if (elem->ValueStr() == "AuraEffect")
        {
            BACAuraEffect t19;

            if (!t19.Compile(elem))
                return false;

            has_type[19] = true;
            type19.push_back(t19);
        }
        else if (elem->ValueStr() == "HomingMovement")
        {
            BACHomingMovement t20;

            if (!t20.Compile(elem))
                return false;

            has_type[20] = true;
            type20.push_back(t20);
        }
        else if (elem->ValueStr() == "BACType21")
        {
            BACType21 t21;

            if (!t21.Compile(elem))
                return false;

            has_type[21] = true;
            type21.push_back(t21);
        }
        else if (elem->ValueStr() == "BACType22")
        {
            BACType22 t22;

            if (!t22.Compile(elem))
                return false;

            has_type[22] = true;
            type22.push_back(t22);
        }
        else if (elem->ValueStr() == "TransparencyEffect")
        {
            BACTransparencyEffect t23;

            if (!t23.Compile(elem))
                return false;

            has_type[23] = true;
            type23.push_back(t23);
        }
        else if (elem->ValueStr() == "DualSkillData")
        {
            BACDualSkillData t24;

            if (!t24.Compile(elem))
                return false;

            has_type[24] = true;
            type24.push_back(t24);
        }
        else if (elem->ValueStr() == "BACType25")
        {
            BACType25 t25;

            if (!t25.Compile(elem))
                return false;

            has_type[25] = true;
            type25.push_back(t25);
        }
        else if (elem->ValueStr() == "BACType26")
        {
            BACType26 t26;

            if (!t26.Compile(elem))
                return false;

            has_type[26] = true;
            type26.push_back(t26);
        }
        else if (elem->ValueStr() == "BACType27")
        {
            BACType27 t27;

            if (!t27.Compile(elem))
                return false;

            has_type[27] = true;
            type27.push_back(t27);
        }
        else if (elem->ValueStr() == "BACType28")
        {
            BACType28 t28;

            if (!t28.Compile(elem))
                return false;

            has_type[28] = true;
            type28.push_back(t28);
        }
        else if (elem->ValueStr() == "BACType29")
        {
            BACType29 t29;

            if (!t29.Compile(elem))
                return false;

            has_type[29] = true;
            type29.push_back(t29);
        }
        else if (elem->ValueStr() == "BACType30")
        {
            BACType30 t30;

            if (!t30.Compile(elem))
                return false;

            has_type[30] = true;
            type30.push_back(t30);
        }
        else if (elem->ValueStr() == "BACType31")
        {
            BACType31 t31;

            if (!t31.Compile(elem))
                return false;

            has_type[31] = true;
            type31.push_back(t31);
        }
    }

    for (int i = 0; i <= MAX_BAC_TYPE; i++)
    {
        if (!has_type[i])
        {
            const std::string param_name = "HAS_DUMMY" + Utils::ToString(i);
            uint32_t val;

            if (Utils::ReadParamUnsigned(root, param_name.c_str(), &val) && val)
            {
                has_type[i] = true;
            }
        }
    }

    valid = false;
    for (int i = 0; i <= MAX_BAC_TYPE; i++)
    {
        if (has_type[i])
        {
            valid = true;
            break;
        }
    }

    return true;
}

BacFile::BacFile()
{
    this->big_endian = false;
}

BacFile::~BacFile()
{

}

void BacFile::Reset()
{
    entries.clear();
    size17_small = false;
}

bool BacFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(BACHeader))
        return false;

    const BACHeader *hdr = (const BACHeader *)buf;
    if (hdr->signature != BAC_SIGNATURE)
        return false;

    entries.resize(hdr->num_entries);

    const BACEntry *file_entries = (const BACEntry *)(buf+hdr->data_start);

    for (size_t i = 0; i < entries.size(); i++)
    {
        BacEntry &entry = entries[i];
        entry.flags = file_entries[i].flags;

        if (file_entries[i].num_subentries != 0)
        {
            entry.valid = true;            

            const BACSubEntry *file_subentries = (const BACSubEntry *)(buf+file_entries[i].sub_entry_offset);

            for (uint16_t j = 0; j < file_entries[i].num_subentries; j++)
            {
                uint16_t type = file_subentries[j].type;
                uint16_t num = file_subentries[j].num;

                if (type == 0)
                {
                    if (num > 0)
                    {
                        const BACAnimation *file_types0 = (const BACAnimation *)(buf+file_subentries[j].offset);
                        entry.type0.resize(num);

                        for (size_t k = 0; k < entry.type0.size(); k++)
                        {
                            entry.type0[k] = file_types0[k];
                        }
                    }
                }
                else if (type == 1)
                {
                    if (num > 0)
                    {
                        const BACHitbox *file_types1 = (const BACHitbox *)(buf+file_subentries[j].offset);
                        entry.type1.resize(num);

                        for (size_t k = 0; k < entry.type1.size(); k++)
                        {
                            entry.type1[k] = file_types1[k];
                        }
                    }
                }
                else if (type == 2)
                {
                    if (num > 0)
                    {
                        const BACAccelerationMovement *file_types2 = (const BACAccelerationMovement *)(buf+file_subentries[j].offset);
                        entry.type2.resize(num);

                        for (size_t k = 0; k < entry.type2.size(); k++)
                        {
                            entry.type2[k] = file_types2[k];
                        }
                    }
                }
                else if (type == 3)
                {
                    if (num > 0)
                    {
                        const BACInvulnerability *file_types3 = (const BACInvulnerability *)(buf+file_subentries[j].offset);
                        entry.type3.resize(num);

                        for (size_t k = 0; k < entry.type3.size(); k++)
                        {
                            entry.type3[k] = file_types3[k];
                        }
                    }
                }
                else if (type == 4)
                {
                    if (num > 0)
                    {
                        const BACMotionAdjust *file_types4 = (const BACMotionAdjust *)(buf+file_subentries[j].offset);
                        entry.type4.resize(num);

                        for (size_t k = 0; k < entry.type4.size(); k++)
                        {
                            entry.type4[k] = file_types4[k];
                        }
                    }
                }
                else if (type == 5)
                {
                    if (num > 0)
                    {
                        const BACOpponentKnockback *file_types5 = (const BACOpponentKnockback *)(buf+file_subentries[j].offset);
                        entry.type5.resize(num);

                        for (size_t k = 0; k < entry.type5.size(); k++)
                        {
                            entry.type5[k] = file_types5[k];
                        }
                    }
                }
                else if (type == 6)
                {
                    if (num > 0)
                    {
                        const BACChainAttackParameters *file_types6 = (const BACChainAttackParameters *)(buf+file_subentries[j].offset);
                        entry.type6.resize(num);

                        for (size_t k = 0; k < entry.type6.size(); k++)
                        {
                            entry.type6[k] = file_types6[k];
                        }
                    }
                }
                else if (type == 7)
                {
                    if (num > 0)
                    {
                        const BACBcmCallback *file_types7 = (const BACBcmCallback *)(buf+file_subentries[j].offset);
                        entry.type7.resize(num);

                        for (size_t k = 0; k < entry.type7.size(); k++)
                        {
                            entry.type7[k] = file_types7[k];
                        }
                    }
                }
                else if (type == 8)
                {
                    if (num > 0)
                    {
                        const BACEffect *file_types8 = (const BACEffect *)(buf+file_subentries[j].offset);
                        entry.type8.resize(num);

                        for (size_t k = 0; k < entry.type8.size(); k++)
                        {
                            entry.type8[k] = file_types8[k];
                        }
                    }
                }
                else if (type == 9)
                {
                    if (num > 0)
                    {
                        const BACProjectile *file_types9 = (const BACProjectile *)(buf+file_subentries[j].offset);
                        entry.type9.resize(num);

                        for (size_t k = 0; k < entry.type9.size(); k++)
                        {
                            entry.type9[k] = file_types9[k];
                        }
                    }
                }
                else if (type == 10)
                {
                    if (num > 0)
                    {
                        const BACCamera *file_types10 = (const BACCamera *)(buf+file_subentries[j].offset);
                        entry.type10.resize(num);

                        for (size_t k = 0; k < entry.type10.size(); k++)
                        {
                            entry.type10[k] = file_types10[k];
                        }
                    }
                }
                else if (type == 11)
                {
                    if (num > 0)
                    {
                        const BACSound *file_types11 = (const BACSound *)(buf+file_subentries[j].offset);
                        entry.type11.resize(num);

                        for (size_t k = 0; k < entry.type11.size(); k++)
                        {
                            entry.type11[k] = file_types11[k];
                        }
                    }
                }
                else if (type == 12)
                {
                    if (num > 0)
                    {
                        const BACType12 *file_types12 = (const BACType12 *)(buf+file_subentries[j].offset);
                        entry.type12.resize(num);

                        for (size_t k = 0; k < entry.type12.size(); k++)
                        {
                            entry.type12[k] = file_types12[k];
                        }
                    }
                }
                else if (type == 13)
                {
                    if (num > 0)
                    {
                        const BACPartInvisibility *file_types13 = (const BACPartInvisibility *)(buf+file_subentries[j].offset);
                        entry.type13.resize(num);

                        for (size_t k = 0; k < entry.type13.size(); k++)
                        {
                            entry.type13[k] = file_types13[k];
                        }
                    }
                }
                else if (type == 14)
                {
                    if (num > 0)
                    {
                        const BACAnimationModification *file_types14 = (const BACAnimationModification *)(buf+file_subentries[j].offset);
                        entry.type14.resize(num);

                        for (size_t k = 0; k < entry.type14.size(); k++)
                        {
                            entry.type14[k] = file_types14[k];
                        }
                    }
                }
                else if (type == 15)
                {
                    if (num > 0)
                    {
                        const BACTransformControl *file_types15 = (const BACTransformControl *)(buf+file_subentries[j].offset);
                        entry.type15.resize(num);

                        for (size_t k = 0; k < entry.type15.size(); k++)
                        {
                            entry.type15[k] = file_types15[k];
                        }
                    }
                }
                else if (type == 16)
                {
                    if (num > 0)
                    {
                        const BACScreenEffect *file_types16 = (const BACScreenEffect *)(buf+file_subentries[j].offset);
                        entry.type16.resize(num);

                        for (size_t k = 0; k < entry.type16.size(); k++)
                        {
                            entry.type16[k] = file_types16[k];
                        }
                    }
                }
                else if (type == 17)
                {
                    if (num > 0)
                    {
                        uint32_t size = 0;

                        if (!size17_small)
                        {
                            if (j != (file_entries[i].num_subentries-1))
                            {
                                size = (file_subentries[j+1].offset - file_subentries[j].offset) / num;
                            }
                            else if (i != (entries.size()-1) && file_entries[i+1].num_subentries > 0)
                            {
                                const BACSubEntry *next_subentry = (const BACSubEntry *)(buf+file_entries[i+1].sub_entry_offset);

                                if (next_subentry->num > 0)
                                    size = (next_subentry->offset - file_subentries[j].offset) / num;
                            }


                            //DPRINTF("size = 0x%x\n", size);

                            if (size == 0x14)
                                size17_small = true;
                        }


                        const BACThrowHandler *file_types17 = (const BACThrowHandler *)(buf+file_subentries[j].offset);
                        entry.type17.resize(num);

                        for (size_t k = 0; k < entry.type17.size(); k++)
                        {
                            if (size17_small)
                            {
                                entry.type17[k].start_time = file_types17->start_time;
                                entry.type17[k].duration = file_types17->duration;
                                entry.type17[k].unk_04 = file_types17->unk_04;
                                entry.type17[k].th_flags = file_types17->th_flags;
                                entry.type17[k].unk_0A = file_types17->unk_0A;
                                entry.type17[k].bone_user_connects_to_victim_from = file_types17->bone_user_connects_to_victim_from;
                                entry.type17[k].bone_victim_connects_to_user_from = file_types17->bone_victim_connects_to_user_from;
                                entry.type17[k].bac_entry = file_types17->bac_entry;
                                entry.type17[k].unk_12 = file_types17->unk_12;
                                file_types17 = (const BACThrowHandler *)GetOffsetPtr(file_types17, 0x14, true);
                            }
                            else
                            {
                                entry.type17[k] = file_types17[k];
                            }
                        }
                    }
                }
                else if (type == 18)
                {
                    if (num > 0)
                    {
                        const BACType18 *file_types18 = (const BACType18 *)(buf+file_subentries[j].offset);
                        entry.type18.resize(num);

                        for (size_t k = 0; k < entry.type18.size(); k++)
                        {
                            entry.type18[k] = file_types18[k];
                        }
                    }
                }
                else if (type == 19)
                {
                    if (num > 0)
                    {
                        const BACAuraEffect *file_types19 = (const BACAuraEffect *)(buf+file_subentries[j].offset);
                        entry.type19.resize(num);

                        for (size_t k = 0; k < entry.type19.size(); k++)
                        {
                            entry.type19[k] = file_types19[k];
                        }
                    }
                }
                else if (type == 20)
                {
                    if (num > 0)
                    {
                        const BACHomingMovement *file_types20 = (const BACHomingMovement *)(buf+file_subentries[j].offset);
                        entry.type20.resize(num);

                        for (size_t k = 0; k < entry.type20.size(); k++)
                        {
                            entry.type20[k] = file_types20[k];
                        }
                    }
                }
                else if (type == 21)
                {
                    if (num > 0)
                    {
                        const BACType21 *file_types21 = (const BACType21 *)(buf+file_subentries[j].offset);
                        entry.type21.resize(num);

                        for (size_t k = 0; k < entry.type21.size(); k++)
                        {
                            entry.type21[k] = file_types21[k];
                        }
                    }
                }
                else if (type == 22)
                {
                    if (num > 0)
                    {
                        const BACType22 *file_types22 = (const BACType22 *)(buf+file_subentries[j].offset);
                        entry.type22.resize(num);

                        for (size_t k = 0; k < entry.type22.size(); k++)
                        {
                            entry.type22[k] = file_types22[k];
                        }
                    }
                }
                else if (type == 23)
                {
                    if (num > 0)
                    {
                        const BACTransparencyEffect *file_types23 = (const BACTransparencyEffect *)(buf+file_subentries[j].offset);
                        entry.type23.resize(num);

                        for (size_t k = 0; k < entry.type23.size(); k++)
                        {
                            entry.type23[k] = file_types23[k];
                        }
                    }
                }
                else if (type == 24)
                {
                    if (num > 0)
                    {
                        const BACDualSkillData *file_types24 = (const BACDualSkillData *)(buf+file_subentries[j].offset);
                        entry.type24.resize(num);

                        for (size_t k = 0; k < entry.type24.size(); k++)
                        {
                            entry.type24[k] = file_types24[k];
                        }
                    }
                }
                else if (type == 25)
                {
                    if (num > 0)
                    {
                        const BACType25 *file_types25 = (const BACType25 *)(buf+file_subentries[j].offset);
                        entry.type25.resize(num);

                        for (size_t k = 0; k < entry.type25.size(); k++)
                        {
                            entry.type25[k] = file_types25[k];
                        }
                    }
                }
                else if (type == 26)
                {
                    if (num > 0)
                    {
                        const BACType26 *file_types26 = (const BACType26 *)(buf+file_subentries[j].offset);
                        entry.type26.resize(num);

                        for (size_t k = 0; k < entry.type26.size(); k++)
                        {
                            entry.type26[k] = file_types26[k];
                        }
                    }
                }
                else if (type == 27)
                {
                    if (num > 0)
                    {
                        const BACType27 *file_types27 = (const BACType27 *)(buf+file_subentries[j].offset);
                        entry.type27.resize(num);

                        for (size_t k = 0; k < entry.type27.size(); k++)
                        {
                            entry.type27[k] = file_types27[k];
                        }
                    }
                }
                else if (type == 28)
                {
                    if (num > 0)
                    {
                        const BACType28 *file_types28 = (const BACType28 *)(buf+file_subentries[j].offset);
                        entry.type28.resize(num);

                        for (size_t k = 0; k < entry.type28.size(); k++)
                        {
                            entry.type28[k] = file_types28[k];
                        }
                    }
                }
                else if (type == 29)
                {
                    if (num > 0)
                    {
                        const BACType29 *file_types29 = (const BACType29 *)(buf+file_subentries[j].offset);
                        entry.type29.resize(num);

                        for (size_t k = 0; k < entry.type29.size(); k++)
                        {
                            entry.type29[k] = file_types29[k];
                        }
                    }
                }
                else if (type == 30)
                {
                    if (num > 0)
                    {
                        const BACType30 *file_types30 = (const BACType30 *)(buf+file_subentries[j].offset);
                        entry.type30.resize(num);

                        for (size_t k = 0; k < entry.type30.size(); k++)
                        {
                            entry.type30[k] = file_types30[k];
                        }
                    }
                }
                else if (type == 31)
                {
                    if (num > 0)
                    {
                        const BACType31 *file_types31 = (const BACType31 *)(buf+file_subentries[j].offset);
                        entry.type31.resize(num);

                        for (size_t k = 0; k < entry.type31.size(); k++)
                        {
                            entry.type31[k] = file_types31[k];
                        }
                    }
                }
                else
                {
                    DPRINTF("%s: Unrecognized bac type: %d (offset=0x%x, subentry=0x%x)\n", FUNCNAME, type, file_subentries[j].offset, Utils::DifPointer(&file_subentries[j], buf));
                    return false;
                }

                entry.has_type[type] = true;
            }
        }
    }

    memcpy(unk_14, hdr->unk_14, sizeof(unk_14));
    memcpy(unk_20, hdr->unk_20, sizeof(unk_20));
    memcpy(unk_48, hdr->unk_48, sizeof(unk_48));
    memcpy(unk_50, hdr->unk_50, sizeof(unk_50));

    return true;
}

size_t BacFile::CalculateFileSize() const
{
    size_t size = sizeof(BACHeader) + entries.size() * sizeof(BACEntry);

    for (const BacEntry &entry :entries)
    {
        if (!entry.valid)
            continue;

        size += entry.GetNumSubEntries() * sizeof(BACSubEntry);
        size += entry.type0.size() * sizeof(BACAnimation);
        size += entry.type1.size() * sizeof(BACHitbox);
        size += entry.type2.size() * sizeof(BACAccelerationMovement);
        size += entry.type3.size() * sizeof(BACInvulnerability);
        size += entry.type4.size() * sizeof(BACMotionAdjust);
        size += entry.type5.size() * sizeof(BACOpponentKnockback);
        size += entry.type6.size() * sizeof(BACChainAttackParameters);
        size += entry.type7.size() * sizeof(BACBcmCallback);
        size += entry.type8.size() * sizeof(BACEffect);
        size += entry.type9.size() * sizeof(BACProjectile);
        size += entry.type10.size() * sizeof(BACCamera);
        size += entry.type11.size() * sizeof(BACSound);
        size += entry.type12.size() * sizeof(BACType12);
        size += entry.type13.size() * sizeof(BACPartInvisibility);
        size += entry.type14.size() * sizeof(BACAnimationModification);
        size += entry.type15.size() * sizeof(BACTransformControl);
        size += entry.type16.size() * sizeof(BACScreenEffect);

        if (size17_small)
            size += entry.type17.size() * 0x14;
        else
            size += entry.type17.size() * sizeof(BACThrowHandler);

        size += entry.type18.size() * sizeof(BACType18);
        size += entry.type19.size() * sizeof(BACAuraEffect);
        size += entry.type20.size() * sizeof(BACHomingMovement);
        size += entry.type21.size() * sizeof(BACType21);
        size += entry.type22.size() * sizeof(BACType22);
        size += entry.type23.size() * sizeof(BACTransparencyEffect);
        size += entry.type24.size() * sizeof(BACDualSkillData);
        size += entry.type25.size() * sizeof(BACType25);
        size += entry.type26.size() * sizeof(BACType26);
        size += entry.type27.size() * sizeof(BACType27);
        size += entry.type28.size() * sizeof(BACType28);
        size += entry.type29.size() * sizeof(BACType29);
        size += entry.type30.size() * sizeof(BACType30);
        size += entry.type31.size() * sizeof(BACType31);
    }

    return size;
}

size_t BacFile::GetDataStart() const
{
    size_t size = sizeof(BACHeader) + entries.size() * sizeof(BACEntry);

    for (const BacEntry &entry :entries)
    {
        if (!entry.valid)
            continue;

        size += entry.GetNumSubEntries() * sizeof(BACSubEntry);
    }

    return size;
}

uint8_t *BacFile::Save(size_t *psize)
{
    size_t size = CalculateFileSize();

    uint8_t *buf = new uint8_t[size];
    memset(buf, 0, size);

    BACHeader *hdr = (BACHeader *)buf;

    hdr->signature = BAC_SIGNATURE;
    hdr->endianess_check = 0xFFFE;
    hdr->num_entries = (uint32_t) entries.size();
    hdr->data_start = sizeof(BACHeader);
    memcpy(hdr->unk_14, unk_14, sizeof(unk_14));
    memcpy(hdr->unk_20, unk_20, sizeof(unk_20));
    memcpy(hdr->unk_48, unk_48, sizeof(unk_48));
    memcpy(hdr->unk_50, unk_50, sizeof(unk_50));

    BACEntry *file_entries = (BACEntry *)(buf+sizeof(BACHeader));
    BACSubEntry *file_subentry = (BACSubEntry *)(file_entries + entries.size());
    uint8_t *ptr = buf + GetDataStart();

    for (size_t i = 0; i < entries.size(); i++)
    {
        const BacEntry &entry = entries[i];
        file_entries[i].flags = entry.flags;

        if (entry.valid)
        {
            file_entries[i].num_subentries = entry.GetNumSubEntries();
            file_entries[i].sub_entry_offset = Utils::DifPointer(file_subentry, buf);

            for (int j = 0; j <= MAX_BAC_TYPE; j++)
            {
                if (entry.has_type[j])
                {
                    file_subentry->type = j;

                    if (j == 0)
                    {
                        BACAnimation *file_type0 = (BACAnimation *)ptr;
                        file_subentry->num = (uint16_t) entry.type0.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type0.size(); k++)
                        {
                            *file_type0 = entry.type0[k];
                            file_type0++;
                            ptr = (uint8_t *)file_type0;
                        }
                    }
                    else if (j == 1)
                    {
                        BACHitbox *file_type1 = (BACHitbox *)ptr;
                        file_subentry->num = (uint16_t) entry.type1.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type1.size(); k++)
                        {
                            *file_type1 = entry.type1[k];
                            file_type1++;
                            ptr = (uint8_t *)file_type1;
                        }
                    }
                    else if (j == 2)
                    {
                        BACAccelerationMovement *file_type2 = (BACAccelerationMovement *)ptr;
                        file_subentry->num = (uint16_t) entry.type2.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type2.size(); k++)
                        {
                            *file_type2 = entry.type2[k];
                            file_type2++;
                            ptr = (uint8_t *)file_type2;
                        }
                    }
                    else if (j == 3)
                    {
                        BACInvulnerability *file_type3 = (BACInvulnerability *)ptr;
                        file_subentry->num = (uint16_t) entry.type3.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type3.size(); k++)
                        {
                            *file_type3 = entry.type3[k];
                            file_type3++;
                            ptr = (uint8_t *)file_type3;
                        }
                    }
                    else if (j == 4)
                    {
                        BACMotionAdjust *file_type4 = (BACMotionAdjust *)ptr;
                        file_subentry->num = (uint16_t) entry.type4.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type4.size(); k++)
                        {
                            *file_type4 = entry.type4[k];
                            file_type4++;
                            ptr = (uint8_t *)file_type4;
                        }
                    }
                    else if (j == 5)
                    {
                        BACOpponentKnockback *file_type5 = (BACOpponentKnockback *)ptr;
                        file_subentry->num = (uint16_t) entry.type5.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type5.size(); k++)
                        {
                            *file_type5 = entry.type5[k];
                            file_type5++;
                            ptr = (uint8_t *)file_type5;
                        }
                    }
                    else if (j == 6)
                    {
                        BACChainAttackParameters *file_type6 = (BACChainAttackParameters *)ptr;
                        file_subentry->num = (uint16_t) entry.type6.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type6.size(); k++)
                        {
                            *file_type6 = entry.type6[k];
                            file_type6++;
                            ptr = (uint8_t *)file_type6;
                        }
                    }
                    else if (j == 7)
                    {
                        BACBcmCallback *file_type7 = (BACBcmCallback *)ptr;
                        file_subentry->num = (uint16_t) entry.type7.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type7.size(); k++)
                        {
                            *file_type7 = entry.type7[k];
                            file_type7++;
                            ptr = (uint8_t *)file_type7;
                        }
                    }
                    else if (j == 8)
                    {
                        BACEffect *file_type8 = (BACEffect *)ptr;
                        file_subentry->num = (uint16_t) entry.type8.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type8.size(); k++)
                        {
                            *file_type8 = entry.type8[k];
                            file_type8++;
                            ptr = (uint8_t *)file_type8;
                        }
                    }
                    else if (j == 9)
                    {
                        BACProjectile *file_type9 = (BACProjectile *)ptr;
                        file_subentry->num = (uint16_t) entry.type9.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type9.size(); k++)
                        {
                            *file_type9 = entry.type9[k];
                            file_type9++;
                            ptr = (uint8_t *)file_type9;
                        }
                    }
                    else if (j == 10)
                    {
                        BACCamera *file_type10 = (BACCamera *)ptr;
                        file_subentry->num = (uint16_t) entry.type10.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type10.size(); k++)
                        {
                            *file_type10 = entry.type10[k];
                            file_type10++;
                            ptr = (uint8_t *)file_type10;
                        }
                    }
                    else if (j == 11)
                    {
                        BACSound *file_type11 = (BACSound *)ptr;
                        file_subentry->num = (uint16_t) entry.type11.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type11.size(); k++)
                        {
                            *file_type11 = entry.type11[k];
                            file_type11++;
                            ptr = (uint8_t *)file_type11;
                        }
                    }
                    else if (j == 12)
                    {
                        BACType12 *file_type12 = (BACType12 *)ptr;
                        file_subentry->num = (uint16_t) entry.type12.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type12.size(); k++)
                        {
                            *file_type12 = entry.type12[k];
                            file_type12++;
                            ptr = (uint8_t *)file_type12;
                        }
                    }
                    else if (j == 13)
                    {
                        BACPartInvisibility *file_type13 = (BACPartInvisibility *)ptr;
                        file_subentry->num = (uint16_t) entry.type13.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type13.size(); k++)
                        {
                            *file_type13 = entry.type13[k];
                            file_type13++;
                            ptr = (uint8_t *)file_type13;
                        }
                    }
                    else if (j == 14)
                    {
                        BACAnimationModification *file_type14 = (BACAnimationModification *)ptr;
                        file_subentry->num = (uint16_t) entry.type14.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type14.size(); k++)
                        {
                            *file_type14 = entry.type14[k];
                            file_type14++;
                            ptr = (uint8_t *)file_type14;
                        }
                    }
                    else if (j == 15)
                    {
                        BACTransformControl *file_type15 = (BACTransformControl *)ptr;
                        file_subentry->num = (uint16_t) entry.type15.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type15.size(); k++)
                        {
                            *file_type15 = entry.type15[k];
                            file_type15++;
                            ptr = (uint8_t *)file_type15;
                        }
                    }
                    else if (j == 16)
                    {
                        BACScreenEffect *file_type16 = (BACScreenEffect *)ptr;
                        file_subentry->num = (uint16_t) entry.type16.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type16.size(); k++)
                        {
                            *file_type16 = entry.type16[k];
                            file_type16++;
                            ptr = (uint8_t *)file_type16;
                        }
                    }
                    else if (j == 17)
                    {
                        BACThrowHandler *file_type17 = (BACThrowHandler *)ptr;
                        file_subentry->num = (uint16_t) entry.type17.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type17.size(); k++)
                        {
                            if (size17_small)
                            {
                                file_type17->start_time = entry.type17[k].start_time;
                                file_type17->duration = entry.type17[k].duration;
                                file_type17->unk_04 = entry.type17[k].unk_04;
                                file_type17->th_flags = entry.type17[k].th_flags;
                                file_type17->unk_0A = entry.type17[k].unk_0A;
                                file_type17->bone_user_connects_to_victim_from = entry.type17[k].bone_user_connects_to_victim_from;
                                file_type17->bone_victim_connects_to_user_from = entry.type17[k].bone_victim_connects_to_user_from;
                                file_type17->bac_entry = entry.type17[k].bac_entry;
                                file_type17->unk_12 = entry.type17[k].unk_12;

                                ptr += 0x14;
                                file_type17 = (BACThrowHandler *)ptr;
                            }
                            else
                            {
                                *file_type17 = entry.type17[k];
                                file_type17++;
                                ptr = (uint8_t *)file_type17;
                            }
                        }
                    }
                    else if (j == 18)
                    {
                        BACType18 *file_type18 = (BACType18 *)ptr;
                        file_subentry->num = (uint16_t) entry.type18.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type18.size(); k++)
                        {
                            *file_type18 = entry.type18[k];
                            file_type18++;
                            ptr = (uint8_t *)file_type18;
                        }
                    }
                    else if (j == 19)
                    {
                        BACAuraEffect *file_type19 = (BACAuraEffect *)ptr;
                        file_subentry->num = (uint16_t) entry.type19.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type19.size(); k++)
                        {
                            *file_type19 = entry.type19[k];
                            file_type19++;
                            ptr = (uint8_t *)file_type19;
                        }
                    }
                    else if (j == 20)
                    {
                        BACHomingMovement *file_type20 = (BACHomingMovement *)ptr;
                        file_subentry->num = (uint16_t) entry.type20.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type20.size(); k++)
                        {
                            *file_type20 = entry.type20[k];
                            file_type20++;
                            ptr = (uint8_t *)file_type20;
                        }
                    }
                    else if (j == 21)
                    {
                        BACType21 *file_type21 = (BACType21 *)ptr;
                        file_subentry->num = (uint16_t) entry.type21.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type21.size(); k++)
                        {
                            *file_type21 = entry.type21[k];
                            file_type21++;
                            ptr = (uint8_t *)file_type21;
                        }
                    }
                    else if (j == 22)
                    {
                        BACType22 *file_type22 = (BACType22 *)ptr;
                        file_subentry->num = (uint16_t) entry.type22.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type22.size(); k++)
                        {
                            *file_type22 = entry.type22[k];
                            file_type22++;
                            ptr = (uint8_t *)file_type22;
                        }
                    }
                    else if (j == 23)
                    {
                        BACTransparencyEffect *file_type23 = (BACTransparencyEffect *)ptr;
                        file_subentry->num = (uint16_t) entry.type23.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type23.size(); k++)
                        {
                            *file_type23 = entry.type23[k];
                            file_type23++;
                            ptr = (uint8_t *)file_type23;
                        }
                    }
                    else if (j == 24)
                    {
                        BACDualSkillData *file_type24 = (BACDualSkillData *)ptr;
                        file_subentry->num = (uint16_t) entry.type24.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type24.size(); k++)
                        {
                            *file_type24 = entry.type24[k];
                            file_type24++;
                            ptr = (uint8_t *)file_type24;
                        }
                    }
                    else if (j == 25)
                    {
                        BACType25 *file_type25 = (BACType25 *)ptr;
                        file_subentry->num = (uint16_t) entry.type25.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type25.size(); k++)
                        {
                            *file_type25 = entry.type25[k];
                            file_type25++;
                            ptr = (uint8_t *)file_type25;
                        }
                    }
                    else if (j == 26)
                    {
                        BACType26 *file_type26 = (BACType26 *)ptr;
                        file_subentry->num = (uint16_t) entry.type26.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type26.size(); k++)
                        {
                            *file_type26 = entry.type26[k];
                            file_type26++;
                            ptr = (uint8_t *)file_type26;
                        }
                    }
                    else if (j == 27)
                    {
                        BACType27 *file_type27 = (BACType27 *)ptr;
                        file_subentry->num = (uint16_t) entry.type27.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type27.size(); k++)
                        {
                            *file_type27 = entry.type27[k];
                            file_type27++;
                            ptr = (uint8_t *)file_type27;
                        }
                    }
                    else if (j == 28)
                    {
                        BACType28 *file_type28 = (BACType28 *)ptr;
                        file_subentry->num = (uint16_t) entry.type28.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type28.size(); k++)
                        {
                            *file_type28 = entry.type28[k];
                            file_type28++;
                            ptr = (uint8_t *)file_type28;
                        }
                    }
                    else if (j == 29)
                    {
                        BACType29 *file_type29 = (BACType29 *)ptr;
                        file_subentry->num = (uint16_t) entry.type29.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type29.size(); k++)
                        {
                            *file_type29 = entry.type29[k];
                            file_type29++;
                            ptr = (uint8_t *)file_type29;
                        }
                    }
                    else if (j == 30)
                    {
                        BACType30 *file_type30 = (BACType30 *)ptr;
                        file_subentry->num = (uint16_t) entry.type30.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type30.size(); k++)
                        {
                            *file_type30 = entry.type30[k];
                            file_type30++;
                            ptr = (uint8_t *)file_type30;
                        }
                    }
                    else if (j == 31)
                    {
                        BACType31 *file_type31 = (BACType31 *)ptr;
                        file_subentry->num = (uint16_t) entry.type31.size();

                        if (file_subentry->num != 0)
                            file_subentry->offset = Utils::DifPointer(ptr, buf);

                        for (size_t k = 0; k < entry.type31.size(); k++)
                        {
                            *file_type31 = entry.type31[k];
                            file_type31++;
                            ptr = (uint8_t *)file_type31;
                        }
                    }
                    else if (j == 13)
                    {
                        // Nothing
                    }
                    else
                    {
                        DPRINTF("%s Internal error.\n", FUNCNAME);
                        return nullptr;
                    }

                    file_subentry++;
                }
            }
        }
    }    

    *psize = size;
    return buf;
}

TiXmlDocument *BacFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("BAC");

    Utils::WriteComment(root, "FLAG (applies to most types, unless a specific comment). Flag 1 -> entry applies to CAC; Flag 2 -> entry applies to roster character (that includes pseudo-cacs of cac2x2m)");

    if (size17_small)
        Utils::WriteParamUnsigned(root, "SIZE17_SMALL", 1);

    Utils::WriteParamMultipleUnsigned(root, "U_14", std::vector<uint32_t>(unk_14, unk_14+3), true);
    Utils::WriteParamMultipleFloats(root, "F_20", std::vector<float>(unk_20, unk_20+10));
    Utils::WriteParamMultipleFloats(root, "F_48", std::vector<float>(unk_48, unk_48+2));
    Utils::WriteParamMultipleUnsigned(root, "U_50", std::vector<uint32_t>(unk_50, unk_50+4), true);

    for (size_t i = 0; i < entries.size(); i++)
    {
        entries[i].Decompile(root, size17_small, (int)i);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool BacFile::Compile(TiXmlDocument *doc, bool )
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "BAC");

    if (!root)
    {
        DPRINTF("Cannot find\"BAC\" in xml.\n");
        return false;
    }

    uint32_t temp;
    if (Utils::ReadParamUnsigned(root, "SIZE17_SMALL", &temp) && temp)
        size17_small = true;

    if (!Utils::GetParamMultipleUnsigned(root, "U_14", unk_14, 3))
        return false;

    if (!Utils::GetParamMultipleFloats(root, "F_20", unk_20, 10))
        return false;

    if (!Utils::GetParamMultipleFloats(root, "F_48", unk_48, 2))
        return false;

    if (!Utils::GetParamMultipleUnsigned(root, "U_50", unk_50, 4))
        return false;

    size_t n = Utils::GetElemCount(root, "BacEntry");
    entries.resize(n);

    std::vector<bool> used;
    used.resize(n, false);

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "BacEntry")
        {
            BacEntry entry;
            uint32_t idx;

            if (!entry.Compile(elem, size17_small))
                return false;

            if (!Utils::ReadAttrUnsigned(elem, "idx", &idx))
            {
                DPRINTF("%s: Attribute idx is not optional (BacEntry).\n", FUNCNAME);
                return false;
            }

            if (idx >= entries.size())
            {
                DPRINTF("%s: Index %d is out of bounds.\n", FUNCNAME, idx);
                return false;
            }

            if (used[idx])
            {
                DPRINTF("%s: Entry with index %d is defined multiple times.\n", FUNCNAME, idx);
                return false;
            }

            entries[idx] = entry;
            used[idx] = true;
        }
    }

    return true;
}

size_t BacFile::ChangeReferencesToSkill(uint16_t old_skill, uint16_t new_skill)
{
    size_t count = 0;
    float old_skill_float = (float) old_skill;
    float new_skill_float = (float) new_skill;

    for (BacEntry &entry : entries)
    {
        if (!entry.valid)
            continue;

        for (BACEffect &t8: entry.type8)
        {
            if (t8.type != 0 && t8.skill_id == old_skill)
            {
                t8.skill_id = new_skill;
                count++;
            }
        }

        for (BACProjectile &t9: entry.type9)
        {
            if (t9.skill_id == old_skill)
            {
                t9.skill_id = new_skill;
                count++;
            }
        }

        for (BACTransformControl &t15 : entry.type15)
        {
            if (t15.type == 0x25 || t15.type == 0x26)
            {
                if (t15.unk_10[1] == old_skill_float)
                {
                    t15.unk_10[1] = new_skill_float;
                    count++;
                }
            }
            else if (t15.type == 0x4E)
            {
                if (t15.parameter == old_skill_float)
                {
                    t15.parameter = new_skill_float;
                    count++;
                }
            }
        }

        for (BACType27 &t27: entry.type27)
        {
            if (t27.skill_id == old_skill)
            {
                t27.skill_id = new_skill;
                count++;
            }
        }
    }

    return count;
}
