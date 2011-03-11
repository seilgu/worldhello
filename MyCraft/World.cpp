#include "World.h"
#include "asmlibran.h"
#include <Windows.h>

#include "Debug.h"

chunk_list *World::GetRenderChunks(float3 pos, float3 dir) {
	world_map.LoadNeededChunks(pos, dir);
	world_map.MarkUnneededChunks(pos, dir);
	world_map.DiscardUnneededChunks();
	return world_map.GetChunkList();
}

void World::UpdateWorld() {
}

int World::LoadWorld() {
	return 0;
}

void World::CalcExposure() {
	for (int i=0; i<WORLD_SIZE; i++) {
		for (int j=0; j<WORLD_SIZE; j++) {
			for (int k=0; k<WORLD_SIZE; k++) {

			}
		}
	}
}