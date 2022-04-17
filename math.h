#ifndef __MATH_H__
#define __MATH_H__

#include <math.h>

#ifndef PI
#define PI 3.1415926f
#endif

float fsinf(const float v);
float fcosf(const float v);
float ftanf(const float x);

float fact(const int n);

unsigned int NextPower2(unsigned int value);
int ComputeLog(unsigned int value);

float Vec2_Dot(const float a[2], const float b[2]);
float Vec3_Dot(const float a[3], const float b[3]);
float Vec4_Dot(const float a[4], const float b[4]);
float Vec2_Length(const float Vector[2]);
float Vec3_Length(const float Vector[3]);
float Vec4_Length(const float Vector[4]);
float Vec2_Distance(const float Vector1[2], const float Vector2[2]);
float Vec3_Distance(const float Vector1[3], const float Vector2[3]);
float Vec4_Distance(const float Vector1[4], const float Vector2[4]);
void Vec2_Normalize(float *v);
void Vec3_Normalize(float *v);
void Vec4_Normalize(float *v);
void Cross(const float v0[3], const float v1[3], float *n);
void Lerp(const float a, const float b, const float t, float *out);
void Vec2_Lerp(const float a[2], const float b[2], const float t, float *out);
void Vec3_Lerp(const float a[3], const float b[3], const float t, float *out);
void Vec4_Lerp(const float a[4], const float b[4], const float t, float *out);

void QuatAngle(const float angle, const float x, const float y, const float z, float *out);
void QuatAnglev(const float angle, const float v[3], float *out);
void QuatEuler(const float roll, const float pitch, const float yaw, float *out);
void QuatMultiply(const float a[4], const float b[4], float *out);
void QuatInverse(float *q);
void QuatRotate(const float q[4], const float v[3], float *out);
void QuatSlerp(const float qa[4], const float qb[4], float *out, const float t);
void QuatMatrix(const float in[4], float *out);

void MatrixIdentity(float *out);
void MatrixMult(const float a[16], const float b[16], float *out);
void MatrixInverse(const float in[16], float *out);
void MatrixRotate(const float angle, const float x, const float y, const float z, float *out);
void MatrixRotatev(const float angle, const float v[3], float *out);
void MatrixTranspose(const float in[16], float *out);
void MatrixTranslate(const float x, const float y, const float z, float *out);
void MatrixTranslatev(const float v[3], float *out);
void MatrixScale(const float x, const float y, const float z, float *out);
void MatrixScalev(const float v[3], float *out);
void Matrix4x4MultVec4(const float in[4], const float m[16], float *out);
void Matrix4x4MultVec3(const float in[3], const float m[16], float *out);
void Matrix3x3MultVec3(const float in[3], const float m[16], float *out);

void LookAt(const float position[3], const float forward[3], const float up[3], float *out);

void InfPerspective(float fovy, float aspect, float zNear, int flip, float *out);
void Perspective(float fovy, float aspect, float zNear, float zFar, int flip, float *out);
void Ortho(float left, float right, float bottom, float top, float zNear, float zFar, float *out);

#endif
