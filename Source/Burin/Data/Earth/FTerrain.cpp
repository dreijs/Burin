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

	// Both colour blocks hold a bare <rgb>, so which one a line belongs to is only knowable from
	// the block it sits in. Tracked rather than guessed from order -- an entry that gained a
	// <photocolor> but lost its <displaycolor0> would otherwise silently take the photographic
	// colour as its map colour.
	bool bInPhotoColor = false;

	for (int32 i = 0; i < take.Num(); i++) {
		FString aString = take[i];

		if (aString.Contains("<terraintype>")) {
			entry = {};
			bInPhotoColor = false;
		}
		if (aString.Contains("<photocolor>")) { bInPhotoColor = true; }
		if (aString.Contains("</photocolor>")) { bInPhotoColor = false; }
		if (aString.Contains("<name>")) { entry.Name = UUtils::ExtractStringFromXMLContentLine(aString); }
		if (aString.Contains("<rgb>")) {
			if (bInPhotoColor) entry.PhotoColor = UUtils::ExtractUInt8ArrayFromXMLContentLine(aString);
			else entry.Color = UUtils::ExtractUInt8ArrayFromXMLContentLine(aString);
		}
		if (aString.Contains("</terraintype>")) {
			if (entry.PhotoColor.Num() < 3) {
				entry.PhotoColor = entry.Color; // no photographic colour authored yet
			}
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

TArray<uint8> FTerrain::GetPhotoColor(int32 idx) {
	if (idx >= 0 && idx < TerrainData.Num()) return TerrainData[idx].PhotoColor;
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

// Which feature names a body of water rather than a landform.
//
// By name and not by index, so that inserting a feature into FeatureData.xml cannot silently
// renumber the test into meaning something else. Icecap is deliberately not here: it sits on land
// in Greenland and on water in an ice shelf, so neither answer would be right for it.
static bool IsWaterFeature(const FString& name) {
	return name.Equals(TEXT("Ocean"), ESearchCase::IgnoreCase)
		|| name.Equals(TEXT("Sea"), ESearchCase::IgnoreCase)
		|| name.Equals(TEXT("Quiet Sea"), ESearchCase::IgnoreCase)
		|| name.Equals(TEXT("Lake"), ESearchCase::IgnoreCase)
		|| name.Equals(TEXT("Icy Water"), ESearchCase::IgnoreCase);
}

TArray<uint8> FTerrain::GetColor(int32 terrainCode, int32 mode) {
	// Mode 0 is the photographic globe and mode 1 the readable map of the same data. They used to
	// share one palette, which meant neither could be right: the contrast that makes a biome
	// legible is exactly what makes it look drawn.
	if (mode == 0 || mode == 1) {
		int32 idx = GetTerrain(terrainCode);
		if (idx >= 0) {
			return (mode == 0) ? GetPhotoColor(idx) : GetDisplayColor0(idx);
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
		// Water and its feature are two different questions, and the earlier rule answered only one.
		// Colouring every water pixel alike did stop a stray ocean seed in the Black Sea from
		// painting a four-degree blob across the Caucasus -- but it did so by throwing away the
		// distinction between ocean, sea, quiet sea and lake, which in a feature map is most of what
		// there is to see. Ask both questions instead.
		const bool bWater = (elevation == 0);
		const bool bWaterFeature = FeatureData.IsValidIndex(feature) && IsWaterFeature(FeatureData[feature].Name);

		if (bWater) {
			// Water keeps its own kind. Water with a landform feature is meaningless, so it falls
			// back to plain water rather than being drawn as hills.
			return bWaterFeature ? FeatureData[feature].Color : ElevationData[0].Color;
		}

		// Land carrying a water feature is the seeding mistake the old rule was hiding. It stays
		// hidden no longer -- but it is drawn as featureless land rather than as sea, so a bad seed
		// cannot masquerade as a lake.
		return bWaterFeature ? FeatureData[0].Color : FeatureData[feature].Color;
	}
	if (mode == 7) {
		if (elevation == 0) return {128, 192, 255};
		return { 255, 255, 224 };
	}

	return { 0, 0, 0 };
}
