#include "G1aFile.h"

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#include "debug.h"

G1aFile::G1aFile()
{
    this->big_endian = false;
}

G1aFile::~G1aFile()
{

}

void G1aFile::Reset()
{
    version = 50;

    frame_rate = 60.0f;
    num_frames = 0;

    animations.clear();
}

bool G1aFile::LoadInternal(Stream *stream)
{
    G1AHeader hdr;

    if (!stream->Read(&hdr, sizeof(hdr)))
        return false;

    if (hdr.signature != G2A_SIGNATURE)
    {
        DPRINTF("Unrecognized g1a signature 0x%08x.\n", hdr.signature);
        return false;
    }

    if (memcmp(hdr.version, "0300", 4) == 0)
    {
        version = 30;
        stream->Seek(sizeof(hdr)-4, SEEK_SET);
    }
    else if (memcmp(hdr.version, "0500", 4) == 0)
    {
        version = 50;
    }
    else
    {
        DPRINTF("This g1a file has an unsupported version.\n");
        return false;
    }

    frame_rate = hdr.frame_rate;
    num_frames = hdr.num_frames;

    if (version == 50 && hdr.unk_1C != 0)
    {
        DPRINTF("Warning: value unk_1C different than 0 (0x%08x)\n", hdr.unk_1C);
    }

    if (hdr.num_bones & 0xF)
    {
        DPRINTF("Warning: This value has something else at bits 0-3 (0x%08x)\n", hdr.num_bones);
    }

    animations.resize(hdr.num_bones >> 4);

    off64_t kf_start = stream->Tell() + (animations.size()*4);
    off64_t packed_vectors_start = kf_start + hdr.keyframes_size;

    for (G1AAnimation &a: animations)
    {
        uint32_t bitfield;

        if (!stream->Read32(&bitfield))
            return false;

        // version 30&40: bits 0-3: num of channels, bits 4-13: bone ID, bits 14-15: unused?, bits 15-31 is offset / 4
        // version 50: bits 0-3: num of channels, bits 4-13 is bone ID. bits 14-31: offset / 4
        if (version < 50)
        {
            // Do the same conversion the game does, and then just parse version 5 format
            uint16_t lw = bitfield&0xFFFF;
            uint16_t hw = bitfield>>16;

            bitfield = (hw << 14) | (lw&0x3FFF);
        }

        off64_t save_offset = stream->Tell();
        off64_t offset = (bitfield>>14) * 4;
        uint16_t num_ops = bitfield&0xF;

        a.bone_id = (bitfield>>4) & 0x3FF;
        stream->Seek(kf_start + offset, SEEK_SET);

        for (uint16_t i = 0; i < num_ops; i++)
        {
            uint16_t op;

            if (!stream->Read16(&op))
                return false;

            std::vector<G1AKeyframe> *kfs;

            if (op == 0)
            {
                if (a.rotation_kfs.size() > 0)
                {
                    DPRINTF("Duplicated rotation detected.\n");
                    return false;
                }

                kfs = &a.rotation_kfs;
            }
            else if (op == 1)
            {
                if (a.translation_kfs.size() > 0)
                {
                    DPRINTF("Duplicated rotation detected.\n");
                    return false;
                }

                kfs = &a.translation_kfs;
            }
            else if (op == 2)
            {
                if (a.scale_kfs.size() > 0)
                {
                    DPRINTF("Duplicated rotation detected.\n");
                    return false;
                }

                kfs = &a.scale_kfs;
            }
            else
            {
                DPRINTF("Unrecognized opcode %u\n", op);
                return false;
            }

            uint16_t num_kfs;
            uint32_t first_pv;

            if (!stream->Read16(&num_kfs))
                return false;

            if (!stream->Read32(&first_pv))
                return false;

            kfs->resize(num_kfs);

            for (G1AKeyframe &kf : *kfs)
            {
                if (!stream->Read16(&kf.time_start))
                    return false;
            }

            off64_t save_position = stream->Tell();
            stream->Seek(packed_vectors_start + first_pv*32, SEEK_SET);

            for (G1AKeyframe &kf : *kfs)
            {
                if (!stream->Read64(&kf.u))
                    return false;

                if (!stream->Read64(&kf.v))
                    return false;

                if (!stream->Read64(&kf.w))
                    return false;

                if (!stream->Read64(&kf.t))
                    return false;
            }

            if (num_kfs & 1)
                stream->Seek(save_position+2, SEEK_SET); // Align to 4
            else
                stream->Seek(save_position, SEEK_SET);
        }

        stream->Seek(save_offset, SEEK_SET);
    }

    return true;
}

bool G1aFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    FixedMemoryStream mem(const_cast<uint8_t *>(buf), size);
    return LoadInternal(&mem);
}

uint8_t *G1aFile::Save(size_t *psize)
{
    MemoryStream an_stream, kf_stream, pv_stream;

    for (const G1AAnimation &a : animations)
    {
        uint32_t bitfield;
        uint32_t num_ops = 0, kf_offset;

        if (a.rotation_kfs.size() > 0)
            num_ops++;

        if (a.translation_kfs.size() > 0)
            num_ops++;

        if (a.scale_kfs.size() > 0)
            num_ops++;

        if (a.bone_id >= 1024)
        {
            DPRINTF("Bone id %u greater than 1024, cannot save.\n", a.bone_id);
            return nullptr;
        }

        kf_offset = (uint32_t)kf_stream.Tell() / 4;

        bitfield = num_ops;
        bitfield |= (a.bone_id&0x3FF) << 4;

        if (version < 50)
        {
            if (kf_offset >= 65536)
            {
                DPRINTF("Animation overflow, try using version 50 of format.\n");
                return nullptr;
            }

            bitfield |= (kf_offset << 16);
        }
        else
        {
            if (kf_offset >= 262144)
            {
                DPRINTF("Animation overflow.\n");
                return nullptr;
            }

            bitfield |= (kf_offset << 14);
        }

        an_stream.Write32(bitfield);

        for (uint16_t i = 0; i < 3; i++)
        {
            const std::vector<G1AKeyframe> *kfs;

            if (i == 0)
            {
                kfs = &a.rotation_kfs;
            }
            else if (i == 1)
            {
                kfs = &a.translation_kfs;
            }
            else
            {
                kfs = &a.scale_kfs;
            }

            if (kfs->size() == 0)
                continue;

            kf_stream.Write16(i);
            kf_stream.Write16((uint16_t)kfs->size());
            kf_stream.Write32((uint32_t)pv_stream.Tell() / 32);

            for (const G1AKeyframe &kf : *kfs)
            {
                kf_stream.Write16(kf.time_start);
                pv_stream.Write64(kf.u);
                pv_stream.Write64(kf.v);
                pv_stream.Write64(kf.w);
                pv_stream.Write64(kf.t);
            }

            if (kfs->size() & 1)
                kf_stream.Write16(0); // align to 4
        }
    }

    size_t hdr_size = sizeof(G1AHeader);
    G1AHeader hdr;

    hdr.signature = G2A_SIGNATURE;
    std::string str_version = Utils::ToStringAndPad(version, 4);

    for (size_t i = 0; i < 4; i++)
    {
        hdr.version[i] = str_version[3-i];
    }

    if (version <= 30)
        hdr_size -= 4;

    hdr.file_size = (uint32_t)(hdr_size + an_stream.GetSize() + kf_stream.GetSize() + pv_stream.GetSize());
    hdr.frame_rate = frame_rate;
    hdr.num_frames = num_frames;
    hdr.num_bones = (uint16_t)(animations.size() << 4);
    hdr.keyframes_size = (uint32_t)kf_stream.GetSize();
    hdr.num_packed_vectors = (uint32_t)pv_stream.GetSize() / 32;
    hdr.unk_1C = 0;

    uint8_t *ret = new uint8_t[hdr.file_size];
    memcpy(ret, &hdr, hdr_size);
    memcpy(ret+hdr_size, an_stream.GetMemory(false), an_stream.GetSize());
    memcpy(ret+hdr_size+an_stream.GetSize(), kf_stream.GetMemory(false), kf_stream.GetSize());
    memcpy(ret+hdr_size+an_stream.GetSize()+kf_stream.GetSize(), pv_stream.GetMemory(false), pv_stream.GetSize());

    *psize = (size_t)hdr.file_size;
    return ret;
}

void G1aFile::DebugTest() const
{
    for (const G1AAnimation &a : animations)
    {
        if (a.bone_id != 1)
            continue;

        for (size_t i = 0; i < a.translation_kfs.size(); i++)
        {
            uint16_t length;

            if (i == a.translation_kfs.size()-1)
            {
                length = num_frames - a.translation_kfs[i].time_start;
            }
            else
            {
                length = a.translation_kfs[i+1].time_start - a.translation_kfs[i].time_start;
            }

            for (uint16_t f = 0; f < length; f++)
            {
                float xyz[3];
                a.translation_kfs[i].GetXYZForFrame(xyz, f, length);
                DPRINTF("(%d) %f %f %f\n", f, xyz[0], xyz[1], xyz[2]);
            }
        }
    }
}

void G1aFile::DebugCreateSampleAnimation()
{
    Reset();

    G1AAnimation a(1); // Animation on "SK_Skeleton"

    // Set origin at 0, 80.0f, 0 and move 20 to left for 4 seconds
    a.AddFirstTranslationLinear(4*60, 0.0f, 80.0f, 0.0f, 20.0f, 00.0f, 0.0f);
    // Move 40 to right for 4 seconds
    a.AddTranslationLinear(4*60,  -40.0f, 0.0f, 0.0f);
    // Move back to start position (20 left) in 1 second
    a.AddTranslationLinear(1*60,  20.0f, 0.0f, 0.0f);
    // Move up 60 in 5 seconds
    a.AddTranslationLinear(5*60, 0.0f, 60.0f, 0.0f);
    // Move down 120 in 5 seconds
    a.AddTranslationLinear(5*60, 0.0f, -120.0f, 0.0f);
    // Move back to start position (60 up) in 1 second
    a.AddTranslationLinear(1*60, 0.0f, 60.0f, 0.0f);
    // Move to front 50 for 3 seconds
    a.AddTranslationLinear(3*60, 0.0f, 0.0f, 50.0f);
    // Move to back 100 for 3 seconds
    a.AddTranslationLinear(3*60, 0.0f, 0.0f, -100.0f);
    // Move to start position (50 front) in 1 seconds
    a.AddTranslationLinear(1*60, 0.0f, 0.0f, 50.0f);

    // For 27 seconds (starting at 0), don't rotate
    a.AddFirstRotationLinear(27*60, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    // Now we start rotating at second 27 (after translations)
    // First rotate X 90 degrees for 2 seconds
    a.AddRotationLinear(2*60, (float)M_PI / 2.0f, 0.0f, 0.0f);
    // This line for testing multiple axis rotation: a.AddRotationLinear(2*60, (float)M_PI / 2.0f, (float)M_PI / 2.0f, 0.0f);
    // Rotate X -180 degrees for 2 seconds
    a.AddRotationLinear(2*60, (float)-M_PI, 0.0f, 0.0f);
    // Rotate to original position (90 degrees X from here) in 1 second
    a.AddRotationLinear(1*30, (float)M_PI / 2.0f, 0.0f, 0.0f);
    // Rotate Y 90 degrees for 4 seconds
    a.AddRotationLinear(4*60, 0.0f, (float)M_PI / 2.0f, 0.0f);
    // Rotate Y -270 degrees for 6 seconds (Honoka, show ass)
    a.AddRotationLinear(6*60, 0.0f, (float)-M_PI*1.5f, 0.0f);
    // Rotate to original position (180 degrees Y from here) in 1 second
    a.AddRotationLinear(1*60, 0.0f, (float)M_PI, 0.0f);
    // Rotate Z 90 degrees in 3 seconds
    a.AddRotationLinear(3*60, 0.0f, 0.0f, (float)M_PI / 2.0f);
    // Rotate Z -180 degrees in 3 seconds
    a.AddRotationLinear(3*60, 0.0f, 0.0f, (float)-M_PI);
    // Rotate Z back to original position (90 degrees from here)in 1 second
    a.AddRotationLinear(1*60, 0.0f, 0.0f, (float)M_PI / 2.0f);

    // For 50 seconds, have original scale
    a.AddFirstScaleLinear(50*60, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f);
    // Scale to 0.5 for 7 seconds (1-0.5= 0.5)
    a.AddScaleLinear(7*60, -0.5f, -0.5f, -0.5f);
    // Scale to 1.5 for 7 seconds (from here, it is 1.0f)
    a.AddScaleLinear(7*60, 1.0f, 1.0f, 1.0f);
    // Scale to original size 1.0 for 1 second (from here, it is -0.5f)
    a.AddScaleLinear(1*60, -0.5f, -0.5f, -0.5f);
    // After here we are at 65 seconds

    AddAnimation(a);

    SetNumFrames(68*60); // 68 seconds. Add a 3 seconds delay so character stays at that position before animation loops
}

#ifdef FBX_SUPPORT

inline float to_degrees(float rad)
{
    return rad * (180.0f / (float)FBXSDK_PI);
}

static void axis_to_quaternion(float *q, float *v)
{
    float length = sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    float s = sinf(length*0.5f);
    float c = cosf(length*0.5f);

    if (length > 0.000011920929f)
    {
        q[0] = v[0] * (s / length);
        q[1] = v[1] * (s / length);
        q[2] = v[2] * (s / length);
    }
    else
    {
        q[0] = v[0] * 0.5f;
        q[1] = v[1] * 0.5f;
        q[2] = v[2] * 0.5f;
    }

    q[3] = c;
}

static void quaternion_to_euler_degrees(float *euler, float *q)
{

    // roll (x-axis rotation)
    float sinr_cosp = 2 * (q[3] * q[0] + q[1] * q[2]);
    float cosr_cosp = 1 - 2 * (q[0] * q[0] + q[1] * q[1]);
    euler[0] = std::atan2f(sinr_cosp, cosr_cosp);

    // pitch (y-axis rotation)
    float sinp = 2 * (q[3] * q[1] - q[2] * q[0]);
    if (std::abs(sinp) >= 1)
        euler[1] = std::copysign((float)M_PI / 2, sinp); // use 90 degrees if out of range
    else
        euler[1] = std::asin(sinp);

    // yaw (z-axis rotation)
    float siny_cosp = 2 * (q[3] * q[2] + q[0] * q[1]);
    float cosy_cosp = 1 - 2 * (q[1] * q[1] + q[2] * q[2]);
    euler[2] = std::atan2f(siny_cosp, cosy_cosp);

    //DPRINTF("(quat to euler) %f %f %f %f -> %f %f %f\n", q[0], q[1], q[2], q[3], euler[0], euler[1], euler[2]);

    euler[0] = to_degrees(euler[0]);
    euler[1] = to_degrees(euler[1]);
    euler[2] = to_degrees(euler[2]);
}

/*static void quaternion_to_euler_degrees(float *euler, float *q)
{
    FbxQuaternion quat = FbxQuaternion(q[0], q[1], q[2], q[3]);
    FbxAMatrix m;

    m.SetQ(quat);
    FbxVector4 v4 = m.GetR();

    double *ptr = v4;
    euler[0] = ptr[0];
    euler[1] = ptr[1];
    euler[2] = ptr[2];
}*/

/*#include "math/EulerAngles.h"

static void quaternion_to_euler_degrees(float *euler, float *q)
{
    Quat quat;
    quat.x = q[0];
    quat.y = q[1];
    quat.z = q[2];
    quat.w = q[3];

    EulerAngles eul = Eul_FromQuat(quat, EulOrdXYZs);
    DPRINTF("(quat to euler) %f %f %f %f -> %f %f %f\n", q[0], q[1], q[2], q[3], eul.x, eul.y, eul.z);
    euler[0] = to_degrees(eul.x);
    euler[1] = to_degrees(eul.y);
    euler[2] = to_degrees(eul.z);
}*/



/*template<class T>
static constexpr const T& clamp( const T& v, const T& lo, const T& hi )
{
    assert( !(hi < lo) );
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

static void quaternion_to_euler_degrees(float *euler, const float *q)
{
    float f[4][4];

    float x = q[0], y = q[1], z = q[2], w = q[3];
    float x2 = x + x, y2 = y + y, z2 = z + z;
    float xx = x * x2, xy = x * y2, xz = x * z2;
    float yy = y * y2, yz = y * z2, zz = z * z2;
    float wx = w * x2, wy = w * y2, wz = w * z2;

    float sx = 1.0f, sy = 1.0f, sz = 1.0f;

    f[0][0] = ( 1 - ( yy + zz ) ) * sx;
    f[1][0] = ( xy + wz ) * sx;
    f[2][0] = ( xz - wy ) * sx;
    f[3][0] = 0.0f;

    f[0][1] = ( xy - wz ) * sy;
    f[1][1] = ( 1 - ( xx + zz ) ) * sy;
    f[2][1] = ( yz + wx ) * sy;
    f[3][1] = 0.0f;

    f[0][2] = ( xz + wy ) * sz;
    f[1][2] = ( yz - wx ) * sz;
    f[2][2] = ( 1 - ( xx + yy ) ) * sz;
    f[3][2] = 0.0f;

    f[0][3] = 0.0f;
    f[1][3] = 0.0f;
    f[2][3] = 0.0f;
    f[3][3] = 1;

    euler[1] = asin(clamp(f[0][2], -1.0f, 1.0f));

    if ( fabs( f[0][2] ) < 0.9999999 ) {

        euler[0] = atan2( - f[1][2], f[2][2] );
        euler[2] = atan2( - f[0][1], f[0][0] );

    } else {

        euler[0] = atan2( f[2][1], f[1][1] );
        euler[2] = 0.0f;
    }



    DPRINTF("(quat to euler) %f %f %f %f -> %f %f %f\n", q[0], q[1], q[2], q[3], euler[0], euler[1], euler[2]);
    euler[0] = to_degrees(euler[0]);
    euler[1] = to_degrees(euler[1]);
    euler[2] = to_degrees(euler[2]);

    if (roundf(euler[0]) == -180.0f && roundf(euler[2]) == -180.0f)
    {
        euler[0] = euler[2] = 0.0f;
        euler[1] = -180.0f;
    }
    else if (roundf(euler[0]) == 180.0f && roundf(euler[2]) == 180.0f)
    {
        euler[0] = euler[2] = 0.0f;
        euler[1] = 180.0f;
    }
}*/

static void axis_to_euler_degrees(float *euler, float *v)
{
    float q[4];

    axis_to_quaternion(q, v);
    //DPRINTF("(axis to quat) %f %f %f -> %f %f %f %f\n", v[0], v[1], v[2], q[0], q[1], q[2], q[3]);
    quaternion_to_euler_degrees(euler, q);
}

bool G1aFile::ExportToFbx(FbxScene *scene, G1mFile &g1m, bool linear, bool correct_singularities) const
{
    FbxAnimStack* lAnimStack = FbxAnimStack::Create(scene, "G1A2FBX");
    FbxAnimLayer* lAnimLayer = FbxAnimLayer::Create(scene, "G1A2FBX Layer");
    FbxTime lTime;
    int lKeyIndex;

    lAnimStack->AddMember(lAnimLayer);

    for (const G1AAnimation &a : animations)
    {
        std::string bone_name;

        if (!g1m.BoneIDToName(a.bone_id, bone_name))
        {
            DPRINTF("Cannot find name for bone id %u in g1m.\n", a.bone_id);
            return false;
        }

        FbxNode *node = scene->FindNodeByName(bone_name.c_str());
        if (!node)
        {
            DPRINTF("Cannot find node %s in scene.\n", bone_name.c_str());
            return false;
        }

        FbxAnimCurve *curve_x, *curve_y, *curve_z;

        if (a.rotation_kfs.size() > 0)
        {
            curve_x = node->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
            curve_y = node->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
            curve_z = node->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);

            curve_x->KeyModifyBegin();
            curve_y->KeyModifyBegin();
            curve_z->KeyModifyBegin();
        }

        for (const G1AKeyframe &kf: a.rotation_kfs)
        {
            float xyz[3], euler[3];

            kf.GetXYZForFrame(xyz, 0, 1);            
            axis_to_euler_degrees(euler, xyz);

            for (int i = 0; i < 3; i++)
            {
                xyz[i] = euler[i];
            }

            if (correct_singularities)
            {
                /*for (int i = 0; i < 3; i++)
                {
                    float r = roundf(xyz[i]);
                    float a = fabsf(r);

                    if (a >= 165.0f && a <= 180.0f)
                    {
                        int prev_index;
                        float val;

                        if (i == 0)
                        {
                            prev_index = curve_x->KeyGetCount()-1;
                            if (prev_index >= 0)
                                val = curve_x->KeyGetValue(prev_index);
                            else
                                continue;
                        }
                        else if (i == 1)
                        {
                            prev_index = curve_y->KeyGetCount()-1;
                            if (prev_index >= 0)
                                val = curve_y->KeyGetValue(prev_index);
                            else
                                continue;
                        }
                        else if (i == 2)
                        {
                            prev_index = curve_z->KeyGetCount()-1;
                            if (prev_index >= 0)
                                val = curve_z->KeyGetValue(prev_index);
                            else
                                continue;
                        }

                        float rp = roundf(val);
                        float ap = fabsf(rp);

                        if (ap >= 165.0f && ap <= 180.0f)
                        {
                            if ((xyz[i] > 0 && val < 0) || (xyz[i] < 0 && val > 0))
                            {
                                xyz[i] = -xyz[i];
                            }
                        }
                    }
                }*/
            }

            double fbx_time = (double)kf.time_start / (double)frame_rate;
            lTime.SetSecondDouble(fbx_time);

            lKeyIndex = curve_x->KeyAdd(lTime);
            curve_x->KeySetValue(lKeyIndex, xyz[0]);
            curve_x->KeySetInterpolation(lKeyIndex, (linear) ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic);

            lKeyIndex = curve_y->KeyAdd(lTime);
            curve_y->KeySetValue(lKeyIndex, xyz[1]);
            curve_y->KeySetInterpolation(lKeyIndex, (linear) ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic);

            lKeyIndex = curve_z->KeyAdd(lTime);
            curve_z->KeySetValue(lKeyIndex, xyz[2]);
            curve_z->KeySetInterpolation(lKeyIndex, (linear) ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic);

            if (correct_singularities)
            {
                FbxAnimCurveFilterUnroll filter;
                FbxAnimCurve *curves[3] = { curve_x, curve_y, curve_z };

                filter.Apply(curves, 3);
            }

            //if (a.bone_id == 4)
              //  DPRINTF("%s: Added rotation %f %f %f at time %f (%u)\n", bone_name.c_str(), xyz[0], xyz[1], xyz[2], (float)fbx_time, kf.time_start);
        }

        if (a.rotation_kfs.size() > 0)
        {
            // Add final frame
            if (a.rotation_kfs.back().time_start < num_frames)
            {
                float x = curve_x->KeyGetValue(curve_x->KeyGetCount()-1);
                float y = curve_y->KeyGetValue(curve_y->KeyGetCount()-1);
                float z = curve_z->KeyGetValue(curve_z->KeyGetCount()-1);

                double fbx_time = (double)num_frames / (double)frame_rate;
                lTime.SetSecondDouble(fbx_time);

                lKeyIndex = curve_x->KeyAdd(lTime);
                curve_x->KeySetValue(lKeyIndex, x);
                curve_x->KeySetInterpolation(lKeyIndex, (linear) ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic);

                lKeyIndex = curve_y->KeyAdd(lTime);
                curve_y->KeySetValue(lKeyIndex, y);
                curve_y->KeySetInterpolation(lKeyIndex, (linear) ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic);

                lKeyIndex = curve_z->KeyAdd(lTime);
                curve_z->KeySetValue(lKeyIndex, z);
                curve_z->KeySetInterpolation(lKeyIndex, (linear) ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic);
            }

            curve_x->KeyModifyEnd();
            curve_y->KeyModifyEnd();
            curve_z->KeyModifyEnd();
        }

        if (a.translation_kfs.size() > 0)
        {
            curve_x = node->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
            curve_y = node->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
            curve_z = node->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);

            curve_x->KeyModifyBegin();
            curve_y->KeyModifyBegin();
            curve_z->KeyModifyBegin();
        }

        for (const G1AKeyframe &kf: a.translation_kfs)
        {
            float xyz[3];

            kf.GetXYZForFrame(xyz, 0, 1);

            double fbx_time = (double)kf.time_start / (double)frame_rate;
            lTime.SetSecondDouble(fbx_time);

            lKeyIndex = curve_x->KeyAdd(lTime);
            curve_x->KeySetValue(lKeyIndex, xyz[0]);
            curve_x->KeySetInterpolation(lKeyIndex, (linear) ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic);

            lKeyIndex = curve_y->KeyAdd(lTime);
            curve_y->KeySetValue(lKeyIndex, xyz[1]);
            curve_y->KeySetInterpolation(lKeyIndex, (linear) ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic);

            lKeyIndex = curve_z->KeyAdd(lTime);
            curve_z->KeySetValue(lKeyIndex, xyz[2]);
            curve_z->KeySetInterpolation(lKeyIndex, (linear) ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic);

            //DPRINTF("Added translation %f %f %f at time %f\n", xyz[0], xyz[1], xyz[2], fbx_time);
        }

        if (a.translation_kfs.size() > 0)
        {
            // Add final frame
            if (a.translation_kfs.back().time_start < num_frames)
            {
                float x = curve_x->KeyGetValue(curve_x->KeyGetCount()-1);
                float y = curve_y->KeyGetValue(curve_y->KeyGetCount()-1);
                float z = curve_z->KeyGetValue(curve_z->KeyGetCount()-1);

                double fbx_time = (double)num_frames / (double)frame_rate;
                lTime.SetSecondDouble(fbx_time);

                lKeyIndex = curve_x->KeyAdd(lTime);
                curve_x->KeySetValue(lKeyIndex, x);
                curve_x->KeySetInterpolation(lKeyIndex, (linear) ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic);

                lKeyIndex = curve_y->KeyAdd(lTime);
                curve_y->KeySetValue(lKeyIndex, y);
                curve_y->KeySetInterpolation(lKeyIndex, (linear) ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic);

                lKeyIndex = curve_z->KeyAdd(lTime);
                curve_z->KeySetValue(lKeyIndex, z);
                curve_z->KeySetInterpolation(lKeyIndex, (linear) ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic);
            }

            curve_x->KeyModifyEnd();
            curve_y->KeyModifyEnd();
            curve_z->KeyModifyEnd();
        }

        if (a.scale_kfs.size() > 0)
        {
            curve_x = node->LclScaling.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
            curve_y = node->LclScaling.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
            curve_z = node->LclScaling.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);

            curve_x->KeyModifyBegin();
            curve_y->KeyModifyBegin();
            curve_z->KeyModifyBegin();
        }

        for (const G1AKeyframe &kf: a.scale_kfs)
        {
            float xyz[3];

            kf.GetXYZForFrame(xyz, 0, 1);

            double fbx_time = (double)kf.time_start / (double)frame_rate;

            lTime.SetSecondDouble(fbx_time);
            lKeyIndex = curve_x->KeyAdd(lTime);
            curve_x->KeySetValue(lKeyIndex, xyz[0]);
            curve_x->KeySetInterpolation(lKeyIndex, (linear) ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic);

            lTime.SetSecondDouble(fbx_time);
            lKeyIndex = curve_y->KeyAdd(lTime);
            curve_y->KeySetValue(lKeyIndex, xyz[1]);
            curve_y->KeySetInterpolation(lKeyIndex, (linear) ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic);

            lTime.SetSecondDouble(fbx_time);
            lKeyIndex = curve_z->KeyAdd(lTime);
            curve_z->KeySetValue(lKeyIndex, xyz[2]);
            curve_z->KeySetInterpolation(lKeyIndex, (linear) ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic);
        }

        if (a.scale_kfs.size() > 0)
        {
            // Add final frame
            if (a.scale_kfs.back().time_start < num_frames)
            {
                float x = curve_x->KeyGetValue(curve_x->KeyGetCount()-1);
                float y = curve_y->KeyGetValue(curve_y->KeyGetCount()-1);
                float z = curve_z->KeyGetValue(curve_z->KeyGetCount()-1);

                double fbx_time = (double)num_frames / (double)frame_rate;
                lTime.SetSecondDouble(fbx_time);

                lKeyIndex = curve_x->KeyAdd(lTime);
                curve_x->KeySetValue(lKeyIndex, x);
                curve_x->KeySetInterpolation(lKeyIndex, (linear) ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic);

                lKeyIndex = curve_y->KeyAdd(lTime);
                curve_y->KeySetValue(lKeyIndex, y);
                curve_y->KeySetInterpolation(lKeyIndex, (linear) ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic);

                lKeyIndex = curve_z->KeyAdd(lTime);
                curve_z->KeySetValue(lKeyIndex, z);
                curve_z->KeySetInterpolation(lKeyIndex, (linear) ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic);
            }

            curve_x->KeyModifyEnd();
            curve_y->KeyModifyEnd();
            curve_z->KeyModifyEnd();
        }

        /*if (a.translation_kfs.size() > 0 && a.rotation_kfs.size() == 0)
        {
            curve_x = node->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
            curve_y = node->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
            curve_z = node->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);

            curve_x->KeyModifyBegin();
            curve_y->KeyModifyBegin();
            curve_z->KeyModifyBegin();

            lTime.SetSecondDouble(0.0);
            lKeyIndex = curve_x->KeyAdd(lTime);
            curve_x->KeySetValue(lKeyIndex, 0.0f);
            curve_x->KeySetInterpolation(lKeyIndex, (linear) ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic);

            lTime.SetSecondDouble(0.0);
            lKeyIndex = curve_y->KeyAdd(lTime);
            curve_y->KeySetValue(lKeyIndex, 0.0f);
            curve_y->KeySetInterpolation(lKeyIndex, (linear) ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic);

            lTime.SetSecondDouble(0.0);
            lKeyIndex = curve_z->KeyAdd(lTime);
            curve_z->KeySetValue(lKeyIndex, 0.0f);
            curve_z->KeySetInterpolation(lKeyIndex, (linear) ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic);

            curve_x->KeyModifyEnd();
            curve_y->KeyModifyEnd();
            curve_z->KeyModifyEnd();
        }
        else if (a.rotation_kfs.size() > 0 && a.translation_kfs.size() == 0)
        {
            curve_x = node->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
            curve_y = node->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
            curve_z = node->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);

            curve_x->KeyModifyBegin();
            curve_y->KeyModifyBegin();
            curve_z->KeyModifyBegin();

            lTime.SetSecondDouble(0.0);
            lKeyIndex = curve_x->KeyAdd(lTime);
            curve_x->KeySetValue(lKeyIndex, 0.0f);
            curve_x->KeySetInterpolation(lKeyIndex, (linear) ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic);

            lTime.SetSecondDouble(0.0);
            lKeyIndex = curve_y->KeyAdd(lTime);
            curve_y->KeySetValue(lKeyIndex, 0.0f);
            curve_y->KeySetInterpolation(lKeyIndex, (linear) ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic);

            lTime.SetSecondDouble(0.0);
            lKeyIndex = curve_z->KeyAdd(lTime);
            curve_z->KeySetValue(lKeyIndex, 0.0f);
            curve_z->KeySetInterpolation(lKeyIndex, (linear) ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic);

            curve_x->KeyModifyEnd();
            curve_y->KeyModifyEnd();
            curve_z->KeyModifyEnd();
        }*/
    }

    return true;
}

void G1aFile::GetFbxAnimatedBones(G1mFile &g1m, FbxNode *node, FbxAnimLayer *layer, std::map<uint16_t, std::string> &bones)
{
    bool has_animation = false;

    if (node->LclTranslation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_X))
        has_animation = true;
    else if (node->LclTranslation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Y))
        has_animation = true;
    else if (node->LclTranslation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Z))
        has_animation = true;
    else if (node->LclRotation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_X))
        has_animation = true;
    else if (node->LclRotation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Y))
        has_animation = true;
    else if (node->LclRotation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Z))
        has_animation = true;
    else if (node->LclScaling.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_X))
        has_animation = true;
    else if (node->LclScaling.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Y))
        has_animation = true;
    else if (node->LclScaling.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Z))
        has_animation = true;

    if (has_animation)
    {
        //DPRINTF("Trying \"%s\"\n", node->GetName());

        int id = g1m.BoneNameToID(node->GetName());
        if (id >= 0)
        {
            bones[(uint16_t)id] = node->GetName();
        }
        else
        {
            //DPRINTF("No ID for name \"%s\".\n", node->GetName());
        }
    }

    for(int child = 0; child < node->GetChildCount(); child++)
    {
        GetFbxAnimatedBones(g1m, node->GetChild(child), layer, bones);
    }
}

static inline double degrees_to_radians(double deg)
{
    return deg * (FBXSDK_PI / 180);
}

static void euler_to_quaternion(float *q, double *euler)
{
    double yaw = euler[2];
    double pitch = euler[1];
    double roll = euler[0];

    double cy = cos(yaw * 0.5);
    double sy = sin(yaw * 0.5);
    double cp = cos(pitch * 0.5);
    double sp = sin(pitch * 0.5);
    double cr = cos(roll * 0.5);
    double sr = sin(roll * 0.5);

    q[3] = (float)(cy * cp * cr + sy * sp * sr);
    q[0] = (float)(cy * cp * sr - sy * sp * cr);
    q[1] = (float)(sy * cp * sr + cy * sp * cr);
    q[2] = (float)(sy * cp * cr - cy * sp * sr);
}

static void euler_degrees_to_axis(float *axis, double *euler)
{
    float q[4];
    double temp[3];

    for (int i = 0; i < 3; i++)
        temp[i] = degrees_to_radians(euler[i]);

    euler_to_quaternion(q, temp);

    float angle = 2 * acosf(q[3]);
    if (1 - (q[3] * q[3]) < 0.000001f)
    {
        axis[0] = q[0];
        axis[1] = q[1];
        axis[2] = q[2];
    }
    else
    {
        float s = sqrtf(1 - (q[3] * q[3]));
        axis[0] = q[0] / s;
        axis[1] = q[1] / s;
        axis[2] = q[2] / s;
    }

    axis[0] = axis[0]*angle;
    axis[1] = axis[1]*angle;
    axis[2] = axis[2]*angle;
}

bool G1aFile::ImportFromFbx(FbxScene *scene, G1mFile &g1m)
{
    Reset();

    int num_anims = scene->GetSrcObjectCount<FbxAnimStack>();
    if (num_anims == 0)
    {
        DPRINTF("Error: this fbx doesn't have any animation stack!\n");
        return false;
    }

    FbxAnimStack* lAnimStack = scene->GetSrcObject<FbxAnimStack>(0);
    if (num_anims > 1)
    {
        DPRINTF("Warning: this fbx has more than one animation stack, I will only use stack %s.\n", lAnimStack->GetName());
    }

    int num_layers = lAnimStack->GetMemberCount<FbxAnimLayer>();
    if (num_layers == 0)
    {
        DPRINTF("Error: this animation stack has 0 layers (stack %s).\n", lAnimStack->GetName());
        return false;
    }

    FbxAnimLayer* lAnimLayer = lAnimStack->GetMember<FbxAnimLayer>(0);
    if (num_layers > 1)
    {
        DPRINTF("Warning: this animation stack has more than one layer, I will only use layer %s.\n", lAnimLayer->GetName());
    }

    std::map<uint16_t, std::string> bones;
    GetFbxAnimatedBones(g1m, scene->GetRootNode(), lAnimLayer, bones);

    if (bones.size() == 0)
    {
        DPRINTF("No animated parts found in the fbx (stack=%s, layer=%s).\n", lAnimStack->GetName(), lAnimLayer->GetName());
        return false;
    }

    animations.resize(bones.size());

    //DPRINTF("Size = %Id\n", animations.size());

    size_t i = 0;

    for (auto &it : bones)
    {
        G1AAnimation &a = animations[i];
        //DPRINTF("%d %s\n", it.first, it.second.c_str());
        a.bone_id = it.first;

        FbxNode *node = scene->FindNodeByName(it.second.c_str());
        FbxAnimCurve *rot_curve_x, *rot_curve_y, *rot_curve_z;
        FbxAnimCurve *trans_curve_x, *trans_curve_y, *trans_curve_z;
        FbxAnimCurve *scale_curve_x, *scale_curve_y, *scale_curve_z;

        rot_curve_x = node->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
        rot_curve_y = node->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
        rot_curve_z = node->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
        trans_curve_x = node->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
        trans_curve_y = node->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
        trans_curve_z = node->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
        scale_curve_x = node->LclScaling.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
        scale_curve_y = node->LclScaling.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
        scale_curve_z = node->LclScaling.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);

        if (rot_curve_x || rot_curve_y || rot_curve_z)
        {
            std::map<uint16_t, FbxVector4> time_map;
            int lKeyCount;
            FbxTime lKeyTime;

            if (rot_curve_x)
            {
                lKeyCount = rot_curve_x->KeyGetCount();

                for(int lCount = 0; lCount < lKeyCount; lCount++)
                {
                    float val = rot_curve_x->KeyGetValue(lCount);
                    lKeyTime  = rot_curve_x->KeyGetTime(lCount);
                    uint16_t g1a_time = (uint16_t) round(lKeyTime.GetSecondDouble() * (double)frame_rate);

                    time_map[g1a_time] = FbxVector4((double)val, 0.0, 0.0);
                }
            }

            if (rot_curve_y)
            {
                lKeyCount = rot_curve_y->KeyGetCount();

                for(int lCount = 0; lCount < lKeyCount; lCount++)
                {
                    float val = rot_curve_y->KeyGetValue(lCount);
                    lKeyTime  = rot_curve_y->KeyGetTime(lCount);
                    uint16_t g1a_time = (uint16_t) round(lKeyTime.GetSecondDouble() * (double)frame_rate);

                    auto it = time_map.find(g1a_time);
                    if (it == time_map.end())
                        time_map[g1a_time] = FbxVector4(0.0, (double)val, 0.0);
                    else
                    {
                        double *buf = (double *)it->second;
                        buf[1] = (double)val;
                    }
                }
            }

            if (rot_curve_z)
            {
                lKeyCount = rot_curve_z->KeyGetCount();

                for(int lCount = 0; lCount < lKeyCount; lCount++)
                {
                    float val = rot_curve_z->KeyGetValue(lCount);
                    lKeyTime  = rot_curve_z->KeyGetTime(lCount);
                    uint16_t g1a_time = (uint16_t) round(lKeyTime.GetSecondDouble() * (double)frame_rate);

                    auto it = time_map.find(g1a_time);
                    if (it == time_map.end())
                        time_map[g1a_time] = FbxVector4(0.0, 0.0, (double)val);
                    else
                    {
                        double *buf = (double *)it->second;
                        buf[2] = (double)val;
                    }
                }
            }

            for (auto &t : time_map)
            {
                G1AKeyframe kf;
                float axis[3];

                euler_degrees_to_axis(axis, (double *)t.second);

                kf.time_start = t.first;
                kf.u = G1AKeyframe::PackVector(axis[0], axis[1], axis[2]);

                a.rotation_kfs.push_back(kf);
            }
        }

        if (trans_curve_x || trans_curve_y || trans_curve_z)
        {
            std::map<uint16_t, FbxVector4> time_map;
            int lKeyCount;
            FbxTime lKeyTime;

            if (trans_curve_x)
            {
                lKeyCount = trans_curve_x->KeyGetCount();

                for(int lCount = 0; lCount < lKeyCount; lCount++)
                {
                    float val = trans_curve_x->KeyGetValue(lCount);
                    lKeyTime  = trans_curve_x->KeyGetTime(lCount);
                    uint16_t g1a_time = (uint16_t) round(lKeyTime.GetSecondDouble() * (double)frame_rate);

                    time_map[g1a_time] = FbxVector4((double)val, 0.0, 0.0);
                }
            }

            if (trans_curve_y)
            {
                lKeyCount = trans_curve_y->KeyGetCount();

                for(int lCount = 0; lCount < lKeyCount; lCount++)
                {
                    float val = trans_curve_y->KeyGetValue(lCount);
                    lKeyTime  = trans_curve_y->KeyGetTime(lCount);
                    uint16_t g1a_time = (uint16_t) round(lKeyTime.GetSecondDouble() * (double)frame_rate);

                    auto it = time_map.find(g1a_time);
                    if (it == time_map.end())
                        time_map[g1a_time] = FbxVector4(0.0, (double)val, 0.0);
                    else
                    {
                        double *buf = (double *)it->second;
                        buf[1] = (double)val;
                    }
                }
            }

            if (trans_curve_z)
            {
                lKeyCount = trans_curve_z->KeyGetCount();

                for(int lCount = 0; lCount < lKeyCount; lCount++)
                {
                    float val = trans_curve_z->KeyGetValue(lCount);
                    lKeyTime  = trans_curve_z->KeyGetTime(lCount);
                    uint16_t g1a_time = (uint16_t) round(lKeyTime.GetSecondDouble() * (double)frame_rate);

                    auto it = time_map.find(g1a_time);
                    if (it == time_map.end())
                        time_map[g1a_time] = FbxVector4(0.0, 0.0, (double)val);
                    else
                    {
                        double *buf = (double *)it->second;
                        buf[2] = (double)val;
                    }
                }
            }

            for (auto &t : time_map)
            {
                G1AKeyframe kf;
                double *buf = (double *)t.second;

                kf.time_start = t.first;
                kf.u = G1AKeyframe::PackVector((float)buf[0], (float)buf[1], (float)buf[2]);

                a.translation_kfs.push_back(kf);
            }
        }

        if (scale_curve_x || scale_curve_y || scale_curve_z)
        {
            std::map<uint16_t, FbxVector4> time_map;
            int lKeyCount;
            FbxTime lKeyTime;

            if (scale_curve_x)
            {
                lKeyCount = scale_curve_x->KeyGetCount();

                for(int lCount = 0; lCount < lKeyCount; lCount++)
                {
                    float val = scale_curve_x->KeyGetValue(lCount);
                    lKeyTime  = scale_curve_x->KeyGetTime(lCount);
                    uint16_t g1a_time = (uint16_t) round(lKeyTime.GetSecondDouble() * (double)frame_rate);

                    time_map[g1a_time] = FbxVector4((double)val, 1.0, 1.0);
                }
            }

            if (scale_curve_y)
            {
                lKeyCount = scale_curve_y->KeyGetCount();

                for(int lCount = 0; lCount < lKeyCount; lCount++)
                {
                    float val = scale_curve_y->KeyGetValue(lCount);
                    lKeyTime  = scale_curve_y->KeyGetTime(lCount);
                    uint16_t g1a_time = (uint16_t) round(lKeyTime.GetSecondDouble() * (double)frame_rate);

                    auto it = time_map.find(g1a_time);
                    if (it == time_map.end())
                        time_map[g1a_time] = FbxVector4(1.0, (double)val, 1.0);
                    else
                    {
                        double *buf = (double *)it->second;
                        buf[1] = (double)val;
                    }
                }
            }

            if (scale_curve_z)
            {
                lKeyCount = scale_curve_z->KeyGetCount();

                for(int lCount = 0; lCount < lKeyCount; lCount++)
                {
                    float val = scale_curve_z->KeyGetValue(lCount);
                    lKeyTime  = scale_curve_z->KeyGetTime(lCount);
                    uint16_t g1a_time = (uint16_t) round(lKeyTime.GetSecondDouble() * (double)frame_rate);

                    auto it = time_map.find(g1a_time);
                    if (it == time_map.end())
                        time_map[g1a_time] = FbxVector4(1.0, 1.0, (double)val);
                    else
                    {
                        double *buf = (double *)it->second;
                        buf[2] = (double)val;
                    }
                }
            }

            for (auto &t : time_map)
            {
                G1AKeyframe kf;
                double *buf = (double *)t.second;

                kf.time_start = t.first;
                kf.u = G1AKeyframe::PackVector((float)buf[0], (float)buf[1], (float)buf[2]);

                a.scale_kfs.push_back(kf);
            }
        }

        i++;
    }

    uint16_t g_time = 0;

    for (G1AAnimation &a : animations)
    {
        for (int i = 0; i < ((int)a.rotation_kfs.size()-1); i++)
        {
            G1AKeyframe &kf = a.rotation_kfs[i];
            const G1AKeyframe &next = a.rotation_kfs[i+1];

            float xyz[3], xyz_next[3];

            G1AKeyframe::UnpackVector(kf.u, xyz);
            G1AKeyframe::UnpackVector(next.u, xyz_next);

            kf.v = G1AKeyframe::PackVector(xyz_next[0]-xyz[0], xyz_next[1]-xyz[1], xyz_next[2]-xyz[2]);

            if (kf.time_start > g_time)
                g_time = kf.time_start;

            if (next.time_start > g_time)
                g_time = next.time_start;
        }

        for (int i = 0; i < ((int)a.translation_kfs.size()-1); i++)
        {
            G1AKeyframe &kf = a.translation_kfs[i];
            const G1AKeyframe &next = a.translation_kfs[i+1];

            float xyz[3], xyz_next[3];

            G1AKeyframe::UnpackVector(kf.u, xyz);
            G1AKeyframe::UnpackVector(next.u, xyz_next);

            kf.v = G1AKeyframe::PackVector(xyz_next[0]-xyz[0], xyz_next[1]-xyz[1], xyz_next[2]-xyz[2]);

            if (kf.time_start > g_time)
                g_time = kf.time_start;

            if (next.time_start > g_time)
                g_time = next.time_start;
        }

        for (int i = 0; i < ((int)a.scale_kfs.size()-1); i++)
        {
            G1AKeyframe &kf = a.scale_kfs[i];
            const G1AKeyframe &next = a.scale_kfs[i+1];

            float xyz[3], xyz_next[3];

            G1AKeyframe::UnpackVector(kf.u, xyz);
            G1AKeyframe::UnpackVector(next.u, xyz_next);

            kf.v = G1AKeyframe::PackVector(xyz_next[0]-xyz[0], xyz_next[1]-xyz[1], xyz_next[2]-xyz[2]);

            if (kf.time_start > g_time)
                g_time = kf.time_start;

            if (next.time_start > g_time)
                g_time = next.time_start;
        }
    }

    if (g_time > 0)
    {
        num_frames = g_time;

        for (G1AAnimation &a : animations)
        {
            if (a.rotation_kfs.size() > 0 && a.rotation_kfs.back().time_start == num_frames)
                a.rotation_kfs.pop_back();

            if (a.translation_kfs.size() > 0 && a.translation_kfs.back().time_start == num_frames)
                a.translation_kfs.pop_back();

            if (a.scale_kfs.size() > 0 && a.scale_kfs.back().time_start == num_frames)
                a.scale_kfs.pop_back();
        }
    }

    return true;
}

#endif

void G1AKeyframe::UnpackVector(uint64_t v, float *fv)
{
    if (fv)
    {
        uint32_t i_exp = (uint32_t) ((v >> 37) & 0x7800000); // bits 63-60 << 23
        i_exp += 0x32000000; // Exponent 100
        float exp = *(float *)&i_exp;

        int32_t ix = (int32_t)((v >> 28) & 0xFFFFF000); // bits 40-59 << 12
        int32_t iy = (int32_t)((v>> 8) & 0xFFFFF000); // bits 20-39 << 12
        int32_t iz = (int32_t)((v << 12) & 0xFFFFF000); // bits 0-19 << 12

        fv[0] = (float)ix * exp;
        fv[1] = (float)iy * exp;
        fv[2] = (float)iz * exp;

        //DPRINTF("%f %f %f (%d %d %d, %d)\n", fv[0], fv[1], fv[2], ix>>12, iy>>12, iz>>12, (uint32_t)((v >> 60) & 0xF));
    }
}

void G1AKeyframe::UnpackVector(uint64_t v, float &x, float &y, float &z)
{
    float fv[3];
    UnpackVector(v, fv);
    x = fv[0], y = fv[1], z = fv[2];
}

void G1AKeyframe::UnpackVectors(float *fu, float *fv, float *fw, float *ft) const
{
    if (fu)
    {
        UnpackVector(u, fu);
    }

    if (fv)
    {
        UnpackVector(v, fv);
    }

    if (fw)
    {
        UnpackVector(w, fw);
    }

    if (ft)
    {
        UnpackVector(t, ft);
    }
}

uint64_t G1AKeyframe::PackVector(float *fv)
{
    uint64_t ret = 0;
    bool found = false;
    uint32_t i_exp = 0;
    float exp = 0.0f;

    // Choose smaller exponent we can use
    for (int i = 0; i < 16; i++)
    {
        found = true;

        for (int j = 0; j < 3; j++)
        {
            uint32_t u_exp = (i << 23) + 0x32000000;
            exp = *(float *)&u_exp;

            int32_t val = (int32_t)roundf((fv[j] / exp) / 4096.0f);
            if (val >= 524288 || val < -524288)
            {
                found = false;
                break;
            }
        }

        if (found)
        {
            i_exp = i;
            break;
        }
    }

    if (!found)
    {
        DPRINTF("No exponent suitable for %f %f %f!\n", fv[0], fv[1], fv[2]);
        return 0;
    }

    ret = ((uint64_t)i_exp) << 60;

    for (int i = 0; i < 3; i++)
    {
        int32_t val = (int32_t)roundf((fv[i] / exp) / 4096.0f);
        ret |= ((uint64_t)val&0xFFFFF) << (40-i*20);
    }

    return ret;
}

uint64_t G1AKeyframe::PackVector(float x, float y, float z)
{
    float fv[3];
    fv[0] = x, fv[1] = y, fv[2] = z;
    return PackVector(fv);
}

void G1AKeyframe::PackVectors(float *fu, float *fv, float *fw, float *ft)
{
    if (fu)
    {
        u = PackVector(fu);
    }

    if (fv)
    {
        v = PackVector(fv);
    }

    if (fw)
    {
        w = PackVector(fw);
    }

    if (ft)
    {
        t = PackVector(ft);
    }
}

void G1AKeyframe::GetXYZForFrame(float *xyz, uint16_t frame, uint16_t duration) const
{
    float f = (float)frame*65536.0f / (float)duration*65536.0f;
    float square_f = f*f;
    float cubic_f = square_f*f;

    float fu[3], fv[3], fw[3], ft[3];
    UnpackVectors(fu, fv, fw, ft);

    xyz[0] = fu[0] + fv[0]*f + fw[0]*square_f + ft[0]*cubic_f;
    xyz[1] = fu[1] + fv[1]*f + fw[1]*square_f + ft[1]*cubic_f;
    xyz[2] = fu[2] + fv[2]*f + fw[2]*square_f + ft[2]*cubic_f;
}

void G1AAnimation::AddFirstTransformLinear(int op, uint16_t duration, float ox, float oy, float oz, float tx, float ty, float tz, uint16_t start_time)
{
    std::vector<G1AKeyframe> *kfs;

    if (op == 0)
    {
        kfs = &rotation_kfs;
    }
    else if (op == 1)
    {
        kfs = &translation_kfs;
    }
    else
    {
        kfs = &scale_kfs;
    }

    G1AKeyframe kf1(start_time), kf2(start_time+duration);

    kf1.u = G1AKeyframe::PackVector(ox, oy, oz);
    kf1.v = G1AKeyframe::PackVector(tx, ty, tz);
    kf2.u = G1AKeyframe::PackVector(ox+tx, oy+ty, oz+tz);

    kfs->clear();
    kfs->push_back(kf1);
    kfs->push_back(kf2);
}

void G1AAnimation::AddTransformLinear(int op, uint16_t duration, float tx, float ty, float tz, uint16_t skip_time)
{
    std::vector<G1AKeyframe> *kfs;

    if (op == 0)
    {
        kfs = &rotation_kfs;
    }
    else if (op == 1)
    {
        kfs = &translation_kfs;
    }
    else
    {
        kfs = &scale_kfs;
    }

    if (kfs->size() == 0)
    {
        AddFirstTransformLinear(op, duration, 0, 0, 0, tx, ty, tz, skip_time);
        return;
    }

    G1AKeyframe &prev = kfs->back();
    G1AKeyframe kf(prev.time_start+duration+skip_time);

    float prev_x, prev_y, prev_z;
    G1AKeyframe::UnpackVector(prev.u, prev_x, prev_y, prev_z);

    prev.v = G1AKeyframe::PackVector(tx, ty, tz);
    kf.u = G1AKeyframe::PackVector(prev_x+tx, prev_y+ty, prev_z+tz);
    kfs->push_back(kf);
}

#ifdef DOA5_IMPORT_SUPPORT

uint16_t G1aFile::Doa5ToDoa6Bone(const std::string &doa5_name)
{
    if (doa5_name == "MOT00_Hips")
        return 2;

    /*if (doa5_name == "MOT01_Head")
        return 12;

    if (doa5_name == "MOT02_Chest")
        return 10; // 565; // ?

    if (doa5_name == "MOT03_LeftFoot")
        return 7;

    if (doa5_name == "MOT04_LeftForeArm")
        return 17;

    if (doa5_name == "MOT05_LeftHand")
        return 19;

    if (doa5_name == "MOT06_LeftUpLeg")
        return 3;

    if (doa5_name == "MOT07_LeftLeg")
        return 5;

    if (doa5_name == "MOT08_LeftArm")
        return 15;

    if (doa5_name == "MOT09_RightFoot")
        return 8;

    if (doa5_name == "MOT10_RightForeArm")
        return 18;

    if (doa5_name == "MOT11_RightHand")
        return 20;

    if (doa5_name == "MOT12_RightUpLeg")
        return 4;

    if (doa5_name == "MOT13_RightLeg")
        return 6;

    if (doa5_name == "MOT14_RightArm")
        return 16;

    if  (doa5_name == "MOT15_Neck")
        return 11;

    if (doa5_name == "MOT16_Waist")
        return 9; // ?

    if (doa5_name == "MOT17_LeftShoulder")
        return 13;

    if (doa5_name == "MOT18_LeftToe")
        return 23;

    if (doa5_name == "MOT19_RightShoulder")
        return 14;

    if (doa5_name == "MOT20_RightToe")
        return 24;*/

    return 0xFFFF;
}

static void euler_to_quaternion(float *q, const double *euler)
{
    double yaw = euler[2];
    double pitch = euler[1];
    double roll = euler[0];

    double cy = cos(yaw * 0.5);
    double sy = sin(yaw * 0.5);
    double cp = cos(pitch * 0.5);
    double sp = sin(pitch * 0.5);
    double cr = cos(roll * 0.5);
    double sr = sin(roll * 0.5);

    q[3] = (float)(cy * cp * cr + sy * sp * sr);
    q[0] = (float)(cy * cp * sr - sy * sp * cr);
    q[1] = (float)(sy * cp * sr + cy * sp * cr);
    q[2] = (float)(sy * cp * cr - cy * sp * sr);
}

static void euler_to_axis(float *axis, const double *euler)
{
    float q[4];

    euler_to_quaternion(q, euler);

    float angle = 2 * acosf(q[3]);
    if (1 - (q[3] * q[3]) < 0.000001f)
    {
        axis[0] = q[0];
        axis[1] = q[1];
        axis[2] = q[2];
    }
    else
    {
        float s = sqrtf(1 - (q[3] * q[3]));
        axis[0] = q[0] / s;
        axis[1] = q[1] / s;
        axis[2] = q[2] / s;
    }

    axis[0] = axis[0]*angle;
    axis[1] = axis[1]*angle;
    axis[2] = axis[2]*angle;
}

bool G1aFile::ImportFromCsv(const CsvFile &csv, float frame_rate)
{
    Reset();
    this->frame_rate = frame_rate;

    if (csv.GetNumAnims() == 0)
        return false;

    for (const CsvAnim &ca : csv)
    {
        G1AAnimation a;

        a.bone_id = Doa5ToDoa6Bone(ca.bone);
        if (a.bone_id == 0xFFFF)
        {
            DPRINTF("Cannot map bone %s, skipping it!\n", ca.bone.c_str());
            continue;
        }

        bool has_scale = false;

        for (const CsvFrame &cf : ca.frames)
        {
            G1AKeyframe r, t, s;

            r.time_start = t.time_start = s.time_start = (uint16_t)cf.frame;

            float xyz[3];
            double euler[3];

            euler[0] = cf.rotx; euler[1] = cf.roty; euler[2] = cf.rotz;
            euler_to_axis(xyz, euler);

            r.u = G1AKeyframe::PackVector(xyz[0], xyz[1], xyz[2]);
            t.u = G1AKeyframe::PackVector(cf.posx, cf.posy, cf.posz);
            s.u = G1AKeyframe::PackVector(cf.scalex, cf.scaley, cf.scalez);

            if (cf.scalex != 1.0f || cf.scaley != 1.0f || cf.scalez != 1.0f)
                has_scale = true;

            a.rotation_kfs.push_back(r);
            a.translation_kfs.push_back(t);
            a.scale_kfs.push_back(s);
        }

        if (!has_scale)
            a.scale_kfs.clear();       

        //DPRINTF("%s has_scale = %d\n", ca.bone.c_str(), has_scale);
        animations.push_back(a);
    }

    num_frames = (uint16_t)csv[0].frames.back().frame+1;
    return true;
}

#endif
