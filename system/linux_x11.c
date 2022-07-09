#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <sys/time.h>
#include <strings.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "../opengl/opengl.h"
#include "../math/math.h"
#include "../camera/camera.h"
#include "../particle/particle.h"
#include "system.h"

Display *dpy;

GLContext_t Context;

char szAppName[]="OpenGL";

extern int32_t Width, Height;

bool Key[65536];

uint64_t Frequency, StartTime, EndTime, EndFrameTime;
float avgfps=0.0f, fps=0.0f, fTimeStep=0.0f, fFrameTime=0.0f, fTime=0.0f;
int32_t Frames=0;

bool Auto=0;

extern Camera_t Camera;

extern ParticleSystem_t ParticleSystem;
extern int32_t EmitterIDs[4];

void Render(void);
bool Init(void);
void Destroy(void);
void UpdateLineChart(const float val);

uint64_t rdtsc(void)
{
	uint32_t l, h;

	__asm__ __volatile__ ("rdtsc" : "=a" (l), "=d" (h));

	return (uint64_t)l|((uint64_t)h<<32);
}

unsigned long long GetFrequency(void)
{
	uint64_t StartTicks, StopTicks;
	struct timeval TimeStart, TimeStop;
	volatile uint32_t i;

	gettimeofday(&TimeStart, NULL);
	StartTicks=rdtsc();

	for(i=0;i<1000000;i++);

	StopTicks=rdtsc();
	gettimeofday(&TimeStop, NULL);

	return (StopTicks-StartTicks)*1000000/(TimeStop.tv_usec-TimeStart.tv_usec);
}

void EventLoop(void)
{
	int32_t Keysym;
	XEvent Event;
	int32_t ox, oy, dx, dy;
	bool Done=false;

	while(!Done)
	{
		while(XPending(dpy)>0)
		{
			ox=Event.xmotion.x;
			oy=Event.xmotion.y;

			XNextEvent(dpy, &Event);

			switch(Event.type)
			{
				case MotionNotify:
					dx=ox-Event.xmotion.x;
					dy=oy-Event.xmotion.y;

					if(Event.xmotion.state&Button1Mask)
					{
						Camera.Yaw+=(float)dx/800.0f;
						Camera.Pitch+=(float)dy/800.0f;
					}

					if(Event.xmotion.state&Button2Mask)
					{
					}

					if(Event.xmotion.state&Button3Mask)
					{
					}
					break;

				case Expose:
					break;

				case ConfigureNotify:
					Width=Event.xconfigure.width;
					Height=Event.xconfigure.height;
					break;

				case ButtonPress:
					break;

				case KeyPress:
					Keysym=XLookupKeysym(&Event.xkey, 0);
					Key[Keysym]=true;

					switch(Keysym)
					{
						case XK_Delete:
							ParticleSystem_DeleteEmitter(&ParticleSystem, EmitterIDs[2]);
							break;
					
						case XK_Return:
							ParticleSystem_ResetEmitter(&ParticleSystem, EmitterIDs[1]);
							break;

						case 'w':
							Camera.key_w=true;
							break;

						case 's':
							Camera.key_s=true;
							break;

						case 'a':
							Camera.key_a=true;
							break;

						case 'd':
							Camera.key_d=true;
							break;

						case 'v':
							Camera.key_v=true;
							break;

						case 'c':
							Camera.key_c=true;
							break;

						case 'q':
							Camera.key_q=true;
							break;

						case 'e':
							Camera.key_e=true;
							break;

						case XK_Up:
							Camera.key_up=true;
							break;

						case XK_Down:
							Camera.key_down=true;
							break;

						case XK_Left:
							Camera.key_left=true;
							break;

						case XK_Right:
							Camera.key_right=true;
							break;

						case XK_Escape:
							Done=true;
							break;

						case XK_space:
							Auto^=true;
							break;

						default:
							break;
					}
					break;

				case KeyRelease:
					Keysym=XLookupKeysym(&Event.xkey, 0);
					Key[Keysym]=false;

					switch(Keysym)
					{
						case 'w':
							Camera.key_w=false;
							break;

						case 's':
							Camera.key_s=false;
							break;

						case 'a':
							Camera.key_a=false;
							break;

						case 'd':
							Camera.key_d=false;
							break;

						case 'v':
							Camera.key_v=false;
							break;

						case 'c':
							Camera.key_c=false;
							break;

						case 'q':
							Camera.key_q=false;
							break;

						case 'e':
							Camera.key_e=false;
							break;

						case XK_Up:
							Camera.key_up=false;
							break;

						case XK_Down:
							Camera.key_down=false;
							break;

						case XK_Left:
							Camera.key_left=false;
							break;

						case XK_Right:
							Camera.key_right=false;
							break;

						default:
							break;
					}
					break;
			}
		}

		StartTime=rdtsc();
		Render();
		EndFrameTime=rdtsc();
		glXSwapBuffers(dpy, Context.win);
		EndTime=rdtsc();

		// Total screen time in seconds
		fTimeStep=(float)(EndTime-StartTime)/Frequency;
		// Total frame render in miliseconds
		fFrameTime=(float)(EndFrameTime-StartTime)/(Frequency/1000);
		// Running time
		fTime+=fTimeStep;

		// Accumulate frames per second
		avgfps+=1.0f/fTimeStep;

		// Average over 100 frames
		if(Frames++>100)
		{
			fps=avgfps/Frames;
			avgfps=0.0f;
			Frames=0;
		}

		UpdateLineChart(fFrameTime);
	}
}

int main(int argc, char **argv)
{
	XVisualInfo *visinfo=NULL;
	XSetWindowAttributes Attrib;
	Window root;

	DBGPRINTF("Opening X display...\n");
	dpy=XOpenDisplay(NULL);

	if(dpy==NULL)
	{
		DBGPRINTF("\t...can't open display.\n");

		return -1;
	}

	DBGPRINTF("Creating OpenGL context...\n");
	if(!CreateContext(&Context, 24, 24, 0, 0, OGL_CORE45|OGL_DOUBLEBUFFER))
	{
		DBGPRINTF("\t...failed.\n");
		XCloseDisplay(dpy);

		return -1;
	}

	if(Context.visualid)
	{
		XVisualInfo visinfotemp;
		int visinfoitems;

		visinfotemp.visualid=Context.visualid;
		visinfo=XGetVisualInfo(dpy, VisualIDMask, &visinfotemp, &visinfoitems);

		if(visinfo==NULL&&visinfoitems!=1)
			return -1;
	}

	root=RootWindow(dpy, DefaultScreen(dpy));

	Attrib.background_pixel=0;
	Attrib.border_pixel=0;
	Attrib.colormap=XCreateColormap(dpy, root, visinfo->visual, AllocNone);
	Attrib.event_mask=StructureNotifyMask|PointerMotionMask|ExposureMask|ButtonPressMask|KeyPressMask|KeyReleaseMask;

	Context.win=XCreateWindow(dpy, root, 0, 0, Width, Height, 0, visinfo->depth, InputOutput, visinfo->visual, CWBackPixel|CWBorderPixel|CWColormap|CWEventMask, &Attrib);
	XStoreName(dpy, Context.win, szAppName);

	XFree(visinfo);

	glXMakeCurrent(dpy, Context.win, Context.ctx);

	DBGPRINTF("\nOpenGL vendor: %s\n", glGetString(GL_VENDOR));
	DBGPRINTF("OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	DBGPRINTF("OpenGL Version: %s\n", glGetString(GL_VERSION));

	DBGPRINTF("Initalizing OpenGL resources...\n");
	if(!Init())
	{
		DBGPRINTF("\t...failed.\n");

		glXMakeCurrent(dpy, None, NULL);
		XDestroyWindow(dpy, Context.win);
		DestroyContext(&Context);

		XCloseDisplay(dpy);

		return -1;
	}

	XMapWindow(dpy, Context.win);

	Frequency=GetFrequency();
	DBGPRINTF("\nCPU freqency: %0.2fGHz\n", (float)Frequency/1000000000);

	DBGPRINTF("\nStarting main loop.\n");
	EventLoop();

	DBGPRINTF("Shutting down...\n");
	Destroy();

	glXMakeCurrent(dpy, None, NULL);
	XDestroyWindow(dpy, Context.win);
	DestroyContext(&Context);

	XCloseDisplay(dpy);

	DBGPRINTF("Exit\n");

	return 0;
}
