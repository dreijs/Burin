// Fill out your copyright notice in the Description page of Project Settings.

#include "UPlaces.h"
#include "UPlaceDataEntry.h"

UPlaces::UPlaces() {

}

UPlaces::~UPlaces() {

}

void UPlaces::InitializePlaces() {
	PlaceData = {};

	FString fPath = FPaths::ProjectContentDir() + TEXT("Data/Earth/ElevationData.xml");

	TArray<FString> take;
	FFileHelper::LoadANSITextFileToStrings(*fPath, NULL, take);

	UPlaceDataEntry entry;
	int r = 0, g = 0, b = 0;
	for (int i = 0; i < take.Num(); i++) {
		FString aString = take[i];

		if (aString.Contains("<terraintype>")) {
			entry = {};
		}
		//if (aString.Contains("<name>")) { entry.name = extractString(aString); }
		//if (aString.Contains("<r>")) { r = FCString::Atoi(*extractString(aString)); }
		//if (aString.Contains("<g>")) { g = FCString::Atoi(*extractString(aString)); }
		//if (aString.Contains("<b>")) { b = FCString::Atoi(*extractString(aString)); }
		//if (aString.Contains("</terraintype>")) {
		//	entry.color = { r, g, b };
		//	PlaceData.Add(entry);
		//}
	}

	UE_LOG(LogTemp, Log, TEXT("Number of place data entries: %d"), PlaceData.Num());
}