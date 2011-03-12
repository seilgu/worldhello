#include <stdio.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "asmlibran.h"
#include "macInit.h"
#include "Render.h"
#include "World.h"
#include "texture.h"
#include "Player.h"

int currX, currY;
int prevX, prevY;

Render *s_Render;
World *s_World;
TextureMgr *s_Texture;
Player *m_Player;

GLenum doubleBuffer;

bool keys[200];

static void Motion(int x, int y)
{
	int midX = WIDTH/2, midY = HEIGHT/2;
	m_Player->phi += -(x - midX)/400.0f;
	m_Player->theta += -(y - midY)/400.0f;
	glutWarpPointer(midX, midY);
}

static void Key(unsigned char key, int x, int y) 
{ 
	fprintf(stderr, "key '%c' is pressed.\n", key);
	switch (key) { 
		case 27: 
			exit(0); 
		case 'w':
		case 'W':
			keys['W'] = true;
			break;
		case 's':
		case'S':
			keys['S'] = true;
			break;
		case 'a':
		case 'A':
			keys['A'] = true;
			break;
		case 'd':
		case 'D':
			keys['D'] = true;
			break;
	} 
} 


DWORD tick1, tick2;
// FPS
int framecount = 0;
double fps = 0;

void DrawGLScene()
{
	//fprintf(stderr, "%lf %lf %lf %lf %lf \n", m_Player->eyepos.x, m_Player->eyepos.y, m_Player->eyepos.z, m_Player->phi, m_Player->theta);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	// Ugly mouse handling
	//	GetCursorPos(&cs);
	//m_Player->phi += -(cs.x - 500)/1000.0f;
	//m_Player->theta += -(cs.y - 500)/1000.0f;
	//SetCursorPos(500, 500);

	if (m_Player->theta < 0.01) m_Player->theta = 0.01f; // theta=0 leads to null crossproduct with unit_z
	if (m_Player->theta > PI) m_Player->theta = PI;
	if (m_Player->phi > 2*PI) m_Player->phi -= 2*PI;
	if (m_Player->phi < 0) m_Player->phi += 2*PI;

	m_Player->dir = spher2car(m_Player->theta, m_Player->phi);

	if (keys['W'] == true) {
		float3 float_tmp = m_Player->dir/1.0;
		
		m_Player->eyepos = m_Player->eyepos + float_tmp;
		keys['W'] = false;
	}
	if (keys['S'] == true) {
		//m_Player->eyepos = m_Player->eyepos - (m_Player->dir/1.0);
		float3 float_tmp = m_Player->dir/1.0;

		m_Player->eyepos -= float_tmp;
		keys['S'] = false;
	}
	float3 unit_z(0, 0, 1);
	float3 crpd = cross_prod(unit_z, m_Player->dir);
	normalize(crpd);
	if (keys['A'] == true) {
		float3 float_tmp = crpd/2.0;
		m_Player->eyepos = m_Player->eyepos + float_tmp;
		keys['A'] = false;
	}
	if (keys['D'] == true) {
		float3 float_tmp = crpd/2.0;
		m_Player->eyepos -= float_tmp;
		keys['D'] = false;
	}

	s_Render->DrawScene0(m_Player->eyepos, m_Player->dir, 200);

	//	currTick = GetTickCount();

	// Debugging purpose
	glBindTexture(GL_TEXTURE_2D, 0);
	glPushMatrix();
	glTranslatef(50, 50, -500);
	glScalef(10, 10, 1);
	glColor3f(1, 1, 1);
	//	glPrint("FPS:%.3f", 1000.0/(currTick-lastTick));
	glPopMatrix();

	//	lastTick = currTick;
	float3 pos = m_Player->eyepos;
	int3 id((int)floor(pos.x / (BLOCK_LEN*CHUNK_W)), 
		(int)floor(pos.y / (BLOCK_LEN*CHUNK_L)), 
		(int)floor(pos.z / (BLOCK_LEN*CHUNK_H)));
	

	//fps = tickFreq/(currTick.LowPart - lastTick.LowPart);

	char buffer[128];
	s_Render->PrintChunkStatistics(buffer);


	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _width, 0, _height, -1, 1);

		glScalef(20, 20, 1);
		
		glPushMatrix();
//		glPrint("FPS:%4.0f %d,%d,%d", fps, id.x, id.y, id.z);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0, 1, 0);
//		glPrint("%s", buffer);
		glPopMatrix();

		//s_World->world_map.PrintChunkStatistics(buffer);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)_width/(GLfloat)_height, 0.1f, 10000.0f);
	
	glutSwapBuffers();
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

int main(int argc, char** argv)
{
	//	ShowWindow(hWnd, SW_SHOW);


	GLenum type;

	glutInit(&argc, argv);

	type = GLUT_RGB | GLUT_DOUBLE;
//	type = GLUT_RGB; 
//	type |= (doubleBuffer) ? GLUT_DOUBLE : GLUT_SINGLE; 
	glutInitDisplayMode(type); 

	glutInitWindowSize(600 ,400);
	glutCreateWindow("ABGR extension");
	//	if (!glutExtensionSupported("GL_EXT_abgr")) {
	//		printf("Couldn't find abgr extension.\n");
	//		exit(0);
	//	}
	
	CheckVBOSupport();
	
	/************* Game things ***************/
	InitClasses();


	ReSizeGLScene(WIDTH, HEIGHT);

	// *****************GL things
		if (!InitGL()) {
			fprintf( stderr, "Init GL failed. End.\n");
			exit(0);
		}
#ifndef APPLE
	LARGE_INTEGER _FREQ;
	QueryPerformanceFrequency(&_FREQ);
	tickFreq = (double)_FREQ.QuadPart;
	
	MersenneRandomInit((int)ReadTSC());
#endif
	//	MersenneRandomInit((int)ReadTSC());

	// load textures
		s_Texture->LoadAllTextures();

	// ****************game preinit
		s_World->LoadWorld();

		m_Player->eyepos = float3(20, 20, 20);
		m_Player->theta = PI/2;
		m_Player->phi = PI/4;
	//	m_Player->dir = float3(0, 10, 0);

	// ***************event loop
	fprintf(stderr, "asdf\n");
		glutKeyboardFunc(Key);
		glutDisplayFunc(DrawGLScene);
		glutIdleFunc(glutPostRedisplay);
		glutPassiveMotionFunc(Motion);
		glutMainLoop();


	DeInitClasses();
	return 0;
}

