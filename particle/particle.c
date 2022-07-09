#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../system/system.h"
#include "../opengl/opengl.h"
#include "../image/image.h"
#include "../math/math.h"
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

	for(uint32_t i=0;i<System->NumEmitter;i++)
	{
		if(System->Emitter[i].ID>=0)
		{
			for(uint32_t j=0;j<System->Emitter[i].NumParticles;j++)
				Count++;
		}
	}

	// Resize vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, System->PartVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*8*Count, NULL, GL_STREAM_DRAW);

	// Resize system memory buffer
	FREE(System->ParticleArray);
	System->ParticleArray=calloc(Count, sizeof(float)*8);

	if(System->ParticleArray==NULL)
		return false;

	return true;
}

// Adds a particle emitter to the system
int32_t ParticleSystem_AddEmitter(ParticleSystem_t *System, vec3 Position, vec3 StartColor, vec3 EndColor, float ParticleSize, uint32_t NumParticles, bool Burst, ParticleInitCallback InitCallback)
{
	if(System==NULL)
		return false;

	// Pull an ID from the global counter
	int32_t ID=System->NextEmitterID++;

	// Index for new emitter = Current number of emitters
	int32_t Index=System->NumEmitter;

	// Increment emitter count and resize emitter memory
	System->NumEmitter++;
	System->Emitter=realloc(System->Emitter, sizeof(ParticleEmitter_t)*System->NumEmitter);

	if(System->Emitter==NULL)
		return -1;

	if(InitCallback==NULL)
		System->Emitter[Index].InitCallback=NULL;
	else
		System->Emitter[Index].InitCallback=InitCallback;

	// Set various flags/parameters
	System->Emitter[Index].Burst=Burst;
	System->Emitter[Index].ID=ID;
	Vec3_Setv(System->Emitter[Index].StartColor, StartColor);
	Vec3_Setv(System->Emitter[Index].EndColor, EndColor);
	System->Emitter[Index].ParticleSize=ParticleSize;

	// Set number of particles and allocate memory
	System->Emitter[Index].NumParticles=NumParticles;
	System->Emitter[Index].Particles=calloc(NumParticles, sizeof(Particle_t));

	// Set emitter position (used when resetting/recycling particles when they die)
	Vec3_Setv(System->Emitter[Index].Position, Position);

	// Set initial particle position and life to -1.0 (dead)
	for(uint32_t i=0;i<System->Emitter[Index].NumParticles;i++)
	{
		Vec3_Setv(System->Emitter[Index].Particles[i].pos, Position);
		System->Emitter[Index].Particles[i].life=-1.0f;
	}

	// Resize vertex buffers (both system memory and OpenGL buffer)
	if(!ParticleSystem_ResizeBuffer(System))
		return -1;

	return ID;
}

// Removes a particle emitter from the system
void ParticleSystem_DeleteEmitter(ParticleSystem_t *System, int32_t ID)
{
	bool found=false;

	// Return if ID isn't valid
	if(ID<0||System==NULL)
		return;

	for(uint32_t i=0;i<System->NumEmitter;i++)
	{
		if(System->Emitter[i].ID==ID)
		{
			FREE(System->Emitter[i].Particles);
			memset(&System->Emitter[i], 0, sizeof(ParticleEmitter_t));
			System->Emitter[i].ID=-1;
			found=true;
			break;
		}
	}

	// ID wasn't found, return out of function
	if(!found)
		return;

	// Otherwise, resize the emitter list
	// (this feels dumb, is there a better way to do this?)
	ParticleEmitter_t *NewEmitters=malloc(sizeof(ParticleEmitter_t)*(System->NumEmitter-1));

	if(NewEmitters==NULL)
		return;

	// Walk the list and copy only valid IDs
	for(uint32_t i=0, j=0;i<System->NumEmitter;i++)
	{
		if(System->Emitter[i].ID>=0)
			memcpy(&NewEmitters[j++], &System->Emitter[i], sizeof(ParticleEmitter_t));
	}

	// Free and reassign memory pointers
	FREE(System->Emitter);
	System->Emitter=NewEmitters;
	System->NumEmitter--;

	// Resize vertex buffers (both system memory and OpenGL buffer)
	ParticleSystem_ResizeBuffer(System);
}

// Resets the emitter to the initial parameters (mostly for a "burst" trigger)
void ParticleSystem_ResetEmitter(ParticleSystem_t *System, int32_t ID)
{
	// Return if ID isn't valid
	if(ID<0||System==NULL)
		return;

	for(uint32_t i=0;i<System->NumEmitter;i++)
	{
		if(System->Emitter[i].ID==ID)
		{
			for(uint32_t j=0;j<System->Emitter[i].NumParticles;j++)
			{
				// Only reset dead particles, limit "total reset" weirdness
				if(System->Emitter[i].Particles[j].life<0.0f)
				{
					// If a velocity/life callback was set, use it... Otherwise use default "fountain" style
					if(System->Emitter[i].InitCallback)
					{
						System->Emitter[i].InitCallback(j, System->Emitter[i].NumParticles, &System->Emitter[i].Particles[j]);

						// Add particle emitter position to the calculated position
						Vec3_Addv(System->Emitter[i].Particles[j].pos, System->Emitter[i].Position);
					}
					else
					{
						float SeedRadius=30.0f;
						float theta=((float)rand()/RAND_MAX)*2.0f*PI;
						float r=((float)rand()/RAND_MAX)*SeedRadius;

						// Set particle start position to emitter position
						Vec3_Setv(System->Emitter[i].Particles[j].pos, System->Emitter[i].Position);

						System->Emitter[i].Particles[j].vel[0]=r*sinf(theta);
						System->Emitter[i].Particles[j].vel[1]=((float)rand()/RAND_MAX)*100.0f;
						System->Emitter[i].Particles[j].vel[2]=r*cosf(theta);

						System->Emitter[i].Particles[j].life=((float)rand()/RAND_MAX)*0.999f+0.001f;
					}
				}
			}

			return;
		}
	}
}

void ParticleSystem_SetEmitterPosition(ParticleSystem_t *System, int32_t ID, vec3 Position)
{
	// Return if ID isn't valid
	if(ID<0||System==NULL)
		return;

	for(uint32_t i=0;i<System->NumEmitter;i++)
	{
		if(System->Emitter[i].ID==ID)
		{
			Vec3_Setv(System->Emitter[i].Position, Position);
			return;
		}
	}
}

bool ParticleSystem_Init(ParticleSystem_t *System)
{
	if(System==NULL)
		return false;

	System->NextEmitterID=0;

	System->NumEmitter=0;
	System->Emitter=NULL;

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

	for(uint32_t i=0;i<System->NumEmitter;i++)
	{
		// Only if an active ID
		if(System->Emitter[i].ID>=0)
		{
			for(uint32_t j=0;j<System->Emitter[i].NumParticles;j++)
			{
				System->Emitter[i].Particles[j].life-=dt*0.75f;

				// If the particle is dead and isn't a one shot (burst), restart it...
				// Otherwise run the math for the particle system motion.
				if(System->Emitter[i].Particles[j].life<0.0f&&!System->Emitter[i].Burst)
				{
					// If a velocity/life callback was set, use it... Otherwise use default "fountain" style
					if(System->Emitter[i].InitCallback)
					{
						System->Emitter[i].InitCallback(j, System->Emitter[i].NumParticles, &System->Emitter[i].Particles[j]);

						// Add particle emitter position to the calculated position
						Vec3_Addv(System->Emitter[i].Particles[j].pos, System->Emitter[i].Position);
					}
					else
					{
						float SeedRadius=30.0f;
						float theta=((float)rand()/RAND_MAX)*2.0f*PI;
						float r=((float)rand()/RAND_MAX)*SeedRadius;

						// Set particle start position to emitter position
						Vec3_Setv(System->Emitter[i].Particles[j].pos, System->Emitter[i].Position);

						System->Emitter[i].Particles[j].vel[0]=r*sinf(theta);
						System->Emitter[i].Particles[j].vel[1]=((float)rand()/RAND_MAX)*100.0f;
						System->Emitter[i].Particles[j].vel[2]=r*cosf(theta);

						System->Emitter[i].Particles[j].life=((float)rand()/RAND_MAX)*0.999f+0.001f;
					}
				}
				else
				{
					if(System->Emitter[i].Particles[j].life>0.0f)
					{
						vec3 temp;

						Vec3_Setv(temp, System->Emitter[i].Particles[j].vel);
						Vec3_Muls(temp, dt);
						Vec3_Addv(System->Emitter[i].Particles[j].pos, temp);

						Vec3_Setv(temp, PartGrav);
						Vec3_Muls(temp, dt);
						Vec3_Addv(System->Emitter[i].Particles[j].vel, temp);
					}
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

	for(uint32_t i=0;i<System->NumEmitter;i++)
	{
		if(System->Emitter[i].ID>=0)
		{
			for(uint32_t j=0;j<System->Emitter[i].NumParticles;j++)
			{
				// Only draw ones that are alive still
				if(System->Emitter[i].Particles[j].life>0.0f)
				{
					vec3 Color;

					*Array++=System->Emitter[i].Particles[j].pos[0];
					*Array++=System->Emitter[i].Particles[j].pos[1];
					*Array++=System->Emitter[i].Particles[j].pos[2];
					*Array++=System->Emitter[i].ParticleSize;
					Vec3_Lerp(System->Emitter[i].StartColor, System->Emitter[i].EndColor, System->Emitter[i].Particles[j].life, Color);
					*Array++=Color[0];
					*Array++=Color[1];
					*Array++=Color[2];
					*Array++=System->Emitter[i].Particles[j].life;

					Count++;
				}
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

	for(uint32_t i=0;i<System->NumEmitter;i++)
	{
		if(System->Emitter[i].ID>=0)
			FREE(System->Emitter[i].Particles);
	}

	FREE(System->Emitter);
	FREE(System->ParticleArray);
}