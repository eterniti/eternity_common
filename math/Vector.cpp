#include <math.h>
#include "Vector.h"

Vector2::Vector2(float x, float y) : x(x), y(y)
{
}

Vector2::Vector2(const float *ptr) : x(ptr[0]), y(ptr[1])
{
}

Vector2::Vector2(uint16_t x, uint16_t y)
{
    this->x = Utils::HalfToFloat(x);
    this->y = Utils::HalfToFloat(y);
}

Vector2::Vector2(const uint16_t *ptr) : Vector2(ptr[0], ptr[1])
{
}

float Vector2::Distance(const Vector2 &other) const
{
    float xx = x - other.x;
    float yy = y - other.y;
    return sqrtf(xx*xx + yy*yy);
}

void Vector2::Get(float *out) const
{
    out[0] = x;
    out[1] = y;
}

void Vector2::Get(uint16_t *out) const
{
    out[0] = Utils::FloatToHalf(x);
    out[1] = Utils::FloatToHalf(y);
}

Vector3::Vector3(float x, float y, float z) : x(x), y(y),  z(z)
{
}

Vector3::Vector3(const float *ptr) : x(ptr[0]), y(ptr[1]), z(ptr[2])
{
}

Vector3::Vector3(uint16_t x, uint16_t y, uint16_t z)
{
    this->x = Utils::HalfToFloat(x);
    this->y = Utils::HalfToFloat(y);
    this->z = Utils::HalfToFloat(z);
}

Vector3::Vector3(const uint16_t *ptr) : Vector3(ptr[0], ptr[1], ptr[2])
{
}

float Vector3::Distance(const Vector3 &other) const
{
    float xx = x - other.x;
    float yy = y - other.y;
    float zz = z - other.z;
    return sqrtf(xx*xx + yy*yy + zz*zz);
}

void Vector3::Get(float *out) const
{
    out[0] = x;
    out[1] = y;
    out[2] = z;
}

void Vector3::Get(uint16_t *out) const
{
    out[0] = Utils::FloatToHalf(x);
    out[1] = Utils::FloatToHalf(y);
    out[2] = Utils::FloatToHalf(z);
}

Vector4::Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w)
{
}

Vector4::Vector4(const float *ptr) : x(ptr[0]), y(ptr[1]), z(ptr[2]), w(ptr[3])
{
}

Vector4::Vector4(uint16_t x, uint16_t y, uint16_t z, uint16_t w)
{
    this->x = Utils::HalfToFloat(x);
    this->y = Utils::HalfToFloat(y);
    this->z = Utils::HalfToFloat(z);
    this->w = Utils::HalfToFloat(w);
}

Vector4::Vector4(const uint16_t *ptr) : Vector4(ptr[0], ptr[1], ptr[2], ptr[3])
{
}

float Vector4::Distance(const Vector4 &other) const
{
    float xx = x - other.x;
    float yy = y - other.y;
    float zz = z - other.z;
    float ww = w - other.w;
    return sqrtf(xx*xx + yy*yy + zz*zz + ww*ww);
}

void Vector4::Get(float *out) const
{
    out[0] = x;
    out[1] = y;
    out[2] = z;
    out[3] = w;
}

void Vector4::Get(uint16_t *out) const
{
    out[0] = Utils::FloatToHalf(x);
    out[1] = Utils::FloatToHalf(y);
    out[2] = Utils::FloatToHalf(z);
    out[3] = Utils::FloatToHalf(w);
}
