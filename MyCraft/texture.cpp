
#include <Windows.h>
#include "texture.h"
#include <stdio.h>

#include "World.h"
#include "SOIL.h"

TextureMgr::TextureMgr() {
}

int TextureMgr::LoadAllTextures() {
	block_texture = SOIL_load_OGL_texture("Textures/terrain.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_INVERT_Y);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	// Linear Min Filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	// Linear Mag Filter

	return 2;
}