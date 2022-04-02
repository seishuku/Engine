#version 420

layout(binding=0) uniform sampler2D image;

layout(location=0) out vec4 Output;

void main()
{
	Output=texelFetch(image, ivec2(gl_FragCoord.xy), 0);
}
