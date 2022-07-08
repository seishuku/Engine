#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../system/system.h"
#include "../opengl/opengl.h"
#include "../image/image.h"
#include "../math/math.h"
#include "particle.h"

const float PartGrav[3]={ 0.0, -9.81f, 0.0 };

const float PartColor1[3]={ 0.3f, 0.3f, 0.3f };
const float PartColor2[3]={ 0.125f, 0.03125f, 0.0f };

GLuint PartShader=0;
GLuint PartTexture=0;

extern matrix Projection, ModelView;

int32_t ParticleSystem_AddEmitter(ParticleSystem_t *System, vec3 Position, uint32_t NumParticles, bool Burst)
{
	int32_t ID=System->NumEmitter;

	System->NumEmitter++;

	System->Emitter=realloc(System->Emitter, sizeof(ParticleEmitter_t)*System->NumEmitter);

	if(System->Emitter==NULL)
		return -1;

	System->Emitter[ID].Burst=Burst;
	System->Emitter[ID].ID=ID;
	System->Emitter[ID].NumParticles=NumParticles;

	System->Emitter[ID].Particles=calloc(NumParticles, sizeof(Particle_t)*NumParticles);

	Vec3_Setv(System->Emitter[ID].Position, Position);

	for(uint32_t i=0;i<System->Emitter[ID].NumParticles;i++)
	{
		//Vec3_Setv(System->Emitter[ID].Particles[i].pos, Position);
		//float SeedRadius=30.0f;
		//float theta=((float)rand()/RAND_MAX)*2.0f*PI;
		//float r=((float)rand()/RAND_MAX)*SeedRadius;

		//System->Emitter[ID].Particles[i].vel[0]=r*sinf(theta);
		//System->Emitter[ID].Particles[i].vel[1]=((float)rand()/RAND_MAX)*100.0f;
		//System->Emitter[ID].Particles[i].vel[2]=r*cosf(theta);

		//System->Emitter[ID].Particles[i].life=((float)rand()/RAND_MAX)*0.999f+0.001f;

		Vec3_Setv(System->Emitter[ID].Particles[i].pos, Position);
		System->Emitter[ID].Particles[i].life=-1.0f;
	}

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
		return -1;

	return ID;
}

// FIX ME:
// This doesn't really remove the emitter from the emitters list,
// just clears the particles so it doesn't draw/update anymore.
// Need to resize emitter list to actually remove it's memory as well.
void ParticleSystem_DeleteEmitter(ParticleSystem_t *System, int32_t ID)
{
	if(ID<0)
		return;

	for(uint32_t i=0;i<System->NumEmitter;i++)
	{
		if(System->Emitter[i].ID==ID&&System->Emitter[i].ID>=0)
		{
			System->Emitter[i].NumParticles=0;
			FREE(System->Emitter[i].Particles);
			break;
		}
	}
}

void ParticleSystem_ResetEmitter(ParticleSystem_t *System, int32_t ID)
{
	for(uint32_t i=0;i<System->NumEmitter;i++)
	{
		if(System->Emitter[i].ID==ID)
		{
			for(uint32_t j=0;j<System->Emitter[i].NumParticles;j++)
			{
				float SeedRadius=30.0f;
				float theta=((float)rand()/RAND_MAX)*2.0f*PI;
				float r=((float)rand()/RAND_MAX)*SeedRadius;

				Vec3_Setv(System->Emitter[i].Particles[j].pos, System->Emitter[i].Position);

				System->Emitter[i].Particles[j].vel[0]=r*sinf(theta);
				System->Emitter[i].Particles[j].vel[1]=((float)rand()/RAND_MAX)*100.0f;
				System->Emitter[i].Particles[j].vel[2]=r*cosf(theta);

				System->Emitter[i].Particles[j].life=((float)rand()/RAND_MAX)*0.999f+0.001f;
			}

			return;
		}
	}
}

bool ParticleSystem_Init(ParticleSystem_t *System)
{
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
	glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STREAM_DRAW);

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
	for(uint32_t i=0;i<System->NumEmitter;i++)
	{
		if(System->Emitter[i].ID>=0)
		{
			for(uint32_t j=0;j<System->Emitter[i].NumParticles;j++)
			{
				System->Emitter[i].Particles[j].life-=dt*0.75f;

				if(System->Emitter[i].Particles[j].life<0.0f&&!System->Emitter[i].Burst)
				{
					float SeedRadius=30.0f;
					float theta=((float)rand()/RAND_MAX)*2.0f*PI;
					float r=((float)rand()/RAND_MAX)*SeedRadius;

					Vec3_Setv(System->Emitter[i].Particles[j].pos, System->Emitter[i].Position);

					System->Emitter[i].Particles[j].vel[0]=r*sinf(theta);
					System->Emitter[i].Particles[j].vel[1]=((float)rand()/RAND_MAX)*100.0f;
					System->Emitter[i].Particles[j].vel[2]=r*cosf(theta);

					System->Emitter[i].Particles[j].life=((float)rand()/RAND_MAX)*0.999f+0.001f;
				}
				else
				{
					System->Emitter[i].Particles[j].pos[0]+=(System->Emitter[i].Particles[j].vel[0]*dt);
					System->Emitter[i].Particles[j].pos[1]+=(System->Emitter[i].Particles[j].vel[1]*dt);
					System->Emitter[i].Particles[j].pos[2]+=(System->Emitter[i].Particles[j].vel[2]*dt);

					System->Emitter[i].Particles[j].vel[0]+=(PartGrav[0]*dt);
					System->Emitter[i].Particles[j].vel[1]+=(PartGrav[1]*dt);
					System->Emitter[i].Particles[j].vel[2]+=(PartGrav[2]*dt);
				}
			}
		}
	}
}

void ParticleSystem_Draw(ParticleSystem_t *System)
{
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
				vec3 Color;

				*Array++=System->Emitter[i].Particles[j].pos[0];
				*Array++=System->Emitter[i].Particles[j].pos[1];
				*Array++=System->Emitter[i].Particles[j].pos[2];
				*Array++=1.0f;
				Vec3_Lerp(PartColor1, PartColor2, System->Emitter[i].Particles[j].life+0.3f, Color);
				*Array++=Color[0];
				*Array++=Color[1];
				*Array++=Color[2];
				*Array++=System->Emitter[i].Particles[j].life;

				Count++;
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, System->PartVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*8*Count, System->ParticleArray);

	glUseProgram(PartShader);
	glUniformMatrix4fv(0, 1, GL_FALSE, Projection);
	glUniformMatrix4fv(1, 1, GL_FALSE, ModelView);
	glUniformMatrix4fv(2, 1, GL_FALSE, Local);

	glBindTextureUnit(0, PartTexture);

	glBindVertexArray(System->PartVAO);
	glDrawArrays(GL_POINTS, 0, Count);
}

void ParticleSystem_Destroy(ParticleSystem_t *System)
{
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