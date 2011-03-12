#include "Render.h"

#include "World.h"

inline float2 get_texture_coord(int tx) {
	int2 coord;
	coord.y = tx/TX_ROW;
	coord.x = tx%TX_ROW;
	float2 ff;
	ff.x = coord.x/16.0f;
	ff.y = coord.y/16.0f;
	return ff;
}

void Render::DrawFaceSimple(int i, int j, int k, int type, int dir) {
	if (type == 0) return;

	glTranslatef(i*1.0f, j*1.0f, k*1.0f);
	GLuint side_texture;

	switch (type) {
	case Block::CRATE: // Draw a crate
		side_texture = TextureMgr::CRATE;
		break;
	case Block::GRASS: // Draw a crate
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

extern DWORD tick1, tick2;



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
	if (chk == 0)
		return;

	glDeleteBuffersARB(1, &chk->vbo);

	delete chk;
	chk = 0;
}

void Render::CalculateVisible(int3 id) {
	if (s_World == 0) {
		MessageBox(0, "s_World NULL", "HAHAH", 0);
		return;
	}

	chunk_list chunks = s_World->world_map.m_chunks;
	chunk_list::iterator it = chunks.find(id);

	map_chunk *map_chk = (*it).second;
	if (map_chk == 0 || map_chk->loaded == 0 || map_chk->failed == 1 || map_chk->unneeded == 1) {
		return;
	}

	int type;
	map_chunk *map_side[6];
	for (int w=0; w<6; w++) {
		chunk_list::iterator tmp;
		switch (w) {
		case PX : tmp = chunks.find(int3(id.x+1, id.y, id.z)); break;
		case NX : tmp = chunks.find(int3(id.x-1, id.y, id.z)); break;
		case PY : tmp = chunks.find(int3(id.x, id.y+1, id.z)); break;
		case NY : tmp = chunks.find(int3(id.x, id.y-1, id.z)); break;
		case PZ : tmp = chunks.find(int3(id.x, id.y, id.z+1)); break;
		case NZ : tmp = chunks.find(int3(id.x, id.y, id.z-1)); break;
		}

		map_side[w] = (tmp == chunks.end() ? 0 : (*tmp).second);
		if (map_side[w] == 0 || map_side[w]->loaded == 0 || map_side[w]->failed == 1 || map_side[w]->unneeded == 1) {
			map_side[w] = 0;
		}
	}

	unsigned int side; // occupied
	for (int i=0; i<CHUNK_W; i++) {
		for (int j=0; j<CHUNK_L; j++) {
			for (int k=0; k<CHUNK_H; k++) {
				type = map_chk->blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].type;
				if (type == Block::NUL) {
					map_chk->blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden = 1;
					continue;
				}

				side = 1; // filled

				if (side == 1 && i == 0) {
					if (map_side[NX] == 0) side = 0;
					else side &= (map_side[NX]->blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + CHUNK_W-1].type != Block::NUL);
				}
				else if (side == 1 && i == CHUNK_W-1) {
					if (map_side[PX] == 0) side = 0;
					else side &= (map_side[PX]->blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + 0].type != Block::NUL);
				}
				else if (side == 1  && i != 0 && i != CHUNK_W-1) {
					side &= (map_chk->blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i + 1].type != Block::NUL);
					side &= (map_chk->blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i - 1].type != Block::NUL);
				}

				if (side == 1 && j == 0) {
					if (map_side[NY] == 0) side = 0;
					else side &= (map_side[NY]->blocks[k*(CHUNK_W*CHUNK_L) + (CHUNK_L-1)*(CHUNK_W) + i].type != Block::NUL);
				} 
				else if (side == 1 && j == CHUNK_L-1) {
					if (map_side[PY] == 0) side = 0;
					else side &= (map_side[PY]->blocks[k*(CHUNK_W*CHUNK_L) + 0*(CHUNK_W) + i].type != Block::NUL);
				}
				else if (side == 1 && j != 0 && j != CHUNK_L-1) {
					side &= (map_chk->blocks[k*(CHUNK_W*CHUNK_L) + (j+1)*(CHUNK_W) + i].type != Block::NUL);
					side &= (map_chk->blocks[k*(CHUNK_W*CHUNK_L) + (j-1)*(CHUNK_W) + i].type != Block::NUL);
				}

				if (side == 1 && k == 0) {
					if (map_side[NZ] == 0) side = 0;
					else side &= (map_side[NZ]->blocks[(CHUNK_H-1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].type != Block::NUL);
				}
				else if (side == 1 && k == CHUNK_H-1) {
					if (map_side[PZ] == 0) side = 0;
					else side &= (map_side[PZ]->blocks[0*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].type != Block::NUL);
				}
				else if (side == 1 && k != 0 && k != CHUNK_H-1) {
					side &= (map_chk->blocks[(k+1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].type != Block::NUL);
					side &= (map_chk->blocks[(k-1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].type != Block::NUL);
				}

				if (side == 1)
					map_chk->blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden = 1;
				else
					map_chk->blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden = 0;
			}
		}
	}
}

void Render::GetTextureCoordinates(short int type, int dir, float2 &dst) {
	// decide texture coordinates corr. face & type
	GLuint side_texture;
	switch (type) {
	case Block::CRATE: // Draw a crate
		side_texture = TextureMgr::CRATE;
		break;
	case Block::GRASS: // Draw a crate
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

	// calculate and set coordinates of textures
	float csize = 1/16.0f;
	dst = get_texture_coord(side_texture);
}

void Render::LoadChunk(render_chunk *ren_chk, map_chunk *map_chk, int urgent) {
	if (urgent == 0) {
		render_pair pair(ren_chk, map_chk);
		m_Thread->PushJobs(pair);
		return;
	}  //  Doesn't work, so disable this feature

	if (ren_chk == 0)
		return;

	ren_chk->failed = 0;
	ren_chk->loaded = 0;
	ren_chk->unneeded = 0;

	if (map_chk == 0 || ren_chk->id != map_chk->id) {
		ren_chk->failed = 1;
		return;
	}

	if (map_chk->loaded == 0 || map_chk->failed == 1 || map_chk->unneeded == 1) {
		ren_chk->failed = 1;
		return;
	}


	// calculate how many blocks we need to store in VBO and its required size
	CalculateVisible(map_chk->id);

	int size = 0;
	for (int i=0; i<CHUNK_W; i++) {
		for (int j=0; j<CHUNK_L; j++) {
			for (int k=0; k<CHUNK_H; k++) {
				if (map_chk->blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden == 0) {
					size++;
				}
			}
		}
	}

	ren_chk->num_faces = size*6;

	// size = #CUBES, required size = #CUBE*6*( (2+3)*4 )
	GLfloat *vertices = 0;
	vertices = new GLfloat[size*6*20];

	if (vertices == 0) {
		MessageBox(0, "VERTICES ALLOC FAILED", "haha", 0);
		ren_chk->failed = 1;
		return;
	}

	int count = 0;
	// now generate vertex & quads
	for (int i=0; i<CHUNK_W; i++) {
		for (int j=0; j<CHUNK_L; j++) {
			for (int k=0; k<CHUNK_H; k++) {
				if (map_chk->blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden == 1)
					continue;

				int type = map_chk->blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].type;

				for (int w=0; w<6; w++) { // 6 faces
					float2 coord;
					GetTextureCoordinates(type, w, coord);
					float csize = 1/16.0f;

					// texture coordinates are in the same order in each face except NX
					if (w == NX) {
						vertices[count*20 + 0] = coord.x;
						vertices[count*20 + 1] = coord.y;
						vertices[count*20 + 5] = coord.x;
						vertices[count*20 + 6] = coord.y+csize;
						vertices[count*20 + 10] = coord.x+csize;
						vertices[count*20 + 11] = coord.y+csize;
						vertices[count*20 + 15] = coord.x+csize;
						vertices[count*20 + 16] = coord.y;
					}
					else {
						vertices[count*20 + 0] = coord.x;
						vertices[count*20 + 1] = coord.y;
						vertices[count*20 + 5] = coord.x+csize;
						vertices[count*20 + 6] = coord.y;
						vertices[count*20 + 10] = coord.x+csize;
						vertices[count*20 + 11] = coord.y+csize;
						vertices[count*20 + 15] = coord.x;
						vertices[count*20 + 16] = coord.y+csize;
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
					for (int s=0; s<4; s++) {
						vertices[count*20 + s*5 + 2] += (GLfloat)i;
						vertices[count*20 + s*5 + 3] += (GLfloat)j;
						vertices[count*20 + s*5 + 4] += (GLfloat)k;
					}

					// next face
					count++;
				}
			}
		}
	}
	
	glGenBuffersARB(1, &ren_chk->vbo);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, ren_chk->vbo);

	// now upload to graphics card
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, size*6*20*sizeof(GLfloat), vertices, GL_STATIC_DRAW_ARB);

	GLenum err = glGetError();
	if (err != 0) {
		MessageBox(0, "OUT of memoery!!!", "aha", 0);
	}

	delete[] vertices;
	vertices = 0;

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

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
			render_it = r_chunks.erase(render_it);
		}
		else if (renderchk->failed == 1 || renderchk->unneeded == 1) { // check flags
			DeleteChunk(renderchk);
			render_it = r_chunks.erase(render_it);
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

	sprintf_s(buffer, 16, "r_total:%d", total); // overkill
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

		if (r_chunks.find(id) == r_chunks.end()) { // not found, need to create it
			render_chunk *renderchk = 0;
			renderchk = new render_chunk();
			if(renderchk == 0) {
				MessageBox(0, "RENDER ALLOCATION FAILED", "haha", 0);
				break;
			}

			renderchk->id = id;
			renderchk->loaded = 0;
			renderchk->failed = 0;
			renderchk->unneeded = 0;
			
			// always insert first
			r_chunks.insert( std::pair<int3, render_chunk *>(id, renderchk) );
			LoadChunk(renderchk, (*map_it).second, 1); // NOT TRYING MULTITHREADING !!!!
		}
	}
}

void Render::RenderChunk(render_chunk *tmp, float3 pos, float3 dir) {
	if (tmp == 0 || tmp->failed == 1 || tmp->loaded == 0 || tmp->unneeded == 1)
		return;

	int3 id((int)floor(pos.x / (BLOCK_LEN*CHUNK_W)), 
		(int)floor(pos.y / (BLOCK_LEN*CHUNK_L)), 
		(int)floor(pos.z / (BLOCK_LEN*CHUNK_H))); // Currently standing on this chunk

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, tmp->vbo);
	
	glInterleavedArrays( GL_T2F_V3F, 0, 0 );
    glDrawArrays( GL_QUADS, 0, tmp->num_faces*4 );

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

// No VBO version

void Render::RenderChunk0(map_chunk *chk, float3 pos, float3 dir) {
	float3 rel;
	float3 unit_x(1, 0, 0);
	float3 unit_y(0, 1, 0);
	float3 unit_z(0, 0, 1);
	
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

				// front side culling
				if (dot_prod(rel, dir) < 0) continue;

				// frustum culling

				
				if (rel.x > 0)
					DrawFaceSimple(i, j, k, type, NX);
				else
					DrawFaceSimple(i, j, k, type, PX);

				if (rel.y > 0)
					DrawFaceSimple(i, j, k, type, NY);
				else
					DrawFaceSimple(i, j, k, type, PY);

				if (rel.z > 0)
					DrawFaceSimple(i, j, k, type, NZ);
				else
					DrawFaceSimple(i, j, k, type, PZ);
			}
		}
	}
}


/*************************************************************************
	Decides what to draw, how much to draw, from given perspective
*************************************************************************/
void Render::DrawScene(float3 pos, float3 dir, float dist) {

	glPushMatrix();	

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

void Render::DrawScene0(float3 pos, float3 dir, float dist) {

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


/*****************************************************
	Multithreading will NOT WORK with OpenGL !!!!
******************************************************/


void Render::RenderChunkThread::threadLoop(void *param) {
	// should get here very early
	wglMakeCurrent(hDC, hRC2);
	
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


void Render::RenderChunkThread::threadLoadChunk(render_pair pair, Render::RenderChunkThread *self) {
	render_chunk *ren_chk = pair.first;
	map_chunk *map_chk = pair.second;

	self->render->LoadChunk(ren_chk, map_chk, 1);
}



/******************* Utility functions ************************/

#define HLSMAX		360
#define RGBMAX		255
#define UNDEFINED	(HLSMAX*2/3)
WORD HueToRGB(WORD n1, WORD n2, WORD hue)
{
	/* range check: note values passed add/subtract thirds of range */ 
	if (hue < 0)
		hue += HLSMAX;

	if (hue > HLSMAX)
		hue -= HLSMAX;

	/* return r,g, or b value from this tridrant */ 
	if (hue < (HLSMAX/6))
		return ( n1 + (((n2-n1)*hue+(HLSMAX/12))/(HLSMAX/6)) );
	if (hue < (HLSMAX/2))
		return ( n2 );
	if (hue < ((HLSMAX*2)/3))
		return ( n1 +    (((n2-n1)*(((HLSMAX*2)/3)-hue)+(HLSMAX/12))/(HLSMAX/6))
		);
	else
		return ( n1 );
}

DWORD HLStoRGB(WORD hue, WORD lum, WORD sat)
{
	WORD R,G,B;                /* RGB component values */ 
	WORD  Magic1,Magic2;       /* calculated magic numbers (really!) */ 

	if (sat == 0) {            /* achromatic case */ 
		R=G=B=(lum*RGBMAX)/HLSMAX;
		if (hue != UNDEFINED) {
			/* ERROR */ 
		}
	}
	else  {                    /* chromatic case */ 
		/* set up magic numbers */ 
		if (lum <= (HLSMAX/2))
			Magic2 = (lum*(HLSMAX + sat) + (HLSMAX/2))/HLSMAX;
		else
			Magic2 = lum + sat - ((lum*sat) + (HLSMAX/2))/HLSMAX;
		Magic1 = 2*lum-Magic2;

		/* get RGB, change units from HLSMAX to RGBMAX */ 
		R = (HueToRGB(Magic1,Magic2,hue+(HLSMAX/3))*RGBMAX +
			(HLSMAX/2))/HLSMAX;
		G = (HueToRGB(Magic1,Magic2,hue)*RGBMAX + (HLSMAX/2)) / HLSMAX;
		B = (HueToRGB(Magic1,Magic2,hue-(HLSMAX/3))*RGBMAX +
			(HLSMAX/2))/HLSMAX;
	}
	return(RGB(R,G,B));
}