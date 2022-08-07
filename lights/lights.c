#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../opengl/opengl.h"
#include "../math/math.h"
#include "../system/system.h"
#include "../gl_objects.h"
#include "../utils/list.h"
#include "../utils/genid.h"
#include "lights.h"

uint32_t Lights_Add(Lights_t *Lights, vec3 Position, float Radius, vec4 Kd)
{
	if(Lights==NULL)
		return UINT32_MAX;

	// Pull the next ID from the global ID count
	uint32_t ID=GenID();

	Light_t Light;

	Light.ID=ID;
	Vec3_Setv(Light.Position, Position);
	Light.Radius=1.0f/Radius;
	Vec4_Setv(Light.Kd, Kd);

	Vec4_Sets(Light.SpotDirection, 0.0f);
	Light.SpotOuterCone=0.0f;
	Light.SpotInnerCone=0.0f;
	Light.SpotExponent=0.0f;

	List_Add(&Lights->Lights, (void *)&Light);

	return ID;
}

void Lights_Del(Lights_t *Lights, uint32_t ID)
{
	if(Lights==NULL&&ID!=UINT32_MAX)
		return;

	for(uint32_t i=0;i<List_GetCount(&Lights->Lights);i++)
	{
		Light_t *Light=List_GetPointer(&Lights->Lights, i);

		if(Light->ID==ID)
		{
			List_Del(&Lights->Lights, i);
			break;
		}
	}
}

void Lights_Update(Lights_t *Lights, uint32_t ID, vec3 Position, float Radius, vec4 Kd)
{
	if(Lights==NULL&&ID!=UINT32_MAX)
		return;

	for(uint32_t i=0;i<List_GetCount(&Lights->Lights);i++)
	{
		Light_t *Light=List_GetPointer(&Lights->Lights, i);

		if(Light->ID==ID)
		{
			Vec3_Setv(Light->Position, Position);
			Light->Radius=1.0f/Radius;
			Vec4_Setv(Light->Kd, Kd);

			return;
		}
	}
}

void Lights_UpdatePosition(Lights_t *Lights, uint32_t ID, vec3 Position)
{
	if(Lights==NULL&&ID!=UINT32_MAX)
		return;

	for(uint32_t i=0;i<List_GetCount(&Lights->Lights);i++)
	{
		Light_t *Light=List_GetPointer(&Lights->Lights, i);

		if(Light->ID==ID)
		{
			Vec3_Setv(Light->Position, Position);
			return;
		}
	}
}

void Lights_UpdateRadius(Lights_t *Lights, uint32_t ID, float Radius)
{
	if(Lights==NULL&&ID!=UINT32_MAX)
		return;

	for(uint32_t i=0;i<List_GetCount(&Lights->Lights);i++)
	{
		Light_t *Light=List_GetPointer(&Lights->Lights, i);

		if(Light->ID==ID)
		{
			Light->Radius=1.0f/Radius;
			return;
		}
	}
}

void Lights_UpdateKd(Lights_t *Lights, uint32_t ID, vec4 Kd)
{
	if(Lights==NULL&&ID!=UINT32_MAX)
		return;

	for(uint32_t i=0;i<List_GetCount(&Lights->Lights);i++)
	{
		Light_t *Light=List_GetPointer(&Lights->Lights, i);

		if(Light->ID==ID)
		{
			Vec4_Setv(Light->Kd, Kd);
			return;
		}
	}
}

void Lights_UpdateSpotlight(Lights_t *Lights, uint32_t ID, vec3 Direction, float OuterCone, float InnerCone, float Exponent)
{
	if(Lights==NULL&&ID!=UINT32_MAX)
		return;

	for(uint32_t i=0;i<List_GetCount(&Lights->Lights);i++)
	{
		Light_t *Light=List_GetPointer(&Lights->Lights, i);

		if(Light->ID==ID)
		{
			Vec3_Setv(Light->SpotDirection, Direction);
			Light->SpotOuterCone=OuterCone;
			Light->SpotInnerCone=InnerCone;
			Light->SpotExponent=Exponent;
			return;
		}
	}
}

extern int32_t DynSize;

void Lights_UpdateSSBO(Lights_t *Lights)
{
	static uint32_t oldNumLights=0;

	// Need to reallocate the distance map cube array if the
	// light count changes, but *only* if the count changes.
	// This is kinda hacky.
	if(oldNumLights!=List_GetCount(&Lights->Lights))
	{
		oldNumLights=(uint32_t)List_GetCount(&Lights->Lights);

		// Immutable texture object have to be deleted and recreated
		glDeleteTextures(1, &Objects[TEXTURE_DISTANCE]);
		glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &Objects[TEXTURE_DISTANCE]);
		glTextureParameteri(Objects[TEXTURE_DISTANCE], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(Objects[TEXTURE_DISTANCE], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(Objects[TEXTURE_DISTANCE], GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTextureParameteri(Objects[TEXTURE_DISTANCE], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(Objects[TEXTURE_DISTANCE], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(Objects[TEXTURE_DISTANCE], GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTextureParameteri(Objects[TEXTURE_DISTANCE], GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTextureStorage3D(Objects[TEXTURE_DISTANCE], 1, GL_DEPTH_COMPONENT32, DynSize, DynSize, 6*(GLsizei)List_GetCount(&Lights->Lights));

		// And re-assign the texture object to the framebuffer object
		glNamedFramebufferTexture(Objects[BUFFER_DISTANCE], GL_DEPTH_ATTACHMENT, Objects[TEXTURE_DISTANCE], 0);
	}

	glNamedBufferData(Lights->SSBO, sizeof(Light_t)*List_GetCount(&Lights->Lights), List_GetPointer(&Lights->Lights, 0), GL_STREAM_DRAW);
}

bool Lights_Init(Lights_t *Lights)
{
	List_Init(&Lights->Lights, sizeof(Light_t), 10, NULL);

	glCreateBuffers(1, &Lights->SSBO);

	return true;
}

void Lights_Destroy(Lights_t *Lights)
{
	glDeleteBuffers(1, &Lights->SSBO);

	List_Destroy(&Lights->Lights);
}
