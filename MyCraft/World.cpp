#ifndef APPLE
#include <Windows.h>
#endif

#include "World.h"
#include "asmlibran.h"

#include "Debug.h"

chunk_list *World::GetRenderChunks(float3 pos, float3 dir) {
	world_map.MarkUnneededChunks(pos, dir);
	world_map.DiscardUnneededChunks();
	world_map.LoadNeededChunks(pos, dir);
	return world_map.GetChunkList();
}

void World::UpdateWorld() {
	map_chunk *mapchk = world_map.GetChunk(int3(0, 0, 0));
	if (mapchk == 0)
		return;

	Block *blocks = mapchk->blocks;

	for_xyz(i, j, k) {
		if (k == 0) continue;

		if (blocks[_1D(i, j, k)].type != Block::LAVA)
			continue;

		if (blocks[_1D(i, j, k-1)].type == Block::NUL) {
			unsigned short state = 0;
			state |= blocks[_1D(i, j, k-1)].setType(Block::LAVA);
			mapchk->modified |= state;
		}
		else {
			if (blocks[_1D(i, j, k-1)].type != Block::LAVA && blocks[_1D(i, j, k)].modified == 0) {
				unsigned short state = 0;
				state |= blocks[_1D(CLAMPW(i+1), j, k)].setType(Block::LAVA);
				state |= blocks[_1D(CLAMPW(i-1), j, k)].setType(Block::LAVA);
				state |= blocks[_1D(i, CLAMPL(j+1), k)].setType(Block::LAVA);
				state |= blocks[_1D(i, CLAMPL(j-1), k)].setType(Block::LAVA);

				mapchk->modified |= state;
			}
		}
	} end_xyz()
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

