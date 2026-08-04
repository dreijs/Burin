// Fill out your copyright notice in the Description page of Project Settings.

#include "FTerrain.h"
#include "FTerrainDataEntry.h"
#include "Burin/UUtils.h"

static bool isEquivalentName(FString a, FString b) {
	return a.ToUpper().Equals(b.ToUpper());
}

void FTerrain::InitializeTerrainMapping() {
	int32 n = 16 * 16 * 16 * 16;
	TerrainMap.SetNum(n);
	for (int32 i = 0; i < n; i++) {
		TerrainMap[i] = GetTerrain(i);
	}
}

TArray<int32> FTerrain::ExtractTerrainArray(const FString& aString) {
	FString s = UUtils::ExtractStringFromXMLContentLine(aString);
	TArray<FString> stringArray1 = {};
	s.ParseIntoArray(stringArray1, TEXT(","), false);

	int32 k = -1, val = -1;
	if (stringArray1.Num() >= 1) {
		if (isEquivalentName(stringArray1[0], "elevation")) {
			k = 0;
			for (int32 i = 0; i < ElevationData.Num(); i++) {
				if (isEquivalentName(ElevationData[i].Name, stringArray1[1])) { val = i; }
			}
		}
		if (isEquivalentName(stringArray1[0], "vegetation")) {
			k = 1;
			for (int32 i = 0; i < VegetationData.Num(); i++) {
				if (isEquivalentName(VegetationData[i].Name, stringArray1[1])) { val = i; }
			}
		}
		if (isEquivalentName(stringArray1[0], "soil")) {
			k = 2;
			for (int32 i = 0; i < SoilData.Num(); i++) {
				if (isEquivalentName(SoilData[i].Name, stringArray1[1])) { val = i; }
			}
		}
		if (isEquivalentName(stringArray1[0], "feature")) {
			k = 3;
			for (int32 i = 0; i < FeatureData.Num(); i++) {
				if (isEquivalentName(FeatureData[i].Name, stringArray1[1])) { val = i; }
			}
		}
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Bad terrain string: %s"), *aString);
	}

	if (k < 0 || val < 0) {
		UE_LOG(LogTemp, Error, TEXT("Unrecognized terrain type: %s"), *aString);
	}

	return {k, val};
}
void FTerrain::InitializeElevation() {
	ElevationData = {};

	FString fPath = FPaths::ProjectContentDir() + TEXT("Data/Earth/ElevationData.xml");

	TArray<FString> take;
	FFileHelper::LoadANSITextFileToStrings(*fPath, NULL, take);

	FColorDataEntry entry;
	for (int32 i = 0; i < take.Num(); i++) {
		FString aString = take[i];

		if (aString.Contains("<terraintype>")) {
			entry = {};
		}
		if (aString.Contains("<name>")) { entry.Name = UUtils::ExtractStringFromXMLContentLine(aString); }
		if (aString.Contains("<rgb>")) { entry.Color = UUtils::ExtractUInt8ArrayFromXMLContentLine(aString); }
		if (aString.Contains("</terraintype>")) {
			ElevationData.Add(entry);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Number of elevation data entries: %d"), ElevationData.Num());
}

void FTerrain::InitializeVegetation() {
	VegetationData = {};

	FString fPath = FPaths::ProjectContentDir() + TEXT("Data/Earth/VegetationData.xml");

	TArray<FString> take;
	FFileHelper::LoadANSITextFileToStrings(*fPath, NULL, take);

	FColorDataEntry entry;
	for (int32 i = 0; i < take.Num(); i++) {
		FString aString = take[i];

		if (aString.Contains("<terraintype>")) {
			entry = {};
		}
		if (aString.Contains("<name>")) { entry.Name = UUtils::ExtractStringFromXMLContentLine(aString); }
		if (aString.Contains("<rgb>")) { entry.Color = UUtils::ExtractUInt8ArrayFromXMLContentLine(aString); }
		if (aString.Contains("</terraintype>")) {
			VegetationData.Add(entry);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Number of vegetation data entries: %d"), VegetationData.Num());
}

void FTerrain::InitializeSoil() {
	SoilData = {};

	FString fPath = FPaths::ProjectContentDir() + TEXT("Data/Earth/SoilData.xml");

	TArray<FString> take;
	FFileHelper::LoadANSITextFileToStrings(*fPath, NULL, take);

	FColorDataEntry entry;
	for (int32 i = 0; i < take.Num(); i++) {
		FString aString = take[i];

		if (aString.Contains("<terraintype>")) {
			entry = {};
		}
		if (aString.Contains("<name>")) { entry.Name = UUtils::ExtractStringFromXMLContentLine(aString); }
		if (aString.Contains("<rgb>")) { entry.Color = UUtils::ExtractUInt8ArrayFromXMLContentLine(aString); }
		if (aString.Contains("</terraintype>")) {
			SoilData.Add(entry);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Number of soil data entries: %d"), SoilData.Num());
}

void FTerrain::InitializeFeatures() {
	FeatureData = {};

	FString fPath = FPaths::ProjectContentDir() + TEXT("Data/Earth/FeatureData.xml");

	TArray<FString> take;
	FFileHelper::LoadANSITextFileToStrings(*fPath, NULL, take);

	FColorDataEntry entry;
	for (int32 i = 0; i < take.Num(); i++) {
		FString aString = take[i];

		if (aString.Contains("<terraintype>")) {
			entry = {};
		}
		if (aString.Contains("<name>")) { entry.Name = UUtils::ExtractStringFromXMLContentLine(aString); }
		if (aString.Contains("<rgb>")) { entry.Color = UUtils::ExtractUInt8ArrayFromXMLContentLine(aString); }
		if (aString.Contains("</terraintype>")) {
			FeatureData.Add(entry);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Number of feature data entries: %d"), FeatureData.Num());
}

void FTerrain::InitializeGeographicLabels() {
	GeographicLabelData = {};

	FString fPath = FPaths::ProjectContentDir() + TEXT("Data/Earth/GeographicLabelData.xml");

	TArray<FString> take;
	FFileHelper::LoadANSITextFileToStrings(*fPath, NULL, take);

	FGeographicLabel entry;
	for (int32 i = 0; i < take.Num(); i++) {
		FString aString = take[i];

		if (aString.Contains("<feature>")) {
			entry = {};
		}
		if (aString.Contains("<name>")) { entry.Name = UUtils::ExtractStringFromXMLContentLine(aString); }
		if (aString.Contains("<latitude>")) { entry.Latitude = UUtils::ExtractDoubleFromXMLContentLine(aString); }
		if (aString.Contains("<longitude>")) { entry.Longitude = UUtils::ExtractDoubleFromXMLContentLine(aString); }
		if (aString.Contains("<type>")) { entry.Type = UUtils::ExtractStringFromXMLContentLine(aString); }
		if (aString.Contains("<importance>")) { entry.Importance = UUtils::ExtractIntFromXMLContentLine(aString); }
		if (aString.Contains("<minyear>")) { entry.MinYear = UUtils::ExtractIntFromXMLContentLine(aString); }
		if (aString.Contains("<maxyear>")) { entry.MaxYear = UUtils::ExtractIntFromXMLContentLine(aString); }
		if (aString.Contains("<rgb>")) { entry.Color = UUtils::ExtractUInt8ArrayFromXMLContentLine(aString); }
		if (aString.Contains("</feature>")) {
			GeographicLabelData.Add(entry);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Number of geographic labels: %d"), GeographicLabelData.Num());
}

void FTerrain::InitializeTerrain() {
	TerrainData = {};

	FString fPath = FPaths::ProjectContentDir() + TEXT("Data/Earth/TerrainData.xml");

	TArray<FString> take;
	FFileHelper::LoadANSITextFileToStrings(*fPath, NULL, take);

	FTerrainDataEntry entry;
	TArray<TArray<TArray<int32>>> conditions = {};
	TArray<TArray<int32>> condition = {};
	for (int32 i = 0; i < take.Num(); i++) {
		FString aString = take[i];

		if (aString.Contains("<terraintype>")) {
			entry = {};
		}
		if (aString.Contains("<name>")) { entry.Name = UUtils::ExtractStringFromXMLContentLine(aString); }
		if (aString.Contains("<rgb>")) { entry.Color = UUtils::ExtractUInt8ArrayFromXMLContentLine(aString); }
		if (aString.Contains("</terraintype>")) {
			TerrainData.Add(entry);
		}
		if (aString.Contains("<conditions>")) {conditions = {};}
		if (aString.Contains("<condition>")) {condition = {};}
		if (aString.Contains("<conditionelement>")) {
			TArray<int32> element = ExtractTerrainArray(aString);
			condition.Add(element);
		}
		if (aString.Contains("</condition>")) { conditions.Add(condition); }
		if (aString.Contains("</conditions>")) { entry.Conditions = conditions;  }
	}

	UE_LOG(LogTemp, Log, TEXT("Number of terrain data entries: %d"), TerrainData.Num());
}

TArray<uint8> FTerrain::GetDisplayColor0(int32 idx) {
	if (idx >= 0 && idx < TerrainData.Num()) return TerrainData[idx].Color;
	UE_LOG(LogTemp, Error, TEXT("Unregonized terrain idx: %d"), idx);
	return { 0, 255, 0 };
}

FString FTerrain::GetTerrainText(int32 idx) {
	if (idx >= 0 && idx < TerrainData.Num()) return TerrainData[idx].Name;
	UE_LOG(LogTemp, Error, TEXT("Unregonized terrain idx: %d"), idx);
	return "";
}

int32 FTerrain::GetTerrainFromCache(int32 terrainCode) {
	return TerrainMap[terrainCode];
}

int32 FTerrain::GetTerrain(int32 terrainCode) {
	int32 elevation = terrainCode % 16;
	int32 vegetation = (terrainCode / 16) % 16;
	int32 soil = (terrainCode / 256) % 16;
	int32 feature = (terrainCode / 4096) % 16;

	for (int32 i = 0; i < TerrainData.Num(); i++) {
		bool success = TerrainData[i].Conditions.Num() > 0;
		for (int32 j = 0; j < TerrainData[i].Conditions.Num(); j++) {
			bool condsuccess = false;
			for (int32 k = 0; k < TerrainData[i].Conditions[j].Num(); k++) {
				if (TerrainData[i].Conditions[j][k][0] == 0 && elevation == TerrainData[i].Conditions[j][k][1]) condsuccess = true;
				else if (TerrainData[i].Conditions[j][k][0] == 1 && vegetation == TerrainData[i].Conditions[j][k][1]) condsuccess = true;
				else if (TerrainData[i].Conditions[j][k][0] == 2 && soil == TerrainData[i].Conditions[j][k][1]) condsuccess = true;
				else if (TerrainData[i].Conditions[j][k][0] == 3 && feature == TerrainData[i].Conditions[j][k][1]) condsuccess = true;
			}
			if (!condsuccess) {
				success = false;
				break;
			}
		}
		if (success) return i;
	}

	return -1;
}

TArray<uint8> FTerrain::GetColor(int32 terrainCode, int32 mode) {
	if (mode == 0 || mode == 1) {
		int32 idx = GetTerrain(terrainCode);
		if (idx >= 0) {
			return GetDisplayColor0(idx);
		}
		else {
			UE_LOG(LogTemp, Error, TEXT("Unrecognized terrain: %d"), terrainCode);
		}
	}

	int32 elevation = terrainCode % 16;
	int32 vegetation = (terrainCode / 16) % 16;
	int32 soil = (terrainCode / 256) % 16;
	int32 feature = (terrainCode / 4096) % 16;

	if (mode == 2) {
		return ElevationData[elevation].Color;
	}
	if (mode == 3) {
		if (elevation == 0) return ElevationData[0].Color;
		return VegetationData[vegetation].Color;
	}
	if (mode == 4) {
		if (elevation == 0) return ElevationData[0].Color;
		return SoilData[soil].Color;
	}
	if (mode == 5) {
		return FeatureData[feature].Color;
	}
	if (mode == 7) {
		if (elevation == 0) return {128, 192, 255};
		return { 255, 255, 224 };
	}

	return { 0, 0, 0 };
}
