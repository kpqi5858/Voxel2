#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "VoxelTracker.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class UVoxelTracker : public UActorComponent
{
	GENERATED_BODY()

public:
	UVoxelTracker();

	void BeginPlay() override;
};