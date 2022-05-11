#include <algorithm>
#include <unordered_set>

#include "BcsFile.h"
#include "debug.h"

#define COPY_VAL(a, b, f) a->f = b->f

#define COPYPU1_I(f) COPY_VAL(this, file_unk1, f)
#define COPYPU1_O(f) COPY_VAL(file_unk1, this, f)

#define COPYPU2_I(f) COPY_VAL(this, file_unk2, f)
#define COPYPU2_O(f) COPY_VAL(file_unk2, this, f)

#define COPYP_I(f) COPY_VAL(this, file_part, f)
#define COPYP_O(f) COPY_VAL(file_part, this, f)

#define COPYB_I(f) COPY_VAL(this, file_bone, f)
#define COPYB_O(f) COPY_VAL(file_bone, this, f)

#define COPY_I(f) COPY_VAL(this, hdr, f)
#define COPY_O(f) COPY_VAL(hdr, this, f)

#define ADD_STR(s) { if (s.length() > 0) { strings.push_back(s); *str_size += s.length()+1; } }

bool BcsColorSelector::Load(const uint8_t *, const BCSColorSelector *file_unk1)
{
    COPYPU1_I(part_colors);
    COPYPU1_I(color);

    return true;
}

size_t BcsColorSelector::Save(BcsFile *, const uint8_t *, BCSColorSelector *file_unk1) const
{
    COPYPU1_O(part_colors);
    COPYPU1_O(color);

    return sizeof(BCSColorSelector);
}

TiXmlElement *BcsColorSelector::Decompile(TiXmlNode *root, const BcsFile *owner) const
{
    TiXmlElement *entry_root = new TiXmlElement("ColorSelector");

    std::string color_comment;

    if (owner)
    {
        if (part_colors < owner->parts_colors.size())
        {
            const BcsPartColors &pc = owner->parts_colors[part_colors];

            if (pc.valid && pc.name.length() > 0)
            {
                Utils::WriteComment(entry_root, pc.name);
            }

            if (color < pc.colors.size())
            {
                uint32_t rgb = pc.colors[color].GetColorPreview(pc.name == "eye_");
                color_comment = "Color preview: " + Utils::ColorToString(rgb);
            }
        }
    }

    Utils::WriteParamUnsigned(entry_root, "PART_COLORS", part_colors);

    if (color_comment.length() > 0)
        Utils::WriteComment(entry_root, color_comment);

    Utils::WriteParamUnsigned(entry_root, "COLOR", color);

    if (root)
        root->LinkEndChild(entry_root);

    return entry_root;
}

bool BcsColorSelector::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root,  "PART_COLORS", &part_colors))
        return false;

    if (!Utils::GetParamUnsigned(root,  "COLOR", &color))
        return false;

    return true;
}

bool BcsPhysics::Load(BcsFile *file, const uint8_t *, const BCSPhysics *file_unk2)
{
    COPYPU2_I(unk_00);
    COPYPU2_I(unk_02);
    COPYPU2_I(unk_04);
    COPYPU2_I(unk_18);
    COPYPU2_I(unk_1C);
    COPYPU2_I(unk_20);

    memcpy(name, file_unk2->name, sizeof(name));

    for (size_t i = 0; i < 6; i++)
    {
        unk_28[i] = file->GetString(file_unk2, file_unk2->unk_28[i]);
    }

    return true;
}

size_t BcsPhysics::PreSave(size_t *str_size, std::vector<std::string> &strings) const
{
    for (size_t i = 0; i < 6; i++)
    {
        ADD_STR(unk_28[i]);
    }

    return sizeof(BCSPhysics);
}

size_t BcsPhysics::Save(BcsFile *, const uint8_t *top, BCSPhysics *file_unk2, std::queue<uint32_t> &strings) const
{
    COPYPU2_O(unk_00);
    COPYPU2_O(unk_02);
    COPYPU2_O(unk_04);
    COPYPU2_O(unk_18);
    COPYPU2_O(unk_1C);
    COPYPU2_O(unk_20);

    memcpy(file_unk2->name, name, 4);

    for (size_t i = 0; i < 6; i++)
    {
        if (unk_28[i].length() > 0)
        {
            file_unk2->unk_28[i] = strings.front() - Utils::DifPointer(file_unk2, top);
            strings.pop();
        }
    }

    return sizeof(BCSPhysics);
}

TiXmlElement *BcsPhysics::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("PhysicsObject");

    Utils::WriteParamUnsigned(entry_root, "U_00", unk_00, true);
    Utils::WriteParamUnsigned(entry_root, "U_02", unk_02, true);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "U_18", unk_18, true);
    Utils::WriteParamUnsigned(entry_root, "U_1C", unk_1C, true);
    Utils::WriteParamUnsigned(entry_root, "U_20", unk_20, true);
    Utils::WriteParamString(entry_root, "NAME", name);
    Utils::WriteParamMultipleStrings(entry_root, "STR_28", std::vector<std::string>(unk_28, unk_28+6));

    if (root)
        root->LinkEndChild(entry_root);

    return entry_root;
}

bool BcsPhysics::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "U_00", &unk_00))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_02", &unk_02))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_18", &unk_18))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_1C", &unk_1C))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_20", &unk_20))
        return false;

    std::string name_str;

    if (!Utils::GetParamString(root, "NAME", name_str))
        return false;

    if (name_str.length() > 3)
    {
        DPRINTF("%s: Parameter NAME cannot have more than 3 chars.\n", FUNCNAME);
        return false;
    }

    memset(name, 0, sizeof(name));
    strcpy(name, name_str.c_str());

    std::vector<std::string> unk28_vec;

    if (!Utils::GetParamMultipleStrings(root, "STR_28", unk28_vec))
        return false;

    if (unk28_vec.size() != 6)
    {
        DPRINTF("%s: STR_28 must have 6 elements.\n", FUNCNAME);
        return false;
    }

    for (size_t i = 0; i < unk28_vec.size(); i++)
        unk_28[i] = unk28_vec[i];

    return true;
}

bool BcsPart::Load(BcsFile *file, const uint8_t *top, const BCSPart *file_part)
{
    uint8_t *floor = (uint8_t *)(file_part+1);

    color_selectors.resize(file_part->num_color_selectors);

    for (size_t i = 0; i < color_selectors.size(); i++)
    {
        BcsColorSelector &unk1 = color_selectors[i];

        if (!unk1.Load(top, (const BCSColorSelector *)floor))
            return false;

        floor += sizeof(BCSColorSelector);
    }

    physics.resize(file_part->num_physics);

    for (size_t i = 0; i < physics.size(); i++)
    {
        BcsPhysics &unk2 = physics[i];

        if (!unk2.Load(file, top, (const BCSPhysics *)floor))
            return false;

        floor += sizeof(BCSPhysics);
    }

    // HACK: this part is a hack because we couldn't figure out something that worked in 100% of cases.
    // There was always one that made the logic fail

    COPYP_I(num_unk3);

    if (file_part->num_unk3 != 0)
    {
        uint32_t *unk3 = (uint32_t *)floor;

        for (int i = 0; i < file_part->num_unk3; )
        {
            if (*(unk3++) == 8)
                i++;
        }

        uint16_t h1, h2;

        do
        {
            h1 = *unk3 & 0xFFFF;
            h2 = (*unk3 >> 16);

            unk3++;

        } while (h1 != h2);

        unk3--;

        uint32_t size = Utils::DifPointer(unk3, floor);
        unk3_data.resize(size);
        memcpy(unk3_data.data(), floor, size);
        //DPRINTF("Size = 0x%x, at 0x%x\n", size, Utils::DifPointer(floor, top));
    }
    // End of HACK

    COPYP_I(model);
    COPYP_I(model2);
    COPYP_I(texture);
    COPYP_I(unk_10);
    COPYP_I(unk_18);
    COPYP_I(unk_1C);
    COPYP_I(unk_20);
    COPYP_I(unk_24);
    COPYP_I(unk_28);
    COPYP_I(unk_2C);
    COPYP_I(unk_30);

    memcpy(name, file_part->name, 4);

    for (size_t i = 0; i < 4; i++)
    {
        files[i] = file->GetString(file_part, file_part->files_offsets[i]);
    }

    valid = true;
    return true;
}

size_t BcsPart::PreSave(size_t *str_size, std::vector<std::string> &strings) const
{
    if (!valid)
        return 0;

    size_t size = sizeof(BCSPart);

    for (const BcsColorSelector &unk1 : color_selectors)
    {
        size += unk1.PreSave();
    }

    for (size_t i = 0; i < 4; i++)
    {
        ADD_STR(files[i]);
    }

    for (const BcsPhysics &unk2 : physics)
    {
        size += unk2.PreSave(str_size, strings);
    }

    size += unk3_data.size();
    return size;
}

size_t BcsPart::Save(BcsFile *file, const uint8_t *top, BCSPart *file_part, std::queue<uint32_t> &strings) const
{
    if (!valid)
        return 0;

    size_t size = sizeof(BCSPart);

    COPYP_O(model);
    COPYP_O(model2);
    COPYP_O(texture);
    COPYP_O(unk_10);

    if (color_selectors.size() > 0)
    {
        file_part->num_color_selectors = (uint16_t)color_selectors.size();
        file_part->color_selector_offset = (uint32_t) size;

        for (const BcsColorSelector &unk1 : color_selectors)
            size += unk1.Save(file, top, (BCSColorSelector *)file->GetOffsetPtr(file_part, (uint32_t)size, true));
    }

    COPYP_O(unk_18);
    COPYP_O(unk_1C);
    COPYP_O(unk_20);
    COPYP_O(unk_24);
    COPYP_O(unk_28);
    COPYP_O(unk_2C);
    COPYP_O(unk_30);

    memcpy(file_part->name, name, 4);

    for (size_t i = 0; i < 4; i++)
    {
        if (files[i].length() > 0)
        {
            file_part->files_offsets[i] = strings.front() - Utils::DifPointer(file_part, top);
            strings.pop();
        }
    }

    if (physics.size() > 0)
    {
        file_part->num_physics = (uint16_t)physics.size();
        file_part->physics_offset = (uint32_t)size;

        for (const BcsPhysics &unk2 : physics)
            size += unk2.Save(file, top, (BCSPhysics *)file->GetOffsetPtr(file_part, (uint32_t)size, true), strings);
    }

    if (unk3_data.size() > 0)
    {
        COPYP_O(num_unk3);
        file_part->unk3_offset = (uint32_t)size;

        memcpy(file->GetOffsetPtr(file_part, (uint32_t)size, true), unk3_data.data(), unk3_data.size());
        size += unk3_data.size();
    }

    return size;
}

TiXmlElement *BcsPart::Decompile(TiXmlNode *root, int idx, const BcsFile *owner) const
{
    TiXmlElement *entry_root = new TiXmlElement("Part");
    entry_root->SetAttribute("idx", idx);

    if (valid)
    {
        Utils::WriteParamUnsigned(entry_root, "MODEL", model);
        Utils::WriteParamUnsigned(entry_root, "MODEL2", model2);
        Utils::WriteParamUnsigned(entry_root, "TEXTURE", texture);
        Utils::WriteParamUnsigned(entry_root, "U_10", unk_10, true);
        Utils::WriteParamUnsigned(entry_root, "U_18", unk_18, true);
        Utils::WriteParamUnsigned(entry_root, "U_1C", unk_1C, true);
        Utils::WriteParamUnsigned(entry_root, "U_20", unk_20, true);
        Utils::WriteParamFloat(entry_root, "F_24", unk_24);
        Utils::WriteParamFloat(entry_root, "F_28", unk_28);
        Utils::WriteParamUnsigned(entry_root, "U_2C", unk_2C, true);
        Utils::WriteParamUnsigned(entry_root, "U_30", unk_30, true);
        Utils::WriteParamString(entry_root, "NAME", name);
        Utils::WriteComment(entry_root, "MODEL, EMM, EMB, EAN");
        Utils::WriteParamMultipleStrings(entry_root, "FILES", std::vector<std::string>(files, files+4));

        if (num_unk3 > 0)
        {
            Utils::WriteParamUnsigned(entry_root, "NUM_UNK3", num_unk3);
            Utils::WriteParamMultipleUnsigned(entry_root, "UNK3", unk3_data, true);
        }

        for (const BcsColorSelector &unk1 : color_selectors)
            unk1.Decompile(entry_root, owner);

        for (const BcsPhysics &unk2 : physics)
            unk2.Decompile(entry_root);

    }
    else
    {
        Utils::WriteComment(entry_root, "This entry is empty.");
    }

    if (root)
        root->LinkEndChild(entry_root);

    return entry_root;
}

bool BcsPart::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadParamUnsigned(root, "MODEL", &model))
    {
        // Empty entry
        valid = false;
        return true;
    }

    if (!Utils::GetParamUnsigned(root, "MODEL2", &model2))
        return false;

    if (!Utils::GetParamUnsigned(root, "TEXTURE", &texture))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_10", &unk_10))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_18", &unk_18))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_1C", &unk_1C))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_20", &unk_20))
        return false;

    if (!Utils::GetParamFloat(root, "F_24", &unk_24))
        return false;

    if (!Utils::GetParamFloat(root, "F_28", &unk_28))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_2C", &unk_2C))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_30", &unk_30))
        return false;

    std::string name_str;

    if (!Utils::GetParamString(root, "NAME", name_str))
        return false;

    if (name_str.length() > 3)
    {
        DPRINTF("%s: Parameter NAME cannot have more than 3 chars.\n", FUNCNAME);
        return false;
    }

    memset(name, 0, sizeof(name));
    strcpy(name, name_str.c_str());

    std::vector<std::string> files_vec;

    if (!Utils::GetParamMultipleStrings(root, "FILES", files_vec))
        return false;

    if (files_vec.size() != 4)
    {
        DPRINTF("%s: FILES must have 4 elements.\n", FUNCNAME);
        return false;
    }

    for (size_t i = 0; i < files_vec.size(); i++)
        files[i] = files_vec[i];

    if (Utils::ReadParamUnsigned(root, "NUM_UNK3", &num_unk3))
    {
        if (!Utils::GetParamMultipleUnsigned(root, "UNK3", unk3_data))
            return false;
    }
    else
    {
        num_unk3 = 0;
        unk3_data.clear();
    }

    color_selectors.clear();
    physics.clear();

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "ColorSelector")
        {
           BcsColorSelector unk1;

           if (!unk1.Compile(elem))
               return false;

           color_selectors.push_back(unk1);
        }
        else if (elem->ValueStr() == "PhysicsObject")
        {
            BcsPhysics unk2;

            if (!unk2.Compile(elem))
                return false;

            physics.push_back(unk2);
        }
    }

    valid = true;
    return true;
}

bool BcsPartSet::Load(BcsFile *file, const uint8_t *top, const BCSPartSet *file_set)
{
    if (file_set->num_parts != 10 || file_set->table_start != 0x20)
    {
        DPRINTF("%s: num_parts or table_start not expected.\n", FUNCNAME);
        return false;
    }

    parts.resize(file_set->num_parts);

    for (size_t i = 0; i < file_set->num_parts; i++)
    {
        if (file_set->parts_offsets[i] != 0)
        {
            BcsPart &part = parts[i];

            if (!part.Load(file, top, (const BCSPart *)file->GetOffsetPtr(file_set, file_set->parts_offsets, (uint32_t)i)))
                return false;
        }
    }

    valid = true;
    return true;
}

size_t BcsPartSet::PreSave(size_t *str_size, std::vector<std::string> &strings) const
{
    if (!valid)
        return 0;

    size_t size = sizeof(BCSPartSet);

    for (const BcsPart &part : parts)
        size += part.PreSave(str_size, strings);

    return size;
}

size_t BcsPartSet::Save(BcsFile *file, const uint8_t *top, BCSPartSet *file_set, std::queue<uint32_t> &strings) const
{
    if (!valid)
        return 0;

    size_t size = sizeof(BCSPartSet);
    file_set->num_parts = (uint32_t)parts.size();
    file_set->table_start = 0x20;

    for (size_t i = 0; i < parts.size(); i++)
    {
        const BcsPart &part = parts[i];

        if (part.valid)
        {
            file_set->parts_offsets[i] = (uint32_t)size;
            size += part.Save(file, top, (BCSPart *)file->GetOffsetPtr(file_set, (uint32_t)size, true), strings);
        }
    }

    return size;
}

TiXmlElement *BcsPartSet::Decompile(TiXmlNode *root, int idx, const BcsFile *owner) const
{
    static const std::vector<std::string> part_names =
    {
        "Face_base",
        "Face_forehead",
        "Face_eye",
        "Face_nose",
        "Face_ear",
        "Hair",
        "Bust",
        "Pants",
        "Rist",
        "Boots"
    };

    TiXmlElement *entry_root = new TiXmlElement("PartSet");

    if (idx != -1)
        entry_root->SetAttribute("idx", idx);

    if (valid)
    {
        for (size_t i = 0; i < parts.size(); i++)
        {
            TiXmlElement *elem = parts[i].Decompile(entry_root, (int)i, owner);

            if (i < part_names.size())
            {
                TiXmlComment comment;
                const TiXmlElement *model;

                comment.SetValue(part_names[i]);

                if (Utils::GetElemCount(elem, "MODEL", &model) != 0)
                {
                    elem->InsertBeforeChild(const_cast<TiXmlElement *>(model), comment);
                }
                else if (elem->FirstChild())
                {
                    elem->InsertBeforeChild(elem->FirstChild(), comment);
                }
            }
        }
    }
    else
    {
        Utils::WriteComment(entry_root, "This entry is empty.");
    }

    if (root)
        root->LinkEndChild(entry_root);

    return entry_root;
}

bool BcsPartSet::Compile(const TiXmlElement *root)
{
    size_t num = Utils::GetElemCount(root, "Part");

    if (num == 0)
    {
        valid = false;
        return true;
    }

    parts.resize(num);

    std::unordered_set<uint32_t> ids;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Part")
        {
            uint32_t idx;

            if (!Utils::ReadAttrUnsigned(elem, "idx", &idx))
            {
                DPRINTF("%s: Attribute idx not found in <Part>.\n", FUNCNAME);
                return false;
            }

            if (idx >= parts.size())
            {
                DPRINTF("%s: idx %d out of bounds.\n", FUNCNAME, idx);
                return false;
            }

            if (ids.find(idx) != ids.end())
            {
                DPRINTF("%s: idx 0x%x was already previously defined for this object.\n", FUNCNAME, idx);
                return false;
            }

            ids.insert(idx);
            BcsPart &part = parts[idx];

            if (!part.Compile(elem))
                return false;
        }
    }

    valid = true;
    return true;
}

bool BcsColor::Load(const float *floats)
{
    memcpy(this->floats, floats, sizeof(this->floats));
    return true;
}

uint32_t BcsColor::GetColorPreview(bool special_case_eyes) const
{
    const float *rgb_floats = floats;

    if (special_case_eyes) // special case for eyes, game actually does this
    {
        rgb_floats = &rgb_floats[12];
    }

    uint32_t u_r = (uint32_t) (rgb_floats[0] * 255.0);
    uint32_t u_g = (uint32_t) (rgb_floats[1] * 255.0);
    uint32_t u_b = (uint32_t) (rgb_floats[2] * 255.0);

    uint32_t color = (u_r << 16);
    color |= (u_g << 8);
    color |= u_b;

    return color;
}

/*bool BcsColor::Save(float *floats) const
{
    memcpy(floats, this->floats, sizeof(this->floats));
    return true;
}*/

TiXmlElement *BcsColor::Decompile(TiXmlNode *root, int idx, bool special_case_eyes) const
{
    TiXmlElement *entry_root = new TiXmlElement("Color");
    entry_root->SetAttribute("idx", idx);

    Utils::WriteComment(entry_root, "Color preview: " + Utils::ColorToString(GetColorPreview(special_case_eyes)));
    Utils::WriteParamMultipleFloats(entry_root, "FLOATS", std::vector<float>(floats, floats+20));

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BcsColor::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamMultipleFloats(root, "FLOATS", floats, 20))
        return false;

    return true;
}

bool BcsPartColors::Load(BcsFile *file, const uint8_t *, const BCSPartColors *file_part_colors)
{
    name = file->GetString(file_part_colors, file_part_colors->name_offset);
    colors.resize(file_part_colors->num);

    const float *floats_data = (const float *)file->GetOffsetPtr(file_part_colors, file_part_colors->floats_offset);

    for (BcsColor &color : colors)
    {
        if (!color.Load(floats_data))
            return false;

        floats_data += 20;
    }

    valid = true;
    return true;
}

size_t BcsPartColors::PreSave(size_t *str_size, std::vector<std::string> &strings) const
{
    if (!valid)
        return 0;

    ADD_STR(name);

    return sizeof(BCSPartColors) + (colors.size()*20*sizeof(float));
}

size_t BcsPartColors::Save(BcsFile *, const uint8_t *top, BCSPartColors *file_part_colors, std::queue<uint32_t> &strings, FixedMemoryStream &s2_stream, uint32_t doffs) const
{
    if (!valid)
        return 0;

    file_part_colors->num = (uint16_t)colors.size();

    if (name.length() > 0)
    {
        file_part_colors->name_offset = strings.front() - Utils::DifPointer(file_part_colors, top);
        strings.pop();
    }

    file_part_colors->floats_offset = (colors.size() == 0) ? 0 : doffs;

    for (const BcsColor &color : colors)
    {
        s2_stream.Write(color.floats, sizeof(color.floats));
    }

    return sizeof(BCSPartColors);
}

TiXmlElement *BcsPartColors::Decompile(TiXmlNode *root, int idx) const
{
    TiXmlElement *entry_root = new TiXmlElement("PartColors");
    entry_root->SetAttribute("idx", idx);

    if (valid)
    {
        Utils::WriteParamString(entry_root, "NAME", name);

        for (size_t i = 0; i < colors.size(); i++)
        {
            colors[i].Decompile(entry_root, (int)i, (name == "eye_"));
        }
    }
    else
    {
        Utils::WriteComment(entry_root, "This entry is empty.");
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BcsPartColors::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadParamString(root, "NAME", name))
    {
        valid = false;
        return true;
    }

    colors.clear();
    colors.resize(Utils::GetElemCount(root, "Color"));

    std::unordered_set<uint32_t> ids;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Color")
        {
            uint32_t idx;

            if (!Utils::ReadAttrUnsigned(elem, "idx", &idx))
            {
                DPRINTF("%s: Cannot get idx for Color.\n", FUNCNAME);
                return false;
            }

            if (idx >= colors.size())
            {
                DPRINTF("%s: Color %d is out of bounds.\n", FUNCNAME, idx);
                return false;
            }

            if (ids.find(idx) != ids.end())
            {
                DPRINTF("%s: idx 0x%x was already previously defined for this object.\n", FUNCNAME, idx);
                return false;
            }

            ids.insert(idx);
            BcsColor &color = colors[idx];

            if (!color.Compile(elem))
                return false;
        }
    }

    valid = true;
    return true;
}

bool BcsBoneScale::Load(BcsFile *file, const uint8_t *, const BCSBoneScale *file_bone_scale)
{
    memcpy(scale, file_bone_scale->scale, sizeof(scale));
    name = file->GetString(file_bone_scale, file_bone_scale->name_offset);

    return true;
}

size_t BcsBoneScale::PreSave(size_t *str_size, std::vector<std::string> &strings) const
{
    ADD_STR(name);
    return sizeof(BCSBoneScale);
}

size_t BcsBoneScale::Save(BcsFile *, const uint8_t *top, FixedMemoryStream &stream, std::queue<uint32_t> &strings) const
{
    uint8_t *myself_ptr = stream.GetMemory(false) + (size_t)stream.Tell();

    stream.Write(scale, sizeof(scale));

    uint32_t name_offset;

    if (name.length() == 0)
    {
        name_offset = 0;
    }
    else
    {
        name_offset = strings.front() - Utils::DifPointer(myself_ptr, top);
        strings.pop();
    }

    stream.Write32(name_offset);
    return sizeof(BCSBoneScale);
}

TiXmlElement *BcsBoneScale::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("BoneScale");

    Utils::WriteParamMultipleFloats(entry_root, "SCALE", std::vector<float>(scale, scale+3));
    Utils::WriteParamString(entry_root, "NAME", name);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BcsBoneScale::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamMultipleFloats(root, "SCALE", scale, 3))
        return false;

    if (!Utils::GetParamString(root, "NAME", name))
        return false;

    return true;
}

bool BcsBody::Load(BcsFile *file, const uint8_t *top, const BCSBody *file_unk2)
{
    unk1s.resize(file_unk2->num);

    const BCSBoneScale *file_unk1s = (const BCSBoneScale *)file->GetOffsetPtr(file_unk2, file_unk2->unk1_offset);

    for (size_t i = 0; i < unk1s.size(); i++)
    {
        BcsBoneScale &unk1 = unk1s[i];

        if (!unk1.Load(file, top, &file_unk1s[i]))
            return false;
    }

    valid = true;
    return true;
}

size_t BcsBody::PreSave(size_t *str_size, std::vector<std::string> &strings) const
{
    if (!valid)
        return 0;

    size_t size = sizeof(BCSBody);

    for (const BcsBoneScale &unk1 : unk1s)
    {
        size += unk1.PreSave(str_size, strings);
    }

    return size;
}

size_t BcsBody::Save(BcsFile *file, const uint8_t *top, BCSBody *file_unk2, std::queue<uint32_t> &strings, FixedMemoryStream &s2_stream, uint32_t doffs) const
{
    if (!valid)
        return 0;

    file_unk2->num = (uint16_t)unk1s.size();
    file_unk2->unk1_offset = (unk1s.size() == 0) ? 0 : doffs;

    for (const BcsBoneScale &unk1 : unk1s)
    {
        unk1.Save(file, top, s2_stream, strings);
    }

    return sizeof(BCSBody);
}

TiXmlElement *BcsBody::Decompile(TiXmlNode *root, int idx) const
{
    TiXmlElement *entry_root = new TiXmlElement("Body");

    if (idx >= 0)
        entry_root->SetAttribute("idx", idx);

    if (valid)
    {
        for (const BcsBoneScale &unk1 : unk1s)
            unk1.Decompile(entry_root);

        if (unk1s.size() == 0)
        {
            entry_root->SetAttribute("force_valid", 1);
        }
    }
    else
    {
        Utils::WriteComment(entry_root, "This entry is empty.");
    }

    if (root)
        root->LinkEndChild(entry_root);

    return entry_root;
}

bool BcsBody::Compile(const TiXmlElement *root)
{
    size_t num = Utils::GetElemCount(root, "BoneScale");

    if (num == 0)
    {
        uint16_t force_valid;

        if (Utils::ReadAttrUnsigned(root, "force_valid", &force_valid))
        {
            valid = (force_valid != 0);

            if (valid)
            {
                unk1s.clear();
            }
        }
        else
        {
            valid = false;
        }

        return true;
    }

    unk1s.clear();
    unk1s.resize(num);
    size_t i = 0;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "BoneScale")
        {
            BcsBoneScale &unk1 = unk1s[i++];

            if (!unk1.Compile(elem))
                return false;
        }
    }

    valid = true;
    return true;
}

bool BcsBone::Load(BcsFile *file, const uint8_t *, const BCSBone *file_bone)
{
    COPYB_I(unk_00);
    COPYB_I(unk_04);
    COPYB_I(unk_08);
    memcpy(unk_0C, file_bone->unk_0C, sizeof(unk_0C));
    name = file->GetString(file_bone, file_bone->name_offset);

    return true;
}

size_t BcsBone::PreSave(size_t *str_size, std::vector<std::string> &strings) const
{
    ADD_STR(name);
    return sizeof(BCSBone);
}

size_t BcsBone::Save(BcsFile *, const uint8_t *top, BCSBone *file_bone, std::queue<uint32_t> &strings) const
{
    COPYB_O(unk_00);
    COPYB_O(unk_04);
    COPYB_O(unk_08);
    memcpy(file_bone->unk_0C, unk_0C, sizeof(unk_0C));

    if (name.length() > 0)
    {
        file_bone->name_offset = strings.front() - Utils::DifPointer(file_bone, top);
        strings.pop();
    }

    return sizeof(BCSBone);
}

TiXmlElement *BcsBone::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Bone");
    entry_root->SetAttribute("name", name);

    Utils::WriteParamUnsigned(entry_root, "U_00", unk_00, true);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);
    Utils::WriteParamUnsigned(entry_root, "U_08", unk_08, true);

    Utils::WriteComment(entry_root, "Following values are half-floats. They will be changed into floats values in a future revision");
    Utils::WriteParamMultipleUnsigned(entry_root, "U_0C", std::vector<uint16_t>(unk_0C, unk_0C+18), true);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BcsBone::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrString(root, "name", name))
    {
        DPRINTF("%s: name attribute is not optional (use empty string if needed instead).\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadParamUnsigned(root, "U_00", &unk_00))
    {
        unk_00 = 18;
    }

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_08", &unk_08))
        return false;

    if (!Utils::GetParamMultipleUnsigned(root, "U_0C", unk_0C, 18))
        return false;

    return true;
}

bool BcsSkeletonData::Load(BcsFile *file, const uint8_t *top, const BCSSkeletonData *file_skl)
{
    const BCSBone *file_bones = (const BCSBone *)file->GetOffsetPtr(file_skl, file_skl->data_start);

    unk_00 = file_skl->unk_00;
    bones.resize(file_skl->num_bones);

    for (size_t i = 0; i < bones.size(); i++)
    {
        BcsBone &bone = bones[i];

        if (!bone.Load(file, top, file_bones+i))
            return false;
    }

    valid = true;
    return true;
}

size_t BcsSkeletonData::PreSave(size_t *str_size, std::vector<std::string> &strings) const
{
    if (!valid)
        return 0;

    size_t size = sizeof(BCSSkeletonData);

    for (const BcsBone &bone : bones)
    {
        size += bone.PreSave(str_size, strings);
    }

    return size;
}

size_t BcsSkeletonData::Save(BcsFile *file, const uint8_t *top, BCSSkeletonData *file_skl, std::queue<uint32_t> &strings) const
{
    if (!valid)
        return 0;

    file_skl->unk_00 = unk_00;
    file_skl->num_bones = (uint16_t)bones.size();
    file_skl->data_start = sizeof(BCSSkeletonData);
    size_t size = sizeof(BCSSkeletonData);

    for (const BcsBone &bone : bones)
    {
        size += bone.Save(file, top, (BCSBone *)file->GetOffsetPtr(file_skl, (uint32_t)size, true), strings);
    }

    return size;
}

TiXmlElement *BcsSkeletonData::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("SkeletonData");

    if (valid)
    {
        Utils::WriteParamUnsigned(entry_root, "U_00", unk_00, true);

        for (const BcsBone &bone : bones)
            bone.Decompile(entry_root);
    }
    else
    {
        Utils::WriteComment(entry_root, "This entry is empty.");
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool BcsSkeletonData::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadParamUnsigned(root, "U_00", &unk_00))
    {
        valid = false;
        return true;
    }

    bones.clear();

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Bone")
        {
            BcsBone bone;

            if (!bone.Compile(elem))
                return false;

            bones.push_back(bone);
        }
    }

    valid = true;
    return true;
}

BcsFile::BcsFile()
{
    this->big_endian = false;
}

BcsFile::~BcsFile()
{

}

void BcsFile::Reset()
{
    sets.clear();
    parts_colors.clear();
    bodies.clear();
    skeleton_data.valid = false;
    additional_skeleton_data.clear();
    has_signature = true;
}

bool BcsFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (size < sizeof(BCSHeader))
        return false;

    const BCSHeader *hdr = (const BCSHeader *)buf;

    if (hdr->signature != BCS_SIGNATURE && hdr->signature != 0)
        return false;

    if (hdr->header_size == 0x48)
    {
        DPRINTF("%s: Xenoverse 1 format not supported.\n", FUNCNAME);
        return false;
    }
    else if (hdr->header_size != sizeof(BCSHeader))
    {
        if (hdr->header_size != 0 || hdr->signature != 0)
        {
            DPRINTF("%s: Unknown header size.\n", FUNCNAME);
            return false;
        }
    }

    if (hdr->unk_08 != 0 || hdr->unk_14)
    {
        DPRINTF("%s: unk_08 or unk_14 not zero.\n", FUNCNAME);
        return false;
    }

    uint32_t *part_sets_table = (uint32_t *)GetOffsetPtr(hdr, hdr->part_sets_table_offset);
    sets.resize(hdr->num_part_sets);

    for (size_t i = 0; i < sets.size(); i++)
    {
        BcsPartSet &set = sets[i];

        if (part_sets_table[i] != 0)
        {
            const BCSPartSet *part_set = (const BCSPartSet *)GetOffsetPtr(hdr, part_sets_table, (uint32_t)i);

            if (!set.Load(this, buf, part_set))
                return false;
        }
    }

    uint32_t *parts_colors_table = (uint32_t *)GetOffsetPtr(hdr, hdr->parts_colors_table_offset);
    parts_colors.resize(hdr->num_parts_colors);

    for (size_t i = 0; i < parts_colors.size(); i++)
    {
        BcsPartColors &part_colors = parts_colors[i];

        if (parts_colors_table[i] != 0)
        {
            const BCSPartColors *unk1_file = (const BCSPartColors *)GetOffsetPtr(hdr, parts_colors_table, (uint32_t)i);

            if (!part_colors.Load(this, buf, unk1_file))
                return false;

        }
    }

    uint32_t *unk2_table = (uint32_t *)GetOffsetPtr(hdr, hdr->bodies_table_offset);
    bodies.resize(hdr->num_bodies);

    for (size_t i = 0; i < bodies.size(); i++)
    {
        BcsBody &unk2 = bodies[i];

        if (unk2_table[i] != 0)
        {
            const BCSBody *unk2_file = (const BCSBody *)GetOffsetPtr(hdr, unk2_table, (uint32_t)i);

            if (!unk2.Load(this, buf, unk2_file))
                return false;
        }
    }

    if (hdr->skeleton_table_offset != 0)
    {
         uint32_t *skeleton_table = (uint32_t *)GetOffsetPtr(hdr, hdr->skeleton_table_offset);

         if (!skeleton_data.Load(this, buf, (const BCSSkeletonData *)GetOffsetPtr(hdr, skeleton_table, 0)))
             return false;
    }

    if (hdr->skeleton2_table_offset != 0)
    {
        uint32_t *skeleton_table = (uint32_t *)GetOffsetPtr(hdr, hdr->skeleton2_table_offset);
        additional_skeleton_data.resize(hdr->num_additional_skeletons);

        for (size_t i = 0; i < additional_skeleton_data.size(); i++)
        {
            BcsSkeletonData &sk_data = additional_skeleton_data[i];

            if (!sk_data.Load(this, buf, (const BCSSkeletonData *)GetOffsetPtr(hdr, skeleton_table, (uint32_t)i)))
                return false;
        }
    }

    has_signature = (hdr->signature == BCS_SIGNATURE);
    COPY_I(unk_2C);
    memcpy(unk_30, hdr->unk_30, sizeof(unk_30));

    return true;
}

uint8_t *BcsFile::Save(size_t *psize)
{
    std::vector<std::string> strings;
    std::queue<uint32_t> strings_offs;

    size_t str_size = 0;
    size_t header_size, sets_size, unk1s_size, unk2s_size, sk_size, sk2_size;
    size_t size;
    uint32_t current_offset;

    header_size = sizeof(BCSHeader) + 4 * (sets.size() + parts_colors.size() + bodies.size());

    if (skeleton_data.valid)
        header_size += 4;

    header_size += 4 * (uint32_t)additional_skeleton_data.size();

    sets_size = 0;
    for (const BcsPartSet &set : sets)
        sets_size += set.PreSave(&str_size, strings);

    unk1s_size = 0;
    for (const BcsPartColors &part_colors : parts_colors)
        unk1s_size += part_colors.PreSave(&str_size, strings);

    unk2s_size = 0;
    for (const BcsBody &unk2 : bodies)
        unk2s_size += unk2.PreSave(&str_size, strings);

    sk_size = skeleton_data.PreSave(&str_size, strings);
    sk2_size = 0;

    for (const BcsSkeletonData &sk_data : additional_skeleton_data)
    {
        sk2_size += sk_data.PreSave(&str_size,  strings);
    }

    size = header_size + sets_size + unk1s_size + unk2s_size + sk_size + sk2_size + str_size;

    current_offset = (uint32_t)(header_size + sets_size + unk1s_size + unk2s_size + sk_size + sk2_size);
    for (const std::string &str : strings)
    {
        strings_offs.push(current_offset);
        current_offset += (uint32_t)str.length()+1;
    }

    uint8_t *buf = new uint8_t[size];
    memset(buf, 0, size);

    BCSHeader *hdr = (BCSHeader *)buf;
    uint32_t *part_sets_table = (uint32_t *)(hdr+1);

    hdr->part_sets_table_offset = Utils::DifPointer(part_sets_table, hdr);
    current_offset = (uint32_t)header_size;

    for (size_t i = 0; i < sets.size(); i++)
    {
        const BcsPartSet &set = sets[i];

        if (set.valid)
        {
            part_sets_table[i] = current_offset;
            current_offset += (uint32_t)set.Save(this, buf, (BCSPartSet *)(buf+current_offset), strings_offs);
        }
    }

    uint32_t *parts_colors_table = part_sets_table+sets.size();
    hdr->parts_colors_table_offset = Utils::DifPointer(parts_colors_table, hdr);

    if (parts_colors.size() > 0)
    {
        size_t s1_size = 0;

        for (const BcsPartColors &part_colors : parts_colors)
        {
            if (part_colors.valid)
                s1_size += sizeof(BCSPartColors);
        }

        FixedMemoryStream s2_stream(buf+header_size+sets_size+s1_size, unk1s_size-s1_size);
        uint32_t doffs = (uint32_t)s1_size;

        for (size_t i = 0; i < parts_colors.size(); i++)
        {
            BcsPartColors &part_colors = parts_colors[i];

            if (part_colors.valid)
            {
                uint64_t prev_pos = s2_stream.Tell();

                parts_colors_table[i] = current_offset;
                current_offset += (uint32_t)part_colors.Save(this, buf, (BCSPartColors *)(buf+current_offset), strings_offs, s2_stream, doffs);
                doffs -= sizeof(BCSPartColors);
                doffs += (uint32_t) (s2_stream.Tell() - prev_pos);
            }
        }

        current_offset += (uint32_t)s2_stream.GetSize();
    }

    uint32_t *unk2s_table = parts_colors_table+parts_colors.size();
    hdr->bodies_table_offset = Utils::DifPointer(unk2s_table, hdr);

    if (bodies.size() > 0)
    {
        size_t s1_size = 0;

        for (const BcsBody &unk2 : bodies)
        {
            if (unk2.valid)
                s1_size += sizeof(BCSBody);
        }

        FixedMemoryStream s2_stream(buf+header_size+sets_size+unk1s_size+s1_size, unk2s_size-s1_size);
        uint32_t doffs = (uint32_t)s1_size;

        for (size_t i = 0; i < bodies.size(); i++)
        {
            BcsBody &unk2 = bodies[i];

            if (unk2.valid)
            {
                uint64_t prev_pos = s2_stream.Tell();

                unk2s_table[i] = current_offset;
                current_offset += (uint32_t)unk2.Save(this, buf, (BCSBody *)(buf+current_offset), strings_offs, s2_stream, doffs);
                doffs -= sizeof(BCSBody);
                doffs += (uint32_t) (s2_stream.Tell() - prev_pos);
            }
        }

        current_offset += (uint32_t) s2_stream.GetSize();
    }

    uint32_t *skl_table = unk2s_table+bodies.size();
    hdr->skeleton_table_offset = Utils::DifPointer(skl_table, hdr);

    if (skeleton_data.valid)
    {
        skl_table[0] = current_offset;
        current_offset += (uint32_t) skeleton_data.Save(this, buf, (BCSSkeletonData *)(buf+current_offset), strings_offs);
    }

    uint32_t *skl2_table = skl_table + ((skeleton_data.valid) ? 1 : 0);
    hdr->skeleton2_table_offset = Utils::DifPointer(skl2_table, hdr);

    if (additional_skeleton_data.size() > 0)
    {
        for (size_t i = 0; i < additional_skeleton_data.size(); i++)
        {
            skl2_table[i] = current_offset;
            current_offset += (uint32_t) additional_skeleton_data[i].Save(this, buf, (BCSSkeletonData *)(buf+current_offset), strings_offs);
        }
    }

    assert(strings_offs.empty());

    WriteStringList(buf+current_offset, strings);

    if (sets.size() == 0)
        hdr->part_sets_table_offset = 0;

    if (parts_colors.size() == 0)
        hdr->parts_colors_table_offset = 0;

    if (bodies.size() == 0)
        hdr->bodies_table_offset = 0;

    if (!skeleton_data.valid)
        hdr->skeleton_table_offset = 0;

    if (additional_skeleton_data.size() > 0)
        hdr->num_additional_skeletons = (uint32_t)additional_skeleton_data.size();
    else
        hdr->skeleton2_table_offset = 0;

    if (has_signature)
    {
        hdr->signature = BCS_SIGNATURE;
        hdr->endianess_check = val16(0xFFFE);
        hdr->header_size = sizeof(BCSHeader);
    }

    COPY_O(unk_2C);
    memcpy(hdr->unk_30, unk_30, sizeof(unk_30));

    hdr->num_part_sets = (uint32_t)sets.size();
    hdr->num_parts_colors = (uint32_t) parts_colors.size();
    hdr->num_bodies = (uint32_t) bodies.size();

    *psize = size;
    return buf;
}

TiXmlDocument *BcsFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("BCS");
    root->SetAttribute("has_signature", (has_signature) ? "true" : "false");

    Utils::WriteParamUnsigned(root, "U_2C", unk_2C, true);
    Utils::WriteParamMultipleFloats(root, "F_30", std::vector<float>(unk_30, unk_30+7));

    for (size_t i = 0; i < sets.size(); i++)
        sets[i].Decompile(root, (int)i, this);

    for (size_t i = 0; i < parts_colors.size(); i++)
        parts_colors[i].Decompile(root, (int)i);

    for (size_t i = 0; i < bodies.size(); i++)
        bodies[i].Decompile(root, (int)i);

    TiXmlElement *skl_root = skeleton_data.Decompile(root);
    skl_root->SetAttribute("idx", 0);

    for (size_t i = 0; i < additional_skeleton_data.size(); i++)
    {
        TiXmlElement *skl2_root = additional_skeleton_data[i].Decompile(root);
        skl2_root->SetAttribute("idx", (int)i+1);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool BcsFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "BCS");

    if (!root)
    {
        DPRINTF("Cannot find\"BCS\" in xml.\n");
        return false;
    }

    std::string signature;

    if (Utils::ReadAttrString(root, "has_signature", signature))
    {
        signature = Utils::ToLowerCase(signature);

        if (signature == "false" || signature == "0")
            has_signature = false;
        else
            has_signature = true;
    }

    if (!Utils::GetParamUnsigned(root, "U_2C", &unk_2C))
        return false;

    if (!Utils::GetParamMultipleFloats(root, "F_30", unk_30, 7))
        return false;

    std::unordered_set<uint32_t> set_ids, unk1_ids, unk2_ids, skl_ids;

    sets.resize(Utils::GetElemCount(root, "PartSet"));
    parts_colors.resize(Utils::GetElemCount(root, "PartColors"));
    bodies.resize(Utils::GetElemCount(root, "Body"));

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        const std::string &name = elem->ValueStr();

        if (name == "PartSet")
        {
            uint32_t idx;

            if (!Utils::ReadAttrUnsigned(elem, "idx", &idx))
            {
                DPRINTF("%s: Attribute idx not found for set.\n", FUNCNAME);
                return false;
            }

            if (idx >= sets.size())
            {
                //DPRINTF("%s: set idx %d out of bounds.\n", FUNCNAME, idx);
                //return false;
                sets.resize(idx+1);
            }

            if (set_ids.find(idx) != set_ids.end())
            {
                DPRINTF("%s: set idx 0x%x was already previously defined for this object.\n", FUNCNAME, idx);
                return false;
            }

            set_ids.insert(idx);
            BcsPartSet &set = sets[idx];

            if (!set.Compile(elem))
                return false;
        }
        else if (name == "PartColors")
        {
            uint32_t idx;

            if (!Utils::ReadAttrUnsigned(elem, "idx", &idx))
            {
                DPRINTF("%s: Attribute idx not found for BcsUnk1.\n", FUNCNAME);
                return false;
            }

            if (idx >= parts_colors.size())
            {
                DPRINTF("%s: PartColors idx 0x%x out of bounds.\n", FUNCNAME, idx);
                return false;
            }

            if (unk1_ids.find(idx) != unk1_ids.end())
            {
                DPRINTF("%s: PartColors idx 0x%x was already previously defined for this object.\n", FUNCNAME, idx);
                return false;
            }

            unk1_ids.insert(idx);
            BcsPartColors &unk1 = parts_colors[idx];

            if (!unk1.Compile(elem))
                return false;
        }
        else if (name == "Body")
        {
            uint32_t idx;

            if (!Utils::ReadAttrUnsigned(elem, "idx", &idx))
            {
                DPRINTF("%s: Attribute idx not found for BcsUnk2.\n", FUNCNAME);
                return false;
            }

            if (idx >= bodies.size())
            {
                DPRINTF("%s: Body idx 0x%x out of bounds.\n", FUNCNAME, idx);
                return false;
            }

            if (unk2_ids.find(idx) != unk2_ids.end())
            {
                DPRINTF("%s: Body idx 0x%x was already previously defined for this object.\n", FUNCNAME, idx);
                return false;
            }

            unk2_ids.insert(idx);
            BcsBody &unk2 = bodies[idx];

            if (!unk2.Compile(elem))
                return false;
        }
        else if (name == "SkeletonData")
        {
            uint32_t idx;

            if (!Utils::ReadAttrUnsigned(elem, "idx", &idx))
            {
                DPRINTF("%s: Attribute idx not found for SkeletonData.\n", FUNCNAME);
                return false;
            }

            /*if (idx > 1)
            {
                DPRINTF("%s: SkeletonData idx can only be 0 or 1.\n", FUNCNAME);
                return false;
            }*/

            if (skl_ids.find(idx) != skl_ids.end())
            {
                DPRINTF("%s: SkeletonData idx 0x%x was already previously defined for this object.\n", FUNCNAME, idx);
                return false;
            }

            skl_ids.insert(idx);

            if (idx == 0)
            {
                if (!skeleton_data.Compile(elem))
                    return false;
            }
            else
            {
                BcsSkeletonData sk_data;

                if (!sk_data.Compile(elem))
                    return false;

                if (idx > additional_skeleton_data.size())
                {
                    additional_skeleton_data.resize(idx);
                }

                additional_skeleton_data[idx-1] = sk_data;
            }
        }
    }

    uint32_t num_invalid = 0;

    for (const BcsSkeletonData &sk_data : additional_skeleton_data)
    {
        if (!sk_data.valid)
            num_invalid++;
    }

    if (num_invalid == 1)
        additional_skeleton_data.clear();
    else if (num_invalid >= 2)
        return false;

    return true;
}

uint32_t BcsFile::FindPartColorsByName(const std::string &name) const
{
    for (size_t i = 0; i < parts_colors.size(); i++)
    {
        const BcsPartColors &part_colors = parts_colors[i];

        if (part_colors.valid && part_colors.name == name)
            return (uint32_t)i;
    }

    return (uint32_t)-1;
}

bool BcsFile::GetPreviewColor(const std::string &name, uint32_t color_id, uint32_t *preview_color) const
{
    uint32_t id = FindPartColorsByName(name);

    if (id == (uint32_t)-1)
        return false;

    const BcsPartColors &part_colors = parts_colors[id];

    if (color_id >= part_colors.colors.size())
        return false;

    *preview_color = part_colors.colors[color_id].GetColorPreview(part_colors.name == "eye_");
    return true;
}

bool BcsFile::ListPreviewColors(const std::string &name, std::vector<uint32_t> &colors) const
{
    uint32_t id = FindPartColorsByName(name);

    if (id == (uint32_t)-1)
        return false;

    const BcsPartColors &part_colors = parts_colors[id];
    colors.clear();

    bool is_eyes = (part_colors.name == "eye_");

    for (const BcsColor &color : part_colors.colors)
    {
        colors.push_back(color.GetColorPreview(is_eyes));
    }

    return true;
}







