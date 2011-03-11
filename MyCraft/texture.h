
#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <Windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#define TX_ROW	16	// 16*16 = 256 textures in one file

#define TX(i, j) ((j)*TX_ROW + (i))

class TextureMgr {
public:
	static const int GRASS_TOP = TX(0, 15);
	static const int GRASS_SIDE = TX(3, 15);
	static const int GRASS_BUTTOM = TX(2, 15);
	static const int CRATE = TX(9, 14);
	static const int SOIL = TX(2, 15);

	GLuint block_texture;

	int LoadBitmap(LPTSTR szFileName, GLuint &texid);

public :
	TextureMgr();
	~TextureMgr() {}

	int LoadAllTextures();
};

#endif