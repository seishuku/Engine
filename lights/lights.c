#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../opengl/opengl.h"
#include "../math/math.h"
#include "../system/system.h"
#include "../gl_objects.h"
#include "lights.h"

int32_t Lights_Add(Lights_t *Lights, vec3 Position, float Radius, vec4 Kd)
{
	if(Lights==NULL)
		return -1;

	// Pull the next ID from the "global" counter
	// TO-DO:
	// Track IDs better, maybe just scan the existing IDs to find the highest number or
	// missing number in sequence? I don't know...
	int32_t ID=Lights->NextLightID++;
	int32_t Index=Lights->NumLights;

	Lights->NumLights++;
	Lights->Lights=realloc(Lights->Lights, sizeof(Light_t)*Lights->NumLights);

	if(Lights->Lights==NULL)
		return -1;

	Lights->Lights[Index].ID=ID;
	Vec3_Setv(Lights->Lights[Index].Position, Position);
	Lights->Lights[Index].Radius=1.0f/Radius;
	Vec4_Setv(Lights->Lights[Index].Kd, Kd);

	return ID;
}

void Lights_Del(Lights_t *Lights, int32_t ID)
{
	bool found=false;

	// Return if ID isn't valid
	if(ID<0||Lights==NULL)
		return;

	for(uint32_t i=0;i<Lights->NumLights;i++)
	{
		if(Lights->Lights[i].ID==ID)
		{
			memset(&Lights->Lights[i], 0, sizeof(Light_t));
			Lights->Lights[i].ID=-1;
			found=true;
			break;
		}
	}

	// ID wasn't found, return out of function
	if(!found)
		return;

	Light_t *NewLights=malloc(sizeof(Light_t)*(Lights->NumLights-1));

	if(NewLights==NULL)
		return;

	// Walk the list and copy only valid IDs
	for(uint32_t i=0, j=0;i<Lights->NumLights;i++)
	{
		if(Lights->Lights[i].ID>=0)
			memcpy(&NewLights[j++], &Lights->Lights[i], sizeof(Light_t));
	}

	// Free and reassign memory pointers
	FREE(Lights->Lights);
	Lights->Lights=NewLights;
	Lights->NumLights--;
}

void Lights_Update(Lights_t *Lights, int32_t ID, vec3 Position, float Radius, vec4 Kd)
{
	if(ID<0&&Lights==NULL)
		return;

	for(uint32_t i=0;i<Lights->NumLights;i++)
	{
		if(Lights->Lights[i].ID==ID)
		{
			Vec3_Setv(Lights->Lights[i].Position, Position);
			Lights->Lights[i].Radius=1.0f/Radius;
			Vec4_Setv(Lights->Lights[i].Kd, Kd);

			return;
		}
	}
}

void Lights_UpdatePosition(Lights_t *Lights, int32_t ID, vec3 Position)
{
	if(ID<0&&Lights==NULL)
		return;

	for(uint32_t i=0;i<Lights->NumLights;i++)
	{
		if(Lights->Lights[i].ID==ID)
		{
			Vec3_Setv(Lights->Lights[i].Position, Position);
			return;
		}
	}
}

void Lights_UpdateRadius(Lights_t *Lights, int32_t ID, float Radius)
{
	if(ID<0&&Lights==NULL)
		return;

	for(uint32_t i=0;i<Lights->NumLights;i++)
	{
		if(Lights->Lights[i].ID==ID)
		{
			Lights->Lights[i].Radius=1.0f/Radius;
			return;
		}
	}
}

void Lights_UpdateKd(Lights_t *Lights, int32_t ID, vec4 Kd)
{
	if(ID<0&&Lights==NULL)
		return;

	for(uint32_t i=0;i<Lights->NumLights;i++)
	{
		if(Lights->Lights[i].ID==ID)
		{
			Vec4_Setv(Lights->Lights[i].Kd, Kd);
			return;
		}
	}
}

extern int32_t DynSize;

static uint32_t oldNumLights=0;

void Lights_UpdateSSBO(Lights_t *Lights)
{
	// Need to reallocate the distance map cube array if the
	// light count changes, but *only* if the count changes.
	// This is kinda hacky.
	if(oldNumLights!=Lights->NumLights)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, Objects[TEXTURE_DISTANCE]);
		glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT32, DynSize, DynSize, 6*Lights->NumLights, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, Lights->SSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Light_t)*Lights->NumLights, Lights->Lights, GL_STREAM_DRAW);

	oldNumLights=Lights->NumLights;
}

bool Lights_Init(Lights_t *Lights)
{
	Lights->NextLightID=0;

	Lights->NumLights=0;
	Lights->Lights=NULL;

	glGenBuffers(1, &Lights->SSBO);

	return true;
}

void Lights_Destroy(Lights_t *Lights)
{
	glDeleteBuffers(1, &Lights->SSBO);

	FREE(Lights->Lights);
}
