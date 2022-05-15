#ifndef _CAMERA_H_
#define _CAMERA_H_

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

	uint8_t key_w, key_s;
	uint8_t key_a, key_d;
	uint8_t key_v, key_c;
	uint8_t key_q, key_e;
	uint8_t key_up, key_down;
	uint8_t key_left, key_right;
} Camera_t;

typedef struct
{
	float Time;
	float EndTime;
	float *Position;
	float *View;
	int32_t NumPoints;
	int32_t *Knots;
} CameraPath_t;

void CameraInit(Camera_t *Camera, const vec3 Position, const vec3 View, const vec3 Up);
void CameraUpdate(Camera_t *Camera, float Time, matrix out);
void CameraCheckCollision(Camera_t *Camera, float *Vertex, uint32_t *Face, int32_t NumFace);
int32_t CameraLoadPath(char *filename, CameraPath_t *Path);
void CameraInterpolatePath(CameraPath_t *Path, Camera_t *Camera, float TimeStep, matrix out);
void CameraDeletePath(CameraPath_t *Path);

#endif
