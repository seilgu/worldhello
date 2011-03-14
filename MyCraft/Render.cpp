#include "Render.h"
#include "World.h"

extern float zNear;
extern int _width, _height;

inline float2 get_texture_coord(int tx) {
	float2 coord;
	coord.y = (tx/TX_ROW)/16.0f;
	coord.x = (tx%TX_ROW)/16.0f;

	return coord;
}

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
	if (chk == 0)
		return;

	glDeleteBuffersARB(1, &chk->vbo);

	delete chk;
	chk = 0;
}

void Render::CalculateVisible2(int3 id) {
	if (s_World == 0) {
		MessageBox(0, "s_World NULL", "HAHAH", 0);
		return;
	}

	chunk_list chunks = s_World->world_map.m_chunks;
	chunk_list::iterator it = chunks.find(id);
	if (it == chunks.end())
		return;

	map_chunk *map_chk = (*it).second;
	if (map_chk == 0 || map_chk->loaded == 0 || map_chk->failed == 1 || map_chk->unneeded == 1) {
		return;
	}

	Block *blocks = map_chk->blocks;
	
	for (int i=0; i<CHUNK_W*CHUNK_L*CHUNK_H; i++) {
		blocks[i].hidden = blocks[i].opaque;
	}

	for (int i=0; i<CHUNK_W; i++) {
		for (int j=0; j<CHUNK_L; j++) {
			for (int k=0; k<CHUNK_H; k++) {
				if (blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].type == Block::NUL) continue;
				blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden &= blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + CLAMPW(i+1)].opaque;
				blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden &= blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + CLAMPW(i-1)].opaque;
				blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden &= blocks[k*(CHUNK_W*CHUNK_L) + CLAMPL(j+1)*(CHUNK_W) + i].opaque;
				blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden &= blocks[k*(CHUNK_W*CHUNK_L) + CLAMPL(j-1)*(CHUNK_W) + i].opaque;
				blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden &= blocks[CLAMPH(k+1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque;
				blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden &= blocks[CLAMPH(k-1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque;
			}
		}
	}

	for (int w=0; w<6; w++) {
		CheckChunkSide(id, w);
	}
}

void Render::CheckChunkSide(int3 id, int dir) {
	chunk_list chunks = s_World->world_map.m_chunks;

	chunk_list::iterator it = chunks.find(id);
	if (it == chunks.end())
		return;

	map_chunk *map_chk = (*it).second;
	Block *blocks = map_chk->blocks;

	map_chunk *map_side;
	chunk_list::iterator tmp;
	switch (dir) {
	case PX : tmp = chunks.find(int3(id.x+1, id.y, id.z)); break;
	case NX : tmp = chunks.find(int3(id.x-1, id.y, id.z)); break;
	case PY : tmp = chunks.find(int3(id.x, id.y+1, id.z)); break;
	case NY : tmp = chunks.find(int3(id.x, id.y-1, id.z)); break;
	case PZ : tmp = chunks.find(int3(id.x, id.y, id.z+1)); break;
	case NZ : tmp = chunks.find(int3(id.x, id.y, id.z-1)); break;
	}

	map_side = (tmp == chunks.end() ? 0 : (*tmp).second);

	if (map_side == 0 || map_side->loaded == 0 || map_side->failed == 1 || map_side->unneeded == 1) {
		// edge of map
		switch (dir) {
		case PX : 
			for (int j=0; j<CHUNK_L; j++) {
				for (int k=0; k<CHUNK_H; k++) {
					blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + (CHUNK_W-1)].hidden &= 1;
				}
			}
			break;
		case NX : 
			for (int j=0; j<CHUNK_L; j++) {
				for (int k=0; k<CHUNK_H; k++) {
					blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + 0].hidden &= 1;
				}
			}
			break;
		case PY : 
			for (int i=0; i<CHUNK_W; i++) {
				for (int k=0; k<CHUNK_H; k++) {
					blocks[k*(CHUNK_W*CHUNK_L) + (CHUNK_L-1)*(CHUNK_W) + i].hidden &= 1;
				}
			}
			break;
		case NY : 
			for (int i=0; i<CHUNK_W; i++) {
				for (int k=0; k<CHUNK_H; k++) {
					blocks[k*(CHUNK_W*CHUNK_L) + 0*(CHUNK_W) + i].hidden &= 1;
				}
			}
			break;
		case PZ : 
			for (int i=0; i<CHUNK_W; i++) {
				for (int j=0; j<CHUNK_L; j++) {
					blocks[(CHUNK_H-1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden &= 1;
				}
			}
			break;
		case NZ : 
			for (int i=0; i<CHUNK_W; i++) {
				for (int j=0; j<CHUNK_L; j++) {
					blocks[0*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden &= 1;
				}
			}
			break;
		}
		return;
	}

	// not edge
	Block *aux = map_side->blocks;
	switch (dir) {
	case PX : 
		for (int j=0; j<CHUNK_L; j++) {
			for (int k=0; k<CHUNK_H; k++) {
				blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + (CHUNK_W-1)].hidden &= aux[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + 0].opaque;
				aux[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + 0].hidden &= blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + (CHUNK_W-1)].opaque;
			}
		}
		break;
	case NX : 
		for (int j=0; j<CHUNK_L; j++) {
			for (int k=0; k<CHUNK_H; k++) {
				blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + 0].hidden &= aux[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + (CHUNK_W-1)].opaque;
				aux[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + (CHUNK_W-1)].hidden &= blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + 0].opaque;
			}
		}
		break;
	case PY : 
		for (int i=0; i<CHUNK_W; i++) {
			for (int k=0; k<CHUNK_H; k++) {
				blocks[k*(CHUNK_W*CHUNK_L) + (CHUNK_L-1)*(CHUNK_W) + i].hidden &= aux[k*(CHUNK_W*CHUNK_L) + 0*(CHUNK_W) + i].opaque;
				aux[k*(CHUNK_W*CHUNK_L) + 0*(CHUNK_W) + i].hidden &= blocks[k*(CHUNK_W*CHUNK_L) + (CHUNK_L-1)*(CHUNK_W) + i].opaque;
			}
		}
		break;
	case NY : 
		for (int i=0; i<CHUNK_W; i++) {
			for (int k=0; k<CHUNK_H; k++) {
				blocks[k*(CHUNK_W*CHUNK_L) + 0*(CHUNK_W) + i].hidden &= aux[k*(CHUNK_W*CHUNK_L) + (CHUNK_L-1)*(CHUNK_W) + i].opaque;
				aux[k*(CHUNK_W*CHUNK_L) + (CHUNK_L-1)*(CHUNK_W) + i].hidden &= blocks[k*(CHUNK_W*CHUNK_L) + 0*(CHUNK_W) + i].opaque;
			}
		}
		break;
	case PZ : 
		for (int i=0; i<CHUNK_W; i++) {
			for (int j=0; j<CHUNK_L; j++) {
				blocks[(CHUNK_H-1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden &= aux[0*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque;
				aux[0*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden &= blocks[(CHUNK_H-1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque;
			}
		}
		break;
	case NZ : 
		for (int i=0; i<CHUNK_W; i++) {
			for (int j=0; j<CHUNK_L; j++) {
				blocks[0*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden &= aux[(CHUNK_H-1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque;
				aux[(CHUNK_H-1)*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden &= blocks[0*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].opaque;
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
	if (urgent == 0) { // multithreading
		render_pair pair(ren_chk, map_chk);
		m_Thread->PushJobs(pair);
		return;
	}
	else {
		m_Thread->threadLoadChunk( std::pair<render_chunk *, map_chunk *>(ren_chk, map_chk), m_Thread);
	}
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

		if (r_chunks.find(id) == r_chunks.end()) { // not found, need to create it
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
		float3 p(id.x, id.y, id.z), n(id.x+1, id.y+1, id.z+1);
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
		if (normal.x > 0) { p.x++; n.x--; }
		if (normal.y > 0) { p.y++; n.y--; }
		if (normal.z > 0) { p.z++; n.z--; }

		p.x *= CHUNK_W; p.y *= CHUNK_L; p.z *= CHUNK_H;
		n.x *= CHUNK_W; n.y *= CHUNK_L; n.z *= CHUNK_H;
		p = BLOCK_LEN*p - pos; n = BLOCK_LEN*n - pos;

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
void Render::RenderChunk0(map_chunk *chk, float3 pos, float3 dir) {
	float3 rel;
	
	int type;
	for (int i=0; i<CHUNK_W; i++) {
		for (int j=0; j<CHUNK_L; j++) {
			for (int k=0; k<CHUNK_H; k++) {
				type = (chk->blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i]).type;
				
				if (type == Block::NUL) continue;
				if (chk->blocks[k*(CHUNK_W*CHUNK_L) + j*(CHUNK_W) + i].hidden == 1)
					continue;

				// frustum culling

				int3 id = int3(i, j, k);

				// right plane normal
				normalize(dir);
				float3 rhs = float3(dir.y, -dir.x, 0);
				normalize(rhs);
				float r = sqrt(dir.x*dir.x + dir.y*dir.y);
				float3 upside = float3(-dir.z*dir.x/r, -dir.z*dir.y/r, r);

				float3 normal;
				for (int w=0; w<4; w++) {
					float3 n(id.x+1, id.y+1, id.z+1);
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

					n = n + float3(CHUNK_W*chk->id.x, CHUNK_L*chk->id.y, CHUNK_H*chk->id.z);
					
					n = BLOCK_LEN*n - pos;

					if (dot_prod(normal, n) > 0)
						continue;
				}

				rel.x = (chk->id.x*CHUNK_W + i)*BLOCK_LEN - pos.x;
				rel.y = (chk->id.y*CHUNK_L + j)*BLOCK_LEN - pos.y;
				rel.z = (chk->id.z*CHUNK_H + k)*BLOCK_LEN - pos.z;
				
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


void Render::RenderChunkThread::threadLoadChunk(render_pair pair, Render::RenderChunkThread *self) {
	render_chunk *ren_chk = pair.first;
	map_chunk *map_chk = pair.second;

	//self->render->LoadChunk(ren_chk, map_chk, 1);

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
	self->render->CalculateVisible2(map_chk->id);

	int size = 0;
	for (int i=0; i<CHUNK_W*CHUNK_L*CHUNK_H; i++) {
		if (map_chk->blocks[i].type == Block::NUL)
			continue;
		if (map_chk->blocks[i].hidden == 0)
			size++;
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

				if (type == Block::NUL)
					continue;

				for (int w=0; w<6; w++) { // 6 faces
					float2 coord;
					self->render->GetTextureCoordinates(type, w, coord);
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
		MessageBox(0, "glGenBuffer ERROR!!", "aha", 0);
	}

	delete[] vertices;
	vertices = 0;

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	ren_chk->loaded = 1;
}