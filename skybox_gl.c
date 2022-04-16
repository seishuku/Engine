#include "opengl.h"

#ifndef BUFFER_OFFSET
#define BUFFER_OFFSET(x) ((char *)NULL+(x))
#endif

GLuint _SkyboxVAO, _SkyboxVertID, _SkyboxElemID;

void DrawSkybox(void)
{
	glBindVertexArray(_SkyboxVAO);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
}

void BuildSkyboxVBO(void)
{
	float scale=25.0f, w=1.0f;
	float SkyboxVerts[]=
	{
		+scale, -scale, -scale, w, +1.0f, -1.0f, -1.0f, //0 Right
		+scale, -scale, +scale, w, +1.0f, -1.0f, +1.0f,	//1
		+scale, +scale, +scale, w, +1.0f, +1.0f, +1.0f, //2
		+scale, +scale, -scale, w, +1.0f, +1.0f, -1.0f, //3
		-scale, -scale, +scale, w, -1.0f, -1.0f, +1.0f, //4 Left
		-scale, -scale, -scale, w, -1.0f, -1.0f, -1.0f, //5
		-scale, +scale, -scale, w, -1.0f, +1.0f, -1.0f, //6
		-scale, +scale, +scale, w, -1.0f, +1.0f, +1.0f, //7
		-scale, +scale, -scale, w, -1.0f, +1.0f, -1.0f, //8 Top
		+scale, +scale, -scale, w, +1.0f, +1.0f, -1.0f, //9
		+scale, +scale, +scale, w, +1.0f, +1.0f, +1.0f, //10
		-scale, +scale, +scale, w, -1.0f, +1.0f, +1.0f, //11
		-scale, -scale, +scale, w, -1.0f, -1.0f, +1.0f, //12 Bottom
		+scale, -scale, +scale, w, +1.0f, -1.0f, +1.0f, //13
		+scale, -scale, -scale, w, +1.0f, -1.0f, -1.0f, //14
		-scale, -scale, -scale, w, -1.0f, -1.0f, -1.0f, //15
		+scale, -scale, +scale, w, +1.0f, -1.0f, +1.0f, //16 Front
		-scale, -scale, +scale, w, -1.0f, -1.0f, +1.0f, //17
		-scale, +scale, +scale, w, -1.0f, +1.0f, +1.0f, //18
		+scale, +scale, +scale, w, +1.0f, +1.0f, +1.0f, //19
		-scale, -scale, -scale, w, -1.0f, -1.0f, -1.0f, //20 Back
		+scale, -scale, -scale, w, +1.0f, -1.0f, -1.0f, //21
		+scale, +scale, -scale, w, +1.0f, +1.0f, -1.0f, //22
		-scale, +scale, -scale, w, -1.0f, +1.0f, -1.0f  //23
	};
	unsigned short SkyboxTris[]=
	{
		 0,  1,  2,  3,  0, 2,	// Right
		 4,  5,  6,  7,  4, 6,	// Left
		 8,  9, 10, 11,  8, 10,	// Top
		12, 13, 14, 15, 12, 14,	// Bottom
		16, 17, 18, 19, 16, 18, // Front
		20, 21, 22, 23, 20, 22	// Back
	};

	glGenVertexArrays(1, &_SkyboxVAO);
	glBindVertexArray(_SkyboxVAO);

	// Generate vertex buffer object and bind it
	glGenBuffers(1, &_SkyboxVertID);
	glBindBuffer(GL_ARRAY_BUFFER, _SkyboxVertID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*24*7, SkyboxVerts, GL_STATIC_DRAW);

	// Set vertex attribute pointer layouts
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float)*7, BUFFER_OFFSET(0));	//Vertex
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*7, BUFFER_OFFSET(sizeof(float)*4));	//Vertex
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &_SkyboxElemID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _SkyboxElemID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*36, SkyboxTris, GL_STATIC_DRAW);

	glBindVertexArray(0);
}
