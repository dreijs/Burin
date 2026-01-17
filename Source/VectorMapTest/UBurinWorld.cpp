// Fill out your copyright notice in the Description page of Project Settings.


#include "UBurinWorld.h"
#include "Data/Earth/UTerrainDataEntry.h"

UBurinWorld::UBurinWorld()
{
}

UBurinWorld::~UBurinWorld()
{
}

FString extractData(FString aString) {
	TArray<FString> stringArray1 = {}, stringArray2 = {};
	aString.ParseIntoArray(stringArray1, TEXT(">"), false);
	stringArray1[1].ParseIntoArray(stringArray2, TEXT(">"), false);
	return stringArray2[0];
}

void UBurinWorld::InitializeTerrain()
{
	TerrainData = {};

	FString fPath = FPaths::ProjectContentDir() + TEXT("Data/TerrainData.txt");

	TArray<FString> take;
	FFileHelper::LoadANSITextFileToStrings(*fPath, NULL, take);

	UTerrainDataEntry entry;
	int r = 0, g = 0, b = 0;
	for (int i = 0; i < take.Num(); i++) {
		FString aString = take[i];
		
		if (aString.Contains("<terraintype>")) {
			entry = {};
		}
		if (aString.Contains("<name>")) {
			entry.name = extractData(aString);
		}
		if (aString.Contains("<r>")) {
			r = FCString::Atoi(*extractData(aString));
		}
		if (aString.Contains("<g>")) {
			g = FCString::Atoi(*extractData(aString));
		}
		if (aString.Contains("<b>")) {
			b = FCString::Atoi(*extractData(aString));
		}
		if (aString.Contains("</terraintype>")) {
			entry.displayColor0 = { r, g, b };
			TerrainData.Add(entry);
		}
	}
}

void UBurinWorld::Initialize()
{
	InitializeTerrain();
}

TArray<int> UBurinWorld::GetDisplayColor0(int idx) {
	if (idx <= TerrainData.Num()) return TerrainData[idx].displayColor0;
	return {0, 255, 0};
}
