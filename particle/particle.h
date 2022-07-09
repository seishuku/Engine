#ifndef __PARTICLE_H__
#define __PARTICLE_H__

typedef struct
{
	float life;
	vec3 pos, vel;
} Particle_t;

typedef void (*ParticleInitCallback)(uint32_t Index, vec3 Velocity, float *life);

typedef struct
{
	int32_t ID;
	bool Burst;
	vec3 Position;
	vec3 StartColor, EndColor;
	uint32_t NumParticles;
	Particle_t *Particles;

	ParticleInitCallback InitCallback;
} ParticleEmitter_t;

typedef struct
{
	uint32_t NumEmitter;
	ParticleEmitter_t *Emitter;

	uint32_t PartVAO, PartVBO;
	float *ParticleArray;
} ParticleSystem_t;

int32_t ParticleSystem_AddEmitter(ParticleSystem_t *System, vec3 Position, vec3 StartColor, vec3 EndColor, uint32_t NumParticles, bool Burst, ParticleInitCallback InitCallback);
void ParticleSystem_DeleteEmitter(ParticleSystem_t *System, int32_t ID);
void ParticleSystem_ResetEmitter(ParticleSystem_t *System, int32_t ID);
void ParticleSystem_SetEmitterPosition(ParticleSystem_t *System, int32_t ID, vec3 Position);

bool ParticleSystem_Init(ParticleSystem_t *System);
void ParticleSystem_Step(ParticleSystem_t *System, float dt);
void ParticleSystem_Draw(ParticleSystem_t *System);
void ParticleSystem_Destroy(ParticleSystem_t *System);

#endif