#ifndef VECTOR_H
#define VECTOR_H

#include "Utils.h"

class Vector2
{
private:

    float x, y;

public:

    Vector2() { x = y = 0.0f; }
    Vector2(float x, float y);
    Vector2(const float *ptr);
    Vector2(uint16_t x, uint16_t y);
    Vector2(const uint16_t *ptr);

    float Distance(const Vector2 &other) const;

    void Get(float *out) const;
    void Get(uint16_t *out) const;
};

class Vector3
{
private:

    float x, y, z;

public:

    Vector3() { x = y = z = 0.0f; }
    Vector3(float x, float y, float z);
    Vector3(const float *ptr);
    Vector3(uint16_t x, uint16_t y, uint16_t z);
    Vector3(const uint16_t *ptr);

    float Distance(const Vector3 &other) const;

    void Get(float *out) const;
    void Get(uint16_t *out) const;
};

class Vector4
{
private:

    float x, y, z, w;

public:

    Vector4() { x = y = z = w = 0.0f; }
    Vector4(float x, float y, float z, float w);
    Vector4(const float *ptr);
    Vector4(uint16_t x, uint16_t y, uint16_t z, uint16_t w);
    Vector4(const uint16_t *ptr);

    float Distance(const Vector4 &other) const;

    void Get(float *out) const;
    void Get(uint16_t *out) const;
};

#endif // VECTOR_H
