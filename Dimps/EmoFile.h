#ifndef __EMOFILE_H__
#define __EMOFILE_H__

#include <stdint.h>
#include "SkeletonFile.h"
#include "EmgFile.h"
#include "EmmFile.h"

// "#EMO"
#define EMO_SIGNATURE	0x4F4D4523

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    uint32_t signature; 		// 0
    uint16_t endianess_check;	// 4
    uint16_t header_size;		// 6
    uint16_t unk_08[2]; 		// 8
    uint32_t parts_offset; 		// 0x0C
    uint32_t skeleton_offset;	// 0x10
    uint32_t vertex_offset;		// 0x14
    uint32_t unk_18[2];			// 0x18
    // 0x20
} PACKED EMOHeader;

static_assert(sizeof(EMOHeader) == 0x20, "Incorrect structure size.");

typedef struct
{
    uint16_t groups_count;	// 0
    uint16_t material_count;// 2
    uint32_t names_offset;	// 4
    uint32_t offsets[1];	// 8
    // 0xC Remaining offsets
} PACKED PartsGroupHeader;

static_assert(sizeof(PartsGroupHeader) == 0xC, "Incorrect structure size.");

typedef struct
{
    uint16_t emg_count; // 0
    uint16_t unk_02; // Let's assume padding
    uint32_t offsets[1]; // 4
    // 0x8 Remaining offsets
} PACKED PartHeader;

static_assert(sizeof(PartHeader) == 8, "Incorrect structure size.");

#ifdef _MSC_VER
#pragma pack(pop)
#endif

class PartsGroup
{
private:

    std::string name;
    std::vector<EmgFile> parts;

    // METADATA - DON'T USE IN COMPARISON
    // Only makes sense after loading, before any modification. It only exists to display original file info
    uint32_t meta_original_offset;

    friend class EmoFile;    

protected:

    size_t GetNumVertex();

public:

    inline uint16_t GetNumParts() const { return parts.size(); }

    inline uint32_t GetOriginalOffset() const { return meta_original_offset; }
    inline std::string GetName() const { return name; }

    inline EmgFile *GetPart(uint16_t idx)
    {
        if (idx >= parts.size())
            return nullptr;

        return &parts[idx];
    }

    inline const EmgFile *GetPart(uint16_t idx) const
    {
        if (idx >= parts.size())
            return nullptr;

        return &parts[idx];
    }

    inline bool PartExists(uint16_t idx) const { return (GetPart(idx) != nullptr); }

    size_t GetLinkedBones(std::vector<Bone*> &list, bool clear_vector, bool unique=true, const SkeletonFile *sorter=nullptr) const;
    size_t GetEmmMaterials(std::vector<std::string> &list, bool clear_vector, bool unique=true) const;
    size_t GetEmbIndexes(std::vector<uint8_t> &list, bool clear_vector, bool unique=true, bool sort=false) const;

    inline void SetName(const std::string &name) { this->name = name; }

    size_t ReplaceEmmMaterial(const std::string &old_mat, const std::string &new_mat);
    size_t ReplaceEmbIndex(uint8_t old_index, uint8_t new_index);

    bool IsEdge() const;

    size_t ExportObj(std::string *vertex_out, std::string *uvmap_out, std::string *normal_out, std::string *topology_out, size_t v_start_idx=0, bool write_group=false) const;

    void Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root, SkeletonFile *skl);

#ifdef FBX_SUPPORT

    bool ExportFbx(const SkeletonFile &skl, const std::vector<FbxNode *> &fbx_bones, FbxScene *scene) const;
    bool InjectFbx(SkeletonFile &skl, FbxScene *scene, bool use_fbx_tangent);

#endif

    inline bool operator==(const PartsGroup &rhs) const
    {
       	return (this->name == rhs.name &&
                this->parts == rhs.parts);
    }

    inline bool operator!=(const PartsGroup &rhs) const
    {
        return !(*this == rhs);
    }

    inline EmgFile &operator[](size_t n) { return parts[n]; }
    inline const EmgFile &operator[](size_t n) const { return parts[n]; }

    inline std::vector<EmgFile>::iterator begin() { return parts.begin(); }
    inline std::vector<EmgFile>::iterator end() { return parts.end(); }

    inline std::vector<EmgFile>::const_iterator begin() const { return parts.begin(); }
    inline std::vector<EmgFile>::const_iterator end() const { return parts.end(); }
};

class EmoFile : public SkeletonFile
{
private:

    std::vector<PartsGroup> groups;
    uint16_t material_count;

    // From EMOHeader
    uint16_t unk_08[2];
    uint32_t unk_18[2];

    void Copy(const EmoFile &other);
    void Reset();

    uint16_t GroupPtrToIndex(const PartsGroup *group) const;
    bool PartPtrToIndex(const EmgFile *part, uint16_t *group_idx, uint16_t *part_idx) const;
    bool SubPartPtrToIndex(const SubPart *subpart, uint16_t *group_idx, uint16_t *part_idx, uint16_t *subpart_idx) const;

    size_t GetNextObjSubPart(const std::string &content, size_t *pos, std::string &ret_obj, std::string &ret_name, size_t *num_v, size_t *num_uv, size_t *num_n, bool show_error=true);

    unsigned int CalculateFileSize(uint32_t *vertex_start);

protected:

    size_t GetNumVertex();
    virtual void RebuildSkeleton(const std::vector<Bone *> &old_bones_ptr) override;

public:

    EmoFile();
    EmoFile(uint8_t *buf, size_t size);
    EmoFile(const EmoFile &other) : SkeletonFile()
    {
        Copy(other);
    }

    virtual ~EmoFile();

    inline uint16_t GetNumGroups() const { return groups.size(); }

    inline PartsGroup *GetGroup(uint16_t idx)
    {
        if (idx >= groups.size())
            return nullptr;

        return &groups[idx];
    }

    inline const PartsGroup *GetGroup(uint16_t idx) const
    {
        if (idx >= groups.size())
            return nullptr;

        return &groups[idx];
    }

    inline PartsGroup *GetGroup(const std::string & name)
    {
        for (PartsGroup &p : groups)
        {
            if (p.name == name)
                return &p;
        }

        return nullptr;
    }

    inline const PartsGroup *GetGroup(const std::string &name) const
    {
        for (const PartsGroup &p : groups)
        {
            if (p.name == name)
                return &p;
        }

        return nullptr;
    }

    inline bool GroupExists(uint16_t idx) const
    {
        return (GetGroup(idx) != nullptr);
    }    

    inline bool GroupExists(const std::string &name) const
    {
        return (GetGroup(name) != nullptr);
    }

    inline bool RemoveGroup(uint16_t idx)
    {
        if (idx >= groups.size())
            return false;

        groups.erase(groups.begin()+idx);
    }    

    inline bool RemoveGroup(const std::string &name)
    {
        for (auto it = groups.begin(); it != groups.end(); ++it)
        {
            if ((*it).name == name)
            {
                groups.erase(it);
                return true;
            }
        }

        return false;
    }

    EmgFile *GetPart(const std::string &part_name);
    const EmgFile *GetPart(const std::string &part_name) const;

    inline bool PartExists(const std::string &part_name) const
    {
        return (GetPart(part_name) != nullptr);
    }

    bool RemovePart(const std::string &part_name);

    SubPart *GetSubPart(const std::string &subpart_name);
    const SubPart *GetSubPart(const std::string &subpart_name) const;

    inline bool SubPartExists(const std::string &subpart_name) const
    {
        return (GetSubPart(subpart_name) != nullptr);
    }

    bool RemoveSubPart(const std::string &subpart_name);

    inline uint16_t GetMaterialCount() const { return material_count; }

    inline size_t GetLinkedBones(const PartsGroup &group, std::vector<Bone*> &list, bool clear_vector, bool unique=true, bool sort=false) const
    {
        return (group.GetLinkedBones(list, clear_vector, unique, (sort) ? this : nullptr));
    }

    size_t GetEmmMaterials(std::vector<std::string> &list, bool clear_vector, bool unique=true) const;
    size_t GetEmbIndexes(std::vector<uint8_t> &list, bool clear_vector, bool unique=true, bool sort=false) const;

    size_t ReplaceEmmMaterial(const std::string &old_mat, const std::string &new_mat);
    size_t ReplaceEmbIndex(uint8_t old_index, uint8_t new_index);

    inline void SetMaterialCount(const EmmFile &emm) { material_count = emm.GetNumMaterials(); }

    inline uint16_t AppendGroup(const PartsGroup &group)
    {
        groups.push_back(group);
        return (groups.size()-1);
    }

    uint16_t AppendGroup(const EmoFile &other, const std::string &name);

    size_t CloneLinkedBones(const EmoFile &other, PartsGroup &group, Bone **not_found=nullptr);

    bool HasEdges() const;

    size_t ExportObj(std::string *vertex_out, std::string *uvmap_out, std::string *normal_out, std::string *topology_out, size_t v_start_idx=0, bool write_group=false) const;

    size_t InjectObj(const std::string &obj, bool do_uv, bool do_normal, bool show_error=true);
    size_t InjectObjBySubParts(const std::string &obj, bool do_uv, bool do_normal, bool show_error=true);

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;
	
	virtual TiXmlDocument *Decompile() const override;
	virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    virtual bool DecompileToFile(const std::string &path, bool show_error=true, bool build_path=false) override;
    virtual bool CompileFromFile(const std::string &path, bool show_error=true, bool big_endian=false) override;

#ifdef FBX_SUPPORT

    bool ExportFbx(FbxScene *scene, bool normal_parts, bool edges) const;
    bool InjectFbx(FbxScene *scene, bool normal_parts, bool edges, bool use_fbx_tangent);

    bool ExportFbx(const std::string &subpart, FbxScene *scene) const;
    bool InjectFbx(const std::string &subpart, FbxScene *scene, bool use_fbx_tangent);

#endif

    inline EmoFile &operator=(const EmoFile &other)
    {
        if (this == &other)
            return *this;

        Copy(other);
        return *this;
    }

    bool operator==(const EmoFile &rhs) const;

    inline bool operator!=(const EmoFile &rhs) const
    {
        return !(*this == rhs);
    }

    inline PartsGroup &operator[](size_t n) { return groups[n]; }
    inline const PartsGroup &operator[](size_t n) const { return groups[n]; }

    inline PartsGroup &operator[](const std::string &group_name)
    {
        PartsGroup *pg = GetGroup(group_name);
        if (!pg)
        {
            throw std::out_of_range("Parts group " + group_name + " doesn't exist.");
        }

        return *pg;
    }

    inline const PartsGroup &operator[](const std::string &group_name) const
    {
        const PartsGroup *pg = GetGroup(group_name);
        if (!pg)
        {
            throw std::out_of_range("Parts group " + group_name + " doesn't exist.");
        }

        return *pg;
    }

    inline const EmoFile operator+(const PartsGroup &group) const
    {
        EmoFile new_emo = *this;

        new_emo.AppendGroup(group);
        return new_emo;
    }

    inline EmoFile &operator+=(const PartsGroup &group)
    {
        this->AppendGroup(group);
        return *this;
    }

    inline std::vector<PartsGroup>::iterator begin() { return groups.begin(); }
    inline std::vector<PartsGroup>::iterator end() { return groups.end(); }

    inline std::vector<PartsGroup>::const_iterator begin() const { return groups.begin(); }
    inline std::vector<PartsGroup>::const_iterator end() const { return groups.end(); }
};

class PartsSorter
{
private:

    PartsGroup group;

    inline uint16_t GetIdx(const EmgFile &part)
    {
        for (uint16_t i = 0; i < group.GetNumParts(); i++)
        {
            if (group[i] == part)
                return i;
        }

        return (uint16_t)-1;
    }

public:

    PartsSorter(const PartsGroup &group)
    {
        this->group = group; // Note: we need a real copy, not reference, pointer, etc
    }

    bool operator()(const EmgFile &part1, const EmgFile &part2)
    {
        if (part1.IsEmpty())
        {
            if (!part2.IsEmpty())
                return false;
        }

        else if (part2.IsEmpty())
        {
            return true;
        }

        uint16_t idx1 = GetIdx(part1);
        uint16_t idx2 = GetIdx(part2);

        assert(idx1 != 0xFFFF);
        assert(idx2 != 0xFFFF);

        return (idx1 < idx2);
    }


};

#endif // __EMOFILE_H__
