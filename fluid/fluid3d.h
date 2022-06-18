#ifndef __FLUID_H__
#define __FLUID_H__

#undef FLUID3D_CPU

typedef struct
{
	int w, h, d;

	float diff;
	float visc;

#ifdef FLUID3D_CPU
	float *d0, *d1;
	float *u0, *u1;
	float *v0, *v1;
	float *w0, *w1;
#else
	OpenCLContext_t Context;

	cl_program Program;

	cl_kernel AddDensityVelocity;
	cl_kernel SetDensityVelocity;
	cl_kernel Diffuse;
	cl_kernel Advect;
	cl_kernel Project1;
	cl_kernel Project2;
	cl_kernel Project3;

	cl_mem d0, d1;
	cl_mem u0, u1;
	cl_mem v0, v1;
	cl_mem w0, w1;

	float *den;
#endif
} Fluid3D_t;

bool Fluid3D_Init(Fluid3D_t *Fluid, int width, int height, int depth, float diffusion, float viscosity);
void Fluid3D_Destroy(Fluid3D_t *Fluid);
void Fluid3D_Step(Fluid3D_t *Fluid, float dt);
float Fluid3D_GetDensity(Fluid3D_t *Fluid, int x, int y, int z);
void Fluid3D_AddDensityVelocity(Fluid3D_t *Fluid, int x, int y, int z, float u, float v, float w, float d);
void Fluid3D_SetDensityVelocity(Fluid3D_t *Fluid, int x, int y, int z, float u, float v, float w, float d);

#endif
