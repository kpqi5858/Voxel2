#pragma once

#include "CoreMinimal.h"
#include "VoxelUtilities.h"
#include "RuntimeMeshComponent.h"
#include "Misc/QueuedThreadPool.h"

#include "VoxelWorld.generated.h"

class UVoxelWorldGenerator;
class UVoxelChunk;
class FVoxelMesher;
enum class EChunkWorkType;

class FQueuedChunkWork : public IQueuedWork
{
	UVoxelChunk* Chunk;
	EChunkWorkType WorkType;
	uint64 WorkId;

public:
	FQueuedChunkWork(UVoxelChunk* InChunk, EChunkWorkType InWorkType, uint64 InWorkId)
		: Chunk(InChunk), WorkType(InWorkType), WorkId(InWorkId)
	{ };

	void DoThreadedWork() override;
	void Abandon() override;
};

UCLASS()
class UVoxelWorld : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	float VoxelSize = 100.0f;

	UPROPERTY()
	UVoxelWorldGenerator* WorldGenerator;

	UPROPERTY()
	float RenderDistance = 4.0f;

	UPROPERTY()
	float DestroyExtent = 2.0f;

	UPROPERTY()
	int MaxUpdatesPerTick = 15;

	FThreadSafeCounter JobsRemaining;

private:
	TSharedPtr<FBlockRegistryInstance> BlockRegistryPtr;

protected:
	TMap<FIntVector, UVoxelChunk*> Chunks;
	TArray<UVoxelChunk*> ChunksArray;

	TArray<UVoxelChunk*> ToDestroy;

	FRWLock ChunksLock;

	UPROPERTY()
	AActor* MeshActor;

	UPROPERTY()
	TArray<URuntimeMeshComponent*> MeshComponents;

	UPROPERTY()
	TArray<URuntimeMeshComponent*> FreeMeshComponents;

	UPROPERTY()
	UWorld* World;

	UPROPERTY()
	TArray<AActor*> Trackers;

	FVoxelMesher* Mesher = nullptr;

	FQueuedThreadPool* ThreadPool;

	int UpdatesThisTick = 0;

	bool bIsWorldCreated = false;

	bool bActorPerMesh = false;

	bool bAsync = true;

public:
	UVoxelWorld();
	~UVoxelWorld();

	void Test();

	void CreateWorld(UWorld* InWorld);
	void DestroyWorld();

	void Tick();

	void RegisterTracker(AActor* Actor);

	void QueueChunkWork(UVoxelChunk* Chunk, EChunkWorkType Type);

	UVoxelChunk* NewChunk(const FIntVector& ChunkPos);
	UVoxelChunk* GetChunk(const FIntVector& ChunkPos, const bool bCreateIfNotExists = false);

	void OnChunkDestroyed(UVoxelChunk* Chunk);
	void FinalizeDestroyChunk(UVoxelChunk* Chunk);

	URuntimeMeshComponent* GetFreeMesh();
	void ReleaseMesh(URuntimeMeshComponent* Comp);

	void SetupMesh(UVoxelChunk* Chunk);

	FVoxelMesher* GetMesher();

	float GetMinDistanceToTrackers(const FIntVector& ChunkPos);

	bool ShouldBeRendered(const FIntVector& Chunk);
	bool ShouldBeDestroyed(const FIntVector& Chunk);

	bool TryUpdate()
	{
		return UpdatesThisTick++ < MaxUpdatesPerTick;
	}

	inline FIntVector ToVoxelPos(const FVector& Val)
	{
		return FIntVector(FMath::Floor(Val.X / VoxelSize), FMath::Floor(Val.Y / VoxelSize), FMath::Floor(Val.Z / VoxelSize));
	}
};