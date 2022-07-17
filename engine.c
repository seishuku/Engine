#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>
#include "math/math.h"
#include "opengl/opengl.h"
#include "system/system.h"
#include "gl_objects.h"
#include "image/image.h"
#include "font/font.h"
#include "camera/camera.h"
#include "model/3ds.h"
#include "model/3ds_gl.h"
#include "model/md5.h"
#include "model/md5_gl.h"
#include "model/skybox_gl.h"
#include "model/obj.h"
#include "model/obj_gl.h"
#include "model/beam_gl.h"
#include "audio/audio.h"
#include "opencl/opencl.h"
#include "fluid/fluid3d.h"
#include "utils/list.h"
#include "particle/particle.h"
#include "lights/lights.h"

#define CAMERA_RECORDING 0

int32_t Width=1280, Height=720;

extern float fps, fFrameTime, fTimeStep, fTime;

Sample_t Hellknight_Idle;

uint32_t Objects[NUM_OBJECTS];

//Fluid3D_t Fluid;

ModelOBJ_t Level;

Model_t Hellknight;
Model_t Fatty;
Model_t Pinky;

Camera_t Camera;
CameraPath_t CameraPath;

extern bool Auto;

matrix Projection, ModelView, ModelViewInv;

Lights_t Lights;
int32_t LightIDs[10];

vec3 BeamStart0={ -75.0f, -80.0f, -120.0f };
vec3 BeamEnd0={ -75.0f, 90.0f, -120.0f };

vec3 BeamStart1={ 75.0f, -80.0f, -120.0f };
vec3 BeamEnd1={ 75.0f, 90.0f, -120.0f };

const float radius=5.0f;

int32_t DynSize=1024;

ParticleSystem_t ParticleSystem;
int32_t EmitterIDs[6];

void APIENTRY error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *user_data)
{
	const char *_severity, *_source, *_type;

	if(severity==GL_DEBUG_SEVERITY_HIGH)
		_severity="High";
	else if(severity==GL_DEBUG_SEVERITY_MEDIUM)
		_severity="Medium";
	else if(severity==GL_DEBUG_SEVERITY_LOW)
		_severity="Low";
	else
		return; // Don't care about these, it's mostly performance hints
//		_severity="Unknown";

	if(source==GL_DEBUG_SOURCE_API)
		_source="API";
	else if(source==GL_DEBUG_SOURCE_WINDOW_SYSTEM)
		_source="Window System";
	else if(source==GL_DEBUG_SOURCE_SHADER_COMPILER)
		_source="Shader Compiler";
	else if(source==GL_DEBUG_SOURCE_THIRD_PARTY)
		_source="Third Party";
	else if(source==GL_DEBUG_SOURCE_APPLICATION)
		_source="Application";
	else if(source==GL_DEBUG_SOURCE_OTHER)
		_source="Other";
	else
		_source="Unknown Source";

	if(type==GL_DEBUG_TYPE_ERROR)
		_type="Error";
	else if(type==GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR)
		_type="Depricated Behavior";
	else if(type==GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)
		_type="Undefined Behavior";
	else if(type==GL_DEBUG_TYPE_PORTABILITY)
		_type="Portability";
	else if(type==GL_DEBUG_TYPE_PERFORMANCE)
		_type="Performance";
	else if(type==GL_DEBUG_TYPE_OTHER)
		_type="Other";
	else
		_type="Unknown";

	DBGPRINTF("OpenGL debug:\n\tID: 0x%4X\n\tType: %s\n\tSeverity: %s\n\tSource: %s\n\tMessage: %s\n\n", id, _type, _severity, _source, message);
}

GLuint VAO, VBO;

#define NUM_SAMPLES 200
float lines[3*NUM_SAMPLES+3];

void UpdateLineChart(const float val)
{
	float temp[3*NUM_SAMPLES+1];
	float xPos=-2.5f;
	float yPos=-0.9f;
	float xScale=2.0f;
	float yScale=0.0625f;

	// Initial seed into the array
	lines[3*0+0]=(1.0f+xPos)/xScale;
	lines[3*0+1]=(val*yScale)+yPos;
	lines[3*0+2]=-1.0f;

	// Propagate out into a temp array
	for(int32_t i=0;i<NUM_SAMPLES;i++)
	{
		temp[3*(i)+0]=(1.0f-((float)i/NUM_SAMPLES*0.5f)+xPos)/xScale;
		temp[3*(i)+1]=lines[3*(i)+1];
		temp[3*(i)+2]=lines[3*(i)+2];
	}

	// Shift the whole array over one, as to create the "continuous" effect
	for(int32_t i=0;i<NUM_SAMPLES-1;i++)
	{
		lines[3*(i+1)+0]=temp[3*(i)+0];
		lines[3*(i+1)+1]=temp[3*(i)+1];
		lines[3*(i+1)+2]=temp[3*(i)+2];
	}

	// Update the vertex buffer
	glNamedBufferSubData(VBO, 0, sizeof(float)*3*NUM_SAMPLES+3, lines);
}

#include "q2bsp.h"

Q2BSP_Model_t Q2Model;

void UpdateShadow(GLuint buffer)
{
	matrix proj, mv[6], local;
	glBindFramebuffer(GL_FRAMEBUFFER, buffer);

	glCullFace(GL_FRONT);

	glViewport(0, 0, DynSize, DynSize);
	MatrixIdentity(proj);
	MatrixInfPerspective(90.0f, 1.0f, 0.01f, 0, proj);

	glClear(GL_DEPTH_BUFFER_BIT);

	// Select the shader program
	glUseProgram(Objects[GLSL_DISTANCE_SHADER]);

	glUniformMatrix4fv(Objects[GLSL_DISTANCE_PROJ], 1, GL_FALSE, proj);

	for(uint32_t i=0;i<List_GetCount(&Lights.Lights);i++)
	{
		Light_t *Light=List_GetPointer(&Lights.Lights, i);
		vec4 pos;

		Vec3_Setv(pos, Light->Position);
		pos[3]=Light->Radius;

		MatrixIdentity(mv[0]);
		MatrixLookAt(pos, (vec3) { pos[0]+1.0f, pos[1]+0.0f, pos[2]+0.0f }, (vec3) { 0.0f, -1.0f, 0.0f }, mv[0]);

		MatrixIdentity(mv[1]);
		MatrixLookAt(pos, (vec3) { pos[0]-1.0f, pos[1]+0.0f, pos[2]+0.0f }, (vec3) { 0.0f, -1.0f, 0.0f }, mv[1]);

		MatrixIdentity(mv[2]);
		MatrixLookAt(pos, (vec3) { pos[0]+0.0f, pos[1]+1.0f, pos[2]+0.0f }, (vec3) { 0.0f, 0.0f, 1.0f }, mv[2]);

		MatrixIdentity(mv[3]);
		MatrixLookAt(pos, (vec3) { pos[0]+0.0f, pos[1]-1.0f, pos[2]+0.0f }, (vec3) { 0.0f, 0.0f, -1.0f }, mv[3]);

		MatrixIdentity(mv[4]);
		MatrixLookAt(pos, (vec3) { pos[0]+0.0f, pos[1]+0.0f, pos[2]+1.0f }, (vec3) { 0.0f, -1.0f, 0.0f }, mv[4]);

		MatrixIdentity(mv[5]);
		MatrixLookAt(pos, (vec3) { pos[0]+0.0f, pos[1]+0.0f, pos[2]-1.0f }, (vec3) { 0.0f, -1.0f, 0.0f }, mv[5]);

		glUniformMatrix4fv(Objects[GLSL_DISTANCE_MV], 6, GL_FALSE, (float *)mv);

		glUniform4fv(Objects[GLSL_DISTANCE_LIGHTPOS], 1, pos);
		glUniform1i(Objects[GLSL_DISTANCE_INDEX], i);

		// Render models
		//MatrixIdentity(local);
		//MatrixTranslate(0.0f, -100.0f, 0.0f, local);
		//MatrixRotate(-PI/2.0f, 1.0f, 0.0f, 0.0f, local);
		//MatrixRotate(-PI/2.0f, 0.0f, 0.0f, 1.0f, local);
		//glUniformMatrix4fv(Objects[GLSL_DISTANCE_LOCAL], 1, GL_FALSE, local);
		//DrawModelMD5(&Hellknight.Model);

		//MatrixIdentity(local);
		//MatrixTranslate(-700.0f, -100.0f, -700.0f, local);
		//MatrixRotate(-PI/2.0f, 1.0f, 0.0f, 0.0f, local);
		//MatrixRotate(-PI/2.0f, 0.0f, 0.0f, 1.0f, local);
		//glUniformMatrix4fv(Objects[GLSL_DISTANCE_LOCAL], 1, GL_FALSE, local);
		//DrawModelMD5(&Fatty.Model);

		//MatrixIdentity(local);
		//MatrixTranslate(700.0f, -100.0f, -700.0f, local);
		//MatrixRotate(-PI/2.0f, 1.0f, 0.0f, 0.0f, local);
		//MatrixRotate(-PI/2.0f, 0.0f, 0.0f, 1.0f, local);
		//glUniformMatrix4fv(Objects[GLSL_DISTANCE_LOCAL], 1, GL_FALSE, local);
		//DrawModelMD5(&Pinky.Model);

		MatrixIdentity(local);
		MatrixRotate(-PI/2.0f, 1.0f, 0.0f, 0.0f, local);
		MatrixRotate(-PI/2.0f, 0.0f, 0.0f, 1.0f, local);
		glUniformMatrix4fv(Objects[GLSL_DISTANCE_LOCAL], 1, GL_FALSE, local);
//		DrawModelOBJ(&Level);
		DrawQ2BSP(&Q2Model);
	}

	glCullFace(GL_BACK);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool retrigger=true;

void Render(void)
{
	matrix local;

	///// 3D Fluid simulation
	//clEnqueueAcquireGLObjects(Fluid.Context.CommandQueue, 1, &Fluid.den, 0, NULL, NULL);

	//Fluid3D_AddDensityVelocity(&Fluid, 5, (Fluid.h/2), Fluid.d/2, 1.0f, 0.0f, 0.0f, 0.5f);
	//Fluid3D_AddDensityVelocity(&Fluid, Fluid.w-5, (Fluid.h/2), Fluid.d/2, -1.0f, 0.0f, 0.0f, 0.5f);

	//Fluid3D_Step(&Fluid, fTimeStep);

	//clEnqueueReleaseGLObjects(Fluid.Context.CommandQueue, 1, &Fluid.den, 0, NULL, NULL);
	//clFlush(Fluid.Context.CommandQueue);
	/////

//	for(int32_t i=0;i<Level.NumMesh;i++)
//		CameraCheckCollision(&Camera, Level.Vertex, Level.Mesh[i].Face, Level.Mesh[i].NumFace);

	// Sphere -> BBox intersection testing
	//float min[3]={ 0.0f, };
	//float max[3]={ 0.0f, };
	//MatrixIdentity(local);
	//MatrixTranslate(0.0f, -100.0f, 0.0f, local);
	//MatrixRotate(-PI/2.0f, 1.0f, 0.0f, 0.0f, local);
	//MatrixRotate(-PI/2.0f, 0.0f, 0.0f, 1.0f, local);
	//Matrix4x4MultVec3(Hellknight.Anim.bboxes[Hellknight.frame].min, local, min);
	//Matrix4x4MultVec3(Hellknight.Anim.bboxes[Hellknight.frame].max, local, max);
	//int32_t collide=SphereBBOXIntersection(Camera.Position, Camera.Radius, min, max);
	////

	// Trigger sound playback at frame 60, retrigger locks it out so it doesn't rapid fire the sound.
	if(Hellknight.frame==60&&retrigger)
	{
		Audio_PlaySample(&Hellknight_Idle, false);
		retrigger=false;
	}

	// Reset trigger lock so it will play again on next loop.
	if(Hellknight.frame>61)
		retrigger=true;

	UpdateAnimation(&Hellknight, fTimeStep);
	UpdateAnimation(&Fatty, fTimeStep);
	UpdateAnimation(&Pinky, fTimeStep);

	Lights_UpdateSSBO(&Lights);

	UpdateShadow(Objects[BUFFER_DISTANCE]);
		
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// Set viewport and calculate a projection matrix (perspective, with infinite z-far plane)
	glViewport(0, 0, Width, Height);
	MatrixIdentity(Projection);
	MatrixInfPerspective(90.0f, (float)Width/Height, 0.01f, 0, Projection);

	// Set up model view matrix (translate and rotation)
	MatrixIdentity(ModelView);

	if(Auto)
		CameraInterpolatePath(&CameraPath, &Camera, fTimeStep, ModelView);
	else
		CameraUpdate(&Camera, fTimeStep, ModelView);

	Audio_SetListenerOrigin(Camera.Position, Camera.Right);

	// Select the shader program
	glUseProgram(Objects[GLSL_LIGHT_SHADER]);

	// Lighting parameters
	glUniform1i(Objects[GLSL_LIGHT_NUMLIGHTS], (GLint)List_GetCount(&Lights.Lights));
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, Lights.SSBO);

	// Projection matrix
	glUniformMatrix4fv(Objects[GLSL_LIGHT_PROJ], 1, GL_FALSE, Projection);

	// Global ModelView
	glUniformMatrix4fv(Objects[GLSL_LIGHT_MV], 1, GL_FALSE, ModelView);

	// Beam area light positions
	glUniform3fv(Objects[GLSL_LIGHT_BEAM_START0], 1, BeamStart0);
	glUniform3fv(Objects[GLSL_LIGHT_BEAM_END0], 1, BeamEnd0);

	glUniform3fv(Objects[GLSL_LIGHT_BEAM_START1], 1, BeamStart1);
	glUniform3fv(Objects[GLSL_LIGHT_BEAM_END1], 1, BeamEnd1);

	// Bind distance texture array for shadows
	glBindTextureUnit(3, Objects[TEXTURE_DISTANCE]);

	// Render model
	//MatrixIdentity(local);
	//MatrixTranslate(0.0f, -100.0f, 0.0f, local);
	//MatrixRotate(-PI/2.0f, 1.0f, 0.0f, 0.0f, local);
	//MatrixRotate(-PI/2.0f, 0.0f, 0.0f, 1.0f, local);
	//glUniformMatrix4fv(Objects[GLSL_LIGHT_LOCAL], 1, GL_FALSE, local);
	//glBindTextureUnit(0, Hellknight.Base);
	//glBindTextureUnit(1, Hellknight.Specular);
	//glBindTextureUnit(2, Hellknight.Normal);
	//DrawModelMD5(&Hellknight.Model);

	//MatrixIdentity(local);
	//MatrixTranslate(-700.0f, -100.0f, -700.0f, local);
	//MatrixRotate(-PI/2.0f, 1.0f, 0.0f, 0.0f, local);
	//MatrixRotate(-PI/2.0f, 0.0f, 0.0f, 1.0f, local);
	//glUniformMatrix4fv(Objects[GLSL_LIGHT_LOCAL], 1, GL_FALSE, local);
	//glBindTextureUnit(0, Fatty.Base);
	//glBindTextureUnit(1, Fatty.Specular);
	//glBindTextureUnit(2, Fatty.Normal);
	//DrawModelMD5(&Fatty.Model);

	MatrixIdentity(local);
	//MatrixTranslate(700.0f, -100.0f, -700.0f, local);
	MatrixRotate(-PI/2.0f, 1.0f, 0.0f, 0.0f, local);
	MatrixRotate(-PI/2.0f, 0.0f, 0.0f, 1.0f, local);
	glUniformMatrix4fv(Objects[GLSL_LIGHT_LOCAL], 1, GL_FALSE, local);
	//glBindTextureUnit(0, Pinky.Base);
	//glBindTextureUnit(1, Pinky.Specular);
	//glBindTextureUnit(2, Pinky.Normal);
	//DrawModelMD5(&Pinky.Model);

//	MatrixIdentity(local);
//	glUniformMatrix4fv(Objects[GLSL_LIGHT_LOCAL], 1, GL_FALSE, local);
//	DrawModelOBJ(&Level);

	glBindTextureUnit(0, Level.Material[1].TexBaseID);
	glBindTextureUnit(1, Level.Material[1].TexSpecularID);
	glBindTextureUnit(2, Level.Material[1].TexNormalID);
	DrawQ2BSP(&Q2Model);

	DrawSkybox();

	///// Particle system stuff

	// Hellknight's local transform matrix
	MatrixIdentity(local);
	MatrixTranslate(0.0f, -100.0f, 0.0f, local);
	MatrixRotate(-PI/2.0f, 1.0f, 0.0f, 0.0f, local);
	MatrixRotate(-PI/2.0f, 0.0f, 0.0f, 1.0f, local);

	// Hellknight's left and right hand locations (joints 16 and 53), and mouth
	// Transform locations into our space and where the hellknight is located
	vec3 left, right, mouth;
	Matrix4x4MultVec3(&Hellknight.Skel[8*16], local, left);
	Matrix4x4MultVec3(&Hellknight.Skel[8*53], local, right);
	Matrix4x4MultVec3(&Hellknight.Skel[8*31], local, mouth);

	// Attach the fire-like emitter to the mouth
	ParticleSystem_SetEmitterPosition(&ParticleSystem, EmitterIDs[0], mouth);

	// Set the two green sparklers to those locations
	ParticleSystem_SetEmitterPosition(&ParticleSystem, EmitterIDs[2], left);
	ParticleSystem_SetEmitterPosition(&ParticleSystem, EmitterIDs[3], right);

	//Lights_UpdatePosition(&Lights, LightIDs[5], mouth);
	//Lights_UpdatePosition(&Lights, LightIDs[6], left);
	//Lights_UpdatePosition(&Lights, LightIDs[7], right);

	vec3 temp;

	Vec3_Set(temp, -700.0f+sinf(fTime*4.0f)*50.0f, -50.0f+sinf(fTime*2.0f)*50.0f, -700.0f+cosf(fTime*4.0f)*50.0f);
	ParticleSystem_SetEmitterPosition(&ParticleSystem, EmitterIDs[4], temp);
	//Lights_UpdatePosition(&Lights, LightIDs[8], temp);

	Vec3_Set(temp, 700.0f+sinf(fTime*4.0f)*100.0f, -50.0f+sinf(fTime*2.0f)*20.0f, -700.0f+cosf(fTime*4.0f)*100.0f);
	ParticleSystem_SetEmitterPosition(&ParticleSystem, EmitterIDs[5], temp);
	//Lights_UpdatePosition(&Lights, LightIDs[9], temp);

	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
//	ParticleSystem_Step(&ParticleSystem, fTimeStep);
//	ParticleSystem_Draw(&ParticleSystem);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	/////

	///// Beam stuff
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_ONE, GL_ONE);
	//DrawBeam(BeamStart0, BeamEnd0, (vec3) { 1.0f, 1.0f, 1.0f }, radius);
	//DrawBeam(BeamStart1, BeamEnd1, (vec3) { 1.0f, 1.0f, 1.0f }, radius);
	//glDisable(GL_BLEND);
	/////

	///// Volume rendering
	//glUseProgram(Objects[GLSL_VOL_SHADER]);
	//glUniformMatrix4fv(Objects[GLSL_VOL_PROJ], 1, GL_FALSE, Projection);
	//glUniformMatrix4fv(Objects[GLSL_VOL_MV], 1, GL_FALSE, ModelView);

	//MatrixIdentity(local);
	//MatrixTranslate(0.0f, 0.0f, 100.0f, local);
	//MatrixScale(50.0f, 50.0f, 50.0f, local);
	//glUniformMatrix4fv(Objects[GLSL_VOL_LOCAL], 1, GL_FALSE, local);

	//glBindTextureUnit(0, Objects[TEXTURE_FLUID]);
	//glBindTextureUnit(1, Objects[TEXTURE_TRANSFER]);

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//DrawSkybox();
	//glDisable(GL_BLEND);
	/////

	glUseProgram(Objects[GLSL_GENERIC_SHADER]);

	glBindVertexArray(VAO);
	glDrawArrays(GL_LINE_STRIP, 0, NUM_SAMPLES);

	glBindVertexArray(0);

	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
		Font_Print(0.0f, 16.0f, "FPS: %0.1f\nFrame time: %0.4fms", fps, fFrameTime);
		//if(collide)
		//	Font_Print(0.0f, (float)Height-16.0f, "Ran into hellknight");
		Font_Print(0.0f, (float)Height-16.0f, "Number of emitters: %d\nNumber of lights: %d\n\nPress \"enter\" for explosion", List_GetCount(&ParticleSystem.Emitters), List_GetCount(&Lights.Lights));
		glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void HandEmitterCallback(uint32_t Index, uint32_t NumParticles, Particle_t *Particle)
{
	Vec3_Sets(Particle->pos, 0.0f);

	// Simple -1.0 to 1.0 random spherical pattern, scaled by 100, fairly short lifespan.
	Vec3_Set(Particle->vel, ((float)rand()/RAND_MAX)*2.0f-1.0f, ((float)rand()/RAND_MAX)*2.0f-1.0f, ((float)rand()/RAND_MAX)*2.0f-1.0f);
	Vec3_Normalize(Particle->vel);
	Vec3_Muls(Particle->vel, 50.0f);

	Particle->life=((float)rand()/RAND_MAX)*0.5f+0.01f;
}

void ExplosionEmitterCallback(uint32_t Index, uint32_t NumParticles, Particle_t *Particle)
{
	// Does a neat helical pattern, similar to Quake 2's railgun
	const float TwoPi=PI*2.0f;
	const float SeedRadius=10.0f;
	float fi=(float)Index/NumParticles;
	float theta=((float)rand()/RAND_MAX)*TwoPi;
	float r=((float)rand()/RAND_MAX)*SeedRadius;

	Particle->pos[0]=fi*200.0f-100.0f;
	Particle->pos[1]=sinf(fi*TwoPi*8.0f)*50.0f;
	Particle->pos[2]=cosf(fi*TwoPi*8.0f)*50.0f;

	Particle->vel[0]=r*sinf(theta);
	Particle->vel[1]=((float)rand()/RAND_MAX)*50.0f;
	Particle->vel[2]=r*cosf(theta);

	Particle->life=((float)rand()/RAND_MAX)*0.999f+0.001f;
}

bool Init(void)
{
	if(Lights_Init(&Lights))
	{
		//LightIDs[0]=Lights_Add(&Lights, (vec3) { 0.0f, 50.0f, 200.0f }, 500.0f, (vec4) { 1.0f, 1.0f, 1.0f, 1.0f });
		//LightIDs[1]=Lights_Add(&Lights, (vec3) { -800.0f, 80.0f, 800.0f }, 1000.0f, (vec4) { 0.75f, 0.75f, 1.0f, 1.0f });
		//LightIDs[2]=Lights_Add(&Lights, (vec3) { 800.0f, 80.0f, 800.0f }, 1000.0f, (vec4) { 0.75f, 1.0f, 1.0f, 1.0f });
		//LightIDs[3]=Lights_Add(&Lights, (vec3) { -800.0f, 80.0f, -800.0f }, 1000.0f, (vec4) { 0.75f, 1.0f, 0.75f, 1.0f });
		//LightIDs[4]=Lights_Add(&Lights, (vec3) { 800.0f, 80.0f, -800.0f }, 1000.0f, (vec4) { 1.0f, 0.75f, 0.75f, 1.0f });

		//LightIDs[5]=Lights_Add(&Lights, (vec3) { 0.0f, 0.0f, 0.0f }, 100.0f, (vec4) { 0.12f, 0.03f, 0.0f, 1.0f });
		//LightIDs[6]=Lights_Add(&Lights, (vec3) { 0.0f, 0.0f, 0.0f }, 75.0f, (vec4) { 0.0f, 1.0f, 0.0f, 1.0f });
		//LightIDs[7]=Lights_Add(&Lights, (vec3) { 0.0f, 0.0f, 0.0f }, 75.0f, (vec4) { 0.0f, 1.0f, 0.0f, 1.0f });

		//LightIDs[8]=Lights_Add(&Lights, (vec3) { 0.0f, 0.0f, 0.0f }, 250.0f, (vec4) { 0.1f, 0.1f, 1.0f, 1.0f });
		//LightIDs[9]=Lights_Add(&Lights, (vec3) { 0.0f, 0.0f, 0.0f }, 250.0f, (vec4) { 0.1f, 0.1f, 1.0f, 1.0f });
	}

	LoadQ2BSP(&Q2Model, "assets/base1.bsp");

	if(ParticleSystem_Init(&ParticleSystem))
	{
		EmitterIDs[0]=ParticleSystem_AddEmitter(&ParticleSystem,
		(vec3) { 0.0f, 0.0f, 0.0f },	// Position of emitter
		(vec3) { 0.1f, 0.1f, 0.1f },	// Starting color
		(vec3) { 0.12f, 0.03f, 0.0f },	// Ending color
		8.0f,							// Particle size
		5000,							// Number of particles
		false,							// "burst" (ResetEmitter triggers)
		NULL);							// Emitter particle rebirth callback (NULL = use build-in default)

		EmitterIDs[1]=ParticleSystem_AddEmitter(&ParticleSystem,
		(vec3) { 0.0f, 0.0f, 100.0f },
		(vec3) { 0.0f, 0.0f, 1.0f },
		(vec3) { 0.05f, 0.05f, 0.05f },
		8.0f,
		10000, true, ExplosionEmitterCallback);

		EmitterIDs[2]=ParticleSystem_AddEmitter(&ParticleSystem,
		(vec3) { 0.0f, 0.0f, 0.0f },
		(vec3) { 1.0f, 1.0f, 1.0f },
		(vec3) { 0.0f, 1.0f, 0.0f },
		4.0f,
		1000, false, HandEmitterCallback);

		EmitterIDs[3]=ParticleSystem_AddEmitter(&ParticleSystem,
		(vec3) { 0.0f, 0.0f, 0.0f },
		(vec3) { 1.0f, 1.0f, 1.0f },
		(vec3) { 0.0f, 1.0f, 0.0f },
		4.0f,
		1000, false, HandEmitterCallback);

		EmitterIDs[4]=ParticleSystem_AddEmitter(&ParticleSystem,
		(vec3) { 0.0f, 0.0f, 0.0f },
		(vec3) { 1.0f, 1.0f, 1.0f },
		(vec3) { 0.1f, 0.1f, 1.0f },
		8.0f,
		1000, false, NULL);

		EmitterIDs[5]=ParticleSystem_AddEmitter(&ParticleSystem,
		(vec3) { 0.0f, 0.0f, 0.0f },
		(vec3) { 1.0f, 1.0f, 1.0f },
		(vec3) { 0.1f, 0.1f, 1.0f },
		8.0f,
		1000, false, HandEmitterCallback);
	}

	if(Audio_Init())
	{
		if(!Audio_LoadStatic("./assets/hellknight_idle.wav", &Hellknight_Idle))
			return false;

		Vec3_Set(Hellknight_Idle.xyz, 0.0f, 0.0f, 0.0f);
	}
	else
		return false;

	glDebugMessageCallback(error_callback, NULL);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glEnable(GL_DEBUG_OUTPUT);

	InitBeam();

	BuildSkyboxVBO();

	/// Line chart buffers
	glCreateVertexArrays(1, &VAO);

	glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(VAO, 0, 0);
	glEnableVertexArrayAttrib(VAO, 0);

	glCreateBuffers(1, &VBO);
	memset(lines, 0, sizeof(float)*3*NUM_SAMPLES+3);
	glNamedBufferData(VBO, sizeof(float)*3*NUM_SAMPLES+3, lines, GL_STREAM_DRAW);

	glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(float)*3);
	///

#if CAMERA_RECORDING
	CameraInit(&Camera, (float[]) { 0.0f, 0.0f, 0.0f }, (float[]) { 0.0f, 0.0f, 1.0f }, (float[3]) { 0.0f, 1.0f, 0.0f });
#else
	if(!CameraLoadPath("path.txt", &CameraPath))
		return false;

	// Set up camera structs
	CameraInit(&Camera, CameraPath.Position, (vec3) { -1.0f, 0.0f, 0.0f }, (vec3) { 0.0f, 1.0f, 0.0f });
#endif

	// Load the "level" Alias/Wavefront model
	if(LoadOBJ(&Level, "./assets/room.obj"))
	{
		BuildVBOOBJ(&Level);
		LoadMaterialsOBJ(&Level);
	}
	else
		return false;

	// Compile/link MD5 skinning compute program
	Objects[GLSL_MD5_GENVERTS_COMPUTE]=CreateShaderProgram((ProgNames_t) { NULL, NULL, NULL, "./shaders/md5_genverts_c.glsl" });

	// Load MD5 model meshes
	if(!LoadMD5Model("./assets/hellknight", &Hellknight))
		return false;

	if(!LoadMD5Model("./assets/fatty", &Fatty))
		return false;

	if(!LoadMD5Model("./assets/pinky", &Pinky))
		return false;

	// Load shaders

	// Generic debugging shader
	Objects[GLSL_GENERIC_SHADER]=CreateShaderProgram((ProgNames_t) { "./shaders/generic_v.glsl", "./shaders/generic_f.glsl", NULL, NULL });

	///// Volume rendering stuff
	//Objects[GLSL_VOL_SHADER]=CreateShaderProgram((ProgNames_t) { "./shaders/vol_v.glsl", "./shaders/vol_f.glsl", NULL, NULL });
	//glUseProgram(Objects[GLSL_VOL_SHADER]);
	//Objects[GLSL_VOL_PROJ]=glGetUniformLocation(Objects[GLSL_VOL_SHADER], "proj");
	//Objects[GLSL_VOL_MV]=glGetUniformLocation(Objects[GLSL_VOL_SHADER], "mv");
	//Objects[GLSL_VOL_LOCAL]=glGetUniformLocation(Objects[GLSL_VOL_SHADER], "local");

	//float transferFunc[]=
	//{
	//	0.0f, 0.0f, 0.0f, 0.0f,
	//	0.0f, 0.0f, 1.0f, 1.0f,
	//	0.0f, 0.5f, 1.0f, 1.0f,
	//	0.0f, 1.0f, 1.0f, 1.0f,
	//	0.0f, 1.0f, 1.0f, 1.0f,
	//	0.0f, 0.5f, 1.0f, 1.0f,
	//	0.0f, 0.0f, 1.0f, 1.0f,
	//	0.0f, 0.0f, 0.0f, 0.0f
	//};

	//glGenTextures(1, &Objects[TEXTURE_TRANSFER]);
	//glBindTexture(GL_TEXTURE_1D, Objects[TEXTURE_TRANSFER]);
	//glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, 8, 0, GL_RGBA, GL_FLOAT, transferFunc);

	//if(!Fluid3D_Init(&Fluid, 128, 128, 128, 0.0f, 0.0f))
	//	return false;
	/////


	// General lighting shader
	Objects[GLSL_LIGHT_SHADER]=CreateShaderProgram((ProgNames_t) { "./shaders/light_v.glsl", "./shaders/light_f.glsl", NULL/*"./shaders/tbnvis_g.glsl"*/, NULL });
	glUseProgram(Objects[GLSL_LIGHT_SHADER]);
	Objects[GLSL_LIGHT_PROJ]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "proj");
	Objects[GLSL_LIGHT_MV]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "mv");
	Objects[GLSL_LIGHT_LOCAL]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "local");
	Objects[GLSL_LIGHT_NUMLIGHTS]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "NumLights");
	Objects[GLSL_LIGHT_BEAM_START0]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "Beam_Start0");
	Objects[GLSL_LIGHT_BEAM_END0]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "Beam_End0");
	Objects[GLSL_LIGHT_BEAM_START1]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "Beam_Start1");
	Objects[GLSL_LIGHT_BEAM_END1]=glGetUniformLocation(Objects[GLSL_LIGHT_SHADER], "Beam_End1");

	// Build program for generating depth cube map
	Objects[GLSL_DISTANCE_SHADER]=CreateShaderProgram((ProgNames_t) { "./shaders/distance_v.glsl", "./shaders/distance_f.glsl", "./shaders/distance_g.glsl", NULL });
	glUseProgram(Objects[GLSL_DISTANCE_SHADER]);
	Objects[GLSL_DISTANCE_PROJ]=glGetUniformLocation(Objects[GLSL_DISTANCE_SHADER], "proj");
	Objects[GLSL_DISTANCE_MV]=glGetUniformLocation(Objects[GLSL_DISTANCE_SHADER], "mv");
	Objects[GLSL_DISTANCE_LOCAL]=glGetUniformLocation(Objects[GLSL_DISTANCE_SHADER], "local");
	Objects[GLSL_DISTANCE_LIGHTPOS]=glGetUniformLocation(Objects[GLSL_DISTANCE_SHADER], "Light_Pos");
	Objects[GLSL_DISTANCE_INDEX]=glGetUniformLocation(Objects[GLSL_DISTANCE_SHADER], "index");

	// Genereate texture and frame buffer for the depth cube map
	glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &Objects[TEXTURE_DISTANCE]);
	glTextureParameteri(Objects[TEXTURE_DISTANCE], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(Objects[TEXTURE_DISTANCE], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(Objects[TEXTURE_DISTANCE], GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(Objects[TEXTURE_DISTANCE], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(Objects[TEXTURE_DISTANCE], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(Objects[TEXTURE_DISTANCE], GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTextureParameteri(Objects[TEXTURE_DISTANCE], GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTextureStorage3D(Objects[TEXTURE_DISTANCE], 1, GL_DEPTH_COMPONENT32, DynSize, DynSize, 6*(GLsizei)List_GetCount(&Lights.Lights));

	glCreateFramebuffers(1, &Objects[BUFFER_DISTANCE]);
	glBindFramebuffer(GL_FRAMEBUFFER, Objects[BUFFER_DISTANCE]);
	glNamedFramebufferTexture(Objects[BUFFER_DISTANCE], GL_DEPTH_ATTACHMENT, Objects[TEXTURE_DISTANCE], 0);

	// Disable drawing, we're only interested in depth information
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Set OpenGL states
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	return true;
}

void Destroy(void)
{
	Lights_Destroy(&Lights);

	ParticleSystem_Destroy(&ParticleSystem);

//	Fluid3D_Destroy(&Fluid);

	Audio_Destroy();
	// Side note: Be sure to free audio data *after* sound system shutdown,
	// it might still be trying to read the data... >_>
	FREE(Hellknight_Idle.data);

	DestroyBeam();

	CameraDeletePath(&CameraPath);

	FreeOBJ(&Level);

	DestroyMD5Model(&Hellknight);
	DestroyMD5Model(&Fatty);
	DestroyMD5Model(&Pinky);
}
