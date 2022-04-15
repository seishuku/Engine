#include "opengl.h"

#ifndef BUFFER_OFFSET
#define BUFFER_OFFSET(x) ((char *)NULL+(x))
#endif

GLuint _QuadVAO, _QuadVBO;

void BuildQuadVBO(void)
{
	float verts[]={
		-1.0f, +1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 1.0f,
		+1.0f, +1.0f, 0.0f, 1.0f,
		+1.0f, -1.0f, 0.0f, 1.0f
	};

	glGenVertexArrays(1, &_QuadVAO);
	glBindVertexArray(_QuadVAO);

	glGenBuffers(1, &_QuadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _QuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*4, verts, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
}

void DrawQuad(void)
{
	glBindVertexArray(_QuadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
