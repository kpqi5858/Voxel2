#include "VoxelRMCProvider.h"
#include "VoxelWorld.h"

UVoxelRMCProvider::UVoxelRMCProvider()
{
}

UVoxelRMCProvider::~UVoxelRMCProvider()
{
}

void UVoxelRMCProvider::InitVoxel(UVoxelWorld* inVoxelWorld)
{
	VoxelWorld = inVoxelWorld;
}

TSharedPtr<FVoxelMeshData, ESPMode::ThreadSafe> UVoxelRMCProvider::GetMeshDataPtr()
{
	if (!MeshDataPtrs[1].IsValid())
	{
		MeshDataPtrs[1] = MakeShareable(new FVoxelMeshData());
	}

	return MeshDataPtrs[1];
}

TSharedPtr<FRuntimeMeshCollisionData, ESPMode::ThreadSafe> UVoxelRMCProvider::GetCollisionDataPtr()
{
	if (!CollisionDataPtrs[1].IsValid())
	{
		CollisionDataPtrs[1] = MakeShareable(new FRuntimeMeshCollisionData());
	}

	return CollisionDataPtrs[1];
}

void UVoxelRMCProvider::UpdateMesh()
{
	check(VoxelWorld);
	check(MeshDataPtrs[1].IsValid());

	MeshDataPtrs[0] = MeshDataPtrs[1];
	MeshDataPtrs[1] = nullptr;

	auto& MeshData = *MeshDataPtrs[0];
	
	if (MeshData.Sections.Num() < LastSections)
	{
		for (int i = MeshData.Sections.Num(); i < LastSections; i++)
		{
			RemoveSection(0, i);
		}
	}

	for (int Index = 0; Index < MeshData.Sections.Num(); Index++)
	{
		auto& Section = MeshData.Sections[Index];

		SetupMaterialSlot(Index, FName(FString::Printf(TEXT("VoxelSection-%d"), Index)), Section.Material);

		FRuntimeMeshSectionProperties Properties;
		Properties.bCastsShadow = false;
		Properties.bIsVisible = true;
		Properties.MaterialSlot = Index;
		Properties.bWants32BitIndices = true;
		Properties.UpdateFrequency = ERuntimeMeshUpdateFrequency::Infrequent;

		CreateSection(0, Index, Properties);
	}

	LastSections = MeshData.Sections.Num();

	MarkAllLODsDirty();
}

void UVoxelRMCProvider::UpdateCollision()
{
	check(VoxelWorld);
	check(CollisionDataPtrs[1].IsValid());

	CollisionDataPtrs[0] = CollisionDataPtrs[1];
	CollisionDataPtrs[1] = nullptr;

	MarkCollisionDirty();
}

void UVoxelRMCProvider::Initialize()
{
	FRuntimeMeshLODProperties LODProperties;
	LODProperties.ScreenSize = 0.0f;

	ConfigureLODs({ LODProperties });
}

bool UVoxelRMCProvider::GetSectionMeshForLOD(int32 LODIndex, int32 SectionId, FRuntimeMeshRenderableMeshData& MeshData)
{
	if (LODIndex != 0 || !MeshDataPtrs[0]->Sections.IsValidIndex(SectionId))
	{
		return false;
	}

	MeshData = MeshDataPtrs[0]->Sections[SectionId].MeshData;

	return true;
}

FBoxSphereBounds UVoxelRMCProvider::GetBounds()
{
	FBox Box = FBox(FVector(0), FVector(VOX_CHUNKSIZE * VoxelWorld->VoxelSize));
	return FBoxSphereBounds(Box);
}

FRuntimeMeshCollisionSettings UVoxelRMCProvider::GetCollisionSettings()
{
	FRuntimeMeshCollisionSettings Settings;

	Settings.bUseAsyncCooking = true;
	Settings.bUseComplexAsSimple = true;

	return Settings;
}

bool UVoxelRMCProvider::GetCollisionMesh(FRuntimeMeshCollisionData& CollisionData)
{
	check(CollisionDataPtrs[0].IsValid());

	CollisionData = *CollisionDataPtrs[0];

	return true;
}

bool UVoxelRMCProvider::HasCollisionMesh()
{
	return CollisionDataPtrs[0]->Vertices.Num() != 0;
}
