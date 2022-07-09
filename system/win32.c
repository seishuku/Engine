#include <windows.h>
#include <process.h>
#include <intrin.h>
#include <crtdbg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#pragma intrinsic(__rdtsc)

#include "../system/system.h"
#include "../opengl/opengl.h"
#include "../math/math.h"
#include "../camera/camera.h"
#include "../particle/particle.h"

GLContext_t Context;

char szAppName[]="OpenGL";

extern int32_t Width, Height;

bool Done=0, Key[256];

unsigned __int64 Frequency, StartTime, EndTime, EndFrameTime;
float avgfps=0.0f, fps=0.0f, fTimeStep=0.0f, fFrameTime=0.0f, fTime=0.0f;
int32_t Frames=0;

bool Auto=0;

extern Camera_t Camera;

extern ParticleSystem_t ParticleSystem;
extern int32_t EmitterIDs[4];

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void Render(void);
bool Init(void);
void Destroy(void);
void UpdateLineChart(const float val);

unsigned __int64 rdtsc(void)
{
	return __rdtsc();
}

unsigned __int64 GetFrequency(void)
{
	unsigned __int64 TimeStart, TimeStop, TimeFreq;
	unsigned __int64 StartTicks, StopTicks;
	volatile unsigned __int64 i;

	QueryPerformanceFrequency((LARGE_INTEGER *)&TimeFreq);

	QueryPerformanceCounter((LARGE_INTEGER *)&TimeStart);
	StartTicks=rdtsc();

	for(i=0;i<1000000;i++);

	StopTicks=rdtsc();
	QueryPerformanceCounter((LARGE_INTEGER *)&TimeStop);

	return (StopTicks-StartTicks)*TimeFreq/(TimeStop-TimeStart);
}

int32_t WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int32_t iCmdShow){
	WNDCLASS wc;
	MSG msg={ 0, };
	RECT Rect;

#ifdef _DEBUG
	_CrtMemState sOld;
	_CrtMemState sNew;
	_CrtMemState sDiff;

	_CrtMemCheckpoint(&sOld); //take a snapshot
#endif

	wc.style=CS_VREDRAW|CS_HREDRAW|CS_OWNDC;
	wc.lpfnWndProc=WndProc;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.hInstance=hInstance;
	wc.hIcon=LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor=LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground=GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName=NULL;
	wc.lpszClassName=szAppName;

	RegisterClass(&wc);

	SetRect(&Rect, 0, 0, Width, Height);
	AdjustWindowRect(&Rect, WS_OVERLAPPEDWINDOW, FALSE);

	Context.hWnd=CreateWindow(szAppName, szAppName, WS_OVERLAPPEDWINDOW|WS_CLIPSIBLINGS, CW_USEDEFAULT, CW_USEDEFAULT, Rect.right-Rect.left, Rect.bottom-Rect.top, NULL, NULL, hInstance, NULL);

	ShowWindow(Context.hWnd, SW_SHOW);
	SetForegroundWindow(Context.hWnd);

	DBGPRINTF("Creating OpenGL context...\n");
	if(!CreateContext(&Context, 32, 24, 0, 0, OGL_CORE45|OGL_DOUBLEBUFFER))
	{
		DBGPRINTF("\t...failed.\n");
		DestroyWindow(Context.hWnd);

		return -1;
	}

	DBGPRINTF("\nOpenGL vendor: %s\n", glGetString(GL_VENDOR));
	DBGPRINTF("OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	DBGPRINTF("OpenGL Version: %s\n", glGetString(GL_VERSION));

	DBGPRINTF("Initalizing OpenGL resources...\n");
	if(!Init())
	{
		DBGPRINTF("\t...failed.\n");
		DestroyContext(&Context);
		DestroyWindow(Context.hWnd);

		return -1;
	}

	Frequency=GetFrequency();
	DBGPRINTF("\nCPU freqency: %0.2fGHz\n", (float)Frequency/1000000000);

	DBGPRINTF("\nStarting main loop.\n");
	while(!Done)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message==WM_QUIT)
				Done=1;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			StartTime=rdtsc();
			Render();
			EndFrameTime=rdtsc();
			SwapBuffers(Context.hDC);
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

	DBGPRINTF("Shutting down...\n");
	Destroy();

	DestroyContext(&Context);
	DestroyWindow(Context.hWnd);

#ifdef _DEBUG
	_CrtMemCheckpoint(&sNew); //take a snapshot 
	if(_CrtMemDifference(&sDiff, &sOld, &sNew)) // if there is a difference
	{
		OutputDebugString("-----------_CrtMemDumpStatistics ---------");
		_CrtMemDumpStatistics(&sDiff);
		OutputDebugString("-----------_CrtMemDumpAllObjectsSince ---------");
		_CrtMemDumpAllObjectsSince(&sOld);
		OutputDebugString("-----------_CrtDumpMemoryLeaks ---------");
		_CrtDumpMemoryLeaks();
	}
#endif

	DBGPRINTF("Exit\n");

	return (int32_t)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static POINT old;
	POINT pos, delta;

	switch(uMsg)
	{
		case WM_CREATE:
			break;

		case WM_CLOSE:
			PostQuitMessage(0);
			break;

		case WM_DESTROY:
			break;

		case WM_SIZE:
			Width=LOWORD(lParam);
			Height=HIWORD(lParam);
			break;

		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			SetCapture(hWnd);
			ShowCursor(FALSE);

			GetCursorPos(&pos);
			old.x=pos.x;
			old.y=pos.y;
			break;

		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			ShowCursor(TRUE);
			ReleaseCapture();
			break;

		case WM_MOUSEMOVE:
			GetCursorPos(&pos);

			if(!wParam)
			{
				old.x=pos.x;
				old.y=pos.y;
				break;
			}

			delta.x=pos.x-old.x;
			delta.y=old.y-pos.y;

			if(!delta.x&&!delta.y)
				break;

			SetCursorPos(old.x, old.y);

			switch(wParam)
			{
				case MK_LBUTTON:
					Camera.Yaw-=(float)delta.x/800.0f;
					Camera.Pitch+=(float)delta.y/800.0f;
					break;

				case MK_MBUTTON:
					break;

				case MK_RBUTTON:
					break;
			}
			break;

		case WM_KEYDOWN:
			Key[wParam]=true;

			switch(wParam)
			{
				case VK_DELETE:
					ParticleSystem_DeleteEmitter(&ParticleSystem, EmitterIDs[2]);
					break;
			
				case VK_RETURN:
					ParticleSystem_ResetEmitter(&ParticleSystem, EmitterIDs[1]);
					break;

				case 'W':
					Camera.key_w=true;
					break;

				case 'S':
					Camera.key_s=true;
					break;

				case 'A':
					Camera.key_a=true;
					break;

				case 'D':
					Camera.key_d=true;
					break;

				case 'V':
					Camera.key_v=true;
					break;

				case 'C':
					Camera.key_c=true;
					break;

				case 'Q':
					Camera.key_q=true;
					break;

				case 'E':
					Camera.key_e=true;
					break;

				case VK_UP:
					Camera.key_up=true;
					break;

				case VK_DOWN:
					Camera.key_down=true;
					break;

				case VK_LEFT:
					Camera.key_left=true;
					break;

				case VK_RIGHT:
					Camera.key_right=true;
					break;

				case VK_SPACE:
#if CAMERA_RECORDING
				{	// Recording camera paths
					FILE *stream=NULL;
					stream=fopen("path.txt", "a");
					if(!stream)
						break;
					fprintf(stream, "%0.3f %0.3f %0.3f %0.3f %0.3f %0.3f\n", Camera.Position[0], Camera.Position[1], Camera.Position[2], Camera.View[0], Camera.View[1], Camera.View[2]);
					fclose(stream);
				}
#else
					Auto^=true;
#endif
					break;

				case VK_ESCAPE:
					PostQuitMessage(0);
					break;

				default:
					break;
			}
			break;

		case WM_KEYUP:
			Key[wParam]=false;

			switch(wParam)
			{
				case 'W':
					Camera.key_w=false;
					break;

				case 'S':
					Camera.key_s=false;
					break;

				case 'A':
					Camera.key_a=false;
					break;

				case 'D':
					Camera.key_d=false;
					break;

				case 'V':
					Camera.key_v=false;
					break;

				case 'C':
					Camera.key_c=false;
					break;

				case 'Q':
					Camera.key_q=false;
					break;

				case 'E':
					Camera.key_e=false;
					break;

				case VK_UP:
					Camera.key_up=false;
					break;

				case VK_DOWN:
					Camera.key_down=false;
					break;

				case VK_LEFT:
					Camera.key_left=false;
					break;

				case VK_RIGHT:
					Camera.key_right=false;
					break;

				default:
					break;
			}
			break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
