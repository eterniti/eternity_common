#include <stdlib.h>

#include <stdexcept>
#include <algorithm>

#include "SkeletonFile.h"
#include "debug.h"

/*
 * Structure of skeleton:
 *
 * Skeleton Header
 * node_count * SkeletonNode
 * node_count * UnkSkeletonData
 * names_table
 * (pad to next 0x10 boundary if necessary)
 * node_count * matrix_data
 * IKData
 *
*/

#ifdef FBX_SUPPORT
void Utils::FbxMatrixToArray(float *mo, const FbxMatrix *mi)
{
    for (int i = 0; i < 4; i++)
    {
        FbxVector4 row = mi->GetRow(i);

        mo[i] = (float)row[0];
        mo[i+4] = (float)row[1];
        mo[i+8] = (float)row[2];
        mo[i+12] = (float)row[3];
    }
}

void Utils::FbxMatrixToArray(double *mo, const FbxMatrix *mi)
{
    for (int i = 0; i < 4; i++)
    {
        FbxVector4 row = mi->GetColumn(i);

        mo[i] = row[0];
        mo[i+4] = row[1];
        mo[i+8] = row[2];
        mo[i+12] = row[3];
    }
}

FbxMatrix Utils::ArrayToFbxMatrix(const float *mi)
{
    return FbxMatrix(mi[0], mi[1], mi[2], mi[3],
                     mi[4], mi[5], mi[6], mi[7],
                     mi[8], mi[9], mi[10], mi[11],
                     mi[12], mi[13], mi[14], mi[15]);
}

FbxAMatrix Utils::ArrayToFbxAMatrix(const float *mi)
{
    FbxMatrix m = ArrayToFbxMatrix(mi);
    FbxVector4 translation, rotation, scaling, shearing;
    double sign;

    m.GetElements(translation, rotation, shearing, scaling, sign);
    return FbxAMatrix(translation, rotation, scaling);
}

FbxAMatrix Utils::GetGlobalDefaultPosition(FbxNode* node)
{
    FbxAMatrix local_position;
    FbxAMatrix global_position;
    FbxAMatrix parent_global_position;

    local_position.SetT(node->LclTranslation.Get());
    local_position.SetR(node->LclRotation.Get());
    local_position.SetS(node->LclScaling.Get());

    if (node->GetParent())
    {
        parent_global_position = GetGlobalDefaultPosition(node->GetParent());
        global_position = parent_global_position * local_position;
    }
    else
    {
        global_position = local_position;
    }

    return global_position;
}

void Utils::SetGlobalDefaultPosition(FbxNode* node, FbxAMatrix global_position)
{
    FbxAMatrix local_position;
    FbxAMatrix parent_global_position;

    if (node->GetParent())
    {
        parent_global_position = GetGlobalDefaultPosition(node->GetParent());
        local_position = parent_global_position.Inverse() * global_position;
    }
    else
    {
        local_position = global_position;
    }

    node->LclTranslation.Set(local_position.GetT());
    node->LclRotation.Set(local_position.GetR());
    node->LclScaling.Set(local_position.GetS());
}

#endif

const static std::vector<std::string> matrix_suffix =
{
    "_X",
    "_Y",
    "_Z",
    "_O"
};

void Bone::DecompileTransformationMatrix(TiXmlElement *root, const char *name, const float *matrix)
{
    std::vector<float> row;

    row.resize(4);

#ifdef FBX_SUPPORT

    FbxMatrix fbx_matrix(matrix[0], matrix[1], matrix[2], matrix[3],
                         matrix[4], matrix[5], matrix[6], matrix[7],
                         matrix[8], matrix[9], matrix[10], matrix[11],
                         matrix[12], matrix[13], matrix[14], matrix[15]);


    FbxVector4 translation, scaling, shearing;
    FbxQuaternion rotation;
    double sign;

    fbx_matrix.GetElements(translation, rotation, shearing, scaling, sign);

    std::string comment = "Translation: " + Utils::FloatToString((float)translation[0]) + ", " +
                          Utils::FloatToString((float)translation[1]) + ", " + Utils::FloatToString((float)translation[2]);
    Utils::WriteComment(root, comment);

    comment = "Rotation: " + Utils::FloatToString((float)rotation[0]) + ", " + Utils::FloatToString((float)rotation[1]) + ", " +
                Utils::FloatToString((float)rotation[2]) + ", " + Utils::FloatToString((float)rotation[3]);
    Utils::WriteComment(root, comment);

    comment = "Scaling: " + Utils::FloatToString((float)scaling[0]) + ", " +
            Utils::FloatToString((float)scaling[1]) + ", " + Utils::FloatToString((float)scaling[2]);
    Utils::WriteComment(root, comment);

    comment = "Shearing: " + Utils::FloatToString((float)shearing[0]) + ", " +
            Utils::FloatToString((float)shearing[1]) + ", " + Utils::FloatToString((float)shearing[2]);
    Utils::WriteComment(root, comment);

    comment = "Sign: " + Utils::FloatToString((float)sign);
    Utils::WriteComment(root, comment);


#endif

    for (int i = 0; i < 4; i++)
    {
        std::string row_name = name + matrix_suffix[i];

        row[0] = matrix[i];
        row[1] = matrix[i+4];
        row[2] = matrix[i+8];
        row[3] = matrix[i+12];

        Utils::WriteParamMultipleFloats(root, row_name.c_str(), row);
    }
}

int Bone::CompileTransformationMatrix(const TiXmlElement *root, const char *name, float *matrix, bool must_exist)
{
    std::vector<float> row;

    for (int i = 0; i < 4; i++)
    {
        std::string row_name = name + matrix_suffix[i];

        if (i == 0 && !must_exist)
        {
            if (!Utils::ReadParamMultipleFloats(root, row_name.c_str(), row))
                return -1;
        }
        else
        {
            if (!Utils::GetParamMultipleFloats(root, row_name.c_str(), row))
                return false;
        }

        if (row.size() != 4)
        {
            DPRINTF("%s: Invalid size for \"%s\"\n", FUNCNAME, row_name.c_str());
            return false;
        }

        matrix[i] = row[0];
        matrix[i+4] = row[1];
        matrix[i+8] = row[2];
        matrix[i+12] = row[3];
    }

    return true;
}

void Bone::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Bone");
    entry_root->SetAttribute("name", name);

    if (parent)
    {
        Utils::WriteParamString(entry_root, "PARENT", parent->name);
    }
    else
    {
        Utils::WriteParamString(entry_root, "PARENT", "NULL");
    }

    if (child1)
    {
        Utils::WriteParamString(entry_root, "CHILD1", child1->name);
    }
    else
    {
        Utils::WriteParamString(entry_root, "CHILD1", "NULL");
    }

    if (child2)
    {
        Utils::WriteParamString(entry_root, "CHILD2", child2->name);
    }
    else
    {
        Utils::WriteParamString(entry_root, "CHILD2", "NULL");
    }

    if (child3)
    {
        Utils::WriteParamString(entry_root, "CHILD3", child3->name);
    }
    else
    {
        Utils::WriteParamString(entry_root, "CHILD3", "NULL");
    }

    if (child4)
    {
        Utils::WriteParamString(entry_root, "CHILD4", child4->name);
    }
    else
    {
        Utils::WriteParamString(entry_root, "CHILD4", "NULL");
    }

    DecompileTransformationMatrix(entry_root, "M1", matrix1);

    if (has_matrix2)
    {
       DecompileTransformationMatrix(entry_root, "M2", matrix2);
    }

    Utils::WriteParamMultipleUnsigned(entry_root, "SN_U_0A", std::vector<uint16_t>(sn_u0A, sn_u0A+3), true);

    if (has_unk)
    {
        Utils::WriteParamMultipleUnsigned(entry_root, "USD_U_00", std::vector<uint16_t>(usd_u00, usd_u00+4), true);
    }

    root->LinkEndChild(entry_root);
}

bool Bone::Compile(const TiXmlElement *root, SkeletonFile *skl)
{
    if (root->QueryStringAttribute("name", &name) != 0)
    {
        DPRINTF("%s: Cannot get name of Bone.\n", FUNCNAME);
        return false;
    }

    if (skl)
    {
        std::string bone_name;

        if (!Utils::GetParamString(root, "PARENT", bone_name))
            return false;

        if (bone_name == "NULL")
        {
            parent = nullptr;
        }
        else
        {
            parent = skl->GetBone(bone_name);
            if (!parent)
            {
                DPRINTF("%s: PARENT bone %s doesn't exist.\n", FUNCNAME, bone_name.c_str());
                return false;
            }
        }

        if (!Utils::GetParamString(root, "CHILD1", bone_name))
            return false;

        if (bone_name == "NULL")
        {
            child1 = nullptr;
        }
        else
        {
            child1 = skl->GetBone(bone_name);
            if (!child1)
            {
                DPRINTF("%s: CHILD1 bone %s doesn't exist.\n", FUNCNAME, bone_name.c_str());
                return false;
            }
        }

        if (!Utils::GetParamString(root, "CHILD2", bone_name))
            return false;

        if (bone_name == "NULL")
        {
            child2 = nullptr;
        }
        else
        {
            child2 = skl->GetBone(bone_name);
            if (!child2)
            {
                DPRINTF("%s: CHILD2 bone %s doesn't exist.\n", FUNCNAME, bone_name.c_str());
                return false;
            }
        }

        if (!Utils::GetParamString(root, "CHILD3", bone_name))
            return false;

        if (bone_name == "NULL")
        {
            child3 = nullptr;
        }
        else
        {
            child3 = skl->GetBone(bone_name);
            if (!child3)
            {
                DPRINTF("%s: CHILD3 bone %s doesn't exist.\n", FUNCNAME, bone_name.c_str());
                return false;
            }
        }

        if (!Utils::GetParamString(root, "CHILD4", bone_name))
            return false;

        if (bone_name == "NULL")
        {
            child4 = nullptr;
        }
        else
        {
            child4 = skl->GetBone(bone_name);
            if (!child4)
            {
                DPRINTF("%s: CHILD4 bone %s doesn't exist.\n", FUNCNAME, bone_name.c_str());
                return false;
            }
        }
    }

    if (!CompileTransformationMatrix(root, "M1", matrix1, true))
        return false;

    int troolean = CompileTransformationMatrix(root, "M2", matrix2, false);

    if (!troolean)
        return false;

    has_matrix2 = (troolean > 0);

    std::vector<uint16_t> sn_u0A;

    if (!Utils::GetParamMultipleUnsigned(root, "SN_U_0A", sn_u0A))
        return false;

    if (sn_u0A.size() != 3)
    {
        DPRINTF("%s: Invalid size for \"SN_U_0A\"\n", FUNCNAME);
        return false;
    }

    memcpy(this->sn_u0A, sn_u0A.data(), sizeof(this->sn_u0A));

    std::vector<uint16_t> usd_u00;

    if (Utils::ReadParamMultipleUnsigned(root, "USD_U_00", usd_u00))
    {
        has_unk = true;

        if (usd_u00.size() != 4)
        {
            DPRINTF("%s: Invalid size for \"USD_U_00\"\n", FUNCNAME);
            return false;
        }

        memcpy(this->usd_u00, usd_u00.data(), sizeof(this->usd_u00));
    }
    else
    {
        has_unk = false;
    }

    meta_original_offset = 0xFFFFFFFF;
    return true;
}

#ifdef FBX_SUPPORT

FbxAMatrix Bone::GetGlobalTransform() const
{
    FbxAMatrix bone_matrix = Utils::ArrayToFbxAMatrix(matrix1);

    if (parent)
    {
        bone_matrix = parent->GetGlobalTransform() * bone_matrix;
    }

    return bone_matrix;
}

#endif

bool Bone::PartialCompare(const Bone *b1, const Bone *b2)
{
    if (b1 == b2)
    {
        return true;
    }
    else if (!b1 || !b2)
    {
        return false;
    }

    if (b1->name != b2->name)
        return false;

    if (b1->has_unk != b2->has_unk)
        return false;

    if (b1->has_matrix2 != b2->has_matrix2)
        return false;

    if (memcmp(b1->matrix1, b2->matrix1, sizeof(b1->matrix1)) != 0)
        return false;

    if (b1->has_matrix2)
    {
        if (memcmp(b1->matrix2, b2->matrix2, sizeof(b1->matrix2)) != 0)
            return false;
    }

    if (memcmp(b1->sn_u0A, b2->sn_u0A, sizeof(b1->sn_u0A)) != 0)
        return false;

    if (b1->has_unk)
    {
        if (memcmp(b1->usd_u00, b2->usd_u00, sizeof(b1->usd_u00)) != 0)
            return false;
    }

    if (b1->parent != b2->parent)
    {
        if (!b1->parent || !b2->parent)
            return false;

        if (b1->parent->name != b2->parent->name)
            return false;
    }

    if (b1->child1 != b2->child1)
    {
        if (!b1->child1 || !b2->child1)
            return false;

        if (b1->child1->name != b2->child1->name)
            return false;
    }

    if (b1->child2 != b2->child2)
    {
        if (!b1->child2 || !b2->child2)
            return false;

        if (b1->child2->name != b2->child2->name)
            return false;
    }

    if (b1->child3 != b2->child3)
    {
        if (!b1->child3 || !b2->child3)
            return false;

        if (b1->child3->name != b2->child3->name)
            return false;
    }

    if (b1->child4 != b2->child4)
    {
        if (!b1->child4 || !b2->child4)
            return false;

        if (b1->child4->name != b2->child4->name)
            return false;
    }

    return true;
}

bool Bone::operator==(const Bone &rhs) const
{
    if (!PartialCompare(this, &rhs))
        return false;

    if (!PartialCompare(this->parent, rhs.parent))
        return false;

    if (!PartialCompare(this->child1, rhs.child1))
        return false;

    if (!PartialCompare(this->child2, rhs.child2))
        return false;

    if (!PartialCompare(this->child3, rhs.child3))
        return false;

    if (!PartialCompare(this->child4, rhs.child4))
        return false;

    return true;
}

SkeletonFile::SkeletonFile()
{
    ik_data = nullptr;
    big_endian = false;
    Reset();
}

SkeletonFile::SkeletonFile(uint8_t *buf, unsigned int size)
{
    ik_data = nullptr;
    big_endian = false;
    Load(buf, size);
}

SkeletonFile::~SkeletonFile()
{
    Reset();
}

void SkeletonFile::Copy(const SkeletonFile &other)
{
    if (other.ik_data)
    {
        this->ik_data = new uint8_t[other.ik_size];
		memcpy(this->ik_data, other.ik_data, other.ik_size);
		
        this->ik_size = other.ik_size;
    }
    else
    {
        this->ik_data = nullptr;
        this->ik_size = 0;
    }

    this->unk_02 = other.unk_02;
    this->unk_06 = other.unk_06;

    memcpy(this->unk_10, other.unk_10, sizeof(this->unk_10));
    memcpy(this->unk_34, other.unk_34, sizeof(this->unk_34));
    memcpy(this->unk_38, other.unk_38, sizeof(this->unk_38));

    this->bones = other.bones;

    for (Bone &b : bones)
    {
        if (b.parent)
        {
            b.parent = GetBone(b.parent->name);
			assert(b.parent != nullptr);
        }
		
		if (b.child1)
		{
			b.child1 = GetBone(b.child1->name);
			assert(b.child1 != nullptr);
		}
		
		if (b.child2)
		{
			b.child2 = GetBone(b.child2->name);
			assert(b.child2 != nullptr);
		}
		
		if (b.child3)
		{
			b.child3 = GetBone(b.child3->name);
			assert(b.child3 != nullptr);
		}
		
		if (b.child4)
		{
			b.child4 = GetBone(b.child4->name);
			assert(b.child4 != nullptr);
		}
    }

    this->big_endian = other.big_endian;
}

void SkeletonFile::Reset()
{
    if (ik_data)
    {
        delete[] ik_data;
        ik_data = nullptr;
    }

    ik_size = 0;
    bones.clear();
}

uint16_t SkeletonFile::FindBone(const std::vector<Bone *> &bones, Bone *bone, bool assert_if_not_found)
{
    assert(bone != nullptr);

    for (size_t i = 0; i < bones.size(); i++)
    {
        if (bones[i] == bone)
            return i;
    }

    if (assert_if_not_found)
    {
        assert(0);
		DPRINTF("%s: Bone not found.\n", FUNCNAME);
		exit(-1);
    }

    return 0xFFFF;
}

void SkeletonFile::RebuildSkeleton(const std::vector<Bone *> &old_bones_ptr)
{
    for (Bone &b : bones)
    {
        if (b.parent != nullptr)
        {
            b.parent = &bones[FindBone(old_bones_ptr, b.parent, true)];
        }

        if (b.child1 != nullptr)
        {
            b.child1 = &bones[FindBone(old_bones_ptr, b.child1, true)];
        }

        if (b.child2 != nullptr)
        {
            b.child2 = &bones[FindBone(old_bones_ptr, b.child2, true)];
        }

        if (b.child3 != nullptr)
        {
           b.child3 = &bones[FindBone(old_bones_ptr, b.child3, true)];
        }

        if (b.child4 != nullptr)
        {
            b.child4 = &bones[FindBone(old_bones_ptr, b.child4, true)];
        }
    }
}

uint16_t SkeletonFile::AppendBone(const SkeletonFile &other, const std::string &name)
{
    const Bone *bone = other.GetBone(name);
    if (!bone)
        return (uint16_t)-1;

    return AppendBone(*bone);
}

uint16_t SkeletonFile::AppendBone(const Bone &bone)
{
    std::vector<Bone *> old_bones_ptr;
    Bone new_bone = bone;

    new_bone.parent = nullptr;
    new_bone.child1 = nullptr;
    new_bone.child2 = nullptr;
    new_bone.child3 = nullptr;
    new_bone.child4 = nullptr;

    for (Bone &b : bones)
    {
        old_bones_ptr.push_back(&b);
    }

    bones.push_back(new_bone);
    RebuildSkeleton(old_bones_ptr);

    return (bones.size()-1);
}

bool SkeletonFile::CloneBoneParentChild(const SkeletonFile &other, const std::string &bone_name, Bone **not_found)
{
    Bone *bone_me;
    const Bone *bone_other;

    //DPRINTF("Clone: %s\n", bone_name.c_str());

    if (not_found)
        *not_found = nullptr;

    bone_me = GetBone(bone_name);
    if (!bone_me)
        return false;

    bone_other = other.GetBone(bone_name);
    if (!bone_other)
        return false;

    Bone bone_temp = Bone(*bone_me);

    if (!bone_other->parent)
    {
        bone_temp.parent = nullptr;
    }
    else
    {
        Bone *parent = GetBone(bone_other->parent->name);

        if (!parent)
        {
            *not_found = bone_other->parent;
            return false;
        }

        bone_temp.parent = parent;
    }

    if (!bone_other->child1)
    {
        bone_temp.child1 = nullptr;
    }
    else
    {
        Bone *child1 = GetBone(bone_other->child1->name);

        if (!child1)
        {
            *not_found = bone_other->child1;
            return false;
        }

        bone_temp.child1 = child1;
    }

    if (!bone_other->child2)
    {
        bone_temp.child2 = nullptr;
    }
    else
    {
        Bone *child2 = GetBone(bone_other->child2->name);

        if (!child2)
        {
            *not_found = bone_other->child2;
            return false;
        }

        bone_temp.child2 = child2;
    }

    if (!bone_other->child3)
    {
        bone_temp.child3 = nullptr;
    }
    else
    {
        Bone *child3 = GetBone(bone_other->child3->name);

        if (!child3)
        {
            *not_found = bone_other->child3;
            return false;
        }

        bone_temp.child3 = child3;
    }

    if (!bone_other->child4)
    {
        bone_temp.child4 = nullptr;
    }
    else
    {
        Bone *child4 = GetBone(bone_other->child4->name);

        if (!child4)
        {
            *not_found = bone_other->child4;
            return false;
        }

        bone_temp.child4 = child4;
    }

    //bones[BoneToIndex(bone_me)] = bone_temp;
    *bone_me = bone_temp;
    return true;
}

uint16_t SkeletonFile::BoneToIndex(Bone *bone) const
{
    if (!bone)
        return 0xFFFF;

    for (size_t i = 0; i < bones.size(); i++)
    {
        if (bone == &bones[i])
            return i;
    }

    DPRINTF("%s: We are about to crash or abort.\n", FUNCNAME);
    DPRINTF("Cannot find bone \"%s\" in this skeleton\n", bone->name.c_str());

    //DPRINTF("%s: coding error somewhere. This bone is not in this object (%s)\n", FUNCNAME, bone->name.c_str());
    throw std::runtime_error("Aborting");
    return 0xFFFF;
}

size_t SkeletonFile::CalculateIKSize(const uint8_t *ik_data, uint16_t count)
{
    size_t size = 0;

    for (uint16_t i = 0; i < count; i++)
    {
        uint16_t data_size = val16(*(uint16_t *)(ik_data + 2));

        size += data_size;
        ik_data += data_size;
    }

    return size;
}

uint16_t SkeletonFile::CalculateIKCount(const uint8_t *ik_data, size_t size)
{
    const uint8_t *end = ik_data + size;
    unsigned int count = 0;

    while (ik_data < end)
    {
        uint16_t data_size = *(uint16_t *)(ik_data + 2);

        count++;
        ik_data += data_size;
    }    

    return count;
}

void SkeletonFile::TranslateIKData(uint8_t *dst, const uint8_t *src, size_t size, bool import)
{
    if (this->big_endian == false)
    {
        memcpy(dst, src, size);
        return;
    }

    const IKEntry *ike_src = (const IKEntry *)src;
    IKEntry *ike_dst = (IKEntry *)dst;
    IKEntry *top = (IKEntry *)(dst + size);

    while (ike_dst < top)
    {
       ike_dst->unk_00 = val16(ike_src->unk_00);
       ike_dst->entry_size = val16(ike_src->entry_size);

       if (import)
       {
           assert (ike_dst->entry_size == 0x18);
       }

       ike_dst->unk_03 = ike_src->unk_03;
       ike_dst->unk_04 = ike_src->unk_04;
       ike_dst->unk_06 = val16(ike_src->unk_06);

       ike_dst->unk_08[0] = val16(ike_src->unk_08[0]);
       ike_dst->unk_08[1] = val16(ike_src->unk_08[1]);

       assert(ike_src->unk_08[2] == 0 && ike_src->unk_08[3] == 0);

       ike_dst->unk_08[2] = 0;
       ike_dst->unk_08[3] = 0;

       ike_dst->unk_10[0] = val32(ike_src->unk_10[0]);
       ike_dst->unk_10[1] = val32(ike_src->unk_10[1]);

       if (import)
       {
           assert(ike_dst->unk_10[0] == 0x3F000000 && ike_dst->unk_10[1] == 0);
       }

       ike_src++;
       ike_dst++;
    }

    //DPRINTF("Comparison after translation: %d\n", (memcmp(dst, src, size) == 0));
}

bool SkeletonFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    UNUSED(size);

    SkeletonHeader *hdr = (SkeletonHeader *)buf;
    uint16_t node_count = val16(hdr->node_count);

    if (hdr->ik_data_offset)
    {
        ik_size = CalculateIKSize(GetOffsetPtr(buf, hdr->ik_data_offset), val16(hdr->ik_count));
        if (ik_size != 0x108)
        {
            // Temporal message, it will be supressed in future
            // Update: supressed now
            //DPRINTF("Warning: ik_size not 0x108. Ignore this if this is not a SSSS CHARACTER file.\n");
        }

        ik_data = new uint8_t[ik_size];

        if (!ik_data)
        {
            DPRINTF("%s: Memory allocation error.\n", FUNCNAME);
            return false;
        }

        TranslateIKData(ik_data, GetOffsetPtr(buf, hdr->ik_data_offset), ik_size, true);
    }
    else
    {
        ik_data = nullptr;
        ik_size = 0;
    }

    unk_02 = val16(hdr->unk_02);
    unk_06 = val16(hdr->unk_06);

    unk_10[0] = val32(hdr->unk_10[0]);
    unk_10[1] = val32(hdr->unk_10[1]);

    if (hdr->unk_24[0] != 0 || hdr->unk_24[1] != 0 || hdr->unk_24[2] != 0 || hdr->unk_24[3] != 0)
    {
        DPRINTF("%s: unk_24 not zero as expected.\n", FUNCNAME);
        return false;
    }

    unk_34[0] = val16(hdr->unk_34[0]);
    unk_34[1] = val16(hdr->unk_34[1]);
    unk_38[0] = val_float(hdr->unk_38[0]);
    unk_38[1] = val_float(hdr->unk_38[1]);

    uint32_t *names_table = (uint32_t *)GetOffsetPtr(buf, hdr->names_offset);
    SkeletonNode *nodes = (SkeletonNode *)GetOffsetPtr(buf, hdr->start_offset);
    UnkSkeletonData *unks = nullptr;
    MatrixData *matrixes = nullptr;

    if (hdr->unk_skd_offset)
    {
        unks = (UnkSkeletonData *)GetOffsetPtr(buf, hdr->unk_skd_offset);
    }

    if (hdr->matrix_offset)
    {
        matrixes = (MatrixData *)GetOffsetPtr(buf, hdr->matrix_offset);
    }

    for (uint16_t i = 0; i < node_count; i++)
    {
        Bone bone;

        bone.meta_original_offset = Utils::DifPointer(&nodes[i], buf);
        bone.name = std::string((char *)GetOffsetPtr(buf, names_table, i));

        for (int j = 0; j < 16; j++)
        {
            bone.matrix1[j] = val_float(nodes[i].matrix[j]);
        }

        bone.sn_u0A[0] = val16(nodes[i].unk_0A[0]);
        bone.sn_u0A[1] = val16(nodes[i].unk_0A[1]);
        bone.sn_u0A[2] = val16(nodes[i].unk_0A[2]);

        if (unks)
        {
            bone.has_unk = true;
            bone.usd_u00[0] = val16(unks[i].unk_00[0]);
            bone.usd_u00[1] = val16(unks[i].unk_00[1]);
            bone.usd_u00[2] = val16(unks[i].unk_00[2]);
            bone.usd_u00[3] = val16(unks[i].unk_00[3]);
        }
        else
        {
            bone.has_unk = false;
        }

        if (matrixes)
        {
            bone.has_matrix2 = true;

            for (int j = 0; j < 16; j++)
            {
                bone.matrix2[j] = val_float(matrixes[i].matrix_00[j]);
            }
        }
        else
        {
            bone.has_matrix2 = false;
        }

        bones.push_back(bone);
    }

    assert(bones.size() == node_count);

    for (uint16_t i = 0; i < node_count; i++)
    {
        uint16_t parent, child1, child2, child3, child4;

        parent = val16(nodes[i].parent_id);
        child1 = val16(nodes[i].child1);
        child2 = val16(nodes[i].child2);
        child3 = val16(nodes[i].child3);
        child4 = val16(nodes[i].child4);

        if (parent == 0xFFFF)
        {
            bones[i].parent = nullptr;
        }
        else if (parent < node_count)
        {
            bones[i].parent = &bones[parent];
        }
        else
        {
            DPRINTF("%s: parent is out of range (on Bone %d %s)\n", FUNCNAME, i, bones[i].name.c_str());
        }

        if (child1 == 0xFFFF)
        {
            bones[i].child1 = nullptr;
        }
        else if (child1 < node_count)
        {
            bones[i].child1 = &bones[child1];
        }
        else
        {
            DPRINTF("%s: child1 is out of range (on Bone %d %s)\n", FUNCNAME, i, bones[i].name.c_str());
        }

        if (child2 == 0xFFFF)
        {
            bones[i].child2 = nullptr;
        }
        else if (child2 < node_count)
        {
            bones[i].child2 = &bones[child2];
        }
        else
        {
            DPRINTF("%s: child2 is out of range (on Bone %d %s)\n", FUNCNAME, i, bones[i].name.c_str());
        }

        if (child3 == 0xFFFF)
        {
            bones[i].child3 = nullptr;
        }
        else if (child3 < node_count)
        {
            bones[i].child3 = &bones[child3];
        }
        else
        {
            DPRINTF("%s: child3 is out of range (on Bone %d %s)\n", FUNCNAME, i, bones[i].name.c_str());
        }

        if (child4 == 0xFFFF)
        {
            bones[i].child4 = nullptr;
        }
        else if (child4 < node_count)
        {
            bones[i].child4 = &bones[child4];
        }
        else
        {
            DPRINTF("%s: child4 is out of range (on Bone %d %s)\n", FUNCNAME, i, bones[i].name.c_str());
        }
    }

    return true;
}

unsigned int SkeletonFile::CalculateFileSize() const
{
    if (bones.size() == 0)
        return 0;

    unsigned int file_size = sizeof(SkeletonHeader);
    file_size += (bones.size() * sizeof(SkeletonNode));

    if (bones[0].has_unk)
    {
        file_size += (bones.size() * sizeof(UnkSkeletonData));
    }

    file_size += (bones.size() * sizeof(uint32_t)); // Names table

    for (const Bone &b : bones)
    {
        file_size += b.name.length() + 1;
    }

    // Padding
    if (file_size & 0xF)
    {
        file_size += (0x10 - (file_size & 0xF));
    }

    if (bones[0].has_matrix2)
    {
        file_size += (bones.size() * sizeof(MatrixData));
    }

    if (ik_data)
    {
        file_size += ik_size;
    }

    return file_size;
}

uint8_t *SkeletonFile::Save(size_t *psize)
{
    if (bones.size() == 0)
        return nullptr;

    uint8_t *buf;
    unsigned int file_size;
    uint32_t offset;

    file_size = CalculateFileSize();
    buf = new uint8_t[file_size];
    if (!buf)
    {
        DPRINTF("%s: Memory allocation error (0x%x)\n", FUNCNAME, file_size);
        return nullptr;
    }

    memset(buf, 0, file_size);

    SkeletonHeader *hdr = (SkeletonHeader *)buf;

    hdr->node_count = val16(bones.size());
    hdr->unk_02 = val16(unk_02);
    hdr->unk_06 = val16(unk_06);
    hdr->unk_10[0] = val32(unk_10[0]);
    hdr->unk_10[1] = val32(unk_10[1]);
    hdr->unk_34[0] = val16(unk_34[0]);
    hdr->unk_34[1] = val16(unk_34[1]);
    copy_float(&hdr->unk_38[0], unk_38[0]);
    copy_float(&hdr->unk_38[1], unk_38[1]);

    offset = sizeof(SkeletonHeader);
    hdr->start_offset = val32(offset);

    SkeletonNode *nodes = (SkeletonNode *)GetOffsetPtr(buf, offset, true);

    for (size_t i = 0; i < bones.size(); i++)
    {
        nodes[i].parent_id = val16(BoneToIndex(bones[i].parent));
        nodes[i].child1 = val16(BoneToIndex(bones[i].child1));
        nodes[i].child2 = val16(BoneToIndex(bones[i].child2));
        nodes[i].child3 = val16(BoneToIndex(bones[i].child3));
        nodes[i].child4 = val16(BoneToIndex(bones[i].child4));

        nodes[i].unk_0A[0] = val16(bones[i].sn_u0A[0]);
        nodes[i].unk_0A[1] = val16(bones[i].sn_u0A[1]);
        nodes[i].unk_0A[2] = val16(bones[i].sn_u0A[2]);

        for (int j = 0; j < 16; j++)
        {
            copy_float(&nodes[i].matrix[j], bones[i].matrix1[j]);
        }

        offset += sizeof(SkeletonNode);
    }

    if (bones[0].has_unk)
    {
        hdr->unk_skd_offset = val32(offset);

        UnkSkeletonData *unks = (UnkSkeletonData *)GetOffsetPtr(buf, offset, true);

        for (size_t i = 0; i < bones.size(); i++)
        {
            unks[i].unk_00[0] = val16(bones[i].usd_u00[0]);
            unks[i].unk_00[1] = val16(bones[i].usd_u00[1]);
            unks[i].unk_00[2] = val16(bones[i].usd_u00[2]);
            unks[i].unk_00[3] = val16(bones[i].usd_u00[3]);

            offset += sizeof(UnkSkeletonData);
        }
    }
    else
    {
        hdr->unk_skd_offset = 0;
    }

    hdr->names_offset = val32(offset);

    uint32_t *names_table = (uint32_t *)GetOffsetPtr(buf, offset, true);
    offset += bones.size() * sizeof(uint32_t);

    for (size_t i = 0; i < bones.size(); i++)
    {
        names_table[i] = val32(offset);
        strcpy((char *)buf+offset, bones[i].name.c_str());
        offset += bones[i].name.length() + 1;
    }

    // Padding
    if (offset & 0xF)
    {
        offset += (0x10 - (offset & 0xF));
    }

    if (bones[0].has_matrix2)
    {
        hdr->matrix_offset = val32(offset);

        MatrixData *matrixes = (MatrixData *)GetOffsetPtr(buf, offset, true);

        for (size_t i = 0; i < bones.size(); i++)
        {
            for (int j = 0; j < 16; j++)
            {
                copy_float(&matrixes[i].matrix_00[j], bones[i].matrix2[j]);
            }

            offset += sizeof(MatrixData);
        }
    }
    else
    {
        hdr->matrix_offset = 0;
    }

    if (ik_data)
    {
       hdr->ik_data_offset = val32(offset);

       TranslateIKData(buf+offset, ik_data, ik_size, false);
       offset += ik_size;

       hdr->ik_count = val16(CalculateIKCount(ik_data, ik_size));
    }
    else
    {
        hdr->ik_data_offset = 0;
        hdr->ik_count = 0;
    }

    assert(offset == file_size);

    *psize = file_size;
    return buf;
}

void SkeletonFile::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Skeleton");

    Utils::WriteParamUnsigned(entry_root, "U_02", unk_02, true);
    Utils::WriteParamUnsigned(entry_root, "U_06", unk_06, true);
    Utils::WriteParamMultipleUnsigned(entry_root, "U_10", std::vector<uint32_t>(unk_10, unk_10+2), true);
    Utils::WriteParamMultipleUnsigned(entry_root, "U_34", std::vector<uint16_t>(unk_34, unk_34+2), true);
    Utils::WriteParamMultipleFloats(entry_root, "U_38", std::vector<float>(unk_38, unk_38+2));

    for (const Bone &b : bones)
    {
        b.Decompile(entry_root);
    }

    if (ik_data)
    {
        Utils::WriteParamBlob(entry_root, "IK_DATA", ik_data, ik_size);
    }

    root->LinkEndChild(entry_root);
}

TiXmlDocument *SkeletonFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();
    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    Decompile(doc);

    return doc;
}

bool SkeletonFile::Compile(const TiXmlElement *root)
{
    unsigned int unk_02, unk_06;
    std::vector<uint32_t> unk_10;
    std::vector<uint16_t> unk_34;
    std::vector<float> unk_38;

    if (!Utils::GetParamUnsigned(root, "U_02", &unk_02))
        return false;

    if (unk_02 > 0xFFFF)
    {
        DPRINTF("%s: \"U_02\" must be a 16 bits value.\n", FUNCNAME);
        return false;
    }

    this->unk_02 = unk_02;

    if (!Utils::GetParamUnsigned(root, "U_06", &unk_06))
        return false;

    if (unk_06 > 0xFFFF)
    {
        DPRINTF("%s: \"U_06\" must be a 16 bits value.\n", FUNCNAME);
        return false;
    }

    this->unk_06 = unk_06;

    if (!Utils::GetParamMultipleUnsigned(root, "U_10", unk_10))
        return false;

    if (!Utils::GetParamMultipleUnsigned(root, "U_34", unk_34))
        return false;

    if (!Utils::GetParamMultipleFloats(root, "U_38", unk_38))
        return false;

    if (unk_10.size() != 2)
    {
        DPRINTF("%s: Invalid size for \"U_10\"\n", FUNCNAME);
        return false;
    }

    if (unk_34.size() != 2)
    {
        DPRINTF("%s: Invalid size for \"U_34\"\n", FUNCNAME);
        return false;
    }

    if (unk_38.size() != 2)
    {
        DPRINTF("%s: Invalid size for \"U_38\"\n", FUNCNAME);
        return false;
    }

    memcpy(this->unk_10, unk_10.data(), sizeof(this->unk_10));
    memcpy(this->unk_34, unk_34.data(), sizeof(this->unk_34));
    memcpy(this->unk_38, unk_38.data(), sizeof(this->unk_38));

    // Skeleton, first pass
    for (const TiXmlElement *elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
    {
        const std::string &str = elem->ValueStr();

        if (str == "Bone")
        {
            Bone b;

            if (!b.Compile(elem, nullptr))
            {
                DPRINTF("%s: Bone compilation failed.\n", FUNCNAME);
                return false;
            }

            bones.push_back(b);
        }
    }

    // Skeleton, second pass
    size_t i = 0;
    for (const TiXmlElement *elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
    {
        const std::string &str = elem->ValueStr();

        if (str == "Bone")
        {
            if (!bones[i++].Compile(elem, this))
            {
                DPRINTF("%s: Bone compilation failed.\n", FUNCNAME);
                return false;
            }
        }
    }
	
	ik_data = Utils::ReadParamBlob(root, "IK_DATA", &ik_size);
	if (!ik_data)
		ik_size = 0;

    return true;
}

bool SkeletonFile::Compile(TiXmlDocument *doc, bool big_endian)
{
    Reset();
    this->big_endian = big_endian;

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "Skeleton");

    if (!root)
    {
        DPRINTF("Cannot find\"Skeleton\" in xml.\n");
        return false;
    }

    return Compile(root);
}

bool SkeletonFile::SaveSkeletonToFile(const std::string &path, bool show_error, bool build_path)
{
    size_t size;

    uint8_t *buf = SkeletonFile::Save(&size);
    if (!buf)
        return false;

    bool ret = Utils::WriteFileBool(path, buf, size, show_error, build_path);
    delete[] buf;

    return ret;
}

bool SkeletonFile::DecompileSkeletonToFile(const std::string &path, bool show_error, bool build_path)
{
    TiXmlDocument *doc = SkeletonFile::Decompile();

    if (!doc)
    {
        if (show_error)
        {
             DPRINTF("Decompilation of file \"%s\" failed.\n", path.c_str());
        }

        return false;
    }

    if (build_path)
    {
        if (!Utils::CreatePath(path))
        {
            if (show_error)
            {
                DPRINTF("Cannot create path for file \"%s\"\n", path.c_str());
            }

            return false;
        }
    }

    bool ret = doc->SaveFile(path);
    delete doc;

    if (!ret && show_error)
    {
        DPRINTF("Cannot create/write file \"%s\"\n", path.c_str());
    }

    return ret;
}

#ifdef FBX_SUPPORT

bool SkeletonFile::ExportFbxBone(const Bone *parent, FbxNode *root_node, FbxScene *scene, std::vector<FbxNode *> &fbx_bones) const
{
    for (size_t i = 0; i < bones.size(); i++)
    {
        const Bone &bone = bones[i];

        if (bone.parent != parent)
            continue;

        FbxSkeleton* skeleton_root_attribute = FbxSkeleton::Create(scene, bone.name.c_str());

        if (!parent)
        {
            skeleton_root_attribute->SetSkeletonType(FbxSkeleton::eRoot);
        }
        else
        {
            skeleton_root_attribute->SetSkeletonType(FbxSkeleton::eLimbNode);
        }

        FbxNode* fbx_bone_node = FbxNode::Create(scene, bone.name.c_str());
        fbx_bone_node->SetNodeAttribute(skeleton_root_attribute);
        root_node->AddChild(fbx_bone_node);

        FbxAMatrix matrix = bone.GetGlobalTransform();

        /*bool invert_yz = false;
        bool change_y = false;

        if (invert_yz)
        {
            FbxVector4 translation, rotation, scaling;
            double temp;

            translation = matrix.GetT();
            rotation = matrix.GetR();
            scaling = matrix.GetS();

            temp = translation[1];
            translation[1] = translation[2];
            translation[2] = temp;

            temp = rotation[1];
            rotation[1] = rotation[2];
            rotation[2] = temp;

            temp = scaling[1];
            scaling[1] = scaling[2];
            scaling[2] = temp;

            if (change_y)
            {
                translation[1] = -translation[1];
                rotation[1] = -rotation[1];
            }

            matrix.SetT(translation);
            matrix.SetR(rotation);
            matrix.SetS(scaling);
        }*/

        Utils::SetGlobalDefaultPosition(fbx_bone_node, matrix);

        fbx_bones[i] = fbx_bone_node;
        ExportFbxBone(&bone, fbx_bone_node, scene, fbx_bones);
    }

    return true;
}

bool SkeletonFile::ExportFbx(FbxScene *scene, std::vector<FbxNode *> &fbx_bones) const
{
    fbx_bones.resize(bones.size());
    return ExportFbxBone(nullptr, scene->GetRootNode(), scene, fbx_bones);
}

#endif

bool SkeletonFile::operator==(const SkeletonFile &rhs) const
{
    if (this->ik_size != rhs.ik_size)
        return false;

    if (this->ik_size != 0)
    {
        if (memcmp(this->ik_data, rhs.ik_data, this->ik_size) != 0)
            return false;
    }

    if (this->unk_02 != rhs.unk_02)
        return false;

    if (this->unk_06 != rhs.unk_06)
        return false;

    if (memcmp(this->unk_10, rhs.unk_10, sizeof(rhs.unk_10)) != 0)
        return false;

    if (memcmp(this->unk_34, rhs.unk_34, sizeof(rhs.unk_34)) != 0)
        return false;

    if (memcmp(this->unk_38, rhs.unk_38, sizeof(rhs.unk_38)) != 0)
        return false;

    if (this->bones != rhs.bones)
        return false;

    return true;
}
