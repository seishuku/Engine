#version 430

layout(binding=0) uniform sampler2D image;

layout(location=0) out vec4 Output;

void main()
{
	Output=vec4(1.0);
}
