// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/Earth/UTerrainDataEntry.h"
#include "UBurinWorld.generated.h"

UCLASS(Blueprintable)
class VECTORMAPTEST_API UBurinWorld : public UObject
{
	GENERATED_BODY()

public:
	UBurinWorld();
	~UBurinWorld();

	TArray<UTerrainDataEntry> terrainData;

	UFUNCTION(BlueprintCallable, Category = "Initialization")
	void Initialize();

	UFUNCTION(BlueprintCallable, Category = "Data Operations")
	TArray<int> GetDisplayColor0(int idx);

private:
	TArray<UTerrainDataEntry> TerrainData;

	void InitializeTerrain();
};
