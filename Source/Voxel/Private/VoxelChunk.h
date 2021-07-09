#pragma once

#include "CoreMinimal.h"
#include "VoxelUtilities.h"

class UVoxelRMCProvider;
class UVoxelWorld;

enum class EChunkState
{
	//Initial state
	Init,
	//Not rendered now
	NotRendered,
	//Rendered now
	Rendered,
	//Will destroyed
	Destroyed
};

class FVoxelBlockStorage
{
private:
	FVoxelBlock Blocks[VOX_ARRAYSIZE];

	FRWLock RWLock;

public:
	FVoxelBlock GetBlock(const int LocalX, const int LocalY, const int LocalZ)
	{
		return Blocks[FVoxelUtilities::GetArrayIndex(LocalX, LocalY, LocalZ)];
	};

	//Set block with color
	void SetBlock(const int LocalX, const int LocalY, const int LocalZ, UVoxelBlockDef* BlockDef, FColor Color)
	{
		int Index = FVoxelUtilities::GetArrayIndex(LocalX, LocalY, LocalZ);

		Blocks[Index].BlockId = BlockDef->TypeId;
		Blocks[Index].Color = Color;
	};

	//Set block with default color
	void SetBlock(const int LocalX, const int LocalY, const int LocalZ, UVoxelBlockDef* BlockDef)
	{
		SetBlock(LocalX, LocalY, LocalZ, BlockDef, BlockDef->DefaultColor);
	};

	void ReadLock()
	{
		RWLock.ReadLock();
	};

	void ReadUnlock()
	{
		RWLock.ReadUnlock();
	};

	void WriteLock()
	{
		RWLock.WriteLock();
	};

	void WriteUnlock()
	{
		RWLock.WriteUnlock();
	};
};

enum class EChunkWorkType
{
	WorldGen, Collision, Mesh
};

struct FChunkWork
{
	FThreadSafeBool bIsWorkOnline = false;
	FThreadSafeBool bIsDone = false;

	FThreadSafeCounter64 CurrentWorkId = -1;

	bool bIsDelaying = false;

	bool IsDone()
	{
		return bIsDone.AtomicSet(false);
	}
};

class UVoxelChunk
{
public:
	FIntVector ChunkPos;

	UVoxelWorld* VoxelWorld = nullptr;

	FThreadSafeCounter RemainingWorks;

protected:
	URuntimeMeshComponent* RMC = nullptr;
	UVoxelRMCProvider* RMCProvider = nullptr;

	bool bIsChunkDirty = false;

	//0 - Not generated, 1 - Generating, 2 - Generated, 3 - Dirty set
	int WorldGenerationPhase = 0;

	FChunkWork WorldGenWork;
	FChunkWork CollisionWork;
	FChunkWork MeshWork;

	FVoxelBlockStorage* BlockStorage = nullptr;

	EChunkState ChunkState = EChunkState::Init;

public:
	UVoxelChunk(UVoxelWorld* InVoxelWorld, FIntVector Pos);
	~UVoxelChunk();

	FVoxelBlockStorage* GetBlockStorage();

	FVoxelBlock GetBlock(const int LocalX, const int LocalY, const int LocalZ)
	{
		BlockStorage->ReadLock();
		FVoxelBlock Block = BlockStorage->GetBlock(LocalX, LocalY, LocalZ);
		BlockStorage->ReadUnlock();
		return Block;;
	};

	//Set block and mark chunk dirty
	void SetBlock(const int LocalX, const int LocalY, const int LocalZ, UVoxelBlockDef* BlockDef, FColor Color);
	void SetBlock(const int LocalX, const int LocalY, const int LocalZ, UVoxelBlockDef* BlockDef)
	{
		SetBlock(LocalX, LocalY, LocalZ, BlockDef, BlockDef->DefaultColor);
	};

	void Tick();

	void DestroyChunk();

	bool IsReadyForFinishDestroy();

	FChunkWork& GetChunkWork(EChunkWorkType Type);

	EChunkState GetChunkState() const
	{
		return ChunkState;
	}

	void GenerateChunk();

	void PolygonizeChunk();

	void BuildCollision();

	//Mesh should be recreated
	void SetChunkDirty();

	void SetAdjacentChunkDirty();

	//Update chunk mesh
	void UpdateMesh();

	void UpdateCollision();

	void InitMesh(URuntimeMeshComponent* Comp);

	inline FIntVector GetMinPos()
	{
		return ChunkPos * VOX_CHUNKSIZE;
	}
};