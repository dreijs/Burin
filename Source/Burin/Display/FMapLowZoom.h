// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Canvas.h"
#include "Burin/Data/Earth/FTerrain.h"
#include "FTriangleDataEntry.h"
#include "FEdgeDataEntry.h"
#include "FPointDataEntry.h"
#include "FGridDataEntry.h"
#include "FTileDomains.h"
#include "FLineDisplayData.h"
#include "Burin/Concepts/Provinces/FPlace.h"
#include "Burin/Concepts/Polities/FPolity.h"

/**
 * Three coordinate spaces meet in this class, and they are NOT interchangeable:
 *
 *   mesh degrees (x, y)   The space the polygon data is stored in. x is longitude, -180..180.
 *                         y is NEGATED latitude, -90..90: y = -90 is the north pole and y = +90
 *                         the south pole (level-2 tile row 0 spans y -90..-45, and the y band
 *                         +80..+90 is 90% land -- Antarctica). Anything holding a real latitude
 *                         must negate it before passing it in; see UWorldCreator::CreatePlace.
 *
 *   tile indices          (tileX, tileY) into TriangleData[zoomCategory], derived from mesh
 *                         degrees as tileX = (x + 180) / 360 * w, tileY = (y + 90) / 180 * h.
 *                         So tileY 0 is the northernmost row.
 *
 *   view fractions        The min/max arguments of GetTriangles(), in 0..1 across the whole
 *                         map rather than in degrees: x = 360 * frac - 180, y = 180 * frac - 90.
 *
 * Every function here works in one of those three spaces. GetSubregionIndices() used to be an
 * exception, deriving its tile row with the opposite sign, which a single-tile level hid.
 */
class BURIN_API FMapLowZoom
{

public:
	int32 GetTerrainDataAtCoordinate(FTerrain* terrain, int32 zoomCategory, double x, double y);
	int32 GetTriangleIDAtCoordinate(int32 zoomCategory, double x, double y);

	/**
	 * The same lookup, also reporting which tile the answer is local to. The returned index means
	 * nothing without it on a level with more than one tile, and deriving the tile a second time
	 * from (x, y) is how a caller ends up disagreeing with this function about which tile a
	 * coordinate on a boundary belongs to.
	 */
	int32 GetTriangleIDAtCoordinate(int32 zoomCategory, double x, double y, int32& outTileX, int32& outTileY);

	/**
	 * Global triangle ids: every tile of a level laid end to end in (tileX, tileY) order, so that a
	 * single integer names a triangle anywhere on the level.
	 *
	 * Domains need this and the rest of the class does not. A domain is grown by walking from
	 * triangle to triangle and a 200 km domain crosses tile boundaries freely at levels 2 and 3,
	 * where a tile is a few hundred kilometres across -- so the flood cannot be expressed in
	 * tile-local indices, which repeat once per tile. Everything the mesh itself stores (edge T1/T2,
	 * cross-layer links, grid cells, fragments) stays tile-local, and is converted at the boundary
	 * of the flood.
	 */
	int32 ToGlobalTriangle(int32 zoomCategory, int32 tileX, int32 tileY, int32 localTriangle);
	bool FromGlobalTriangle(int32 zoomCategory, int32 globalTriangle, int32& outTileX, int32& outTileY, int32& outLocalTriangle);
	int32 NumGlobalTriangles(int32 zoomCategory);

	FString GetTerrainText(FTerrain* terrain, int32 v);

	/**
	 * The nearest land triangle to (x, y) in mesh degrees, or INDEX_NONE when there is no land
	 * within maxDistanceKm. Returns the containing triangle when that one is already land, since
	 * it sits at distance zero.
	 *
	 * Distance is measured to the triangle itself, not to its centroid: the mesh is ear-clipped,
	 * so a third of its triangles are thin slivers whose centroid is nowhere near the edge that
	 * actually faces the query point.
	 *
	 * Returns a global triangle id (see ToGlobalTriangle), and searches every tile the query circle
	 * touches rather than only the one the point falls in -- at level 3 a tile boundary runs through
	 * the middle of the Mediterranean, and a coastal place a few kilometres on the wrong side of one
	 * would otherwise be told there is no land near it.
	 */
	int32 FindLandTriangleNear(int32 zoomCategory, double x, double y, double maxDistanceKm);

	// Terrain only. Domains used to be folded in here so every draw path got them for free, which
	// left no way to show terrain without them -- the caller draws GetProvinceTriangles() on top
	// when it wants domains, and simply does not when it does not.
	TArray<FCanvasUVTri> GetTriangles(FTerrain* terrain, int32 mode, int32 zoomCategory, double minFracY, double minFracX, double maxFracY, double maxFracX, int32 offsetX, int32 offsetY, int32 width, int32 height);
	TArray<FCanvasUVTri> GetMaterialTriangles(FTerrain* terrain, int32 mode, int32 zoomCategory, int32 tileX, int32 tileY);
	TArray<FLineDisplayData> GetBorders(int32 mode, int32 zoomCategory, int32 tileX, int32 tileY);
	TArray<FLineDisplayData> GetRivers(int32 mode, int32 zoomCategory, int32 tileX, int32 tileY);
	/**
	 * Paints each place's domain in its polity's map colour, projected for the view exactly as
	 * GetTriangles() projects terrain -- same fractions, same offsets, same tile those fractions
	 * land on. Give it the arguments GetTriangles() was given and draw it afterwards.
	 *
	 * `mode` is accepted for symmetry with the other accessors but is not used: which modes show
	 * domains is the caller's decision, made by calling this or not. Returns nothing until
	 * BuildPlaceDomains() has run.
	 */
	TArray<FCanvasUVTri> GetProvinceTriangles(int32 mode, int32 zoomCategory, double minFracY, double minFracX, double maxFracY, double maxFracX, int32 offsetX, int32 offsetY, int32 width, int32 height);

	/**
	 * Whether `mode` is one that paints domains -- the non-geographic modes, of which there is
	 * currently one. Public because the caller has to know before deciding whether to build them:
	 * building a level's domains costs the same whether or not anything then draws them.
	 */
	static bool ModeShowsDomains(int32 mode) { return mode == 7; }

	/**
	 * Grows a domain around every place that has a SeedTriangle and records the result in
	 * FPlace::Triangles and in the level's owner map, replacing whatever was there before.
	 *
	 * Worked in two phases, which is the whole point of the design.
	 *
	 * First, each place separately floods the land it can reach: outward from its seed triangle,
	 * never over a coastline, never past maxRadiusKm measured straight from the place. Reach sets
	 * overlap freely, and nothing is decided yet.
	 *
	 * Then every triangle is divided between all the places that reached it, each taking the part
	 * nearer to it than to any of the others, clipped to its own radius. A border between two
	 * domains is therefore exactly the bisector of the two places, and the outer edge of a domain
	 * exactly a circular arc.
	 *
	 * Deciding ownership during the flood, as this used to, made the border depend on an accident:
	 * a triangle went whole to whichever place was nearest to any one of its corners, and this mesh
	 * has triangles 278 km long, so that was close to a coin-toss. Splitting only against places
	 * that owned an adjacent triangle then missed most rivals -- 619 triangles were treated as
	 * contested where 2495 actually are.
	 *
	 * Since the shares of a triangle tile it exactly, domains cannot overlap or leave a seam. What
	 * is left bare is only ground no place can reach, or ground past every radius.
	 *
	 */
	void BuildPlaceDomains(TArray<FPlace>& places, const TArray<FPolity>& polities, int32 zoomCategory, double maxRadiusKm, bool bRiversBlockDomains);

	/**
	 * BuildPlaceDomains() for one level, done once and remembered.
	 *
	 * Domains are per level: each level has its own mesh, its own coastline and its own triangle
	 * ids, so a domain built at level 1 says nothing about which level-3 triangles it covers.
	 * Building all of them up front would mean flooding roughly ten times level 1's mesh before the
	 * first frame, most of it for levels the camera may never reach, so a level is built the first
	 * time something asks for it and kept until InvalidatePlaceDomains() says otherwise.
	 */
	void EnsurePlaceDomains(TArray<FPlace>& places, const TArray<FPolity>& polities, int32 zoomCategory, double maxRadiusKm, bool bRiversBlockDomains);

	/** Marks every level's domains stale, so the next EnsurePlaceDomains() rebuilds. */
	void InvalidatePlaceDomains();

	/**
	 * The place whose domain covers (x, y) in mesh degrees, or INDEX_NONE.
	 *
	 * A triangle the domain radius or a rival's claim passes through is split into pieces belonging
	 * to different places, so where that has happened the answer comes from the piece containing the
	 * point rather than from the triangle's owner -- Owner names only the nearest contender, which
	 * for a contested triangle is not who holds the ground under the cursor.
	 */
	int32 GetPlaceAtCoordinate(int32 zoomCategory, double x, double y);

	/**
	 * Area by raw terrain value over everything one place holds, in square kilometres.
	 *
	 * Ground beneath a later-drawn layer is discounted using the generator's CoveredArea, so a patch
	 * covered by an overlay is counted once rather than once per layer. For a triangle split between
	 * places, that discount is applied in proportion to each piece: the generator records what is
	 * hidden per triangle, not per piece, and dividing it by area is the closest honest reading.
	 */
	void GetDomainTerrainAreas(int32 zoomCategory, int32 placeIndex, TMap<int32, double>& outAreasKm2, double& outTotalKm2);

	/**
	 * The range of tiles covering the box of half-extent (xDelta, yDelta) about (x, y), returned as
	 * { minTileX, minTileY, maxTileX, maxTileY }. The X pair is not wrapped, so a view crossing the
	 * antimeridian yields indices outside [0, w) for the caller to wrap.
	 */
	TArray<int32> GetSubregionIndices(int32 zoomCategory, double y, double x, double yDelta, double xDelta);
	int32 GetNumSubregions(int32 zoomCategory, bool isX);

	void Initialize();

private:
	// for each zoom level, for each x coordinate, for each y coordinate, a list of triangle/point/edge data
	TArray < TArray < TArray < TArray <FTriangleDataEntry> > > > TriangleData;
	TArray < TArray < TArray < TArray <FEdgeDataEntry> > > > EdgeData;
	TArray < TArray < TArray < TArray <FPointDataEntry> > > > PointData;

	// for each zoom level, for each x coordinate, for each y coordinate, that tile's precomputed
	// spatial index over TriangleData[level][x][y] (see FGridDataEntry)
	TArray < TArray < TArray <FGridDataEntry> > > GridData;

	// for each zoom level, for each x coordinate, for each y coordinate, which place owns each of
	// that tile's triangles. Written by BuildPlaceDomains() and read by GetProvinceTriangles();
	// empty for any level that has not been built.
	TArray < TArray < TArray <FTileDomains> > > DomainData;

	// Per level, whether DomainData holds a current answer. Kept beside DomainData rather than
	// inferred from it being non-empty, because a level with no places at all builds to an empty
	// result that is nonetheless up to date.
	TArray<bool> DomainsBuilt;

	// Per level, the prefix sums that map (tileX, tileY) to the start of that tile's block of
	// global triangle ids: w*h + 1 entries, so entry i is where tile i begins and the last entry is
	// the level's total. Built on demand by EnsureTriangleBases().
	TArray < TArray <int32> > TriangleBases;
	void EnsureTriangleBases(int32 zoomCategory);

	FPointDataEntry GetFirstPoint(bool b, int32 edge, int32 zoomCategory, int32 tileX, int32 tileY);

	// Appends the domains of one tile, projected the same way the caller projected its terrain.
	void AddProvinceTriangles(TArray<FCanvasUVTri>& result, int32 zoomCategory, int32 tileX, int32 tileY, double minX, double maxX, double minY, double maxY, int32 offsetX, int32 offsetY, double width, double height);

	// The nearest land triangle to (x, y) within one tile, as a tile-local index. bestDistanceKm is
	// the search limit on the way in and the distance found on the way out, so a caller sweeping
	// several tiles carries it along and each tile only has to beat what came before. (x, y) may lie
	// outside the tile: the grid cell it starts from is clamped to the tile's edge, which is where a
	// search from outside should begin anyway.
	int32 FindLandTriangleInTile(int32 zoomCategory, int32 tileX, int32 tileY, double x, double y, double& bestDistanceKm);

	// The three corners of a triangle, in mesh degrees.
	void GetTriangleVertices(int32 zoomCategory, int32 tileX, int32 tileY, int32 triangle, FPointDataEntry& outP1, FPointDataEntry& outP2, FPointDataEntry& outP3);
	TArray<FCanvasUVTri>& AddBordersAsTriangles(TArray<FCanvasUVTri>& result, int32 zoomCategory, int32 tileX, int32 tileY, double minX, double maxX, double minY, double maxY, int32 offsetX, int32 offsetY, int32 width, int32 height);

	// True if zoomCategory has been loaded (LevelData.bin was found and parsed for it). A level
	// whose binary is missing (e.g. mid-regeneration) is left as an empty entry by Initialize()
	// rather than causing a load failure, so every public accessor below checks this first instead
	// of indexing TriangleData/EdgeData/PointData/GridData unconditionally.
	bool HasZoomLevelData(int32 zoomCategory) const;
	bool HasTileData(int32 zoomCategory, int32 tileX, int32 tileY) const;
};
