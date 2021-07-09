#pragma once

#include "CoreMinimal.h"
#include "Voxel/Voxel.h"

class UVoxelBlockDef;

class FBlockRegistryInstance
{
private:
	TMap<FName, UVoxelBlockDef*> BlockInstanceRegistry;
	TArray<UVoxelBlockDef*> UniqueIndices;

public:
	FBlockRegistryInstance();
	~FBlockRegistryInstance();

	bool bIsInitialized = false;

	void Setup();
	void Reset();

	bool IsValidIndex(const uint32 Index)
	{
		return UniqueIndices.IsValidIndex(Index);
	}

	UVoxelBlockDef* GetBlockByIndex(const uint32 Index)
	{
		if (!ensureMsgf(IsValidIndex(Index), TEXT("Invalid index in GetBlockByIndex : %d"), Index))
		{
			return GetBlockByIndex(0);
		}

		return UniqueIndices[Index];
	}

	UVoxelBlockDef* GetBlockInternal(FName Name)
	{
		UVoxelBlockDef** Find = BlockInstanceRegistry.Find(Name);
		if (Find)
		{
			return *Find;
		}
		else
		{
			UE_LOG(LogVoxel, Error, TEXT("Unknown block name : %s"), *Name.ToString());
			return GetBlockByIndex(0);
		}
	}

	template<typename T>
	UVoxelBlockDef* GetBlock(T Name)
	{
		static_assert(false, "GetBlock with such type is not implemented");
		return nullptr;
	}

	template<>
	UVoxelBlockDef* GetBlock<>(const FName Name)
	{
		return GetBlockInternal(Name);
	}

	template<>
	UVoxelBlockDef* GetBlock<>(const TCHAR* Name)
	{
		//Avoid unnecessary allocations on FName
		const FName Nam = FName(Name, EFindName::FNAME_Find);
		if (Nam != NAME_None)
		{
			return GetBlockInternal(Nam);
		}
		else
		{
			UE_LOG(LogVoxel, Error, TEXT("Unknown block name string : %s"), Name);
			return GetBlockByIndex(0);
		}
	}

	template<>
	UVoxelBlockDef* GetBlock<>(const FString Name)
	{
		return GetBlock(*Name);
	}

	template<>
	UVoxelBlockDef* GetBlock<>(const uint32 Id)
	{
		return GetBlockByIndex(Id);
	}

	template<>
	UVoxelBlockDef* GetBlock<>(const int Id)
	{
		if (Id < 0)
		{
			UE_LOG(LogVoxel, Error, TEXT("GetBlock with negative id : %d"), Id);
			return GetBlockByIndex(0);
		}
		return GetBlockByIndex(Id);
	}
};

class FBlockRegistry
{
private:
	static TWeakPtr<FBlockRegistryInstance> InstancePtr;
	static FBlockRegistryInstance* InstancePtrRaw;

public:
	//Iterates classes that inherits UBlock
	static void FindVoxelBlocks(TArray<TWeakObjectPtr<UClass>>& BlockClassesOut);

	//AVoxelWorld should have this in member
	static TSharedPtr<FBlockRegistryInstance> GetInstance();

	//Macro only
	static FBlockRegistryInstance* GetInstance_Ptr();
};