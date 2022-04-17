#version 430

layout(location=0) in vec4 vPosition;
layout(location=1) in vec4 vUV;

uniform mat4 proj;
uniform mat4 mv;
uniform mat4 local;

out vec3 Position;
out vec2 UV;

void main()
{
	gl_Position=proj*mv*local*vPosition;

	Position=(local*vPosition).xyz;

	UV=vUV.xy;
}
