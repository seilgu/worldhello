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
#include "World.h"
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
	
	glPushMatrix();

	// shadow setup
	glMatrixMode(GL_MODELVIEW); // just using modelview stack as a tool to calculate matrices

	glLoadIdentity();
	gluPerspective(fovY, (GLfloat)_width/(GLfloat)_height, zNear, 1000.0f);
	glGetFloatv(GL_MODELVIEW_MATRIX, cameraProjectionMatrix);
	float3 pos = m_Player->eyepos;
	float3 dir = m_Player->dir;
	glLoadIdentity();
	gluLookAt(pos.x, pos.y, pos.z, pos.x+dir.x, pos.y+dir.y, pos.z+dir.z, 0, 0, 1);
	glGetFloatv(GL_MODELVIEW_MATRIX, cameraViewMatrix);
	
	// light
	glLoadIdentity();
	gluPerspective(fovY, (GLfloat)_width/(GLfloat)_height, zNear, 1000.0f);
	glGetFloatv(GL_MODELVIEW_MATRIX, lightProjectionMatrix);
	float3 lightPosition(40*BLOCK_LEN, 40*BLOCK_LEN, 100*BLOCK_LEN);
	float3 lightDir(0.01f, 0.0f, -1.0f);
	glLoadIdentity();
	gluLookAt( lightPosition.x, lightPosition.y, lightPosition.z,
		lightPosition.x + lightDir.x, lightPosition.y + lightDir.y, lightPosition.z + lightDir.z,
		0.0f, 1.0f, 0.0f);
    glGetFloatv(GL_MODELVIEW_MATRIX, lightViewMatrix);

	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(lightProjectionMatrix);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(lightViewMatrix);
	glViewport(0, 0, WIDTH, HEIGHT);

	glShadeModel(GL_FLAT);
	glColorMask(0, 0, 0, 0);

	// draw in light perspective
#ifndef APPLE
	QueryPerformanceCounter(&lastTick);
#endif
	s_World->UpdateWorld();

	s_Render->DiscardUnneededChunks(m_Player->eyepos, m_Player->dir, s_World);
	
	s_Render->LoadNeededChunks(m_Player->eyepos, m_Player->dir, s_World);

	//s_Render->DrawScene(m_Player->eyepos, m_Player->dir, 400, s_World);
	s_Render->DrawScene(lightPosition, lightPosition + lightDir, 400, s_World, 0, 0);
	#ifndef APPLE
	QueryPerformanceCounter(&currTick);
#endif

	// copy texture
	glBindTexture(GL_TEXTURE_2D, shadowTexture);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, WIDTH, HEIGHT);

	
	glShadeModel(GL_SMOOTH);
	glColorMask(1, 1, 1, 1);

	glClear(GL_DEPTH_BUFFER_BIT);

	/*static MATRIX4X4 biasMatrix(0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f);*/
	static MATRIX4X4 biasMatrix(16.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 16.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 16.0f, 0.0f,
		16.0f, 16.0f, 16.0f, 1.0f);
	
	MATRIX4X4 textureMatrix = biasMatrix*lightProjectionMatrix*lightViewMatrix;

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(cameraProjectionMatrix);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(cameraViewMatrix);

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGenfv(GL_S, GL_EYE_PLANE, textureMatrix.GetRow(0));
    glEnable(GL_TEXTURE_GEN_S);

    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGenfv(GL_T, GL_EYE_PLANE, textureMatrix.GetRow(1));
    glEnable(GL_TEXTURE_GEN_T);

    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGenfv(GL_R, GL_EYE_PLANE, textureMatrix.GetRow(2));
    glEnable(GL_TEXTURE_GEN_R);

    glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGenfv(GL_Q, GL_EYE_PLANE, textureMatrix.GetRow(3));
    glEnable(GL_TEXTURE_GEN_Q);

	
	glBindTexture(GL_TEXTURE_2D, shadowTexture);
	glEnable(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_INTENSITY);

	glAlphaFunc(GL_GEQUAL, 1.0f);
    glEnable(GL_ALPHA_TEST);


	// draw in camera perspective
	s_Render->DrawScene(m_Player->eyepos, m_Player->dir, 400, s_World, 0, 1);

	glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);
    glDisable(GL_TEXTURE_GEN_Q);

    //Restore other states
    glDisable(GL_LIGHTING);
    glDisable(GL_ALPHA_TEST);

	if (keys['K'] == TRUE) {
		keys['K'] = FALSE;

		map_chunk *mapchk;
		chunk_list::iterator it = s_World->world_map.m_chunks.find(int3(2, 2, 0));
		if (it != s_World->world_map.m_chunks.end()) {
			mapchk = it->second;
			if (mapchk == 0 || mapchk->loaded == 0 || mapchk->failed == 1) {}
			else {
				for (int w=0; w<3400; w++) {
					int3 tmp = int3(MersenneIRandom(0, 15), MersenneIRandom(0, 15), MersenneIRandom(0, 15));
					tmp.z += 64;
					
					(it->second)->blocks[tmp.z*(CHUNK_W*CHUNK_L) + tmp.y*(CHUNK_W) + tmp.x].setType(Block::LAVA);
				}
				(it->second)->modified = 1;
			}
		}
	}
	if (keys['I'] == TRUE) {
		keys['I'] = FALSE;

		map_chunk *mapchk;
		chunk_list::iterator it = s_World->world_map.m_chunks.find(int3(2, 2, 0));
		if (it != s_World->world_map.m_chunks.end()) {
			for (int i=0; i<CHUNK_W*CHUNK_L*CHUNK_H; i++) {
				mapchk = it->second;
				if (mapchk == 0 || mapchk->loaded == 0 || mapchk->failed == 1) {}
				else {
					if ( (it->second)->blocks[i].type == Block::LAVA ) {
						(it->second)->blocks[i].setType(Block::NUL);
						(it->second)->modified = 1;
						continue;
					}
				}
			}
		}
	}

	if (++framecount > 0) {
		framecount = 0;
		map_chunk *mapchk;
		chunk_list::iterator it = s_World->world_map.m_chunks.find(int3(2, 2, 0));
		if (it != s_World->world_map.m_chunks.end()) {
			mapchk = it->second;
			if (mapchk == 0 || mapchk->loaded == 0 || mapchk->failed == 1) {}
			else {
				Block *blocks = mapchk->blocks;
				for_xyz(i, j, k) {
					blocks[_1DC(i, j, k)].data = blocks[_1DC(i, j, k)].type;
				} end_xyz()

				for_xyz(i, j, k) {
					if (blocks[_1D(i, j, k)].type == Block::NUL) {
						int cnt = 0;
						
						for (int w=0; w<27; w++) {
							int n, m, l;
							n = (w/9)%3;
							m = (w/3)%3;
							l = w%3;
							if (n == 1 && m == 1 && l == 1) continue;
							if (blocks[_1DC(i-1+n, j-1+m, k-1+l)].data == Block::LAVA) cnt++;
						}

						if (cnt > 8)
							blocks[_1D(i, j, k)].setType(Block::LAVA);
					}
					else if (blocks[_1D(i, j, k)].type == Block::LAVA) {
						int cnt = 0;

						for (int w=0; w<27; w++) {
							int n, m, l;
							n = (w/9)%3;
							m = (w/3)%3;
							l = w%3;
							if (n == 1 && m == 1 && l == 1) continue;
							if (blocks[_1DC(i-1+n, j-1+m, k-1+l)].data == Block::NUL) cnt++;
						}

						if (cnt > 23 || cnt < 18)
							blocks[_1D(i, j, k)].setType(Block::NUL);
					}
				} end_xyz()
				
				mapchk->modified = 1;
			}
		}
	}

	// Debugging purpose
	glBindTexture(GL_TEXTURE_2D, 0);

	int3 id((int)floor(pos.x / (BLOCK_LEN*CHUNK_W)), 
		(int)floor(pos.y / (BLOCK_LEN*CHUNK_L)), 
		(int)floor(pos.z / (BLOCK_LEN*CHUNK_H)));
#ifndef APPLE
	fps = tickFreq/(currTick.LowPart - lastTick.LowPart);
#endif
	char buffer[128];

	glMatrixMode(GL_PROJECTION);
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

	/*sprintf(buffer, "size:%d", s_Render->r_chunks.find(int3(0, 0, 0))->second->blockList.size());
	glPushMatrix();
	glTranslatef(0, 2, 0);
	glPrint("%s", buffer);
	glPopMatrix();*/

	//s_World->world_map.PrintChunkStatistics(buffer);
	

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

	CompileDisplayLists();

	LARGE_INTEGER _FREQ;
	QueryPerformanceFrequency(&_FREQ);
	tickFreq = (double)_FREQ.QuadPart;
	
	MersenneRandomInit((int)ReadTSC());

	// load textures
	s_Texture->LoadAllTextures();
	//s_Texture->BuildTextureArrays();

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

	glDeleteTextures(1, &shadowTexture);
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
