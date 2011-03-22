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

