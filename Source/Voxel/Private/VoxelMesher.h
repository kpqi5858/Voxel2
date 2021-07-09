#pragma once

#include "VoxelUtilities.h"

class UVoxelChunk;
class UVoxelWorld;

class FVoxelMesher
{
	UVoxelWorld* VoxelWorld;

	float VoxelSize;

public:
	FVoxelMesher(UVoxelWorld* InVoxelWorld);

	void DoMesh(UVoxelChunk* Chunk, TSharedPtr<FVoxelMeshData> MeshData);
};
