#ifndef __SKELETONFILE_H__
#define __SKELETONFILE_H__

#include <stdint.h>

#include <string>
#include <vector>
#include <stdexcept>

#include "BaseFile.h"

#ifdef FBX_SUPPORT
#include <fbxsdk.h>
#endif

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    uint16_t node_count;		// 0
    uint16_t unk_02;			// 2
    uint16_t ik_count;          // 4
    uint16_t unk_06;            // 6
    uint32_t start_offset;		// 8
    uint32_t names_offset;		// 0x0C
    uint32_t unk_10[2];			// 0x10
    uint32_t unk_skd_offset;	// 0x18   UnkSkeletonData
    uint32_t matrix_offset;		// 0x1C   MatrixData
    uint32_t ik_data_offset;	// 0x20   only one entry!
    uint32_t unk_24[4];		// 0x24 apprently 0
    uint16_t unk_34[2];         // 0x34
    float unk_38[2];            // 0x38
    // 0x40
} PACKED SkeletonHeader;

static_assert(sizeof(SkeletonHeader) == 0x40, "Incorrect structure size.");

typedef struct
{
    uint16_t parent_id;	// 0
    uint16_t child1;		// 2
    uint16_t child2;	// 4
    uint16_t child3;	// 6
    uint16_t child4;	// 8
    uint16_t unk_0A[3];	// 0xA
    float matrix[16]; // 0x10
    // 0x50
} PACKED SkeletonNode;

static_assert(sizeof(SkeletonNode) == 0x50, "Incorrect structure size.");

typedef struct
{
    uint16_t unk_00[4]; // 0
    // 8
} PACKED UnkSkeletonData;

static_assert(sizeof(UnkSkeletonData) == 8, "Incorrect structure size.");

typedef struct
{
    float matrix_00[16]; // 0
    // 0x40
} PACKED MatrixData;

static_assert(sizeof(MatrixData) == 0x40, "Incorrect structure size.");

typedef struct
{
    uint16_t unk_00; // 0
    uint16_t entry_size; // 2
    uint8_t unk_03; // 4
    uint8_t unk_04; // 5
    uint16_t unk_06; // 6
    uint16_t unk_08[4]; // 8
    uint32_t unk_10[2]; // 0x10
} PACKED IKEntry;

static_assert(sizeof(IKEntry) == 0x18, "Incorrect structure size.");

#ifdef _MSC_VER
#pragma pack(pop)
#endif

namespace Utils
{
#ifdef FBX_SUPPORT

    void FbxMatrixToArray(float *mo, const FbxMatrix *mi);
    void FbxMatrixToArray(double *mo, const FbxMatrix *mi);
    FbxMatrix ArrayToFbxMatrix(const float *mi);
    FbxAMatrix ArrayToFbxAMatrix(const float *mi);

    FbxAMatrix GetGlobalDefaultPosition(FbxNode* node);
    void SetGlobalDefaultPosition(FbxNode* node, FbxAMatrix global_position);

#endif
}

class SkeletonFile;
class EmoFile;
class EmgFile;
class BoneSorter;

class Bone
{
private:

    std::string name;

    Bone *parent = nullptr;
    Bone *child1 = nullptr;
    Bone *child2 = nullptr;
    Bone *child3 = nullptr;
    Bone *child4 = nullptr;

    float matrix1[16];
    float matrix2[16];

    uint16_t sn_u0A[3];
    uint16_t usd_u00[4];

    bool has_unk, has_matrix2;

    // METADATA - DON'T USE IN COMPARISON!
    // This is only to display information about original file.
    // This looses its meaning when the skeleton is modified
    uint32_t meta_original_offset;

    static void DecompileTransformationMatrix(TiXmlElement *root, const char *name, const float *matrix);
    static int CompileTransformationMatrix(const TiXmlElement *root, const char *name, float *matrix, bool must_exist);
    static bool PartialCompare(const Bone *b1, const Bone *b2);

    friend class SkeletonFile;
    friend class EmoFile;
    friend class EmgFile;

public:

    inline std::string GetName() const { return name; }

    inline uint32_t GetOriginalOffset() const { return meta_original_offset; }

    inline void GetMatrix1(float *m) const
    {
        memcpy(m, matrix1, sizeof(matrix1));
    }

    inline bool GetMatrix2(float *m) const
    {
        if (!has_matrix2)
            return false;

        memcpy(m, matrix2, sizeof(matrix2));
        return true;
    }

    void SetMatrix(float *m)
    {
        memcpy(matrix1, m, sizeof(matrix1));
    }

    bool SetMatrix2(float *m)
    {
        if (!has_matrix2)
            return false;

        memcpy(matrix2, m, sizeof(matrix2));
        return true;
    }

    inline bool HasMatrix2() const { return has_matrix2; }

    inline void SetHasMatrix2(bool has_matrix2) { this->has_matrix2 = has_matrix2; }

    inline Bone *GetParent() { return parent; }
    inline Bone *GetChild1() { return child1; }
    inline Bone *GetChild2() { return child2; }
    inline Bone *GetChild3() { return child3; }
    inline Bone *GetChild4() { return child4; }

    inline const Bone *GetParent() const { return parent; }
    inline const Bone *GetChild1() const { return child1; }
    inline const Bone *GetChild2() const { return child2; }
    inline const Bone *GetChild3() const { return child3; }
    inline const Bone *GetChild4() const { return child4; }

    inline void SetName(const std::string & str)
    {
        name = str;
    }

    void Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root, SkeletonFile *skl);

#ifdef FBX_SUPPORT

    FbxAMatrix GetGlobalTransform() const;

#endif

    bool operator==(const Bone & rhs) const;
    inline bool operator!=(const Bone & rhs) const
    {
        return !(*this == rhs);
    }

};

class SkeletonFile : public BaseFile
{
private:

    uint8_t *ik_data;
    size_t ik_size;

    uint16_t unk_02;
    uint16_t unk_06;

    uint32_t unk_10[2];
    uint16_t unk_34[2];
    float unk_38[2];

    friend class EmgFile;
    friend class SubPart;
    friend class BoneSorter;

protected:

    std::vector<Bone> bones;

    void Copy(const SkeletonFile &other);
    void Reset();

    static uint16_t FindBone(const std::vector<Bone *> &bones, Bone *bone, bool assert_if_not_found);
    virtual void RebuildSkeleton(const std::vector<Bone *> &old_bones_ptr);
    uint16_t BoneToIndex(Bone *bone) const;

    size_t CalculateIKSize(const uint8_t *ik_data, uint16_t count);
    uint16_t CalculateIKCount(const uint8_t *ik_data, size_t size);

    void TranslateIKData(uint8_t *dst, const uint8_t *src, size_t size, bool import);

    unsigned int CalculateFileSize() const;

#ifdef FBX_SUPPORT

    bool ExportFbxBone(const Bone *parent, FbxNode *root_node, FbxScene *scene, std::vector<FbxNode *> &fbx_bones) const;

#endif

public:

    SkeletonFile();
    SkeletonFile(uint8_t *buf, unsigned int size);
    SkeletonFile(const SkeletonFile &other)
    {
        Copy(other);
    }

    virtual ~SkeletonFile();

    inline uint16_t GetNumBones() const { return bones.size(); }
    inline Bone *GetBone(uint16_t idx)
    {
        if (idx >= bones.size())
            return nullptr;

        return &bones[idx];
    }   

    inline Bone *GetBone(const std::string &name)
    {
        for (Bone &b : bones)
        {
            if (b.name == name)
                return &b;
        }

        return nullptr;
    }

    inline const Bone *GetBone(const std::string &name) const
    {
        for (const Bone &b : bones)
        {
            if (b.name == name)
                return &b;
        }

        return nullptr;
    }

    inline bool BoneExists(const std::string &name) const { return (GetBone(name) != nullptr); }

    uint16_t AppendBone(const Bone &bone);
    uint16_t AppendBone(const SkeletonFile &other, const std::string &name);

    bool CloneBoneParentChild(const SkeletonFile &other, const std::string &bone_name, Bone **not_found=nullptr);

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    void Decompile(TiXmlNode *root) const;
    virtual TiXmlDocument *Decompile() const override;

    bool Compile(const TiXmlElement *root);
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    virtual bool SaveSkeletonToFile(const std::string &path, bool show_error=true, bool build_path=false);
    virtual bool DecompileSkeletonToFile(const std::string &path, bool show_error=true, bool build_path=false);

#ifdef FBX_SUPPORT

    bool ExportFbx(FbxScene *scene, std::vector<FbxNode *> &fbx_bones) const;

#endif

    inline SkeletonFile &operator=(const SkeletonFile &other)
    {
        if (this == &other)
            return *this;

        Copy(other);
        return *this;
    }

    bool operator==(const SkeletonFile &rhs) const;

    inline bool operator!=(const SkeletonFile &rhs) const
    {
        return !(*this == rhs);
    }

    inline Bone &operator[](size_t n) { return bones[n]; }
    inline const Bone &operator[](size_t n) const { return bones[n]; }

    inline Bone &operator[](const std::string &bone_name)
    {
        Bone *b = GetBone(bone_name);
        if (!b)
        {
            throw std::out_of_range("Bone " + bone_name + " doesn't exist.");
        }

        return *b;
    }

    inline const Bone &operator[](const std::string &bone_name) const
    {
        const Bone *b = GetBone(bone_name);
        if (!b)
        {
            throw std::out_of_range("Bone " + bone_name + " doesn't exist.");
        }

        return *b;
    }

    inline const SkeletonFile operator+(const Bone &bone) const
    {
        SkeletonFile new_skl = *this;

        new_skl.AppendBone(bone);
        return new_skl;
    }

    inline SkeletonFile &operator+=(const Bone &bone)
    {
        this->AppendBone(bone);
        return *this;
    }

    inline std::vector<Bone>::iterator begin() { return bones.begin(); }
    inline std::vector<Bone>::iterator end() { return bones.end(); }

    inline std::vector<Bone>::const_iterator begin() const { return bones.begin(); }
    inline std::vector<Bone>::const_iterator end() const { return bones.end(); }
};

class BoneSorter
{
private:

    const SkeletonFile *skl;

public:

    BoneSorter(const SkeletonFile *skl) : skl(skl) {}

    bool operator()(Bone * const &b1, Bone * const &b2)
    {
        return (skl->BoneToIndex(b1) < skl->BoneToIndex(b2));
    }

};

#endif // __SKELETONFILE_H__
