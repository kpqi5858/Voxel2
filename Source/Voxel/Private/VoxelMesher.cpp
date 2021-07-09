#include "VoxelMesher.h"
#include "VoxelWorld.h"
#include "VoxelChunk.h"

FVoxelMesher::FVoxelMesher(UVoxelWorld* InVoxelWorld)
{
	VoxelWorld = InVoxelWorld;

	VoxelSize = VoxelWorld->VoxelSize;
}

void FVoxelMesher::DoMesh(UVoxelChunk* Chunk, TSharedPtr<FVoxelMeshData, ESPMode::ThreadSafe> MeshData, const FVoxelMesherParameters& Params)
{
	auto AddFace = [](FVoxelMeshSection& Section, int X, int Y, int Z, float VoxelSize, FColor Color, EBlockFace Face)
	{
		auto& MeshData = Section.MeshData;

		const uint32 VertIndex = MeshData.Positions.Num();

		// Generate verts
		const FVector BoxVerts[8] =
		{
			FVector(0, 1, 1),
			FVector(1, 1, 1),
			FVector(1, 0, 1),
			FVector(0, 0, 1),
			FVector(0, 1, 0),
			FVector(1, 1, 0),
			FVector(1, 0, 0),
			FVector(0, 0, 0)
		};

		FVector TangentX, TangentY, TangentZ;


		auto AddVertex = [&](const FVector& InPosition, const FVector& InTangentX, const FVector& InTangentZ, const FVector2D& InTexCoord)
		{
			MeshData.Positions.Add((InPosition + FVector(X, Y, Z)) * VoxelSize);
			MeshData.Tangents.Add(InTangentZ, InTangentX);
			MeshData.Colors.Add(Color);
			MeshData.TexCoords.Add(InTexCoord);
		};

		auto AddTriangle = [&](const uint32 A, const uint32 B, const uint32 C)
		{
			MeshData.Triangles.AddTriangle(A + VertIndex, B + VertIndex, C + VertIndex);
		};


		switch (Face)
		{
		case EBlockFace::TOP:
		{
			// Pos Z
			TangentZ = FVector(0.0f, 0.0f, 1.0f);
			TangentX = FVector(0.0f, -1.0f, 0.0f);
			AddVertex(BoxVerts[0], TangentX, TangentZ, FVector2D(0.0f, 0.0f));
			AddVertex(BoxVerts[1], TangentX, TangentZ, FVector2D(0.0f, 1.0f));
			AddVertex(BoxVerts[2], TangentX, TangentZ, FVector2D(1.0f, 1.0f));
			AddVertex(BoxVerts[3], TangentX, TangentZ, FVector2D(1.0f, 0.0f));
			break;
		}
		case EBlockFace::BACK:
		{
			// Neg X
			TangentZ = FVector(-1.0f, 0.0f, 0.0f);
			TangentX = FVector(0.0f, -1.0f, 0.0f);
			AddVertex(BoxVerts[4], TangentX, TangentZ, FVector2D(0.0f, 0.0f));
			AddVertex(BoxVerts[0], TangentX, TangentZ, FVector2D(0.0f, 1.0f));
			AddVertex(BoxVerts[3], TangentX, TangentZ, FVector2D(1.0f, 1.0f));
			AddVertex(BoxVerts[7], TangentX, TangentZ, FVector2D(1.0f, 0.0f));
			break;
		}
		case EBlockFace::RIGHT:
		{
			// Pos Y
			TangentZ = FVector(0.0f, 1.0f, 0.0f);
			TangentX = FVector(-1.0f, 0.0f, 0.0f);
			AddVertex(BoxVerts[5], TangentX, TangentZ, FVector2D(0.0f, 0.0f));
			AddVertex(BoxVerts[1], TangentX, TangentZ, FVector2D(0.0f, 1.0f));
			AddVertex(BoxVerts[0], TangentX, TangentZ, FVector2D(1.0f, 1.0f));
			AddVertex(BoxVerts[4], TangentX, TangentZ, FVector2D(1.0f, 0.0f));
			break;
		}
		case EBlockFace::FRONT:
		{
			// Pos X
			TangentZ = FVector(1.0f, 0.0f, 0.0f);
			TangentX = FVector(0.0f, 1.0f, 0.0f);
			AddVertex(BoxVerts[6], TangentX, TangentZ, FVector2D(0.0f, 0.0f));
			AddVertex(BoxVerts[2], TangentX, TangentZ, FVector2D(0.0f, 1.0f));
			AddVertex(BoxVerts[1], TangentX, TangentZ, FVector2D(1.0f, 1.0f));
			AddVertex(BoxVerts[5], TangentX, TangentZ, FVector2D(1.0f, 0.0f));
			break;
		}
		case EBlockFace::LEFT:
		{
			// Neg Y
			TangentZ = FVector(0.0f, -1.0f, 0.0f);
			TangentX = FVector(1.0f, 0.0f, 0.0f);
			AddVertex(BoxVerts[7], TangentX, TangentZ, FVector2D(0.0f, 0.0f));
			AddVertex(BoxVerts[3], TangentX, TangentZ, FVector2D(0.0f, 1.0f));
			AddVertex(BoxVerts[2], TangentX, TangentZ, FVector2D(1.0f, 1.0f));
			AddVertex(BoxVerts[6], TangentX, TangentZ, FVector2D(1.0f, 0.0f));
			break;
		}
		case EBlockFace::BOTTOM:
		{
			// Neg Z
			TangentZ = FVector(0.0f, 0.0f, -1.0f);
			TangentX = FVector(0.0f, 1.0f, 0.0f);
			AddVertex(BoxVerts[7], TangentX, TangentZ, FVector2D(0.0f, 0.0f));
			AddVertex(BoxVerts[6], TangentX, TangentZ, FVector2D(0.0f, 1.0f));
			AddVertex(BoxVerts[5], TangentX, TangentZ, FVector2D(1.0f, 1.0f));
			AddVertex(BoxVerts[4], TangentX, TangentZ, FVector2D(1.0f, 0.0f));
			break;
		}
		}
		
		AddTriangle(0, 1, 3);
		AddTriangle(1, 2, 3);
	};

	auto BlockStorage = Chunk->GetBlockStorage();

	const FIntVector ChunkMinPos = Chunk->GetMinPos();

	for (int X = 0; X < VOX_CHUNKSIZE; X++)
	{
		for (int Y = 0; Y < VOX_CHUNKSIZE; Y++)
		{
			for (int Z = 0; Z < VOX_CHUNKSIZE; Z++)
			{
				const FIntVector LocalPos = FIntVector(X, Y, Z);

				const FVoxelBlock& ThisBlock = BlockStorage->GetBlock(X, Y, Z);
				UVoxelBlockDef* BlockDef = ThisBlock.GetBlock();

				if (BlockDef->ShouldBePolygonized())
				{
					auto& Section = MeshData->GetSectionFor(BlockDef);

					for (int FaceNum = 0; FaceNum < 6; FaceNum++)
					{
						const EBlockFace Face = static_cast<EBlockFace>(FaceNum);
						const FIntVector CheckPos = LocalPos + FVoxelUtilities::GetFaceOffset(Face);

						bool bOcculdeThisFace = false;

						if (FVoxelUtilities::IsInLocalPosition(CheckPos.X, CheckPos.Y, CheckPos.Z))
						{
							const FVoxelBlock CheckBlock = BlockStorage->GetBlock(CheckPos.X, CheckPos.Y, CheckPos.Z);

							bOcculdeThisFace = BlockDef->VisiblityType == CheckBlock.GetBlock()->VisiblityType;
							
						}
						else if (Params.bOcculdeFaceBorder)
						{
							const FIntVector CheckGlobalPos = ChunkMinPos + CheckPos;

							const FIntVector AdjChunkPos = FVoxelUtilities::VoxelPosToChunkPos(CheckGlobalPos);
							const FIntVector AdjChunkLocalPos = FVoxelUtilities::VoxelPosToLocalPos(CheckGlobalPos);

							UVoxelChunk* AdjChunk = VoxelWorld->GetChunk(AdjChunkPos);

							if (AdjChunk)
							{
								const FVoxelBlock CheckBlock = AdjChunk->GetBlock(AdjChunkLocalPos.X, AdjChunkLocalPos.Y, AdjChunkLocalPos.Z);

								bOcculdeThisFace = BlockDef->VisiblityType == CheckBlock.GetBlock()->VisiblityType;
							}
							else
							{
								bOcculdeThisFace = true;
							}
						}

						if (!bOcculdeThisFace)
						{
							AddFace(Section, X, Y, Z, VoxelSize, ThisBlock.Color, Face);
						}
					}
				}
			}
		}
	}
}

void FVoxelMesher::DoCollision(UVoxelChunk* Chunk, TSharedPtr<FRuntimeMeshCollisionData, ESPMode::ThreadSafe> ColData)
{
	auto AddFace = [](FRuntimeMeshCollisionData& MeshData, int X, int Y, int Z, float VoxelSize, EBlockFace Face)
	{
		const uint32 VertIndex = MeshData.Vertices.Num();

		// Generate verts
		const FVector BoxVerts[8] =
		{
			FVector(0, 1, 1),
			FVector(1, 1, 1),
			FVector(1, 0, 1),
			FVector(0, 0, 1),
			FVector(0, 1, 0),
			FVector(1, 1, 0),
			FVector(1, 0, 0),
			FVector(0, 0, 0)
		};

		FVector TangentX, TangentY, TangentZ;


		auto AddVertex = [&](const FVector& InPosition, const FVector& InTangentX, const FVector& InTangentZ, const FVector2D& InTexCoord)
		{
			MeshData.Vertices.Add((InPosition + FVector(X, Y, Z)) * VoxelSize);
		};

		auto AddTriangle = [&](const uint32 A, const uint32 B, const uint32 C)
		{
			MeshData.Triangles.Add(A + VertIndex, B + VertIndex, C + VertIndex);
		};


		switch (Face)
		{
		case EBlockFace::TOP:
		{
			// Pos Z
			TangentZ = FVector(0.0f, 0.0f, 1.0f);
			TangentX = FVector(0.0f, -1.0f, 0.0f);
			AddVertex(BoxVerts[0], TangentX, TangentZ, FVector2D(0.0f, 0.0f));
			AddVertex(BoxVerts[1], TangentX, TangentZ, FVector2D(0.0f, 1.0f));
			AddVertex(BoxVerts[2], TangentX, TangentZ, FVector2D(1.0f, 1.0f));
			AddVertex(BoxVerts[3], TangentX, TangentZ, FVector2D(1.0f, 0.0f));
			break;
		}
		case EBlockFace::BACK:
		{
			// Neg X
			TangentZ = FVector(-1.0f, 0.0f, 0.0f);
			TangentX = FVector(0.0f, -1.0f, 0.0f);
			AddVertex(BoxVerts[4], TangentX, TangentZ, FVector2D(0.0f, 0.0f));
			AddVertex(BoxVerts[0], TangentX, TangentZ, FVector2D(0.0f, 1.0f));
			AddVertex(BoxVerts[3], TangentX, TangentZ, FVector2D(1.0f, 1.0f));
			AddVertex(BoxVerts[7], TangentX, TangentZ, FVector2D(1.0f, 0.0f));
			break;
		}
		case EBlockFace::RIGHT:
		{
			// Pos Y
			TangentZ = FVector(0.0f, 1.0f, 0.0f);
			TangentX = FVector(-1.0f, 0.0f, 0.0f);
			AddVertex(BoxVerts[5], TangentX, TangentZ, FVector2D(0.0f, 0.0f));
			AddVertex(BoxVerts[1], TangentX, TangentZ, FVector2D(0.0f, 1.0f));
			AddVertex(BoxVerts[0], TangentX, TangentZ, FVector2D(1.0f, 1.0f));
			AddVertex(BoxVerts[4], TangentX, TangentZ, FVector2D(1.0f, 0.0f));
			break;
		}
		case EBlockFace::FRONT:
		{
			// Pos X
			TangentZ = FVector(1.0f, 0.0f, 0.0f);
			TangentX = FVector(0.0f, 1.0f, 0.0f);
			AddVertex(BoxVerts[6], TangentX, TangentZ, FVector2D(0.0f, 0.0f));
			AddVertex(BoxVerts[2], TangentX, TangentZ, FVector2D(0.0f, 1.0f));
			AddVertex(BoxVerts[1], TangentX, TangentZ, FVector2D(1.0f, 1.0f));
			AddVertex(BoxVerts[5], TangentX, TangentZ, FVector2D(1.0f, 0.0f));
			break;
		}
		case EBlockFace::LEFT:
		{
			// Neg Y
			TangentZ = FVector(0.0f, -1.0f, 0.0f);
			TangentX = FVector(1.0f, 0.0f, 0.0f);
			AddVertex(BoxVerts[7], TangentX, TangentZ, FVector2D(0.0f, 0.0f));
			AddVertex(BoxVerts[3], TangentX, TangentZ, FVector2D(0.0f, 1.0f));
			AddVertex(BoxVerts[2], TangentX, TangentZ, FVector2D(1.0f, 1.0f));
			AddVertex(BoxVerts[6], TangentX, TangentZ, FVector2D(1.0f, 0.0f));
			break;
		}
		case EBlockFace::BOTTOM:
		{
			// Neg Z
			TangentZ = FVector(0.0f, 0.0f, -1.0f);
			TangentX = FVector(0.0f, 1.0f, 0.0f);
			AddVertex(BoxVerts[7], TangentX, TangentZ, FVector2D(0.0f, 0.0f));
			AddVertex(BoxVerts[6], TangentX, TangentZ, FVector2D(0.0f, 1.0f));
			AddVertex(BoxVerts[5], TangentX, TangentZ, FVector2D(1.0f, 1.0f));
			AddVertex(BoxVerts[4], TangentX, TangentZ, FVector2D(1.0f, 0.0f));
			break;
		}
		}

		AddTriangle(0, 1, 3);
		AddTriangle(1, 2, 3);
	};

	auto BlockStorage = Chunk->GetBlockStorage();
	auto& MeshData = *ColData;

	for (int X = 0; X < VOX_CHUNKSIZE; X++)
	{
		for (int Y = 0; Y < VOX_CHUNKSIZE; Y++)
		{
			for (int Z = 0; Z < VOX_CHUNKSIZE; Z++)
			{
				const FIntVector LocalPos = FIntVector(X, Y, Z);
				const FVoxelBlock& ThisBlock = BlockStorage->GetBlock(X, Y, Z);
				UVoxelBlockDef* BlockDef = ThisBlock.GetBlock();

				if (BlockDef->bDoCollisions)
				{
					for (int FaceNum = 0; FaceNum < 6; FaceNum++)
					{
						const EBlockFace Face = static_cast<EBlockFace>(FaceNum);
						const FIntVector CheckPos = LocalPos + FVoxelUtilities::GetFaceOffset(Face);

						bool bOcculdeThisFace = false;

						if (FVoxelUtilities::IsInLocalPosition(CheckPos.X, CheckPos.Y, CheckPos.Z))
						{
							const FVoxelBlock& CheckBlock = BlockStorage->GetBlock(CheckPos.X, CheckPos.Y, CheckPos.Z);

							bOcculdeThisFace = CheckBlock.GetBlock()->bDoCollisions;

						}

						if (!bOcculdeThisFace)
						{
							AddFace(MeshData, X, Y, Z, VoxelSize, Face);
						}
					}
				}
			}
		}
	}
}
