#include "VoxelWorld.h"
#include "VoxelChunk.h"
#include "VoxelRMCProvider.h"
#include "VoxelWorldGenerator.h"
#include "VoxelMesher.h"
#include "RuntimeMeshActor.h"

UVoxelWorld::UVoxelWorld()
{
}

UVoxelWorld::~UVoxelWorld()
{
    if (bIsWorldCreated)
    {
        DestroyWorld();
    }
}

void UVoxelWorld::Test()
{
    check(bIsWorldCreated);

    UVoxelChunk* Chunk = GetChunk(FIntVector(0), true);

    InitMesh(GetFreeMesh(), Chunk);
    Chunk->GenerateChunk();
    Chunk->PolygonizeChunk();
    Chunk->UpdateMesh();
}

void UVoxelWorld::CreateWorld(UWorld* InWorld)
{
    check(!bIsWorldCreated);

    World = InWorld;

    if (!bActorPerMesh)
    {
        FActorSpawnParameters SpawnPara;
        SpawnPara.Name = FName("VoxelWorldMesh");

        MeshActor = World->SpawnActor(AActor::StaticClass());
    }

    WorldGenerator = NewObject<UVoxelFlatWorldGenerator>(this);

    Mesher = new FVoxelMesher(this);

    bIsWorldCreated = true;
}

void UVoxelWorld::DestroyWorld()
{
    check(bIsWorldCreated);

    for (auto& Chunk : ChunksArray)
    {
        delete Chunk;
    }

    Chunks.Empty();
    ChunksArray.Empty();

    delete Mesher;
    Mesher = nullptr;

    WorldGenerator = nullptr;

    if (bActorPerMesh)
    {
        for (auto& Comp : MeshComponents)
        {
            ARuntimeMeshActor* Parent = Comp->GetOwner<ARuntimeMeshActor>();
            check(Parent);

            Parent->Destroy();
        }
    }
    else
    {
        MeshActor->Destroy();
        MeshActor = nullptr;
    }

    MeshComponents.Empty();

    bIsWorldCreated = false;
}

UVoxelChunk* UVoxelWorld::NewChunk(const FIntVector& ChunkPos)
{
    check(!Chunks.Contains(ChunkPos));

    UVoxelChunk* NewChunk = new UVoxelChunk(this, ChunkPos);

    Chunks.Add(ChunkPos, NewChunk);
    ChunksArray.Add(NewChunk);

    return NewChunk;
}

UVoxelChunk* UVoxelWorld::GetChunk(const FIntVector& ChunkPos, const bool bCreateIfNotExists)
{
    UVoxelChunk** Find = Chunks.Find(ChunkPos);

    if (!Find && bCreateIfNotExists)
    {
        return NewChunk(ChunkPos);
    }

    if (!Find)
    {
        return nullptr;
    }

    return *Find;
}

URuntimeMeshComponent* UVoxelWorld::GetFreeMesh()
{
    URuntimeMeshComponent* Comp = nullptr;

    if (bActorPerMesh)
    {
        FActorSpawnParameters Para;
        Para.Name = FName(FString::Printf(TEXT("VoxelChunkMesh-%d"), FMath::Rand() % 1024));

        ARuntimeMeshActor* Actor = World->SpawnActor<ARuntimeMeshActor>(ARuntimeMeshActor::StaticClass(), Para);

        Actor->SetMobility(EComponentMobility::Movable);

        Comp = Actor->GetRuntimeMeshComponent();
    }
    else
    {
        Comp = NewObject<URuntimeMeshComponent>(MeshActor);
        Comp->SetupAttachment(MeshActor->GetRootComponent());
        Comp->RegisterComponent();
    }

    UVoxelRMCProvider* Provider = NewObject<UVoxelRMCProvider>(Comp);

    Provider->InitVoxel(this);
    Comp->Initialize(Provider);
    
    return Comp;
}

void UVoxelWorld::InitMesh(URuntimeMeshComponent* Comp, UVoxelChunk* Chunk)
{
    UVoxelRMCProvider* Provider = CastChecked<UVoxelRMCProvider>(Comp->GetProvider());

    Comp->SetRelativeLocation(FVector(Chunk->GetMinPos()) * VoxelSize);
    Chunk->InitMesh(Provider);
}

FVoxelMesher* UVoxelWorld::GetMesher()
{
    check(bIsWorldCreated);

    return Mesher;
}
