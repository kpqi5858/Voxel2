#pragma once

#include "CoreMinimal.h"
#include "VoxelUtilities.h"
#include "RuntimeMeshComponent.h"

#include "VoxelWorld.generated.h"

class UVoxelWorldGenerator;
class UVoxelChunk;
class FVoxelMesher;

UCLASS()
class UVoxelWorld : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	float VoxelSize = 100.0f;

	UPROPERTY()
	UVoxelWorldGenerator* WorldGenerator;

protected:
	TMap<FIntVector, UVoxelChunk*> Chunks;
	TArray<UVoxelChunk*> ChunksArray;

	UPROPERTY()
	AActor* MeshActor;

	UPROPERTY()
	TArray<URuntimeMeshComponent*> MeshComponents;

	UPROPERTY()
	UWorld* World;

	FVoxelMesher* Mesher = nullptr;

	bool bIsWorldCreated = false;

	bool bActorPerMesh = false;

public:
	UVoxelWorld();
	~UVoxelWorld();

	void Test();

	void CreateWorld(UWorld* InWorld);
	void DestroyWorld();

	UVoxelChunk* NewChunk(const FIntVector& ChunkPos);
	UVoxelChunk* GetChunk(const FIntVector& ChunkPos, const bool bCreateIfNotExists = false);

	URuntimeMeshComponent* GetFreeMesh();
	void InitMesh(URuntimeMeshComponent* Comp, UVoxelChunk* Chunk);

	FVoxelMesher* GetMesher();
};