// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPlaceDataEntry.h"

class BURIN_API FPlaces
{
public:

	TArray<FPlaceDataEntry> HistoricalPlaceData;

	void InitializeHistoricalPlaces();

private:
	TMap<FString, int32> HistoricalPlaceMap;
};
