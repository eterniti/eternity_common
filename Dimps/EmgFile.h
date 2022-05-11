#ifndef __EMGFILE_H__
#define __EMGFILE_H__

#include <utility>

#include "SkeletonFile.h"

#ifdef FBX_SUPPORT
#include <fbxsdk.h>
#endif


#define EMG_SIGNATURE	0x474D4523

#define EMG_VTX_FLAG_POS            1
#define EMG_VTX_FLAG_NORM           2
#define EMG_VTX_FLAG_TEX            4
#define EMG_VTX_FLAG_TEX2           8
#define EMG_VTX_FLAG_COLOR          0x40
#define EMG_VTX_FLAG_TANGENT        0x80
#define EMG_VTX_FLAG_BLEND_WEIGHT   0x200

enum EmgVertexFormat
{
    EMG_VTX_FORMAT_64 = EMG_VTX_FLAG_POS | EMG_VTX_FLAG_NORM | EMG_VTX_FLAG_TEX | EMG_VTX_FLAG_COLOR | EMG_VTX_FLAG_TANGENT | EMG_VTX_FLAG_BLEND_WEIGHT,
    EMG_VTX_FORMAT_52 = EMG_VTX_FLAG_POS | EMG_VTX_FLAG_NORM | EMG_VTX_FLAG_TEX | EMG_VTX_FLAG_COLOR | EMG_VTX_FLAG_BLEND_WEIGHT,
};

static_assert(EMG_VTX_FORMAT_52 == 0x247, "Buu");

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    uint32_t signature; // 0    #EMG
    uint16_t unk_04;	// 4
    uint16_t subparts_count; // 6   //
    uint32_t offsets[1]; // 8
    // remaining offsets
} PACKED EMGChunkHeader;

static_assert(sizeof(EMGChunkHeader) == 0xC, "Incorrect structure size.");

typedef struct
{
    uint16_t flags; // 0
    uint16_t unk_02; // 2
    uint16_t textures_lists_count; // 4
    uint16_t unk_06; // 6
    uint32_t unk_08; // 8
    uint32_t textures_lists_offset; // 0x0C

    uint16_t vertex_count; // 0x10
    uint16_t vertex_size;  // 0x12
    uint32_t vertex_offset; // 0x14
    uint16_t strips; // 0x18
    uint16_t submesh_count; // 0x1A
    uint32_t submesh_list_offset; // 0x1C
    float vectors[12]; // 0x20
    // size 0x50
} PACKED EMGHeader;

static_assert(sizeof(EMGHeader) == 0x50, "Incorrect structure size.");

typedef struct
{
    uint8_t unk_00; // 0
    uint8_t emb_index; // 1
    uint8_t unk_02[2]; // 2
    float f1; // 4
    float f2; // 8
    // size 0xC
} PACKED EMGTextureHeader;

static_assert(sizeof(EMGTextureHeader) == 0xC, "Incorrect structure size.");

typedef struct
{
    float vector[4]; // 0
    uint16_t tl_index; // 0x10
    uint16_t face_count; // 0x12
    uint16_t linked_bones_count; // 0x14 bone mapping
    char emm_material[0x20]; // 0x16
    // size 0x36
} PACKED EMGSubMeshHeader;

static_assert(sizeof(EMGSubMeshHeader) == 0x36, "Incorrect structure size.");

typedef struct
{
    struct
    {
        float x; // 0
        float y; // 4
        float z; // 8
    } pos;

    struct
    {
        float x; // 0xC
        float y; // 0x10
        float z; // 0x14
    } norm;

    struct
    {
        float u; // 0x18
        float v; // 0x1C
    } tex;

    // size 0x20
} PACKED VertexCommon;

typedef struct
{
    VertexCommon common; // 0
    float tex2[3]; // 0x20  vertex tangent
    uint32_t color; // 0x2C    aarrggbb
    uint8_t blend[4]; // 0x30
    float blend_weight[3]; // 0x34
    // size 0x40
} PACKED Vertex64;

static_assert(sizeof(Vertex64) == 0x40, "Incorrect structure size.");

typedef struct
{
    VertexCommon common; // 0
    uint32_t color; // 0x20 aarrggbb
    uint8_t blend[4]; // 0x24
    float blend_weight[3]; // 0x28
    // size 0x34
} PACKED Vertex52;

static_assert(sizeof(Vertex52) == 0x34, "Incorrect structure size.");

#ifdef _MSC_VER
#pragma pack(pop)
#endif

namespace Utils
{
    bool GetNextObjLine(const std::string &content, size_t *pos, std::string &line);
    bool ReadObj(const std::string &obj, std::vector<VertexCommon> &vertex, bool do_uv, bool do_normal, size_t v_start_idx, size_t vt_start_idx, size_t vn_start_idx, size_t *vtx_count, size_t *uv_count, size_t *n_count, bool show_error=true);

    void ColorToFloat(uint32_t color, float *r, float *g, float *b, float *a);
    uint32_t DoubleToColor(double r, double g, double b, double a);
}

struct VertexData
{
    union
    {
        Vertex64 vertex64;
        Vertex52 vertex52;
    } VertexUnion;

    unsigned int size;
	
    void Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root, unsigned int vertex_size);

    inline bool operator==(const VertexData &rhs) const
    {
        if (this->size != rhs.size)
            return false;

        assert(size != sizeof(Vertex52) || size != sizeof(Vertex64));

        if (size == sizeof(Vertex52))
        {
           	return (memcmp(&this->VertexUnion.vertex52, &rhs.VertexUnion.vertex52, sizeof(Vertex52)) == 0);
        }		

        return (memcmp(&this->VertexUnion.vertex64, &rhs.VertexUnion.vertex64, sizeof(Vertex64)) == 0);
    }

    inline bool operator!=(const VertexData &rhs) const
    {
        return !(*this == rhs);
    }
};

class EmoFile;
class PartsGroup;
class EmgFile;

class SubPart;
class TexturesList;
class SubMesh;
class Texture;

class SubPart
{
private:

    std::vector<TexturesList> textures_lists;
    std::vector<VertexData> vertex;
    std::vector<SubMesh> submeshes;

    uint16_t strips;
    float vectors[12];

    uint16_t flags;

    // from EMGHeader
    uint16_t unk_02;
    uint16_t unk_06;
    uint32_t unk_08;

    // METADATA - Don't use in comparison!
    std::string meta_name;

    bool InjectVertex(const std::vector<VertexCommon> &new_vertex, bool do_pos, bool do_uvmap, bool do_normal);

    void GetSubMeshVertexAndTriangles(uint16_t submesh_idx, std::vector<VertexData> &sm_vertex, std::vector<uint16_t> &sm_triangles) const;
    size_t GetTrianglesInternal(size_t submesh_idx, std::vector<uint16_t> *tri_faces) const;

#ifdef FBX_SUPPORT
    void ExportSubMeshFbxSkin(const SkeletonFile &skl, const SubMesh &submesh, const std::vector<VertexData> &sm_vertex, const std::vector<FbxNode *> &fbx_bones, FbxScene *scene, FbxMesh *fbx_mesh, FbxAMatrix skin_matrix) const;

    static bool LoadFbxBlendWeights(FbxMesh* fbx_mesh, std::vector<std::vector<std::pair<double, FbxNode *>>>& weights);
    static bool LoadFbxUV(FbxMesh *fbx_mesh, int poly_index, int pos_in_poly, int control_point_index, VertexData *v);
    static bool LoadFbxNormal(FbxNode *fbx_node, int vertex_index, int control_point_index, VertexData *v);
    static bool LoadFbxVertexColor(FbxMesh *fbx_mesh, int vertex_index, int control_point_index, uint32_t *color);
    static bool LoadFbxTangent(FbxMesh *fbx_mesh, int vertex_index, int control_point_index, VertexData *v);
    static bool LoadFbxBinormal(FbxMesh *fbx_mesh, int vertex_index, int control_point_index, VertexData *v);
    static bool LoadFbxBlendData(SkeletonFile &skl, const std::vector<Bone *> &linked_bones, const std::vector<std::pair<double, FbxNode *>> &vertex_weights, uint8_t *blend, float *blend_weight);

    bool InjectSubMeshFbx(SkeletonFile &skl, SubMesh &submesh, FbxNode *fbx_node, std::vector<VertexData> &sm_vertex, int vertex_size, size_t v_start, bool use_fbx_tangent);
    bool InjectFbx(SkeletonFile &skl, const std::vector<FbxNode *> fbx_nodes, bool use_fbx_tangent);
#endif

    friend class EmgFile;
    friend class PartsGroup;
    friend class EmoFile;

public:   

    inline const std::string &GetMetaName() const { return meta_name; }

    inline uint16_t GetNumSubMeshes() const { return submeshes.size(); }

    inline unsigned int GetVertexSize() const
    {
        return vertex[0].size;
    }

    inline const std::vector<VertexData> &GetVertex() const
    {
        return vertex;
    }

    inline void SetVertex(const std::vector<VertexData> &vertex)
    {
        this->vertex = vertex;

        if (flags == 0x247)
        {
            if (vertex[0].size == sizeof(Vertex64))
            {
                flags = 0x2C7;
            }
            else if (vertex[0].size == sizeof(Vertex52))
            {
                // Nothing
            }
            else
            {
                assert(0);
            }

        }
        else if (flags == 0x2C7)
        {
            if (vertex[0].size == sizeof(Vertex52))
            {
                flags = 0x247;
            }
            else if (vertex[0].size == sizeof(Vertex64))
            {
                // Nothing
            }
            else
            {
                assert(0);
            }
        }
        else
        {
            assert(0);
        }
    }


    std::vector<uint16_t> GetTriangles(size_t submesh_idx) const;
    size_t GetNumberOfPolygons(size_t submesh_idx) const;

    inline bool GetStrips() const { return (strips == 0) ? false : true; }
    inline void SetStrips(bool strips) { this->strips = strips; }

    size_t GetLinkedBones(std::vector<Bone*> &list, bool clear_vector, bool unique=true, const SkeletonFile *sorter=nullptr) const;
    size_t GetEmmMaterials(std::vector<std::string> &list, bool clear_vector, bool unique=true) const;
    size_t GetEmbIndexes(std::vector<uint8_t> &list, bool clear_vector, bool unique=true, bool sort=false) const;

    size_t ReplaceEmmMaterial(const std::string &old_mat, const std::string &new_mat);
    size_t ReplaceEmbIndex(uint8_t old_index, uint8_t new_index);

    bool IsEdge() const;

    inline bool RemoveSubMesh(size_t submesh_idx)
    {
        if (submesh_idx >= submeshes.size())
            return false;

        submeshes.erase(submeshes.begin() + submesh_idx);
        return true;
    }

    size_t ExportObj(std::string *vertex_out, std::string *uvmap_out, std::string *normal_out, std::string *topology_out, size_t v_start_idx=0, bool write_group=true) const;
    bool InjectObj(const std::string &obj, bool do_uv, bool do_normal, int v_start_idx=0, int vt_start_idx=0, int vn_start_idx=0, bool show_error=true);

    void Decompile(TiXmlNode *root, uint16_t id) const;
    bool Compile(const TiXmlElement *root, SkeletonFile *skl);

#ifdef FBX_SUPPORT
    bool ExportSubMeshFbx(const SkeletonFile &skl, uint16_t submesh_idx, const std::vector<FbxNode *> &fbx_bones, FbxScene *scene) const;
    bool ExportFbx(const SkeletonFile &skl, const std::vector<FbxNode *> &fbx_bones, FbxScene *scene) const;

    bool InjectFbx(SkeletonFile &skl, FbxScene *scene, bool use_fbx_tangent);
#endif

    bool operator==(const SubPart &rhs) const;

    inline bool operator!=(const SubPart &rhs) const
    {
        return !(*this == rhs);
    }

    inline SubMesh &operator[](size_t n) { return submeshes[n]; }
    inline const SubMesh &operator[](size_t n) const { return submeshes[n]; }

    inline std::vector<SubMesh>::iterator begin() { return submeshes.begin(); }
    inline std::vector<SubMesh>::iterator end() { return submeshes.end(); }

    inline std::vector<SubMesh>::const_iterator begin() const { return submeshes.begin(); }
    inline std::vector<SubMesh>::const_iterator end() const { return submeshes.end(); }
};

class TexturesList
{
private:

    std::vector<Texture> textures;

    friend class SubPart;
    friend class EmgFile;
    friend class PartsGroup;
    friend class EmoFile;

public:

    size_t GetEmbIndexes(std::vector<uint8_t> &list, bool clear_vector, bool unique=true, bool sort=false) const;
    size_t ReplaceEmbIndex(uint8_t old_index, uint8_t new_index);

    void Decompile(TiXmlNode *root, uint16_t id) const;
    bool Compile(const TiXmlElement *root);

    inline bool operator==(const TexturesList &rhs) const
    {
       	return (this->textures == rhs.textures);
    }

    inline bool operator!=(const TexturesList &rhs) const
    {
        return !(*this == rhs);
    }

    inline Texture &operator[](size_t n) { return textures[n]; }
    inline const Texture &operator[](size_t n) const { return textures[n]; }

    inline std::vector<Texture>::iterator begin() { return textures.begin(); }
    inline std::vector<Texture>::iterator end() { return textures.end(); }

    inline std::vector<Texture>::const_iterator begin() const { return textures.begin(); }
    inline std::vector<Texture>::const_iterator end() const { return textures.end(); }
};

class SubMesh
{
private:

    std::string emm_material;
    uint16_t tl_index;
    std::vector<uint16_t> faces;
    std::vector<Bone *> linked_bones;
    float vector[4];

    // METADATA - DON'T USE IN COMPARISON
    std::string meta_name;

    friend class SubPart;
    friend class EmgFile;
    friend class PartsGroup;
    friend class EmoFile;

public:

    static const size_t MAX_EMM_MAT_LENGTH = 31;

    inline const std::string &GetMetaName() const { return meta_name; }

    inline const std::vector<uint16_t> GetFaces() const { return faces; }
    inline void SetFaces(const std::vector<uint16_t> &faces) { this->faces = faces; }

    size_t GetLinkedBones(std::vector<Bone*> &list, bool clear_vector, bool unique=true, const SkeletonFile *sorter=nullptr) const;

    inline bool SetEmmMaterial(const std::string &emm_material)
    {
        if (emm_material.length() > MAX_EMM_MAT_LENGTH)
            return false;

        this->emm_material = emm_material;
        return true;
    }

    inline bool IsEdge() const
    {
        return Utils::BeginsWith(emm_material, "edge", false);
    }

    void Decompile(TiXmlNode *root, uint32_t id, size_t polygon_count) const;
    bool Compile(const TiXmlElement *root, SkeletonFile *skl);

    bool operator==(const SubMesh &rhs) const;

    inline bool operator!=(const SubMesh &rhs) const
    {
        return !(*this == rhs);
    }
};

class Texture
{
private:

    uint8_t emb_index;
    float f1, f2;

    // from EMGTextureHeader
    uint8_t unk_00;
    uint8_t unk_02[2];

    friend class TexturesList;
    friend class SubPart;
    friend class EmgFile;
    friend class PartsGroup;
    friend class EmoFile;

public:

    inline uint8_t GetEmbIndex() { return emb_index; }
    inline void SetEmbIndex(uint8_t index) { this->emb_index = index; }

    void Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);

    inline bool operator==(const Texture &rhs) const
    {
        return (this->emb_index == rhs.emb_index &&
                this->f1 == rhs.f1 &&
                this->f2 == rhs.f2 &&
                this->unk_00 == rhs.unk_00 &&
                this->unk_02[0] == rhs.unk_02[0] &&
                this->unk_02[1] == rhs.unk_02[1]);
    }

    inline bool operator!=(const Texture &rhs) const
    {
        return !(*this == rhs);
    }
};

class EmgFile : public BaseFile
{
private:

    std::vector<SubPart> subparts;

    uint16_t unk_04; // from EMGChunkHeader

    // METADATA - DON'T USE IN COMPARISON
    std::string meta_name;

    friend class PartsGroup;
    friend class EmoFile;

    void Reset();

    unsigned int CalculatePartSize();
    unsigned int CalculateVertexSize();

    unsigned int CreatePart(uint8_t *buf, SkeletonFile *skl, uint32_t *vertex_start);
    unsigned int CreateVertex(uint8_t *buf);

protected:

    size_t GetNumVertex();

public:

    EmgFile();
    EmgFile(bool big_endian) : EmgFile() { this->big_endian = big_endian; }
    virtual ~EmgFile();

    bool Load(uint8_t *buf, size_t size, SkeletonFile *skl);

    inline bool IsEmpty() const { return (subparts.size() == 0); }

    inline uint16_t GetNumSubParts() const { return subparts.size(); }

    inline SubPart *GetSubPart(uint16_t idx)
    {
        if (idx >= subparts.size())
            return nullptr;

        return &subparts[idx];
    }

    inline const SubPart *GetSubPart(uint16_t idx) const
    {
        if (idx >= subparts.size())
            return nullptr;

        return &subparts[idx];
    }

    inline bool SubPartExists(uint16_t idx) { return (GetSubPart(idx) != nullptr); }

    size_t GetLinkedBones(std::vector<Bone*> &list, bool clear_vector, bool unique=true, const SkeletonFile *sorter=nullptr) const;
    size_t GetEmmMaterials(std::vector<std::string> &list, bool clear_vector, bool unique=true) const;
    size_t GetEmbIndexes(std::vector<uint8_t> &list, bool clear_vector, bool unique=true, bool sort=false) const;

    size_t ReplaceEmmMaterial(const std::string &old_mat, const std::string &new_mat);
    size_t ReplaceEmbIndex(uint8_t old_index, uint8_t new_index);

    bool IsEdge() const;

    size_t ExportObj(std::string *vertex_out, std::string *uvmap_out, std::string *normal_out, std::string *topology_out, size_t v_start_idx=0, bool write_group=true) const;

    void Decompile(TiXmlNode *root, uint16_t id) const;
    bool Compile(const TiXmlElement *root, SkeletonFile *skl);

#ifdef FBX_SUPPORT

    bool ExportFbx(const SkeletonFile &skl, const std::vector<FbxNode *> &fbx_bones, FbxScene *scene) const;
    bool InjectFbx(SkeletonFile &skl, FbxScene *scene, bool use_fbx_tangent);

#endif

    inline bool operator==(const EmgFile &rhs) const
    {
        if (this->IsEmpty() && rhs.IsEmpty())
            return true;

        return (this->subparts == rhs.subparts &&
                this->unk_04 == rhs.unk_04);
    }

    inline bool operator!=(const EmgFile &rhs) const
    {
        return !(*this == rhs);
    }

    inline SubPart &operator[](size_t n) { return subparts[n]; }
    inline const SubPart &operator[](size_t n) const { return subparts[n]; }

    inline std::vector<SubPart>::iterator begin() { return subparts.begin(); }
    inline std::vector<SubPart>::iterator end() { return subparts.end(); }

    inline std::vector<SubPart>::const_iterator begin() const { return subparts.begin(); }
    inline std::vector<SubPart>::const_iterator end() const { return subparts.end(); }
};

#endif
