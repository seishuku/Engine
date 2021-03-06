#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "system/system.h"
#include "math/math.h"
#include "utils/list.h"
#include "particle/particle.h"
#include "lights/lights.h"
#include "opengl/opengl.h"
#include "image/image.h"
#include "q2bsp.h"

#define IDBSPHEADER			(('P'<<24)+('S'<<16)+('B'<<8)+'I')
#define BSPVERSION			38

#define	SURF_PLANEBACK		0x02
#define	SURF_DRAWSKY		0x04
#define SURF_DRAWTURB		0x10
#define SURF_DRAWBACKGROUND	0x40
#define SURF_UNDERWATER		0x80

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
extern ParticleSystem_t ParticleSystem;

typedef struct
{
	int8_t TextureName[32];
	GLuint TexBaseID;
	GLuint TexSpecularID;
	GLuint TexNormalID;
} TextureItem_t;

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

	// NumMesh = number of TextureInfo struct, geometry is grouped by texture
	Model->NumMesh=Header.TextureInfo.Length/sizeof(Q2BSP_TextureInfo_t);
	Model->Mesh=(Q2BSP_Mesh_t *)malloc(sizeof(Q2BSP_Mesh_t)*Model->NumMesh);

	uint32_t NumFaces=Header.Faces.Length/sizeof(Q2BSP_Face_t);

	///// Build mesh geometry
	List_t VertexList, TextureList;
	char buf[512];

	List_Init(&VertexList, sizeof(float)*20, 0, NULL);
	List_Init(&TextureList, sizeof(TextureItem_t), 0, NULL);

	for(uint32_t i=0;i<Model->NumMesh;i++)
	{
		Q2BSP_TextureInfo_t *texInfo=&TextureInfo[i];

		// Ignore anything that's flagged as a trigger, clip, sky, or water surfaces
		if(strstr(texInfo->Texture_Name, "trigger")||
		   strstr(texInfo->Texture_Name, "clip")||
		   texInfo->flags&SURF_DRAWSKY||
		   texInfo->flags&SURF_DRAWTURB)
			continue;

		TextureItem_t *Item=NULL;

		for(int j=0;j<List_GetCount(&TextureList);j++)
		{
			Item=List_GetPointer(&TextureList, j);

			if(strncmp(Item->TextureName, texInfo->Texture_Name, 32)==0)
				break; // Found the texture
			else
				Item=NULL; // Texture not found
		}

		if(Item)
		{
			// Texture was found, so use those IDs
			Model->Mesh[i].TexBaseID=Item->TexBaseID;
			Model->Mesh[i].TexSpecularID=Item->TexSpecularID;
			Model->Mesh[i].TexNormalID=Item->TexNormalID;
		}
		else
		{
			// Texture was not found, load the texture and add it to the list

			snprintf(buf, 512, "./assets/%s.qoi", texInfo->Texture_Name);
			Model->Mesh[i].TexBaseID=Image_Upload(buf, IMAGE_MIPMAP|IMAGE_TRILINEAR);

			snprintf(buf, 512, "./assets/%s_s.qoi", texInfo->Texture_Name);
			Model->Mesh[i].TexSpecularID=Image_Upload(buf, IMAGE_MIPMAP|IMAGE_TRILINEAR);

			snprintf(buf, 512, "./assets/%s_n.qoi", texInfo->Texture_Name);
			Model->Mesh[i].TexNormalID=Image_Upload(buf, IMAGE_MIPMAP|IMAGE_TRILINEAR);

			if(!Model->Mesh[i].TexNormalID)
			{
				snprintf(buf, 512, "./assets/%s_b.tga", texInfo->Texture_Name);
				Model->Mesh[i].TexNormalID=Image_Upload(buf, IMAGE_MIPMAP|IMAGE_TRILINEAR|IMAGE_NORMALMAP);
			}
			else if(!Model->Mesh[i].TexNormalID)
				Model->Mesh[i].TexNormalID=Image_Upload("./assets/white_b.tga", IMAGE_MIPMAP|IMAGE_TRILINEAR|IMAGE_NORMALMAP);

			TextureItem_t ItemToAdd;

			memcpy(ItemToAdd.TextureName, texInfo->Texture_Name, 32);
			ItemToAdd.TexBaseID=Model->Mesh[i].TexBaseID;
			ItemToAdd.TexSpecularID=Model->Mesh[i].TexSpecularID;
			ItemToAdd.TexNormalID=Model->Mesh[i].TexNormalID;

			List_Add(&TextureList, &ItemToAdd);
		}

		GLuint texWidth=1, texHeight=1;

		glGetTextureLevelParameteriv(Model->Mesh[i].TexBaseID, 0, GL_TEXTURE_WIDTH, &texWidth);
		glGetTextureLevelParameteriv(Model->Mesh[i].TexBaseID, 0, GL_TEXTURE_HEIGHT, &texHeight);

		for(uint32_t j=0;j<NumFaces;j++)
		{
			Q2BSP_Face_t *Face=&Faces[j];

			if(Face->Texture_Info==i)
			{
				// Get an index for the first triangle and get a pointer to that vertex
				int32_t edgeIdx=FaceEdges[Face->First_Edge];
				float *vert0=&Vertex[3*Edges[2*abs(edgeIdx)+(edgeIdx<0?1:0)]];
				// Calculate a texture coord for that vertex
				vec2 tex0={ (Vec3_Dot(vert0, texInfo->U_Axis)+texInfo->U_Offset)/texWidth, (Vec3_Dot(vert0, texInfo->V_Axis)+texInfo->V_Offset)/texHeight };

				// Triangulate the remaining vertices for the triangle fan
				// (not using triangle fans here, this uses the above vertex as the root vertex
				//     and will be referenced in this loop every time)
				for(int k=1;k<Face->Num_Edges-1;k++)
				{
					// Second vertex in triangle
					edgeIdx=FaceEdges[Face->First_Edge+k+1];
					float *vert1=&Vertex[3*Edges[2*abs(edgeIdx)+(edgeIdx<0?1:0)]];
					vec2 tex1={ (Vec3_Dot(vert1, texInfo->U_Axis)+texInfo->U_Offset)/texWidth, (Vec3_Dot(vert1, texInfo->V_Axis)+texInfo->V_Offset)/texHeight };

					// Third vertex in triangle
					edgeIdx=FaceEdges[Face->First_Edge+k];
					float *vert2=&Vertex[3*Edges[2*abs(edgeIdx)+(edgeIdx<0?1:0)]];
					vec2 tex2={ (Vec3_Dot(vert2, texInfo->U_Axis)+texInfo->U_Offset)/texWidth, (Vec3_Dot(vert2, texInfo->V_Axis)+texInfo->V_Offset)/texHeight };

					// variables used in calculating tangenet space
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

					// Add the first vertex (root)
					List_Add(&VertexList, (float[20])
					{
						vert0[0], vert0[1], vert0[2], 1.0f, tex0[0], tex0[1], 0.0f, 0.0f, t[0], t[1], t[2], 0.0f, b[0], b[1], b[2], 0.0f, n[0], n[1], n[2], 0.0f
					});

					// Second
					List_Add(&VertexList, (float[20])
					{
						vert1[0], vert1[1], vert1[2], 1.0f, tex1[0], tex1[1], 0.0f, 0.0f, t[0], t[1], t[2], 0.0f, b[0], b[1], b[2], 0.0f, n[0], n[1], n[2], 0.0f
					});

					// Third
					List_Add(&VertexList, (float[20])
					{
						vert2[0], vert2[1], vert2[2], 1.0f, tex2[0], tex2[1], 0.0f, 0.0f, t[0], t[1], t[2], 0.0f, b[0], b[1], b[2], 0.0f, n[0], n[1], n[2], 0.0f
					});
				}
			}
		}

		// Number of triangles is actually NumTris/3, but GL wants number of vertices for drawing
		Model->Mesh[i].NumTris=(uint32_t)List_GetCount(&VertexList);

		// FIX-ME: the above loop has some "meshes" that have no verts,
		// apparently with Quake levels, there can be texInfo structs that are never referenced.
		if(Model->Mesh[i].NumTris)
		{
			glCreateVertexArrays(1, &Model->Mesh[i].VAO);

			glVertexArrayAttribFormat(Model->Mesh[i].VAO, 0, 4, GL_FLOAT, GL_FALSE, 0);
			glVertexArrayAttribBinding(Model->Mesh[i].VAO, 0, 0);
			glEnableVertexArrayAttrib(Model->Mesh[i].VAO, 0);

			glVertexArrayAttribFormat(Model->Mesh[i].VAO, 1, 4, GL_FLOAT, GL_FALSE, sizeof(vec4));
			glVertexArrayAttribBinding(Model->Mesh[i].VAO, 1, 0);
			glEnableVertexArrayAttrib(Model->Mesh[i].VAO, 1);

			glVertexArrayAttribFormat(Model->Mesh[i].VAO, 2, 4, GL_FLOAT, GL_FALSE, sizeof(vec4)*2);
			glVertexArrayAttribBinding(Model->Mesh[i].VAO, 2, 0);
			glEnableVertexArrayAttrib(Model->Mesh[i].VAO, 2);

			glVertexArrayAttribFormat(Model->Mesh[i].VAO, 3, 4, GL_FLOAT, GL_FALSE, sizeof(vec4)*3);
			glVertexArrayAttribBinding(Model->Mesh[i].VAO, 3, 0);
			glEnableVertexArrayAttrib(Model->Mesh[i].VAO, 3);

			glVertexArrayAttribFormat(Model->Mesh[i].VAO, 4, 4, GL_FLOAT, GL_FALSE, sizeof(vec4)*4);
			glVertexArrayAttribBinding(Model->Mesh[i].VAO, 4, 0);
			glEnableVertexArrayAttrib(Model->Mesh[i].VAO, 4);

			glCreateBuffers(1, &Model->Mesh[i].VBO);
			glNamedBufferData(Model->Mesh[i].VBO, VertexList.Size, VertexList.Buffer, GL_STATIC_DRAW);
			glVertexArrayVertexBuffer(Model->Mesh[i].VAO, 0, Model->Mesh[i].VBO, 0, sizeof(float)*20);
		}

		List_Clear(&VertexList);
	}

	List_Destroy(&TextureList);
	List_Destroy(&VertexList);
	/////

	///// Parse the entity list for lights
	fseek(Stream, Header.Entities.Offset, SEEK_SET);

	while((unsigned)ftell(Stream)<(Header.Entities.Offset+Header.Entities.Length))
	{
		int8_t buff[512];

		fgets(buff, sizeof(buff), Stream);

		if(strncmp(buff, "{", 1)==0)
		{
			bool IsLight=false, IsEmitter=false, IsPlayerStart=false;
			vec3 origin={ 0.0f, 0.0f, 0.0f };
			float radius=300.0f;;
			vec4 color={ 1.0f, 1.0f, 1.0f, 1.0f };
			vec4 color2={ 1.0f, 1.0f, 1.0f, 1.0f };
			float angle=0.0f, size=1.0f;
			uint32_t numParticles=1000;

			while((buff[0]!='}')&&!feof(Stream))
			{
				// Read line
				fgets(buff, sizeof(buff), Stream);

				if(strncmp(buff, "\"classname\" \"light\"", 19)==0)
				{
					IsLight=true;
					continue;
				}

				if(strncmp(buff, "\"classname\" \"info_player_start\"", 31)==0)
				{
					IsPlayerStart=true;
					continue;
				}

				if(strncmp(buff, "\"classname\" \"emitter\"", 21)==0)
				{
					IsEmitter=true;
					continue;
				}

				if(sscanf(buff, "\"origin\" \"%f %f %f\"", &origin[2], &origin[0], &origin[1])==3)
					continue;

				if(sscanf(buff, "\"light\" \"%f\"", &radius)==1)
					continue;

				if(sscanf(buff, "\"angle\" \"%f\"", &angle)==1)
					continue;

				if(sscanf(buff, "\"color\" \"%f %f %f\"", &color[0], &color[1], &color[2])==3)
					continue;

				if(sscanf(buff, "\"color2\" \"%f %f %f\"", &color2[0], &color2[1], &color2[2])==3)
					continue;

				if(sscanf(buff, "\"particleSize\" \"%f\"", &size)==1)
					continue;

				if(sscanf(buff, "\"numParticles\" \"%d\"", &numParticles)==1)
					continue;
			}

			if(IsEmitter)
				ParticleSystem_AddEmitter(&ParticleSystem, origin, color, color2, size, numParticles, false, NULL);

			if(IsLight)
				Lights_Add(&Lights, origin, radius*2, color);

			if(IsPlayerStart)
			{
				Vec3_Setv(Model->PlayerOrigin, origin);
				Model->PlayerDirection=deg2rad(angle);
			}
		}
	}
	/////

	fclose(Stream);

	// Quick clean up of the memory
	FREE(Vertex);
	FREE(FaceEdges);
	FREE(Faces);
	FREE(TextureInfo);
	FREE(Edges);

	return true;
}

void DrawQ2BSP(Q2BSP_Model_t *Model)
{
	for(uint32_t i=0;i<Model->NumMesh;i++)
	{
		if(Model->Mesh[i].NumTris)
		{
			glBindTextureUnit(0, Model->Mesh[i].TexBaseID);
			glBindTextureUnit(1, Model->Mesh[i].TexSpecularID);
			glBindTextureUnit(2, Model->Mesh[i].TexNormalID);
			glBindVertexArray(Model->Mesh[i].VAO);
			glDrawArrays(GL_TRIANGLES, 0, Model->Mesh[i].NumTris);
		}
	}
}

void DestroyQ2BSP(Q2BSP_Model_t *Model)
{
	FREE(Model->Mesh);
}