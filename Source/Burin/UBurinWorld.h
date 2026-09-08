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
#include "Display/FMapDrawCall.h"
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
	 * How far map mode 0's colour blending reaches, in degrees. 0 disables it.
	 *
	 * The blend is sampled onto a grid in world coordinates rather than spread along the mesh, so
	 * this is a real ground distance: each grid point averages the land within this radius, and a
	 * biome transition ends up spread over roughly twice it. Raise it where transitions should be
	 * gradual over hundreds of kilometres -- the Sahara thinning into the Sahel is the case it
	 * exists for. Coastlines are unaffected at any value, since only land is averaged and water is
	 * drawn flat.
	 */
	/**
	 * A coordinate to report on. Every draw call whose box covers it logs the palette colour it is
	 * about to write there and the exact render target pixel it will write it to.
	 *
	 * The sphere and a map tile cover the same ground at different levels, into different targets, at
	 * different pixels. Comparing them means holding a coordinate fixed and letting each side say
	 * where that lands in its own target -- working the pixel out by hand has now produced two
	 * comparisons of the wrong places, which is a slow way to learn nothing.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
	double ProbeLatitude = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
	double ProbeLongitude = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
	double BlendRadiusDegrees = 0.75;

	/**
	 * How strongly map mode 0 shades slopes from the elevation data. 0 disables it.
	 *
	 * Mountains read from orbit as lit and shadowed faces rather than as a different colour, which a
	 * flat fill cannot express and the photographic palette actively works against -- pulling every
	 * terrain toward one earth tone removes what little hue difference a range had. This lights the
	 * elevation field from the north-west instead. Level ground is left exactly as the palette has
	 * it; only slopes move.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
	double HillshadeStrength = 0.55;

	/**
	 * Master switch for the domain layer. GetTerrainTriangles and GetMaterialTriangles no longer draw
	 * domains at all -- the caller decides, per map mode, by drawing GetDomainTriangles() over the
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
	// pole). Tile indices are (tileX, tileY), and GetTerrainTriangles() takes 0..1 view fractions --
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
	TArray<FCanvasUVTri> GetTerrainTriangles(int32 mode, int32 zoomCategory, double minFracY, double minFracX, double maxFracY, double maxFracX, int32 offsetX, int32 offsetY, int32 width, int32 height);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FCanvasUVTri> GetMaterialTriangles(int32 mode, int32 zoomCategory, int32 tileX, int32 tileY);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FLineDisplayData> GetBorders(int32 mode, int32 zoomCategory, int32 tileX, int32 tileY);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FLineDisplayData> GetRivers(int32 mode, int32 zoomCategory, int32 tileX, int32 tileY);

	/**
	 * The domain layer for the same view GetTerrainTriangles() was given. Call it with the identical
	 * fractions and offsets and draw the result afterwards; the terrain pass no longer includes
	 * domains, so a mode that should not show them simply skips this.
	 *
	 * Returns nothing when bShowPlaceDomains is off, or before BuildPlaceDomains has run.
	 */
	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FCanvasUVTri> GetDomainTriangles(int32 mode, int32 zoomCategory, double minFracY, double minFracX, double maxFracY, double maxFracX, int32 offsetX, int32 offsetY, int32 width, int32 height);

	/**
	 * What to draw for this view, one entry per tile, ready to hand straight back to the
	 * ...ForDrawCall accessors below.
	 *
	 * This replaces working out, per frame: which tiles are visible, which of them still need
	 * painting, which zoom category each should be drawn at, and where on the render target each
	 * one goes. All four have to agree, and they are all derived from the category here.
	 *
	 * It also stops a zoom-out from destroying detail. The render target accumulates, and one
	 * coarse tile covers the same pixels as four fine ones, so painting coarse over ground that was
	 * already painted fine makes the map worse as you zoom out. Where that would happen this
	 * returns the fine tiles needed to finish the region instead of the coarse tile that would
	 * overwrite it.
	 *
	 * Draw every call it returns -- they are recorded as painted the moment they are returned.
	 * Call ResetMapCoverage() whenever the render target is cleared.
	 */
	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FMapDrawCall> PlanDraws(int32 mapMode, int32 zoomCategory, double y, double x, double yDelta, double xDelta, int32 renderTargetWidth, int32 renderTargetHeight);

	/**
	 * PlanDraws in the caller's own terms: a real latitude, a real longitude, and how far the
	 * camera can see, as an angle on the globe.
	 *
	 * Prefer this to PlanDraws. It takes the two conversions the caller would otherwise have to get
	 * right, and gets them right once here instead of at every call site:
	 *
	 *   - The mesh's y axis is a negated latitude, so a real latitude has to be flipped before the
	 *     tile row is derived from it. Miss it and the map is mirrored north for south, which on a
	 *     globe is subtle enough to survive a casual look.
	 *
	 *   - Longitude degrees narrow towards the poles, so the same visible ground spans more of them
	 *     the further from the equator you are. A half-extent used unchanged for both axes asks for
	 *     too few tiles in x at high latitude, and the map loses its left and right edges -- worse
	 *     the further north you look, and correct at the equator, which makes it look like
	 *     something else.
	 *
	 * halfAngleDegrees is the angular radius of what the camera can see, measured at the globe's
	 * centre: acos(globeRadius / cameraDistance) is the horizon, and anything smaller crops inside
	 * it. Overestimating is the safe direction -- the extra tiles are drawn once and then remembered.
	 */
	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FMapDrawCall> PlanDrawsForView(int32 mapMode, int32 zoomCategory, double latitude, double longitude, double halfAngleDegrees, int32 renderTargetWidth, int32 renderTargetHeight);

	/**
	 * One tile, drawn to fill a render target of its own -- what a high-zoom map tile has.
	 *
	 * The same arithmetic PlanDraws lays its own calls out with, so the two cannot drift: both go
	 * through MakeDrawCall(), and this one then moves the rectangle to the origin because the target
	 * holds a single tile. Feed the result to the same GetTerrainTrianglesForDrawCall() and friends
	 * the sphere uses.
	 *
	 * PlanDraws itself is the wrong tool here, not merely a heavier one. It maintains a record of
	 * what is already painted on one shared target: it answers with nothing for a tile it believes
	 * is still on screen, resizes that record to whatever target it is handed, and throws it away on
	 * a map mode change. A tile owning its own target and repainting on demand would both corrupt
	 * that record and get an empty answer on its second render.
	 *
	 * Take this rather than working the fractions out in Blueprint. The fractions are interleaved
	 * (minFracY, minFracX, maxFracY, maxFracX), the tile index they imply is recovered from the
	 * ratio of the box's corner to its size, and a pair of them wired to the wrong pins produces no
	 * error at all: the caller gets a full set of triangles for some other tile, mapped through a
	 * box that puts them off the edge of the target. That is invisible output for full cost.
	 */
	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	FMapDrawCall MakeTileDrawCall(int32 zoomCategory, int32 tileX, int32 tileY, int32 renderTargetWidth, int32 renderTargetHeight);

	/**
	 * Forgets what has been drawn. Call it wherever the render target is cleared -- a map mode
	 * change, a new year -- or PlanDraws will believe the old picture is still on screen and
	 * decline to repaint it.
	 */
	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	void ResetMapCoverage();

	// The four draw passes, each taking a call from PlanDraws so that nothing has to be unpacked
	// and rewired by hand. Borders and rivers ignore the destination rectangle, as they always have.
	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FCanvasUVTri> GetTerrainTrianglesForDrawCall(int32 mode, const FMapDrawCall& drawCall);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FCanvasUVTri> GetDomainTrianglesForDrawCall(int32 mode, const FMapDrawCall& drawCall);

	/**
	 * Coastlines and rivers as triangles, for the same view GetTerrainTriangles() was given.
	 *
	 * This replaces GetBorders()/GetRivers() for drawing. Those return one FLineDisplayData per
	 * segment, which costs a Blueprint Draw Line call each; this returns two triangles per segment
	 * in one array that a single Draw Triangles call consumes. They also ignore the destination
	 * rectangle and project into a fixed 16384 square, so they only ever lined up for a full-world
	 * draw at that exact size.
	 *
	 * Terrain no longer includes coastlines, so draw this over GetTerrainTriangles wherever they are
	 * wanted -- which is most modes, coastlines being geography rather than politics.
	 */
	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FCanvasUVTri> GetBorderTriangles(int32 mode, int32 zoomCategory, double minFracY, double minFracX, double maxFracY, double maxFracX, int32 offsetX, int32 offsetY, int32 width, int32 height, double thickness = 5.0);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FCanvasUVTri> GetBorderTrianglesForDrawCall(int32 mode, const FMapDrawCall& drawCall, double thickness = 5.0);

	// (x, y) is in mesh degrees, like the lookups above. Returns { minTileX, minTileY, maxTileX,
	// maxTileY }; the X pair is not wrapped for a view that crosses the antimeridian.
	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<int32> GetSubregionIndices(int32 zoomCategory, double y, double x, double yDelta, double xDelta);

	/**
	 * GetSubregionIndices in the caller's own terms: a real latitude, a real longitude, and how far
	 * the view reaches. What PlanDrawsForView is to PlanDraws, and for the same reason.
	 *
	 * Prefer this. GetSubregionIndices takes MESH y, which is negated latitude -- passing a latitude
	 * straight into it mirrors the tile range about the equator. That is invisible at the equator,
	 * where the mirror is the identity, and grows into a whole-map error as the view moves away from
	 * it: the tiles fetched are the ones as far south as the view is north, so they are created,
	 * positioned by their true latitude, and land off the far side of the screen while the ground
	 * actually on screen has no tiles at all.
	 *
	 * It also widens the longitude reach by 1 / cos(latitude), as PlanDrawsForView does. A tile
	 * covers less of the screen the further it is from the equator, so a view of fixed angular width
	 * needs more of them across; a fixed longitude delta quietly stops filling the sides.
	 */
	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<int32> GetSubregionIndicesForView(int32 zoomCategory, double latitude, double longitude, double halfAngleDegrees);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	int32 GetNumSubregions(int32 zoomCategory, bool isX);

	/**
	 * Which pixel of a whole-world render target covers a coordinate.
	 *
	 * The sphere's target spans the globe and a map tile's spans one tile, so the same ground is at
	 * different pixels in each, and comparing the two by hand has already produced one comparison of
	 * the Sahara against the South Atlantic. Ask for the pixel rather than working it out twice.
	 *
	 * Pass the tile's own box (its centre coordinate +- half a tile) to address a tile's target; pass
	 * the whole globe to address the sphere's.
	 */
	UFUNCTION(BlueprintPure, Category = "RenderMap")
	void GetRenderTargetPixel(double latitude, double longitude, double minLatitude, double maxLatitude, double minLongitude, double maxLongitude, int32 renderTargetWidth, int32 renderTargetHeight, int32& outX, int32& outY);

	/**
	 * Where a tile's centre is, as a real latitude and longitude.
	 *
	 * Latitude, not mesh y. The two differ by a sign, and that sign has already been got wrong once
	 * here -- a latitude fed into GetSubregionIndices mirrored the whole tile window about the
	 * equator, which is invisible at the equator and a whole-map error anywhere else. Anything
	 * placing a tile in the world wants this form, and so does anything placing a marker beside it.
	 */
	UFUNCTION(BlueprintPure, Category = "RenderMap")
	void GetTileCentreCoordinate(int32 zoomCategory, int32 tileX, int32 tileY, double& latitude, double& longitude);

private:
	// Logs and returns false if the accessors below are called before Initialize().
	bool EnsureInitialized(const TCHAR* callerName) const;

	void InitializeTerrain();
	void InitializeHistory();
	void InitializeMap();

};
