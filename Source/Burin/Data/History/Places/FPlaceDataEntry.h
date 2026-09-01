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

	// The history entry in effect as of `year` (the latest entry with year <= `year`), or
	// nullptr if the place hasn't been founded yet.
	const FPlaceHistoryItemEntry* GetHistoryItemForYear(int32 year) const
	{
		const FPlaceHistoryItemEntry* current = nullptr;
		for (const FPlaceHistoryItemEntry& item : History) {
			if (item.year <= year && (current == nullptr || item.year > current->year)) {
				current = &item;
			}
		}
		return current;
	}

	// True if, as of `year`, the place has been founded and its most recent owner change at or
	// before `year` isn't "ruined". A place can be ruined and later revived by a subsequent
	// entry with a different owner, so this looks at the latest applicable entry, not just
	// whether a "ruined" entry ever occurred in the past.
	bool ExistsInYear(int32 year) const
	{
		const FPlaceHistoryItemEntry* current = GetHistoryItemForYear(year);
		return current != nullptr && !current->Owner.Equals(TEXT("ruined"), ESearchCase::IgnoreCase);
	}
};
