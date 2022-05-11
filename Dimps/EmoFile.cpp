#include <algorithm>

#include "EmoFile.h"
#include "debug.h"

/*
 * Structure of emo files
 *
 * EMOHeader
 * PartsGroupHeader
 * [REPEAT for num groups]
 *  align to 0x10
 *  PartHeader
 *  [REPEAT for num parts]
 *      Align to 0x10 bytes
 *      EMGChunkHeader
 *      [REPEAT for num subparts]
 *          Align to 0x10 bytes
 *          EMGHeader
 *          Materials list offsets
 *          [REPEAT for num materials lists]
 *              num_materials
 *              num_materials * EMGMaterialHeader
 *          [END REPEAT for num materials lists]
 *          submesh list offsets
 *          Align to 0x10 bytes
 *          [REPEAT for num submeshes]
 *              EMGSubMeshHeader
 *              faces * num_faces
 *              bones idx * num_linked_bones
 *          [END REPEAT for num submeshes]
 *      [END REPEAT for num subparts]
 * align to 0x10 bytes
 * part names table
 * [END REPEAT for num parts]
 * [END REPEAT for num groups]
 * Align to 0x40 bytes
 * SkeltonFile
 * Vertex
*/

size_t PartsGroup::GetNumVertex()
{
    size_t count = 0;

    for (EmgFile &p : parts)
    {
        count += p.GetNumVertex();
    }

    return count;
}

size_t PartsGroup::GetLinkedBones(std::vector<Bone *> &list, bool clear_vector, bool unique, const SkeletonFile *sorter) const
{
    size_t count = 0;

    if (clear_vector)
        list.clear();

    for (const EmgFile &p : parts)
    {
        count += p.GetLinkedBones(list, false, unique);
    }

    if (sorter)
        std::sort(list.begin(), list.end(), BoneSorter(sorter));

    return count;
}

size_t PartsGroup::GetEmmMaterials(std::vector<std::string> &list, bool clear_vector, bool unique) const
{
    size_t count = 0;

    if (clear_vector)
        list.clear();

    for (const EmgFile &p : parts)
    {
        count += p.GetEmmMaterials(list, false, unique);
    }

    return count;
}

size_t PartsGroup::GetEmbIndexes(std::vector<uint8_t> &list, bool clear_vector, bool unique, bool sort) const
{
    size_t count = 0;

    if (clear_vector)
        list.clear();

    for (const EmgFile &p : parts)
    {
        count += p.GetEmbIndexes(list, false, unique);
    }

    if (sort)
        std::sort(list.begin(), list.end());

    return count;
}

size_t PartsGroup::ReplaceEmmMaterial(const std::string &old_mat, const std::string &new_mat)
{
    if (new_mat.length() > SubMesh::MAX_EMM_MAT_LENGTH)
        return (size_t)-1;

    size_t count = 0;

    for (EmgFile &p : parts)
    {
        count += p.ReplaceEmmMaterial(old_mat, new_mat);
    }

    return count;
}

size_t PartsGroup::ReplaceEmbIndex(uint8_t old_index, uint8_t new_index)
{
    size_t count = 0;

    for (EmgFile &p : parts)
    {
        count += p.ReplaceEmbIndex(old_index, new_index);
    }

    return count;
}

bool PartsGroup::IsEdge() const
{
    size_t count = 0;
    size_t non_empty = 0;

    for (const EmgFile &p : parts)
    {
        if (!p.IsEmpty())
        {
            non_empty++;

            if (p.IsEdge())
                count++;
        }
    }

    assert(count == 0 || count == non_empty);
    return (count != 0);
}

size_t PartsGroup::ExportObj(std::string *vertex_out, std::string *uvmap_out, std::string *normal_out, std::string *topology_out, size_t v_start_idx, bool write_group) const
{
    if (!vertex_out && !uvmap_out && !normal_out && !topology_out)
        return 0;

    size_t count = 0;
    std::string ret_v, ret_uv, ret_n, ret_t;

    for (const EmgFile &p : parts)
    {
        std::string v, uv, n, t;

        size_t ret = p.ExportObj(&v, &uv, &n, &t, v_start_idx, write_group);

        if (ret > 0)
        {
            count += ret;
            v_start_idx += ret;

            ret_v += v;
            ret_uv += uv;
            ret_n += n;
            ret_t += t;
        }
    }

    if (vertex_out)
    {
        *vertex_out = ret_v;
    }

    if (uvmap_out)
    {
        *uvmap_out = ret_uv;
    }

    if (normal_out)
    {
        *normal_out = ret_n;
    }

    if (topology_out)
    {
        *topology_out = ret_t;
    }

    return count;
}

void PartsGroup::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("PartsGroup");

    entry_root->SetAttribute("name", name);

    for (size_t i = 0; i < parts.size(); i++)
    {
        parts[i].Decompile(entry_root, i);
    }

    root->LinkEndChild(entry_root);
}

bool PartsGroup::Compile(const TiXmlElement *root, SkeletonFile *skl)
{
    parts.clear();

    if (root->QueryStringAttribute("name", &name) != TIXML_SUCCESS)
    {
        DPRINTF("%s: Cannot get name of PartsGroup\n", FUNCNAME);
        return false;
    }

    size_t count = Utils::GetElemCount(root, "EMG");
    if (count > 0)
    {
        std::vector<bool> initialized;

        parts.resize(count);
        initialized.resize(count);

        for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
        {
            if (elem->ValueStr() == "EMG")
            {
                uint32_t id;

                if (!Utils::ReadAttrUnsigned(elem, "id", &id))
                {
                    DPRINTF("%s: Cannot read attribute \"id\"\n", FUNCNAME);
                    return false;
                }

                if (id >= parts.size())
                {
                    DPRINTF("%s: EMG id 0x%x out of range.\n", FUNCNAME, id);
                    return false;
                }

                if (initialized[id])
                {
                    DPRINTF("%s: EMG id 0x%x was already specified.\n", FUNCNAME, id);
                    return false;
                }

                if (!parts[id].Compile(elem, skl))
                {
                    DPRINTF("%s: Compilation of EmgFile failed.\n", FUNCNAME);
                    return false;
                }

                char meta_name[2048];
                snprintf(meta_name, sizeof(meta_name), "%s_%04x", name.c_str(), id);

                parts[id].meta_name = meta_name;
                initialized[id] = true;
            }
        }
    }

    meta_original_offset = 0xFFFFFFFF;
    return true;
}

#ifdef FBX_SUPPORT

bool PartsGroup::ExportFbx(const SkeletonFile &skl, const std::vector<FbxNode *> &fbx_bones, FbxScene *scene) const
{
    for (const EmgFile &p : parts)
    {
        if (!p.IsEmpty() && !p.ExportFbx(skl, fbx_bones, scene))
            return false;
    }

    return true;
}

bool PartsGroup::InjectFbx(SkeletonFile &skl, FbxScene *scene, bool use_fbx_tangent)
{
    size_t first_empty = parts.size();

    for (size_t i = 0; i < parts.size(); i++)
    {
        if (parts[i].IsEmpty())
        {
            first_empty = i;
            break;
        }
    }

    for (EmgFile &p : parts)
    {
        if (!p.IsEmpty() && !p.InjectFbx(skl, scene, use_fbx_tangent))
            return false;
    }

    for (size_t i = 0; i < first_empty; i++)
    {
        if (parts[i].IsEmpty())
        {
            DPRINTF("Part \"%s\" has become empty. We will delete it.\n", parts[i].meta_name.c_str());
            parts.erase(parts.begin() + i);
            i--;
            first_empty--;
        }
    }

    return true;
}

#endif

EmoFile::EmoFile()
{
    Reset();
}

EmoFile::EmoFile(uint8_t *buf, size_t size)
{
    Load(buf, size);
}

EmoFile::~EmoFile()
{
    Reset();
}

void EmoFile::Copy(const EmoFile &other)
{
    SkeletonFile::Copy(other);

    this->groups = other.groups;
    this->material_count = other.material_count;
    memcpy(this->unk_08, other.unk_08, sizeof(this->unk_08));
    memcpy(this->unk_18, other.unk_18, sizeof(this->unk_18));

    for (PartsGroup &pg : groups)
    {
        for (EmgFile &p : pg.parts)
        {
            for (SubPart &sp : p.subparts)
            {
                for (SubMesh &sm : sp.submeshes)
                {
                    for (Bone *&b : sm.linked_bones)
                    {
                        b = GetBone(b->GetName());
                        assert(b != nullptr);
                    }
                }
            }
        }
    }

    this->big_endian = other.big_endian;
}

void EmoFile::Reset()
{
    SkeletonFile::Reset();
    groups.clear();
}

uint16_t EmoFile::GroupPtrToIndex(const PartsGroup *group) const
{
    for (size_t i = 0; i < groups.size(); i++)
    {
        if (group == &groups[i])
            return i;
    }

    return (uint16_t)-1;
}

bool EmoFile::PartPtrToIndex(const EmgFile *part, uint16_t *group_idx, uint16_t *part_idx) const
{
    for (size_t i = 0; i < groups.size(); i++)
    {
        for (size_t j = 0; j < groups[i].GetNumParts(); j++)
        {
            if (part == &groups[i][j])
            {
                *group_idx = i;
                *part_idx = j;
                return true;
            }
        }
    }

    return false;
}

bool EmoFile::SubPartPtrToIndex(const SubPart *subpart, uint16_t *group_idx, uint16_t *part_idx, uint16_t *subpart_idx) const
{
    for (size_t i = 0; i < groups.size(); i++)
    {
        for (size_t j = 0; j < groups[i].GetNumParts(); j++)
        {
            for (size_t k = 0; k < groups[i][j].GetNumSubParts(); k++)
            {
                if (subpart == &groups[i][j][k])
                {
                    *group_idx = i;
                    *part_idx = j;
                    *subpart_idx = k;
                    return true;
                }
            }
        }
    }

    return false;
}

size_t EmoFile::GetNumVertex()
{
    size_t count = 0;

    for (PartsGroup &pg : groups)
    {
        count += pg.GetNumVertex();
    }

    return count;
}

void EmoFile::RebuildSkeleton(const std::vector<Bone *> &old_bones_ptr)
{
    SkeletonFile::RebuildSkeleton(old_bones_ptr);
    //DPRINTF("Rebuild skeleton.\n");

    for (PartsGroup &pg : groups)
    {
        for (EmgFile &p : pg)
        {
            for (SubPart &sp : p)
            {
                for (SubMesh &sm : sp)
                {
                    for (Bone *&b : sm.linked_bones)
                    {
                        assert(b != nullptr);
                        uint16_t id = FindBone(old_bones_ptr, b, false);

                        if (id != 0xFFFF)
                        {
                            b = &bones[id];
                        }
                        else
                        {
                            // In this case, this is a bone from other emo that hasn't still been resolved
                            //DPRINTF("Not resolved yet: %s\n", b->GetName().c_str());
                        }
                    }
                }
            }
        }
    }
}

EmgFile *EmoFile::GetPart(const std::string &part_name)
{
    std::string group_name;
    unsigned int part_idx;

    size_t len = part_name.length();
    size_t pos = len-5;

    if (len <= 5 || part_name[pos] != '_')
        return nullptr;

    group_name = part_name.substr(0, pos);

    PartsGroup *group = GetGroup(group_name);
    if (!group)
        return nullptr;

    if (sscanf(part_name.substr(pos+1).c_str(), "%04x", &part_idx) != 1)
        return nullptr;   

    return group->GetPart(part_idx);
}

const EmgFile *EmoFile::GetPart(const std::string &part_name) const
{
    std::string group_name;
    unsigned int part_idx;

    size_t len = part_name.length();
    size_t pos = len-5;

    if (len <= 5 || part_name[pos] != '_')
        return nullptr;

    group_name = part_name.substr(0, pos);

    const PartsGroup *group = GetGroup(group_name);
    if (!group)
        return nullptr;

    if (sscanf(part_name.substr(pos+1).c_str(), "%04x", &part_idx) != 1)
        return nullptr;

    return group->GetPart(part_idx);
}

bool EmoFile::RemovePart(const std::string &part_name)
{
    uint16_t group_idx, part_idx;

    EmgFile *part = GetPart(part_name);
    if (!part)
        return false;

    bool ret = PartPtrToIndex(part, &group_idx, &part_idx);
    assert(ret);

    groups[group_idx].parts.erase(groups[group_idx].begin() + part_idx);
    return ret;
}

SubPart *EmoFile::GetSubPart(const std::string &subpart_name)
{
    std::string part_name;
    unsigned int subpart_idx;

    size_t len = subpart_name.length();
    size_t pos = len-5;

    if (len <= 5 || subpart_name[pos] != '_')
        return nullptr;

    part_name = subpart_name.substr(0, pos);

    EmgFile *part = GetPart(part_name);
    if (!part)
        return nullptr;

    if (sscanf(subpart_name.substr(pos+1).c_str(), "%04x", &subpart_idx) != 1)
        return nullptr;    

    return part->GetSubPart(subpart_idx);
}

const SubPart *EmoFile::GetSubPart(const std::string &subpart_name) const
{
    std::string part_name;
    unsigned int subpart_idx;

    size_t len = subpart_name.length();
    size_t pos = len-5;

    if (len <= 5 || subpart_name[pos] != '_')
        return nullptr;

    part_name = subpart_name.substr(0, pos);

    const EmgFile *part = GetPart(part_name);
    if (!part)
        return nullptr;

    if (sscanf(subpart_name.substr(pos+1).c_str(), "%04x", &subpart_idx) != 1)
        return nullptr;

    return part->GetSubPart(subpart_idx);
}

bool EmoFile::RemoveSubPart(const std::string &subpart_name)
{
    uint16_t group_idx, part_idx, subpart_idx;

    SubPart *subpart = GetSubPart(subpart_name);
    if (!subpart)
        return false;

    bool ret = SubPartPtrToIndex(subpart, &group_idx, &part_idx, &subpart_idx);
    assert(ret);

    groups[group_idx][part_idx].subparts.erase(groups[group_idx][part_idx].begin() + subpart_idx);
    return ret;
}

size_t EmoFile::GetEmmMaterials(std::vector<std::string> &list, bool clear_vector, bool unique) const
{
    size_t count = 0;

    if (clear_vector)
        list.clear();

    for (const PartsGroup &pg : groups)
    {
        count += pg.GetEmmMaterials(list, false, unique);
    }

    return count;
}

size_t EmoFile::GetEmbIndexes(std::vector<uint8_t> &list, bool clear_vector, bool unique, bool sort) const
{
    size_t count = 0;

    if (clear_vector)
        list.clear();

    for (const PartsGroup &pg : groups)
    {
        count += pg.GetEmbIndexes(list, false, unique);
    }

    if (sort)
        std::sort(list.begin(), list.end());

    return count;
}

size_t EmoFile::ReplaceEmmMaterial(const std::string &old_mat, const std::string &new_mat)
{
    if (new_mat.length() > SubMesh::MAX_EMM_MAT_LENGTH)
        return (size_t)-1;

    size_t count = 0;

    for (PartsGroup &pg : groups)
    {
        count += pg.ReplaceEmmMaterial(old_mat, new_mat);
    }

    return count;
}

size_t EmoFile::ReplaceEmbIndex(uint8_t old_index, uint8_t new_index)
{
    size_t count = 0;

    for (PartsGroup &pg : groups)
    {
        count += pg.ReplaceEmbIndex(old_index, new_index);
    }

    return count;
}

uint16_t EmoFile::AppendGroup(const EmoFile &other, const std::string &name)
{
    const PartsGroup *group = other.GetGroup(name);
    if (!group)
        return (uint16_t)-1;

    return AppendGroup(*group);
}

size_t EmoFile::CloneLinkedBones(const EmoFile &other, PartsGroup &group, Bone **not_found)
{
    size_t count = 0;

    for (EmgFile &p : group.parts)
    {
        for (SubPart &sp : p.subparts)
        {
            for (SubMesh &sm : sp.submeshes)
            {
                // First check that all bone exists
                for (Bone *b : sm.linked_bones)
                {
                    assert(b != nullptr);

                    if (!other.BoneExists(b->name))
                    {
                        // This is a critical error
                        if (not_found)
                            *not_found = b;

                        return (size_t)-1;
                    }

                    if (!BoneExists(b->name))
                    {
                        if (not_found)
                            *not_found = b;

                        return (size_t)-1;
                    }
                }

                // Now point the data to the local bones
                for (Bone *&b : sm.linked_bones)
                {
                    b = GetBone(b->name);
                    count++;
                }
            }
        }
    }

    return count;
}

bool EmoFile::HasEdges() const
{
    for (const PartsGroup &pg : groups)
        if (pg.IsEdge())
            return true;

    return false;
}

size_t EmoFile::ExportObj(std::string *vertex_out, std::string *uvmap_out, std::string *normal_out, std::string *topology_out, size_t v_start_idx, bool write_group) const
{
    if (!vertex_out && !uvmap_out && !normal_out && !topology_out)
        return 0;

    size_t count = 0;
    std::string ret_v, ret_uv, ret_n, ret_t;

    for (const PartsGroup &pg : groups)
    {
        std::string v, uv, n, t;

        size_t ret = pg.ExportObj(&v, &uv, &n, &t, v_start_idx, write_group);

        if (ret > 0)
        {
            count += ret;
            v_start_idx += ret;

            ret_v += v;
            ret_uv += uv;
            ret_n += n;
            ret_t += t;
        }
    }

    if (vertex_out)
    {
        *vertex_out = ret_v;
    }

    if (uvmap_out)
    {
        *uvmap_out = ret_uv;
    }

    if (normal_out)
    {
        *normal_out = ret_n;
    }

    if (topology_out)
    {
        *topology_out = ret_t;
    }

    return count;
}

size_t EmoFile::InjectObj(const std::string &obj, bool do_uv, bool do_normal, bool show_error)
{
    std::vector<VertexCommon> vertex;
    size_t vtx_count = 0, uv_count = 0, n_count = 0;

    size_t total_vertex = GetNumVertex();
    size_t count = 0;

    vertex.reserve(total_vertex);

    if (!Utils::ReadObj(obj, vertex, do_uv, do_normal, 0, 0, 0, &vtx_count, &uv_count, &n_count, show_error))
        return (size_t)-1;

    if (vertex.size() != total_vertex)
    {
        if (show_error)
        {
            DPRINTF("Number of vertex in obj is different to the one used in the .emo, cannot proceed "
                    "(obj=%Iu, emo=%Iu)\n", vertex.size(), total_vertex);
        }

        return (size_t)-1;
    }

    std::vector<PartsGroup> temp_groups = groups;
    size_t current_v = 0;

    for (PartsGroup &pg : temp_groups)
    {
        for (EmgFile &p : pg.parts)
        {
            for (SubPart &sp : p.subparts)
            {
                std::vector<VertexCommon> this_sp_vertex(&vertex[current_v], &vertex[current_v+sp.vertex.size()]);

                if (!sp.InjectVertex(this_sp_vertex, true, (uv_count != 0), (n_count != 0)))
                    return false;

                count++;
                current_v += sp.vertex.size();
            }
        }
    }

    groups = temp_groups;
    return count;
}

size_t EmoFile::GetNextObjSubPart(const std::string &content, size_t *pos, std::string &ret_obj, std::string &ret_name, size_t *num_v, size_t *num_uv, size_t *num_n, bool show_error)
{
    std::string line;
    bool name_set = false;
    int parse_state = 0; // 0 -> initial, 1 -> v group found, 2-> not in v group, 3 -> group found (next part)

    size_t part_begin = 0;
    size_t part_end = 0;

    ret_obj.clear();
    ret_name.clear();

    *num_v = *num_uv = *num_n = 0;

    size_t prev_pos = *pos;

    while (Utils::GetNextObjLine(content, pos, line))
    {
        if (line.length() < 3)
        {
            prev_pos = *pos;
            continue;
        }

        if (line[0] == 'v' && line[1] > 0 && line[1] <= ' ')
        {
            if (parse_state == 0)
            {
                parse_state = 1;
                *num_v = 1;
                part_begin = prev_pos;
            }
            else if (parse_state == 1)
            {
                *num_v = *num_v + 1;
            }
            else if (parse_state == 2)
            {
                parse_state = 3;

                if (part_end == 0)
                {
                    part_end = prev_pos;
                }

                if (!name_set)
                {
                    if (show_error)
                    {
                        DPRINTF("%s: Next group found but part name had not been set, "
                                "I can't understand this .obj, faulting line: \"%s\"\n", FUNCNAME, line.c_str());
                    }

                    *pos = std::string::npos;
                    return (size_t)-1;
                }

                ret_obj = content.substr(part_begin, part_end-part_begin);
                *pos = part_end;
                return *num_v;
            }
        }
        else
        {
            if (parse_state == 1)
            {
                parse_state = 2;
            }
        }

        if (parse_state == 2)
        {
            if (line[0] == 'v' && line[1] == 't' && line[2] > 0 && line[2] <= ' ')
            {
                *num_uv = *num_uv + 1;
            }
            else if (line[0] == 'v' && line[1] == 'n' && line[2] > 0 && line[2] <= ' ')
            {
                *num_n = *num_n + 1;
            }
        }

        if (line[0] == 'g' && line[1] > 0 && line[1] <= ' ')
        {
            if (name_set)
            {
                if (parse_state != 2 || part_end != 0)
                {
                    if (show_error)
                    {
                        DPRINTF("Part name had previously been set, I can't understand this .obj, faulting line: \"%s\"\n", line.c_str());
                    }

                    *pos = std::string::npos;
                    return (size_t)-1;
                }

                part_end = prev_pos;
            }
            else
            {
                size_t name_start = 2;

                while (name_start < line.length())
                {
                    if (line[name_start] > 0 && line[name_start] <= ' ')
                    {
                        name_start++;
                    }
                    else
                    {
                        break;
                    }
                }

                if (name_start == line.length())
                {
                    if (show_error)
                    {
                        DPRINTF("Cannot get name and cannot understand this .obj, faulting line: \"%s\"\n", line.c_str());
                    }

                    *pos = std::string::npos;
                    return (size_t)-1;
                }

                ret_name = line.substr(name_start);
                name_set = true;
            }
        }

        prev_pos = *pos;
    }

    *pos = std::string::npos; // Redundant

    if (name_set && parse_state == 2)
    {
        // Last part
        ret_obj = content.substr(part_begin);
        return *num_v;
    }

    return 0;
}

size_t EmoFile::InjectObjBySubParts(const std::string &obj, bool do_uv, bool do_normal, bool show_error)
{
    int ret;
    size_t pos = 0;
    size_t count = 0;

    std::string next_obj;
    std::string subpart_name;

    size_t num_v, num_uv, num_n;
    size_t v_start, uv_start, n_start;

    v_start = uv_start = n_start = 0;

    std::vector<PartsGroup> backup = groups;

    while ((ret = (int)GetNextObjSubPart(obj, &pos, next_obj, subpart_name, &num_v, &num_uv, &num_n)) > 0)
    {
        SubPart *sp = GetSubPart(subpart_name);
        if (!sp)
        {
            if (show_error)
            {
                DPRINTF(".obj specifies subpart \"%s\", but it doesn't exist on this .emo.\n", subpart_name.c_str());
            }

            if (count != 0)
                groups = backup;

            return (size_t)-1;
        }

        if (!sp->InjectObj(next_obj, do_uv, do_normal, v_start, uv_start, n_start, show_error))
        {
            if (count != 0)
                groups = backup;

            return (size_t)-1;
        }

        v_start += num_v;
        uv_start += num_uv;
        n_start += num_n;
        count++;
    }

    if (ret < 0)
    {
        if (show_error)
        {
            DPRINTF("%s: unknown error.\n", FUNCNAME);
        }

        if (count != 0)
            groups = backup;

        return (size_t)-1;
    }

    return count;
}

bool EmoFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    EMOHeader *hdr = (EMOHeader *)buf;
    if (size < sizeof(EMOHeader) || hdr->signature != EMO_SIGNATURE)
        return false;

    this->big_endian = (buf[4] != 0xFE);

    if (!SkeletonFile::Load(GetOffsetPtr(buf, hdr->skeleton_offset), size-val32(hdr->skeleton_offset)))
        return false;

    for (Bone &b : this->bones)
    {
        b.meta_original_offset += val32(hdr->skeleton_offset);
    }

    this->unk_08[0] = val16(hdr->unk_08[0]);
    this->unk_08[1] = val16(hdr->unk_08[1]);
    this->unk_18[0] = val32(hdr->unk_18[0]);
    this->unk_18[1] = val32(hdr->unk_18[1]);

    PartsGroupHeader *pghdr = (PartsGroupHeader *)GetOffsetPtr(buf, hdr->parts_offset);
    this->material_count = val16(pghdr->material_count);

    uint32_t *names_table = (uint32_t *)GetOffsetPtr(pghdr, pghdr->names_offset);

    for (uint16_t i = 0; i < val16(pghdr->groups_count); i++)
    {
        PartsGroup group;

        group.name = std::string((char *)GetOffsetPtr(pghdr, names_table, i));

        PartHeader *phdr = (PartHeader *)GetOffsetPtr(pghdr, pghdr->offsets, i);
        group.meta_original_offset = Utils::DifPointer(phdr, buf);

        if (phdr->unk_02 != 0)
        {
            DPRINTF("%s: unk_02 not 0 as expected, in group %s\n", FUNCNAME, group.name.c_str());
            return false;
        }

        for (uint16_t j = 0; j < val16(phdr->emg_count); j++)
        {
            EmgFile part(big_endian);
            char meta_name[2048];

            snprintf(meta_name, sizeof(meta_name), "%s_%04x", group.name.c_str(), j);
            part.meta_name = meta_name;

            if (phdr->offsets[j] == 0)
            {
                group.parts.push_back(part);
                continue;
            }

            uint8_t *emg_buf = GetOffsetPtr(phdr, phdr->offsets, j);

            if (!part.Load(emg_buf, Utils::DifPointer(buf + size, emg_buf), this))
            {
                DPRINTF("%s: Bad file, #EMG sub file couldn't be loaded (in group %s, part %04x)\n", FUNCNAME, group.name.c_str(), j);
                return false;
            }            

            group.parts.push_back(part);
        }

        this->groups.push_back(group);
    }

    return true;
}

unsigned int EmoFile::CalculateFileSize(uint32_t *vertex_start)
{
    unsigned int file_size;

    file_size = sizeof(EMOHeader);
    file_size += sizeof(PartsGroupHeader);

    if (groups.size() != 0)
        file_size += (groups.size() - 1) * sizeof(uint32_t);

    for (PartsGroup &pg : groups)
    {
        if (file_size & 0xF)
            file_size += (0x10 - (file_size & 0xF));

        //DPRINTF("file size before part %s: %x\n", pg.name.c_str(), file_size);

        file_size += sizeof(PartHeader);
        if (pg.parts.size() != 0)
        {
            file_size += (pg.parts.size() - 1) * sizeof(uint32_t);
        }

        for (EmgFile &p : pg.parts)
        {
            if (p.IsEmpty())
                continue;

            if (file_size & 0xF)
                file_size += (0x10 - (file_size & 0xF));

            file_size += p.CalculatePartSize();
        }
    }

    if (file_size & 0xF)
    {
        file_size += (0x10 - (file_size & 0xF));
    }

    //DPRINTF("file_size before names table %x\n", file_size);

    file_size += groups.size() * sizeof(uint32_t);
    for (PartsGroup &pg : groups)
    {
        file_size += (pg.name.length() + 1);
    }

    if (file_size & 0x3F)
        file_size += (0x40 - (file_size & 0x3F));


    //DPRINTF("file size before skeleton: %x\n", file_size);

    file_size += SkeletonFile::CalculateFileSize();

    *vertex_start = file_size;
    bool first = true;

    for (PartsGroup &pg : groups)
    {
        for (EmgFile &p : pg.parts)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                if (file_size & 0xf)
                    file_size += (0x10 - (file_size & 0xF));
            }

            file_size += p.CalculateVertexSize();
        }
    }

    //DPRINTF("File size = %x\n", file_size);
    return file_size;
}

uint8_t *EmoFile::Save(size_t *psize)
{
    unsigned int file_size;
    uint32_t offset, vertex_start;
    bool first = true;

    // We need to reorder parts first: but only when there are empty parts before non-empty parts!
    for (PartsGroup &pg: groups)
    {
        // Ignore these...
        if (pg.name == "face" || pg.name == "edge")
            continue;

        uint16_t first_empty = 0xFFFF;

        for (size_t i = 0; i < pg.parts.size(); i++)
        {
            if (first_empty == 0xFFFF && pg.parts[i].IsEmpty())
            {
                first_empty = i;
                break;
            }
        }

        if (first_empty != 0xFFFF)
        {
            bool do_sort = false;

            for (size_t i = 0; i < pg.parts.size(); i++)
            {
                if (!pg.parts[i].IsEmpty() && i > first_empty)
                {
                    do_sort = true;
                    break;
                }
            }

            //printf("do_sort = %d\n", do_sort);

            if (do_sort)
            {
                std::sort(pg.parts.begin(), pg.parts.end(), PartsSorter(pg));
            }
        }
    }

    file_size = CalculateFileSize(&vertex_start);

    uint8_t *buf = new uint8_t[file_size];
    memset(buf, 0, file_size);

    EMOHeader *hdr = (EMOHeader *)buf;

    hdr->signature = EMO_SIGNATURE;
    hdr->endianess_check = val16(0xFFFE);
    hdr->header_size = val16(sizeof(EMOHeader));
    hdr->unk_08[0] = val16(this->unk_08[0]);
    hdr->unk_08[1] = val16(this->unk_08[1]);
    hdr->unk_18[0] = val32(this->unk_18[0]);
    hdr->unk_18[1] = val32(this->unk_18[1]);

    offset = sizeof(EMOHeader);
    hdr->parts_offset = val32(offset);

    PartsGroupHeader *pghdr = (PartsGroupHeader *)GetOffsetPtr(buf, offset, true);

    assert(groups.size() < 65536);
    pghdr->groups_count = val16(groups.size());
    pghdr->material_count = val16(this->material_count);

    offset += sizeof(PartsGroupHeader);

    if (groups.size() != 0)
        offset += (groups.size() - 1) * sizeof(uint32_t);

    for (size_t i = 0; i < groups.size(); i++)
    {
        if (offset & 0xF)
            offset += (0x10 - (offset & 0xF));

        pghdr->offsets[i] = val32(offset - sizeof(EMOHeader));

        PartHeader *phdr = (PartHeader *)GetOffsetPtr(buf, offset, true);
        PartsGroup &pg = groups[i];

        assert(pg.parts.size() < 65536);

        phdr->emg_count = val16(pg.parts.size());

        offset += sizeof(PartHeader);

        if (pg.parts.size() != 0)
        {
            offset += (pg.parts.size() - 1) * sizeof(uint32_t);
        }

        for (size_t j = 0; j < pg.parts.size(); j++)
        {
            EmgFile &part = pg.parts[j];

            part.big_endian = big_endian;

            if (part.IsEmpty())
            {
                phdr->offsets[j] = 0;
                continue;
            }

            if (offset & 0xF)
                offset += (0x10 - (offset & 0xF));

            phdr->offsets[j] = val32(Utils::DifPointer(buf + offset, phdr));

            if (first)
            {
                first = false;
            }
            else
            {
                if (vertex_start & 0xf)
                    vertex_start += (0x10 - (vertex_start & 0xF));
            }

            uint32_t vertex_start_local1, vertex_start_local2;
            vertex_start_local1 = vertex_start_local2 = vertex_start - offset;

            offset += part.CreatePart(buf + offset, this, &vertex_start_local2);

            vertex_start += (vertex_start_local2 - vertex_start_local1);
        }
    }

    if (offset & 0xF)
    {
        offset += (0x10 - (offset & 0xF));
    }

    pghdr->names_offset = val32(offset - sizeof(EMOHeader));

    uint32_t *names_table = (uint32_t *)GetOffsetPtr(buf, offset, true);
    offset += groups.size() * sizeof(uint32_t);

    for (size_t i = 0; i < groups.size(); i++)
    {
        PartsGroup &pg = groups[i];

        names_table[i] = val32(offset - sizeof(EMOHeader));

        strcpy((char *)buf+offset, pg.name.c_str());
        offset += pg.name.length() + 1;
    }

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

    hdr->vertex_offset = val32(offset);
    first = true;

    for (PartsGroup &pg : groups)
    {
        for (EmgFile &p : pg.parts)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                if (offset & 0xf)
                    offset += (0x10 - (offset & 0xF));
            }

            offset += p.CreateVertex(buf + offset);
        }
    }

    assert(offset == file_size);

    *psize = file_size;
    return buf;
}

TiXmlDocument *EmoFile::Decompile() const
{	
	TiXmlDocument *doc = new TiXmlDocument();
	
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );	
	doc->LinkEndChild(decl);
	
	TiXmlElement *root = new TiXmlElement("EMO");
	
	Utils::WriteParamUnsigned(root, "MATERIAL_COUNT", material_count);
    Utils::WriteParamMultipleUnsigned(root, "U_08", std::vector<uint16_t>(unk_08, unk_08+2), true);
    Utils::WriteParamMultipleUnsigned(root, "U_18", std::vector<uint32_t>(unk_18, unk_18+2), true);
	
	for (const PartsGroup &pg : groups)
	{
		pg.Decompile(root);
	}
	
	doc->LinkEndChild(root);
	SkeletonFile::Decompile(doc);	
	
	return doc;
}

bool EmoFile::Compile(TiXmlDocument *doc, bool big_endian)
{	
    Reset();
    this->big_endian = big_endian;
	
	if (!SkeletonFile::Compile(doc, big_endian))
		return false;
	
	TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "EMO");

    if (!root)
    {
        DPRINTF("Cannot find\"EMO\" in xml.\n");
        return false;
    }
	
	unsigned int value;
	
	if (!Utils::GetParamUnsigned(root, "MATERIAL_COUNT", &value))
		return false;
	
	if (value > 0xFFFF)
	{
        DPRINTF("%s: material_count must be a 16 bits value.\n", FUNCNAME);
		return false;
	}
	
	material_count = value;

    std::vector<uint16_t> unk_08;
    std::vector<uint32_t> unk_18;

    if (!Utils::GetParamMultipleUnsigned(root, "U_08", unk_08))
        return false;

    if (unk_08.size() != 2)
    {
        DPRINTF("%s: Invalid size of U_08\n", FUNCNAME);
        return false;
    }

    if (!Utils::GetParamMultipleUnsigned(root, "U_18", unk_18))
        return false;

    if (unk_18.size() != 2)
    {
        DPRINTF("%s: Invalid size of U_18\n", FUNCNAME);
        return false;
    }
	
	for (int i = 0; i < 2; i++)
	{
        this->unk_08[i] = unk_08[i];
        this->unk_18[i] = unk_18[i];
	}
	
    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        const std::string &str = elem->ValueStr();

        if (str == "PartsGroup")
        {
            PartsGroup pg;

            if (!pg.Compile(elem, this))
            {
                DPRINTF("%s: PartsGroup compilation failed.\n", FUNCNAME);
                return false;
            }

            groups.push_back(pg);
        }
    }
	
    return true;
}

bool EmoFile::DecompileToFile(const std::string &path, bool show_error, bool build_path)
{
    UPRINTF("Emo is being saved to .xml file. This process may take several seconds.\n");
    bool ret = BaseFile::DecompileToFile(path, show_error, build_path);

    if (ret)
    {
        UPRINTF("Emo has been saved to .xml.\n");
    }

    return ret;
}

bool EmoFile::CompileFromFile(const std::string &path, bool show_error, bool big_endian)
{
    UPRINTF("Emo is being loaded from .xml file. This process may take several seconds.\n");
    bool ret = BaseFile::CompileFromFile(path, show_error, big_endian);

    if (ret)
    {
        UPRINTF("Emo has been loaded from .xml.\n");
    }

    return ret;
}

#ifdef FBX_SUPPORT

bool EmoFile::ExportFbx(FbxScene *scene, bool normal_parts, bool edges) const
{
    std::vector<FbxNode *> fbx_bones;

    if (!SkeletonFile::ExportFbx(scene, fbx_bones))
        return false;

    for (const PartsGroup &pg : groups)
    {
        if (!edges && pg.IsEdge())
            continue;

        if (!normal_parts && !pg.IsEdge())
            continue;

        if (!pg.ExportFbx(*this, fbx_bones, scene))
            return false;
    }

    return true;
}

bool EmoFile::InjectFbx(FbxScene *scene, bool normal_parts, bool edges, bool use_fbx_tangent)
{
    for (PartsGroup &pg : groups)
    {
        if (!edges && pg.IsEdge())
            continue;

        if (!normal_parts && !pg.IsEdge())
            continue;

        if (!pg.InjectFbx(*this, scene, use_fbx_tangent))
            return false;
    }

    for (size_t i = 0; i < groups.size(); i++)
    {
        if (groups[i].parts.size() == 0 || groups[i].parts[0].IsEmpty())
        {
            DPRINTF("Parts group \"%s\" has become empty. We will delete it.\n", groups[i].name.c_str());
            groups.erase(groups.begin()+i);
            i--;
        }
    }

    return true;
}

bool EmoFile::ExportFbx(const std::string &subpart, FbxScene *scene) const
{
    const SubPart *sp = GetSubPart(subpart);
    if (!sp)
        return false;

    std::vector<FbxNode *> fbx_bones;

    if (!SkeletonFile::ExportFbx(scene, fbx_bones))
        return false;

    return sp->ExportFbx(*this, fbx_bones, scene);
}

bool EmoFile::InjectFbx(const std::string &subpart, FbxScene *scene, bool use_fbx_tangent)
{
    SubPart *sp = GetSubPart(subpart);
    if (!sp)
        return false;

    return sp->InjectFbx(*this, scene, use_fbx_tangent);
}

#endif

bool EmoFile::operator==(const EmoFile &rhs) const
{
    if (this->material_count != rhs.material_count)
        return false;

    if (memcmp(this->unk_08, rhs.unk_08, sizeof(this->unk_08)) != 0)
        return false;

    if (memcmp(this->unk_18, rhs.unk_18, sizeof(this->unk_18)) != 0)
        return false;

    if (this->groups != rhs.groups)
        return false;

    if (SkeletonFile::operator ==(rhs) != true)
        return false;

    return true;
}


