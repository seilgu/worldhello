
#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#ifndef APPLE

#include <Windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#else

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#endif


#include "common.h"

#define TX_ROW	16	// 16*16 = 256 textures in one file

#define TX(i, j) ((j)*TX_ROW + (i))

inline float2 get_texture_coord(int tx) {
	float2 coord;
	coord.y = (tx/TX_ROW)/16.0f;
	coord.x = (tx%TX_ROW)/16.0f;

	return coord;
}

class TextureMgr {
public:
	static const int GRASS_TOP = TX(0, 15);
	static const int GRASS_SIDE = TX(3, 15);
	static const int GRASS_BUTTOM = TX(2, 15);
	static const int CRATE = TX(9, 14);
	static const int SOIL = TX(2, 15);

	GLfloat *textureArray[256];
	GLuint block_texture;

public :
	TextureMgr() {
		for (int i=0; i<256; i++) {
			textureArray[i] = 0;
		}
	}
	~TextureMgr() {
		for (int i=0; i<256; i++) {
			if (textureArray[i] != 0) {
				delete[] textureArray[i];
			}
		}
	}

	int LoadAllTextures();
	void BuildTextureArrays();
};

#endif
