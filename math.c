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
float Vec2_Dot(const float a[2], const float b[2])
{
	return (a[0]*b[0]+a[1]*b[1]);
}

float Vec3_Dot(const float a[3], const float b[3])
{
	return (a[0]*b[0]+a[1]*b[1]+a[2]*b[2]);
}

float Vec4_Dot(const float a[4], const float b[4])
{
	return (a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3]);
}

float Vec2_Length(const float Vector[2])
{
	return sqrtf(Vec2_Dot(Vector, Vector));
}

float Vec3_Length(const float Vector[3])
{
	return sqrtf(Vec3_Dot(Vector, Vector));
}

float Vec4_Length(const float Vector[4])
{
	return sqrtf(Vec4_Dot(Vector, Vector));
}

float Vec2_Distance(const float Vector1[2], const float Vector2[2])
{
	const float Vector[2]=
	{
		Vector2[0]-Vector1[0],
		Vector2[1]-Vector1[1]
	};

	return Vec2_Length(Vector);
}

float Vec3_Distance(const float Vector1[3], const float Vector2[3])
{
	const float Vector[3]=
	{
		Vector2[0]-Vector1[0],
		Vector2[1]-Vector1[1],
		Vector2[2]-Vector1[2]
	};

	return Vec3_Length(Vector);
}

float Vec4_Distance(const float Vector1[4], const float Vector2[4])
{
	const float Vector[4]=
	{
		Vector2[0]-Vector1[0],
		Vector2[1]-Vector1[1],
		Vector2[2]-Vector1[2],
		Vector2[3]-Vector1[3]
	};

	return Vec4_Length(Vector);
}

void Vec2_Normalize(float *v)
{
	if(!v)
		return;

	float mag=Vec2_Length(v);

	if(mag)
	{
		mag=1.0f/mag;

		v[0]*=mag;
		v[1]*=mag;
	}
}

void Vec3_Normalize(float *v)
{
	if(!v)
		return;

	float mag=Vec3_Length(v);

	if(mag)
	{
		mag=1.0f/mag;

		v[0]*=mag;
		v[1]*=mag;
		v[2]*=mag;
	}
}

void Vec4_Normalize(float *v)
{
	if(!v)
		return;

	float mag=Vec4_Length(v);

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

void Lerp(const float a, const float b, const float t, float *out)
{
	*out=t*(b-a)+a;
}

void Vec2_Lerp(const float a[2], const float b[2], const float t, float *out)
{
	out[0]=t*(b[0]-a[0])+a[0];
	out[1]=t*(b[1]-a[1])+a[1];
}

void Vec3_Lerp(const float a[3], const float b[3], const float t, float *out)
{
	out[0]=t*(b[0]-a[0])+a[0];
	out[1]=t*(b[1]-a[1])+a[1];
	out[2]=t*(b[2]-a[2])+a[2];
}

void Vec4_Lerp(const float a[4], const float b[4], const float t, float *out)
{
	out[0]=t*(b[0]-a[0])+a[0];
	out[1]=t*(b[1]-a[1])+a[1];
	out[2]=t*(b[2]-a[2])+a[2];
	out[3]=t*(b[3]-a[3])+a[3];
}

// Quaternion functions
void QuatAngle(const float angle, const float x, const float y, const float z, float *out)
{
	float v[3]={ x, y, z };
	float s=sinf(angle*0.5f);

	Vec3_Normalize(v);

	out[0]=s*v[0];
	out[1]=s*v[1];
	out[2]=s*v[2];
	out[3]=cosf(angle*0.5f);
}

void QuatAnglev(const float angle, const float v[3], float *out)
{
	QuatAngle(angle, v[0], v[1], v[2], out);
}

void QuatEuler(const float roll, const float pitch, const float yaw, float *out)
{
	float sr=sinf(roll*0.5f);
	float cr=cosf(roll*0.5f);

	float sp=sinf(pitch*0.5f);
	float cp=cosf(pitch*0.5f);

	float sy=sinf(yaw*0.5f);
	float cy=cosf(yaw*0.5f);

	out[0]=sr*cp*cy-cr*sp*sy;
	out[1]=cr*sp*cy+sr*cp*sy;
	out[2]=cr*cp*sy-sr*sp*cy;
	out[3]=cr*cp*cy+sr*sp*sy;
}

void QuatMultiply(const float a[4], const float b[4], float *out)
{
	float res[4];

	if(!out)
		return;

	res[0]=a[3]*b[0]+a[0]*b[3]+a[1]*b[2]-a[2]*b[1];
	res[1]=a[3]*b[1]-a[0]*b[2]+a[1]*b[3]+a[2]*b[0];
	res[2]=a[3]*b[2]+a[0]*b[1]-a[1]*b[0]+a[2]*b[3];
	res[3]=a[3]*b[3]-a[0]*b[0]-a[1]*b[1]-a[2]*b[2];

	memcpy(out, res, sizeof(float)*4);
}

void QuatInverse(float *q)
{
	float invNorm=1.0f/(q[0]*q[0]+q[1]*q[1]+q[2]*q[2]+q[3]*q[3]);

	q[0]=-q[0]*invNorm;
	q[1]=-q[1]*invNorm;
	q[2]=-q[2]*invNorm;
	q[3]=q[3]*invNorm;
}

void QuatRotate(const float q[4], const float v[3], float *out)
{
	float p[4]={ q[0], q[1], q[2], q[3] };
	float u[3], v1[3], v2[3];
	float s;

	if(!out)
		return;

	Vec4_Normalize(p);

	u[0]=p[0];
	u[1]=p[1];
	u[2]=p[2];
	s=p[3];

	v1[0]=u[0]*2.0f*Vec3_Dot(u, v)+v[0]*s*s-Vec3_Dot(u, u);
	v1[1]=u[1]*2.0f*Vec3_Dot(u, v)+v[1]*s*s-Vec3_Dot(u, u);
	v1[2]=u[2]*2.0f*Vec3_Dot(u, v)+v[2]*s*s-Vec3_Dot(u, u);

	Cross(u, v, v2);

	out[0]=v2[0]*2.0f*s+v1[0];
	out[1]=v2[1]*2.0f*s+v1[1];
	out[2]=v2[2]*2.0f*s+v1[2];
}

void QuatSlerp(const float qa[4], const float qb[4], float *out, const float t)
{
	// Check for out-of range parameter and return edge points if so
	if(t<=0.0)
	{
		memcpy(out, qa, sizeof(float[4]));
		return;
	}

	if(t>=1.0)
	{
		memcpy(out, qb, sizeof(float[4]));
		return;
	}

	// Compute "cosine of angle between quaternions" using dot product
	float cosOmega=(qa[0]*qb[0])+(qa[1]*qb[1])+(qa[2]*qb[2])+(qa[3]*qb[3]);

	// If negative dot, use -q1.  Two quaternions q and -q represent the same rotation, but may produce different slerp.
	// We chose q or -q to rotate using the acute angle.
	float q1[4]={ qb[0], qb[1], qb[2], qb[3] };

	if(cosOmega<0.0f)
	{
		q1[0]=-q1[0];
		q1[1]=-q1[1];
		q1[2]=-q1[2];
		q1[3]=-q1[3];
		cosOmega=-cosOmega;
	}

	// Compute interpolation fraction, checking for quaternions almost exactly the same
	float k0, k1;

	if(cosOmega>0.9999f)
	{
		// Very close - just use linear interpolation, which will protect againt a divide by zero

		k0=1.0f-t;
		k1=t;
	}
	else
	{
		// Compute the sin of the angle using the trig identity sin^2(omega) + cos^2(omega) = 1
		float sinOmega=sqrtf(1.0f-(cosOmega*cosOmega));

		// Compute the angle from its sine and cosine
		float omega=atan2f(sinOmega, cosOmega);

		// Compute inverse of denominator, so we only have to divide once
		float oneOverSinOmega=1.0f/sinOmega;

		// Compute interpolation parameters
		k0=sinf((1.0f-t)*omega)*oneOverSinOmega;
		k1=sinf(t*omega)*oneOverSinOmega;
	}

	// Interpolate and return new quaternion
	out[0]=(k0*qa[0])+(k1*q1[0]);
	out[1]=(k0*qa[1])+(k1*q1[1]);
	out[2]=(k0*qa[2])+(k1*q1[2]);
	out[3]=(k0*qa[3])+(k1*q1[3]);
}

void QuatMatrix(const float q[4], float *out)
{
	float m[16], s=0.0f;
	float norm=sqrtf(Vec4_Dot(q, q));

	if(norm>0.0f)
		s=2.0f/norm;

	float xx=s*q[0]*q[0];
	float xy=s*q[0]*q[1];
	float xz=s*q[0]*q[2];
	float yy=s*q[1]*q[1];
	float yz=s*q[1]*q[2];
	float zz=s*q[2]*q[2];
	float wx=s*q[3]*q[0];
	float wy=s*q[3]*q[1];
	float wz=s*q[3]*q[2];

	m[0]=1.0f-yy-zz;
	m[1]=xy+wz;
	m[2]=xz-wy;
	m[3]=0.0f;
	m[4]=xy-wz;
	m[5]=1.0f-xx-zz;
	m[6]=yz+wx;
	m[7]=0.0f;
	m[8]=xz+wy;
	m[9]=yz-wx;
	m[10]=1.0f-xx-yy;
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
	float c=cosf(angle);
	float s=sinf(angle);

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

void MatrixRotatev(const float angle, const float v[3], float *out)
{
	MatrixRotate(angle, v[0], v[1], v[2], out);
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

void MatrixTranslatev(const float v[3], float *out)
{
	MatrixTranslate(v[0], v[1], v[2], out);
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

void MatrixScalev(const float v[3], float *out)
{
	MatrixScale(v[0], v[1], v[2], out);
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

	Vec3_Normalize(u);
	Vec3_Normalize(f);
	Cross(f, u, s);
	Vec3_Normalize(s);
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
	out[12]=-Vec3_Dot(s, position);
	out[13]=-Vec3_Dot(u, position);
	out[14]=Vec3_Dot(f, position);
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
