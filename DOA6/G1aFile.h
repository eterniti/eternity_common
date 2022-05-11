#ifndef G1AFILE_H
#define G1AFILE_H

#include <map>

#include "G1mFile.h"
#include "FixedMemoryStream.h"

//#define FBX_SUPPORT
#define DOA5_IMPORT_SUPPORT

#ifdef FBX_SUPPORT
#ifdef _MSC_VER
#pragma warning(disable : 4800)
#endif

#include <fbxsdk.h>
#endif

#ifdef DOA5_IMPORT_SUPPORT
#include "DOA5LR/CsvFile.h"
#endif

#define G2A_SIGNATURE   0x4732415F

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct PACKED G1AHeader
{
    uint32_t signature;
    char version[4]; // 0x4
    uint32_t file_size; // 0x8
    float frame_rate; // 0xC
    uint16_t num_frames;  // 0x10 Length in seconds can be obtained with num_frames/frame_rate
    uint16_t num_bones; // 0x12 Has something else in bits 0-3
    uint32_t keyframes_size; // 0x14
    uint32_t num_packed_vectors; // 0x18
    uint32_t unk_1C; //  Only for format G2A 0500
};
CHECK_STRUCT_SIZE(G1AHeader, 0x20);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct G1AKeyframe
{
    uint16_t time_start;
    uint64_t u, v, w, t;

    G1AKeyframe()
    {
        time_start = 0;
        u = v = w = t = 0;
    }

    G1AKeyframe(uint16_t time_start) : time_start(time_start) { u = v = w = t = 0; }

    static void UnpackVector(uint64_t v, float *fv);
    static void UnpackVector(uint64_t v, float &x, float &y, float &z);
    void UnpackVectors(float *fu, float *fv, float *fw, float *ft) const;

    static uint64_t PackVector(float *fv);
    static uint64_t PackVector(float x, float y, float z);
    void PackVectors(float *fu, float *fv, float *fw, float *ft);

    void GetXYZForFrame(float *xyz, uint16_t frame, uint16_t duration) const;
};

struct G1AAnimation
{
    uint16_t bone_id;

    std::vector<G1AKeyframe> rotation_kfs;
    std::vector<G1AKeyframe> translation_kfs;
    std::vector<G1AKeyframe> scale_kfs;

    G1AAnimation() : bone_id(0) { }
    G1AAnimation(uint16_t bone_id) : bone_id(bone_id) { }

    void AddFirstTransformLinear(int op, uint16_t duration, float ox, float oy, float oz, float tx, float ty, float tz, uint16_t start_time=0);

    inline void AddFirstRotationLinear(uint16_t duration, float ox, float oy, float oz, float rx, float ry, float rz, uint16_t start_time=0)
    {
        AddFirstTransformLinear(0, duration, ox, oy, oz, rx, ry, rz, start_time);
    }

    inline void AddFirstTranslationLinear(uint16_t duration, float ox, float oy, float oz, float tx, float ty, float tz, uint16_t start_time=0)
    {
        AddFirstTransformLinear(1, duration, ox, oy, oz, tx, ty, tz, start_time);
    }

    inline void AddFirstScaleLinear(uint16_t duration, float ox, float oy, float oz, float sx, float sy, float sz, uint16_t start_time=0)
    {
        AddFirstTransformLinear(2, duration, ox, oy, oz, sx, sy, sz, start_time);
    }

    void AddTransformLinear(int op, uint16_t duration, float tx, float ty, float tz, uint16_t skip_time=0);
    inline void AddRotationLinear(uint16_t duration, float rx, float ry, float rz, uint16_t skip_time=0) { AddTransformLinear(0, duration, rx, ry, rz, skip_time); }
    inline void AddTranslationLinear(uint16_t duration, float tx, float ty, float tz, uint16_t skip_time=0) { AddTransformLinear(1, duration, tx, ty, tz, skip_time); }
    inline void AddScaleLinear(uint16_t duration, float sx, float sy, float sz, uint16_t skip_time=0) { AddTransformLinear(2, duration, sx, sy, sz, skip_time); }
};

class G1aFile : public BaseFile
{
private:

    uint32_t version;
    float frame_rate;
    uint16_t num_frames;

    std::vector<G1AAnimation> animations;

    bool LoadInternal(Stream *stream);

#ifdef FBX_SUPPORT
    void GetFbxAnimatedBones(G1mFile &g1m, FbxNode *node, FbxAnimLayer *layer, std::map<uint16_t, std::string> &bones);
#endif

#ifdef DOA5_IMPORT_SUPPORT
    static uint16_t Doa5ToDoa6Bone(const std::string &doa5_name);
#endif

protected:

   void Reset();

public:
    G1aFile();
    virtual ~G1aFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    inline float GetFrameRate() const { return frame_rate; }
    inline uint16_t GetNumFrames() const { return num_frames; }

    inline float GetDurationSeconds() const
    {
        if (frame_rate == 0.0f)
            return 0.0f;

        return (float)num_frames / frame_rate;
    }

    inline void SetFrameRate(float fps) { frame_rate = fps; }
    inline void SetNumFrames(uint16_t num) { num_frames = num; }

    inline void AddAnimation(const G1AAnimation &a) { animations.push_back(a); }

    void DebugTest() const;
    void DebugCreateSampleAnimation();    

#ifdef FBX_SUPPORT

    bool ExportToFbx(FbxScene *scene, G1mFile &g1m, bool linear, bool correct_singularities) const;
    bool ImportFromFbx(FbxScene *scene, G1mFile &g1m);

#endif

#ifdef DOA5_IMPORT_SUPPORT

    bool ImportFromCsv(const CsvFile &csv, float frame_rate);

#endif

};

#endif // G1AFILE_H
