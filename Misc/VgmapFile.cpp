#include "VgmapFile.h"
#include "debug.h"

VgmapFile::VgmapFile()
{
    this->big_endian = false;
}

VgmapFile::~VgmapFile()
{
}

void VgmapFile::Reset()
{
    entries.clear();
    greater_vg = 0;
}

bool VgmapFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    std::string content;
    content.resize(size);

    for (size_t i = 0; i < size; i++)
        content.push_back((char)buf[i]);

    std::vector<std::string> lines;
    if (Utils::GetMultipleStrings(content, lines, '\n') == 0)
        return true; // empty file

    // Ok, this is not a json parser, we'll just check lines that have a ":", and that's it.
    for (std::string &line : lines)
    {
        Utils::TrimString(line);

        if (line.find(':') == std::string::npos)
            continue;

        std::vector<std::string> values;

        if (Utils::GetMultipleStrings(line, values, ':') != 2)
        {
            DPRINTF("Failed to understand this line from vgmap: %s\n", line.c_str());
            return false;
        }

        Utils::TrimString(values[0]);
        Utils::TrimString(values[1]);

        if (values[1].length() > 0 && values[1].back() == ',')
            values[1].pop_back();

        if (values[0].length() == 0 || values[1].length() == 0)
        {
            DPRINTF("Failed to understand this line from vgmap: %s\n", line.c_str());
            return false;
        }

        std::string name;
        uint32_t vg;

        name = values[0];

        if (name.length() > 0 && name.front() == '"')
            name = name.substr(1);

        if (name.length() > 0 && name.back() == '"')
            name.pop_back();

        vg = Utils::GetUnsigned(values[1], 0xFFFFFFFF);

        if (vg == 0xFFFFFFFF)
        {
            DPRINTF("Failed to understand this line from vgmap: %s\n", line.c_str());
            return false;
        }

        entries[vg] = name;
        reverse_entries[name] = vg;
        if (vg > greater_vg)
            greater_vg = vg;
    }

    return true;
}

uint8_t *VgmapFile::Save(size_t *psize)
{
    std::string ret;

    if (greater_vg > 255)
    {
        DPRINTF("Warning: bigger vg > 255, this won't work on 8-bit index blend indices.\n");
    }

    ret = "{\n";

    bool first_line = true;

    for (const auto &it : entries)
    {
        if (first_line)
        {
            first_line = false;
        }
        else
        {
            ret += ",\n";
        }

        ret += "\t\"";
        ret += it.second;
        ret += "\": ";
        ret += Utils::UnsignedToString(it.first, false);
    }

    ret += "\n}\n";

    uint8_t *buf = new uint8_t[ret.length()];
    memcpy(buf, ret.data(), ret.length());
    *psize = ret.length();
    return buf;
}

void VgmapFile::Append(const VgmapFile &other)
{
    for (const auto &it : other.entries)
    {
        auto it2 = reverse_entries.find(it.second);
        if (it2 == reverse_entries.end())
        {
            greater_vg += 3;
            entries[greater_vg] = it.second;
            reverse_entries[it.second] = greater_vg;
        }
    }
}

