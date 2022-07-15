#version 430

in vec4 Position;

uniform vec4 Light_Pos;

void main()
{
	// Instead of dividing by the far plane, I prefer using the set radius of the point light.
	gl_FragDepth=length(Position.xyz-Light_Pos.xyz)*Light_Pos.w;
}
