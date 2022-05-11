#include <algorithm>

#include "2ryFile.h"

#include "debug.h"

bool PtcEntry::PartialCompare(const PtcEntry *ptc1, const PtcEntry *ptc2) const
{
    if (!ptc1 && !ptc2)
        return true;
    else if (!ptc1 || !ptc2)
        return false;

    if (memcmp(ptc1->unk_10, ptc2->unk_10, sizeof(ptc1->unk_10)) != 0)
        return false;

    if ((ptc1->parent && !ptc2->parent) || (!ptc1->parent && ptc2->parent))
        return false;

    if ((ptc1->child && !ptc2->child) || (!ptc1->child && ptc2->child))
        return false;

    return true;
}

void PtcEntry::Decompile(TiXmlNode *root, const ScdEntry &scd) const
{
    TiXmlElement *entry_root = new TiXmlElement("PtcEntry");

    entry_root->SetAttribute("id", Utils::UnsignedToString(scd.PointerToIndex(this), true));
    entry_root->SetAttribute("parent", Utils::UnsignedToString(scd.PointerToIndex(parent), true));
    entry_root->SetAttribute("child", Utils::UnsignedToString(scd.PointerToIndex(child), true));
    Utils::WriteParamMultipleFloats(entry_root, "F_10", std::vector<float>(unk_10, unk_10+7));

    root->LinkEndChild(entry_root);
}

bool PtcEntry::Compile(const TiXmlElement *root, ScdEntry &scd)
{
    uint32_t parent_id, child_id;

    if (!Utils::ReadAttrUnsigned(root, "parent", &parent_id))
    {
        DPRINTF("%s: Cannot get parent.\n", FUNCNAME);
        return false;
    }

    if (parent_id == 0xFFFFFFFF)
    {
        this->parent = nullptr;
    }
    else if (parent_id >= scd.ptcs.size())
    {
        DPRINTF("%s: parent index 0x%x out of bounds.\n", FUNCNAME, parent_id);
        return false;
    }
    else
    {
        this->parent = &scd.ptcs[parent_id];
    }

    if (!Utils::ReadAttrUnsigned(root, "child", &child_id))
    {
        DPRINTF("%s: Cannot get child.\n", FUNCNAME);
        return false;
    }

    if (child_id == 0xFFFFFFFF)
    {
        this->child = nullptr;
    }
    else if (child_id >= scd.ptcs.size())
    {
        DPRINTF("%s: child index 0x%x out of bounds.\n", FUNCNAME, child_id);
        return false;
    }
    else
    {
        this->child = &scd.ptcs[child_id];
    }

    if (!Utils::GetParamMultipleFloats(root, "F_10", this->unk_10, 7))
        return false;

    return true;
}

bool PtcEntry::operator==(const PtcEntry &rhs) const
{
    if (!PartialCompare(this, &rhs))
        return false;

    if (!PartialCompare(this->parent, rhs.parent))
        return false;

    if (!PartialCompare(this->child, rhs.child))
        return false;

    return true;
}

void JntEntry::Decompile(TiXmlNode *root, const ScdEntry &scd, uint32_t id) const
{
    TiXmlElement *entry_root = new TiXmlElement("JntEntry");
    entry_root->SetAttribute("id", Utils::UnsignedToString(id, true));

    Utils::WriteParamUnsigned(entry_root, "PTC1", scd.PointerToIndex(ptc1), true);
    Utils::WriteParamUnsigned(entry_root, "PTC2", scd.PointerToIndex(ptc2), true);
    Utils::WriteParamUnsigned(entry_root, "U_08", unk_08);
    Utils::WriteParamMultipleFloats(entry_root, "F_0C", std::vector<float>(unk_0C, unk_0C+5));

    root->LinkEndChild(entry_root);
}

bool JntEntry::Compile(const TiXmlElement *root, ScdEntry &scd)
{
    uint32_t ptc1_id, ptc2_id;

    if (!Utils::GetParamUnsigned(root, "PTC1", &ptc1_id))
        return false;

    if (ptc1_id == 0xFFFFFFFF)
    {
        this->ptc1 = nullptr;
    }
    else if (ptc1_id >= scd.ptcs.size())
    {
        DPRINTF("%s: parent index 0x%x out of bounds.\n", FUNCNAME, ptc1_id);
        return false;
    }
    else
    {
        this->ptc1 = &scd.ptcs[ptc1_id];
    }

    if (!Utils::GetParamUnsigned(root, "PTC2", &ptc2_id))
        return false;

    if (ptc2_id == 0xFFFFFFFF)
    {
        this->ptc2 = nullptr;
    }
    else if (ptc2_id >= scd.ptcs.size())
    {
        DPRINTF("%s: ptc2 index 0x%x out of bounds.\n", FUNCNAME, ptc2_id);
        return false;
    }
    else
    {
        this->ptc2 = &scd.ptcs[ptc2_id];
    }

    if (!Utils::GetParamUnsigned(root, "U_08", &this->unk_08))
        return false;

    if (!Utils::GetParamMultipleFloats(root, "F_0C", this->unk_0C, 5))
        return false;

    return true;
}

bool JntEntry::operator==(const JntEntry &rhs) const
{
    if (this->unk_08 != rhs.unk_08)
        return false;

    if (memcmp(this->unk_0C, rhs.unk_0C, sizeof(this->unk_0C)) != 0)
        return false;

    if ((this->ptc1 && !rhs.ptc1) || (!this->ptc1 && rhs.ptc1))
        return false;

    if (this->ptc1 && *this->ptc1 != *rhs.ptc1)
        return false;

    if ((this->ptc2 && !rhs.ptc2) || (!this->ptc2 && rhs.ptc2))
        return false;

    if (this->ptc2 && *this->ptc2 != *rhs.ptc2)
        return false;

    return true;
}

void ColEntry::Decompile(TiXmlNode *root, uint32_t id) const
{
    TiXmlElement *entry_root = new TiXmlElement("ColEntry");
    entry_root->SetAttribute("id", Utils::UnsignedToString(id, true));

    Utils::WriteParamString(entry_root, "BONE", bone);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04);
    Utils::WriteParamMultipleFloats(entry_root, "F_94", std::vector<float>(unk_94, unk_94+8));

    root->LinkEndChild(entry_root);
}

bool ColEntry::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamString(root, "BONE", bone))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04))
        return false;

    if (!Utils::GetParamMultipleFloats(root, "F_94", unk_94, 8))
        return false;

    return true;
}

void ScdEntry::Copy(const ScdEntry &other)
{
    this->name = other.name;
    this->ptc_roots = other.ptc_roots;
    this->ptcs = other.ptcs;
    this->jnts = other.jnts;
    this->cols = other.cols;
    this->bones = other.bones;

    this->unk_08 = other.unk_08;
    this->unk_0C = other.unk_0C;

    for (PtcEntry * &pptc : ptc_roots)
    {
        pptc = &ptcs[other.PointerToIndex(pptc)];
    }

    for (PtcEntry &ptc : ptcs)
    {
        if (ptc.parent)
        {
            ptc.parent = &ptcs[other.PointerToIndex(ptc.parent)];
            assert(ptc.parent != nullptr);
        }

        if (ptc.child)
        {
            ptc.child = &ptcs[other.PointerToIndex(ptc.child)];
            assert(ptc.child != nullptr);
        }
    }

    for (JntEntry &jnt : jnts)
    {
        if (jnt.ptc1)
        {
            jnt.ptc1 = &ptcs[other.PointerToIndex(jnt.ptc1)];
            assert(jnt.ptc1 != nullptr);
        }

        if (jnt.ptc2)
        {
            jnt.ptc2 = &ptcs[other.PointerToIndex(jnt.ptc2)];
        }
    }
}

uint32_t ScdEntry::PointerToIndex(const PtcEntry *ptc) const
{
    if (ptc == nullptr)
        return (uint32_t)-1;

    for (size_t i = 0; i < ptcs.size(); i++)
    {
        if (&ptcs[i] == ptc)
            return i;
    }

    DPRINTF("%s: we shouldn't be here!\n", FUNCNAME);
    assert(0);
    return (uint32_t)-1;
}

uint32_t ScdEntry::BoneNameToIndex(const std::string &name) const
{
    if (name == "")
        return (uint32_t)-1;

    for (size_t i = 0; i < bones.size(); i++)
    {
        if (name == bones[i])
            return i;
    }

    DPRINTF("%s: we shouldn't be here!\n", FUNCNAME);
    assert(0);
    return (uint32_t)-1;
}

void ScdEntry::Decompile(TiXmlNode *root) const
{
    std::vector<uint32_t> ptc_roots_id;

    TiXmlElement *entry_root = new TiXmlElement("ScdEntry");
    entry_root->SetAttribute("name", name);

    Utils::WriteParamFloat(entry_root, "F_08", unk_08);
    Utils::WriteParamFloat(entry_root, "F_0C", unk_0C);

    for (PtcEntry * const &pptc: ptc_roots)
    {
        ptc_roots_id.push_back(PointerToIndex(pptc));
    }

    Utils::WriteParamMultipleUnsigned(entry_root, "PTC_ROOTS", ptc_roots_id, true);
    Utils::WriteParamMultipleStrings(entry_root, "BONES", bones);

    for (const PtcEntry &ptc : ptcs)
    {
        ptc.Decompile(entry_root, *this);
    }

    for (size_t i = 0; i < jnts.size(); i++)
    {
        jnts[i].Decompile(entry_root, *this, i);
    }

    for (size_t i = 0; i < cols.size(); i++)
    {
        cols[i].Decompile(entry_root, i);
    }

    root->LinkEndChild(entry_root);
}

bool ScdEntry::Compile(const TiXmlElement *root)
{
    std::vector<bool> initialized;
    std::vector<uint32_t> ptc_roots_id;
    uint32_t num_ptc, num_jnt, num_col;

    ptc_roots.clear();
    ptcs.clear();
    jnts.clear();
    cols.clear();
    bones.clear();

    if (!Utils::ReadAttrString(root, "name", name))
    {
        DPRINTF("%s: cannot get attribute \"name\"\n", FUNCNAME);
        return false;
    }

    if (!Utils::GetParamFloat(root, "F_08", &unk_08))
        return false;

    if (!Utils::GetParamFloat(root, "F_0C", &unk_0C))
        return false;

    if (!Utils::GetParamMultipleStrings(root, "BONES", bones))
        return false;

    num_ptc = Utils::GetElemCount(root, "PtcEntry");
    if (num_ptc > 0)
    {
        ptcs.resize(num_ptc);
        initialized.resize(num_ptc);

        for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
        {
            if (elem->ValueStr() == "PtcEntry")
            {
                uint32_t id;

                if (!Utils::ReadAttrUnsigned(elem, "id", &id))
                {
                    DPRINTF("%s: Cannot read attribute id.\n", FUNCNAME);
                    return false;
                }

                if (id >= ptcs.size())
                {
                    DPRINTF("%s: PtcEntry id 0x%x out of range.\n", FUNCNAME, id);
                    return false;
                }

                if (initialized[id])
                {
                    DPRINTF("%s: PtcEntry id 0x%x was already specified.\n", FUNCNAME, id);
                    return false;
                }

                if (!ptcs[id].Compile(elem, *this))
                    return false;

                initialized[id] = true;
            }
        }

    }

    num_jnt = Utils::GetElemCount(root, "JntEntry");
    if (num_jnt > 0)
    {
        jnts.resize(num_jnt);
        initialized.clear();
        initialized.resize(num_jnt);

        for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
        {
            if (elem->ValueStr() == "JntEntry")
            {
                uint32_t id;

                if (!Utils::ReadAttrUnsigned(elem, "id", &id))
                    return false;

                if (id >= jnts.size())
                {
                    DPRINTF("%s: JntEntry id 0x%x out of range.\n", FUNCNAME, id);
                    return false;
                }

                if (initialized[id])
                {
                    DPRINTF("%s: JntEntry id 0x%x was already specified.\n", FUNCNAME, id);
                    return false;
                }

                if (!jnts[id].Compile(elem, *this))
                    return false;

                initialized[id] = true;
            }
        }
    }

    num_col = Utils::GetElemCount(root, "ColEntry");
    if (num_col > 0)
    {
        cols.resize(num_col);
        initialized.clear();
        initialized.resize(num_col);

        for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
        {
            if (elem->ValueStr() == "ColEntry")
            {
                uint32_t id;

                if (!Utils::ReadAttrUnsigned(elem, "id", &id))
                    return false;

                if (id >= cols.size())
                {
                    DPRINTF("%s: ColEntry id 0x%x out of range.\n", FUNCNAME, id);
                    return false;
                }

                if (initialized[id])
                {
                    DPRINTF("%s: ColEntry id 0x%x was already specified.\n", FUNCNAME, id);
                    return false;
                }

                if (!cols[id].Compile(elem))
                    return false;

                if (cols[id].bone != "")
                {
                    if (std::find(bones.begin(), bones.end(), cols[id].bone) == bones.end())
                    {
                        DPRINTF("%s: Bone %s specified in ColEntry 0x%x doesn't exist in BONES\n",
                                FUNCNAME, cols[id].bone.c_str(), id);
                    }
                }

                initialized[id] = true;
            }
        }
    }

    if (!Utils::GetParamMultipleUnsigned(root, "PTC_ROOTS", ptc_roots_id))
        return false;

    for (uint32_t id : ptc_roots_id)
    {
        if (id >= ptcs.size())
        {
            DPRINTF("%s: id 0x%x out of range in \"PTC_ROOTS\"\n", FUNCNAME, id);
            return false;
        }

        ptc_roots.push_back(&ptcs[id]);
    }

    return true;
}

bool ScdEntry::operator==(const ScdEntry &rhs) const
{
    if (name != rhs.name)
        return false;

    if (ptcs != rhs.ptcs)
        return false;

    if (jnts != rhs.jnts)
        return false;

    if (cols != rhs.cols)
        return false;

    if (bones != rhs.bones)
        return false;

    if (unk_08 != rhs.unk_08)
        return false;

    if (unk_0C != rhs.unk_0C)
        return false;

    if (ptc_roots.size() != rhs.ptc_roots.size())
        return false;

    for (size_t i = 0; i < ptc_roots.size(); i++)
    {
        if (PointerToIndex(ptc_roots[i]) != rhs.PointerToIndex(rhs.ptc_roots[i]))
            return false;
    }

    return true;
}

_2ryFile::_2ryFile()
{
    this->big_endian = false;
    Reset();
}

_2ryFile::~_2ryFile()
{
    Reset();
}

void _2ryFile::Reset()
{
    scds.clear();
}

bool _2ryFile::Load(const uint8_t *buf, size_t size)
{
    _2RYHeader *hdr = (_2RYHeader *)buf;

    if (size < sizeof(_2RYHeader) || memcmp(hdr->signature, _2RY_SIGNATURE, 4) != 0)
        return false;

    big_endian = (buf[4] != 0xFE);

    unk_06[0] = hdr->unk_06[0];
    unk_06[1] = hdr->unk_06[1];

    if (hdr->unk_08[0] != 0 || hdr->unk_08[1] != 0)
    {
        DPRINTF("%s: unk_08 is not zero.\n", FUNCNAME);
        return false;
    }

    SCDListHeader *scdl_hdr = (SCDListHeader *)GetOffsetPtr(hdr, sizeof(_2RYHeader), true);
    uint32_t *scd_table = (uint32_t *)GetOffsetPtr(scdl_hdr, scdl_hdr->scd_table_offset);

    SCDNamesListHeader *scdnl_hdr = (SCDNamesListHeader *)GetOffsetPtr(scdl_hdr, sizeof(SCDListHeader), true);
    uint32_t *scd_names_table = (uint32_t *)GetOffsetPtr(scdnl_hdr, scdnl_hdr->scd_names_table_offset);

    if (scdl_hdr->num_scd != scdnl_hdr->num_scd_names)
    {
        DPRINTF("%s: number of scd doesn't match number of scd names.\n", FUNCNAME);
        return false;
    }

    scds.resize(val32(scdl_hdr->num_scd));

    for (size_t i = 0; i < scds.size(); i++)
    {
        ScdEntry &scd = scds[i];
        SCDHeader *shdr = (SCDHeader *)GetOffsetPtr(scd_table, scd_table, i);
        std::vector<uint32_t> ptc_roots_id;

        // This aren't null terminated but space terminated and last one is "file terminated"
        const char *name = (const char *)GetOffsetPtr(scd_names_table, scd_names_table, i);
        const char *top = (const char *)(buf + size);

        while (*name != ' ' && name < top)
        {
            scd.name += *name;
            name++;
        }

        if (memcmp(shdr->signature, SCD_SIGNATURE, 4) != 0)
        {
            DPRINTF("%s: Invalid SCD signature, for index 0x%x\n", FUNCNAME, (uint32_t)i);
            return false;
        }

        scd.unk_08 = val_float(shdr->unk_08);
        scd.unk_0C = val_float(shdr->unk_0C);

        PtcRootsListHeader *pr_hdr = (PtcRootsListHeader *)GetOffsetPtr(shdr, sizeof(SCDHeader), true);
        uint32_t *prs = (uint32_t *)GetOffsetPtr(pr_hdr, pr_hdr->ptc_roots_offset);

        if (pr_hdr->ptc_roots_offset == 0xFFFFFFFF)
        {
            DPRINTF("%s: we weren't expecting this ptc_roots_offset, in SCD 0x%x\n", FUNCNAME, (uint32_t)i);
            return false;
        }

        ptc_roots_id.resize(val32(pr_hdr->num_ptc_roots));

        for (size_t j = 0; j < ptc_roots_id.size(); j++)
        {
            ptc_roots_id[j] = val32(prs[j]);
        }

        PTCListHeader *pl_hdr = (PTCListHeader *)GetOffsetPtr(pr_hdr, sizeof(PtcRootsListHeader), true);
        PTCEntry *ptcs = (PTCEntry *)GetOffsetPtr(pl_hdr, pl_hdr->ptc_offset);

        if (pl_hdr->ptc_offset == 0xFFFFFFFF)
        {
            DPRINTF("%s: we weren't expecting this ptc_offset, in SCD 0x%x\n", FUNCNAME, (uint32_t)i);
            return false;
        }

        scd.ptcs.resize(val32(pl_hdr->num_ptc));

        for (size_t j = 0; j < scd.ptcs.size(); j++)
        {
            PtcEntry &ptc = scd.ptcs[j];

            if (memcmp(ptcs[j].signature, PTC_SIGNATURE, 4) != 0)
            {
                DPRINTF("%s: Invalid PTC signature, in SCD 0x%x, in PTC 0x%x\n", FUNCNAME, (uint32_t)i, (uint32_t)j);
                return false;
            }

            if (val32(ptcs[j].id) != j)
            {
                DPRINTF("%s: PTC id doesn't match position in the list, in SCD 0x%x, in PTC 0x%x\n", FUNCNAME, (uint32_t)i, (uint32_t)j);
                return false;
            }

            if (ptcs[j].unk_2C != 0)
            {
                DPRINTF("%s: unk_2C not 0 as expected, in SCD 0x%x, in PTC 0x%x, near offset 0x%x\n", FUNCNAME, (uint32_t)i, (uint32_t)j, Utils::DifPointer(ptcs+j, buf));
                return false;
            }

            for (int k = 0; k < 7; k++)
            {
                ptc.unk_10[k] = val_float(ptcs[j].unk_10[k]);
            }

            if (ptcs[j].parent == 0xFFFFFFFF)
            {
                ptc.parent = nullptr;
            }
            else
            {
                uint32_t index = val32(ptcs[j].parent);

                if (index >= scd.ptcs.size())
                {
                    DPRINTF("%s: parent index out of bounds, in SCD 0x%x, in PTC 0x%x\n", FUNCNAME, (uint32_t)i, (uint32_t)j);
                    return false;
                }

                ptc.parent = &scd.ptcs[index];
            }

            if (ptcs[j].child == 0xFFFFFFFF)
            {
                ptc.child = nullptr;
            }
            else
            {
                uint32_t index = val32(ptcs[j].child);

                if (index >= scd.ptcs.size())
                {
                    DPRINTF("%s: child index out of bounds, in SCD 0x%x, in PTC 0x%x\n", FUNCNAME, (uint32_t)i, (uint32_t)j);
                    return false;
                }

                ptc.child = &scd.ptcs[index];
            }
        }

        JNTListHeader *jl_hdr = (JNTListHeader *)GetOffsetPtr(pl_hdr, sizeof(PTCListHeader), true);
        JNTEntry *jnts = (JNTEntry *)GetOffsetPtr(jl_hdr, jl_hdr->jnt_offset);

        if (jl_hdr->jnt_offset == 0xFFFFFFFF)
        {
            DPRINTF("%s: we weren't expecting this jnt_offset, in SCD 0x%x.\n", FUNCNAME, (uint32_t)i);
            return false;
        }

        scd.jnts.resize(val32(jl_hdr->num_jnt));

        for (size_t j = 0; j < scd.jnts.size(); j++)
        {
            JntEntry &jnt = scd.jnts[j];

            if (memcmp(jnts[j].signature, JNT_SIGNATURE, 4) != 0)
            {
                DPRINTF("%s: Invalid JNT signature, in SCD 0x%x, in JNT 0x%x\n", FUNCNAME, (uint32_t)i, (uint32_t)j);
                return false;
            }

            if (jnts[j].unk_20[0] != 0 || jnts[j].unk_20[1] != 0 || jnts[j].unk_20[2] != 0 || jnts[j].unk_20[3] != 0)
            {
                DPRINTF("%s: unk_20 not 0 as expected, in SCD 0x%x, in JNT 0x%x\n", FUNCNAME, (uint32_t)i, (uint32_t)j);
                return false;
            }

            jnt.unk_08 = val32(jnts[j].unk_08);

            for (int k = 0; k < 5; k++)
            {
                jnt.unk_0C[k] = val_float(jnts[j].unk_0C[k]);
            }

            uint16_t index1 = val16(jnts[j].ptc1);
            uint16_t index2 = val16(jnts[j].ptc2);

            if (index1 == 0xFFFF)
            {
                jnt.ptc1 = nullptr;
            }
            else
            {
                if (index1 >= scd.ptcs.size())
                {
                    DPRINTF("%s: ptc index1 out of bounds, in SCD entry 0x%x, in JNT 0x%x\n", FUNCNAME, (uint32_t)i, (uint32_t)j);
                    return false;
                }

                jnt.ptc1 = &scd.ptcs[index1];
            }

            if (index2 == 0xFFFF)
            {
                jnt.ptc2 = nullptr;
            }
            else
            {
                if (index2 >= scd.ptcs.size())
                {
                    DPRINTF("%s: ptc index2 out of bounds, in SCD entry 0x%x, in JNT 0x%x\n", FUNCNAME, (uint32_t)i, (uint32_t)j);
                    return false;
                }

                jnt.ptc2 = &scd.ptcs[index2];
            }
        }

        COLListHeader *cl_hdr = (COLListHeader *)GetOffsetPtr(jl_hdr, sizeof(JNTListHeader), true);
        COLEntry *cols = (COLEntry *)GetOffsetPtr(cl_hdr, cl_hdr->col_offset);

        if (cl_hdr->col_offset == 0xFFFFFFFF)
        {
            DPRINTF("%s: we weren't expecting this col_offset, in SCD 0x%x.\n", FUNCNAME, (uint32_t)i);
            return false;
        }

        scd.cols.resize(val32(cl_hdr->num_col));

        BonesNamesListHeader *bl_hdr = (BonesNamesListHeader *)GetOffsetPtr(cl_hdr, sizeof(COLListHeader), true);
        uint32_t *bones_names_table = (uint32_t *)GetOffsetPtr(bl_hdr, bl_hdr->bones_names_offset);

        if (bl_hdr->bones_names_offset == 0xFFFFFFFF)
        {
            DPRINTF("%s: we weren't expecting this bones_names_offset, in SCD 0x%x.\n", FUNCNAME, (uint32_t)i);
            return false;
        }

        uint32_t *bones_names_table2 = (uint32_t *)GetOffsetPtr(shdr, shdr->bones_names_offset);

        if (bones_names_table != bones_names_table2)
        {
            DPRINTF("The bones table are not same as expected.\n");
            return false;
        }

        scd.bones.resize(val32(bl_hdr->num_bone_names));

        for (size_t j = 0; j < scd.bones.size(); j++)
        {
            const std::string base_name = (char *)GetOffsetPtr(bones_names_table, bones_names_table, j);
            std::string name = base_name;

            if (std::find(scd.bones.begin(), scd.bones.end(), name) != scd.bones.end())
            {
                for (int k = 1; ; k++)
                {
                    name = base_name + ':' + Utils::ToString(k);

                    if (std::find(scd.bones.begin(), scd.bones.end(), name) == scd.bones.end())
                        break;
                }
            }

            scd.bones[j] = name;
        }

        for (size_t j = 0; j < scd.cols.size(); j++)
        {
            ColEntry &col = scd.cols[j];

            if (memcmp(cols[j].signature, COL_SIGNATURE, 4) != 0)
            {
                DPRINTF("%s: Invalid COL signature, in SCD 0x%x, in COL 0x%x\n", FUNCNAME, (uint32_t)i, (uint32_t)j);
                return false;
            }

            for (int k = 0; k < 34; k++)
            {
                if (cols[j].unk_08[k] != 0)
                {
                    DPRINTF("%s: unk_08 not 0 as expected, in SCD 0x%x, in COL 0x%x\n", FUNCNAME, (uint32_t)i, (uint32_t)j);
                    return false;
                }
            }

            if (cols[j].unk_B4[0] != 0 || cols[j].unk_B4[1] != 0 || cols[j].unk_B4[2] != 0)
            {
                DPRINTF("%s: unk_B4 not 0 as expected, in SCD 0x%x, in COL 0x%x\n", FUNCNAME, (uint32_t)i, (uint32_t)j);
                return false;
            }

            col.unk_04 = val32(cols[j].unk_04);

            for (int k = 0; k < 8; k++)
            {
                col.unk_94[k] = val_float(cols[j].unk_94[k]);
            }

            uint32_t index = val32(cols[j].bone_idx);

            if (index == 0xFFFFFFFF)
            {
                col.bone = "";
            }
            else
            {
                if (index >= scd.bones.size())
                {
                    DPRINTF("%s: bone index out of bounds, in SCD entry 0x%x, in COL 0x%x\n", FUNCNAME, (uint32_t)i, (uint32_t)j);
                    return false;
                }

                col.bone = scd.bones[index];                
            }
        }

        Unknown2ListHeader *ul2_hdr = (Unknown2ListHeader *)GetOffsetPtr(bl_hdr, sizeof(BonesNamesListHeader), true);

        if (ul2_hdr->unk[0] != 0xFFFFFFFF || ul2_hdr->unk[1] != 0xFFFFFFFF)
        {
            DPRINTF("%s: ul2 unk 0 not as expected, in SCD 0x%x\n", FUNCNAME, (uint32_t)i);
            return false;
        }

        // Now check for ptc roots
        for (uint32_t id : ptc_roots_id)
        {
            if (id >= scd.ptcs.size())
            {
                DPRINTF("%s: id 0x%x out of range in ptc_roots_id.\n", FUNCNAME, id);
                return false;
            }

            scd.ptc_roots.push_back(&scd.ptcs[id]);
        }
    }

    return true;
}

unsigned int _2ryFile::CalculateFileSize() const
{
    unsigned int file_size = sizeof(_2RYHeader) + sizeof(SCDListHeader) + sizeof(SCDNamesListHeader);

    file_size += (scds.size() * sizeof(uint32_t));
    file_size += (0x10 - (file_size & 0xF));

    for (const ScdEntry &scd : scds)
    {
        file_size += sizeof(SCDHeader);
        file_size += sizeof(PtcRootsListHeader) + sizeof(PTCListHeader);
        file_size += sizeof(JNTListHeader) + sizeof(COLListHeader);
        file_size += sizeof(BonesNamesListHeader) + sizeof(Unknown2ListHeader);

        file_size += scd.ptc_roots.size() * sizeof(uint32_t);

        file_size += (0x10 - (file_size & 0xF));

        file_size += scd.ptcs.size() * sizeof(PTCEntry);
        file_size += scd.jnts.size() * sizeof(JNTEntry);
        file_size += scd.cols.size() * sizeof(COLEntry);

        file_size += scd.bones.size() * sizeof(uint32_t);

        file_size += (0x10 - (file_size & 0xF));

        for (const std::string &s : scd.bones)
        {
            size_t pos = s.find(':');

            assert(pos != 0);

            if (pos == std::string::npos)
                file_size += s.length() + 1;
            else
                file_size += pos + 1;
        }

        file_size += (0x10 - (file_size & 0xF));
    }

    file_size += scds.size() * sizeof(uint32_t);
    file_size += (0x10 - (file_size & 0xF));

    for (const ScdEntry &scd : scds)
    {
        file_size += scd.name.length() + 1;
    }

    file_size = file_size - 1; // Remove from last string

    return file_size;
}

uint8_t *_2ryFile::Save(size_t *psize)
{
    size_t file_size = CalculateFileSize();
    uint32_t offset;
	
	uint8_t *buf = new uint8_t[file_size];
    memset(buf, 0, file_size);

    _2RYHeader *hdr = (_2RYHeader *)buf;
    memcpy(hdr->signature, _2RY_SIGNATURE, 4);
    hdr->endianess_check = val16(0xFFFE);
    hdr->unk_06[0] = unk_06[0];
    hdr->unk_06[1] = unk_06[1];

    offset = sizeof(_2RYHeader);

    SCDListHeader *scdl_hdr = (SCDListHeader *)GetOffsetPtr(buf, offset, true);
    scdl_hdr->num_scd = val32(scds.size());

    offset += sizeof(SCDListHeader);

    SCDNamesListHeader *scdnl_hdr = (SCDNamesListHeader *)GetOffsetPtr(buf, offset, true);
    scdnl_hdr->num_scd_names = val32(scds.size());

    offset += sizeof(SCDNamesListHeader);

    uint32_t *scd_table = (uint32_t *)GetOffsetPtr(buf, offset, true);
    scdl_hdr->scd_table_offset = val32(Utils::DifPointer(scd_table, scdl_hdr));

    offset += (scds.size() * sizeof(uint32_t));
    offset += (0x10 - (offset & 0xF));

    for (size_t i = 0; i < scds.size(); i++)
    {
        const ScdEntry &scd = scds[i];
        SCDHeader *shdr = (SCDHeader *)GetOffsetPtr(buf, offset, true);

        assert(scd.ptcs.size() < 65536);

        scd_table[i] = val32(Utils::DifPointer(shdr, scd_table));

        memcpy(shdr->signature, SCD_SIGNATURE, 4);
        copy_float(&shdr->unk_08, scd.unk_08);
        copy_float(&shdr->unk_0C, scd.unk_0C);

        offset += sizeof(SCDHeader);

        PtcRootsListHeader *pr_hdr = (PtcRootsListHeader *)GetOffsetPtr(buf, offset, true);
        offset += sizeof(PtcRootsListHeader);

        PTCListHeader *pl_hdr = (PTCListHeader *)GetOffsetPtr(buf, offset, true);
        offset += sizeof(PTCListHeader);

        JNTListHeader *jl_hdr = (JNTListHeader *)GetOffsetPtr(buf, offset, true);
        offset += sizeof(JNTListHeader);

        COLListHeader *cl_hdr = (COLListHeader *)GetOffsetPtr(buf, offset, true);
        offset += sizeof(COLListHeader);

        BonesNamesListHeader *bl_hdr = (BonesNamesListHeader *)GetOffsetPtr(buf, offset, true);
        offset += sizeof(BonesNamesListHeader);

        Unknown2ListHeader *ul2_hdr = (Unknown2ListHeader *)GetOffsetPtr(buf, offset, true);
        offset += sizeof(Unknown2ListHeader);

        uint32_t *ptc_roots_id = (uint32_t *)GetOffsetPtr(buf, offset, true);
        offset += scd.ptc_roots.size() * sizeof(uint32_t);
        offset += (0x10 - (offset & 0xF));

        PTCEntry *ptcs = (PTCEntry *)GetOffsetPtr(buf, offset, true);
        offset += scd.ptcs.size() * sizeof(PTCEntry);

        JNTEntry *jnts = (JNTEntry *)GetOffsetPtr(buf, offset, true);
        offset += scd.jnts.size() * sizeof(JNTEntry);

        COLEntry *cols = (COLEntry *)GetOffsetPtr(buf, offset, true);
        offset += scd.cols.size() * sizeof(COLEntry);

        uint32_t *bones_names_table = (uint32_t *)GetOffsetPtr(buf, offset, true);
        offset += scd.bones.size() * sizeof(uint32_t);
        offset += (0x10 - (offset & 0xF));

        for (size_t j = 0; j < scd.bones.size(); j++)
        {
            std::string bone = scd.bones[j];
            size_t pos = bone.find(':');

            if (pos != std::string::npos)
            {
                bone = bone.substr(0, pos);
            }

            bones_names_table[j] = val32(Utils::DifPointer(buf + offset, bones_names_table));
            strcpy((char *)buf + offset, bone.c_str());
            offset += bone.length() + 1;
        }

        offset += (0x10 - (offset & 0xF)); // Now points to next scd entry;

        pr_hdr->num_ptc_roots = val32(scd.ptc_roots.size());
        pr_hdr->ptc_roots_offset = val32(Utils::DifPointer(ptc_roots_id, pr_hdr));

        pl_hdr->num_ptc = val32(scd.ptcs.size());
        pl_hdr->ptc_offset = val32(Utils::DifPointer(ptcs, pl_hdr));

        jl_hdr->num_jnt = val32(scd.jnts.size());
        jl_hdr->jnt_offset = val32(Utils::DifPointer(jnts, jl_hdr));

        cl_hdr->num_col = val32(scd.cols.size());
        cl_hdr->col_offset = val32(Utils::DifPointer(cols, cl_hdr));

        bl_hdr->num_bone_names = val32(scd.bones.size());
        bl_hdr->bones_names_offset = val32(Utils::DifPointer(bones_names_table, bl_hdr));
        shdr->bones_names_offset = val32(Utils::DifPointer(bones_names_table, shdr));

        ul2_hdr->unk[0] = ul2_hdr->unk[1] = 0xFFFFFFFF;

        for (size_t j = 0; j < scd.ptc_roots.size(); j++)
        {
            ptc_roots_id[j] = val32(scd.PointerToIndex(scd.ptc_roots[j]));
        }

        for (size_t j = 0; j < scd.ptcs.size(); j++)
        {
            const PtcEntry &ptc = scd.ptcs[j];

            memcpy(ptcs[j].signature, PTC_SIGNATURE, 4);
            ptcs[j].id = val32(j);
            ptcs[j].parent = val32(scd.PointerToIndex(ptc.parent));
            ptcs[j].child = val32(scd.PointerToIndex(ptc.child));

            for (int k = 0; k < 7; k++)
            {
                copy_float(ptcs[j].unk_10+k, ptc.unk_10[k]);
            }
        }

        for (size_t j = 0; j < scd.jnts.size(); j++)
        {
            const JntEntry &jnt = scd.jnts[j];

            memcpy(jnts[j].signature, JNT_SIGNATURE, 4);
            jnts[j].ptc1 = val16(scd.PointerToIndex(jnt.ptc1));
            jnts[j].ptc2 = val16(scd.PointerToIndex(jnt.ptc2));
            jnts[j].unk_08 = val32(jnt.unk_08);

            for (int k = 0; k < 5; k++)
            {
                copy_float(jnts[j].unk_0C+k, jnt.unk_0C[k]);
            }
        }

        for (size_t j = 0; j < scd.cols.size(); j++)
        {
            const ColEntry &col = scd.cols[j];

            memcpy(cols[j].signature, COL_SIGNATURE, 4);
            cols[j].unk_04 = val32(col.unk_04);
            cols[j].bone_idx = val32(scd.BoneNameToIndex(col.bone));

            for (int k = 0; k < 8; k++)
            {
                copy_float(cols[j].unk_94+k, col.unk_94[k]);
            }
        }
    }

    uint32_t *scd_names_table = (uint32_t *)GetOffsetPtr(buf, offset, true);
    scdnl_hdr->scd_names_table_offset = val32(Utils::DifPointer(scd_names_table, scdnl_hdr));

    offset += scds.size() * sizeof(uint32_t);
    offset += (0x10 - (offset & 0xF));

    for (size_t i = 0; i < scds.size(); i++)
    {
        scd_names_table[i] = val32(Utils::DifPointer(buf + offset, scd_names_table));

        strcpy((char *)buf+offset, scds[i].name.c_str());
        offset += scds[i].name.length();

        if (i != (scds.size()-1))
        {
            buf[offset++] = ' ';
        }
    }

    assert(offset == file_size);

    *psize = file_size;
    return buf;
}

TiXmlDocument *_2ryFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();
    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("TWORY");
    Utils::WriteParamMultipleUnsigned(root, "U_06", std::vector<uint8_t>(unk_06, unk_06+2), true);

    for (const ScdEntry &scd : scds)
    {
        scd.Decompile(root);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool _2ryFile::Compile(TiXmlDocument *doc, bool big_endian)
{
    Reset();
    this->big_endian = big_endian;

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "TWORY");

    if (!root)
    {
        DPRINTF("%s: Cannot find TWORY in xml.\n", FUNCNAME);
        return false;
    }

    if (!Utils::GetParamMultipleUnsigned(root, "U_06", unk_06, 2))
        return false;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "ScdEntry")
        {
            ScdEntry scd;

            scds.push_back(scd);

            if (!scds[scds.size()-1].Compile(elem))
                return false;
        }
    }

    return true;
}



