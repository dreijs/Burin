// Fill out your copyright notice in the Description page of Project Settings.

#include "UTerrain.h"
#include "UTerrainDataEntry.h"

UTerrain::UTerrain() {

}

UTerrain::~UTerrain() {

}

static bool isEquivalentName(FString a, FString b) {
	return a.ToUpper().Equals(b.ToUpper());
}

static FString extractString(FString aString) {
	TArray<FString> stringArray1 = {}, stringArray2 = {};
	aString.ParseIntoArray(stringArray1, TEXT(">"), false);
	if (stringArray1.Num() >= 2) {
		stringArray1[1].ParseIntoArray(stringArray2, TEXT("<"), false);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Bad terrain element string: %s"), *aString);
		return "";
	}
	return stringArray2[0];
}

void UTerrain::InitializeTerrainMapping() {
	int n = 16 * 16 * 16 * 16;
	terrainMap.SetNum(n);
	for (int i = 0; i < n; i++) {
		terrainMap[i] = GetTerrain(i);
	}
}

TArray<int> UTerrain::extractTerrainArray(FString aString) {
	FString s = extractString(aString);
	TArray<FString> stringArray1 = {};
	s.ParseIntoArray(stringArray1, TEXT(","), false);

	int k = -1, val = -1;
	if (stringArray1.Num() >= 1) {
		if (isEquivalentName(stringArray1[0], "elevation")) {
			k = 0;
			for (int i = 0; i < ElevationData.Num(); i++) {
				if (isEquivalentName(ElevationData[i].name, stringArray1[1])) { val = i; }
			}
		}
		if (isEquivalentName(stringArray1[0], "vegetation")) {
			k = 1;
			for (int i = 0; i < VegetationData.Num(); i++) {
				if (isEquivalentName(VegetationData[i].name, stringArray1[1])) { val = i; }
			}
		}
		if (isEquivalentName(stringArray1[0], "soil")) {
			k = 2;
			for (int i = 0; i < SoilData.Num(); i++) {
				if (isEquivalentName(SoilData[i].name, stringArray1[1])) { val = i; }
			}
		}
		if (isEquivalentName(stringArray1[0], "feature")) {
			k = 3;
			for (int i = 0; i < FeatureData.Num(); i++) {
				if (isEquivalentName(FeatureData[i].name, stringArray1[1])) { val = i; }
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
void UTerrain::InitializeElevation() {
	ElevationData = {};

	FString fPath = FPaths::ProjectContentDir() + TEXT("Data/Earth/ElevationData.xml");

	TArray<FString> take;
	FFileHelper::LoadANSITextFileToStrings(*fPath, NULL, take);

	UColorDataEntry entry;
	int r = 0, g = 0, b = 0;
	for (int i = 0; i < take.Num(); i++) {
		FString aString = take[i];

		if (aString.Contains("<terraintype>")) {
			entry = {};
		}
		if (aString.Contains("<name>")) { entry.name = extractString(aString); }
		if (aString.Contains("<r>")) { r = FCString::Atoi(*extractString(aString)); }
		if (aString.Contains("<g>")) { g = FCString::Atoi(*extractString(aString)); }
		if (aString.Contains("<b>")) { b = FCString::Atoi(*extractString(aString)); }
		if (aString.Contains("</terraintype>")) {
			entry.color = { r, g, b };
			ElevationData.Add(entry);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Number of elevation data entries: %d"), ElevationData.Num());
}

void UTerrain::InitializeVegetation() {
	VegetationData = {};

	FString fPath = FPaths::ProjectContentDir() + TEXT("Data/Earth/VegetationData.xml");

	TArray<FString> take;
	FFileHelper::LoadANSITextFileToStrings(*fPath, NULL, take);

	UColorDataEntry entry;
	int r = 0, g = 0, b = 0;
	for (int i = 0; i < take.Num(); i++) {
		FString aString = take[i];

		if (aString.Contains("<terraintype>")) {
			entry = {};
		}
		if (aString.Contains("<name>")) { entry.name = extractString(aString); }
		if (aString.Contains("<r>")) { r = FCString::Atoi(*extractString(aString)); }
		if (aString.Contains("<g>")) { g = FCString::Atoi(*extractString(aString)); }
		if (aString.Contains("<b>")) { b = FCString::Atoi(*extractString(aString)); }
		if (aString.Contains("</terraintype>")) {
			entry.color = { r, g, b };
			VegetationData.Add(entry);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Number of vegetation data entries: %d"), VegetationData.Num());
}

void UTerrain::InitializeSoil() {
	SoilData = {};

	FString fPath = FPaths::ProjectContentDir() + TEXT("Data/Earth/SoilData.xml");

	TArray<FString> take;
	FFileHelper::LoadANSITextFileToStrings(*fPath, NULL, take);

	UColorDataEntry entry;
	int r = 0, g = 0, b = 0;
	for (int i = 0; i < take.Num(); i++) {
		FString aString = take[i];

		if (aString.Contains("<terraintype>")) {
			entry = {};
		}
		if (aString.Contains("<name>")) { entry.name = extractString(aString); }
		if (aString.Contains("<r>")) { r = FCString::Atoi(*extractString(aString)); }
		if (aString.Contains("<g>")) { g = FCString::Atoi(*extractString(aString)); }
		if (aString.Contains("<b>")) { b = FCString::Atoi(*extractString(aString)); }
		if (aString.Contains("</terraintype>")) {
			entry.color = { r, g, b };
			SoilData.Add(entry);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Number of soil data entries: %d"), SoilData.Num());
}

void UTerrain::InitializeFeatures() {
	FeatureData = {};

	FString fPath = FPaths::ProjectContentDir() + TEXT("Data/Earth/FeatureData.xml");

	TArray<FString> take;
	FFileHelper::LoadANSITextFileToStrings(*fPath, NULL, take);

	UColorDataEntry entry;
	int r = 0, g = 0, b = 0;
	for (int i = 0; i < take.Num(); i++) {
		FString aString = take[i];

		if (aString.Contains("<terraintype>")) {
			entry = {};
		}
		if (aString.Contains("<name>")) { entry.name = extractString(aString); }
		if (aString.Contains("<r>")) { r = FCString::Atoi(*extractString(aString)); }
		if (aString.Contains("<g>")) { g = FCString::Atoi(*extractString(aString)); }
		if (aString.Contains("<b>")) { b = FCString::Atoi(*extractString(aString)); }
		if (aString.Contains("</terraintype>")) {
			entry.color = { r, g, b };
			FeatureData.Add(entry);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Number of feature data entries: %d"), FeatureData.Num());
}

void UTerrain::InitializeTerrain() {
	TerrainData = {};

	FString fPath = FPaths::ProjectContentDir() + TEXT("Data/Earth/TerrainData.xml");

	TArray<FString> take;
	FFileHelper::LoadANSITextFileToStrings(*fPath, NULL, take);

	UTerrainDataEntry entry;
	int r = 0, g = 0, b = 0;
	TArray<TArray<TArray<int>>> conditions = {};
	TArray<TArray<int>> condition = {};
	for (int i = 0; i < take.Num(); i++) {
		FString aString = take[i];

		if (aString.Contains("<terraintype>")) {
			entry = {};
		}
		if (aString.Contains("<name>")) { entry.name = extractString(aString); }
		if (aString.Contains("<r>")) { r = FCString::Atoi(*extractString(aString)); }
		if (aString.Contains("<g>")) { g = FCString::Atoi(*extractString(aString)); }
		if (aString.Contains("<b>")) { b = FCString::Atoi(*extractString(aString)); }
		if (aString.Contains("</terraintype>")) {
			entry.color = { r, g, b };
			TerrainData.Add(entry);
		}
		if (aString.Contains("<conditions>")) {conditions = {};}
		if (aString.Contains("<condition>")) {condition = {};}
		if (aString.Contains("<conditionelement>")) { 
			TArray<int> element = extractTerrainArray(aString);
			condition.Add(element);
		}
		if (aString.Contains("</condition>")) { conditions.Add(condition); }
		if (aString.Contains("</conditions>")) { entry.conditions = conditions;  }
	}

	UE_LOG(LogTemp, Log, TEXT("Number of terrain data entries: %d"), TerrainData.Num());
}

TArray<int> UTerrain::getDisplayColor0(int idx) {
	if (idx >= 0 && idx <= TerrainData.Num()) return TerrainData[idx].color;
	UE_LOG(LogTemp, Error, TEXT("Unregonized terrain idx: %d"), idx);
	return { 0, 255, 0 };
}

FString UTerrain::GetTerrainText(int idx) {
	if (idx >= 0 && idx <= TerrainData.Num()) return TerrainData[idx].name;
	UE_LOG(LogTemp, Error, TEXT("Unregonized terrain idx: %d"), idx);
	return "";
}

int UTerrain::GetTerrainFromCache(int terrainData) {
	return terrainMap[terrainData];
}

int UTerrain::GetTerrain(int terrainData) {
	int elevation = terrainData % 16;
	int vegetation = (terrainData / 16) % 16;
	int soil = (terrainData / 256) % 16;
	int feature = (terrainData / 4096) % 16;

	for (int i = 0; i < TerrainData.Num(); i++) {
		bool success = TerrainData[i].conditions.Num() > 0;
		for (int j = 0; j < TerrainData[i].conditions.Num(); j++) {
			bool condsuccess = false;
			for (int k = 0; k < TerrainData[i].conditions[j].Num(); k++) {
				if (TerrainData[i].conditions[j][k][0] == 0 && elevation == TerrainData[i].conditions[j][k][1]) condsuccess = true;
				else if (TerrainData[i].conditions[j][k][0] == 1 && vegetation == TerrainData[i].conditions[j][k][1]) condsuccess = true;
				else if (TerrainData[i].conditions[j][k][0] == 2 && soil == TerrainData[i].conditions[j][k][1]) condsuccess = true;
				else if (TerrainData[i].conditions[j][k][0] == 3 && feature == TerrainData[i].conditions[j][k][1]) condsuccess = true;
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

TArray<int> UTerrain::GetColor(int terrainData, int mode) {
	if (mode == 0) {
		int idx = GetTerrain(terrainData);
		if (idx >= 0) { 
			return getDisplayColor0(idx); 
		}
		else {
			UE_LOG(LogTemp, Error, TEXT("Unrecognized terrain: %d"), terrainData);
		}
	}

	int elevation = terrainData % 16;
	int vegetation = (terrainData / 16) % 16;
	int soil = (terrainData / 256) % 16;
	int feature = (terrainData / 4096) % 16;

	if (mode == 1) {
		return ElevationData[elevation].color;
	}
	if (mode == 2) {
		if (elevation == 0) return ElevationData[0].color;
		return VegetationData[vegetation].color;
	}
	if (mode == 3) {
		if (elevation == 0) return ElevationData[0].color;
		return SoilData[soil].color;
	}
	if (mode == 4) {
		return FeatureData[feature].color;
	}

	return { 0, 0, 0 };
}