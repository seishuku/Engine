#ifndef __PARTICLE_H__
#define __PARTICLE_H__

typedef struct
{
	float life;
	vec3 pos, vel;
} Particle_t;

typedef struct
{
	int32_t ID;
	bool Burst;
	vec3 Position;
	uint32_t NumParticles;
	Particle_t *Particles;
} ParticleEmitter_t;

typedef struct
{
	uint32_t NumEmitter;
	ParticleEmitter_t *Emitter;

	uint32_t PartVAO, PartVBO;
	float *ParticleArray;
} ParticleSystem_t;

int32_t ParticleSystem_AddEmitter(ParticleSystem_t *System, vec3 Position, uint32_t NumParticles, bool Burst);
void ParticleSystem_DeleteEmitter(ParticleSystem_t *System, int32_t ID);
void ParticleSystem_ResetEmitter(ParticleSystem_t *System, int32_t ID);

bool ParticleSystem_Init(ParticleSystem_t *System);
void ParticleSystem_Step(ParticleSystem_t *System, float dt);
void ParticleSystem_Draw(ParticleSystem_t *System);
void ParticleSystem_Destroy(ParticleSystem_t *System);

#endif