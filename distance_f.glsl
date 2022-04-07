#version 330

in vec3 Position;

uniform vec4 Light_Pos;

void main()
{
	gl_FragDepth=length((Light_Pos.xyz-Position)*Light_Pos.w);
}
