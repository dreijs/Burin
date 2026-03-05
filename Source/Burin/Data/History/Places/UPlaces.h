// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UPlaceDataEntry.h"

class BURIN_API UPlaces
{
public:

	UPlaces();
	~UPlaces();

	TArray<UPlaceDataEntry> PlaceData;

	void InitializePlaces();

private:
	TMap<FString, int32> placeMap;
};
