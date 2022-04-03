#include "opengl.h"
#include "math.h"
#include <stdio.h>
#include <malloc.h>
#include "camera.h"

#ifndef FREE
#define FREE(p) { if(p) { free(p); p=NULL; } }
#endif

int ClassifySphere(float Center[3], float Normal[3], float Point[3], float radius, float *distance)
{
	*distance=Dot3(Normal, Center)-Dot3(Normal, Point);

	if(fabsf(*distance)<radius)
		return 1;
	else
	{
		if(*distance>=radius)
			return 2;
	}

	return 0;
}

float AngleBetweenVectors(float Vector1[3], float Vector2[3])
{
	return acosf(Dot3(Vector1, Vector2)/(sqrtf(Dot3(Vector1, Vector1))*sqrtf(Dot3(Vector2, Vector2))));
}

int InsidePolygon(float Intersection[3], float Tri[3][3])
{
	float Angle=0.0f;
	float A[3], B[3], C[3];

	A[0]=Tri[0][0]-Intersection[0];
	A[1]=Tri[0][1]-Intersection[1];
	A[2]=Tri[0][2]-Intersection[2];

	B[0]=Tri[1][0]-Intersection[0];
	B[1]=Tri[1][1]-Intersection[1];
	B[2]=Tri[1][2]-Intersection[2];

	C[0]=Tri[2][0]-Intersection[0];
	C[1]=Tri[2][1]-Intersection[1];
	C[2]=Tri[2][2]-Intersection[2];

	Angle+=AngleBetweenVectors(A, B);
	Angle+=AngleBetweenVectors(B, C);
	Angle+=AngleBetweenVectors(C, A);

	if(Angle>=6.220353348f)
		return 1;

	return 0;
}

void ClosestPointOnLine(float A[3], float B[3], float Point[3], float *ClosestPoint)
{
	float Vector1[3]={ Point[0]-A[0], Point[1]-A[1], Point[2]-A[2] };
	float Vector2[3]={ B[0]-A[0], B[1]-A[1], B[2]-A[2] };
	float d=Distance(A, B), t;

	Normalize3(Vector2);

	t=Dot3(Vector1, Vector2);

	if(t<=0.0f)
	{
		ClosestPoint[0]=A[0];
		ClosestPoint[1]=A[1];
		ClosestPoint[2]=A[2];

		return;
	}

	if(t>=d)
	{
		ClosestPoint[0]=B[0];
		ClosestPoint[1]=B[1];
		ClosestPoint[2]=B[2];

		return;
	}

	ClosestPoint[0]=A[0]+(Vector2[0]*t);
	ClosestPoint[1]=A[1]+(Vector2[1]*t);
	ClosestPoint[2]=A[2]+(Vector2[2]*t);
}

int EdgeSphereCollision(float *Center, float Tri[3][3], float radius)
{
	int i;
	float Point[3], distance;

	for(i=0;i<3;i++)
	{
		ClosestPointOnLine(Tri[i], Tri[(i+1)%3], Center, Point);

		distance=Distance(Point, Center);

		if(distance<radius)
			return 1;
	}

	return 0;
}

void GetCollisionOffset(float Normal[3], float radius, float distance, float *Offset)
{
	if(distance>0.0f)
	{
		float distanceOver=radius-distance;

		Offset[0]=Normal[0]*distanceOver;
		Offset[1]=Normal[1]*distanceOver;
		Offset[2]=Normal[2]*distanceOver;

		return;
	}
	else
	{
		float distanceOver=radius+distance;

		Offset[0]=Normal[0]*-distanceOver;
		Offset[1]=Normal[1]*-distanceOver;
		Offset[2]=Normal[2]*-distanceOver;

		return;
	}

	Offset[0]=0.0f;
	Offset[1]=0.0f;
	Offset[2]=0.0f;
}

float Blend(int k, int t, int *knots, float v)
{
	float b;

	if(t==1)
	{
		if((knots[k]<=v)&&(v<knots[k+1]))
			b=1.0f;
		else
			b=0.0f;
	}
	else
	{
		if((knots[k+t-1]==knots[k])&&(knots[k+t]==knots[k+1]))
			b=0.0f;
		else
		{
			if(knots[k+t-1]==knots[k])
				b=(knots[k+t]-v)/(knots[k+t]-knots[k+1])*Blend(k+1, t-1, knots, v);
			else
			{
				if(knots[k+t]==knots[k+1])
					b=(v-knots[k])/(knots[k+t-1]-knots[k])*Blend(k, t-1, knots, v);
				else
					b=(v-knots[k])/(knots[k+t-1]-knots[k])*Blend(k, t-1, knots, v)+(knots[k+t]-v)/(knots[k+t]-knots[k+1])*Blend(k+1, t-1, knots, v);
			}
		}
	}

	return b;
}

void CalculateKnots(int *knots, int n, int t)
{
	int i;
	
	for(i=0;i<=n+t;i++)
	{
		if(i<t)
			knots[i]=0;
		else
		{
			if((t<=i)&&(i<=n))
				knots[i]=i-t+1;
			else
			{
				if(i>n)
					knots[i]=n-t+2;
			}
		}
	}
}

void CalculatePoint(int *knots, int n, int t, float v, float *control, float *output)
{
	int k;
	float b;

	output[0]=output[1]=output[2]=0.0f;

	for(k=0;k<=n;k++)
	{
		b=Blend(k, t, knots, v);

		output[0]+=control[3*k]*b;
		output[1]+=control[3*k+1]*b;
		output[2]+=control[3*k+2]*b;
	}
}

void CameraInit(Camera_t *Camera, float Position[3], float View[3], float Up[3])
{
	Camera->Position[0]=Position[0];
	Camera->Position[1]=Position[1];
	Camera->Position[2]=Position[2];

	Camera->View[0]=View[0];
	Camera->View[1]=View[1];
	Camera->View[2]=View[2];

	Camera->Up[0]=Up[0];
	Camera->Up[1]=Up[1];
	Camera->Up[2]=Up[2];

	Camera->PitchVelocity=0.0f;
	Camera->YawVelocity=0.0f;

	Camera->Radius=10.0f;

	Camera->Velocity[0]=0.0f;
	Camera->Velocity[1]=0.0f;
	Camera->Velocity[2]=0.0f;

	Camera->key_w=0;
	Camera->key_s=0;
	Camera->key_a=0;
	Camera->key_d=0;
	Camera->key_v=0;
	Camera->key_c=0;
	Camera->key_left=0;
	Camera->key_right=0;
	Camera->key_up=0;
	Camera->key_down=0;
}

void QuatVec3Multiply(const float in[3], const float q[4], float *out)
{
	float xxzz=q[0]*q[0]-q[2]*q[2];
	float wwyy=q[3]*q[3]-q[1]*q[1];
	float xw2=q[0]*q[3]*2.0f;
	float xy2=q[0]*q[1]*2.0f;
	float xz2=q[0]*q[2]*2.0f;
	float yw2=q[1]*q[3]*2.0f;
	float yz2=q[1]*q[2]*2.0f;
	float zw2=q[2]*q[3]*2.0f;
	float res[3];

	res[0]=(xxzz+wwyy)*in[0]+(xy2+zw2)*in[1]+(xz2-yw2)*in[2];
	res[1]=(xy2-zw2)*in[0]+(q[1]*q[1]+q[3]*q[3]-q[0]*q[0]-q[2]*q[2])*in[1]+(yz2+xw2)*in[2];
	res[2]=(xz2+yw2)*in[0]+(yz2-xw2)*in[1]+(wwyy-xxzz)*in[2];

	memcpy(out, res, sizeof(float)*3);
}


static float PitchTrack=0.0f;

void CameraUpdate(Camera_t *Camera, float Time, float *out)
{
	float speed=25.0f, m[16];
	float QuatYaw[4], QuatPitch[4], QuatFinal[4];
	float NewView[3];

	Camera->Forward[0]=Camera->View[0]-Camera->Position[0];
	Camera->Forward[1]=Camera->View[1]-Camera->Position[1];
	Camera->Forward[2]=Camera->View[2]-Camera->Position[2];
	Normalize3(Camera->Forward);

	Cross(Camera->Forward, Camera->Up, Camera->Right);
	Normalize3(Camera->Right);

	if(Camera->key_d)
		Camera->Velocity[0]+=Time;

	if(Camera->key_a)
		Camera->Velocity[0]-=Time;

	if(Camera->key_v)
		Camera->Velocity[1]+=Time;

	if(Camera->key_c)
		Camera->Velocity[1]-=Time;

	if(Camera->key_w)
		Camera->Velocity[2]+=Time;

	if(Camera->key_s)
		Camera->Velocity[2]-=Time;

	if(Camera->key_left)
		Camera->YawVelocity+=Time*0.188f;

	if(Camera->key_right)
		Camera->YawVelocity-=Time*0.188f;

	if(Camera->key_up)
		Camera->PitchVelocity+=Time*0.188f;

	if(Camera->key_down)
		Camera->PitchVelocity-=Time*0.188f;

	Camera->Velocity[0]*=0.91f;
	Camera->Velocity[1]*=0.91f;
	Camera->Velocity[2]*=0.91f;

	Camera->YawVelocity*=0.91f;
	Camera->PitchVelocity*=0.91f;

	PitchTrack-=Camera->PitchVelocity;

	if(PitchTrack>1.57f)
		PitchTrack=1.57f;
	else
	{
		if(PitchTrack<-1.57f)
			PitchTrack=-1.57f;
		else
			QuatAngle(Camera->PitchVelocity, Camera->Right[0], Camera->Right[1], Camera->Right[2], QuatPitch);
	}

	QuatAngle(Camera->YawVelocity, Camera->Up[0], Camera->Up[1], Camera->Up[2], QuatYaw);

	QuatMultiply(QuatPitch, QuatYaw, QuatFinal);

	MatrixIdentity(m);
	QuatMatrix(QuatFinal, m);
	Matrix3x3MultVec3(Camera->Forward, m, NewView);

	Camera->View[0]=Camera->Position[0]+NewView[0];
	Camera->View[1]=Camera->Position[1]+NewView[1];
	Camera->View[2]=Camera->Position[2]+NewView[2];

	Camera->Position[0]+=Camera->Right[0]*speed*Camera->Velocity[0];
	Camera->Position[1]+=Camera->Right[1]*speed*Camera->Velocity[0];
	Camera->Position[2]+=Camera->Right[2]*speed*Camera->Velocity[0];
	Camera->View[0]+=Camera->Right[0]*speed*Camera->Velocity[0];
	Camera->View[1]+=Camera->Right[1]*speed*Camera->Velocity[0];
	Camera->View[2]+=Camera->Right[2]*speed*Camera->Velocity[0];

	Camera->Position[0]+=Camera->Up[0]*speed*Camera->Velocity[1];
	Camera->Position[1]+=Camera->Up[1]*speed*Camera->Velocity[1];
	Camera->Position[2]+=Camera->Up[2]*speed*Camera->Velocity[1];
	Camera->View[0]+=Camera->Up[0]*speed*Camera->Velocity[1];
	Camera->View[1]+=Camera->Up[1]*speed*Camera->Velocity[1];
	Camera->View[2]+=Camera->Up[2]*speed*Camera->Velocity[1];

	Camera->Position[0]+=Camera->Forward[0]*speed*Camera->Velocity[2];
	Camera->Position[1]+=Camera->Forward[1]*speed*Camera->Velocity[2];
	Camera->Position[2]+=Camera->Forward[2]*speed*Camera->Velocity[2];
	Camera->View[0]+=Camera->Forward[0]*speed*Camera->Velocity[2];
	Camera->View[1]+=Camera->Forward[1]*speed*Camera->Velocity[2];
	Camera->View[2]+=Camera->Forward[2]*speed*Camera->Velocity[2];

	LookAt(Camera->Position, Camera->View, Camera->Up, m);
	MatrixMult(m, out, out);
}

void CameraCheckCollision(Camera_t *Camera, float *Vertex, unsigned short *Face, int NumFace)
{
	unsigned short i;
	int classification;
	float distance=0.0f;
	float v0[3], v1[3], n[3];

	for(i=0;i<NumFace;i++)
	{
		float Tri[3][3]=
		{
			{ Vertex[3*Face[3*i+0]], Vertex[3*Face[3*i+0]+1], Vertex[3*Face[3*i+0]+2] },
			{ Vertex[3*Face[3*i+1]], Vertex[3*Face[3*i+1]+1], Vertex[3*Face[3*i+1]+2] },
			{ Vertex[3*Face[3*i+2]], Vertex[3*Face[3*i+2]+1], Vertex[3*Face[3*i+2]+2] }
		};

		v0[0]=Tri[1][0]-Tri[0][0];
		v0[1]=Tri[1][1]-Tri[0][1];
		v0[2]=Tri[1][2]-Tri[0][2];

		v1[0]=Tri[2][0]-Tri[0][0];
		v1[1]=Tri[2][1]-Tri[0][1];
		v1[2]=Tri[2][2]-Tri[0][2];

		n[0]=(v0[1]*v1[2]-v0[2]*v1[1]);
		n[1]=(v0[2]*v1[0]-v0[0]*v1[2]);
		n[2]=(v0[0]*v1[1]-v0[1]*v1[0]);

		Normalize3(n);

		classification=ClassifySphere(Camera->Position, n, Tri[0], Camera->Radius, &distance);

		if(classification==1)
		{
			float Offset[3]={ n[0]*distance, n[1]*distance, n[2]*distance };
			float Intersection[3]={ Camera->Position[0]-Offset[0], Camera->Position[1]-Offset[1], Camera->Position[2]-Offset[2] };

			if(InsidePolygon(Intersection, Tri)||EdgeSphereCollision(Camera->Position, Tri, Camera->Radius*0.5f))
			{
				GetCollisionOffset(n, Camera->Radius, distance, Offset);

				Camera->Position[0]+=Offset[0];
				Camera->Position[1]+=Offset[1];
				Camera->Position[2]+=Offset[2];

				Camera->View[0]+=Offset[0];
				Camera->View[1]+=Offset[1];
				Camera->View[2]+=Offset[2];
			}
		}
	}
}

int CameraLoadPath(char *filename, CameraPath_t *Path)
{
	FILE *stream;
	int i;

	Path->NumPoints=0;

	if((stream=fopen(filename, "rt"))==NULL)
		return 0;

	if(fscanf(stream, "%d", &Path->NumPoints)!=1)
	{
		fclose(stream);
		return 0;
	}

	Path->Position=(float *)malloc(sizeof(float)*Path->NumPoints*3);

	if(Path->Position==NULL)
	{
		fclose(stream);

		return 0;
	}

	Path->View=(float *)malloc(sizeof(float)*Path->NumPoints*3);

	if(Path->View==NULL)
	{
		FREE(Path->Position);
		fclose(stream);

		return 0;
	}

	for(i=0;i<Path->NumPoints;i++)
	{
		if(fscanf(stream, "%f %f %f %f %f %f", &Path->Position[3*i], &Path->Position[3*i+1], &Path->Position[3*i+2], &Path->View[3*i], &Path->View[3*i+1], &Path->View[3*i+2])!=6)
		{
			FREE(Path->Position);
			FREE(Path->View);
			fclose(stream);

			return 0;
		}
	}

	fclose(stream);

	Path->Time=0.0f;
	Path->EndTime=(float)(Path->NumPoints-2);

	Path->Knots=(int *)malloc(sizeof(int)*Path->NumPoints*3);

	if(Path->Knots==NULL)
	{
		FREE(Path->Position);
		FREE(Path->View);
		fclose(stream);

		return 0;
	}

	CalculateKnots(Path->Knots, Path->NumPoints-1, 3);

	return 1;
}

void CameraInterpolatePath(CameraPath_t *Path, Camera_t *Camera, float TimeStep)
{
	Path->Time+=TimeStep;

	if(Path->Time>Path->EndTime)
		Path->Time=0.0f;

	CalculatePoint(Path->Knots, Path->NumPoints-1, 3, Path->Time, Path->Position, Camera->Position);
	CalculatePoint(Path->Knots, Path->NumPoints-1, 3, Path->Time, Path->View, Camera->View);
}

void CameraDeletePath(CameraPath_t *Path)
{
	FREE(Path->Position);
	FREE(Path->View);
}