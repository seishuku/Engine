#ifndef __LIGHTS_H__
#define __LIGHTS_H__

typedef struct
{
	int32_t ID;

	uint32_t Pad[3];

	vec3 Position;
	float Radius;
	vec4 Kd;
} Light_t;

typedef struct
{
	int32_t NextLightID;

	List_t Lights;

	uint32_t SSBO;
} Lights_t;

int32_t Lights_Add(Lights_t *Lights, vec3 Position, float Radius, vec4 Kd);
void Lights_Del(Lights_t *Lights, int32_t ID);

void Lights_Update(Lights_t *Lights, int32_t ID, vec3 Position, float Radius, vec4 Kd);
void Lights_UpdatePosition(Lights_t *Lights, int32_t ID, vec3 Position);
void Lights_UpdateRadius(Lights_t *Lights, int32_t ID, float Radius);
void Lights_UpdateKd(Lights_t *Lights, int32_t ID, vec4 Kd);

void Lights_UpdateSSBO(Lights_t *Lights);
bool Lights_Init(Lights_t *Lights);
void Lights_Destroy(Lights_t *Lights);

#endif
