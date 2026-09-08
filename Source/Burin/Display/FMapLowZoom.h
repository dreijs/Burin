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
#include "FMapDrawCall.h"
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
 *   view fractions        The min/max arguments of GetTerrainTriangles(), in 0..1 across the whole
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
	// left no way to show terrain without them -- the caller draws GetDomainTriangles() on top
	// when it wants domains, and simply does not when it does not.
	TArray<FCanvasUVTri> GetTerrainTriangles(FTerrain* terrain, int32 mode, int32 zoomCategory, double minFracY, double minFracX, double maxFracY, double maxFracX, int32 offsetX, int32 offsetY, int32 width, int32 height);
	TArray<FCanvasUVTri> GetMaterialTriangles(FTerrain* terrain, int32 mode, int32 zoomCategory, int32 tileX, int32 tileY);
	TArray<FLineDisplayData> GetBorders(int32 mode, int32 zoomCategory, int32 tileX, int32 tileY);
	TArray<FLineDisplayData> GetRivers(int32 mode, int32 zoomCategory, int32 tileX, int32 tileY);
	/**
	 * Paints each place's domain in its polity's map colour, projected for the view exactly as
	 * GetTerrainTriangles() projects terrain -- same fractions, same offsets, same tile those fractions
	 * land on. Give it the arguments GetTerrainTriangles() was given and draw it afterwards.
	 *
	 * `mode` is accepted for symmetry with the other accessors but is not used: which modes show
	 * domains is the caller's decision, made by calling this or not. Returns nothing until
	 * BuildPlaceDomains() has run.
	 */
	TArray<FCanvasUVTri> GetDomainTriangles(int32 mode, int32 zoomCategory, double minFracY, double minFracX, double maxFracY, double maxFracX, int32 offsetX, int32 offsetY, int32 width, int32 height);

	/**
	 * Coastlines and rivers, as thickened quads rather than lines, projected for the view the way
	 * GetTerrainTriangles() projects terrain.
	 *
	 * Two triangles per segment in one array, drawn by a single Draw Triangles call, instead of one
	 * Blueprint call per segment -- which is where the cost of the FLineDisplayData path went. It
	 * also honours the destination rectangle, which GetBorders()/GetRivers() never did: those
	 * project into a fixed 16384 square regardless of where they are being drawn.
	 *
	 * Drawn only in the modes ModeShowsBorders() names, the same way domains are gated. The check
	 * lives here rather than in the caller so that every draw path agrees about it: the sphere and
	 * the map tiles both draw this pass, and a rule spelled out separately in each is a rule that
	 * eventually differs between them.
	 *
	 * Draw this LAST, after the domains. A domain is an opaque area fill covering whole triangles,
	 * so a coastline or river drawn before it is simply painted over -- rivers worst of all, since
	 * they run through the middle of the land a domain covers rather than along its edge. The order
	 * that works is terrain, then domains, then this.
	 */
	TArray<FCanvasUVTri> GetBorderTriangles(int32 mode, int32 zoomCategory, double minFracY, double minFracX, double maxFracY, double maxFracX, int32 offsetX, int32 offsetY, int32 width, int32 height, double thickness);

	/**
	 * Whether `mode` is one that paints domains -- the non-geographic modes, of which there is
	 * currently one. Public because the caller has to know before deciding whether to build them:
	 * building a level's domains costs the same whether or not anything then draws them.
	 */
	static bool ModeShowsDomains(int32 mode) { return mode == 7; }

	/**
	 * Whether `mode` is one that paints coastlines and rivers.
	 *
	 * Kept separate from ModeShowsDomains() even though both currently name mode 7. They answer
	 * different questions -- one is about political geography, the other about physical -- and a
	 * mode that wants a coastline without domains, or the reverse, would otherwise have to untangle
	 * them first.
	 */
	static bool ModeShowsBorders(int32 mode) { return mode == 7; }

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
	 * What to draw, so the caller does not have to decide.
	 *
	 * Given the view and the size of the render target being drawn into, returns one FMapDrawCall
	 * per tile that needs painting -- already carrying its grid, its mesh box and its destination
	 * rectangle, all derived from the same category.
	 *
	 * It also remembers what is already on the target, which is what makes zooming out safe. The
	 * render target accumulates: a category-1 tile covers the same pixels as four category-2 tiles,
	 * so painting the coarse tile over ground that was already painted fine throws detail away and
	 * the map appears to get worse as you zoom out. Where that would happen, this returns the finer
	 * tiles needed to *complete* the region at the resolution already there, instead of the coarse
	 * tile that would overwrite it. Coarse over bare ground still draws the coarse tile -- the rule
	 * is only ever to avoid going backwards.
	 *
	 * Every returned call must actually be drawn: they are marked as painted here, so a caller that
	 * skips one leaves the record claiming ground that is still blank.
	 *
	 * Call ResetCoverage() whenever the target is cleared -- a map mode change, a new year -- or
	 * this will believe the old picture is still there and decline to redraw it.
	 */
	TArray<FMapDrawCall> PlanDraws(int32 mapMode, int32 zoomCategory, double y, double x, double yDelta, double xDelta, int32 renderTargetWidth, int32 renderTargetHeight);

	/**
	 * The one call for a target that holds a single tile, as a high-zoom map tile's does.
	 *
	 * Same tile arithmetic as PlanDraws produces -- it is MakeDrawCall(), asked for a target of
	 * renderTargetWidth x renderTargetHeight PER TILE, with the rectangle then moved to the origin.
	 * So there is one definition of where a tile sits in mesh degrees, and the two callers differ
	 * only in the thing that genuinely differs between them: where it lands on the target.
	 *
	 * Not PlanDraws, because PlanDraws is a record of what is already painted on one shared target.
	 * It returns nothing for a tile it believes is still on screen, it resizes that record to the
	 * target it was given, and it discards the record when the map mode changes -- all correct for
	 * the sphere, and all wrong for a tile that owns its own target and repaints it on demand.
	 */
	FMapDrawCall MakeTileDrawCall(int32 zoomCategory, int32 tileX, int32 tileY, int32 renderTargetWidth, int32 renderTargetHeight) const;

	/** Forgets what has been drawn, so the next PlanDraws() repaints from nothing. */
	void ResetCoverage();

	/**
	 * The range of tiles covering the box of half-extent (xDelta, yDelta) about (x, y), returned as
	 * { minTileX, minTileY, maxTileX, maxTileY }. The X pair is not wrapped, so a view crossing the
	 * antimeridian yields indices outside [0, w) for the caller to wrap.
	 */
	TArray<int32> GetSubregionIndices(int32 zoomCategory, double y, double x, double yDelta, double xDelta);
	int32 GetNumSubregions(int32 zoomCategory, bool isX);

	// The last range GetSubregionIndices() answered with, and the view that produced it. Only so the
	// log records a camera move once rather than once a frame.
	int32 LastSubregionRange[4] = { INDEX_NONE, INDEX_NONE, INDEX_NONE, INDEX_NONE };

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
	// that tile's triangles. Written by BuildPlaceDomains() and read by GetDomainTriangles();
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

	// What category is currently painted on each patch of the render target, or INDEX_NONE for
	// bare ground. Held at the finest loaded level's grid, which every coarser grid divides evenly,
	// so one cell is never half-owned by a tile. A few hundred integers -- it is a record of what
	// has been drawn, not a copy of it.
	TArray<int32> Coverage;
	int32 CoverageWidth = 0;
	int32 CoverageHeight = 0;
	int32 CoverageTargetWidth = 0;
	int32 CoverageTargetHeight = 0;

	// Which map mode the record describes. Coverage says a patch is painted, not what it was
	// painted with, so a mode change makes every entry a lie -- the ground is covered, but in the
	// wrong colours. Remembering the mode lets PlanDraws() notice and start again by itself, rather
	// than depending on the caller to remember. It did not, and the globe stayed on one map mode
	// however many times the mode was switched.
	int32 CoverageMapMode = INDEX_NONE;

	// Sizes the record to the finest loaded level and the given target, clearing it if either has
	// changed -- a different target is a different picture, and its old coverage means nothing.
	void EnsureCoverageGrid(int32 renderTargetWidth, int32 renderTargetHeight);

	// Adds whatever it takes to have this tile painted at zoomCategory or finer, recursing into
	// finer tiles where finer paint is already down. `depth` only bounds the recursion against a
	// malformed level table; the levels themselves terminate it.
	void EnsureTileCovered(int32 zoomCategory, int32 tileX, int32 tileY, int32 renderTargetWidth, int32 renderTargetHeight, TArray<FMapDrawCall>& outCalls, int32 depth);

	// One tile's call into a world-sized target: grid, mesh box and destination rectangle, all from
	// the one category. MakeTileDrawCall() is the public face of it for a target holding one tile.
	FMapDrawCall MakeDrawCall(int32 zoomCategory, int32 tileX, int32 tileY, int32 renderTargetWidth, int32 renderTargetHeight) const;

	FPointDataEntry GetFirstPoint(bool b, int32 edge, int32 zoomCategory, int32 tileX, int32 tileY);

	/**
	 * A regular grid of sampled land colour and elevation, in world coordinates, independent of the
	 * mesh. Mode 0 only.
	 *
	 * This replaces four attempts at blending along the mesh itself, each of which failed for the
	 * same underlying reason: the mesh's topology has nothing to do with what is near what on the
	 * ground. Its triangles run 360 degrees wide at the poles and a thousandth of a degree wide on a
	 * coast; its vertices stop at tile boundaries; a tenth of its region borders share no vertex at
	 * all. Colour spread along that graph fanned at the poles, stepped at every tile edge, and drew
	 * dark strokes wherever two short edges met across an elevation step.
	 *
	 * A grid has none of those properties by construction. Rows are spaced evenly in latitude and
	 * each row holds however many columns keep its cells roughly square, so the grid does not
	 * converge at the poles. Position is the only thing that decides a colour, so nothing changes at
	 * a tile boundary -- the sampling goes through GetTriangleIDAtCoordinate(), which resolves tiles
	 * itself.
	 *
	 * Each point averages a small disc of samples rather than being smoothed against its neighbours.
	 * That keeps every point INDEPENDENTLY computable, which is what lets regions be filled in lazily
	 * as tiles are drawn with no risk of two regions disagreeing along their join -- the failure that
	 * the mesh-based versions kept reproducing in new forms.
	 */
	struct FBlendGrid
	{
		double Spacing = 1.0;          // degrees between rows, and roughly between columns
		double RowStep = 1.0;          // exact latitude step, 180 / (NumRows - 1)
		int32 NumRows = 0;

		TArray<int32> ColumnCount;     // per row; fewer towards the poles, so cells stay square
		TArray<int32> RowStart;        // prefix sums into the arrays below, NumRows + 1 entries

		// Linear RGB, not the palette's sRGB bytes. Averaging is the whole purpose of this grid, and
		// the mean of two sRGB values is not the colour halfway between them -- sRGB is a curve, so
		// averaging along it pulls every blend toward the lighter of the two. Kept as floats rather
		// than packed back into bytes because linear 8-bit quantises where the eye is most sensitive:
		// a mid green sits around 0.15 linear, where one step of 1/255 is a 3% jump, and a grid that
		// exists to produce smooth gradients would band.
		TArray<FVector3f> Colour;
		TArray<float> Elevation;
		TArray<float> Shade;

		// 0 = never sampled, 1 = sampled and has land, 2 = sampled and is all water.
		TArray<uint8> State;
	};
	TArray<FBlendGrid> BlendGrids;

	// Lays out a level's rows and columns. Cheap: allocation and a cosine per row, no sampling.
	void EnsureBlendGrid(int32 zoomCategory);

	// Samples every grid point covering this box that has not been sampled yet, then works out the
	// shading for them from the elevation of their neighbours. The box is grown by a cell first, so
	// that a point on its edge has the neighbours its gradient needs.
	void EnsureBlendGridRegion(FTerrain* terrain, int32 zoomCategory, double minX, double maxX, double minY, double maxY);

	// The grid's colour and shading at a point, interpolated from the four surrounding grid points.
	// False where none of them found land -- open sea, or ground the mesh has no land triangle for --
	// in which case the caller should fall back to the triangle's own colour.
	bool SampleBlendGrid(int32 zoomCategory, double x, double y, FLinearColor& outColour, float& outShade) const;

	/**
	 * Emits one land triangle for mode 0, splitting it first if it is large.
	 *
	 * A triangle carries three colours, one per corner, and the canvas fills between them linearly.
	 * That makes the colour continuous across a shared edge -- both triangles read the grid at the
	 * same position -- but not its rate of change, which jumps at every edge. Where the grid varies
	 * strongly, as it does over the Carpathians or the Ethiopian highlands, a triangle spanning
	 * several grid cells becomes a flat facet with creased edges: the mesh showing through a field
	 * that has nothing to do with it.
	 *
	 * So a triangle wider than a grid cell is cut at its edge midpoints into four and each part asked
	 * again, until the parts are small enough that a straight ramp between three corners is a fair
	 * account of the field across them. Depth is capped, because the mesh contains triangles 360
	 * degrees wide and subdividing one of those to a quarter degree would make a quarter of a million
	 * pieces out of it.
	 */
	void AddBlendedLandTriangle(TArray<FCanvasUVTri>& result, int32 zoomCategory,
		const FVector2D& a, const FVector2D& b, const FVector2D& c,
		const FLinearColor& ownColour, double maxEdgeDegrees, int32 depth,
		double minX, double maxX, double minY, double maxY,
		int32 offsetX, int32 offsetY, double width, double height,
		int32& outFlat, int32& outBlended);

public:
	/**
	 * How far map mode 0's colour blending reaches, in degrees of latitude, and how strongly slopes
	 * are shaded. Changing either discards the sampled grid, which refills as tiles are drawn.
	 *
	 * The radius is the disc each grid point averages over, so it is the real blur scale: a biome
	 * transition ends up spread over roughly twice it. Coastlines are unaffected at any value --
	 * only land samples are averaged, and water is drawn flat.
	 */
	void SetVertexBlendSettings(double blendRadiusDegrees, double hillshadeStrength);

	// The coordinate the draw-call probe reports on. See UBurinWorld::ProbeLatitude.
	void SetProbeCoordinate(double latitude, double longitude);

private:
	double ProbeLatitude = 11.25;
	double ProbeLongitude = 11.25;
	double BlendRadiusDegrees = 0.75;
	double HillshadeStrength = 0.55;

	// Appends the domains of one tile, projected the same way the caller projected its terrain.
	void AddDomainTriangles(TArray<FCanvasUVTri>& result, int32 zoomCategory, int32 tileX, int32 tileY, double minX, double maxX, double minY, double maxY, int32 offsetX, int32 offsetY, double width, double height);

	// The nearest land triangle to (x, y) within one tile, as a tile-local index. bestDistanceKm is
	// the search limit on the way in and the distance found on the way out, so a caller sweeping
	// several tiles carries it along and each tile only has to beat what came before. (x, y) may lie
	// outside the tile: the grid cell it starts from is clamped to the tile's edge, which is where a
	// search from outside should begin anyway.
	int32 FindLandTriangleInTile(int32 zoomCategory, int32 tileX, int32 tileY, double x, double y, double& bestDistanceKm);

	// The three corners of a triangle, in mesh degrees.
	void GetTriangleVertices(int32 zoomCategory, int32 tileX, int32 tileY, int32 triangle, FPointDataEntry& outP1, FPointDataEntry& outP2, FPointDataEntry& outP3);
	TArray<FCanvasUVTri>& AddBordersAsTriangles(TArray<FCanvasUVTri>& result, int32 zoomCategory, int32 tileX, int32 tileY, double minX, double maxX, double minY, double maxY, int32 offsetX, int32 offsetY, int32 width, int32 height, double thickness);

	// True if zoomCategory has been loaded (LevelData.bin was found and parsed for it). A level
	// whose binary is missing (e.g. mid-regeneration) is left as an empty entry by Initialize()
	// rather than causing a load failure, so every public accessor below checks this first instead
	// of indexing TriangleData/EdgeData/PointData/GridData unconditionally.
	bool HasZoomLevelData(int32 zoomCategory) const;
	bool HasTileData(int32 zoomCategory, int32 tileX, int32 tileY) const;
};
