#include "Render.h"
#include "World.h"

extern float zNear;
extern int _width, _height;

void Render::DrawFaceSimple(int i, int j, int k, int type, int dir) {
	if (type == 0) return;

	glTranslatef(i*1.0f, j*1.0f, k*1.0f);
	GLuint side_texture;

	switch (type) {
	case Block::CRATE: // Draw a crate
		side_texture = TextureMgr::CRATE;
		break;
	case Block::GRASS: // Draw a grass
		switch (dir) {
		case PZ: side_texture = TextureMgr::GRASS_TOP; break;
		case NZ: side_texture = TextureMgr::GRASS_BUTTOM; break;
		default: side_texture = TextureMgr::GRASS_SIDE; break;
		}
		break;
	case Block::SOIL:
		side_texture = TextureMgr::SOIL;
		break;
	default: break;
	}

	float2 coord;
	float csize = 1/16.0f;

	glBegin(GL_QUADS);
		coord = get_texture_coord(side_texture);
		switch (dir) {
		case PZ: // top
			glTexCoord2f(coord.x, coord.y);				glVertex3f(0.0f, 0.0f, 1.0f);
			glTexCoord2f(coord.x+csize, coord.y);		glVertex3f(1.0f, 0.0f, 1.0f);
			glTexCoord2f(coord.x+csize, coord.y+csize); glVertex3f(1.0f, 1.0f, 1.0f);
			glTexCoord2f(coord.x, coord.y+csize);		glVertex3f(0.0f, 1.0f, 1.0f);
			break;
		case NZ: // buttom
			glTexCoord2f(coord.x, coord.y);				glVertex3f(1.0f, 0.0f, 0.0f);
			glTexCoord2f(coord.x+csize, coord.y);		glVertex3f(0.0f, 0.0f, 0.0f);
			glTexCoord2f(coord.x+csize, coord.y+csize); glVertex3f(0.0f, 1.0f, 0.0f);
			glTexCoord2f(coord.x, coord.y+csize);		glVertex3f(1.0f, 1.0f, 0.0f);
			break;
		case PY: // +y
			glTexCoord2f(coord.x, coord.y);				glVertex3f(1.0f, 1.0f, 0.0f);
			glTexCoord2f(coord.x+csize, coord.y);		glVertex3f(0.0f, 1.0f, 0.0f);
			glTexCoord2f(coord.x+csize, coord.y+csize);	glVertex3f(0.0f, 1.0f, 1.0f);
			glTexCoord2f(coord.x, coord.y+csize);		glVertex3f(1.0f, 1.0f, 1.0f);
			break;
		case NY: // -y
			glTexCoord2f(coord.x, coord.y);				glVertex3f(0.0f, 0.0f, 0.0f);
			glTexCoord2f(coord.x+csize, coord.y);		glVertex3f(1.0f, 0.0f, 0.0f);
			glTexCoord2f(coord.x+csize, coord.y+csize);	glVertex3f(1.0f, 0.0f, 1.0f);
			glTexCoord2f(coord.x, coord.y+csize);		glVertex3f(0.0f, 0.0f, 1.0f);
			break;
		case PX: // +x
			glTexCoord2f(coord.x, coord.y);				glVertex3f(1.0f, 0.0f, 0.0f);
			glTexCoord2f(coord.x+csize, coord.y);		glVertex3f(1.0f, 1.0f, 0.0f);
			glTexCoord2f(coord.x+csize, coord.y+csize);	glVertex3f(1.0f, 1.0f, 1.0f);
			glTexCoord2f(coord.x, coord.y+csize);		glVertex3f(1.0f, 0.0f, 1.0f);
			break;
		case NX: // -x
			glTexCoord2f(coord.x, coord.y);				glVertex3f(0.0f, 0.0f, 0.0f);
			glTexCoord2f(coord.x, coord.y+csize);		glVertex3f(0.0f, 0.0f, 1.0f);
			glTexCoord2f(coord.x+csize, coord.y+csize); glVertex3f(0.0f, 1.0f, 1.0f);
			glTexCoord2f(coord.x+csize, coord.y);		glVertex3f(0.0f, 1.0f, 0.0f);
			break;
		}
	glEnd();

	glTranslatef(-i*1.0f, -j*1.0f, -k*1.0f);
}

Render::~Render() {
	KillWorkThread();

	render_list::iterator it;

	for (it = r_chunks.begin(); it != r_chunks.end(); ) {
		render_chunk *chk = (*it).second;
		DeleteChunk(chk);
		r_chunks.erase(it++);
	}
}

void Render::DeleteChunk(render_chunk *chk) {
	if (chk == 0) return;

	glDeleteBuffersARB(1, &chk->vbo);

	if (chk->vertices != 0) {
		delete[] chk->vertices;
		chk->vertices = 0;
	}

	delete chk;
	chk = 0;
}

extern LARGE_INTEGER lastTick, currTick;
void Render::CalculateVisible(int3 id, World* world) {
	chunk_list *m_chunks = s_World->world_map.GetChunkList();
	chunk_list::iterator it = m_chunks->find(id);
	if (it == m_chunks->end())
		return;

	map_chunk *map_chk = (*it).second;
	if (map_chk == 0 || map_chk->loaded == 0 || map_chk->failed == 1 || map_chk->unneeded == 1) {
		return;
	}

	Block *blocks = map_chk->blocks;
	
	
	// cleaning and setup
	for (int i=0; i<CHUNK_W*CHUNK_L*CHUNK_H; i++) {
		if (blocks[i].modified == 1) {
			blocks[i].hidden = blocks[i].opaque;
			blocks[i].outside = 0;
		}
	}

	// first pass, find solids
	for (int i=0; i<CHUNK_W*CHUNK_L*CHUNK_H; i++) {
		if (blocks[i].modified == 1) {
			if (blocks[i].type == Block::GLASS)
				blocks[i].opaque = 0;
		}
	}
	// scan in 3 directions, toggle inside/outside
	for (int i=0; i<CHUNK_W; i++) {
		for (int j=0; j<CHUNK_L; j++) {
			int inside = blocks[0*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque;
			for (int k=1; k<CHUNK_H; k++) {
				if (inside == 0 && blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque == 1) {
					inside = 1;
					blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden = 0;
					blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].outside |= 1<<NZ;
				}
				else if (inside == 1 && blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque == 0) {
					inside = 0;
					blocks[(k-1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden = 0;
					blocks[(k-1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].outside |= 1<<PZ;
				}
			}
		}
	}
	for (int i=0; i<CHUNK_W; i++) {
		for (int k=0; k<CHUNK_H; k++) {
			int inside = blocks[k*(CHUNK_W*CHUNK_L) + 0*(CHUNK_W) + i].opaque;
			for (int j=1; j<CHUNK_L; j++) {
				if (inside == 0 && blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque == 1) {
					inside = 1;
					blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden = 0;
					blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].outside |= 1<<NY;
				}
				else if (inside == 1 && blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque == 0) {
					inside = 0;
					blocks[k*(CHUNK_W*CHUNK_L) + (j-1)*(CHUNK_W) + i].hidden = 0;
					blocks[k*(CHUNK_W*CHUNK_L) + (j-1)*(CHUNK_W) + i].outside |= 1<<PY;
				}
			}
		}
	}
	for (int k=0; k<CHUNK_H; k++) {
		for (int j=0; j<CHUNK_L; j++) {
			int inside = blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + 0].opaque;
			for (int i=1; i<CHUNK_W; i++) {
				if (inside == 0 && blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque == 1) {
					inside = 1;
					blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden = 0;
					blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].outside |= 1<<NX;
				}
				else if (inside == 1 && blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque == 0) {
					inside = 0;
					blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i-1].hidden = 0;
					blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i-1].outside |= 1<<PX;
				}
			}
		}
	}

	for (int w=0; w<6; w++) {
		CheckChunkSide(id, w);
	}

	// second pass, find liquid
	for (int i=0; i<CHUNK_W*CHUNK_L*CHUNK_H; i++) {
		if (blocks[i].modified == 1) {
			if (blocks[i].type == Block::GLASS) {
				blocks[i].opaque = 1;
				blocks[i].hidden = 1;
				blocks[i].outside = 0;
			}
		}
	}
	// scan in 3 directions, toggle inside/outside
	for (int i=0; i<CHUNK_W; i++) {
		for (int j=0; j<CHUNK_L; j++) {
			int inside = blocks[0*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque;
			for (int k=1; k<CHUNK_H; k++) {
				if (inside == 0 && blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque == 1) {
					inside = 1;
					blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden = 0;
					blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].outside |= 1<<NZ;
				}
				else if (inside == 1 && blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque == 0) {
					inside = 0;
					blocks[(k-1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden = 0;
					blocks[(k-1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].outside |= 1<<PZ;
				}
			}
		}
	}
	for (int i=0; i<CHUNK_W; i++) {
		for (int k=0; k<CHUNK_H; k++) {
			int inside = blocks[k*(CHUNK_W*CHUNK_L) + 0*(CHUNK_W) + i].opaque;
			for (int j=1; j<CHUNK_L; j++) {
				if (inside == 0 && blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque == 1) {
					inside = 1;
					blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden = 0;
					blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].outside |= 1<<NY;
				}
				else if (inside == 1 && blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque == 0) {
					inside = 0;
					blocks[k*(CHUNK_W*CHUNK_L) + (j-1)*(CHUNK_W) + i].hidden = 0;
					blocks[k*(CHUNK_W*CHUNK_L) + (j-1)*(CHUNK_W) + i].outside |= 1<<PY;
				}
			}
		}
	}
	for (int k=0; k<CHUNK_H; k++) {
		for (int j=0; j<CHUNK_L; j++) {
			int inside = blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + 0].opaque;
			for (int i=1; i<CHUNK_W; i++) {
				if (inside == 0 && blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque == 1) {
					inside = 1;
					blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden = 0;
					blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].outside |= 1<<NX;
				}
				else if (inside == 1 && blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque == 0) {
					inside = 0;
					blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i-1].hidden = 0;
					blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i-1].outside |= 1<<PX;
				}
			}
		}
	}
	
	for (int w=0; w<6; w++) {
		CheckChunkSide(id, w);
	}
}

void Render::CheckChunkSide(int3 id, int dir) {

	chunk_list *m_chunks = s_World->world_map.GetChunkList();
	chunk_list::iterator it = m_chunks->find(id);
	if (it == m_chunks->end())
		return;

	map_chunk *map_chk = (*it).second;
	Block *blocks = map_chk->blocks;

	map_chunk *map_side;
	chunk_list::iterator tmp;
	switch (dir) {
	case Render::PX : tmp = m_chunks->find(int3(id.x+1, id.y, id.z)); break;
	case Render::NX : tmp = m_chunks->find(int3(id.x-1, id.y, id.z)); break;
	case Render::PY : tmp = m_chunks->find(int3(id.x, id.y+1, id.z)); break;
	case Render::NY : tmp = m_chunks->find(int3(id.x, id.y-1, id.z)); break;
	case Render::PZ : tmp = m_chunks->find(int3(id.x, id.y, id.z+1)); break;
	case Render::NZ : tmp = m_chunks->find(int3(id.x, id.y, id.z-1)); break;
	}

	map_side = (tmp == m_chunks->end() ? 0 : (*tmp).second);

	if (map_side == 0 || map_side->loaded == 0 || map_side->failed == 1 || map_side->unneeded == 1) {
		// edge of map
		return;
	}

	// not edge
	Block *aux = map_side->blocks;
	switch (dir) {
	case Render::PX : 
		for (int j=0; j<CHUNK_L; j++) {
			for (int k=0; k<CHUNK_H; k++) {
				if (aux[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + 0].opaque == 0 && 
					blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + (CHUNK_W-1)].opaque == 0) continue;
				if (aux[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + 0].opaque == 0) {
					blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + (CHUNK_W-1)].hidden = 0;
					blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + (CHUNK_W-1)].outside |= 1<<PX;
				}
				if (blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + (CHUNK_W-1)].opaque == 0) {
					aux[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + 0].hidden = 0;
					aux[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + 0].outside |= 1<<NX;
				}
				//blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + (CHUNK_W-1)].hidden &= aux[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + 0].opaque;
				//aux[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + 0].hidden &= blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + (CHUNK_W-1)].opaque;
			}
		}
		break;
	case Render::NX : 
		for (int j=0; j<CHUNK_L; j++) {
			for (int k=0; k<CHUNK_H; k++) {
				if (aux[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + (CHUNK_W-1)].opaque == 0 && 
					blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + 0].opaque == 0) continue;
				if (aux[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + (CHUNK_W-1)].opaque == 0) {
					blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + 0].hidden = 0;
					blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + 0].outside |= 1<<NX;
				}
				if (blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + 0].opaque == 0) {
					aux[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + (CHUNK_W-1)].hidden = 0;
					aux[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + (CHUNK_W-1)].outside |= 1<<PX;
				}
				//blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + 0].hidden &= aux[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + (CHUNK_W-1)].opaque;
				//aux[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + (CHUNK_W-1)].hidden &= blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + 0].opaque;
			}
		}
		break;
	case Render::PY : 
		for (int i=0; i<CHUNK_W; i++) {
			for (int k=0; k<CHUNK_H; k++) {
				if (aux[k*(CHUNK_W*CHUNK_L) + 0*(CHUNK_W) + i].opaque == 0 && 
					blocks[k*(CHUNK_W*CHUNK_L) + (CHUNK_L-1)*(CHUNK_W) + i].opaque == 0) continue;
				if (aux[k*(CHUNK_W*CHUNK_L) + 0*(CHUNK_W) + i].opaque == 0) {
					blocks[k*(CHUNK_W*CHUNK_L) + (CHUNK_L-1)*(CHUNK_W) + i].hidden = 0;
					blocks[k*(CHUNK_W*CHUNK_L) + (CHUNK_L-1)*(CHUNK_W) + i].outside |= 1<<PY;
				}
				if (blocks[k*(CHUNK_W*CHUNK_L) + (CHUNK_L-1)*(CHUNK_W) + i].opaque == 0) {
					aux[k*(CHUNK_W*CHUNK_L) + 0*(CHUNK_W) + i].hidden = 0;
					aux[k*(CHUNK_W*CHUNK_L) + 0*(CHUNK_W) + i].outside |= 1<<NY;
				}
				//blocks[k*(CHUNK_W*CHUNK_L) + (CHUNK_L-1)*(CHUNK_W) + i].hidden &= aux[k*(CHUNK_W*CHUNK_L) + 0*(CHUNK_W) + i].opaque;
				//aux[k*(CHUNK_W*CHUNK_L) + 0*(CHUNK_W) + i].hidden &= blocks[k*(CHUNK_W*CHUNK_L) + (CHUNK_L-1)*(CHUNK_W) + i].opaque;
			}
		}
		break;
	case Render::NY : 
		for (int i=0; i<CHUNK_W; i++) {
			for (int k=0; k<CHUNK_H; k++) {
				if (aux[k*(CHUNK_W*CHUNK_L) + (CHUNK_L-1)*(CHUNK_W) + i].opaque == 0 &&
					blocks[k*(CHUNK_W*CHUNK_L) + 0*(CHUNK_W) + i].opaque == 0) continue;
				if (aux[k*(CHUNK_W*CHUNK_L) + (CHUNK_L-1)*(CHUNK_W) + i].opaque == 0) {
					blocks[k*(CHUNK_W*CHUNK_L) + 0*(CHUNK_W) + i].hidden = 0;
					blocks[k*(CHUNK_W*CHUNK_L) + 0*(CHUNK_W) + i].outside |= 1<<NY;
				}
				if (blocks[k*(CHUNK_W*CHUNK_L) + 0*(CHUNK_W) + i].opaque == 0) {
					aux[k*(CHUNK_W*CHUNK_L) + (CHUNK_L-1)*(CHUNK_W) + i].hidden = 0;
					aux[k*(CHUNK_W*CHUNK_L) + (CHUNK_L-1)*(CHUNK_W) + i].outside |= 1<<PY;
				}
				//blocks[k*(CHUNK_W*CHUNK_L) + 0*(CHUNK_W) + i].hidden &= aux[k*(CHUNK_W*CHUNK_L) + (CHUNK_L-1)*(CHUNK_W) + i].opaque;
				//aux[k*(CHUNK_W*CHUNK_L) + (CHUNK_L-1)*(CHUNK_W) + i].hidden &= blocks[k*(CHUNK_W*CHUNK_L) + 0*(CHUNK_W) + i].opaque;
			}
		}
		break;
	case Render::PZ : 
		for (int i=0; i<CHUNK_W; i++) {
			for (int j=0; j<CHUNK_L; j++) {
				if (aux[0*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque == 0 &&
					blocks[(CHUNK_H-1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque == 0) continue;
				if (aux[0*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque == 0) {
					blocks[(CHUNK_H-1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden = 0;
					blocks[(CHUNK_H-1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].outside |= 1<<PZ;
				}
				if (blocks[(CHUNK_H-1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque == 0) {
					aux[0*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden = 0;
					aux[0*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].outside |= 1<< NZ;
				}
				//blocks[(CHUNK_H-1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden &= aux[0*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque;
				//aux[0*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden &= blocks[(CHUNK_H-1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque;
			}
		}
		break;
	case Render::NZ : 
		for (int i=0; i<CHUNK_W; i++) {
			for (int j=0; j<CHUNK_L; j++) {
				if (aux[(CHUNK_H-1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque == 0 &&
					blocks[0*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque == 0) continue;
				if (aux[(CHUNK_H-1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque == 0) {
					blocks[0*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden = 0;
					blocks[0*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].outside |= 1<<NZ;
				}
				if (blocks[0*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque == 0) {
					aux[(CHUNK_H-1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden = 0;
					aux[(CHUNK_H-1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].outside |= 1<<PZ;
				}
				//blocks[0*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden &= aux[(CHUNK_H-1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque;
				//aux[(CHUNK_H-1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden &= blocks[0*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque;
			}
		}
		break;
	}
}

void Render::GetTextureCoordinates(short int type, int dir, float2 &dst) {
	// decide texture coordinates corr. face & type
	GLuint side_texture;
	switch (type) {
	case Block::CRATE: // Draw a crate
		side_texture = TextureMgr::CRATE;
		break;
	case Block::GRASS:
		switch (dir) {
		case PZ: side_texture = TextureMgr::GRASS_TOP; break;
		case NZ: side_texture = TextureMgr::GRASS_BUTTOM; break;
		default: side_texture = TextureMgr::GRASS_SIDE; break;
		}
		break;
	case Block::SOIL:
		side_texture = TextureMgr::SOIL;
		break;
	case Block::STONE:
		side_texture = TextureMgr::STONE;
		break;
	case Block::GOLD_MINE:
		side_texture = TextureMgr::GOLD_MINE;
		break;
	case Block::COAL_MINE:
		side_texture = TextureMgr::COAL_MINE;
		break;
	case Block::COAL:
		side_texture = TextureMgr::COAL;
		break;
	case Block::SAND:
		side_texture = TextureMgr::SAND;
		break;
	case Block::GLASS:
		side_texture = TextureMgr::GLASS;
		break;
	case Block::LAVA:
		side_texture = TextureMgr::LAVA;
		break;
	case Block::SNOW:
		switch (dir) {
		case PZ: side_texture = TextureMgr::SNOW_TOP; break;
		case NZ: side_texture = TextureMgr::SNOW_BUTTOM; break;
		default: side_texture = TextureMgr::SNOW_SIDE; break;
		}
		break;
	default: break;
	}

	// calculate and set coordinates of textures
	dst = get_texture_coord(side_texture);
}

void Render::LoadChunk(render_chunk *ren_chk, map_chunk *map_chk, int urgent) {
	if (urgent == 0) { // multithreading
		render_pair pair(ren_chk, map_chk);
		m_Thread->PushJobs(pair);
		return;
	}
	else {
		m_Thread->threadLoadChunk( std::pair<render_chunk *, map_chunk *>(ren_chk, map_chk), m_Thread);
	}
}

/**********************************************************
	now the only work is to improve UpdateVBO to make
	it faster and need not rebuild all shit around

	LoadNeededChunks :	if (didn't have) call threadLoadChunk, make empty one, modified = 1   # since VBO = 0, drawing will do nothing
						else if (had) if (modified == 1) UpdateVBO, modified = 0

		## rename 'modified' to 'rendered'

***********************************************************/

void Render::UpdateVBO(render_chunk *ren_chk, map_chunk *map_chk) {
	if (ren_chk == 0)
		return;

	if (map_chk == 0 || ren_chk->id != map_chk->id || map_chk->loaded == 0 || map_chk->failed == 1 || map_chk->unneeded == 1) {
		ren_chk->failed = 1;
		return;
	}
	
	ren_chk->failed = 0;
	ren_chk->loaded = 0;
	ren_chk->unneeded = 0;


	// calculate how many blocks we need to store in VBO and its required size
	
	CalculateVisible(map_chk->id, s_World);
	
	Block *blocks = map_chk->blocks;
	int size = 0;
	for (int i=0; i<CHUNK_W*CHUNK_L*CHUNK_H; i++) {
		if (blocks[i].type == Block::NUL) continue;
		
		if (blocks[i].hidden == 1)
			continue;

		for (int w=0; w<6; w++) {
			if ((blocks[i].outside & (1<<w)) != 0)
				size++;
		}
		//size++;
	}
		
	int newvbo = 0;
	if (ren_chk->vertices == 0) {
		newvbo = 1;
		ren_chk->vertices = new GLfloat[size*20];
		ren_chk->vbo_size = size;
	}
	if (size > ren_chk->vbo_size) {
		if (ren_chk->vertices != 0) {
			delete[] ren_chk->vertices;
			ren_chk->vertices = 0;
		}
		ren_chk->vertices = new GLfloat[size*20];
		
		ren_chk->vbo_size = size;
		newvbo = 1;
	}

	GLfloat *vertices = ren_chk->vertices;

	if (vertices == 0) {
		MessageBox(0, "vertices alloc failed", "haha", 0);
		ren_chk->failed = 1;
		return;
	}
	
	QueryPerformanceCounter(&lastTick);
	GenerateVBOArray(vertices, map_chk->blocks);
	QueryPerformanceCounter(&currTick);

	ren_chk->num_faces = size;

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, ren_chk->vbo);

	// now upload to graphics card
	
	if (newvbo == 1)
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, size*20*sizeof(GLfloat), ren_chk->vertices, GL_STATIC_DRAW_ARB);
	else
		glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, size*20*sizeof(GLfloat), ren_chk->vertices);
	
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	map_chk->modified = 0;
	ren_chk->loaded = 1;
}


void Render::DiscardUnneededChunks(float3 pos, float3 dir, World *world) {
	chunk_list *chunks = world->GetRenderChunks(pos, dir);
	render_list::iterator render_it;

	for (render_it = r_chunks.begin(); render_it != r_chunks.end(); ) {
		int3 id = (*render_it).first;
		render_chunk *renderchk = (*render_it).second;
		
		if (renderchk == 0)
			continue;
		
		if (renderchk->failed == 0 && renderchk->loaded == 0) // still loading
			continue;

		if (chunks->find(id) == chunks->end()) { // in render but no in map -> unneeded
			DeleteChunk(renderchk);
			r_chunks.erase(render_it++);
		}
		else if (renderchk->failed == 1 || renderchk->unneeded == 1) { // check flags
			DeleteChunk(renderchk);
			r_chunks.erase(render_it++);
		}
		else {
			++render_it;
		}
	}
}

void Render::PrintChunkStatistics(char *buffer) {
	render_list::iterator it;

	int total = 0;
	for (it = r_chunks.begin(); it != r_chunks.end(); ++it) {
		total++;
	}

	snprintf(buffer, 16, "r_total:%d", total); // overkill
}

render_chunk *Render::CreateEmptyChunk() {
	render_chunk *renderchk = 0;
	renderchk = new render_chunk();

	if(renderchk == 0) {
		return 0;
	}

	renderchk->loaded = 0;
	renderchk->failed = 0;
	renderchk->unneeded = 0;
	renderchk->num_faces = 0;
	renderchk->id = int3(0, 0, 0);
	renderchk->vbo = 0;
	renderchk->vbo_size = 0;
	renderchk->vertices = 0;

	return renderchk;
}

void Render::LoadNeededChunks(float3 pos, float3 dir, World *world) {
	chunk_list *chunks = world->GetRenderChunks(pos, dir);
	chunk_list::iterator map_it;

	for (map_it = chunks->begin(); map_it != chunks->end(); ++map_it) {
		int3 id = (*map_it).first;
		map_chunk *map_chk = (*map_it).second;

		if (map_chk->failed == 1 || map_chk->loaded == 0 || map_chk->unneeded == 1)
			continue;

		// if we have it in vbo list

		render_list::iterator ren_it = r_chunks.find(id);
		if (ren_it == r_chunks.end()) { // not found, need to create it
			render_chunk *renderchk = CreateEmptyChunk();

			if(renderchk == 0) {
				MessageBox(0, "RENDER ALLOCATION FAILED", "haha", 0);
				break;
			}

			renderchk->id = id;
			// always insert first
			r_chunks.insert( std::pair<int3, render_chunk *>(id, renderchk) );
			LoadChunk(renderchk, (*map_it).second, 1); // NOT TRYING MULTITHREADING!!!!
		}
		else {
			if (map_chk->modified == 1) {
	
				UpdateVBO(ren_it->second, map_chk);
				
				/*for (int w=0; w<6; w++) {
					int3 side = id;
					switch (w) {
					case PX: side.x++; break;
					case NX: side.x--; break;
					case PY: side.y++; break;
					case NY: side.y--; break;
					case PZ: side.z++; break;
					case NZ: side.z--; break;
					}
					render_list::iterator ren_it = r_chunks.find(side);
					chunk_list::iterator map_it = chunks->find(side);
					
					if (ren_it != r_chunks.end() && map_it != chunks->end()) {
						UpdateVBO(ren_it->second, map_it->second);
					}
				}*/
				map_chk->modified = 0;
			}
		}
	}
}

void Render::RenderChunk(render_chunk *tmp, float3 pos, float3 dir) {
	if (tmp == 0 || tmp->failed == 1 || tmp->loaded == 0 || tmp->unneeded == 1)
		return;

	// frustum culling

	int3 id = tmp->id;

	// right plane normal
	normalize(dir);
	float3 rhs = float3(dir.y, -dir.x, 0);
	normalize(rhs);
	float r = sqrt(dir.x*dir.x + dir.y*dir.y);
	float3 upside = float3(-dir.z*dir.x/r, -dir.z*dir.y/r, r);

	float3 normal;
	for (int w=0; w<4; w++) {
		float3 n(id.x+1.0f, id.y+1.0f, id.z+1.0f);
		switch (w) {
		case 0:
			normal = cross_prod( zNear*dir + zNear*rhs, upside);
			break;
		case 1:
			normal = cross_prod(upside, zNear*dir - rhs*zNear);
			break;
		case 2:
			normal = cross_prod(rhs, zNear*dir + upside*zNear);
			break;
		case 3:
			normal = cross_prod(zNear*dir - upside*zNear, rhs);
			break;
		}
		if (normal.x > 0) { n.x--; }
		if (normal.y > 0) { n.y--; }
		if (normal.z > 0) { n.z--; }

		n.x *= CHUNK_W; n.y *= CHUNK_L; n.z *= CHUNK_H;
		n = BLOCK_LEN*n - pos;

		if (dot_prod(normal, n) > 0)
			return;
	}
	
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, tmp->vbo);
	
	glTexCoordPointer(2, GL_FLOAT, 5*sizeof(GLfloat), 0);
	glVertexPointer(3, GL_FLOAT, 5*sizeof(GLfloat), (void *)(2*sizeof(GLfloat)));
	
    glDrawArrays( GL_QUADS, 0, tmp->num_faces*4 );

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

// No VBO version
extern float fovY;
extern GLuint blockDisplayList;
void Render::RenderChunk0(map_chunk *chk, float3 pos, float3 dir) {


	float3 rel;
	
	// do whole chunk's culling

	int3 cid = chk->id;

	// right plane normal
	normalize(dir);
	float3 rhs = float3(dir.y, -dir.x, 0);
	normalize(rhs);
	float r = sqrt(dir.x*dir.x + dir.y*dir.y);
	float3 upside = float3(-dir.z*dir.x/r, -dir.z*dir.y/r, r);

	float3 normal;

	for (int w=0; w<4; w++) {
		float3 n(cid.x+1.0f, cid.y+1.0f, cid.z+1.0f);
		switch (w) {
		case 0:
			normal = cross_prod(zNear*dir + rhs*zNear, upside);
			break;
		case 1:
			normal = cross_prod(upside, zNear*dir - rhs*zNear);
			break;
		case 2:
			normal = cross_prod(rhs, zNear*dir + upside*zNear);
			break;
		case 3:
			normal = cross_prod(zNear*dir - upside*zNear, rhs);
			break;
		}
		if (normal.x > 0) { n.x--; }
		if (normal.y > 0) { n.y--; }
		if (normal.z > 0) { n.z--; }

		n.x *= CHUNK_W; n.y *= CHUNK_L; n.z *= CHUNK_H;

		n = BLOCK_LEN*n - pos;

		if (dot_prod(normal, n) > 0) // all outside
			return;
	}

	int type;
	for (int i=0; i<CHUNK_W; i++) {
		for (int j=0; j<CHUNK_L; j++) {
			for (int k=0; k<CHUNK_H; k++) {
				type = (chk->blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i]).type;
				
				if (type == Block::NUL) continue;
				if (chk->blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden == 1)
					continue;

				rel.x = (chk->id.x*CHUNK_W + i)*BLOCK_LEN - pos.x;
				rel.y = (chk->id.y*CHUNK_L + j)*BLOCK_LEN - pos.y;
				rel.z = (chk->id.z*CHUNK_H + k)*BLOCK_LEN - pos.z;
				
				glTranslatef(i*1.0f, j*1.0f, k*1.0f);
				glCallList(blockDisplayList + type);
				glTranslatef(-i*1.0f, -j*1.0f, -k*1.0f);
			}
		}
	}
}


/*************************************************************************
	Decides what to draw, how much to draw, from given perspective
*************************************************************************/
void Render::DrawScene(float3 pos, float3 dir, float dist, World* world) {

	glPushMatrix();

	glBegin(GL_QUADS);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 100, 0);
	glVertex3f(100, 100, 0);
	glVertex3f(100, 0, 0);
	glEnd();
	gluLookAt(pos.x, pos.y, pos.z, pos.x+dir.x, pos.y+dir.y, pos.z+dir.z, 0, 0, 1);

	glBindTexture(GL_TEXTURE_2D, s_Texture->block_texture);

	glScalef(BLOCK_LEN, BLOCK_LEN, BLOCK_LEN);
	
	render_list::iterator it;

	for (it = r_chunks.begin(); it != r_chunks.end(); ++it) {
		int3 id = (*it).first;
		render_chunk *tmp = (*it).second;

		if (tmp == 0 || tmp->failed == 1 || tmp->loaded == 0 || tmp->unneeded == 1)
			continue;

		// calculate to world coordinates
		id.x *= CHUNK_W;
		id.y *= CHUNK_L;
		id.z *= CHUNK_H;
		
		glTranslatef((GLfloat)id.x, (GLfloat)id.y, (GLfloat)id.z);
		RenderChunk(tmp, pos, dir);
		glTranslatef(-(GLfloat)id.x, -(GLfloat)id.y, -(GLfloat)id.z);
	}

	glPopMatrix();

}

// No VBO version

void Render::DrawScene0(float3 pos, float3 dir, float dist, World* world) {

	glPushMatrix();	

	gluLookAt(pos.x, pos.y, pos.z, pos.x+dir.x, pos.y+dir.y, pos.z+dir.z, 0, 0, 1);

	chunk_list *list = s_World->GetRenderChunks(pos, dir);
	chunk_list::iterator it;
	
	glBindTexture(GL_TEXTURE_2D, s_Texture->block_texture);
	
	glScalef(BLOCK_LEN, BLOCK_LEN, BLOCK_LEN);
	
	for (it = list->begin(); it != list->end(); ++it) {
		int3 id = (*it).first;
		map_chunk *tmp = (*it).second;
		if (tmp->loaded == 0 || tmp->failed == 1 || tmp->unneeded == 1)
			continue;

		if (tmp->modified == 1) {
			CalculateVisible(tmp->id, world);
			tmp->modified = 0;
		}
		// calculate to world coordinates
		id.x *= CHUNK_W;
		id.y *= CHUNK_L;
		id.z *= CHUNK_H;
		
		glTranslatef((GLfloat)id.x, (GLfloat)id.y, (GLfloat)id.z);
		RenderChunk0(tmp, pos, dir);
		glTranslatef(-(GLfloat)id.x, -(GLfloat)id.y, -(GLfloat)id.z);
	}

	glPopMatrix();
}

void Render::RenderChunkThread::threadLoop(void *param) {
	// should get here very early
#ifndef APPLE
	wglMakeCurrent(hDC, hRC2);
#endif
	
	Render::RenderChunkThread *self = (Render::RenderChunkThread *)param;
	while (self->active) {
		// peek jobs
		if (self->jobs.empty()) {
			Sleep(20);
		}
		else {
			// process jobs
			render_pair pair = self->jobs.front();
			self->threadLoadChunk(pair, self);
			self->jobs.pop();
		}
	}
}

extern TextureMgr *s_Texture;
void Render::GenerateVBOArray(GLfloat *vertices, Block *blocks) {
	
	if (vertices == 0) {
		MessageBox(0, "wrong usage", "ha", 0);
		return;
	}
	
	int count = 0;
	// now generate vertex & quads
	for (int i=0; i<CHUNK_W; i++) {
		for (int j=0; j<CHUNK_L; j++) {
			for (int k=0; k<CHUNK_H; k++) {
				int type = blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].type;

				if (type == Block::NUL)
					continue;

				if (blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden == 1)
					continue;

				for (int w=0; w<6; w++) { // 6 faces
					if ((blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].outside & (1<<w)) == 0) continue;
					float2 coord;
					GetTextureCoordinates(type, w, coord);
					coord.x += 0.001f;
					coord.y += 0.001f;
					float csize = 1/16.0f;
					csize -= 0.002f;

					// texture coordinates are in the same order in each face except NX
					if (w != NX) {
						vertices[count*20 + 0] = coord.x;
						vertices[count*20 + 1] = coord.y;
						vertices[count*20 + 5] = coord.x+csize;
						vertices[count*20 + 6] = coord.y;
						vertices[count*20 + 10] = coord.x+csize;
						vertices[count*20 + 11] = coord.y+csize;
						vertices[count*20 + 15] = coord.x;
						vertices[count*20 + 16] = coord.y+csize;
					}
					else {
						vertices[count*20 + 0] = coord.x;
						vertices[count*20 + 1] = coord.y;
						vertices[count*20 + 5] = coord.x;
						vertices[count*20 + 6] = coord.y+csize;
						vertices[count*20 + 10] = coord.x+csize;
						vertices[count*20 + 11] = coord.y+csize;
						vertices[count*20 + 15] = coord.x+csize;
						vertices[count*20 + 16] = coord.y;
					}
					// now setup vertex coordinates of each face
					switch (w) {
					case PZ:
						vertices[count*20 + 2] = 0.0f;
						vertices[count*20 + 3] = 0.0f;
						vertices[count*20 + 4] = 1.0f;

						vertices[count*20 + 7] = 1.0f;
						vertices[count*20 + 8] = 0.0f;
						vertices[count*20 + 9] = 1.0f;

						vertices[count*20 + 12] = 1.0f;
						vertices[count*20 + 13] = 1.0f;
						vertices[count*20 + 14] = 1.0f;

						vertices[count*20 + 17] = 0.0f;
						vertices[count*20 + 18] = 1.0f;
						vertices[count*20 + 19] = 1.0f;
						break;
					case NZ:
						vertices[count*20 + 2] = 1.0f;
						vertices[count*20 + 3] = 0.0f;
						vertices[count*20 + 4] = 0.0f;

						vertices[count*20 + 7] = 0.0f;
						vertices[count*20 + 8] = 0.0f;
						vertices[count*20 + 9] = 0.0f;

						vertices[count*20 + 12] = 0.0f;
						vertices[count*20 + 13] = 1.0f;
						vertices[count*20 + 14] = 0.0f;

						vertices[count*20 + 17] = 1.0f;
						vertices[count*20 + 18] = 1.0f;
						vertices[count*20 + 19] = 0.0f;
						break;
					case PY:
						vertices[count*20 + 2] = 1.0f;
						vertices[count*20 + 3] = 1.0f;
						vertices[count*20 + 4] = 0.0f;

						vertices[count*20 + 7] = 0.0f;
						vertices[count*20 + 8] = 1.0f;
						vertices[count*20 + 9] = 0.0f;

						vertices[count*20 + 12] = 0.0f;
						vertices[count*20 + 13] = 1.0f;
						vertices[count*20 + 14] = 1.0f;

						vertices[count*20 + 17] = 1.0f;
						vertices[count*20 + 18] = 1.0f;
						vertices[count*20 + 19] = 1.0f;
						break;
					case NY:
						vertices[count*20 + 2] = 0.0f;
						vertices[count*20 + 3] = 0.0f;
						vertices[count*20 + 4] = 0.0f;

						vertices[count*20 + 7] = 1.0f;
						vertices[count*20 + 8] = 0.0f;
						vertices[count*20 + 9] = 0.0f;

						vertices[count*20 + 12] = 1.0f;
						vertices[count*20 + 13] = 0.0f;
						vertices[count*20 + 14] = 1.0f;

						vertices[count*20 + 17] = 0.0f;
						vertices[count*20 + 18] = 0.0f;
						vertices[count*20 + 19] = 1.0f;
						break;
					case PX:
						vertices[count*20 + 2] = 1.0f;
						vertices[count*20 + 3] = 0.0f;
						vertices[count*20 + 4] = 0.0f;

						vertices[count*20 + 7] = 1.0f;
						vertices[count*20 + 8] = 1.0f;
						vertices[count*20 + 9] = 0.0f;

						vertices[count*20 + 12] = 1.0f;
						vertices[count*20 + 13] = 1.0f;
						vertices[count*20 + 14] = 1.0f;

						vertices[count*20 + 17] = 1.0f;
						vertices[count*20 + 18] = 0.0f;
						vertices[count*20 + 19] = 1.0f;
						break;
					case NX:
						vertices[count*20 + 2] = 0.0f;
						vertices[count*20 + 3] = 0.0f;
						vertices[count*20 + 4] = 0.0f;

						vertices[count*20 + 7] = 0.0f;
						vertices[count*20 + 8] = 0.0f;
						vertices[count*20 + 9] = 1.0f;

						vertices[count*20 + 12] = 0.0f;
						vertices[count*20 + 13] = 1.0f;
						vertices[count*20 + 14] = 1.0f;

						vertices[count*20 + 17] = 0.0f;
						vertices[count*20 + 18] = 1.0f;
						vertices[count*20 + 19] = 0.0f;
						break;
					}

					// translate to relative position
					vertices[count*20 + 2] += (GLfloat)i;
					vertices[count*20 + 3] += (GLfloat)j;
					vertices[count*20 + 4] += (GLfloat)k;

					vertices[count*20 + 7] += (GLfloat)i;
					vertices[count*20 + 8] += (GLfloat)j;
					vertices[count*20 + 9] += (GLfloat)k;

					vertices[count*20 + 12] += (GLfloat)i;
					vertices[count*20 + 13] += (GLfloat)j;
					vertices[count*20 + 14] += (GLfloat)k;

					vertices[count*20 + 17] += (GLfloat)i;
					vertices[count*20 + 18] += (GLfloat)j;
					vertices[count*20 + 19] += (GLfloat)k;
					// next face
					count++;
				}
			}
		}
	}
}

void Render::RenderChunkThread::threadLoadChunk(render_pair pair, Render::RenderChunkThread *self) {
	render_chunk *ren_chk = pair.first;
	map_chunk *map_chk = pair.second;

	if (ren_chk == 0)
		return;

	ren_chk->failed = 0;
	ren_chk->loaded = 0;
	ren_chk->unneeded = 0;
	ren_chk->vbo_size = 0;
	ren_chk->num_faces = 0;

	if (map_chk == 0 || ren_chk->id != map_chk->id) {
		ren_chk->failed = 1;
		return;
	}

	if (map_chk->loaded == 0 || map_chk->failed == 1 || map_chk->unneeded == 1) {
		ren_chk->failed = 1;
		return;
	}

	//glDeleteBuffersARB(1, &ren_chk->vbo);
	glGenBuffersARB(1, &ren_chk->vbo);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, ren_chk->vbo);

	// now upload to graphics card
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, 0, 0, GL_STATIC_DRAW_ARB);
	
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	ren_chk->loaded = 1;
}
