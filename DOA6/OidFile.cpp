#include "OidFile.h"
#include "debug.h"

#include "DOA6/hashes/bones_names.h"

static bool get_hex_name(const std::string &fn, uint32_t *ret)
{
    if (!Utils::BeginsWith(fn, "0x", false))
        return false;

    if (fn.length() == 2)
        return false;

    for (size_t i = 2; i < fn.length(); i++)
    {
        char ch = tolower(fn[i]);

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

OidFile::OidFile()
{
    this->big_endian = false;
    Reset();
}

OidFile::~OidFile()
{

}

void OidFile::CreateNamesMap()
{
    names_map.clear();
    const char *names = (const char *)bones_names;

    // Implementaton asumes there are no inner spaces in filenames
    char comp1[32], comp2[256];
    bool in_comment = false;
    bool in_c2 = false;

    size_t len = strlen(names);
    size_t c1 = 0, c2 = 0;
    int line_num = 1;


    for (size_t i = 0; i < len; i++)
    {
        char ch = names[i];

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
                    return;
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
}

void OidFile::Reset()
{
    max_bone_id = 0;
    entries.clear();
}

bool OidFile::Load(const uint8_t *buf, size_t size)
{
    Reset();
    FixedMemoryStream mem(const_cast<uint8_t *>(buf), size);

    if (size < 4)
        return false;

    if (((size - 4) % sizeof(OIDEntry)) != 0)
    {
        DPRINTF("%s: Invalid size.\n", FUNCNAME);
        return false;
    }

    size_t num_entries = (size - 4) / sizeof(OIDEntry);
    entries.resize(num_entries);

    if (!mem.Read32(&max_bone_id))
        return false;

    if (!mem.Read(entries.data(), entries.size()*sizeof(OIDEntry)))
        return false;

    return true;
}

bool OidFile::SaveToTextFile(const std::string &path, bool *crack_success)
{
    if (crack_success)
        *crack_success = true;

    if (names_map.size() == 0)
        CreateNamesMap();

    FILE *f = fopen(path.c_str(), "w");
    if (!f)
    {
        DPRINTF("Cannot open \"%s\" for writing.\n", path.c_str());
        return false;
    }

    fprintf(f, ";\n; object id,name\n;\n");

    for (const OIDEntry &entry: entries)
    {
        std::string fail_name = "HASH_" + Utils::UnsignedToString(entry.hash, true);

        auto it = names_map.find(entry.hash);
        if (it == names_map.end())
        {
            if (crack_success)
                *crack_success = false;

            fprintf(f, "%u,%s\n", entry.bone_id, fail_name.c_str());
        }
        else
        {
            fprintf(f, "%u,%s\n", entry.bone_id, it->second.c_str());
        }
    }

    fprintf(f, "\n");
    fclose(f);
    return true;
}
