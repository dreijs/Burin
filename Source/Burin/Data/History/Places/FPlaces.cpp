// Fill out your copyright notice in the Description page of Project Settings.

#include "FPlaces.h"
#include "FPlaceDataEntry.h"
#include "Burin/UUtils.h"
#include "Burin/Data/History/Polities/FPolities.h"

TArray<FPlaceHistoryItemEntry> processHistory(const TArray<FString>& strings, const FPolities& polities) {
	TArray<FPlaceHistoryItemEntry> result = {};
	FPlaceHistoryItemEntry entry;

	// An item that carries no <owner> is not an ownership change: <template> items name a cultural
	// or economic modifier ("greek dominance", "vegetable oil industry"), and the place carries on
	// under whoever held it. Left unset, Owner stayed empty, which resolved to no polity at all --
	// so the place existed but had no controller, and its domain went unpainted. Which items said
	// nothing is recorded here and resolved once the whole history is parsed.
	TArray<bool> bDeclaredOwner;
	bool bItemDeclaredOwner = false;

	for (int32 i = 0; i < strings.Num(); i++) {
		const FString& aString = strings[i];
		if (aString.Contains("<item>")) {
			entry = {};
			bItemDeclaredOwner = false;
		}

		if (aString.Contains("<year>")) { entry.year = UUtils::ExtractIntFromXMLContentLine(aString); }
		if (aString.Contains("<destroyed>")) { entry.destroyed = UUtils::ExtractBoolFromXMLContentLine(aString); }
		if (aString.Contains("<population>")) { entry.population = UUtils::ExtractIntFromXMLContentLine(aString); }
		if (aString.Contains("<owner>")) {
			bItemDeclaredOwner = true;
			entry.Owner = UUtils::ExtractStringFromXMLContentLine(aString);

			// "null" (unclaimed/founding placeholder) and "ruined" (destroyed) are sentinels,
			// not real polities, so they're not expected to resolve.
			if (entry.Owner.Equals(TEXT("null"), ESearchCase::IgnoreCase) || entry.Owner.Equals(TEXT("ruined"), ESearchCase::IgnoreCase)) {
				entry.OwnerPolityIndex = INDEX_NONE;
			} else {
				entry.OwnerPolityIndex = polities.FindPolityIndex(entry.Owner);
				if (entry.OwnerPolityIndex == INDEX_NONE) {
					UE_LOG(LogTemp, Warning, TEXT("Place history references unknown owner polity: %s"), *entry.Owner);
				}
			}
		}


		if (aString.Contains("</item>")) {
			result.Add(entry);
			bDeclaredOwner.Add(bItemDeclaredOwner);
		}
	}

	// Inheritance follows time, not file position: 11 of the 981 places list their items out of
	// chronological order, and for 9 of those items the two disagree -- Be'er Sheva's -550 item sits
	// after Eber-Nari in the file but after Babylon in time, and it is Babylon that held the place.
	TArray<int32> chronological;
	chronological.Reserve(result.Num());
	for (int32 i = 0; i < result.Num(); i++) {
		chronological.Add(i);
	}
	chronological.Sort([&result](int32 a, int32 b) {
		if (result[a].year != result[b].year) return result[a].year < result[b].year;
		return a < b; // items sharing a year keep their file order, so the sort is deterministic
	});

	FString inheritedOwner;
	int32 inheritedPolityIndex = INDEX_NONE;
	bool bHaveOwnerToInherit = false;

	for (int32 index : chronological) {
		if (bDeclaredOwner[index]) {
			inheritedOwner = result[index].Owner;
			inheritedPolityIndex = result[index].OwnerPolityIndex;
			bHaveOwnerToInherit = true;
			continue;
		}

		if (!bHaveOwnerToInherit) {
			// Nothing precedes it in time, so there is no owner to carry forward. Only Kalos Limen
			// hits this, because its -450 item is listed after its -350 one.
			UE_LOG(LogTemp, Warning, TEXT("Place history's earliest item (year %d) carries no owner, so it has no controller."), result[index].year);
			continue;
		}

		// A run of consecutive items without an owner all inherit the last one that declared it.
		result[index].Owner = inheritedOwner;
		result[index].OwnerPolityIndex = inheritedPolityIndex;
	}

	return result;
}

void FPlaces::InitializeHistoricalPlaces(const FPolities& polities) {
	HistoricalPlaceData = {};
	HistoricalPlaceMap = {};

	FString fPath = FPaths::ProjectContentDir() + TEXT("Data/History/PlaceData.xml");

	TArray<FString> take;
	if (!FFileHelper::LoadANSITextFileToStrings(*fPath, NULL, take)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to load place data file: %s"), *fPath);
		return;
	}

	FPlaceDataEntry entry;
	bool history = false;
	TArray<FString> historyString = {};

	for (int32 i = 0; i < take.Num(); i++) {
		const FString& aString = take[i];

		if (aString.Contains("</history>")) { entry.History = processHistory(historyString, polities); history = false; historyString = {}; }
		if (history) {
			historyString.Add(aString);
		} else {
			if (aString.Contains("<place>")) {
				entry = {};
			}
			if (aString.Contains("<name>")) { entry.Name = UUtils::ExtractStringFromXMLContentLine(aString); }
			if (aString.Contains("<commonname>")) { entry.CommonName = UUtils::ExtractStringFromXMLContentLine(aString); }
			if (aString.Contains("<latitude>")) {
				entry.Latitude = UUtils::ExtractDoubleFromXMLContentLine(aString);
			}
			if (aString.Contains("<longitude>")) {
				entry.Longitude = UUtils::ExtractDoubleFromXMLContentLine(aString);
			}
			//if (aString.Contains("<mapcolor1>")) { entry.MapColor1 = UUtils::ExtractUInt8ArrayFromXMLContentLine(aString); }
			//if (aString.Contains("<mapcolor2>")) { entry.MapColor2 = UUtils::ExtractUInt8ArrayFromXMLContentLine(aString); }
			//if (aString.Contains("<mapcolor3>")) { entry.MapColor3 = UUtils::ExtractUInt8ArrayFromXMLContentLine(aString); }

			if (aString.Contains("</place>")) {
				if (const int32* existingIndex = HistoricalPlaceMap.Find(entry.Name)) {
					UE_LOG(LogTemp, Warning, TEXT("Duplicate place name in place data, overwriting earlier entry: %s"), *entry.Name);
					HistoricalPlaceData[*existingIndex] = entry;
				} else {
					HistoricalPlaceMap.Add(entry.Name, HistoricalPlaceData.Num());
					HistoricalPlaceData.Add(entry);
				}
			}
		}
		if (aString.Contains("<history>")) { history = true; }
	}

	UE_LOG(LogTemp, Log, TEXT("Number of place data entries: %d"), HistoricalPlaceData.Num());
}
