#ifndef __BCSFILE_H__
#define __BCSFILE_H__

#include <queue>
#include "BaseFile.h"
#include "FixedMemoryStream.h"

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

#define BCS_SIGNATURE   0x53434223

typedef struct
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t header_size; // 6
    uint32_t unk_08; // 8 always zero
    uint16_t num_part_sets; // 0xC
    uint16_t num_parts_colors; // 0xE
    uint16_t num_bodies; // 0x10
    uint16_t num_additional_skeletons; // > 0 if skeleton2_table_offset is valid
    uint32_t unk_14; // New in xenoverse 2. Always found to be zero.
    uint32_t part_sets_table_offset; // 0x18
    uint32_t parts_colors_table_offset; // 0x1C
    uint32_t bodies_table_offset; // 0x20
    uint32_t skeleton2_table_offset; // 0x24 Found only in very few files (like NMC.bcs) Despite this value being above skeleton_table_offset in the header, the data is found later!
    uint32_t skeleton_table_offset; // 0x28 Single entry table? Can be 0
    uint32_t unk_2C; // Observed values: 1 and 0x100
    float unk_30[7]; // Last value has always been observed to be zero, so it is difficult to know if really a float
} PACKED BCSHeader;

STATIC_ASSERT_STRUCT(BCSHeader, 0x4C);

typedef struct
{
    uint32_t unk_00[5]; // Always zero? Check!!!
    uint32_t num_parts; // 0x14
    uint32_t table_start; // 0x18  always 0x20... (to be checked in global bcs files test)
    uint32_t unl_1C; // Check for always 0
    uint32_t parts_offsets[10]; // 0x20
} PACKED BCSPartSet;

STATIC_ASSERT_STRUCT(BCSPartSet, 0x48);

typedef struct
{
    uint16_t model; // 0
    uint16_t model2; // 2
    uint16_t texture; // 4
    uint16_t unk_06; // Only 0 was observed in 100% of XV2 files
    uint64_t unk_08; // Only 0 was observed in 100% of XV2 files
    uint16_t unk_10; // 0 and 1 have been observed
    uint16_t num_color_selectors; // 0x12
    uint32_t color_selector_offset; // 0x14  0 or offset to X elements of 4 bytes pointed by above
    uint32_t unk_18; // Not always zero
    uint32_t unk_1C; // Not always zero
    uint32_t unk_20; // Not always zero
    float unk_24;
    float unk_28;
    uint32_t unk_2C; // Only 0 was observed in 100% of XV2 files
    uint32_t unk_30; // Only 0 was observed in 100% of XV2 files
    char name[4]; // 0x34
    uint32_t files_offsets[4]; // String pointers. They can be 0
    uint16_t unk_48; // Always zero
    uint16_t num_physics; // Num of 0x48 struct size after   example at 0xA9FC
    uint32_t physics_offset; // If prev not zero, this indicates start of data
    uint16_t unk_50; // Always zero
    uint16_t num_unk3; // Num of [variable] size after
    uint32_t unk3_offset; // start of pointed by previous
} PACKED BCSPart;

STATIC_ASSERT_STRUCT(BCSPart, 0x58);

typedef struct
{
    uint16_t part_colors;
    uint16_t color;
} PACKED BCSColorSelector;

STATIC_ASSERT_STRUCT(BCSColorSelector, 4);

typedef struct
{
    uint16_t unk_00;
    uint16_t unk_02;
    uint16_t unk_04;
    uint16_t unk_06; // Zero was observed 100% of cases in 100% of files
    uint64_t unk_08; // Zero was observed 100% of cases in 100% of files
    uint32_t unk_10; // Zero was observed 100% of cases in 100% of files
    uint32_t unk_14; // Zero was observed 100% of cases in 100% of files
    uint32_t unk_18; //
    uint32_t unk_1C; //
    uint32_t unk_20; //
    char name[4]; // 0x24
    uint32_t unk_28[6]; // String pointers
    uint64_t unk_40; // Zero was observed 100% of cases in 100% of files
} PACKED BCSPhysics;

STATIC_ASSERT_STRUCT(BCSPhysics, 0x48);

typedef struct
{
    uint32_t name_offset; // Pointer to string
    uint32_t unk_04; // Always zero
    uint16_t unk_08; // Always zero
    uint16_t num; // 0xA
    uint32_t floats_offset; // 0xC Pointer to num*20 floats NEVER zero (but probably it would if num were zero). The last 4 of every group of 20 is always zero, so it is not possible to guess if part of the array of floats
} PACKED BCSPartColors;

STATIC_ASSERT_STRUCT(BCSPartColors, 0x10);

typedef struct
{
    uint16_t unk_00; // Always zero
    uint16_t num; // 4
    uint32_t unk1_offset; // 8 Can be zero (if num = 0)
} PACKED BCSBody;

STATIC_ASSERT_STRUCT(BCSBody, 8);

typedef struct
{
    float scale[3]; // 0
    uint32_t name_offset; // 0xC
} PACKED BCSBoneScale;

STATIC_ASSERT_STRUCT(BCSBoneScale, 0x10);

typedef struct
{
    uint16_t unk_00;
    uint16_t num_bones; // 2
    uint32_t data_start; // 4
} PACKED BCSSkeletonData;

STATIC_ASSERT_STRUCT(BCSSkeletonData, 8);

typedef struct
{
    uint32_t unk_00; // Always 0x12 (18) except in BJK.bcs
    uint32_t unk_04;
    uint32_t unk_08; // Found always zero
    uint16_t unk_0C[18]; // Half floats
    uint32_t name_offset; // 0x30
} PACKED BCSBone;

STATIC_ASSERT_STRUCT(BCSBone, 0x34);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

class BcsFile;

struct BcsColorSelector
{
    uint16_t part_colors;
    uint16_t color;

    bool Load(const uint8_t *top, const BCSColorSelector *file_unk1);
    inline size_t PreSave() const { return sizeof(BCSColorSelector); }
    size_t Save(BcsFile *file, const uint8_t *top, BCSColorSelector *file_unk1) const;

    TiXmlElement *Decompile(TiXmlNode *root, const BcsFile *owner=nullptr) const;
    bool Compile(const TiXmlElement *root);

    inline bool operator==(const BcsColorSelector &rhs) const
    {
        return (this->part_colors == rhs.part_colors &&
                this->color == rhs.color);
    }

    inline bool operator!=(const BcsColorSelector &rhs) const
    {
        return !(*this == rhs);
    }
};

struct BcsPhysics
{
    uint16_t unk_00;
    uint16_t unk_02;
    uint16_t unk_04; // color?
    uint32_t unk_18;
    uint32_t unk_1C;
    uint32_t unk_20;
    char name[4];
    std::string unk_28[6];

    bool Load(BcsFile *file, const uint8_t *top, const BCSPhysics *file_unk2);
    size_t PreSave(size_t *str_size, std::vector<std::string> &strings) const;
    size_t Save(BcsFile *file, const uint8_t *top, BCSPhysics *file_unk2, std::queue<uint32_t> &strings) const;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);

    inline bool operator==(const BcsPhysics &rhs) const
    {
        bool ret = (this->unk_00 == rhs.unk_00 &&
                    this->unk_02 == rhs.unk_02 &&
                    this->unk_04 == rhs.unk_04 &&
                    this->unk_18 == rhs.unk_18 &&
                    this->unk_1C == rhs.unk_1C &&
                    this->unk_20 == rhs.unk_20 &&
                    memcmp(this->name, rhs.name, 4) == 0);

        if (!ret)
            return ret;

        for (size_t i = 0; i < 6; i++)
            if (unk_28[i] != rhs.unk_28[i])
                return false;

        return true;
    }

    inline bool operator!=(const BcsPhysics &rhs) const
    {
        return !(*this == rhs);
    }
};

struct BcsPart
{
    bool valid;

    uint16_t model;
    uint16_t model2;
    uint16_t texture;
    uint16_t unk_10;

    std::vector<BcsColorSelector> color_selectors;

    uint32_t unk_18;
    uint32_t unk_1C;
    uint32_t unk_20;
    float unk_24;
    float unk_28;
    uint32_t unk_2C;
    uint32_t unk_30;

    char name[4];
    std::string files[4];    // <STR_38 value="FULL-items, EMM, EMB, EAN" />

    std::vector<BcsPhysics> physics;

    uint16_t num_unk3;
    std::vector<uint8_t> unk3_data;

    BcsPart() : valid(false) { }
    bool Load(BcsFile *file, const uint8_t *top, const BCSPart *file_part);
    size_t PreSave(size_t *str_size, std::vector<std::string> &strings) const;
    size_t Save(BcsFile *file, const uint8_t *top, BCSPart *file_part, std::queue<uint32_t> &strings) const;

    TiXmlElement *Decompile(TiXmlNode *root, int idx, const BcsFile *owner=nullptr) const;
    bool Compile(const TiXmlElement *root);

    inline bool operator==(const BcsPart &rhs) const
    {
        if (this->valid != rhs.valid)
            return false;

        if (!this->valid)
            return true;

        bool ret = (this->model == rhs.model &&
                    this->model2 == rhs.model2 &&
                    this->texture == rhs.texture &&
                    this->unk_10 == rhs.unk_10 &&
                    this->color_selectors == rhs.color_selectors &&
                    this->unk_18 == rhs.unk_18 &&
                    this->unk_1C == rhs.unk_1C &&
                    this->unk_20 == rhs.unk_20 &&
                    this->unk_24 == rhs.unk_24 &&
                    this->unk_28 == rhs.unk_28 &&
                    this->unk_2C == rhs.unk_2C &&
                    this->unk_30 == rhs.unk_30 &&
                    memcmp(this->name, rhs.name, 4) == 0);

        if (!ret)
            return false;

        for (size_t i = 0; i < 4; i++)
            if (this->files[i] != rhs.files[i])
                return false;

        return (this->physics == rhs.physics &&
                this->num_unk3 == rhs.num_unk3 &&
                this->unk3_data == rhs.unk3_data);
    }

    inline bool operator!=(const BcsPart &rhs) const
    {
        return !(*this == rhs);
    }
};

struct BcsPartSet
{
    bool valid;
    std::vector<BcsPart> parts;
    /*0 - Face_base
    1 - Face_forehead
    2 - Eye_base
    3 - Nose
    4 - Ear
    5 - Hair
    6 - Bust
    7 - Pant
    8 - Risk
    9 - Boots*/


    BcsPartSet() : valid(false) { }
    bool Load(BcsFile *file, const uint8_t *top, const BCSPartSet *file_set);
    size_t PreSave(size_t *str_size, std::vector<std::string> &strings) const;
    size_t Save(BcsFile *file, const uint8_t *top, BCSPartSet *file_set, std::queue<uint32_t> &strings) const;

    TiXmlElement *Decompile(TiXmlNode *root, int idx, const BcsFile *owner=nullptr) const;
    bool Compile(const TiXmlElement *root);

    inline bool operator==(const BcsPartSet &rhs) const
    {
        if (this->valid != rhs.valid)
            return false;

        if (!this->valid)
            return true;

        return (this->parts == rhs.parts);
    }

    inline bool operator!=(const BcsPartSet &rhs) const
    {
        return !(*this == rhs);
    }
};

struct BcsColor
{
    float floats[20];

    bool Load(const float *floats);
    //bool Save(float *floats) const;

    uint32_t GetColorPreview(bool special_case_eyes) const;

    TiXmlElement *Decompile(TiXmlNode *root, int idx, bool special_case_eyes) const;
    bool Compile(const TiXmlElement *root);

    inline bool operator==(const BcsColor &rhs) const
    {
        return (memcmp(this->floats, rhs.floats, sizeof(floats)) == 0);
    }

    inline bool operator!=(const BcsColor &rhs) const
    {
        return !(*this == rhs);
    }
};

struct BcsPartColors
{
    bool valid;
    std::string name;
    std::vector<BcsColor> colors;

    BcsPartColors() : valid(false) { }
    bool Load(BcsFile *file, const uint8_t *top, const BCSPartColors *file_part_colors);
    size_t PreSave(size_t *str_size, std::vector<std::string> &strings) const;
    size_t Save(BcsFile *file, const uint8_t *top, BCSPartColors *file_part_colors, std::queue<uint32_t> &strings, FixedMemoryStream &s2_stream, uint32_t doffs) const;

    TiXmlElement *Decompile(TiXmlNode *root, int idx) const;
    bool Compile(const TiXmlElement *root);

    inline bool operator==(const BcsPartColors &rhs) const
    {
        if (this->valid != rhs.valid)
            return false;

        if (!this->valid)
            return true;

        return (this->name == rhs.name &&
                this->colors == rhs.colors);
    }

    inline bool operator!=(const BcsPartColors &rhs) const
    {
        return !(*this == rhs);
    }
};

struct BcsBoneScale
{
    float scale[3];
    std::string name;

    bool Load(BcsFile *file, const uint8_t *top, const BCSBoneScale *file_bone_scale);
    size_t PreSave(size_t *str_size, std::vector<std::string> &strings) const;
    size_t Save(BcsFile *file, const uint8_t *top, FixedMemoryStream &stream, std::queue<uint32_t> &strings) const;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);

    inline bool operator==(const BcsBoneScale &rhs) const
    {
        return (memcmp(this->scale, rhs.scale, sizeof(scale)) == 0 &&
                this->name == rhs.name);
    }

    inline bool operator!=(const BcsBoneScale &rhs) const
    {
        return !(*this == rhs);
    }
};

struct BcsBody
{
    bool valid;
    std::vector<BcsBoneScale> unk1s;

    BcsBody() : valid(false) { }
    bool Load(BcsFile *file, const uint8_t *top, const BCSBody *file_unk2);
    size_t PreSave(size_t *str_size, std::vector<std::string> &strings) const;
    size_t Save(BcsFile *file, const uint8_t *top, BCSBody *file_unk2, std::queue<uint32_t> &strings, FixedMemoryStream &s2_stream, uint32_t doffs) const;

    TiXmlElement *Decompile(TiXmlNode *root, int idx) const;
    bool Compile(const TiXmlElement *root);

    inline bool IsEmpty() const
    {
        return (!valid || unk1s.size() == 0);
    }

    inline bool operator==(const BcsBody &rhs) const
    {
        if (this->valid != rhs.valid)
            return false;

        if (!this->valid)
            return true;

        return (this->unk1s == rhs.unk1s);
    }

    inline bool operator!=(const BcsBody &rhs) const
    {
        return !(*this == rhs);
    }
};

struct BcsBone
{
    uint32_t unk_00;
    uint32_t unk_04;
    uint32_t unk_08; // Found always zero
    uint16_t unk_0C[18]; // Half floats
    std::string name;

    bool Load(BcsFile *file, const uint8_t *top, const BCSBone *file_bone);
    size_t PreSave(size_t *str_size, std::vector<std::string> &strings) const;
    size_t Save(BcsFile *file, const uint8_t *top, BCSBone *file_bone, std::queue<uint32_t> &strings) const;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);

    inline bool operator==(const BcsBone &rhs) const
    {
        return (this->unk_00 == rhs.unk_00 &&
                this->unk_04 == rhs.unk_04 &&
                this->unk_08 == rhs.unk_08 &&
                memcmp(this->unk_0C, rhs.unk_0C, sizeof(unk_0C)) == 0 &&
                this->name == rhs.name);
    }

    inline bool operator!=(const BcsBone &rhs) const
    {
        return !(*this == rhs);
    }
};

struct BcsSkeletonData
{
    bool valid;

    uint16_t unk_00;
    std::vector<BcsBone> bones;

    BcsSkeletonData() : valid(false) { }
    bool Load(BcsFile *file, const uint8_t *top, const BCSSkeletonData *file_skl);
    size_t PreSave(size_t *str_size, std::vector<std::string> &strings) const;
    size_t Save(BcsFile *file, const uint8_t *top, BCSSkeletonData *file_skl, std::queue<uint32_t> &strings) const;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);

    inline bool operator==(const BcsSkeletonData &rhs) const
    {
        if (this->valid != rhs.valid)
            return false;

        if (!this->valid)
            return true;

        return (this->unk_00 == rhs.unk_00 &&
                this->bones == rhs.bones);
    }

    inline bool operator!=(const BcsSkeletonData &rhs) const
    {
        return !(*this == rhs);
    }
};

class BcsFile : public BaseFile
{
private:

    friend struct BcsPartSet;
    friend struct BcsPart;
    friend struct BcsPhysics;
    friend struct BcsPartColors;
    friend struct BcsBody;
    friend struct BcsBoneScale;
    friend struct BcsSkeletonData;
    friend struct BcsBone;
    friend struct BcsColorSelector;

    std::vector<BcsPartSet> sets;
    std::vector<BcsPartColors> parts_colors;
    std::vector<BcsBody> bodies;
    BcsSkeletonData skeleton_data;

    std::vector<BcsSkeletonData> additional_skeleton_data; // Found only in NMC.bcs and ZMD.bcs in tests

    bool has_signature;
    uint32_t unk_2C;
    float unk_30[7];

protected:

    void Reset();

public:
    BcsFile();
    virtual ~BcsFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    inline const std::vector<BcsPartSet> &GetPartSets() const { return sets; }
    inline std::vector<BcsPartSet> &GetPartSets() { return sets; }

    inline const std::vector<BcsPartColors> &GetPartsColors() const { return parts_colors; }
    inline std::vector<BcsPartColors> &GetPartsColors() { return parts_colors; }

    inline const std::vector<BcsBody> &GetBodies() const { return bodies; }
    inline std::vector<BcsBody> &GetBodies() { return bodies; }

    uint32_t FindPartColorsByName(const std::string &name) const;

    bool GetPreviewColor(const std::string &name, uint32_t color_id, uint32_t *preview_color) const;
    bool ListPreviewColors(const std::string &name, std::vector<uint32_t> &colors) const;

    inline bool operator==(const BcsFile &rhs) const
    {
        return (this->sets == rhs.sets &&
                this->parts_colors == rhs.parts_colors &&
                this->bodies == rhs.bodies &&
                this->skeleton_data == rhs.skeleton_data &&
                this->additional_skeleton_data == rhs.additional_skeleton_data &&
                this->has_signature == rhs.has_signature &&
                this->unk_2C == rhs.unk_2C &&
                memcmp(this->unk_30, rhs.unk_30, sizeof(unk_30)) == 0);
    }

    inline bool operator!=(const BcsFile &rhs) const
    {
        return !(*this == rhs);
    }
};

#endif // __BCSFILE_H__
