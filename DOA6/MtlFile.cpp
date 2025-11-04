#include <stdio.h>
#include <unordered_map>

#include "MtlFile.h"
#include "debug.h"

#include "small_dictionary.h"

static std::unordered_map<uint32_t, std::string> names_map;

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
        /*types.push_back("COS");
        types.push_back("HAIR");
        types.push_back("FACE");*/
        return false;
    }

    for (int i = 0; i <= 150; i++)
    {
        std::string num_str = Utils::ToStringAndPad(i, 3);

        for (const std::string &type : types)
        {
            std::string type_str = type + num_str;

            for (const std::string &ch : char_codes)
            {
                std::string str = ch + type_str;

                for (const std::string &w : small_dictionary)
                {
                    ret = "MPR_Muscle_Character_" + str + "_" + w;
                    if (hash_func(ret) == hash)
                        return true;

                    ret = "MPR_Muscle_Character_" + str + "_" + w + "_BLEND";
                    if (hash_func(ret) == hash)
                        return true;
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
    std::vector<std::string> crack_list;

    crack_list.push_back(mn);

    if (mn2.length() > 0)
        crack_list.push_back(mn2);

    for (const std::string &str : crack_list)
    {
        for (const std::string &w : small_dictionary)
        {
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

MtlFile::MtlFile()
{
    this->big_endian = false;
}

MtlFile::~MtlFile()
{

}

void MtlFile::Reset()
{
    names.clear();
    mat_ids.clear();
    cloths.clear();
    ponytails.clear();
}

bool MtlFile::Load(const uint8_t *buf, size_t size)
{
    Reset();
    FixedMemoryStream mem(const_cast<uint8_t *>(buf), size);

    uint32_t num_names, num_mat;

    if (!mem.Read32(&num_names) || !mem.Read32(&num_mat))
        return false;

    names.resize(num_names);
    mat_ids.resize(num_names); // Not an error, ignore num_mat

    uint32_t num_cloths, num_ponytails;

    if (!mem.Read32(&num_cloths) || !mem.Read32(&num_ponytails))
        return false;

    for (size_t i = 0; i < names.size(); i++)
    {
        uint32_t num;

        if (!mem.Read32(&names[i]))
            return false;

        if (!mem.Read32(&num))
            return false;

        mat_ids[i].resize(num);

        for (uint32_t &m : mat_ids[i])
        {
            if (!mem.Read32(&m))
                return false;
        }
    }

    cloths.resize(num_cloths);

    for (auto &it: cloths)
    {
        if (!mem.Read32(&it.first) || !mem.Read32(&it.second))
            return false;
    }

    ponytails.resize(num_ponytails);

    for (auto &it: ponytails)
    {
        if (!mem.Read32(&it.first) || !mem.Read32(&it.second))
            return false;
    }

    return true;
}

uint8_t *MtlFile::Save(size_t *psize)
{
    MemoryStream mem;
    uint32_t higher_mat = 0;

    for (auto &it : mat_ids)
    {
        for (uint32_t m : it)
        {
            if (m > higher_mat)
                higher_mat = m;
        }
    }

    mem.Write32((uint32_t)names.size());
    mem.Write32(higher_mat+1);
    mem.Write32((uint32_t)cloths.size());
    mem.Write32((uint32_t)ponytails.size());

    for (size_t i = 0; i < names.size(); i++)
    {
        mem.Write32(names[i]);
        mem.Write32((uint32_t)mat_ids[i].size());

        for (uint32_t m : mat_ids[i])
        {
            mem.Write32(m);
        }
    }

    for (auto &it : cloths)
    {
        mem.Write32(it.first);
        mem.Write32(it.second);
    }

    for (auto &it : ponytails)
    {
        mem.Write32(it.first);
        mem.Write32(it.second);
    }

    *psize = (size_t) mem.GetSize();
    return mem.GetMemory(true);
}

bool MtlFile::LoadFromTextFile(const std::string &path)
{
    std::string text;
    std::vector<std::string> lines;
    bool in_section2 = false;

    Reset();

    if (!Utils::ReadTextFile(path, text))
        return false;

    Utils::GetMultipleStrings(text, lines, '\n');

    for (std::string &line : lines)
    {
        Utils::TrimString(line);

        if (line.length() == 0 || line.front() == ';')
            continue;

        if (!in_section2)
        {
            if (line.front() == '#')
            {
                in_section2 = true;
                continue;
            }

            std::vector<std::string> components;

            Utils::GetMultipleStrings(line, components, ',', false);
            if (components.size() < 2)
            {
                DPRINTF("Failed to decode line \"%s\"\n", line.c_str());
                return false;
            }

            std::vector<uint32_t> mat_id;

            if (Utils::BeginsWith(components[0], "HASH_", false))
            {
                names.push_back(Utils::GetUnsigned(components[0].substr(5)));
            }
            else
            {
                names.push_back(hash_func(components[0]));
            }

            for (size_t i = 1; i < components.size(); i++)
            {
                mat_id.push_back(Utils::GetUnsigned(components[i]));
            }

            mat_ids.push_back(mat_id);
        }
        else
        {
            if (line.front() == '#')
                continue;

            std::vector<std::string> components;

            Utils::GetMultipleStrings(line, components, ',', false);
            if (components.size() != 3)
            {
                DPRINTF("Failed to decode line \"%s\"\n", line.c_str());
                return false;
            }

            bool is_ponytail = (Utils::GetUnsigned(components[2]) != 0);

            if (is_ponytail)
            {
                ponytails.push_back(std::pair<uint32_t, uint32_t>(Utils::GetUnsigned(components[0]), Utils::GetUnsigned(components[1])));
            }
            else
            {
                cloths.push_back(std::pair<uint32_t, uint32_t>(Utils::GetUnsigned(components[0]), Utils::GetUnsigned(components[1])));
            }
        }
    }

    return true;
}

bool MtlFile::SaveToTextFile(const std::string &path, bool *crack_success, bool full_crack)
{
    if (crack_success)
        *crack_success = true;

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

    fprintf(f, ";\n; Material name,matpalid[0],matpalid[1],matpalid[2],...\n;\n");

    for (size_t i = 0; i < names.size();  i++)
    {
        std::string fail_name = "HASH_" + Utils::UnsignedToString(names[i], true);
        std::string cracked_name;

        auto it = names_map.find(names[i]);
        if (it == names_map.end())
        {
            if (!crack_hash(path, names[i], cracked_name, fail_name))
            {
                if (full_crack)
                {
                    if (!crack_hash_full(names[i], cracked_name, fail_name, hint) && crack_success)
                        *crack_success = false;
                }
                else if (crack_success)
                {
                     *crack_success = false;
                }
            }
        }
        else
        {
            cracked_name = it->second;
        }

        fprintf(f, "%s", cracked_name.c_str());

        for (uint32_t m : mat_ids[i])
        {
            fprintf(f, ",%u", m);
        }

        fprintf(f, "\n");
    }

    fprintf(f, "\n###\n\n;\n; cloth id in g1m,cloth id in XSI,cloth type(0:Cloth,1:Ponytail)\n;\n");

    for (auto &it : cloths)
    {
        fprintf(f, "%u,%u,0\n", it.first, it.second);
    }

    for (auto &it : ponytails)
    {
        fprintf(f, "%u,%u,1\n", it.first, it.second);
    }

    fclose(f);
    return true;
}

static bool get_hex_name(const std::string &fn, uint32_t *ret)
{
    if (!Utils::BeginsWith(fn, "0x", false))
        return false;

    if (fn.length() == 2)
        return false;

    for (size_t i = 2; i < fn.length(); i++)
    {
        char ch = (char)tolower(fn[i]);

        if (ch == '.')
            break;

        bool ok = false;

        if (ch >= '0' && ch <= '9')
            ok = true;
        else if (ch >= 'a' && ch <= 'f')
            ok = true;

        if (!ok)
            return false;
    }

    *ret = Utils::GetUnsigned(fn);
    return true;
}

bool MtlFile::LoadNamesFileBuf(const char *buf)
{
    // New implementaton asumes there are no inner spaces in filenames
    char comp1[32], comp2[256];
    bool in_comment = false;
    bool in_c2 = false;

    size_t len = strlen(buf);
    size_t c1 = 0, c2 = 0;
    int line_num = 1;


    for (size_t i = 0; i < len; i++)
    {
        char ch = buf[i];

        if (ch == '\n')
        {
            if (in_comment)
            {
                in_comment = false;
            }
            else if (c1 != 0)
            {
                if (!in_c2)
                    DPRINTF("%s: Faulty line %d.\n", FUNCNAME, line_num);

                comp1[c1] = 0;
                comp2[c2] = 0;

                uint32_t id;

                if (!get_hex_name(comp1, &id))
                {
                    DPRINTF("%s: parse error: \"%s\" is not a valid hash. (line %d)\n", FUNCNAME, comp1, line_num);
                    return false;
                }

                names_map[id] = comp2;
            }

            c1 = c2 = 0;
            in_c2 = false;
            line_num++;
            continue;
        }

        if (in_comment)
            continue;

        if (ch >= 0 && ch <= ' ')
            continue;

        if (c1 == 0 && (ch == ';' || ch == '#'))
        {
            in_comment = true;
            continue;
        }

        if (!in_c2 && ch == ',')
        {
            in_c2 = true;
            continue;
        }

        if (in_c2)
            comp2[c2++] = ch;
        else
            comp1[c1++] = ch;
    }

    UPRINTF("Map loaded with %Id values.\n", names_map.size());
    return true;
}

bool MtlFile::LoadNamesFile(const std::string &path)
{
    size_t size = Utils::GetFileSize(path);
    if (size == (size_t)-1)
    {
        DPRINTF("Cannot stat file \"%s\"", path.c_str());
        return false;
    }

    uint8_t *buf = new uint8_t[size+1];
    buf[size] = 0;

    FILE *f = fopen(path.c_str(), "rb");
    bool ret = false;

    if (f)
    {
        ret = (fread(buf, 1, size, f) == size);
        if (ret)
        {
            ret = LoadNamesFileBuf((const char *)buf);
        }

        fclose(f);
    }
    else
    {
        DPRINTF("Cannot open file \"%s\"", path.c_str());
    }

    delete[] buf;
    return ret;
}

bool MtlFile::LoadNamesFileBinBuf(const uint8_t *buf)
{
    const uint8_t *ptr = buf;
    uint32_t num = *(const uint32_t *)ptr;

    names_map.reserve(num);
    ptr += 4;

    for (size_t i = 0; i < (size_t)num; i++)
    {
        std::string str;

        uint32_t hash = *(const uint32_t *)ptr;
        ptr += 4;

        names_map[hash] = (const char *)ptr;
        ptr += strlen((const char *)ptr) + 1;
    }


    UPRINTF("Map loaded with %Id values.\n", names_map.size());

    return true;
}

bool MtlFile::LoadNamesFileBin(const std::string &path)
{
    size_t size;
    uint8_t *buf = Utils::ReadFile(path, &size);
    if (!buf)
        return false;

    bool ret = LoadNamesFileBinBuf(buf);

    delete[] buf;
    return ret;
}


