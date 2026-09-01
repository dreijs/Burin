// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPlaceDataEntry.h"

class FPolities;

class BURIN_API FPlaces
{
public:

	TArray<FPlaceDataEntry> HistoricalPlaceData;

	void InitializeHistoricalPlaces(const FPolities& polities);

private:
	TMap<FString, int32> HistoricalPlaceMap;
};
