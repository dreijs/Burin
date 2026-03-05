// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Concepts/Provinces/UProvince.h"
#include "UWorldCreatorSettings.h"
#include "UBurinWorld.h"

#include "UWorldCreator.generated.h"

UCLASS(Blueprintable)
class BURIN_API UWorldCreator : public UObject
{
	GENERATED_BODY()

public:
	UWorldCreator();
	~UWorldCreator();

	static UArea CreateArea(FString name, double latitude, double longitude);
	static void DestroyArea(FString name);
	static UProvince CreateProvince(FString name, double latitude, double longitude);
	static void DestroyProvince(FString name);

	static void CreateRandomWorld(UBurinWorld* world);
	static void CreateHistoricalWorld(UBurinWorld* world);
};
