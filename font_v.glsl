#version 330

layout(location=0) in vec4 vVert;

out vec2 UV;

void main()
{
	gl_Position=vec4(vVert.xy, -1.0, 1.0);
	UV=vVert.zw;
}
