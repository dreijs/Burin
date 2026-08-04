// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FTerrainDataEntry.h"
#include "FGeographicLabel.h"
#include "FColorDataEntry.h"

/**
 *
 */
class BURIN_API FTerrain
{
public:

	TArray<FTerrainDataEntry> TerrainData;
	TArray<FColorDataEntry> ElevationData;
	TArray<FColorDataEntry> VegetationData;
	TArray<FColorDataEntry> SoilData;
	TArray<FColorDataEntry> FeatureData;
	TArray<FGeographicLabel> GeographicLabelData;

	void InitializeTerrain();
	void InitializeElevation();
	void InitializeVegetation();
	void InitializeSoil();
	void InitializeFeatures();
	void InitializeGeographicLabels();

	void InitializeTerrainMapping();

	int32 GetTerrain(int32 terrainCode);
	int32 GetTerrainFromCache(int32 terrainCode);
	TArray<uint8> GetColor(int32 terrainCode, int32 mode);
	FString GetTerrainText(int32 idx);

	private:
		TArray<int32> TerrainMap;

		TArray<uint8> GetDisplayColor0(int32 idx);
		TArray<int32> ExtractTerrainArray(const FString& aString);
};
