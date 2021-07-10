#include "VoxelWorld.h"
#include "VoxelChunk.h"
#include "VoxelRMCProvider.h"
#include "VoxelWorldGenerator.h"
#include "VoxelMesher.h"
#include "RuntimeMeshActor.h"
#include "Async/Async.h"

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
}

void UVoxelWorld::CreateWorld(UWorld* InWorld)
{
    check(!bIsWorldCreated);

    World = InWorld;

    if (!bActorPerMesh)
    {
        FActorSpawnParameters SpawnPara;
        SpawnPara.Name = FName("VoxelWorldMesh");

        MeshActor = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnPara);
    }

    WorldGenerator = NewObject<UVoxelFlatWorldGenerator>(this);

    Mesher = new FVoxelMesher(this);

    BlockRegistryPtr = FBlockRegistry::GetInstance();

    ThreadPool = FQueuedThreadPool::Allocate();
    ThreadPool->Create(8, 409600U, EThreadPriority::TPri_Normal);

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

    BlockRegistryPtr = nullptr;

    ThreadPool->Destroy();

    bIsWorldCreated = false;
}

void UVoxelWorld::Tick()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(314159, 0, FColor::Emerald, FString::Printf(TEXT("Loaded chunks : %d, Jobs remaining : %d, Updates this tick : %d"), ChunksArray.Num(), JobsRemaining.GetValue(), UpdatesThisTick));
    }

    UpdatesThisTick = 0;

    for (auto& Tracker : Trackers)
    {
        FVector Location = Tracker->GetActorLocation();

        FIntVector VoxelPos = ToVoxelPos(Location);
        FIntVector ChunkPos = FVoxelUtilities::VoxelPosToChunkPos(VoxelPos);

        float CreationDistance = RenderDistance + DestroyExtent;

        for (int X = ChunkPos.X - CreationDistance; X <= ChunkPos.X + CreationDistance; X++)
        {
            for (int Y = ChunkPos.Y - CreationDistance; Y <= ChunkPos.Y + CreationDistance; Y++)
            {
                for (int Z = ChunkPos.Z - CreationDistance; Z <= ChunkPos.Z + CreationDistance; Z++)
                {
                    FIntVector NewChunk = FIntVector(X, Y, Z);
                    if (GetMinDistanceToTrackers(NewChunk) <= CreationDistance)
                    {
                        GetChunk(NewChunk, true);
                    }
                }
            }
        }
        //GetChunk(ChunkPos, true);
    }

    for (auto& Chunk : ChunksArray)
    {
        if (Chunk->GetChunkState() == EChunkState::Destroyed)
        {
            continue;
        }

        Chunk->Tick();
    }

    TArray<UVoxelChunk*> NewToDestroy;
    NewToDestroy.Reserve(ToDestroy.Num());

    for (auto& Chunk : ToDestroy)
    {
        if (Chunk->IsReadyForFinishDestroy())
        {
            FinalizeDestroyChunk(Chunk);
        }
        else
        {
            NewToDestroy.Add(Chunk);
        }
    }

    ToDestroy = NewToDestroy;
}

void UVoxelWorld::RegisterTracker(AActor* Actor)
{
    Trackers.AddUnique(Actor);
}

void UVoxelWorld::QueueChunkWork(UVoxelChunk* Chunk, EChunkWorkType Type)
{
    FChunkWork& Work = Chunk->GetChunkWork(Type);

    if (Work.bIsWorkOnline)
    {
        return;
    }

    Chunk->RemainingWorks.Increment();

    Work.bIsDelaying = false;
    Work.bIsWorkOnline = true;
    Work.bIsDone = false;

    if (bAsync)
    {
        static int64 UniqueWorkId = 1;

        Work.CurrentWorkId = UniqueWorkId;

        JobsRemaining.Increment();
        ThreadPool->AddQueuedWork(new FQueuedChunkWork(Chunk, Type, UniqueWorkId));
        
        UniqueWorkId++;
    }
    else
    {
        switch (Type)
        {
        case EChunkWorkType::WorldGen:
        {
            Chunk->GenerateChunk();
            break;
        }
        case EChunkWorkType::Collision:
        {
            Chunk->BuildCollision();
            break;
        }
        case EChunkWorkType::Mesh:
        {
            Chunk->PolygonizeChunk();
            break;
        }
        }

        Work.bIsWorkOnline = false;
        Work.bIsDone = true;

        Chunk->RemainingWorks.Decrement();
    }

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
    FRWScopeLock sl(ChunksLock, FRWScopeLockType::SLT_ReadOnly);

    UVoxelChunk** Find = Chunks.Find(ChunkPos);

    if (!Find && bCreateIfNotExists)
    {
        sl.ReleaseReadOnlyLockAndAcquireWriteLock_USE_WITH_CAUTION();
        return NewChunk(ChunkPos);
    }

    if (!Find)
    {
        return nullptr;
    }

    return *Find;
}

void UVoxelWorld::OnChunkDestroyed(UVoxelChunk* Chunk)
{
    ToDestroy.Add(Chunk);
}

void UVoxelWorld::FinalizeDestroyChunk(UVoxelChunk* Chunk)
{
    Chunks.Remove(Chunk->ChunkPos);
    ChunksArray.RemoveSwap(Chunk);

    delete Chunk;
}

URuntimeMeshComponent* UVoxelWorld::GetFreeMesh()
{
    URuntimeMeshComponent* Comp = nullptr;

    if (FreeMeshComponents.Num())
    {
        Comp = FreeMeshComponents.Pop(false);
    }
    else
    {
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
    }

    UVoxelRMCProvider* Provider = NewObject<UVoxelRMCProvider>(Comp);

    Provider->InitVoxel(this);
    Comp->Initialize(Provider);
    
    return Comp;
}

void UVoxelWorld::ReleaseMesh(URuntimeMeshComponent* Comp)
{
    Comp->Initialize(nullptr);
    FreeMeshComponents.Add(Comp);
}

void UVoxelWorld::SetupMesh(UVoxelChunk* Chunk)
{
    URuntimeMeshComponent* Comp = GetFreeMesh();

    Comp->SetRelativeLocation(FVector(Chunk->GetMinPos()) * VoxelSize);
    Chunk->InitMesh(Comp);
}

FVoxelMesher* UVoxelWorld::GetMesher()
{
    check(bIsWorldCreated);

    return Mesher;
}

float UVoxelWorld::GetMinDistanceToTrackers(const FIntVector& ChunkPos)
{
    float Min = 999999999.9f;

    for (auto& Tracker : Trackers)
    {
        FVector Location = Tracker->GetActorLocation();

        FIntVector VoxelPos = ToVoxelPos(Location);
        FIntVector OfChunkPos = FVoxelUtilities::VoxelPosToChunkPos(VoxelPos);

        Min = FMath::Min(Min, FVector::Dist(FVector(ChunkPos), FVector(OfChunkPos)));
    }

    return Min;
}

bool UVoxelWorld::ShouldBeRendered(const FIntVector& Chunk)
{
    return GetMinDistanceToTrackers(Chunk) < RenderDistance;
}

bool UVoxelWorld::ShouldBeDestroyed(const FIntVector& Chunk)
{
    return GetMinDistanceToTrackers(Chunk) > RenderDistance + DestroyExtent;
}

void FQueuedChunkWork::DoThreadedWork()
{
    auto& Work = Chunk->GetChunkWork(WorkType);

    //Is this work invalid?
    if (Work.CurrentWorkId.GetValue() != WorkId)
    {
        delete this;
        return;
    }

    switch (WorkType)
    {
    case EChunkWorkType::WorldGen:
    {
        Chunk->GenerateChunk();
        break;
    }
    case EChunkWorkType::Collision:
    {
        Chunk->BuildCollision();
        break;
    }
    case EChunkWorkType::Mesh:
    {
        Chunk->PolygonizeChunk();
        break;
    }
    }

    if (Work.CurrentWorkId.GetValue() != WorkId)
    {
        ensureAlways(false);

        delete this;
        return;
    }

    Work.bIsDone = true;
    Work.bIsWorkOnline = false;

    Chunk->RemainingWorks.Decrement();
    Chunk->VoxelWorld->JobsRemaining.Decrement();

    delete this;
}

void FQueuedChunkWork::Abandon()
{
    Chunk->RemainingWorks.Decrement();
    Chunk->VoxelWorld->JobsRemaining.Decrement();

    delete this;
}
