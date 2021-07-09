#pragma once

#include "CoreMinimal.h"
#include "Voxel/Voxel.h"
#include "RuntimeMeshComponent.h"

enum class EBlockFace : uint8
{
	FRONT, BACK, LEFT, RIGHT, TOP, BOTTOM
};

struct FVoxelMeshSection
{
	FRuntimeMeshRenderableMeshData MeshData;
	UMaterialInterface* Material = nullptr;
};

struct FVoxelMeshData
{
	TArray<FVoxelMeshSection> Sections;
};

class FVoxelUtilities
{
public:
	static inline int GetArrayIndex(const int X, const int Y, const int Z)
	{
		check(X >= 0 && X < VOX_CHUNKSIZE
			&& Y >= 0 && Y < VOX_CHUNKSIZE
			&& Z >= 0 && Z < VOX_CHUNKSIZE);

		return X
			+ (Y * VOX_CHUNKSIZE)
			+ (Z * VOX_CHUNKSIZE * VOX_CHUNKSIZE);
	};

	static inline int GetArrayIndex(const FIntVector& InVec)
	{
		return GetArrayIndex(InVec.X, InVec.Y, InVec.Z);
	}

	static inline FIntVector FromArrayIndex(int Index)
	{
		check(Index >= 0 && Index < VOX_ARRAYSIZE);

		const int Z = Index / (VOX_CHUNKSIZE * VOX_CHUNKSIZE);
		Index %= VOX_CHUNKSIZE * VOX_CHUNKSIZE;

		const int Y = Index / (VOX_CHUNKSIZE);
		Index %= VOX_CHUNKSIZE;

		const int X = Index;

		return FIntVector(X, Y, Z);
	}
};

struct FVoxelBlock
{
	uint32 BlockId = 0;
	FColor Color = FColor::White;
};
