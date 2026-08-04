// Fill out your copyright notice in the Description page of Project Settings.


#include "FMapLowZoom.h"
#include "FLineDisplayData.h"
#include "CanvasTypes.h" // Required for FCanvas
#include "Engine/Canvas.h" // Required for UCanvas static functions
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

TArray<int32> FMapLowZoom::GetSubregionIndices(int32 zoomCategory, double lat, double lon, double latDelta, double lonDelta) {
    int32 n = TriangleData[zoomCategory].Num();
    int32 minX = FMath::FloorToInt32((lon - lonDelta + 180) / 360 * n);
    int32 maxX = FMath::FloorToInt32((lon + lonDelta + 180) / 360 * n);
    int32 m = TriangleData[zoomCategory][0].Num();
    int32 minY = FMath::FloorToInt32((90 - (lat + latDelta)) / 180 * m);
    int32 maxY = FMath::FloorToInt32((90 - (lat - latDelta)) / 180 * m);
    if (minY < 0) minY = 0;
    if (maxY > m - 1) maxY = m - 1;
    return { minX, minY, maxX, maxY };
}

int32 FMapLowZoom::GetNumSubregions(int32 zoomCategory, bool isX) {
    if (isX) return TriangleData[zoomCategory].Num();
    return TriangleData[zoomCategory][0].Num();
}

FPointDataEntry FMapLowZoom::GetFirstPoint(bool b, int32 edge, int32 zoomCategory, int32 x, int32 y) {
    if (b) { return PointData[zoomCategory][x][y][EdgeData[zoomCategory][x][y][edge].P1]; }
    return PointData[zoomCategory][x][y][EdgeData[zoomCategory][x][y][edge].P2];
}

FString FMapLowZoom::GetTerrainText(FTerrain* terrain, int32 v) {
    return terrain->GetTerrainText(v);
}

int32 FMapLowZoom::GetTerrainDataAtCoordinate(FTerrain* terrain, int32 zoomCategory, double lon, double lat) {
    int32 x = FMath::FloorToInt32((lon + 180) / 360 * TriangleData[zoomCategory].Num());
    int32 y = FMath::FloorToInt32((lat + 90) / 180 * TriangleData[zoomCategory][x].Num());
    int32 idx = GetTriangleIDAtCoordinate(zoomCategory, lon, lat);
    if(idx >= 0) return terrain->GetTerrainFromCache(TriangleData[zoomCategory][x][y][idx].TerrainData);

    return -1;
}

int32 FMapLowZoom::GetTriangleIDAtCoordinate(int32 zoomCategory, double lon, double lat) {
    int32 x = FMath::FloorToInt32((lon + 180) / 360 * TriangleData[zoomCategory].Num());
    int32 y = FMath::FloorToInt32((lat + 90) / 180 * TriangleData[zoomCategory][x].Num());
    UE_LOG(LogTemp, Log, TEXT("Triangle region: %d, %d"), x, y);
    for (int32 i = TriangleData[zoomCategory][x][y].Num() - 1; i >= 0; i--) {
        FPointDataEntry p1 = GetFirstPoint(TriangleData[zoomCategory][x][y][i].bB1, TriangleData[zoomCategory][x][y][i].E1, zoomCategory, x, y);
        FPointDataEntry p2 = GetFirstPoint(TriangleData[zoomCategory][x][y][i].bB2, TriangleData[zoomCategory][x][y][i].E2, zoomCategory, x, y);
        FPointDataEntry p3 = GetFirstPoint(TriangleData[zoomCategory][x][y][i].bB3, TriangleData[zoomCategory][x][y][i].E3, zoomCategory, x, y);
        if (isPointInCoordTriangle(lon, lat, p1.X, p1.Y, p2.X, p2.Y, p3.X, p3.Y)) {
            UE_LOG(LogTemp, Log, TEXT("Triangle region: %d"), i);
            return i;
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Triangle region: -1"));
    return -1;
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

static FCanvasUVTri convertToTri(const TArray<uint8>& rgb,
    double x1, double y1, double x2, double y2, double x3, double y3, double minX, double maxX, double minY, double maxY, int32 offsetX, int32 offsetY, double width, double height) {
    const FVector2D P0 = toScreen(x1, y1, minX, maxX, minY, maxY, offsetX, offsetY, width, height);
    const FVector2D P1 = toScreen(x2, y2, minX, maxX, minY, maxY, offsetX, offsetY, width, height);
    const FVector2D P2 = toScreen(x3, y3, minX, maxX, minY, maxY, offsetX, offsetY, width, height);

    const FLinearColor color(rgb[0] / 255.f, rgb[1] / 255.f, rgb[2] / 255.f, 1.f);

    const double ss = 0.0075;
    const FVector2D UV0(ss * 20, ss * 20);
    const FVector2D UV1(ss * 22.5, ss * 20);
    const FVector2D UV2(ss * 25, ss * 25);

    return makeTri(color, P0, UV0, P1, UV1, P2, UV2);
}

static FPointDataEntry getPointData(FString aString) {
    TArray<FString> stringArray = {};
    aString.ParseIntoArray(stringArray, TEXT(","), false);

    FPointDataEntry point = {};
    point.X = FCString::Atod(*stringArray[0]);
    point.Y = FCString::Atod(*stringArray[1]);
    point.Neighbors = {};
    for (int32 j = 2; j < stringArray.Num(); j++) {
        point.Neighbors.Add(FCString::Atoi(*stringArray[j]));
    }

    return point;
}

static FEdgeDataEntry getEdgeData(FString aString) {
    TArray<FString> stringArray = {};
    aString.ParseIntoArray(stringArray, TEXT(","), false);

    FEdgeDataEntry edge = {};
    edge.P1 = FCString::Atoi(*stringArray[0]);
    edge.P2 = FCString::Atoi(*stringArray[1]);
    edge.T1 = FCString::Atoi(*stringArray[2]);
    if (stringArray.Num() > 3) {
        edge.T2 = FCString::Atoi(*stringArray[3]);
    }
    if (stringArray.Num() > 4) {
        edge.RiverData = FCString::Atoi(*stringArray[4]);
    }
    else {
        edge.RiverData = -1;
    }

    return edge;
}

static FTriangleDataEntry getTriangleData(FString aString, int32 terrainData) {
    TArray<FString> stringArray = {};
    aString.ParseIntoArray(stringArray, TEXT(","), false);

    FTriangleDataEntry triangle = {};
    triangle.E1 = FCString::Atoi(*stringArray[0]);
    triangle.E2 = FCString::Atoi(*stringArray[1]);
    triangle.E3 = FCString::Atoi(*stringArray[2]);
    triangle.bB1 = FCString::Atoi(*stringArray[3]) == 1;
    triangle.bB2 = FCString::Atoi(*stringArray[4]) == 1;
    triangle.bB3 = FCString::Atoi(*stringArray[5]) == 1;
    triangle.TerrainData = terrainData;

    return triangle;
}

static int32 getTerrain(FString aString, int32 defaultValue) {
    TArray<FString> stringArray = {};
    aString.ParseIntoArray(stringArray, TEXT(","), false);
    if (stringArray.Num() > 6) {
        return FCString::Atoi(*stringArray[6]);
    }
    return defaultValue;
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

static TArray<int32> getWidthAndHeight(FString ZoomLevelsPath) {
    TArray<FString> ZoomLevelNames = {};
    IFileManager::Get().FindFiles(ZoomLevelNames, *ZoomLevelsPath, false, true);
    int32 width = 0;
    int32 height = 0;
    for (FString s : ZoomLevelNames) {
        TArray<FString> stringArray = {};
        s.ParseIntoArray(stringArray, TEXT("_"), false);
        if (stringArray.Num() > 1) {
            int32 x = FCString::Atoi(*stringArray[0]);
            int32 y = FCString::Atoi(*stringArray[1]);
            if (x > width) width = x;
            if (y > height) height = y;
        }
    }
    return { width , height };
}

void FMapLowZoom::Initialize() {
    TriangleData = {};
    EdgeData = {};
    PointData = {};

    // zoom levels
    int32 maxLevel = getMaxZoomLevel(FPaths::ProjectContentDir() + TEXT("Data/Earth/Polygons/*"));
    UE_LOG(LogTemp, Log, TEXT("Number of zoom levels: %d"), maxLevel);

    for (int32 level = 0; level < maxLevel; level++) {
        TArray < TArray < TArray <FTriangleDataEntry> > > TriangleDataForZoomLevel = {};
        TArray < TArray < TArray <FEdgeDataEntry> > > EdgeDataForZoomLevel = {};
        TArray < TArray < TArray <FPointDataEntry> > > PointDataForZoomLevel = {};

        TArray<int32> wh = getWidthAndHeight(FPaths::ProjectContentDir() + TEXT("Data/Earth/Polygons/Level_") + FString::FromInt((level + 1)) + "/*");
        int32 w = wh[0] + 1;
        int32 h = wh[1] + 1;

        UE_LOG(LogTemp, Log, TEXT("Zoom level %d: width = %d, height = %d"), maxLevel, w, h);

        for (int32 x = 0; x < w; x++) {
            TArray < TArray <FTriangleDataEntry> > TriangleDataForZoomLevelX = {};
            TArray < TArray <FEdgeDataEntry> > EdgeDataForZoomLevelX = {};
            TArray < TArray <FPointDataEntry> > PointDataForZoomLevelX = {};
            for (int32 y = 0; y < h; y++) {
                TArray <FTriangleDataEntry> TriangleDataForZoomLevelXY = {};
                TArray <FEdgeDataEntry> EdgeDataForZoomLevelXY = {};
                TArray <FPointDataEntry> PointDataForZoomLevelXY = {};

                FString basePath = FPaths::ProjectContentDir() + TEXT("Data/Earth/Polygons/Level_") + FString::FromInt((level + 1)) + TEXT("/") + FString::FromInt(x) + TEXT("_") + FString::FromInt(y) + TEXT("/");

                FString fPath1 = basePath + TEXT("Triangles.txt");
                TArray<FString> take1;
                FFileHelper::LoadANSITextFileToStrings(*fPath1, NULL, take1);

                int32 terrainData = 0;
                for (int32 i = 0; i < take1.Num(); i++) {
                    terrainData = getTerrain(take1[i], terrainData);
                    TriangleDataForZoomLevelXY.Add(getTriangleData(take1[i], terrainData));
                }

                FString fPath2 = basePath + TEXT("Edges.txt");
                TArray<FString> take2;
                FFileHelper::LoadANSITextFileToStrings(*fPath2, NULL, take2);

                for (int32 i = 0; i < take2.Num(); i++) {
                    EdgeDataForZoomLevelXY.Add(getEdgeData(take2[i]));
                }

                FString fPath3 = basePath + TEXT("Points.txt");
                TArray<FString> take3;
                FFileHelper::LoadANSITextFileToStrings(*fPath3, NULL, take3);

                for (int32 i = 0; i < take3.Num(); i++) {
                    PointDataForZoomLevelXY.Add(getPointData(take3[i]));
                }

                UE_LOG(LogTemp, Log, TEXT("Number of triangles, edges, points (zoom level: %d, x: %d, y: %d): %d, %d, %d"), level, x, y, TriangleDataForZoomLevelXY.Num(), EdgeDataForZoomLevelXY.Num(), PointDataForZoomLevelXY.Num());

                TriangleDataForZoomLevelX.Add(TriangleDataForZoomLevelXY);
                EdgeDataForZoomLevelX.Add(EdgeDataForZoomLevelXY);
                PointDataForZoomLevelX.Add(PointDataForZoomLevelXY);
            }
            TriangleDataForZoomLevel.Add(TriangleDataForZoomLevelX);
            EdgeDataForZoomLevel.Add(EdgeDataForZoomLevelX);
            PointDataForZoomLevel.Add(PointDataForZoomLevelX);
        }

        TriangleData.Add(TriangleDataForZoomLevel);
        EdgeData.Add(EdgeDataForZoomLevel);
        PointData.Add(PointDataForZoomLevel);
    }
}

static int32 floorMod(int32 A, int32 B) {
    return ((A % B) + B) % B;
}



TArray<FCanvasUVTri> FMapLowZoom::GetTriangles(FTerrain* terrain, int32 mode, int32 zoomCategory, double minLat, double minLon, double maxLat, double maxLon, int32 offsetX, int32 offsetY, int32 width, int32 height) {
    TArray<FCanvasUVTri> result = {};
    if (mode == 0) return result;

    int32 xBase = FMath::RoundToInt32(minLon / (maxLon - minLon));
    int32 yBase = FMath::RoundToInt32(minLat / (maxLat - minLat));
    int32 x = floorMod(xBase, TriangleData[zoomCategory].Num());
    int32 y = floorMod(yBase, TriangleData[zoomCategory][0].Num());

    double minX = 360 * minLon - 180;
    double maxX = 360 * maxLon - 180;
    if (xBase < 0) minX += 360;
    if (xBase < 0) maxX += 360;
    if (xBase >= TriangleData[zoomCategory].Num()) minX -= 360;
    if (xBase >= TriangleData[zoomCategory].Num()) maxX -= 360;
    double minY = 180 * minLat - 90;
    double maxY = 180 * maxLat - 90;

    UE_LOG(LogTemp, Log, TEXT("Min/max X and Y : %f, %f ; %f, %f, x and y: %i, %i"), minX, maxX, minY, maxY, x, y);

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

        for (int32 i = 0; i < TriangleData[zoomCategory][x][y].Num(); i++) {
            FPointDataEntry p1 = GetFirstPoint(TriangleData[zoomCategory][x][y][i].bB1, TriangleData[zoomCategory][x][y][i].E1, zoomCategory, x, y);
            FPointDataEntry p2 = GetFirstPoint(TriangleData[zoomCategory][x][y][i].bB2, TriangleData[zoomCategory][x][y][i].E2, zoomCategory, x, y);
            FPointDataEntry p3 = GetFirstPoint(TriangleData[zoomCategory][x][y][i].bB3, TriangleData[zoomCategory][x][y][i].E3, zoomCategory, x, y);
            result.Add(convertToTri(colors[i % colors.Num()], p1.X, p1.Y, p2.X, p2.Y, p3.X, p3.Y, minX, maxX, minY, maxY, offsetX, offsetY, width, height));
        }

        return result;
    }

    for (int32 i = 0; i < TriangleData[zoomCategory][x][y].Num(); i++) {
        FPointDataEntry p1 = GetFirstPoint(TriangleData[zoomCategory][x][y][i].bB1, TriangleData[zoomCategory][x][y][i].E1, zoomCategory, x, y);
        FPointDataEntry p2 = GetFirstPoint(TriangleData[zoomCategory][x][y][i].bB2, TriangleData[zoomCategory][x][y][i].E2, zoomCategory, x, y);
        FPointDataEntry p3 = GetFirstPoint(TriangleData[zoomCategory][x][y][i].bB3, TriangleData[zoomCategory][x][y][i].E3, zoomCategory, x, y);
        result.Add(convertToTri(terrain->GetColor(TriangleData[zoomCategory][x][y][i].TerrainData, mode), p1.X, p1.Y, p2.X, p2.Y, p3.X, p3.Y, minX, maxX, minY, maxY, offsetX, offsetY, width, height));
    }

    if (mode == 7) AddBordersAsTriangles(result, zoomCategory, x, y, minX, maxX, minY, maxY, offsetX, offsetY, width, height);

    UE_LOG(LogTemp, Log, TEXT("Num render triangles: %d"), result.Num());
    return result;
}

TArray<FCanvasUVTri>& FMapLowZoom::AddBordersAsTriangles(TArray<FCanvasUVTri>& result, int32 zoomCategory, int32 x, int32 y, double minX, double maxX, double minY, double maxY, int32 offsetX, int32 offsetY, int32 width, int32 height) {
    int32 c = 0;
    for (FEdgeDataEntry& edgeData : EdgeData[zoomCategory][x][y]) {
        const double x1 = PointData[zoomCategory][x][y][edgeData.P1].X;
        const double y1 = PointData[zoomCategory][x][y][edgeData.P1].Y;
        const double x2 = PointData[zoomCategory][x][y][edgeData.P2].X;
        const double y2 = PointData[zoomCategory][x][y][edgeData.P2].Y;

        if (edgeData.T2 >= 0) {
            bool i1 = TriangleData[zoomCategory][x][y][edgeData.T1].TerrainData % 16 == 0;
            bool i2 = TriangleData[zoomCategory][x][y][edgeData.T2].TerrainData % 16 == 0;
            if (i1 != i2) {
                convertEdgeToTriAndAdd(result, x1, y1, x2, y2, 5, minX, maxX, minY, maxY, offsetX, offsetY, width, height);
                c += 2;
            }
        }
        else  if (edgeData.T2 < -1) {
            bool i1 = TriangleData[zoomCategory][x][y][edgeData.T1].TerrainData % 16 == 0;
            bool i2 = (-edgeData.T2 - 2) % 16 == 0;
            if (i1 != i2) {
                convertEdgeToTriAndAdd(result, x1, y1, x2, y2, 5, minX, maxX, minY, maxY, offsetX, offsetY, width, height);
                c += 2;
            }
        }
        if (edgeData.RiverData >= 0) {
            bool i1 = true;
            bool i2 = true;
            if (edgeData.T1 >= 0) i1 = TriangleData[zoomCategory][x][y][edgeData.T1].TerrainData % 16 != 0;
            if (edgeData.T2 >= 0) i2 = TriangleData[zoomCategory][x][y][edgeData.T2].TerrainData % 16 != 0;
            if (i1 && i2) {
                convertEdgeToTriAndAdd(result, x1, y1, x2, y2, 5, minX, maxX, minY, maxY, offsetX, offsetY, width, height);
                c += 2;
            }
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Number of border lines: %d"), c);
    return result;
}


TArray<FCanvasUVTri> FMapLowZoom::GetMaterialTriangles(FTerrain* terrain, int32 mode, int32 zoomCategory, int32 x, int32 y) {
    TArray<FCanvasUVTri> result = {};
    if (mode > 0) return result;

    for (int32 i = 0; i < TriangleData[zoomCategory][x][y].Num(); i++) {
        FPointDataEntry p1 = GetFirstPoint(TriangleData[zoomCategory][x][y][i].bB1, TriangleData[zoomCategory][x][y][i].E1, zoomCategory, x, y);
        FPointDataEntry p2 = GetFirstPoint(TriangleData[zoomCategory][x][y][i].bB2, TriangleData[zoomCategory][x][y][i].E2, zoomCategory, x, y);
        FPointDataEntry p3 = GetFirstPoint(TriangleData[zoomCategory][x][y][i].bB3, TriangleData[zoomCategory][x][y][i].E3, zoomCategory, x, y);
        result.Add(convertToTri(terrain->GetColor(TriangleData[zoomCategory][x][y][i].TerrainData, mode), p1.X, p1.Y, p2.X, p2.Y, p3.X, p3.Y, -180, 180, -90, 90, 0, 0, 16384, 16384));
    }

    return result;
}

TArray<FLineDisplayData> FMapLowZoom::GetBorders(int32 mode, int32 zoomCategory, int32 x, int32 y) {
    if (mode != 6) return {};
    TArray<FLineDisplayData> result = {};
    for (FEdgeDataEntry& edgeData : EdgeData[zoomCategory][x][y]) {
        if (edgeData.T2 >= 0) {
            bool i1 = TriangleData[zoomCategory][x][y][edgeData.T1].TerrainData % 16 == 0;
            bool i2 = TriangleData[zoomCategory][x][y][edgeData.T2].TerrainData % 16 == 0;
            if (i1 != i2) {
                double x1 = (PointData[zoomCategory][x][y][edgeData.P1].X + 180) / 360 * 16384;
                double y1 = (PointData[zoomCategory][x][y][edgeData.P1].Y + 90) / 180 * 16384;
                double x2 = (PointData[zoomCategory][x][y][edgeData.P2].X + 180) / 360 * 16384;
                double y2 = (PointData[zoomCategory][x][y][edgeData.P2].Y + 90) / 180 * 16384;
                result.Add(FLineDisplayData{ FVector2D{ x1, y1 }, FVector2D{ x2, y2 } });
            }
        }
        else  if (edgeData.T2 < -1) {
            bool i1 = TriangleData[zoomCategory][x][y][edgeData.T1].TerrainData % 16 == 0;
            bool i2 = (-edgeData.T2-2) % 16 == 0;
            if (i1 != i2) {
                double x1 = (PointData[zoomCategory][x][y][edgeData.P1].X + 180) / 360 * 16384;
                double y1 = (PointData[zoomCategory][x][y][edgeData.P1].Y + 90) / 180 * 16384;
                double x2 = (PointData[zoomCategory][x][y][edgeData.P2].X + 180) / 360 * 16384;
                double y2 = (PointData[zoomCategory][x][y][edgeData.P2].Y + 90) / 180 * 16384;
                result.Add(FLineDisplayData{ FVector2D{ x1, y1 }, FVector2D{ x2, y2 } });
            }
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Number of border lines: %d"), result.Num());
    return result;
}

TArray<FLineDisplayData> FMapLowZoom::GetRivers(int32 mode, int32 zoomCategory, int32 x, int32 y) {
    if (mode != 6) return {};
    TArray<FLineDisplayData> result = {};
    for (FEdgeDataEntry& edgeData : EdgeData[zoomCategory][x][y]) {
        if (edgeData.RiverData >= 0) {
            double x1 = (PointData[zoomCategory][x][y][edgeData.P1].X + 180) / 360 * 16384;
            double y1 = (PointData[zoomCategory][x][y][edgeData.P1].Y + 90) / 180 * 16384;
            double x2 = (PointData[zoomCategory][x][y][edgeData.P2].X + 180) / 360 * 16384;
            double y2 = (PointData[zoomCategory][x][y][edgeData.P2].Y + 90) / 180 * 16384;
            result.Add(FLineDisplayData{ FVector2D{ x1, y1 }, FVector2D{ x2, y2 } });
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Number of river lines: %d"), result.Num());
    return result;
}



TArray<FCanvasUVTri> FMapLowZoom::GetProvinceTriangles(const TArray<FPlace>& places, int32 mode, int32 zoomCategory, int32 x, int32 y) {
    TArray<FCanvasUVTri> result = {};

    //for (int32 i = 0; i < provinces.Num(); i++) {
    //    for (int32 j = 0; j < provinces[i].Triangles.Num(); j++) {
    //        int32 k = provinces[i].Triangles[j];
    //        FPointDataEntry p1 = GetFirstPoint(TriangleData[zoomCategory][x][y][k].bB1, TriangleData[zoomCategory][x][y][k].E1, zoomCategory, x, y);
    //        FPointDataEntry p2 = GetFirstPoint(TriangleData[zoomCategory][x][y][k].bB2, TriangleData[zoomCategory][x][y][k].E2, zoomCategory, x, y);
    //        FPointDataEntry p3 = GetFirstPoint(TriangleData[zoomCategory][x][y][k].bB3, TriangleData[zoomCategory][x][y][k].E3, zoomCategory, x, y);
    //        {
    //            result.Add(convertToTri({255, 0, 0}, p1.X, p1.Y, p2.X, p2.Y, p3.X, p3.Y, -180, 180, -90, 90, 0, 0, 16384, 16384));
    //        }
    //    }
    //}

    return result;
}
