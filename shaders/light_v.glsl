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
out vec2 UV;
out vec3 TangentX;
out vec3 TangentY;
out vec3 TangentZ;
out vec3 Eye;

void main()
{
	gl_Position=proj*mv*local*vec4(vPosition.xyz, 1.0);

	Position=(local*vec4(vPosition.xyz, 1.0)).xyz;
	UV=vUV.xy;

	TangentX.x=vTangent.x;
	TangentX.y=vBinormal.x;
	TangentX.z=vNormal.x;

	TangentY.x=vTangent.y;
	TangentY.y=vBinormal.y;
	TangentY.z=vNormal.y;

	TangentZ.x=vTangent.z;
	TangentZ.y=vBinormal.z;
	TangentZ.z=vNormal.z;
}
