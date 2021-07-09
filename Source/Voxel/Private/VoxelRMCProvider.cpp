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

TSharedPtr<FVoxelMeshData> UVoxelRMCProvider::GetMeshDataPtr()
{
	if (!MeshDataPtrs[1].IsValid())
	{
		MeshDataPtrs[1] = MakeShareable(new FVoxelMeshData());
	}

	return MeshDataPtrs[1];
}

void UVoxelRMCProvider::UpdateMesh()
{
	check(VoxelWorld);
	check(MeshDataPtrs[1].IsValid());

	MeshDataPtrs[0] = MeshDataPtrs[1];
	MeshDataPtrs[1] = nullptr;

	auto& MeshData = *MeshDataPtrs[0];

	for (int Index = 0; Index < MeshData.Sections.Num(); Index++)
	{
		auto& Section = MeshData.Sections[Index];

		SetupMaterialSlot(Index, FName(FString::Printf(TEXT("VoxelSection-%d"), Index)), Section.Material);

		FRuntimeMeshSectionProperties Properties;
		Properties.bCastsShadow = true;
		Properties.bIsVisible = true;
		Properties.MaterialSlot = Index;
		Properties.bWants32BitIndices = true;
		Properties.UpdateFrequency = ERuntimeMeshUpdateFrequency::Infrequent;

		CreateSection(0, Index, Properties);
	}

	MarkAllLODsDirty();
}

void UVoxelRMCProvider::Initialize()
{
	FRuntimeMeshLODProperties LODProperties;
	LODProperties.ScreenSize = 0.0f;

	ConfigureLODs({ LODProperties });
}

bool UVoxelRMCProvider::GetSectionMeshForLOD(int32 LODIndex, int32 SectionId, FRuntimeMeshRenderableMeshData& MeshData)
{
	if (LODIndex != 0)
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

bool UVoxelRMCProvider::HasCollisionMesh()
{
	return false;
}
