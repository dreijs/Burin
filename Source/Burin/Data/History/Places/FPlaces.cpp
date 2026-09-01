// Fill out your copyright notice in the Description page of Project Settings.

#include "FPlaces.h"
#include "FPlaceDataEntry.h"
#include "Burin/UUtils.h"
#include "Burin/Data/History/Polities/FPolities.h"

TArray<FPlaceHistoryItemEntry> processHistory(const TArray<FString>& strings, const FPolities& polities) {
	TArray<FPlaceHistoryItemEntry> result = {};
	FPlaceHistoryItemEntry entry;

	for (int32 i = 0; i < strings.Num(); i++) {
		const FString& aString = strings[i];
		if (aString.Contains("<item>")) {
			entry = {};
		}

		if (aString.Contains("<year>")) { entry.year = UUtils::ExtractIntFromXMLContentLine(aString); }
		if (aString.Contains("<destroyed>")) { entry.destroyed = UUtils::ExtractBoolFromXMLContentLine(aString); }
		if (aString.Contains("<population>")) { entry.population = UUtils::ExtractIntFromXMLContentLine(aString); }
		if (aString.Contains("<owner>")) {
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
		}
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
