#ifndef __2RYFILE_H__
#define __2RYFILE_H__

#include <stdexcept>
#include "BaseFile.h"

#define _2RY_SIGNATURE	"#2RY"
#define SCD_SIGNATURE	"#SCD"
#define PTC_SIGNATURE	"#PTC"
#define JNT_SIGNATURE   "#JNT"
#define COL_SIGNATURE   "#COL"

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    char signature[4]; // 0
    uint16_t endianess_check; // 4
    uint8_t unk_06[2]; // 6 Maybe version or something like that
    uint32_t unk_08[2]; // 8  zeroes...
    // size 0x10
} PACKED _2RYHeader;

static_assert(sizeof(_2RYHeader) == 0x10, "Incorrect structure size.");

typedef struct
{
    uint32_t num_scd; // 0
    uint32_t scd_table_offset; // 4
    // size 0x8
} PACKED SCDListHeader;

static_assert(sizeof(SCDListHeader) == 8, "Incorrect structure size.");

typedef struct
{
    uint32_t num_scd_names; // 0  should match num_scd
    uint32_t scd_names_table_offset; // 4
    // size 0x8
} PACKED SCDNamesListHeader;

static_assert(sizeof(SCDNamesListHeader) == 8, "Incorrect structure size.");

typedef struct
{
    char signature[4]; // 0
    uint32_t bones_names_offset; // 4
    float unk_08; // 8
    float unk_0C; // 0xC
    // size 0x10
} PACKED SCDHeader;

static_assert(sizeof(SCDHeader) == 0x10, "Incorrect structure size.");

typedef struct
{
    uint32_t num_ptc_roots; // 0
    uint32_t ptc_roots_offset; // 04 Points to a serie of 32 bits number. Maybe they are indexes to something?
    // size 8
} PACKED PtcRootsListHeader;

static_assert(sizeof(PtcRootsListHeader) == 8, "Incorrect structure size.");

typedef struct
{
    uint32_t num_ptc; // 0
    uint32_t ptc_offset; // 4
    // size 8
} PACKED PTCListHeader;

static_assert(sizeof(PTCListHeader) == 8, "Incorrect structure size.");

typedef struct
{
    uint32_t num_jnt; // 0
    uint32_t jnt_offset; // 4
    // size 8
} PACKED JNTListHeader;

static_assert(sizeof(JNTListHeader) == 8, "Incorrect structure size.");

typedef struct
{
    uint32_t num_col; // 0
    uint32_t col_offset; // 3
    // size 8
} PACKED COLListHeader;

static_assert(sizeof(COLListHeader) == 8, "Incorrect structure size.");

typedef struct
{
    uint32_t num_bone_names;
    uint32_t bones_names_offset; // Seems to point to same location as SCDHeader->bones_names_offset
    // size 8
} PACKED BonesNamesListHeader;

static_assert(sizeof(BonesNamesListHeader) == 8, "Incorrect structure size.");

typedef struct
{
    uint32_t unk[2]; // They are FFFFFFFF each one. Probably they are num and offset when they are different, but we haven't seen that yet
    // size 8
} PACKED Unknown2ListHeader;

static_assert(sizeof(Unknown2ListHeader) == 8, "Incorrect structure size.");

typedef struct
{
    char signature[4]; // 0
    uint32_t id; // 4  should match the position in the list, from 0 to N-1
    uint32_t parent; // 8  parent index or FFFFFFFF if none
    uint32_t child; // 0xC child or FFFFFFFF if none
    float unk_10[7]; // 0x10
    uint32_t unk_2C; // 0x2C  zero... maybe it is part of unk_10, or maybe it is padding
    // size 0x30
} PACKED PTCEntry;

static_assert(sizeof(PTCEntry) == 0x30, "Incorrect structure size.");

typedef struct
{
    char signature[4]; // 0
    uint16_t ptc1; // 2
    uint16_t ptc2; // 4
    uint32_t unk_08; // 8  Only zero and 1 found so far
    float unk_0C[5]; // 0xC
    uint32_t unk_20[4]; // 0x20  zeroes... some of it may be part of unk_0C and the rest padding?
    // size 0x30
} PACKED JNTEntry;

static_assert(sizeof(JNTEntry) == 0x30, "Incorrect structure size.");

typedef struct
{
    char signature[4]; // 0
    uint32_t unk_04; // 4 only seen 0 or 1.
    uint32_t unk_08[34]; // 8 seems to be 0...
    uint32_t bone_idx; // 0x90   indexes to name
    float unk_94[8]; // 0x94
    uint32_t unk_B4[3]; // 0xB4  zeroes. padding or maybe some of them are part of unk_94
    // size 0xC0
} PACKED COLEntry;

static_assert(sizeof(COLEntry) == 0xC0, "Incorrect structure size.");

#ifdef _MSC_VER
#pragma pack(pop)
#endif

class _2ryFile;
struct ScdEntry;

struct PtcEntry
{
    PtcEntry *parent;
    PtcEntry *child;

    float unk_10[7];

    void Decompile(TiXmlNode *root, const ScdEntry &scd) const;
    bool Compile(const TiXmlElement *root, ScdEntry &scd);

    bool operator==(const PtcEntry &rhs) const;
    inline bool operator!=(const PtcEntry &rhs) const
    {
        return !(*this == rhs);
    }

private:

    bool PartialCompare(const PtcEntry *ptc1, const PtcEntry *ptc2) const;
};

struct JntEntry
{
    PtcEntry *ptc1;
    PtcEntry *ptc2;

    uint32_t unk_08;
    float unk_0C[5];

    void Decompile(TiXmlNode *root, const ScdEntry &scd, uint32_t id) const;
    bool Compile(const TiXmlElement *root, ScdEntry &scd);

    bool operator==(const JntEntry &rhs) const;
    inline bool operator!=(const JntEntry &rhs) const
    {
        return !(*this == rhs);
    }
};

struct ColEntry
{
    std::string bone;

    uint32_t unk_04;
    float unk_94[8];

    void Decompile(TiXmlNode *root, uint32_t id) const;
    bool Compile(const TiXmlElement *root);

    inline bool operator==(const ColEntry &rhs) const
    {
        if (this->bone != rhs.bone)
            return false;

        if (this->unk_04 != rhs.unk_04)
            return false;

        if (memcmp(this->unk_94, rhs.unk_94, sizeof(this->unk_94)) != 0)
            return false;

        return true;
    }

    inline bool operator!=(const ColEntry &rhs) const
    {
        return !(*this == rhs);
    }
};

struct ScdEntry
{
    std::string name;
    std::vector<PtcEntry *> ptc_roots;
    std::vector<PtcEntry> ptcs;
    std::vector<JntEntry> jnts;
    std::vector<ColEntry> cols;
    std::vector<std::string> bones;

    float unk_08;
    float unk_0C;

    ScdEntry() { }

    ScdEntry(const ScdEntry &other)
    {
        Copy(other);
    }

    void Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);

    inline ScdEntry &operator=(const ScdEntry &other)
    {
        if (this == &other)
            return *this;

        Copy(other);
        return *this;
    }

    bool operator==(const ScdEntry &rhs) const;

    inline bool operator!=(const ScdEntry &rhs) const
    {
        return !(*this == rhs);
    }

private:

    void Copy(const ScdEntry &other);

    uint32_t PointerToIndex(const PtcEntry *ptc) const;
    uint32_t BoneNameToIndex(const std::string &name) const;

    friend class _2ryFile;
    friend struct PtcEntry;
    friend struct JntEntry;
};

class _2ryFile : public BaseFile
{
private:

    uint8_t unk_06[2];
    std::vector<ScdEntry> scds;

    void Reset();

    unsigned int CalculateFileSize() const;

public:

    _2ryFile();
    virtual ~_2ryFile();

    inline uint32_t GetNumScd() const { return scds.size(); }

    inline ScdEntry *GetScd(uint32_t index)
    {
        if (index >= scds.size())
            return nullptr;

        return &scds[index];
    }

    inline const ScdEntry *GetScd(uint32_t index) const
    {
        if (index >= scds.size())
            return nullptr;

        return &scds[index];
    }

    ScdEntry *GetScd(const std::string &name)
    {
        for (ScdEntry &scd : scds)
        {
            if (scd.name == name)
                return &scd;
        }

        return nullptr;
    }

    const ScdEntry *GetScd(const std::string &name) const
    {
        for (const ScdEntry &scd : scds)
        {
            if (scd.name == name)
                return &scd;
        }

        return nullptr;
    }

    inline bool ScdExists(uint32_t idx) const { return (GetScd(idx) != nullptr); }
    inline bool ScdExists(const std::string &name) const { return (GetScd(name) != nullptr); }


    inline uint32_t AppendScd(const ScdEntry &scd)
    {
        scds.push_back(scd);
        return (scds.size()-1);
    }

    inline uint32_t AppendScd(const _2ryFile &other, const std::string &name)
    {
        const ScdEntry *scd = other.GetScd(name);
        if (!scd)
            return (uint32_t)-1;

        return AppendScd(*scd);
    }

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    inline bool operator==(const _2ryFile &rhs) const
    {
        return (unk_06[0] == rhs.unk_06[0] &&
                unk_06[1] == rhs.unk_06[1] &&
                scds == rhs.scds);
    }

    inline bool operator!=(const _2ryFile &rhs) const
    {
        return !(*this == rhs);
    }

    inline ScdEntry &operator[](size_t n) { return scds[n]; }
    inline const ScdEntry &operator[](size_t n) const { return scds[n]; }

    inline ScdEntry &operator[](const std::string &name)
    {
        ScdEntry *scd = GetScd(name);
        if (!scd)
        {
            throw std::out_of_range("ScdEntry " + name + " doesn't exist.");
        }

        return *scd;
    }

    inline const ScdEntry &operator[](const std::string &name) const
    {
        const ScdEntry *scd = GetScd(name);
        if (!scd)
        {
            throw std::out_of_range("ScdEntry " + name + " doesn't exist.");
        }

        return *scd;
    }

    inline const _2ryFile operator+(const ScdEntry &scd) const
    {
        _2ryFile new_2ry = *this;

        new_2ry.AppendScd(scd);
        return new_2ry;
    }

    inline _2ryFile &operator+=(const ScdEntry &scd)
    {
        this->AppendScd(scd);
        return *this;
    }

    inline std::vector<ScdEntry>::iterator begin() { return scds.begin(); }
    inline std::vector<ScdEntry>::iterator end() { return scds.end(); }

    inline std::vector<ScdEntry>::const_iterator begin() const { return scds.begin(); }
    inline std::vector<ScdEntry>::const_iterator end() const { return scds.end(); }
};

#endif
