#include "VoxelChunk.h"
#include "VoxelRMCProvider.h"
#include "VoxelWorld.h"
#include "VoxelWorldGenerator.h"
#include "VoxelMesher.h"

UVoxelChunk::UVoxelChunk(UVoxelWorld* InVoxelWorld, FIntVector Pos)
{
	VoxelWorld = InVoxelWorld;
	ChunkPos = Pos;
}

void UVoxelChunk::GenerateChunk()
{
	VoxelWorld->WorldGenerator->GenerateChunk(this);
}

void UVoxelChunk::PolygonizeChunk()
{
	auto MeshData = RMCProvider->GetMeshDataPtr();

	VoxelWorld->GetMesher()->DoMesh(this, MeshData);
}

void UVoxelChunk::SetChunkDirty()
{
}

void UVoxelChunk::UpdateMesh()
{
	RMCProvider->UpdateMesh();
}

void UVoxelChunk::InitMesh(UVoxelRMCProvider* Provider)
{
	RMCProvider = Provider;
}
