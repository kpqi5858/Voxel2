#pragma once

#include "CoreMinimal.h"
#include "RuntimeMeshProvider.h"
#include "VoxelUtilities.h"

#include "VoxelRMCProvider.generated.h"

class UVoxelChunk;
class UVoxelWorld;

UCLASS()
class VOXEL_API UVoxelRMCProvider : public URuntimeMeshProvider
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	UVoxelWorld* VoxelWorld;

	//0 - Current mesh data used, 1 - Temporary mesh data
	TSharedPtr<FVoxelMeshData, ESPMode::ThreadSafe> MeshDataPtrs[2];

	//0 - Current collision data used, 1 - Temporary collision data
	TSharedPtr<FRuntimeMeshCollisionData, ESPMode::ThreadSafe> CollisionDataPtrs[2];

	int LastSections = 0;

	mutable FCriticalSection PropertySyncRoot;

public:
	UVoxelRMCProvider();
	~UVoxelRMCProvider() override;

	void InitVoxel(UVoxelWorld* inVoxelWorld);

	//Provide mesh data into here
	TSharedPtr<FVoxelMeshData, ESPMode::ThreadSafe> GetMeshDataPtr();

	//Provide mesh data into here
	TSharedPtr<FRuntimeMeshCollisionData, ESPMode::ThreadSafe> GetCollisionDataPtr();


	void UpdateMesh();

	void UpdateCollision();

	void Initialize() override;

	bool GetSectionMeshForLOD(int32 LODIndex, int32 SectionId, FRuntimeMeshRenderableMeshData& MeshData) override;

	FBoxSphereBounds GetBounds() override;

	FRuntimeMeshCollisionSettings GetCollisionSettings() override;

	bool GetCollisionMesh(FRuntimeMeshCollisionData& CollisionData) override;

	bool HasCollisionMesh() override;

	bool IsThreadSafe() override
	{
		return true;
	};
};