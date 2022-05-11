#include "EmaFile.h"

#include "debug.h"

void EmaStep::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Step");

    Utils::WriteParamUnsigned(entry_root, "TIME", time);
    Utils::WriteParamUnsigned(entry_root, "INDEX", index, true);

    root->LinkEndChild(entry_root);
}

bool EmaStep::Compile(const TiXmlElement *root)
{
    uint32_t time;

    if (!Utils::GetParamUnsigned(root, "TIME", &time))
        return false;

    if (time > 65535)
    {
        DPRINTF("%s: time can't be bigger than 65535.\n", FUNCNAME);
        return false;
    }

    this->time = time;

    if (!Utils::GetParamUnsigned(root, "INDEX", &index))
        return false;

    return true;
}

void EmaCommand::Decompile(TiXmlNode *root, uint32_t id) const
{
    TiXmlElement *entry_root = new TiXmlElement("Command");
    entry_root->SetAttribute("id", Utils::UnsignedToString(id, true));

    if (bone)
        Utils::WriteParamString(entry_root, "BONE", bone->GetName());

    Utils::WriteParamUnsigned(entry_root, "TRANSFORM", transform, true);
    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);

    for (const EmaStep &step : steps)
    {
        step.Decompile(entry_root);
    }

    root->LinkEndChild(entry_root);
}

bool EmaCommand::Compile(const TiXmlElement *root, SkeletonFile &skl)
{
    std::string bone;
    uint32_t transform, flags;

    if (skl.GetNumBones() > 0)
    {
        if (!Utils::GetParamString(root, "BONE", bone))
            return false;

        this->bone = skl.GetBone(bone);
        if (!this->bone)
        {
            DPRINTF("%s: Bone \"%s\" doesn't exist.\n", FUNCNAME, bone.c_str());
            return false;
        }
    }
    else
    {
        this->bone = nullptr;
    }

    if (!Utils::GetParamUnsigned(root, "TRANSFORM", &transform))
        return false;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (transform > 255)
    {
        DPRINTF("%s: \"TRANSFORM\" cannot be bigger than 255.\n", FUNCNAME);
        return false;
    }

    if (flags > 255)
    {
        DPRINTF("%s: \"FLAGS\" cannot be bigger than 255.\n", FUNCNAME);
        return false;
    }

    this->transform = transform;
    this->flags = flags;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Step")
        {
            EmaStep step;

            if (!step.Compile(elem))
                return false;

            steps.push_back(step);
        }
    }

    return true;
}

void EmaAnimation::Decompile(TiXmlNode *root, uint32_t id) const
{
     TiXmlElement *entry_root = new TiXmlElement("Animation");
     entry_root->SetAttribute("id", Utils::UnsignedToString(id, true));
     entry_root->SetAttribute("name", name);

     Utils::WriteParamUnsigned(entry_root, "DURATION", duration);
     Utils::WriteParamUnsigned(entry_root, "U_08", unk_08, true);

     for (size_t i = 0; i < commands.size(); i++)
     {
         commands[i].Decompile(entry_root, i);
     }

     Utils::WriteParamMultipleFloats(entry_root, "VALUES", values);
     //Utils::WriteParamBlob(entry_root, "VALUES", (const uint8_t *)values.data(), values.size() * sizeof(float));

     root->LinkEndChild(entry_root);
}

bool EmaAnimation::Compile(const TiXmlElement *root, SkeletonFile &skl)
{
    uint32_t duration;

    if (!Utils::ReadAttrString(root, "name", name))
        return false;

    if (!Utils::GetParamUnsigned(root, "DURATION", &duration))
        return false;

    if (duration > 65535)
    {
        DPRINTF("%s: duration cannot be bigger than 65535 (on animation \"%s\")\n", FUNCNAME, name.c_str());
        return false;
    }

    this->duration = duration;

    if (!Utils::GetParamUnsigned(root, "U_08", &unk_08))
        return false;

    size_t count = Utils::GetElemCount(root, "Command");
    if (count > 0)
    {
        std::vector<bool> initialized;

        commands.resize(count);
        initialized.resize(count);

        for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
        {
            if (elem->ValueStr() == "Command")
            {
                uint32_t id;

                if (!Utils::ReadAttrUnsigned(elem, "id", &id))
                {
                    DPRINTF("%s: Cannot read attribute \"id\"\n", FUNCNAME);
                    return false;
                }

                if (id >= commands.size())
                {
                    DPRINTF("%s: Command id 0x%x out of range.\n", FUNCNAME, id);
                    return false;
                }

                if (initialized[id])
                {
                    DPRINTF("%s: Command id 0x%x was already specified.\n", FUNCNAME, id);
                    return false;
                }

                if (!commands[id].Compile(elem, skl))
                {
                    DPRINTF("%s: Compilation of Command failed.\n", FUNCNAME);
                    return false;
                }

                initialized[id] = true;
            }
        }
    }

    if (!Utils::GetParamMultipleFloats(root, "VALUES", values))
        return false;

    return true;
}

EmaFile::EmaFile()
{
    this->big_endian = false;
}

EmaFile::~EmaFile()
{
}

bool EmaFile::LinkAnimation(EmaAnimation &anim, Bone **not_found)
{
    if (!HasSkeleton())
        return true;

    EmaAnimation temp = anim;

    for (EmaCommand &c : temp)
    {
        Bone *b = GetBone(c.bone->GetName());

        if (!b)
        {
            if (not_found)
                *not_found = c.bone;

            return false;
        }

        c.bone = b;
    }

    anim = temp;
    return true;
}

uint16_t EmaFile::AppendAnimation(const EmaAnimation &animation)
{
    if (HasSkeleton())
    {
        for (const EmaCommand &c : animation.commands)
        {
            if (!BoneExists(c.bone->GetName()))
                return 0xFFFF;
        }
    }

    uint16_t id = animations.size();
    animations.push_back(animation);

    if (HasSkeleton())
    {
        for (EmaCommand &c : animations[id].commands)
        {
            c.bone = GetBone(c.bone->GetName());
        }
    }

    return id;
}

uint16_t EmaFile::AppendAnimation(const EmaFile &other, const std::string &name)
{
    const EmaAnimation *animation = other.GetAnimation(name);
    if (!animation)
        return (uint16_t)-1;

    return AppendAnimation(*animation);
}

void EmaFile::Copy(const EmaFile &other)
{
    SkeletonFile::Copy(other);

    this->animations = other.animations;
    this->unk_08 = other.unk_08;
    this->unk_12 = other.unk_12;

    if (HasSkeleton())
    {
        for (EmaAnimation &a : animations)
        {
            for (EmaCommand &c : a.commands)
            {
                c.bone = GetBone(c.bone->GetName());
                assert(c.bone != nullptr);
            }
        }
    }
}

void EmaFile::Reset()
{
    SkeletonFile::Reset();
    animations.clear();
}

bool EmaFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    EMAHeader *hdr = (EMAHeader *)buf;
    if (size < sizeof(EMAHeader) || hdr->signature != EMA_SIGNATURE)
        return false;

    this->big_endian = (buf[4] != 0xFE);

    if (hdr->skeleton_offset == 0)
    {
        // No skeleton
    }

    else
    {
        if (!SkeletonFile::Load(GetOffsetPtr(buf, hdr->skeleton_offset), size-val32(hdr->skeleton_offset)))
            return false;
    }

    unk_08 = val16(hdr->unk_08);
    unk_12 = val16(hdr->unk_12);

    uint32_t *anim_offsets = (uint32_t *)GetOffsetPtr(buf, val16(hdr->header_size), true);
    animations.resize(val16(hdr->anim_count));

    for (size_t i = 0; i < animations.size(); i++)
    {
        EmaAnimation &animation = animations[i];
        EMAAnimationHeader *ahdr = (EMAAnimationHeader *)GetOffsetPtr(buf, anim_offsets, i);

        animation.duration = val16(ahdr->duration);
        animation.name = (char *)GetOffsetPtr(ahdr, ahdr->name_offset) + 11;
        animation.unk_08 = val32(ahdr->unk_08);

        animation.commands.resize(val16(ahdr->cmd_count));

        for (size_t j = 0; j < animation.commands.size(); j++)
        {
            EmaCommand &command = animation.commands[j];
            EMAAnimationCommandHeader *chdr = (EMAAnimationCommandHeader *)GetOffsetPtr(ahdr, ahdr->cmd_offsets, j);

            if (HasSkeleton())
            {

                if (val16(chdr->bone_idx) >= GetNumBones())
                {
                    DPRINTF("Bone idx 0x%x out of bounds, in animation \"%s\", in command 0x%x\n", chdr->bone_idx, animation.name.c_str(), (uint32_t)j);
                    return false;
                }

                command.bone = &bones[val16(chdr->bone_idx)];
            }
            else
            {
                command.bone = nullptr;
            }

            command.flags = chdr->flags;
            command.transform = chdr->transform;
            command.steps.resize(val16(chdr->step_count));

            for (size_t k = 0; k < command.steps.size(); k++)
            {
                EmaStep &step = command.steps[k];

                if (command.flags & 0x20)
                {
                    uint16_t *timing = (uint16_t *)GetOffsetPtr(chdr, sizeof(EMAAnimationCommandHeader), true);
                    step.time = val16(timing[k]);
                }
                else
                {
                    uint8_t *timing = (uint8_t *)GetOffsetPtr(chdr, sizeof(EMAAnimationCommandHeader), true);
                    step.time = timing[k];
                }

                if (command.flags & 0x40)
                {
                    uint32_t *indices = (uint32_t *)GetOffsetPtr(chdr, val16(chdr->indices_offset), true);
                    step.index = val32(indices[k]);
                }
                else
                {
                    uint16_t *indices = (uint16_t *)GetOffsetPtr(chdr, val16(chdr->indices_offset), true);
                    step.index = val16(indices[k]);
                }
            }
        }

        animation.values.resize(val32(ahdr->value_count));
        float *values = (float *)GetOffsetPtr(ahdr, ahdr->values_offset);

        for (size_t j = 0; j < animation.values.size(); j++)
        {
            animation.values[j] = val_float(values[j]);
        }
    }

    return true;
}

unsigned int EmaFile::CalculateFileSize() const
{
    unsigned int file_size = sizeof(EMAHeader);

    file_size += animations.size() * sizeof(uint32_t);

    for (const EmaAnimation &a : animations)
    {
        if (file_size & 3)
            file_size += (4 - (file_size & 3));

        file_size += sizeof(EMAAnimationHeader) - sizeof(uint32_t);
        file_size += a.commands.size() * sizeof(uint32_t);

        for (const EmaCommand &c : a.commands)
        {
            if (file_size & 3)
                file_size += (4 - (file_size & 3));

            file_size += sizeof(EMAAnimationCommandHeader);

            if (c.flags & 0x20)
            {
                file_size += c.steps.size() * sizeof(uint16_t);
            }
            else
            {
                file_size += c.steps.size();
            }

            if (file_size & 3)
                file_size += (4 - (file_size & 3));

            if (c.flags & 0x40)
            {
                file_size += c.steps.size() * sizeof(uint32_t);
            }
            else
            {
                file_size += c.steps.size() * sizeof(uint16_t);
            }
        }

        if (file_size & 3)
            file_size += (4 - (file_size & 3));

        file_size += a.values.size() * sizeof(float);
    }

    if (HasSkeleton())
    {
        if (file_size & 0x3F)
            file_size += (0x40 - (file_size & 0x3F));

        file_size += SkeletonFile::CalculateFileSize();
    }

    for (const EmaAnimation &a : animations)
    {
        if (file_size & 3)
            file_size += (4 - (file_size & 3));

        assert(a.name.length() < 256);
        file_size += 11 + a.name.length()+1;
    }

    return file_size;
}

void EmaFile::RebuildSkeleton(const std::vector<Bone *> &old_bones_ptr)
{
    if (!HasSkeleton())
        return;

    SkeletonFile::RebuildSkeleton(old_bones_ptr);

    for (EmaAnimation &a : animations)
    {
        for (EmaCommand &c: a.commands)
        {
            uint16_t id = FindBone(old_bones_ptr, c.bone, false);

            if (id == 0xFFFF)
            {
                throw std::runtime_error(std::string(FUNCNAME) + ": bone " + c.bone->GetName() + " is not resolved.\n");
            }

            c.bone = &bones[id];
        }
    }
}

uint8_t *EmaFile::Save(size_t *psize)
{
    unsigned int file_size;
    uint32_t offset;

    file_size = CalculateFileSize();
    uint8_t *buf = new uint8_t[file_size];    
    memset(buf, 0, file_size);
	
    assert(animations.size() < 65536);

    EMAHeader *hdr = (EMAHeader *)buf;
    hdr->signature = EMA_SIGNATURE;
    hdr->endianess_check = val16(0xFFFE);
    hdr->header_size = val16(sizeof(EMAHeader));
    hdr->unk_08 = val16(unk_08);
    hdr->anim_count = val16(animations.size());
    hdr->unk_12 = val16(unk_12);

    offset = sizeof(EMAHeader);
    uint32_t *anim_offsets = (uint32_t *)GetOffsetPtr(buf, offset, true);

    offset += animations.size() * sizeof(uint32_t);

    for (size_t i = 0; i < animations.size(); i++)
    {
        if (offset & 3)
            offset += (4 - (offset & 3));

        const EmaAnimation &animation = animations[i];
        EMAAnimationHeader *ahdr = (EMAAnimationHeader *)GetOffsetPtr(buf, offset, true);
        anim_offsets[i] = val32(offset);

        assert(animation.commands.size() < 65536);

        ahdr->duration = val16(animation.duration);
        ahdr->cmd_count = val16(animation.commands.size());
        ahdr->value_count = val32(animation.values.size());
        ahdr->unk_08 = val32(animation.unk_08);

        offset += sizeof(EMAAnimationHeader) - sizeof(uint32_t);
        offset += animation.commands.size() * sizeof(uint32_t);

        for (size_t j = 0; j < animation.commands.size(); j++)
        {
            if (offset & 3)
                offset += (4 - (offset & 3));

            const EmaCommand &command = animation.commands[j];
            EMAAnimationCommandHeader *chdr = (EMAAnimationCommandHeader *)GetOffsetPtr(buf, offset, true);
            ahdr->cmd_offsets[j] = val32(Utils::DifPointer(buf + offset, ahdr));

            assert(command.steps.size() < 65536);

            if (HasSkeleton())
                chdr->bone_idx = val16(BoneToIndex(command.bone));
            else
                chdr->bone_idx = 0;

            chdr->transform = command.transform;
            chdr->flags = command.flags;
            chdr->step_count = val16(command.steps.size());
            offset += sizeof(EMAAnimationCommandHeader);

            if (command.flags & 0x20)
            {
                uint16_t *timing = (uint16_t *)GetOffsetPtr(buf, offset, true);

                for (size_t k = 0; k < command.steps.size(); k++)
                {
                    timing[k] = val16(command.steps[k].time);
                    offset += sizeof(uint16_t);
                }
            }
            else
            {
                uint8_t *timing = GetOffsetPtr(buf, offset, true);

                for (size_t k = 0; k < command.steps.size(); k++)
                {
                    assert(command.steps[k].time < 256);
                    timing[k] = (uint8_t)command.steps[k].time;
                    offset += sizeof(uint8_t);
                }
            }


            if (offset & 3)
                offset += (4 - (offset & 3));

            uint32_t dif = Utils::DifPointer(buf + offset, chdr);
            assert(dif < 65536);

            chdr->indices_offset = val16(dif);

            if (command.flags & 0x40)
            {
                uint32_t *indices = (uint32_t *)GetOffsetPtr(buf, offset, true);

                for (size_t k = 0; k < command.steps.size(); k++)
                {
                    indices[k] = val32(command.steps[k].index);
                    offset += sizeof(uint32_t);
                }
            }
            else
            {
                uint16_t *indices = (uint16_t *)GetOffsetPtr(buf, offset, true);

                for (size_t k = 0; k < command.steps.size(); k++)
                {
                    assert(command.steps[k].index < 65536);
                    indices[k] = val16(command.steps[k].index);
                    offset += sizeof(uint16_t);
                }
            }
        }

        if (offset & 3)
            offset += (4 - (offset & 3));

        float *values = (float *)GetOffsetPtr(buf, offset, true);
        ahdr->values_offset = val32(Utils::DifPointer(buf + offset, ahdr));

        for (size_t j = 0; j < animation.values.size(); j++)
        {
            values[j] = val_float(animation.values[j]);
            copy_float(values+j, animation.values[j]);
            offset += sizeof(float);
        }
    }

    if (HasSkeleton())
    {

        if (offset & 0x3F)
            offset += (0x40 - (offset & 0x3F));

        size_t skl_size;
        uint8_t *skl = SkeletonFile::Save(&skl_size);
        if (!skl)
        {
            delete[] buf;
            return nullptr;
        }

        hdr->skeleton_offset = val32(offset);
        memcpy(buf+offset, skl, skl_size);
        delete[] skl;

        offset += (uint32_t)skl_size;
    }

    for (size_t i = 0; i < animations.size(); i++)
    {
        const EmaAnimation &animation = animations[i];
        EMAAnimationHeader *ahdr = (EMAAnimationHeader *)GetOffsetPtr(buf, anim_offsets, i);

        if (offset & 3)
            offset += (4 - (offset & 3));

        ahdr->name_offset = val32(Utils::DifPointer(buf + offset, ahdr));
        offset += 10;

        assert(animation.name.length() < 256);
        buf[offset] = animation.name.length();
        offset++;

        strcpy((char *)buf + offset, animation.name.c_str());
        offset += animation.name.length() + 1;
    }

    assert(offset == file_size);

    *psize = file_size;
    return buf;
}

TiXmlDocument *EmaFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("EMA");

    Utils::WriteParamUnsigned(root, "U_08", unk_08, true);
    Utils::WriteParamUnsigned(root, "U_12", unk_12, true);

    for (size_t i = 0; i < animations.size(); i++)
    {
        animations[i].Decompile(root, i);
    }

    doc->LinkEndChild(root);

    if (HasSkeleton())
    {
        SkeletonFile::Decompile(doc);
    }

    return doc;
}

bool EmaFile::Compile(TiXmlDocument *doc, bool big_endian)
{
    Reset();
    this->big_endian = big_endian;

    TiXmlHandle handle(doc);

    if (Utils::FindRoot(&handle, "Skeleton"))
    {
        if (!SkeletonFile::Compile(doc, big_endian))
            return false;
    }

    const TiXmlElement *root = Utils::FindRoot(&handle, "EMA");

    if (!root)
    {
        DPRINTF("Cannot find\"EMA\" in xml.\n");
        return false;
    }

    uint32_t unk_08, unk_12;

    if (!Utils::GetParamUnsigned(root, "U_08", &unk_08))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_12", &unk_12))
        return false;

    if (unk_08 > 65535)
    {
        DPRINTF("%s: U_08 cannot be bigger than 65535.\n", FUNCNAME);
        return false;
    }

    if (unk_12 > 65535)
    {
        DPRINTF("%s: U_12 cannot be bigger than 65535.\n", FUNCNAME);
        return false;
    }

    this->unk_08 = unk_08;
    this->unk_12 = unk_12;

    size_t count = Utils::GetElemCount(root, "Animation");
    if (count > 0)
    {
        std::vector<bool> initialized;

        animations.resize(count);
        initialized.resize(count);

        for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
        {
            if (elem->ValueStr() == "Animation")
            {
                uint32_t id;

                if (!Utils::ReadAttrUnsigned(elem, "id", &id))
                {
                    DPRINTF("%s: Cannot read attribute \"id\"\n", FUNCNAME);
                    return false;
                }

                if (id >= animations.size())
                {
                    DPRINTF("%s: Animation id 0x%x out of range.\n", FUNCNAME, id);
                    return false;
                }

                if (initialized[id])
                {
                    DPRINTF("%s: Animation id 0x%x was already specified.\n", FUNCNAME, id);
                    return false;
                }

                if (!animations[id].Compile(elem, *this))
                {
                    DPRINTF("%s: Compilation of Animation failed.\n", FUNCNAME);
                    return false;
                }

                initialized[id] = true;
            }
        }
    }

    return true;
}

bool EmaFile::DecompileToFile(const std::string &path, bool show_error, bool build_path)
{
    UPRINTF("Ema is being saved to .xml file. This process may take a while.\n");
    bool ret = BaseFile::DecompileToFile(path, show_error, build_path);

    if (ret)
    {
        UPRINTF("Ema has been saved to .xml.\n");
    }

    return ret;
}

bool EmaFile::CompileFromFile(const std::string &path, bool show_error, bool big_endian)
{
    UPRINTF("Ema is being loaded from .xml file. This process may take a while.\n");
    bool ret = BaseFile::CompileFromFile(path, show_error, big_endian);

    if (ret)
    {
        UPRINTF("Ema has been loaded from .xml.\n");
    }

    return ret;
}

bool EmaFile::operator==(const EmaFile &rhs) const
{
    if (this->animations != rhs.animations)
        return false;

    if (this->unk_08 != rhs.unk_08)
        return false;

    if (this->unk_12 != rhs.unk_12)
        return false;

    if (!HasSkeleton())
    {
        if (rhs.HasSkeleton())
            return false;
    }
    else if (!rhs.HasSkeleton())
    {
        return false;
    }

    if (HasSkeleton())
    {
        if (SkeletonFile::operator ==(rhs) != true)
            return false;
    }

    return true;
}




