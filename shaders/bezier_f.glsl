#version 430

in vec4 Color;
	
layout(location=0) out vec4 Output;

void main()
{
	Output=vec4(Color);
}
