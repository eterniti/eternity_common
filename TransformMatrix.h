#ifndef TRANSFORMMATRIX_H
#define TRANSFORMMATRIX_H

#include "Utils.h"

// scale->rotate->translate

class TransformMatrix
{
private:

    float m[4][4];

public:

    TransformMatrix();
    TransformMatrix(const void *buf);

    static const TransformMatrix FromRotation(float x, float y, float z, float w);
    static const TransformMatrix FromScaling(float x, float y, float z);
    static const TransformMatrix FromTranslation(float x, float y, float z);

    const TransformMatrix operator*(const TransformMatrix &rhs) const;
    const TransformMatrix operator*(float f) const;

    TransformMatrix Inverse() const;

    TransformMatrix Rotate(float x, float y, float z, float w);
    TransformMatrix RotateNeg(float x, float y, float z, float w);
    TransformMatrix Scale(float x, float y, float z);
    TransformMatrix Translate(float x, float y, float z);
    TransformMatrix TranslateNeg(float x, float y, float z);

    void CreateQuaternion(float *x, float *y, float *z, float *w);

    void GetRotation(float *x, float *y, float *z, float *w);
    void GetScale(float *x, float *y, float *z);
    void GetTranslation(float *x, float *y, float *z);

    TransformMatrix GetRotation();
    TransformMatrix GetScale();
    TransformMatrix GetTranslation();

    void Print() const;

    void CopyTo(void *buf);
};

#endif // TRANSFORMMATRIX_H
