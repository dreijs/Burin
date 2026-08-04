// Fill out your copyright notice in the Description page of Project Settings.

#include "FPolities.h"
#include "FPolityDataEntry.h"
#include "Burin/UUtils.h"

void FPolities::InitializeHistoricalPolities() {
	HistoricalPolityMap = {};
	HistoricalPolityData = {};

	FString fPath = FPaths::ProjectContentDir() + TEXT("Data/History/PolityData.xml");

	TArray<FString> take;
	FFileHelper::LoadANSITextFileToStrings(*fPath, NULL, take);

	FPolityDataEntry entry;

	for (int32 i = 0; i < take.Num(); i++) {
		FString aString = take[i];

		if (aString.Contains("<polity>")) {
			entry = {};
		}
		if (aString.Contains("<name>")) { entry.Name = UUtils::ExtractStringFromXMLContentLine(aString); }
		if (aString.Contains("<mapcolor1>")) { entry.MapColor1 = UUtils::ExtractUInt8ArrayFromXMLContentLine(aString); }
		if (aString.Contains("<mapcolor2>")) { entry.MapColor2 = UUtils::ExtractUInt8ArrayFromXMLContentLine(aString); }
		if (aString.Contains("<mapcolor3>")) { entry.MapColor3 = UUtils::ExtractUInt8ArrayFromXMLContentLine(aString); }
		if (aString.Contains("<history>")) { entry.History = {}; }
		if (aString.Contains("</polity>")) {
			HistoricalPolityMap.Add(entry.Name, HistoricalPolityData.Num());
			HistoricalPolityData.Add(entry);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Number of polity data entries: %d"), HistoricalPolityData.Num());
}
