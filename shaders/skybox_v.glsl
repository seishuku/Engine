#version 430

layout(location=0) in vec4 vPosition;
layout(location=1) in vec3 vUVW;

uniform mat4 proj;
uniform mat4 mv;

out vec3 UVW;

void main()
{
	gl_Position=proj*mv*vPosition;
	UVW=vUVW;
}
