#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <time.h>
#include <commctrl.h>
#include <math.h>

#include "asmlibran.h"
#include "init.h"
#include "Render.h"
#include "World.h"
#include "texture.h"
#include "Player.h"
#include "Debug.h"

int currX, currY;
int prevX, prevY;

Render *s_Render;
World *s_World;
TextureMgr *s_Texture;
Player *m_Player;

// WINAPI event procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		return 0;
	case WM_ACTIVATE:
		if (!HIWORD(wParam))
			active = TRUE;
		else
			active = FALSE;
		return 0;
	case WM_SIZE:
		_width = LOWORD(lParam);
		_height = HIWORD(lParam);
		ReSizeGLScene(_width, _height);
		return 0;
	case WM_MOUSEMOVE:
		return 0;
	case WM_LBUTTONDOWN:
		return 0;
	case WM_LBUTTONUP:
		return 0;
	case WM_KEYDOWN:
		keys[wParam] = TRUE;
		return 0;
	case WM_KEYUP:
		keys[wParam] = FALSE;
		return 0;
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	};

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

DWORD tick1, tick2;
POINT cs;
int captureMouse = 1;
// FPS
int framecount = 0;
double fps = 0;
int DrawGLScene(GLvoid)
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	
	// Ugly mouse handling
	if (keys['R'] == TRUE) {
		keys['R'] = FALSE;
		captureMouse = 1 - captureMouse;
	}
	
	if (captureMouse) {
		GetCursorPos(&cs);
		m_Player->phi += -(cs.x - 500)/1000.0f;
		m_Player->theta += -(cs.y - 500)/1000.0f;
		SetCursorPos(500, 500);
	}

	if (m_Player->theta < 0.01) m_Player->theta = 0.01f; // theta=0 leads to null crossproduct with unit_z
	if (m_Player->theta > PI) m_Player->theta = PI;
	if (m_Player->phi > 2*PI) m_Player->phi -= 2*PI;
	if (m_Player->phi < 0) m_Player->phi += 2*PI;

	m_Player->dir = spher2car(m_Player->theta, m_Player->phi);

	if (keys['W'] == TRUE) {
		m_Player->eyepos = m_Player->eyepos + m_Player->dir/0.5;
	}
	if (keys['S'] == TRUE) {
		m_Player->eyepos = m_Player->eyepos - m_Player->dir/0.5;
	}
	float3 unit_z(0, 0, 1);
	float3 crpd = cross_prod(unit_z, m_Player->dir);
	normalize(crpd);
	if (keys['A'] == TRUE) {
		m_Player->eyepos = m_Player->eyepos + crpd/2.0;
	}
	if (keys['D'] == TRUE) {
		m_Player->eyepos = m_Player->eyepos - crpd/2.0;
	}

	

	QueryPerformanceCounter(&lastTick);
	s_Render->LoadNeededChunks(m_Player->eyepos, m_Player->dir, s_World);
	s_Render->DiscardUnneededChunks(m_Player->eyepos, m_Player->dir, s_World);
	s_Render->DrawScene2(m_Player->eyepos, m_Player->dir, 400);
	QueryPerformanceCounter(&currTick);
	
	

	// Debugging purpose
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glPushMatrix();
	glTranslatef(-50, 50, -200);
	glScalef(10, 10, 1);
	glColor3f(1, 1, 1);

	float3 pos = m_Player->eyepos;
	int3 id((int)floor(pos.x / (BLOCK_LEN*CHUNK_W)), 
		(int)floor(pos.y / (BLOCK_LEN*CHUNK_L)), 
		(int)floor(pos.z / (BLOCK_LEN*CHUNK_H)));
	
		fps = tickFreq/(currTick.LowPart - lastTick.LowPart);

		glPushMatrix();
		glPrint("FPS:%.3f %d,%d,%d", fps, id.x, id.y, id.z);
		glPopMatrix();

	char buffer[128];
	
	s_Render->PrintChunkStatistics(buffer);

		glPushMatrix();
		glTranslatef(0, -1, 0);
		glPrint("%s", buffer);
		glPopMatrix();

	//s_World->world_map.PrintChunkStatistics(buffer);
	

		glPushMatrix();
		glTranslatef(0, -2, 0);
			glPrint("COUNT:%d", s_World->world_map.counter);
		glPopMatrix();

	glPopMatrix();



	return TRUE;
}

void InitClasses() {
	s_Render = new Render();
	s_World = new World();
	s_Texture = new TextureMgr();
	m_Player = new Player();
}

void DeInitClasses() {
	if (s_Render != 0)
		delete s_Render;
	if (s_World != 0)
		delete s_World;
	if (s_Texture != 0)
		delete s_Texture;
	if (m_Player != 0)
		delete m_Player;

	s_Render = 0;
	s_World = 0;
	s_Texture = 0;
	m_Player = 0;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
	MSG		msg;
	BOOL	done = FALSE;

	::hInst = hInst;

	if (!CreateGLWindow())
		return 0;

	ShowWindow(hWnd, SW_SHOW);

	ReSizeGLScene(WIDTH, HEIGHT);

	//************* GL things ***************/

	if (!InitGL()) {
		MessageBox(0, "Fail to init GL", "ERROR", MB_OK);
		return FALSE;
	}

	InitClasses();

	LARGE_INTEGER _FREQ;
	QueryPerformanceFrequency(&_FREQ);
	tickFreq = (double)_FREQ.QuadPart;
	
	MersenneRandomInit((int)ReadTSC());

	// load textures
	s_Texture->LoadAllTextures();

	//************* Game preinit ***************/

	s_World->LoadWorld();

	m_Player->eyepos = float3(20, 20, 20);
	m_Player->theta = PI/2;
	m_Player->phi = PI/4;
	//m_Player.dir = float3(0, 10, 0);

	//************* Event loop ***************/

	ShowCursor(false);

	while (!done) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT)
				done = TRUE;
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else {
			if (active && !DrawGLScene()) {
				done = TRUE;
			}
			else {
				SwapBuffers(hDC);
			}
		}
	}
	
	ShowCursor(true);

	DeInitClasses();

	wglMakeCurrent(0, 0);
	wglDeleteContext(hRC);
	ReleaseDC(hWnd, hDC);
	DestroyWindow(hWnd);
	UnregisterClass("OpenGL", hInst);

	KillFont();

	return 0;
}