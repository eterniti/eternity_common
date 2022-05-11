#include <unordered_map>
#include "BPEboot.h"

struct VarParams
{
    off64_t offset;
    size_t len; // Not counting \0
    bool integer;
};

static const std::unordered_map<std::string, VarParams> bp_strings =
{
    // Strings
    { "KEY_CONFIG", { 0x6E9DA4, 0x15, false } },
    { "SOUND_SETTINGS", { 0x6E9DBC, 0x12, false } },
    { "TITLE_RETURN", { 0x6E9DD0, 0x15, false } },
    { "RESTORE_DEFAULT1", { 0x6E9910, 0x18, false} },
    { "RESTORE_DEFAULT2", { 0x6E9EC4, 0x18, false} },
    { "CAMERA_CONTROL", { 0x6E9844, 0xF, false } },
    { "CAMERA_ROTDIR", { 0x6E9854, 0x15, false } },
    { "SOULGEM_USE", { 0x6E986C, 0x18, false } },
    { "ATTACK", { 0x6E9888, 0xC, false } },
    { "SPECIAL_MAGIC", { 0x6E9898, 0x1B, false } },
    { "JUMP", { 0x6E98B4, 0xC, false } },
    { "DASH_STEP", { 0x6E98C4, 0x19, false} },
    { "LOCK_ON_OFF", { 0x6E98E0, 0x16, false } },
    { "CAMERA_RESET", { 0x6E98F8, 0x15, false } },
    { "DIRECTIONAL_KEY", { 0x6E992C, 0xC, false } },
    { "NORMAL", { 0x6E993C, 0xC, false } },
    { "RIGHT_STICK", { 0x6E9954, 0x12, false } },
    { "LEFT_RIGHT_REV", { 0x6E9968, 0xC, false } },
    { "UP_DOWN_REV", { 0x6E9978, 0xC, false } },
    { "ALL_REV", { 0x6E9988, 0x12, false } },
    { "DKEY_DOWN", { 0x6E999C, 0xF, false } },
    { "DKEY_UP", { 0x6E99AC, 0xF, false } },
    { "RIGHT_STICK_DOWN", { 0x6E99BC, 0x15, false } },
    { "RIGHT_STICK_UP", { 0x6E99D4, 0x15, false } },
    { "CHOOSE_PLAYER", { 0x6D4DA8, 0x15, false } },
    { "CHOOSE_PARTNER", { 0x6D4DC0, 0x15, false } },
    { "START_STAGE", { 0x6D4DD8, 0x12, false } },
    { "TIME_SCORE", { 0x6D44A0, 0x12, false } },
    { "HIT_POINT_SCORE", { 0x6D44B4, 0x1E, false } },
    { "TOTAL_SCORE", { 0x6D44D4, 0x15, false } },
    { "TOTAL_DEFEATED", { 0x6D44EC, 0x18, false } },
    { "EARNED_EXPERIENCE", { 0x6D4508, 0xF, false } },
    { "LEVEL_N", { 0x6D214C, 0xB, false } },
    { "SPECIAL_MAGIC_STATUS", { 0x6DCED0, 0x1B, false } },
    { "SUPPORT_SKILL", { 0x6DD4B0, 0x15, false } },

    // Integers
    { "LANGUAGE_CODE", { 0x315268, 1, true }, },
};

BPEboot::BPEboot()
{
    elf = nullptr;
}

void BPEboot::Reset()
{
    if (elf)
    {
        delete elf;
        elf = nullptr;

        if (elf_path.length() > 0)
            Utils::RemoveFile(elf_path);
    }
}

BPEboot::~BPEboot()
{
    Reset();
}

bool BPEboot::LoadFromFile(const std::string &path, bool show_error)
{
    Reset();

    if (decoder_bin.length() == 0)
    {
        if (show_error)
            DPRINTF("%s: decoder not set.\n", FUNCNAME);

        return false;
    }

    if (!Utils::FileExists(decoder_bin))
    {
        if (show_error)
            DPRINTF("%s: decoder program \"%s\" doesn't exist.\n", FUNCNAME, decoder_bin.c_str());

        return false;
    }

    self_path = path;
    elf_path = path + ".elf";

    int ret = Utils::RunProgram(decoder_bin, { self_path });
    if (ret != 0)
    {
        if (show_error)
            DPRINTF("%s: cannot run program \"%s\" or it failed.\n", FUNCNAME, decoder_bin.c_str());

        return false;
    }

    elf = new FileStream();
    if (!elf->LoadFromFile(elf_path))
    {
        if (show_error)
            DPRINTF("%s: elf load failed.\n", FUNCNAME);

        return false;
    }

    const uint64_t qword_check = 0xA00036AE58B4248;
    const off64_t offset_check = 0x10070;
    uint64_t check;

    elf->SavePos();
    if (!elf->Seek(offset_check, SEEK_SET) || !elf->Read64(&check) || check != qword_check)
    {
        DPRINTF("%s: Invalid eboot. This is either not the BattlePentagram eboot, or it wasn't decrypted properly or it is a different version.\n", FUNCNAME);
        return false;
    }
    elf->RestorePos();

    return true;
}

bool BPEboot::SaveToFile(const std::string &path, bool show_error, bool build_path)
{
    if (injector_bin.length() == 0)
    {
        if (show_error)
            DPRINTF("%s: injector not set.\n", FUNCNAME);

        return false;
    }

    if (!Utils::FileExists(injector_bin))
    {
        if (show_error)
            DPRINTF("%s: injector program \"%s\" doesn't exist.\n", FUNCNAME, injector_bin.c_str());

        return false;
    }

    if (Utils::ToLowerCase(path) != Utils::ToLowerCase(self_path) && !Utils::DoCopyFile(self_path, path, build_path))
    {
        if (show_error)
            DPRINTF("%s: Failed to create faile \"%s\".\n", FUNCNAME, path.c_str());

        return false;
    }

    delete elf; // Save elf
    elf = nullptr;

    int ret = Utils::RunProgram(injector_bin, { path, elf_path });
    if (ret != 0)
    {
        if (show_error)
            DPRINTF("%s: cannot run program \"%s\" or it failed.\n", FUNCNAME, injector_bin.c_str());

        return false;
    }

    elf = new FileStream();
    return elf->LoadFromFile(elf_path, show_error);
}

bool BPEboot::VarExists(const std::string &var) const
{
    return (bp_strings.find(var) != bp_strings.end());
}

bool BPEboot::VarFits(const std::string &var, const std::string &value) const
{
    auto it = bp_strings.find(var);
    if (it == bp_strings.end())
        return false;

    if (it->second.integer)
        return true;

    if (value.length() > it->second.len)
        return false;

    return true;
}

bool BPEboot::SetVar(const std::string &var, const std::string &value)
{
    if (!elf)
        return false;

    auto it = bp_strings.find(var);
    if (it == bp_strings.end())
        return false;

    if (!elf->Seek(it->second.offset, SEEK_SET))
        return false;

    if (it->second.integer)
    {
        int32_t ival32 = Utils::GetSigned(value, -1);
        if (it->second.len == 1)
        {
            int8_t ival8 = (int8_t)ival32;
            if (!elf->Write8((uint8_t)ival8))
                return false;
        }
        else if (it->second.len == 2)
        {
            int16_t ival16 = (int8_t)ival32;
            if (!elf->Write16((uint16_t)ival16))
                return false;
        }
        else if (it->second.len == 4)
        {
            if (!elf->Write32((uint32_t)ival32))
                return false;
        }
        else
        {
            DPRINTF("%s: should not be here.\n", FUNCNAME);
            return false;
        }
    }
    else
    {
        size_t len = value.length();
        if (len > it->second.len)
            return false;

        if (!elf->WriteString(value, true))
            return false;

        // Clean remaining bytes -if any- with 'F'
        for (size_t i = len+1; i < it->second.len; i++)
            if (!elf->Write8('F'))
                return false;
    }

    return true;
}
