#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices=18) out;

uniform mat4 proj;
uniform mat4 mv[6];

layout(location=0) out vec4 vPosition;

void main()
{
    for(int i=0;i<6;i++)
    {
        gl_Layer=i;

        for(int j=0;j<3;j++)
        {
            vPosition=gl_in[j].gl_Position;
            gl_Position=proj*mv[i]*vPosition;

            EmitVertex();
        }

        EndPrimitive();
    }
}
