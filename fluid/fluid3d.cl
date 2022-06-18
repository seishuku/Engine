#define IX(x, y, z, sx, sy, sz) (min(sx-1, max(1, (x)))+min(sy-1, max(1, (y)))*sx+min(sz-1, max(1, (z)))*sx*sx)

kernel void set_density_velocity(global float *u0, global float *v0, global float *w0, global float *d0, int x, int y, int z, float u, float v, float w, float den, int width, int height, int depth)
{
	u0[IX(x, y, z, width, height, depth)]=u;
	v0[IX(x, y, z, width, height, depth)]=v;
	w0[IX(x, y, z, width, height, depth)]=w;
	d0[IX(x, y, z, width, height, depth)]=den;
}

kernel void add_density_velocity(global float *u0, global float *v0, global float *w0, global float *d0, int x, int y, int z, float u, float v, float w, float den, int width, int height, int depth)
{
	u0[IX(x, y, z, width, height, depth)]+=u;
	v0[IX(x, y, z, width, height, depth)]+=v;
	w0[IX(x, y, z, width, height, depth)]+=w;
	d0[IX(x, y, z, width, height, depth)]+=den;
}

void lin_solve(float *x, float *x0, float a, float c, int iter)
{
	int i=get_global_id(0);
	int j=get_global_id(1);
	int k=get_global_id(2);
	int w=get_global_size(0);
	int h=get_global_size(1);
	int d=get_global_size(2);
	float cRecip=1.0f/c;

	for(int l=0;l<iter;l++)
	{
		x[IX(i, j, k, w, h, d)]=
		(
			x0[IX(i, j, k, w, h, d)]+
			a*
			(
				x[IX(i+1, j, k, w, h, d)]+
				x[IX(i-1, j, k, w, h, d)]+
				x[IX(i, j+1, k, w, h, d)]+
				x[IX(i, j-1, k, w, h, d)]+
				x[IX(i, j, k+1, w, h, d)]+
				x[IX(i, j, k-1, w, h, d)]
			)
		)*cRecip;
	}
}

kernel void diffuse(global float *x, global float *x0, float diff, float dt, int iter)
{
	int w=get_global_size(0);
	int h=get_global_size(1);
	int d=get_global_size(2);
	float a=dt*diff*(w-2)*(d-2);

	lin_solve(x, x0, a, 1.0f+6.0f*a, iter);
}

kernel void advect(global float *dst, global float *d0, global float *velocX, global float *velocY, global float *velocZ, float dt)
{
	int i=get_global_id(0);
	int j=get_global_id(1);
	int k=get_global_id(2);
	int w=get_global_size(0);
	int h=get_global_size(1);
	int d=get_global_size(2);
	float dtx=dt*(w-2);
	float dty=dt*(h-2);
	float dtz=dt*(d-2);

	float x=(float)i-dtx*velocX[IX(i, j, k, w, h, d)];
	float y=(float)j-dty*velocY[IX(i, j, k, w, h, d)];
	float z=(float)k-dtz*velocZ[IX(i, j, k, w, h, d)];

	if(x<0.5f)
		x=0.5f;

	if(x>(float)w+0.5f)
		x=(float)w+0.5f;

	int i0=(int)floor(x);
	int i1=i0+1;

	if(y<0.5f)
		y=0.5f;

	if(y>(float)h+0.5f)
		y=(float)h+0.5f;

	int j0=(int)floor(y);
	int j1=j0+1;

	if(z<0.5f)
		z=0.5f;

	if(z>(float)d+0.5f)
		z=(float)d+0.5f;

	int k0=(int)floor(z);
	int k1=k0+1;

	float s1=x-i0;
	float s0=1.0f-s1;
	float t1=y-j0;
	float t0=1.0f-t1;
	float r1=z-k0;
	float r0=1.0f-r1;

	dst[IX(i, j, k, w, h, d)]=
		s0*
		(
			t0*
			(
				r0*d0[IX(i0, j0, k0, w, h, d)]+
				r1*d0[IX(i0, j0, k1, w, h, d)]
			)+
			t1*
			(
				r0*d0[IX(i0, j1, k0, w, h, d)]+
				r1*d0[IX(i0, j1, k1, w, h, d)]
			)
		)+
		s1*
		(
			t0*
			(
				r0*d0[IX(i1, j0, k0, w, h, d)]+
				r1*d0[IX(i1, j0, k1, w, h, d)]
			)+
			t1*
			(
				r0*d0[IX(i1, j1, k0, w, h, d)]+
				r1*d0[IX(i1, j1, k1, w, h, d)]
			)
		);
}

kernel void project1(global float *velocX, global float *velocY, global float *velocZ, global float *p, global float *div, int iter)
{
	int i=get_global_id(0);
	int j=get_global_id(1);
	int k=get_global_id(2);
	int w=get_global_size(0);
	int h=get_global_size(1);
	int d=get_global_size(2);

	div[IX(i, j, k, w, h, d)]=
		-0.5f*
		(
			velocX[IX(i+1, j, k, w, h, d)]-
			velocX[IX(i-1, j, k, w, h, d)]+
			velocY[IX(i, j+1, k, w, h, d)]-
			velocY[IX(i, j-1, k, w, h, d)]+
			velocZ[IX(i, j, k+1, w, h, d)]-
			velocZ[IX(i, j, k-1, w, h, d)]
		)/w;

	p[IX(i, j, k, w, h, d)]=0.0f;
}

kernel void project2(global float *velocX, global float *velocY, global float *velocZ, global float *p, global float *div, int iter)
{
	lin_solve(p, div, 1, 6, iter);
}

kernel void project3(global float *velocX, global float *velocY, global float *velocZ, global float *p, global float *div, int iter)
{
	int i=get_global_id(0);
	int j=get_global_id(1);
	int k=get_global_id(2);
	int w=get_global_size(0);
	int h=get_global_size(1);
	int d=get_global_size(2);

	velocX[IX(i, j, k, w, h, d)]-=0.5f*(p[IX(i+1, j, k, w, h, d)]-p[IX(i-1, j, k, w, h, d)])*w;
	velocY[IX(i, j, k, w, h, d)]-=0.5f*(p[IX(i, j+1, k, w, h, d)]-p[IX(i, j-1, k, w, h, d)])*h;
	velocZ[IX(i, j, k, w, h, d)]-=0.5f*(p[IX(i, j, k+1, w, h, d)]-p[IX(i, j, k-1, w, h, d)])*d;
}
