#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "system/system.h"
#include "math/math.h"
#include "utils/list.h"
#include "lights/lights.h"
#include "opengl/opengl.h"
#include "q2bsp.h"

#define IDBSPHEADER	(('P'<<24)+('S'<<16)+('B'<<8)+'I')
#define BSPVERSION	38

typedef struct
{
	uint32_t Offset;
	uint32_t Length;
} Q2BSP_Lump_t;

typedef struct
{
	uint32_t Magic;
	uint32_t Version;
	Q2BSP_Lump_t Entities;
	Q2BSP_Lump_t Planes;
	Q2BSP_Lump_t Vertices;
	Q2BSP_Lump_t Visibility;
	Q2BSP_Lump_t Nodes;
	Q2BSP_Lump_t TextureInfo;
	Q2BSP_Lump_t Faces;
	Q2BSP_Lump_t Lightmaps;
	Q2BSP_Lump_t Leaves;
	Q2BSP_Lump_t LeafFace;
	Q2BSP_Lump_t LeafBrush;
	Q2BSP_Lump_t Edges;
	Q2BSP_Lump_t FaceEdges;
	Q2BSP_Lump_t Models;
	Q2BSP_Lump_t Brushes;
	Q2BSP_Lump_t BrushSides;
	Q2BSP_Lump_t Pop;
	Q2BSP_Lump_t Areas;
	Q2BSP_Lump_t AreaPortals;
} Q2BSP_Header_t;

typedef struct
{
	uint16_t Plane;
	uint16_t Plane_Side;

	uint32_t First_Edge;
	uint16_t Num_Edges;

	uint16_t Texture_Info;

	uint8_t Lightmap_Styles[4];
	uint32_t Lightmap_Offset;
} Q2BSP_Face_t;

typedef struct
{
	vec3 U_Axis;
	float U_Offset;

	vec3 V_Axis;
	float V_Offset;

	uint32_t flags;
	uint32_t value;

	int8_t Texture_Name[32];

	uint32_t Next_TexInfo;
} Q2BSP_TextureInfo_t;

extern Lights_t Lights;

GLuint BSPVAO;
GLuint BSPVertex;
GLuint BSPUV;
GLuint BSPElement;

bool LoadQ2BSP(Q2BSP_Model_t *Model, const char *Filename)
{
	FILE *Stream=NULL;

	Stream=fopen(Filename, "rb");

	if(Stream==NULL)
		return false;

	Q2BSP_Header_t Header;

	fread(&Header, sizeof(Q2BSP_Header_t), 1, Stream);

	if(Header.Magic!=IDBSPHEADER&&Header.Version!=BSPVERSION)
		return false;

	///// Read vertices
	fseek(Stream, Header.Vertices.Offset, SEEK_SET);
	float *Vertex=(float *)malloc(Header.Vertices.Length);
	fread(Vertex, 1, Header.Vertices.Length, Stream);
	/////

	///// Read face edges
	fseek(Stream, Header.FaceEdges.Offset, SEEK_SET);
	int32_t *FaceEdges=(int32_t *)malloc(Header.FaceEdges.Length);
	fread(FaceEdges, 1, Header.FaceEdges.Length, Stream);
	/////

	///// Read faces
	fseek(Stream, Header.Faces.Offset, SEEK_SET);
	Q2BSP_Face_t *Faces=(Q2BSP_Face_t *)malloc(Header.Faces.Length);
	fread(Faces, 1, Header.Faces.Length, Stream);
	/////

	///// Read texinfo
	fseek(Stream, Header.TextureInfo.Offset, SEEK_SET);
	Q2BSP_TextureInfo_t *TextureInfo=(Q2BSP_TextureInfo_t *)malloc(Header.TextureInfo.Length);
	fread(TextureInfo, 1, Header.TextureInfo.Length, Stream);
	/////

	///// Read edges
	fseek(Stream, Header.Edges.Offset, SEEK_SET);
	uint16_t *Edges=(uint16_t *)malloc(Header.Edges.Length);
	fread(Edges, 1, Header.Edges.Length, Stream);
	/////

	///// Build index list and triangulate
	List_t VertexList;

	List_Init(&VertexList, sizeof(float)*20, 0, NULL);

	uint32_t NumFaces=Header.Faces.Length/sizeof(Q2BSP_Face_t);

	for(uint32_t i=0;i<NumFaces;i++)
	{
		Q2BSP_Face_t *face=&Faces[i];
		Q2BSP_TextureInfo_t *texInfo=&TextureInfo[face->Texture_Info];

		if(strstr(texInfo->Texture_Name, "trigger")||texInfo->flags&4)
			continue;

		int32_t edgeIdx=FaceEdges[face->First_Edge];
		float *vert0=&Vertex[3*Edges[2*abs(edgeIdx)+(edgeIdx<0?1:0)]];
		vec2 tex0={ (Vec3_Dot(vert0, texInfo->U_Axis)+texInfo->U_Offset)/128.0f, (Vec3_Dot(vert0, texInfo->V_Axis)+texInfo->V_Offset)/128.0f };

		for(int j=1;j<face->Num_Edges-1;j++)
		{
			edgeIdx=FaceEdges[face->First_Edge+j+1];
			float *vert1=&Vertex[3*Edges[2*abs(edgeIdx)+(edgeIdx<0?1:0)]];
			vec2 tex1={ (Vec3_Dot(vert1, texInfo->U_Axis)+texInfo->U_Offset)/128.0f, (Vec3_Dot(vert1, texInfo->V_Axis)+texInfo->V_Offset)/128.0f };

			edgeIdx=FaceEdges[face->First_Edge+j];
			float *vert2=&Vertex[3*Edges[2*abs(edgeIdx)+(edgeIdx<0?1:0)]];
			vec2 tex2={ (Vec3_Dot(vert2, texInfo->U_Axis)+texInfo->U_Offset)/128.0f, (Vec3_Dot(vert2, texInfo->V_Axis)+texInfo->V_Offset)/128.0f };

			vec3 v0, v1, t, b, n;
			vec2 uv0, uv1;
			float r;

			Vec3_Setv(v0, vert1);
			Vec3_Subv(v0, vert0);

			Vec2_Setv(uv0, tex1);
			Vec2_Subv(uv0, tex0);

			Vec3_Setv(v1, vert2);
			Vec3_Subv(v1, vert0);

			Vec2_Setv(uv1, tex2);
			Vec2_Subv(uv1, tex0);

			r=1.0f/(uv0[0]*uv1[1]-uv1[0]*uv0[1]);

			t[0]=(uv1[1]*v0[0]-uv0[1]*v1[0])*r;
			t[1]=(uv1[1]*v0[1]-uv0[1]*v1[1])*r;
			t[2]=(uv1[1]*v0[2]-uv0[1]*v1[2])*r;
			Vec3_Normalize(t);

			b[0]=(uv0[0]*v1[0]-uv1[0]*v0[0])*r;
			b[1]=(uv0[0]*v1[1]-uv1[0]*v0[1])*r;
			b[2]=(uv0[0]*v1[2]-uv1[0]*v0[2])*r;
			Vec3_Normalize(b);

			Cross(v0, v1, n);
			Vec3_Normalize(n);

			List_Add(&VertexList, (float[20])
			{
				vert0[0], vert0[1], vert0[2], 1.0f,
				tex0[0], tex0[1], 0.0f, 0.0f,
				t[0], t[1], t[2], 0.0f,
				b[0], b[1], b[2], 0.0f,
				n[0], n[1], n[2], 0.0f
			});

			List_Add(&VertexList, (float[20])
			{
				vert1[0], vert1[1], vert1[2], 1.0f,
				tex1[0], tex1[1], 0.0f, 0.0f,
				t[0], t[1], t[2], 0.0f,
				b[0], b[1], b[2], 0.0f,
				n[0], n[1], n[2], 0.0f
			});

			List_Add(&VertexList, (float[20])
			{
				vert2[0], vert2[1], vert2[2], 1.0f,
				tex2[0], tex2[1], 0.0f, 0.0f,
				t[0], t[1], t[2], 0.0f,
				b[0], b[1], b[2], 0.0f,
				n[0], n[1], n[2], 0.0f
			});
		}
	}

	Model->NumTris=(uint32_t)List_GetCount(&VertexList);

	glCreateVertexArrays(1, &BSPVAO);

	glVertexArrayAttribFormat(BSPVAO, 0, 4, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(BSPVAO, 0, 0);
	glEnableVertexArrayAttrib(BSPVAO, 0);

	glVertexArrayAttribFormat(BSPVAO, 1, 4, GL_FLOAT, GL_FALSE, sizeof(vec4));
	glVertexArrayAttribBinding(BSPVAO, 1, 0);
	glEnableVertexArrayAttrib(BSPVAO, 1);

	glVertexArrayAttribFormat(BSPVAO, 2, 4, GL_FLOAT, GL_FALSE, sizeof(vec4)*2);
	glVertexArrayAttribBinding(BSPVAO, 2, 0);
	glEnableVertexArrayAttrib(BSPVAO, 2);

	glVertexArrayAttribFormat(BSPVAO, 3, 4, GL_FLOAT, GL_FALSE, sizeof(vec4)*3);
	glVertexArrayAttribBinding(BSPVAO, 3, 0);
	glEnableVertexArrayAttrib(BSPVAO, 3);

	glVertexArrayAttribFormat(BSPVAO, 4, 4, GL_FLOAT, GL_FALSE, sizeof(vec4)*4);
	glVertexArrayAttribBinding(BSPVAO, 4, 0);
	glEnableVertexArrayAttrib(BSPVAO, 4);

	glCreateBuffers(1, &BSPVertex);
	glNamedBufferData(BSPVertex, VertexList.Size, VertexList.Buffer, GL_STATIC_DRAW);
	glVertexArrayVertexBuffer(BSPVAO, 0, BSPVertex, 0, sizeof(float)*20);
	/////

	///// Parse the entity list for lights
	fseek(Stream, Header.Entities.Offset, SEEK_SET);

	while((unsigned)ftell(Stream)<(Header.Entities.Offset+Header.Entities.Length))
	{
		int8_t buff[512];

		fgets(buff, sizeof(buff), Stream);

		if(strncmp(buff, "{", 1)==0)
		{
			bool IsLight=false, HasTarget=false;
			vec3 origin={ 0.0f, 0.0f, 0.0f };
			float radius=0.0f;;
			vec4 color={ 1.0f, 1.0f, 1.0f, 1.0f };

			while((buff[0]!='}')&&!feof(Stream))
			{
				// Read line
				fgets(buff, sizeof(buff), Stream);

				if(strncmp(buff, "\"classname\" \"light\"", 19)==0)
				{
					IsLight=true;
					continue;
				}

				if(strncmp(buff, "\"target\"", 8)==0)
				{
					HasTarget=true;
					continue;
				}

				if(sscanf(buff, "\"origin\" \"%f %f %f\"", &origin[2], &origin[0], &origin[1])==3)
					continue;

				if(sscanf(buff, "\"light\" \"%f\"", &radius)==1)
					continue;

				if(sscanf(buff, "\"_color\" \"%f %f %f\"", &color[0], &color[1], &color[2])==3)
					continue;

				if(sscanf(buff, "\"color\" \"%f %f %f\"", &color[0], &color[1], &color[2])==3)
					continue;
			}

			if(IsLight&&!HasTarget)
				Lights_Add(&Lights, origin, radius*2, color);
		}
	}
	/////

	fclose(Stream);

	return true;
}

void DrawQ2BSP(Q2BSP_Model_t *Model)
{
	glBindVertexArray(BSPVAO);
	glDrawArrays(GL_TRIANGLES, 0, Model->NumTris);
}