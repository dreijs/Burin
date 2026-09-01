// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/Canvas.h"
#include "Data/Earth/FTerrainDataEntry.h"
#include "Data/Earth/FTerrain.h"
#include "Data/History/Polities/FPolities.h"
#include "Data/History/Places/FPlaces.h"
#include "Concepts/Polities/FPolity.h"
#include "Concepts/Provinces/FPlace.h"
#include "Concepts/Provinces/FArea.h"
#include "UWorldCreatorSettings.h"
#include "Display/FLineDisplayData.h"
#include "Display/FMapLowZoom.h"

#include "UBurinWorld.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentYearChanged, int32, NewYear);

UCLASS(Blueprintable)
class BURIN_API UBurinWorld : public UObject
{
	GENERATED_BODY()

public:
	// Created once by Initialize(), released when this object is garbage collected.
	TUniquePtr<FTerrain> Terrain;
	TUniquePtr<FPolities> HistoricalPolities;
	TUniquePtr<FPlaces> HistoricalPlaces;
	TUniquePtr<FMapLowZoom> MapLowZoom;

	UPROPERTY()
	TObjectPtr<UWorldCreatorSettings> Settings = nullptr;

	TArray<FArea> Areas;
	TArray<FPlace> Places;
	TArray<FPolity> Polities;

	// The year Places/Polities were last loaded for. Set by SetCurrentYear().
	int32 CurrentYear = 0;

	UFUNCTION(BlueprintPure, Category = "Initialization")
	bool IsInitialized() const;

	UFUNCTION(BlueprintCallable, Category = "Initialization")
	void Initialize();

	UFUNCTION(BlueprintCallable, Category = "Initialization")
	void InitializeProvinces();

	UFUNCTION(BlueprintPure, Category = "History")
	int32 GetCurrentYear() const { return CurrentYear; }

	/** Resets Places and Polities, then reloads them from HistoricalPlaceData/HistoricalPolityData, keeping only the entries that exist in `year`. Broadcasts OnCurrentYearChanged afterward. */
	UFUNCTION(BlueprintCallable, Category = "History")
	void SetCurrentYear(int32 year);

	/** Fired at the end of SetCurrentYear(), after Places/Polities have been rebuilt. Anything that renders them (world sphere, world map, marker layers, ...) should bind here and refresh itself, rather than being called manually by whoever triggered the year change. */
	UPROPERTY(BlueprintAssignable, Category = "History")
	FOnCurrentYearChanged OnCurrentYearChanged;

	UFUNCTION(BlueprintCallable, Category = "Initialization")
	void SetWorldCreatorSettings();


	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	int32 GetTerrainDataAtCoordinate(int32 zoomCategory, double x, double y);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	int32 GetTriangleIDAtCoordinate(int32 zoomCategory, double x, double y);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	FString GetTerrainText(int32 v);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FCanvasUVTri> GetTriangles(int32 mode, int32 zoomCategory, double minLat, double minLon, double maxLat, double maxLon, int32 offsetX, int32 offsetY, int32 width, int32 height);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FCanvasUVTri> GetMaterialTriangles(int32 mode, int32 zoomCategory, int32 x, int32 y);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FLineDisplayData> GetBorders(int32 mode, int32 zoomCategory, int32 x, int32 y);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FLineDisplayData> GetRivers(int32 mode, int32 zoomCategory, int32 x, int32 y);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FCanvasUVTri> GetProvinceTriangles(int32 mode, int32 zoomCategory, int32 x, int32 y);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<int32> GetSubregionIndices(int32 zoomCategory, double lat, double lon, double latDelta, double lonDelta);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	int32 GetNumSubregions(int32 zoomCategory, bool isX);

private:
	// Logs and returns false if the accessors below are called before Initialize().
	bool EnsureInitialized(const TCHAR* callerName) const;

	void InitializeTerrain();
	void InitializeHistory();
	void InitializeMap();

};
