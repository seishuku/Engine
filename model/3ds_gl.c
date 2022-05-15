#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../opengl/opengl.h"
#include "../image/image.h"
#include "../math/math.h"
#include "../system/system.h"
#include "3ds.h"

void LoadMaterials3DS(Model3DS_t *Model)
{
	for(int32_t i=0;i<Model->NumMaterial;i++)
	{
		char buf[256], nameNoExt[256], fileExt[256], *ptr=NULL;

		for(char *p=Model->Material[i].Texture;*p;p++)
			*p=*p>0x40&&*p<0x5b?*p|0x60:*p;

		strncpy(nameNoExt, Model->Material[i].Texture, 256);
		ptr=strstr(nameNoExt, ".");

		if(ptr==NULL)
			continue;

		strncpy(fileExt, ptr, 256);

		ptr[0]='\0';

		snprintf(buf, 256, "./assets/%s", Model->Material[i].Texture);
		Model->Material[i].TexBaseID=Image_Upload(buf, IMAGE_MIPMAP|IMAGE_TRILINEAR);

		snprintf(buf, 256, "./assets/%s_b%s", nameNoExt, fileExt);
		Model->Material[i].TexNormalID=Image_Upload(buf, IMAGE_MIPMAP|IMAGE_NORMALMAP|IMAGE_TRILINEAR);

		snprintf(buf, 256, "./assets/%s_s%s", nameNoExt, fileExt);
		Model->Material[i].TexSpecularID=Image_Upload(buf, IMAGE_MIPMAP|IMAGE_TRILINEAR);
	}
}

void DrawModel3DS(Model3DS_t *Model)
{
	for(int32_t i=0;i<Model->NumMesh;i++)
	{
		if(Model->Material)
		{
			glBindTextureUnit(0, Model->Material[Model->Mesh[i].MaterialNumber].TexBaseID);
			glBindTextureUnit(1, Model->Material[Model->Mesh[i].MaterialNumber].TexSpecularID);
			glBindTextureUnit(2, Model->Material[Model->Mesh[i].MaterialNumber].TexNormalID);
		}

		glBindVertexArray(Model->Mesh[i].VAO);
		glDrawElements(GL_TRIANGLES, Model->Mesh[i].NumFace*3, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
	}

	glBindVertexArray(0);
}

void BuildVBO3DS(Model3DS_t *Model)
{
	for(int32_t i=0;i<Model->NumMesh;i++)
	{
		float *data=NULL;

		if(!Model->Mesh[i].NumVertex)
			continue;

		// Create vertex array object
		glCreateVertexArrays(1, &Model->Mesh[i].VAO);

		// Set vertex array attribute layouts, binding point and offsets
		glVertexArrayAttribFormat(Model->Mesh[i].VAO, 0, 4, GL_FLOAT, GL_FALSE, sizeof(float)*0);
		glVertexArrayAttribBinding(Model->Mesh[i].VAO, 0, 0);
		glEnableVertexArrayAttrib(Model->Mesh[i].VAO, 0);

		glVertexArrayAttribFormat(Model->Mesh[i].VAO, 1, 4, GL_FLOAT, GL_FALSE, sizeof(float)*4);
		glVertexArrayAttribBinding(Model->Mesh[i].VAO, 1, 0);
		glEnableVertexArrayAttrib(Model->Mesh[i].VAO, 1);

		glVertexArrayAttribFormat(Model->Mesh[i].VAO, 2, 4, GL_FLOAT, GL_FALSE, sizeof(float)*(4+4));
		glVertexArrayAttribBinding(Model->Mesh[i].VAO, 2, 0);
		glEnableVertexArrayAttrib(Model->Mesh[i].VAO, 2);

		glVertexArrayAttribFormat(Model->Mesh[i].VAO, 3, 4, GL_FLOAT, GL_FALSE, sizeof(float)*(4+4+4));
		glVertexArrayAttribBinding(Model->Mesh[i].VAO, 3, 0);
		glEnableVertexArrayAttrib(Model->Mesh[i].VAO, 3);

		glVertexArrayAttribFormat(Model->Mesh[i].VAO, 4, 4, GL_FLOAT, GL_FALSE, sizeof(float)*(4+4+4+4));
		glVertexArrayAttribBinding(Model->Mesh[i].VAO, 4, 0);
		glEnableVertexArrayAttrib(Model->Mesh[i].VAO, 4);

		// Create vertex buffer object
		glCreateBuffers(1, &Model->Mesh[i].VertID);

		// Assign the vertex buffer to the vertex array on binding point 0, and set buffer stride
		glVertexArrayVertexBuffer(Model->Mesh[i].VAO, 0, Model->Mesh[i].VertID, 0, sizeof(float)*20);

		// Allocate vertex buffer data
		glNamedBufferData(Model->Mesh[i].VertID, sizeof(float)*Model->Mesh[i].NumVertex*20, NULL, GL_STATIC_DRAW);

		// Map data buffer and copy vertex data in the correct format
		data=(float *)glMapNamedBuffer(Model->Mesh[i].VertID, GL_WRITE_ONLY);

		if(data==NULL)
		{
			DBGPRINTF("VBO data buffer memory map failed or out of memory for object: %s", Model->Mesh[i].Name);

			glDeleteBuffers(1, &Model->Mesh[i].VertID);
			Model->Mesh[i].VertID=0;
			break;
		}

		float *fPtr=data;

		for(int32_t j=0;j<Model->Mesh[i].NumVertex;j++)
		{
			// Copy vertex/texture/tangent/binormal/normal data, padded to 16 floats (64 byte alignment, is this needed?)
			*fPtr++=Model->Mesh[i].Vertex[3*j+0];
			*fPtr++=Model->Mesh[i].Vertex[3*j+1];
			*fPtr++=Model->Mesh[i].Vertex[3*j+2];
			*fPtr++=1.0f; // Padding
			*fPtr++=Model->Mesh[i].UV[2*j+0];
			*fPtr++=Model->Mesh[i].UV[2*j+1];
			*fPtr++=0.0f; // Padding
			*fPtr++=0.0f; // Padding
			*fPtr++=Model->Mesh[i].Tangent[3*j+0];
			*fPtr++=Model->Mesh[i].Tangent[3*j+1];
			*fPtr++=Model->Mesh[i].Tangent[3*j+2];
			*fPtr++=0.0f; // Padding
			*fPtr++=Model->Mesh[i].Binormal[3*j+0];
			*fPtr++=Model->Mesh[i].Binormal[3*j+1];
			*fPtr++=Model->Mesh[i].Binormal[3*j+2];
			*fPtr++=0.0f; // Padding
			*fPtr++=Model->Mesh[i].Normal[3*j+0];
			*fPtr++=Model->Mesh[i].Normal[3*j+1];
			*fPtr++=Model->Mesh[i].Normal[3*j+2];
			*fPtr++=0.0f; // Padding
		}

		// Unmap the data pointer
		glUnmapNamedBuffer(Model->Mesh[i].VertID);

		// Create element (index) buffer, copy data directly, no processing needed.
		glCreateBuffers(1, &Model->Mesh[i].ElemID);
		glNamedBufferData(Model->Mesh[i].ElemID, sizeof(uint16_t)*Model->Mesh[i].NumFace*3, Model->Mesh[i].Face, GL_STATIC_DRAW);
		glVertexArrayElementBuffer(Model->Mesh[i].VAO, Model->Mesh[i].ElemID);
	}

	glBindVertexArray(0);
}
