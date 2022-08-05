#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../system/system.h"
#include "../opengl/opengl.h"
#include "../image/image.h"
#include "../math/math.h"
#include "../utils/list.h"
#include "../utils/genid.h"
#include "../camera/camera.h"
#include "particle.h"

float PartGrav[3]={ 0.0, -9.81f, 0.0 };

GLuint PartShader=0;
GLuint PartTexture=0;

extern matrix Projection, ModelView;

// Resizes the OpenGL vertex buffer and system memory vertex buffer
bool ParticleSystem_ResizeBuffer(ParticleSystem_t *System)
{
	if(System==NULL)
		return false;

	uint32_t Count=0;

	for(uint32_t i=0;i<List_GetCount(&System->Emitters);i++)
	{
		ParticleEmitter_t *Emitter=List_GetPointer(&System->Emitters, i);

		for(uint32_t j=0;j<Emitter->NumParticles;j++)
			Count++;
	}

	// Resize vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, System->PartVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*8*Count, NULL, GL_STREAM_DRAW);

	// Resize system memory buffer
	FREE(System->ParticleArray);
	System->ParticleArray=malloc(Count*sizeof(float)*8);

	if(System->ParticleArray==NULL)
		return false;

	return true;
}

// Adds a particle emitter to the system
uint32_t ParticleSystem_AddEmitter(ParticleSystem_t *System, vec3 Position, vec3 StartColor, vec3 EndColor, float ParticleSize, uint32_t NumParticles, bool Burst, ParticleInitCallback InitCallback)
{
	if(System==NULL)
		return false;

	// Pull the next ID from the global ID count
	uint32_t ID=GenID();

	// Increment emitter count and resize emitter memory

	ParticleEmitter_t Emitter;

	if(InitCallback==NULL)
		Emitter.InitCallback=NULL;
	else
		Emitter.InitCallback=InitCallback;

	// Set various flags/parameters
	Emitter.Burst=Burst;
	Emitter.ID=ID;
	Vec3_Setv(Emitter.StartColor, StartColor);
	Vec3_Setv(Emitter.EndColor, EndColor);
	Emitter.ParticleSize=ParticleSize;

	// Set number of particles and allocate memory
	Emitter.NumParticles=NumParticles;
	Emitter.Particles=calloc(NumParticles, sizeof(Particle_t));

	// Set emitter position (used when resetting/recycling particles when they die)
	Vec3_Setv(Emitter.Position, Position);

	// Set initial particle position and life to -1.0 (dead)
	for(uint32_t i=0;i<Emitter.NumParticles;i++)
	{
		Vec3_Setv(Emitter.Particles[i].pos, Position);
		Emitter.Particles[i].life=-1.0f;
	}

	List_Add(&System->Emitters, &Emitter);

	// Resize vertex buffers (both system memory and OpenGL buffer)
	if(!ParticleSystem_ResizeBuffer(System))
		return UINT32_MAX;

	return ID;
}

// Removes a particle emitter from the system
void ParticleSystem_DeleteEmitter(ParticleSystem_t *System, uint32_t ID)
{
	if(System==NULL&&ID!=UINT32_MAX)
		return;

	for(uint32_t i=0;i<List_GetCount(&System->Emitters);i++)
	{
		ParticleEmitter_t *Emitter=List_GetPointer(&System->Emitters, i);

		if(Emitter->ID==ID)
		{
			List_Del(&System->Emitters, i);

			// Resize vertex buffers (both system memory and OpenGL buffer)
			ParticleSystem_ResizeBuffer(System);
			return;
		}
	}
}

// Resets the emitter to the initial parameters (mostly for a "burst" trigger)
void ParticleSystem_ResetEmitter(ParticleSystem_t *System, uint32_t ID)
{
	if(System==NULL&&ID!=UINT32_MAX)
		return;

	for(uint32_t i=0;i<List_GetCount(&System->Emitters);i++)
	{
		ParticleEmitter_t *Emitter=List_GetPointer(&System->Emitters, i);

		if(Emitter->ID==ID)
		{
			for(uint32_t j=0;j<Emitter->NumParticles;j++)
			{
				// Only reset dead particles, limit "total reset" weirdness
				if(Emitter->Particles[j].life<0.0f)
				{
					// If a velocity/life callback was set, use it... Otherwise use default "fountain" style
					if(Emitter->InitCallback)
					{
						Emitter->InitCallback(j, Emitter->NumParticles, &Emitter->Particles[j]);

						// Add particle emitter position to the calculated position
						Vec3_Addv(Emitter->Particles[j].pos, Emitter->Position);
					}
					else
					{
						float SeedRadius=30.0f;
						float theta=((float)rand()/RAND_MAX)*2.0f*PI;
						float r=((float)rand()/RAND_MAX)*SeedRadius;

						// Set particle start position to emitter position
						Vec3_Setv(Emitter->Particles[j].pos, Emitter->Position);

						Emitter->Particles[j].vel[0]=r*sinf(theta);
						Emitter->Particles[j].vel[1]=((float)rand()/RAND_MAX)*100.0f;
						Emitter->Particles[j].vel[2]=r*cosf(theta);

						Emitter->Particles[j].life=((float)rand()/RAND_MAX)*0.999f+0.001f;
					}
				}
			}

			return;
		}
	}
}

void ParticleSystem_SetEmitterPosition(ParticleSystem_t *System, uint32_t ID, vec3 Position)
{
	if(System==NULL&&ID!=UINT32_MAX)
		return;

	for(uint32_t i=0;i<List_GetCount(&System->Emitters);i++)
	{
		ParticleEmitter_t *Emitter=List_GetPointer(&System->Emitters, i);

		if(Emitter->ID==ID)
		{
			Vec3_Setv(Emitter->Position, Position);
			return;
		}
	}
}

bool ParticleSystem_Init(ParticleSystem_t *System)
{
	if(System==NULL)
		return false;

	List_Init(&System->Emitters, sizeof(ParticleEmitter_t), 10, NULL);

	System->ParticleArray=NULL;

	PartTexture=Image_Upload("./assets/particle.tga", IMAGE_BILINEAR|IMAGE_MIPMAP);

	if(PartTexture==0)
		return false;

	glGenVertexArrays(1, &System->PartVAO);
	glBindVertexArray(System->PartVAO);

	glGenBuffers(1, &System->PartVBO);
	glBindBuffer(GL_ARRAY_BUFFER, System->PartVBO);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float)*8, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float)*8, BUFFER_OFFSET(sizeof(float)*4));
	glEnableVertexAttribArray(1);

	PartShader=CreateShaderProgram((ProgNames_t)
	{
		"./shaders/particle_v.glsl", "./shaders/particle_f.glsl", "./shaders/particle_g.glsl", NULL
	});

	return true;
}

void ParticleSystem_Step(ParticleSystem_t *System, float dt)
{
	if(System==NULL)
		return;

	for(uint32_t i=0;i<List_GetCount(&System->Emitters);i++)
	{
		ParticleEmitter_t *Emitter=List_GetPointer(&System->Emitters, i);

		for(uint32_t j=0;j<Emitter->NumParticles;j++)
		{
			Emitter->Particles[j].life-=dt*0.75f;

			// If the particle is dead and isn't a one shot (burst), restart it...
			// Otherwise run the math for the particle system motion.
			if(Emitter->Particles[j].life<0.0f&&!Emitter->Burst)
			{
				// If a velocity/life callback was set, use it... Otherwise use default "fountain" style
				if(Emitter->InitCallback)
				{
					Emitter->InitCallback(j, Emitter->NumParticles, &Emitter->Particles[j]);

					// Add particle emitter position to the calculated position
					Vec3_Addv(Emitter->Particles[j].pos, Emitter->Position);
				}
				else
				{
					float SeedRadius=30.0f;
					float theta=((float)rand()/RAND_MAX)*2.0f*PI;
					float r=((float)rand()/RAND_MAX)*SeedRadius;

					// Set particle start position to emitter position
					Vec3_Setv(Emitter->Particles[j].pos, Emitter->Position);

					Emitter->Particles[j].vel[0]=r*sinf(theta);
					Emitter->Particles[j].vel[1]=((float)rand()/RAND_MAX)*100.0f;
					Emitter->Particles[j].vel[2]=r*cosf(theta);

					Emitter->Particles[j].life=((float)rand()/RAND_MAX)*0.999f+0.001f;
				}
			}
			else
			{
				if(Emitter->Particles[j].life>0.0f)
				{
					vec3 temp;

					Vec3_Setv(temp, PartGrav);
					Vec3_Muls(temp, dt);
					Vec3_Addv(Emitter->Particles[j].vel, temp);

					Vec3_Setv(temp, Emitter->Particles[j].vel);
					Vec3_Muls(temp, dt);
					Vec3_Addv(Emitter->Particles[j].pos, temp);
				}
			}
		}
	}
}

void ParticleSystem_Draw(ParticleSystem_t *System)
{
	if(System==NULL)
		return;

	matrix Local;
	uint32_t Count=0;

	MatrixIdentity(Local);

	float *Array=System->ParticleArray;

	if(Array==NULL)
		return;

	for(uint32_t i=0;i<List_GetCount(&System->Emitters);i++)
	{
		ParticleEmitter_t *Emitter=List_GetPointer(&System->Emitters, i);

		for(uint32_t j=0;j<Emitter->NumParticles;j++)
		{
			// Only draw ones that are alive still
			if(Emitter->Particles[j].life>0.0f)
			{
				vec3 Color;

				*Array++=Emitter->Particles[j].pos[0];
				*Array++=Emitter->Particles[j].pos[1];
				*Array++=Emitter->Particles[j].pos[2];
				*Array++=Emitter->ParticleSize;
				Vec3_Lerp(Emitter->StartColor, Emitter->EndColor, Emitter->Particles[j].life, Color);
				*Array++=Color[0];
				*Array++=Color[1];
				*Array++=Color[2];
				*Array++=Emitter->Particles[j].life;

				Count++;
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, System->PartVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*8*Count, System->ParticleArray);

	glUseProgram(PartShader);
	glUniformMatrix4fv(0, 1, GL_FALSE, Projection);
	glUniformMatrix4fv(1, 1, GL_FALSE, ModelView);

	glBindTextureUnit(0, PartTexture);

	glBindVertexArray(System->PartVAO);
	glDrawArrays(GL_POINTS, 0, Count);
}

void ParticleSystem_Destroy(ParticleSystem_t *System)
{
	if(System==NULL)
		return;

	glDeleteProgram(PartShader);

	glDeleteTextures(1, &PartTexture);

	glDeleteBuffers(1, &System->PartVBO);
	glDeleteVertexArrays(1, &System->PartVAO);

	for(uint32_t i=0;i<List_GetCount(&System->Emitters);i++)
	{
		ParticleEmitter_t *Emitter=List_GetPointer(&System->Emitters, i);

		FREE(Emitter->Particles);
	}

	List_Destroy(&System->Emitters);

	FREE(System->ParticleArray);
}