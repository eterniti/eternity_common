#ifndef G1MFILE_H
#define G1MFILE_H

//#define FBX_SUPPORT
//#define USE_DIRXMATH

#include <unordered_map>
#include <unordered_set>
#include <set>
#include <stdexcept>

#ifdef USE_DIRXMATH
#include <directxmath.h>
using namespace DirectX;

#else

#include <d3dx9.h>

#endif

#ifdef FBX_SUPPORT
#ifdef _MSC_VER
#pragma warning(disable : 4800)
#endif

#include <fbxsdk.h>
#endif

#include "FixedMemoryStream.h"
#include "TransformMatrix.h"

#define G1M_SIGNATURE   0x47314D5F

#define G1MF_SIGNATURE  0x47314D46
#define G1MS_SIGNATURE  0x47314D53
#define G1MM_SIGNATURE  0x47314D4D
#define G1MG_SIGNATURE  0x47314D47
#define COLL_SIGNATURE  0x434F4C4C
#define NUNO_SIGNATURE  0x4E554E4F
#define NUNV_SIGNATURE  0x4E554E56
#define NUNS_SIGNATURE  0x4E554E53
#define EXTR_SIGNATURE  0x45585452
#define HAIR_SIGNATURE  0x48414952
#define SOFT_SIGNATURE  0x534F4654


#undef isnan

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED G1MHeader
{
    uint32_t signature;
    uint32_t version;
    uint32_t file_size;
    uint32_t header_size;
    uint32_t unk_10;
    uint32_t num_chunks;
};
CHECK_STRUCT_SIZE(G1MHeader, 0x18);

struct PACKED G1MChunk
{
    uint32_t signature;
    uint32_t version;
    uint32_t size;
};
CHECK_STRUCT_SIZE(G1MChunk, 0xC);

// G1MF versions:
// Version 21 & 23: found in system folder in DOA6 (they also have "DX9" instead of "DX11" in the G1MG)
// DOA6: mostly 29, but also 31 (Kula hair) and instances of 26/28/30
// OPPW4: mostly 33
struct PACKED G1MFData
{
    uint32_t signature;
    uint32_t version;
    uint32_t size;
    uint32_t unk_0C; // TODO
    uint32_t num_bones; // 0x10
    uint32_t unk_14; // TODO: number of G1MS sections (check)
    uint32_t num_matrix; // 0x18
    uint32_t unk_1C; // TODO: number of G1MG section 1 elements 
    uint32_t num_materials; // 0x20
    uint32_t num_material_attributes; // 0x24
    uint32_t num_attributes; // 0x28
    uint32_t unk_2C; // TODO: the sum of the padded name string length of the shader params G1MG section 
    uint32_t unk_30; // TODO: the the length of the "useful" shader params data. Take the section length, retrieve the header length (12) and the variables that are used to give the count (so resource count * 4) 
    uint32_t num_vb; // 0x34
    uint32_t num_layouts; // 0x38
    uint32_t num_layout_refs; // 0x3C  Note: We used to think this was num_ib, but 0x0140e596.g1m from OPPW4 proved otherwise
    uint32_t num_bone_maps;  // 0x40
    uint32_t num_individual_bone_maps; // 0x44
    uint32_t num_non_shared_vb; // 0x48  TODO:is probably the IBO count 
    uint32_t num_submeshes; // 0x4C
    uint32_t num_submeshes2; // 0x50
    uint32_t unk_54; // TODO: the mesh groups count
    uint32_t num_meshes; // 0x58
    uint32_t num_submeshes_in_meshes; // 0x5C
    uint32_t unk_60; // TODO (coll related)
    uint32_t unk_64; // TODO (coll related)
    uint32_t num_nuno2s; // TODO
    uint32_t unk_6C; // TODO
    uint32_t num_nuno2s_unk11; // 0x70
    uint32_t num_nuno1s; // 0x74
    uint32_t num_nuno1s_unk4; // 0x78
    uint32_t num_nuno1s_control_points; // 0x7C
    uint32_t num_nuno1s_unk1; // 0x80
    uint32_t num_nuno1s_unk2_and_unk3; // 0x84
    uint32_t bones_id_size; // 0x88
    uint32_t unk_8C; // TODO Probably related with the EXTR section:  the number of EXTR sections 
    // If version > 21
    uint32_t num_nunv1s; // 0x90
    uint32_t num_nunv1s_unk4; // 0x94
    uint32_t num_nunv1s_control_points; // 0x98
    uint32_t num_nunv1s_unk1; // 0x9C
    uint32_t unk_A0; // TODO
    uint32_t unk_A4; // TODO
    uint32_t unk_A8; // TODO
    uint32_t unk_AC; // TODO
    uint32_t unk_B0; // TODO
    uint32_t unk_B4; // TODO
    uint32_t unk_B8; // TODO
    uint32_t unk_BC; // TODO
    uint32_t unk_C0; // TODO
    uint32_t unk_C4; // TODO
    uint32_t unk_C8; // TODO
    // If version >= 24
    uint32_t unk_CC; // TODO
    uint32_t unk_D0; // TODO probably related with SOFT
    uint32_t unk_D4; // TODO
    uint32_t unk_D8; // TODO
    uint32_t unk_DC; // TODO
    uint32_t unk_E0; // TODO
    uint32_t unk_E4; // TODO
    uint32_t unk_E8; // TODO probably related with SOFT
    uint32_t unk_EC; // TODO
    uint32_t num_nuno3s; // 0xF0
    uint32_t num_nuno3s_unk4; // 0xF4
    uint32_t num_nuno3s_control_points; // 0xF8
    uint32_t num_nuno3s_unk1; // 0xFC
    uint32_t unk_100; // TODO
    // If version >= 25
    uint32_t unk_104; // TODO
    uint32_t unk_108; // TODO
    // If version >= 26
    uint32_t unk_10C; // TODO
    uint32_t unk_110; // TODO
    // If version >= 27
    uint32_t num_nuno4s; // 0x114
    uint32_t num_nuno4s_unk7; // 0x118 Note: because number of unk7 and unk10 match, we don't know for sure which is which
    uint32_t num_nuno4s_unk8; // 0x11C
    uint32_t num_nuno4s_unk9; // 0x120
    uint32_t num_nuno4s_unk10; // 0x124
    uint32_t unk_128; // TODO
    // If version >= 29
    uint32_t unk_12C;
    // If version >= 30
    uint32_t unk_130[22]; // TODO
};
CHECK_STRUCT_SIZE(G1MFData, 0x188); // Currently updated for version 33 (OPPW4)
CHECK_FIELD_OFFSET(G1MFData, num_bone_maps, 0x40);
CHECK_FIELD_OFFSET(G1MFData, num_individual_bone_maps, 0x44);
CHECK_FIELD_OFFSET(G1MFData, num_bones, 0x10);
CHECK_FIELD_OFFSET(G1MFData, num_matrix, 0x18);

struct PACKED G1MSChunkHeader
{
    uint32_t signature;
    uint32_t version;
    uint32_t chunk_size;
    uint32_t bones_offset; // 0xC
    uint32_t unk_10; // Almost always 1, but found 0 in some file.
    uint16_t num_bones; // 0x14
    uint16_t num_indices; // 0x16
    uint16_t num_parents; // 0x18
    uint16_t unk_1A; // Always 0?
};
CHECK_STRUCT_SIZE(G1MSChunkHeader, 0x1C);

struct PACKED G1MSBone
{
    float scale[3]; // 0
    uint16_t parent; // C
    uint16_t flags; // E
    float rotation[4]; // 0x10
    float position[4]; // 0x20
};
CHECK_STRUCT_SIZE(G1MSBone, 0x30);

struct PACKED G1MMMatrix
{
    float matrix[16];
};
CHECK_STRUCT_SIZE(G1MMMatrix, 0x40);

struct PACKED G1MGChunkHeader
{
    uint32_t signature;
    uint32_t version;
    uint32_t chunk_size;
    uint32_t platform; // 0xC
    uint32_t unk_10; // always 0?
    float min_x, min_y, min_z; // 0x14
    float max_x, max_y, max_z; // 0x20
    uint32_t num_sections; // 0x2C
};
CHECK_STRUCT_SIZE(G1MGChunkHeader, 0x30);

struct PACKED G1MGMaterialProperties
{
    uint32_t index;
    uint32_t num_textures;
    uint32_t unk_08;
    uint32_t unk_0C;
};
CHECK_STRUCT_SIZE(G1MGMaterialProperties, 0x10);

struct PACKED G1MGTexture
{
    uint16_t tex_id;
    uint16_t tex_type;
    uint16_t tex_type2;
    uint16_t unk_06;
    uint16_t unk_08; // Always 4?
    uint16_t unk_0A; //Always 4?
};
CHECK_STRUCT_SIZE(G1MGTexture, 0xC);

struct PACKED G1MGAttributeHeader
{
    uint32_t size;
    uint32_t name_length; // actually length+1, and then aligned to 4
    uint32_t unk_08;
    uint16_t data_type;
    uint16_t count;
};
CHECK_STRUCT_SIZE(G1MGAttributeHeader, 0x10);

struct PACKED G1MGVertexBufHeader
{
    uint32_t unk_00; // always 0?
    uint32_t vertex_size;
    uint32_t num_vertex;
    uint32_t unk_0C; // always 0?
};
CHECK_STRUCT_SIZE(G1MGVertexBufHeader, 0x10);

struct PACKED G1MGIndexBufferHeader
{
    uint32_t num_indices;
    uint32_t type;
    uint32_t unk_08;
};
CHECK_STRUCT_SIZE(G1MGIndexBufferHeader, 0xC);

struct PACKED NUNOControlPoint
{
    float x;
    float y;
    float z;
    float w; // Should be 1.0

    NUNOControlPoint()
    {
        x = y = z = 0.0f;
        w = 1.0f;
    }
};
CHECK_STRUCT_SIZE(NUNOControlPoint, 0x10);

struct PACKED NUNOInfluence
{
    int32_t P1;
    int32_t P2;
    int32_t P3;
    int32_t P4;

    float P5;
    float P6;
};
CHECK_STRUCT_SIZE(NUNOInfluence, 0x18);

struct PACKED NUNOUnk1
{
    float unk_00[8]; // Warning: index 3 only values of 0 found, they could be something other than float
    uint32_t unk_20[4]; // Warning: indexes 2, 3 could be something else, since only value of 0 found
};
CHECK_STRUCT_SIZE(NUNOUnk1, 0x30);

struct PACKED NUNOUnk5
{
    uint32_t unk_00;
    uint32_t unk_04;
};
CHECK_STRUCT_SIZE(NUNOUnk5, 8);

struct PACKED NUNOUnk6
{
    uint32_t unk_00;
    uint32_t unk_04;
    float unk_08;
};
CHECK_STRUCT_SIZE(NUNOUnk6, 0xC);

struct PACKED NUNOUnk7
{
    float unk_00[7]; // Warning: index 6 was found to be 0 in all files
};
CHECK_STRUCT_SIZE(NUNOUnk7, 0x1C);

struct PACKED NUNOUnk8
{
    float unk_00;
    float unk_04;
};
CHECK_STRUCT_SIZE(NUNOUnk8, 8);

struct PACKED NUNOUnk10
{
    uint32_t unk_00;
    uint32_t unk_04;
};
CHECK_STRUCT_SIZE(NUNOUnk10, 8);

struct PACKED NUNOUnk11
{
    uint32_t unk_00[4]; // Warning, in index 1, 2, 3, the only value in all files was 0, so they could be float
    float unk_10[4];
};
CHECK_STRUCT_SIZE(NUNOUnk11, 0x20);

struct PACKED NUNO4UnkV29
{
    float unk_14;
    uint32_t unk_18;
    float unk_1C[5];
    uint32_t unk_30;
    float unk_34[16]; // Warning: Index 2, 3, 4, 5, 6, 7, 8 -> value of 0 in all files, they could be float
};
CHECK_STRUCT_SIZE(NUNO4UnkV29, 0x60);

// Becasue we don't have much samples of NUNO v30, this struct may be incorrect. In concrete, the first two fields could be "number of something", which may screw thing
struct PACKED NUNO4UnkV30
{
    uint32_t unk_14;
    uint32_t unk_18;
    float unk_1C;
    uint32_t unk_20;
    float unk_24[5];
    uint32_t unk_38;
    float unk_3C[14]; // Warning: Index 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 -> value of always 0 in the few files of DOA6 with this struct. They could be something other than float.
    uint32_t unk_74;
    float unk_78[5];  // Warning: Index 1, 2, 3, 4 -> value of always 0 in the few files of DOA6 with this struct. They could be something other than float.
    int32_t unk_8C;
    float unk_90[11]; // Warning: index 3 -> value of always 0 in the few files of DOA6 with this struct. It could be something other than float.
};
CHECK_STRUCT_SIZE(NUNO4UnkV30, 0xA8);

struct PACKED NUNChunkHeader : G1MChunk
{
    uint32_t section_count;
};
CHECK_STRUCT_SIZE(NUNChunkHeader, 0x10);

struct PACKED NUNSectionHeader
{
    uint32_t id;
    uint32_t size;
    uint32_t count;
};
CHECK_STRUCT_SIZE(NUNSectionHeader, 0xC);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

class G1mFile;
struct G1MGChunk;

struct G1MFChunk
{
    uint32_t version;
    G1MFData data;

    bool Read(FixedMemoryStream &in, uint32_t chunk_version, uint32_t chunk_size);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root, bool *g1mf_auto);
};

struct G1MSChunk
{
    uint32_t version;
    uint32_t unk_10;

    std::vector<uint16_t> indices;
    std::vector<uint16_t> final_bones; // ?
    std::vector<G1MSBone> bones;    

    // This is not part of g1m, it is for internal usage by the program (bones names are not in .g1m)
    std::vector<std::string> bone_names;

    int IndexToID(uint16_t idx) const;
    uint16_t NameToIndex(const std::string &name) const;

    bool Read(FixedMemoryStream &in, uint32_t chunk_version, uint32_t chunk_size);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct G1MMChunk
{
    uint32_t version;
    std::vector<G1MMMatrix> matrices;

    bool Read(FixedMemoryStream &in, uint32_t chunk_version, uint32_t chunk_size);
    bool Write(MemoryStream &out) const;

    static void DecompileMatrix(TiXmlElement *root, const float *matrix);
    static bool CompileMatrix(const TiXmlElement *root, float *matrix);

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

// TODO:
struct G1MGUnkSection1
{
    bool valid;
    std::vector<uint8_t> unk;

    G1MGUnkSection1()
    {
        valid = false;
    }

    bool Read(FixedMemoryStream &in, uint32_t section_size);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct G1MGMaterial
{
    uint32_t index;
    uint32_t unk_08;
    uint32_t unk_0C;

    std::vector<G1MGTexture> textures;

    bool Read(FixedMemoryStream &in);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root, uint32_t idx) const;
    bool Compile(const TiXmlElement *root);
};

struct G1MGMaterialSection
{
    bool valid;
    std::vector<G1MGMaterial> materials;

    G1MGMaterialSection()
    {
        valid = false;
    }

    bool Read(FixedMemoryStream &in, uint32_t section_size);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct G1MGAttribute
{
    uint32_t unk_08;
    uint16_t data_type;
    uint16_t count;

    std::string name;
    std::vector<uint8_t> data;

    bool Read(FixedMemoryStream &in);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct G1MGMaterialAttributes
{
    std::vector<G1MGAttribute> attributes;

    bool Read(FixedMemoryStream &in);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root, uint32_t idx) const;
    bool Compile(const TiXmlElement *root);
};

struct G1MGAttributesSection
{
    bool valid;
    std::vector<G1MGMaterialAttributes> mat_attributes;

    G1MGAttributesSection()
    {
        valid = false;
    }

    bool Read(FixedMemoryStream &in, uint32_t section_size);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct G1MGVertexBuffer
{
    uint32_t unk_00;
    uint32_t vertex_size;
    uint32_t unk_0C;

    std::vector<uint8_t> vertex;

    bool Read(FixedMemoryStream &in);
    bool Write(MemoryStream &out) const;

    inline uint32_t GetNumVertex() const
    {
        return (uint32_t)(vertex.size() / vertex_size);
    }

    TiXmlElement *Decompile(TiXmlNode *root, const std::string &att_dir, uint32_t idx) const;
    bool Compile(const TiXmlElement *root, const std::string &att_dir);
};

struct G1MGVertexSection
{
    bool valid;
    std::vector<G1MGVertexBuffer> vertex_bufs;

    G1MGVertexSection()
    {
        valid = false;
    }

    bool Read(FixedMemoryStream &in, uint32_t section_size);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root, const std::string &att_dir) const;
    bool Compile(const TiXmlElement *root, const std::string &att_dir);
};

struct G1MGSemantic
{
    uint16_t buffer_index;
    uint16_t offset;
    uint16_t data_type;
    uint16_t semantic;

    bool Read(FixedMemoryStream &in);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct G1MGLayout
{
    std::vector<uint32_t> refs;
    std::vector<G1MGSemantic> semantics;

    bool Read(FixedMemoryStream &in);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root, uint32_t idx) const;
    bool Compile(const TiXmlElement *root);
};

struct G1MGLayoutSection
{
    bool valid;
    std::vector<G1MGLayout> entries;

    G1MGLayoutSection()
    {
        valid = false;
    }

    bool Read(FixedMemoryStream &in, uint32_t section_size);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct G1MGBoneMapEntry
{
    uint32_t matrix;
    uint32_t cloth;
    uint16_t mapped;
    uint16_t flags;

    G1MGBoneMapEntry()
    {
        matrix = 0;
        cloth = 0;
        mapped = 0;
        flags = 0;
    }

    inline bool operator==(const G1MGBoneMapEntry &rhs) const
    {
        if (matrix != rhs.matrix)
            return false;

        if (cloth != rhs.cloth)
            return false;

        if (mapped != rhs.mapped)
            return false;

        if (flags != rhs.flags)
            return false;

        return true;
    }

    inline bool operator!=(const G1MGBoneMapEntry &rhs) const
    {
        return !(*this == rhs);
    }

    bool Read(FixedMemoryStream &in);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root, const std::vector<std::string> &bone_names) const;
    bool Compile(const TiXmlElement *root, const std::vector<std::string> &bone_names);
};

struct G1MGBonesMap
{
    std::vector<G1MGBoneMapEntry> map;

    inline bool operator==(const G1MGBonesMap &rhs) const
    {
        return (map == rhs.map);
    }

    inline bool operator!=(const G1MGBonesMap &rhs) const
    {
        return !(*this == rhs);
    }

    bool Read(FixedMemoryStream &in);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root, const std::vector<std::string> &bone_names, uint32_t idx) const;
    bool Compile(const TiXmlElement *root, const std::vector<std::string> &bone_names);
};

struct G1MGBonesMapSection
{
    bool valid;
    std::vector<G1MGBonesMap> bones_maps;

    G1MGBonesMapSection()
    {
        valid = false;
    }

    bool Read(FixedMemoryStream &in, uint32_t section_size);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root, const std::vector<std::string> &bone_names) const;
    bool Compile(const TiXmlElement *root, const std::vector<std::string> &bone_names);
};

struct G1MGIndexBuffer
{
    uint32_t type;
    uint32_t unk_08;

    std::vector<uint8_t> indices;

    inline size_t GetNumIndices() const
    {
        switch (type)
        {
            case 8:
                return indices.size();
            case 16:
                return (indices.size() / 2);

            case 32:
                return (indices.size() / 4);
        }

        return 0;
    }

    inline uint32_t GetIndiceSize() const
    {
        switch (type)
        {
            case 8:
                return 1;
            case 16:
                return 2;

            case 32:
                return 4;
        }

        return 0;
    }

    bool Read(FixedMemoryStream &in);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root, const std::string &att_dir, uint32_t idx) const;
    bool Compile(const TiXmlElement *root, const std::string &att_dir);
};

struct G1MGIndexBufferSection
{
    bool valid;
    std::vector<G1MGIndexBuffer> buffers;

    G1MGIndexBufferSection()
    {
        valid = false;
    }

    bool Read(FixedMemoryStream &in, uint32_t section_size);    
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root, const std::string &att_dir) const;
    bool Compile(const TiXmlElement *root, const std::string &att_dir);
};

struct G1MGSubmesh
{
    uint32_t flags;
    int32_t vertex_buf_ref;

    int32_t bones_map_index;
    uint32_t matpalid;
    uint32_t unk_10;
    int32_t attribute;
    int32_t material;
    int32_t index_buf_ref;
    uint32_t unk_20;
    uint32_t index_buf_fmt;
    uint32_t vertex_buf_start;
    uint32_t num_vertices;
    uint32_t index_buf_start;
    uint32_t num_indices;

    std::vector<uint8_t> temp_vb, temp_ib;

    bool Read(FixedMemoryStream &in);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root, uint32_t idx) const;
    bool Compile(const TiXmlElement *root);
};

struct G1MGSubmeshesSection
{
    bool valid;
    std::vector<G1MGSubmesh> submeshes;

    G1MGSubmeshesSection()
    {
        valid = false;
    }

    bool Read(FixedMemoryStream &in, uint32_t section_size);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct G1MGMesh
{
    std::string shader;
    uint16_t type; // 0x10
    uint16_t unk_12; // 0x12
    int32_t external_section_id; // 14

    std::vector<uint32_t> submeshes;

    bool Read(FixedMemoryStream &in);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root, uint32_t idx) const;
    bool Compile(const TiXmlElement *root);
};

struct G1MGLodGroup
{
    uint32_t unk_00;
    uint32_t unk_04;
    uint32_t unk_08;
    uint32_t count1;
    uint32_t count2;
    int32_t unk_14;
    uint32_t unk_18;
    int32_t unk_1C;
    uint32_t unk_20;

    std::vector<G1MGMesh> meshes;

    bool CanAutoCalcMeshCounts(const G1MGChunk &g1mg) const;

    bool Read(FixedMemoryStream &in);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root, const G1MGChunk &g1mg, size_t idx) const;
    bool Compile(const TiXmlElement *root, bool *auto_meshes);
};

struct G1MGMeshesSection
{
    bool valid;   

    std::vector<G1MGLodGroup> groups;

    G1MGMeshesSection()
    {
        valid = false;
    }

    bool Read(FixedMemoryStream &in, uint32_t section_size);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root, const G1MGChunk &g1mg) const;
    bool Compile(const TiXmlElement *root, std::vector<bool> &auto_meshes);
};

struct G1MGChunk
{
    float min_x, min_y, min_z;
    float max_x, max_y, max_z;
    uint32_t version;
    uint32_t platform;

    G1MGUnkSection1 unk_section1;
    G1MGMaterialSection mat_section;
    G1MGAttributesSection att_section;
    G1MGVertexSection vert_section;
    G1MGLayoutSection layout_section;
    G1MGBonesMapSection bones_map_section;
    G1MGIndexBufferSection index_buf_section;
    G1MGSubmeshesSection submeshes_section;
    G1MGMeshesSection meshes_section;

    bool Read(FixedMemoryStream &in, uint32_t chunk_version, uint32_t chunk_size);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root, const std::string &att_dir, const std::vector<std::string> &bone_names) const;
    bool Compile(const TiXmlElement *root, const std::string &att_dir, const std::vector<std::string> &bone_names,  std::vector<bool> &auto_meshes);
};

struct NUNO1
{
    uint16_t parent_bone;

    // The unknown 0x5C bytes starting at 0x18
    struct PACKED Unk
    {
        float unk_18[2];
        uint32_t unk_20; // Possibly bonemap index
        float unk_24[9];
        uint32_t unk_48; // write as hex in decompilation
        float unk_4C[6]; // Warning: index 5 -> only seen 0 in all files, it could be a integer.
        uint32_t unk_64[4]; // Warning: index 1, 2, 3 -> only 0 seen in all files, they could be float.
    } unk;
    CHECK_STRUCT_SIZE(Unk, 0x5C);
    //

    std::vector<NUNOControlPoint> control_points;
    std::vector<NUNOInfluence> influences;
    std::vector<NUNOUnk1> unk1s;
    std::vector<uint32_t> unk2s;
    std::vector<uint32_t> unk3s;
    std::vector<uint32_t> unk4s;

    bool Read(FixedMemoryStream &in);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root, const G1mFile &g1m, size_t idx) const;
    bool Compile(const TiXmlElement *root, const G1mFile &g1m);
};

struct NUNO2
{
    uint16_t parent_bone;
    uint16_t dummy;
    int32_t unk_04;

    // Unknown 0x4C bytes
    struct PACKED Unk
    {       
        float unk_10[9]; // Warning: index 2 -> only 0 seen in all files, could be a integer
        uint32_t unk_34; // write as hex in decompilation
        float unk_38[9]; // Warning: index 0, 2, 4, 5 are 0 in all files, could be an integer.
    } unk;
    CHECK_STRUCT_SIZE(Unk, 0x4C);
    //

    std::vector<NUNOUnk11> unk11s;
    std::vector<uint32_t> unk2s;

    bool Read(FixedMemoryStream &in);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root, const G1mFile &g1m, size_t idx) const;
    bool Compile(const TiXmlElement *root, const G1mFile &g1m);
};

struct NUNO3
{
    uint32_t version;
    uint16_t parent_bone;
    uint16_t dummy;
    uint32_t unk_10; // Possibly bonemap index

    uint32_t unk_1C[4]; // write as hex in decompilation
    uint32_t nun_opcode; // 0x2C

    // If version >= 30
    uint32_t unk_30;
    uint32_t unk_34;
    //

    // Unknown 0xA8 bytes
    // Notice: for version >= 30, the offsets here are actually 8 more
    struct PACKED Unk
    {
        float unk_30[9];
        uint32_t unk_54; // write as hex in decompilation
        uint32_t unk_58; // write as hex in decompilation
        float unk_5C[11];
        uint32_t unk_88[4];  // write as hex in decompilation. Warning: Index 3 0 in all files, it could be float
        float unk_98[7];
        uint32_t unk_B4; // write as hex in decompilation
        float unk_B8;
        uint32_t unk_BC[7]; // Write as hex in decompilation. Warning: Index 1-6 are 0 in all files, they could be float.
    } unk;
    CHECK_STRUCT_SIZE(Unk, 0xA8);
    //

    // if version >= 30
    struct PACKED Unk2
    {
        uint32_t unk_E0[4]; // Warning: all of them 0 in the few files of DOA6 that have this, they could be float
        int unk_F0;
    } unk2;
    CHECK_STRUCT_SIZE(Unk2, 0x14);
    // endif

    // if version >= 32 && nun_opcode == 3
    // Note: we don't currently know if this would apply to version 30 as well
    struct PACKED Unk3
    {
        float unk_F4[5];
    } unk3;
    CHECK_STRUCT_SIZE(Unk3, 0x14);
    //endif

    std::vector<NUNOControlPoint> control_points;
    std::vector<NUNOInfluence> influences;
    std::vector<NUNOUnk1> unk1s;
    std::vector<uint32_t> unk4s;
    std::vector<NUNOUnk5> unk5s;
    std::vector<NUNOUnk6> unk6s;

    bool Read(FixedMemoryStream &in, uint32_t version);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root, const G1mFile &g1m, size_t idx) const;
    bool Compile(const TiXmlElement *root, const G1mFile &g1m, uint32_t version);
};

struct NUNO4
{
    uint32_t version;
    uint32_t parent_bone;

    // if version 29
    NUNO4UnkV29 u29;
    //

    // if version >= 30
    NUNO4UnkV30 u30;
    //

    std::vector<NUNOUnk7> unk7s;
    std::vector<NUNOUnk8> unk8s;
    std::vector<uint32_t> unk9s;
    std::vector<NUNOUnk10> unk10s;

    bool Read(FixedMemoryStream &in, uint32_t version);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root, const G1mFile &g1m, size_t idx) const;
    bool Compile(const TiXmlElement *root, const G1mFile &g1m, uint32_t version);
};

struct NUNOChunk
{
    uint32_t version;

    std::vector<NUNO1> nuno1s;
    std::vector<NUNO2> nuno2s;
    std::vector<NUNO3> nuno3s;
    std::vector<NUNO4> nuno4s;

    bool Read(FixedMemoryStream &in, uint32_t chunk_version, uint32_t chunk_size);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root, const G1mFile &g1m) const;
    bool Compile(const TiXmlElement *root, const G1mFile &g1m);
};

struct NUNV1
{
    uint16_t parent_bone;

    // The unknown 0x64 bytes starting at 0x10
    struct Unk
    {
        uint32_t unk_10; // Possibly bonemap index
        float unk_14[19]; // Warning: index 17 is 0 in all files. Could be an integer.
        uint32_t unk_60[5]; // Write as hex in decompilation. Warning: index 1, 3 & 4 are 0 in all files, they could be float
    } unk;
    CHECK_STRUCT_SIZE(Unk, 0x64);
    //

    std::vector<NUNOControlPoint> control_points;
    std::vector<NUNOInfluence> influences;
    std::vector<NUNOUnk1> unk1s;
    std::vector<uint32_t> unk4s;

    bool Read(FixedMemoryStream &in);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root, const G1mFile &g1m, size_t idx) const;
    bool Compile(const TiXmlElement *root, const G1mFile &g1m);
};

struct NUNVChunk
{
    uint32_t version;
    std::vector<NUNV1> nunv1s;

    bool Read(FixedMemoryStream &in, uint32_t chunk_version, uint32_t chunk_size);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root, const G1mFile &g1m) const;
    bool Compile(const TiXmlElement *root, const G1mFile &g1m);
};

// TODO: NUNS, COLL, EXTR, HAIR, SOFT
struct UnkChunk
{
    uint32_t type;
    uint32_t version;
    std::vector<uint8_t> unk;

    bool Read(FixedMemoryStream &in, uint32_t chunk_type, uint32_t chunk_version, uint32_t chunk_size);
    bool Write(MemoryStream &out) const;

    TiXmlElement *Decompile(TiXmlNode *root, const std::string &att_dir, uint32_t idx) const;
    bool Compile(const TiXmlElement *root, const std::string &att_dir, uint32_t type);
};

class G1mFile : public BaseFile
{
private:

    uint32_t version;
    bool update_g1mf;
    bool parse_nun;

    std::vector<G1MFChunk> g1mfs;
    std::vector<G1MSChunk> g1mss;
    std::vector<G1MMChunk> g1mms;
    std::vector<G1MGChunk> g1mgs;
    std::vector<UnkChunk> colls;
    std::vector<NUNOChunk> nunos;
    std::vector<NUNVChunk> nunvs;
    std::vector<UnkChunk> nunss;
    std::vector<UnkChunk> extrs;
    std::vector<UnkChunk> hairs;
    std::vector<UnkChunk> softs;

    std::vector<UnkChunk> unparsed_nunos;
    std::vector<UnkChunk> unparsed_nunvs;

    std::unordered_map<size_t, size_t> upgraded_ibs;
    std::string att_dir; // For xml decompilation/compilation only

    void UpdateG1MF();

    void BreakVB(size_t vidx);
    void BreakIB(size_t iidx);

    void MergeVB(size_t vidx);
    void MergeIB(size_t iidx);

    void UpgradeIB(G1MGIndexBuffer &ib, size_t num_vertex);
    uint8_t *UpgradeBuffer(uint8_t *old, size_t *abs_size, uint32_t old_format, uint32_t new_format);

    uint16_t FindBoneIndexByName(const std::string &name) const;
    G1MGBoneMapEntry *FindBoneMapEntry(G1MGBonesMap *map, uint16_t bone_idx, uint16_t flags, bool ignore_different_cloth=false, uint32_t ref_cloth=0);
    bool CreateBonesMapFromVgmapFile(const std::string &file, G1MGBonesMap &ret, G1MGBonesMap *ref_map);
    size_t CountBonesMapUsage(size_t bm_idx) const;

#ifdef USE_DIRXMATH
    XMMATRIX GetGlobalTransformDX(uint16_t bone_idx) const;
#else
    D3DXMATRIX GetGlobalTransformDX(uint16_t bone_idx) const;
#endif

    G1MMMatrix GetGlobalTransform(uint16_t bone_idx) const;


#ifdef FBX_SUPPORT

    bool GetPosition(const uint8_t *vbuf, FbxVector4 &pos, uint16_t format) const;
    bool GetNormal(const uint8_t *vbuf, FbxVector4 &normal, uint16_t format) const;
    bool GetUV(const uint8_t *vbuf, FbxVector2 &uv, uint16_t format) const;
    bool GetWeights(const uint8_t *vbuf, float *vec, uint16_t format) const;
    bool GetIndices(const uint8_t *vbuf, uint32_t *vec, uint16_t format) const;

    bool ExportBoneToFbx(uint16_t parent, FbxNode *root_node, FbxScene *scene, std::vector<FbxNode *> &fbx_bones) const;
    bool ExportSkinToFbx(const G1MGBonesMap &bmap, const std::vector<float> &weights, const std::vector<uint32_t> &indices, const std::vector<bool> &referenced, const std::vector<FbxNode *> &fbx_bones, FbxScene *scene, FbxMesh *fbx_mesh, FbxAMatrix skin_matrix, const G1mFile &skl_g1m) const;
    bool ExportMeshToFbx(size_t idx, FbxScene *scene, std::vector<FbxNode *> *pfbx_bones, const G1mFile *external_skl) const;

#endif

protected:

    void Reset();

public:
    G1mFile();
    virtual ~G1mFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    virtual bool DecompileToFile(const std::string &path, bool show_error=true, bool build_path=false) override;
    virtual bool CompileFromFile(const std::string &path, bool show_error=true, bool big_endian=false) override;

    inline void SetParseNun(bool enabled) { parse_nun = enabled; }

    inline size_t GetNumSubmeshes() const
    {
        if (g1mgs.size() == 0)
            return 0;

        return g1mgs[0].submeshes_section.submeshes.size();
    }

    bool ExportTo3DM(const std::string &out_path, bool vgmaps);
    bool ImportSubmeshFrom3DM(size_t idx, const std::string &vb_file, const std::string &ib_file, const std::string &vgmap);

    bool HideSubmesh(size_t idx);

    uint16_t GetNumBones() const;
    uint16_t GetNumBonesID() const;

    bool LoadBoneNames(const std::string &oid_file);
    void SetDefaultBoneNames();

    int BoneIndexToID(uint16_t idx) const;
    uint16_t BoneIDToIndex(int id) const;

    inline uint16_t BoneNameToIndex(const std::string &name) const { return FindBoneIndexByName(name); }
    bool BoneIndexToName(uint16_t idx, std::string &name) const;

    int BoneNameToID(const std::string &name) const;
    bool BoneIDToName(int id, std::string &name) const;

    inline bool BoneExists(int id) const
    {
        return (BoneIDToIndex(id) != 0xFFFF);
    }

    bool CopyBone(const G1mFile &other, int id, bool resolve_ancestors, bool debug_print); // Warning, on error, the G1mFile object will be dirty (no save should be done on this case)

    bool RecalcLodGroup(size_t group);

    bool HasExternalSkeleton() const;

#ifdef FBX_SUPPORT
    bool ExportFbxSkeleton(FbxScene *scene, std::vector<FbxNode *> &fbx_bones);
    bool ExportFbx(FbxScene *scene, std::vector<FbxNode *> *pfbx_bones, G1mFile *external_skl=nullptr) const;
#endif

    // Debug section. These are just test things
    void DebugMatrixTest() const;

#ifndef USE_DIRXMATH
    void DebugBonesTest();
#endif

    bool DebugTest(const G1mFile &skl);

    bool DebugUpgradeUV(size_t sm_idx);
    bool DebugWeight3To4(size_t *pcount);
    bool DebugUVNormalTangentTransfer(size_t idx, const G1mFile &other, uint32_t matpalid_filter=0xFFFFFFFF);
    void DebugCopySoft(const G1mFile &g1m_src) { this->softs = g1m_src.softs; }
};

#endif // G1MFILE_H
