#pragma once

#include "VoxelUtilities.h"

class UVoxelChunk;
class UVoxelWorld;

struct FVoxelMesherParameters
{
	//Should occulde non-visible faces locally?
	bool bOcculdeFace = true;

	//Should occulde non-visible faces in borders, referencing adjacent chunks?
	bool bOcculdeFaceBorder = true;
};

class FVoxelMesher
{
	UVoxelWorld* VoxelWorld;

	float VoxelSize;

public:
	FVoxelMesher(UVoxelWorld* InVoxelWorld);

	void DoMesh(UVoxelChunk* Chunk, TSharedPtr<FVoxelMeshData, ESPMode::ThreadSafe> MeshData, const FVoxelMesherParameters& Params);

	void DoCollision(UVoxelChunk* Chunk, TSharedPtr<FRuntimeMeshCollisionData, ESPMode::ThreadSafe> ColData);
};
