#include "KtidFile.h"
#include "debug.h"

#include "small_dictionary.h"

#include <algorithm>

enum class Hint
{
    HINT_NONE,
    HINT_COS,
    HINT_HAIR,
    HINT_FACE
};

static uint32_t hash_func(const char *a1, int a2, int a3)
{
  int v3;

  while ( 1 )
  {
    v3 = a3;

    char c = *a1;
    if (!c)
        break;

    a3 *= 31;
    a2 += 31 * v3 * c;
    a1++;
  }
  return (uint32_t)a2;
}

static uint32_t hash_func(const char *a1)
{
    return hash_func(a1+1, a1[0]*31, 31);
}

static uint32_t hash_func(const std::string &s)
{
    return hash_func(s.c_str());
}

static std::string get_model_name(const std::string &path, bool remove_underscore, bool num_terminated, bool convert_to_uppercase)
{
    std::string fn = Utils::GetFileNameString(path);
    std::string mn;

    size_t pos = fn.find('.');
    if (pos == std::string::npos)
        mn = fn;
    else
        mn = fn.substr(0, pos);

    if (Utils::BeginsWith(fn, "MPR_Muscle_Character_", false))
    {
        pos = mn.find('_', 21);
        if (pos != std::string::npos)
        {
            mn = mn.substr(21, pos-21);
        }
    }

    if (num_terminated)
    {
        while (mn.length() > 0 && !isdigit(mn.back()))
            mn.pop_back();
    }

    if (remove_underscore)
        Utils::Replace(mn, "_", "");

    if (convert_to_uppercase)
        mn = Utils::ToUpperCase(mn);

    return mn;
}

static bool crack_hash_full(uint32_t hash, std::string &ret, const std::string &fail_name, Hint hint=Hint::HINT_NONE)
{
    std::vector<std::string> types;

    if (hint == Hint::HINT_COS)
        types.push_back("COS");
    else if (hint == Hint::HINT_HAIR)
        types.push_back("HAIR");
    else if (hint == Hint::HINT_FACE)
        types.push_back("FACE");
    else
    {
        types.push_back("COS");
        types.push_back("HAIR");
        types.push_back("FACE");
    }

    static std::string saved_char;
    static int saved_i = -1;

    std::vector<std::string> chars = char_codes;

    if (saved_char.length() > 0)
    {
        if (chars[0] != saved_char)
        {
            auto it = std::find(chars.begin(), chars.end(), saved_char);
            if (it != chars.end())
                chars.erase(it);

            chars.insert(chars.begin(), saved_char);
        }
    }

    for (const std::string &ch : chars)
    {
        //DPRINTF("Testing %s\n", ch.c_str());

        for (int i = -1; i <= 150; i++)
        {
            int n = i;

            if (n == -1)
            {
                if (saved_i < 0)
                    continue;

                n = saved_i;
            }

            std::string num_str = Utils::ToStringAndPad(n, 3) + "_";
            std::string num_str2 = std::string("_") + num_str;

            for (const std::string &type : types)
            {
                std::string type_str = type + num_str;
                std::string str = "CE1ResourceStaticTexture［MPR_Muscle_Character_" + ch + type_str;
                std::string pre = ch + "_" + type + num_str2 + "MPR_Muscle_Character_";
                std::string str2 = "CE1ResourceStaticTexture［" + pre + ch + type_str;

                for (const std::string &w : small_dictionary)
                {
                    std::string sw = str + w;
                    std::string sw2 = str2 + w;

                    for (const std::string &tt : texture_types)
                    {
                        ret = sw + "_" + tt + "］";
                        if (hash_func(ret) == hash)
                        {
                            saved_char = ch;
                            saved_i = n;
                            return true;
                        }

                        //DPRINTF("Testing %s\n", ret.c_str());

                        ret = sw + "_BLEND_" + tt + "］";
                        if (hash_func(ret) == hash)
                        {
                            saved_char = ch;
                            return true;
                        }

                        ret = sw2 + "_" + tt + "］";
                        if (hash_func(ret) == hash)
                        {
                            saved_char = ch;
                            saved_i = n;
                            return true;
                        }

                        //DPRINTF("Testing %s\n", ret.c_str());

                        ret = sw2 + "_BLEND_" + tt + "］";
                        if (hash_func(ret) == hash)
                        {
                            saved_char = ch;
                            saved_i = n;
                            return true;
                        }
                    }
                }
            }
        }
    }

    ret = fail_name;
    return false;
}

static bool crack_hash(const std::string &path, uint32_t hash, std::string &ret, const std::string &fail_name)
{
    std::string mn = get_model_name(path, true, false, true);
    std::string mn2 = get_model_name(path, true, true, true);
    std::string mn3 = get_model_name(path,  false, true, true);
    std::vector<std::string> crack_list;

    crack_list.push_back("MPR_Muscle_Character_" + mn + "_");

    if (mn2.length() > 0 && mn2 != mn)
        crack_list.push_back("MPR_Muscle_Character_" + mn2 + "_");

    crack_list.push_back(mn3 + "_MPR_Muscle_Character_" + mn2 + "_");

    for (const std::string &str : crack_list)
    {
        for (const std::string &w : small_dictionary)
        {
            for (const std::string &tt : texture_types)
            {
                ret = "CE1ResourceStaticTexture［" + str + w + "_" + tt + "］";
                if (hash_func(ret) == hash)
                    return true;

                //DPRINTF("Testing %s\n", ret.c_str());

                ret = "CE1ResourceStaticTexture［" + str + w + "_BLEND_" + tt + "］";
                if (hash_func(ret) == hash)
                    return true;
            }

            ret = "MPR_Muscle_Character_" + str + "_" + w;
            if (hash_func(ret) == hash)
                return true;

            ret = "MPR_Muscle_Character_" + str + "_" + w + "_BLEND";
            if (hash_func(ret) == hash)
                return true;
        }
    }

    ret = fail_name;
    return false;
}

KtidFile::KtidFile()
{
    this->big_endian = false;
}

KtidFile::~KtidFile()
{

}

void KtidFile::Reset()
{
    textures.clear();
}

bool KtidFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (size & 7)
    {
        DPRINTF("Ktid binary file size must be multiple of 8.\n");
        return false;
    }

    FixedMemoryStream mem(const_cast<uint8_t *>(buf), size);

    textures.resize(size / 8);

    for (auto &it: textures)
    {
        if (!mem.Read32(&it.first) || !mem.Read32(&it.second))
            return false;
    }

    return true;
}

uint8_t *KtidFile::Save(size_t *psize)
{
    MemoryStream mem;

    for (auto &it: textures)
    {
        mem.Write32(it.first);
        mem.Write32(it.second);
    }

    *psize = (size_t)mem.GetSize();
    return mem.GetMemory(true);
}

bool KtidFile::LoadFromTextFile(const std::string &path)
{
    std::string text;
    std::vector<std::string> lines;

    Reset();

    if (!Utils::ReadTextFile(path, text))
        return false;

    Utils::GetMultipleStrings(text, lines, '\n');

    for (std::string &line : lines)
    {
        Utils::TrimString(line);

        if (line.length() == 0 || line.front() == ';' || line == "\xEF\xBB\xBF;")
            continue;

        std::vector<std::string> components;

        Utils::GetMultipleStrings(line, components, ',', false);
        if (components.size() != 2)
        {
            DPRINTF("Failed to decode line \"%s\"\n", line.c_str());
            return false;
        }

        textures.push_back(std::pair<uint32_t, uint32_t>(Utils::GetUnsigned(components[0]), hash_func(components[1])));
    }

    return true;
}

bool KtidFile::SaveToTextFile(const std::string &path, bool *crack_success, bool full_crack)
{
    bool is_me = false;

    if (crack_success)
        *crack_success = true;

    if (Utils::BeginsWith(Utils::GetFileNameString(path), "MPR_Muscle_Character_"))
        is_me = true;

    FILE *f = fopen(path.c_str(), "w");
    if (!f)
        return false;

    Hint hint = Hint::HINT_NONE;

    if (full_crack)
    {
        if (path.find("COS") != std::string::npos)
            hint = Hint::HINT_COS;
        else if (path.find("HAIR") != std::string::npos)
            hint = Hint::HINT_HAIR;
        else if (path.find("FACE") != std::string::npos)
            hint = Hint::HINT_FACE;
    }

    if (is_me)
        fprintf(f, "\xEF\xBB\xBF;\n");
    else
        fprintf(f, ";\n");

    fprintf(f, "; texture index,filename\n;\n");

    for (auto &it : textures)
    {
        std::string fail_name = "HASH_" + Utils::UnsignedToString(it.second, true);
        std::string cracked_name;

        if (!crack_hash(path, it.second, cracked_name, fail_name))
        {
            if (full_crack)
            {
                if (!crack_hash_full(it.second, cracked_name, fail_name, hint) && crack_success)
                    *crack_success = false;
            }
            else if (crack_success)
            {
                *crack_success = false;
            }
        }

        fprintf(f, "%d,%s\n", it.first, cracked_name.c_str());
    }

    if (!is_me)
        fprintf(f, "\n");

    fclose(f);
    return true;
}
