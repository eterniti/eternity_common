// Nier automata WMB parser
// Low-level parser for the moment, may switch to high level one in the future.

#ifndef WMBFILE_H
#define WMBFILE_H

#include "BaseFile.h"
#include <set>
#include <unordered_set>

#define WMB_SIGNATURE    0x33424D57

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED WMBHeader
{
    uint32_t signature; // 0
    uint32_t version; // 4
    uint32_t unk_08; // Always zero?
    uint32_t flags; // 0xC
    float bounding_box[6]; // 0x10
    uint32_t bones_offset; // 0x28
    uint32_t num_bones; // 0x2C
    uint32_t chunk1_offset; // 0x30
    uint32_t num_chunk1; // 0x34
    uint32_t vertex_group_offset; // 0x38
    uint32_t num_vertex_group; // 0x3C
    uint32_t meshes_offset; // 0x40
    uint32_t num_meshes; // 0x44
    uint32_t mesh_group_infos_offset; // 0x48
    uint32_t num_mesh_group_infos; // 0x4C
    uint32_t chunk2_offset; // 0x50
    uint32_t num_chunk2; // 0x54
    uint32_t bone_map_offset; // 0x58
    uint32_t num_bone_map; // 0x5C
    uint32_t boneset_offset; // 0x60
    uint32_t num_boneset; // 0x64
    uint32_t materials_offset; // 0x68
    uint32_t num_materials; // 0x6C
    uint32_t mesh_groups_offset; // 0x70
    uint32_t num_mesh_groups; // 0x74
    uint32_t chunk3_offset; // 0x78
    uint32_t num_chunk3; // 0x7C
    uint32_t unk_80[4];
};
CHECK_STRUCT_SIZE(WMBHeader, 0x90);

struct PACKED WMBMesh
{
    uint32_t vertex_group_index; // 0
    uint32_t boneset_index;  // 4
    uint32_t vertex_start; // 8
    uint32_t face_start; // 0xC
    uint32_t num_vertex; // 0x10
    uint32_t num_faces; // 0x14
    uint32_t unk_18;
};
CHECK_STRUCT_SIZE(WMBMesh, 0x1C);

struct PACKED WMBGroupedMesh
{
    uint32_t vertex_group_index; // 0
    uint32_t mesh_group_index; // 4
    uint32_t material_index; // 8
    uint32_t unk_0C; // 0xC
    uint32_t mesh_group_info_material_pair; // 0x10
    uint32_t unk_14;
};
CHECK_STRUCT_SIZE(WMBGroupedMesh, 0x18);

struct PACKED WMBMeshGroupInfo
{
    uint32_t name_offset; // 0
    uint32_t unk_04;
    uint32_t mesh_start; // 8
    uint32_t grouped_mesh_start; // 0xC
    uint32_t num_meshes; // 0x10

    void GetMeshGroups(uint8_t *base, uint32_t mesh, std::set<uint32_t> &groups);
};
CHECK_STRUCT_SIZE(WMBMeshGroupInfo, 0x14);

struct PACKED WMBMeshGroup
{
    uint32_t name_offset; // 0
    float bounding_box[6]; // 4
    uint32_t material_index_offset; // 0x1C
    uint32_t num_material_index; // 0x20
    uint32_t bones_index_offset; // 0x24
    uint32_t num_bones_index; // 0x28

    inline std::string GetName(uint8_t *base) { return (char *)(base + name_offset); }
};
CHECK_STRUCT_SIZE(WMBMeshGroup, 0x2C);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

class WmbFile : public BaseFile
{
private:

    uint8_t *base;

public:
    WmbFile();
    virtual ~WmbFile();

    void Parse(uint8_t *buf);
    void Test();
};

#endif // WMBFILE_H
