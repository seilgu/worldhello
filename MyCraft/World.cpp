#ifndef APPLE
#include <Windows.h>
#endif

#include "World.h"
#include "asmlibran.h"


chunk_list *World::GetRenderChunks(float3 pos, float3 dir) {
	world_map.MarkUnneededChunks(pos, dir);
	world_map.DiscardUnneededChunks();
	world_map.LoadNeededChunks(pos, dir);
	return world_map.GetChunkList();
}

void World::AddBlock(int3 chkId, int3 offset, int type) {
	Block *block = world_map.GetBlock(chkId, offset, 1);

	if (block != 0) {
		block->setType(type);
	}
}

void World::RemoveBlock(int3 chkId, int3 offset) {
	Block *block = world_map.GetBlock(chkId, offset, 1);

	if (block != 0) {
		block->setType(Block::NUL);
	}
}

void World::UpdateWorld() {

	chunk_list *chunks = world_map.GetChunkList();

	chunk_list::iterator chk_it;
	for (int w=0; w<9; w++) {
		chk_it = chunks->find(int3(w/3, w%3, 0));
		if (chk_it == chunks->end()) continue;

		map_chunk *mapchk = chk_it->second;

		if (mapchk == 0 || mapchk->failed == 1 || mapchk->loaded == 0)
			continue;

		int3 id = chk_it->first;

		Block *blocks = mapchk->blocks;

		for_xyz(i, j, k) {
			if (blocks[_1D(i, j, k)].type == Block::LAVA) {
				Block *tmp = world_map.GetBlock(id, int3(i, j, k-1), 0);
				if (tmp != 0 && (tmp->type == Block::NUL || tmp->type == Block::LAVA)) {
					tmp->data = Block::LAVA;
				}
				else {
					int x=0, y=0;
					switch (MersenneIRandom(0, 3)) {
					case 0: x=1; break;
					case 1: x=-1; break;
					case 2: y=1; break;
					case 3: y=-1; break;
					}

					if ( (tmp = world_map.GetBlock(id, int3(i+x, j+y, k), 0)) != 0 && tmp->type == Block::NUL) {
						tmp->data = Block::LAVA;
					}
				}
			}
			if (blocks[_1D(i, j, k)].data == Block::LAVA) {
				blocks[_1D(i, j, k)].data = 0;
				blocks[_1D(i, j, k)].setType(Block::LAVA);
				mapchk->modified = 1;
			}
			
		} end_xyz()
	}
}

Block *World::GetBlocks(int3 id) {
	map_chunk *mapchk = world_map.GetChunk(id);
	if (mapchk != 0)
		return mapchk->blocks;

	return 0;
}

int World::LoadWorld() {
	return 0;
}

