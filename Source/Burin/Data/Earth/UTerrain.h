// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UTerrainDataEntry.h"
#include "UColorDataEntry.h"

/**
 *
 */
class BURIN_API UTerrain
{
public:

	UTerrain();
	~UTerrain();

	TArray<UTerrainDataEntry> TerrainData;
	TArray<UColorDataEntry> ElevationData;
	TArray<UColorDataEntry> VegetationData;
	TArray<UColorDataEntry> SoilData;
	TArray<UColorDataEntry> FeatureData;

	

	void InitializeTerrain();
	void InitializeElevation();
	void InitializeVegetation();
	void InitializeSoil();
	void InitializeFeatures();

	void InitializeTerrainMapping();

	int GetTerrain(int terrainData);
	int GetTerrainFromCache(int terrainData);
	TArray<int> GetColor(int i, int mode);
	FString GetTerrainText(int idx);

	private:
		TArray<int> terrainMap;

		TArray<int> getDisplayColor0(int idx);
		TArray<int> extractTerrainArray(FString aString);
};
