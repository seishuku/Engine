#version 430

layout(location=0) in vec4 vPosition;

uniform mat4 local;

void main()
{
	gl_Position=local*vPosition;
}
