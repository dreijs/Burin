// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/Earth/UTerrainDataEntry.h"
#include "Data/Earth/UTerrain.h"

#include "UBurinWorld.generated.h"

UCLASS(Blueprintable)
class BURIN_API UBurinWorld : public UObject
{
	GENERATED_BODY()

public:
	UBurinWorld();
	~UBurinWorld();

	UTerrain* Terrain;

	UFUNCTION(BlueprintCallable, Category = "Initialization")
	void Initialize();

private:
	void InitializeTerrain();
};
