#version 330

in vec2 UV;

uniform sampler2D Font;

layout(location=0) out vec4 Output;

void main()
{
	Output=vec4(1.0, 1.0, 1.0, texture(Font, UV));
}
