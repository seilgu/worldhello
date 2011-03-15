#ifndef APPLE
	#include <windows.h>
	#include <process.h>
	#include <commctrl.h>
	#include <gl\gl.h>
	#include <gl\glu.h>
#else
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#include <GLUT/glut.h>
#endif

#include <stdio.h>
#include <time.h>
#include <math.h>

#include "asmlibran.h"
#include "init.h"
#include "Render.h"
#include "World.h"
#include "texture.h"
#include "Player.h"
#include "File.h"
#include "Debug.h"


int currX, currY;
int prevX, prevY;

Render *s_Render;
World *s_World;
TextureMgr *s_Texture;
Player *m_Player;
FileMgr *s_File;

#ifdef APPLE
#include "macLayer.h"
#endif


#ifndef APPLE

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
		switch (wParam) {
		case VK_ESCAPE: captureMouse = 1 - captureMouse;
			if (captureMouse == 1) {
				ShowCursor(false);
			} else {
				ShowCursor(true);
			}
			break;
		default: keys[wParam] = TRUE;			break;
		}
		return 0;
	case WM_KEYUP:
		switch (wParam) {
		case VK_ESCAPE: break;
		default: keys[wParam] = FALSE; break;
		}
		return 0;
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	};

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

POINT cs;
// FPS

#endif

int framecount = 0;
double fps = 0;

DWORD tick1, tick2;
#ifndef APPLE
int DrawGLScene(GLvoid)
#else
void DrawGLScene()
#endif
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	
	// Ugly mouse handling
#ifndef APPLE	
	if (captureMouse) {
		GetCursorPos(&cs);
		m_Player->phi += -(cs.x - 500)/1000.0f;
		m_Player->theta += -(cs.y - 500)/1000.0f;
		SetCursorPos(500, 500);
	}
#endif

	if (m_Player->theta < 0.01) m_Player->theta = 0.0001f; // theta=0 leads to null crossproduct with unit_z
	if (m_Player->theta > PI) m_Player->theta = PI-0.0001f;
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
#ifndef APPLE
	QueryPerformanceCounter(&lastTick);
#endif
	s_Render->LoadNeededChunks(m_Player->eyepos, m_Player->dir, s_World);
	s_Render->DiscardUnneededChunks(m_Player->eyepos, m_Player->dir, s_World);
	s_Render->DrawScene(m_Player->eyepos, m_Player->dir, 400);
#ifndef APPLE
	QueryPerformanceCounter(&currTick);
#endif
	
	

	// Debugging purpose
	glBindTexture(GL_TEXTURE_2D, 0);

	float3 pos = m_Player->eyepos;
	int3 id((int)floor(pos.x / (BLOCK_LEN*CHUNK_W)), 
		(int)floor(pos.y / (BLOCK_LEN*CHUNK_L)), 
		(int)floor(pos.z / (BLOCK_LEN*CHUNK_H)));
#ifndef APPLE
	fps = tickFreq/(currTick.LowPart - lastTick.LowPart);
#endif
	char buffer[128];
	s_Render->PrintChunkStatistics(buffer);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _width, 0, _height, -1, 1);

	glScalef(20, 20, 1);

	glPushMatrix();
	glPrint("FPS:%4.0f %d,%d,%d", fps, id.x, id.y, id.z);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 1, 0);
	glPrint("%s", buffer);
	glPopMatrix();

	//s_World->world_map.PrintChunkStatistics(buffer);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(45.0f, (GLfloat)_width/(GLfloat)_height, 1.0f, 1000.0f);
	gluPerspective(fovY, (GLfloat)_width/(GLfloat)_height, zNear, 1000.0f);

#ifdef APPLE
	glutSwapBuffers();
#else
	return TRUE;
#endif
}

void InitClasses() {
	s_File = new FileMgr();
	s_Render = new Render(supportVBO);
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
	if (s_File != 0)
		delete s_File;

	s_Render = 0;
	s_World = 0;
	s_Texture = 0;
	m_Player = 0;
	s_File = 0;
}

#ifndef APPLE

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
	MSG		msg;
	BOOL	done = FALSE;

	::hInst = hInst;

	if (!CreateGLWindow())
		return 0;

	supportVBO = CheckVBOSupport();

	/************* Game things ***************/
	ShowWindow(hWnd, SW_SHOW);
	ReSizeGLScene(WIDTH, HEIGHT);
	InitClasses();
	
	/************* GL things ***************/
	if (!InitGL()) {
		MessageBox(0, "Fail to init GL", "ERROR", MB_OK);
		return FALSE;
	}


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

#else
int main(int argc, char** argv)
{
	GLenum type;

	glutInit(&argc, argv);

	type = GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH;
	glutInitDisplayMode(type); 

	glutInitWindowSize(800 ,600);
	glutCreateWindow("ABGR extension");

	supportVBO = false;
	
	/************* Game things ***************/
	ReSizeGLScene(WIDTH, HEIGHT);
	InitClasses();
	
	/************* GL things ***************/
	if (!InitGL()) {
		fprintf( stderr, "Init GL failed. End.\n");
		exit(0);
	}
	// load textures
	s_Texture->LoadAllTextures();

	//************* Game preinit ***************/

	s_World->LoadWorld();

	m_Player->eyepos = float3(20, 20, 20);
	m_Player->theta = PI/2;
	m_Player->phi = PI/4;

	glutKeyboardFunc(Key);
	glutDisplayFunc(DrawGLScene);
	glutIdleFunc(glutPostRedisplay);
	glutPassiveMotionFunc(Motion);
	glutMainLoop();


	DeInitClasses();
	return 0;
}
#endif
