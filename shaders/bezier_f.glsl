#version 430

in vec3 Position;
	
uniform mat4 mv;
uniform mat4 local;

layout(location=0) out vec4 Output;

void main()
{
	Output=vec4(1.0);
}
