#version 330

layout(location=0) in vec3 vPosition;

uniform mat4 mvp;

out vec3 Position;

void main()
{
	gl_Position=mvp*vec4(vPosition, 1.0);

	Position=vPosition;
}
