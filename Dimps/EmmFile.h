#ifndef __EMMFILE_H__
#define __EMMFILE_H__

#include <stdexcept>
#include "BaseFile.h"
#include "Utils.h"

// "#EMM"
#define EMM_SIGNATURE	0x4D4D4523

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t header_size; // 6    (? is it really header size?)
    uint32_t unk_08; // 8
    uint32_t material_base_address; // 0x0C
    // Size 0x10
} PACKED EMMHeader;

static_assert(sizeof(EMMHeader) == 0x10, "Incorrect structure size.");

typedef struct
{
    uint16_t mat_count; // 0
    uint16_t unk_02; // 2
    uint32_t offsets[1]; // 4
    // Remaining offsets
} PACKED EMMSubHeader;

static_assert(sizeof(EMMSubHeader) == 8, "Incorrect structure size.");

typedef struct
{
    char name[0x20]; // 0
    char shader_name[0x20]; // 0x20
    uint16_t param_count; // 0x40
    uint16_t unk_42; // 0x42 "always" 0
    // 0x44
} PACKED EMMMaterial;

static_assert(sizeof(EMMMaterial) == 0x44, "Incorrect structure size.");

typedef struct
{
    char name[0x20]; // 0
    uint32_t type; // 0x20
    uint32_t value; // 0x24
    // size 0x28
} PACKED EMMMaterialParameter;

static_assert(sizeof(EMMMaterialParameter) == 0x28, "Incorrect structure size.");

#ifdef _MSC_VER
#pragma pack(pop)
#endif

class EmmFile;
class EmmMaterial;

class EmmMaterialParameter
{
private:

    std::string name;
    uint32_t type;

    union
    {
        uint32_t value_u32;
        float value_float;
    } value;

    friend class EmmMaterial;
    friend class EmmFile;

public:

    inline std::string GetName() const { return name; }
    inline bool SetName(const std::string &name)
    {
        if (name.length() > 31)
            return false;

        this->name = name;
        return true;
    }

    inline uint32_t GetType() const { return type; }
    inline void SetType(uint32_t type) { this->type = type; }

    inline uint32_t GetValueU32() const { return value.value_u32; }
    inline void SetValueU32(uint32_t value) { this->value.value_u32 = value; }

    inline float GetValueFloat() const { return value.value_float; }
    inline void SetValueFloat(float value) { this->value.value_float = value; }

    void Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);

    inline bool operator==(const EmmMaterialParameter &rhs) const
    {
        if (strcasecmp(this->name.c_str(), rhs.name.c_str()) != 0)
            return false;

        return (this->type == rhs.type &&
                this->value.value_u32 == rhs.value.value_u32);
    }

    inline bool operator!=(const EmmMaterialParameter &rhs) const
    {
        return !(*this == rhs);
    }
};

class EmmMaterial
{
private:

    std::string name;
    std::string shader_name;

    std::vector<EmmMaterialParameter> parameters;

    friend class EmmFile;

public:

    inline size_t GetNumParameters() { return parameters.size(); }

    inline std::string GetName() const { return name; }
    inline bool SetName(const std::string &name)
    {
        if (name.length() > 31)
            return false;

        this->name = name;
        return true;
    }

    inline std::string GetShaderName() const { return shader_name; }
    inline bool SetShaderName(const std::string &shader_name)
    {
        if (shader_name.length() > 31)
            return false;

        this->shader_name = shader_name;
        return true;
    }

    void Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);

    inline bool operator==(const EmmMaterial &rhs) const
    {        
        if (strcasecmp(this->name.c_str(), rhs.name.c_str()) != 0)
            return false;

        return (this->shader_name == rhs.shader_name &&
                this->parameters == rhs.parameters);
    }

    inline bool operator!=(const EmmMaterial &rhs) const
    {
        return !(*this == rhs);
    }

    inline EmmMaterialParameter & operator[](size_t n) { return parameters[n]; }
    inline const EmmMaterialParameter & operator[](size_t n) const { return parameters[n]; }

    inline std::vector<EmmMaterialParameter>::iterator begin() { return parameters.begin(); }
    inline std::vector<EmmMaterialParameter>::iterator end() { return parameters.end(); }

    inline std::vector<EmmMaterialParameter>::const_iterator begin() const { return parameters.begin(); }
    inline std::vector<EmmMaterialParameter>::const_iterator end() const { return parameters.end(); }
};


class EmmFile : public BaseFile
{
private:

      std::vector<EmmMaterial> materials;

      void Reset();
      unsigned int CalculateFileSize();

public:

    EmmFile();
    EmmFile(uint8_t *buf, size_t size);
    virtual ~EmmFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    inline size_t GetNumMaterials() const { return materials.size(); }

    inline EmmMaterial *GetMaterial(const std::string &name)
    {
        for (EmmMaterial &m :materials)
        {
            if (strcasecmp(m.name.c_str(), name.c_str()) == 0)
                return &m;
        }

        return nullptr;
    }

    inline const EmmMaterial *GetMaterial(const std::string &name) const
    {
        for (const EmmMaterial &m :materials)
        {
            if (strcasecmp(m.name.c_str(), name.c_str()) == 0)
                return &m;
        }

        return nullptr;
    }

    inline bool MaterialExists(const std::string &name) const { return (GetMaterial(name) != nullptr); }

    uint16_t AppendMaterial(const EmmFile &other, uint16_t idx);
    inline uint16_t AppendMaterial(const EmmMaterial &material)
    {
        materials.push_back(material);
        return (materials.size()-1);
    }

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    inline bool operator==(const EmmFile &rhs) const
    {
        return (this->materials == rhs.materials);
    }

    inline bool operator!=(const EmmFile &rhs) const
    {
        return !(*this == rhs);
    }

    inline EmmMaterial &operator[](size_t n) { return materials[n]; }
    inline const EmmMaterial &operator[](size_t n) const { return materials[n]; }

    inline EmmMaterial &operator[](const std::string &name)
    {
        EmmMaterial *mat = GetMaterial(name);
        if (!mat)
        {
            throw std::out_of_range("Material " + name + " doesn't exist.");
        }

        return *mat;
    }

    inline const EmmMaterial &operator[](const std::string &name) const
    {
        const EmmMaterial *mat = GetMaterial(name);
        if (!mat)
        {
            throw std::out_of_range("Material " + name + " doesn't exist.");
        }

        return *mat;
    }

    inline const EmmFile operator+(const EmmMaterial &material) const
    {
        EmmFile new_emm = *this;

        new_emm.AppendMaterial(material);
        return new_emm;
    }

    inline EmmFile &operator+=(const EmmMaterial &material)
    {
        this->AppendMaterial(material);
        return *this;
    }

    inline std::vector<EmmMaterial>::iterator begin() { return materials.begin(); }
    inline std::vector<EmmMaterial>::iterator end() { return materials.end(); }

    inline std::vector<EmmMaterial>::const_iterator begin() const { return materials.begin(); }
    inline std::vector<EmmMaterial>::const_iterator end() const { return materials.end(); }
};

#endif // EMMFILE_H
