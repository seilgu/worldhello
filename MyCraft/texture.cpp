#ifndef APPLE

#include <Windows.h>

#endif

#include <stdio.h>

#include "texture.h"
#include "World.h"
#include "SOIL.h"
#include "Block.h"
#include "Render.h"


int TextureMgr::LoadAllTextures() {
	block_texture = SOIL_load_OGL_texture("./Textures/terrain.png", SOIL_LOAD_RGBA, SOIL_CREATE_NEW_ID, SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_INVERT_Y);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	// Linear Min Filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	// Linear Mag Filter

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// Linear Min Filter
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// Linear Mag Filter

	return 1;
}

// NOT USING THIS
void TextureMgr::BuildTextureArrays() {
	for (int i=0; i<4; i++) {
		if (textureArray[i] != 0) continue;

		switch (i) {
		case Block::CRATE: 
		case Block::GRASS: 
		case Block::SOIL: textureArray[i] = new GLfloat[120];
			break;
		default: continue;
		}

		GLfloat *vertices = textureArray[i];

				for (int w=0; w<6; w++) { // 6 faces
					float2 coord;
					Render::GetTextureCoordinates(i, w, coord);
					coord.x += 0.001f;
					coord.y += 0.001f;
					float csize = 1/16.0f;
					csize -= 0.002f;

					// texture coordinates are in the same order in each face except NX
					if (w == Render::NX) {
						vertices[w*20 + 0] = coord.x;
						vertices[w*20 + 1] = coord.y;
						vertices[w*20 + 5] = coord.x;
						vertices[w*20 + 6] = coord.y+csize;
						vertices[w*20 + 10] = coord.x+csize;
						vertices[w*20 + 11] = coord.y+csize;
						vertices[w*20 + 15] = coord.x+csize;
						vertices[w*20 + 16] = coord.y;
					}
					else {
						vertices[w*20 + 0] = coord.x;
						vertices[w*20 + 1] = coord.y;
						vertices[w*20 + 5] = coord.x+csize;
						vertices[w*20 + 6] = coord.y;
						vertices[w*20 + 10] = coord.x+csize;
						vertices[w*20 + 11] = coord.y+csize;
						vertices[w*20 + 15] = coord.x;
						vertices[w*20 + 16] = coord.y+csize;
					}
					// now setup vertex coordinates of each face
					switch (w) {
					case Render::PZ:
						vertices[w*20 + 2] = 0.0f;
						vertices[w*20 + 3] = 0.0f;
						vertices[w*20 + 4] = 1.0f;

						vertices[w*20 + 7] = 1.0f;
						vertices[w*20 + 8] = 0.0f;
						vertices[w*20 + 9] = 1.0f;

						vertices[w*20 + 12] = 1.0f;
						vertices[w*20 + 13] = 1.0f;
						vertices[w*20 + 14] = 1.0f;

						vertices[w*20 + 17] = 0.0f;
						vertices[w*20 + 18] = 1.0f;
						vertices[w*20 + 19] = 1.0f;
						break;
					case Render::NZ:
						vertices[w*20 + 2] = 1.0f;
						vertices[w*20 + 3] = 0.0f;
						vertices[w*20 + 4] = 0.0f;

						vertices[w*20 + 7] = 0.0f;
						vertices[w*20 + 8] = 0.0f;
						vertices[w*20 + 9] = 0.0f;

						vertices[w*20 + 12] = 0.0f;
						vertices[w*20 + 13] = 1.0f;
						vertices[w*20 + 14] = 0.0f;

						vertices[w*20 + 17] = 1.0f;
						vertices[w*20 + 18] = 1.0f;
						vertices[w*20 + 19] = 0.0f;
						break;
					case Render::PY:
						vertices[w*20 + 2] = 1.0f;
						vertices[w*20 + 3] = 1.0f;
						vertices[w*20 + 4] = 0.0f;

						vertices[w*20 + 7] = 0.0f;
						vertices[w*20 + 8] = 1.0f;
						vertices[w*20 + 9] = 0.0f;

						vertices[w*20 + 12] = 0.0f;
						vertices[w*20 + 13] = 1.0f;
						vertices[w*20 + 14] = 1.0f;

						vertices[w*20 + 17] = 1.0f;
						vertices[w*20 + 18] = 1.0f;
						vertices[w*20 + 19] = 1.0f;
						break;
					case Render::NY:
						vertices[w*20 + 2] = 0.0f;
						vertices[w*20 + 3] = 0.0f;
						vertices[w*20 + 4] = 0.0f;

						vertices[w*20 + 7] = 1.0f;
						vertices[w*20 + 8] = 0.0f;
						vertices[w*20 + 9] = 0.0f;

						vertices[w*20 + 12] = 1.0f;
						vertices[w*20 + 13] = 0.0f;
						vertices[w*20 + 14] = 1.0f;

						vertices[w*20 + 17] = 0.0f;
						vertices[w*20 + 18] = 0.0f;
						vertices[w*20 + 19] = 1.0f;
						break;
					case Render::PX:
						vertices[w*20 + 2] = 1.0f;
						vertices[w*20 + 3] = 0.0f;
						vertices[w*20 + 4] = 0.0f;

						vertices[w*20 + 7] = 1.0f;
						vertices[w*20 + 8] = 1.0f;
						vertices[w*20 + 9] = 0.0f;

						vertices[w*20 + 12] = 1.0f;
						vertices[w*20 + 13] = 1.0f;
						vertices[w*20 + 14] = 1.0f;

						vertices[w*20 + 17] = 1.0f;
						vertices[w*20 + 18] = 0.0f;
						vertices[w*20 + 19] = 1.0f;
						break;
					case Render::NX:
						vertices[w*20 + 2] = 0.0f;
						vertices[w*20 + 3] = 0.0f;
						vertices[w*20 + 4] = 0.0f;

						vertices[w*20 + 7] = 0.0f;
						vertices[w*20 + 8] = 0.0f;
						vertices[w*20 + 9] = 1.0f;

						vertices[w*20 + 12] = 0.0f;
						vertices[w*20 + 13] = 1.0f;
						vertices[w*20 + 14] = 1.0f;

						vertices[w*20 + 17] = 0.0f;
						vertices[w*20 + 18] = 1.0f;
						vertices[w*20 + 19] = 0.0f;
						break;
					}
		}
	}
}