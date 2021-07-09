#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "VoxelWorldActor.generated.h"

class UVoxelWorld;

UCLASS()
class VOXEL_API AVoxelWorldActor : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	float VoxelSize = 100.0f;

	UPROPERTY()
	UVoxelWorld* VoxelWorld;

public:
	AVoxelWorldActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void InitWorld();
	void DestroyWorld();

	UFUNCTION(BlueprintCallable)
	void Test();
};
