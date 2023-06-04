#ifndef BACFILE_H
#define BACFILE_H

#include "BaseFile.h"

/*Type 0 - Animation
Type 1 - hit box
Type 2 - Acceleration/Movement
Type 4 - something to do with motion adjusting
Type 5 - opponent knockback
Type 8 - effects
type 9 - Projectile/Wave/etc Launch
type 10 - camera
*/


// 00 -> start time
// 02 -> length

// Xenoverse 2 bac files only.
#define BAC_SIGNATURE   0x43414223
#define MAX_BAC_TYPE    30

#pragma pack(push,1)

struct PACKED BACHeader
{
    uint32_t signature; // 0
    uint16_t endianess_check; // 4
    uint16_t unk_06; // Always zero
    uint32_t num_entries; // 8
    uint32_t unk_0C; // Always zero
    uint32_t data_start; // 0x10
    uint32_t unk_14[3]; // Zero
    float unk_20[10];
    float unk_48[2];
    uint32_t unk_50[4]; // Zero
};
CHECK_STRUCT_SIZE(BACHeader, 0x60);

struct PACKED BACEntry
{
    uint32_t flags; // 0
    uint16_t num_subentries; // 4
    uint16_t unk_06; // Zero, probably padding
    uint32_t sub_entry_offset; // 8 aboslute offset
    uint32_t unk_0C; //Zero, probably padding
};
CHECK_STRUCT_SIZE(BACEntry, 0x10);

struct PACKED BACSubEntry
{
    uint16_t type; // 0
    uint16_t num; // 2
    uint32_t unk_04; //
    uint32_t offset; // 8, absolute offset
    uint32_t unk_0C; // 0xC
};
CHECK_STRUCT_SIZE(BACSubEntry, 0x10);

struct BACMatrix3x3
{
    TiXmlElement *Decompile(TiXmlNode *root, const std::string &comment) const;
    bool Compile(const TiXmlElement *root);

    float floats[9];
};
CHECK_STRUCT_SIZE(BACMatrix3x3, 0x24);

// Type 0
struct BACAnimation // Skill control and animation
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags;
    uint16_t ean_to_use;
    uint16_t ean_index;
    uint16_t unk_0C;
    uint16_t unk_0E;
    uint16_t frame_start; // 0x10
    uint16_t frame_end; // 0x12
    uint16_t frame_loop_start; // 0x14
    uint16_t unk_16;
    float speed;
    float transitory_animation_connection_type;
    float transitory_animation_compression;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACAnimation, 0x24);

// Type 1
struct BACHitbox
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags; // 6
    uint16_t bdm_entry; // 8
    uint16_t hitbox_flags; // A
    uint16_t damage; // C
    uint16_t damage_when_blocked; // E
    uint16_t stamina_taken_when_blocked; // 10
    uint8_t unk_12;
    uint8_t bdm_flags; // 13
    uint32_t unk_14[2];
    BACMatrix3x3 matrix;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACHitbox, 0x40);

// Type 2
struct BACAccelerationMovement
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags;
    uint16_t movement_flags;
    uint16_t unk_0A;
    float x_axis_movement; // 0xC
    float y_axis_movement; // 0x10
    float z_axis_movement; // 0x14
    float x_axis_drag; // 0x18
    float y_axis_drag; // 0x1C
    float z_axis_drag; // 0x20
    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACAccelerationMovement, 0x24);

// Type 3
struct BACInvulnerability
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags;
    uint32_t type;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACInvulnerability, 0xC);

// Type 4
struct BACMotionAdjust
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags;
    float time_scale;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACMotionAdjust, 0xC);

// Type 5
struct BACOpponentKnockback
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags;
    float tracking;
    uint16_t unk_0C;
    uint16_t unk_0E;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACOpponentKnockback, 0x10);

// Type 6
struct BACChainAttackParameters
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags;
    uint32_t unk_08;
    uint16_t total_chain_time;
    uint16_t unk_0E;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACChainAttackParameters, 0x10);

// Type 7
struct BACBcmCallback
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags;
    uint16_t bcm_link_flags; // 8
    uint16_t unk_0A;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACBcmCallback, 0xC);

// Type 8
struct BACEffect
{
    uint16_t start_time;
    uint16_t duration;
    uint32_t unk_04;
    uint16_t type; // 8
    uint16_t bone_link; // 0xA
    uint16_t skill_id; // 0xC
    uint16_t use_skill_id; // 0xE
    uint32_t effect_id; // 0x10
    uint32_t unk_14[6];
    uint32_t on_off_switch; // 0x2C

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACEffect, 0x30);

// Type 9
struct BACProjectile
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags;
    uint16_t skill_id; // 8
    uint16_t can_use_cmn_bsa;
    uint32_t projectile_id; // 0xC
    uint16_t bone_to_spawn_from; // 0x10
    uint16_t spawn_source;
    float position[3]; // 0x14
    uint32_t skill_type; // 0x20
    uint32_t unk_24;
    float unk_28;
    uint16_t unk_2C;
    uint16_t unk_2E;
    uint32_t projectile_health;
    uint32_t unk_34[3];

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACProjectile, 0x40);

// Typr 10
struct BACCamera
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags;
    uint16_t ean_to_use;
    uint16_t bone_to_focus_on;
    uint16_t ean_index;
    uint16_t camera_frame_start; // 0xE
    uint16_t unk_10;
    uint16_t unk_12;
    // Are next a 3x3 matrix?
    float z_axis_camera_position; // 0x14
    float camera_disp_xz; // 0x18
    float camera_disp_yz; // 0x1C
    float camera_y_rot; // 0x20
    float camera_x_rot; // 0x24
    float camera_x_pos; // 0x28
    float camera_y_pos; // 0x2C
    float camera_zoom; // 0x30
    float camera_z_rot; // 0x34
    uint32_t unk_38[4];
    uint16_t unk_48;
    uint16_t camera_flags; // 0x4A

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACCamera, 0x4C);

// Type 11
struct BACSound
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags;
    uint16_t acb_to_use;
    uint16_t unk_0A;
    uint16_t cue_id;
    uint16_t unk_0E;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACSound, 0x10);

struct BACType12
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags;
    uint16_t unk_08;
    uint16_t unk_0A;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACType12, 0xC);

// Type 13
struct BACPartInvisibility
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags;
    uint16_t bcs_part_id;
    uint16_t on_off_switch;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACPartInvisibility, 0xC);

// Typr 14
struct BACAnimationModification
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags;
    uint16_t modification;
    uint16_t unk_0A;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACAnimationModification, 0xC);

// Type 15
struct BACTransformControl
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags;
    uint16_t type;
    uint16_t unk_0A;
    float parameter;
    float unk_10[2];
    uint32_t unk_18[2];

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACTransformControl, 0x20);

// Type 16
struct BACScreenEffect
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags;
    uint32_t bpe_effect_id;
    uint16_t unk_0C;
    uint16_t unk_0E;
    uint32_t unk_10[4];

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACScreenEffect, 0x20);

// Type 17
struct BACThrowHandler
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags;
    uint16_t th_flags; // 8
    uint16_t unk_0A;
    uint16_t bone_user_connects_to_victim_from; // 0xC
    uint16_t bone_victim_connects_to_user_from; // 0xE
    uint16_t bac_entry; // 0x10
    uint16_t unk_12;
    float victim_displacement[3]; // 0x14

    TiXmlElement *Decompile(TiXmlNode *root, bool _small) const;
    bool Compile(const TiXmlElement *root, bool _small);
};
CHECK_STRUCT_SIZE(BACThrowHandler, 0x20);

struct BACType18
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags;
    uint32_t unk_08[3];
    float unk_14[2];
    uint32_t unk_1C;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACType18, 0x20);

// Type 19
struct BACAuraEffect
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags;
    uint16_t type;
    uint16_t on_off_switch;
    uint32_t unk_0C;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACAuraEffect, 0x10);

// Type 20
struct BACHomingMovement
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags;
    uint16_t type;
    uint16_t horizontal_homing_arc_direction;
    uint32_t speed_modifier;
    uint16_t unk_10;
    uint16_t unk_12;
    float horizontal_direction_modifier;
    float vertical_direction_modifier;
    float z_direction_modifier;
    uint32_t unk_20[4];

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACHomingMovement, 0x30);

struct BACType21
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags;
    uint32_t unk_08;
    uint16_t unk_0C;
    uint16_t unk_0E;
    uint16_t unk_10;
    uint16_t unk_12;
    uint16_t unk_14;
    uint16_t unk_16;
    float unk_18;
    float unk_1C;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACType21, 0x20);

struct BACType22
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags;
    uint16_t unk_08;
    uint16_t unk_0A;
    float unk_0C;
    char name[32];

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACType22, 0x30);

// Type 23
struct BACTransparencyEffect
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags;
    uint16_t transparency_flags;
    uint16_t transparency_flags2;
    uint16_t dilution; // 0xC
    uint16_t unk_0E;
    uint32_t unk_10;
    float rgb[3]; // 0x14
    float unk_20[8];

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACTransparencyEffect, 0x40);

// Type 24
struct BACDualSkillData
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags;
    uint16_t unk_08;
    uint16_t unk_0A;
    uint16_t unk_0C;
    uint16_t unk_0E;
    uint32_t unk_10;
    float unk_14;
    float unk_18;
    float unk_1C;
    uint16_t unk_20;
    uint16_t unk_22;
    uint32_t unk_24;
    float unk_28;
    float unk_2C;
    float unk_30;
    uint16_t unk_34;
    uint16_t unk_36;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACDualSkillData, 0x38);

// Type 25
struct BACType25
{
    uint16_t start_time;
    uint16_t duration;
    uint16_t unk_04;
    uint16_t flags;
    uint32_t unk_08;
    uint32_t unk_0C;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACType25, 0x10);

// Type 26
struct BACType26
{
    uint16_t start_time;
    uint16_t duration;
    uint32_t unk_04;
    uint32_t unk_08;
    uint32_t unk_0C;
    float unk_10;
    uint32_t unk_14[15];

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACType26, 0x50);

// Type 27
struct BACType27
{
    uint16_t start_time;
    uint16_t duration;
    uint32_t flags;
    uint16_t skill_id; // 8
    uint16_t unk_0A;
    uint16_t unk_0C;
    uint16_t unk_0E;
    uint32_t unk_10;
    uint32_t unk_14;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACType27, 0x18);

// Type 28
struct BACType28
{
    uint16_t start_time;
    uint16_t duration;
    uint32_t unk_04;
    uint16_t unk_08;
    uint16_t unk_0A;
    uint32_t unk_0C;
    float unk_10;
    float unk_14;
    uint32_t unk_18[3];

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACType28, 0x24);

// Type 29
struct BACType29
{
    uint16_t start_time;
    uint16_t duration;
    uint32_t unk_04;
    uint16_t unk_08;
    uint16_t unk_0A;
    uint32_t unk_0C;
    float unk_10[8];
    uint32_t unk_30[3];

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACType29, 0x3C);


// Type 30
struct BACType30
{
    uint16_t start_time;
    uint16_t duration;
    uint32_t unk_04;
    float unk_08;
    uint32_t unk_0C[9];

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};
CHECK_STRUCT_SIZE(BACType30, 0x30);

#pragma pack(pop)

struct BacEntry
{
    bool valid;
    uint32_t flags; // Flags is used even if not valid
    bool has_type[MAX_BAC_TYPE+1];

    std::vector<BACAnimation> type0;
    std::vector<BACHitbox> type1;
    std::vector<BACAccelerationMovement> type2;
    std::vector<BACInvulnerability> type3;
    std::vector<BACMotionAdjust> type4;
    std::vector<BACOpponentKnockback> type5;
    std::vector<BACChainAttackParameters> type6;
    std::vector<BACBcmCallback> type7;
    std::vector<BACEffect> type8;
    std::vector<BACProjectile> type9;
    std::vector<BACCamera> type10;
    std::vector<BACSound> type11;
    std::vector<BACType12> type12;
    std::vector<BACPartInvisibility> type13;
    std::vector<BACAnimationModification> type14;
    std::vector<BACTransformControl> type15;
    std::vector<BACScreenEffect> type16;
    std::vector<BACThrowHandler> type17;
    std::vector<BACType18> type18;
    std::vector<BACAuraEffect> type19;
    std::vector<BACHomingMovement> type20;
    std::vector<BACType21> type21;
    std::vector<BACType22> type22;
    std::vector<BACTransparencyEffect> type23;
    std::vector<BACDualSkillData> type24;
    std::vector<BACType25> type25;
    std::vector<BACType26> type26;
    std::vector<BACType27> type27;
    std::vector<BACType28> type28;
    std::vector<BACType29> type29;
    std::vector<BACType30> type30;

    BacEntry() : valid(false) { memset(has_type, 0, sizeof(has_type)); }

    uint16_t GetNumSubEntries() const
    {
        uint16_t count = 0;

        for (int i = 0; i <= MAX_BAC_TYPE; i++)
            if (has_type[i])
                count++;

        return count;
    }

    TiXmlElement *Decompile(TiXmlNode *root, bool small_17, int idx) const;
    bool Compile(const TiXmlElement *root, bool small_17);
};

class BacFile : public BaseFile
{
private:

    uint32_t unk_14[3];
    float unk_20[10];
    float unk_48[2];
    uint32_t unk_50[4];

    std::vector<BacEntry> entries;
    bool size17_small;

protected:

    void Reset();

    size_t CalculateFileSize() const;
    size_t GetDataStart() const;

public:

    BacFile();
    virtual ~BacFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

    size_t ChangeReferencesToSkill(uint16_t old_skill, uint16_t new_skill);

    inline size_t GetNumEntries() const { return entries.size(); }

    inline const std::vector<BacEntry> &GetEntries() const { return entries; }
    inline std::vector<BacEntry> &GetEntries() { return entries; }

    inline const BacEntry &operator[](size_t n) const { return entries[n]; }
    inline BacEntry &operator[](size_t n) { return entries[n]; }

    inline std::vector<BacEntry>::const_iterator begin() const { return entries.begin(); }
    inline std::vector<BacEntry>::const_iterator end() const { return entries.end(); }

    inline std::vector<BacEntry>::iterator begin() { return entries.begin(); }
    inline std::vector<BacEntry>::iterator end() { return entries.end(); }
};

#endif // BACFILE_H
