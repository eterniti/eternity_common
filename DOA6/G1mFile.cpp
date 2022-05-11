#include "G1mFile.h"
#include "FileStream.h"
#include "TransformMatrix.h"
#include "debug.h"

#include "math/Vector.h"

bool G1MFChunk::Read(FixedMemoryStream &in, uint32_t chunk_version, uint32_t)
{
    size_t size;

    version = Utils::GetShortVersion(chunk_version);

    if (version == 21)
    {
        size = 0x90;
    }
    else if (version == 23)
    {
        size = 0xCC;
    }
    else if (version == 24)
    {
        size = 0x104;
    }
    else if (version == 25)
    {
        size = 0x10C;
    }
    else if (version == 26)
    {
        size = 0x114;
    }
    else if (version == 28)
    {
        size = 0x12C;
    }
    else if (version == 29)
    {
        size = 0x130;
    }
    else if (version == 30)
    {
        size = 0x134;
    }
    else if (version == 31)
    {
        size = 0x148;
    }
    else if (version == 33)
    {
        size = sizeof(G1MFData);
    }
    else
    {
        DPRINTF("Unsupported G1MF version: %08x (%d)\n", chunk_version, (version));
        return false;
    }    

    if (!in.Read(&data, size))
        return false;

    return true;
}

bool G1MFChunk::Write(MemoryStream &out) const
{
    size_t size;

    if (version == 21)
    {
        size = 0x90;
    }
    else if (version == 23)
    {
        size = 0xCC;
    }
    else if (version == 24)
    {
        size = 0x104;
    }
    else if (version == 25)
    {
        size = 0x10C;
    }
    else if (version == 26)
    {
        size = 0x114;
    }
    else if (version == 28)
    {
        size = 0x12C;
    }
    else if (version == 29)
    {
        size = 0x130;
    }
    else if (version == 30)
    {
        size = 0x134;
    }
    else if (version == 31)
    {
        size = 0x148;
    }
    else if (version == 33)
    {
        size = sizeof(G1MFData);
    }
    else
    {
        return false;
    }

    return out.Write(&data, size);
}

TiXmlElement *G1MFChunk::Decompile(TiXmlNode *root) const
{   
    TiXmlElement *entry_root = new TiXmlElement("G1MF");
    entry_root->SetAttribute("version", version);
    entry_root->SetAttribute("auto", "true");

    Utils::WriteParamUnsigned(entry_root, "U_0C", data.unk_0C);
    Utils::WriteParamUnsigned(entry_root, "num_bones", data.num_bones);
    Utils::WriteParamUnsigned(entry_root, "U_14", data.unk_14);
    Utils::WriteParamUnsigned(entry_root, "num_matrix", data.num_matrix);
    Utils::WriteParamUnsigned(entry_root, "U_1C", data.unk_1C);
    Utils::WriteParamUnsigned(entry_root, "num_materials", data.num_materials);
    Utils::WriteParamUnsigned(entry_root, "num_material_attributes", data.num_material_attributes);
    Utils::WriteParamUnsigned(entry_root, "num_attributes", data.num_attributes);
    Utils::WriteParamUnsigned(entry_root, "U_2C", data.unk_2C);
    Utils::WriteParamUnsigned(entry_root, "U_30", data.unk_30);
    Utils::WriteParamUnsigned(entry_root, "num_vb", data.num_vb);
    Utils::WriteParamUnsigned(entry_root, "num_layouts", data.num_layouts);
    Utils::WriteParamUnsigned(entry_root, "num_layout_refs", data.num_layout_refs);
    Utils::WriteParamUnsigned(entry_root, "num_bone_maps", data.num_bone_maps);
    Utils::WriteParamUnsigned(entry_root, "num_individual_bone_maps", data.num_individual_bone_maps);
    Utils::WriteParamUnsigned(entry_root, "num_non_shared_vb", data.num_non_shared_vb);
    Utils::WriteParamUnsigned(entry_root, "num_submeshes", data.num_submeshes);
    Utils::WriteParamUnsigned(entry_root, "num_submeshes2", data.num_submeshes2);
    Utils::WriteParamUnsigned(entry_root, "U_54", data.unk_54);
    Utils::WriteParamUnsigned(entry_root, "num_meshes", data.num_meshes);
    Utils::WriteParamUnsigned(entry_root, "num_submeshes_in_meshes", data.num_submeshes_in_meshes);
    Utils::WriteParamUnsigned(entry_root, "U_60", data.unk_60);
    Utils::WriteParamUnsigned(entry_root, "U_64", data.unk_64);
    Utils::WriteParamUnsigned(entry_root, "num_nuno2s", data.num_nuno2s);
    Utils::WriteParamUnsigned(entry_root, "U_6C", data.unk_6C);
    Utils::WriteParamUnsigned(entry_root, "num_nuno2s_unk11", data.num_nuno2s_unk11);
    Utils::WriteParamUnsigned(entry_root, "num_nuno1s", data.num_nuno1s);
    Utils::WriteParamUnsigned(entry_root, "num_nuno1s_unk4", data.num_nuno1s_unk4);
    Utils::WriteParamUnsigned(entry_root, "num_nuno1s_control_points", data.num_nuno1s_control_points);
    Utils::WriteParamUnsigned(entry_root, "num_nuno1s_unk1", data.num_nuno1s_unk1);
    Utils::WriteParamUnsigned(entry_root, "num_nuno1s_unk2_and_unk3", data.num_nuno1s_unk2_and_unk3);
    Utils::WriteParamUnsigned(entry_root, "bones_id_size", data.bones_id_size);
    Utils::WriteParamUnsigned(entry_root, "U_8C", data.unk_8C);

    if (version > 21)
    {
        Utils::WriteParamUnsigned(entry_root, "num_nunv1s", data.num_nunv1s);
        Utils::WriteParamUnsigned(entry_root, "num_nunv1s_unk4", data.num_nunv1s_unk4);
        Utils::WriteParamUnsigned(entry_root, "num_nunv1s_control_points", data.num_nunv1s_control_points);
        Utils::WriteParamUnsigned(entry_root, "num_nunv1s_unk1", data.num_nunv1s_unk1);
        Utils::WriteParamUnsigned(entry_root, "U_A0", data.unk_A0);
        Utils::WriteParamUnsigned(entry_root, "U_A4", data.unk_A4);
        Utils::WriteParamUnsigned(entry_root, "U_A8", data.unk_A8);
        Utils::WriteParamUnsigned(entry_root, "U_AC", data.unk_AC);
        Utils::WriteParamUnsigned(entry_root, "U_B0", data.unk_B0);
        Utils::WriteParamUnsigned(entry_root, "U_B4", data.unk_B4);
        Utils::WriteParamUnsigned(entry_root, "U_B8", data.unk_B8);
        Utils::WriteParamUnsigned(entry_root, "U_BC", data.unk_BC);
        Utils::WriteParamUnsigned(entry_root, "U_C0", data.unk_C0);
        Utils::WriteParamUnsigned(entry_root, "U_C4", data.unk_C4);
        Utils::WriteParamUnsigned(entry_root, "U_C8", data.unk_C8);

        if (version >= 24)
        {
            Utils::WriteParamUnsigned(entry_root, "U_CC", data.unk_CC);
            Utils::WriteParamUnsigned(entry_root, "U_D0", data.unk_D0);
            Utils::WriteParamUnsigned(entry_root, "U_D4", data.unk_D4);
            Utils::WriteParamUnsigned(entry_root, "U_D8", data.unk_D8);
            Utils::WriteParamUnsigned(entry_root, "U_DC", data.unk_DC);
            Utils::WriteParamUnsigned(entry_root, "U_E0", data.unk_E0);
            Utils::WriteParamUnsigned(entry_root, "U_E4", data.unk_E4);
            Utils::WriteParamUnsigned(entry_root, "U_E8", data.unk_E8);
            Utils::WriteParamUnsigned(entry_root, "U_EC", data.unk_EC);
            Utils::WriteParamUnsigned(entry_root, "num_nuno3s", data.num_nuno3s);
            Utils::WriteParamUnsigned(entry_root, "num_nuno3s_unk4", data.num_nuno3s_unk4);
            Utils::WriteParamUnsigned(entry_root, "num_nuno3s_control_points", data.num_nuno3s_control_points);
            Utils::WriteParamUnsigned(entry_root, "num_nuno3s_unk1", data.num_nuno3s_unk1);
            Utils::WriteParamUnsigned(entry_root, "U_100", data.unk_100);

            if (version >= 25)
            {
                Utils::WriteParamUnsigned(entry_root, "U_104", data.unk_104);
                Utils::WriteParamUnsigned(entry_root, "U_108", data.unk_108);

                if (version >= 26)
                {
                    Utils::WriteParamUnsigned(entry_root, "U_10C", data.unk_10C);
                    Utils::WriteParamUnsigned(entry_root, "U_110", data.unk_110);

                    if (version >= 27)
                    {
                        Utils::WriteParamUnsigned(entry_root, "num_nuno4s", data.num_nuno4s);
                        Utils::WriteParamUnsigned(entry_root, "num_nuno4s_unk7", data.num_nuno4s_unk7);
                        Utils::WriteParamUnsigned(entry_root, "num_nuno4s_unk8", data.num_nuno4s_unk8);
                        Utils::WriteParamUnsigned(entry_root, "num_nuno4s_unk9", data.num_nuno4s_unk9);
                        Utils::WriteParamUnsigned(entry_root, "num_nuno4s_unk10", data.num_nuno4s_unk10);
                        Utils::WriteParamUnsigned(entry_root, "U_128", data.unk_128);

                        if (version >= 29)
                        {
                            Utils::WriteParamUnsigned(entry_root, "U_12C", data.unk_12C);
                        }
                    }

                    if (version >= 30)
                    {
                        size_t size = sizeof(G1MFData);

                        if (version == 30)
                        {
                            size = 0x134;
                        }
                        else if (version == 31)
                        {
                            size = 0x148;
                        }

                        size = (size - 0x130) / 4;

                        Utils::WriteParamMultipleUnsigned(entry_root, "MU_130", std::vector<uint32_t>(data.unk_130, data.unk_130+size));
                    }
                }
            }
        }
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MFChunk::Compile(const TiXmlElement *root, bool *g1mf_auto)
{
    data.signature = G1MF_SIGNATURE;

    if (!Utils::ReadAttrUnsigned(root, "version", &version))
    {
        DPRINTF("\"version\" attribute was ommited (in object at line %d).\n", root->Row());
        return false;
    }

    data.version = Utils::GetLongVersion(version);

    std::string auto_value;

    if (!Utils::ReadAttrString(root, "auto", auto_value))
        auto_value = "true";

    *g1mf_auto = Utils::GetBoolean(auto_value);

    if (!Utils::GetParamUnsigned(root, "U_0C", &data.unk_0C)) return false;
    if (!Utils::GetParamUnsigned(root, "num_bones", &data.num_bones)) return false;
    if (!Utils::GetParamUnsigned(root, "U_14", &data.unk_14)) return false;
    if (!Utils::GetParamUnsigned(root, "num_matrix", &data.num_matrix)) return false;
    if (!Utils::GetParamUnsigned(root, "U_1C", &data.unk_1C)) return false;
    if (!Utils::GetParamUnsigned(root, "num_materials", &data.num_materials)) return false;
    if (!Utils::GetParamUnsigned(root, "num_material_attributes", &data.num_material_attributes)) return false;
    if (!Utils::GetParamUnsigned(root, "num_attributes", &data.num_attributes)) return false;
    if (!Utils::GetParamUnsigned(root, "U_2C", &data.unk_2C)) return false;
    if (!Utils::GetParamUnsigned(root, "U_30", &data.unk_30)) return false;
    if (!Utils::GetParamUnsigned(root, "num_vb", &data.num_vb)) return false;
    if (!Utils::GetParamUnsigned(root, "num_layouts", &data.num_layouts)) return false;
    if (!Utils::GetParamUnsigned(root, "num_layout_refs", &data.num_layout_refs)) return false;
    if (!Utils::GetParamUnsigned(root, "num_bone_maps", &data.num_bone_maps)) return false;
    if (!Utils::GetParamUnsigned(root, "num_individual_bone_maps", &data.num_individual_bone_maps)) return false;
    if (!Utils::GetParamUnsigned(root, "num_non_shared_vb", &data.num_non_shared_vb)) return false;
    if (!Utils::GetParamUnsigned(root, "num_submeshes", &data.num_submeshes)) return false;
    if (!Utils::GetParamUnsigned(root, "num_submeshes2", &data.num_submeshes2)) return false;
    if (!Utils::GetParamUnsigned(root, "U_54", &data.unk_54)) return false;
    if (!Utils::GetParamUnsigned(root, "num_meshes", &data.num_meshes)) return false;
    if (!Utils::GetParamUnsigned(root, "num_submeshes_in_meshes", &data.num_submeshes_in_meshes)) return false;
    if (!Utils::GetParamUnsigned(root, "U_60", &data.unk_60)) return false;
    if (!Utils::GetParamUnsigned(root, "U_64", &data.unk_64)) return false;
    if (!Utils::GetParamUnsigned(root, "U_60", &data.unk_60)) return false;
    if (!Utils::GetParamUnsigned(root, "U_64", &data.unk_64)) return false;
    if (!Utils::GetParamUnsigned(root, "num_nuno2s", &data.num_nuno2s)) return false;
    if (!Utils::GetParamUnsigned(root, "U_6C", &data.unk_6C)) return false;
    if (!Utils::GetParamUnsigned(root, "num_nuno2s_unk11", &data.num_nuno2s_unk11)) return false;
    if (!Utils::GetParamUnsigned(root, "num_nuno1s", &data.num_nuno1s)) return false;
    if (!Utils::GetParamUnsigned(root, "num_nuno1s_unk4", &data.num_nuno1s_unk4)) return false;
    if (!Utils::GetParamUnsigned(root, "num_nuno1s_control_points", &data.num_nuno1s_control_points)) return false;
    if (!Utils::GetParamUnsigned(root, "num_nuno1s_unk1", &data.num_nuno1s_unk1)) return false;
    if (!Utils::GetParamUnsigned(root, "num_nuno1s_unk2_and_unk3", &data.num_nuno1s_unk2_and_unk3)) return false;
    if (!Utils::GetParamUnsigned(root, "bones_id_size", &data.bones_id_size)) return false;
    if (!Utils::GetParamUnsigned(root, "U_8C", &data.unk_8C)) return false;

    if (version > 21)
    {
        if (!Utils::GetParamUnsigned(root, "num_nunv1s", &data.num_nunv1s)) return false;
        if (!Utils::GetParamUnsigned(root, "num_nunv1s_unk4", &data.num_nunv1s_unk4)) return false;
        if (!Utils::GetParamUnsigned(root, "num_nunv1s_control_points", &data.num_nunv1s_control_points)) return false;
        if (!Utils::GetParamUnsigned(root, "num_nunv1s_unk1", &data.num_nunv1s_unk1)) return false;
        if (!Utils::GetParamUnsigned(root, "U_A0", &data.unk_A0)) return false;
        if (!Utils::GetParamUnsigned(root, "U_A4", &data.unk_A4)) return false;
        if (!Utils::GetParamUnsigned(root, "U_A8", &data.unk_A8)) return false;
        if (!Utils::GetParamUnsigned(root, "U_AC", &data.unk_AC)) return false;
        if (!Utils::GetParamUnsigned(root, "U_B0", &data.unk_B0)) return false;
        if (!Utils::GetParamUnsigned(root, "U_B4", &data.unk_B4)) return false;
        if (!Utils::GetParamUnsigned(root, "U_B8", &data.unk_B8)) return false;
        if (!Utils::GetParamUnsigned(root, "U_BC", &data.unk_BC)) return false;
        if (!Utils::GetParamUnsigned(root, "U_C0", &data.unk_C0)) return false;
        if (!Utils::GetParamUnsigned(root, "U_C4", &data.unk_C4)) return false;
        if (!Utils::GetParamUnsigned(root, "U_C8", &data.unk_C8)) return false;

        if (version >= 24)
        {
            if (!Utils::GetParamUnsigned(root, "U_CC", &data.unk_CC)) return false;
            if (!Utils::GetParamUnsigned(root, "U_D0", &data.unk_D0)) return false;
            if (!Utils::GetParamUnsigned(root, "U_D4", &data.unk_D4)) return false;
            if (!Utils::GetParamUnsigned(root, "U_D8", &data.unk_D8)) return false;
            if (!Utils::GetParamUnsigned(root, "U_DC", &data.unk_DC)) return false;
            if (!Utils::GetParamUnsigned(root, "U_E0", &data.unk_E0)) return false;
            if (!Utils::GetParamUnsigned(root, "U_E4", &data.unk_E4)) return false;
            if (!Utils::GetParamUnsigned(root, "U_E8", &data.unk_E8)) return false;
            if (!Utils::GetParamUnsigned(root, "U_EC", &data.unk_EC)) return false;
            if (!Utils::GetParamUnsigned(root, "num_nuno3s", &data.num_nuno3s)) return false;
            if (!Utils::GetParamUnsigned(root, "num_nuno3s_unk4", &data.num_nuno3s_unk4)) return false;
            if (!Utils::GetParamUnsigned(root, "num_nuno3s_control_points", &data.num_nuno3s_control_points)) return false;
            if (!Utils::GetParamUnsigned(root, "num_nuno3s_unk1", &data.num_nuno3s_unk1)) return false;
            if (!Utils::GetParamUnsigned(root, "U_100", &data.unk_100)) return false;

            if (version >= 25)
            {
                if (!Utils::GetParamUnsigned(root, "U_104", &data.unk_104)) return false;
                if (!Utils::GetParamUnsigned(root, "U_108", &data.unk_108)) return false;

                if (version >= 26)
                {
                    if (!Utils::GetParamUnsigned(root, "U_10C", &data.unk_10C)) return false;
                    if (!Utils::GetParamUnsigned(root, "U_110", &data.unk_110)) return false;

                    if (version >= 27)
                    {
                        if (!Utils::GetParamUnsigned(root, "num_nuno4s", &data.num_nuno4s)) return false;
                        if (!Utils::GetParamUnsigned(root, "num_nuno4s_unk7", &data.num_nuno4s_unk7)) return false;
                        if (!Utils::GetParamUnsigned(root, "num_nuno4s_unk8", &data.num_nuno4s_unk8)) return false;
                        if (!Utils::GetParamUnsigned(root, "num_nuno4s_unk9", &data.num_nuno4s_unk9)) return false;
                        if (!Utils::GetParamUnsigned(root, "num_nuno4s_unk10", &data.num_nuno4s_unk10)) return false;
                        if (!Utils::GetParamUnsigned(root, "U_128", &data.unk_128)) return false;

                        if (version >= 29)
                        {
                            if (!Utils::GetParamUnsigned(root, "U_12C", &data.unk_12C)) return false;
                        }
                    }
                }
            }
        }
    }

    size_t size;

    if (version == 21)
    {
        size = 0x90;
    }
    else if (version == 23)
    {
        size = 0xCC;
    }
    else if (version == 24)
    {
        size = 0x104;
    }
    else if (version == 25)
    {
        size = 0x10C;
    }
    else if (version == 26)
    {
        size = 0x114;
    }
    else if (version == 28)
    {
        size = 0x12C;
    }
    else if (version == 29)
    {
        size = 0x130;
    }
    else if (version == 30)
    {
        size = 0x134;
    }
    else if (version == 31)
    {
        size = 0x148;
    }
    else if (version == 33)
    {
        size = sizeof(G1MFData);
    }
    else
    {
        DPRINTF("Unsupported G1MF version %d\n", version);
        return false;
    }

    data.size = (uint32_t)size;

    if (version >= 30)
    {
        size = (size - 0x130) / 4;
        if (!Utils::GetParamMultipleUnsigned(root, "MU_130", data.unk_130, size)) return false;
    }

    return true;
}

int G1MSChunk::IndexToID(uint16_t idx) const
{
    for (size_t i = 0; i < indices.size(); i++)
    {
        if (indices[i] == idx)
            return (int)i;
    }

    return -1;
}

uint16_t G1MSChunk::NameToIndex(const std::string &name) const
{
    std::string lc_name = Utils::ToLowerCase(name);

    for (size_t i = 0; i < bone_names.size(); i++)
    {
        if (Utils::ToLowerCase(bone_names[i]) == lc_name)
            return (uint16_t)i;
    }

    if (Utils::BeginsWith(name, "UnnamedBone#", false))
    {
        uint16_t ret = (uint16_t)Utils::GetUnsigned(name.substr(12), 0xFFFF);
        if (ret < bones.size())
            return ret;
    }

    return 0xFFFF;
}

bool G1MSChunk::Read(FixedMemoryStream &in, uint32_t chunk_version, uint32_t)
{
    version = Utils::GetShortVersion(chunk_version);

    if (version != 32)
    {
        DPRINTF("G1MS: cannot understand this version of chunk: 0x%08X (%d)\n", chunk_version, version);
        return false;
    }

    G1MSChunkHeader *hdr;
    off64_t chunk_start = (off64_t)in.Tell();

    if (!in.FastRead((uint8_t **)&hdr, sizeof(G1MSChunkHeader)))
        return false;

    unk_10 = hdr->unk_10;

    indices.resize(hdr->num_indices);

    if (indices.size() > 0 && !in.Read(indices.data(), indices.size()*sizeof(uint16_t)))
        return false;

    final_bones.resize(hdr->num_parents);

    if (final_bones.size() > 0 && !in.Read(final_bones.data(), final_bones.size()*sizeof(uint16_t)))
        return false;

    // From here to bones start there may be padding to align to 4

    if (!in.Seek(chunk_start + hdr->bones_offset, SEEK_SET))
        return false;

    bones.resize(hdr->num_bones);

    if (bones.size() > 0 && !in.Read(bones.data(), bones.size()*sizeof(G1MSBone)))
        return false;

    return true;
}

bool G1MSChunk::Write(MemoryStream &out) const
{
    G1MSChunkHeader hdr;

    uint64_t start = out.Tell();

    memset(&hdr, 0, sizeof(hdr));
    hdr.signature = G1MS_SIGNATURE;
    hdr.version = Utils::GetLongVersion(version);

    if (!out.Write(&hdr, sizeof(G1MSChunkHeader)))
        return false;

    hdr.num_indices = (uint16_t)indices.size();
    if (indices.size() > 0 && !out.Write(indices.data(), indices.size()*sizeof(uint16_t)))
        return false;

    hdr.num_parents = (uint16_t)final_bones.size();
    if (final_bones.size() > 0 && !out.Write(final_bones.data(), final_bones.size()*sizeof(uint16_t)))
        return false;

    while (out.GetSize() & 3)
        out.Write8(0);

    hdr.bones_offset = (uint32_t)(out.Tell() - start);
    hdr.num_bones = (uint16_t)bones.size();

    if (bones.size() > 0 && !out.Write(bones.data(), bones.size()*sizeof(G1MSBone)))
        return false;

    uint64_t end = out.Tell();

    hdr.chunk_size = (uint32_t)(end-start);
    hdr.unk_10 = unk_10;

    if (!out.Seek((off64_t)start, SEEK_SET))
        return false;

    if (!out.Write(&hdr, sizeof(G1MSChunkHeader)))
        return false;

    return out.Seek((off64_t)end, SEEK_SET);
}

TiXmlElement *G1MSChunk::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("G1MS");

    entry_root->SetAttribute("version", version);
    entry_root->SetAttribute("id_size", (int)indices.size());
    entry_root->SetAttribute("u_10", unk_10);

    if (bone_names.size() != bones.size())
    {
        DPRINTF("G1MSChunk::Decompile called before bones names set");
        throw std::runtime_error("G1MSChunk::Decompile called before bones names set.\n");
        return nullptr;
    }

    std::vector<std::string> final_bones_str;

    for (uint16_t b : final_bones)
    {
        if ((size_t)b >= bone_names.size())
        {
            DPRINTF("%s: Bone %d in final_bones is over names array.\n", FUNCNAME, b);
            return nullptr;
        }

        final_bones_str.push_back(bone_names[b]);
    }

    Utils::WriteParamMultipleStrings(entry_root, "final_bones", final_bones_str);

    for (size_t i = 0; i < bones.size(); i++)
    {
        TiXmlElement *bone_root = new TiXmlElement("Bone");

        bone_root->SetAttribute("name", bone_names[i]);
        bone_root->SetAttribute("id", IndexToID((uint16_t)i));
        bone_root->SetAttribute("idx", (int)i);

        if (bones[i].parent != 0xFFFF)
        {
            if (bones[i].parent >= bone_names.size())
            {
                bone_root->SetAttribute("parent", bones[i].parent);
            }
            else
            {
                bone_root->SetAttribute("parent", bone_names[bones[i].parent]);
            }
        }

        if (bones[i].flags != 0xFFFF && bones[i].flags != 0)
        {
             bone_root->SetAttribute("flags", Utils::UnsignedToString(bones[i].flags, true));
        }

        Utils::WriteParamMultipleFloats(bone_root, "Translation", std::vector<float>(bones[i].position, bones[i].position+4));
        Utils::WriteParamMultipleFloats(bone_root, "RotationQ", std::vector<float>(bones[i].rotation, bones[i].rotation+4));
        Utils::WriteParamMultipleFloats(bone_root, "Scale", std::vector<float>(bones[i].scale, bones[i].scale+3));

        entry_root->LinkEndChild(bone_root);
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MSChunk::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "version", &version))
    {
        version = 32;
    }

     if (version != 32)
    {
        DPRINTF("G1MS: cannot understand this version of chunk (%d) (object at line %d)\n", version, root->Row());
        return false;
    }

    uint32_t id_size;

    if (!Utils::ReadAttrUnsigned(root, "id_size", &id_size))
        id_size = 0;

    indices.clear();
    indices.resize(id_size, 0xFFFF);

    if (!Utils::ReadAttrUnsigned(root, "u_10", &unk_10))
    {
        DPRINTF("Missing \"u_10\" (object at line %d)", root->Row());
        return false;
    }

    bones.clear();
    bone_names.clear();

    size_t count = Utils::GetElemCount(root, "Bone");

    bones.resize(count);
    bone_names.resize(count);

    std::unordered_set<std::string> bones_dic;
    std::unordered_set<uint32_t> id_dic;
    std::vector<bool> defined;

    defined.resize(count, false);

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Bone")
        {
           G1MSBone bone;
           std::string name;
           uint32_t id;
           uint16_t idx;
           uint16_t flags;

           if (!Utils::ReadAttrString(elem, "name", name))
           {
               DPRINTF("Missing attribute \"name\" (object at line %d)\n", elem->Row());
               return false;
           }

           if (bones_dic.find(name) != bones_dic.end())
           {
               DPRINTF("Name \"%s\" was used in other bone. (duplication in object at line %d)\n", name.c_str(), elem->Row());
               return false;
           }

           bones_dic.insert(name);

           if (!Utils::ReadAttrUnsigned(elem, "idx", &idx))
           {
               DPRINTF("Missing attribute \"id\" (object at line %d)\n", elem->Row());
               return false;
           }

           if ((size_t)idx >= bones.size())
           {
               DPRINTF("\"idx\" %d is out of bounds (there are %Id bones) (object at line %d)\n", idx, bones.size(), elem->Row());
               return false;
           }

           if (defined[idx])
           {
               DPRINTF("\"idx\" %d was already used in other bone (duplication in object at line %d)\n", idx, elem->Row());
               return false;
           }

           defined[idx] = true;
           bone_names[idx] = name;

           if (!Utils::ReadAttrUnsigned(elem, "id", &id))
           {
               DPRINTF("Missing attribute \"id\" (object at line %d)\n", elem->Row());
               return false;
           }

           if (id_dic.find(id) != id_dic.end())
           {
               DPRINTF("id \"%d\" was used in other bone. (duplication in object at line %d)\n", id, elem->Row());
               return false;
           }

           id_dic.insert(id);

           if (id >= indices.size())
               indices.resize(id+1, 0xFFFF);

           indices[id] = idx;

           flags = 0xFFFF;
           Utils::ReadAttrUnsigned(elem, "flags", &flags);

           memset(&bone, 0, sizeof(bone));
           bone.parent = 0xFFFF;
           bone.flags = flags;

           if (!Utils::GetParamMultipleFloats(elem, "Translation", bone.position, 4))
               return false;

           if (!Utils::GetParamMultipleFloats(elem, "RotationQ", bone.rotation, 4))
               return false;

           if (!Utils::GetParamMultipleFloats(elem, "Scale", bone.scale, 3))
               return false;

           bones[idx] = bone;
        }
    }

    // Resolve parents now
    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Bone")
        {
            std::string parent;
            uint16_t idx;

            Utils::ReadAttrUnsigned(elem, "idx", &idx); // Since this was succesfully read in first pass, it shouldn't give error.

            if (Utils::ReadAttrString(elem, "parent", parent))
            {
                uint16_t parent_idx = NameToIndex(parent);
                if (parent_idx == 0xFFFF)
                {
                    if (Utils::HasOnlyDigits(parent) || Utils::BeginsWith(parent, "0x", false))
                    {
                        parent_idx = (uint16_t)Utils::GetUnsigned(parent);
                    }
                    else
                    {
                        DPRINTF("Cannot find parent bone \"%s\" (object at line %d)\n", parent.c_str(), elem->Row());
                        return false;
                    }
                }

                bones[idx].parent = parent_idx;

                if (bones[idx].flags == 0xFFFF)
                    bones[idx].flags = 0;
            }
        }
    }

    std::vector<std::string> final_bones_str;

    if (!Utils::GetParamMultipleStrings(root, "final_bones", final_bones_str))
        return false;

    final_bones.clear();

    for (const std::string &name : final_bones_str)
    {
        uint16_t idx = NameToIndex(name);
        if (idx == 0xFFFF)
        {
            DPRINTF("Bone \"%s\" referenced in final_bones is not in the bones list.\n", name.c_str());
            return false;
        }

        final_bones.push_back(idx);
    }

    return true;
}

bool G1MMChunk::Read(FixedMemoryStream &in, uint32_t chunk_version, uint32_t)
{
    version = Utils::GetShortVersion(chunk_version);

    if (version != 20)
    {
        DPRINTF("G1MM: cannot understand this version of chunk: 0x%08X (%d))\n", chunk_version, version);
        return false;
    }

    if (!in.Seek(0xC, SEEK_CUR))
        return false;

    uint32_t num_matrices;

    if (!in.Read32(&num_matrices))
        return false;

    //DPRINTF("Num matrices: %x\n", num_matrices);

    matrices.resize(num_matrices);

    if (num_matrices > 0 && !in.Read(matrices.data(), num_matrices*sizeof(G1MMMatrix)))
        return false;

    return true;
}

bool G1MMChunk::Write(MemoryStream &out) const
{
    if (!out.Write32(G1MM_SIGNATURE))
        return false;

    if (!out.Write32(Utils::GetLongVersion(version)))
        return false;

    if (!out.Write32((uint32_t)matrices.size()*sizeof(G1MMMatrix) + 0xC + 4))
        return false;

    if (!out.Write32((uint32_t)matrices.size()))
        return false;

    if (matrices.size() > 0)
        return out.Write(matrices.data(), matrices.size()*sizeof(G1MMMatrix));

    return true;
}

void G1MMChunk::DecompileMatrix(TiXmlElement *root, const float *matrix)
{
    std::vector<float> row;
    row.resize(4);

    TiXmlElement *matrix_root = new TiXmlElement("Matrix");

    for (int i = 0; i < 4; i++)
    {
        std::string row_name = "X" + Utils::ToString(i+1);

        row[0] = matrix[i*4];
        row[1] = matrix[(i*4)+1];
        row[2] = matrix[(i*4)+2];
        row[3] = matrix[(i*4)+3];

        Utils::WriteParamMultipleFloats(matrix_root, row_name.c_str(), row);
    }

    root->LinkEndChild(matrix_root);
}

bool G1MMChunk::CompileMatrix(const TiXmlElement *root, float *matrix)
{
    std::vector<float> row;

    for (int i = 0; i < 4; i++)
    {
        std::string row_name = "X" + Utils::ToString(i+1);

        if (!Utils::GetParamMultipleFloats(root, row_name.c_str(), row))
            return false;

        if (row.size() != 4)
        {
            DPRINTF("Invalid size for row \"%s\" (object at line %d)\n", row_name.c_str(), root->Row());
            return false;
        }

        matrix[i*4] = row[0];
        matrix[(i*4)+1] = row[1];
        matrix[(i*4)+2] = row[2];
        matrix[(i*4)+3] = row[3];
    }

    return true;
}

TiXmlElement *G1MMChunk::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("G1MM");

    entry_root->SetAttribute("version", version);

    for (const G1MMMatrix &m: matrices)
        DecompileMatrix(entry_root, m.matrix);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MMChunk::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrUnsigned(root, "version", &version))
        version = 20;

    if (version != 20)
    {
        DPRINTF("G1MM: cannot understand this version of chunk: %d\n", version);
        return false;
    }

    matrices.clear();

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Matrix")
        {
            G1MMMatrix matrix;

            if (!CompileMatrix(elem, matrix.matrix))
                return false;

            matrices.push_back(matrix);
        }
    }

    return true;
}

bool G1MGUnkSection1::Read(FixedMemoryStream &in, uint32_t section_size)
{
    if (!in.Seek(8, SEEK_CUR))
        return false;

    unk.resize(section_size-8);

    if (unk.size() > 0 && !in.Read(unk.data(), unk.size()))
        return false;

    valid = true;
    return true;
}

bool G1MGUnkSection1::Write(MemoryStream &out) const
{
    if (!valid)
        return false;

    if (!out.Write32(0x10001))
        return false;

    if (!out.Write32((uint32_t)unk.size() + 8))
        return false;

    if (unk.size() > 0)
        return out.Write(unk.data(), unk.size());

    return true;
}

TiXmlElement *G1MGUnkSection1::Decompile(TiXmlNode *root) const
{
    if (!valid)
        return nullptr;

    TiXmlElement *entry_root = new TiXmlElement("Section1");
    Utils::WriteComment(entry_root, "Section 1 is currently not parsed. Raw base64 ahead.");
    Utils::WriteParamBlob(entry_root, "data", unk.data(), unk.size());

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MGUnkSection1::Compile(const TiXmlElement *root)
{
    size_t size;
    uint8_t *buf = Utils::GetParamBlob(root, "data", &size);
    if (!buf)
    {
        DPRINTF("Failed in getting \"data\" parameter (in object at line %d)\n", root->Row());
        return false;
    }

    unk = std::vector<uint8_t>(buf, buf+size);
    delete[] buf;

    valid = true;
    return true;
}

bool G1MGMaterial::Read(FixedMemoryStream &in)
{
    G1MGMaterialProperties *prop;

    if (!in.FastRead((uint8_t **)&prop, sizeof(G1MGMaterialProperties)))
        return false;

    index = prop->index;
    unk_08 = prop->unk_08;
    unk_0C = prop->unk_0C;

    textures.resize(prop->num_textures);

    if (textures.size() > 0 && !in.Read(textures.data(), textures.size()*sizeof(G1MGTexture)))
        return false;

    return true;
}

bool G1MGMaterial::Write(MemoryStream &out) const
{
    G1MGMaterialProperties prop;

    prop.index = index;
    prop.num_textures = (uint32_t)textures.size();
    prop.unk_08 = unk_08;
    prop.unk_0C = unk_0C;

    if (!out.Write(&prop, sizeof(G1MGMaterialProperties)))
        return false;

    if (textures.size() > 0 && !out.Write(textures.data(), textures.size()*sizeof(G1MGTexture)))
        return false;

    return true;
}

TiXmlElement *G1MGMaterial::Decompile(TiXmlNode *root, uint32_t idx) const
{
    TiXmlElement *entry_root = new TiXmlElement("Material");
    entry_root->SetAttribute("idx", idx);

    Utils::WriteParamUnsigned(entry_root, "Index", index);
    Utils::WriteParamUnsigned(entry_root, "U_08", unk_08);
    Utils::WriteParamUnsigned(entry_root, "U_0C", unk_0C);

    for (const G1MGTexture &texture : textures)
    {
        TiXmlElement *texture_root = new TiXmlElement("Texture");
        texture_root->SetAttribute("id", texture.tex_id);
        texture_root->SetAttribute("type", texture.tex_type);
        texture_root->SetAttribute("type2", texture.tex_type2);
        texture_root->SetAttribute("u_06", texture.unk_06);
        texture_root->SetAttribute("u_08", texture.unk_08);
        texture_root->SetAttribute("u_0a", texture.unk_0A);

        entry_root->LinkEndChild(texture_root);
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MGMaterial::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "Index", &index))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_08", &unk_08))
        return false;

    if (!Utils::GetParamUnsigned(root, "U_0C", &unk_0C))
        return false;

    textures.clear();

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Texture")
        {
            G1MGTexture texture;

            if (!Utils::ReadAttrUnsigned(elem, "id", &texture.tex_id))
            {
                DPRINTF("Missing attribute \"id\" (in object at line %d)\n", elem->Row());
                return false;
            }

            if (!Utils::ReadAttrUnsigned(elem, "type", &texture.tex_type))
            {
                DPRINTF("Missing attribute \"type\" (in object at line %d)\n", elem->Row());
                return false;
            }

            if (!Utils::ReadAttrUnsigned(elem, "type2", &texture.tex_type2))
            {
                DPRINTF("Missing attribute \"type2\" (in object at line %d)\n", elem->Row());
                return false;
            }

            if (!Utils::ReadAttrUnsigned(elem, "u_06", &texture.unk_06))
            {
                DPRINTF("Missing attribute \"u_06\" (in object at line %d)\n", elem->Row());
                return false;
            }

            if (!Utils::ReadAttrUnsigned(elem, "u_08", &texture.unk_08))
            {
                DPRINTF("Missing attribute \"u_08\" (in object at line %d)\n", elem->Row());
                return false;
            }

            if (!Utils::ReadAttrUnsigned(elem, "u_0a", &texture.unk_0A))
            {
                DPRINTF("Missing attribute \"u_0a\" (in object at line %d)\n", elem->Row());
                return false;
            }

            textures.push_back(texture);
        }
    }

    return true;
}

bool G1MGMaterialSection::Read(FixedMemoryStream &in, uint32_t)
{
    if (!in.Seek(8, SEEK_CUR))
        return false;

    uint32_t num_materials;

    if (!in.Read32(&num_materials))
        return false;

    materials.resize(num_materials);

    for (G1MGMaterial &mat : materials)
    {
        if (!mat.Read(in))
            return false;
    }

    valid = true;
    return true;
}

bool G1MGMaterialSection::Write(MemoryStream &out) const
{
    if (!valid)
        return false;

    uint64_t start = out.Tell();

    if (!out.Write32(0x10002))
        return false;

    if (!out.Write32(0))
        return false;

    if (!out.Write32((uint32_t)materials.size()))
        return false;

    for (const G1MGMaterial &mat : materials)
    {
        if (!mat.Write(out))
            return false;
    }

    uint64_t end = out.Tell();

    if (!out.Seek((off64_t)start + 4, SEEK_SET))
        return false;

    if (!out.Write32((uint32_t)(end-start)))
        return false;

    return out.Seek((off64_t)end, SEEK_SET);
}

TiXmlElement *G1MGMaterialSection::Decompile(TiXmlNode *root) const
{
    if (!valid)
        return nullptr;

    TiXmlElement *entry_root = new TiXmlElement("MaterialsSection");
    for (uint32_t i = 0; i < (uint32_t)materials.size(); i++)
        materials[i].Decompile(entry_root, i);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MGMaterialSection::Compile(const TiXmlElement *root)
{    
    size_t count = Utils::GetElemCount(root, "Material");
    std::vector<bool> defined;

    materials.clear();
    materials.resize(count);
    defined.resize(count, false);

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Material")
        {
            uint32_t idx;

            if (!Utils::ReadAttrUnsigned(elem, "idx", &idx))
            {
                DPRINTF("Cannot get attribute \"idx\" (in object at line %d).\n", elem->Row());
                return false;
            }

            if (idx >= (uint32_t)materials.size())
            {
                DPRINTF("idx %d is out of bounds (there are %Id Material) (in object at line %d).\n", idx, materials.size(), elem->Row());
                return false;
            }

            if (defined[idx])
            {
                DPRINTF("idx %d is duplicated (duplication in object at line %d).\n", idx, elem->Row());
                return false;
            }

            defined[idx] = true;

            if (!materials[idx].Compile(elem))
                return false;
        }
    }

    valid = true;
    return true;
}

bool G1MGAttribute::Read(FixedMemoryStream &in)
{
    G1MGAttributeHeader *hdr;

    uint64_t start = in.Tell();

    if (!in.FastRead((uint8_t **)&hdr, sizeof(G1MGAttributeHeader)))
        return false;

    unk_08 = hdr->unk_08;
    data_type = hdr->data_type;
    count = hdr->count;

    name.clear();

    bool eos = false;

    for (uint32_t i = 0; i < hdr->name_length; i++)
    {
        char ch;

        if (!in.Read8((int8_t *)&ch))
            return false;

        if (!eos)
        {
            if (ch != 0)
                name.push_back(ch);
            else
                eos = true;
        }
    }

    //DPRINTF("%s\n", name.c_str());

    size_t current_size = (size_t)(in.Tell() - start);
    data.clear();

    if (current_size < hdr->size)
    {
        size_t rem_size = hdr->size - current_size;
        data.resize(rem_size);

        if (!in.Read(data.data(), rem_size))
            return false;
    }

    return true;
}

bool G1MGAttribute::Write(MemoryStream &out) const
{
    G1MGAttributeHeader hdr;
    uint32_t name_length = (uint32_t)Utils::Align2(name.length()+1, 4);

    hdr.size = sizeof(G1MGAttributeHeader) + name_length + (uint32_t)data.size();
    hdr.name_length = name_length;
    hdr.unk_08 = unk_08;
    hdr.data_type = data_type;
    hdr.count = count;

    if (!out.Write(&hdr, sizeof(G1MGAttributeHeader)))
        return false;

    if (!out.Write(name.c_str(), name.length()))
        return false;

    for (uint32_t i = (uint32_t)name.length(); i < name_length; i++)
        if (!out.Write8(0))
            return false;

    if (data.size() > 0 && !out.Write(data.data(), data.size()))
        return false;

    return true;
}

TiXmlElement *G1MGAttribute::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Attribute");
    entry_root->SetAttribute("name", name);
    entry_root->SetAttribute("u_08", unk_08);

    const float *float_data = (const float *)data.data();
    const int32_t *i32_data = (const int32_t *)data.data();
    std::string value;

    switch (data_type)
    {
        case 1:

            entry_root->SetAttribute("data_type", "float");

            for (size_t i = 0; i < count; i++)
            {
                value += Utils::FloatToString(float_data[i]);
                if (i != (size_t)(count-1))
                    value += ", ";
            }

        break;

        case 2:
            entry_root->SetAttribute("data_type", "vector2D");
            value = Utils::Vectors2DToString(float_data, count);
        break;

        case 3:
            entry_root->SetAttribute("data_type", "vector3D");
            value = Utils::Vectors3DToString(float_data, count);
        break;

        case 4:
            entry_root->SetAttribute("data_type", "vector4D");
            value = Utils::Vectors4DToString(float_data, count);
        break;

        case 5:

            entry_root->SetAttribute("data_type", "int");

            for (size_t i = 0; i < count; i++)
            {
                value += Utils::SignedToString(i32_data[i]);
                if (i != (size_t)(count-1))
                    value += ", ";
            }

        break;

        default:
            entry_root->SetAttribute("data_type", data_type);
            entry_root->SetAttribute("data_count", count);

            for (size_t i = 0; i < data.size(); i++)
            {
                value += Utils::ToString(data[i]);
                if (i != (data.size()-1))
                    value += ", ";
            }
    }

    entry_root->SetAttribute("value", value);
    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MGAttribute::Compile(const TiXmlElement *root)
{
    bool ret = true;
    std::string data_type_str;

    if (!Utils::ReadAttrString(root, "name", name))
        ret = false;

    if (!Utils::ReadAttrUnsigned(root, "u_08", &unk_08))
        ret = false;

    if (!Utils::ReadAttrString(root, "data_type", data_type_str))
        ret = false;

    data_type_str = Utils::ToLowerCase(data_type_str);

    std::vector<float> data_float;
    std::vector<int32_t> data_int;
    std::string value_str;

    data.clear();

    if (data_type_str == "float")
    {
        data_type = 1;

        if (Utils::ReadAttrMultipleFloats(root, "value", data_float))
        {
            count = (uint16_t)data_float.size();
        }
        else
            ret = false;
    }
    else if (data_type_str == "vector2d")
    {
        data_type = 2;

        if (Utils::ReadAttrString(root, "value", value_str))
        {
            size_t num = Utils::GetVectors2DFromString(value_str, data_float);
            if (num == (size_t)-1)
                ret = false;
            else
                count = (uint16_t)num;
        }
        else
            ret = false;
    }
    else if (data_type_str == "vector3d")
    {
        data_type = 3;

        if (Utils::ReadAttrString(root, "value", value_str))
        {
            size_t num = Utils::GetVectors3DFromString(value_str, data_float);
            if (num == (size_t)-1)
                ret = false;
            else
                count = (uint16_t)num;
        }
        else
            ret = false;
    }
    else if (data_type_str == "vector4d")
    {
        data_type = 4;

        if (Utils::ReadAttrString(root, "value", value_str))
        {
            size_t num = Utils::GetVectors4DFromString(value_str, data_float);
            if (num == (size_t)-1)
                ret = false;
            else
                count = (uint16_t)num;
        }
        else
            ret = false;
    }
    else if (data_type_str == "int")
    {
        data_type = 5;

        if (Utils::ReadAttrMultipleSigned(root, "value", data_int))
            count = (uint16_t)data_int.size();
        else
            ret = false;
    }
    else
    {
        if (!Utils::ReadAttrUnsigned(root, "data_type", &data_type))
            ret = false;

        if (!Utils::ReadAttrUnsigned(root, "data_count", &count))
            ret = false;

        if (!Utils::ReadAttrMultipleUnsigned(root, "value", data))
            ret = false;
    }

    if (ret)
    {
        if (data.size() == 0)
        {
            if (data_float.size() > 0)
            {
                data.resize(data_float.size()*sizeof(float));
                memcpy(data.data(), data_float.data(), data.size());
            }
            else if (data_int.size() > 0)
            {
                data.resize(data_int.size()*sizeof(int32_t));
                memcpy(data.data(), data_int.data(), data.size());
            }
        }
    }
    else
    {
        DPRINTF("Failed to compile Attribute at line %d.\n", root->Row());
    }

    return ret;
}

bool G1MGMaterialAttributes::Read(FixedMemoryStream &in)
{
    uint32_t num_attributes;

    if (!in.Read32(&num_attributes))
        return false;

    attributes.resize(num_attributes);

    for (G1MGAttribute &att : attributes)
    {
        if (!att.Read(in))
            return false;
    }

    return true;
}

bool G1MGMaterialAttributes::Write(MemoryStream &out) const
{
    if (!out.Write32((uint32_t)attributes.size()))
        return false;

    for (const G1MGAttribute &att : attributes)
    {
        if (!att.Write(out))
            return false;
    }

    return true;
}

TiXmlElement *G1MGMaterialAttributes::Decompile(TiXmlNode *root, uint32_t idx) const
{
    TiXmlElement *entry_root = new TiXmlElement("MaterialAttributes");
    entry_root->SetAttribute("idx", idx);

    for (const G1MGAttribute &attr: attributes)
    {
        attr.Decompile(entry_root);
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MGMaterialAttributes::Compile(const TiXmlElement *root)
{
    attributes.clear();

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Attribute")
        {
            G1MGAttribute attr;

            if (!attr.Compile(elem))
                return false;

            attributes.push_back(attr);
        }
    }

    return true;
}

bool G1MGAttributesSection::Read(FixedMemoryStream &in, uint32_t)
{
    if (!in.Seek(8, SEEK_CUR))
        return false;

    uint32_t num_materials;

    if (!in.Read32(&num_materials))
        return false;

    mat_attributes.resize(num_materials);

    for (G1MGMaterialAttributes &m_at: mat_attributes)
    {
        if (!m_at.Read(in))
            return false;
    }

    valid = true;
    return true;
}

bool G1MGAttributesSection::Write(MemoryStream &out) const
{
    if (!valid)
        return false;

    uint64_t start = out.Tell();

    if (!out.Write32(0x10003))
        return false;

    if (!out.Write32(0))
        return false;

    if (!out.Write32((uint32_t)mat_attributes.size()))
        return false;

    for (const G1MGMaterialAttributes &m_at : mat_attributes)
    {
        if (!m_at.Write(out))
            return false;
    }

    uint64_t end = out.Tell();

    if (!out.Seek((off64_t)start + 4, SEEK_SET))
        return false;

    if (!out.Write32((uint32_t)(end-start)))
        return false;

    return out.Seek((off64_t)end, SEEK_SET);
}

TiXmlElement *G1MGAttributesSection::Decompile(TiXmlNode *root) const
{
    if (!valid)
        return nullptr;

    TiXmlElement *entry_root = new TiXmlElement("AttributesSection");

    for (uint32_t i = 0; i < (uint32_t)mat_attributes.size(); i++)
    {
        mat_attributes[i].Decompile(entry_root, i);
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MGAttributesSection::Compile(const TiXmlElement *root)
{    
    size_t count = Utils::GetElemCount(root, "MaterialAttributes");
    std::vector<bool> defined;

    mat_attributes.clear();
    mat_attributes.resize(count);
    defined.resize(count, false);

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "MaterialAttributes")
        {
            uint32_t idx;

            if (!Utils::ReadAttrUnsigned(elem, "idx", &idx))
            {
                DPRINTF("Cannot get attribute \"idx\" (in object at line %d).\n", elem->Row());
                return false;
            }

            if (idx >= (uint32_t)mat_attributes.size())
            {
                DPRINTF("idx %d is out of bounds (there are %Id MaterialAttributes) (in object at line %d).\n", idx, mat_attributes.size(), elem->Row());
                return false;
            }

            if (defined[idx])
            {
                DPRINTF("idx %d is duplicated (duplication in object at line %d).\n", idx, elem->Row());
                return false;
            }

            defined[idx] = true;

            if (!mat_attributes[idx].Compile(elem))
                return false;
        }
    }

    valid = true;
    return true;
}

bool G1MGVertexBuffer::Read(FixedMemoryStream &in)
{
    G1MGVertexBufHeader *hdr;

    if (!in.FastRead((uint8_t **)&hdr, sizeof(G1MGVertexBufHeader)))
        return false;

    unk_00 = hdr->unk_00;
    vertex_size = hdr->vertex_size;
    unk_0C = hdr->unk_0C;

    //DPRINTF("Vertex size: %x\n", vertex_size);

    vertex.resize(vertex_size*hdr->num_vertex);

    if (!in.Read(vertex.data(), vertex.size()))
        return false;   

    return true;
}

bool G1MGVertexBuffer::Write(MemoryStream &out) const
{
    G1MGVertexBufHeader hdr;

    hdr.unk_00 = unk_00;
    hdr.vertex_size = vertex_size;
    hdr.num_vertex = GetNumVertex();
    hdr.unk_0C = unk_0C;

    if (!out.Write(&hdr, sizeof(G1MGVertexBufHeader)))
        return false;

    if (vertex.size() > 0 && !out.Write(vertex.data(), vertex.size()))
        return false;

    return true;
}

TiXmlElement *G1MGVertexBuffer::Decompile(TiXmlNode *root, const std::string &att_dir, uint32_t idx) const
{
    TiXmlElement *entry_root = new TiXmlElement("VB");
    std::string fn = "VB_" + Utils::ToString(idx) + ".bin";

    entry_root->SetAttribute("idx", idx);
    entry_root->SetAttribute("vertex_size", vertex_size);
    entry_root->SetAttribute("u_00", unk_00);
    entry_root->SetAttribute("u_0c", unk_0C);
    entry_root->SetAttribute("binary", fn);

    std::string path = Utils::MakePathString(att_dir, fn);
    if (!Utils::WriteFileBool(path, vertex.data(), vertex.size(), true, true))
        return nullptr;

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MGVertexBuffer::Compile(const TiXmlElement *root, const std::string &att_dir)
{
    if (!Utils::ReadAttrUnsigned(root, "vertex_size", &vertex_size))
    {
        DPRINTF("Cannot read parameter \"vertex\" (object at line %d).\n", root->Row());
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "u_00", &unk_00))
    {
        DPRINTF("Cannot read parameter \"u_00\" (object at line %d).\n", root->Row());
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "u_0c", &unk_0C))
    {
        DPRINTF("Cannot read parameter \"u_0c\" (object at line %d).\n", root->Row());
        return false;
    }

    std::string fn, path;
    size_t size;

    if (!Utils::ReadAttrString(root, "binary", fn))
    {
        DPRINTF("Cannot read parameter \"binary\" (object at line %d).\n", root->Row());
        return false;
    }

    path = Utils::MakePathString(att_dir, fn);
    uint8_t *buf = Utils::ReadFile(path, &size);
    if (!buf)
        return false;

    if ((size % vertex_size) != 0)
    {
        DPRINTF("Size of file \"%s\" is not multiple of vertex_size (object at line %d).\n", path.c_str(), root->Row());
        delete[] buf;
        return false;
    }

    vertex.resize(size);
    memcpy(vertex.data(), buf, size);
    delete[] buf;

    return true;
}

bool G1MGVertexSection::Read(FixedMemoryStream &in, uint32_t)
{
    if (!in.Seek(8, SEEK_CUR))
        return false;

    uint32_t num_submeshes;

    if (!in.Read32(&num_submeshes))
        return false;

    vertex_bufs.resize(num_submeshes);

    //DPRINTF("Num submeshes: %Ix\n", vertex_bufs.size());

    for (G1MGVertexBuffer &vb : vertex_bufs)
    {
        if (!vb.Read(in))
            return false;
    }

    valid = true;
    return true;
}

bool G1MGVertexSection::Write(MemoryStream &out) const
{
    if (!valid)
        return false;

    uint64_t start = out.Tell();

    if (!out.Write32(0x10004))
        return false;

    if (!out.Write32(0))
        return false;

    if (!out.Write32((uint32_t)vertex_bufs.size()))
        return false;

    for (const G1MGVertexBuffer &vb : vertex_bufs)
    {
        if (!vb.Write(out))
            return false;
    }

    uint64_t end = out.Tell();

    if (!out.Seek((off64_t)start + 4, SEEK_SET))
        return false;

    if (!out.Write32((uint32_t)(end-start)))
        return false;

    return out.Seek((off64_t)end, SEEK_SET);
}

TiXmlElement *G1MGVertexSection::Decompile(TiXmlNode *root, const std::string &att_dir) const
{
    if (!valid)
        return nullptr;

    TiXmlElement *entry_root = new TiXmlElement("VertexSection");

    for (uint32_t i = 0; i < (uint32_t)vertex_bufs.size(); i++)
    {
        if (!vertex_bufs[i].Decompile(entry_root, att_dir, i))
            return nullptr;
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MGVertexSection::Compile(const TiXmlElement *root, const std::string &att_dir)
{
    size_t count = Utils::GetElemCount(root, "VB");

    vertex_bufs.clear();
    vertex_bufs.resize(count);

    std::vector<bool> defined;
    defined.resize(count, false);

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "VB")
        {
            uint32_t idx;

            if (!Utils::ReadAttrUnsigned(elem, "idx", &idx))
            {
                DPRINTF("Cannot get attribute \"idx\" (in object at line %d).\n", elem->Row());
                return false;
            }

            if (idx >= (uint32_t)vertex_bufs.size())
            {
                DPRINTF("idx %d is out of bounds (there are %Id VB) (in object at line %d).\n", idx, vertex_bufs.size(), elem->Row());
                return false;
            }

            if (defined[idx])
            {
                DPRINTF("idx %d is duplicated (duplication in object at line %d).\n", idx, elem->Row());
                return false;
            }

            defined[idx] = true;

            if (!vertex_bufs[idx].Compile(elem, att_dir))
                return false;
        }
    }

    valid = true;
    return true;
}

bool G1MGSemantic::Read(FixedMemoryStream &in)
{
    if (!in.Read16(&buffer_index))
        return false;

    if (!in.Read16(&offset))
        return false;

    if (!in.Read16(&data_type))
        return false;

    if (!in.Read16(&semantic))
        return false;

    return true;
}

bool G1MGSemantic::Write(MemoryStream &out) const
{
    if (!out.Write16(buffer_index))
        return false;

    if (!out.Write16(offset))
        return false;

    if (!out.Write16(data_type))
        return false;

    if (!out.Write16(semantic))
        return false;

    return true;
}

TiXmlElement *G1MGSemantic::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("Semantic");

    uint8_t type = semantic&0xFF;
    uint8_t idx = semantic >> 8;
    std::string type_str, format_str;

    switch (type)
    {
        case 0:
            type_str = "POSITION";
        break;

        case 1:
            type_str = "BLENDWEIGHT";
        break;

        case 2:
            type_str = "BLENDINDICES";
        break;

        case 3:
            type_str = "NORMAL";
        break;

        case 4:
            type_str = "PSIZE";
        break;

        case 5:
            type_str = "TEXCOORD";
        break;

        case 6:
            type_str = "TANGENT";
        break;

        case 7:
            type_str = "BINORMAL";
        break;

        case 8:
            type_str = "TESSFACTOR";
        break;

        case 9:
            type_str = "POSITIONT";
        break;

        case 10:
            type_str = "COLOR";
        break;

        case 11:
            type_str = "FOG";
        break;

        case 12:
            type_str = "DEPTH";
        break;

        case 13:
            type_str = "SAMPLE";
        break;

        default:
            DPRINTF("%s: Unrecognized semantic  %d\n", FUNCNAME, type);
            return nullptr;
    }

    switch (data_type)
    {
        case 0:
            format_str = "R32_FLOAT";
        break;

        case 1:
            format_str = "R32G32_FLOAT";
        break;

        case 2:
            format_str = "R32G32B32_FLOAT";
        break;

        case 3:
            format_str = "R32G32B32A32_FLOAT";
        break;

        case 5:
            format_str = "R8G8B8A8_UINT";
        break;

        case 7:
            format_str = "R16G16B16A16_UINT";
        break;

        case 9:
            format_str = "R32G32B32A32_UINT";
        break;

        case 10:
            format_str = "R16G16_FLOAT";
        break;

        case 11:
            format_str = "R16G16B16A16_FLOAT";
        break;

        // Needs confirmation! We only saw thsi data type with FF FF FF FF data
        // It actually may be DXGI_FORMAT_X24_TYPELESS_G8_UINT
        case 13:
            format_str = "R8G8B8A8_UINT_";
        break;

        default:
            DPRINTF("%s: Unrecognized data type %d\n", FUNCNAME, data_type);
            return nullptr;
    }

    entry_root->SetAttribute("index", idx);
    entry_root->SetAttribute("type", type_str);
    entry_root->SetAttribute("format", format_str);
    entry_root->SetAttribute("buffer_index", buffer_index);
    entry_root->SetAttribute("offset", offset);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MGSemantic::Compile(const TiXmlElement *root)
{
    uint8_t type;
    uint8_t idx;
    std::string type_str, format_str;

    if (!Utils::ReadAttrUnsigned(root, "index", &idx))
    {
        DPRINTF("Cannot read attribute \"index\" (in object at line %d).\n", root->Row());
        return false;
    }

    if (!Utils::ReadAttrString(root, "type", type_str))
    {
        DPRINTF("Cannot read attribute \"type\" (in object at line %d).\n", root->Row());
        return false;
    }

    if (!Utils::ReadAttrString(root, "format", format_str))
    {
        DPRINTF("Cannot read attribute \"format\" (in object at line %d).\n", root->Row());
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "buffer_index", &buffer_index))
    {
        DPRINTF("Cannot read attribute \"buffer_index\" (in object at line %d).\n", root->Row());
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "offset", &offset))
    {
        DPRINTF("Cannot read attribute \"offset\" (in object at line %d).\n", root->Row());
        return false;
    }

    if (type_str == "POSITION")
    {
        type = 0;
    }
    else if (type_str == "BLENDWEIGHT")
    {
        type = 1;
    }
    else if (type_str == "BLENDINDICES")
    {
        type = 2;
    }
    else if (type_str == "NORMAL")
    {
        type = 3;
    }
    else if (type_str == "PSIZE")
    {
        type = 4;
    }
    else if (type_str == "TEXCOORD")
    {
        type = 5;
    }
    else if (type_str == "TANGENT")
    {
        type = 6;
    }
    else if (type_str == "BINORMAL")
    {
        type = 7;
    }
    else if (type_str == "TESSFACTOR")
    {
        type = 8;
    }
    else if (type_str == "POSITIONT")
    {
        type = 9;
    }
    else if (type_str == "COLOR")
    {
        type = 10;
    }
    else if (type_str == "FOG")
    {
        type = 11;
    }
    else if (type_str == "DEPTH")
    {
        type = 12;
    }
    else if (type_str == "SAMPLE")
    {
        type = 13;
    }
    else
    {
        DPRINTF("Unrecognized type \"%s\" (in object at line %d).\n", type_str.c_str(), root->Row());
        return false;
    }

    if (format_str == "R32_FLOAT")
    {
        data_type = 0;
    }
    else if (format_str == "R32G32_FLOAT")
    {
        data_type = 1;
    }
    else if (format_str == "R32G32B32_FLOAT")
    {
        data_type = 2;
    }
    else if (format_str == "R32G32B32A32_FLOAT")
    {
        data_type = 3;
    }
    else if (format_str == "R8G8B8A8_UINT")
    {
        data_type = 5;
    }
    else if (format_str == "R16G16B16A16_UINT")
    {
        data_type = 7;
    }
    else if (format_str == "R32G32B32A32_UINT")
    {
        data_type = 9;
    }
    else if (format_str == "R16G16_FLOAT")
    {
        data_type = 10;
    }
    else if (format_str == "R16G16B16A16_FLOAT")
    {
        data_type = 11;
    }
    else if (format_str == "R8G8B8A8_UINT_")
    {
        data_type = 13;
    }
    else
    {
        DPRINTF("Unrecognized format \"%s\" (in object at line %d).\n", format_str.c_str(), root->Row());
        return false;
    }

    semantic = (uint16_t) (idx << 8) | type;
    return true;
}

bool G1MGLayout::Read(FixedMemoryStream &in)
{
    uint32_t n_refs, n_sem;

    if (!in.Read32(&n_refs))
        return false;

    refs.resize(n_refs);

    if (refs.size() > 0 && !in.Read(refs.data(), refs.size()*sizeof(uint32_t)))
        return false;

    if (!in.Read32(&n_sem))
        return false;

    semantics.resize(n_sem);

    for (G1MGSemantic &sem : semantics)
    {
        if (!sem.Read(in))
            return false;
    }

    return true;
}

bool G1MGLayout::Write(MemoryStream &out) const
{
    if (!out.Write32((uint32_t)refs.size()))
        return false;

    if (refs.size() >0 && !out.Write(refs.data(), refs.size()*sizeof(uint32_t)))
        return false;

    if (!out.Write32((uint32_t)semantics.size()))
        return false;

    for (const G1MGSemantic &sem : semantics)
    {
        if (!sem.Write(out))
            return false;
    }

    return true;
}

TiXmlElement *G1MGLayout::Decompile(TiXmlNode *root, uint32_t idx) const
{
    TiXmlElement *entry_root = new TiXmlElement("Layout");
    entry_root->SetAttribute("idx", idx);

    Utils::WriteParamMultipleUnsigned(entry_root, "Refs", refs);
    for (const G1MGSemantic &sem: semantics)
    {
        if (!sem.Decompile(entry_root))
            return nullptr;
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MGLayout::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamMultipleUnsigned(root, "Refs", refs))
        return false;

    semantics.clear();

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Semantic")
        {
            G1MGSemantic sem;

            if (!sem.Compile(elem))
                return false;

            semantics.push_back(sem);
        }
    }

    return true;
}

bool G1MGLayoutSection::Read(FixedMemoryStream &in, uint32_t)
{
    if (!in.Seek(8, SEEK_CUR))
        return false;

    uint32_t num_submeshes;

    if (!in.Read32(&num_submeshes))
        return false;

    entries.resize(num_submeshes);

    for (G1MGLayout &ly : entries)
    {
        if (!ly.Read(in))
            return false;
    }

    valid = true;
    return true;
}

bool G1MGLayoutSection::Write(MemoryStream &out) const
{
    if (!valid)
        return false;

    uint64_t start = out.Tell();

    if (!out.Write32(0x10005))
        return false;

    if (!out.Write32(0))
        return false;

    if (!out.Write32((uint32_t)entries.size()))
        return false;

    for (const G1MGLayout &ly : entries)
    {
        if (!ly.Write(out))
            return false;
    }

    uint64_t end = out.Tell();

    if (!out.Seek((off64_t)start + 4, SEEK_SET))
        return false;

    if (!out.Write32((uint32_t)(end-start)))
        return false;

    return out.Seek((off64_t)end, SEEK_SET);
}

TiXmlElement *G1MGLayoutSection::Decompile(TiXmlNode *root) const
{
    if (!valid)
        return nullptr;

    TiXmlElement *entry_root = new TiXmlElement("LayoutSection");

    for (uint32_t i = 0; i < (uint32_t)entries.size(); i++)
    {
        if (!entries[i].Decompile(entry_root, i))
            return nullptr;
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MGLayoutSection::Compile(const TiXmlElement *root)
{
    size_t count = Utils::GetElemCount(root, "Layout");
    std::vector<bool> defined;

    entries.clear();
    entries.resize(count);
    defined.resize(count, false);

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Layout")
        {
            uint32_t idx;

            if (!Utils::ReadAttrUnsigned(elem, "idx", &idx))
            {
                DPRINTF("Cannot get attribute \"idx\" (in object at line %d).\n", elem->Row());
                return false;
            }

            if (idx >= (uint32_t)entries.size())
            {
                DPRINTF("idx %d is out of bounds (there are %Id Layout) (in object at line %d).\n", idx, entries.size(), elem->Row());
                return false;
            }

            if (defined[idx])
            {
                DPRINTF("idx %d is duplicated (duplication in object at line %d).\n", idx, elem->Row());
                return false;
            }

            defined[idx] = true;

            if (!entries[idx].Compile(elem))
                return false;
        }
    }

    valid = true;
    return true;
}

bool G1MGBoneMapEntry::Read(FixedMemoryStream &in)
{
    if (!in.Read32(&matrix))
        return false;

    if (!in.Read32(&cloth))
        return false;

    if (!in.Read16(&mapped))
        return false;

    if (!in.Read16(&flags))
        return false;

    return true;
}

bool G1MGBoneMapEntry::Write(MemoryStream &out) const
{
    if (!out.Write32(matrix))
        return false;

    if (!out.Write32(cloth))
        return false;

    if (!out.Write16(mapped))
        return false;

    if (!out.Write16(flags))
        return false;

    return true;
}

TiXmlElement *G1MGBoneMapEntry::Decompile(TiXmlNode *root, const std::vector<std::string> &bone_names) const
{
    TiXmlElement *entry_root = new TiXmlElement("BoneEntry");

    if (mapped >= bone_names.size() && flags != 0)
    {
        /*DPRINTF("%s: mapped bone (0x%x aka %d) is greater than names array (%Id) (u_04=%d).\n", FUNCNAME, mapped, mapped, bone_names.size(), unk_04);
        return nullptr;*/
        // Happens in AYA_COS_010.g1m, possibly other
        entry_root->SetAttribute("mapped", mapped);

        if (flags != 0)
            entry_root->SetAttribute("flags", Utils::UnsignedToString(flags, true));
    }
    else
    {
        entry_root->SetAttribute("bone", bone_names[mapped]);

        if (flags != 0)
            entry_root->SetAttribute("flags", Utils::UnsignedToString(flags, true));
    }

    entry_root->SetAttribute("cloth", cloth);
    entry_root->SetAttribute("matrix", matrix);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MGBoneMapEntry::Compile(const TiXmlElement *root, const std::vector<std::string> &bone_names)
{
    std::string name;
    bool use_name = false;

    flags = 0;

    if (!Utils::ReadAttrString(root, "bone", name))
    {
        /*DPRINTF("Missing attribute \"bone\" (in object at line %d).\n", root->Row());
        return false;*/
        if (!Utils::ReadAttrSigned(root, "mapped", (int32_t *)&mapped))
        {
            DPRINTF("Missing attribute \"bone\" or \"mapped\" (in object at line %d).\n", root->Row());
            return false;
        }        
    }
    else
    {
        use_name = true;
    }

    if (!Utils::ReadAttrUnsigned(root, "flags", &flags))
        flags = 0;

    if (!Utils::ReadAttrUnsigned(root, "matrix", &matrix))
    {
        DPRINTF("Missing attribute \"matrix\" (in object at line %d).\n", root->Row());
        return false;
    }

    if (!Utils::ReadAttrSigned(root, "cloth", (int32_t *)&cloth))
        cloth = 0;

    if (use_name)
    {
        uint16_t idx = 0xFFFF;

        for (uint16_t i = 0; i < (uint16_t)bone_names.size(); i++)
        {
            if (bone_names[i] == name)
            {
                idx = i;
                break;
            }
        }

        if (idx == 0xFFFF)
        {
            DPRINTF("Cannot recognize bone \"%s\" (in object at line %d).\n", name.c_str(), root->Row());
            return false;
        }

        mapped = idx;
    }

    return true;
}

bool G1MGBonesMap::Read(FixedMemoryStream &in)
{
    uint32_t num;

    if (!in.Read32(&num))
        return false;

    map.resize(num);

    for (G1MGBoneMapEntry &entry : map)
    {
        if (!entry.Read(in))
            return false;
    }

    return true;
}

bool G1MGBonesMap::Write(MemoryStream &out) const
{
    if (!out.Write32((uint32_t)map.size()))
        return false;

    for (const G1MGBoneMapEntry &entry : map)
    {
        if (!entry.Write(out))
            return false;
    }

    return true;
}

TiXmlElement *G1MGBonesMap::Decompile(TiXmlNode *root, const std::vector<std::string> &bone_names, uint32_t idx) const
{
    TiXmlElement *entry_root = new TiXmlElement("BonesMap");
    entry_root->SetAttribute("idx", idx);

    for (const G1MGBoneMapEntry &entry : map)
    {
        if (!entry.Decompile(entry_root, bone_names))
            return nullptr;
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MGBonesMap::Compile(const TiXmlElement *root, const std::vector<std::string> &bone_names)
{
    map.clear();

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "BoneEntry")
        {
            G1MGBoneMapEntry entry;

            if (!entry.Compile(elem, bone_names))
                return false;

            map.push_back(entry);
        }
    }

    return true;
}

bool G1MGBonesMapSection::Read(FixedMemoryStream &in, uint32_t)
{
    if (!in.Seek(8, SEEK_CUR))
        return false;

    uint32_t num;

    if (!in.Read32(&num))
        return false;

    bones_maps.resize(num);

    for (G1MGBonesMap &map : bones_maps)
    {
        if (!map.Read(in))
            return false;
    }

    valid = true;
    return true;
}

bool G1MGBonesMapSection::Write(MemoryStream &out) const
{
    if (!valid)
        return false;

    uint64_t start = out.Tell();

    if (!out.Write32(0x10006))
        return false;

    if (!out.Write32(0))
        return false;

    if (!out.Write32((uint32_t)bones_maps.size()))
        return false;

    for (const G1MGBonesMap &map : bones_maps)
    {
        if (!map.Write(out))
            return false;
    }

    uint64_t end = out.Tell();

    if (!out.Seek((off64_t)start + 4, SEEK_SET))
        return false;

    if (!out.Write32((uint32_t)(end-start)))
        return false;

    return out.Seek((off64_t)end, SEEK_SET);
}

TiXmlElement *G1MGBonesMapSection::Decompile(TiXmlNode *root, const std::vector<std::string> &bone_names) const
{
    if (!valid)
        return nullptr;

    TiXmlElement *entry_root = new TiXmlElement("BonesMapSection");

    for (uint32_t i = 0; i < (uint32_t)bones_maps.size(); i++)
    {
        if (!bones_maps[i].Decompile(entry_root, bone_names, i))
            return nullptr;
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MGBonesMapSection::Compile(const TiXmlElement *root, const std::vector<std::string> &bone_names)
{
    size_t count = Utils::GetElemCount(root, "BonesMap");
    std::vector<bool> defined;

    bones_maps.clear();
    bones_maps.resize(count);
    defined.resize(count, false);

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "BonesMap")
        {
            uint32_t idx;

            if (!Utils::ReadAttrUnsigned(elem, "idx", &idx))
            {
                DPRINTF("Cannot get attribute \"idx\" (in object at line %d).\n", elem->Row());
                return false;
            }

            if (idx >= (uint32_t)bones_maps.size())
            {
                DPRINTF("idx %d is out of bounds (there are %Id BonesMap) (in object at line %d).\n", idx, bones_maps.size(), elem->Row());
                return false;
            }

            if (defined[idx])
            {
                DPRINTF("idx %d is duplicated (duplication in object at line %d).\n", idx, elem->Row());
                return false;
            }

            defined[idx] = true;

            if (!bones_maps[idx].Compile(elem, bone_names))
                return false;
        }
    }

    valid = true;
    return true;
}

bool G1MGIndexBuffer::Read(FixedMemoryStream &in)
{
    G1MGIndexBufferHeader *hdr;

    while (in.Tell() & 3)
        in.Seek(1, SEEK_CUR);

    uint64_t start = in.Tell();

    if (!in.FastRead((uint8_t **)&hdr, sizeof(G1MGIndexBufferHeader)))
        return false;

    type = hdr->type;
    unk_08 = hdr->unk_08;

    size_t size_mult = 0;

    if (type == 8)
    {
        size_mult = 1;
    }
    else if (type == 0x10)
    {
        size_mult = 2;
    }
    else if (type == 0x20)
    {
        size_mult = 4;
    }
    else
    {
        DPRINTF("G1MGIndexBuffer: unknown type %08x at %Ix\n", type, (size_t)start);
        return false;
    }

    indices.resize(hdr->num_indices*size_mult);
    if (!in.Read(indices.data(), indices.size()))
        return false;

    while (in.Tell() & 3)
        in.Seek(1, SEEK_CUR);

    return true;
}

bool G1MGIndexBuffer::Write(MemoryStream &out) const
{
    G1MGIndexBufferHeader hdr;

    hdr.num_indices = (uint32_t)GetNumIndices();
    hdr.type = type;
    hdr.unk_08 = unk_08;

    if (!out.Write(&hdr, sizeof(G1MGIndexBufferHeader)))
        return false;

    if (indices.size() > 0 && !out.Write(indices.data(), indices.size()))
        return false;

    while (out.Tell() & 3)
        out.Write8(0);

    return true;
}

TiXmlElement *G1MGIndexBuffer::Decompile(TiXmlNode *root, const std::string &att_dir, uint32_t idx) const
{
    TiXmlElement *entry_root = new TiXmlElement("IB");
    std::string fn = "IB_" + Utils::ToString(idx) + ".bin";

    entry_root->SetAttribute("idx", idx);
    entry_root->SetAttribute("bits", type);
    entry_root->SetAttribute("u_08", unk_08);
    entry_root->SetAttribute("binary", fn);

    std::string path = Utils::MakePathString(att_dir, fn);
    if (!Utils::WriteFileBool(path, indices.data(), indices.size(), true, true))
        return nullptr;

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MGIndexBuffer::Compile(const TiXmlElement *root, const std::string &att_dir)
{
    if (!Utils::ReadAttrUnsigned(root, "bits", &type))
    {
        DPRINTF("Cannot read parameter \"bits\" (object at line %d).\n", root->Row());
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "u_08", &unk_08))
    {
        DPRINTF("Cannot read parameter \"u_08\" (object at line %d).\n", root->Row());
        return false;
    }

    std::string fn, path;
    size_t size;

    if (!Utils::ReadAttrString(root, "binary", fn))
    {
        DPRINTF("Cannot read parameter \"binary\" (object at line %d).\n", root->Row());
        return false;
    }

    path = Utils::MakePathString(att_dir, fn);
    uint8_t *buf = Utils::ReadFile(path, &size);
    if (!buf)
        return false;

    indices.resize(size);
    memcpy(indices.data(), buf, size);
    delete[] buf;

    return true;
}

bool G1MGIndexBufferSection::Read(FixedMemoryStream &in, uint32_t)
{
    if (!in.Seek(8, SEEK_CUR))
        return false;

    uint32_t num;

    if (!in.Read32(&num))
        return false;

    buffers.resize(num);

    for (G1MGIndexBuffer &buf : buffers)
    {
        if (!buf.Read(in))
            return false;
    }

    valid = true;
    return true;
}

bool G1MGIndexBufferSection::Write(MemoryStream &out) const
{
    if (!valid)
        return false;

    uint64_t start = out.Tell();

    if (!out.Write32(0x10007))
        return false;

    if (!out.Write32(0))
        return false;

    if (!out.Write32((uint32_t)buffers.size()))
        return false;

    for (const G1MGIndexBuffer &buf : buffers)
    {
        if (!buf.Write(out))
            return false;
    }

    uint64_t end = out.Tell();

    if (!out.Seek((off64_t)start + 4, SEEK_SET))
        return false;

    if (!out.Write32((uint32_t)(end-start)))
        return false;

    return out.Seek((off64_t)end, SEEK_SET);
}

TiXmlElement *G1MGIndexBufferSection::Decompile(TiXmlNode *root, const std::string &att_dir) const
{
    if (!valid)
        return nullptr;

    TiXmlElement *entry_root = new TiXmlElement("IndicesSection");

    for (uint32_t i = 0; i < (uint32_t)buffers.size(); i++)
    {
        if (!buffers[i].Decompile(entry_root, att_dir, i))
            return nullptr;
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MGIndexBufferSection::Compile(const TiXmlElement *root, const std::string &att_dir)
{
    size_t count = Utils::GetElemCount(root, "IB");

    buffers.clear();
    buffers.resize(count);

    std::vector<bool> defined;
    defined.resize(count, false);

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "IB")
        {
            uint32_t idx;

            if (!Utils::ReadAttrUnsigned(elem, "idx", &idx))
            {
                DPRINTF("Cannot get attribute \"idx\" (in object at line %d).\n", elem->Row());
                return false;
            }

            if (idx >= (uint32_t)buffers.size())
            {
                DPRINTF("idx %d is out of bounds (there are %Id IB) (in object at line %d).\n", idx, buffers.size(), elem->Row());
                return false;
            }

            if (defined[idx])
            {
                DPRINTF("idx %d is duplicated (duplication in object at line %d).\n", idx, elem->Row());
                return false;
            }

            defined[idx] = true;

            if (!buffers[idx].Compile(elem, att_dir))
                return false;
        }
    }

    valid = true;
    return true;
}

bool G1MGSubmesh::Read(FixedMemoryStream &in)
{
    if (!in.Read32(&flags))
        return false;

    if (!in.Read32(&vertex_buf_ref))
        return false;

    if (!in.Read32(&bones_map_index))
            return false;

    if (!in.Read32(&matpalid))
        return false;

    if (!in.Read32(&unk_10))
        return false;

    if (!in.Read32(&attribute))
        return false;

    if (!in.Read32(&material))
        return false;

    if (!in.Read32(&index_buf_ref))
        return false;

    if (!in.Read32(&unk_20))
        return false;

    if (!in.Read32(&index_buf_fmt))
        return false;

    if (!in.Read32(&vertex_buf_start))
        return false;

    if (!in.Read32(&num_vertices))
        return false;

    if (!in.Read32(&index_buf_start))
        return false;

    if (!in.Read32(&num_indices))
        return false;

    return true;
}

bool G1MGSubmesh::Write(MemoryStream &out) const
{
    if (!out.Write32(flags))
        return false;

    if (!out.Write32(vertex_buf_ref))
        return false;

    if (!out.Write32(bones_map_index))
        return false;

    if (!out.Write32(matpalid))
        return false;

    if (!out.Write32(unk_10))
        return false;

    if (!out.Write32(attribute))
        return false;

    if (!out.Write32(material))
        return false;

    if (!out.Write32(index_buf_ref))
        return false;

    if (!out.Write32(unk_20))
        return false;

    if (!out.Write32(index_buf_fmt))
        return false;

    if (!out.Write32(vertex_buf_start))
        return false;

    if (!out.Write32(num_vertices))
        return false;

    if (!out.Write32(index_buf_start))
        return false;

    if (!out.Write32(num_indices))
        return false;

    return true;
}

TiXmlElement *G1MGSubmesh::Decompile(TiXmlNode *root, uint32_t idx) const
{
    TiXmlElement *entry_root = new TiXmlElement("Submesh");
    entry_root->SetAttribute("idx", idx);

    Utils::WriteParamUnsigned(entry_root, "Flags", flags, true);
    Utils::WriteParamSigned(entry_root, "VB", vertex_buf_ref);
    Utils::WriteParamSigned(entry_root, "Bones_map", bones_map_index);
    Utils::WriteComment(entry_root, "If you want a submesh to use the material of other, replace the Matpalid, U_10, Attribute and Material. Matpalid matches the number in the .mtl in DOA6.");
    Utils::WriteParamUnsigned(entry_root, "Matpalid", matpalid);
    Utils::WriteParamUnsigned(entry_root, "U_10", unk_10);
    Utils::WriteParamSigned(entry_root, "Attribute", attribute);
    Utils::WriteParamSigned(entry_root, "Material", material);
    Utils::WriteParamSigned(entry_root, "IB", index_buf_ref);
    Utils::WriteParamUnsigned(entry_root, "U_20", unk_20);
    Utils::WriteParamUnsigned(entry_root, "IB_format", index_buf_fmt);
    Utils::WriteParamUnsigned(entry_root, "VB_Start", vertex_buf_start);
    Utils::WriteParamUnsigned(entry_root, "NumVertices", num_vertices);
    Utils::WriteParamUnsigned(entry_root, "IB_Start", index_buf_start);
    Utils::WriteParamUnsigned(entry_root, "NumIndices", num_indices);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MGSubmesh::Compile(const TiXmlElement *root)
{
    if (!Utils::GetParamUnsigned(root, "Flags", &flags)) return false;
    if (!Utils::GetParamSigned(root, "VB", &vertex_buf_ref)) return false;
    if (!Utils::GetParamSigned(root, "Bones_map", &bones_map_index)) return false;
    if (!Utils::GetParamUnsigned(root, "Matpalid", &matpalid)) return false;
    if (!Utils::GetParamUnsigned(root, "U_10", &unk_10)) return false;
    if (!Utils::GetParamSigned(root, "Attribute", &attribute)) return false;
    if (!Utils::GetParamSigned(root, "Material", &material)) return false;
    if (!Utils::GetParamSigned(root, "IB", &index_buf_ref)) return false;
    if (!Utils::GetParamUnsigned(root, "U_20", &unk_20)) return false;
    if (!Utils::GetParamUnsigned(root, "IB_format", &index_buf_fmt)) return false;
    if (!Utils::GetParamUnsigned(root, "VB_Start", &vertex_buf_start)) return false;
    if (!Utils::GetParamUnsigned(root, "NumVertices", &num_vertices)) return false;
    if (!Utils::GetParamUnsigned(root, "IB_Start", &index_buf_start)) return false;
    if (!Utils::GetParamUnsigned(root, "NumIndices", &num_indices)) return false;

    return true;
}

bool G1MGSubmeshesSection::Read(FixedMemoryStream &in, uint32_t)
{
    if (!in.Seek(8, SEEK_CUR))
        return false;

    uint32_t num;

    if (!in.Read32(&num))
        return false;

    submeshes.resize(num);

    for (G1MGSubmesh &surf : submeshes)
    {
        if (!surf.Read(in))
            return false;
    }

    valid = true;
    return true;
}

bool G1MGSubmeshesSection::Write(MemoryStream &out) const
{
    if (!valid)
        return false;

    uint64_t start = out.Tell();

    if (!out.Write32(0x10008))
        return false;

    if (!out.Write32(0))
        return false;

    if (!out.Write32((uint32_t)submeshes.size()))
        return false;

    for (const G1MGSubmesh &surf : submeshes)
    {
        if (!surf.Write(out))
            return false;
    }

    uint64_t end = out.Tell();

    if (!out.Seek((off64_t)start + 4, SEEK_SET))
        return false;

    if (!out.Write32((uint32_t)(end-start)))
        return false;

    return out.Seek((off64_t)end, SEEK_SET);
}

TiXmlElement *G1MGSubmeshesSection::Decompile(TiXmlNode *root) const
{
    if (!valid)
        return nullptr;

    TiXmlElement *entry_root = new TiXmlElement("SubmeshesSection");

    for (uint32_t i = 0; i < submeshes.size(); i++)
    {
        if (!submeshes[i].Decompile(entry_root, i))
            return nullptr;
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MGSubmeshesSection::Compile(const TiXmlElement *root)
{
    size_t count = Utils::GetElemCount(root, "Submesh");

    submeshes.clear();
    submeshes.resize(count);

    std::vector<bool> defined;
    defined.resize(count, false);

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Submesh")
        {
            uint32_t idx;

            if (!Utils::ReadAttrUnsigned(elem, "idx", &idx))
            {
                DPRINTF("Cannot get attribute \"idx\" (in object at line %d).\n", elem->Row());
                return false;
            }

            if (idx >= (uint32_t)submeshes.size())
            {
                DPRINTF("idx %d is out of bounds (there are %Id Submesh) (in object at line %d).\n", idx, submeshes.size(), elem->Row());
                return false;
            }

            if (defined[idx])
            {
                DPRINTF("idx %d is duplicated (duplication in object at line %d).\n", idx, elem->Row());
                return false;
            }

            defined[idx] = true;

            if (!submeshes[idx].Compile(elem))
                return false;
        }
    }

    valid = true;
    return true;
}

bool G1MGMesh::Read(FixedMemoryStream &in)
{
    char str[17];

    if (!in.Read(str, 16))
        return false;

    str[16] = 0; shader = str;

    if (!in.Read16(&type))
        return false;

    if (!in.Read16(&unk_12))
        return false;

    if (!in.Read32(&external_section_id))
        return false;

    uint32_t count;

    if (!in.Read32(&count))
        return false;

    submeshes.resize(count);

    if (!in.Read(submeshes.data(), count*sizeof(uint32_t)))
        return false;

    return true;
}

bool G1MGMesh::Write(MemoryStream &out) const
{
    char str[17];

    strncpy(str, shader.c_str(), sizeof(str));
    if (!out.Write(str, 16))
        return false;

    if (!out.Write16(type))
        return false;

    if (!out.Write16(unk_12))
        return false;

    if (!out.Write32(external_section_id))
        return false;

    if (!out.Write32((uint32_t)submeshes.size()))
        return false;

    if (!out.Write(submeshes.data(), submeshes.size()*sizeof(uint32_t)))
        return false;

    return true;
}

static bool get_hex_name(const std::string &name, uint32_t *ret)
{
    if (!Utils::BeginsWith(name, "0x", false))
        return false;

    if (name.length() == 2)
        return false;

    for (size_t i = 2; i < name.length(); i++)
    {
        char ch = (char)tolower(name[i]);

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

    *ret = Utils::GetUnsigned(name);
    return true;
}


static bool load_names_buf(const char *buf, std::unordered_map<uint32_t, std::string> &names_map)
{
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

    //UPRINTF("Map loaded with %Id values.\n", names_map.size());
    return true;
}

static bool load_names(const std::string &path, std::unordered_map<uint32_t, std::string> &names_map)
{
    size_t size = Utils::GetFileSize(path);
    if (size == (size_t)-1)
    {
        //DPRINTF("Cannot stat file \"%s\"", path.c_str());
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
            ret = load_names_buf((const char *)buf, names_map);
        }
    }
    else
    {
        //DPRINTF("Cannot open file \"%s\"", path.c_str());
    }

    delete[] buf;
    return ret;
}

TiXmlElement *G1MGMesh::Decompile(TiXmlNode *root, uint32_t idx) const
{
    static std::unordered_map<uint16_t, std::string> types_map =
    {
        { 0, "normal" },
        { 1, "physics1" },
        { 2, "physics2" },
        { 4, "soft" }
    };

    static std::unordered_map<uint32_t, std::string> shader_map;

    if (shader_map.size() == 0)
    {
        load_names("sid_alt_hash.txt", shader_map);
    }

    TiXmlElement *entry_root = new TiXmlElement("Mesh");
    entry_root->SetAttribute("idx", idx);

    if (shader_map.size() > 0 && shader.length() >= 2 && shader.front() == '@')
    {
        uint32_t hash;

        if (sscanf(shader.c_str()+1, "%x", &hash) == 1)
        {
            auto it = shader_map.find(hash);
            if (it != shader_map.end())
            {
                Utils::WriteComment(entry_root, "Shader ID is " + it->second);
            }
        }
    }

    Utils::WriteParamString(entry_root, "Shader", shader);

    auto it = types_map.find(type);
    if (it != types_map.end())
    {
        Utils::WriteParamString(entry_root, "Type", it->second);
    }
    else
    {
        Utils::WriteParamUnsigned(entry_root, "Type", type);
        //DPRINTF("*********Warning: unrecognized type %x.\n", type);
    }

    Utils::WriteParamUnsigned(entry_root, "U_12", unk_12);
    Utils::WriteParamSigned(entry_root, "External_Section_ID", external_section_id);
    Utils::WriteParamMultipleUnsigned(entry_root, "Submeshes", submeshes);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MGMesh::Compile(const TiXmlElement *root)
{
    static std::unordered_map<std::string, uint16_t> types_map =
    {
        { "normal", 0 },
        { "physics1", 1 },
        { "physics2", 2 },
        { "soft", 4 }
    };

    std::string type_str;

    if (!Utils::GetParamString(root, "Shader", shader)) return false;
    if (!Utils::GetParamString(root, "Type", type_str)) return false;
    if (!Utils::GetParamUnsigned(root, "U_12", &unk_12)) return false;
    if (!Utils::GetParamSigned(root, "External_Section_ID", &external_section_id)) return false;
    if (!Utils::GetParamMultipleUnsigned(root, "Submeshes", submeshes)) return false;


    type_str = Utils::ToLowerCase(type_str);
    auto it = types_map.find(type_str);
    if (it != types_map.end())
    {
        type = it->second;
    }
    else
    {
        if (!Utils::HasOnlyDigits(type_str) && !Utils::BeginsWith(type_str, "0x", false))
        {
            DPRINTF("Unrecognized Mesh type \"%s\" (in line %d).\n", type_str.c_str(), root->Row());
            return false;
        }

        type = (uint16_t)Utils::GetUnsigned(type_str);
    }

    if (shader.length() > 16)
    {
        DPRINTF("Invalid Shader \"%s\" (string must be <= 16 characters) (in object at line %d).\n", shader.c_str(), root->Row());
        shader.clear();
        return false;
    }

    return true;
}

bool G1MGLodGroup::CanAutoCalcMeshCounts(const G1MGChunk &g1mg) const
{
    size_t calc_count1 = 0;
    size_t calc_count2 = 0;

    for (const G1MGMesh &mesh : meshes)
    {
        for (uint32_t sm_idx : mesh.submeshes)
        {
            if (sm_idx >= (uint32_t)g1mg.submeshes_section.submeshes.size())
                return false;

            const G1MGSubmesh &sm = g1mg.submeshes_section.submeshes[sm_idx];
            if (sm.flags & 8)
            {
                calc_count2++;
                break;
            }
            else
            {
                if (calc_count2 > 0)
                    return false;

                calc_count1++;
                break;
            }
        }
    }

    if (calc_count1 != count1)
    {
        //DPRINTF("Logic failed %d != %d (count1).\n", calc_count1, count1);
        return false;
    }

    if (calc_count2 != count2)
    {
        //DPRINTF("Logic failed %d != %d (count2).\n", calc_count2, count2);
        return false;
    }

    return true;
}

bool G1MGLodGroup::Read(FixedMemoryStream &in)
{
    if (!in.Read32(&unk_00))
        return false;

    if (!in.Read32(&unk_04))
        return false;

    if (!in.Read32(&unk_08))
        return false;

    if (!in.Read32(&count1))
        return false;

    if (!in.Read32(&count2))
        return false;

    if (!in.Read32(&unk_14))
        return false;

    if (!in.Read32(&unk_18))
        return false;

    if (!in.Read32(&unk_1C))
        return false;

    if (!in.Read32(&unk_20))
        return false;

    meshes.resize(count1+count2);

    for (G1MGMesh &mesh : meshes)
    {
        if (!mesh.Read(in))
            return false;
    }

    return true;
}

bool G1MGLodGroup::Write(MemoryStream &out) const
{
    if (!out.Write32(unk_00))
        return false;

    if (!out.Write32(unk_04))
        return false;

    if (!out.Write32(unk_08))
        return false;

    if (!out.Write32(count1))
        return false;

    if (!out.Write32(count2))
        return false;

    if (!out.Write32(unk_14))
        return false;

    if (!out.Write32(unk_18))
        return false;

    if (!out.Write32(unk_1C))
        return false;

    if (!out.Write32(unk_20))
        return false;

    for (const G1MGMesh &mesh : meshes)
    {
        if (!mesh.Write(out))
            return false;
    }

    return true;
}

TiXmlElement *G1MGLodGroup::Decompile(TiXmlNode *root, const G1MGChunk &g1mg, size_t idx) const
{
    TiXmlElement *entry_root = new TiXmlElement("LodGroup");
    entry_root->SetAttribute("idx", (uint32_t)idx);
    entry_root->SetAttribute("auto", CanAutoCalcMeshCounts(g1mg) ? "true" : "false");

    Utils::WriteParamUnsigned(entry_root, "U_00", unk_00);
    Utils::WriteParamUnsigned(entry_root, "U_04", unk_04);
    Utils::WriteParamUnsigned(entry_root, "U_08", unk_08);
    //Utils::WriteComment(entry_root, "Notice: the sum of Count1+Count2 must equal the number of <Mesh> entries.");
    Utils::WriteParamUnsigned(entry_root, "Count1", count1);
    Utils::WriteParamUnsigned(entry_root, "Count2", count2);
    Utils::WriteParamSigned(entry_root, "S_14", unk_14);
    Utils::WriteParamUnsigned(entry_root, "U_18", unk_18);
    Utils::WriteParamSigned(entry_root, "S_1C", unk_1C);
    Utils::WriteParamUnsigned(entry_root, "U_20", unk_20);

    for (uint32_t i = 0; i < (uint32_t)meshes.size(); i++)
    {
        if (!meshes[i].Decompile(entry_root, i))
            return nullptr;
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MGLodGroup::Compile(const TiXmlElement *root, bool *auto_meshes)
{
    std::string auto_value;

    if (!Utils::ReadAttrString(root, "auto", auto_value))
        auto_value = "true";

    *auto_meshes = Utils::GetBoolean(auto_value);

    if (!Utils::GetParamUnsigned(root, "U_00", &unk_00)) return false;
    if (!Utils::GetParamUnsigned(root, "U_04", &unk_04)) return false;
    if (!Utils::GetParamUnsigned(root, "U_08", &unk_08)) return false;

    if (!(*auto_meshes))
    {
        if (!Utils::GetParamUnsigned(root, "Count1", &count1)) return false;
        if (!Utils::GetParamUnsigned(root, "Count2", &count2)) return false;
    }

    if (!Utils::GetParamSigned(root, "S_14", &unk_14)) return false;
    if (!Utils::GetParamUnsigned(root, "U_18", &unk_18)) return false;
    if (!Utils::GetParamSigned(root, "S_1C", &unk_1C)) return false;
    if (!Utils::GetParamUnsigned(root, "U_20", &unk_20)) return false;

    size_t count = Utils::GetElemCount(root, "Mesh");

    if (!(*auto_meshes) && count != (count1+count2))
    {
        DPRINTF("When auto=false, the number of meshes must equal Count1+Count2. (error in object at line %d).\n", root->Row());
        return false;
    }

    meshes.clear();
    meshes.resize(count);

    std::vector<bool> defined;
    defined.resize(count, false);

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Mesh")
        {
            uint32_t idx;

            if (!Utils::ReadAttrUnsigned(elem, "idx", &idx))
            {
                DPRINTF("Cannot get attribute \"idx\" (in object at line %d).\n", elem->Row());
                return false;
            }

            if (idx >= (uint32_t)meshes.size())
            {
                DPRINTF("idx %d is out of bounds (there are %Id Mesh) (in object at line %d).\n", idx, meshes.size(), elem->Row());
                return false;
            }

            if (defined[idx])
            {
                DPRINTF("idx %d is duplicated (duplication in object at line %d).\n", idx, elem->Row());
                return false;
            }

            defined[idx] = true;

            if (!meshes[idx].Compile(elem))
                return false;
        }
    }

    return true;
}

bool G1MGMeshesSection::Read(FixedMemoryStream &in, uint32_t)
{
    if (!in.Seek(8, SEEK_CUR))
        return false;

    uint32_t num_lods;

    if (!in.Read32(&num_lods))
        return false;   

    groups.resize(num_lods);

    for (G1MGLodGroup &group : groups)
    {
        if (!group.Read(in))
            return false;
    }

    valid = true;
    return true;
}

bool G1MGMeshesSection::Write(MemoryStream &out) const
{
    if (!valid)
        return false;

    uint64_t start = out.Tell();

    if (!out.Write32(0x10009))
        return false;

    if (!out.Write32(0))
        return false;

    if (!out.Write32((uint32_t)groups.size()))
        return false;   

    for (const G1MGLodGroup &group : groups)
    {
        if (!group.Write(out))
            return false;
    }

    uint64_t end = out.Tell();

    if (!out.Seek((off64_t)start + 4, SEEK_SET))
        return false;

    if (!out.Write32((uint32_t)(end-start)))
        return false;

    return out.Seek((off64_t)end, SEEK_SET);
}

TiXmlElement *G1MGMeshesSection::Decompile(TiXmlNode *root, const G1MGChunk &g1mg) const
{
    if (!valid)
        return nullptr;

    TiXmlElement *entry_root = new TiXmlElement("MeshesSection");

    for (uint32_t i = 0; i < (uint32_t)groups.size(); i++)
    {
        Utils::WriteComment(entry_root, "If auto = true, Count1 & Count2 will be calculated automatically, and the meshes re-ordered if needed.");
        if (!groups[i].Decompile(entry_root, g1mg, i))
            return nullptr;
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MGMeshesSection::Compile(const TiXmlElement *root, std::vector<bool> &auto_meshes)
{
    size_t count = Utils::GetElemCount(root, "LodGroup");

    groups.clear();
    groups.resize(count);

    std::vector<bool> defined;

    defined.resize(count, false);
    auto_meshes.clear();
    auto_meshes.resize(count, false);

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "LodGroup")
        {
            uint32_t idx;

            if (!Utils::ReadAttrUnsigned(elem, "idx", &idx))
            {
                DPRINTF("Cannot get attribute \"idx\" (in object at line %d).\n", elem->Row());
                return false;
            }

            if (idx >= (uint32_t)groups.size())
            {
                DPRINTF("idx %d is out of bounds (there are %Id LodGroup) (in object at line %d).\n", idx, groups.size(), elem->Row());
                return false;
            }

            if (defined[idx])
            {
                DPRINTF("idx %d is duplicated (duplication in object at line %d).\n", idx, elem->Row());
                return false;
            }

            defined[idx] = true;
            bool do_auto;

            if (!groups[idx].Compile(elem, &do_auto))
                return false;

            auto_meshes[idx] = do_auto;
        }
    }

    valid = true;
    return true;
}

bool G1MGChunk::Read(FixedMemoryStream &in, uint32_t chunk_version, uint32_t)
{
    version = Utils::GetShortVersion(chunk_version);

    if (version != 44)
    {
        DPRINTF("G1MG: cannot understand this version of chunk: 0x%08X (%d))\n", chunk_version, version);
        return false;
    }

    G1MGChunkHeader *hdr;
    //off64_t chunk_start = in.Tell();

    if (!in.FastRead((uint8_t **)&hdr, sizeof(G1MGChunkHeader)))
        return false;

    platform = hdr->platform;

    min_x = hdr->min_x, min_y = hdr->min_y, min_z = hdr->min_z;
    max_x = hdr->max_x, max_y = hdr->max_y, max_z = hdr->max_z;

    for (uint32_t i = 0; i < hdr->num_sections; i++)
    {
        uint32_t section_type, section_size;
        uint32_t section_start = (uint32_t)in.Tell();

        if (!in.Read32(&section_type))
            return false;

        if (!in.Read32(&section_size))
            return false;

        in.Seek(section_start, SEEK_SET);

        if (section_type == 0x10001)
        {
            if (!unk_section1.Read(in, section_size))
                return false;
        }
        else if (section_type == 0x10002)
        {
            if (!mat_section.Read(in, section_size))
                return false;
        }
        else if (section_type == 0x10003)
        {
            if (!att_section.Read(in, section_size))
                return false;
        }
        else if (section_type == 0x10004)
        {
            if (!vert_section.Read(in, section_size))
                return false;
        }
        else if (section_type == 0x10005)
        {
            if (!layout_section.Read(in, section_size))
                return false;
        }
        else if (section_type == 0x10006)
        {
            if (!bones_map_section.Read(in, section_size))
                return false;
        }
        else if (section_type == 0x10007)
        {
            if (!index_buf_section.Read(in, section_size))
                return false;
        }
        else if (section_type == 0x10008)
        {
            if (!submeshes_section.Read(in, section_size))
                return false;
        }
        else if (section_type == 0x10009)
        {
            if (!meshes_section.Read(in, section_size))
                return false;
        }
        else
        {
            DPRINTF("G1MG: Unknown section 0x%08X at offset %Ix\n", section_type, (size_t)section_start);
            return false;
        }

        if (i != (hdr->num_sections-1))
        {
            if (!in.Seek(section_start+section_size, SEEK_SET))
                return false;
        }
    }

    return true;
}

bool G1MGChunk::Write(MemoryStream &out) const
{
    uint64_t start = out.Tell();
    G1MGChunkHeader hdr;

    memset(&hdr, 0, sizeof(G1MGChunkHeader));
    hdr.signature = G1MG_SIGNATURE;
    hdr.version = Utils::GetLongVersion(version);
    hdr.platform = platform;
    hdr.min_x = min_x, hdr.min_y = min_y, hdr.min_z = min_z;
    hdr.max_x = max_x, hdr.max_y = max_y, hdr.max_z = max_z;

    if (!out.Write(&hdr, sizeof(G1MGChunkHeader)))
        return false;

    if (unk_section1.valid)
    {
        if (!unk_section1.Write(out))
            return false;

        hdr.num_sections++;
    }

    if (mat_section.valid)
    {
        if (!mat_section.Write(out))
            return false;

        hdr.num_sections++;
    }

    if (att_section.valid)
    {
        if (!att_section.Write(out))
            return false;

        hdr.num_sections++;
    }

    if (vert_section.valid)
    {
        if (!vert_section.Write(out))
            return false;

        hdr.num_sections++;
    }

    if (layout_section.valid)
    {
        if (!layout_section.Write(out))
            return false;

        hdr.num_sections++;
    }

    if (bones_map_section.valid)
    {
        if (!bones_map_section.Write(out))
            return false;

        hdr.num_sections++;
    }

    if (index_buf_section.valid)
    {
        if (!index_buf_section.Write(out))
            return false;

        hdr.num_sections++;
    }

    if (submeshes_section.valid)
    {
        if (!submeshes_section.Write(out))
            return false;

        hdr.num_sections++;
    }

    if (meshes_section.valid)
    {
        if (!meshes_section.Write(out))
            return false;

        hdr.num_sections++;
    }

    uint64_t end = out.Tell();

    hdr.chunk_size = (uint32_t)(end - start);

    if (!out.Seek((off64_t)start, SEEK_SET))
        return false;

    if (!out.Write(&hdr, sizeof(G1MGChunkHeader)))
        return false;

    return out.Seek((off64_t)end, SEEK_SET);
}

TiXmlElement *G1MGChunk::Decompile(TiXmlNode *root, const std::string &att_dir, const std::vector<std::string> &bone_names) const
{
    std::string platform_str;
    const char *ptr_plat = (const char *)&platform;

    while (*ptr_plat && platform_str.length() < 4)
    {
        platform_str.push_back(*ptr_plat++);
    }

    TiXmlElement *entry_root = new TiXmlElement("G1MG");
    entry_root->SetAttribute("version", version);
    entry_root->SetAttribute("platform", platform_str);

    std::vector<float> min = { min_x, min_y, min_z };
    std::vector<float> max = { max_x, max_y, max_z };

    Utils::WriteParamMultipleFloats(entry_root, "Min", min);
    Utils::WriteParamMultipleFloats(entry_root, "Max", max);

    if (unk_section1.valid)
    {
        if (!unk_section1.Decompile(entry_root))
            return nullptr;
    }

    if (mat_section.valid)
    {
        if (!mat_section.Decompile(entry_root))
            return nullptr;
    }

    if (att_section.valid)
    {
        if (!att_section.Decompile(entry_root))
            return nullptr;
    }

    if (vert_section.valid)
    {
        if (!vert_section.Decompile(entry_root, att_dir))
            return nullptr;
    }

    if (layout_section.valid)
    {
        if (!layout_section.Decompile(entry_root))
            return nullptr;
    }

    if (bones_map_section.valid)
    {
        if (!bones_map_section.Decompile(entry_root, bone_names))
            return nullptr;
    }

    if (index_buf_section.valid)
    {
        if (!index_buf_section.Decompile(entry_root, att_dir))
            return nullptr;
    }

    if (submeshes_section.valid)
    {
        if (!submeshes_section.Decompile(entry_root))
            return nullptr;
    }

    if (meshes_section.valid)
    {       
        if (!meshes_section.Decompile(entry_root, *this))
            return nullptr;
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool G1MGChunk::Compile(const TiXmlElement *root, const std::string &att_dir, const std::vector<std::string> &bone_names, std::vector<bool> &auto_meshes)
{
    std::vector<float> min, max;
    std::string platform_str;

    if (!Utils::ReadAttrUnsigned(root, "version", &version))
    {
        version = 44;
    }    

    if (version != 44)
    {
        DPRINTF("G1MG: cannot understand this version (%d) (error in object at line %d)\n", version, root->Row());
        return false;
    }

    if (!Utils::ReadAttrString(root, "platform", platform_str))
    {
        DPRINTF("Missing attribute \"platform\" (object at line %d).\n", root->Row());
        return false;
    }

    if (platform_str.length() > 4)
    {
        DPRINTF("G1MG platform can't be more than 4 chars (line %d).\n", root->Row());
        return false;
    }

    platform = 0;
    char *ptr_plat = (char *)&platform;
    for (char ch : platform_str)
        *ptr_plat++ = ch;

    if (!Utils::GetParamMultipleFloats(root, "Min", min)) return false;
    if (!Utils::GetParamMultipleFloats(root, "Max", max)) return false;

    if (min.size() != 3)
    {
        DPRINTF("\"Min\" must have exactly 3 elements (error in object at line %d).\n", root->Row());
        return false;
    }

    if (max.size() != 3)
    {
        DPRINTF("\"Max\" must have exactly 3 elements (error in object at line %d).\n", root->Row());
        return false;
    }

    min_x = min[0]; min_y = min[1]; min_z = min[2];
    max_x = max[0]; max_y = max[1]; max_z = max[2];

    std::vector<bool> defined;
    defined.resize(9, false);

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Section1")
        {
            if (defined[0])
            {
                DPRINTF("Section1 was already defined before. (duplication at line %d).\n", elem->Row());
                return false;
            }

            defined[0] = true;

            if (!unk_section1.Compile(elem))
                return false;
        }
        else if (elem->ValueStr() == "MaterialsSection")
        {
            if (defined[1])
            {
                DPRINTF("MaterialsSection was already defined before. (duplication at line %d).\n", elem->Row());
                return false;
            }

            defined[1] = true;

            if (!mat_section.Compile(elem))
                return false;
        }
        else if (elem->ValueStr() == "AttributesSection")
        {
            if (defined[2])
            {
                DPRINTF("AttributesSection was already defined before. (duplication at line %d).\n", elem->Row());
                return false;
            }

            defined[2] = true;

            if (!att_section.Compile(elem))
                return false;
        }
        else if (elem->ValueStr() == "VertexSection")
        {
            if (defined[3])
            {
                DPRINTF("VertexSection was already defined before. (duplication at line %d).\n", elem->Row());
                return false;
            }

            defined[3] = true;

            if (!vert_section.Compile(elem, att_dir))
                return false;
        }
        else if (elem->ValueStr() == "LayoutSection")
        {
            if (defined[4])
            {
                DPRINTF("LayoutSection was already defined before. (duplication at line %d).\n", elem->Row());
                return false;
            }

            defined[4] = true;

            if (!layout_section.Compile(elem))
                return false;
        }
        else if (elem->ValueStr() == "BonesMapSection")
        {
            if (defined[5])
            {
                DPRINTF("BonesMapSection was already defined before. (duplication at line %d).\n", elem->Row());
                return false;
            }

            defined[5] = true;

            if (!bones_map_section.Compile(elem, bone_names))
                return false;
        }
        else if (elem->ValueStr() == "IndicesSection")
        {
            if (defined[6])
            {
                DPRINTF("IndicesSection was already defined before. (duplication at line %d).\n", elem->Row());
                return false;
            }

            defined[6] = true;

            if (!index_buf_section.Compile(elem, att_dir))
                return false;
        }
        else if (elem->ValueStr() == "SubmeshesSection")
        {
            if (defined[7])
            {
                DPRINTF("SubmeshesSection was already defined before. (duplication at line %d).\n", elem->Row());
                return false;
            }

            defined[7] = true;

            if (!submeshes_section.Compile(elem))
                return false;
        }
        else if (elem->ValueStr() == "MeshesSection")
        {
            if (defined[8])
            {
                DPRINTF("MeshesSection was already defined before. (duplication at line %d).\n", elem->Row());
                return false;
            }

            defined[8] = true;

            if (!meshes_section.Compile(elem, auto_meshes))
                return false;
        }
    }

    return true;
}

bool NUNO1::Read(FixedMemoryStream &in)
{
    uint32_t num_control_points;
    uint32_t num_unk1, num_unk2, num_unk3, num_unk4;
    uint16_t parent_bone_BE;

    if (!in.Read16(&parent_bone))
        return false;

    if (!in.Read16(&parent_bone_BE))
        return false;

    if (parent_bone_BE != 0)
    {
        DPRINTF("%s: unexpected value 0x%04x\n", FUNCNAME, parent_bone_BE);
        return false;
    }

    if (!in.Read32(&num_control_points))
        return false;

    if (!in.Read32(&num_unk1))
        return false;

    if (!in.Read32(&num_unk2))
        return false;

    if (!in.Read32(&num_unk3))
        return false;

    if (!in.Read32(&num_unk4))
        return false;

    if (!in.Read(&unk, sizeof(NUNO1::Unk)))
        return false;

    control_points.resize(num_control_points);
    influences.resize(num_control_points);

    if (num_control_points > 0 && !in.Read(control_points.data(), num_control_points*sizeof(NUNOControlPoint)))
        return false;

    if (num_control_points > 0 && !in.Read(influences.data(), num_control_points*sizeof(NUNOInfluence)))
        return false;

    unk1s.resize(num_unk1);

    if (num_unk1 > 0 && !in.Read(unk1s.data(), num_unk1*sizeof(NUNOUnk1)))
        return false;

    unk2s.resize(num_unk2);

    if (num_unk2 > 0 && !in.Read(unk2s.data(), num_unk2*sizeof(uint32_t)))
        return false;

    unk3s.resize(num_unk3);

    if (num_unk3 > 0 && !in.Read(unk3s.data(), num_unk3*sizeof(uint32_t)))
        return false;

    unk4s.resize(num_unk4);

    if (num_unk4 > 0 && !in.Read(unk4s.data(), num_unk4*sizeof(uint32_t)))
        return false;

    return true;
}

bool NUNO1::Write(MemoryStream &out) const
{
    if (!out.Write16(parent_bone))
        return false;

    if (!out.Write16(0))
        return false;

    if (!out.Write32((uint32_t)control_points.size()))
        return false;

    if (!out.Write32((uint32_t)unk1s.size()))
        return false;

    if (!out.Write32((uint32_t)unk2s.size()))
        return false;

    if (!out.Write32((uint32_t)unk3s.size()))
        return false;

    if (!out.Write32((uint32_t)unk4s.size()))
        return false;

    if (!out.Write(&unk, sizeof(NUNO1::Unk)))
        return false;

    if (control_points.size() != influences.size())
    {
        DPRINTF("%s: Bad state, control_points.size()!= influences.size()\n", FUNCNAME);
        return false;
    }

    if (control_points.size() > 0 && !out.Write(control_points.data(), control_points.size()*sizeof(NUNOControlPoint)))
        return false;

    if (influences.size() > 0 && !out.Write(influences.data(), influences.size()*sizeof(NUNOInfluence)))
        return false;

    if (unk1s.size() > 0 && !out.Write(unk1s.data(), unk1s.size()*sizeof(NUNOUnk1)))
        return false;

    if (unk2s.size() > 0 && !out.Write(unk2s.data(), unk2s.size()*sizeof(uint32_t)))
        return false;

    if (unk3s.size() > 0 && !out.Write(unk3s.data(), unk3s.size()*sizeof(uint32_t)))
        return false;

    if (unk4s.size() > 0 && !out.Write(unk4s.data(), unk4s.size()*sizeof(uint32_t)))
        return false;

    return true;
}

static bool DecompileControlPointsInfluences(TiXmlElement *root, const std::vector<NUNOControlPoint> &control_points, const std::vector<NUNOInfluence> &influences)
{
    Utils::WriteComment(root, "Number of Control_Point: " + Utils::ToString(control_points.size()));

    for (size_t i = 0; i < control_points.size(); i++)
    {
         TiXmlElement *control_point_root = new TiXmlElement("Control_Point");

        std::string value = Utils::FloatToString(control_points[i].x) + ", " + Utils::FloatToString(control_points[i].y) +  ", " +
                            Utils::FloatToString(control_points[i].z) + ", " + Utils::FloatToString(control_points[i].w);


        control_point_root->SetAttribute("value", value);
        control_point_root->SetAttribute("p1", influences[i].P1);
        control_point_root->SetAttribute("p2", influences[i].P2);
        control_point_root->SetAttribute("p3", influences[i].P3);
        control_point_root->SetAttribute("p4", influences[i].P4);
        control_point_root->SetAttribute("p5", Utils::FloatToString(influences[i].P5));
        control_point_root->SetAttribute("p6", Utils::FloatToString(influences[i].P6));

        root->LinkEndChild(control_point_root);
    }

    return true;
}

static void DecompileNunoUnk1(TiXmlElement *root, const std::vector<NUNOUnk1> &unk1s)
{
    Utils::WriteComment(root, "Number of NunoUnk1: " + Utils::ToString(unk1s.size()));

    for (const NUNOUnk1 &unk1 : unk1s)
    {
        TiXmlElement *unk1_root = new TiXmlElement("NunoUnk1");
        Utils::WriteParamMultipleFloats(unk1_root, "F_00", std::vector<float>(unk1.unk_00, unk1.unk_00+8));
        Utils::WriteParamMultipleUnsigned(unk1_root, "U_20", std::vector<uint32_t>(unk1.unk_20, unk1.unk_20+4), true);
        root->LinkEndChild(unk1_root);
    }
}

static bool CompileControlPointInfluence(const TiXmlElement *root, NUNOControlPoint &control_point, NUNOInfluence &influence)
{
    bool ret = true;
    float *vec = &control_point.x;

    if (!Utils::ReadAttrMultipleFloats(root, "value", vec, 4))
        ret = false;

    if (!root->Attribute("p1", &influence.P1))
        ret = false;

    if (!root->Attribute("p2", &influence.P2))
        ret = false;

    if (!root->Attribute("p3", &influence.P3))
        ret = false;

    if (!root->Attribute("p4", &influence.P4))
        ret = false;

    if (!Utils::ReadAttrFloat(root, "p5", &influence.P5))
        ret = false;

    if (!Utils::ReadAttrFloat(root, "p6", &influence.P6))
        ret = false;

    if (!ret)
        DPRINTF("Failed to compile Control_Point at line %d.\n", root->Row());

    return ret;
}

static bool CompileNunoUnk1(const TiXmlElement *root, NUNOUnk1 &unk1)
{
    if (!Utils::GetParamMultipleFloats(root, "F_00", unk1.unk_00, 8)) return false;
    if (!Utils::GetParamMultipleUnsigned(root, "U_20", unk1.unk_20, 4)) return false;

    return true;
}

TiXmlElement *NUNO1::Decompile(TiXmlNode *root, const G1mFile &g1m, size_t idx) const
{
    TiXmlElement *entry_root = new TiXmlElement("Nuno1");
    entry_root->SetAttribute("idx", (uint32_t)idx);
    std::string bone_name;

    if (!g1m.BoneIDToName(parent_bone, bone_name))
    {
        DPRINTF("%s: bone %d couldn't be resolved to a name.\n", FUNCNAME, parent_bone);
        return nullptr;
    }
    else
    {
        entry_root->SetAttribute("parent_bone", bone_name);
    }

    Utils::WriteParamMultipleFloats(entry_root, "F_18", std::vector<float>(unk.unk_18, unk.unk_18+2));
    Utils::WriteParamUnsigned(entry_root, "U_20", unk.unk_20);
    Utils::WriteParamMultipleFloats(entry_root, "F_24", std::vector<float>(unk.unk_24, unk.unk_24+9));
    Utils::WriteParamUnsigned(entry_root, "U_48", unk.unk_48, true);
    Utils::WriteParamMultipleFloats(entry_root, "F_4C", std::vector<float>(unk.unk_4C, unk.unk_4C+6));
    Utils::WriteParamMultipleUnsigned(entry_root, "U_64", std::vector<uint32_t>(unk.unk_64, unk.unk_64+4));

    if (!DecompileControlPointsInfluences(entry_root, control_points, influences))
        return nullptr;

    DecompileNunoUnk1(entry_root, unk1s);

    if (unk2s.size() > 0)
        Utils::WriteParamMultipleUnsigned(entry_root, "NunoUnk2", unk2s);

    if (unk3s.size() > 0)
        Utils::WriteParamMultipleUnsigned(entry_root, "NunoUnk3", unk3s);

    if (unk4s.size() > 0)
        Utils::WriteParamMultipleUnsigned(entry_root, "NunoUnk4", unk4s);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool NUNO1::Compile(const TiXmlElement *root, const G1mFile &g1m)
{
    std::string bone_name;

    if (!Utils::ReadAttrString(root, "parent_bone", bone_name))
    {
        DPRINTF("Cannot get attribute \"parent_bone\" (in object at line %d)\n", root->Row());
        return false;
    }

    int id = g1m.BoneNameToID(bone_name);
    if (id < 0)
    {
       DPRINTF("Cannot find bone \"%s\" (in object at line %d)\n", bone_name.c_str(), root->Row());
       return false;
    }

    parent_bone = (uint16_t)id;

    if (!Utils::GetParamMultipleFloats(root, "F_18", unk.unk_18, 2)) return false;
    if (!Utils::GetParamUnsigned(root, "U_20", &unk.unk_20)) return false;
    if (!Utils::GetParamMultipleFloats(root, "F_24", unk.unk_24, 9)) return false;
    if (!Utils::GetParamUnsigned(root, "U_48", &unk.unk_48)) return false;
    if (!Utils::GetParamMultipleFloats(root, "F_4C", unk.unk_4C, 6)) return false;
    if (!Utils::GetParamMultipleUnsigned(root, "U_64", unk.unk_64, 4)) return false;

    unk2s.clear();
    unk3s.clear();
    unk4s.clear();
    Utils::ReadParamMultipleUnsigned(root, "NunoUnk2", unk2s);
    Utils::ReadParamMultipleUnsigned(root, "NunoUnk3", unk3s);
    Utils::ReadParamMultipleUnsigned(root, "NunoUnk4", unk4s);

    control_points.clear();
    influences.clear();
    unk1s.clear();

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Control_Point")
        {
            NUNOControlPoint control_point;
            NUNOInfluence influence;

            if (!CompileControlPointInfluence(elem, control_point, influence))
                return false;

            control_points.push_back(control_point);
            influences.push_back(influence);
        }       
        else if (elem->ValueStr() == "NunoUnk1")
        {
            NUNOUnk1 unk1;

            if (!CompileNunoUnk1(elem, unk1)) return false;
            unk1s.push_back(unk1);
        }
    }

    return true;
}

bool NUNO2::Read(FixedMemoryStream &in)
{
    uint32_t num_unk11;
    uint32_t num_unk2;

    if (!in.Read16(&parent_bone))
        return false;

    if (!in.Read16(&dummy))
        return false;

    if (!in.Read32(&unk_04))
        return false;

    if (!in.Read32(&num_unk11))
        return false;

    if (!in.Read32(&num_unk2))
        return false;

    if (!in.Read(&unk, sizeof(NUNO2::Unk)))
        return false;

    unk11s.resize(num_unk11);

    if (num_unk11 > 0 && !in.Read(unk11s.data(), num_unk11*sizeof(NUNOUnk11)))
        return false;

    unk2s.resize(num_unk2);

    if (num_unk2 > 0 && !in.Read(unk2s.data(), num_unk2*sizeof(uint32_t)))
        return false;

    return true;
}

bool NUNO2::Write(MemoryStream &out) const
{
    if (!out.Write16(parent_bone))
        return false;

    if (!out.Write16(dummy))
        return false;

    if (!out.Write32(unk_04))
        return false;

    if (!out.Write32((uint32_t)unk11s.size()))
        return false;

    if (!out.Write32((uint32_t)unk2s.size()))
        return false;

    if (!out.Write(&unk, sizeof(NUNO2::Unk)))
        return false;

    if (unk11s.size() > 0 && !out.Write(unk11s.data(), unk11s.size()*sizeof(NUNOUnk11)))
        return false;

    if (unk2s.size() > 0 && !out.Write(unk2s.data(), unk2s.size()*sizeof(uint32_t)))
        return false;

    return true;
}

TiXmlElement *NUNO2::Decompile(TiXmlNode *root, const G1mFile &g1m, size_t idx) const
{
    TiXmlElement *entry_root = new TiXmlElement("Nuno2");
    entry_root->SetAttribute("idx", (uint32_t)idx);

    std::string bone_name;

    if (!g1m.BoneIDToName(parent_bone, bone_name))
    {
        /*DPRINTF("%s: bone %d couldn't be resolved to a name.\n", FUNCNAME, parent_bone);
        return nullptr;*/
        entry_root->SetAttribute("parent_bone", parent_bone);
    }
    else
    {
        entry_root->SetAttribute("parent_bone", bone_name);
    }

    if (dummy != 0)
        Utils::WriteParamUnsigned(entry_root, "dummy", dummy, true);

    Utils::WriteParamSigned(entry_root, "I_04", unk_04);
    Utils::WriteParamMultipleFloats(entry_root, "F_10", std::vector<float>(unk.unk_10, unk.unk_10+9));
    Utils::WriteParamUnsigned(entry_root, "U_34", unk.unk_34, true);
    Utils::WriteParamMultipleFloats(entry_root, "F_38", std::vector<float>(unk.unk_38, unk.unk_38+9));

    for (const NUNOUnk11 &unk11 : unk11s)
    {
        TiXmlElement *unk11_root = new TiXmlElement("NunoUnk11");

        Utils::WriteParamMultipleUnsigned(unk11_root, "U_00", std::vector<uint32_t>(unk11.unk_00, unk11.unk_00+4));
        Utils::WriteParamMultipleFloats(unk11_root, "F_10", std::vector<float>(unk11.unk_10, unk11.unk_10+4));

        entry_root->LinkEndChild(unk11_root);
    }

    if (unk2s.size() > 0)
        Utils::WriteParamMultipleUnsigned(entry_root, "NunoUnk2", unk2s);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool NUNO2::Compile(const TiXmlElement *root, const G1mFile &g1m)
{
    std::string bone_name;

    if (!Utils::ReadAttrString(root, "parent_bone", bone_name))
    {
        DPRINTF("Cannot get attribute \"parent_bone\" (in object at line %d)\n", root->Row());
        return false;
    }

    int id = g1m.BoneNameToID(bone_name);
    if (id < 0)
    {
       if (Utils::HasOnlyDigits(bone_name) || Utils::BeginsWith(bone_name, "0x", false))
       {
           parent_bone = (uint16_t)Utils::GetUnsigned(bone_name);
       }
       else
       {
           DPRINTF("Cannot find bone \"%s\" (in object at line %d)\n", bone_name.c_str(), root->Row());
           return false;
       }
    }
    else
    {
        parent_bone = (uint16_t)id;
    }

    if (!Utils::ReadParamUnsigned(root, "dummy", &dummy)) dummy = 0;
    if (!Utils::GetParamSigned(root, "I_04", &unk_04)) return false;
    if (!Utils::GetParamMultipleFloats(root, "F_10", unk.unk_10, 9)) return false;
    if (!Utils::GetParamUnsigned(root, "U_34", &unk.unk_34)) return false;
    if (!Utils::GetParamMultipleFloats(root, "F_38", unk.unk_38, 9)) return false;

    unk11s.clear();
    unk2s.clear();

    Utils::ReadParamMultipleUnsigned(root, "NunoUnk2", unk2s);

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "NunoUnk11")
        {
            NUNOUnk11 unk11;

            if (!Utils::GetParamMultipleUnsigned(elem, "U_00", unk11.unk_00, 4)) return false;
            if (!Utils::GetParamMultipleFloats(elem, "F_10", unk11.unk_10, 4)) return false;

            unk11s.push_back(unk11);
        }
    }

    return true;
}

bool NUNO3::Read(FixedMemoryStream &in, uint32_t version)
{
    uint32_t num_control_points;
    uint32_t num_unk1, num_unk4, num_unk5, num_unk6;

    this->version = version;

    if (!in.Read16(&parent_bone))
        return false;

    if (!in.Read16(&dummy))
        return false;    

    if (!in.Read32(&num_control_points))
        return false;

    if (!in.Read32(&num_unk1))
        return false;

    if (!in.Read32(&num_unk4))
        return false;

    if (!in.Read32(&unk_10))
        return false;

    if (!in.Read32(&num_unk5))
        return false;

    if (!in.Read32(&num_unk6))
        return false;

    if (!in.Read(unk_1C, sizeof(unk_1C)))
        return false;

    if (!in.Read32(&nun_opcode))
        return false;

    if (version >= 30)
    {
        // Note: only seen 0 in these values. If they are number of something, then when different than 0 this code will be screwed
        if (!in.Read32(&unk_30))
            return false;

        if (!in.Read32(&unk_34))
            return false;
    }

    if (!in.Read(&unk, sizeof(NUNO3::Unk)))
        return false;

    if (version >= 30)
    {
        if (!in.Read(&unk2, sizeof(NUNO3::Unk2)))
            return false;

        if (version >= 32 && nun_opcode == 3)
        {
            if (!in.Read(&unk3, sizeof(NUNO3::Unk3)))
                return false;
        }
    }

    control_points.resize(num_control_points);
    influences.resize(num_control_points);

    if (num_control_points > 0 && !in.Read(control_points.data(), num_control_points*sizeof(NUNOControlPoint)))
        return false;

    if (num_control_points > 0 && !in.Read(influences.data(), num_control_points*sizeof(NUNOInfluence)))
        return false;

    unk1s.resize(num_unk1);

    if (num_unk1 > 0 && !in.Read(unk1s.data(), num_unk1*sizeof(NUNOUnk1)))
        return false;

    unk4s.resize(num_unk4);

    if (num_unk4 > 0 && !in.Read(unk4s.data(), num_unk4*sizeof(uint32_t)))
        return false;

    unk5s.resize(num_unk5);

    if (num_unk5 > 0 && !in.Read(unk5s.data(), num_unk5*sizeof(NUNOUnk5)))
        return false;

    unk6s.resize(num_unk6);

    if (num_unk6 > 0 && !in.Read(unk6s.data(), num_unk6*sizeof(NUNOUnk6)))
        return false;

    return true;
}

bool NUNO3::Write(MemoryStream &out) const
{
    if (!out.Write16(parent_bone))
        return false;

    if (!out.Write16(dummy))
        return false;

    if (!out.Write32((uint32_t)control_points.size()))
        return false;

    if (!out.Write32((uint32_t)unk1s.size()))
        return false;

    if (!out.Write32((uint32_t)unk4s.size()))
        return false;

    if (!out.Write32(unk_10))
        return false;

    if (!out.Write32((uint32_t)unk5s.size()))
        return false;

    if (!out.Write32((uint32_t)unk6s.size()))
        return false;

    if (!out.Write(unk_1C, sizeof(unk_1C)))
        return false;

    if (!out.Write32(nun_opcode))
        return false;

    if (version >= 30)
    {
        // Note: only seen 0 in these values. If they are number of something, then when different than 0 this code will be screwed
        if (!out.Write32(unk_30))
            return false;

        if (!out.Write32(unk_34))
            return false;
    }

    if (!out.Write(&unk, sizeof(NUNO3::Unk)))
        return false;

    if (version >= 30)
    {
        if (!out.Write(&unk2, sizeof(NUNO3::Unk2)))
            return false;

        if (version >= 32 && nun_opcode == 3)
        {
            if (!out.Write(&unk3, sizeof(NUNO3::Unk3)))
                return false;
        }
    }

    if (control_points.size() != influences.size())
    {
        DPRINTF("%s: Bad state, control_points.size()!= influences.size()\n", FUNCNAME);
        return false;
    }

    if (control_points.size() > 0 && !out.Write(control_points.data(), control_points.size()*sizeof(NUNOControlPoint)))
        return false;

    if (influences.size() > 0 && !out.Write(influences.data(), influences.size()*sizeof(NUNOInfluence)))
        return false;

    if (unk1s.size() > 0 && !out.Write(unk1s.data(), unk1s.size()*sizeof(NUNOUnk1)))
        return false;

    if (unk4s.size() > 0 && !out.Write(unk4s.data(), unk4s.size()*sizeof(uint32_t)))
        return false;

    if (unk5s.size() > 0 && !out.Write(unk5s.data(), unk5s.size()*sizeof(NUNOUnk5)))
        return false;

    if (unk6s.size() > 0 && !out.Write(unk6s.data(), unk6s.size()*sizeof(NUNOUnk6)))
        return false;

    return true;
}

TiXmlElement *NUNO3::Decompile(TiXmlNode *root, const G1mFile &g1m, size_t idx) const
{
    TiXmlElement *entry_root = new TiXmlElement("Nuno3");
    entry_root->SetAttribute("idx", (uint32_t)idx);

    std::string bone_name;

    if (!g1m.BoneIDToName(parent_bone, bone_name))
    {
        /*DPRINTF("%s: bone %d couldn't be resolved to a name.\n", FUNCNAME, parent_bone);
        return nullptr;*/
        entry_root->SetAttribute("parent_bone", parent_bone);
    }
    else
    {
        entry_root->SetAttribute("parent_bone", bone_name);
    }

    if (dummy != 0)
        Utils::WriteParamUnsigned(entry_root, "dummy", dummy, true);

    Utils::WriteParamUnsigned(entry_root, "U_10", unk_10);
    Utils::WriteParamMultipleUnsigned(entry_root, "U_1C", std::vector<uint32_t>(unk_1C, unk_1C+4), true);
    Utils::WriteParamUnsigned(entry_root, "Opcode", nun_opcode);

    if (version >= 30)
    {
        Utils::WriteParamUnsigned(entry_root, "U_30", unk_30);
        Utils::WriteParamUnsigned(entry_root, "U_34", unk_34);
    }

    Utils::WriteParamMultipleFloats(entry_root, "F_30", std::vector<float>(unk.unk_30, unk.unk_30+9));
    Utils::WriteParamUnsigned(entry_root, "U_54", unk.unk_54, true);
    Utils::WriteParamUnsigned(entry_root, "U_58", unk.unk_58, true);
    Utils::WriteParamMultipleFloats(entry_root, "F_5C", std::vector<float>(unk.unk_5C, unk.unk_5C+11));
    Utils::WriteParamMultipleUnsigned(entry_root, "U_88", std::vector<uint32_t>(unk.unk_88, unk.unk_88+4), true);
    Utils::WriteParamMultipleFloats(entry_root, "F_98", std::vector<float>(unk.unk_98, unk.unk_98+7));
    Utils::WriteParamUnsigned(entry_root, "U_B4", unk.unk_B4, true);
    Utils::WriteParamFloat(entry_root, "F_B8", unk.unk_B8);
    Utils::WriteParamMultipleUnsigned(entry_root, "U_BC", std::vector<uint32_t>(unk.unk_BC, unk.unk_BC+7), true);

    if (version >= 30)
    {
        Utils::WriteParamMultipleUnsigned(entry_root, "U_E0", std::vector<uint32_t>(unk2.unk_E0, unk2.unk_E0+4));
        Utils::WriteParamSigned(entry_root, "I_F0", unk2.unk_F0);

        if (version >= 32 && nun_opcode == 3)
        {
            Utils::WriteParamMultipleFloats(entry_root, "F_F4", std::vector<float>(unk3.unk_F4, unk3.unk_F4+5));
        }
    }

    if (!DecompileControlPointsInfluences(entry_root, control_points, influences))
        return nullptr;

    DecompileNunoUnk1(entry_root, unk1s);

    if (unk4s.size() > 0)
        Utils::WriteParamMultipleUnsigned(entry_root, "NunoUnk4", unk4s);

    for (const NUNOUnk5 &unk5 : unk5s)
    {
        TiXmlElement *unk1_root = new TiXmlElement("NunoUnk5");
        unk1_root->SetAttribute("u_00", unk5.unk_00);
        unk1_root->SetAttribute("u_04", unk5.unk_04);
        entry_root->LinkEndChild(unk1_root);
    }

    for (const NUNOUnk6 &unk6 : unk6s)
    {
        TiXmlElement *unk1_root = new TiXmlElement("NunoUnk6");
        unk1_root->SetAttribute("u_00", unk6.unk_00);
        unk1_root->SetAttribute("u_04", unk6.unk_04);
        unk1_root->SetAttribute("f_08", Utils::FloatToString(unk6.unk_08));
        entry_root->LinkEndChild(unk1_root);
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool NUNO3::Compile(const TiXmlElement *root, const G1mFile &g1m, uint32_t version)
{
    this->version = version;

    std::string bone_name;

    if (!Utils::ReadAttrString(root, "parent_bone", bone_name))
    {
        DPRINTF("Cannot get attribute \"parent_bone\" (in object at line %d)\n", root->Row());
        return false;
    }

    int id = g1m.BoneNameToID(bone_name);
    if (id < 0)
    {
       if (Utils::HasOnlyDigits(bone_name) || Utils::BeginsWith(bone_name, "0x", false))
       {
           parent_bone = (uint16_t)Utils::GetUnsigned(bone_name);
       }
       else
       {
           DPRINTF("Cannot find bone \"%s\" (in object at line %d)\n", bone_name.c_str(), root->Row());
           return false;
       }
    }
    else
    {
        parent_bone = (uint16_t)id;
    }

    if (!Utils::ReadParamUnsigned(root, "dummy", &dummy)) dummy = 0;
    if (!Utils::GetParamUnsigned(root, "U_10", &unk_10)) return false;
    if (!Utils::GetParamMultipleUnsigned(root, "U_1C", unk_1C, 4)) return false;
    if (!Utils::GetParamUnsigned(root, "Opcode", &nun_opcode)) return false;

    if (version >= 30)
    {
        if (!Utils::GetParamUnsigned(root, "U_30", &unk_30)) return false;
        if (!Utils::GetParamUnsigned(root, "U_34", &unk_34)) return false;
    }

    if (!Utils::GetParamMultipleFloats(root, "F_30", unk.unk_30, 9)) return false;
    if (!Utils::GetParamUnsigned(root, "U_54", &unk.unk_54)) return false;
    if (!Utils::GetParamUnsigned(root, "U_58", &unk.unk_58)) return false;
    if (!Utils::GetParamMultipleFloats(root, "F_5C", unk.unk_5C, 11)) return false;
    if (!Utils::GetParamMultipleUnsigned(root, "U_88", unk.unk_88, 4)) return false;
    if (!Utils::GetParamMultipleFloats(root, "F_98", unk.unk_98, 7)) return false;
    if (!Utils::GetParamUnsigned(root, "U_B4", &unk.unk_B4)) return false;
    if (!Utils::GetParamFloat(root, "F_B8", &unk.unk_B8)) return false;
    if (!Utils::GetParamMultipleUnsigned(root, "U_BC", unk.unk_BC, 7)) return false;

    if (version >= 30)
    {
        if (!Utils::GetParamMultipleUnsigned(root, "U_E0", unk2.unk_E0, 4)) return false;
        if (!Utils::GetParamSigned(root, "I_F0", &unk2.unk_F0)) return false;

        if (version >= 32 && nun_opcode == 3)
        {
            if (!Utils::GetParamMultipleFloats(root, "F_F4", unk3.unk_F4, 5)) return false;
        }
    }

    control_points.clear();
    influences.clear();
    unk1s.clear();
    unk4s.clear();
    unk5s.clear();
    unk6s.clear();

    Utils::ReadParamMultipleUnsigned(root, "NunoUnk4", unk4s);

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Control_Point")
        {
            NUNOControlPoint control_point;
            NUNOInfluence influence;

            if (!CompileControlPointInfluence(elem, control_point, influence))
                return false;

            control_points.push_back(control_point);
            influences.push_back(influence);
        }
        else if (elem->ValueStr() == "NunoUnk1")
        {
            NUNOUnk1 unk1;

            if (!CompileNunoUnk1(elem, unk1)) return false;
            unk1s.push_back(unk1);
        }
        else if (elem->ValueStr() == "NunoUnk5")
        {
            NUNOUnk5 unk5;

            if (!Utils::ReadAttrUnsigned(elem, "u_00", &unk5.unk_00))
            {
                DPRINTF("Missing attribute \"u_00\" (in object at line %d).\n", elem->Row());
                return false;
            }

            if (!Utils::ReadAttrUnsigned(elem, "u_04", &unk5.unk_04))
            {
                DPRINTF("Missing attribute \"u_04\" (in object at line %d).\n", elem->Row());
                return false;
            }

            unk5s.push_back(unk5);
        }
        else if (elem->ValueStr() == "NunoUnk6")
        {
            NUNOUnk6 unk6;

            if (!Utils::ReadAttrUnsigned(elem, "u_00", &unk6.unk_00))
            {
                DPRINTF("Missing attribute \"u_00\" (in object at line %d).\n", elem->Row());
                return false;
            }

            if (!Utils::ReadAttrUnsigned(elem, "u_04", &unk6.unk_04))
            {
                DPRINTF("Missing attribute \"u_04\" (in object at line %d).\n", elem->Row());
                return false;
            }

            if (!Utils::ReadAttrFloat(elem, "f_08", &unk6.unk_08))
            {
                DPRINTF("Missing attribute \"f_08\" (in object at line %d).\n", elem->Row());
                return false;
            }

            unk6s.push_back(unk6);
        }
    }

    return true;
}

bool NUNO4::Read(FixedMemoryStream &in, uint32_t version)
{
    uint32_t num_unk7, num_unk8, num_unk9, num_unk10;

    this->version = version;

    if (version < 29)
    {
        DPRINTF("%s: Unexpected version %d.\n", FUNCNAME, version);
        return false;
    }

    if (!in.Read32(&parent_bone))
        return false;

    if (!in.Read32(&num_unk7))
        return false;

    if (!in.Read32(&num_unk8))
        return false;

    if (!in.Read32(&num_unk9))
        return false;

    if (!in.Read32(&num_unk10))
        return false;

    if (version == 29)
    {
        if (!in.Read(&u29, sizeof(u29)))
            return false;
    }
    else if (version >= 30)
    {
        if (!in.Read(&u30, sizeof(u30)))
            return false;
    }

    unk7s.resize(num_unk7);
    unk8s.resize(num_unk8);
    unk9s.resize(num_unk9);
    unk10s.resize(num_unk10);

    if (num_unk7 > 0 && !in.Read(unk7s.data(), num_unk7*sizeof(NUNOUnk7)))
        return false;

    if (num_unk8 > 0 && !in.Read(unk8s.data(), num_unk8*sizeof(NUNOUnk8)))
        return false;

    if (num_unk9 > 0 && !in.Read(unk9s.data(), num_unk9*sizeof(uint32_t)))
        return false;

    if (num_unk10 && !in.Read(unk10s.data(), num_unk10*sizeof(NUNOUnk10)))
        return false;

    return true;
}

bool NUNO4::Write(MemoryStream &out) const
{
    if (!out.Write32(parent_bone))
        return false;

    if (!out.Write32((uint32_t)unk7s.size()))
        return false;

    if (!out.Write32((uint32_t)unk8s.size()))
        return false;

    if (!out.Write32((uint32_t)unk9s.size()))
        return false;

    if (!out.Write32((uint32_t)unk10s.size()))
        return false;

    if (version == 29)
    {
        if (!out.Write(&u29, sizeof(u29)))
            return false;
    }
    else if (version >= 30)
    {
        if (!out.Write(&u30, sizeof(u30)))
            return false;
    }

    if (unk7s.size() > 0 && !out.Write(unk7s.data(), unk7s.size()*sizeof(NUNOUnk7)))
        return false;

    if (unk8s.size() > 0 && !out.Write(unk8s.data(), unk8s.size()*sizeof(NUNOUnk8)))
        return false;

    if (unk9s.size() > 0 && !out.Write(unk9s.data(), unk9s.size()*sizeof(uint32_t)))
        return false;

    if (unk10s.size() > 0 && !out.Write(unk10s.data(), unk10s.size()*sizeof(NUNOUnk10)))
        return false;

    return true;
}

TiXmlElement *NUNO4::Decompile(TiXmlNode *root, const G1mFile &g1m, size_t idx) const
{
    TiXmlElement *entry_root = new TiXmlElement("Nuno4");
    entry_root->SetAttribute("idx", (uint32_t)idx);

    std::string bone_name;

    if (!g1m.BoneIDToName(parent_bone, bone_name))
    {
        DPRINTF("%s: bone %d couldn't be resolved to a name.\n", FUNCNAME, parent_bone);
        return nullptr;
    }
    else
    {
        entry_root->SetAttribute("parent_bone", bone_name);
    }

    if (version < 30)
    {
        Utils::WriteParamFloat(entry_root, "F_14", u29.unk_14);
        Utils::WriteParamUnsigned(entry_root, "U_18", u29.unk_18);
        Utils::WriteParamMultipleFloats(entry_root, "F_1C", std::vector<float>(u29.unk_1C, u29.unk_1C+5));
        Utils::WriteParamUnsigned(entry_root, "U_30", u29.unk_30);
        Utils::WriteParamMultipleFloats(entry_root, "F_34", std::vector<float>(u29.unk_34, u29.unk_34+16));
    }
    else
    {
        Utils::WriteParamUnsigned(entry_root, "U_14", u30.unk_14);
        Utils::WriteParamUnsigned(entry_root, "U_18", u30.unk_18);
        Utils::WriteParamFloat(entry_root, "F_1C", u30.unk_1C);
        Utils::WriteParamUnsigned(entry_root, "U_20", u30.unk_20);
        Utils::WriteParamMultipleFloats(entry_root, "F_24", std::vector<float>(u30.unk_24, u30.unk_24+5));
        Utils::WriteParamUnsigned(entry_root, "U_38", u30.unk_38);
        Utils::WriteParamMultipleFloats(entry_root, "F_3C", std::vector<float>(u30.unk_3C, u30.unk_3C+14));
        Utils::WriteParamUnsigned(entry_root, "U_74", u30.unk_74);
        Utils::WriteParamMultipleFloats(entry_root, "F_78", std::vector<float>(u30.unk_78, u30.unk_78+5));
        Utils::WriteParamSigned(entry_root, "U_8C", u30.unk_8C);
        Utils::WriteParamMultipleFloats(entry_root, "F_90", std::vector<float>(u30.unk_90, u30.unk_90+11));
    }

    for (const NUNOUnk7 &unk7 : unk7s)
    {
        TiXmlElement *unk7_root = new TiXmlElement("NunoUnk7");
        Utils::WriteParamMultipleFloats(unk7_root, "F_00", std::vector<float>(unk7.unk_00, unk7.unk_00+7));
        entry_root->LinkEndChild(unk7_root);
    }

    for (const NUNOUnk8 &unk8: unk8s)
    {
        TiXmlElement *unk8_root = new TiXmlElement("NunoUnk8");
        unk8_root->SetAttribute("f_00", Utils::FloatToString(unk8.unk_00));
        unk8_root->SetAttribute("f_04", Utils::FloatToString(unk8.unk_04));
        entry_root->LinkEndChild(unk8_root);
    }

    if (unk9s.size() > 0)
        Utils::WriteParamMultipleUnsigned(entry_root, "NunoUnk9", unk9s);

    for (const NUNOUnk10 &unk10 : unk10s)
    {
        TiXmlElement *unk10_root = new TiXmlElement("NunoUnk10");
        unk10_root->SetAttribute("u_00", unk10.unk_00);
        unk10_root->SetAttribute("u_04", unk10.unk_04);
        entry_root->LinkEndChild(unk10_root);
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool NUNO4::Compile(const TiXmlElement *root, const G1mFile &g1m, uint32_t version)
{
    this->version = version;

    std::string bone_name;

    if (!Utils::ReadAttrString(root, "parent_bone", bone_name))
    {
        DPRINTF("Cannot get attribute \"parent_bone\" (in object at line %d)\n", root->Row());
        return false;
    }

    int id = g1m.BoneNameToID(bone_name);
    if (id < 0)
    {
       DPRINTF("Cannot find bone \"%s\" (in object at line %d)\n", bone_name.c_str(), root->Row());
       return false;
    }

    parent_bone = (uint16_t)id;

    if (version < 30)
    {
        if (!Utils::GetParamFloat(root, "F_14", &u29.unk_14)) return false;
        if (!Utils::GetParamUnsigned(root, "U_18", &u29.unk_18)) return false;
        if (!Utils::GetParamMultipleFloats(root, "F_1C", u29.unk_1C, 5)) return false;
        if (!Utils::GetParamUnsigned(root, "U_30", &u29.unk_30)) return false;
        if (!Utils::GetParamMultipleFloats(root, "F_34", u29.unk_34, 16)) return false;
    }
    else
    {
        if (!Utils::GetParamUnsigned(root, "U_14", &u30.unk_14)) return false;
        if (!Utils::GetParamUnsigned(root, "U_18", &u30.unk_18)) return false;
        if (!Utils::GetParamFloat(root, "F_1C", &u30.unk_1C)) return false;
        if (!Utils::GetParamUnsigned(root, "U_20", &u30.unk_20)) return false;
        if (!Utils::GetParamMultipleFloats(root, "F_24", u30.unk_24, 5)) return false;
        if (!Utils::GetParamUnsigned(root, "U_38", &u30.unk_38)) return false;
        if (!Utils::GetParamMultipleFloats(root, "F_3C", u30.unk_3C, 14)) return false;
        if (!Utils::GetParamUnsigned(root, "U_74", &u30.unk_74)) return false;
        if (!Utils::GetParamMultipleFloats(root, "F_78", u30.unk_78, 5)) return false;
        if (!Utils::GetParamSigned(root, "U_8C", &u30.unk_8C)) return false;
        if (!Utils::GetParamMultipleFloats(root, "F_90", u30.unk_90, 11)) return false;
    }

    unk7s.clear();
    unk8s.clear();
    unk9s.clear();
    unk10s.clear();

    Utils::ReadParamMultipleUnsigned(root, "NunoUnk9", unk9s);

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "NunoUnk7")
        {
            NUNOUnk7 unk7;
            if (!Utils::GetParamMultipleFloats(elem, "F_00", unk7.unk_00, 7)) return false;
            unk7s.push_back(unk7);
        }
        else if (elem->ValueStr() == "NunoUnk8")
        {
            NUNOUnk8 unk8;

            if (!Utils::ReadAttrFloat(elem, "f_00", &unk8.unk_00)) return false;
            if (!Utils::ReadAttrFloat(elem, "f_04", &unk8.unk_04)) return false;

            unk8s.push_back(unk8);
        }
        else if (elem->ValueStr() == "NunoUnk10")
        {
            NUNOUnk10 unk10;

            if (!Utils::ReadAttrUnsigned(elem, "u_00", &unk10.unk_00)) return false;
            if (!Utils::ReadAttrUnsigned(elem, "u_04", &unk10.unk_04)) return false;

            unk10s.push_back(unk10);
        }
    }

    return true;
}

bool NUNOChunk::Read(FixedMemoryStream &in, uint32_t chunk_version, uint32_t)
{
    version = Utils::GetShortVersion(chunk_version);

    if (version != 29 && version != 30 && version != 32)
    {
        DPRINTF("NUNO: cannot understand this version of chunk (0x%08X, aka %d)\n", chunk_version, version);
        return false;
    }

    NUNChunkHeader hdr;

    if (!in.Read(&hdr, sizeof(NUNChunkHeader)))
        return false;

    nuno1s.clear();
    nuno2s.clear();
    nuno3s.clear();
    nuno4s.clear();

    for (uint32_t n = 0; n < hdr.section_count; n++)
    {
        uint64_t section_start = in.Tell();
        NUNSectionHeader sec_hdr;

        if (!in.Read(&sec_hdr, sizeof(NUNSectionHeader)))
            return false;

        if (sec_hdr.id == 0x30001)
        {
            nuno1s.resize(sec_hdr.count);
        }
        else if (sec_hdr.id == 0x30002)
        {
            nuno2s.resize(sec_hdr.count);
        }
        else if (sec_hdr.id == 0x30003)
        {
            nuno3s.resize(sec_hdr.count);
        }
        else if (sec_hdr.id == 0x30004)
        {
            nuno4s.resize(sec_hdr.count);
        }
        else
        {
            DPRINTF("Unknown NUNO section 0x%x\n", sec_hdr.id);
            return false;
        }

        for (uint32_t i = 0; i < sec_hdr.count; i++)
        {
            //DPRINTF("Going to read 0x%x. Starting at offset 0x%Ix\n", sec_hdr.id, (size_t)in.Tell());

            if (sec_hdr.id == 0x30001)
            {
                if (!nuno1s[i].Read(in))
                    return false;
            }
            else if (sec_hdr.id == 0x30002)
            {
                if (!nuno2s[i].Read(in))
                    return false;
            }
            else if (sec_hdr.id == 0x30003)
            {
                if (!nuno3s[i].Read(in, version))
                    return false;
            }
            if (sec_hdr.id == 0x30004)
            {
                if (!nuno4s[i].Read(in, version))
                    return false;
            }
        }

        in.Seek(section_start + sec_hdr.size, SEEK_SET);
    }

    return true;
}

bool NUNOChunk::Write(MemoryStream &out) const
{
    uint64_t start = out.Tell();
    NUNChunkHeader hdr;

    hdr.signature = NUNO_SIGNATURE;
    hdr.version = Utils::GetLongVersion(version);
    hdr.size = 0;
    hdr.section_count = 0;

    if (!out.Write(&hdr, sizeof(hdr)))
        return false;

    if (nuno1s.size() > 0)
    {
        NUNSectionHeader shdr;
        uint64_t section_start = out.Tell();

        shdr.id = 0x30001;
        shdr.count = (uint32_t)nuno1s.size();

        if (!out.Write(&shdr, sizeof(NUNSectionHeader)))
            return false;

        for (const NUNO1 &n1 : nuno1s)
        {
            if (!n1.Write(out))
                return false;
        }

        uint64_t section_end = out.Tell();
        shdr.size = (uint32_t)(section_end - section_start);

        out.Seek(section_start, SEEK_SET);

        if (!out.Write(&shdr, sizeof(NUNSectionHeader)))
            return false;

        out.Seek(section_end, SEEK_SET);
        hdr.section_count++;
    }

    if (nuno2s.size() > 0)
    {
        NUNSectionHeader shdr;
        uint64_t section_start = out.Tell();

        shdr.id = 0x30002;
        shdr.count = (uint32_t)nuno2s.size();

        if (!out.Write(&shdr, sizeof(NUNSectionHeader)))
            return false;

        for (const NUNO2 &n2 : nuno2s)
        {
            if (!n2.Write(out))
                return false;
        }

        uint64_t section_end = out.Tell();
        shdr.size = (uint32_t)(section_end - section_start);

        out.Seek(section_start, SEEK_SET);

        if (!out.Write(&shdr, sizeof(NUNSectionHeader)))
            return false;

        out.Seek(section_end, SEEK_SET);
        hdr.section_count++;
    }

    if (nuno3s.size() > 0)
    {
        NUNSectionHeader shdr;
        uint64_t section_start = out.Tell();

        shdr.id = 0x30003;
        shdr.count = (uint32_t)nuno3s.size();

        if (!out.Write(&shdr, sizeof(NUNSectionHeader)))
            return false;

        for (const NUNO3 &n3 : nuno3s)
        {
            if (!n3.Write(out))
                return false;
        }

        uint64_t section_end = out.Tell();
        shdr.size = (uint32_t)(section_end - section_start);

        out.Seek(section_start, SEEK_SET);

        if (!out.Write(&shdr, sizeof(NUNSectionHeader)))
            return false;

        out.Seek(section_end, SEEK_SET);
        hdr.section_count++;
    }

    if (nuno4s.size() > 0)
    {
        NUNSectionHeader shdr;
        uint64_t section_start = out.Tell();

        shdr.id = 0x30004;
        shdr.count = (uint32_t)nuno4s.size();

        if (!out.Write(&shdr, sizeof(NUNSectionHeader)))
            return false;

        for (const NUNO4 &n4 : nuno4s)
        {
            if (!n4.Write(out))
                return false;
        }

        uint64_t section_end = out.Tell();
        shdr.size = (uint32_t)(section_end - section_start);

        out.Seek(section_start, SEEK_SET);

        if (!out.Write(&shdr, sizeof(NUNSectionHeader)))
            return false;

        out.Seek(section_end, SEEK_SET);
        hdr.section_count++;
    }

    uint64_t end = out.Tell();
    hdr.size = (uint32_t)(end - start);

    if (!out.Seek((off64_t)start, SEEK_SET))
        return false;

    if (!out.Write(&hdr, sizeof(hdr)))
        return false;

    return out.Seek((off64_t)end, SEEK_SET);
}

TiXmlElement *NUNOChunk::Decompile(TiXmlNode *root, const G1mFile &g1m) const
{
    TiXmlElement *entry_root = new TiXmlElement("NUNO");
    entry_root->SetAttribute("version", version);

    for (size_t i = 0; i < nuno1s.size(); i++)
    {
        if (!nuno1s[i].Decompile(entry_root, g1m, i))
            return nullptr;
    }

    for (size_t i = 0; i < nuno2s.size(); i++)
    {
        if (!nuno2s[i].Decompile(entry_root, g1m, i))
            return nullptr;
    }

    for (size_t i = 0; i < nuno3s.size(); i++)
    {
        if (!nuno3s[i].Decompile(entry_root, g1m, i))
            return nullptr;
    }

    for (size_t i = 0; i < nuno4s.size(); i++)
    {
        if (!nuno4s[i].Decompile(entry_root, g1m, i))
            return nullptr;
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool NUNOChunk::Compile(const TiXmlElement *root, const G1mFile &g1m)
{
    if (!Utils::ReadAttrUnsigned(root, "version", &version))
        version = 29;

    if (version != 29 && version != 30 && version != 32)
    {
        DPRINTF("Cannot understand NUNO version %d.\n", version);
        return false;
    }

    nuno1s.clear();
    nuno2s.clear();
    nuno3s.clear();
    nuno4s.clear();

    nuno1s.resize(Utils::GetElemCount(root, "Nuno1"));
    nuno2s.resize(Utils::GetElemCount(root, "Nuno2"));
    nuno3s.resize(Utils::GetElemCount(root, "Nuno3"));
    nuno4s.resize(Utils::GetElemCount(root, "Nuno4"));

    std::vector<bool> nuno1s_compiled, nuno2s_compiled, nuno3s_compiled, nuno4s_compiled;

    nuno1s_compiled.resize(nuno1s.size(), false);
    nuno2s_compiled.resize(nuno2s.size(), false);
    nuno3s_compiled.resize(nuno3s.size(), false);
    nuno4s_compiled.resize(nuno4s.size(), false);

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Nuno1")
        {
            uint32_t idx;

            if (!Utils::ReadAttrUnsigned(elem, "idx", &idx))
            {
                DPRINTF("Missing attribute \"idx\" (in object at line %d).\n", elem->Row());
                return false;
            }

            if (idx >= nuno1s.size())
            {
                DPRINTF("\"idx\" out of bounds (in object at line %d).\n", elem->Row());
                return false;
            }

            if (nuno1s_compiled[idx])
            {
                DPRINTF("Duplicate Nuno1 idx %d (duplication at line %d)\n", idx, elem->Row());
                return false;
            }

            if (!nuno1s[idx].Compile(elem, g1m))
                return false;

            nuno1s_compiled[idx] = true;
        }
        else if (elem->ValueStr() == "Nuno2")
        {
            uint32_t idx;

            if (!Utils::ReadAttrUnsigned(elem, "idx", &idx))
            {
                DPRINTF("Missing attribute \"idx\" (in object at line %d).\n", elem->Row());
                return false;
            }

            if (idx >= nuno2s.size())
            {
                DPRINTF("\"idx\" out of bounds (in object at line %d).\n", elem->Row());
                return false;
            }

            if (nuno2s_compiled[idx])
            {
                DPRINTF("Duplicate Nuno2 idx %d (duplication at line %d)\n", idx, elem->Row());
                return false;
            }

            if (!nuno2s[idx].Compile(elem, g1m))
                return false;

            nuno2s_compiled[idx] = true;
        }
        else if (elem->ValueStr() == "Nuno3")
        {
            uint32_t idx;

            if (!Utils::ReadAttrUnsigned(elem, "idx", &idx))
            {
                DPRINTF("Missing attribute \"idx\" (in object at line %d).\n", elem->Row());
                return false;
            }

            if (idx >= nuno3s.size())
            {
                DPRINTF("\"idx\" out of bounds (in object at line %d).\n", elem->Row());
                return false;
            }

            if (nuno3s_compiled[idx])
            {
                DPRINTF("Duplicate Nuno3 idx %d (duplication at line %d)\n", idx, elem->Row());
                return false;
            }

            if (!nuno3s[idx].Compile(elem, g1m, version))
                return false;

            nuno3s_compiled[idx] = true;
        }
        else if (elem->ValueStr() == "Nuno4")
        {
            uint32_t idx;

            if (!Utils::ReadAttrUnsigned(elem, "idx", &idx))
            {
                DPRINTF("Missing attribute \"idx\" (in object at line %d).\n", elem->Row());
                return false;
            }

            if (idx >= nuno4s.size())
            {
                DPRINTF("\"idx\" out of bounds (in object at line %d).\n", elem->Row());
                return false;
            }

            if (nuno4s_compiled[idx])
            {
                DPRINTF("Duplicate Nuno41 idx %d (duplication at line %d)\n", idx, elem->Row());
                return false;
            }

            if (!nuno4s[idx].Compile(elem, g1m, version))
                return false;

            nuno4s_compiled[idx] = true;
        }
    }

    return true;
}

bool NUNV1::Read(FixedMemoryStream &in)
{
    uint32_t num_control_points;
    uint32_t num_unk1, num_unk2;
    uint16_t parent_bone_BE;

    if (!in.Read16(&parent_bone))
        return false;

    if (!in.Read16(&parent_bone_BE))
        return false;

    if (parent_bone_BE != 0)
    {
        DPRINTF("%s: unexpected value 0x%04x\n", FUNCNAME, parent_bone_BE);
        return false;
    }

    if (!in.Read32(&num_control_points))
        return false;

    if (!in.Read32(&num_unk1))
        return false;

    if (!in.Read32(&num_unk2))
        return false;

    if (!in.Read(&unk, sizeof(NUNV1::Unk)))
        return false;

    control_points.resize(num_control_points);
    influences.resize(num_control_points);

    if (num_control_points > 0 && !in.Read(control_points.data(), num_control_points*sizeof(NUNOControlPoint)))
        return false;

    if (num_control_points > 0 && !in.Read(influences.data(), num_control_points*sizeof(NUNOInfluence)))
        return false;

    unk1s.resize(num_unk1);

    if (num_unk1 > 0 && !in.Read(unk1s.data(), num_unk1*sizeof(NUNOUnk1)))
        return false;

    unk4s.resize(num_unk2);

    if (num_unk2 > 0 && !in.Read(unk4s.data(), num_unk2*sizeof(uint32_t)))
        return false;

    return true;
}

bool NUNV1::Write(MemoryStream &out) const
{
    if (!out.Write16(parent_bone))
        return false;

    if (!out.Write16(0))
        return false;

    if (!out.Write32((uint32_t)control_points.size()))
        return false;

    if (!out.Write32((uint32_t)unk1s.size()))
        return false;

    if (!out.Write32((uint32_t)unk4s.size()))
        return false;

    if (!out.Write(&unk, sizeof(NUNV1::Unk)))
        return false;

    if (control_points.size() != influences.size())
    {
        DPRINTF("%s: Bad state, control_points.size()!= influences.size()\n", FUNCNAME);
        return false;
    }

    if (control_points.size() > 0 && !out.Write(control_points.data(), control_points.size()*sizeof(NUNOControlPoint)))
        return false;

    if (influences.size() > 0 && !out.Write(influences.data(), influences.size()*sizeof(NUNOInfluence)))
        return false;

    if (unk1s.size() > 0 && !out.Write(unk1s.data(), unk1s.size()*sizeof(NUNOUnk1)))
        return false;

    if (unk4s.size() > 0 && !out.Write(unk4s.data(), unk4s.size()*sizeof(uint32_t)))
        return false;

    return true;
}

TiXmlElement *NUNV1::Decompile(TiXmlNode *root, const G1mFile &g1m, size_t idx) const
{
    TiXmlElement *entry_root = new TiXmlElement("Nunv1");
    entry_root->SetAttribute("idx", (uint32_t)idx);

    std::string bone_name;

    if (!g1m.BoneIDToName(parent_bone, bone_name))
    {
        DPRINTF("%s: bone %d couldn't be resolved to a name.\n", FUNCNAME, parent_bone);
        return nullptr;
    }
    else
    {
        entry_root->SetAttribute("parent_bone", bone_name);
    }

    Utils::WriteParamUnsigned(entry_root, "U_10", unk.unk_10);
    Utils::WriteParamMultipleFloats(entry_root, "F_14", std::vector<float>(unk.unk_14, unk.unk_14+19));
    Utils::WriteParamMultipleUnsigned(entry_root, "U_60", std::vector<uint32_t>(unk.unk_60, unk.unk_60+5), true);

    if (!DecompileControlPointsInfluences(entry_root, control_points, influences))
        return nullptr;

    DecompileNunoUnk1(entry_root, unk1s);

    if (unk4s.size() > 0)
        Utils::WriteParamMultipleUnsigned(entry_root, "NunoUnk4", unk4s);

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool NUNV1::Compile(const TiXmlElement *root, const G1mFile &g1m)
{
    std::string bone_name;

    if (!Utils::ReadAttrString(root, "parent_bone", bone_name))
    {
        DPRINTF("Cannot get attribute \"parent_bone\" (in object at line %d)\n", root->Row());
        return false;
    }

    int id = g1m.BoneNameToID(bone_name);
    if (id < 0)
    {
       DPRINTF("Cannot find bone \"%s\" (in object at line %d)\n", bone_name.c_str(), root->Row());
       return false;
    }

    parent_bone = (uint16_t)id;

    if (!Utils::GetParamUnsigned(root, "U_10", &unk.unk_10)) return false;
    if (!Utils::GetParamMultipleFloats(root, "F_14", unk.unk_14, 19)) return false;
    if (!Utils::GetParamMultipleUnsigned(root, "U_60", unk.unk_60, 5)) return false;

    control_points.clear();
    influences.clear();
    unk1s.clear();
    unk4s.clear();

    Utils::ReadParamMultipleUnsigned(root, "NunoUnk4", unk4s);

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Control_Point")
        {
            NUNOControlPoint control_point;
            NUNOInfluence influence;

            if (!CompileControlPointInfluence(elem, control_point, influence))
                return false;

            control_points.push_back(control_point);
            influences.push_back(influence);
        }
        else if (elem->ValueStr() == "NunoUnk1")
        {
            NUNOUnk1 unk1;

            if (!CompileNunoUnk1(elem, unk1)) return false;
            unk1s.push_back(unk1);
        }
    }

    return true;
}

bool NUNVChunk::Read(FixedMemoryStream &in, uint32_t chunk_version, uint32_t)
{
    version = Utils::GetShortVersion(chunk_version);

    if (version != 11)
    {
        DPRINTF("NUNV: cannot understand this version of chunk (0x%08X, aka %d)\n", chunk_version, version);
        return false;
    }

    NUNChunkHeader hdr;

    if (!in.Read(&hdr, sizeof(NUNChunkHeader)))
        return false;

    nunv1s.clear();

    for (uint32_t n = 0; n < hdr.section_count; n++)
    {
        uint64_t section_start = in.Tell();
        NUNSectionHeader sec_hdr;

        if (!in.Read(&sec_hdr, sizeof(NUNSectionHeader)))
            return false;

        if (sec_hdr.id == 0x50001)
        {
            nunv1s.resize(sec_hdr.count);
        }
        else
        {
            DPRINTF("Unknown NUNO section 0x%x\n", sec_hdr.id);
            return false;
        }

        for (uint32_t i = 0; i < sec_hdr.count; i++)
        {
            //DPRINTF("Going to read 0x%x. Starting at offset 0x%Ix\n", sec_hdr.id, (size_t)in.Tell());

            if (sec_hdr.id == 0x50001)
            {
                if (!nunv1s[i].Read(in))
                    return false;
            }
        }

        in.Seek(section_start + sec_hdr.size, SEEK_SET);
    }

    return true;
}

bool NUNVChunk::Write(MemoryStream &out) const
{
    uint64_t start = out.Tell();
    NUNChunkHeader hdr;

    hdr.signature = NUNV_SIGNATURE;
    hdr.version = Utils::GetLongVersion(version);
    hdr.size = 0;
    hdr.section_count = 0;

    if (!out.Write(&hdr, sizeof(hdr)))
        return false;

    if (nunv1s.size() > 0)
    {
        NUNSectionHeader shdr;
        uint64_t section_start = out.Tell();

        shdr.id = 0x50001;
        shdr.count = (uint32_t)nunv1s.size();

        if (!out.Write(&shdr, sizeof(NUNSectionHeader)))
            return false;

        for (const NUNV1 &n1 : nunv1s)
        {
            if (!n1.Write(out))
                return false;
        }

        uint64_t section_end = out.Tell();
        shdr.size = (uint32_t)(section_end - section_start);

        out.Seek(section_start, SEEK_SET);

        if (!out.Write(&shdr, sizeof(NUNSectionHeader)))
            return false;

        out.Seek(section_end, SEEK_SET);
        hdr.section_count++;
    }

    uint64_t end = out.Tell();
    hdr.size = (uint32_t)(end - start);

    if (!out.Seek((off64_t)start, SEEK_SET))
        return false;

    if (!out.Write(&hdr, sizeof(hdr)))
        return false;

    return out.Seek((off64_t)end, SEEK_SET);
}

TiXmlElement *NUNVChunk::Decompile(TiXmlNode *root, const G1mFile &g1m) const
{
    TiXmlElement *entry_root = new TiXmlElement("NUNV");
    entry_root->SetAttribute("version", version);

    for (size_t i = 0; i < nunv1s.size(); i++)
    {
        if (!nunv1s[i].Decompile(entry_root, g1m, i))
            return nullptr;
    }

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool NUNVChunk::Compile(const TiXmlElement *root, const G1mFile &g1m)
{
    if (!Utils::ReadAttrUnsigned(root, "version", &version))
        version = 11;

    if (version != 11)
    {
        DPRINTF("Cannot understand NUNV version %d.\n", version);
        return false;
    }

    nunv1s.clear();
    nunv1s.resize(Utils::GetElemCount(root, "Nunv1"));

    std::vector<bool> compiled;
    compiled.resize(nunv1s.size(), false);

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Nunv1")
        {
            uint32_t idx;

            if (!Utils::ReadAttrUnsigned(elem, "idx", &idx))
            {
                DPRINTF("Missing attribute \"idx\" (in object at line %d).\n", elem->Row());
                return false;
            }

            if (idx >= nunv1s.size())
            {
                DPRINTF("\"idx\" out of bounds (in object at line %d).\n", elem->Row());
                return false;
            }

            if (compiled[idx])
            {
                DPRINTF("Duplicate Nunv1 idx %d (duplication at line %d)\n", idx, elem->Row());
                return false;
            }

            if (!nunv1s[idx].Compile(elem, g1m))
                return false;

            compiled[idx] = true;
        }
    }

    return true;
}

bool UnkChunk::Read(FixedMemoryStream &in, uint32_t chunk_type, uint32_t chunk_version, uint32_t chunk_size)
{
    this->type = chunk_type;
    this->version = chunk_version;

    if (!in.Seek(0xC, SEEK_CUR))
        return false;

    unk.resize(chunk_size-0xC);

    if (unk.size() > 0 && !in.Read(unk.data(), unk.size()))
        return false;

    return true;
}

bool UnkChunk::Write(MemoryStream &out) const
{
    if (!out.Write32(type))
        return false;

    if (!out.Write32(version))
        return false;

    if (!out.Write32((uint32_t)unk.size()+0xC))
        return false;

    if (unk.size() > 0)
        return out.Write(unk.data(), unk.size());

    return true;
}

TiXmlElement *UnkChunk::Decompile(TiXmlNode *root, const std::string &att_dir, uint32_t idx) const
{
    std::string chunk_name;

    if (type == COLL_SIGNATURE)
    {
        chunk_name = "COLL";
    }
    else if (type == NUNO_SIGNATURE)
    {
        chunk_name = "NUNO";
    }
    else if (type == NUNV_SIGNATURE)
    {
        chunk_name = "NUNV";
    }
    else if (type == NUNS_SIGNATURE)
    {
        chunk_name = "NUNS";
    }
    else if (type == EXTR_SIGNATURE)
    {
        chunk_name = "EXTR";
    }
    else if (type == HAIR_SIGNATURE)
    {
        chunk_name = "HAIR";
    }
    else if (type == SOFT_SIGNATURE)
    {
        chunk_name = "SOFT";
    }
    else
    {
        throw std::runtime_error("Not recognized chunk type, shouldn't be here.\n");
    }

    TiXmlElement *entry_root = new TiXmlElement(chunk_name);
    entry_root->SetAttribute("version", Utils::GetShortVersion(version));

    std::string fn = Utils::ToString(idx) + "." + Utils::ToLowerCase(chunk_name);
    entry_root->SetAttribute("binary", fn);

    std::string path = Utils::MakePathString(att_dir, fn);
    if (!Utils::WriteFileBool(path, unk.data(), unk.size(), true, true))
        return nullptr;

    root->LinkEndChild(entry_root);
    return entry_root;
}

bool UnkChunk::Compile(const TiXmlElement *root, const std::string &att_dir, uint32_t type)
{
    this->type = type;

    if (!Utils::ReadAttrUnsigned(root, "version", &version))
    {
        DPRINTF("Missing attribute \"version\". (in object at line %d).\n", root->Row());
        return false;
    }

    version = Utils::GetLongVersion(version);

    std::string fn, path;

    if (!Utils::ReadAttrString(root, "binary", fn))
    {
        DPRINTF("Missing attribute \"binary\". (in object at line %d).\n", root->Row());
        return false;
    }

    path = Utils::MakePathString(att_dir, fn);

    size_t size;
    uint8_t *buf = Utils::ReadFile(path, &size);
    if (!buf)
        return false;

    unk = std::vector<uint8_t>(buf, buf+size);
    delete[] buf;

    return true;
}

G1mFile::G1mFile()
{
    this->big_endian = false;
    update_g1mf = true;
    parse_nun = true;
}

G1mFile::~G1mFile()
{

}

void G1mFile::Reset()
{
    g1mfs.clear();
    g1mss.clear();
    g1mms.clear();
    g1mgs.clear();
    colls.clear();
    nunos.clear();
    nunvs.clear();
    nunss.clear();
    extrs.clear();
    hairs.clear();
    softs.clear();

    unparsed_nunos.clear();
    unparsed_nunvs.clear();

    upgraded_ibs.clear();

    version = 37;
    update_g1mf = true;
}

bool G1mFile::Load(const uint8_t *buf, size_t size)
{
    FixedMemoryStream in(const_cast<uint8_t *>(buf), size);

    G1MHeader *hdr;

    Reset();

    if (!in.FastRead((uint8_t **)&hdr, sizeof(G1MHeader)))
        return false;

    if (hdr->signature != G1M_SIGNATURE)
    {
        DPRINTF("Not a g1m file.\n");
        return false;
    }

    version = Utils::GetShortVersion(hdr->version);

    if (version != 37 && version != 36 && version != 34)
    {
        DPRINTF("Not supported version of g1m format (0x%08x, aka %d)\n", hdr->version, version);
        return false;
    }    

    if (hdr->file_size > size)
        return false;

    if (!in.Seek(hdr->header_size, SEEK_SET))
        return false;

    for (uint32_t i = 0; i < hdr->num_chunks; i++)
    {
        uint32_t chunk_type, chunk_version, chunk_size;
        off64_t chunk_start = (off64_t)in.Tell();

        if (!in.Read32(&chunk_type))
            return false;

        if (!in.Read32(&chunk_version))
            return false;

        if (!in.Read32(&chunk_size))
            return false;

        in.Seek(chunk_start, SEEK_SET);

        //DPRINTF("Chunk %08x\n", chunk_type);

        if (chunk_type == G1MF_SIGNATURE)
        {
            G1MFChunk g1mf;

            if (!g1mf.Read(in, chunk_version, chunk_size))
                return false;

            g1mfs.push_back(g1mf);
        }
        else if (chunk_type == G1MS_SIGNATURE)
        {
            G1MSChunk g1ms;

            if (!g1ms.Read(in, chunk_version, chunk_size))
                return false;

            g1mss.push_back(g1ms);
        }
        else if (chunk_type == G1MM_SIGNATURE)
        {
            G1MMChunk g1mm;

            if (!g1mm.Read(in, chunk_version, chunk_size))
                return false;

            g1mms.push_back(g1mm);
        }
        else if (chunk_type == G1MG_SIGNATURE)
        {
            G1MGChunk g1mg;

            if (!g1mg.Read(in, chunk_version, chunk_size))
                return false;

            g1mgs.push_back(g1mg);
        }
        else if (chunk_type == COLL_SIGNATURE)
        {
            UnkChunk coll;

            if (!coll.Read(in, chunk_type, chunk_version, chunk_size))
                return false;

            colls.push_back(coll);
        }
        else if (chunk_type == NUNO_SIGNATURE)
        {
            if (parse_nun)
            {
                NUNOChunk nuno;

                if (!nuno.Read(in, chunk_version, chunk_size))
                    return false;

                nunos.push_back(nuno);
            }
            else
            {
                UnkChunk nuno;

                if (!nuno.Read(in, chunk_type, chunk_version, chunk_size))
                    return false;

                unparsed_nunos.push_back(nuno);
            }
        }
        else if (chunk_type == NUNV_SIGNATURE)
        {
            if (parse_nun)
            {
                NUNVChunk nunv;

                if (!nunv.Read(in, chunk_version, chunk_size))
                    return false;

                nunvs.push_back(nunv);
            }
            else
            {
                UnkChunk nunv;

                if (!nunv.Read(in, chunk_type, chunk_version, chunk_size))
                    return false;

                unparsed_nunvs.push_back(nunv);
            }
        }
        else if (chunk_type == NUNS_SIGNATURE)
        {
            UnkChunk nuns;

            if (!nuns.Read(in, chunk_type, chunk_version, chunk_size))
                return false;

            nunss.push_back(nuns);
        }
        else if (chunk_type == EXTR_SIGNATURE)
        {
            UnkChunk extr;

            if (!extr.Read(in, chunk_type, chunk_version, chunk_size))
                return false;

            extrs.push_back(extr);
        }
        else if (chunk_type == HAIR_SIGNATURE)
        {
            UnkChunk extr;

            if (!extr.Read(in, chunk_type, chunk_version, chunk_size))
                return false;

            hairs.push_back(extr);
        }
        else if (chunk_type == SOFT_SIGNATURE)
        {
            UnkChunk soft;

            if (!soft.Read(in, chunk_type, chunk_version, chunk_size))
                return false;

            softs.push_back(soft);
        }
        else
        {
            DPRINTF("Unknown chunk 0x%08X at offset %Ix\n", chunk_type, (size_t)chunk_start);
            return false;
        }

        if (i != (hdr->num_chunks-1))
        {
            if (!in.Seek(chunk_start+chunk_size, SEEK_SET))
                return false;
        }
    }

    return true;
}

void G1mFile::UpdateG1MF()
{
    if (!update_g1mf)
        return;

    if (g1mfs.size() == 0)
        return;

    G1MFData &g1mf = g1mfs[0].data;

    if (g1mss.size() > 0)
    {
        g1mf.num_bones = (uint32_t)g1mss[0].bones.size();
        g1mf.bones_id_size = (uint32_t)g1mss[0].indices.size();
    }
    else
    {
        g1mf.num_bones = 0;
    }

    if (g1mms.size() > 0)
        g1mf.num_matrix = (uint32_t)g1mms[0].matrices.size();
    else
        g1mf.num_matrix = 0;

    g1mf.num_bone_maps = g1mf.num_individual_bone_maps = 0;

    if (g1mgs.size() > 0)
    {
        G1MGBonesMapSection &bsec = g1mgs[0].bones_map_section;

        if (bsec.valid)
        {
            g1mf.num_bone_maps = (uint32_t)bsec.bones_maps.size();

            for (const G1MGBonesMap &maps : bsec.bones_maps)
            {
                g1mf.num_individual_bone_maps += (uint32_t)maps.map.size();
            }
        }

        g1mf.num_materials = 0;

        if (g1mgs[0].mat_section.valid)
            g1mf.num_materials = (uint32_t)g1mgs[0].mat_section.materials.size();

        g1mf.num_material_attributes = g1mf.num_attributes = 0;

        if (g1mgs[0].att_section.valid)
        {
            g1mf.num_material_attributes = (uint32_t)g1mgs[0].att_section.mat_attributes.size();

            for (const G1MGMaterialAttributes &mat : g1mgs[0].att_section.mat_attributes)
            {
                g1mf.num_attributes += (uint32_t)mat.attributes.size();
            }
        }

        g1mf.num_vb = 0;
        g1mf.num_layout_refs = 0;
        g1mf.num_layouts = 0;

        if (g1mgs[0].vert_section.valid)
            g1mf.num_vb = (uint32_t)g1mgs[0].vert_section.vertex_bufs.size();

        if (g1mgs[0].layout_section.valid)
            g1mf.num_layouts = (uint32_t)g1mgs[0].layout_section.entries.size();        

        if (g1mgs[0].layout_section.valid)
        {
            for (const G1MGLayout &ly : g1mgs[0].layout_section.entries)
            {
                g1mf.num_layout_refs += (uint32_t)ly.refs.size();
            }
        }

        g1mf.num_submeshes = g1mf.num_submeshes2 = 0;
        g1mf.num_non_shared_vb = 0;

        if (g1mgs[0].submeshes_section.valid)
        {
            std::unordered_set<uint32_t> vb;

            g1mf.num_submeshes = g1mf.num_submeshes2 = (uint32_t)g1mgs[0].submeshes_section.submeshes.size();

            for (const G1MGSubmesh &surf : g1mgs[0].submeshes_section.submeshes)
            {
                vb.insert(surf.vertex_buf_ref);
            }

            g1mf.num_non_shared_vb = (uint32_t)vb.size();
        }

        g1mf.num_meshes = g1mf.num_submeshes_in_meshes = 0;

        if (g1mgs[0].meshes_section.valid)
        {
            for (const G1MGLodGroup &group : g1mgs[0].meshes_section.groups)
            {
                g1mf.num_meshes += (uint32_t)group.meshes.size();

                for (const G1MGMesh &mesh : group.meshes)
                {
                    g1mf.num_submeshes_in_meshes += (uint32_t)mesh.submeshes.size();
                }
            }
        }
    }

    if (unparsed_nunos.size() == 0) // Only do these ones when nuno was parsed
    {
        g1mf.num_nuno2s = g1mf.num_nuno2s_unk11 = 0;
        g1mf.num_nuno1s = g1mf.num_nuno1s_unk4 = g1mf.num_nuno1s_control_points = g1mf.num_nuno1s_unk1 = g1mf.num_nuno1s_unk2_and_unk3 = 0;
        g1mf.num_nuno3s = g1mf.num_nuno3s_unk4 = g1mf.num_nuno3s_control_points = g1mf.num_nuno3s_unk1 = 0;
        g1mf.num_nuno4s = g1mf.num_nuno4s_unk7 = g1mf.num_nuno4s_unk8 = g1mf.num_nuno4s_unk9 = g1mf.num_nuno4s_unk10 = 0;

        for (const NUNOChunk &nuno : nunos)
        {
            g1mf.num_nuno2s += (uint32_t)nuno.nuno2s.size();
            g1mf.num_nuno1s += (uint32_t)nuno.nuno1s.size();
            g1mf.num_nuno3s += (uint32_t)nuno.nuno3s.size();
            g1mf.num_nuno4s += (uint32_t)nuno.nuno4s.size();

            for (const NUNO2 &nuno2: nuno.nuno2s)
            {
                g1mf.num_nuno2s_unk11 += (uint32_t)nuno2.unk11s.size();
            }

            for (const NUNO1 &nuno1 : nuno.nuno1s)
            {
                g1mf.num_nuno1s_unk4 += (uint32_t)nuno1.unk4s.size();
                g1mf.num_nuno1s_control_points += (uint32_t)nuno1.control_points.size();
                g1mf.num_nuno1s_unk1 += (uint32_t)nuno1.unk1s.size();
                g1mf.num_nuno1s_unk2_and_unk3 += (uint32_t)(nuno1.unk2s.size() + nuno1.unk3s.size());
            }

            for (const NUNO3 &nuno3 : nuno.nuno3s)
            {
                g1mf.num_nuno3s_unk4 += (uint32_t)nuno3.unk4s.size();
                g1mf.num_nuno3s_control_points += (uint32_t)nuno3.control_points.size();
                g1mf.num_nuno3s_unk1 += (uint32_t)nuno3.unk1s.size();
            }

            for (const NUNO4 &nuno4: nuno.nuno4s)
            {
                g1mf.num_nuno4s_unk7 += (uint32_t)nuno4.unk7s.size();
                g1mf.num_nuno4s_unk8 += (uint32_t)nuno4.unk8s.size();
                g1mf.num_nuno4s_unk9 += (uint32_t)nuno4.unk9s.size();
                g1mf.num_nuno4s_unk10 += (uint32_t)nuno4.unk10s.size();
            }
        }
    }

    if (unparsed_nunvs.size() == 0) // Only do these ones when nunv was parsed
    {
        g1mf.num_nunv1s = g1mf.num_nunv1s_unk4 = g1mf.num_nunv1s_control_points = g1mf.num_nunv1s_unk1 = 0;

        for (const NUNVChunk &nunv : nunvs)
        {
            g1mf.num_nunv1s += (uint32_t)nunv.nunv1s.size();

            for (const NUNV1 &nunv1 : nunv.nunv1s)
            {
                g1mf.num_nunv1s_unk4 += (uint32_t)nunv1.unk4s.size();
                g1mf.num_nunv1s_control_points += (uint32_t)nunv1.control_points.size();
                g1mf.num_nunv1s_unk1 += (uint32_t)nunv1.unk1s.size();
            }
        }
    }
}

uint8_t *G1mFile::Save(size_t *psize)
{
    MemoryStream out;
    G1MHeader hdr;

    UpdateG1MF();

    memset(&hdr, 0, sizeof(G1MHeader));

    hdr.signature = G1M_SIGNATURE;
    hdr.version = Utils::GetLongVersion(version);
    hdr.header_size = sizeof(hdr);

    if (!out.Write(&hdr, sizeof(G1MHeader)))
        return nullptr;

    for (const G1MFChunk &g1mf : g1mfs)
    {
        if (!g1mf.Write(out))
            return nullptr;

        hdr.num_chunks++;
    }

    for (const G1MSChunk &g1ms : g1mss)
    {
        if (!g1ms.Write(out))
            return nullptr;

        hdr.num_chunks++;
    }

    for (const G1MMChunk &g1mm : g1mms)
    {
        if (!g1mm.Write(out))
            return nullptr;

        hdr.num_chunks++;
    }

    for (const G1MGChunk &g1mg : g1mgs)
    {
        if (!g1mg.Write(out))
            return nullptr;

        hdr.num_chunks++;
    }

    for (const UnkChunk &coll : colls)
    {
        if (!coll.Write(out))
            return nullptr;

        hdr.num_chunks++;
    }

    if (nunos.size() > 0)
    {
        for (const NUNOChunk &nuno : nunos)
        {
            if (!nuno.Write(out))
                return nullptr;

            hdr.num_chunks++;
        }
    }
    else
    {
        for (const UnkChunk &nuno : unparsed_nunos)
        {
            if (!nuno.Write(out))
                return nullptr;

            hdr.num_chunks++;
        }
    }

    if (nunvs.size() > 0)
    {
        for (const NUNVChunk &nunv : nunvs)
        {
            if (!nunv.Write(out))
                return nullptr;

            hdr.num_chunks++;
        }
    }
    else
    {
        for (const UnkChunk &nunv : unparsed_nunvs)
        {
            if (!nunv.Write(out))
                return nullptr;

            hdr.num_chunks++;
        }
    }

    for (const UnkChunk &nuns : nunss)
    {
        if (!nuns.Write(out))
            return nullptr;

        hdr.num_chunks++;
    }

    for (const UnkChunk &extr : extrs)
    {
        if (!extr.Write(out))
            return nullptr;

        hdr.num_chunks++;
    }

    for (const UnkChunk &hair : hairs)
    {
        if (!hair.Write(out))
            return nullptr;

        hdr.num_chunks++;
    }

    for (const UnkChunk &soft : softs)
    {
        if (!soft.Write(out))
            return nullptr;

        hdr.num_chunks++;
    }

    hdr.file_size = (uint32_t)out.GetSize();

    if (!out.Seek(0, SEEK_SET))
        return nullptr;

    if (!out.Write(&hdr, sizeof(G1MHeader)))
        return nullptr;

    *psize = (size_t)out.GetSize();
    return out.GetMemory(true);
}

TiXmlDocument *G1mFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();
    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("G1M");
    root->SetAttribute("version", version);    

    for (const G1MFChunk &g1mf : g1mfs)
    {
         Utils::WriteComment(root, "If auto = true, the known values will be automatically calculated by the program.");

        if (!g1mf.Decompile(root))
            return nullptr;
    }

    for (const G1MSChunk &g1ms : g1mss)
    {
        if (!g1ms.Decompile(root))
            return nullptr;
    }

    for (const G1MMChunk &g1mm : g1mms)
    {
        if (!g1mm.Decompile(root))
            return nullptr;
    }    

    for (const G1MGChunk &g1mg : g1mgs)
    {
        if (g1mss.size() == 0)
        {
            DPRINTF("Umm, this is a weird case of a .g1m with G1MG but without G1MS, cannot decompile!\n");
            return nullptr;
        }

        if (!g1mg.Decompile(root, att_dir, g1mss[0].bone_names))
            return nullptr;
    }

    for (uint32_t i = 0; i < (uint32_t)colls.size(); i++)
    {
        if (!colls[i].Decompile(root, att_dir, i))
            return nullptr;
    }

    if (nunos.size() > 0)
    {
        for (uint32_t i = 0; i < (uint32_t)nunos.size(); i++)
        {
            if (!nunos[i].Decompile(root, *this))
                return nullptr;
        }
    }
    else
    {
        for (uint32_t i = 0; i < (uint32_t)unparsed_nunos.size(); i++)
        {
            if (!unparsed_nunos[i].Decompile(root, att_dir, i))
                return nullptr;
        }
    }

    if (nunvs.size() > 0)
    {
        for (uint32_t i = 0; i < (uint32_t)nunvs.size(); i++)
        {
            if (!nunvs[i].Decompile(root, *this))
                return nullptr;
        }
    }
    else
    {
        for (uint32_t i = 0; i < (uint32_t)unparsed_nunvs.size(); i++)
        {
            if (!unparsed_nunvs[i].Decompile(root, att_dir, i))
                return nullptr;
        }
    }

    for (uint32_t i = 0; i < (uint32_t)nunss.size(); i++)
    {
        if (!nunss[i].Decompile(root, att_dir, i))
            return nullptr;
    }

    for (uint32_t i = 0; i < (uint32_t)extrs.size(); i++)
    {
        if (!extrs[i].Decompile(root, att_dir, i))
            return nullptr;
    }

    for (uint32_t i = 0; i < (uint32_t)hairs.size(); i++)
    {
        if (!hairs[i].Decompile(root, att_dir, i))
            return nullptr;
    }

    for (uint32_t i = 0; i < (uint32_t)softs.size(); i++)
    {
        if (!softs[i].Decompile(root, att_dir, i))
            return nullptr;
    }

    doc->LinkEndChild(root);
    return doc;
}

bool G1mFile::Compile(TiXmlDocument *doc, bool)
{
    Reset();

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "G1M");

    if (!root)
    {
        DPRINTF("Cannot find\"G1M\" in xml.\n");
        return false;
    }

    if (!Utils::ReadAttrUnsigned(root, "version", &version))
        version = 37; 

    if (version != 37 && version != 36 && version != 34)
    {
        DPRINTF("Not supported version of g1m format (%d)\n", version);
        return false;
    }   

    bool g1mf_auto = true;
    std::vector<bool> auto_meshes;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "G1MF")
        {
            G1MFChunk g1mf;

            if (!g1mf.Compile(elem, &g1mf_auto))
            {
                return false;
            }

            g1mfs.push_back(g1mf);
        }
        else if (elem->ValueStr() == "G1MS")
        {
            G1MSChunk g1ms;

            if (!g1ms.Compile(elem))
                return false;

            g1mss.push_back(g1ms);
        }
        else if (elem->ValueStr() == "G1MM")
        {
            G1MMChunk g1mm;

            if (!g1mm.Compile(elem))
                return false;

            g1mms.push_back(g1mm);
        }

        else if (elem->ValueStr() == "G1MG")
        {
            if (g1mss.size() == 0)
            {
                DPRINTF("Error: If there is a G1MG, a G1MS must exist and must be specified anywhere BEFORE the G1MG.\n");
                return false;
            }

            G1MGChunk g1mg;

            if (!g1mg.Compile(elem, att_dir, g1mss[0].bone_names, auto_meshes))
                return false;

            g1mgs.push_back(g1mg);
        }
        else if (elem->ValueStr() == "COLL")
        {
            UnkChunk coll;

            if (!coll.Compile(elem, att_dir, COLL_SIGNATURE))
                return false;

            colls.push_back(coll);
        }
        else if (elem->ValueStr() == "NUNO")
        {
            if (parse_nun)
            {
                NUNOChunk nuno;

                if (!nuno.Compile(elem, *this))
                    return false;

                nunos.push_back(nuno);
            }
            else
            {
                UnkChunk nuno;

                if (!nuno.Compile(elem, att_dir, NUNO_SIGNATURE))
                    return false;

                unparsed_nunos.push_back(nuno);
            }
        }
        else if (elem->ValueStr() == "NUNV")
        {
            if (parse_nun)
            {
                NUNVChunk nunv;

                if (!nunv.Compile(elem, *this))
                    return false;

                nunvs.push_back(nunv);
            }
            else
            {
                UnkChunk nunv;

                if (!nunv.Compile(elem, att_dir, NUNV_SIGNATURE))
                    return false;

                unparsed_nunvs.push_back(nunv);
            }
        }
        else if (elem->ValueStr() == "NUNS")
        {
            UnkChunk nuns;

            if (!nuns.Compile(elem, att_dir, NUNS_SIGNATURE))
                return false;

            nunss.push_back(nuns);
        }
        else if (elem->ValueStr() == "EXTR")
        {
            UnkChunk extr;

            if (!extr.Compile(elem, att_dir, EXTR_SIGNATURE))
                return false;

            extrs.push_back(extr);
        }
        else if (elem->ValueStr() == "HAIR")
        {
            UnkChunk hair;

            if (!hair.Compile(elem, att_dir, HAIR_SIGNATURE))
                return false;

            hairs.push_back(hair);
        }
        else if (elem->ValueStr() == "SOFT")
        {
            UnkChunk soft;

            if (!soft.Compile(elem, att_dir, SOFT_SIGNATURE))
                return false;

            softs.push_back(soft);
        }
    }

    for (size_t i = 0; i < auto_meshes.size(); i++)
    {
        if (auto_meshes[i] && !RecalcLodGroup(i))
        {
            DPRINTF("Failed to auto recalculate meshes (LodGroup idx %Id).\n", i);
            return false;
        }
    }

    update_g1mf = g1mf_auto;
    return true;
}

bool G1mFile::DecompileToFile(const std::string &path, bool show_error, bool build_path)
{
    size_t pos = Utils::NormalizePath(path).rfind('/');

    std::string dir = Utils::GetDirNameString(path);
    std::string fn = Utils::GetFileNameString(path);

    if (pos == std::string::npos)
        dir = "./";

    pos = Utils::ToLowerCase(fn).rfind(".g1m.xml");
    if (pos == std::string::npos)
    {
        fn += "_BIN";
    }
    else
    {
       fn = fn.substr(0, pos);
    }

    att_dir = Utils::MakePathString(dir, fn);
    return BaseFile::DecompileToFile(path, show_error, build_path);
}

bool G1mFile::CompileFromFile(const std::string &path, bool show_error, bool big_endian)
{
    size_t pos = Utils::NormalizePath(path).rfind('/');

    std::string dir = Utils::GetDirNameString(path);
    std::string fn = Utils::GetFileNameString(path);

    if (pos == std::string::npos)
        dir = "./";

    pos = Utils::ToLowerCase(fn).rfind(".g1m");
    if (pos == std::string::npos)
    {
        fn += "_BIN";
    }
    else
    {
       fn = fn.substr(0, pos);
    }

    att_dir = Utils::MakePathString(dir, fn);
    return BaseFile::CompileFromFile(path, show_error, big_endian);
}

/*void G1mFile::TestExport()
{
    if (g1mgs.size() == 0)
        return;

    G1MGChunk &g1mg = g1mgs[0];

    if (g1mg.vert_section.vertex_bufs.size() != g1mg.index_buf_section.buffers.size())
    {
        DPRINTF("Mismatch number of meshes.\n");
        return;
    }

    size_t num_meshes = g1mg.vert_section.vertex_bufs.size();

    for (size_t i = 0; i < num_meshes; i++)
    {
        G1MGVertexBuf &vb = g1mg.vert_section.vertex_bufs[i];
        G1MGIndexBuffer &ib = g1mg.index_buf_section.buffers[i];

        if (vb.vertex_size == 0x2C)
        {
            std::string ib_name = "g1m_test/" + Utils::ToString(i) + ".ib";
            std::string vb_name = "g1m_test/" + Utils::ToString(i) + ".vb";

            if (!Utils::WriteFileBool(ib_name, ib.indices.GetMemory(false), ib.indices.GetSize()))
                return;

            if (!Utils::WriteFileBool(vb_name, vb.vertex.GetMemory(false), vb.vertex.GetSize()))
                return;
        }
    }
}*/

bool G1mFile::ExportTo3DM(const std::string &out_path, bool vgmaps)
{
    if (g1mgs.size() == 0)
        return true;

    const G1MGChunk &g1mg = g1mgs[0];

    if (!g1mg.submeshes_section.valid)
        return true;

    if (g1mg.vert_section.vertex_bufs.size() != g1mg.index_buf_section.buffers.size() || g1mg.vert_section.vertex_bufs.size() != g1mg.layout_section.entries.size())
    {
        DPRINTF("Mismatch in VB/IB/Layout (%Id vs %Id vs %Id).\n", g1mg.vert_section.vertex_bufs.size(), g1mg.index_buf_section.buffers.size(), g1mg.layout_section.entries.size());
        return false;
    }

    if (vgmaps)
    {
        if (g1mss.size() == 0)
            return false;

        if (g1mss[0].bone_names.size() == 0)
            SetDefaultBoneNames();
    }

    size_t num_sm = GetNumSubmeshes();

    for (size_t i = 0; i < num_sm; i++)
    {
        const G1MGSubmesh &surf = g1mg.submeshes_section.submeshes[i];

        const G1MGVertexBuffer &vb = g1mg.vert_section.vertex_bufs[surf.vertex_buf_ref];
        const G1MGIndexBuffer &ib = g1mg.index_buf_section.buffers[surf.index_buf_ref];
        const G1MGLayout &ly = g1mg.layout_section.entries[surf.vertex_buf_ref];

        std::string name = Utils::MakePathString(out_path, Utils::ToString(i));
        std::string vb_name = name + ".vb";
        std::string ib_name = name + ".ib";
        std::string fmt_name = name + ".fmt";
        std::string vg_name = name + ".vgmap";

        if (!Utils::WriteFileBool(vb_name, vb.vertex.data(), vb.vertex.size()))
            return false;

        if (!Utils::WriteFileBool(ib_name, ib.indices.data() + (surf.index_buf_start*ib.GetIndiceSize()), surf.num_indices * ib.GetIndiceSize()))
            return false;

        FILE *fmt = fopen(fmt_name.c_str(), "w");
        if (!fmt)
        {
            DPRINTF("Failed to create or open for write %s\n", fmt_name.c_str());
            return false;
        }

        fprintf(fmt, "stride: %u\ntopology: trianglelist\nformat: DXGI_FORMAT_R16_UINT\n", vb.vertex_size);

        for (size_t j = 0; j < ly.semantics.size(); j++)
        {
            const G1MGSemantic &sem = ly.semantics[j];
            uint8_t type = sem.semantic&0xFF;
            uint8_t idx = sem.semantic >> 8;
            std::string type_str, format_str;

            fprintf(fmt, "element[%u]:\n", (uint32_t)j);

            switch (type)
            {
                case 0:
                    type_str = "POSITION";
                break;

                case 1:
                    type_str = "BLENDWEIGHT";
                break;

                case 2:
                    type_str = "BLENDINDICES";
                break;

                case 3:
                    type_str = "NORMAL";
                break;

                case 4:
                    type_str = "PSIZE";
                break;

                case 5:
                    type_str = "TEXCOORD";
                break;

                case 6:
                    type_str = "TANGENT";
                break;

                case 7:
                    type_str = "BINORMAL";
                break;

                case 8:
                    type_str = "TESSFACTOR";
                break;

                case 9:
                    type_str = "POSITIONT";
                break;

                case 10:
                    type_str = "COLOR";
                break;

                case 11:
                    type_str = "FOG";
                break;

                case 12:
                    type_str = "DEPTH";
                break;

                case 13:
                    type_str = "SAMPLE";
                break;

                default:
                    DPRINTF("Unrecognized semantic  %d\n", type);
                    return false;
            }

            fprintf(fmt, "  SemanticName: %s\n", type_str.c_str());
            fprintf(fmt, "  SemanticIndex: %d\n", idx);

            switch (sem.data_type)
            {
                case 0:
                    format_str = "R32_FLOAT";
                break;

                case 1:
                    format_str = "R32G32_FLOAT";
                break;

                case 2:
                    format_str = "R32G32B32_FLOAT";
                break;

                case 3:
                    format_str = "R32G32B32A32_FLOAT";
                break;

                case 5:
                    format_str = "R8G8B8A8_UINT";
                break;

                case 7:
                    format_str = "R16G16B16A16_UINT";
                break;

                case 9:
                    format_str = "R32G32B32A32_UINT";
                break;

                case 10:
                    format_str = "R16G16_FLOAT";
                break;

                case 11:
                    format_str = "R16G16B16A16_FLOAT";
                break;

                // Needs confirmation! We only saw thsi data type with FF FF FF FF data
                // It actually may be DXGI_FORMAT_X24_TYPELESS_G8_UINT
                case 13:
                    format_str = "R8G8B8A8_UINT";
                break;

                default:
                    DPRINTF("Unrecognized data type %d\n", sem.data_type);
                    return false;
            }

            fprintf(fmt, "  Format: %s\n", format_str.c_str());
            fprintf(fmt, "  InputSlot: 0\n"); // mmm
            fprintf(fmt, "  AlignedByteOffset: %u\n", sem.offset);
            fprintf(fmt, "  InputSlotClass: per-vertex\n  InstanceDataStepRate: 0\n");
        }

        fclose(fmt);

        if (vgmaps)
        {
            FILE *vg = fopen(vg_name.c_str(), "w");
            if (!vg)
            {
                DPRINTF("Failed to create or open for write %s\n", vg_name.c_str());
                return false;
            }

            fprintf(vg, "{\n");

            const G1MGBonesMap &maps = g1mg.bones_map_section.bones_maps[surf.bones_map_index];
            for (size_t i = 0; i < maps.map.size(); i++)
            {
                const G1MGBoneMapEntry &entry = maps.map[i];

                if (entry.flags == 0)
                {
                    fprintf(vg, "\t\"%s\": %d%s\n", g1mss[0].bone_names[entry.mapped].c_str(), (uint32_t)i*3, (i == maps.map.size()-1) ? "" : ",");
                }
                else
                {
                    fprintf(vg, "\t\"ExternalBone%d\": %d%s\n", entry.mapped, (uint32_t)i*3, (i == maps.map.size()-1) ? "" : ",");
                }
            }

            fprintf(vg, "}\n");

            fclose(vg);
        }
    }

    return true;
}

void G1mFile::BreakVB(size_t vidx)
{
    // Private function, no validation of indexes
    G1MGVertexBuffer &vb = g1mgs[0].vert_section.vertex_bufs[vidx];

    for (G1MGSubmesh &surf : g1mgs[0].submeshes_section.submeshes)
    {
        if (surf.vertex_buf_ref == (int)vidx)
        {
            surf.temp_vb.clear();

            const uint8_t *ptr = vb.vertex.data() + surf.vertex_buf_start*vb.vertex_size;
            size_t size = surf.num_vertices*vb.vertex_size;

            surf.temp_vb.resize(size);
            memcpy(surf.temp_vb.data(), ptr, size);
        }
    }
}

void G1mFile::BreakIB(size_t iidx)
{
    // Private function, no validation of indexes
    G1MGIndexBuffer &ib = g1mgs[0].index_buf_section.buffers[iidx];

    for (G1MGSubmesh &surf : g1mgs[0].submeshes_section.submeshes)
    {
        if (surf.index_buf_ref == (int)iidx)
        {
            surf.temp_ib.clear();

            const uint8_t *ptr = ib.indices.data() + surf.index_buf_start*ib.GetIndiceSize();
            size_t size = surf.num_indices * ib.GetIndiceSize();

            surf.temp_ib.resize(size);
            memcpy(surf.temp_ib.data(), ptr, size);
        }
    }
}

void G1mFile::MergeVB(size_t vidx)
{
    // Private function, no validation of indexes
    G1MGVertexBuffer &vb = g1mgs[0].vert_section.vertex_bufs[vidx];

    vb.vertex.clear();

    for (G1MGSubmesh &surf : g1mgs[0].submeshes_section.submeshes)
    {
        if (surf.vertex_buf_ref == (int)vidx)
        {
            size_t previous_size = vb.vertex.size();
            surf.vertex_buf_start = (uint32_t)previous_size / vb.vertex_size;

            vb.vertex.resize(previous_size + surf.temp_vb.size());
            uint8_t *ptr = vb.vertex.data() + previous_size;
            memcpy(ptr, surf.temp_vb.data(), surf.temp_vb.size());

            surf.temp_vb.clear();
        }
    }
}

void G1mFile::MergeIB(size_t iidx)
{
    // Private function, no validation of indexes
    G1MGIndexBuffer &ib = g1mgs[0].index_buf_section.buffers[iidx];

    ib.indices.clear();

    for (G1MGSubmesh &surf : g1mgs[0].submeshes_section.submeshes)
    {
        if (surf.index_buf_ref == (int)iidx)
        {
            size_t previous_size = ib.indices.size();
            surf.index_buf_start = (uint32_t)previous_size / ib.GetIndiceSize();

            ib.indices.resize(previous_size + surf.temp_ib.size());
            uint8_t *ptr = ib.indices.data() + previous_size;
            memcpy(ptr, surf.temp_ib.data(), surf.temp_ib.size());

            surf.temp_ib.clear();
        }
    }
}

void G1mFile::UpgradeIB(G1MGIndexBuffer &ib, size_t num_vertex)
{
    if (ib.type == 32)
        return;

    uint16_t *buf16 = nullptr;
    uint32_t *buf32 = nullptr;

    uint8_t *current8 = nullptr;
    uint16_t *current16 = nullptr;

    size_t num = ib.GetNumIndices();

    if (ib.type == 8)
    {
        if (num_vertex < 256)
            return;

        if (num_vertex >= 65536)
            buf32 = new uint32_t[num];
        else
            buf16 = new uint16_t[num];

        current8 = ib.indices.data();
    }
    else
    {
        if (num_vertex < 65536)
            return;

        buf32 = new uint32_t[num];
        current16 = (uint16_t *)ib.indices.data();
    }

    for (size_t i = 0; i < num; i++)
    {
        if (buf16)
            buf16[i] = current8[i];
        else
            buf32[i] = (current8) ? current8[i] : current16[i];
    }

    if (buf16)
    {
        ib.type = 16;
        ib.indices.resize(num*2);
        memcpy(ib.indices.data(), buf16, ib.indices.size());
        delete[] buf16;
    }
    else
    {
        ib.type = 32;
        ib.indices.resize(num*4);
        memcpy(ib.indices.data(), buf32, ib.indices.size());
        delete[] buf32;
    }
}

uint8_t *G1mFile::UpgradeBuffer(uint8_t *old, size_t *abs_size, uint32_t old_format, uint32_t new_format)
{
    uint8_t *ret = old;

    if (new_format == 16)
    {
        if (old_format == 8)
        {
            size_t num_idx = *abs_size;
            uint16_t *ret16 = new uint16_t[num_idx];

            for (size_t i = 0; i < num_idx; i++)
                ret16[i] = old[i];

            *abs_size *= 2;
            ret = (uint8_t *)ret16;
        }
    }
    else if (new_format == 32)
    {
        if (old_format == 8)
        {
            size_t num_idx = *abs_size;
            uint32_t *ret32 = new uint32_t[num_idx];

            for (size_t i = 0; i < num_idx; i++)
                ret32[i] = old[i];

            *abs_size *= 4;
            ret = (uint8_t *)ret32;
        }
        else if (old_format == 16)
        {
            size_t num_idx = *abs_size / 2;
            uint32_t *ret32 = new uint32_t[num_idx];

            for (size_t i = 0; i < num_idx; i++)
                ret32[i] = ((uint16_t *)old)[i];

            *abs_size *= 2;
            ret = (uint8_t *)ret32;
        }
    }

    if (old != ret)
        delete[] old;

    return ret;
}

uint16_t G1mFile::FindBoneIndexByName(const std::string &name) const
{
    if (g1mss.size() > 0)
    {
        std::string lc_name = Utils::ToLowerCase(name);

        for (size_t i = 0; i < g1mss[0].bone_names.size(); i++)
        {
            if (Utils::ToLowerCase(g1mss[0].bone_names[i]) == lc_name)
                return (uint16_t)i;
        }

        if (Utils::BeginsWith(name, "UnnamedBone#", false))
        {
            uint16_t ret = (uint16_t)Utils::GetUnsigned(name.substr(12), 0xFFFF);
            if (ret < g1mss[0].bones.size())
                return ret;
        }
    }

    return 0xFFFF;
}

G1MGBoneMapEntry *G1mFile::FindBoneMapEntry(G1MGBonesMap *map, uint16_t bone_idx, uint16_t flags, bool ignore_different_cloth, uint32_t ref_cloth)
{
    G1MGBoneMapEntry *ret = nullptr;

    if (map)
    {
        for (G1MGBoneMapEntry &entry : map->map)
        {
            if (ignore_different_cloth && entry.cloth != ref_cloth)
                continue;

            if (entry.mapped == bone_idx && entry.flags == flags)
            {
                ret = &entry;
                break;
            }
        }
    }
    else
    {
        for (G1MGBonesMap &maps : g1mgs[0].bones_map_section.bones_maps)
        {
            ret = FindBoneMapEntry(&maps, bone_idx, flags);
            if (ret)
                break;
        }
    }

    return ret;
}

bool G1mFile::CreateBonesMapFromVgmapFile(const std::string &file, G1MGBonesMap &ret, G1MGBonesMap *ref_map)
{
    if (g1mss.size() == 0)
        return false;

    const G1MSChunk &g1ms = g1mss[0];

    if (g1ms.bone_names.size() == 0)
        SetDefaultBoneNames();

    ret.map.clear();

    std::string content;

    if (!Utils::ReadTextFile(file, content))
        return false;

    std::vector<std::string> lines;
    if (Utils::GetMultipleStrings(content, lines, '\n') == 0)
        return false;

    std::vector<bool> referenced;

    ret.map.reserve(lines.size()-2);
    referenced.reserve(lines.size()-2);

    // Ok, this is not a json parser, we'll just check lines that have a ":", and that's it.

    for (std::string &line : lines)
    {
        Utils::TrimString(line);

        if (line.find(':') == std::string::npos)
            continue;

        std::vector<std::string> values;
        uint32_t vg;

        if (Utils::GetMultipleStrings(line, values, ':') != 2)
        {
            DPRINTF("Failed to understand this line from vgmap: %s\n", line.c_str());
            return false;
        }

        Utils::TrimString(values[0]);
        Utils::TrimString(values[1]);

        std::string &name = values[0];
        uint16_t bone_idx;
        uint16_t flags = 0;

        if (name.length() > 0 && name.front() == '"')
            name = name.substr(1);

        if (name.length() > 0 && name.back() == '"')
            name.pop_back();

        if (values[1].length() > 0 && values[1].back() == ',')
            values[1].pop_back();

        if (name.length() == 0 || values[1].length() == 0)
        {
            DPRINTF("Failed to understand this line from vgmap: %s\n", line.c_str());
            return false;
        }

        if (Utils::BeginsWith(name, "ExternalBone", false))
        {
            bone_idx = (uint16_t)Utils::GetUnsigned(name.substr(12), 0xFFFF);
            flags = 0x8000;
        }
        else
        {
            bone_idx = FindBoneIndexByName(name);
            flags = 0;
        }

        if (bone_idx == 0xFFFF)
        {
            DPRINTF("VGMAP: Cannot find bone \"%s\" in the skeleton.\n", name.c_str());
            return false;
        }

        vg = Utils::GetUnsigned(values[1], 0xFFFFFFFF);
        if (vg == 0xFFFFFFFF)
        {
            DPRINTF("Failed to understand this line from vgmap: %s\n", line.c_str());
            return false;
        }

        if (vg % 3)
        {
            DPRINTF("I was expecting VG to be a multiple of 3, but got value %d\n", vg);
            return false;
        }

        size_t idx = vg / 3;

        if (idx >= ret.map.size())
        {
            ret.map.resize(idx+1);
            referenced.resize(idx+1, false);
        }
        else if (referenced[idx])
        {
            DPRINTF("Failed to import VGMAP: VG group %d had already been used before.\n", vg);
            return false;
        }

        referenced[idx] = true;

        G1MGBoneMapEntry entry;
        G1MGBoneMapEntry *ref_entry = nullptr;

        entry.mapped = bone_idx;
        entry.flags = flags;

        if (ref_map)
            ref_entry = FindBoneMapEntry(ref_map, bone_idx, flags);

        // Try global search
        // This is actually a bug, we should have checked ref_entry instead. But now I'm afraid of breaking something if I fix this.
        if (!ref_map)
            ref_entry = FindBoneMapEntry(nullptr, bone_idx, flags);

        // But on external bone, do always the global search if initial search fails, since we can't create the matrix.
        if (flags == 0x8000 && !ref_entry)
        {
            ref_entry = FindBoneMapEntry(nullptr, bone_idx, flags, true);
            if (!ref_entry)
                ref_entry = FindBoneMapEntry(nullptr, bone_idx, flags);
        }

        if (ref_entry)
        {
            entry.matrix = ref_entry->matrix;
            entry.cloth = ref_entry->cloth;
        }
        else
        {
            if (flags != 0)
            {
                DPRINTF("%s: Cannot create matrix for external bone (%s).\n", FUNCNAME, name.c_str());
                return false;
            }

            // Create new matrix
            entry.cloth = 0;

            G1MMMatrix mat = GetGlobalTransform(bone_idx);

            entry.matrix = (uint16_t)g1mms[0].matrices.size();
            g1mms[0].matrices.push_back(mat);
        }

        ret.map[idx] = entry;
    }

    for (size_t i = 0; i < referenced.size(); i++)
    {
        if (!referenced[i])
        {
            DPRINTF("VGMAP: I was expecting VG %Id to be referenced, but it wasn't.\n", i*3);
            return false;
        }
    }

    return true;
}

size_t G1mFile::CountBonesMapUsage(size_t bm_idx) const
{
    size_t count = 0;

    for (const G1MGSubmesh &surf : g1mgs[0].submeshes_section.submeshes)
    {
        if (surf.bones_map_index == (int)bm_idx)
            count++;
    }

    return count;
}

bool G1mFile::ImportSubmeshFrom3DM(size_t idx, const std::string &vb_file, const std::string &ib_file, const std::string &vgmap)
{
    if (g1mgs.size() == 0)
        return false;

    G1MGChunk &g1mg = g1mgs[0];

    if (!g1mg.submeshes_section.valid)
        return false;

    size_t num_sm = GetNumSubmeshes();

    if (idx >= num_sm)
        return false;

    if (g1mg.vert_section.vertex_bufs.size() != g1mg.index_buf_section.buffers.size())
    {
        DPRINTF("Mismatch number of VB/IB (%Id vs %Id).\n", g1mg.vert_section.vertex_bufs.size(), g1mg.index_buf_section.buffers.size());
        return false;
    }

    G1MGSubmesh &surf = g1mg.submeshes_section.submeshes[idx];
    G1MGVertexBuffer &vb = g1mg.vert_section.vertex_bufs[surf.vertex_buf_ref];
    G1MGIndexBuffer &ib = g1mg.index_buf_section.buffers[surf.index_buf_ref];

    size_t size;
    uint8_t *buf = Utils::ReadFile(vb_file, &size);
    if (!buf)
        return false;

    bool local_vb = false;

    if (num_sm == g1mg.vert_section.vertex_bufs.size())
    {
        vb.vertex.resize(size);
        memcpy(vb.vertex.data(), buf, size);
        delete[] buf;
        surf.num_vertices = vb.GetNumVertex();
    }
    else
    {
        // as of now, this operation may increase the number of vertex by a lot. TODO: optimize that.
        // Atm, we will just skip the case someone is reimporting same mesh:

        if (vb.vertex.size() == size && memcmp(vb.vertex.data(), buf, size) == 0)
        {
            // Skip
            delete[] buf;
        }
        else
        {
            BreakVB(surf.vertex_buf_ref);
            surf.temp_vb.resize(size);
            memcpy(surf.temp_vb.data(), buf, size);
            delete[] buf;
            surf.num_vertices = ((uint32_t)surf.temp_vb.size() / vb.vertex_size);
            MergeVB(surf.vertex_buf_ref);
            local_vb = true;
        }
    }

    buf = Utils::ReadFile(ib_file, &size);
    if (!buf)
        return false;

    // Fix for reimport of 0 size file fucking up the g1m
    if (size == 0)
    {
        delete[] buf;
        surf.num_indices = surf.num_vertices = 0;
    }
    //
    else
    {
        if (num_sm == g1mg.index_buf_section.buffers.size())
        {
            ib.indices.resize(size);
            memcpy(ib.indices.data(), buf, size);
            delete[] buf;
            surf.num_indices = (uint32_t)ib.GetNumIndices();
        }
        else
        {
            auto it = upgraded_ibs.find(surf.index_buf_ref);

            if (it == upgraded_ibs.end())
            {
                bool upgrade;
                size_t num_vertex = vb.GetNumVertex();

                if (ib.type == 8 && num_vertex  >= 256)
                {
                    upgrade = true;
                }
                else if (ib.type == 16 && num_vertex >= 65536)
                {
                    upgrade = true;
                }
                else
                {
                    upgrade = false;
                }

                if (upgrade)
                {
                    uint32_t old_format = ib.type;

                    UpgradeIB(ib, num_vertex);
                    buf = UpgradeBuffer(buf, &size, old_format, ib.type);
                    upgraded_ibs[surf.index_buf_ref] = old_format;
                }
            }
            else
            {
                buf = UpgradeBuffer(buf, &size, (uint32_t)it->second, ib.type);
            }

            BreakIB(surf.index_buf_ref);

            if (local_vb)
            {
                uint32_t first_v = 0;

                for (size_t j = 0; j < idx; j++)
                {
                    if (g1mg.submeshes_section.submeshes[j].index_buf_ref == surf.index_buf_ref)
                    {
                        first_v += g1mg.submeshes_section.submeshes[j].num_vertices;
                    }
                }

                if (ib.type == 8)
                {
                    for (size_t j = 0; j < size; j++)
                        buf[j] += first_v;
                }
                else if (ib.type == 0x10)
                {
                    uint16_t *buf16 = (uint16_t *)buf;
                    size_t size16 = size / 2;

                    for (size_t j = 0; j < size16; j++)
                        buf16[j] += first_v;
                }
                else if (ib.type == 0x20)
                {
                    uint32_t *buf32 = (uint32_t *)buf;
                    size_t size32 = size / 4;

                    for (size_t j = 0; j < size32; j++)
                        buf32[j] += first_v;
                }
            }


            surf.temp_ib.resize(size);
            memcpy(surf.temp_ib.data(), buf, size);
            surf.num_indices = ((uint32_t)surf.temp_ib.size() / ib.GetIndiceSize());
            MergeIB(surf.index_buf_ref);
        }
    }

    if (vgmap.length() == 0) // No vgmap
        return true;

    G1MGBonesMap &ref_map = g1mgs[0].bones_map_section.bones_maps[surf.bones_map_index];
    G1MGBonesMap new_map;

    if (!CreateBonesMapFromVgmapFile(vgmap, new_map, &ref_map))
        return false;

    if (ref_map != new_map)
    {
        if (CountBonesMapUsage(surf.bones_map_index) > 1)
        {
            // Used by some other mesh, we better create a new one
            surf.bones_map_index = (uint32_t)g1mgs[0].bones_map_section.bones_maps.size();
            g1mgs[0].bones_map_section.bones_maps.push_back(new_map);
        }
        else
        {
            // No used by anything else, we'll just overwrite the map
            g1mgs[0].bones_map_section.bones_maps[surf.bones_map_index] = new_map;
        }
    }
    else
    {
        // No need to do anything
    }

    update_g1mf = true;
    return true;
}

#ifdef USE_DIRXMATH

XMMATRIX G1mFile::GetGlobalTransformDX(uint16_t bone_idx) const
{
    const G1MSBone &bone = g1mss[0].bones[bone_idx];
    XMVECTOR qrot = XMLoadFloat4(&XMFLOAT4(bone.rotation[0], bone.rotation[1], bone.rotation[2], bone.rotation[3]));
    XMMATRIX s = XMMatrixScaling(bone.scale[0], bone.scale[1], bone.scale[2]);
    XMMATRIX r = XMMatrixRotationQuaternion(qrot);
    XMMATRIX t = XMMatrixTranslation(bone.position[0], bone.position[1], bone.position[2]);

    s = XMMatrixTranspose(s);
    r = XMMatrixTranspose(r);
    //t = XMMatrixTranspose(t);

    //XMMATRIX m = s*r*t;
    XMMATRIX m = t*r*s;

    if (bone.parent == 0xFFFF)
        return m;

    return GetGlobalTransformDX(bone.parent)*m;
}

G1MMMatrix G1mFile::GetGlobalTransform(uint16_t bone_idx) const
{
    G1MMMatrix gm;
    XMMATRIX xm = GetGlobalTransformDX(bone_idx);

    gm.matrix[0] = xm.r[0].m128_f32[0];
    gm.matrix[1] = xm.r[0].m128_f32[1];
    gm.matrix[2] = xm.r[0].m128_f32[2];
    gm.matrix[3] = xm.r[0].m128_f32[3];

    gm.matrix[4] = xm.r[1].m128_f32[0];
    gm.matrix[5] = xm.r[1].m128_f32[1];
    gm.matrix[6] = xm.r[1].m128_f32[2];
    gm.matrix[7] = xm.r[1].m128_f32[3];

    gm.matrix[8] = xm.r[2].m128_f32[0];
    gm.matrix[9] = xm.r[2].m128_f32[1];
    gm.matrix[10] = xm.r[2].m128_f32[2];
    gm.matrix[11] = xm.r[2].m128_f32[3];

    gm.matrix[12] = xm.r[3].m128_f32[0];
    gm.matrix[13] = xm.r[3].m128_f32[1];
    gm.matrix[14] = xm.r[3].m128_f32[2];
    gm.matrix[15] = xm.r[3].m128_f32[3];

    if (gm.matrix[12] != 0.0f)
        gm.matrix[12] = -gm.matrix[12];

    if (gm.matrix[13] != 0.0f)
        gm.matrix[13] = -gm.matrix[13];

    if (gm.matrix[14] != 0.0f)
        gm.matrix[14] = -gm.matrix[14];

    return gm;
}

#else

D3DXMATRIX G1mFile::GetGlobalTransformDX(uint16_t bone_idx) const
{
    const G1MSBone &bone = g1mss[0].bones[bone_idx];
    D3DXMATRIX s, r, t, m;
    D3DXQUATERNION qr = D3DXQUATERNION(bone.rotation[0], bone.rotation[1], bone.rotation[2], bone.rotation[3]);

    D3DXMatrixScaling(&s, bone.scale[0], bone.scale[1], bone.scale[2]);
    D3DXMatrixRotationQuaternion(&r, &qr);
    D3DXMatrixTranslation(&t, bone.position[0], bone.position[1], bone.position[2]);

    D3DXMatrixTranspose(&s, &s);
    D3DXMatrixTranspose(&r, &r);

    m = t*r*s;

    if (bone.parent == 0xFFFF)
        return m;

    return GetGlobalTransformDX(bone.parent)*m;
}

G1MMMatrix G1mFile::GetGlobalTransform(uint16_t bone_idx) const
{
    G1MMMatrix gm;
    D3DXMATRIX dm = GetGlobalTransformDX(bone_idx);

    gm.matrix[0] = dm._11;
    gm.matrix[1] = dm._12;
    gm.matrix[2] = dm._13;
    gm.matrix[3] = dm._14;

    gm.matrix[4] = dm._21;
    gm.matrix[5] = dm._22;
    gm.matrix[6] = dm._23;
    gm.matrix[7] = dm._24;

    gm.matrix[8] = dm._31;
    gm.matrix[9] = dm._32;
    gm.matrix[10] = dm._33;
    gm.matrix[11] = dm._34;

    gm.matrix[12] = dm._41;
    gm.matrix[13] = dm._42;
    gm.matrix[14] = dm._43;
    gm.matrix[15] = dm._44;

    if (gm.matrix[12] != 0.0f)
        gm.matrix[12] = -gm.matrix[12];

    if (gm.matrix[13] != 0.0f)
        gm.matrix[13] = -gm.matrix[13];

    if (gm.matrix[14] != 0.0f)
        gm.matrix[14] = -gm.matrix[14];

    return gm;
}

#endif

bool G1mFile::HideSubmesh(size_t idx)
{
    if (g1mgs.size() == 0)
        return false;

    G1MGChunk &g1mg = g1mgs[0];

    if (idx >= GetNumSubmeshes())
        return false;

    /*g1mg.vert_section.vertex_bufs.erase(g1mg.vert_section.vertex_bufs.begin()+idx);
    g1mg.index_buf_section.buffers.erase(g1mg.index_buf_section.buffers.begin()+idx);
    g1mg.layout_section.entries.erase(g1mg.layout_section.entries.begin()+idx);
    g1mg.surf_section.surfaces.erase(g1mg.surf_section.surfaces.begin()+idx);*/

    g1mg.submeshes_section.submeshes[idx].num_indices = g1mg.submeshes_section.submeshes[idx].num_vertices = 0;
    update_g1mf = true;

    return true;
}

uint16_t G1mFile::GetNumBones() const
{
    if (g1mss.size() == 0)
        return 0;

    return (uint16_t)g1mss[0].bones.size();
}

uint16_t G1mFile::GetNumBonesID() const
{
    if (g1mss.size() == 0)
        return 0;

    return (uint16_t)g1mss[0].indices.size();
}

bool G1mFile::LoadBoneNames(const std::string &oid_file)
{
    if (g1mss.size() == 0)
        return false;

    G1MSChunk &skl = g1mss[0];

    std::string content;
    if (!Utils::ReadTextFile(oid_file, content))
        return false;

    std::vector<std::string> lines;
    Utils::GetMultipleStrings(content, lines, '\n');

    skl.bone_names.clear();
    skl.bone_names.resize(skl.bones.size());

    for (std::string &line : lines)
    {
        Utils::TrimString(line);

        if (line.front() == ';' || line.front() == '#')
            continue;

        std::vector<std::string> split;
        if (Utils::GetMultipleStrings(line, split) == 2)
        {
            Utils::TrimString(split[0]);
            Utils::TrimString(split[1]);

            uint32_t bone_id = Utils::GetUnsigned(split[0]);
            if (bone_id >= (uint32_t)skl.indices.size())
            {
                /*DPRINTF("This OID is not for this .g1m. (%d >= %Id)\n", bone_id, skl.indices.size());
                return false;*/
                continue;
            }

            uint16_t idx = skl.indices[bone_id];
            if (idx == 0xFFFF)
                continue;

            if (idx < skl.bones.size())
            {
                skl.bone_names[idx] = split[1];
            }
        }
    }

    // Load default names if any is empty
    for (size_t i = 0; i < skl.indices.size(); i++)
    {
        uint16_t idx = skl.indices[i];

        if (idx < skl.bones.size() && skl.bone_names[idx].length() == 0)
        {
            skl.bone_names[idx] = "UnnamedBone#" + Utils::ToString(idx); // Utils::ToString(i);
        }
    }

    return true;
}

void G1mFile::SetDefaultBoneNames()
{
    if (g1mss.size() == 0)
        return;

    G1MSChunk &skl = g1mss[0];
    skl.bone_names.clear();
    skl.bone_names.resize(skl.bones.size());

    for (size_t i = 0; i < skl.indices.size(); i++)
    {
        uint16_t idx = skl.indices[i];

        if (idx < skl.bones.size())
        {
            skl.bone_names[idx] = "UnnamedBone#" + Utils::ToString(idx); //Utils::ToString(i);
        }
    }
}

int G1mFile::BoneIndexToID(uint16_t idx) const
{
    if (g1mss.size() > 0 && idx < g1mss[0].bones.size())
    {
        for (size_t i = 0; i < g1mss[0].indices.size(); i++)
        {
            if (g1mss[0].indices[i] == idx)
                return (int)i;
        }
    }

    return -1;
}

uint16_t G1mFile::BoneIDToIndex(int id) const
{
    if (g1mss.size() > 0 && id >= 0 && id < (int)g1mss[0].indices.size())
    {
        return g1mss[0].indices[(size_t)id];
    }

    return 0xFFFF;
}

bool G1mFile::BoneIndexToName(uint16_t idx, std::string &name) const
{
    if (g1mss.size() > 0 && idx < g1mss[0].bone_names.size())
    {
        name = g1mss[0].bone_names[idx];
        return true;
    }

    return false;
}

int G1mFile::BoneNameToID(const std::string &name) const
{
    uint16_t idx = BoneNameToIndex(name);
    if (idx == 0xFFFF)
        return -1;

    return BoneIndexToID(idx);
}

bool G1mFile::BoneIDToName(int id, std::string &name) const
{
    uint16_t idx = BoneIDToIndex(id);
    if (idx == 0xFFFF)
        return false;

    return BoneIndexToName(idx, name);
}

bool G1mFile::CopyBone(const G1mFile &other, int id, bool resolve_ancestors, bool debug_print)
{
    uint16_t other_idx = other.BoneIDToIndex(id);
    if (other_idx == 0xFFFF)
        return false;

    const G1MSChunk &other_skl = other.g1mss[0];
    G1MSChunk &this_skl = this->g1mss[0];

    const G1MSBone &other_bone = other_skl.bones[other_idx];
    G1MSBone *copy_bone;
    G1MSBone new_bone;
    bool add;

    uint16_t this_idx = this->BoneIDToIndex(id);

    if (this_idx != 0xFFFF)
    {
        copy_bone = &this_skl.bones[this_idx];
        add = false;
    }
    else
    {
        copy_bone = &new_bone;
        add = true;
    }

    *copy_bone = other_bone;

    if (other_bone.parent != 0xFFFF)
    {
        int parent_id = other.BoneIndexToID(other_bone.parent);
        if (parent_id < 0)
            return false;

        if (!this->BoneExists(parent_id))
        {
            if (!resolve_ancestors)
                return false;

            if (!this->CopyBone(other, parent_id, true, debug_print))
                return false;
        }

        uint16_t this_parent_idx = this->BoneIDToIndex(parent_id);
        assert(this_parent_idx != 0xFFFF); // Should not assert

        copy_bone->parent = this_parent_idx;
    }

    if (add)
    {
        if (id >= (int)this_skl.indices.size())
            this_skl.indices.resize((size_t)id+1, 0xFFFF);

        this_skl.indices[(size_t)id] = (uint16_t)this_skl.bones.size();
        this_skl.bones.push_back(new_bone);
    }

    if (debug_print)
    {
        std::string name;

        if (other.BoneIDToName(id, name))
        {
            DPRINTF("Bone %s copied\n", name.c_str());
        }
        else
        {
            DPRINTF("Bone %d copied\n", id);
        }
    }

    update_g1mf = true;
    return true;
}

bool G1mFile::RecalcLodGroup(size_t group)
{
    //DPRINTF("Recalc %Id\n", group);

    if (g1mgs.size() == 0 || !g1mgs[0].meshes_section.valid || !g1mgs[0].submeshes_section.valid)
        return true;

    G1MGMeshesSection &section = g1mgs[0].meshes_section;

    if (group >= section.groups.size())
        return false;

    G1MGLodGroup &lg = section.groups[group];

    std::vector<G1MGMesh> meshes1, meshes2;

    lg.count1 = 0;
    lg.count2 = 0;

    for (const G1MGMesh &mesh : lg.meshes)
    {
        for (uint32_t sm_idx : mesh.submeshes)
        {
            if (sm_idx >= (uint32_t)g1mgs[0].submeshes_section.submeshes.size())
                return false;

            const G1MGSubmesh &sm = g1mgs[0].submeshes_section.submeshes[sm_idx];
            if (sm.flags & 8)
            {
                meshes2.push_back(mesh);
                lg.count2++;
                break;
            }
            else
            {
                meshes1.push_back(mesh);
                lg.count1++;
                break;
            }
        }
    }

    lg.meshes = meshes1;
    lg.meshes.insert(lg.meshes.end(), meshes2.begin(), meshes2.end());

    return true;
}



bool G1mFile::HasExternalSkeleton() const
{
    if (g1mss.size() == 0)
        return true;

    if (g1mss[0].bones.size() == 0)
        return true;

    for (const G1MSBone &bone : g1mss[0].bones)
    {
        if (bone.flags == 0x8000)
            return true;
    }

    return false;
}

#ifdef FBX_SUPPORT

bool G1mFile::GetPosition(const uint8_t *vbuf, FbxVector4 &pos, uint16_t format) const
{
    const float *pf32 = (const float *)vbuf;

    if (format == 2)
    {
        pos = FbxVector4((double)pf32[0], (double)pf32[1], (double)pf32[2]);
    }
    else if (format == 3)
    {
        pos = FbxVector4((double)pf32[0], (double)pf32[1], (double)pf32[2], (double)pf32[3]);
    }
    else
    {
        DPRINTF("Unknown format for position (%d)\n", format);
        return false;
    }

    return true;
}

bool G1mFile::GetNormal(const uint8_t *vbuf, FbxVector4 &normal, uint16_t format) const
{
    const float *pf32 = (const float *)vbuf;
    const uint16_t *pf16 = (const uint16_t *)vbuf;

    if (format == 2)
    {
        normal = FbxVector4((double)pf32[0], (double)pf32[1], (double)pf32[2]);
    }
    else if (format == 3)
    {
        normal = FbxVector4((double)pf32[0], (double)pf32[1], (double)pf32[2], (double)pf32[3]);
    }
    else if (format == 11)
    {
        normal = FbxVector4((double)half_float::detail::half2float(pf16[0]),
                            (double)half_float::detail::half2float(pf16[1]),
                            (double)half_float::detail::half2float(pf16[2]),
                            (double)half_float::detail::half2float(pf16[3]));
    }
    else
    {
        DPRINTF("Unknown format for normal (%d)\n", format);
        return false;
    }

    return true;
}

bool G1mFile::GetUV(const uint8_t *vbuf, FbxVector2 &uv, uint16_t format) const
{
    const float *pf32 = (const float *)vbuf;
    const uint16_t *pf16 = (const uint16_t *)vbuf;

    if (format == 1)
    {
        uv = FbxVector2((double)pf32[0], (double)pf32[1]);
    }
    else if (format == 10)
    {
        uv = FbxVector2((double)half_float::detail::half2float(pf16[0]), (double)half_float::detail::half2float(pf16[1]));
    }
    else
    {
        DPRINTF("Unknown format for UV (%d)\n", format);
        return false;
    }

    return true;
}

bool G1mFile::GetWeights(const uint8_t *vbuf, float *vec, uint16_t format) const
{
    const float *pf32 = (const float *)vbuf;
    const uint16_t *pf16 = (const uint16_t *)vbuf;

    if (format == 1)
    {
        vec[0] = pf32[0];
        vec[1] = pf32[1];
        vec[2] = 1.0f - vec[0] - vec[1];
        if (vec[2] < 0.0001f) vec[2] = 0.0f;
        vec[3] = 0.0f;
    }
    else if (format == 2)
    {
        vec[0] = pf32[0];
        vec[1] = pf32[1];
        vec[2] = pf32[2];
        vec[3] = 1.0f - vec[0] - vec[1] - vec[2];
        if (vec[3] < 0.0001f) vec[3] = 0.0f;
    }
    else if (format == 3)
    {
        vec[0] = pf32[0];
        vec[1] = pf32[1];
        vec[2] = pf32[2];
        vec[3] = pf32[3];
    }
    else if (format == 10)
    {
        vec[0] = half_float::detail::half2float(pf16[0]);
        vec[1] = half_float::detail::half2float(pf16[1]);
        vec[2] = 1.0f - vec[0] - vec[1];
        if (vec[2] < 0.0001f) vec[2] = 0.0f;
        vec[3] = 0.0f;
    }
    else if (format == 11)
    {
        vec[0] = half_float::detail::half2float(pf16[0]);
        vec[1] = half_float::detail::half2float(pf16[1]);
        vec[2] = half_float::detail::half2float(pf16[2]);
        vec[3] = half_float::detail::half2float(pf16[3]);
    }
    else
    {
        DPRINTF("Unknown format for weights (%d)\n", format);
        return false;
    }

    return true;
}

bool G1mFile::GetIndices(const uint8_t *vbuf, uint32_t *vec, uint16_t format) const
{
    const uint8_t *pu8 = (const uint8_t *)vbuf;
    const uint16_t *pu16 = (const uint16_t *)vbuf;

    if (format == 5)
    {
        vec[0] = pu8[0];
        vec[1] = pu8[1];
        vec[2] = pu8[2];
        vec[3] = pu8[3];
    }
    else if (format == 7)
    {
        vec[0] = pu16[0];
        vec[1] = pu16[1];
        vec[2] = pu16[2];
        vec[3] = pu16[3];
    }
    else
    {
        DPRINTF("Unknown format for indices (%d)\n", format);
        return false;
    }

    return true;
}

bool G1mFile::ExportBoneToFbx(uint16_t parent, FbxNode *root_node, FbxScene *scene, std::vector<FbxNode *> &fbx_bones) const
{
    const std::vector<G1MSBone> &bones = g1mss[0].bones;

    for (uint16_t i = 0; i < (uint16_t)bones.size(); i++)
    {
        const G1MSBone &bone = bones[i];

        if (bone.parent != parent)
            continue;

        std::string bone_name = g1mss[0].bone_names[i];

        FbxSkeleton* skeleton_root_attribute = FbxSkeleton::Create(scene, bone_name.c_str());

        if (parent == 0xFFFF)
        {
            skeleton_root_attribute->SetSkeletonType(FbxSkeleton::eRoot);
        }
        else
        {
            skeleton_root_attribute->SetSkeletonType(FbxSkeleton::eLimbNode);
        }

        FbxNode* fbx_bone_node = FbxNode::Create(scene, bone_name.c_str());
        fbx_bone_node->SetNodeAttribute(skeleton_root_attribute);
        root_node->AddChild(fbx_bone_node);

        fbx_bone_node->LclTranslation.Set(FbxDouble3((double)bone.position[0], (double)bone.position[1], (double)bone.position[2]));

        FbxVector4 rot;
        rot.SetXYZ(FbxQuaternion((double)bone.rotation[0], (double)bone.rotation[1], (double)bone.rotation[2], (double)bone.rotation[3]));

        fbx_bone_node->LclRotation.Set(rot);
        fbx_bone_node->LclScaling.Set(FbxDouble3((double)bone.scale[0], (double)bone.scale[1], (double)bone.scale[2]));

        fbx_bones[i] = fbx_bone_node;
        ExportBoneToFbx(i, fbx_bone_node, scene, fbx_bones);
    }

    return true;
}

bool G1mFile::ExportSkinToFbx(const G1MGBonesMap &bmap, const std::vector<float> &weights,  const std::vector<uint32_t> &indices, const std::vector<bool> &referenced, const std::vector<FbxNode *> &fbx_bones, FbxScene *scene, FbxMesh *fbx_mesh, FbxAMatrix skin_matrix, const G1mFile &skl_g1m) const
{
    UNUSED(skin_matrix);

    FbxSkin* skin = FbxSkin::Create(scene, "");
    std::vector<FbxCluster *> skin_clusters;

    skin_clusters.resize(bmap.map.size());

    for (size_t i = 0; i < skin_clusters.size(); i++)
    {
        FbxNode *fbx_bone = nullptr;

        for (size_t j = 0; j < fbx_bones.size(); j++)
        {
            if (skl_g1m.g1mss[0].bone_names[bmap.map[i].mapped] == fbx_bones[j]->GetName())
            {
                fbx_bone = fbx_bones[j];
                //DPRINTF("%Id\n", j);
                break;
            }
        }

        if (!fbx_bone)
            return false;

        //DPRINTF("%s: %d \n", fbx_bone->GetName(), i*3);

        FbxCluster *cluster = FbxCluster::Create(scene, fbx_bone->GetName());

        cluster->SetLink(fbx_bone);
        cluster->SetLinkMode(FbxCluster::eTotalOne);
        //cluster->SetTransformMatrix(skin_matrix);

        FbxAMatrix XMatrix = fbx_bone->EvaluateGlobalTransform();
        cluster->SetTransformLinkMatrix(XMatrix);

        skin->AddCluster(cluster);
        skin_clusters[i] = cluster;
    }

    for (size_t i = 0; i < weights.size() / 4; i++)
    {
        if (!referenced[i])
            continue;

        for (size_t j = 0; j < 4; j++)
        {
            size_t n = i*4 + j;

            if (indices[n] % 3)
            {
                //DPRINTF("Not multiple of 3 %d\n", indices[n]);
                //return false;
                continue;
            }

            size_t idx = indices[n] / 3;

            if (idx >= skin_clusters.size())
            {
                //DPRINTF("Skin export, out range.  %Id  %Id\n", idx, skin_clusters.size());
                //return false;
                continue;
            }

            skin_clusters[idx]->AddControlPointIndex((int)i, (double)weights[n]);
        }
    }

    fbx_mesh->AddDeformer(skin);
    return true;
}


bool G1mFile::ExportMeshToFbx(size_t idx, FbxScene *scene, std::vector<FbxNode *> *pfbx_bones, const G1mFile *external_skl) const
{
    if (g1mgs.size() == 0)
        return false;

    const G1MGVertexSection &vsection = g1mgs[0].vert_section;
    const G1MGIndexBufferSection &isection = g1mgs[0].index_buf_section;
    const G1MGLayoutSection &lsection = g1mgs[0].layout_section;
    const G1MGSubmeshesSection &ssection = g1mgs[0].submeshes_section;

    if (idx >= ssection.submeshes.size())
        return false;

    const G1MGSubmesh &surf = ssection.submeshes[idx];

    if (surf.num_indices == 0)
        return true;

    const G1MGVertexBuffer &vb = vsection.vertex_bufs[surf.vertex_buf_ref];
    const G1MGIndexBuffer &ib = isection.buffers[surf.index_buf_ref];
    const G1MGLayout &ly = lsection.entries[surf.vertex_buf_ref];

    std::string mesh_name = "Mesh" + Utils::ToString(idx);

    FbxMesh *fbx_mesh = FbxMesh::Create(scene, mesh_name.c_str());
    if (!fbx_mesh)
        return false;

    fbx_mesh->InitControlPoints((int)vb.GetNumVertex());

    FbxGeometryElementNormal *fbx_normal = nullptr;
    FbxGeometryElementUV *fbx_uv = nullptr;

    size_t pos_offset = (size_t)-1, normal_offset = 0, uv_offset = 0, weights_offset = 0, indices_offset = 0;
    uint16_t pos_format = 0, normal_format = 0, uv_format = 0, weights_format = 0, indices_format = 0;

    bool has_weights = false;
    bool has_indices = false;

    for (const G1MGSemantic &sem : ly.semantics)
    {
        uint8_t type = sem.semantic&0xFF;
        uint8_t idx = sem.semantic >> 8;

        if (idx != 0)
            continue;

        if (type == 0)
        {
            pos_offset = sem.offset;
            pos_format = sem.data_type;
        }
        else if (type == 3)
        {
            normal_offset = sem.offset;
            normal_format = sem.data_type;

            fbx_normal = fbx_mesh->CreateElementNormal();
            fbx_normal->SetMappingMode(FbxGeometryElement::eByControlPoint);
            //fbx_normal->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
            fbx_normal->SetReferenceMode(FbxGeometryElement::eDirect);
        }
        else if (type == 5)
        {
            uv_offset = sem.offset;
            uv_format = sem.data_type;

            fbx_uv = fbx_mesh->CreateElementUV("DiffuseUV");
            fbx_uv->SetMappingMode(FbxGeometryElement::eByControlPoint);
            //fbx_uv->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
            fbx_uv->SetReferenceMode(FbxGeometryElement::eDirect);
        }
        else if (pfbx_bones)
        {
            if (type == 1)
            {
                has_weights = true;
                weights_offset = sem.offset;
                weights_format = sem.data_type;
            }
            else if (type == 2)
            {
                has_indices = true;
                indices_offset = sem.offset;
                indices_format = sem.data_type;
            }
        }
    }

    if (pos_offset == (size_t)-1)
    {
        DPRINTF("No position info found on mesh %Id\n", idx);
        return false;
    }

    if (!fbx_normal)
    {
        DPRINTF("Warning: no normal on mesh %Id\n", idx);
    }

    if (!fbx_uv)
    {
        DPRINTF("Warning: no uv on mesh %Id\n", idx);
    }

    if (has_weights)
    {
        if (!has_indices)
            DPRINTF("Warning: %Id has weights but not indices.\n", idx);
    }
    else if (has_indices)
    {
        DPRINTF("Warning: %Id has indices but not weights.\n", idx);
    }

    FbxNode *mesh_node = FbxNode::Create(scene, mesh_name.c_str());
    mesh_node->LclTranslation.Set(FbxVector4(0, 0, 0));
    mesh_node->LclScaling.Set(FbxVector4(1, 1, 1));
    mesh_node->LclRotation.Set(FbxVector4(0, 0, 0));

    std::vector<float> weights;
    std::vector<uint32_t> indices;

    FbxVector4 *control_points = fbx_mesh->GetControlPoints();

    const uint8_t *vb_buf = vb.vertex.data();
    const uint8_t *ib_buf = ib.indices.data();

    const uint8_t *vb_ptr = vb_buf;
    //const uint8_t *ib_ptr = ib_buf;

    if (has_weights)
        weights.reserve(vb.GetNumVertex()*4);

    if (has_indices)
        indices.reserve(vb.GetNumVertex()*4);

    for (size_t i = 0, v = 0; i < vb.vertex.size(); v++)
    {
        FbxVector4 pos;

        if (!GetPosition(vb_ptr + pos_offset, pos, pos_format))
            return false;

        control_points[v] = pos;

        if (fbx_normal)
        {
            FbxVector4 normal;

            if (!GetNormal(vb_ptr + normal_offset, normal, normal_format))
                return false;

            fbx_normal->GetDirectArray().Add(normal);
        }

        if (fbx_uv)
        {
            FbxVector2 uv;

            if (!GetUV(vb_ptr + uv_offset, uv, uv_format))
                return false;

            fbx_uv->GetDirectArray().Add(uv);
        }

        if (has_weights)
        {
            float w[4];

            if (!GetWeights(vb_ptr + weights_offset, w, weights_format))
                return false;

            weights.insert(weights.end(), w, w+4);
        }

        if (has_indices)
        {
            uint32_t in[4];

            if (!GetIndices(vb_ptr + indices_offset, in, indices_format))
                return false;

            indices.insert(indices.end(), in, in+4);
        }

        i += vb.vertex_size;
        vb_ptr = vb_buf + i;
    }

    size_t num_faces = surf.num_indices;
    if (num_faces % 3)
    {
        DPRINTF("Num faces not multiple of 3 (%Id)\n", num_faces);
        return false;
    }

    const uint8_t *indices8 = (const uint8_t *)ib_buf + surf.index_buf_start;
    const uint16_t *indices16 = ((const uint16_t *)ib_buf) + surf.index_buf_start;
    const uint32_t *indices32 = ((const uint32_t *)ib_buf) + surf.index_buf_start;
    std::vector<bool> referenced;

    if (has_indices && has_weights)
    {
        referenced.resize(vb.GetNumVertex(), false);
    }

    for (size_t i = 0; i < num_faces; i += 3)
    {
        fbx_mesh->BeginPolygon();

        if (ib.type == 8)
        {
            fbx_mesh->AddPolygon(indices8[i]);
            fbx_mesh->AddPolygon(indices8[i+1]);
            fbx_mesh->AddPolygon(indices8[i+2]);

            if (referenced.size() > 0)
            {
                referenced[indices8[i]] = true;
                referenced[indices8[i+1]] = true;
                referenced[indices8[i+2]] = true;
            }
        }
        else if (ib.type == 0x10)
        {
            fbx_mesh->AddPolygon(indices16[i]);
            fbx_mesh->AddPolygon(indices16[i+1]);
            fbx_mesh->AddPolygon(indices16[i+2]);

            if (referenced.size() > 0)
            {
                referenced[indices16[i]] = true;
                referenced[indices16[i+1]] = true;
                referenced[indices16[i+2]] = true;
            }
        }
        else if (ib.type == 0x20)
        {
            fbx_mesh->AddPolygon((int)indices32[i]);
            fbx_mesh->AddPolygon((int)indices32[i+1]);
            fbx_mesh->AddPolygon((int)indices32[i+2]);

            if (referenced.size() > 0)
            {
                referenced[indices32[i]] = true;
                referenced[indices32[i+1]] = true;
                referenced[indices32[i+2]] = true;
            }
        }
        else
        {
            DPRINTF("Unknown indices format (%d)\n", ib.type);
            return false;
        }

        fbx_mesh->EndPolygon();
    }

    mesh_node->SetNodeAttribute(fbx_mesh);

    if (pfbx_bones && has_indices && has_weights)
    {
        if ((size_t)surf.bones_map_index >= g1mgs[0].bones_map_section.bones_maps.size())
            return false;

        //DPRINTF("%Id %d\n", idx, surf.bones_map_index);

        if (!ExportSkinToFbx(g1mgs[0].bones_map_section.bones_maps[surf.bones_map_index], weights, indices, referenced, *pfbx_bones,
                             scene, fbx_mesh, mesh_node->EvaluateGlobalTransform(), (external_skl) ? *external_skl : *this))
        {
            DPRINTF("Skin failed on %Id\n", idx);
            return false;
        }
    }

    scene->GetRootNode()->AddChild(mesh_node);

    return true;
}

bool G1mFile::ExportFbxSkeleton(FbxScene *scene, std::vector<FbxNode *> &fbx_bones)
{
    if (HasExternalSkeleton())
        return false;

    if (g1mss.size() == 0)
        return false;

    if (g1mss[0].bone_names.size() == 0)
        SetDefaultBoneNames();

    fbx_bones.resize(g1mss[0].bones.size());
    return ExportBoneToFbx(0xFFFF, scene->GetRootNode(), scene, fbx_bones);
}

bool G1mFile::ExportFbx(FbxScene *scene, std::vector<FbxNode *> *pfbx_bones, G1mFile *external_skl) const
{
    if (g1mgs.size() == 0)
        return false;

    size_t num_sm = GetNumSubmeshes();

    for (size_t i = 0; i < num_sm; i++)
    {
        if (!ExportMeshToFbx(i, scene, pfbx_bones, (external_skl) ? external_skl : this))
            return false;
    }

    return true;
}

#endif


void G1mFile::DebugMatrixTest() const
{
    const G1MGBonesMapSection &bsec = g1mgs[0].bones_map_section;
    const G1MMChunk &chunk = g1mms[0];

    for (const G1MGBonesMap &map : bsec.bones_maps)
    {
        for (const G1MGBoneMapEntry &entry : map.map)
        {
            const G1MMMatrix &matrix = chunk.matrices[entry.matrix];

            /*XMMATRIX tm1 = XMMATRIX(matrix1.matrix);
            XMMATRIX tm2 = GetGlobalTransformDX(entry.mapped);


            DPRINTF("------------ Bone %d (matrix %d, unk %d)\nOriginal matrix:\n", entry.mapped, entry.matrix, entry.unk_04);
            DPRINTF("%f %f %f  %f\n", tm1.r[0].m128_f32[0], tm1.r[0].m128_f32[1], tm1.r[0].m128_f32[2], tm1.r[0].m128_f32[3]);
            DPRINTF("%f %f %f  %f\n", tm1.r[1].m128_f32[0], tm1.r[1].m128_f32[1], tm1.r[1].m128_f32[2], tm1.r[1].m128_f32[3]);
            DPRINTF("%f %f %f  %f\n", tm1.r[2].m128_f32[0], tm1.r[2].m128_f32[1], tm1.r[2].m128_f32[2], tm1.r[2].m128_f32[3]);
            DPRINTF("%f %f %f  %f\n", tm1.r[3].m128_f32[0], tm1.r[3].m128_f32[1], tm1.r[3].m128_f32[2], tm1.r[3].m128_f32[3]);

            DPRINTF("Calculated matrix (global):\n");
            DPRINTF("%f %f %f  %f\n", tm2.r[0].m128_f32[0], tm2.r[0].m128_f32[1], tm2.r[0].m128_f32[2], tm2.r[0].m128_f32[3]);
            DPRINTF("%f %f %f  %f\n", tm2.r[1].m128_f32[0], tm2.r[1].m128_f32[1], tm2.r[1].m128_f32[2], tm2.r[1].m128_f32[3]);
            DPRINTF("%f %f %f  %f\n", tm2.r[2].m128_f32[0], tm2.r[2].m128_f32[1], tm2.r[2].m128_f32[2], tm2.r[2].m128_f32[3]);
            DPRINTF("%f %f %f  %f\n", tm2.r[3].m128_f32[0], tm2.r[3].m128_f32[1], tm2.r[3].m128_f32[2], tm2.r[3].m128_f32[3]);*/

            G1MMMatrix tm1, tm2;

            tm1 = matrix;
            tm2 = GetGlobalTransform((uint16_t)entry.mapped);

            DPRINTF("------------ Bone %d (matrix %d, unk %d)\nOriginal matrix:\n", entry.mapped, entry.matrix, entry.cloth);
            DPRINTF("%f %f %f  %f\n", (double)tm1.matrix[0], (double)tm1.matrix[1], (double)tm1.matrix[2], (double)tm1.matrix[3]);
            DPRINTF("%f %f %f  %f\n", (double)tm1.matrix[4], (double)tm1.matrix[5],(double) tm1.matrix[6], (double)tm1.matrix[7]);
            DPRINTF("%f %f %f  %f\n", (double)tm1.matrix[8], (double)tm1.matrix[9],(double) tm1.matrix[10], (double)tm1.matrix[11]);
            DPRINTF("%f %f %f  %f\n", (double)tm1.matrix[12], (double)tm1.matrix[13], (double)tm1.matrix[14],(double)tm1.matrix[15]);

            DPRINTF("Calculated matrix (global):\n");
            DPRINTF("%f %f %f  %f\n", (double)tm2.matrix[0], (double)tm2.matrix[1], (double)tm2.matrix[2], (double)tm2.matrix[3]);
            DPRINTF("%f %f %f  %f\n", (double)tm2.matrix[4], (double)tm2.matrix[5], (double)tm2.matrix[6], (double)tm2.matrix[7]);
            DPRINTF("%f %f %f  %f\n", (double)tm2.matrix[8], (double)tm2.matrix[9], (double)tm2.matrix[10], (double)tm2.matrix[11]);
            DPRINTF("%f %f %f  %f\n", (double)tm2.matrix[12], (double)tm2.matrix[13], (double)tm2.matrix[14], (double)tm2.matrix[15]);
        }
    }
}

#ifndef USE_DIRXMATH
void G1mFile::DebugBonesTest()
{
    const G1MSChunk &skl = g1mss[0];
    if (skl.bone_names.size() == 0)
        SetDefaultBoneNames();

    for (size_t i = 0; i < skl.bones.size(); i++)
    {
        D3DXMATRIX m = GetGlobalTransformDX((uint16_t)i);

        //DPRINTF("%f %f %f  %f /   %f %f %f %f\n",m._D3DMATRIX::_41, m._D3DMATRIX::_42, m._D3DMATRIX::_43, m._D3DMATRIX::_44, m._D3DMATRIX::_14, m._D3DMATRIX::_24, m._D3DMATRIX::_34, m._D3DMATRIX::_44);
        float x = m._D3DMATRIX::_41;
        float y = m._D3DMATRIX::_42;
        float z = m._D3DMATRIX::_43;

        DPRINTF("%s %f %f %f\n", skl.bone_names[i].c_str(), x, y, z);

    }
}
#endif

bool G1mFile::DebugTest(const G1mFile &skl)
{
    std::vector<uint16_t> list =
    {
        333, 334, 335, 336, 337, 338,
        2000, 2001, 2002, 2003, 2004,
        2010, 2011, 2012, 2013, 2014,
        2020, 2021, 2022, 2023, 2024,
        2030, 2031, 2032, 2033, 2034,
        2040, 2041, 2042, 2043, 2044,
        2050, 2051, 2052, 2053, 2054,
        2060, 2061, 2062, 2063, 2064,
        2070, 2071, 2072, 2073, 2074,
        2080, 2081, 2082, 2083, 2084,
    };

    G1MGBonesMap &bm = g1mgs[0].bones_map_section.bones_maps[15];

    for (uint16_t id : list)
    {
        uint16_t idx = skl.BoneIDToIndex(id);
        if (idx == 0xFFFF)
        {
            DPRINTF("Failed to get index for ID %u\n", id);
            return false;
        }

        G1MMMatrix mat = skl.GetGlobalTransform(idx);
        G1MGBoneMapEntry entry;

        entry.cloth = -2147483648;
        entry.flags = 0x8000;
        entry.mapped = idx;
        entry.matrix = (uint16_t)g1mms[0].matrices.size();
        g1mms[0].matrices.push_back(mat);
        bm.map.push_back(entry);
    }

    return true;
}

bool G1mFile::DebugUpgradeUV(size_t sm_idx)
{
    if (g1mgs.size() == 0 || sm_idx >= g1mgs[0].submeshes_section.submeshes.size())
        return false;

    if (g1mgs[0].layout_section.entries.size() != g1mgs[0].vert_section.vertex_bufs.size())
        return false;

    G1MGSubmesh &sm = g1mgs[0].submeshes_section.submeshes[sm_idx];
    if (sm.vertex_buf_ref >= (int32_t)g1mgs[0].vert_section.vertex_bufs.size())
        return false;

    G1MGVertexBuffer &vb = g1mgs[0].vert_section.vertex_bufs[sm.vertex_buf_ref];
    G1MGLayout &ly = g1mgs[0].layout_section.entries[sm.vertex_buf_ref];

    bool found = false;
    uint16_t offset = 0;

    for (G1MGSemantic &sem : ly.semantics)
    {
        uint8_t type = sem.semantic&0xFF;
        uint8_t idx = sem.semantic >> 8;

        if (idx != 0)
            continue;

        if (type == 5)
        {
            if (sem.data_type == 1)
                return true;

            if (sem.data_type != 10)
                return false;

            found = true;
            offset = sem.offset;
            sem.data_type = 1;
        }
    }

    if (!found)
        return true;

    for (G1MGSemantic &sem : ly.semantics)
    {
        if (sem.offset > offset)
        {
            sem.offset += 4;
        }
    }

    std::vector<uint8_t> ret;
    ret.reserve((size_t)(vb.vertex.size() * 1.5f));

    for (size_t i = 0; i < vb.GetNumVertex(); i++)
    {
        uint8_t *top = vb.vertex.data() + (i*vb.vertex_size);
        uint32_t remaining_size = vb.vertex_size;

        if (offset != 0)
        {
            size_t prev_size = ret.size();
            ret.resize(prev_size+offset);

            memcpy(ret.data()+prev_size, top, offset);
            remaining_size -= offset;
        }

        {

            uint16_t *orig_uv = (uint16_t *)(top+offset);
            float new_uv[2];

            new_uv[0] = Utils::HalfToFloat(orig_uv[0]);
            new_uv[1] = Utils::HalfToFloat(orig_uv[1]);

            size_t prev_size = ret.size();
            ret.resize(prev_size+8);

            memcpy(ret.data()+prev_size, new_uv, 8);
            remaining_size -= 4;
        }

        uint8_t *after_uv = top + offset + 4;

        if (remaining_size != 0)
        {
            size_t prev_size = ret.size();
            ret.resize(prev_size+remaining_size);

            memcpy(ret.data()+prev_size, after_uv, remaining_size);
        }
    }

    vb.vertex_size += 4;
    vb.vertex = ret;
    return true;
}

bool G1mFile::DebugWeight3To4(size_t *pcount)
{
    *pcount = 0;

    if (g1mgs.size() == 0 || g1mgs[0].vert_section.vertex_bufs.size() != g1mgs[0].layout_section.entries.size())
        return false;

    for (size_t i = 0; i < g1mgs[0].vert_section.vertex_bufs.size(); i++)
    {
        G1MGVertexBuffer &vb = g1mgs[0].vert_section.vertex_bufs[i];
        G1MGLayout &ly = g1mgs[0].layout_section.entries[i];

        bool found = false;
        uint16_t offset = 0;

        for (G1MGSemantic &sem : ly.semantics)
        {
            uint8_t type = sem.semantic&0xFF;
            uint8_t idx = sem.semantic >> 8;

            if (idx != 0)
                continue;

            if (type == 1 && sem.data_type == 2)
            {
                found = true;
                offset = sem.offset;
                sem.data_type = 3;
            }
        }

        if (!found)
            continue;

        for (G1MGSemantic &sem : ly.semantics)
        {
            if (sem.offset > offset)
            {
                sem.offset += 4;
            }
        }

        std::vector<uint8_t> ret;
        ret.reserve((size_t)(vb.vertex.size() * 1.5f));

        for (size_t i = 0; i < vb.GetNumVertex(); i++)
        {
            uint8_t *top = vb.vertex.data() + (i*vb.vertex_size);
            uint32_t remaining_size = vb.vertex_size;

            if (offset != 0)
            {
                size_t prev_size = ret.size();
                ret.resize(prev_size+offset);

                memcpy(ret.data()+prev_size, top, offset);
                remaining_size -= offset;
            }

            {

                float *orig_weights = (float *)(top+offset);
                float new_weights[4];

                memcpy(new_weights, orig_weights, 3*sizeof(float));
                new_weights[3] = 0.0f;

                size_t prev_size = ret.size();
                ret.resize(prev_size+16);

                memcpy(ret.data()+prev_size, new_weights, 16);
                remaining_size -= 12;
            }

            uint8_t *after_weights = top + offset + 12;

            if (remaining_size != 0)
            {
                size_t prev_size = ret.size();
                ret.resize(prev_size+remaining_size);

                memcpy(ret.data()+prev_size, after_weights, remaining_size);
            }
        }

        vb.vertex_size += 4;
        vb.vertex = ret;
        *pcount += 1;
    }

    return true;
}

bool G1mFile::DebugUVNormalTangentTransfer(size_t idx, const G1mFile &other, uint32_t matpalid_filter)
{
    if (g1mgs.size() == 0 || other.g1mgs.size() == 0 ||
        g1mgs[0].vert_section.vertex_bufs.size() != g1mgs[0].layout_section.entries.size() ||
        other.g1mgs[0].vert_section.vertex_bufs.size() != other.g1mgs[0].layout_section.entries.size())
        return false;

    if (idx >= g1mgs[0].submeshes_section.submeshes.size())
        return false;

    G1MGSubmesh &sm_src = g1mgs[0].submeshes_section.submeshes[idx];

    if (sm_src.vertex_buf_ref >= (int32_t)g1mgs[0].vert_section.vertex_bufs.size())
        return false;

    G1MGVertexBuffer &vb_src = g1mgs[0].vert_section.vertex_bufs[sm_src.vertex_buf_ref];
    const G1MGLayout &ly_src = g1mgs[0].layout_section.entries[sm_src.vertex_buf_ref];

    uint16_t src_pos_ofs = 0xFFFF, src_uv_ofs = 0xFFFF, src_normal_ofs = 0xFFFF, src_tangent_ofs = 0xFFFF;
    bool src_uv16 = false, src_normal16 = false, src_tangent16 = false;

    for (const G1MGSemantic &sem : ly_src.semantics)
    {
        uint8_t type = sem.semantic&0xFF;
        uint8_t idx = sem.semantic >> 8;

        if (idx != 0)
            continue;

        if (type == 0)
        {
            src_pos_ofs = sem.offset;

            if (sem.data_type != 2)
            {
                DPRINTF("%s: Unsupported data type(pos) %d.\n", FUNCNAME, sem.data_type);
                return false;
            }
        }
        else if (type == 3)
        {
            src_normal_ofs = sem.offset;

            if (sem.data_type == 11)
            {
                src_normal16 = true;
            }
            else if (sem.data_type != 3)
            {

                DPRINTF("%s: Unsupported data type(normal) %d.\n", FUNCNAME, sem.data_type);
                return false;
            }
        }
        else if (type == 5)
        {
            src_uv_ofs = sem.offset;

            if (sem.data_type == 10)
            {
                src_uv16 = true;
            }
            else if (sem.data_type != 1)
            {
                DPRINTF("%s: Unsupported data type(uv) %d.\n", FUNCNAME, sem.data_type);
                return false;
            }
        }
        else if (type == 6)
        {
            src_tangent_ofs = sem.offset;

            if (sem.data_type == 11)
            {
                src_tangent16 = true;
            }
            else if (sem.data_type != 3)
            {

                DPRINTF("%s: Unsupported data type(tangent) %d.\n", FUNCNAME, sem.data_type);
                return false;
            }
        }
    }

    if (src_pos_ofs == 0xFFFF || src_uv_ofs == 0xFFFF || src_normal_ofs == 0xFFFF)
        return false;

    for (uint32_t i = 0; i < sm_src.num_vertices; i++)
    {
        uint32_t v = sm_src.vertex_buf_start + i;
        uint8_t *src_vertex_top = vb_src.vertex.data() + (v * vb_src.vertex_size);
        const uint8_t *src_pos_buf = src_vertex_top + src_pos_ofs;
        uint8_t *src_uv_buf = src_vertex_top + src_uv_ofs;
        uint8_t *src_normal_buf = src_vertex_top + src_normal_ofs;
        uint8_t *src_tangent_buf = src_vertex_top + src_tangent_ofs;

        Vector3 src_pos((const float *)src_pos_buf);
        Vector2 ret_uv;
        Vector4 ret_normal;
        Vector4 ret_tangent;
        float distance = 100000.0f;

        for (size_t j = 0; j < other.g1mgs[0].vert_section.vertex_bufs.size(); j++)
        {
            const G1MGVertexBuffer &vb_dst = other.g1mgs[0].vert_section.vertex_bufs[j];
            const G1MGLayout &ly_dst = other.g1mgs[0].layout_section.entries[j];

            if (matpalid_filter != 0xFFFFFFFF)
            {
                bool include = false;

                for (const G1MGSubmesh &sm : other.g1mgs[0].submeshes_section.submeshes)
                {
                    if (sm.vertex_buf_ref == (int32_t)j && sm.matpalid == matpalid_filter)
                    {
                        include = true;
                        break;
                    }
                }

                if (!include)
                    continue;
            }

            uint16_t dst_pos_ofs = 0xFFFF, dst_uv_ofs = 0xFFFF, dst_normal_ofs = 0xFFFF, dst_tangent_ofs = 0xFFFF;
            bool dst_uv16 = false, dst_normal16 = false, dst_tangent16 = false;

            for (const G1MGSemantic &sem : ly_dst.semantics)
            {
                uint8_t type = sem.semantic&0xFF;
                uint8_t idx = sem.semantic >> 8;

                if (idx != 0)
                    continue;

                if (type == 0)
                {
                    dst_pos_ofs = sem.offset;

                    if (sem.data_type != 2)
                    {
                        //DPRINTF("%s: Unsupported data type(pos) %d.\n", FUNCNAME, sem.data_type);
                        //return false;
                        dst_pos_ofs = 0xFFFF;
                    }
                }
                else if (type == 3)
                {
                    dst_normal_ofs = sem.offset;

                    if (sem.data_type == 11)
                    {
                        dst_normal16 = true;
                    }
                    else if (sem.data_type != 3)
                    {

                        //DPRINTF("%s: Unsupported data type(normal) %d.\n", FUNCNAME, sem.data_type);
                        //return false;
                        dst_normal_ofs = 0xFFFF;
                    }
                }
                else if (type == 5)
                {
                    dst_uv_ofs = sem.offset;

                    if (sem.data_type == 10)
                    {
                        dst_uv16 = true;
                    }
                    else if (sem.data_type != 1)
                    {
                        //DPRINTF("%s: Unsupported data type(uv) %d.\n", FUNCNAME, sem.data_type);
                        //return false;
                        dst_uv_ofs = 0xFFFF;
                    }
                }
                else if (type == 6)
                {
                    dst_tangent_ofs = sem.offset;

                    if (sem.data_type == 11)
                    {
                        dst_tangent16 = true;
                    }
                    else if (sem.data_type != 3)
                    {

                        //DPRINTF("%s: Unsupported data type(tangent) %d.\n", FUNCNAME, sem.data_type);
                        //return false;
                        dst_tangent_ofs = 0xFFFF;
                    }
                }
            }

            if (dst_pos_ofs == 0xFFFF || dst_uv_ofs == 0xFFFF || dst_normal_ofs == 0xFFFF || dst_tangent_ofs == 0xFFFF)
                continue;

            for (size_t k = 0; k < vb_dst.GetNumVertex(); k++)
            {
                const uint8_t *dst_vertex_top = vb_dst.vertex.data() + (k * vb_dst.vertex_size);
                const uint8_t *dst_pos_buf = dst_vertex_top + dst_pos_ofs;
                const uint8_t *dst_uv_buf = dst_vertex_top + dst_uv_ofs;
                const uint8_t *dst_normal_buf = dst_vertex_top + dst_normal_ofs;
                const uint8_t *dst_tangent_buf = dst_vertex_top + dst_tangent_ofs;

                Vector3 dst_pos((const float *)dst_pos_buf);
                float this_distance = src_pos.Distance(dst_pos);
                if (this_distance < distance)
                {
                    distance = this_distance;

                    if (dst_uv16)
                    {
                        ret_uv = Vector2((const uint16_t *)dst_uv_buf);
                    }
                    else
                    {
                        ret_uv = Vector2((const float *)dst_uv_buf);
                    }

                    if (dst_normal16)
                    {
                        ret_normal = Vector4((const uint16_t *)dst_normal_buf);
                    }
                    else
                    {
                        ret_normal = Vector4((const float *)dst_normal_buf);
                    }

                    if (dst_tangent16)
                    {
                        ret_tangent = Vector4((const uint16_t *)dst_tangent_buf);
                    }
                    else
                    {
                        ret_tangent = Vector4((const float *)dst_tangent_buf);
                    }
                }
            }
        }

        //DPRINTF("Distance %f\n", distance);

        if (src_uv16)
        {
            ret_uv.Get((uint16_t *)src_uv_buf);
        }
        else
        {
            ret_uv.Get((float *)src_uv_buf);
        }

        if (src_normal16)
        {
            ret_normal.Get((uint16_t *)src_normal_buf);
        }
        else
        {
            ret_normal.Get((float *)src_normal_buf);
        }

        if (src_tangent16)
        {
            ret_tangent.Get((uint16_t *)src_tangent_buf);
        }
        else
        {
            ret_tangent.Get((float *)src_tangent_buf);
        }
    }

    return true;
}

