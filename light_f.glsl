#version 430

in vec3 Position;
in vec2 UV;
in vec3 TangentX;
in vec3 TangentY;
in vec3 TangentZ;

layout(binding=0) uniform sampler2D TexBase;
layout(binding=1) uniform sampler2D TexSpec;
layout(binding=2) uniform sampler2D TexNorm;
layout(binding=3) uniform sampler2D TexReflect;

uniform mat4 mvinv;
uniform mat4 mvp;

uniform vec4 Light0_Pos;
uniform vec4 Light0_Kd;

layout(location=0) out vec4 Output;

const vec4 Matrix[12]=vec4[]
(
	vec4(+0.011737, +0.032676, +0.030911, +0.065802),
	vec4(+0.032676, -0.011737, +0.004615, +0.183023),
	vec4(+0.030911, +0.004615, -0.086244, +0.025234),
	vec4(+0.065802, +0.183023, +0.025234, +0.445843),
	vec4(-0.001528, +0.029016, +0.027102, +0.065680),
	vec4(+0.029016, +0.001528, -0.007317, +0.227819),
	vec4(+0.027102, -0.007317, -0.116121, +0.020076),
	vec4(+0.065680, +0.227819, +0.020076, +0.508944),
	vec4(-0.023472, +0.020561, +0.019608, +0.056267),
	vec4(+0.020561, +0.023472, -0.024419, +0.263672),
	vec4(+0.019608, -0.024419, -0.141788, +0.007638),
	vec4(+0.056267, +0.263672, +0.007638, +0.544789)
);

const float PI=3.1415926;

vec2 getUVLatLon(const vec3 v)
{
    float r=1.0/length(v);
    float phi=acos(v.y*r);
    float theta=atan(v.z*r, v.x*r);

    return vec2(0.5*(theta/PI)+0.5, -phi/PI);
}

vec2 getUVAngularMap(const vec3 v)
{
    float r=1.0/length(v);
    float phi=acos(v.z*r);
	float theta=atan(v.y*r, v.x*r);

	return 0.5*vec2((phi/PI)*cos(theta), (phi/PI)*sin(theta))+0.5;
}

void main()
{
	vec4 Base=texture(TexBase, UV);
	vec4 Specular=texture(TexSpec, UV);
	vec4 Normal=texture(TexNorm, UV);
//	vec3 n=normalize(vec3(TangentX.z, TangentY.z, TangentZ.z));
	vec3 n=normalize(vec3(dot(TangentX, Normal.xyz), dot(TangentY, Normal.xyz), dot(TangentZ, Normal.xyz)));
	vec3 e=normalize(mvinv[3].xyz-Position), r;

	vec3 l0=Light0_Pos.xyz-Position;

	float l0_atten=max(0.0, 1.0-length(l0*Light0_Pos.w));

	l0=normalize(l0);

	r=reflect(-e, n);

	vec3 l0_diffuse=Light0_Kd.rgb*max(0.0, dot(l0, n));
	vec3 l0_specular=vec3(1.0, 1.0, 1.0)*max(0.0, pow(dot(l0, r), 16.0)*dot(l0, n));

	vec3 Diffuse;
	vec4 Temp;

	Temp.x=dot(Matrix[0],  vec4(n, 1.0));
	Temp.y=dot(Matrix[1],  vec4(n, 1.0));
	Temp.z=dot(Matrix[2],  vec4(n, 1.0));
	Temp.w=dot(Matrix[3],  vec4(n, 1.0));
	Diffuse.x=dot(Temp,  vec4(n, 1.0));
	Temp.x=dot(Matrix[4],  vec4(n, 1.0));
	Temp.y=dot(Matrix[5],  vec4(n, 1.0));
	Temp.z=dot(Matrix[6],  vec4(n, 1.0));
	Temp.w=dot(Matrix[7],  vec4(n, 1.0));
	Diffuse.y=dot(Temp,  vec4(n, 1.0));
	Temp.x=dot(Matrix[8],  vec4(n, 1.0));
	Temp.y=dot(Matrix[9],  vec4(n, 1.0));
	Temp.z=dot(Matrix[10], vec4(n, 1.0));
	Temp.w=dot(Matrix[11], vec4(n, 1.0));
	Diffuse.z=dot(Temp,  vec4(n, 1.0));

	// Phong lighting
	Output=vec4((Base.xyz*l0_diffuse+l0_specular*Specular.xyz)*l0_atten, 1.0);

	// PBR lighting
//	Output=Base*pow(Diffuse.xyzz+texture(TexReflect, getUVAngularMap(r))*Specular,  vec4(0.45, 0.45, 0.45, 0.0));
}
