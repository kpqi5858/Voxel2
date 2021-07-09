#include "VoxelMesher.h"
#include "VoxelWorld.h"
#include "VoxelChunk.h"

FVoxelMesher::FVoxelMesher(UVoxelWorld* InVoxelWorld)
{
	VoxelWorld = InVoxelWorld;

	VoxelSize = VoxelWorld->VoxelSize;
}

void FVoxelMesher::DoMesh(UVoxelChunk* Chunk, TSharedPtr<FVoxelMeshData> MeshData)
{
	auto AddBox = [](FVoxelMeshSection& Section, int X, int Y, int Z, float VoxelSize)
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
			MeshData.Colors.Add(FColor::White);
			MeshData.TexCoords.Add(InTexCoord);
		};

		auto AddTriangle = [&](const uint32 A, const uint32 B, const uint32 C)
		{
			MeshData.Triangles.AddTriangle(A + VertIndex, B + VertIndex, C + VertIndex);
		};



		// Pos Z
		TangentZ = FVector(0.0f, 0.0f, 1.0f);
		TangentX = FVector(0.0f, -1.0f, 0.0f);
		AddVertex(BoxVerts[0], TangentX, TangentZ, FVector2D(0.0f, 0.0f));
		AddVertex(BoxVerts[1], TangentX, TangentZ, FVector2D(0.0f, 1.0f));
		AddVertex(BoxVerts[2], TangentX, TangentZ, FVector2D(1.0f, 1.0f));
		AddVertex(BoxVerts[3], TangentX, TangentZ, FVector2D(1.0f, 0.0f));
		AddTriangle(0, 1, 3);
		AddTriangle(1, 2, 3);

		// Neg X
		TangentZ = FVector(-1.0f, 0.0f, 0.0f);
		TangentX = FVector(0.0f, -1.0f, 0.0f);
		AddVertex(BoxVerts[4], TangentX, TangentZ, FVector2D(0.0f, 0.0f));
		AddVertex(BoxVerts[0], TangentX, TangentZ, FVector2D(0.0f, 1.0f));
		AddVertex(BoxVerts[3], TangentX, TangentZ, FVector2D(1.0f, 1.0f));
		AddVertex(BoxVerts[7], TangentX, TangentZ, FVector2D(1.0f, 0.0f));
		AddTriangle(4, 5, 7);
		AddTriangle(5, 6, 7);

		// Pos Y
		TangentZ = FVector(0.0f, 1.0f, 0.0f);
		TangentX = FVector(-1.0f, 0.0f, 0.0f);
		AddVertex(BoxVerts[5], TangentX, TangentZ, FVector2D(0.0f, 0.0f));
		AddVertex(BoxVerts[1], TangentX, TangentZ, FVector2D(0.0f, 1.0f));
		AddVertex(BoxVerts[0], TangentX, TangentZ, FVector2D(1.0f, 1.0f));
		AddVertex(BoxVerts[4], TangentX, TangentZ, FVector2D(1.0f, 0.0f));
		AddTriangle(8, 9, 11);
		AddTriangle(9, 10, 11);

		// Pos X
		TangentZ = FVector(1.0f, 0.0f, 0.0f);
		TangentX = FVector(0.0f, 1.0f, 0.0f);
		AddVertex(BoxVerts[6], TangentX, TangentZ, FVector2D(0.0f, 0.0f));
		AddVertex(BoxVerts[2], TangentX, TangentZ, FVector2D(0.0f, 1.0f));
		AddVertex(BoxVerts[1], TangentX, TangentZ, FVector2D(1.0f, 1.0f));
		AddVertex(BoxVerts[5], TangentX, TangentZ, FVector2D(1.0f, 0.0f));
		AddTriangle(12, 13, 15);
		AddTriangle(13, 14, 15);

		// Neg Y
		TangentZ = FVector(0.0f, -1.0f, 0.0f);
		TangentX = FVector(1.0f, 0.0f, 0.0f);
		AddVertex(BoxVerts[7], TangentX, TangentZ, FVector2D(0.0f, 0.0f));
		AddVertex(BoxVerts[3], TangentX, TangentZ, FVector2D(0.0f, 1.0f));
		AddVertex(BoxVerts[2], TangentX, TangentZ, FVector2D(1.0f, 1.0f));
		AddVertex(BoxVerts[6], TangentX, TangentZ, FVector2D(1.0f, 0.0f));
		AddTriangle(16, 17, 19);
		AddTriangle(17, 18, 19);

		// Neg Z
		TangentZ = FVector(0.0f, 0.0f, -1.0f);
		TangentX = FVector(0.0f, 1.0f, 0.0f);
		AddVertex(BoxVerts[7], TangentX, TangentZ, FVector2D(0.0f, 0.0f));
		AddVertex(BoxVerts[6], TangentX, TangentZ, FVector2D(0.0f, 1.0f));
		AddVertex(BoxVerts[5], TangentX, TangentZ, FVector2D(1.0f, 1.0f));
		AddVertex(BoxVerts[4], TangentX, TangentZ, FVector2D(1.0f, 0.0f));
		AddTriangle(20, 21, 23);
		AddTriangle(21, 22, 23);
	};

	auto& Section = MeshData->Sections.AddDefaulted_GetRef();

	Section.Material = UMaterial::GetDefaultMaterial(EMaterialDomain::MD_Surface);

	for (int X = 0; X < VOX_CHUNKSIZE; X++)
	{
		for (int Y = 0; Y < VOX_CHUNKSIZE; Y++)
		{
			for (int Z = 0; Z < VOX_CHUNKSIZE; Z++)
			{
				FVoxelBlock& ThisBlock = Chunk->Blocks[FVoxelUtilities::GetArrayIndex(X, Y, Z)];

				if (ThisBlock.BlockId == 1)
				{
					AddBox(Section, X, Y, Z, VoxelSize);
				}
			}
		}
	}
}
