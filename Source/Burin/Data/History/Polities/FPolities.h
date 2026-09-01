// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPolityDataEntry.h"

class BURIN_API FPolities
{
public:

	TArray<FPolityDataEntry> HistoricalPolityData;

	void InitializeHistoricalPolities();

	/** Index into HistoricalPolityData for the given polity id, or INDEX_NONE if there's no such polity. */
	int32 FindPolityIndex(const FString& id) const;

private:
	TMap<FString, int32> HistoricalPolityMap;
};
