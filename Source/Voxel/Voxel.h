// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVoxel, Log, All)

#define VOX_CHUNKSIZE 32
#define VOX_ARRAYSIZE (VOX_CHUNKSIZE*VOX_CHUNKSIZE*VOX_CHUNKSIZE)
