#version 430

layout(location=0) in vec4 vPosition;
layout(location=1) in vec4 vUV;
layout(location=2) in vec4 vTangent;
layout(location=3) in vec4 vBinormal;
layout(location=4) in vec4 vNormal;

uniform mat4 proj;
uniform mat4 mv;
uniform mat4 local;

out vec3 Position;
out vec3 UV;
out mat3 Tangent;

void main()
{
	gl_Position=proj*mv*local*vPosition;

	Position=(local*vPosition).xyz;
	UV=vUV.xyz;

	Tangent=mat3(vTangent.xyz, vBinormal.xyz, vNormal.xyz);
}
