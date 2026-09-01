// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct FPlaceHistoryItemEntry
{
	int year = 0;
	int population = 0;
	bool destroyed = false;

	// Raw <owner> value from the source data, e.g. a polity id, or the sentinels "null"/"ruined".
	FString Owner;

	// Index into FPolities::HistoricalPolityData for Owner, or INDEX_NONE when Owner is a
	// sentinel ("null", "ruined") or doesn't match any known polity.
	int32 OwnerPolityIndex = INDEX_NONE;
};
