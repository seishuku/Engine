#version 430

layout(location=0) in vec3 vPosition;

uniform mat4 local;

void main()
{
	gl_Position=local*vec4(vPosition, 1.0);
}
