#include "VoxelTracker.h"
#include "VoxelWorld.h"
#include "VoxelWorldActor.h"
#include "Kismet/GameplayStatics.h"

UVoxelTracker::UVoxelTracker()
{


}
void UVoxelTracker::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVoxelWorldActor::StaticClass(), FoundActors);

	for (auto& Actor : FoundActors)
	{
		AVoxelWorldActor* VoxelWorldActor = CastChecked<AVoxelWorldActor>(Actor);

		VoxelWorldActor->GetVoxelWordChecked()->RegisterTracker(GetOwner());
	}
}
