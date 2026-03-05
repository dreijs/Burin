// Fill out your copyright notice in the Description page of Project Settings.

#include "UPolities.h"
#include "UPolityDataEntry.h"
#include <Burin/UUtils.h>

UPolities::UPolities() {

}

UPolities::~UPolities() {

}

void UPolities::InitializePolities() {
	polityMap = {};
	PolityData = {};

	FString fPath = FPaths::ProjectContentDir() + TEXT("Data/History/PolityData.xml");

	TArray<FString> take;
	FFileHelper::LoadANSITextFileToStrings(*fPath, NULL, take);

	UPolityDataEntry entry;

	for (int i = 0; i < take.Num(); i++) {
		FString aString = take[i];

		if (aString.Contains("<polity>")) {
			entry = {};
		}
		if (aString.Contains("<name>")) { entry.name = UUtils::ExtractStringFromXMLContentLine(aString); }
		if (aString.Contains("<mapcolor1>")) { entry.mapcolor1 = UUtils::ExtractUInt8ArrayFromXMLContentLine(aString); }
		if (aString.Contains("<mapcolor2>")) { entry.mapcolor2 = UUtils::ExtractUInt8ArrayFromXMLContentLine(aString); }
		if (aString.Contains("<mapcolor3>")) { entry.mapcolor3 = UUtils::ExtractUInt8ArrayFromXMLContentLine(aString); }
		if (aString.Contains("<history>")) { entry.history = {}; }
		if (aString.Contains("</polity>")) {
			polityMap.Add(entry.name, PolityData.Num());
			PolityData.Add(entry);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Number of polity data entries: %d"), PolityData.Num());
}