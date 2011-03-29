#ifndef APPLE
	#include <windows.h>
	#include <process.h>
	#include <commctrl.h>
#endif

#include <stdio.h>
#include <time.h>
#include <math.h>

#include "init.h"

#include "asmlibran.h"

int currX, currY;
int prevX, prevY;

#ifdef APPLE
#include "common.h"
#include <GLUT/glut.h>
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
	case WM_LBUTTONDOWN: mouseDown = true;
		return 0;
	case WM_LBUTTONUP: mouseDown = false;
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
		case VK_CONTROL:
			controlKey = true;
			break;
		default: keys[wParam] = TRUE;
			break;
		}
		return 0;
	case WM_KEYUP:
		switch (wParam) {
		case VK_ESCAPE: break;
		case VK_CONTROL:
			controlKey = false;
			break;
		default: keys[wParam] = FALSE; break;
		}
		return 0;
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	};

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

// FPS

#else

POINT motion;
void GetCursorPos(POINT *cs)
{
	cs->x = motion.x;
	cs->y = motion.y;
}

void SetCursorPos(int x, int y)
{
	glutWarpPointer(x, y);
	captureMouse = 0;
}
void Motion(int x, int y)
{
	motion.x = x;
	motion.y = y;
	captureMouse = 1;

/*	int midX = WIDTH/2, midY = HEIGHT/2;
	m_Player->phi += -(x - midX)/400.0f;
	m_Player->theta += -(y - midY)/400.0f;
	glutWarpPointer(midX, midY);*/
}
void Key(unsigned char key, int x, int y) 
{ 
	fprintf(stderr, "key '%c' is pressed.\n", key);
	switch (key) { 
		case 27: 
			exit(0); 
		default:
			if (key>='a' && key <='z')
				keys[key-'a'+'A'] = true;
			else if (key>='A' && key <= 'Z')
				keys[key-'A'+'a'] = true;
	} 
}

#endif

void ProcessInput() {
	
	// Ugly mouse handling
	if (captureMouse) {
		POINT cs;
		GetCursorPos(&cs);
		m_Player->phi += -(cs.x - 500)/1000.0f;
		m_Player->theta += -(cs.y - 500)/1000.0f;
		SetCursorPos(500, 500);
	}

	if (mouseDown == true) {
		//mouseDown = false;

		int3 tmpId;
		int3 tmpOffset;
		int tmpSide;
		if (s_Render->FindBlock(m_Player->eyepos, m_Player->dir, 128, tmpId, tmpOffset, tmpSide) == 1) {
			if (controlKey == true) {
				switch (tmpSide) {
				case Render::PX: tmpOffset.x += 1; break;
				case Render::NX: tmpOffset.x -= 1; break;
				case Render::PY: tmpOffset.y += 1; break;
				case Render::NY: tmpOffset.y -= 1; break;
				case Render::PZ: tmpOffset.z += 1; break;
				case Render::NZ: tmpOffset.z -= 1; break;
				}
				if (tmpOffset.x >= CHUNK_W) {
					tmpOffset.x = 0;
					tmpId.x++;
				}
				else if (tmpOffset.x < 0) {
					tmpOffset.x = CHUNK_W-1;
					tmpId.x--;
				}
				if (tmpOffset.y >= CHUNK_L) {
					tmpOffset.y = 0;
					tmpId.y++;
				}
				else if (tmpOffset.y < 0) {
					tmpOffset.y = CHUNK_L-1;
					tmpId.y--;
				}
				if (tmpOffset.z >= CHUNK_H) {
					tmpOffset.z = 0;
					tmpId.z++;
				}
				else if (tmpOffset.z < 0) {
					tmpOffset.z = CHUNK_H-1;
					tmpId.z--;
				}

				s_World->AddBlock(tmpId, tmpOffset, Block::GRASS);
			}
			else {
				s_World->RemoveBlock(tmpId, tmpOffset);
			}
		}

	}

	if (m_Player->theta < 0.01) m_Player->theta = 0.0001f; // theta=0 leads to null crossproduct with unit_z
	if (m_Player->theta > PI) m_Player->theta = PI-0.0001f;
	if (m_Player->phi > 2*PI) m_Player->phi -= 2*PI;
	if (m_Player->phi < 0) m_Player->phi += 2*PI;

	m_Player->dir = spher2car(m_Player->theta, m_Player->phi);

	// keyboard
	if (keys['W'] == TRUE) {
		m_Player->eyepos = m_Player->eyepos + m_Player->dir/0.5;
#ifdef APPLE
		keys['W'] = FALSE;
#endif
	}
	if (keys['S'] == TRUE) {
		m_Player->eyepos = m_Player->eyepos - m_Player->dir/0.5;
#ifdef APPLE
		keys['S'] = FALSE;
#endif
	}
	float3 unit_z(0, 0, 1);
	float3 crpd = cross_prod(unit_z, m_Player->dir);
	normalize(crpd);
	// strafe
	if (keys['A'] == TRUE) {
		m_Player->eyepos = m_Player->eyepos + crpd/2.0;
#ifdef APPLE
		keys['A'] = FALSE;
#endif
	}
	if (keys['D'] == TRUE) {
		m_Player->eyepos = m_Player->eyepos - crpd/2.0;
#ifdef APPLE
		keys['D'] = FALSE;
#endif
	}

	// shaders	
	if (keys['V'] == TRUE) {
		keys['V'] = FALSE;
		s_Shader->UseProgram(Shader::BLUE);
#ifdef APPLE
		keys['V'] = FALSE;
#endif
	}
	if (keys['F'] == TRUE) {
		keys['F'] = FALSE;
		s_Shader->UseProgram(Shader::NUL);
#ifdef APPLE
		keys['F'] = FALSE;
#endif
	}
}

int framecount = 0;
double fps = 0;
void PrintDebugMessage() {
	glBindTexture(GL_TEXTURE_2D, 0);

	float3 pos = m_Player->eyepos;

	int3 id((int)floor(pos.x / (BLOCK_LEN*CHUNK_W)), 
		(int)floor(pos.y / (BLOCK_LEN*CHUNK_L)), 
		(int)floor(pos.z / (BLOCK_LEN*CHUNK_H)));
#ifndef APPLE
	fps = tickFreq/(currTick.LowPart - lastTick.LowPart);
#endif
	char buffer[128];

	glPushMatrix();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glOrtho(0, _width, 0, _height, -1, 1);

	glScalef(20, 20, 1);
	
	glPushMatrix();
	glPrint("FPS:%4.0f %d,%d,%d", fps, id.x, id.y, id.z);
	glPopMatrix();
	
	s_Render->PrintChunkStatistics(buffer);
	glPushMatrix();
	glTranslatef(0, 1, 0);
	glPrint("%s", buffer);
	glPopMatrix();

	glPopMatrix();
}

DWORD tick1, tick2;
#ifndef APPLE
int DrawGLScene(GLvoid)
#else
void DrawGLScene()
#endif
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	
	ProcessInput();

	//float3 lightPosition(60*BLOCK_LEN, -60*BLOCK_LEN, 110*BLOCK_LEN);
	//float3 lightDir(0.001f, 0.001f, -1.0f);

	glBlendFunc(GL_ONE, GL_ZERO);
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovY, (GLfloat)_width/(GLfloat)_height, zNear, 1000.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	// frame update
	s_World->UpdateWorld();
	s_Render->DiscardUnneededChunks(m_Player->eyepos, m_Player->dir, s_World);
	s_Render->LoadNeededChunks(m_Player->eyepos, m_Player->dir, s_World);
	s_Render->DrawScene(m_Player->eyepos, m_Player->dir, 400, s_World, 1, 1);

	
#ifndef APPLE
	QueryPerformanceCounter(&lastTick);
#endif
	for (int i=0; i<10000; i++) {
		int3 tmpId;
		int3 tmpOffset;
		int tmpType;
		s_Render->FindBlock(m_Player->eyepos, m_Player->dir, 128, tmpId, tmpOffset, tmpType);
		//map_chunk *mapchk = s_World->world_map.GetChunk(int3(0, 1, 2));
		//mapchk = s_World->world_map.GetChunk(int3(1, 1, 2));
	}
#ifndef APPLE
	QueryPerformanceCounter(&currTick);
	PrintDebugMessage();
#endif

	// Draw Red Dot
	glPointSize(3.0f);
	glColor3f(1, 0, 0);
	glBegin(GL_POINTS);
		glVertex3f(0, 0, 0);
	glEnd();
	glColor3f(1, 1, 1);

	/*int3 tmpId;
	int3 tmpOffset;
	int tmpType;
	if (s_Render->FindBlock(m_Player->eyepos, m_Player->dir, 128, tmpId, tmpOffset, tmpType) == 1) {
		glPushMatrix();

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glOrtho(0, _width, 0, _height, -1, 1);

		glScalef(20, 20, 1);

		glPushMatrix();
			glTranslatef(0, 4, 0);
			glPrint("ID: (%d,%d,%d), Offset: (%d,%d,%d), Type: %d", tmpId.x, tmpId.y, tmpId.z, tmpOffset.x, tmpOffset.y, tmpOffset.z, tmpType);
		glPopMatrix();

		glPopMatrix();

	}*/


	
#ifdef APPLE
	glutSwapBuffers();
#else
	return TRUE;
#endif
}

#ifndef APPLE
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
	MSG		msg;
	BOOL	done = FALSE;

	::hInst = hInst;

	if (!CreateGLWindow())
		return 0;

	ShowWindow(hWnd, SW_SHOW);
	ReSizeGLScene(WIDTH, HEIGHT);

	glewInit();
	
	/************* GL things ***************/
	if (!InitGL()) {
		MessageBox(0, "Fail to init GL", "ERROR", MB_OK);
		return FALSE;
	}
	
	/************* Game things ***************/
	InitClasses();

	CompileDisplayLists();

	LARGE_INTEGER _FREQ;
	QueryPerformanceFrequency(&_FREQ);
	tickFreq = (double)_FREQ.QuadPart;
	
	MersenneRandomInit((int)ReadTSC());

	// load textures
	s_Texture->LoadAllTextures();

	//************* Game preinit ***************/

	s_World->LoadWorld();

	m_Player->eyepos = float3(20, 20, 80);
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

	DeleteDisplayLists();

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
	glewInit();
	
	/************* GL things ***************/
	if (!InitGL()) {
		fprintf( stderr, "Init GL failed. End.\n");
		exit(0);
	}
	InitClasses();
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
