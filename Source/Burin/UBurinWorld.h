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
#include "Display/FDomainInfo.h"
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

	/** How far, in kilometres, a place's domain may reach from the place. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
	double DomainRadiusKm = 200.0;

	/**
	 * Master switch for the domain layer. GetTriangles and GetMaterialTriangles no longer draw
	 * domains at all -- the caller decides, per map mode, by drawing GetProvinceTriangles() over the
	 * terrain or leaving it out. This turns the layer off everywhere without editing each mode.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
	bool bShowPlaceDomains = true;

	/**
	 * Whether a river stops a domain. Off by default, so a river city holds both of its banks as
	 * they historically did and the border with a neighbour is simply the bisector of the two.
	 * On, a river is impassable and bounds the domain, which at level 1 means following a coarse
	 * polyline -- the Euphrates crosses the Mari basin as a single 98 km chord.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
	bool bRiversBlockDomains = false;

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

	/**
	 * The level-1 triangle a place at (latitude, longitude) should be seeded on. Unlike the
	 * RenderMap lookups below, this takes a TRUE latitude and negates it into mesh space itself.
	 *
	 * Level 1's coastline is Douglas-Peucker simplified, so coastal places routinely fall just
	 * offshore of it -- 104 of the 981 places in the data land on a water triangle. When that
	 * happens this returns the nearest land triangle instead, provided one is within
	 * maxDistanceKm. Beyond that the place is not a simplification artefact but an island too
	 * small to survive level 1's minRegionSize (Thera, Naxos, Delos), and INDEX_NONE is better
	 * than a domain planted on the nearest mainland.
	 */
	UFUNCTION(BlueprintCallable, Category = "History")
	int32 FindSeedTriangleForPlace(double latitude, double longitude, double maxDistanceKm = 50.0);

	/**
	 * Discards every level's domains and rebuilds level 1's. The finer levels are rebuilt lazily,
	 * the first time something draws or queries them.
	 *
	 * Grows every place's domain out to DomainRadiusKm, stopping at coastlines, rivers and the
	 * neighbouring places' domains. Called at the end of world creation, so it does not normally
	 * need calling by hand -- re-run it after changing DomainRadiusKm.
	 */
	UFUNCTION(BlueprintCallable, Category = "History")
	void BuildPlaceDomains();


	// (x, y) is in mesh degrees: x is longitude, y is NEGATED latitude (y = -90 is the north
	// pole). Tile indices are (tileX, tileY), and GetTriangles() takes 0..1 view fractions --
	// see the FMapLowZoom class comment for all three spaces.
	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	int32 GetTerrainDataAtCoordinate(int32 zoomCategory, double x, double y);

	/**
	 * Everything about the domain under (x, y) in mesh degrees: which place holds it, that place's
	 * polity, and what the place's whole domain is made of, largest terrain first.
	 *
	 * One call rather than one per answer. They come from a single triangle lookup and a single walk
	 * over the domain, and -- more to the point -- from a single state of the world: Places and
	 * Polities are rebuilt by SetCurrentYear(), so two calls could straddle a year change and
	 * describe two different centuries. Check bValid before reading anything else; it is false on
	 * water, off the mesh, and on land no place has claimed.
	 */
	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	FDomainInfo GetDomainAtCoordinate(int32 zoomCategory, double x, double y);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	int32 GetTriangleIDAtCoordinate(int32 zoomCategory, double x, double y);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	FString GetTerrainText(int32 v);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FCanvasUVTri> GetTriangles(int32 mode, int32 zoomCategory, double minFracY, double minFracX, double maxFracY, double maxFracX, int32 offsetX, int32 offsetY, int32 width, int32 height);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FCanvasUVTri> GetMaterialTriangles(int32 mode, int32 zoomCategory, int32 tileX, int32 tileY);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FLineDisplayData> GetBorders(int32 mode, int32 zoomCategory, int32 tileX, int32 tileY);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FLineDisplayData> GetRivers(int32 mode, int32 zoomCategory, int32 tileX, int32 tileY);

	/**
	 * The domain layer for the same view GetTriangles() was given. Call it with the identical
	 * fractions and offsets and draw the result afterwards; the terrain pass no longer includes
	 * domains, so a mode that should not show them simply skips this.
	 *
	 * Returns nothing when bShowPlaceDomains is off, or before BuildPlaceDomains has run.
	 */
	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FCanvasUVTri> GetProvinceTriangles(int32 mode, int32 zoomCategory, double minFracY, double minFracX, double maxFracY, double maxFracX, int32 offsetX, int32 offsetY, int32 width, int32 height);

	// (x, y) is in mesh degrees, like the lookups above. Returns { minTileX, minTileY, maxTileX,
	// maxTileY }; the X pair is not wrapped for a view that crosses the antimeridian.
	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<int32> GetSubregionIndices(int32 zoomCategory, double y, double x, double yDelta, double xDelta);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	int32 GetNumSubregions(int32 zoomCategory, bool isX);

private:
	// Logs and returns false if the accessors below are called before Initialize().
	bool EnsureInitialized(const TCHAR* callerName) const;

	void InitializeTerrain();
	void InitializeHistory();
	void InitializeMap();

};
