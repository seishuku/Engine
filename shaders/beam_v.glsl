#version 430

layout(location=0) in vec4 vPosition;
layout(location=1) in vec4 vUV;

layout(location=0) uniform mat4 proj;
layout(location=1) uniform mat4 mv;
layout(location=2) uniform mat4 local;

out vec3 Position;
out vec2 UV;

void main()
{
	gl_Position=proj*mv*local*vPosition;

	Position=(local*vPosition).xyz;

	UV=vUV.xy;
}
