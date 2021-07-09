#include "VoxelWorldActor.h"
#include "VoxelWorld.h"

// Sets default values
AVoxelWorldActor::AVoxelWorldActor()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	PrimaryActorTick.bCanEverTick = true;
}

void AVoxelWorldActor::BeginPlay()
{
	Super::BeginPlay();

}

void AVoxelWorldActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AVoxelWorldActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (VoxelWorld)
	{
		DestroyWorld();
	}
}

void AVoxelWorldActor::InitWorld()
{
	check(!VoxelWorld);

	VoxelWorld = NewObject<UVoxelWorld>(this);
	VoxelWorld->CreateWorld(GetWorld());
}

void AVoxelWorldActor::DestroyWorld()
{
	VoxelWorld->DestroyWorld();
	VoxelWorld = nullptr;
}

void AVoxelWorldActor::Test()
{
	InitWorld();

	VoxelWorld->Test();
}

