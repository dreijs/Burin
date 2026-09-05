// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Concepts/Provinces/FArea.h"
#include "Concepts/Provinces/FPlace.h"
#include "UWorldCreatorSettings.h"
#include "UBurinWorld.h"

#include "UWorldCreator.generated.h"

UCLASS(Blueprintable)
class BURIN_API UWorldCreator : public UObject
{
	GENERATED_BODY()

public:
	static FArea CreateArea(FString name, double latitude, double longitude);
	static void DestroyArea(FString name);
	static void CreateHistoricalPlace(UBurinWorld* world, const FPlaceDataEntry& entry, int32 controllerIndex);

	/** Appends `entry` to world->Polities and returns its index there. */
	static int32 CreateHistoricalPolity(UBurinWorld* world, const FPolityDataEntry& entry);

	/** `controllerIndex` indexes world->Polities, which must already hold the controlling polity. */
	static void CreatePlace(UBurinWorld* world, FString name, double latitude, double longitude, int32 controllerIndex = INDEX_NONE);
	static void DestroyProvince(FString name);

	static void CreateRandomWorld(UBurinWorld* world);
	static void CreateHistoricalWorld(UBurinWorld* world, int32 year);
};
