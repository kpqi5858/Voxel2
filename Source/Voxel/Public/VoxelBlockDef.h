#pragma once

#include "CoreMinimal.h"

#include "VoxelBlockDef.generated.h"

UCLASS(Blueprintable, Abstract)
class UVoxelBlockDef : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
		FName RegistryName;

	//If false, no collision will be created
	UPROPERTY(EditDefaultsOnly)
		bool bDoCollisions = true;

	//Is this block treated as an empty space?
	UPROPERTY(EditDefaultsOnly)
		bool bIsEmptyBlock = false;

	//Blocks with same VisitlibyType will occlude each other
	UPROPERTY(EditDefaultsOnly)
		int VisiblityType = 0;

	//If none, the block will be not visible
	UPROPERTY(EditDefaultsOnly)
		UMaterialInterface* Material = nullptr;

	//Overrides the type id
	UPROPERTY(EditDefaultsOnly)
		int32 OverrideTypeId = -1;

	//If false, other blocks with same material will meshed together in one mesh section.
	UPROPERTY(EditDefaultsOnly)
		bool bSeparateMeshSections = false;

	//Don't register this block?
	UPROPERTY(EditDefaultsOnly)
		bool bDontRegister = false;

	UPROPERTY(EditDefaultsOnly)
		FColor DefaultColor = FColor::White;

	UPROPERTY(BlueprintReadOnly)
		bool bIsRegistered = false;

	//Unique type id, DON'T SAVE THIS VALUE, Save RegistryName instead
	uint32 TypeId = 0;


	UFUNCTION(BlueprintCallable)
		int GetTypeId() const
	{
		return TypeId;
	};

	inline bool ShouldBePolygonized() const
	{
		return Material != nullptr;
	};
};


//Default blocks

UCLASS()
class UDefaultBlockEmpty : public UVoxelBlockDef
{
	GENERATED_BODY()

public:
	UDefaultBlockEmpty()
	{
		RegistryName = TEXT("Empty");
		bIsEmptyBlock = true;
		bDoCollisions = false;
		VisiblityType = 0;
		OverrideTypeId = 0;
		Material = nullptr;
	};
};

UCLASS()
class UDefaultBlockSolid : public UVoxelBlockDef
{
	GENERATED_BODY()

public:
	UDefaultBlockSolid()
	{
		RegistryName = TEXT("SolidDefault");
		bDoCollisions = true;
		VisiblityType = 1;
		OverrideTypeId = 1;
		Material = UMaterial::GetDefaultMaterial(EMaterialDomain::MD_Surface);
	};
};
