#ifndef __AURFILE_H__
#define __AURFILE_H__

#include "BaseFile.h"

#define AUR_SIGNATURE   0x52554123

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t header_size; // 6
    uint32_t auras_count; // 8
    uint32_t auras_offset; // 0xC
    uint32_t types_count; // 0x10
    uint32_t types_offset; // 0x14
    uint32_t chara_links_count; // 0x18
    uint32_t chara_links_offset; // 0x1C
} PACKED AURHeader;

STATIC_ASSERT_STRUCT(AURHeader, 0x20);

typedef struct
{
    uint32_t id; // 0
    uint32_t unk_04; // Always zero
    uint32_t effects_count; // 8
    uint32_t effects_address; // 0xC
} PACKED AURAura;

STATIC_ASSERT_STRUCT(AURAura, 0x10);

typedef struct
{
    uint32_t type_index;
    uint32_t id;
} PACKED AUREffect;

STATIC_ASSERT_STRUCT(AUREffect, 8);

typedef struct
{
    uint32_t char_id; // 0
    uint32_t costume_id; // 4
    uint32_t aura_id; // 8
    uint32_t glare; // 0xC
} PACKED AURCharaLink;

STATIC_ASSERT_STRUCT(AURCharaLink, 0x10);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct AurEffect
{
    uint32_t type_index;
    uint32_t id;

    AurEffect()
    {
        type_index = 0;
        id = 0xFFFFFFFF;
    }

    TiXmlElement *Decompile(TiXmlNode *root, const std::vector<std::string> &types) const;
    bool Compile(const TiXmlElement *root, const std::vector<std::string> &types);

    inline bool operator==(const AurEffect &rhs) const
    {
        return (this->type_index == rhs.type_index && this->id == rhs.id);
    }

    inline bool operator!=(const AurEffect &rhs) const
    {
        return !(*this == rhs);
    }
};

struct AurAura
{
    uint32_t id;
    uint32_t unk_04;
    std::vector<AurEffect> effects;

    AurAura()
    {
        id = 0;
        unk_04 = 0;
    }

    TiXmlElement *Decompile(TiXmlNode *root, const std::vector<std::string> &types) const;
    bool Compile(const TiXmlElement *root, const std::vector<std::string> &types);

    inline bool operator==(const AurAura &rhs) const
    {
        return (this->id == rhs.id && this->unk_04 == rhs.unk_04 && this->effects == rhs.effects);
    }

    inline bool operator!=(const AurAura &rhs) const
    {
        return !(*this == rhs);
    }
};

struct AurCharaLink
{
    uint32_t char_id;
    uint32_t costume_id;
    uint32_t aura_id;
    bool glare;

    AurCharaLink()
    {
        char_id = 0;
        costume_id = 0;
        aura_id = 0;
        glare = false;
    }

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);

    inline bool operator==(const AurCharaLink &rhs) const
    {
        return (this->char_id == rhs.char_id && this->costume_id == rhs.costume_id && this->aura_id == rhs.aura_id && this->glare == rhs.glare);
    }

    inline bool operator!=(const AurCharaLink &rhs) const
    {
        return !(*this == rhs);
    }
};

class AurFile : public BaseFile
{
private:

    std::vector<AurAura> auras;
    std::vector<std::string> types;
    std::vector<AurCharaLink> chara_links;

protected:

    void Reset();
    size_t CalculateFileSize();

public:

    AurFile();
    virtual ~AurFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    inline const std::vector<AurCharaLink> &GetCharaLinks() const { return chara_links; }
    inline std::vector<AurCharaLink> &GetCharaLinks() { return chara_links; }

    inline const std::vector<AurAura> &GetAuras() const { return auras; }
    inline std::vector<AurAura> &GetAuras() { return auras; }

    size_t FindCharaLinks(uint32_t char_id, std::vector<AurCharaLink *> &links);
    bool AddCharaLink(const AurCharaLink &link, bool unique_char_id);
    size_t RemoveCharaLinks(uint32_t char_id);

    AurAura *FindAuraByID(uint32_t id);
    bool AddAura(AurAura &aura);
    //void RemoveAura(uint32_t id);
    void RemoveAuraIfLast(uint32_t id);

    inline bool operator==(const AurFile &rhs) const
    {
        return (this->auras == rhs.auras && this->types == rhs.types && this->chara_links == rhs.chara_links);
    }

    inline bool operator!=(const AurFile &rhs) const
    {
        return !(*this == rhs);
    }
};

#endif // __AURFILE_H__
