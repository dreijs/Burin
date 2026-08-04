// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPolityDataEntry.h"

class BURIN_API FPolities
{
public:

	TArray<FPolityDataEntry> HistoricalPolityData;

	void InitializeHistoricalPolities();

private:
	TMap<FString, int32> HistoricalPolityMap;
};
