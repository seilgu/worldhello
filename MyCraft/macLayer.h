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

// fake functions


