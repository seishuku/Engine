#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math.h"
#include "obj.h"

#ifndef FREE
#define FREE(p) { if(p) { free(p); p=NULL; } }
#endif

void CalculateTangentOBJ(ModelOBJ_t *Model)
{
	if(Model->UV)
	{
		vec3 v0, v1, s, t, n;
		vec2 uv0, uv1;
		float r;

		Model->Tangent=(float *)malloc(sizeof(float)*3*Model->NumVertex);

		if(Model->Tangent==NULL)
			return;

		memset(Model->Tangent, 0, sizeof(float)*3*Model->NumVertex);

		Model->Binormal=(float *)malloc(sizeof(float)*3*Model->NumVertex);

		if(Model->Binormal==NULL)
			return;

		memset(Model->Binormal, 0, sizeof(float)*3*Model->NumVertex);

		Model->Normal=(float *)malloc(sizeof(float)*3*Model->NumVertex);

		if(Model->Normal==NULL)
			return;

		memset(Model->Normal, 0, sizeof(float)*3*Model->NumVertex);

		for(int j=0;j<Model->NumMesh;j++)
		{
			for(unsigned long i=0;i<Model->Mesh[j].NumFace;i++)
			{
				unsigned long i1=Model->Mesh[j].Face[3*i+0];
				unsigned long i2=Model->Mesh[j].Face[3*i+1];
				unsigned long i3=Model->Mesh[j].Face[3*i+2];

				v0[0]=Model->Vertex[3*i2+0]-Model->Vertex[3*i1+0];
				v0[1]=Model->Vertex[3*i2+1]-Model->Vertex[3*i1+1];
				v0[2]=Model->Vertex[3*i2+2]-Model->Vertex[3*i1+2];

				uv0[0]=Model->UV[2*i2+0]-Model->UV[2*i1+0];
				uv0[1]=Model->UV[2*i2+1]-Model->UV[2*i1+1];

				v1[0]=Model->Vertex[3*i3+0]-Model->Vertex[3*i1+0];
				v1[1]=Model->Vertex[3*i3+1]-Model->Vertex[3*i1+1];
				v1[2]=Model->Vertex[3*i3+2]-Model->Vertex[3*i1+2];

				uv1[0]=Model->UV[2*i3+0]-Model->UV[2*i1+0];
				uv1[1]=Model->UV[2*i3+1]-Model->UV[2*i1+1];

				r=1.0f/(uv0[0]*uv1[1]-uv1[0]*uv0[1]);

				s[0]=(uv1[1]*v0[0]-uv0[1]*v1[0])*r;
				s[1]=(uv1[1]*v0[1]-uv0[1]*v1[1])*r;
				s[2]=(uv1[1]*v0[2]-uv0[1]*v1[2])*r;
				Vec3_Normalize(s);

				Model->Tangent[3*i1+0]+=s[0];	Model->Tangent[3*i1+1]+=s[1];	Model->Tangent[3*i1+2]+=s[2];
				Model->Tangent[3*i2+0]+=s[0];	Model->Tangent[3*i2+1]+=s[1];	Model->Tangent[3*i2+2]+=s[2];
				Model->Tangent[3*i3+0]+=s[0];	Model->Tangent[3*i3+1]+=s[1];	Model->Tangent[3*i3+2]+=s[2];

				t[0]=(uv0[0]*v1[0]-uv1[0]*v0[0])*r;
				t[1]=(uv0[0]*v1[1]-uv1[0]*v0[1])*r;
				t[2]=(uv0[0]*v1[2]-uv1[0]*v0[2])*r;
				Vec3_Normalize(t);

				Model->Binormal[3*i1+0]-=t[0];	Model->Binormal[3*i1+1]-=t[1];	Model->Binormal[3*i1+2]-=t[2];
				Model->Binormal[3*i2+0]-=t[0];	Model->Binormal[3*i2+1]-=t[1];	Model->Binormal[3*i2+2]-=t[2];
				Model->Binormal[3*i3+0]-=t[0];	Model->Binormal[3*i3+1]-=t[1];	Model->Binormal[3*i3+2]-=t[2];

				Cross(v0, v1, n);
				Vec3_Normalize(n);

				Model->Normal[3*i1+0]+=n[0];	Model->Normal[3*i1+1]+=n[1];	Model->Normal[3*i1+2]+=n[2];
				Model->Normal[3*i2+0]+=n[0];	Model->Normal[3*i2+1]+=n[1];	Model->Normal[3*i2+2]+=n[2];
				Model->Normal[3*i3+0]+=n[0];	Model->Normal[3*i3+1]+=n[1];	Model->Normal[3*i3+2]+=n[2];
			}
		}
	}
}

int LoadMTL(ModelOBJ_t *Model, const char *Filename)
{
	FILE *fp;
	char buff[512];
	unsigned long NumVertex=0;
	unsigned long NumUV=0;

	if(!(fp=fopen(Filename, "r")))
		return 0;

	while(!feof(fp))
	{
		// Read line
		fgets(buff, sizeof(buff), fp);

		if(strncmp(buff, "newmtl ", 7)==0)
		{
			Model->NumMaterial++;

			if(!Model->Material)
				Model->Material=(MaterialOBJ_t *)malloc(sizeof(MaterialOBJ_t));
			else
				Model->Material=(MaterialOBJ_t *)realloc(Model->Material, sizeof(MaterialOBJ_t)*Model->NumMaterial);

			if(Model->Material)
			{
				memset(&Model->Material[Model->NumMaterial-1], 0, sizeof(MaterialOBJ_t));

				if(sscanf(buff, "newmtl %s", &Model->Material[Model->NumMaterial-1].Name)!=1)
					return 0;
			}
		}
		else if(strncmp(buff, "Ka ", 3)==0)
		{
			if(Model->Material)
			{
				if(sscanf(buff, "Ka %f %f %f",
					&Model->Material[Model->NumMaterial-1].Ambient[0],
					&Model->Material[Model->NumMaterial-1].Ambient[1],
					&Model->Material[Model->NumMaterial-1].Ambient[2])!=3)
					return 0;
			}
		}
		else if(strncmp(buff, "Kd ", 3)==0)
		{
			if(Model->Material)
			{
				if(sscanf(buff, "Kd %f %f %f",
					&Model->Material[Model->NumMaterial-1].Diffuse[0],
					&Model->Material[Model->NumMaterial-1].Diffuse[1],
					&Model->Material[Model->NumMaterial-1].Diffuse[2])!=3)
					return 0;
			}
		}
		else if(strncmp(buff, "Ks ", 3)==0)
		{
			if(Model->Material)
			{
				if(sscanf(buff, "Ks %f %f %f",
					&Model->Material[Model->NumMaterial-1].Specular[0],
					&Model->Material[Model->NumMaterial-1].Specular[1],
					&Model->Material[Model->NumMaterial-1].Specular[2])!=3)
					return 0;
			}
		}
		else if(strncmp(buff, "Ke ", 3)==0)
		{
			if(Model->Material)
			{
				if(sscanf(buff, "Ke %f %f %f",
					&Model->Material[Model->NumMaterial-1].Emission[0],
					&Model->Material[Model->NumMaterial-1].Emission[1],
					&Model->Material[Model->NumMaterial-1].Emission[2])!=3)
					return 0;
			}
		}
		else if(strncmp(buff, "Ns ", 3)==0)
		{
			if(Model->Material)
			{
				if(sscanf(buff, "Ns %f",
					&Model->Material[Model->NumMaterial-1].Shininess)!=1)
					return 0;
			}
		}
		else if(strncmp(buff, "map_Kd ", 7)==0)
		{
			if(Model->Material)
			{
				if(sscanf(buff, "map_Kd %s", &Model->Material[Model->NumMaterial-1].Texture)!=1)
					return 0;
			}
		}
	}

	// Match up material names with their meshes with an index number
	if(Model->Material)
	{
		for(int i=0;i<Model->NumMesh;i++)
		{
			for(int j=0;j<Model->NumMaterial;j++)
			{
				if(strcmp(Model->Mesh[i].MaterialName, Model->Material[j].Name)==0)
					Model->Mesh[i].MaterialNumber=j;
			}
		}
	}

	return 1;
}

// Apparently Windows CRT lib doesn't have this?
char *strrstr(const char *haystack, const char *needle)
{
	char *r=NULL;

	if(!needle[0])
		return (char *)haystack+strlen(haystack);
	while(1)
	{
		char *p=strstr(haystack, needle);
		if(!p)
			return r;
		r=p;
		haystack=p+1;
	}
}

int LoadOBJ(ModelOBJ_t *Model, const char *Filename)
{
	FILE *fp;
	char buff[512];
	unsigned long NumVertex=0;
	unsigned long NumUV=0;
	unsigned long vi[3]={ 0, 0, 0 };
	unsigned long ti[3]={ 0, 0, 0 };
	unsigned long ni[3]={ 0, 0, 0 };

	if(!(fp=fopen(Filename, "r")))
		return 0;

	memset(Model, 0, sizeof(ModelOBJ_t));

	while(!feof(fp))
	{
		// Read line
		fgets(buff, sizeof(buff), fp);

		if(strncmp(buff, "o ", 2)==0)
		{
			Model->NumMesh++;

			if(!Model->Mesh)
				Model->Mesh=(MeshOBJ_t *)malloc(sizeof(MeshOBJ_t));
			else
				Model->Mesh=(MeshOBJ_t *)realloc(Model->Mesh, sizeof(MeshOBJ_t)*Model->NumMesh);

			if(Model->Mesh)
			{
				memset(&Model->Mesh[Model->NumMesh-1], 0, sizeof(MeshOBJ_t));

				if(sscanf(buff, "o %s", &Model->Mesh[Model->NumMesh-1].Name)!=1)
					return 0;
			}
		}
		else if(strncmp(buff, "usemtl ", 7)==0)
		{
			if(Model->Mesh)
			{
				if(sscanf(buff, "usemtl %s", &Model->Mesh[Model->NumMesh-1].MaterialName)!=1)
					return 0;
			}
		}
		else if(strncmp(buff, "mtllib ", 7)==0)
		{
			if(sscanf(buff, "mtllib %s", &Model->MaterialFilename)!=1)
				return 0;
		}
		else if(strncmp(buff, "v ", 2)==0)
		{
			Model->NumVertex++;

			if(!Model->Vertex)
				Model->Vertex=(float *)malloc(sizeof(float)*3);
			else
				Model->Vertex=(float *)realloc(Model->Vertex, sizeof(float)*3*Model->NumVertex);
			
			if(Model->Vertex)
			{
				if(sscanf(buff, "v %f %f %f",
					&Model->Vertex[3*(Model->NumVertex-1)+0],
					&Model->Vertex[3*(Model->NumVertex-1)+1],
					&Model->Vertex[3*(Model->NumVertex-1)+2])!=3)
					return 0;
			}
		}
		else if(strncmp(buff, "vt ", 3)==0)
		{
			NumUV++;

			if(!Model->UV)
				Model->UV=(float *)malloc(sizeof(float)*2);
			else
				Model->UV=(float *)realloc(Model->UV, sizeof(float)*2*NumUV);

			if(Model->UV)
			{
				if(sscanf(buff, "vt %f %f",
					&Model->UV[2*(NumUV-1)+0],
					&Model->UV[2*(NumUV-1)+1])!=2)
					return 0;
			}
		}
		else if(strncmp(buff, "f ", 2)==0)
		{
			if(Model->Mesh)
			{
				Model->Mesh[Model->NumMesh-1].NumFace++;

				if(!Model->Mesh[Model->NumMesh-1].Face)
					Model->Mesh[Model->NumMesh-1].Face=(unsigned long *)malloc(sizeof(unsigned long)*3);
				else
					Model->Mesh[Model->NumMesh-1].Face=(unsigned long *)realloc(Model->Mesh[Model->NumMesh-1].Face, sizeof(unsigned long)*3*Model->Mesh[Model->NumMesh-1].NumFace);

				if(Model->Mesh[Model->NumMesh-1].Face)
				{
					unsigned long temp=0;

					// So... Alias Wavefront models allow a different index buffer for vertices, normals, and UV
					// But, I'm ignoring that and only using the vertex indices for everything and hope that
					// whatever exported the model used the same index for all attribs.
					//
					// Maybe add in a index unifier layer?
					//
					if(sscanf(buff, "f %d/%d/%d %d/%d/%d %d/%d/%d",
						&vi[0], &ti[0], &ni[0],
						&vi[1], &ti[1],	&ni[1],
						&vi[2], &ti[2], &ni[2])!=9)
					{
						if(sscanf(buff, "f %d/%d %d/%d %d/%d",
							&vi[0], &ti[0],
							&vi[1], &ti[1],
							&vi[2], &ti[2])!=6)
						{
							if(sscanf(buff, "f %d//%d %d//%d %d//%d",
								&vi[0], &ni[0],
								&vi[1], &ni[1],
								&vi[2], &ni[2])!=6)
							{
								if(sscanf(buff, "f %d %d %d",
									&vi[0],
									&vi[1],
									&vi[2])!=3)
									return 0;
							}
						}
					}

					// Alias Wavefront models have face indices that start on 1, not 0
					Model->Mesh[Model->NumMesh-1].Face[3*(Model->Mesh[Model->NumMesh-1].NumFace-1)+0]=vi[0]-1;
					Model->Mesh[Model->NumMesh-1].Face[3*(Model->Mesh[Model->NumMesh-1].NumFace-1)+1]=vi[1]-1;
					Model->Mesh[Model->NumMesh-1].Face[3*(Model->Mesh[Model->NumMesh-1].NumFace-1)+2]=vi[2]-1;
				}
			}
		}
	}

	fclose(fp);

	CalculateTangentOBJ(Model);

	char nameNoExt[256], *ptr;

	strncpy(nameNoExt, Filename, 256);
	ptr=strrstr(nameNoExt, ".");

	if(ptr==NULL)
		return 1;

	ptr[0]='\0';

	strcat(nameNoExt, ".mtl");
	printf(nameNoExt);
	LoadMTL(Model, nameNoExt);

	return 1;
}

// Free memory allocated for the model
void FreeOBJ(ModelOBJ_t *Model)
{
	FREE(Model->Vertex);
	FREE(Model->UV);
	FREE(Model->Normal);
	FREE(Model->Tangent);
	FREE(Model->Binormal);

	if(Model->NumMesh)
	{
		/* Free mesh data */
		for(int i=0;i<Model->NumMesh;i++)
			FREE(Model->Mesh[i].Face);

		FREE(Model->Mesh);
	}

	FREE(Model->Material);
}
