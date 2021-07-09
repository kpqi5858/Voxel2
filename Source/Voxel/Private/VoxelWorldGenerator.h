#pragma once

#include "CoreMinimal.h"
#include "VoxelUtilities.h"
#include "VoxelChunk.h"

#include "VoxelWorldGenerator.generated.h"

UCLASS(Abstract)
class UVoxelWorldGenerator : public UObject
{
	GENERATED_BODY()

public:
	UVoxelWorldGenerator() { };

	virtual void GenerateChunk(UVoxelChunk* Chunk)
	{
		unimplemented();

		const FIntVector ChunkPos = Chunk->GetMinPos();
		const auto BlockStorage = Chunk->GetBlockStorage();

		for (int X = 0; X < VOX_CHUNKSIZE; X++)
		{
			for (int Y = 0; Y < VOX_CHUNKSIZE; Y++)
			{
				for (int Z = 0; Z < VOX_CHUNKSIZE; Z++)
				{
					FIntVector GlobalPos = ChunkPos + FIntVector(X, Y, Z);
				}
			}
		}
	}
};

UCLASS()
class UVoxelFlatWorldGenerator : public UVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	void GenerateChunk(UVoxelChunk* Chunk) override
	{
		const FIntVector ChunkPos = Chunk->GetMinPos();
		const auto BlockStorage = Chunk->GetBlockStorage();

		for (int X = 0; X < VOX_CHUNKSIZE; X++)
		{
			for (int Y = 0; Y < VOX_CHUNKSIZE; Y++)
			{
				for (int Z = 0; Z < VOX_CHUNKSIZE; Z++)
				{
					FIntVector GlobalPos = ChunkPos + FIntVector(X, Y, Z);

					if (GlobalPos.Z < 2)
					{
						BlockStorage->SetBlock(X, Y, Z, GetVoxelBlock(TEXT("SolidDefault")));
					}
				}
			}
		}
	};
};