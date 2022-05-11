#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "EmgFile.h"
#include "debug.h"

bool Utils::GetNextObjLine(const std::string &content, size_t *pos, std::string &line)
{
    if (*pos >= content.length())
        return false;

    size_t new_pos = content.find('\n', *pos);

    if (new_pos == std::string::npos)
    {
        line = content.substr(*pos);
    }
    else if (new_pos == *pos)
    {
        line = "";
        *pos = *pos + 1;
        return true;
    }
    else
    {
        line = content.substr(*pos, new_pos - *pos);
    }

    Utils::TrimString(line, true, true);
    *pos = new_pos;

    return true;
}

bool Utils::ReadObj(const std::string &obj, std::vector<VertexCommon> &vertex, bool do_uv, bool do_normal, size_t v_start_idx, size_t vt_start_idx, size_t vn_start_idx, size_t *vtx_count, size_t *uv_count, size_t *n_count, bool show_error)
{
    std::string line;
    size_t pos = 0;

    std::vector<VertexCommon> uv;
    std::vector<VertexCommon> normal;

    while (GetNextObjLine(obj, &pos, line))
    {
        if (line.length() < 3)
            continue;

        if (line[0] == 'v')
        {
            if (line[1] > 0 && line[1] <= ' ')
            {
                float x, y, z;

                if (sscanf(line.c_str()+2, "%f %f %f", &x, &y, &z) == 3)
                {
                    size_t size = vertex.size();

                    assert(*vtx_count <= size);

                    if (*vtx_count == size)
                    {
                        VertexCommon vc;

                        vc.pos.x = x;
                        vc.pos.y = y;
                        vc.pos.z = z;

                        vertex.push_back(vc);
                    }
                    else // <
                    {
                        vertex[*vtx_count].pos.x = x;
                        vertex[*vtx_count].pos.y = y;
                        vertex[*vtx_count].pos.z = z;
                    }

                    *vtx_count = *vtx_count + 1;
                }
            }
            else if (do_uv && line[1] == 't' && line[2] > 0 && line[2] <= ' ')
            {
                float u, v;

                if (sscanf(line.c_str()+3, "%f %f", &u, &v) == 2)
                {
                    size_t size = uv.size();

                    assert(*uv_count <= size);

                    if (*uv_count == size)
                    {
                        VertexCommon vc;

                        vc.tex.u = u;
                        vc.tex.v = -v;

                        uv.push_back(vc);
                    }
                    else // <
                    {
                        uv[*uv_count].tex.u = u;
                        uv[*uv_count].tex.v = -v;
                    }

                    *uv_count = *uv_count + 1;
                }
            }
            else if (do_normal && line[1] == 'n' && line[2] > 0 && line[2] <= ' ')
            {
                float x, y, z;

                if (sscanf(line.c_str()+3, "%f %f %f", &x, &y, &z) == 3)
                {
                    size_t size = normal.size();

                    assert(*n_count <= size);

                    if (*n_count == size)
                    {
                        VertexCommon vc;

                        vc.norm.x = x;
                        vc.norm.y = y;
                        vc.norm.z = z;

                        normal.push_back(vc);
                    }
                    else // <
                    {
                        normal[*n_count].norm.x = x;
                        normal[*n_count].norm.y = y;
                        normal[*n_count].norm.z = z;
                    }

                    *n_count = *n_count + 1;
                }
            }
        }
    }

    if (*vtx_count == 0)
    {
        if (show_error)
        {
            DPRINTF("No geometry vertex in input file!\n");
        }

        return false;
    }

    if (*uv_count > *vtx_count)
    {
        if (show_error)
        {
            DPRINTF("ERROR: Number of uv coordinates can't be greater than number of geometry vertex.\n");
        }

        return false;
    }

    if (*n_count > *vtx_count)
    {
        if (show_error)
        {
            DPRINTF("ERROR: Number of vertex normals can't be greater than number of geometry vertex.\n");
        }

        return false;
    }

    if (*uv_count > 0 || *n_count > 0)
    {
        pos = 0;

        while (GetNextObjLine(obj, &pos, line))
        {
            if (line.length() < 3)
                continue;

            if (line[0] == 'f' && line[1] > 0 && line[1] <= ' ')
            {
                uint32_t v_idx, uv_idx, n_idx;
                std::string current = line.substr(2);
                size_t count = 0;

                while (current.length() > 0)
                {
                    if (sscanf(current.c_str(), "%u/%u/%u", &v_idx, &uv_idx, &n_idx) == 3)
                    {
                        //printf("%u %u %u\n", v_idx, uv_idx, n_idx);

                        v_idx = v_idx - 1 - v_start_idx ;
                        uv_idx = uv_idx - 1 - vt_start_idx;
                        n_idx = n_idx - 1 - vn_start_idx;

                        if (v_idx >= vertex.size())
                        {
                            DPRINTF("ERROR: vertex index out of bounds (%u) at line \"%s\"\n", v_idx, line.c_str());
                            return false;
                        }

                        if (uv_idx >= *uv_count && *uv_count != 0)
                        {
                            DPRINTF("ERROR: texture coordinate index out of bounds (%u) at line \"%s\"\n", uv_idx, line.c_str());
                            return false;
                        }

                        if (n_idx >= *n_count && *n_count != 0)
                        {
                            DPRINTF("ERROR: vertex normal index out of bounds (%u) at line \"%s\"\n", n_idx, line.c_str());
                            return false;
                        }

                        if (*uv_count != 0)
                        {
                            vertex[v_idx].tex.u = uv[uv_idx].tex.u;
                            vertex[v_idx].tex.v = uv[uv_idx].tex.v;
                        }

                        if (*n_count != 0)
                        {
                            vertex[v_idx].norm.x = normal[n_idx].norm.x;
                            vertex[v_idx].norm.y = normal[n_idx].norm.y;
                            vertex[v_idx].norm.z = normal[n_idx].norm.z;
                        }

                        count++;
                    }
                    else
                    {
                        if (count == 0)
                        {
                            DPRINTF("WARNING: this line is being ignored: \"%s\"\n", line.c_str());
                            count = 3; // To skip warning
                            break;
                        }
                        else
                        {
                            DPRINTF("WARNING: this line is being partially ignored: \"%s\"\n", line.c_str());
                            count = 3; // To skip warning
                            break;
                        }
                    }

                    std::string next;
                    bool in_space = false;

                    for (size_t i = 0; i < current.length(); i++)
                    {
                        if (!in_space)
                        {
                            if (current[i] > 0 && current[i] <= ' ')
                            {
                                in_space = true;
                            }
                        }
                        else
                        {
                            if (current[i] > ' ')
                            {
                                next = current.substr(i);
                                break;
                            }
                        }
                    }

                    current = next;
                } // end while

                if (count < 3)
                {
                    DPRINTF("WARNING: it was expected 3 groups or more, at line \"%s\"\n", line.c_str());
                }
                else if (count > 3)
                {

                }
            }
        }
    }

    return true;
}

void Utils::ColorToFloat(uint32_t color, float *r, float *g, float *b, float *a)
{
    uint32_t u_r, u_g, u_b, u_a;

    u_a = (color & 0xFF000000) >> 24;
    u_r = (color & 0x00FF0000) >> 16;
    u_g = (color & 0x0000FF00) >> 8;
    u_b = color&0xFF;

    *a = ((float) u_a) / 255.0f;
    *r = ((float) u_r) / 255.0f;
    *g = ((float) u_g) / 255.0f;
    *b = ((float) u_b) / 255.0f;
}

uint32_t Utils::DoubleToColor(double r, double g, double b, double a)
{
    uint32_t u_r, u_g, u_b, u_a;

    u_r = (uint32_t) (r * 255.0);
    u_g = (uint32_t) (g * 255.0);
    u_b = (uint32_t) (b * 255.0);
    u_a = (uint32_t) (a * 255.0);

    uint32_t color = (u_a << 24);
    color |= (u_r << 16);
    color |= (u_g << 8);
    color |= u_b;

    return color;
}

void VertexData::Decompile(TiXmlNode *root) const
{
	TiXmlElement *entry_root;
	bool is64 = (size == sizeof(Vertex64));

	if (is64)
	{
		entry_root = new TiXmlElement("Vertex64");
	}
	else
	{
		entry_root = new TiXmlElement("Vertex52");
	}
	
	const VertexCommon *vc = &VertexUnion.vertex64.common;
	
	Utils::WriteParamMultipleFloats(entry_root, "POS", { vc->pos.x, vc->pos.y, vc->pos.z });
	Utils::WriteParamMultipleFloats(entry_root, "NORM", { vc->norm.x, vc->norm.y, vc->norm.z });
	Utils::WriteParamMultipleFloats(entry_root, "TEX", { vc->tex.u, vc->tex.v });
	
	if (is64)
	{
		Utils::WriteParamMultipleFloats(entry_root, "TEX2", std::vector<float>(VertexUnion.vertex64.tex2, VertexUnion.vertex64.tex2+3));
        Utils::WriteParamUnsigned(entry_root, "COLOR", VertexUnion.vertex64.color, true);
      	Utils::WriteParamMultipleUnsigned(entry_root, "BLEND", std::vector<uint8_t>(VertexUnion.vertex64.blend, VertexUnion.vertex64.blend+4), true);
       	Utils::WriteParamMultipleFloats(entry_root, "BLEND_WEIGHT", std::vector<float>(VertexUnion.vertex64.blend_weight, VertexUnion.vertex64.blend_weight+3));
	}
    else
    {
        Utils::WriteParamUnsigned(entry_root, "COLOR", VertexUnion.vertex52.color, true);
		Utils::WriteParamMultipleUnsigned(entry_root, "BLEND", std::vector<uint8_t>(VertexUnion.vertex52.blend, VertexUnion.vertex52.blend+4), true);
		Utils::WriteParamMultipleFloats(entry_root, "BLEND_WEIGHT", std::vector<float>(VertexUnion.vertex52.blend_weight, VertexUnion.vertex52.blend_weight+3));		
    }
	
    root->LinkEndChild(entry_root);
}

bool VertexData::Compile(const TiXmlElement *root, unsigned int vertex_size)
{
    assert(vertex_size == sizeof(Vertex52) || vertex_size == sizeof(Vertex64))    ;
	
	this->size = vertex_size;
	
	std::vector<float> f_vector;
	VertexCommon *vc = &VertexUnion.vertex64.common;

    if (!Utils::GetParamMultipleFloats(root, "POS", f_vector))
        return false;
	
	if (f_vector.size() != 3)
	{
        DPRINTF("%s: Invalid number of elements for POS!\n", FUNCNAME);
		return false;
	}
	
	vc->pos.x = f_vector[0];
	vc->pos.y = f_vector[1];
	vc->pos.z = f_vector[2];

	if (!Utils::GetParamMultipleFloats(root, "NORM", f_vector))
        return false;
	
	if (f_vector.size() != 3)
	{
        DPRINTF("%s: Invalid number of elements for NORM!\n", FUNCNAME);
		return false;
	}
	
	vc->norm.x = f_vector[0];
	vc->norm.y = f_vector[1];
	vc->norm.z = f_vector[2];

	if (!Utils::GetParamMultipleFloats(root, "TEX", f_vector))
		return false;

	if (f_vector.size() != 2)
	{
        DPRINTF("%s: Invalid number of elements for TEX!\n", FUNCNAME);
		return false;
	}

	vc->tex.u = f_vector[0];
	vc->tex.v = f_vector[1];
   
    if (vertex_size == sizeof(Vertex64))
    {
        std::vector<float> tex2;
		
		if (!Utils::GetParamMultipleFloats(root, "TEX2", tex2))
			return false;
		
		if (tex2.size() != 3)
		{
            DPRINTF("%s: Invalid number of elements for TEX2!\n", FUNCNAME);
			return false;
		}
		
		memcpy(VertexUnion.vertex64.tex2, tex2.data(), sizeof(VertexUnion.vertex64.tex2));
		
        if (!Utils::GetParamUnsigned(root, "COLOR", &VertexUnion.vertex64.color))
            return false;
    }
    else
    {
        if (!Utils::GetParamUnsigned(root, "COLOR", &VertexUnion.vertex52.color))
            return false;
    }

	std::vector<uint8_t> blend;
    std::vector<float> blend_weight;

    if (!Utils::GetParamMultipleUnsigned(root, "BLEND", blend))
		return false;
	
	if (blend.size() != 4)
	{
        DPRINTF("%s: Invalid number of elements for blend.\n", FUNCNAME);
		return false;
	}
	
	if (!Utils::GetParamMultipleFloats(root, "BLEND_WEIGHT", blend_weight))
        return false;

    if (blend_weight.size() != 3)
	{
        DPRINTF("%s: Invalid number of elements for blend_weight.\n", FUNCNAME);
		return false;
	}

    for (int i = 0; i < 4; i++)
    {
        if (vertex_size == sizeof(Vertex64))
        {
            VertexUnion.vertex64.blend[i] = blend[i];

            if (i < 3)
                VertexUnion.vertex64.blend_weight[i] = blend_weight[i];
        }
        else
        {
            VertexUnion.vertex52.blend[i] = blend[i];

            if (i < 3)
                VertexUnion.vertex52.blend_weight[i] = blend_weight[i];
        }
    }

    return true;
}

void Texture::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Texture");

    Utils::WriteParamUnsigned(entry_root, "EMB_INDEX", emb_index);
    Utils::WriteParamFloat(entry_root, "F1", f1);
    Utils::WriteParamFloat(entry_root, "F2", f2);
    Utils::WriteParamUnsigned(entry_root, "U_00", unk_00, true);
    Utils::WriteParamMultipleUnsigned(entry_root, "U_02", std::vector<uint8_t>(unk_02, unk_02+2), true);

    root->LinkEndChild(entry_root);
}

bool Texture::Compile(const TiXmlElement *root)
{
    unsigned int emb_index;
    unsigned int unk_00;
    std::vector<uint8_t> unk_02;

    if (!Utils::GetParamUnsigned(root, "EMB_INDEX", &emb_index))
        return false;

    if (!Utils::GetParamFloat(root, "F1", &this->f1))
        return false;

    if (!Utils::GetParamFloat(root, "F2", &this->f2))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_00", &unk_00))
        return false;

    if (!Utils::GetParamMultipleUnsigned(root, "U_02", unk_02))
        return false;

    if (emb_index > 0xFF)
    {
        DPRINTF("%s: emb_index must be a 8 bits value.\n", FUNCNAME);
        return false;
    }

    if (unk_00 > 0xFF)
    {
        DPRINTF("%s: U_00 must be a 8 bits value.\n", FUNCNAME);
        return false;
    }

    if (unk_02.size() != 2)
    {
        DPRINTF("%s: invalid number of elements for \"U_02\"\n", FUNCNAME);
        return false;
    }

    this->emb_index = emb_index;
    this->unk_00 = unk_00;
    this->unk_02[0] = unk_02[0];
    this->unk_02[1] = unk_02[1];

    return true;
}

bool SubPart::InjectVertex(const std::vector<VertexCommon> &new_vertex, bool do_pos, bool do_uvmap, bool do_normal)
{
    if (vertex.size() != new_vertex.size())
    {
        DPRINTF("ERROR: When injecting, number of vertex must be the same than existing ones! (tried: %Iu, existing: %Iu), on subpart \"%s\"\n", new_vertex.size(), vertex.size(), meta_name.c_str());
        return false;
    }

    assert(vertex.size() > 0);
    assert(vertex[0].size == sizeof(Vertex52) || vertex[0].size == sizeof(Vertex64));

    for (size_t i = 0; i < vertex.size(); i++)
    {
        VertexCommon *vtx = &vertex[i].VertexUnion.vertex64.common;
        const VertexCommon &vtx_inj = new_vertex[i];

        if (do_pos)
        {
            vtx->pos.x = vtx_inj.pos.x;
            vtx->pos.y = vtx_inj.pos.y;
            vtx->pos.z = vtx_inj.pos.z;
        }

        if (do_uvmap)
        {
            vtx->tex.u = vtx_inj.tex.u;
            vtx->tex.v = vtx_inj.tex.v;
        }

        if (do_normal)
        {
            vtx->norm.x = vtx_inj.norm.x;
            vtx->norm.y = vtx_inj.norm.y;
            vtx->norm.z = vtx_inj.norm.z;
        }
    }

    return true;
}

void SubPart::GetSubMeshVertexAndTriangles(uint16_t submesh_idx, std::vector<VertexData> &sm_vertex, std::vector<uint16_t> &sm_triangles) const
{
    std::vector<uint16_t> map;

    sm_triangles = GetTriangles(submesh_idx);
    sm_vertex.clear();

    for (uint16_t &id : sm_triangles)
    {
        std::vector<uint16_t>::iterator it = std::find(map.begin(), map.end(), id);
        if (it == map.end())
        {
            sm_vertex.push_back(vertex[id]);
            map.push_back(id);
            id = sm_vertex.size()-1;
        }
        else
        {
            id = it - map.begin();
        }
    }
}

size_t SubPart::GetTrianglesInternal(size_t submesh_idx, std::vector<uint16_t> *tri_faces) const
{
    if (submesh_idx >= submeshes.size())
    {
        throw std::out_of_range("submesh_idx out of range.");
    }

    const SubMesh &submesh = submeshes[submesh_idx];

    if (!strips)
    {
        assert((submesh.faces.size() % 3) == 0);

        if (tri_faces)
            *tri_faces = submesh.faces;

        return submesh.faces.size();
    }

    size_t count = 0;

    if (tri_faces)
        tri_faces->clear();

    bool backface = false;
    int strip_size = 1;
    uint16_t idx1 = 0xFFFF, idx2 = 0xFFFF;
    size_t f_index = 0;

    for (size_t i = 0; i < submesh.faces.size(); i++, strip_size++)
    {
        assert(f_index < submesh.faces.size());

        uint16_t idx = submesh.faces[f_index];
        f_index++;

        if(idx == idx1)
        {
            backface = !backface;
            idx1 = idx2;
            idx2 = idx;
            strip_size = 2;
            continue;
        }

        if(idx == idx2)
        {
            backface = !backface;
            strip_size = 1;
            continue;
        }

        if(strip_size >= 3)
        {
            if (tri_faces)
            {
                if(backface)
                {
                    tri_faces->push_back(idx);
                    tri_faces->push_back(idx2);
                    tri_faces->push_back(idx1);
                }
                else
                {
                    tri_faces->push_back(idx1);
                    tri_faces->push_back(idx2);
                    tri_faces->push_back(idx);
                }
            }

            count += 3;
        }

        backface = !backface;
        idx1 = idx2;
        idx2 = idx;
    }

    return count;
}

std::vector<uint16_t> SubPart::GetTriangles(size_t submesh_idx) const
{
    std::vector<uint16_t> tri_faces;

    GetTrianglesInternal(submesh_idx, &tri_faces);
    return tri_faces;
}

size_t SubPart::GetNumberOfPolygons(size_t submesh_idx) const
{
    size_t faces_num = GetTrianglesInternal(submesh_idx, nullptr);

    assert((faces_num % 3) == 0);
    return (faces_num / 3);
}

size_t SubPart::GetLinkedBones(std::vector<Bone *> &list, bool clear_vector, bool unique, const SkeletonFile *sorter) const
{
    size_t count = 0;

    if (clear_vector)
        list.clear();

    for (const SubMesh &sm : submeshes)
    {
        count += sm.GetLinkedBones(list, false, unique);
    }

    if (sorter)
    {
        std::sort(list.begin(), list.end(), BoneSorter(sorter));
    }

    return count;
}

size_t SubPart::GetEmmMaterials(std::vector<std::string> &list, bool clear_vector, bool unique) const
{
    size_t count = 0;

    if (clear_vector)
    {
        list.clear();
    }

    for (const SubMesh &sm : submeshes)
    {
        if (unique)
        {
            if (std::find_if(list.begin(), list.end(), Utils::StringFinder(sm.emm_material)) != list.end())
                continue;
        }

        list.push_back(sm.emm_material);
        count++;
    }

    return count;
}

size_t SubPart::GetEmbIndexes(std::vector<uint8_t> &list, bool clear_vector, bool unique, bool sort) const
{
    size_t count = 0;

    if (clear_vector)
        list.clear();

    for (const TexturesList &tl : textures_lists)
    {
        count += tl.GetEmbIndexes(list, false, unique);
    }

    if (sort)
    {
        std::sort(list.begin(), list.end());
    }

    return count;
}

size_t SubPart::ReplaceEmmMaterial(const std::string &old_mat, const std::string &new_mat)
{
    if (new_mat.length() > SubMesh::MAX_EMM_MAT_LENGTH)
        return (size_t)-1;

    size_t count = 0;

    for (SubMesh &sm : submeshes)
    {
        if (strcasecmp(sm.emm_material.c_str(), old_mat.c_str()) == 0)
        {
            sm.emm_material = new_mat;
            count++;
        }
    }

    return count;
}

size_t SubPart::ReplaceEmbIndex(uint8_t old_index, uint8_t new_index)
{
    size_t count = 0;

    for (TexturesList &tl : textures_lists)
    {
        count += tl.ReplaceEmbIndex(old_index, new_index);
    }

    return count;
}

bool SubPart::IsEdge() const
{
    size_t count = 0;

    for (const SubMesh &sm : submeshes)
    {
        if (sm.IsEdge())
            count++;
    }

    assert(count == 0 || count == submeshes.size());
    return (count != 0);
}

size_t SubPart::ExportObj(std::string *vertex_out, std::string *uvmap_out, std::string *normal_out, std::string *topology_out, size_t v_start_idx, bool write_group) const
{
    if (!vertex_out && !uvmap_out && !normal_out && !topology_out)
        return 0;

    std::ostringstream vss, uvss, nss, tss;

    vss.setf(std::ios::scientific, std::ios::floatfield);
    uvss.setf(std::ios::scientific, std::ios::floatfield);
    nss.setf(std::ios::scientific, std::ios::floatfield);

    if (vertex_out || uvmap_out || normal_out)
    {
        assert(vertex.size() != 0);
        assert(vertex[0].size == sizeof(Vertex52) || vertex[0].size == sizeof(Vertex64));

        for (const VertexData &vd : vertex)
        {
            const VertexCommon *vtx = &vd.VertexUnion.vertex64.common;

            if (vertex_out)
            {
                vss << "v " << Utils::FloatToString(vtx->pos.x) << " " << Utils::FloatToString(vtx->pos.y) << " " << Utils::FloatToString(vtx->pos.z) << " \n";
            }

            if (uvmap_out)
            {
                uvss << "vt " << Utils::FloatToString(vtx->tex.u) << " " << Utils::FloatToString(-vtx->tex.v) << " \n";
            }

            if (normal_out)
            {
                nss << "vn " << Utils::FloatToString(vtx->norm.x) << " " << Utils::FloatToString(vtx->norm.y) << " " << Utils::FloatToString(vtx->norm.z) << " \n";
            }
        }
    }

    if (topology_out)
    {
        for (const SubMesh &sm : submeshes)
        {
            if (write_group)
            {
                tss << "g " << sm.emm_material << " \n";
            }

            bool backface = false;
            int strip_size = 1;
            uint16_t idx1 = 0xFFFF, idx2 = 0xFFFF;
            unsigned int n_vert = 0;
            uint16_t idx[3];

            size_t f_index = 0;

            for (size_t i = 0; i < sm.faces.size(); i++, strip_size++)
            {
                if (strips != 0)
                {
                    assert(f_index < sm.faces.size());

                    uint16_t idx = 1 + sm.faces[f_index] + v_start_idx;
                    f_index++;

                    if(idx == idx1)
                    {
                        backface = !backface;
                        idx1 = idx2;
                        idx2 = idx;
                        strip_size = 2;
                        continue;
                    }

                    if(idx == idx2)
                    {
                        backface = !backface;
                        strip_size = 1;
                        continue;
                    }

                    if(strip_size >= 3)
                    {
                        if(backface)
                        {
                           tss << "f " << idx << "/" << idx << "/" << idx << " " << idx2 << "/" << idx2 << "/" << idx2 << " " << idx1 << "/" << idx1 << "/" << idx1 << " \n";
                        }
                        else
                        {
                            tss << "f "  << idx1 << "/" << idx1 << "/" << idx1 << " " << idx2 << "/" << idx2 << "/" << idx2 << " " << idx << "/" << idx << "/" << idx << " \n";
                        }
                    }

                    backface = !backface;
                    idx1 = idx2;
                    idx2 = idx;
                }
                else
                {
                    assert(f_index < sm.faces.size());

                    idx[n_vert] = 1 + sm.faces[f_index] + v_start_idx;
                    f_index++;

                    if(n_vert >= 2)
                    {
                        tss << "f "  << idx[0] << "/" << idx[0] << "/" << idx[0] << " " << idx[1] << "/" << idx[1] << "/" << idx[1] << " " << idx[2] << "/" << idx[2] << "/" << idx[2] << " \n";
                        n_vert = 0;
                    }
                    else
                    {
                        n_vert++;
                    }
                }
            }
        }
    }

    if (vertex_out)
    {
        *vertex_out = vss.str();
    }

    if (uvmap_out)
    {
        *uvmap_out = uvss.str();
    }

    if (normal_out)
    {
        *normal_out = nss.str();
    }

    if (topology_out)
    {
        *topology_out = tss.str();
    }

    return vertex.size();
}

bool SubPart::InjectObj(const std::string &obj, bool do_uv, bool do_normal, int v_start_idx, int vt_start_idx, int vn_start_idx, bool show_error)
{
    std::vector<VertexCommon> vertex;
    size_t vtx_count = 0, uv_count = 0, n_count = 0;

    if (!Utils::ReadObj(obj, vertex, do_uv, do_normal, v_start_idx, vt_start_idx, vn_start_idx, &vtx_count, &uv_count, &n_count, show_error))
        return false;

    return InjectVertex(vertex, true, (uv_count != 0), (n_count != 0));
}

void SubPart::Decompile(TiXmlNode *root, uint16_t id) const
{
    TiXmlElement *entry_root = new TiXmlElement("SubPart");
    entry_root->SetAttribute("id", Utils::UnsignedToString(id, true));

    if (meta_name != "")
    {
        Utils::WriteComment(entry_root, meta_name.c_str());
    }

    Utils::WriteParamUnsigned(entry_root, "STRIPS", strips);
    Utils::WriteParamMultipleFloats(entry_root, "VECTORS", std::vector<float>(vectors, vectors+12));

    Utils::WriteParamUnsigned(entry_root, "FLAGS", flags, true);
    Utils::WriteParamUnsigned(entry_root, "U_02", unk_02, true);
    Utils::WriteParamUnsigned(entry_root, "U_06", unk_06, true);
    Utils::WriteParamUnsigned(entry_root, "U_08", unk_08, true);

    for (size_t i = 0; i < textures_lists.size(); i++)
    {
        textures_lists[i].Decompile(entry_root, i);
    }

    for (size_t i = 0; i < submeshes.size(); i++)
    {
        submeshes[i].Decompile(entry_root, i, GetNumberOfPolygons(i));
    }

    for (const VertexData &v : vertex)
    {
        v.Decompile(entry_root);
    }

    root->LinkEndChild(entry_root);
}

bool SubPart::Compile(const TiXmlElement *root, SkeletonFile *skl)
{
    unsigned int strips;
    size_t count;
    std::vector<bool> initialized;

    submeshes.clear();
    textures_lists.clear();
    vertex.clear();

    if (!Utils::GetParamUnsigned(root, "STRIPS", &strips))
        return false;

    if (strips > 0xFFFF)
    {
        DPRINTF("%s: strips must be a 16 bits value.\n", FUNCNAME);
        return false;
    }

    this->strips = strips;

    std::vector<float> vectors;

    if (!Utils::GetParamMultipleFloats(root, "VECTORS", vectors))
        return false;

    if (vectors.size() != 12)
    {
        DPRINTF("%s: Invalid number of elements for \"vectors\"\n", FUNCNAME);
        return false;
    }

    for (int i = 0; i < 12; i++)
    {
        this->vectors[i] = vectors[i];
    }

    unsigned int flags, unk_02, unk_06;

    if (!Utils::GetParamUnsigned(root, "FLAGS", &flags))
        return false;

    if (flags > 0xFFFF)
    {
        DPRINTF("%s: FLAGS must be a 16 bits value.\n", FUNCNAME);
        return false;
    }

    this->flags = flags;

    if (!Utils::GetParamUnsigned(root, "U_02", &unk_02))
        return false;

    if (unk_02 > 0xFFFF)
    {
        DPRINTF("%s: U_02 must be a 16 bits value.\n", FUNCNAME);
        return false;
    }

    this->unk_02 = unk_02;

    if (!Utils::GetParamUnsigned(root, "U_06", &unk_06))
        return false;

    if (unk_06 > 0xFFFF)
    {
        DPRINTF("%s: U_06 must be a 16 bits value.\n", FUNCNAME);
        return false;
    }

    this->unk_06 = unk_06;

    if (!Utils::GetParamUnsigned(root, "U_08", &this->unk_08))
            return false;

    count = Utils::GetElemCount(root, "SubMesh");
    if (count > 0)
    {
        submeshes.resize(count);
        initialized.resize(count);

        for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
        {
            if (elem->ValueStr() == "SubMesh")
            {
                uint32_t id;

                if (!Utils::ReadAttrUnsigned(elem, "id", &id))
                {
                    DPRINTF("%s: Cannot read attribute \"id\"\n", FUNCNAME);
                    return false;
                }

                if (id >= submeshes.size())
                {
                    DPRINTF("%s: SubMesh id 0x%x out of range.\n", FUNCNAME, id);
                    return false;
                }

                if (initialized[id])
                {
                    DPRINTF("%s: SubMesh id 0x%x was already specified.\n", FUNCNAME, id);
                    return false;
                }

                if (!submeshes[id].Compile(elem, skl))
                {
                    DPRINTF("%s: Compilation of SubMesh failed.\n", FUNCNAME);
                    return false;
                }

                char sm_meta_name[2048];
                snprintf(sm_meta_name, sizeof(sm_meta_name), "%s_%04x", meta_name.c_str(), id);

                submeshes[id].meta_name = sm_meta_name;
                initialized[id] = true;
            }
        }
    }

    count = Utils::GetElemCount(root, "TexturesList");
    if (count > 0)
    {
        textures_lists.resize(count);
        initialized.clear();
        initialized.resize(count);

        for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
        {
            if (elem->ValueStr() == "TexturesList")
            {
                uint32_t id;

                if (!Utils::ReadAttrUnsigned(elem, "id", &id))
                {
                    DPRINTF("%s: Cannot read attribute \"id\"\n", FUNCNAME);
                    return false;
                }

                if (id >= textures_lists.size())
                {
                    DPRINTF("%s: TexturesList id 0x%x out of range.\n", FUNCNAME, id);
                    return false;
                }

                if (initialized[id])
                {
                    DPRINTF("%s: TexturesList id 0x%x was already specified.\n", FUNCNAME, id);
                    return false;
                }

                if (!textures_lists[id].Compile(elem))
                {
                    DPRINTF("%s: Compilation of TexturesList failed.\n", FUNCNAME);
                    return false;
                }

                initialized[id] = true;
            }
        }
    }

    int vertex_size = -1;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        const std::string &str = elem->ValueStr();

        if (str == "Vertex64" || str == "Vertex52")
        {
            if (vertex_size == -1)
            {
                vertex_size = (str == "Vertex64") ? sizeof(Vertex64) : sizeof(Vertex52);
            }
            else if ((vertex_size == sizeof(Vertex64) && str == "Vertex52") ||
                     (vertex_size == sizeof(Vertex52) && str == "Vertex64"))
            {
                DPRINTF("%s: All vertex of the subpart must be of the same kind!\n", FUNCNAME);
                return false;
            }

            VertexData vd;

            if (!vd.Compile(elem, vertex_size))
            {
                DPRINTF("%s: Compilation of Vertex failed.\n", FUNCNAME);
                return false;
            }

            this->vertex.push_back(vd);
        }
    }

    return true;
}

#ifdef FBX_SUPPORT

void SubPart::ExportSubMeshFbxSkin(const SkeletonFile &skl, const SubMesh &submesh, const std::vector<VertexData> &sm_vertex, const std::vector<FbxNode *> &fbx_bones, FbxScene *scene, FbxMesh *fbx_mesh, FbxAMatrix skin_matrix) const
{
    FbxSkin* skin = FbxSkin::Create(scene, "");
    std::vector<FbxCluster *> skin_clusters;

    UNUSED(skl);


    /*skin_clusters.resize(fbx_bones.size());

    for (size_t i = 0; i < fbx_bones.size(); i++)
    {
       FbxCluster *cluster = nullptr; */

       /*const int cluster_count = scene->GetSrcObjectCount<FbxCluster>();
       for (int j = 0; j < cluster_count; j++)
       {
           FbxCluster *this_cluster = scene->GetSrcObject<FbxCluster>(j);
           FbxString name = this_cluster->GetName();

           if (name == fbx_bones[i]->GetName())
           {
               cluster = this_cluster;
               break;
           }
       }*/ /*

       if (!cluster)
       {
           cluster = FbxCluster::Create(scene, fbx_bones[i]->GetName());

           cluster->SetLink(fbx_bones[i]);
           cluster->SetLinkMode(FbxCluster::eTotalOne);
           //cluster->SetTransformMatrix(skin_matrix);

           FbxAMatrix XMatrix = fbx_bones[i]->EvaluateGlobalTransform();
           cluster->SetTransformLinkMatrix(XMatrix);
       }

       skin->AddCluster(cluster);
       skin_clusters[i] = cluster;
    }*/

    skin_clusters.resize(submesh.linked_bones.size());

    for (size_t i = 0; i < skin_clusters.size(); i++)
    {
        Bone *const &bone = submesh.linked_bones[i];
        FbxNode *fbx_bone = nullptr;

        for (size_t j = 0; j < fbx_bones.size(); j++)
        {
            if (bone->GetName() == fbx_bones[j]->GetName())
            {
                fbx_bone = fbx_bones[j];
                break;
            }
        }

        assert(fbx_bone);

        FbxCluster *cluster = FbxCluster::Create(scene, fbx_bone->GetName());

        cluster->SetLink(fbx_bone);
        cluster->SetLinkMode(FbxCluster::eTotalOne);
        //cluster->SetTransformMatrix(skin_matrix);

        FbxAMatrix XMatrix = fbx_bone->EvaluateGlobalTransform();
        cluster->SetTransformLinkMatrix(XMatrix);

        skin->AddCluster(cluster);
        skin_clusters[i] = cluster;
    }

    for (size_t i = 0; i < sm_vertex.size(); i++)
    {
        const VertexData &v = sm_vertex[i];

        for (size_t j = 0; j < 4; j++)
        {
            const float *blend_weight;
            const uint8_t *blend;

            float this_weight;

            if (v.size == sizeof(Vertex64))
            {
                blend_weight = v.VertexUnion.vertex64.blend_weight;
                blend = v.VertexUnion.vertex64.blend;
            }
            else if (v.size == sizeof(Vertex52))
            {
                blend_weight = v.VertexUnion.vertex52.blend_weight;
                blend = v.VertexUnion.vertex52.blend;
            }
            else
            {
                assert(0);
            }

            if (j == 3)
            {
               this_weight = 1.0f - blend_weight[0] - blend_weight[1] - blend_weight[2];
            }
            else
            {
                this_weight = blend_weight[j];
            }

            if (this_weight > 0.0f)
            {
                uint8_t linked_bone_index = blend[j];
                //uint16_t bone_index;

                assert(linked_bone_index < submesh.linked_bones.size());
                Bone *bone = submesh.linked_bones[linked_bone_index];

                /*bone_index = skl.BoneToIndex(bone);

                assert(bone->GetName() == skin_clusters[bone_index]->GetName().Buffer());
                skin_clusters[bone_index]->AddControlPointIndex(i, this_weight);*/
                assert(bone->GetName() == skin_clusters[linked_bone_index]->GetName().Buffer());
                skin_clusters[linked_bone_index]->AddControlPointIndex(i, this_weight);

                UNUSED(bone); 
            }
        }
    }

    fbx_mesh->AddDeformer(skin);
}

bool SubPart::ExportSubMeshFbx(const SkeletonFile &skl, uint16_t submesh_idx, const std::vector<FbxNode *> &fbx_bones, FbxScene *scene) const
{
    assert(vertex.size() > 0);
    assert(vertex[0].size == sizeof(Vertex52) || vertex[0].size == sizeof(Vertex64));

    if (submesh_idx >= submeshes.size())
        return false;

    const SubMesh &submesh = submeshes[submesh_idx];

    std::vector<VertexData> sm_vertex;
    std::vector<uint16_t> sm_triangles;

    GetSubMeshVertexAndTriangles(submesh_idx, sm_vertex, sm_triangles);

    //DPRINTF("%s: Vertex = %x, triangles = %x\n", submeshes[submesh_idx].meta_name.c_str(), sm_vertex.size(), sm_triangles.size());

    FbxMesh *fbx_mesh = FbxMesh::Create(scene, submesh.meta_name.c_str());
    if (!fbx_mesh)
        return false;

    unsigned int control_point_base = 0;
    unsigned int total_control_points = sm_vertex.size();
    fbx_mesh->InitControlPoints(total_control_points);

    FbxGeometryElementNormal *fbx_normal = fbx_mesh->CreateElementNormal();
    fbx_normal->SetMappingMode(FbxGeometryElement::eByControlPoint);
    //fbx_normal->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
    fbx_normal->SetReferenceMode(FbxGeometryElement::eDirect);

    FbxGeometryElementUV *fbx_uv = fbx_mesh->CreateElementUV("DiffuseUV");
    fbx_uv->SetMappingMode(FbxGeometryElement::eByControlPoint);
    //fbx_uv->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
    fbx_uv->SetReferenceMode(FbxGeometryElement::eDirect);

    FbxGeometryElementVertexColor *fbx_vcolor = fbx_mesh->CreateElementVertexColor();
    fbx_vcolor->SetMappingMode(FbxGeometryElement::eByControlPoint);
    //fbx_uv->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
    fbx_vcolor->SetReferenceMode(FbxGeometryElement::eDirect);

    FbxGeometryElementTangent *fbx_tangent = nullptr;

    if (vertex[0].size == sizeof(Vertex64))
    {
        fbx_tangent = fbx_mesh->CreateElementTangent();
        fbx_tangent->SetMappingMode(FbxGeometryElement::eByControlPoint);
        //fbx_uv->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
        fbx_tangent->SetReferenceMode(FbxGeometryElement::eDirect);
    }

    /*FbxGeometryElementMaterial* fbx_material = fbx_mesh->CreateElementMaterial();
    fbx_material->SetMappingMode(FbxGeometryElement::eByPolygon);
    fbx_material->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

    std::vector<FbxSurfaceMaterial *> materials;
    materials.resize(1);
    materials[0] = exportFBXMaterial(scene, submeshes[submesh_idx].meta_name + "Material");
    mesh_node->AddMaterial(materials[0]);*/    

    FbxNode *mesh_node = FbxNode::Create(scene, submesh.meta_name.c_str());
    mesh_node->LclTranslation.Set(FbxVector4(0, 0, 0));
    mesh_node->LclScaling.Set(FbxVector4(1, 1, 1));
    mesh_node->LclRotation.Set(FbxVector4(0, 0, 0));

    FbxVector4 *control_points = fbx_mesh->GetControlPoints();

    for (size_t i = 0; i < sm_vertex.size(); i++)
    {
        const VertexData &v = sm_vertex[i];
        float r, g, b, a;

        control_points[control_point_base+i] = FbxVector4(v.VertexUnion.vertex64.common.pos.x,
                                                          v.VertexUnion.vertex64.common.pos.y,
                                                          v.VertexUnion.vertex64.common.pos.z);

        fbx_normal->GetDirectArray().Add(FbxVector4(v.VertexUnion.vertex64.common.norm.x,
                                                    v.VertexUnion.vertex64.common.norm.y,
                                                    v.VertexUnion.vertex64.common.norm.z));

        fbx_uv->GetDirectArray().Add(FbxVector2(v.VertexUnion.vertex64.common.tex.u,
                                                -v.VertexUnion.vertex64.common.tex.v));

        if (v.size == sizeof(Vertex64))
        {
            Utils::ColorToFloat(v.VertexUnion.vertex64.color, &r, &g, &b, &a);
        }
        else
        {
            Utils::ColorToFloat(v.VertexUnion.vertex52.color, &r, &g, &b, &a);
        }

        fbx_vcolor->GetDirectArray().Add(FbxColor(r, g, b, a));

        // No need to add tangent for export
        if (fbx_tangent)
        {
            fbx_tangent->GetDirectArray().Add(FbxVector4(v.VertexUnion.vertex64.tex2[0],
                                                         v.VertexUnion.vertex64.tex2[1],
                                                         v.VertexUnion.vertex64.tex2[2],
                                                         0.0));
        }
    }

    assert((sm_triangles.size() % 3) == 0);

    for (size_t i = 0; i < sm_triangles.size(); i += 3)
    {
        //fbx_mesh->BeginPolygon(fbx_material_index);
        fbx_mesh->BeginPolygon();
        fbx_mesh->AddPolygon(control_point_base + sm_triangles[i]);
        fbx_mesh->AddPolygon(control_point_base + sm_triangles[i+1]);
        fbx_mesh->AddPolygon(control_point_base + sm_triangles[i+2]);
        fbx_mesh->EndPolygon();
    }

    mesh_node->SetNodeAttribute(fbx_mesh);
    ExportSubMeshFbxSkin(skl, submesh, sm_vertex, fbx_bones, scene, fbx_mesh, mesh_node->EvaluateGlobalTransform());

    scene->GetRootNode()->AddChild(mesh_node);
    return true;
}

bool SubPart::ExportFbx(const SkeletonFile &skl, const std::vector<FbxNode *> &fbx_bones, FbxScene *scene) const
{
    for (size_t i = 0; i < submeshes.size(); i++)
    {
        if (!ExportSubMeshFbx(skl, i, fbx_bones, scene))
            return false;
    }

    return true;
}

bool SubPart::LoadFbxBlendWeights(FbxMesh *fbx_mesh, std::vector<std::vector<std::pair<double, FbxNode *>>> &weights)
{
    const int vertex_count = fbx_mesh->GetControlPointsCount();

    if (fbx_mesh->GetDeformerCount() == 0)
    {
        DPRINTF("ERROR: deformer count is zero! (in submesh %s)\n", fbx_mesh->GetName());
        return false;
    }

    if (fbx_mesh->GetDeformerCount() != 1)
    {
        DPRINTF("WARNING: deformer count is different than 1 (in submesh %s)\n", fbx_mesh->GetName());
    }

    FbxDeformer* deformer = fbx_mesh->GetDeformer(0);
    if (deformer->GetDeformerType() != FbxDeformer::eSkin)
    {
        DPRINTF("Invalid deformer type.\n");
        return false;
    }

    FbxSkin *skin = (FbxSkin *)deformer;
    weights.resize(vertex_count);

    const int cluster_count = skin->GetClusterCount();
    for (int i = 0; i < cluster_count; i++)
    {
         FbxCluster* cluster = skin->GetCluster(i);
         assert(cluster);
         FbxNode* linked_node = cluster->GetLink();
         assert(linked_node);

         const int vertex_index_count = cluster->GetControlPointIndicesCount();
         for (int j = 0; j < vertex_index_count; j++)
         {
             int index = cluster->GetControlPointIndices()[j];
             if (index >= vertex_count)
             {
                 continue;
             }

             std::pair<double, FbxNode *> skin_link;

             skin_link.first = cluster->GetControlPointWeights()[j];
             skin_link.second = linked_node;

             bool added = false;
             std::vector<std::pair<double, FbxNode *>> &skin_bindings = weights[index];

             for (size_t k = 0; k < skin_bindings.size(); k++)
             {
                 if (skin_bindings[k].first < skin_link.first)
                 {
                     skin_bindings.insert(skin_bindings.begin()+k, skin_link);
                     added = true;
                     break;
                 }
             }

             if (!added)
                 weights[index].push_back(skin_link);
         }
    }

    return true;
}

bool SubPart::LoadFbxUV(FbxMesh *fbx_mesh, int poly_index, int pos_in_poly, int control_point_index, VertexData *v)
{
    if (fbx_mesh->GetElementUVCount() <= 0)
    {
        DPRINTF("%s: No UV!\n", FUNCNAME);
        return false;
    }

    FbxGeometryElementUV* uv = fbx_mesh->GetElementUV(0);

    switch (uv->GetMappingMode())
    {
        case FbxGeometryElement::eByControlPoint:

            switch (uv->GetReferenceMode())
            {
                case FbxGeometryElement::eDirect:
                    v->VertexUnion.vertex64.common.tex.u = (float)uv->GetDirectArray().GetAt(control_point_index)[0];
                    v->VertexUnion.vertex64.common.tex.v = (float)uv->GetDirectArray().GetAt(control_point_index)[1];
                break;

                case FbxGeometryElement::eIndexToDirect:
                {
                    int id = uv->GetIndexArray().GetAt(control_point_index);
                    v->VertexUnion.vertex64.common.tex.u = (float)uv->GetDirectArray().GetAt(id)[0];
                    v->VertexUnion.vertex64.common.tex.v = (float)uv->GetDirectArray().GetAt(id)[1];
                }

                break;

                default:
                    DPRINTF("%s: Unsupported reference mode.\n", FUNCNAME);
                    return false;
                break;
            }            

        break;

        case FbxGeometryElement::eByPolygonVertex:
        {
            int texture_uv_index = fbx_mesh->GetTextureUVIndex(poly_index, pos_in_poly);
            switch (uv->GetReferenceMode())
            {
                case FbxGeometryElement::eDirect:
                case FbxGeometryElement::eIndexToDirect:
                    v->VertexUnion.vertex64.common.tex.u = (float)uv->GetDirectArray().GetAt(texture_uv_index)[0];
                    v->VertexUnion.vertex64.common.tex.v = (float)uv->GetDirectArray().GetAt(texture_uv_index)[1];                    

                break;

                default:
                    DPRINTF("%s: Unsupported reference mode (2)\n", FUNCNAME);
                    return false;
                break;
            }
        }
        break;

        default:
            DPRINTF("%s: Unsupported mapping mode.\n", FUNCNAME);
            return false;
        break;
    }

    v->VertexUnion.vertex64.common.tex.u = v->VertexUnion.vertex64.common.tex.u;
    v->VertexUnion.vertex64.common.tex.v = -v->VertexUnion.vertex64.common.tex.v;
    return true;
}

bool SubPart::LoadFbxNormal(FbxNode *fbx_node, int vertex_index, int control_point_index, VertexData *v)
{
    FbxVector4 vec4;
    FbxMesh *fbx_mesh = fbx_node->GetMesh();

    if (fbx_mesh->GetElementNormalCount() <= 0)
    {
        DPRINTF("%s: no normals!\n", FUNCNAME);
        return false;
    }

    FbxGeometryElementNormal* normal = fbx_mesh->GetElementNormal(0);
    FbxGeometryElement::EMappingMode mapping_mode = normal->GetMappingMode();
    if (mapping_mode == FbxGeometryElement::eByControlPoint)
    {
        switch (normal->GetReferenceMode())
        {
            case FbxGeometryElement::eDirect:
            {
                vec4 = normal->GetDirectArray().GetAt(control_point_index);
            }
            break;

            case FbxGeometryElement::eIndexToDirect:
            {
                int id = normal->GetIndexArray().GetAt(control_point_index);
                vec4 = normal->GetDirectArray().GetAt(id);
            }
            break;

            default:
                DPRINTF("%s: Unsuuported reference mode.\n", FUNCNAME);
                return false;
            break;
        }
    }
    else if (mapping_mode == FbxGeometryElement::eByPolygonVertex)
    {
        switch (normal->GetReferenceMode())
        {
            case FbxGeometryElement::eDirect:
            {
                vec4 = normal->GetDirectArray().GetAt(vertex_index);
            }
            break;

            case FbxGeometryElement::eIndexToDirect:
            {
                int id = normal->GetIndexArray().GetAt(vertex_index);
                vec4 = normal->GetDirectArray().GetAt(id);
            }
            break;

            default:
                DPRINTF("%s: Unsupported reference mode (2)\n", FUNCNAME);
                return false;
            break;
        }
    }
    else
    {
        DPRINTF("%s: Unsupported mapping mode.\n", FUNCNAME);
        return false;
    }

    /*FbxAMatrix rotation_matrix = fbx_node->EvaluateGlobalTransform();
    rotation_matrix.SetT(FbxVector4(0.0, 0.0, 0.0, 0.0));
    rotation_matrix.SetS(FbxVector4(1.0, 1.0, 1.0, 1.0));

    vec4 = rotation_matrix.MultT(vec4);*/

    // +++ --- -++ --+ -+- ++- +-+ +--
    v->VertexUnion.vertex64.common.norm.x = (float)vec4[0];
    v->VertexUnion.vertex64.common.norm.y = (float)vec4[1];
    v->VertexUnion.vertex64.common.norm.z = (float)vec4[2];

    //normal->GetDirectArray().SetAt(control_point_index, vec4);

    return true;
}

bool SubPart::LoadFbxVertexColor(FbxMesh *fbx_mesh, int vertex_index, int control_point_index, uint32_t *color)
{
    if (fbx_mesh->GetElementVertexColorCount() <= 0)
    {
        DPRINTF("%s: no vertex colors!\n", FUNCNAME);
        return false;
    }

    FbxGeometryElementVertexColor* vertex_color = fbx_mesh->GetElementVertexColor(0);
    FbxGeometryElement::EMappingMode mapping_mode = vertex_color->GetMappingMode();
    if (mapping_mode == FbxGeometryElement::eByControlPoint)
    {
        switch (vertex_color->GetReferenceMode())
        {
            case FbxGeometryElement::eDirect:
            {
                FbxColor v_color = vertex_color->GetDirectArray().GetAt(control_point_index);
                *color = Utils::DoubleToColor(v_color.mRed, v_color.mGreen, v_color.mBlue, v_color.mAlpha);
            }
            break;

            case FbxGeometryElement::eIndexToDirect:
            {
                int id = vertex_color->GetIndexArray().GetAt(control_point_index);
                FbxColor v_color = vertex_color->GetDirectArray().GetAt(id);
                *color = Utils::DoubleToColor(v_color.mRed, v_color.mGreen, v_color.mBlue, v_color.mAlpha);
            }
            break;

            default:
                DPRINTF("%s: Unsuuported reference mode.\n", FUNCNAME);
                return false;
            break;
        }
    }
    else if (mapping_mode == FbxGeometryElement::eByPolygonVertex)
    {
        switch (vertex_color->GetReferenceMode())
        {
            case FbxGeometryElement::eDirect:
            {
                FbxColor v_color = vertex_color->GetDirectArray().GetAt(vertex_index);
                *color = Utils::DoubleToColor(v_color.mRed, v_color.mGreen, v_color.mBlue, v_color.mAlpha);
            }
            break;

            case FbxGeometryElement::eIndexToDirect:
            {
                int id = vertex_color->GetIndexArray().GetAt(vertex_index);
                FbxColor v_color = vertex_color->GetDirectArray().GetAt(id);
                *color = Utils::DoubleToColor(v_color.mRed, v_color.mGreen, v_color.mBlue, v_color.mAlpha);
            }
            break;

            default:
                DPRINTF("%s: Unsupported reference mode (2)\n", FUNCNAME);
                return false;
            break;
        }
    }
    else
    {
        DPRINTF("%s: Unsupported mapping mode.\n", FUNCNAME);
        return false;
    }

    return true;
}

bool SubPart::LoadFbxTangent(FbxMesh *fbx_mesh, int vertex_index, int control_point_index, VertexData *v)
{
    FbxVector4 vec4;    

    if (fbx_mesh->GetElementTangentCount() <= 0)
    {
        DPRINTF("%s: no tangents!\n", FUNCNAME);
        return false;
    }

    FbxGeometryElementTangent* tangent = fbx_mesh->GetElementTangent(0);
    FbxGeometryElement::EMappingMode mapping_mode = tangent->GetMappingMode();
    if (mapping_mode == FbxGeometryElement::eByControlPoint)
    {
        switch (tangent->GetReferenceMode())
        {
            case FbxGeometryElement::eDirect:
            {
                vec4 = tangent->GetDirectArray().GetAt(control_point_index);
            }
            break;

            case FbxGeometryElement::eIndexToDirect:
            {
                int id = tangent->GetIndexArray().GetAt(control_point_index);
                vec4 = tangent->GetDirectArray().GetAt(id);
            }
            break;

            default:
                DPRINTF("%s: Unsuuported reference mode.\n", FUNCNAME);
                return false;
            break;
        }
    }
    else if (mapping_mode == FbxGeometryElement::eByPolygonVertex)
    {
        switch (tangent->GetReferenceMode())
        {
            case FbxGeometryElement::eDirect:
            {
                vec4 = tangent->GetDirectArray().GetAt(vertex_index);
            }
            break;

            case FbxGeometryElement::eIndexToDirect:
            {
                int id = tangent->GetIndexArray().GetAt(vertex_index);
                vec4 = tangent->GetDirectArray().GetAt(id);
            }
            break;

            default:
                DPRINTF("%s: Unsupported reference mode (2)\n", FUNCNAME);
                return false;
            break;
        }
    }
    else
    {
        DPRINTF("%s: Unsupported mapping mode.\n", FUNCNAME);
        return false;
    }

    v->VertexUnion.vertex64.tex2[0] = (float)vec4[0];
    v->VertexUnion.vertex64.tex2[1] = (float)vec4[1];
    v->VertexUnion.vertex64.tex2[2] = (float)vec4[2];

    if (vec4[3] > 0) // direction
    {
        v->VertexUnion.vertex64.tex2[0] = -v->VertexUnion.vertex64.tex2[0];
        v->VertexUnion.vertex64.tex2[1] = -v->VertexUnion.vertex64.tex2[1];
        v->VertexUnion.vertex64.tex2[2] = -v->VertexUnion.vertex64.tex2[2];
    }

    return true;
}

// No binormals in emo, tex2 are tangent...
/*bool SubPart::LoadFbxBinormal(FbxMesh *fbx_mesh, int vertex_index, int control_point_index, VertexData *v)
{
    FbxVector4 vec4;

    if (fbx_mesh->GetElementBinormalCount() <= 0)
    {
        DPRINTF("%s: no binormals!\n", FUNCNAME);
        return false;
    }

    FbxGeometryElementBinormal* binormal = fbx_mesh->GetElementBinormal(0);
    FbxGeometryElement::EMappingMode mapping_mode = binormal->GetMappingMode();
    if (mapping_mode == FbxGeometryElement::eByControlPoint)
    {
        switch (binormal->GetReferenceMode())
        {
            case FbxGeometryElement::eDirect:
            {
                vec4 = binormal->GetDirectArray().GetAt(control_point_index);
            }
            break;

            case FbxGeometryElement::eIndexToDirect:
            {
                int id = binormal->GetIndexArray().GetAt(control_point_index);
                vec4 = binormal->GetDirectArray().GetAt(id);
            }
            break;

            default:
                DPRINTF("%s: Unsuuported reference mode.\n", FUNCNAME);
                return false;
            break;
        }
    }
    else if (mapping_mode == FbxGeometryElement::eByPolygonVertex)
    {
        switch (binormal->GetReferenceMode())
        {
            case FbxGeometryElement::eDirect:
            {
                vec4 = binormal->GetDirectArray().GetAt(vertex_index);
            }
            break;

            case FbxGeometryElement::eIndexToDirect:
            {
                int id = binormal->GetIndexArray().GetAt(vertex_index);
                vec4 = binormal->GetDirectArray().GetAt(id);
            }
            break;

            default:
                DPRINTF("%s: Unsupported reference mode (2)\n", FUNCNAME);
                return false;
            break;
        }
    }
    else
    {
        DPRINTF("%s: Unsupported mapping mode.\n", FUNCNAME);
        return false;
    }

    v->VertexUnion.vertex64.tex2[0] = -(float)vec4[0];
    v->VertexUnion.vertex64.tex2[1] = -(float)vec4[1];
    v->VertexUnion.vertex64.tex2[2] = -(float)vec4[2];

    if (vec4[3] < 0) // direction
    {
        v->VertexUnion.vertex64.tex2[0] = -v->VertexUnion.vertex64.tex2[0];
        v->VertexUnion.vertex64.tex2[1] = -v->VertexUnion.vertex64.tex2[1];
        v->VertexUnion.vertex64.tex2[2] = -v->VertexUnion.vertex64.tex2[2];
    }

    return true;
}*/

bool SubPart::LoadFbxBlendData(SkeletonFile &skl, const std::vector<Bone *> &linked_bones, const std::vector<std::pair<double, FbxNode *> > &vertex_weights, uint8_t *blend, float *blend_weight)
{
    size_t size = vertex_weights.size();

    if (size > 4)
    {
        DPRINTF("blend_weight has more than 4 elements (%d)! Not allowed.\n", size);
        return false;
    }

    blend[0] = blend[1] = blend[2] = blend[3] = 0;
    blend_weight[0] = blend_weight[1] = blend_weight[2] = 0.0f;

    for (size_t i = 0; i < size; i++)
    {

        FbxNode *fbx_bone = vertex_weights[i].second;
        assert(fbx_bone != nullptr);

        const char *bone_name = fbx_bone->GetName();
        Bone *b = skl.GetBone(bone_name);
        if (!b)
        {
            DPRINTF("%s: Cannot find bone \"%s\"\n", bone_name);
            return false;
        }

        const std::vector<Bone *>::const_iterator it = std::find(linked_bones.begin(), linked_bones.end(), b);
        if (it == linked_bones.end())
        {
            DPRINTF("%s: Bone is not in linked bones. Weird.\n", FUNCNAME);
            return false;
        }

        blend[i] = it - linked_bones.begin();

        if (i < 3)
            blend_weight[i] = (float)vertex_weights[i].first;

    }

    return true;
}

bool SubPart::InjectSubMeshFbx(SkeletonFile &skl, SubMesh &submesh, FbxNode *fbx_node, std::vector<VertexData> &sm_vertex, int vertex_size, size_t v_start, bool use_fbx_tangent)
{
    std::vector<std::vector<std::pair<double, FbxNode *>>> blend_weights;
    FbxMesh *fbx_mesh = fbx_node->GetMesh();

    assert(fbx_mesh);

    if (IsEdge() && vertex_size == sizeof(Vertex64))
    {
        throw std::runtime_error("We weren't expecting an edge subpart with Vertex64 data! Report the emo file to eternity.\n");
    }

    if (IsEdge())
    {
        // both 3ds max and fbx sdk mess the normals of edge parts, so we have to generate them in inverse form.
        fbx_mesh->GenerateNormals(true, true, true);
    }

    if (vertex_size == sizeof(Vertex64) && !use_fbx_tangent)
    {
        fbx_mesh->GenerateTangentsData(0, true);
    }

    if (!LoadFbxBlendWeights(fbx_mesh, blend_weights))
        return false;

    submesh.linked_bones.clear();

    for (auto &i : blend_weights)
    {
        for (std::pair<double, FbxNode *> &link: i)
        {
            const char *bone_name = link.second->GetName();
            Bone *b = skl.GetBone(bone_name);
            if (!b)
            {
                DPRINTF("Cannot find bone \"%s\" in skeleton.\n", bone_name);
                return false;
            }

            if (std::find(submesh.linked_bones.begin(), submesh.linked_bones.end(), b) == submesh.linked_bones.end())
            {
                submesh.linked_bones.push_back(b);
            }
        }
    }

    FbxVector4* control_points = fbx_mesh->GetControlPoints();
    const int polygon_count = fbx_mesh->GetPolygonCount();
    size_t vertex_index = 0;

    submesh.faces.resize(polygon_count*3);
    sm_vertex.clear();

    for (int poly_index = 0; poly_index < polygon_count; poly_index++)
    {
        const int polygon_size = fbx_mesh->GetPolygonSize(poly_index);

        if (polygon_size != 3)
        {
            DPRINTF("Unsupported polygin_size: %d\n", polygon_size);
            return false;
        }

        for (int pos_in_poly = 0; pos_in_poly < polygon_size; pos_in_poly++)
        {
            VertexData v;
            uint32_t *color;
            uint8_t *indices;
            float *weights;

            v.size = vertex_size;

            assert(vertex_size == sizeof(Vertex64) || vertex_size == sizeof(Vertex52));

            if (vertex_size == sizeof(Vertex64))
            {
                color = &v.VertexUnion.vertex64.color;
                indices = v.VertexUnion.vertex64.blend;
                weights = v.VertexUnion.vertex64.blend_weight;
            }
            else
            {
                color = &v.VertexUnion.vertex52.color;
                indices = v.VertexUnion.vertex52.blend;
                weights = v.VertexUnion.vertex52.blend_weight;
            }

            int control_point_index = fbx_mesh->GetPolygonVertex(poly_index, pos_in_poly);

            FbxVector4& position = control_points[control_point_index];

            v.VertexUnion.vertex64.common.pos.x = (float)position[0];
            v.VertexUnion.vertex64.common.pos.y = (float)position[1];
            v.VertexUnion.vertex64.common.pos.z = (float)position[2];

            if (!LoadFbxUV(fbx_mesh, poly_index, pos_in_poly, control_point_index, &v))
                return false;

            if (!LoadFbxNormal(fbx_node, vertex_index, control_point_index, &v))
                return false;

            if (!LoadFbxVertexColor(fbx_mesh, vertex_index, control_point_index, color))
                return false;

            if (vertex_size == sizeof(Vertex64))
            {
                if (!LoadFbxTangent(fbx_mesh, vertex_index, control_point_index, &v))
                    return false;               
            }

            if (!LoadFbxBlendData(skl, submesh.linked_bones, blend_weights[control_point_index], indices, weights))
                return false;

            auto it = std::find(sm_vertex.begin(), sm_vertex.end(), v);
            if (it == sm_vertex.end())
            {
                submesh.faces[(poly_index*3)+pos_in_poly] = sm_vertex.size() + v_start;
                sm_vertex.push_back(v);
            }
            else
            {
                submesh.faces[(poly_index*3)+pos_in_poly] = v_start + (it - sm_vertex.begin());
            }

            vertex_index++;
        }
    }

    return true;
}

bool SubPart::InjectFbx(SkeletonFile &skl, const std::vector<FbxNode *> fbx_nodes, bool use_fbx_tangent)
{
    DPRINTF("Injecting subpart %s\n", meta_name.c_str());

    if (fbx_nodes.size() != submeshes.size())
    {
        DPRINTF("%s: function usage error, number of fbx_meshes must be same as nuber of submeshes.\n", FUNCNAME);
        return false;
    }

    if (submeshes.size() == 0)
        return true; // yes, true

    assert(vertex.size() > 0);
    const int vertex_size = vertex[0].size;

    size_t v_start = 0;
    //DPRINTF("Vertex before: %d\n", vertex.size());
    vertex.clear();

    for (size_t i = 0; i < submeshes.size(); i++)
    {
        std::vector<VertexData> sm_vertex;

        if (!InjectSubMeshFbx(skl, submeshes[i], fbx_nodes[i], sm_vertex, vertex_size, v_start, use_fbx_tangent))
            return false;

        vertex.resize(v_start + sm_vertex.size());

        for (size_t j = v_start; j < vertex.size(); j++)
        {
            vertex[j] = sm_vertex[j-v_start];
        }

        v_start += sm_vertex.size();
    }

    SetStrips(false);
    //DPRINTF("Vertex after: %d\n", vertex.size());
    return true;
}

bool SubPart::InjectFbx(SkeletonFile &skl, FbxScene *scene, bool use_fbx_tangent)
{
    std::vector<FbxNode *> fbx_nodes;

    for (size_t i = 0; i < submeshes.size(); i++)
    {
        bool found = false;

        const int node_count = scene->GetSrcObjectCount<FbxNode>();
        for (int j = 0; j < node_count; j++)
        {
            FbxNode *node = scene->GetSrcObject<FbxNode>(j);
            if (!node)
                continue;

            FbxMesh *mesh = node->GetMesh();
            if (!mesh)
                continue;

            //DPRINTF("Name: %s\n", mesh->GetName());

            if (submeshes[i].meta_name == node->GetName())
            {
                fbx_nodes.push_back(node);
                found = true;
            }
        }

        if (!found)
        {
            DPRINTF("SubMesh \"%s\" not found in fbx, we will delete it from .emo.\n", submeshes[i].meta_name.c_str());
            submeshes.erase(submeshes.begin() + i);
            i--;
        }
    }

    if (submeshes.size() == 0)
        return true; // All submeshes were deleted

    return InjectFbx(skl, fbx_nodes, use_fbx_tangent);
}

#endif

bool SubPart::operator==(const SubPart &rhs) const
{	
    if (this->textures_lists != rhs.textures_lists)
        return false;

    if (this->vertex != rhs.vertex)
        return false;

    if (this->submeshes != rhs.submeshes)
        return false;

    if (this->strips != rhs.strips)
        return false;

    if (memcmp(this->vectors, rhs.vectors, sizeof(this->vectors)) != 0)
        return false;

    if (this->flags != rhs.flags)
        return false;

    if (this->unk_02 != rhs.unk_02)
        return false;

    if (this->unk_06 != rhs.unk_06)
        return false;

    if (this->unk_08 != rhs.unk_08)
        return false;

    return true;
}

size_t TexturesList::GetEmbIndexes(std::vector<uint8_t> &list, bool clear_vector, bool unique, bool sort) const
{
    size_t count = 0;

    if (clear_vector)
        list.clear();

    for (const Texture &t : textures)
    {
        if (unique)
        {
            if (std::find(list.begin(), list.end(), t.emb_index) != list.end())
                continue;
        }

        list.push_back(t.emb_index);
        count++;
    }

    if (sort)
    {
        std::sort(list.begin(), list.end());
    }

    return count;
}

size_t TexturesList::ReplaceEmbIndex(uint8_t old_index, uint8_t new_index)
{
    size_t count = 0;

    for (Texture &t : textures)
    {
        if (t.emb_index == old_index)
        {
            t.emb_index = new_index;
            count++;
        }
    }

    return count;
}

void TexturesList::Decompile(TiXmlNode *root, uint16_t id) const
{
    TiXmlElement *entry_root = new TiXmlElement("TexturesList");
    entry_root->SetAttribute("id", Utils::UnsignedToString(id, true));

    for (const Texture &t : textures)
    {
        t.Decompile(entry_root);
    }

    root->LinkEndChild(entry_root);
}

bool TexturesList::Compile(const TiXmlElement *root)
{
    for (const TiXmlElement *elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
    {
        const std::string &str = elem->ValueStr();

        if (str == "Texture")
        {
            Texture t;

            if (!t.Compile(elem))
            {
                DPRINTF("%s: Compilation of texture failed.\n", FUNCNAME);
                return false;
            }

            textures.push_back(t);
        }
    }

    return true;
}

size_t SubMesh::GetLinkedBones(std::vector<Bone *> &list, bool clear_vector, bool unique, const SkeletonFile *sorter) const
{
    size_t count = 0;

    if (clear_vector)
    {
        list.clear();
    }

    for (Bone *b : linked_bones)
    {
        if (unique)
        {
            if (std::find(list.begin(), list.end(), b) != list.end())
                continue;
        }

        list.push_back(b);
        count++;
    }

    if (sorter)
    {
        std::sort(list.begin(), list.end(), BoneSorter(sorter));
    }

    return count;
}

void SubMesh::Decompile(TiXmlNode *root, uint32_t id, size_t polygon_count) const
{
    TiXmlElement *entry_root = new TiXmlElement("SubMesh");
    entry_root->SetAttribute("id", Utils::UnsignedToString(id, true));

    if (meta_name != "")
    {
        Utils::WriteComment(entry_root, meta_name.c_str());
    }

    Utils::WriteParamString(entry_root, "EMMMaterial", emm_material);
    Utils::WriteParamUnsigned(entry_root, "TEXTURES_LISTS_INDEX", tl_index, true);
    Utils::WriteParamMultipleFloats(entry_root, "VECTOR", std::vector<float>(vector, vector+4));

    Utils::WriteComment(entry_root, Utils::ToString(faces.size()) + " faces. " + Utils::ToString(polygon_count) + " polygons.");
    Utils::WriteParamMultipleUnsigned(entry_root, "FACES", faces, true);
	
	std::string bones_names;

    for (Bone *b : linked_bones)
    {
        if (bones_names.length() != 0)
			bones_names += ", ";
		
		bones_names += b->GetName();
    }
	
	Utils::WriteParamString(entry_root, "LINKED_BONES", bones_names);

    root->LinkEndChild(entry_root);
}

bool SubMesh::Compile(const TiXmlElement *root, SkeletonFile *skl)
{
    if (!Utils::GetParamString(root, "EMMMaterial", emm_material))
        return false;

    if (emm_material.length() > 31)
    {
        DPRINTF("%s: EMMMaterial can't have more than 31 characters. (faulting value = %s)\n", FUNCNAME, emm_material.c_str());
        return false;
    }

    unsigned int tl_index;

    if (!Utils::GetParamUnsigned(root, "TEXTURES_LISTS_INDEX", &tl_index))
        return false;

    if (tl_index > 0xFFFF)
    {
        DPRINTF("%s: TEXTURES_LISTS_INDEX must be a 16 bits value.\n", FUNCNAME);
        return false;
    }

    this->tl_index = tl_index;

    std::vector<float> vector;

    if (!Utils::GetParamMultipleFloats(root, "VECTOR", vector))
        return false;

    if (vector.size() != 4)
    {
        DPRINTF("%s: Invalid size for \"VECTOR\"\n", FUNCNAME);
        return false;
    }

    memcpy(this->vector, vector.data(), sizeof(this->vector));

    if (!Utils::GetParamMultipleUnsigned(root, "FACES", faces))
        return false;
	
	std::vector<std::string> bone_names;
	
	if (!Utils::GetParamMultipleStrings(root, "LINKED_BONES", bone_names))
		return false;
	
	for (const std::string &s : bone_names)
	{		
		Bone *bone = skl->GetBone(s);
		if (!bone)
		{
            DPRINTF("%s: bone \"%s\" doesn't exist in skeleton.\n", FUNCNAME, s.c_str());
			return false;
		}

		linked_bones.push_back(bone);
	}    

    return true;
}

bool SubMesh::operator==(const SubMesh &rhs) const
{
    if (this->emm_material != rhs.emm_material)
        return false;

    if (this->tl_index != rhs.tl_index)
        return false;

    if (this->faces != rhs.faces)
        return false;

    if (memcmp(this->vector, rhs.vector, sizeof(this->vector)) != 0)
        return false;

    if (this->linked_bones.size() != rhs.linked_bones.size())
        return false;

    for (size_t i = 0; i < this->linked_bones.size(); i++)
    {
        assert(this->linked_bones[i] != nullptr && rhs.linked_bones[i] != nullptr);

        if (*this->linked_bones[i] != *rhs.linked_bones[i])
            return false;
    }

    return true;
}

EmgFile::EmgFile()
{
    this->big_endian = false;
}

EmgFile::~EmgFile()
{
    Reset();
}

void EmgFile::Reset()
{
    subparts.clear();    
}

size_t EmgFile::GetNumVertex()
{
    size_t count = 0;

    for (SubPart &sp : subparts)
    {
        count += sp.vertex.size();
    }

    return count;
}

bool EmgFile::Load(uint8_t *buf, size_t size, SkeletonFile *skl)
{
    UNUSED(size);

    Reset();

    EMGChunkHeader *emgc = (EMGChunkHeader *)buf;

    if (memcmp(&emgc->signature, "#EMG", 4) != 0)
    {
        //DPRINTF("%s: Bad file, #EMG signature was not found where expected (in group %s)\n", __PRETTY_FUNCTION__, group.name.c_str());
        return false;
    }

    this->unk_04 = val16(emgc->unk_04);

    for (uint16_t i = 0; i < val16(emgc->subparts_count); i++)
    {
        SubPart subpart;
        EMGHeader *ehdr = (EMGHeader *)GetOffsetPtr(emgc, emgc->offsets, i);

        assert(emgc->offsets[i] != 0);

        if (meta_name != "")
        {
            char sp_meta_name[2048];

            snprintf(sp_meta_name, sizeof(sp_meta_name), "%s_%04x", meta_name.c_str(), i);
            subpart.meta_name = sp_meta_name;
        }

        subpart.flags = val16(ehdr->flags);
        subpart.unk_02 = val16(ehdr->unk_02);
        subpart.unk_06 = val16(ehdr->unk_06);
        subpart.unk_08 = val32(ehdr->unk_08);
        subpart.strips = val16(ehdr->strips);

        for (int j = 0; j < 12; j++)
        {
            subpart.vectors[j] = val_float(ehdr->vectors[j]);
        }

        uint32_t *ttable = (uint32_t *)GetOffsetPtr(ehdr, ehdr->textures_lists_offset);

        for (uint16_t j = 0; j < val16(ehdr->textures_lists_count); j++)
        {
            TexturesList textures_list;
            uint16_t *pnum_th = (uint16_t *)GetOffsetPtr(ehdr, ttable, j);
            EMGTextureHeader *thdr = (EMGTextureHeader *)GetOffsetPtr(pnum_th, sizeof(uint32_t), true);

            if (pnum_th[1] != 0)
            {
                DPRINTF("%s: pnum_th[1] not 0 as expected.\n", FUNCNAME);
                return false;
            }

            for (uint32_t k = 0; k < val16(*pnum_th); k++)
            {
                Texture texture;

                texture.unk_00 = thdr[k].unk_00;
                texture.unk_02[0] = thdr[k].unk_02[0];
                texture.unk_02[1] = thdr[k].unk_02[1];
                texture.emb_index = thdr[k].emb_index;
                texture.f1 = val_float(thdr[k].f1);
                texture.f2 = val_float(thdr[k].f2);

                textures_list.textures.push_back(texture);
            }

            subpart.textures_lists.push_back(textures_list);
        }

        uint32_t *smtable = (uint32_t *)GetOffsetPtr(ehdr, ehdr->submesh_list_offset);

        for (uint16_t j = 0; j < val16(ehdr->submesh_count); j++)
        {
            SubMesh submesh;
            EMGSubMeshHeader *shdr = (EMGSubMeshHeader *)GetOffsetPtr(ehdr, smtable, j);

            assert(smtable[j] != 0);

            if (subpart.meta_name != "")
            {
                char sm_meta_name[2048];
                snprintf(sm_meta_name, sizeof(sm_meta_name), "%s_%04x", subpart.meta_name.c_str(), j);

                submesh.meta_name = sm_meta_name;
            }

            submesh.emm_material = shdr->emm_material;
            submesh.tl_index = val16(shdr->tl_index);
            submesh.vector[0] = val_float(shdr->vector[0]);
            submesh.vector[1] = val_float(shdr->vector[1]);
            submesh.vector[2] = val_float(shdr->vector[2]);
            submesh.vector[3] = val_float(shdr->vector[3]);

            uint16_t *faces = (uint16_t *)GetOffsetPtr(shdr, sizeof(EMGSubMeshHeader), true);
            for (uint16_t k = 0; k < val16(shdr->face_count); k++)
            {
                submesh.faces.push_back(val16(faces[k]));
            }

            uint16_t *bones_idx = (uint16_t *)GetOffsetPtr(faces, val16(shdr->face_count) * sizeof(uint16_t), true);

            for (uint16_t k = 0; k < val16(shdr->linked_bones_count); k++)
            {
                if (val16(bones_idx[k]) >= skl->bones.size())
                {
                    DPRINTF("%s: bone 0x%x out of bounds in subpart %04x\n", FUNCNAME, k, i);
                    return false;
                }

                submesh.linked_bones.push_back(&skl->bones[val16(bones_idx[k])]);
            }

            subpart.submeshes.push_back(submesh);
        }

        if (ehdr->vertex_count == 0)
        {
            DPRINTF("Number of vertex can't be 0!\n");
            return false;
        }

        uint16_t vertex_size = val16(ehdr->vertex_size);
        if (vertex_size != sizeof(Vertex52) && vertex_size != sizeof(Vertex64))
        {
            DPRINTF("%s: unknown vertex size: %x\n", FUNCNAME, vertex_size);
            return false;
        }

        uint8_t *vertex = GetOffsetPtr(ehdr, ehdr->vertex_offset);

        for (uint16_t j = 0; j < val16(ehdr->vertex_count); j++)
        {
            VertexData vd;
            VertexCommon *vci, *vco;

            if (vertex_size == sizeof(Vertex52))
            {
                Vertex52 *v = (Vertex52 *)vertex;

                vd.size = sizeof(Vertex52);

                if (!big_endian)
                {
                    vd.VertexUnion.vertex52.blend[0] = v->blend[0];
                    vd.VertexUnion.vertex52.blend[1] = v->blend[1];
                    vd.VertexUnion.vertex52.blend[2] = v->blend[2];
                    vd.VertexUnion.vertex52.blend[3] = v->blend[3];
                }
                else
                {
                    vd.VertexUnion.vertex52.blend[0] = v->blend[3];
                    vd.VertexUnion.vertex52.blend[1] = v->blend[2];
                    vd.VertexUnion.vertex52.blend[2] = v->blend[1];
                    vd.VertexUnion.vertex52.blend[3] = v->blend[0];
                }

                vd.VertexUnion.vertex52.blend_weight[0] = val_float(v->blend_weight[0]);
                vd.VertexUnion.vertex52.blend_weight[1] = val_float(v->blend_weight[1]);
                vd.VertexUnion.vertex52.blend_weight[2] = val_float(v->blend_weight[2]);

                vd.VertexUnion.vertex52.color = val32(v->color);

                vci = &v->common;
                vco = &vd.VertexUnion.vertex52.common;
            }
            else
            {
                Vertex64 *v = (Vertex64 *)vertex;

                vd.size = sizeof(Vertex64);

                vd.VertexUnion.vertex64.color = val32(v->color);

                if (!big_endian)
                {
                    vd.VertexUnion.vertex64.blend[0] = v->blend[0];
                    vd.VertexUnion.vertex64.blend[1] = v->blend[1];
                    vd.VertexUnion.vertex64.blend[2] = v->blend[2];
                    vd.VertexUnion.vertex64.blend[3] = v->blend[3];
                }
                else
                {
                    vd.VertexUnion.vertex64.blend[0] = v->blend[3];
                    vd.VertexUnion.vertex64.blend[1] = v->blend[2];
                    vd.VertexUnion.vertex64.blend[2] = v->blend[1];
                    vd.VertexUnion.vertex64.blend[3] = v->blend[0];
                }

                vd.VertexUnion.vertex64.blend_weight[0] = val_float(v->blend_weight[0]);
                vd.VertexUnion.vertex64.blend_weight[1] = val_float(v->blend_weight[1]);
                vd.VertexUnion.vertex64.blend_weight[2] = val_float(v->blend_weight[2]);
                vd.VertexUnion.vertex64.tex2[0] = val_float(v->tex2[0]);
                vd.VertexUnion.vertex64.tex2[1] = val_float(v->tex2[1]);
                vd.VertexUnion.vertex64.tex2[2] = val_float(v->tex2[2]);

                vci = &v->common;
                vco = &vd.VertexUnion.vertex64.common;
            }

            vco->pos.x = val_float(vci->pos.x);
            vco->pos.y = val_float(vci->pos.y);
            vco->pos.z = val_float(vci->pos.z);

            vco->norm.x = val_float(vci->norm.x);
            vco->norm.y = val_float(vci->norm.y);
            vco->norm.z = val_float(vci->norm.z);

            vco->tex.u = val_float(vci->tex.u);
            vco->tex.v = val_float(vci->tex.v);

            subpart.vertex.push_back(vd);
            vertex += vertex_size;
        }

        this->subparts.push_back(subpart);
    }

    return true;
}

size_t EmgFile::GetLinkedBones(std::vector<Bone *> &list, bool clear_vector, bool unique, const SkeletonFile *sorter) const
{
    size_t count = 0;

    if (clear_vector)
        list.clear();

    for (const SubPart &sp : subparts)
    {
        count += sp.GetLinkedBones(list, false, unique);
    }

    if (sorter)
    {
        std::sort(list.begin(), list.end(), BoneSorter(sorter));
    }

    return count;
}

size_t EmgFile::GetEmmMaterials(std::vector<std::string> &list, bool clear_vector, bool unique) const
{
    size_t count = 0;

    if (clear_vector)
        list.clear();

    for (const SubPart &sp : subparts)
    {
        count += sp.GetEmmMaterials(list, false, unique);
    }

    return count;
}

size_t EmgFile::GetEmbIndexes(std::vector<uint8_t> &list, bool clear_vector, bool unique, bool sort) const
{
    size_t count = 0;

    if (clear_vector)
        list.clear();

    for (const SubPart &sp : subparts)
    {
        count += sp.GetEmbIndexes(list, false, unique);
    }

    if (sort)
    {
        std::sort(list.begin(), list.end());
    }

    return count;
}

size_t EmgFile::ReplaceEmmMaterial(const std::string &old_mat, const std::string &new_mat)
{
    if (new_mat.length() > SubMesh::MAX_EMM_MAT_LENGTH)
        return (size_t)-1;

    size_t count = 0;

    for (SubPart &sp : subparts)
    {
        count += sp.ReplaceEmmMaterial(old_mat, new_mat);
    }

    return count;
}

size_t EmgFile::ReplaceEmbIndex(uint8_t old_index, uint8_t new_index)
{
    size_t count = 0;

    for (SubPart &sp : subparts)
    {
        count += sp.ReplaceEmbIndex(old_index, new_index);
    }

    return count;
}

bool EmgFile::IsEdge() const
{
    size_t count = 0;

    for (const SubPart &sp : subparts)
    {
        if (sp.IsEdge())
            count++;
    }

    assert(count == 0 || count == subparts.size());
    return (count != 0);
}

size_t EmgFile::ExportObj(std::string *vertex_out, std::string *uvmap_out, std::string *normal_out, std::string *topology_out, size_t v_start_idx, bool write_group) const
{
    if (!vertex_out && !uvmap_out && !normal_out && !topology_out)
        return 0;

    size_t count = 0;
    std::string ret_v, ret_uv, ret_n, ret_t;

    for (const SubPart &sp : subparts)
    {
        std::string v, uv, n, t;

        size_t ret = sp.ExportObj(&v, &uv, &n, &t, v_start_idx, write_group);

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

void EmgFile::Decompile(TiXmlNode *root, uint16_t id) const
{
    TiXmlElement *entry_root = new TiXmlElement("EMG");
    entry_root->SetAttribute("id", Utils::UnsignedToString(id, true));

    Utils::WriteComment(entry_root, meta_name);

    if (!IsEmpty())
    {
        Utils::WriteParamUnsigned(entry_root, "U_04", unk_04, true);

        for (size_t i = 0; i < subparts.size(); i++)
        {
            subparts[i].Decompile(entry_root, i);
        }
    }
    else
    {
        Utils::WriteComment(entry_root, "Empty part. This is normal.");
    }

    root->LinkEndChild(entry_root);
}

bool EmgFile::Compile(const TiXmlElement *root, SkeletonFile *skl)
{
    unsigned int unk_04;

    subparts.clear();

    if (!Utils::ReadParamUnsigned(root, "U_04", &unk_04))
    {
        // empty Part, return true
        return true;
    }

    if (unk_04 > 0xFFFF)
    {
        DPRINTF("%s: U_04 must be a 16 bits value.\n", FUNCNAME);
        return false;
    }

    this->unk_04 = unk_04;

    size_t count = Utils::GetElemCount(root, "SubPart");
    if (count > 0)
    {
        std::vector<bool> initialized;

        subparts.resize(count);
        initialized.resize(count);

        for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
        {
            if (elem->ValueStr() == "SubPart")
            {
                uint32_t id;

                if (!Utils::ReadAttrUnsigned(elem, "id", &id))
                {
                    DPRINTF("%s: Cannot read attribute \"id\"\n", FUNCNAME);
                    return false;
                }

                if (id >= subparts.size())
                {
                    DPRINTF("%s: SubPart id 0x%x out of range.\n", FUNCNAME, id);
                    return false;
                }

                if (initialized[id])
                {
                    DPRINTF("%s: SubPart id 0x%x was already specified.\n", FUNCNAME, id);
                    return false;
                }

                if (!subparts[id].Compile(elem, skl))
                {
                    DPRINTF("%s: compilation of SubPart failed.\n", FUNCNAME);
                    return false;
                }

                char sp_meta_name[2048];
                snprintf(sp_meta_name, sizeof(sp_meta_name), "%s_%04x", meta_name.c_str(), id);

                subparts[id].meta_name = sp_meta_name;
                initialized[id] = true;
            }
        }
    }

    return true;
}

#ifdef FBX_SUPPORT

bool EmgFile::ExportFbx(const SkeletonFile &skl, const std::vector<FbxNode *> &fbx_bones, FbxScene *scene) const
{
    for (const SubPart &sp : subparts)
    {
        if (!sp.ExportFbx(skl, fbx_bones, scene))
            return false;
    }

    return true;
}

bool EmgFile::InjectFbx(SkeletonFile &skl, FbxScene *scene, bool use_fbx_tangent)
{
    for (SubPart &sp : subparts)
    {
        if (!sp.InjectFbx(skl, scene, use_fbx_tangent))
            return false;
    }

    for (size_t i = 0; i < subparts.size(); i++)
    {
        if (subparts[i].submeshes.size() == 0)
        {
            DPRINTF("Subpart \"%s\" has become empty. We will delete it.\n", subparts[i].meta_name.c_str());
            subparts.erase(subparts.begin()+i);
            i--;
        }
    }

    return true;
}

#endif

unsigned int EmgFile::CalculatePartSize()
{
    unsigned int file_size = sizeof(EMGChunkHeader);

    if (this->subparts.size() != 0)
    {
        file_size += (this->subparts.size() - 1) * sizeof(uint32_t);
    }

    for (SubPart &sp : this->subparts)
    {
        if (file_size & 0xF)
            file_size += (0x10 - (file_size & 0xF));

        //DPRINTF("File size in this subpart: %x\n", file_size);

        file_size += sizeof(EMGHeader);
        file_size += sp.textures_lists.size() * sizeof(uint32_t);

        for (TexturesList &tl : sp.textures_lists)
        {
            file_size += sizeof(uint32_t);
            file_size += tl.textures.size() * sizeof(EMGTextureHeader);
        }

        file_size += sp.submeshes.size() * sizeof(uint32_t);

        for (SubMesh &sm : sp.submeshes)
        {
            if (file_size & 0xF)
                file_size += (0x10 - (file_size & 0xF));

            //DPRINTF("File_size in submesh: %x\n", file_size);

            file_size += sizeof(EMGSubMeshHeader);
            file_size += sm.faces.size() * sizeof(uint16_t);
            file_size += sm.linked_bones.size() * sizeof(uint16_t);
        }
    }

    return file_size;
}

unsigned int EmgFile::CalculateVertexSize()
{
    unsigned int file_size = 0;

    for (SubPart &sp : this->subparts)
    {
        if (sp.vertex.size() != 0)
        {
            file_size += sp.vertex.size() * sp.vertex[0].size;
        }
    }

    return file_size;
}

unsigned int EmgFile::CreatePart(uint8_t *buf, SkeletonFile *skl, uint32_t *vertex_start)
{
    EMGChunkHeader *emgc = (EMGChunkHeader *)buf;
    unsigned int file_size = CalculatePartSize();
    uint32_t offset = 0;

    memset(buf, 0, file_size);

    emgc->signature = EMG_SIGNATURE;
    emgc->unk_04 = val16(this->unk_04);
    emgc->subparts_count = val16(this->subparts.size());

    offset = sizeof(EMGChunkHeader);
    if (this->subparts.size() != 0)
    {
        offset += (this->subparts.size() - 1) * sizeof(uint32_t);
    }

    for (size_t i = 0; i < this->subparts.size(); i++)
    {
        if (offset & 0xF)
            offset += (0x10 - (offset & 0xF));

        emgc->offsets[i] = val32(offset);

        EMGHeader *ehdr = (EMGHeader *)GetOffsetPtr(buf, offset, true);
        SubPart &subpart = this->subparts[i];

        assert(subpart.textures_lists.size() < 65536);
        assert(subpart.submeshes.size() < 65536);
        assert(subpart.vertex.size() > 0);

        ehdr->flags = val16(subpart.flags);
        ehdr->unk_02 = val16(subpart.unk_02);
        ehdr->textures_lists_count = val16(subpart.textures_lists.size());
        ehdr->unk_06 = val16(subpart.unk_06);
        ehdr->unk_08 = val32(subpart.unk_08);

        ehdr->vertex_count = val16(subpart.vertex.size());
        ehdr->vertex_size = val16(subpart.vertex[0].size);
        ehdr->strips = val16(subpart.strips);
        ehdr->submesh_count = val16(subpart.submeshes.size());
        ehdr->vertex_offset = val32(*vertex_start - offset);
        *vertex_start += (subpart.vertex.size() * subpart.vertex[0].size);

        for (int j = 0; j < 12; j++)
        {
            copy_float(&ehdr->vectors[j], subpart.vectors[j]);
        }

        offset += sizeof(EMGHeader);
        ehdr->textures_lists_offset = val32(Utils::DifPointer(buf + offset, ehdr));

        uint32_t *ttable = (uint32_t *)GetOffsetPtr(buf, offset, true);
        offset += subpart.textures_lists.size() * sizeof(uint32_t);

        for (size_t j = 0; j < subpart.textures_lists.size(); j++)
        {
            ttable[j] = val32(Utils::DifPointer(buf + offset, ehdr));

            const TexturesList &textures_list = subpart.textures_lists[j];

            *(uint16_t *)(buf + offset) = val16(textures_list.textures.size());
            offset += sizeof(uint32_t); // yes, uint32_t: uint16_t + an empty uint16_t

            for (size_t k = 0; k < textures_list.textures.size(); k++)
            {
                EMGTextureHeader *thdr = (EMGTextureHeader *)GetOffsetPtr(buf, offset, true);
                const Texture &t = textures_list.textures[k];

                thdr->unk_00 = t.unk_00;
                thdr->emb_index = t.emb_index;
                thdr->unk_02[0] = t.unk_02[0];
                thdr->unk_02[1] = t.unk_02[1];
                copy_float(&thdr->f1, t.f1);
                copy_float(&thdr->f2, t.f2);

                offset += sizeof(EMGTextureHeader);
            }
        }

        ehdr->submesh_list_offset = val32(Utils::DifPointer(buf + offset, ehdr));

        uint32_t *smtable = (uint32_t *)GetOffsetPtr(buf, offset, true);
        offset += subpart.submeshes.size() * sizeof(uint32_t);

        for (size_t j = 0; j < subpart.submeshes.size(); j++)
        {
            if (offset & 0xF)
                offset += (0x10 - (offset & 0xF));

            smtable[j] = val32(Utils::DifPointer(buf + offset, ehdr));

            EMGSubMeshHeader *shdr = (EMGSubMeshHeader *)GetOffsetPtr(buf, offset, true);
            SubMesh &submesh = subpart.submeshes[j];

            assert(submesh.emm_material.length() < 32);
            assert(submesh.faces.size() < 65536);
            assert(submesh.linked_bones.size() < 65536);

            copy_float(&shdr->vector[0], submesh.vector[0]);
            copy_float(&shdr->vector[1], submesh.vector[1]);
            copy_float(&shdr->vector[2], submesh.vector[2]);
            copy_float(&shdr->vector[3], submesh.vector[3]);

            shdr->tl_index = val16(submesh.tl_index);
            shdr->face_count = val16(submesh.faces.size());
            shdr->linked_bones_count = val16(submesh.linked_bones.size());
            strcpy(shdr->emm_material, submesh.emm_material.c_str());

            offset += sizeof(EMGSubMeshHeader);
            uint16_t *faces = (uint16_t *)GetOffsetPtr(buf, offset, true);

            for (size_t k = 0; k < submesh.faces.size(); k++)
            {
                faces[k] = val16(submesh.faces[k]);
                offset += sizeof(uint16_t);
            }

            uint16_t *bones_idx = (uint16_t *)GetOffsetPtr(buf, offset, true);

            for (size_t k = 0; k < submesh.linked_bones.size(); k++)
            {
                uint16_t idx = skl->BoneToIndex(submesh.linked_bones[k]);
                assert(idx != 0xFFFF);

                bones_idx[k] = val16(idx);
                offset += sizeof(uint16_t);
            }
        }
    }

    //DPRINTF("offset = %x, file_size = %x\n", offset, file_size);

    assert(offset == file_size);
    return file_size;
}

unsigned int EmgFile::CreateVertex(uint8_t *buf)
{
    unsigned int file_size = CalculateVertexSize();
    uint32_t offset = 0;

    memset(buf, 0, file_size);

    for (SubPart &sp : this->subparts)
    {
        if (sp.vertex.size() != 0)
        {
            int vertex_size = sp.vertex[0].size;

            assert(vertex_size == sizeof(Vertex52) || vertex_size == sizeof(Vertex64));

            for (VertexData &vd : sp.vertex)
            {
                VertexCommon *vci;

                if (vertex_size == sizeof(Vertex52))
                {
                    Vertex52 *v52 = (Vertex52 *)GetOffsetPtr(buf, offset, true);

                    v52->color = val32(vd.VertexUnion.vertex52.color);

                    if (!big_endian)
                    {
                        v52->blend[0] = vd.VertexUnion.vertex52.blend[0];
                        v52->blend[1] = vd.VertexUnion.vertex52.blend[1];
                        v52->blend[2] = vd.VertexUnion.vertex52.blend[2];
                        v52->blend[3] = vd.VertexUnion.vertex52.blend[3];
                    }
                    else
                    {
                        v52->blend[0] = vd.VertexUnion.vertex52.blend[3];
                        v52->blend[1] = vd.VertexUnion.vertex52.blend[2];
                        v52->blend[2] = vd.VertexUnion.vertex52.blend[1];
                        v52->blend[3] = vd.VertexUnion.vertex52.blend[0];
                    }

                    copy_float(&v52->blend_weight[0], vd.VertexUnion.vertex52.blend_weight[0]);
                    copy_float(&v52->blend_weight[1], vd.VertexUnion.vertex52.blend_weight[1]);
                    copy_float(&v52->blend_weight[2], vd.VertexUnion.vertex52.blend_weight[2]);

                    vci = &vd.VertexUnion.vertex52.common;
                }
                else
                {
                    Vertex64 *v64 = (Vertex64 *)GetOffsetPtr(buf, offset, true);

                    copy_float(&v64->tex2[0], vd.VertexUnion.vertex64.tex2[0]);
                    copy_float(&v64->tex2[1], vd.VertexUnion.vertex64.tex2[1]);
                    copy_float(&v64->tex2[2], vd.VertexUnion.vertex64.tex2[2]);

                    v64->color = val32(vd.VertexUnion.vertex64.color);

                    if (!big_endian)
                    {
                        v64->blend[0] = vd.VertexUnion.vertex64.blend[0];
                        v64->blend[1] = vd.VertexUnion.vertex64.blend[1];
                        v64->blend[2] = vd.VertexUnion.vertex64.blend[2];
                        v64->blend[3] = vd.VertexUnion.vertex64.blend[3];
                    }
                    else
                    {
                        v64->blend[0] = vd.VertexUnion.vertex64.blend[3];
                        v64->blend[1] = vd.VertexUnion.vertex64.blend[2];
                        v64->blend[2] = vd.VertexUnion.vertex64.blend[1];
                        v64->blend[3] = vd.VertexUnion.vertex64.blend[0];
                    }

                    copy_float(&v64->blend_weight[0], vd.VertexUnion.vertex64.blend_weight[0]);
                    copy_float(&v64->blend_weight[1], vd.VertexUnion.vertex64.blend_weight[1]);
                    copy_float(&v64->blend_weight[2], vd.VertexUnion.vertex64.blend_weight[2]);

                    vci = &vd.VertexUnion.vertex64.common;
                }

                VertexCommon *vco = (VertexCommon *)GetOffsetPtr(buf, offset, true);

                copy_float(&vco->pos.x, vci->pos.x);
                copy_float(&vco->pos.y, vci->pos.y);
                copy_float(&vco->pos.z, vci->pos.z);

                copy_float(&vco->norm.x, vci->norm.x);
                copy_float(&vco->norm.y, vci->norm.y);
                copy_float(&vco->norm.z, vci->norm.z);

                copy_float(&vco->tex.u, vci->tex.u);
                copy_float(&vco->tex.v, vci->tex.v);

                offset += vertex_size;
            }
        }
    }

    assert(offset == file_size);
    return file_size;
}

