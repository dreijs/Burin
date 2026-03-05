// Fill out your copyright notice in the Description page of Project Settings.

#include "UPlaces.h"
#include "UPlaceDataEntry.h"
#include <Burin/UUtils.h>

UPlaces::UPlaces() {

}

UPlaces::~UPlaces() {

}

void UPlaces::InitializePlaces() {
	PlaceData = {};
	placeMap = {};

	FString fPath = FPaths::ProjectContentDir() + TEXT("Data/History/PlaceData.xml");

	TArray<FString> take;
	FFileHelper::LoadANSITextFileToStrings(*fPath, NULL, take);

	UPlaceDataEntry entry;

	for (int i = 0; i < take.Num(); i++) {
		FString aString = take[i];

		if (aString.Contains("<place>")) {
			entry = {};
		}
		if (aString.Contains("<name>")) { entry.name = UUtils::ExtractStringFromXMLContentLine(aString); }
		if (aString.Contains("<commonname>")) { entry.commonName = UUtils::ExtractStringFromXMLContentLine(aString); }
		if (aString.Contains("<location>")) { 
			TArray location = UUtils::ExtractFloatArrayFromXMLContentLine(aString); 
			entry.latitude = location[0];
			entry.longitude = location[1];
		}
		//if (aString.Contains("<mapcolor1>")) { entry.mapcolor1 = UUtils::ExtractUInt8ArrayFromXMLContentLine(aString); }
		//if (aString.Contains("<mapcolor2>")) { entry.mapcolor2 = UUtils::ExtractUInt8ArrayFromXMLContentLine(aString); }
		//if (aString.Contains("<mapcolor3>")) { entry.mapcolor3 = UUtils::ExtractUInt8ArrayFromXMLContentLine(aString); }
		//if (aString.Contains("<history>")) { entry.history = {}; }
		if (aString.Contains("</place>")) {
			placeMap.Add(entry.name, PlaceData.Num());
			PlaceData.Add(entry);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Number of place data entries: %d"), PlaceData.Num());
}