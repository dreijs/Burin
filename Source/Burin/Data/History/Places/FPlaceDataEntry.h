// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPlaceHistoryItemEntry.h"

struct FPlaceDataEntry
{
	FString Name;
	FString CommonName;
	double Latitude = 0.0;
	double Longitude = 0.0;

	TArray<FPlaceHistoryItemEntry> History;
};
