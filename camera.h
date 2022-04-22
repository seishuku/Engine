#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "math.h"

typedef struct
{
	vec3 Position;
	vec3 Forward;
	vec3 View;
	vec3 Up;
	vec3 Right;
	vec3 Velocity;

	float Pitch;
	float Yaw;
	float Roll;

	float Radius;

	unsigned char key_w, key_s;
	unsigned char key_a, key_d;
	unsigned char key_v, key_c;
	unsigned char key_q, key_e;
	unsigned char key_up, key_down;
	unsigned char key_left, key_right;
} Camera_t;

typedef struct
{
	float Time;
	float EndTime;
	float *Position;
	float *View;
	int NumPoints;
	int *Knots;
} CameraPath_t;

void CameraInit(Camera_t *Camera, const vec3 Position, const vec3 View, const vec3 Up);
void CameraUpdate(Camera_t *Camera, float Time, matrix out);
void CameraCheckCollision(Camera_t *Camera, float *Vertex, unsigned short *Face, int NumFace);
int CameraLoadPath(char *filename, CameraPath_t *Path);
void CameraInterpolatePath(CameraPath_t *Path, Camera_t *Camera, float TimeStep, matrix out);
void CameraDeletePath(CameraPath_t *Path);

#endif
