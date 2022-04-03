#ifndef _CAMERA_H_
#define _CAMERA_H_

typedef struct
{
	float Position[3];
	float Forward[3];
	float View[3];
	float Up[3];
	float Right[3];
	float Velocity[3];

	float PitchVelocity;
	float YawVelocity;

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

void CameraInit(Camera_t *Camera, float Position[3], float View[3], float Up[3]);
void CameraUpdate(Camera_t *Camera, float Time, float *out);
void CameraCheckCollision(Camera_t *Camera, float *Vertex, unsigned short *Face, int NumFace);
int CameraLoadPath(char *filename, CameraPath_t *Path);
void CameraInterpolatePath(CameraPath_t *Path, Camera_t *Camera, float TimeStep);
void CameraDeletePath(CameraPath_t *Path);

#endif
