// Fill out your copyright notice in the Description page of Project Settings.

#include "FFlags.h"

void FFlags::InitializeFlags() {
	FlagMap = {};
	FlagData = {};

	FString fPath = FPaths::ProjectContentDir() + TEXT("Data/Earth/ElevationData.xml");

	TArray<FString> take;
	FFileHelper::LoadANSITextFileToStrings(*fPath, NULL, take);

	FFlagDataEntry entry;

	for (int32 i = 0; i < take.Num(); i++) {
		FString aString = take[i];

		if (aString.Contains("<polity>")) {
			entry = {};
		}
		//if (aString.Contains("<name>")) { entry.Name = extractString(aString); }
		//if (aString.Contains("<r>")) { r = FCString::Atoi(*extractString(aString)); }
		//if (aString.Contains("<g>")) { g = FCString::Atoi(*extractString(aString)); }
		//if (aString.Contains("<b>")) { b = FCString::Atoi(*extractString(aString)); }
		//if (aString.Contains("</terraintype>")) {
		//	entry.Color = { r, g, b };
		//	PlaceData.Add(entry);
		//}
	}

	UE_LOG(LogTemp, Log, TEXT("Number of polity data entries: %d"), FlagData.Num());
}
