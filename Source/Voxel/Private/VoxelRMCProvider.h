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

public:
	UPROPERTY()
	UVoxelWorld* VoxelWorld;

	//0 - Current mesh data used, 1 - Temporary mesh data
	TSharedPtr<FVoxelMeshData> MeshDataPtrs[2];

public:
	UVoxelRMCProvider();
	~UVoxelRMCProvider() override;

	void InitVoxel(UVoxelWorld* inVoxelWorld);

	//Provide mesh data into here
	TSharedPtr<FVoxelMeshData> GetMeshDataPtr();

	void UpdateMesh();

	void Initialize() override;

	bool GetSectionMeshForLOD(int32 LODIndex, int32 SectionId, FRuntimeMeshRenderableMeshData& MeshData) override;

	FBoxSphereBounds GetBounds() override;

	bool IsThreadSafe() override
	{
		//Idk how it is threaded so return false for this time
		return false;
	};

	bool HasCollisionMesh() override;
};