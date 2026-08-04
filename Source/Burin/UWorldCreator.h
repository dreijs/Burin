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
	static void CreateHistoricalPlace(UBurinWorld* world, FPlaceDataEntry entry);
	static void CreatePlace(UBurinWorld* world, FString name, double latitude, double longitude);
	static void DestroyProvince(FString name);

	static void CreateRandomWorld(UBurinWorld* world);
	static void CreateHistoricalWorld(UBurinWorld* world);
};
