#pragma once

#include "CoreMinimal.h"
#include "Voxel/Voxel.h"
#include "VoxelBlockDef.h"
#include "VoxelBlockRegistry.h"
#include "RuntimeMeshComponent.h"

template<typename T>
inline UVoxelBlockDef* GetVoxelBlock(T Name)
{
	return FBlockRegistry::GetInstance_Ptr()->GetBlock(Name);
}

enum class EBlockFace : uint8
{
	FRONT, BACK, LEFT, RIGHT, TOP, BOTTOM
};

struct FVoxelBlock
{
	uint32 BlockId = 0;
	FColor Color = FColor::White;

	UVoxelBlockDef* GetBlock() const
	{
		return GetVoxelBlock(BlockId);
	}
};


struct FVoxelMeshSection
{
	FRuntimeMeshRenderableMeshData MeshData = FRuntimeMeshRenderableMeshData(false, false, 1, true);

	UMaterialInterface* Material = nullptr;
	UVoxelBlockDef* BlockDef = nullptr;
};

struct FVoxelMeshData
{
	TArray<FVoxelMeshSection> Sections;

	TMap<UVoxelBlockDef*, int> IndexCache;

	FVoxelMeshSection& GetSectionFor(UVoxelBlockDef* BlockDef)
	{
		check(BlockDef->ShouldBePolygonized());

		int* FindCache = IndexCache.Find(BlockDef);

		if (FindCache)
		{
			return Sections[*FindCache];
		}

		int ResultIndex = -1;

		for (int i = 0; i < Sections.Num(); i++)
		{
			if ((BlockDef->bSeparateMeshSections && Sections[i].BlockDef == BlockDef)
				|| (!BlockDef->bSeparateMeshSections && Sections[i].Material == BlockDef->Material))
			{
				ResultIndex = i;
				break;
			}
		}

		if (ResultIndex == -1)
		{
			ResultIndex = Sections.Num();

			FVoxelMeshSection NewSection;

			NewSection.Material = BlockDef->Material;
			NewSection.BlockDef = BlockDef;

			Sections.Emplace(NewSection);
		}

		IndexCache.Add(BlockDef, ResultIndex);

		return Sections[ResultIndex];
	}
};

class FVoxelUtilities
{
public:
	static inline bool IsInLocalPosition(const int X, const int Y, const int Z)
	{
		return X >= 0 && X < VOX_CHUNKSIZE
			&& Y >= 0 && Y < VOX_CHUNKSIZE
			&& Z >= 0 && Z < VOX_CHUNKSIZE;
	}

	static inline int GetArrayIndex(const int X, const int Y, const int Z)
	{
		check(IsInLocalPosition(X, Y, Z));

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

	static inline FIntVector VoxelPosToChunkPos(const FIntVector& VoxelPos)
	{
		//-1 / 16 needs to be -1, not zero
		auto CustomDiv = [](const int Val, const int Div)
		{
			const int Result = Val / Div;
			return Div * Result == Val ? Result : Result - ((Val < 0) ^ (Div < 0));
		};

		return FIntVector(CustomDiv(VoxelPos.X, VOX_CHUNKSIZE)
			, CustomDiv(VoxelPos.Y, VOX_CHUNKSIZE)
			, CustomDiv(VoxelPos.Z, VOX_CHUNKSIZE));
	}

	static inline FIntVector VoxelPosToLocalPos(const FIntVector& VoxelPos)
	{
		auto CustomModulo = [](const int Val, const int Div) 
		{
			const int Result = Val % Div; return Result < 0 ? Result + Div : Result; 
		}
		;
		return FIntVector(CustomModulo(VoxelPos.X, VOX_CHUNKSIZE)
			, CustomModulo(VoxelPos.Y, VOX_CHUNKSIZE)
			, CustomModulo(VoxelPos.Z, VOX_CHUNKSIZE));
	}

	static inline FIntVector GetFaceOffset(const EBlockFace& Face)
	{
		switch (Face)
		{
		case EBlockFace::FRONT:
			return FIntVector(1, 0, 0);
		case EBlockFace::BACK:
			return FIntVector(-1, 0, 0);
		case EBlockFace::LEFT:
			return FIntVector(0, -1, 0);
		case EBlockFace::RIGHT:
			return FIntVector(0, 1, 0);
		case EBlockFace::TOP:
			return FIntVector(0, 0, 1);
		case EBlockFace::BOTTOM:
			return FIntVector(0, 0, -1);
		default:
			check(false);
			return FIntVector(0);
		}
	};

	static inline EBlockFace GetOppositeFace(const EBlockFace& Face)
	{
		switch (Face)
		{
		case EBlockFace::FRONT:
			return EBlockFace::BACK;
		case EBlockFace::BACK:
			return EBlockFace::FRONT;
		case EBlockFace::LEFT:
			return EBlockFace::RIGHT;
		case EBlockFace::RIGHT:
			return EBlockFace::LEFT;
		case EBlockFace::TOP:
			return EBlockFace::BOTTOM;
		case EBlockFace::BOTTOM:
			return EBlockFace::TOP;
		default:
			check(false);
			return EBlockFace::FRONT;
		}
	}
};

