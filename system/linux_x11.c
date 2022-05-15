#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <sys/time.h>
#include <strings.h>
#include <stdio.h>
#include <stdint.h>
#include "../opengl/opengl.h"
#include "../math/math.h"
#include "../camera/camera.h"
#include "system.h"

Display *dpy;

GLContext_t Context;

char szAppName[]="OpenGL";

extern int32_t Width, Height;

uint8_t Key[65536];

uint64_t Frequency, StartTime, EndTime, EndFrameTime;
float avgfps=0.0f, fps=0.0f, fTimeStep=0.0f, fFrameTime=0.0f, fTime=0.0f;
int32_t Frames=0;

int32_t Auto=0;

extern Camera_t Camera;

void Render(void);
int32_t Init(void);
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
	int32_t Done=0;

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
					Key[Keysym]=1;

					switch(Keysym)
					{
						case 'w':
							Camera.key_w=1;
							break;

						case 's':
							Camera.key_s=1;
							break;

						case 'a':
							Camera.key_a=1;
							break;

						case 'd':
							Camera.key_d=1;
							break;

						case 'v':
							Camera.key_v=1;
							break;

						case 'c':
							Camera.key_c=1;
							break;

						case 'q':
							Camera.key_q=1;
							break;

						case 'e':
							Camera.key_e=1;
							break;

						case XK_Up:
							Camera.key_up=1;
							break;

						case XK_Down:
							Camera.key_down=1;
							break;

						case XK_Left:
							Camera.key_left=1;
							break;

						case XK_Right:
							Camera.key_right=1;
							break;

						case XK_Escape:
							Done=1;
							break;

						case XK_space:
							Auto^=1;
							break;

						default:
							break;
					}
					break;

				case KeyRelease:
					Keysym=XLookupKeysym(&Event.xkey, 0);
					Key[Keysym]=0;

					switch(Keysym)
					{
						case 'w':
							Camera.key_w=0;
							break;

						case 's':
							Camera.key_s=0;
							break;

						case 'a':
							Camera.key_a=0;
							break;

						case 'd':
							Camera.key_d=0;
							break;

						case 'v':
							Camera.key_v=0;
							break;

						case 'c':
							Camera.key_c=0;
							break;

						case 'q':
							Camera.key_q=0;
							break;

						case 'e':
							Camera.key_e=0;
							break;

						case XK_Up:
							Camera.key_up=0;
							break;

						case XK_Down:
							Camera.key_down=0;
							break;

						case XK_Left:
							Camera.key_left=0;
							break;

						case XK_Right:
							Camera.key_right=0;
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
