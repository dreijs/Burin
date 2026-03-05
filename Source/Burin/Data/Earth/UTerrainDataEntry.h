// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class BURIN_API UTerrainDataEntry
{
public:
	UTerrainDataEntry();
	~UTerrainDataEntry();

	FString name;
	TArray<uint8_t> color;
	TArray<TArray<TArray<int>>> conditions;
};
