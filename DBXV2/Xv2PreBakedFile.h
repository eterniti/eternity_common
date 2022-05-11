#ifndef XV2PREBAKEDFILE_H
#define XV2PREBAKEDFILE_H

#include <unordered_map>
#include "BaseFile.h"

struct BodyShape
{
    uint32_t cms_entry;
    uint32_t body_shape;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct CusAuraData
{
    uint16_t cus_aura_id;
    uint16_t aur_aura_id;

    uint8_t behaviour_11;
    uint32_t integer_2;
    uint8_t behaviour_10;
    uint32_t integer_3;

    bool force_teleport;
    uint8_t behaviour_13;

    uint8_t behaviour_66 = 0xFF;
    uint8_t remove_hair_accessories; // Troolean, 0xFF = default game behaviour, 0 = don't remove, 1 = remove

    uint32_t bcs_hair_color;
    uint32_t bcs_eyes_color;

    CusAuraData()
    {
        cus_aura_id = 0xFFFF;
        aur_aura_id = 0;
        behaviour_11 = 0;
        integer_2 = 0;
        behaviour_10 = 0;
        integer_3 = 0;
        force_teleport = false;
        behaviour_13 = 0;
        behaviour_66 = 0xFF;        
        remove_hair_accessories = 0xFF;
        bcs_hair_color = bcs_eyes_color = 0xFFFFFFFF;
    }

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct PreBakedAlias
{
    // Fields for "this" char
    uint32_t cms_id;
    std::string cms_name;

    // Fields for the char we are "impersonating".
    std::string ttc_files; // Will load the ttc files of this char (audio, msg subs, and msg voice)

    PreBakedAlias()
    {
        cms_id = 0xFFFFFFFF;
    }

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct BcsColorsMap
{
    std::unordered_map<std::string, uint32_t> map;

    TiXmlElement *Decompile(TiXmlNode *root, uint32_t cms_id, uint32_t costume) const;
    bool Compile(const TiXmlElement *root, uint32_t *cms_id, uint32_t *costume);
};


class Xv2PreBakedFile : public BaseFile
{
private:

    std::vector<std::string> ozarus;
    std::vector<uint32_t> auto_btl_portrait_list;
    std::vector<BodyShape> body_shapes;
    std::vector<CusAuraData> cus_aura_datas;
    std::vector<PreBakedAlias> aliases;
    std::vector<uint32_t> any_dual_skill_list;
    std::unordered_map<uint32_t, BcsColorsMap> colors_map;

protected:

    void Reset();

public:

    Xv2PreBakedFile();
    virtual ~Xv2PreBakedFile();

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    inline const std::vector<std::string> &GetOzarus() const { return ozarus; }
    inline std::vector<std::string> &GetOzarus() { return ozarus; }

    inline const std::vector<uint32_t> &GetAutoBtlPortraitList() const { return auto_btl_portrait_list; }
    inline std::vector<uint32_t> &GetAutoBtlPortraitList() { return auto_btl_portrait_list; }

    inline const std::vector<BodyShape> &GetBodyShapes() const { return body_shapes; }
    inline std::vector<BodyShape> &GetBodyShapes() { return body_shapes; }

    inline const std::vector<CusAuraData> &GetCusAuraDatas() const { return cus_aura_datas; }
    inline std::vector<CusAuraData> &GetCusAuraDatas() { return cus_aura_datas; }

    inline const std::vector<PreBakedAlias> &GetAliases() const { return aliases; }
    inline std::vector<PreBakedAlias> &GetAliases() { return aliases; }

    inline const std::vector<uint32_t> &GetAnyDualSkillList() const { return any_dual_skill_list; }
    inline std::vector<uint32_t> &GetAnyDualSkillList() { return any_dual_skill_list; }

    inline const std::unordered_map<uint32_t, BcsColorsMap> &GetColorsMap() const { return colors_map; }
    inline std::unordered_map<uint32_t, BcsColorsMap> &GetColorsMap() { return colors_map; }

    void AddOzaru(const std::string &ozaru);
    void RemoveOzaru(const std::string &ozaru);

    void AddAutoBtlPortrait(uint32_t cms_entry);
    void RemoveAutoBtlPortrait(uint32_t cms_entry);

    void AddBodyShape(uint32_t cms_entry, uint32_t body_shape);
    void RemoveBodyShape(uint32_t cms_entry);

    void AddCharToAnyDualSkillList(uint32_t cms_entry);
    void RemoveCharFromAnyDualSkillList(uint32_t cms_entry);

    CusAuraData *FindAuraData(uint16_t cus_aura_id);
    CusAuraData *FindAuraDataByAurId(uint16_t aur_aura_id);

    bool AddConsecutiveAuraData(std::vector<CusAuraData> &datas);
    void RemoveAuraData(uint16_t cus_aura_id);

    PreBakedAlias *FindAlias(uint32_t cms_id);
    PreBakedAlias *FindAlias(const std::string &cms_name);

    void AddAlias(const PreBakedAlias &alias);
    size_t RemoveAlias(uint32_t cms_id);
    size_t RemoveAlias(const std::string &cms_name);

    inline void AddColorsMap(uint32_t cms_id, uint32_t costume, const BcsColorsMap &map) { colors_map[(cms_id << 16) | (costume&0xFFFF)] = map; }
    inline void RemoveColorsMap(uint32_t cms_id)
    {
        for (uint16_t costume = 0; costume < 100; costume++)
        {
            colors_map.erase((cms_id << 16) | costume);
        }
    }
};

#endif // XV2PREBAKEDFILE_H
