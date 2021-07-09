#pragma once

#include "CoreMinimal.h"
#include "VoxelUtilities.h"

class UVoxelRMCProvider;
class UVoxelWorld;

class UVoxelChunk
{
private:
	FIntVector ChunkPos;

	UVoxelWorld* VoxelWorld = nullptr;

	UVoxelRMCProvider* RMCProvider = nullptr;

	bool IsChunkDirty = false;

public:
	FVoxelBlock Blocks[VOX_ARRAYSIZE];

public:
	UVoxelChunk(UVoxelWorld* InVoxelWorld, FIntVector Pos);

	void GenerateChunk();

	void PolygonizeChunk();

	//Mesh should be recreated
	void SetChunkDirty();

	//Update chunk mesh
	void UpdateMesh();

	void InitMesh(UVoxelRMCProvider* Provider);

	inline FIntVector GetMinPos()
	{
		return ChunkPos * VOX_CHUNKSIZE;
	}
};