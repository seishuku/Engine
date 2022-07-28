#version 430

layout(location=0) in vec4 vPosition;

void main()
{
	// Incoming vertices are actually control points
	gl_Position=vPosition;
}
