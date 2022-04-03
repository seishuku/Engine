#include <string.h>
#include "math.h"

float fsinf(const float v)
{
	float fx=v*0.1591549f+0.5f;
	float ix=fx-(float)floor(fx);
	float x=ix*6.2831852f-3.1415926f;
	float x2=x*x;
	float x3=x2*x;

	return x-x3/6.0f
			+x3*x2/120.0f
			-x3*x2*x2/5040.0f
			+x3*x2*x2*x2/362880.0f
			-x3*x2*x2*x2*x2/39916800.0f
			+x3*x2*x2*x2*x2*x2/6227020800.0f
			-x3*x2*x2*x2*x2*x2*x2/1307674279936.0f
			+x3*x2*x2*x2*x2*x2*x2*x2/355687414628352.0f;
}

float fcosf(const float v)
{
	float fx=v*0.1591549f+0.5f;
	float ix=fx-(float)floor(fx);
	float x=ix*6.2831852f-3.1415926f;
	float x2=x*x;
	float x4=x2*x2;

	return 1-x2/2.0f
			+x4/24.0f
			-x4*x2/720.0f
			+x4*x4/40320.0f
			-x4*x4*x2/3628800.0f
			+x4*x4*x4/479001600.0f
			-x4*x4*x4*x2/87178289152.0f
			+x4*x4*x4*x4/20922788478976.0f;
}

float ftanf(const float x)
{
	return fsinf(x)/fcosf(x);
}

// Misc functions
float fact(const int n)
{
	int i;
	float j=1.0f;

	for(i=1;i<n;i++)
		j*=i;

	return j;
}

unsigned int NextPower2(unsigned int value)
{
	value--;
	value|=value>>1;
	value|=value>>2;
	value|=value>>4;
	value|=value>>8;
	value|=value>>16;
	value++;

	return value;
}

int ComputeLog(unsigned int value)
{
	int i=0;

	if(value==0)
		return -1;

	for(;;)
	{
		if(value&1)
		{
			if(value!=1)
				return -1;

			return i;
		}

		value>>=1;
		i++;
	}
}

// Vector functions
float Dot3(const float a[3], const float b[3])
{
	return (a[0]*b[0]+a[1]*b[1]+a[2]*b[2]);
}

float Dot4(const float a[4], const float b[4])
{
	return (a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3]);
}

float Distance(const float Vector1[3], const float Vector2[3])
{
	const float Vector[3]={ Vector2[0]-Vector1[0], Vector2[1]-Vector1[1], Vector2[2]-Vector1[2] };

	return (float)sqrt(Dot3(Vector, Vector));
}

void Normalize3(float *v)
{
	float mag;

	if(!v)
		return;

	mag=sqrtf(Dot3(v, v));

	if(mag)
	{
		mag=1.0f/mag;

		v[0]*=mag;
		v[1]*=mag;
		v[2]*=mag;
	}
}

void Normalize4(float *v)
{
	float mag;

	if(!v)
		return;

	mag=sqrtf(Dot4(v, v));

	if(mag)
	{
		mag=1.0f/mag;

		v[0]*=mag;
		v[1]*=mag;
		v[2]*=mag;
		v[3]*=mag;
	}
}

void Cross(const float v0[3], const float v1[3], float *n)
{
	if(!n)
		return;

	n[0]=v0[1]*v1[2]-v0[2]*v1[1];
	n[1]=v0[2]*v1[0]-v0[0]*v1[2];
	n[2]=v0[0]*v1[1]-v0[1]*v1[0];
}

// Quaternion functions
//
//	FIX ME: Are these quat functions all wrong???
//
void QuatAngle(const float angle, const float x, const float y, const float z, float *out)
{
	float s, v[3]={ x, y, z };

	if(!out)
		return;

	Normalize3(v);

	s=sinf(angle);

	out[0]=cosf(angle);
	out[1]=x*s;
	out[2]=y*s;
	out[3]=z*s;
}

void QuatEuler(const float roll, const float pitch, const float yaw, float *out)
{
	float cr, cp, cy, sr, sp, sy, cpcy, spsy;

	if(!out)
		return;

	sr=sinf(roll*0.5f);
	cr=cosf(roll*0.5f);

	sp=sinf(pitch*0.5f);
	cp=cosf(pitch*0.5f);

	sy=sinf(yaw*0.5f);
	cy=cosf(yaw*0.5f);

	cpcy=cp*cy;
	spsy=sp*sy;

	out[0]=cr*cpcy+sr*spsy;
	out[1]=sr*cpcy-cr*spsy;
	out[2]=cr*sp*cy+sr*cp*sy;
	out[3]=cr*cp*sy-sr*sp*cy;
}

void QuatMultiply(const float a[4], const float b[4], float *out)
{
	float res[4];

	if(!out)
		return;

	res[0]=b[0]*a[0]-b[1]*a[1]-b[2]*a[2]-b[3]*a[3];
	res[1]=b[0]*a[1]+b[1]*a[0]+b[2]*a[3]-b[3]*a[2];
	res[2]=b[0]*a[2]-b[1]*a[3]+b[2]*a[0]+b[3]*a[1];
	res[3]=b[0]*a[3]+b[1]*a[2]-b[2]*a[1]+b[3]*a[0];

	//res[0]=a[3]*b[0]+a[0]*b[3]+a[1]*b[2]-a[2]*b[1];
	//res[1]=a[3]*b[1]+a[1]*b[3]+a[2]*b[0]-a[0]*b[2];
	//res[2]=a[3]*b[2]+a[2]*b[3]+a[0]*b[1]-a[1]*b[0];
	//res[3]=a[3]*b[3]-a[0]*b[0]-a[1]*b[1]-a[2]*b[2];

	memcpy(out, res, sizeof(float)*4);
}

void QuatMatrix(const float q[4], float *out)
{
	float m[16], in[4]={ q[0], q[1], q[2], q[3] };
	float xx, yy, zz;

	if(!out)
		return;

	Normalize4(in);

	xx=in[1]*in[1];
	yy=in[2]*in[2];
	zz=in[3]*in[3];

	m[ 0]=1.0f-2.0f*(yy+zz);
	m[ 1]=2.0f*(in[1]*in[2]+in[0]*in[3]);
	m[ 2]=2.0f*(in[1]*in[3]-in[0]*in[2]);
	m[ 3]=0.0f;
	m[ 4]=2.0f*(in[1]*in[2]-in[0]*in[3]);
	m[ 5]=1.0f-2.0f*(xx+zz);
	m[ 6]=2.0f*(in[2]*in[3]+in[0]*in[1]);
	m[ 7]=0.0f;
	m[ 8]=2.0f*(in[1]*in[3]+in[0]*in[2]);
	m[ 9]=2.0f*(in[2]*in[3]-in[0]*in[1]);
	m[10]=1.0f-2.0f*(xx+yy);
	m[11]=0.0f;
	m[12]=0.0f;
	m[13]=0.0f;
	m[14]=0.0f;
	m[15]=1.0f;

	MatrixMult(m, out, out);
}

// Matrix functions
void MatrixIdentity(float *out)
{
	if(!out)
		return;

	out[ 0]=1.0f;	out[ 1]=0.0f;	out[ 2]=0.0f;	out[ 3]=0.0f;
	out[ 4]=0.0f;	out[ 5]=1.0f;	out[ 6]=0.0f;	out[ 7]=0.0f;
	out[ 8]=0.0f;	out[ 9]=0.0f;	out[10]=1.0f;	out[11]=0.0f;
	out[12]=0.0f;	out[13]=0.0f;	out[14]=0.0f;	out[15]=1.0f;
}

void MatrixMult(const float a[16], const float b[16], float *out)
{
	float res[16];

	if(!out)
		return;

	res[ 0]=a[ 0]*b[ 0]+a[ 1]*b[ 4]+a[ 2]*b[ 8]+a[ 3]*b[12];
	res[ 1]=a[ 0]*b[ 1]+a[ 1]*b[ 5]+a[ 2]*b[ 9]+a[ 3]*b[13];
	res[ 2]=a[ 0]*b[ 2]+a[ 1]*b[ 6]+a[ 2]*b[10]+a[ 3]*b[14];
	res[ 3]=a[ 0]*b[ 3]+a[ 1]*b[ 7]+a[ 2]*b[11]+a[ 3]*b[15];
	res[ 4]=a[ 4]*b[ 0]+a[ 5]*b[ 4]+a[ 6]*b[ 8]+a[ 7]*b[12];
	res[ 5]=a[ 4]*b[ 1]+a[ 5]*b[ 5]+a[ 6]*b[ 9]+a[ 7]*b[13];
	res[ 6]=a[ 4]*b[ 2]+a[ 5]*b[ 6]+a[ 6]*b[10]+a[ 7]*b[14];
	res[ 7]=a[ 4]*b[ 3]+a[ 5]*b[ 7]+a[ 6]*b[11]+a[ 7]*b[15];
	res[ 8]=a[ 8]*b[ 0]+a[ 9]*b[ 4]+a[10]*b[ 8]+a[11]*b[12];
	res[ 9]=a[ 8]*b[ 1]+a[ 9]*b[ 5]+a[10]*b[ 9]+a[11]*b[13];
	res[10]=a[ 8]*b[ 2]+a[ 9]*b[ 6]+a[10]*b[10]+a[11]*b[14];
	res[11]=a[ 8]*b[ 3]+a[ 9]*b[ 7]+a[10]*b[11]+a[11]*b[15];
	res[12]=a[12]*b[ 0]+a[13]*b[ 4]+a[14]*b[ 8]+a[15]*b[12];
	res[13]=a[12]*b[ 1]+a[13]*b[ 5]+a[14]*b[ 9]+a[15]*b[13];
	res[14]=a[12]*b[ 2]+a[13]*b[ 6]+a[14]*b[10]+a[15]*b[14];
	res[15]=a[12]*b[ 3]+a[13]*b[ 7]+a[14]*b[11]+a[15]*b[15];

	memcpy(out, res, sizeof(float)*16);
}

void MatrixInverse(const float in[16], float *out)
{
	float res[16];

	if(!out)
		return;

	res[ 0]=in[ 0];
	res[ 1]=in[ 4];
	res[ 2]=in[ 8];
	res[ 3]=0.0f;
	res[ 4]=in[ 1];
	res[ 5]=in[ 5];
	res[ 6]=in[ 9];
	res[ 7]=0.0f;
	res[ 8]=in[ 2];
	res[ 9]=in[ 6];
	res[10]=in[10];
	res[11]=0.0f;
	res[12]=-(in[12]*in[ 0])-(in[13]*in[ 1])-(in[14]*in[ 2]);
	res[13]=-(in[12]*in[ 4])-(in[13]*in[ 5])-(in[14]*in[ 6]);
	res[14]=-(in[12]*in[ 8])-(in[13]*in[ 9])-(in[14]*in[10]);
	res[15]=1.0f;

	memcpy(out, res, sizeof(float)*16);
}

void MatrixTranspose(const float in[16], float *out)
{
	float res[16];

	if(!out)
		return;

	res[ 0]=in[ 0];
	res[ 1]=in[ 4];
	res[ 2]=in[ 8];
	res[ 3]=in[12];
	res[ 4]=in[ 1];
	res[ 5]=in[ 5];
	res[ 6]=in[ 9];
	res[ 7]=in[13];
	res[ 8]=in[ 2];
	res[ 9]=in[ 6];
	res[10]=in[10];
	res[11]=in[14];
	res[12]=in[ 3];
	res[13]=in[ 7];
	res[14]=in[11];
	res[15]=in[15];

	memcpy(out, res, sizeof(float)*16);
}

void MatrixRotate(const float angle, const float x, const float y, const float z, float *out)
{
	float m[16];
	float c=cos(angle);
	float s=sin(angle);

	float temp[3]={ (1.0f-c)*x, (1.0f-c)*y, (1.0f-c)*z };

	m[0]=c+temp[0]*x;
	m[1]=temp[0]*y+s*z;
	m[2]=temp[0]*z-s*y;
	m[3]=0.0f;
	m[4]=temp[1]*x-s*z;
	m[5]=c+temp[1]*y;
	m[6]=temp[1]*z+s*x;
	m[7]=0.0f;
	m[8]=temp[2]*x+s*y;
	m[9]=temp[2]*y-s*x;
	m[10]=c+temp[2]*z;
	m[11]=0.0f;
	m[12]=0.0f;
	m[13]=0.0f;
	m[14]=0.0f;
	m[15]=1.0f;

	MatrixMult(m, out, out);
}

void MatrixTranslate(const float x, const float y, const float z, float *out)
{
	float m[16];

	if(!out)
		return;

	m[ 0]=1.0f;	m[ 1]=0.0f;	m[ 2]=0.0f;	m[ 3]=0.0f;
	m[ 4]=0.0f;	m[ 5]=1.0f;	m[ 6]=0.0f;	m[ 7]=0.0f;
	m[ 8]=0.0f;	m[ 9]=0.0f;	m[10]=1.0f;	m[11]=0.0f;
	m[12]=x;	m[13]=y;	m[14]=z;	m[15]=1.0f;

	MatrixMult(m, out, out);
}

void MatrixScale(const float x, const float y, const float z, float *out)
{
	float m[16];

	if(!out)
		return;

	m[ 0]=x;	m[ 1]=0.0f;	m[ 2]=0.0f;	m[ 3]=0.0f;
	m[ 4]=0.0f;	m[ 5]=y;	m[ 6]=0.0f;	m[ 7]=0.0f;
	m[ 8]=0.0f;	m[ 9]=0.0f;	m[10]=z;	m[11]=0.0f;
	m[12]=0.0f;	m[13]=0.0f;	m[14]=0.0f;	m[15]=1.0f;

	MatrixMult(m, out, out);
}

void Matrix4x4MultVec4(const float in[4], const float m[16], float *out)
{
	float res[4];

	if(!out)
		return;

	res[0]=in[0]*m[ 0]+in[1]*m[ 4]+in[2]*m[ 8]+in[3]*m[12];
	res[1]=in[0]*m[ 1]+in[1]*m[ 5]+in[2]*m[ 9]+in[3]*m[13];
	res[2]=in[0]*m[ 2]+in[1]*m[ 6]+in[2]*m[10]+in[3]*m[14];
	res[3]=in[0]*m[ 3]+in[1]*m[ 7]+in[2]*m[11]+in[3]*m[15];

	memcpy(out, res, sizeof(float)*4);
}

void Matrix4x4MultVec3(const float in[3], const float m[16], float *out)
{
	float res[3];

	if(!out)
		return;

	res[0]=in[0]*m[ 0]+in[1]*m[ 4]+in[2]*m[ 8]+m[12];
	res[1]=in[0]*m[ 1]+in[1]*m[ 5]+in[2]*m[ 9]+m[13];
	res[2]=in[0]*m[ 2]+in[1]*m[ 6]+in[2]*m[10]+m[14];

	memcpy(out, res, sizeof(float)*3);
}

void Matrix3x3MultVec3(const float in[3], const float m[16], float *out)
{
	float res[3];

	if(!out)
		return;

	res[0]=in[0]*m[ 0]+in[1]*m[ 4]+in[2]*m[ 8];
	res[1]=in[0]*m[ 1]+in[1]*m[ 5]+in[2]*m[ 9];
	res[2]=in[0]*m[ 2]+in[1]*m[ 6]+in[2]*m[10];

	memcpy(out, res, sizeof(float)*3);
}

void LookAt(const float position[3], const float forward[3], const float up[3], float *out)
{
	float f[3]={ forward[0]-position[0], forward[1]-position[1], forward[2]-position[2] };
	float u[3]={ up[0], up[1], up[2] }, s[3];

	if(!out)
		return;

	Normalize3(u);
	Normalize3(f);
	Cross(f, u, s);
	Normalize3(s);
	Cross(s, f, u);

	out[0]=s[0];
	out[1]=u[0];
	out[2]=-f[0];
	out[3]=0.0f;
	out[4]=s[1];
	out[5]=u[1];
	out[6]=-f[1];
	out[7]=0.0f;
	out[8]=s[2];
	out[9]=u[2];
	out[10]=-f[2];
	out[11]=0.0f;
	out[12]=-Dot3(s, position);
	out[13]=-Dot3(u, position);
	out[14]=Dot3(f, position);
	out[15]=1.0f;
}

// Projection matrix functions
void InfPerspective(const float fovy, const float aspect, const float zNear, const int flip, float *out)
{
	float y=tanf((fovy/2.0f)*3.14159f/180.0f)*zNear, x=aspect*y;
	float nudge=1.0f-(1.0f/(1<<16));
	float m[16];

	if(!out)
		return;

	m[0]=zNear/x;
	m[1]=0.0f;
	m[2]=0.0f;
	m[3]=0.0f;
	m[4]=0.0f;
	m[5]=flip?-zNear/y:zNear/y;
	m[6]=0.0f;
	m[7]=0.0f;
	m[8]=0.0f;
	m[9]=0.0f;
	m[10]=-1.0f*nudge;
	m[11]=-1.0f;
	m[12]=0.0f;
	m[13]=0.0f;
	m[14]=-2.0f*zNear*nudge;
	m[15]=0.0f;

	MatrixMult(m, out, out);
}

void Perspective(const float fovy, const float aspect, const float zNear, const float zFar, const int flip, float *out)
{
	float y=tanf((fovy/2.0f)*3.14159f/180.0f)*zNear, x=aspect*y;
	float m[16];

	if(!out)
		return;

	m[0]=zNear/x;
	m[1]=0.0f;
	m[2]=0.0f;
	m[3]=0.0f;
	m[4]=0.0f;
	m[5]=flip?-zNear/y:zNear/y;
	m[6]=0.0f;
	m[7]=0.0f;
	m[8]=0.0f;
	m[9]=0.0f;
	m[10]=-(zFar+zNear)/(zFar-zNear);
	m[11]=-1.0f;
	m[12]=0.0f;
	m[13]=0.0f;
	m[14]=-(2.0f*zNear*zFar)/(zFar-zNear);
	m[15]=0.0f;

	MatrixMult(m, out, out);
}

void Ortho(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar, float *out)
{
	float m[16];

	MatrixIdentity(m);

	m[0]=2/(right-left);
	m[5]=2/(top-bottom);	
	m[10]=-2/(zFar-zNear);
	m[12]=-(right+left)/(right-left);
	m[13]=-(top+bottom)/(top-bottom);
	m[14]=-(zFar+zNear)/(zFar-zNear);

	MatrixMult(m, out, out);
}
