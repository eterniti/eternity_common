#include "TransformMatrix.h"
#include <math.h>
#include "debug.h"

TransformMatrix::TransformMatrix()
{
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = 0.0f;
    m[1][0] = 0.0f;
    m[1][2] = 0.0f;
    m[1][3] = 0.0f;
    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][3] = 0.0f;
    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[0][0] = 1.0f;
    m[1][1] = 1.0f;
    m[2][2] = 1.0f;
    m[3][3] = 1.0f;
}

TransformMatrix::TransformMatrix(const void *buf)
{
    memcpy(m, buf, sizeof(m));
}

const TransformMatrix TransformMatrix::FromRotation(float x, float y, float z, float w)
{
    TransformMatrix ret;

    ret.m[0][0] = 1.0f - 2.0f * (y * y + z * z);
    ret.m[0][1] = 2.0f * (x *y + z * w);
    ret.m[0][2] = 2.0f * (x * z - y * w);
    ret.m[1][0] = 2.0f * (x * y - z * w);
    ret.m[1][1] = 1.0f - 2.0f * (x * x + z * z);
    ret.m[1][2] = 2.0f * (y *z + x *w);
    ret.m[2][0] = 2.0f * (x * z + y * w);
    ret.m[2][1] = 2.0f * (y *z - x *w);
    ret.m[2][2] = 1.0f - 2.0f * (x * x + y * y);

    return ret;
}

const TransformMatrix TransformMatrix::FromScaling(float x, float y, float z)
{
    TransformMatrix ret;

    ret.m[0][0] = x;
    ret.m[1][1] = y;
    ret.m[2][2] = z;

    return ret;
}

const TransformMatrix TransformMatrix::FromTranslation(float x, float y, float z)
{
    TransformMatrix ret;

    ret.m[3][0] = x;
    ret.m[3][1] = y;
    ret.m[3][2] = z;

    return ret;
}

const TransformMatrix TransformMatrix::operator*(const TransformMatrix &rhs) const
{
    TransformMatrix ret;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0 ; j < 4; j++)
        {
            ret.m[i][j] = this->m[i][0] * rhs.m[0][j] + this->m[i][1] * rhs.m[1][j] + this->m[i][2] * rhs.m[2][j] + this->m[i][3] * rhs.m[3][j];
        }
    }

    return ret;
}

const TransformMatrix TransformMatrix::operator*(float f) const
{
    TransformMatrix ret;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            ret.m[i][j] *= f;
        }
    }

    return ret;
}

TransformMatrix TransformMatrix::Inverse() const
{
    float det, t[3], v[16];
    TransformMatrix ret;

    t[0] = this->m[2][2] * this->m[3][3] - this->m[2][3] * this->m[3][2];
    t[1] = this->m[1][2] * this->m[3][3] - this->m[1][3] * this->m[3][2];
    t[2] = this->m[1][2] * this->m[2][3] - this->m[1][3] * this->m[2][2];
    v[0] = this->m[1][1] * t[0] - this->m[2][1] * t[1] + this->m[3][1] * t[2];
    v[4] = -this->m[1][0] * t[0] + this->m[2][0] * t[1] - this->m[3][0] * t[2];

    t[0] = this->m[1][0] * this->m[2][1] - this->m[2][0] * this->m[1][1];
    t[1] = this->m[1][0] * this->m[3][1] - this->m[3][0] * this->m[1][1];
    t[2] = this->m[2][0] * this->m[3][1] - this->m[3][0] * this->m[2][1];
    v[8] = this->m[3][3] * t[0] - this->m[2][3] * t[1] + this->m[1][3] * t[2];
    v[12] = -this->m[3][2] * t[0] + this->m[2][2] * t[1] - this->m[1][2] * t[2];

    det = this->m[0][0] * v[0] + this->m[0][1] * v[4] +
        this->m[0][2] * v[8] + this->m[0][3] * v[12];

    if (det == 0.0f)
    {
        DPRINTF("%s: determinant is 0!\n", FUNCNAME);
        exit(-1);
    }

    t[0] = this->m[2][2] * this->m[3][3] - this->m[2][3] * this->m[3][2];
    t[1] = this->m[0][2] * this->m[3][3] - this->m[0][3] * this->m[3][2];
    t[2] = this->m[0][2] * this->m[2][3] - this->m[0][3] * this->m[2][2];
    v[1] = -this->m[0][1] * t[0] + this->m[2][1] * t[1] - this->m[3][1] * t[2];
    v[5] = this->m[0][0] * t[0] - this->m[2][0] * t[1] + this->m[3][0] * t[2];

    t[0] = this->m[0][0] * this->m[2][1] - this->m[2][0] * this->m[0][1];
    t[1] = this->m[3][0] * this->m[0][1] - this->m[0][0] * this->m[3][1];
    t[2] = this->m[2][0] * this->m[3][1] - this->m[3][0] * this->m[2][1];
    v[9] = -this->m[3][3] * t[0] - this->m[2][3] * t[1]- this->m[0][3] * t[2];
    v[13] = this->m[3][2] * t[0] + this->m[2][2] * t[1] + this->m[0][2] * t[2];

    t[0] = this->m[1][2] * this->m[3][3] - this->m[1][3] * this->m[3][2];
    t[1] = this->m[0][2] * this->m[3][3] - this->m[0][3] * this->m[3][2];
    t[2] = this->m[0][2] * this->m[1][3] - this->m[0][3] * this->m[1][2];
    v[2] = this->m[0][1] * t[0] - this->m[1][1] * t[1] + this->m[3][1] * t[2];
    v[6] = -this->m[0][0] * t[0] + this->m[1][0] * t[1] - this->m[3][0] * t[2];

    t[0] = this->m[0][0] * this->m[1][1] - this->m[1][0] * this->m[0][1];
    t[1] = this->m[3][0] * this->m[0][1] - this->m[0][0] * this->m[3][1];
    t[2] = this->m[1][0] * this->m[3][1] - this->m[3][0] * this->m[1][1];
    v[10] = this->m[3][3] * t[0] + this->m[1][3] * t[1] + this->m[0][3] * t[2];
    v[14] = -this->m[3][2] * t[0] - this->m[1][2] * t[1] - this->m[0][2] * t[2];

    t[0] = this->m[1][2] * this->m[2][3] - this->m[1][3] * this->m[2][2];
    t[1] = this->m[0][2] * this->m[2][3] - this->m[0][3] * this->m[2][2];
    t[2] = this->m[0][2] * this->m[1][3] - this->m[0][3] * this->m[1][2];
    v[3] = -this->m[0][1] * t[0] + this->m[1][1] * t[1] - this->m[2][1] * t[2];
    v[7] = this->m[0][0] * t[0] - this->m[1][0] * t[1] + this->m[2][0] * t[2];

    v[11] = -this->m[0][0] * (this->m[1][1] * this->m[2][3] - this->m[1][3] * this->m[2][1]) +
        this->m[1][0] * (this->m[0][1] * this->m[2][3] - this->m[0][3] * this->m[2][1]) -
        this->m[2][0] * (this->m[0][1] * this->m[1][3] - this->m[0][3] * this->m[1][1]);

    v[15] = this->m[0][0] * (this->m[1][1] * this->m[2][2] - this->m[1][2] * this->m[2][1]) -
        this->m[1][0] * (this->m[0][1] * this->m[2][2] - this->m[0][2] * this->m[2][1]) +
        this->m[2][0] * (this->m[0][1] * this->m[1][2] - this->m[0][2] * this->m[1][1]);

    det = 1.0f / det;

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            ret.m[i][j] = v[4 * i + j] * det;

    return ret;
}

TransformMatrix TransformMatrix::Rotate(float x, float y, float z, float w)
{
    return *this * FromRotation(x, y, z, w);
}

TransformMatrix TransformMatrix::RotateNeg(float x, float y, float z, float w)
{
    return *this * FromRotation(-x, -y, -z, w);
}

TransformMatrix TransformMatrix::Scale(float x, float y, float z)
{
    return *this * FromScaling(x, y, z);
}

TransformMatrix TransformMatrix::Translate(float x, float y, float z)
{
    return *this * FromTranslation(x, y, z);
}

TransformMatrix TransformMatrix::TranslateNeg(float x, float y, float z)
{
    return Translate(-x, -y, -z);
}

void TransformMatrix::CreateQuaternion(float *x, float *y, float *z, float *w)
{
    float s, trace;

    trace = m[0][0] + m[1][1] + m[2][2] + 1.0f;
    if (trace > 1.0f)
    {
        s = 2.0f * sqrtf(trace);
        *x = (m[1][2] - m[2][1]) / s;
        *y = (m[2][0] - m[0][2]) / s;
        *z = (m[0][1] - m[1][0]) / s;
        *w = 0.25f * s;
    }
    else
    {
        int i, maxi = 0;

        for (i = 1; i < 3; i++)
        {
            if (m[i][i] > m[maxi][maxi])
                maxi = i;
        }

        switch (maxi)
        {
            case 0:
                s = 2.0f * sqrtf(1.0f + m[0][0] - m[1][1] - m[2][2]);
                *x = 0.25f * s;
                *y = (m[0][1] + m[1][0]) / s;
                *z = (m[0][2] + m[2][0]) / s;
                *w = (m[1][2] - m[2][1]) / s;
                break;

            case 1:
                s = 2.0f * sqrtf(1.0f + m[1][1] - m[0][0] - m[2][2]);
                *x = (m[0][1] + m[1][0]) / s;
                *y = 0.25f * s;
                *z = (m[1][2] + m[2][1]) / s;
                *w = (m[2][0] - m[0][2]) / s;
                break;

            case 2:
                s = 2.0f * sqrtf(1.0f + m[2][2] - m[0][0] - m[1][1]);
                *x = (m[0][2] + m[2][0]) / s;
                *y = (m[1][2] + m[2][1]) / s;
                *z = 0.25f * s;
                *w = (m[0][1] - m[1][0]) / s;
                break;
        }
    }
}

void TransformMatrix::GetRotation(float *x, float *y, float *z, float *w)
{
    TransformMatrix r;
    float sx, sy, sz;

    GetScale(&sx, &sy, &sz);

    r.m[0][0] = m[0][0]/sx;
    r.m[0][1] = m[0][1]/sx;
    r.m[0][2] = m[0][2]/sx;
    r.m[1][0] = m[1][0]/sy;
    r.m[1][1] = m[1][1]/sy;
    r.m[1][2] = m[1][2]/sy;
    r.m[2][0] = m[2][0]/sz;
    r.m[2][1] = m[2][1]/sz;
    r.m[2][2] = m[2][2]/sz;

    r.CreateQuaternion(x, y, z, w);
}

static inline float GetVectorLength(float x, float y, float z)
{
   return sqrt( (x) * (x) + (y) * (y) + (z) * (z) );
}

void TransformMatrix::GetScale(float *x, float *y, float *z)
{
    float vx, vy, vz;

    vx = m[0][0];
    vy = m[0][1];
    vz = m[0][2];
    *x = GetVectorLength(vx, vy, vz);

    vx = m[1][0];
    vy = m[1][1];
    vz = m[1][2];
    *y = GetVectorLength(vx, vy, vz);

    vx = m[2][0];
    vy = m[2][1];
    vz = m[2][2];
    *z = GetVectorLength(vx, vy, vz);
}

void TransformMatrix::GetTranslation(float *x, float *y, float *z)
{
    *x = m[3][0];
    *y = m[3][1];
    *z = m[3][2];
}

TransformMatrix TransformMatrix::GetRotation()
{
    float x, y, z, w;

    GetRotation(&x, &y, &z, &w);
    return FromRotation(x, y, z, w);
}

TransformMatrix TransformMatrix::GetScale()
{
    float x, y, z;

    GetScale(&x, &y, &z);
    return FromScaling(x, y, z);
}

TransformMatrix TransformMatrix::GetTranslation()
{
    float x, y, z;

    GetTranslation(&x, &y, &z);
    return FromTranslation(x, y, z);
}

void TransformMatrix::Print() const
{
    DPRINTF("%.7g %.7g %.7g %.7g\n", m[0][0], m[1][0], m[2][0], m[3][0]);
    DPRINTF("%.7g %.7g %.7g %.7g\n", m[0][1], m[1][1], m[2][1], m[3][1]);
    DPRINTF("%.7g %.7g %.7g %.7g\n", m[0][2], m[1][2], m[2][2], m[3][2]);
    DPRINTF("%.7g %.7g %.7g %.7g\n\n", m[0][3], m[1][3], m[2][3], m[3][3]);
}

void TransformMatrix::CopyTo(void *buf)
{
    memcpy(buf, m, sizeof(m));
}
