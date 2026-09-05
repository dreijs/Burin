// Fill out your copyright notice in the Description page of Project Settings.


#include "FMapLowZoom.h"
#include "FLineDisplayData.h"
#include "CanvasTypes.h" // Required for FCanvas
#include "Engine/Canvas.h" // Required for UCanvas static functions
#include "Async/ParallelFor.h"
#include "Serialization/MemoryReader.h"
#include <cstdlib> // Required for srand/rand
#include <Burin/Concepts/Provinces/fplace.h>

// Function to calculate the signed area of a triangle defined by three points
static double sign(double x1, double y1, double x2, double y2, double x3, double y3) {
    return (x1 - x3) * (y2 - y3) - (x2 - x3) * (y1 - y3);
}

// Function to check if a point is inside a triangle
static bool isPointInTriangle(double xp, double yp, double x1, double y1, double x2, double y2, double x3, double y3) {
    if (x1 == x2 && y1 == y2 && x1 == x3 && y1 == y3) return false;
    // Calculate signs of areas of the three subtriangles formed by the point
    double d1 = sign(xp, yp, x1, y1, x2, y2);
    double d2 = sign(xp, yp, x2, y2, x3, y3);
    double d3 = sign(xp, yp, x3, y3, x1, y1);

    // Check if the signs are consistent (all positive or all negative/zero)
    bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    // The point is inside the triangle if not all signs are negative and not all signs are positive
    // (i.e., all signs are the same, including zero)
    return !(has_neg && has_pos);
}

static bool isPointInCoordTriangle(double xp, double yp, double x1, double y1, double x2, double y2, double x3, double y3) {
    return isPointInTriangle(
        xp,
        yp,
        x1,
        y1,
        x2,
        y2,
        x3,
        y3
    );
}

//double bound(double x, double l, double r) {
//    if (x >= r) return 1;
//    if (x <= l) return 0;
//    return x;
//}

bool FMapLowZoom::HasZoomLevelData(int32 zoomCategory) const {
    return TriangleData.IsValidIndex(zoomCategory) && TriangleData[zoomCategory].Num() > 0;
}

bool FMapLowZoom::HasTileData(int32 zoomCategory, int32 tileX, int32 tileY) const {
    return TriangleData.IsValidIndex(zoomCategory)
        && TriangleData[zoomCategory].IsValidIndex(tileX)
        && TriangleData[zoomCategory][tileX].IsValidIndex(tileY);
}

static int32 floorMod(int32 A, int32 B) {
    return ((A % B) + B) % B;
}

// ---- global triangle ids ----
//
// Tile (tileX, tileY) owns the block of ids starting at TriangleBases[level][tileX * h + tileY].
// Column-major, matching the order the arrays are nested in, so a tile's block is contiguous and a
// sweep in ascending global order visits each tile's triangles in ascending local order -- which
// FTileDomains::Fragments depends on, since drawing walks fragments and triangles together.

void FMapLowZoom::EnsureTriangleBases(int32 zoomCategory) {
    if (TriangleBases.IsValidIndex(zoomCategory) && TriangleBases[zoomCategory].Num() > 0) {
        return;
    }
    if (!HasZoomLevelData(zoomCategory)) {
        return;
    }

    TriangleBases.SetNum(FMath::Max(TriangleBases.Num(), TriangleData.Num()));

    const int32 w = TriangleData[zoomCategory].Num();
    const int32 h = TriangleData[zoomCategory][0].Num();

    TArray<int32>& bases = TriangleBases[zoomCategory];
    bases.Reset(w * h + 1);

    int32 running = 0;
    for (int32 tx = 0; tx < w; tx++) {
        for (int32 ty = 0; ty < h; ty++) {
            bases.Add(running);
            running += TriangleData[zoomCategory][tx][ty].Num();
        }
    }
    bases.Add(running);
}

int32 FMapLowZoom::ToGlobalTriangle(int32 zoomCategory, int32 tileX, int32 tileY, int32 localTriangle) {
    if (localTriangle < 0 || !HasTileData(zoomCategory, tileX, tileY)) {
        return INDEX_NONE;
    }
    EnsureTriangleBases(zoomCategory);

    const int32 h = TriangleData[zoomCategory][tileX].Num();
    return TriangleBases[zoomCategory][tileX * h + tileY] + localTriangle;
}

bool FMapLowZoom::FromGlobalTriangle(int32 zoomCategory, int32 globalTriangle, int32& outTileX, int32& outTileY, int32& outLocalTriangle) {
    outTileX = 0;
    outTileY = 0;
    outLocalTriangle = INDEX_NONE;

    if (globalTriangle < 0 || !HasZoomLevelData(zoomCategory)) {
        return false;
    }
    EnsureTriangleBases(zoomCategory);

    const TArray<int32>& bases = TriangleBases[zoomCategory];
    if (bases.Num() < 2 || globalTriangle >= bases.Last()) {
        return false;
    }

    // The last block starting at or before this id. Binary search rather than a walk: at level 3
    // this is 128 blocks and the flood resolves an id for every triangle it touches.
    int32 low = 0;
    int32 high = bases.Num() - 2;
    while (low < high) {
        const int32 mid = (low + high + 1) / 2;
        if (bases[mid] <= globalTriangle) low = mid;
        else high = mid - 1;
    }

    const int32 h = TriangleData[zoomCategory][0].Num();
    outTileX = low / h;
    outTileY = low % h;
    outLocalTriangle = globalTriangle - bases[low];

    // An empty tile occupies no ids at all, so `low` can land on one only if the id belongs to a
    // later tile -- which the search above cannot produce. Checked anyway; an out-of-range local
    // index indexed into a tile reads as a random triangle rather than as a crash.
    return TriangleData[zoomCategory][outTileX][outTileY].IsValidIndex(outLocalTriangle);
}

int32 FMapLowZoom::NumGlobalTriangles(int32 zoomCategory) {
    if (!HasZoomLevelData(zoomCategory)) {
        return 0;
    }
    EnsureTriangleBases(zoomCategory);
    return TriangleBases[zoomCategory].Num() >= 1 ? TriangleBases[zoomCategory].Last() : 0;
}

TArray<int32> FMapLowZoom::GetSubregionIndices(int32 zoomCategory, double y, double x, double yDelta, double xDelta) {
    if (!HasZoomLevelData(zoomCategory)) {
        return {};
    }

    const int32 n = TriangleData[zoomCategory].Num();
    const int32 m = TriangleData[zoomCategory][0].Num();

    // Mesh space, the same as every other coordinate function here: x is longitude, y is negated
    // latitude, and a tile row is (y + 90) / 180 * m.
    //
    // This used to derive the row as (90 - lat) / 180 * m, mirroring north for south. It was
    // invisible for as long as level 1 was the only level that loaded -- a single tile answers
    // {0, 0, 0, 0} whatever you ask it -- and became wrong the moment levels 2 and 3 arrived with
    // 4 and 8 rows.
    //
    // X is deliberately left unwrapped: a view straddling the antimeridian returns indices outside
    // [0, n) and the caller wraps them, the way GetTriangles() does with floorMod().
    const int32 minX = FMath::FloorToInt32((x - xDelta + 180) / 360 * n);
    const int32 maxX = FMath::FloorToInt32((x + xDelta + 180) / 360 * n);

    const int32 minY = FMath::Clamp(FMath::FloorToInt32((y - yDelta + 90) / 180 * m), 0, m - 1);
    const int32 maxY = FMath::Clamp(FMath::FloorToInt32((y + yDelta + 90) / 180 * m), 0, m - 1);

    return { minX, minY, maxX, maxY };
}

int32 FMapLowZoom::GetNumSubregions(int32 zoomCategory, bool isX) {
    if (!HasZoomLevelData(zoomCategory)) {
        return 0;
    }

    if (isX) return TriangleData[zoomCategory].Num();
    return TriangleData[zoomCategory][0].Num();
}

FPointDataEntry FMapLowZoom::GetFirstPoint(bool b, int32 edge, int32 zoomCategory, int32 tileX, int32 tileY) {
    if (b) { return PointData[zoomCategory][tileX][tileY][EdgeData[zoomCategory][tileX][tileY][edge].P1]; }
    return PointData[zoomCategory][tileX][tileY][EdgeData[zoomCategory][tileX][tileY][edge].P2];
}

void FMapLowZoom::GetTriangleVertices(int32 zoomCategory, int32 tileX, int32 tileY, int32 triangle, FPointDataEntry& outP1, FPointDataEntry& outP2, FPointDataEntry& outP3) {
    const FTriangleDataEntry& tri = TriangleData[zoomCategory][tileX][tileY][triangle];
    outP1 = GetFirstPoint(tri.bB1, tri.E1, zoomCategory, tileX, tileY);
    outP2 = GetFirstPoint(tri.bB2, tri.E2, zoomCategory, tileX, tileY);
    outP3 = GetFirstPoint(tri.bB3, tri.E3, zoomCategory, tileX, tileY);
}

FString FMapLowZoom::GetTerrainText(FTerrain* terrain, int32 v) {
    return terrain->GetTerrainText(v);
}

int32 FMapLowZoom::GetTerrainDataAtCoordinate(FTerrain* terrain, int32 zoomCategory, double x, double y) {
    if (!HasZoomLevelData(zoomCategory)) {
        return -1;
    }

    // The tile comes back from the lookup rather than being worked out again here. Deriving it
    // twice is how the two ended up disagreeing about a coordinate sitting exactly on a tile
    // boundary, and the returned index is local to whichever tile the lookup chose.
    int32 tileX = 0, tileY = 0;
    int32 idx = GetTriangleIDAtCoordinate(zoomCategory, x, y, tileX, tileY);
    if(idx >= 0) return terrain->GetTerrainFromCache(TriangleData[zoomCategory][tileX][tileY][idx].TerrainData);

    return -1;
}

// Called once per coordinate hit test, so its tracing stays at Verbose: at Log it would be
// three lines per lookup, which drowns the log the moment anything queries the mesh in bulk.
int32 FMapLowZoom::GetTriangleIDAtCoordinate(int32 zoomCategory, double x, double y) {
    int32 tileX = 0, tileY = 0;
    return GetTriangleIDAtCoordinate(zoomCategory, x, y, tileX, tileY);
}

int32 FMapLowZoom::GetTriangleIDAtCoordinate(int32 zoomCategory, double x, double y, int32& outTileX, int32& outTileY) {
    outTileX = 0;
    outTileY = 0;
    if (!HasZoomLevelData(zoomCategory)) {
        return -1;
    }

    double fractionX = (x + 180) / 360;
    double fractionY = (y + 90) / 180;

    // x is longitude and wraps around the globe; y is latitude and does not. Both are expected to
    // already lie in [-180,180) / [-90,90], but a probe offset just past a pole (see the edge
    // stitching in BuildPlaceDomains()) or a coordinate landing exactly on a boundary can push the
    // computed fraction a hair outside [0,1), so the tile index is clamped rather than trusted.
    int32 w = TriangleData[zoomCategory].Num();
    int32 tileX = FMath::Clamp(FMath::FloorToInt32(fractionX * w), 0, w - 1);
    int32 h = TriangleData[zoomCategory][tileX].Num();
    int32 tileY = FMath::Clamp(FMath::FloorToInt32(fractionY * h), 0, h - 1);
    outTileX = tileX;
    outTileY = tileY;
    UE_LOG(LogTemp, Verbose, TEXT("Triangle region: %d, %d"), tileX, tileY);

    const TArray<FTriangleDataEntry>& triangles = TriangleData[zoomCategory][tileX][tileY];
    const FGridDataEntry& grid = GridData[zoomCategory][tileX][tileY];

    // Find which fine cell (x, y) falls into within this tile's precomputed spatial grid
    // (the same [0,1) local-fraction subdivision the offline generator used to build it), then
    // only test that cell's candidates instead of every triangle in the tile. fractionX*w - tileX is
    // the fractional remainder left over from picking the tile above, i.e. position within the
    // tile, so this doesn't need to reconstruct the generator's local pixel space at all.
    double localFractionX = fractionX * w - tileX;
    double localFractionY = fractionY * h - tileY;
    int32 cellX = FMath::Clamp(FMath::FloorToInt32(localFractionX * grid.Width), 0, grid.Width - 1);
    int32 cellY = FMath::Clamp(FMath::FloorToInt32(localFractionY * grid.Height), 0, grid.Height - 1);

    const TArray<int32>& candidates = grid.Cells[cellY * grid.Width + cellX];

    // Highest triangle index wins on overlap, matching the original full-scan's draw-order
    // priority. A candidate list is always a superset of the triangles that could actually
    // contain this point (built from bounding-box overlap), so this can't miss the true match.
    for (int32 i = candidates.Num() - 1; i >= 0; i--) {
        const FTriangleDataEntry& tri = triangles[candidates[i]];
        FPointDataEntry p1 = GetFirstPoint(tri.bB1, tri.E1, zoomCategory, tileX, tileY);
        FPointDataEntry p2 = GetFirstPoint(tri.bB2, tri.E2, zoomCategory, tileX, tileY);
        FPointDataEntry p3 = GetFirstPoint(tri.bB3, tri.E3, zoomCategory, tileX, tileY);
        if (isPointInCoordTriangle(x, y, p1.X, p1.Y, p2.X, p2.Y, p3.X, p3.Y)) {
            UE_LOG(LogTemp, Verbose, TEXT("Triangle region: %d"), candidates[i]);
            return candidates[i];
        }
    }
    UE_LOG(LogTemp, Verbose, TEXT("Triangle region: -1"));
    return -1;
}

// Water is elevation index 0, which is the low nibble of the packed terrain code; see
// FTerrain::GetTerrain() for the rest of the packing.
// A neighbour the edge records could not name, and whether a river runs along the boundary to it.
// The flag has to travel with the link: rivers are recorded per edge, and a stitched link exists
// precisely because no single edge record spans the boundary.
struct FStitchedLink
{
    int32 Triangle = INDEX_NONE;
    bool bRiver = false;
};

static double distancePointToSegment(double px, double py, double ax, double ay, double bx, double by) {
    const double dx = bx - ax;
    const double dy = by - ay;
    const double lengthSquared = dx * dx + dy * dy;
    double t = 0.0;
    if (lengthSquared > 0.0) {
        t = FMath::Clamp(((px - ax) * dx + (py - ay) * dy) / lengthSquared, 0.0, 1.0);
    }
    return FMath::Sqrt(FMath::Square(px - (ax + t * dx)) + FMath::Square(py - (ay + t * dy)));
}

// True when two segments run along the same stretch of boundary rather than merely touching. Either
// may be the shorter of the two -- an unpaired edge is usually one piece of a longer edge on the far
// side -- so the midpoint of each is tested against the other.
static bool segmentsLieAlong(const FPointDataEntry& a, const FPointDataEntry& b,
    const FPointDataEntry& c, const FPointDataEntry& d, double toleranceDegrees) {
    const double abx = 0.5 * (a.X + b.X), aby = 0.5 * (a.Y + b.Y);
    const double cdx = 0.5 * (c.X + d.X), cdy = 0.5 * (c.Y + d.Y);
    return distancePointToSegment(abx, aby, c.X, c.Y, d.X, d.Y) < toleranceDegrees
        || distancePointToSegment(cdx, cdy, a.X, a.Y, b.X, b.Y) < toleranceDegrees;
}

static bool isWaterTerrain(int32 terrainData) {
    return terrainData % 16 == 0;
}

// A mesh-degree point expressed in kilometres relative to an origin. The origin's y is a negated
// latitude, but cosine is even, so cos(y) is the right longitude foreshortening either way.
static FVector2D toLocalKm(double x, double y, double originX, double originY) {
    double dx = x - originX;
    if (dx > 180.0) dx -= 360.0;
    else if (dx < -180.0) dx += 360.0;

    const double kmPerDegreeX = 111.320 * FMath::Cos(FMath::DegreesToRadians(originY));
    const double kmPerDegreeY = 110.574;
    return FVector2D(dx * kmPerDegreeX, (y - originY) * kmPerDegreeY);
}

static double distanceToSegment(const FVector2D& p, const FVector2D& a, const FVector2D& b) {
    const FVector2D ab = b - a;
    const double lengthSquared = ab.SizeSquared();
    const double t = (lengthSquared <= UE_DOUBLE_SMALL_NUMBER)
        ? 0.0
        : FMath::Clamp(FVector2D::DotProduct(p - a, ab) / lengthSquared, 0.0, 1.0);
    return FVector2D::Distance(p, a + t * ab);
}

// Distance in km from (x, y) to the triangle itself -- zero when the point is inside it.
static double distanceToTriangleKm(double x, double y, const FVector2D& v1, const FVector2D& v2, const FVector2D& v3) {
    if (isPointInCoordTriangle(x, y, v1.X, v1.Y, v2.X, v2.Y, v3.X, v3.Y)) {
        return 0.0;
    }

    const FVector2D query(0.0, 0.0);
    const FVector2D a = toLocalKm(v1.X, v1.Y, x, y);
    const FVector2D b = toLocalKm(v2.X, v2.Y, x, y);
    const FVector2D c = toLocalKm(v3.X, v3.Y, x, y);

    return FMath::Min3(distanceToSegment(query, a, b), distanceToSegment(query, b, c), distanceToSegment(query, c, a));
}

// Area of a triangle given in mesh degrees, in square kilometres. Longitude degrees shorten with
// the cosine of the latitude, and the mesh's y is a negated latitude -- cosine is even, so the sign
// does not matter here, but it does two lines later in anything that reads a latitude out.
static double triangleAreaKm2(double x1, double y1, double x2, double y2, double x3, double y3) {
    const double latitude = -(y1 + y2 + y3) / 3.0;
    const double kmPerDegreeX = 111.32 * FMath::Cos(FMath::DegreesToRadians(latitude));
    const double kmPerDegreeY = 110.57;
    return FMath::Abs((x2 - x1) * kmPerDegreeX * (y3 - y1) * kmPerDegreeY
                    - (x3 - x1) * kmPerDegreeX * (y2 - y1) * kmPerDegreeY) / 2.0;
}

static double distanceToTriangleKm(double x, double y, const FPointDataEntry& p1, const FPointDataEntry& p2, const FPointDataEntry& p3) {
    return distanceToTriangleKm(x, y, FVector2D(p1.X, p1.Y), FVector2D(p2.X, p2.Y), FVector2D(p3.X, p3.Y));
}

int32 FMapLowZoom::FindLandTriangleInTile(int32 zoomCategory, int32 tileX, int32 tileY, double x, double y, double& bestDistanceKm) {
    if (!HasTileData(zoomCategory, tileX, tileY)) {
        return INDEX_NONE;
    }

    const double fractionX = (x + 180) / 360;
    const double fractionY = (y + 90) / 180;

    const int32 w = TriangleData[zoomCategory].Num();
    const int32 h = TriangleData[zoomCategory][tileX].Num();

    const TArray<FTriangleDataEntry>& triangles = TriangleData[zoomCategory][tileX][tileY];
    const FGridDataEntry& grid = GridData[zoomCategory][tileX][tileY];
    if (grid.Width <= 0 || grid.Height <= 0) {
        return INDEX_NONE;
    }

    // Where the query point sits in this tile's grid. Clamped, so a point in a neighbouring tile
    // starts from the cell of this tile nearest to it rather than off the end of the array -- which
    // is what a search sweeping in from outside wants anyway.
    const int32 cellX = FMath::Clamp(FMath::FloorToInt32((fractionX * w - tileX) * grid.Width), 0, grid.Width - 1);
    const int32 cellY = FMath::Clamp(FMath::FloorToInt32((fractionY * h - tileY) * grid.Height), 0, grid.Height - 1);

    // How far one cell reaches, so an untested ring can be dismissed once it is further away than
    // the best hit so far. The smaller of the two axes keeps that a genuine lower bound.
    const double cellWidthKm = (360.0 / w / grid.Width) * 111.320 * FMath::Cos(FMath::DegreesToRadians(y));
    const double cellHeightKm = (180.0 / h / grid.Height) * 110.574;
    const double cellStepKm = FMath::Min(cellWidthKm, cellHeightKm);

    // Level 1 holds the whole globe in one tile, so its cells wrap in x. With more than one tile the
    // ring stops at the tile edge on purpose: FindLandTriangleNear() below sweeps the neighbouring
    // tiles itself, which also gets the tile's own triangles rather than reading past the array.
    const bool bWrapX = (w == 1);

    int32 bestTriangle = INDEX_NONE;

    const int32 maxRing = FMath::Max(grid.Width, grid.Height);
    for (int32 ring = 0; ring <= maxRing; ring++) {
        // Nothing in this ring is closer than (ring - 1) cells, so once that alone matches the
        // best hit, no further ring can improve on it. Testing >= rather than > means a place
        // already standing on land (best distance zero) stops after ring 0 instead of sweeping
        // a ring of neighbouring cells that cannot beat it.
        if ((ring - 1) * cellStepKm >= bestDistanceKm) {
            break;
        }

        for (int32 offsetY = -ring; offsetY <= ring; offsetY++) {
            for (int32 offsetX = -ring; offsetX <= ring; offsetX++) {
                if (FMath::Max(FMath::Abs(offsetX), FMath::Abs(offsetY)) != ring) {
                    continue; // interior of the ring was covered by an earlier pass
                }

                const int32 cy = cellY + offsetY;
                if (cy < 0 || cy >= grid.Height) {
                    continue;
                }

                int32 cx = cellX + offsetX;
                if (bWrapX) {
                    cx = ((cx % grid.Width) + grid.Width) % grid.Width;
                }
                else if (cx < 0 || cx >= grid.Width) {
                    continue;
                }

                for (int32 candidate : grid.Cells[cy * grid.Width + cx]) {
                    const FTriangleDataEntry& tri = triangles[candidate];
                    if (isWaterTerrain(tri.TerrainData)) {
                        continue;
                    }

                    const FPointDataEntry p1 = GetFirstPoint(tri.bB1, tri.E1, zoomCategory, tileX, tileY);
                    const FPointDataEntry p2 = GetFirstPoint(tri.bB2, tri.E2, zoomCategory, tileX, tileY);
                    const FPointDataEntry p3 = GetFirstPoint(tri.bB3, tri.E3, zoomCategory, tileX, tileY);

                    // <= so that on a tie the later candidate wins, matching the highest-index-wins
                    // draw-order priority GetTriangleIDAtCoordinate uses where triangles overlap.
                    const double distanceKm = distanceToTriangleKm(x, y, p1, p2, p3);
                    if (distanceKm <= bestDistanceKm) {
                        bestDistanceKm = distanceKm;
                        bestTriangle = candidate;
                    }
                }
            }
        }
    }

    return bestTriangle;
}

int32 FMapLowZoom::FindLandTriangleNear(int32 zoomCategory, double x, double y, double maxDistanceKm) {
    if (!HasZoomLevelData(zoomCategory)) {
        return INDEX_NONE;
    }

    const int32 w = TriangleData[zoomCategory].Num();
    const int32 h = TriangleData[zoomCategory][0].Num();

    // Which tiles the query circle touches. At level 1 that is the one tile there is; at level 3 a
    // tile is 22.5 degrees across and this is one tile almost everywhere, two or four for a point
    // near a boundary. Getting that handful right is the difference between a coastal place a few
    // kilometres over a tile edge finding its land and being told there is none: the mesh's y axis
    // puts a tile boundary through the middle of the Mediterranean at level 3.
    //
    // The longitude margin blows up towards the poles, where a few kilometres span many degrees, so
    // it is capped at half a turn -- past that the sweep would visit the same tile repeatedly.
    const double latitude = -y;
    const double kmPerDegreeX = FMath::Max(111.320 * FMath::Cos(FMath::DegreesToRadians(latitude)), 0.001);
    const double xMargin = FMath::Min(maxDistanceKm / kmPerDegreeX, 180.0);
    const double yMargin = maxDistanceKm / 110.574;

    const int32 firstTileX = FMath::FloorToInt32((x - xMargin + 180.0) / 360.0 * w);
    const int32 lastTileX = FMath::Min(FMath::FloorToInt32((x + xMargin + 180.0) / 360.0 * w), firstTileX + w - 1);
    const int32 firstTileY = FMath::Clamp(FMath::FloorToInt32((y - yMargin + 90.0) / 180.0 * h), 0, h - 1);
    const int32 lastTileY = FMath::Clamp(FMath::FloorToInt32((y + yMargin + 90.0) / 180.0 * h), 0, h - 1);

    int32 bestGlobal = INDEX_NONE;
    double bestDistanceKm = maxDistanceKm;

    for (int32 tileY = firstTileY; tileY <= lastTileY; tileY++) {
        for (int32 rawTileX = firstTileX; rawTileX <= lastTileX; rawTileX++) {
            // x wraps around the globe, so a circle straddling the antimeridian reaches tiles at
            // both ends of the row.
            const int32 tileX = floorMod(rawTileX, w);

            // bestDistanceKm is carried between tiles, so each one only searches for something
            // better than what has already been found and a later tile cannot overwrite a nearer
            // hit with a further one.
            const int32 local = FindLandTriangleInTile(zoomCategory, tileX, tileY, x, y, bestDistanceKm);
            if (local != INDEX_NONE) {
                bestGlobal = ToGlobalTriangle(zoomCategory, tileX, tileY, local);
            }
        }
    }

    return bestGlobal;
}

static FVector2D toScreen(double x, double y, double minX, double maxX, double minY, double maxY,
    int32 offsetX, int32 offsetY, double width, double height) {
    return FVector2D((x - minX) / (maxX - minX) * width + offsetX,
        (y - minY) / (maxY - minY) * height + offsetY);
}

static FCanvasUVTri makeTri(const FLinearColor& color,
    const FVector2D& A, const FVector2D& AUV,
    const FVector2D& B, const FVector2D& BUV,
    const FVector2D& C, const FVector2D& CUV) {
    FCanvasUVTri tri;
    tri.V0_Pos = A; tri.V0_UV = AUV; tri.V0_Color = color;
    tri.V1_Pos = B; tri.V1_UV = BUV; tri.V1_Color = color;
    tri.V2_Pos = C; tri.V2_UV = CUV; tri.V2_Color = color;
    return tri;
}

static void addDebugTriangles(TArray<FCanvasUVTri>& result, double x1, double y1, double x2, double y2, double d) {

}
static void convertEdgeToTriAndAdd(TArray<FCanvasUVTri>& result,
    double x1, double y1, double x2, double y2, double d, double minX, double maxX, double minY, double maxY, int32 offsetX, int32 offsetY, double width, double height) {
    // Project first, then thicken: d is a screen-space width, and the offsets only translate the quad.
    const FVector2D A = toScreen(x1, y1, minX, maxX, minY, maxY, offsetX, offsetY, width, height);
    const FVector2D B = toScreen(x2, y2, minX, maxX, minY, maxY, offsetX, offsetY, width, height);

    const double Dx = B.X - A.X;
    const double Dy = B.Y - A.Y;
    const double Len = FMath::Sqrt(Dx * Dx + Dy * Dy);

    double Ux = 1.0, Uy = 0.0;
    if (Len > UE_DOUBLE_SMALL_NUMBER)
    {
        Ux = Dx / Len;
        Uy = Dy / Len;
    }

    const double H = 0.5 * d;
    const FVector2D N(-Uy * H, Ux * H);

    const FVector2D P0 = A - N;
    const FVector2D P1 = B - N;
    const FVector2D P2 = B + N;
    const FVector2D P3 = A + N;

    const FLinearColor color(0, 0, 0, 1.f);

    const FVector2D UV0(0.0, 0.0);
    const FVector2D UV1(1.0, 0.0);
    const FVector2D UV2(1.0, 1.0);
    const FVector2D UV3(0.0, 1.0);

    result.Add(makeTri(color, P0, UV0, P1, UV1, P2, UV2));
    result.Add(makeTri(color, P0, UV0, P2, UV2, P3, UV3));
}

static FCanvasUVTri convertToTri(const FLinearColor& color,
    double x1, double y1, double x2, double y2, double x3, double y3, double minX, double maxX, double minY, double maxY, int32 offsetX, int32 offsetY, double width, double height) {
    const FVector2D P0 = toScreen(x1, y1, minX, maxX, minY, maxY, offsetX, offsetY, width, height);
    const FVector2D P1 = toScreen(x2, y2, minX, maxX, minY, maxY, offsetX, offsetY, width, height);
    const FVector2D P2 = toScreen(x3, y3, minX, maxX, minY, maxY, offsetX, offsetY, width, height);

    const double ss = 0.0075;
    const FVector2D UV0(ss * 20, ss * 20);
    const FVector2D UV1(ss * 22.5, ss * 20);
    const FVector2D UV2(ss * 25, ss * 25);

    return makeTri(color, P0, UV0, P1, UV1, P2, UV2);
}

// FTerrain::GetColor() hands back a loose rgb array, which is empty for a terrain code it has no
// entry for; treat that as black rather than reading off the end of it.
static FCanvasUVTri convertToTri(const TArray<uint8>& rgb,
    double x1, double y1, double x2, double y2, double x3, double y3, double minX, double maxX, double minY, double maxY, int32 offsetX, int32 offsetY, double width, double height) {
    const FLinearColor color = (rgb.Num() >= 3)
        ? FLinearColor(rgb[0] / 255.f, rgb[1] / 255.f, rgb[2] / 255.f, 1.f)
        : FLinearColor(0.f, 0.f, 0.f, 1.f);
    return convertToTri(color, x1, y1, x2, y2, x3, y3, minX, maxX, minY, maxY, offsetX, offsetY, width, height);
}

static FCanvasUVTri convertToTri(const FColor& rgb,
    double x1, double y1, double x2, double y2, double x3, double y3, double minX, double maxX, double minY, double maxY, int32 offsetX, int32 offsetY, double width, double height) {
    const FLinearColor color(rgb.R / 255.f, rgb.G / 255.f, rgb.B / 255.f, 1.f);
    return convertToTri(color, x1, y1, x2, y2, x3, y3, minX, maxX, minY, maxY, offsetX, offsetY, width, height);
}

// Domain pieces are drawn with this rather than convertToTri(), for two reasons that both come from
// their being small and numerous where the terrain triangles are large and few.
//
// The UVs are collapsed to a single point. convertToTri() spreads a fixed little UV triangle over
// whatever screen area a triangle covers, so the smaller the triangle the larger the UV step per
// pixel -- and a sampler picking its mip from that step gives a small triangle a different shade
// than a large one carrying the same colour. Terrain never showed it because its triangles are all
// big; a mesh triangle split five ways produces pieces small enough to shift.
//
// The piece is also grown by half a pixel about its own centre. Pieces meet along shared edges, and
// a rasteriser that misses a pixel on both sides of such an edge leaves the terrain showing through
// as a speck. Growing each piece makes neighbours overlap by a sub-pixel sliver instead, which is
// invisible, where a gap is not.
static FCanvasUVTri convertToDomainTri(const FColor& rgb,
    double x1, double y1, double x2, double y2, double x3, double y3, double minX, double maxX, double minY, double maxY, int32 offsetX, int32 offsetY, double width, double height) {

    FVector2D P0 = toScreen(x1, y1, minX, maxX, minY, maxY, offsetX, offsetY, width, height);
    FVector2D P1 = toScreen(x2, y2, minX, maxX, minY, maxY, offsetX, offsetY, width, height);
    FVector2D P2 = toScreen(x3, y3, minX, maxX, minY, maxY, offsetX, offsetY, width, height);

    const FVector2D centre = (P0 + P1 + P2) / 3.0;
    const double growPixels = 0.5;
    auto Grow = [&centre, growPixels](FVector2D& p) {
        const FVector2D away = p - centre;
        const double length = away.Size();
        if (length > UE_DOUBLE_SMALL_NUMBER) {
            p += away * (growPixels / length);
        }
    };
    Grow(P0); Grow(P1); Grow(P2);

    const FLinearColor color(rgb.R / 255.f, rgb.G / 255.f, rgb.B / 255.f, 1.f);
    const FVector2D UV(0.0075 * 20, 0.0075 * 20);
    return makeTri(color, P0, UV, P1, UV, P2, UV);
}

static int32 getMaxZoomLevel(FString ZoomLevelsPath) {
    TArray<FString> ZoomLevelNames = {};
    IFileManager::Get().FindFiles(ZoomLevelNames, *ZoomLevelsPath, false, true);
    int32 maxLevel = 0;
    for (FString s : ZoomLevelNames) {
        TArray<FString> stringArray = {};
        s.ParseIntoArray(stringArray, TEXT("_"), false);
        if (stringArray.Num() > 1) {
            int32 level = FCString::Atoi(*stringArray[1]);
            if (level > maxLevel) maxLevel = level;
        }
    }
    return maxLevel;
}

// Must match util.FileOperator's LEVEL_DATA_MAGIC/LEVEL_DATA_VERSION in the Java generator
// (burin-helper) exactly, or LevelData.bin's layout has diverged from what this reads.
static constexpr int32 LevelDataMagic = 0x4255524E; // "BURN"
// Version 2 added the cross-layer section. Version 1 files still load: they simply have no
// overlaps recorded, which is exactly what a version 2 file with an empty section looks like, so
// nothing downstream needs to know the difference. Rejecting them would mean no map at all until
// every level had been regenerated, for a section that is optional by construction.
static constexpr int32 LevelDataVersionMin = 1;
static constexpr int32 LevelDataVersionCurrent = 2;
static constexpr int32 LevelDataVersionCrossLayer = 2;

// Reads one tile's worth of data (matching the layout FileOperator.writeTileBinary writes)
// sequentially from Ar into freshly-sized TriangleData/EdgeData/PointData/Grid entries.
static void ReadTileBinary(FMemoryReader& Ar, int32 version, TArray<FTriangleDataEntry>& Triangles, TArray<FEdgeDataEntry>& Edges, TArray<FPointDataEntry>& Points, FGridDataEntry& Grid) {
    int32 numTriangles = 0;
    Ar << numTriangles;
    Triangles.SetNum(numTriangles);
    for (int32 i = 0; i < numTriangles; i++) {
        uint8 b1 = 0, b2 = 0, b3 = 0;
        Ar << Triangles[i].E1 << Triangles[i].E2 << Triangles[i].E3;
        Ar << b1 << b2 << b3;
        Ar << Triangles[i].TerrainData;
        Triangles[i].bB1 = b1 != 0;
        Triangles[i].bB2 = b2 != 0;
        Triangles[i].bB3 = b3 != 0;
    }

    int32 numEdges = 0;
    Ar << numEdges;
    Edges.SetNum(numEdges);
    for (int32 i = 0; i < numEdges; i++) {
        Ar << Edges[i].P1 << Edges[i].P2 << Edges[i].T1 << Edges[i].T2 << Edges[i].RiverData;
    }

    int32 numPoints = 0;
    Ar << numPoints;
    Points.SetNum(numPoints);
    for (int32 i = 0; i < numPoints; i++) {
        Ar << Points[i].X << Points[i].Y;
        int32 numNeighbors = 0;
        Ar << numNeighbors;
        Points[i].Neighbors.SetNum(numNeighbors);
        for (int32 n = 0; n < numNeighbors; n++) {
            Ar << Points[i].Neighbors[n];
        }
    }

    // Cross-layer section, written after the points and before the grid. Absent entirely before
    // version 2; within version 2 a count of zero means a tile that genuinely has no overlaps.
    int32 numLayerEntries = 0;
    if (version >= LevelDataVersionCrossLayer) {
        Ar << numLayerEntries;
    }
    for (int32 i = 0; i < numLayerEntries; i++) {
        double coveredArea = 0.0;
        int32 numCrossLayer = 0;
        Ar << coveredArea;
        Ar << numCrossLayer;
        TArray<int32> crossLayer;
        crossLayer.SetNum(numCrossLayer);
        for (int32 k = 0; k < numCrossLayer; k++) {
            Ar << crossLayer[k];
        }
        if (Triangles.IsValidIndex(i)) {
            Triangles[i].CoveredArea = coveredArea;
            Triangles[i].CrossLayer = MoveTemp(crossLayer);
        }
    }

    Ar << Grid.Width << Grid.Height;
    int32 numCells = Grid.Width * Grid.Height;
    Grid.Cells.SetNum(numCells);
    for (int32 c = 0; c < numCells; c++) {
        int32 numCandidates = 0;
        Ar << numCandidates;
        Grid.Cells[c].SetNum(numCandidates);
        for (int32 k = 0; k < numCandidates; k++) {
            Ar << Grid.Cells[c][k];
        }
    }
}

void FMapLowZoom::Initialize() {
    TriangleData = {};
    EdgeData = {};
    PointData = {};
    GridData = {};

    // All three are derived from the data below and index into it, so a reload drops them rather
    // than leaving ids pointing into a mesh that no longer exists.
    DomainData = {};
    DomainsBuilt = {};
    TriangleBases = {};

    // zoom levels
    int32 maxLevel = getMaxZoomLevel(FPaths::ProjectContentDir() + TEXT("Data/Earth/Polygons/*"));
    UE_LOG(LogTemp, Log, TEXT("Number of zoom levels: %d"), maxLevel);

    // Each level is independent (own file, own slot below), so levels load in parallel.
    TriangleData.SetNum(maxLevel);
    EdgeData.SetNum(maxLevel);
    PointData.SetNum(maxLevel);
    GridData.SetNum(maxLevel);

    ParallelFor(maxLevel, [this](int32 level) {
        FString binPath = FPaths::ProjectContentDir() + TEXT("Data/Earth/Polygons/Level_") + FString::FromInt(level + 1) + TEXT("/LevelData.bin");

        TArray<uint8> fileBytes;
        if (!FFileHelper::LoadFileToArray(fileBytes, *binPath)) {
            UE_LOG(LogTemp, Error, TEXT("Failed to load level binary: %s"), *binPath);
            return;
        }

        FMemoryReader Ar(fileBytes, true);

        int32 magic = 0, version = 0, w = 0, h = 0;
        Ar << magic << version << w << h;

        if (magic != LevelDataMagic) {
            UE_LOG(LogTemp, Error, TEXT("Bad magic number in level binary (expected %d, got %d): %s"), LevelDataMagic, magic, *binPath);
            return;
        }
        if (version < LevelDataVersionMin || version > LevelDataVersionCurrent) {
            UE_LOG(LogTemp, Error, TEXT("Unsupported level binary version (expected %d to %d, got %d): %s"), LevelDataVersionMin, LevelDataVersionCurrent, version, *binPath);
            return;
        }

        UE_LOG(LogTemp, Log, TEXT("Zoom level %d: width = %d, height = %d"), level, w, h);

        TArray < TArray < TArray <FTriangleDataEntry> > > TriangleDataForZoomLevel;
        TArray < TArray < TArray <FEdgeDataEntry> > > EdgeDataForZoomLevel;
        TArray < TArray < TArray <FPointDataEntry> > > PointDataForZoomLevel;
        TArray < TArray <FGridDataEntry> > GridDataForZoomLevel;

        TriangleDataForZoomLevel.SetNum(w);
        EdgeDataForZoomLevel.SetNum(w);
        PointDataForZoomLevel.SetNum(w);
        GridDataForZoomLevel.SetNum(w);

        for (int32 x = 0; x < w; x++) {
            TriangleDataForZoomLevel[x].SetNum(h);
            EdgeDataForZoomLevel[x].SetNum(h);
            PointDataForZoomLevel[x].SetNum(h);
            GridDataForZoomLevel[x].SetNum(h);

            for (int32 y = 0; y < h; y++) {
                ReadTileBinary(Ar, version, TriangleDataForZoomLevel[x][y], EdgeDataForZoomLevel[x][y], PointDataForZoomLevel[x][y], GridDataForZoomLevel[x][y]);

                UE_LOG(LogTemp, Log, TEXT("Number of triangles, edges, points (zoom level: %d, x: %d, y: %d): %d, %d, %d"), level, x, y, TriangleDataForZoomLevel[x][y].Num(), EdgeDataForZoomLevel[x][y].Num(), PointDataForZoomLevel[x][y].Num());
            }
        }

        // Each thread only ever writes its own `level` slot, which was pre-sized
        // above via SetNum() before ParallelFor started, so this is race-free.
        TriangleData[level] = MoveTemp(TriangleDataForZoomLevel);
        EdgeData[level] = MoveTemp(EdgeDataForZoomLevel);
        PointData[level] = MoveTemp(PointDataForZoomLevel);
        GridData[level] = MoveTemp(GridDataForZoomLevel);
    });
}

TArray<FCanvasUVTri> FMapLowZoom::GetTriangles(FTerrain* terrain, int32 mode, int32 zoomCategory, double minFracY, double minFracX, double maxFracY, double maxFracX, int32 offsetX, int32 offsetY, int32 width, int32 height) {
    TArray<FCanvasUVTri> result = {};
    if (mode == 0) return result;
    if (!HasZoomLevelData(zoomCategory)) return result;

    int32 xBase = FMath::RoundToInt32(minFracX / (maxFracX - minFracX));
    int32 yBase = FMath::RoundToInt32(minFracY / (maxFracY - minFracY));
    int32 tileX = floorMod(xBase, TriangleData[zoomCategory].Num());
    int32 tileY = floorMod(yBase, TriangleData[zoomCategory][0].Num());

    double minX = 360 * minFracX - 180;
    double maxX = 360 * maxFracX - 180;
    if (xBase < 0) minX += 360;
    if (xBase < 0) maxX += 360;
    if (xBase >= TriangleData[zoomCategory].Num()) minX -= 360;
    if (xBase >= TriangleData[zoomCategory].Num()) maxX -= 360;
    double minY = 180 * minFracY - 90;
    double maxY = 180 * maxFracY - 90;

    UE_LOG(LogTemp, Log, TEXT("Min/max X and Y : %f, %f ; %f, %f, tileX and tileY: %i, %i"), minX, maxX, minY, maxY, tileX, tileY);

    if (mode == 8) {
        TArray<TArray<uint8>> colors;
        uint8 stepsize = 64;
        for (uint8 k = 0; k <= 256 / stepsize; k++) {
            for (uint8 l = 0; l <= 256 / stepsize; l++) {
                for (uint8 m = 0; m <= 256 / stepsize; m++) {
                    uint8 r = FMath::Min<int32>(stepsize * k, 255);
                    uint8 g = FMath::Min<int32>(stepsize * l, 255);
                    uint8 b = FMath::Min<int32>(stepsize * m, 255);
                    colors.Add({r, g, b});
                }
            }
        }
        for (int32 i = colors.Num() - 1; i > 0; --i)
        {
            srand(42);
            int32 j = (rand() % i);
            colors.Swap(i, j);
        }

        for (int32 i = 0; i < TriangleData[zoomCategory][tileX][tileY].Num(); i++) {
            FPointDataEntry p1 = GetFirstPoint(TriangleData[zoomCategory][tileX][tileY][i].bB1, TriangleData[zoomCategory][tileX][tileY][i].E1, zoomCategory, tileX, tileY);
            FPointDataEntry p2 = GetFirstPoint(TriangleData[zoomCategory][tileX][tileY][i].bB2, TriangleData[zoomCategory][tileX][tileY][i].E2, zoomCategory, tileX, tileY);
            FPointDataEntry p3 = GetFirstPoint(TriangleData[zoomCategory][tileX][tileY][i].bB3, TriangleData[zoomCategory][tileX][tileY][i].E3, zoomCategory, tileX, tileY);
            result.Add(convertToTri(colors[i % colors.Num()], p1.X, p1.Y, p2.X, p2.Y, p3.X, p3.Y, minX, maxX, minY, maxY, offsetX, offsetY, width, height));
        }

        return result;
    }

    for (int32 i = 0; i < TriangleData[zoomCategory][tileX][tileY].Num(); i++) {
        FPointDataEntry p1 = GetFirstPoint(TriangleData[zoomCategory][tileX][tileY][i].bB1, TriangleData[zoomCategory][tileX][tileY][i].E1, zoomCategory, tileX, tileY);
        FPointDataEntry p2 = GetFirstPoint(TriangleData[zoomCategory][tileX][tileY][i].bB2, TriangleData[zoomCategory][tileX][tileY][i].E2, zoomCategory, tileX, tileY);
        FPointDataEntry p3 = GetFirstPoint(TriangleData[zoomCategory][tileX][tileY][i].bB3, TriangleData[zoomCategory][tileX][tileY][i].E3, zoomCategory, tileX, tileY);
        result.Add(convertToTri(terrain->GetColor(TriangleData[zoomCategory][tileX][tileY][i].TerrainData, mode), p1.X, p1.Y, p2.X, p2.Y, p3.X, p3.Y, minX, maxX, minY, maxY, offsetX, offsetY, width, height));
    }

    if (mode == 7) AddBordersAsTriangles(result, zoomCategory, tileX, tileY, minX, maxX, minY, maxY, offsetX, offsetY, width, height);

    UE_LOG(LogTemp, Log, TEXT("Num render triangles: %d"), result.Num());
    return result;
}

TArray<FCanvasUVTri>& FMapLowZoom::AddBordersAsTriangles(TArray<FCanvasUVTri>& result, int32 zoomCategory, int32 tileX, int32 tileY, double minX, double maxX, double minY, double maxY, int32 offsetX, int32 offsetY, int32 width, int32 height) {
    int32 c = 0;
    for (FEdgeDataEntry& edgeData : EdgeData[zoomCategory][tileX][tileY]) {
        const double x1 = PointData[zoomCategory][tileX][tileY][edgeData.P1].X;
        const double y1 = PointData[zoomCategory][tileX][tileY][edgeData.P1].Y;
        const double x2 = PointData[zoomCategory][tileX][tileY][edgeData.P2].X;
        const double y2 = PointData[zoomCategory][tileX][tileY][edgeData.P2].Y;

        if (edgeData.T2 >= 0) {
            bool i1 = TriangleData[zoomCategory][tileX][tileY][edgeData.T1].TerrainData % 16 == 0;
            bool i2 = TriangleData[zoomCategory][tileX][tileY][edgeData.T2].TerrainData % 16 == 0;
            if (i1 != i2) {
                convertEdgeToTriAndAdd(result, x1, y1, x2, y2, 5, minX, maxX, minY, maxY, offsetX, offsetY, width, height);
                c += 2;
            }
        }
        else  if (edgeData.T2 < -1) {
            bool i1 = TriangleData[zoomCategory][tileX][tileY][edgeData.T1].TerrainData % 16 == 0;
            bool i2 = (-edgeData.T2 - 2) % 16 == 0;
            if (i1 != i2) {
                convertEdgeToTriAndAdd(result, x1, y1, x2, y2, 5, minX, maxX, minY, maxY, offsetX, offsetY, width, height);
                c += 2;
            }
        }
        if (edgeData.RiverData >= 0) {
            bool i1 = true;
            bool i2 = true;
            if (edgeData.T1 >= 0) i1 = TriangleData[zoomCategory][tileX][tileY][edgeData.T1].TerrainData % 16 != 0;
            if (edgeData.T2 >= 0) i2 = TriangleData[zoomCategory][tileX][tileY][edgeData.T2].TerrainData % 16 != 0;
            if (i1 && i2) {
                convertEdgeToTriAndAdd(result, x1, y1, x2, y2, 5, minX, maxX, minY, maxY, offsetX, offsetY, width, height);
                c += 2;
            }
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Number of border lines: %d"), c);
    return result;
}


TArray<FCanvasUVTri> FMapLowZoom::GetMaterialTriangles(FTerrain* terrain, int32 mode, int32 zoomCategory, int32 tileX, int32 tileY) {
    TArray<FCanvasUVTri> result = {};
    if (mode > 0) return result;
    if (!HasTileData(zoomCategory, tileX, tileY)) return result;

    for (int32 i = 0; i < TriangleData[zoomCategory][tileX][tileY].Num(); i++) {
        FPointDataEntry p1 = GetFirstPoint(TriangleData[zoomCategory][tileX][tileY][i].bB1, TriangleData[zoomCategory][tileX][tileY][i].E1, zoomCategory, tileX, tileY);
        FPointDataEntry p2 = GetFirstPoint(TriangleData[zoomCategory][tileX][tileY][i].bB2, TriangleData[zoomCategory][tileX][tileY][i].E2, zoomCategory, tileX, tileY);
        FPointDataEntry p3 = GetFirstPoint(TriangleData[zoomCategory][tileX][tileY][i].bB3, TriangleData[zoomCategory][tileX][tileY][i].E3, zoomCategory, tileX, tileY);
        result.Add(convertToTri(terrain->GetColor(TriangleData[zoomCategory][tileX][tileY][i].TerrainData, mode), p1.X, p1.Y, p2.X, p2.Y, p3.X, p3.Y, -180, 180, -90, 90, 0, 0, 16384, 16384));
    }


    return result;
}

TArray<FLineDisplayData> FMapLowZoom::GetBorders(int32 mode, int32 zoomCategory, int32 tileX, int32 tileY) {
    if (mode != 6) return {};
    if (!HasTileData(zoomCategory, tileX, tileY)) return {};
    TArray<FLineDisplayData> result = {};
    for (FEdgeDataEntry& edgeData : EdgeData[zoomCategory][tileX][tileY]) {
        if (edgeData.T2 >= 0) {
            bool i1 = TriangleData[zoomCategory][tileX][tileY][edgeData.T1].TerrainData % 16 == 0;
            bool i2 = TriangleData[zoomCategory][tileX][tileY][edgeData.T2].TerrainData % 16 == 0;
            if (i1 != i2) {
                double x1 = (PointData[zoomCategory][tileX][tileY][edgeData.P1].X + 180) / 360 * 16384;
                double y1 = (PointData[zoomCategory][tileX][tileY][edgeData.P1].Y + 90) / 180 * 16384;
                double x2 = (PointData[zoomCategory][tileX][tileY][edgeData.P2].X + 180) / 360 * 16384;
                double y2 = (PointData[zoomCategory][tileX][tileY][edgeData.P2].Y + 90) / 180 * 16384;
                result.Add(FLineDisplayData{ FVector2D{ x1, y1 }, FVector2D{ x2, y2 } });
            }
        }
        else  if (edgeData.T2 < -1) {
            bool i1 = TriangleData[zoomCategory][tileX][tileY][edgeData.T1].TerrainData % 16 == 0;
            bool i2 = (-edgeData.T2-2) % 16 == 0;
            if (i1 != i2) {
                double x1 = (PointData[zoomCategory][tileX][tileY][edgeData.P1].X + 180) / 360 * 16384;
                double y1 = (PointData[zoomCategory][tileX][tileY][edgeData.P1].Y + 90) / 180 * 16384;
                double x2 = (PointData[zoomCategory][tileX][tileY][edgeData.P2].X + 180) / 360 * 16384;
                double y2 = (PointData[zoomCategory][tileX][tileY][edgeData.P2].Y + 90) / 180 * 16384;
                result.Add(FLineDisplayData{ FVector2D{ x1, y1 }, FVector2D{ x2, y2 } });
            }
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Number of border lines: %d"), result.Num());
    return result;
}

TArray<FLineDisplayData> FMapLowZoom::GetRivers(int32 mode, int32 zoomCategory, int32 tileX, int32 tileY) {
    if (mode != 6) return {};
    if (!HasTileData(zoomCategory, tileX, tileY)) return {};
    TArray<FLineDisplayData> result = {};
    for (FEdgeDataEntry& edgeData : EdgeData[zoomCategory][tileX][tileY]) {
        if (edgeData.RiverData >= 0) {
            double x1 = (PointData[zoomCategory][tileX][tileY][edgeData.P1].X + 180) / 360 * 16384;
            double y1 = (PointData[zoomCategory][tileX][tileY][edgeData.P1].Y + 90) / 180 * 16384;
            double x2 = (PointData[zoomCategory][tileX][tileY][edgeData.P2].X + 180) / 360 * 16384;
            double y2 = (PointData[zoomCategory][tileX][tileY][edgeData.P2].Y + 90) / 180 * 16384;
            result.Add(FLineDisplayData{ FVector2D{ x1, y1 }, FVector2D{ x2, y2 } });
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Number of river lines: %d"), result.Num());
    return result;
}



// The domain radius is a circle, and clipping a triangle to a circle exactly would leave an arc.
// This many straight segments stand in for that arc. The polygon is inscribed, so a fragment never
// reaches past the radius; at 200 km, 48 segments sit within 0.43 km of the true circle, which is
// well under a pixel at any zoom this map draws.
static constexpr int32 DomainClipSegments = 48;

static double distanceFromKm(const FVector2D& point, const FVector2D& origin) {
    return toLocalKm(point.X, point.Y, origin.X, origin.Y).Size();
}

// Inverse of toLocalKm().
static FVector2D fromLocalKm(const FVector2D& local, const FVector2D& origin) {
    const double kmPerDegreeX = 111.320 * FMath::Cos(FMath::DegreesToRadians(origin.Y));
    const double kmPerDegreeY = 110.574;

    double x = origin.X + ((FMath::Abs(kmPerDegreeX) > UE_DOUBLE_SMALL_NUMBER) ? local.X / kmPerDegreeX : 0.0);
    if (x > 180.0) x -= 360.0;
    else if (x < -180.0) x += 360.0;

    return FVector2D(x, origin.Y + local.Y / kmPerDegreeY);
}

// Sutherland-Hodgman: keeps the part of a convex polygon satisfying dot(p, normal) <= offset.
static void clipPolygonToHalfPlane(const TArray<FVector2D>& polygon, const FVector2D& normal, double offset, TArray<FVector2D>& outClipped) {
    outClipped.Reset();

    for (int32 i = 0; i < polygon.Num(); i++) {
        const FVector2D& a = polygon[i];
        const FVector2D& b = polygon[(i + 1) % polygon.Num()];

        const double aOutside = FVector2D::DotProduct(a, normal) - offset;
        const double bOutside = FVector2D::DotProduct(b, normal) - offset;

        if (aOutside <= 0.0) {
            outClipped.Add(a);
        }
        if ((aOutside > 0.0) != (bOutside > 0.0)) {
            outClipped.Add(a + (b - a) * (aOutside / (aOutside - bOutside)));
        }
    }
}

// Splits one mesh triangle between the places contesting it, clips each share to that place's
// radius, and appends the pieces.
//
// Everything here is an intersection of convex shapes, which is why it can be done exactly. The
// ground nearer to one place than to another is the half-plane on its side of the perpendicular
// bisector; a place's share of the triangle is the triangle intersected with one such half-plane
// per rival; and the radius is a disc, approximated by a polygon inscribed in it. Clipping a convex
// polygon against each in turn leaves the exact share, and fanning it gives a few pieces.
//
// Without this the seam between two domains had to follow whole triangles, so it inherited the
// triangulation's jaggedness -- one 3387 km2 triangle south of Mari went entirely to Terqa when
// Tadmur had a fair claim to part of it. Domains with no neighbour never showed the problem, since
// nothing contested them.
static void clipTriangleToDomains(int32 triangleIndex, const FVector2D& a, const FVector2D& b, const FVector2D& c,
    const TArray<int32>& contenders, const TArray<FVector2D>& placePosition, double radiusKm,
    TArray<FDomainFragment>& outFragments) {

    // One local frame for the whole triangle, so the triangle and every place contesting it are in
    // the same kilometre coordinates.
    const FVector2D origin((a.X + b.X + c.X) / 3.0, (a.Y + b.Y + c.Y) / 3.0);

    TArray<FVector2D> placeLocal;
    placeLocal.Reserve(contenders.Num());
    for (int32 placeIndex : contenders) {
        placeLocal.Add(toLocalKm(placePosition[placeIndex].X, placePosition[placeIndex].Y, origin.X, origin.Y));
    }

    const double apothem = radiusKm * FMath::Cos(UE_DOUBLE_PI / DomainClipSegments);

    TArray<FVector2D> polygon;
    TArray<FVector2D> clipped;

    for (int32 i = 0; i < contenders.Num(); i++) {
        polygon.Reset();
        polygon.Add(toLocalKm(a.X, a.Y, origin.X, origin.Y));
        polygon.Add(toLocalKm(b.X, b.Y, origin.X, origin.Y));
        polygon.Add(toLocalKm(c.X, c.Y, origin.X, origin.Y));

        const FVector2D& mine = placeLocal[i];

        // Keep only what is nearer to this place than to each rival. |x-mine| <= |x-other| is the
        // half-plane dot(x, other - mine) <= (|other|^2 - |mine|^2) / 2.
        for (int32 j = 0; j < contenders.Num() && polygon.Num() >= 3; j++) {
            if (j == i) {
                continue;
            }

            const FVector2D toRival = placeLocal[j] - mine;
            const double separation = toRival.Size();
            if (separation <= UE_DOUBLE_SMALL_NUMBER) {
                // Two places standing on the same spot have no bisector; the lower index takes it,
                // so the triangle is still covered exactly once.
                if (contenders[j] < contenders[i]) {
                    polygon.Reset();
                    break;
                }
                continue;
            }

            // The halfway point along the line between the two places. Because the two sides use
            // the same divide, and their offsets from each place sum to the separation exactly, the
            // shares tile the triangle with no seam and no overlap between them.
            const FVector2D direction = toRival / separation;
            clipPolygonToHalfPlane(polygon, direction, FVector2D::DotProduct(mine, direction) + 0.5 * separation, clipped);
            Swap(polygon, clipped);
        }

        // Then to this place's own radius. In this shared frame the disc is centred on the place
        // rather than the origin, which just shifts each half-plane by the place's own offset.
        for (int32 segment = 0; segment < DomainClipSegments && polygon.Num() >= 3; segment++) {
            const double angle = 2.0 * UE_DOUBLE_PI * segment / DomainClipSegments;
            const FVector2D normal(FMath::Cos(angle), FMath::Sin(angle));
            clipPolygonToHalfPlane(polygon, normal, apothem + FVector2D::DotProduct(mine, normal), clipped);
            Swap(polygon, clipped);
        }

        if (polygon.Num() < 3) {
            continue; // this place ends up with none of this triangle
        }

        const FVector2D fan = fromLocalKm(polygon[0], origin);
        for (int32 k = 1; k + 1 < polygon.Num(); k++) {
            outFragments.Add(FDomainFragment{ contenders[i], triangleIndex, fan, fromLocalKm(polygon[k], origin), fromLocalKm(polygon[k + 1], origin) });
        }
    }
}

void FMapLowZoom::BuildPlaceDomains(TArray<FPlace>& places, const TArray<FPolity>& polities, int32 zoomCategory, double maxRadiusKm, bool bRiversBlockDomains) {
    for (FPlace& place : places) {
        place.Triangles.Reset();
    }

    if (!HasZoomLevelData(zoomCategory)) {
        return;
    }

    // Timed in three parts, because they scale with different things and only one of them has to
    // be paid again when the year changes. The stitch depends on the mesh alone -- the same links
    // come out whatever the places are -- so if it turns out to dominate here, caching it per level
    // is the fix. Worth knowing the number before restructuring anything for it.
    const double startSeconds = FPlatformTime::Seconds();

    const int32 w = TriangleData[zoomCategory].Num();
    const int32 h = TriangleData[zoomCategory][0].Num();

    // Everything from here to phase two works in global triangle ids (see ToGlobalTriangle). A
    // 200 km domain crosses tile boundaries freely at levels 2 and 3 -- a level-3 tile is 22.5
    // degrees across, and a boundary runs through the middle of the Mediterranean -- so the flood
    // cannot be written in indices that restart at every tile. The mesh's own records (edge T1/T2,
    // cross-layer links, fragments) stay tile-local and are converted where they are read.
    EnsureTriangleBases(zoomCategory);
    const int32 totalTriangles = NumGlobalTriangles(zoomCategory);

    // Where each place stands, in mesh degrees, and the colour its domain is painted. The colour is
    // resolved now, while the polities are to hand, so that drawing needs nothing but the tile. A
    // place whose owner never resolved to a polity keeps the zero alpha and goes unpainted.
    TArray<FVector2D> placePosition;
    placePosition.SetNum(places.Num());
    TArray<FColor> placeColor;
    placeColor.Init(FColor(0, 0, 0, 0), places.Num());

    for (int32 placeIndex = 0; placeIndex < places.Num(); placeIndex++) {
        const FPlace& place = places[placeIndex];
        placePosition[placeIndex] = FVector2D(place.Longitude, -place.Latitude);

        if (polities.IsValidIndex(place.ControllerIndex)) {
            placeColor[placeIndex] = polities[place.ControllerIndex].MapColor1;
            placeColor[placeIndex].A = 255;
        }
    }

    DomainData.SetNum(TriangleData.Num());
    DomainData[zoomCategory].Reset();
    DomainData[zoomCategory].SetNum(w);
    for (int32 tx = 0; tx < w; tx++) {
        DomainData[zoomCategory][tx].SetNum(h);
        for (int32 ty = 0; ty < h; ty++) {
            FTileDomains& tileDomains = DomainData[zoomCategory][tx][ty];
            tileDomains.Reset(TriangleData[zoomCategory][tx][ty].Num(), places.Num());
            tileDomains.PlaceColor = placeColor;
        }
    }

    // Reading a triangle through its global id. Resolving one costs a binary search over the
    // level's tiles, so anything walked per triangle resolves once and reads the tile arrays
    // through the result rather than calling these in a loop.
    auto TileOf = [this, zoomCategory](int32 global, int32& outTileX, int32& outTileY, int32& outLocal) {
        return FromGlobalTriangle(zoomCategory, global, outTileX, outTileY, outLocal);
    };
    auto TerrainOf = [&](int32 global) -> int32 {
        int32 tx = 0, ty = 0, local = INDEX_NONE;
        if (!TileOf(global, tx, ty, local)) {
            return 0; // reads as water, so nothing walks into a triangle that does not resolve
        }
        return TriangleData[zoomCategory][tx][ty][local].TerrainData;
    };
    auto VerticesOf = [&](int32 global, FPointDataEntry& p1, FPointDataEntry& p2, FPointDataEntry& p3) -> bool {
        int32 tx = 0, ty = 0, local = INDEX_NONE;
        if (!TileOf(global, tx, ty, local)) {
            return false;
        }
        GetTriangleVertices(zoomCategory, tx, ty, local, p1, p2, p3);
        return true;
    };

    // ---- Links the edge records do not carry ----
    //
    // A triangle's neighbours come from FEdgeDataEntry::T1/T2, and an edge that was never paired at
    // generation time carries no neighbour at all: T2 holds the opposing terrain in -terrain-2 form
    // instead of an index. Those are boundaries where two regions describe the same ground with
    // different vertices, and the flood stops dead at every one of them. At level 1 that used to
    // sever 71,594 km2 of Canada from its own mainland.
    //
    // At levels 2 and 3 the same records also stop at every tile boundary, since a tile's edges are
    // paired only within that tile. That is what used to make multi-tile domains impossible, and it
    // needs no separate fix: a tile boundary is simply another unpaired edge, and the probe below
    // steps across it into the neighbouring tile like any other.
    //
    // The far side is found geometrically rather than trusted: step a little past the edge's
    // midpoint, away from the triangle that owns it, and ask what is there. Done once here rather
    // than inside the flood, which walks every triangle again for every place.
    //
    // A link to water is harmless -- the flood's own coastline test discards it -- so no terrain
    // check is made here. Better to record the adjacency and let one place decide what blocks.
    TArray<TArray<FStitchedLink>> stitched;
    stitched.SetNum(totalTriangles);
    int32 stitchedLinks = 0;
    int32 stitchedRivers = 0;
    int32 unpairedEdges = 0;
    {
        // ~11 m: past any rounding in the point coordinates, which are stored to six decimal
        // places, and far short of the smallest triangle the mesh contains.
        const double probeDegrees = 1e-4;

        auto AddLink = [&stitched](int32 from, int32 to, bool bIsRiver) {
            for (FStitchedLink& existing : stitched[from]) {
                if (existing.Triangle == to) {
                    existing.bRiver = existing.bRiver || bIsRiver;
                    return;
                }
            }
            stitched[from].Add(FStitchedLink{ to, bIsRiver });
        };

        for (int32 tileX = 0; tileX < w; tileX++) {
            for (int32 tileY = 0; tileY < h; tileY++) {
                const TArray<FTriangleDataEntry>& tileTriangles = TriangleData[zoomCategory][tileX][tileY];
                const TArray<FEdgeDataEntry>& tileEdges = EdgeData[zoomCategory][tileX][tileY];
                const TArray<FPointDataEntry>& tilePoints = PointData[zoomCategory][tileX][tileY];
                const int32 base = ToGlobalTriangle(zoomCategory, tileX, tileY, 0);

                for (const FEdgeDataEntry& edge : tileEdges) {
                    if (edge.T1 >= 0 && edge.T2 >= 0) {
                        continue; // already paired
                    }
                    const int32 ownerLocal = (edge.T1 >= 0) ? edge.T1 : edge.T2;
                    if (!tileTriangles.IsValidIndex(ownerLocal)
                        || !tilePoints.IsValidIndex(edge.P1) || !tilePoints.IsValidIndex(edge.P2)) {
                        continue;
                    }
                    unpairedEdges++;

                    const FPointDataEntry& a = tilePoints[edge.P1];
                    const FPointDataEntry& b = tilePoints[edge.P2];
                    FPointDataEntry q1, q2, q3;
                    GetTriangleVertices(zoomCategory, tileX, tileY, ownerLocal, q1, q2, q3);

                    const FVector2D mid(0.5 * (a.X + b.X), 0.5 * (a.Y + b.Y));
                    const FVector2D centre((q1.X + q2.X + q3.X) / 3.0, (q1.Y + q2.Y + q3.Y) / 3.0);
                    FVector2D away = mid - centre;
                    const double length = away.Size();
                    if (length <= UE_DOUBLE_SMALL_NUMBER) {
                        continue; // a degenerate triangle has no outward direction to speak of
                    }
                    away /= length;

                    double probeX = mid.X + away.X * probeDegrees;
                    const double probeY = mid.Y + away.Y * probeDegrees;
                    if (probeX > 180.0) probeX -= 360.0;
                    else if (probeX < -180.0) probeX += 360.0;

                    // The tile comes back from the lookup because the probe may well have landed in
                    // a different one -- that is the whole point of it at a tile boundary -- and the
                    // index it returns is local to whichever tile that was.
                    int32 acrossTileX = 0, acrossTileY = 0;
                    const int32 acrossLocal = GetTriangleIDAtCoordinate(zoomCategory, probeX, probeY, acrossTileX, acrossTileY);
                    if (acrossLocal < 0) {
                        continue; // outside the mesh
                    }
                    const int32 acrossGlobal = ToGlobalTriangle(zoomCategory, acrossTileX, acrossTileY, acrossLocal);
                    const int32 ownerGlobal = base + ownerLocal;
                    if (acrossGlobal == INDEX_NONE || acrossGlobal == ownerGlobal) {
                        continue; // the probe never left the triangle
                    }

                    // Whether a river runs along this boundary. The near edge may carry the flag,
                    // but so may the far side alone: where the two sides split the boundary
                    // differently, the side that kept the whole stretch as one edge is the side
                    // holding the river. Miss that and a domain walks across a river meant to stop
                    // it. The far side's edges and points live in the far side's tile.
                    //
                    // Coordinates are compared as stored, so a river lying exactly along the
                    // antimeridian -- where the two sides read as 180 and -180 -- would not match,
                    // and a river there would fail to block a domain. The line crosses land in
                    // Chukotka, Fiji and Antarctica, so this is narrow rather than impossible; the
                    // same seam is why a domain straddling it gets fragment corners from
                    // fromLocalKm() that fall outside [-180, 180).
                    bool bRiver = edge.RiverData >= 0;
                    if (!bRiver) {
                        const TArray<FEdgeDataEntry>& farEdgeData = EdgeData[zoomCategory][acrossTileX][acrossTileY];
                        const TArray<FPointDataEntry>& farPointData = PointData[zoomCategory][acrossTileX][acrossTileY];
                        const FTriangleDataEntry& farTriangle = TriangleData[zoomCategory][acrossTileX][acrossTileY][acrossLocal];
                        const int32 farEdges[3] = { farTriangle.E1, farTriangle.E2, farTriangle.E3 };
                        for (int32 farEdgeIndex : farEdges) {
                            if (!farEdgeData.IsValidIndex(farEdgeIndex)) {
                                continue;
                            }
                            const FEdgeDataEntry& farEdge = farEdgeData[farEdgeIndex];
                            if (farEdge.RiverData < 0
                                || !farPointData.IsValidIndex(farEdge.P1) || !farPointData.IsValidIndex(farEdge.P2)) {
                                continue;
                            }
                            if (segmentsLieAlong(a, b, farPointData[farEdge.P1], farPointData[farEdge.P2], probeDegrees)) {
                                bRiver = true;
                                break;
                            }
                        }
                    }

                    AddLink(ownerGlobal, acrossGlobal, bRiver);
                    AddLink(acrossGlobal, ownerGlobal, bRiver);
                    stitchedLinks++;
                    if (bRiver) stitchedRivers++;
                }
            }
        }
    }

    const double stitchedSeconds = FPlatformTime::Seconds() - startSeconds;

    // ---- Phase one: what each place can reach, worked out for one place at a time ----
    //
    // No competition here: reach sets overlap freely, and a triangle collects every place that can
    // walk to it. Deciding ownership during the flood was the flaw in the previous version -- a
    // triangle went whole to whoever was nearest to any one of its corners, which for a 278 km
    // sliver is very nearly arbitrary, and the border between two domains then depended on that
    // coin-toss instead of on where the two places actually are.
    TArray<TArray<int32>> contendersOf;
    contendersOf.SetNum(totalTriangles);

    TArray<int32> reachedBy;             // which place last reached a triangle, so no clearing per place
    reachedBy.Init(INDEX_NONE, totalTriangles);

    TArray<int32> queue;
    int32 seededPlaces = 0;

    // The same distance UBurinWorld::FindSeedTriangleForPlace() searches with, so that at level 1
    // the seed found here is the one FPlace::SeedTriangle already holds.
    const double seedSearchRadiusKm = 50.0;

    for (int32 placeIndex = 0; placeIndex < places.Num(); placeIndex++) {
        // Found in this level's own mesh rather than taken from FPlace::SeedTriangle, which names a
        // level-1 triangle: triangle ids mean nothing across levels, and an island too small for
        // level 1 to carry -- which leaves that place seedless there -- does exist at level 3.
        const int32 seed = FindLandTriangleNear(zoomCategory,
            places[placeIndex].Longitude, -places[placeIndex].Latitude, seedSearchRadiusKm);
        if (seed == INDEX_NONE) {
            continue;
        }
        seededPlaces++;

        queue.Reset();
        queue.Add(seed);
        reachedBy[seed] = placeIndex;
        contendersOf[seed].Add(placeIndex);

        const FVector2D& origin = placePosition[placeIndex];

        for (int32 head = 0; head < queue.Num(); head++) {
            const int32 current = queue[head];

            int32 tileX = 0, tileY = 0, local = INDEX_NONE;
            if (!TileOf(current, tileX, tileY, local)) {
                continue;
            }
            const TArray<FEdgeDataEntry>& tileEdges = EdgeData[zoomCategory][tileX][tileY];
            const FTriangleDataEntry& tri = TriangleData[zoomCategory][tileX][tileY][local];
            const int32 base = current - local;

            auto TryReach = [&](int32 neighbour) {
                if (neighbour < 0 || neighbour >= totalTriangles) {
                    return; // edge of the mesh
                }
                if (reachedBy[neighbour] == placeIndex) {
                    return;
                }
                if (isWaterTerrain(TerrainOf(neighbour))) {
                    return; // a coastline always stops a domain
                }

                FPointDataEntry p1, p2, p3;
                if (!VerticesOf(neighbour, p1, p2, p3)) {
                    return;
                }
                if (distanceToTriangleKm(origin.X, origin.Y, p1, p2, p3) > maxRadiusKm) {
                    return;
                }

                reachedBy[neighbour] = placeIndex;
                contendersOf[neighbour].Add(placeIndex);
                queue.Add(neighbour);
            };

            // Neighbours the mesh names itself. T1/T2 are local to this triangle's tile, so they
            // are only ids at all once the tile's base is added back on.
            const int32 incidentEdges[3] = { tri.E1, tri.E2, tri.E3 };
            for (int32 edgeIndex : incidentEdges) {
                if (!tileEdges.IsValidIndex(edgeIndex)) {
                    continue;
                }
                const FEdgeDataEntry& edge = tileEdges[edgeIndex];
                if (bRiversBlockDomains && edge.RiverData >= 0) {
                    continue;
                }
                const int32 neighbourLocal = (edge.T1 == local) ? edge.T2 : edge.T1;
                if (neighbourLocal >= 0) {
                    TryReach(base + neighbourLocal);
                }
            }

            // Triangles on another layer covering this one's ground, worked out by the generator.
            // Also tile-local, and never across a tile boundary: the generator computes them one
            // tile at a time. A river cannot block one of these -- they are not a crossing at all,
            // but the same ground seen on two layers, and a river runs along a boundary, not
            // through it.
            for (int32 neighbourLocal : tri.CrossLayer) {
                if (neighbourLocal >= 0) {
                    TryReach(base + neighbourLocal);
                }
            }

            // The neighbours the mesh could not name, tile boundaries among them, blocked by a
            // river on the same terms as any edge -- the flag was worked out when the link was
            // made, from whichever side recorded it. Already global.
            for (const FStitchedLink& link : stitched[current]) {
                if (bRiversBlockDomains && link.bRiver) {
                    continue;
                }
                TryReach(link.Triangle);
            }
        }
    }

    const double floodSeconds = FPlatformTime::Seconds() - startSeconds - stitchedSeconds;

    // ---- Phase two: split each triangle between everyone who reached it ----
    int32 claimed = 0;
    int32 clipped = 0;
    int32 contested = 0;
    int32 fragments = 0;

    // Ascending global order, which within a tile is ascending local order, so each tile's
    // Fragments come out sorted by triangle -- which is what AddProvinceTriangles() walks them in.
    for (int32 triangle = 0; triangle < totalTriangles; triangle++) {
        const TArray<int32>& contenders = contendersOf[triangle];
        if (contenders.Num() == 0) {
            continue;
        }

        int32 tileX = 0, tileY = 0, local = INDEX_NONE;
        if (!TileOf(triangle, tileX, tileY, local)) {
            continue;
        }
        FTileDomains& tileDomains = DomainData[zoomCategory][tileX][tileY];
        claimed++;

        FPointDataEntry p1, p2, p3;
        GetTriangleVertices(zoomCategory, tileX, tileY, local, p1, p2, p3);
        const FVector2D a(p1.X, p1.Y), b(p2.X, p2.Y), c(p3.X, p3.Y);

        // Owner is only bookkeeping now -- it names the place holding the largest share, so that
        // FPlace::Triangles still reads as "the ground this place is on". The picture comes from
        // the pieces, and a contested triangle belongs partly to each contender.
        int32 nearest = contenders[0];
        double nearestKm = TNumericLimits<double>::Max();
        for (int32 placeIndex : contenders) {
            const double distanceKm = distanceToTriangleKm(placePosition[placeIndex].X, placePosition[placeIndex].Y, p1, p2, p3);
            if (distanceKm < nearestKm) {
                nearestKm = distanceKm;
                nearest = placeIndex;
            }
        }
        tileDomains.Owner[local] = nearest;
        places[nearest].Triangles.Add(triangle);

        if (contenders.Num() == 1) {
            const FVector2D& origin = placePosition[nearest];
            if (distanceFromKm(a, origin) <= maxRadiusKm
                && distanceFromKm(b, origin) <= maxRadiusKm
                && distanceFromKm(c, origin) <= maxRadiusKm) {
                continue; // one place, wholly inside its radius: drawn as itself
            }
        }
        else {
            contested++;
        }

        tileDomains.bClipped[local] = true;
        const int32 fragmentsBefore = tileDomains.Fragments.Num();
        clipTriangleToDomains(local, a, b, c, contenders, placePosition, maxRadiusKm, tileDomains.Fragments);
        fragments += tileDomains.Fragments.Num() - fragmentsBefore;
        clipped++;
    }

    int32 placesWithDomain = 0;
    for (const FPlace& place : places) {
        if (place.Triangles.Num() > 0) placesWithDomain++;
    }

    int32 crossLayerLinks = 0;
    for (int32 tileX = 0; tileX < w; tileX++) {
        for (int32 tileY = 0; tileY < h; tileY++) {
            for (const FTriangleDataEntry& tri : TriangleData[zoomCategory][tileX][tileY]) {
                crossLayerLinks += tri.CrossLayer.Num();
            }
        }
    }

    const double totalSeconds = FPlatformTime::Seconds() - startSeconds;

    UE_LOG(LogTemp, Log, TEXT("BuildPlaceDomains: level %d (%dx%d tiles, %d triangles), %d of %d places seeded, %d with a domain, %d triangles reached (%d contested, %d clipped, into %d pieces), %d cross-layer links, %d of %d unpaired edges stitched geometrically (%d of them along a river), radius %.0f km, rivers %s -- %.0f ms total (stitch %.0f, flood %.0f, split %.0f)"),
        zoomCategory, w, h, totalTriangles,
        seededPlaces, places.Num(), placesWithDomain, claimed, contested, clipped, fragments, crossLayerLinks, stitchedLinks, unpairedEdges, stitchedRivers, maxRadiusKm,
        bRiversBlockDomains ? TEXT("block") : TEXT("passable"),
        totalSeconds * 1000.0, stitchedSeconds * 1000.0, floodSeconds * 1000.0,
        (totalSeconds - stitchedSeconds - floodSeconds) * 1000.0);
}

void FMapLowZoom::EnsurePlaceDomains(TArray<FPlace>& places, const TArray<FPolity>& polities, int32 zoomCategory, double maxRadiusKm, bool bRiversBlockDomains) {
    if (!HasZoomLevelData(zoomCategory)) {
        return;
    }

    DomainsBuilt.SetNum(FMath::Max(DomainsBuilt.Num(), TriangleData.Num()));
    if (DomainsBuilt[zoomCategory]) {
        return;
    }

    BuildPlaceDomains(places, polities, zoomCategory, maxRadiusKm, bRiversBlockDomains);
    DomainsBuilt[zoomCategory] = true;
}

void FMapLowZoom::InvalidatePlaceDomains() {
    for (bool& built : DomainsBuilt) {
        built = false;
    }
}

void FMapLowZoom::AddProvinceTriangles(TArray<FCanvasUVTri>& result, int32 zoomCategory, int32 tileX, int32 tileY, double minX, double maxX, double minY, double maxY, int32 offsetX, int32 offsetY, double width, double height) {
    if (!HasTileData(zoomCategory, tileX, tileY)) return;
    if (!DomainData.IsValidIndex(zoomCategory)
        || !DomainData[zoomCategory].IsValidIndex(tileX)
        || !DomainData[zoomCategory][tileX].IsValidIndex(tileY)) {
        return; // BuildPlaceDomains() has not run for this level
    }

    const FTileDomains& tile = DomainData[zoomCategory][tileX][tileY];

    auto TryGetPlaceColor = [&tile](int32 placeIndex, FColor& outColor) -> bool {
        if (placeIndex == INDEX_NONE || !tile.PlaceColor.IsValidIndex(placeIndex)) return false;
        if (tile.PlaceColor[placeIndex].A == 0) return false;   // owner never resolved to a polity
        outColor = tile.PlaceColor[placeIndex];
        return true;
    };

    // One pass in ascending triangle order, taking each triangle either whole or as its pieces.
    //
    // The order matters. This mesh overlaps itself here and there -- 0.15% of land points fall
    // inside two triangles -- and its convention is that the later, higher-index triangle wins,
    // which is why the terrain pass draws in this order and GetTriangleIDAtCoordinate searches
    // backwards. Drawing every whole triangle first and every piece afterwards broke that: the
    // pieces of a low-index triangle painted over a high-index one, leaving a speck of the wrong
    // domain's colour inside another domain. Pieces are built in ascending triangle order, so a
    // cursor walked alongside the triangles restores the mesh's own layering.
    int32 fragmentCursor = 0;

    for (int32 triangle = 0; triangle < tile.Owner.Num(); triangle++) {
        if (!tile.bClipped[triangle]) {
            FColor color;
            if (!TryGetPlaceColor(tile.Owner[triangle], color)) {
                continue;
            }

            FPointDataEntry p1, p2, p3;
            GetTriangleVertices(zoomCategory, tileX, tileY, triangle, p1, p2, p3);
            result.Add(convertToDomainTri(color, p1.X, p1.Y, p2.X, p2.Y, p3.X, p3.Y, minX, maxX, minY, maxY, offsetX, offsetY, width, height));
            continue;
        }

        while (fragmentCursor < tile.Fragments.Num() && tile.Fragments[fragmentCursor].Triangle < triangle) {
            fragmentCursor++;
        }
        for (; fragmentCursor < tile.Fragments.Num() && tile.Fragments[fragmentCursor].Triangle == triangle; fragmentCursor++) {
            const FDomainFragment& fragment = tile.Fragments[fragmentCursor];

            FColor color;
            if (!TryGetPlaceColor(fragment.Place, color)) {
                continue;
            }
            result.Add(convertToDomainTri(color, fragment.A.X, fragment.A.Y, fragment.B.X, fragment.B.Y, fragment.C.X, fragment.C.Y, minX, maxX, minY, maxY, offsetX, offsetY, width, height));
        }
    }
}

TArray<FCanvasUVTri> FMapLowZoom::GetProvinceTriangles(int32 mode, int32 zoomCategory, double minFracY, double minFracX, double maxFracY, double maxFracX, int32 offsetX, int32 offsetY, int32 width, int32 height) {
    TArray<FCanvasUVTri> result = {};
    if (!HasZoomLevelData(zoomCategory)) return result;

    if (!ModeShowsDomains(mode)) return result;

    // Deliberately the same arithmetic as GetTriangles(), including the antimeridian shifts: the two
    // are drawn on top of each other, so anything that differs here shows up as domains sliding off
    // their terrain at the edges of the view.
    const int32 xBase = FMath::RoundToInt32(minFracX / (maxFracX - minFracX));
    const int32 yBase = FMath::RoundToInt32(minFracY / (maxFracY - minFracY));
    const int32 tileX = floorMod(xBase, TriangleData[zoomCategory].Num());
    const int32 tileY = floorMod(yBase, TriangleData[zoomCategory][0].Num());

    double minX = 360 * minFracX - 180;
    double maxX = 360 * maxFracX - 180;
    if (xBase < 0) minX += 360;
    if (xBase < 0) maxX += 360;
    if (xBase >= TriangleData[zoomCategory].Num()) minX -= 360;
    if (xBase >= TriangleData[zoomCategory].Num()) maxX -= 360;
    const double minY = 180 * minFracY - 90;
    const double maxY = 180 * maxFracY - 90;

    AddProvinceTriangles(result, zoomCategory, tileX, tileY, minX, maxX, minY, maxY, offsetX, offsetY, width, height);

    // Logged the way GetTriangles() logs its own count, and for the same reason: the two are drawn
    // on top of each other per tile, so a terrain line with no domain line beside it says the
    // caller drew one pass and not the other. That is how the zoomed-in draw paths were found to be
    // missing this call entirely while the whole-world path had it.
    UE_LOG(LogTemp, Log, TEXT("Num domain triangles: %d (level %d, tile %d, %d)"), result.Num(), zoomCategory, tileX, tileY);
    return result;
}

int32 FMapLowZoom::GetPlaceAtCoordinate(int32 zoomCategory, double x, double y) {
    if (!HasZoomLevelData(zoomCategory) || !DomainData.IsValidIndex(zoomCategory)) {
        return INDEX_NONE;
    }

    // The tile comes back from the lookup: the index it returns is local to whichever tile it
    // chose, and at levels 2 and 3 that is one of many.
    int32 tileX = 0, tileY = 0;
    const int32 triangle = GetTriangleIDAtCoordinate(zoomCategory, x, y, tileX, tileY);
    if (triangle < 0
        || !DomainData[zoomCategory].IsValidIndex(tileX)
        || !DomainData[zoomCategory][tileX].IsValidIndex(tileY)) {
        return INDEX_NONE;
    }

    const FTileDomains& tile = DomainData[zoomCategory][tileX][tileY];
    if (!tile.Owner.IsValidIndex(triangle)) {
        return INDEX_NONE; // domains have not been built for this level
    }

    if (!tile.bClipped[triangle]) {
        return tile.Owner[triangle];
    }

    // Fragments are stored in ascending triangle order, so this could binary search; a click is
    // rare enough that the scan is not worth the extra code to get wrong.
    for (const FDomainFragment& fragment : tile.Fragments) {
        if (fragment.Triangle != triangle) {
            continue;
        }
        if (isPointInCoordTriangle(x, y, fragment.A.X, fragment.A.Y, fragment.B.X, fragment.B.Y, fragment.C.X, fragment.C.Y)) {
            return fragment.Place;
        }
    }

    // Inside a clipped triangle but outside every piece: the ground here is past the domain radius,
    // in the sliver the inscribed polygon leaves along the edge of the circle.
    return INDEX_NONE;
}

void FMapLowZoom::GetDomainTerrainAreas(int32 zoomCategory, int32 placeIndex, TMap<int32, double>& outAreasKm2, double& outTotalKm2) {
    outAreasKm2.Reset();
    outTotalKm2 = 0.0;

    if (placeIndex == INDEX_NONE || !HasZoomLevelData(zoomCategory) || !DomainData.IsValidIndex(zoomCategory)) {
        return;
    }

    auto Add = [&outAreasKm2, &outTotalKm2](int32 terrainData, double areaKm2) {
        if (areaKm2 <= 0.0) return;
        outAreasKm2.FindOrAdd(terrainData) += areaKm2;
        outTotalKm2 += areaKm2;
    };

    // Every tile, because a domain crosses tile boundaries: at level 3 a 200 km radius spans two
    // tiles more often than not, and stopping at the tile the place stands in would report a
    // fraction of its ground with no sign that anything was missing.
    const int32 w = TriangleData[zoomCategory].Num();
    const int32 h = TriangleData[zoomCategory][0].Num();

    for (int32 tileX = 0; tileX < w; tileX++) {
        if (!DomainData[zoomCategory].IsValidIndex(tileX)) {
            continue;
        }
        for (int32 tileY = 0; tileY < h; tileY++) {
            if (!DomainData[zoomCategory][tileX].IsValidIndex(tileY)) {
                continue;
            }

            const TArray<FTriangleDataEntry>& triangles = TriangleData[zoomCategory][tileX][tileY];
            const FTileDomains& tile = DomainData[zoomCategory][tileX][tileY];

            // FPlace::Triangles is not used here on purpose: a contested triangle is recorded there
            // only for the nearest contender, so a place holding a share of ground it is not
            // nearest to would be missing from its own total. Sur reads as holding nothing at
            // 1500 BC on that basis, despite 4,606 km2 of domain.
            for (int32 triangle = 0; triangle < tile.Owner.Num(); triangle++) {
                if (tile.bClipped[triangle] || tile.Owner[triangle] != placeIndex) {
                    continue;
                }
                FPointDataEntry p1, p2, p3;
                GetTriangleVertices(zoomCategory, tileX, tileY, triangle, p1, p2, p3);
                const double whole = triangleAreaKm2(p1.X, p1.Y, p2.X, p2.Y, p3.X, p3.Y);
                Add(triangles[triangle].TerrainData, whole - triangles[triangle].CoveredArea);
            }

            for (const FDomainFragment& fragment : tile.Fragments) {
                if (fragment.Place != placeIndex || !triangles.IsValidIndex(fragment.Triangle)) {
                    continue;
                }
                const double piece = triangleAreaKm2(fragment.A.X, fragment.A.Y, fragment.B.X, fragment.B.Y, fragment.C.X, fragment.C.Y);

                FPointDataEntry p1, p2, p3;
                GetTriangleVertices(zoomCategory, tileX, tileY, fragment.Triangle, p1, p2, p3);
                const double whole = triangleAreaKm2(p1.X, p1.Y, p2.X, p2.Y, p3.X, p3.Y);
                const double covered = triangles[fragment.Triangle].CoveredArea;

                const double visibleShare = (whole > UE_DOUBLE_SMALL_NUMBER)
                    ? FMath::Clamp(1.0 - covered / whole, 0.0, 1.0)
                    : 1.0;
                Add(triangles[fragment.Triangle].TerrainData, piece * visibleShare);
            }
        }
    }
}
