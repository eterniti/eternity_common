#ifndef __EMAFILE_H__
#define __EMAFILE_H__

#include <vector>
#include "SkeletonFile.h"

// "#EMA"
#define EMA_SIGNATURE	0x414D4523

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t header_size; // 6
    uint16_t unk_08; // 8  non-zero
    uint16_t unk_0A; // A, zero
    uint32_t skeleton_offset; // 0xC
    uint16_t anim_count; // 0x10
    uint16_t unk_12; // 0x12
    uint32_t unk_14[3]; // 0x14 probably just padding
    // size 0x20
} PACKED EMAHeader;

static_assert(sizeof(EMAHeader) == 0x20, "Incorrect structure size.");

typedef struct
{
    uint16_t duration; // 0
    uint16_t cmd_count; // 2
    uint32_t value_count; // 4
    uint32_t unk_08; // 8
    uint32_t name_offset; // 0xC
    uint32_t values_offset; // 0x10
    uint32_t cmd_offsets[1]; // 0x14
    // remaining cmd_offsets
} PACKED EMAAnimationHeader;

static_assert(sizeof(EMAAnimationHeader) == 0x18, "Incorrect structure size.");

typedef struct
{
    uint16_t bone_idx; // 0
    uint8_t transform; // 2 // 0 -> translation, 1 -> rotation, 2 -> scale
    uint8_t flags; // 3 //4bits : ? / 2bits: transform component ( a record has 0, the next 1, then 2 )
    uint16_t step_count; // 4
    uint16_t indices_offset; // 6
} PACKED EMAAnimationCommandHeader;

static_assert(sizeof(EMAAnimationCommandHeader) == 8, "Incorrect structure size.");

#ifdef _MSC_VER
#pragma pack(pop)
#endif

class EmaFile;
class EmaAnimation;

struct EmaStep
{
    uint16_t time;
    uint32_t index;

    void Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);

    inline bool operator==(const EmaStep &rhs) const
    {
        return (this->time == rhs.time && this->index == rhs.index);
    }

    inline bool operator!=(const EmaStep &rhs) const
    {
        return !(*this == rhs);
    }
};

class EmaCommand
{
private:

    Bone *bone;
    uint8_t transform;
    uint8_t flags;

    std::vector<EmaStep> steps;

    friend class EmaFile;
    friend class EmaAnimation;

public:


    inline Bone *GetBone()
    {
        return bone;
    }

    inline uint16_t GetNumSteps() const
    {
        return steps.size();
    }

    inline bool RemoveStep(uint16_t id)
    {
        if (id >= steps.size())
            return false;

        steps.erase(steps.begin()+id);
        return true;
    }

    void Decompile(TiXmlNode *root, uint32_t id) const;
    bool Compile(const TiXmlElement *root, SkeletonFile &skl);

    inline bool operator==(const EmaCommand &rhs) const
    {
        if (!this->bone)
        {
            if (rhs.bone)
                return false;
        }
        else if (!rhs.bone)
        {
            return false;
        }

        if (this->bone)
        {
            if (this->bone->GetName() != rhs.bone->GetName())
                return false;
        }

        return (this->transform == rhs.transform &&
                this->flags == rhs.flags &&
                this->steps == rhs.steps);
    }

    inline bool operator!=(const EmaCommand &rhs) const
    {
        return !(*this == rhs);
    }


    inline EmaStep &operator[](size_t n) { return steps[n]; }
    inline const EmaStep &operator[](size_t n) const { return steps[n]; }

    inline std::vector<EmaStep>::iterator begin() { return steps.begin(); }
    inline std::vector<EmaStep>::iterator end() { return steps.end(); }

    inline std::vector<EmaStep>::const_iterator begin() const { return steps.begin(); }
    inline std::vector<EmaStep>::const_iterator end() const { return steps.end(); }
};

class EmaAnimation
{
private:

    std::string name;
    uint16_t duration;

    std::vector<float> values;
    std::vector<EmaCommand> commands;

    uint32_t unk_08;

    friend class EmaFile;

public:

    inline std::string GetName() const
    {
        return name;
    }

    inline void SetName(const std::string &name)
    {
        this->name = name;
    }

    inline uint16_t GetDuration() const
    {
        return duration;
    }

    inline void SetDuration(uint16_t duration)
    {
        this->duration = duration;
    }

    inline uint16_t GetNumCommands() const
    {
        return commands.size();
    }


    inline bool RemoveCommand(uint16_t id)
    {
        if (id >= commands.size())
            return false;

        commands.erase(commands.begin()+id);
        return true;
    }

    void Decompile(TiXmlNode *root, uint32_t id) const;
    bool Compile(const TiXmlElement *root, SkeletonFile &skl);

    inline bool operator==(const EmaAnimation &rhs) const
    {
        return (this->name == rhs.name &&
                this->duration == rhs.duration &&
                this->values == rhs.values &&
                this->commands == rhs.commands);
    }

    inline bool operator!=(const EmaAnimation &rhs) const
    {
        return !(*this == rhs);
    }

    inline EmaCommand &operator[](size_t n) { return commands[n]; }
    inline const EmaCommand &operator[](size_t n) const { return commands[n]; }

    inline std::vector<EmaCommand>::iterator begin() { return commands.begin(); }
    inline std::vector<EmaCommand>::iterator end() { return commands.end(); }

    inline std::vector<EmaCommand>::const_iterator begin() const { return commands.begin(); }
    inline std::vector<EmaCommand>::const_iterator end() const { return commands.end(); }
};

class EmaFile : public SkeletonFile
{
private:

    std::vector<EmaAnimation> animations;
    uint16_t unk_08;
    uint16_t unk_12;

    void Copy(const EmaFile &other);

    void Reset();
    unsigned int CalculateFileSize() const;

protected:

    virtual void RebuildSkeleton(const std::vector<Bone *> &old_bones_ptr) override;

public:

    EmaFile();
    EmaFile(const EmoFile &other) : SkeletonFile()
    {
        Copy(other);
    }

    virtual ~EmaFile();

    inline bool HasSkeleton() const
    {
        return (bones.size() > 0);
    }

    inline uint16_t GetNumAnimations() const { return animations.size(); }

    inline EmaAnimation *GetAnimation(uint16_t idx)
    {
        if (idx >= animations.size())
            return nullptr;

        return &animations[idx];
    }

    inline const EmaAnimation *GetAnimation(uint16_t idx) const
    {
        if (idx >= animations.size())
            return nullptr;

        return &animations[idx];
    }

    inline EmaAnimation *GetAnimation(const std::string &name)
    {
        for (EmaAnimation &a : animations)
        {
            if (a.name == name)
                return &a;
        }

        return nullptr;
    }

    inline const EmaAnimation *GetAnimation(const std::string &name) const
    {
        for (const EmaAnimation &a : animations)
        {
            if (a.name == name)
                return &a;
        }

        return nullptr;
    }

    inline bool AnimationExists(uint16_t idx) const
    {
        return (GetAnimation(idx) != nullptr);
    }

    inline bool AnimationExists(const std::string &name) const
    {
        return (GetAnimation(name) != nullptr);
    }

    inline uint16_t FindAnimation(const std::string &name) const
    {
        for (size_t i = 0; i < animations.size(); i++)
        {
            if (animations[i].name == name)
                return i;
        }

        return 0xFFFF;
    }

    inline bool RemoveAnimation(uint16_t idx)
    {
        if (idx >= animations.size())
            return false;

        animations.erase(animations.begin()+idx);
    }

    inline bool RemoveAnimation(const std::string &name)
    {
        for (auto it = animations.begin(); it != animations.end(); ++it)
        {
            if ((*it).name == name)
            {
                animations.erase(it);
                return true;
            }
        }

        return false;
    }

    bool LinkAnimation(EmaAnimation &anim, Bone **not_found=nullptr);

    uint16_t AppendAnimation(const EmaAnimation &animation);
    uint16_t AppendAnimation(const EmaFile &other, const std::string &name);

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    virtual bool DecompileToFile(const std::string &path, bool show_error=true, bool build_path=false) override;
    virtual bool CompileFromFile(const std::string &path, bool show_error=true, bool big_endian=false) override;

    inline EmaFile &operator=(const EmaFile &other)
    {
        if (this == &other)
            return *this;

        Copy(other);
        return *this;
    }

    bool operator==(const EmaFile &rhs) const;

    inline bool operator!=(const EmaFile &rhs) const
    {
        return !(*this == rhs);
    }

    inline EmaAnimation &operator[](size_t n) { return animations[n]; }
    inline const EmaAnimation &operator[](size_t n) const { return animations[n]; }

    inline EmaAnimation &operator[](const std::string &name)
    {
        EmaAnimation *a = GetAnimation(name);
        if (!a)
        {
            throw std::out_of_range("Animation " + name + " doesn't exist.");
        }

        return *a;
    }

    inline const EmaAnimation &operator[](const std::string &name) const
    {
        const EmaAnimation *a = GetAnimation(name);
        if (!a)
        {
            throw std::out_of_range("Animation " + name + " doesn't exist.");
        }

        return *a;
    }

    inline const EmaFile operator+(const EmaAnimation &animation) const
    {
        EmaFile new_ema = *this;

        new_ema.AppendAnimation(animation);
        return new_ema;
    }

    inline EmaFile &operator+=(const EmaAnimation &animation)
    {
        this->AppendAnimation(animation);
        return *this;
    }

    inline std::vector<EmaAnimation>::iterator begin() { return animations.begin(); }
    inline std::vector<EmaAnimation>::iterator end() { return animations.end(); }

    inline std::vector<EmaAnimation>::const_iterator begin() const { return animations.begin(); }
    inline std::vector<EmaAnimation>::const_iterator end() const { return animations.end(); }
};

#endif
