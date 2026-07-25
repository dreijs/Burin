// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/Earth/UTerrainDataEntry.h"
#include "Data/Earth/UTerrain.h"
#include "Data/History/Polities/UPolities.h"
#include "Data/History/Places/UPlaces.h"
#include "Concepts/Provinces/UProvince.h"
#include "Concepts/Provinces/UArea.h"
#include "UWorldCreatorSettings.h"
#include "Display/UMapLowZoom.h"

#include "UBurinWorld.generated.h"

UCLASS(Blueprintable)
class BURIN_API UBurinWorld : public UObject
{
	GENERATED_BODY()

public:
	UBurinWorld();
	~UBurinWorld();

	UTerrain* Terrain;
	UPolities* HistoricalPolities;
	UPlaces* HistoricalPlaces;

	UWorldCreatorSettings* Settings;

	UMapLowZoom* MapLowZoom;

	TArray<UArea> Areas;
	TArray<UProvince> Provinces;
	TArray<UPolity> Polities;

	UFUNCTION(BlueprintCallable, Category = "Initialization")
	void Initialize();

	UFUNCTION(BlueprintCallable, Category = "Initialization")
	void InitializeProvinces();

	UFUNCTION(BlueprintCallable, Category = "Initialization")
	void SetWorldCreatorSettings();


	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	int GetTerrainDataAtCoordinate(int zoomCategory, double x, double y);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	int GetTriangleIDAtCoordinate(int zoomCategory, double x, double y);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	FString GetTerrainText(int v);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FCanvasUVTri> GetTriangles(int mode, int zoomCategory, double minLat, double minLon, double maxLat, double maxLon, int width, int height);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FCanvasUVTri> GetMaterialTriangles(int mode, int zoomCategory, int x, int y);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FLineDisplayData> GetBorders(int mode, int zoomCategory, int x, int y);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FLineDisplayData> GetRivers(int mode, int zoomCategory, int x, int y);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FCanvasUVTri> GetProvinceTriangles(int mode, int zoomCategory, int x, int y);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<int> GetSubregionIndices(int zoomCategory, double lat, double lon, double latDelta, double lonDelta);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	int GetNumSubregions(int zoomCategory, bool isX);

private:
	void initializeTerrain();
	void initializeHistory();
	void initializeMap();
	
};
