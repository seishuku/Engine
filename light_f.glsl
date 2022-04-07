#version 430

in vec3 Position;
in vec2 UV;
in vec3 TangentX;
in vec3 TangentY;
in vec3 TangentZ;

layout(binding=0) uniform sampler2D TexBase;
layout(binding=1) uniform sampler2D TexSpec;
layout(binding=2) uniform sampler2D TexNorm;
layout(binding=3) uniform samplerCube TexDistance0;
	
uniform mat4 mvinv;
uniform mat4 mvp;

uniform vec4 Light0_Pos;
uniform vec4 Light0_Kd;

layout(location=0) out vec4 Output;

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

	float Shadow0=((texture(TexDistance0, -l0).x+0.01)>=length(l0*Light0_Pos.w))?1.0:0.0;

	l0=normalize(l0);

	r=reflect(-e, n);

	vec3 l0_diffuse=Light0_Kd.rgb*max(0.0, dot(l0, n));
	vec3 l0_specular=vec3(1.0, 1.0, 1.0)*max(0.0, pow(dot(l0, r), 16.0)*dot(l0, n));

	Output=vec4((Base.xyz*l0_diffuse+l0_specular*Specular.xyz)*Shadow0*l0_atten, 1.0);
}
