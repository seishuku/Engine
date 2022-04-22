#ifndef __MATH_H__
#define __MATH_H__

#include <math.h>

#ifndef PI
#define PI 3.1415926f
#endif

typedef float vec2[2];
typedef float vec3[3];
typedef float vec4[4];
typedef float matrix[16];

float fsinf(const float v);
float fcosf(const float v);
float ftanf(const float x);

float fact(const int n);

unsigned int NextPower2(unsigned int value);
int ComputeLog(unsigned int value);

float Vec2_Dot(const vec2 a, const vec2 b);
float Vec3_Dot(const vec3 a, const vec3 b);
float Vec4_Dot(const vec4 a, const vec4 b);
float Vec2_Length(const vec2 Vector);
float Vec3_Length(const vec3 Vector);
float Vec4_Length(const vec4 Vector);
float Vec2_Distance(const vec2 Vector1, const vec2 Vector2);
float Vec3_Distance(const vec3 Vector1, const vec3 Vector2);
float Vec4_Distance(const vec4 Vector1, const vec4 Vector2);
float Vec3_GetAngle(const vec3 Vector1, const vec3 Vector2);
void Vec2_Normalize(vec2 v);
void Vec3_Normalize(vec3 v);
void Vec4_Normalize(vec4 v);
void Cross(const vec3 v0, const vec3 v1, vec3 n);
void Lerp(const float a, const float b, const float t, float *out);
void Vec2_Lerp(const vec2 a, const vec2 b, const float t, vec2 out);
void Vec3_Lerp(const vec3 a, const vec3 b, const float t, vec3 out);
void Vec4_Lerp(const vec4 a, const vec4 b, const float t, vec4 out);

void QuatAngle(const float angle, const float x, const float y, const float z, vec4 out);
void QuatAnglev(const float angle, const vec3 v, vec4 out);
void QuatEuler(const float roll, const float pitch, const float yaw, vec4 out);
void QuatMultiply(const vec4 a, const vec4 b, vec4 out);
void QuatInverse(vec4 q);
void QuatRotate(const vec4 q, const vec3 v, vec3 out);
void QuatSlerp(const vec4 qa, const vec4 qb, vec4 out, const float t);
void QuatMatrix(const vec4 in, matrix out);

void MatrixIdentity(matrix out);
void MatrixMult(const matrix a, const matrix b, matrix out);
void MatrixInverse(const matrix in, matrix out);
void MatrixRotate(const float angle, const float x, const float y, const float z, matrix out);
void MatrixRotatev(const float angle, const vec3 v, matrix out);
void MatrixTranspose(const float in[16], matrix out);
void MatrixTranslate(const float x, const float y, const float z, matrix out);
void MatrixTranslatev(const vec3 v, matrix out);
void MatrixScale(const float x, const float y, const float z, matrix out);
void MatrixScalev(const vec3 v, matrix out);
void Matrix4x4MultVec4(const vec4 in, const matrix m, matrix out);
void Matrix4x4MultVec3(const vec3 in, const matrix m, matrix out);
void Matrix3x3MultVec3(const vec3 in, const matrix m, matrix out);

void LookAt(const vec3 position, const vec3 forward, const vec3 up, matrix out);

void InfPerspective(float fovy, float aspect, float zNear, int flip, matrix out);
void Perspective(float fovy, float aspect, float zNear, float zFar, int flip, matrix out);
void Ortho(float left, float right, float bottom, float top, float zNear, float zFar, matrix out);

#endif
