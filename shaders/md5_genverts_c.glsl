#version 430

layout(local_size_x=1) in;

struct Vertex_t
{
	vec2 st;
	int start;
	int count;
};

layout(std430, binding=0) buffer layoutVertices
{
	Vertex_t Vertices[];
};

struct Weight_t
{
	int joint;
	float bias;
	float pad[2];
	vec4 pos;
	vec4 tangent;
	vec4 binormal;
	vec4 normal;
};

layout(std430, binding=1) buffer layoutWeights
{
	Weight_t Weights[];
};

struct FinalVertex_t
{
	vec4 vertex;
	vec4 uv;
	vec4 tangent;
	vec4 binormal;
	vec4 normal;
};

layout(std430, binding=2) buffer layoutFinalVertices
{
	FinalVertex_t FinalVertex[];
};

struct Skeleton_t
{
	vec4 pos;
	vec4 orient;
};

layout(std430, binding=3) buffer layoutSkeleton
{
	Skeleton_t skeleton[];
};

layout(location=0) uniform int numVerts;

vec4 Quat_Inverse(const vec4 q)
{
	float invNorm=1.0f/dot(q, q);

	return vec4(-q.xyz*invNorm, q.w*invNorm);
}

vec4 Quat_multVec(const vec4 q, const vec3 v)
{
	return vec4(
		(q.w*v.x)+(q.y*v.z)-(q.z*v.y),
		(q.w*v.y)+(q.z*v.x)-(q.x*v.z),
		(q.w*v.z)+(q.x*v.y)-(q.y*v.x),
		-(q.x*v.x)-(q.y*v.y)-(q.z*v.z)
	);
}

vec4 Quat_multQuat(const vec4 qa, const vec4 qb)
{
	return vec4(
		(qa.x*qb.w)+(qa.w*qb.x)+(qa.y*qb.z)-(qa.z*qb.y),
		(qa.y*qb.w)+(qa.w*qb.y)+(qa.z*qb.x)-(qa.x*qb.z),
		(qa.z*qb.w)+(qa.w*qb.z)+(qa.x*qb.y)-(qa.y*qb.x),
		(qa.w*qb.w)-(qa.x*qb.x)-(qa.y*qb.y)-(qa.z*qb.z)
	);
}

vec4 Quat_rotatePoint(const vec4 q, const vec3 v)
{
	return Quat_multQuat(Quat_multVec(q, v), Quat_Inverse(q));
}

void main()
{
	int i=int(gl_GlobalInvocationID.x);

	FinalVertex[i].vertex=vec4(0.0);
	FinalVertex[i].uv=vec4(Vertices[i].st, 0.0, 0.0);
	FinalVertex[i].tangent=vec4(0.0);
	FinalVertex[i].binormal=vec4(0.0);
	FinalVertex[i].normal=vec4(0.0);

	for(int j=Vertices[i].start;j<Vertices[i].start+Vertices[i].count;j++)
	{
		vec4 temp;

		temp=Quat_rotatePoint(skeleton[Weights[j].joint].orient, Weights[j].pos.xyz);
		FinalVertex[i].vertex+=(skeleton[Weights[j].joint].pos+temp)*Weights[j].bias;
		FinalVertex[i].vertex.w=1.0;

		temp=Quat_rotatePoint(skeleton[Weights[j].joint].orient, Weights[j].tangent.xyz);
		FinalVertex[i].tangent+=temp*Weights[j].bias;
		FinalVertex[i].tangent.w=0.0;

		temp=Quat_rotatePoint(skeleton[Weights[j].joint].orient, Weights[j].binormal.xyz);
		FinalVertex[i].binormal+=temp*Weights[j].bias;
		FinalVertex[i].binormal.w=0.0;

		temp=Quat_rotatePoint(skeleton[Weights[j].joint].orient, Weights[j].normal.xyz);
		FinalVertex[i].normal+=temp*Weights[j].bias;
		FinalVertex[i].normal.w=0.0;
	}
}
