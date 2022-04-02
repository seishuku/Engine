#include "opengl.h"
#include "3ds.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#define DBGPRINTF(...) { char buf[512]; snprintf(buf, sizeof(buf), __VA_ARGS__); OutputDebugString(buf); }
#else
#define DBGPRINTF(...) { fprintf(stderr, __VA_ARGS__); }
#endif

#ifndef BUFFER_OFFSET
#define BUFFER_OFFSET(x) ((char *)NULL+(x))
#endif

#ifndef FREE
#define FREE(p) { if(p) { free(p); p=NULL; } }
#endif

void DrawModel3DS(Model3DS_t *Model)
{
	for(int i=0;i<Model->NumMesh;i++)
	{
		glBindVertexArray(Model->Mesh[i].VAO);
		glDrawElements(GL_TRIANGLES, Model->Mesh[i].NumFace*3, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
	}

	glBindVertexArray(0);
}

void BuildVBO3DS(Model3DS_t *Model)
{
	for(int i=0;i<Model->NumMesh;i++)
	{
		float *data=NULL;

		if(!Model->Mesh[i].NumVertex)
			continue;

		// Generate vertex array object and bind it
		glGenVertexArrays(1, &Model->Mesh[i].VAO);
		glBindVertexArray(Model->Mesh[i].VAO);

		// Generate vertex buffer object and bind it
		glGenBuffers(1, &Model->Mesh[i].VertID);
		glBindBuffer(GL_ARRAY_BUFFER, Model->Mesh[i].VertID);

		// Set vertex attribute pointer layouts
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float)*16, BUFFER_OFFSET(sizeof(float)*0));	//Vertex
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*16, BUFFER_OFFSET(sizeof(float)*4));	//UV
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float)*16, BUFFER_OFFSET(sizeof(float)*7));	//TANGENT
		glEnableVertexAttribArray(2);

		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(float)*16, BUFFER_OFFSET(sizeof(float)*10));	//BINORMAL
		glEnableVertexAttribArray(3);

		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(float)*16, BUFFER_OFFSET(sizeof(float)*13));	//NORMAL
		glEnableVertexAttribArray(4);

		// Allocate a temp buffer in system memory
		data=(float *)malloc(sizeof(float)*Model->Mesh[i].NumVertex*16);

		if(data==NULL)
		{
			DBGPRINTF("VBO data buffer memory map failed or out of memory for object: %s", Model->Mesh[i].Name);

			glDeleteBuffers(1, &Model->Mesh[i].VertID);
			Model->Mesh[i].VertID=0;
			break;
		}

		float *fPtr=data;

		for(int j=0;j<Model->Mesh[i].NumVertex;j++)
		{
			// Copy vertex/texture/tangent/binormal/normal data, padded to 16 floats (64 byte alignment, is this needed?)
			*fPtr++=Model->Mesh[i].Vertex[3*j+0];
			*fPtr++=Model->Mesh[i].Vertex[3*j+1];
			*fPtr++=Model->Mesh[i].Vertex[3*j+2];
			*fPtr++=1.0f; // Padding
			*fPtr++=Model->Mesh[i].UV[2*j+0];
			*fPtr++=Model->Mesh[i].UV[2*j+1];
			*fPtr++=0.0f; // Padding
			*fPtr++=Model->Mesh[i].Tangent[3*j+0];
			*fPtr++=Model->Mesh[i].Tangent[3*j+1];
			*fPtr++=Model->Mesh[i].Tangent[3*j+2];
			*fPtr++=Model->Mesh[i].Binormal[3*j+0];
			*fPtr++=Model->Mesh[i].Binormal[3*j+1];
			*fPtr++=Model->Mesh[i].Binormal[3*j+2];
			*fPtr++=Model->Mesh[i].Normal[3*j+0];
			*fPtr++=Model->Mesh[i].Normal[3*j+1];
			*fPtr++=Model->Mesh[i].Normal[3*j+2];
		}

		// Upload to GPU memory and free host memory buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*Model->Mesh[i].NumVertex*16, data, GL_STATIC_DRAW);
		FREE(data);

		// Generate element (index) buffer, copy data directly, no processing needed.
		glGenBuffers(1, &Model->Mesh[i].ElemID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Model->Mesh[i].ElemID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*Model->Mesh[i].NumFace*3, Model->Mesh[i].Face, GL_STATIC_DRAW);
	}

	glBindVertexArray(0);
}
