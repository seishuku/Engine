#version 430

layout(location=0) in vec4 vPosition;
layout(location=1) in vec3 vUVW;

uniform mat4 mvp;

out vec3 UVW;

void main()
{
	gl_Position=mvp*vPosition;
	UVW=vUVW;
}
