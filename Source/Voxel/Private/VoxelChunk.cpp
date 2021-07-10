#include "VoxelChunk.h"
#include "VoxelRMCProvider.h"
#include "VoxelWorld.h"
#include "VoxelWorldGenerator.h"
#include "VoxelMesher.h"

UVoxelChunk::UVoxelChunk(UVoxelWorld* InVoxelWorld, FIntVector Pos)
{
	VoxelWorld = InVoxelWorld;
	ChunkPos = Pos;

	BlockStorage = new FVoxelBlockStorage();
}

UVoxelChunk::~UVoxelChunk()
{
	delete BlockStorage;
}

FVoxelBlockStorage* UVoxelChunk::GetBlockStorage()
{
	return BlockStorage;
}

void UVoxelChunk::SetBlock(int LocalX, int LocalY, int LocalZ, UVoxelBlockDef* BlockDef, FColor Color)
{
	BlockStorage->WriteLock();
	BlockStorage->SetBlock(LocalX, LocalY, LocalZ, BlockDef, Color);
	BlockStorage->WriteUnlock();
	SetChunkDirty();
}

void UVoxelChunk::Tick()
{
	if (VoxelWorld->ShouldBeDestroyed(ChunkPos) && VoxelWorld->TryUpdate())
	{
		DestroyChunk();

		ChunkState = EChunkState::Destroyed;
	}

	if (ChunkState == EChunkState::Destroyed)
	{
		return;
	}

	if (ChunkState == EChunkState::Init || ChunkState == EChunkState::NotRendered)
	{
		if (VoxelWorld->ShouldBeRendered(ChunkPos))
		{
			ChunkState = EChunkState::Rendered;
		}
		else
		{
			ChunkState = EChunkState::NotRendered;
		}
	}

	if (ChunkState == EChunkState::NotRendered)
	{
		if (WorldGenerationPhase == 0)
		{
			WorldGenerationPhase = 1;

			VoxelWorld->QueueChunkWork(this, EChunkWorkType::WorldGen);
		}
	}

	if (ChunkState == EChunkState::Rendered)
	{
		if (RMCProvider == nullptr)
		{
			VoxelWorld->SetupMesh(this);
		}

		if (WorldGenerationPhase == 0)
		{
			WorldGenerationPhase = 1;

			VoxelWorld->QueueChunkWork(this, EChunkWorkType::WorldGen);
		}
		
		if (WorldGenerationPhase == 2)
		{
			SetChunkDirty();
			SetAdjacentChunkDirty();

			WorldGenerationPhase = 3;
		}

		if (bIsChunkDirty && RMCProvider)
		{
			VoxelWorld->QueueChunkWork(this, EChunkWorkType::Collision);
			VoxelWorld->QueueChunkWork(this, EChunkWorkType::Mesh);

			bIsChunkDirty = false;
		}
	}

	if (WorldGenWork.IsDone())
	{
		WorldGenerationPhase = 2;
	}
	if (CollisionWork.IsDone() || CollisionWork.bIsDelaying)
	{
		if (VoxelWorld->TryUpdate())
		{
			UpdateCollision();
			CollisionWork.bIsDelaying = false;
		}
		else
		{
			CollisionWork.bIsDelaying = true;
		}
	}
	if (MeshWork.IsDone() || MeshWork.bIsDelaying)
	{
		if (VoxelWorld->TryUpdate())
		{
			UpdateMesh();
			MeshWork.bIsDelaying = false;
		}
		else
		{
			MeshWork.bIsDelaying = true;
		}
	}
}

void UVoxelChunk::DestroyChunk()
{
	if (RMC)
	{
		VoxelWorld->ReleaseMesh(RMC);
	}

	VoxelWorld->OnChunkDestroyed(this);
}

bool UVoxelChunk::IsReadyForFinishDestroy()
{
	return RemainingWorks.GetValue() == 0;
}

void UVoxelChunk::GenerateChunk()
{
	check(WorldGenerationPhase == 1);

	BlockStorage->WriteLock();

	VoxelWorld->WorldGenerator->GenerateChunk(this);

	BlockStorage->WriteUnlock();
}

void UVoxelChunk::PolygonizeChunk()
{
	BlockStorage->ReadLock();

	auto MeshData = RMCProvider->GetMeshDataPtr();

	FVoxelMesherParameters Params;
	
	VoxelWorld->GetMesher()->DoMesh(this, MeshData, Params);

	BlockStorage->ReadUnlock();
}

void UVoxelChunk::BuildCollision()
{
	BlockStorage->ReadLock();

	auto ColData = RMCProvider->GetCollisionDataPtr();

	VoxelWorld->GetMesher()->DoCollision(this, ColData);

	BlockStorage->ReadUnlock();
}

FChunkWork& UVoxelChunk::GetChunkWork(EChunkWorkType Type)
{
	switch (Type)
	{
	case EChunkWorkType::WorldGen:
		return WorldGenWork;
	case EChunkWorkType::Collision:
		return CollisionWork;
	case EChunkWorkType::Mesh:
		return MeshWork;
	}

	check(false);
	return WorldGenWork;
}

void UVoxelChunk::SetChunkDirty()
{
	bIsChunkDirty = true;
}

void UVoxelChunk::SetAdjacentChunkDirty()
{
	for (int FaceNum = 0; FaceNum < 6; FaceNum++)
	{
		const EBlockFace Face = static_cast<EBlockFace>(FaceNum);

		UVoxelChunk* AdjacentChunk = VoxelWorld->GetChunk(ChunkPos + FVoxelUtilities::GetFaceOffset(Face));
		if (AdjacentChunk)
		{
			AdjacentChunk->SetChunkDirty();
		}
	}
}

void UVoxelChunk::UpdateMesh()
{
	RMCProvider->UpdateMesh();
}

void UVoxelChunk::UpdateCollision()
{
	RMCProvider->UpdateCollision();
}

void UVoxelChunk::InitMesh(URuntimeMeshComponent* Comp)
{
	RMCProvider = CastChecked<UVoxelRMCProvider>(Comp->GetProvider());
	RMC = Comp;
}

