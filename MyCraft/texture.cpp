#ifndef APPLE

#include <Windows.h>

#endif

#include <stdio.h>

#include "Render.h"
#include "texture.h"
#include "World.h"
#include "SOIL.h"
#include "Block.h"


int TextureMgr::LoadAllTextures() {
	block_texture = SOIL_load_OGL_texture("./Textures/terrain.png", SOIL_LOAD_RGBA, SOIL_CREATE_NEW_ID, SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_INVERT_Y);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	return 1;
}
