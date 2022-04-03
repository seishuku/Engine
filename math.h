#ifndef __MATH_H__
#define __MATH_H__

#include <math.h>

float fsinf(const float v);
float fcosf(const float v);
float ftanf(const float x);

float fact(const int n);

unsigned int NextPower2(unsigned int value);
int ComputeLog(unsigned int value);

float Dot3(const float a[3], const float b[3]);
float Dot4(const float a[4], const float b[4]);
float Distance(const float Vector1[3], const float Vector2[3]);
void Normalize3(float *v);
void Normalize4(float *v);
void Cross(const float v0[3], const float v1[3], float *n);

void QuatAngle(const float angle, const float x, const float y, const float z, float *out);
void QuatEuler(const float roll, const float pitch, const float yaw, float *out);
void QuatMultiply(const float a[4], const float b[4], float *out);
void QuatMatrix(const float in[4], float *out);

void MatrixIdentity(float *out);
void MatrixMult(const float a[16], const float b[16], float *out);
void MatrixInverse(const float in[16], float *out);
void MatrixRotate(const float angle, const float x, const float y, const float z, float *out);
void MatrixTranspose(const float in[16], float *out);
void MatrixTranslate(const float x, const float y, const float z, float *out);
void MatrixScale(const float x, const float y, const float z, float *out);
void Matrix4x4MultVec4(const float in[4], const float m[16], float *out);
void Matrix4x4MultVec3(const float in[3], const float m[16], float *out);
void Matrix3x3MultVec3(const float in[3], const float m[16], float *out);

void LookAt(const float position[3], const float forward[3], const float up[3], float *out);

void InfPerspective(float fovy, float aspect, float zNear, int flip, float *out);
void Perspective(float fovy, float aspect, float zNear, float zFar, int flip, float *out);
void Ortho(float left, float right, float bottom, float top, float zNear, float zFar, float *out);

#endif
