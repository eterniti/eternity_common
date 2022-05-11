#include "CsvFile.h"
#include "debug.h"

CsvFile::CsvFile()
{
    this->big_endian = false;
}

CsvFile::~CsvFile()
{

}

void CsvFile::Reset()
{
    anims.clear();
}

bool CsvFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size == 0)
        return false;

    uint8_t *copy = new uint8_t[size+1];
    memcpy(copy, buf, size);
    copy[size] = 0;

    std::string text_file = (const char *)copy;
    delete[] copy;

    std::vector<std::string> lines;
    if (Utils::GetMultipleStrings(text_file, lines, '\n') < 3)
        return false;

    if (Utils::BeginsWith(lines[0], "\xEF\xBB\xBF"))
    {
        lines[0] = lines[0].substr(3);
    }

    std::vector<std::string> bones;
    Utils::GetMultipleStrings(lines[0], bones);

    for (size_t i = 0; i < bones.size(); i++)
    {
        std::string &bone = bones[i];
        Utils::TrimString(bone);

        if (bone.length() == 0)
        {
            bones.erase(bones.begin()+i);
            i--;
            continue;
        }

        //DPRINTF("%s\n", bone.c_str());
    }

    //DPRINTF("Num bones = %Id\n", bones.size());
    anims.resize(bones.size());

    size_t expected_components = (anims.size() * 9) + 1;

    for (size_t i = 2; i < lines.size(); i++)
    {
        std::string &line = lines[i];
        Utils::TrimString(line);

        if (line.length() == 0)
            continue;

        std::vector<std::string> components;
        Utils::GetMultipleStrings(line, components);

        for (size_t j = 0; j < components.size(); j++)
        {
            std::string &comp = components[j];
            Utils::TrimString(comp);

            if (comp.length() == 0)
            {
                components.erase(components.begin()+j);
                j--;
            }
        }

        if (components.size() == 0)
            continue;

        if (components.size() != expected_components)
        {
            DPRINTF("CSV parse error: In line %Id, expected %Id elements, but got %Id\n", i, expected_components, components.size());
            return false;
        }

        for (size_t b = 0; b < anims.size(); b++)
        {
            CsvAnim &a = anims[b];
            CsvFrame f;

            f.frame = Utils::GetUnsigned(components[0]);
            f.posx = Utils::GetFloat(components[1+b*9]);
            f.posy = Utils::GetFloat(components[2+b*9]);
            f.posz = Utils::GetFloat(components[3+b*9]);
            f.rotx = Utils::GetFloat(components[4+b*9]);
            f.roty = Utils::GetFloat(components[5+b*9]);
            f.rotz = Utils::GetFloat(components[6+b*9]);
            f.scalex = Utils::GetFloat(components[7+b*9], 1.0f);
            f.scaley = Utils::GetFloat(components[8+b*9], 1.0f);
            f.scalez = Utils::GetFloat(components[9+b*9], 1.0f);

            a.bone = bones[b];
            a.frames.push_back(f);
        }
    }

    //DPRINTF("Num frames %Id\n", anims[0].frames.size());
   // DPRINTF("%s %d %f %f %f %f %f %f %f %f %f\n", anims[0].bone.c_str(), anims[0].frames[0].frame, anims[0].frames[0].posx, anims[0].frames[0].posy, anims[0].frames[0].posz, anims[0].frames[0].rotx, anims[0].frames[0].roty, anims[0].frames[0].rotz, anims[0].frames[0].scalex, anims[0].frames[0].scaley, anims[0].frames[0].scalez);


    return true;
}
