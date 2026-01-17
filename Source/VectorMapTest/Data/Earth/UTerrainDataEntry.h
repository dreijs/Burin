// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class VECTORMAPTEST_API UTerrainDataEntry
{
public:
	UTerrainDataEntry();
	~UTerrainDataEntry();

	FString name;
	TArray<int> displayColor0;
};
