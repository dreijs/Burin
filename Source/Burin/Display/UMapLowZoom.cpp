// Fill out your copyright notice in the Description page of Project Settings.


#include "UMapLowZoom.h"
#include "FLineDisplayData.h"
#include "Engine.h" // Required for GEngine
#include <vector>
#include "CanvasTypes.h" // Required for FCanvas
#include "Engine/Canvas.h" // Required for UCanvas static functions
#include <fstream> // For file input/output operations
#include <string>  // For string manipulation
#include <sstream> // For parsing strings
#include <iostream>



TArray<UTriangleDataEntry> TriangleData;
TArray<UEdgeDataEntry> EdgeData;

// Sets default values
UMapLowZoom::UMapLowZoom()
{

}
UMapLowZoom::~UMapLowZoom()
{

}

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

TArray<int> UMapLowZoom::GetSubregionIndices(int zoomCategory, double lat, double lon, double latDelta, double lonDelta) {
    int n = TriangleData[zoomCategory].Num();
    int minX = static_cast<int>(std::floor((lon - lonDelta + 180) / 360 * n));
    int maxX = static_cast<int>(std::floor((lon + lonDelta + 180) / 360 * n));
    int m = TriangleData[zoomCategory][0].Num();
    int minY = static_cast<int>(std::floor((90 - (lat + latDelta)) / 180 * m));
    int maxY = static_cast<int>(std::floor((90 - (lat - latDelta)) / 180 * m));
    if (minY < 0) minY = 0;
    if (maxY > m - 1) maxY = m - 1;
    return { minX, minY, maxX, maxY };
}

int UMapLowZoom::GetNumSubregions(int zoomCategory, bool isX) {
    if (isX) return TriangleData[zoomCategory].Num();
    return TriangleData[zoomCategory][0].Num();
}

UPointDataEntry UMapLowZoom::getFirstPoint(bool b, int edge, int zoomCategory, int x, int y) {
    if (b) { return PointData[zoomCategory][x][y][EdgeData[zoomCategory][x][y][edge].p1]; }
    return PointData[zoomCategory][x][y][EdgeData[zoomCategory][x][y][edge].p2];
}

FString UMapLowZoom::GetTerrainText(UTerrain* terrain, int v) {
    return terrain->GetTerrainText(v);
}

int UMapLowZoom::GetTerrainDataAtCoordinate(UTerrain* terrain, int zoomCategory, double lon, double lat) {
    int x = static_cast<int>(std::floor((lon +180) / 360 * TriangleData[zoomCategory].Num()));
    int y = static_cast<int>(std::floor((lat +90) / 180 * TriangleData[zoomCategory][x].Num()));
    int idx = GetTriangleIDAtCoordinate(zoomCategory, lon, lat);
    if(idx >= 0) return terrain->GetTerrainFromCache(TriangleData[zoomCategory][x][y][idx].terrainData);

    return -1;
}

int UMapLowZoom::GetTriangleIDAtCoordinate(int zoomCategory, double lon, double lat) {
    int x = static_cast<int>(std::floor((lon + 180) / 360 * TriangleData[zoomCategory].Num()));
    int y = static_cast<int>(std::floor((lat + 90) / 180 * TriangleData[zoomCategory][x].Num()));
    UE_LOG(LogTemp, Log, TEXT("Triangle region: %d, %d"), x, y);
    for (int i = TriangleData[zoomCategory][x][y].Num() - 1; i >= 0; i--) {
        UPointDataEntry p1 = getFirstPoint(TriangleData[zoomCategory][x][y][i].b1, TriangleData[zoomCategory][x][y][i].e1, zoomCategory, x, y);
        UPointDataEntry p2 = getFirstPoint(TriangleData[zoomCategory][x][y][i].b2, TriangleData[zoomCategory][x][y][i].e2, zoomCategory, x, y);
        UPointDataEntry p3 = getFirstPoint(TriangleData[zoomCategory][x][y][i].b3, TriangleData[zoomCategory][x][y][i].e3, zoomCategory, x, y);
        if (isPointInCoordTriangle(lon, lat, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y)) {
            UE_LOG(LogTemp, Log, TEXT("Triangle region: %d"), i);
            return i;
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Triangle region: -1"));
    return -1;
}

FCanvasUVTri* convertToTri(TArray<uint8_t> rgb, double x1, double y1, double x2, double y2, double x3, double y3, double minX, double maxX, double minY, double maxY, double width, double height) {
    FCanvasUVTri* result = new FCanvasUVTri();

    FVector2D* v0 = new FVector2D();
    
    v0->X = (x1 - minX) / (maxX - minX) * width;
    v0->Y = (y1 - minY) / (maxY - minY) * height;
    result->V0_Pos = *v0;

    FVector2D* v1 = new FVector2D();
    v1->X = (x2 - minX) / (maxX - minX) * width;
    v1->Y = (y2 - minY) / (maxY - minY) * height;
    result->V1_Pos = *v1;

    FVector2D* v2 = new FVector2D();
    v2->X = (x3 - minX) / (maxX - minX) * width;
    v2->Y = (y3 - minY) / (maxY - minY) * height;
    result->V2_Pos = *v2;

    FLinearColor* color = new FLinearColor(1.f * rgb[0]/255, 1.f * rgb[1] / 255, 1.f * rgb[2] / 255, 1.f);

    result->V0_Color = *color;
    result->V1_Color = *color;
    result->V2_Color = *color;

    double ss = 0.0075;

    FVector2D* uv0 = new FVector2D();
    uv0->X = ss * 20;
    uv0->Y = ss * 20;
    result->V0_UV = *uv0;

    FVector2D* uv1 = new FVector2D();
    uv1->X = ss * 22.5;
    uv1->Y = ss * 20;
    result->V1_UV = *uv1;

    FVector2D* uv2 = new FVector2D();
    uv2->X = ss * 25;
    uv2->Y = ss * 25;
    result->V2_UV = *uv2;

    return result;
}

static UPointDataEntry getPointData(FString aString) {
    TArray<FString> stringArray = {};
    aString.ParseIntoArray(stringArray, TEXT(","), false);

    UPointDataEntry point = {};
    point.x = FCString::Atod(*stringArray[0]);
    point.y = FCString::Atod(*stringArray[1]);
    point.neighbors = {};
    for (int j = 2; j < stringArray.Num(); j++) {
        point.neighbors.Add(FCString::Atoi(*stringArray[j]));
    }

    return point;
}

static UEdgeDataEntry getEdgeData(FString aString) {
    TArray<FString> stringArray = {};
    aString.ParseIntoArray(stringArray, TEXT(","), false);

    UEdgeDataEntry edge = {};
    edge.p1 = FCString::Atoi(*stringArray[0]);
    edge.p2 = FCString::Atoi(*stringArray[1]);
    edge.t1 = FCString::Atoi(*stringArray[2]);
    if (stringArray.Num() > 3) {
        edge.t2 = FCString::Atoi(*stringArray[3]);
    }
    if (stringArray.Num() > 4) {
        edge.riverData = FCString::Atoi(*stringArray[4]);
    }
    else {
        edge.riverData = -1;
    }

    return edge;
}

static UTriangleDataEntry getTriangleData(FString aString, int terrainData) {
    TArray<FString> stringArray = {};
    aString.ParseIntoArray(stringArray, TEXT(","), false);

    UTriangleDataEntry triangle = {};
    triangle.e1 = FCString::Atoi(*stringArray[0]);
    triangle.e2 = FCString::Atoi(*stringArray[1]);
    triangle.e3 = FCString::Atoi(*stringArray[2]);
    triangle.b1 = FCString::Atoi(*stringArray[3]) == 1;
    triangle.b2 = FCString::Atoi(*stringArray[4]) == 1;
    triangle.b3 = FCString::Atoi(*stringArray[5]) == 1;
    triangle.terrainData = terrainData;

    return triangle;
}

static int getTerrain(FString aString, int defaultValue) {
    TArray<FString> stringArray = {};
    aString.ParseIntoArray(stringArray, TEXT(","), false);
    if (stringArray.Num() > 6) {
        return FCString::Atoi(*stringArray[6]);
    }
    return defaultValue;
}

int getMaxZoomLevel(FString ZoomLevelsPath) {
    TArray<FString> ZoomLevelNames = {};
    IFileManager::Get().FindFiles(ZoomLevelNames, *ZoomLevelsPath, false, true);
    int maxLevel = 0;
    for (FString s : ZoomLevelNames) {
        TArray<FString> stringArray = {};
        s.ParseIntoArray(stringArray, TEXT("_"), false);
        if (stringArray.Num() > 1) {
            int level = FCString::Atoi(*stringArray[1]);
            if (level > maxLevel) maxLevel = level;
        }
    }
    return maxLevel;
}

TArray<int> getWidthAndHeight(FString ZoomLevelsPath) {
    TArray<FString> ZoomLevelNames = {};
    IFileManager::Get().FindFiles(ZoomLevelNames, *ZoomLevelsPath, false, true);
    int width = 0;
    int height = 0;
    for (FString s : ZoomLevelNames) {
        TArray<FString> stringArray = {};
        s.ParseIntoArray(stringArray, TEXT("_"), false);
        if (stringArray.Num() > 1) {
            int x = FCString::Atoi(*stringArray[0]);
            int y = FCString::Atoi(*stringArray[1]);
            if (x > width) width = x;
            if (y > height) height = y;
        }
    }
    return { width , height };
}

void UMapLowZoom::Initialize() {
    TriangleData = {};
    EdgeData = {};
    PointData = {};

    // zoom levels
    int maxLevel = getMaxZoomLevel(FPaths::ProjectContentDir() + TEXT("Data/Earth/Polygons/*"));
    UE_LOG(LogTemp, Log, TEXT("Number of zoom levels: %d"), maxLevel);

    for (int level = 0; level < maxLevel; level++) {
        TArray < TArray < TArray <UTriangleDataEntry> > > TriangleDataForZoomLevel = {};
        TArray < TArray < TArray <UEdgeDataEntry> > > EdgeDataForZoomLevel = {};
        TArray < TArray < TArray <UPointDataEntry> > > PointDataForZoomLevel = {};

        TArray<int> wh = getWidthAndHeight(FPaths::ProjectContentDir() + TEXT("Data/Earth/Polygons/Level_") + FString::FromInt((level + 1)) + "/*");
        int w = wh[0] + 1;
        int h = wh[1] + 1;

        UE_LOG(LogTemp, Log, TEXT("Zoom level %d: width = %d, height = %d"), maxLevel, w, h);

        for (int x = 0; x < w; x++) {
            TArray < TArray <UTriangleDataEntry> > TriangleDataForZoomLevelX = {};
            TArray < TArray <UEdgeDataEntry> > EdgeDataForZoomLevelX = {};
            TArray < TArray <UPointDataEntry> > PointDataForZoomLevelX = {};
            for (int y = 0; y < h; y++) {
                TArray <UTriangleDataEntry> TriangleDataForZoomLevelXY = {};
                TArray <UEdgeDataEntry> EdgeDataForZoomLevelXY = {};
                TArray <UPointDataEntry> PointDataForZoomLevelXY = {};

                FString basePath = FPaths::ProjectContentDir() + TEXT("Data/Earth/Polygons/Level_") + FString::FromInt((level + 1)) + TEXT("/") + FString::FromInt(x) + TEXT("_") + FString::FromInt(y) + TEXT("/");

                FString fPath1 = basePath + TEXT("Triangles.txt");
                TArray<FString> take1;
                FFileHelper::LoadANSITextFileToStrings(*fPath1, NULL, take1);

                int terrainData = 0;
                for (int i = 0; i < take1.Num(); i++) {
                    terrainData = getTerrain(take1[i], terrainData);
                    TriangleDataForZoomLevelXY.Add(getTriangleData(take1[i], terrainData));
                }

                FString fPath2 = basePath + TEXT("Edges.txt");
                TArray<FString> take2;
                FFileHelper::LoadANSITextFileToStrings(*fPath2, NULL, take2);

                for (int i = 0; i < take2.Num(); i++) {
                    EdgeDataForZoomLevelXY.Add(getEdgeData(take2[i]));
                }

                FString fPath3 = basePath + TEXT("Points.txt");
                TArray<FString> take3;
                FFileHelper::LoadANSITextFileToStrings(*fPath3, NULL, take3);

                for (int i = 0; i < take3.Num(); i++) {
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

    /*FString SearchPath = FPaths::Combine("Data/Earth/Polygons", TEXT("*"));
    TArray<FString> OutFolderNames = {};
    IFileManager::Get().FindFiles(OutFolderNames, *SearchPath, false, true);

    for (const FString& zoomCategoryPath : OutFolderNames) {

        TArray <UTriangleDataEntry> LevelTriangleData = {};
        TArray <UEdgeDataEntry> LevelEdgeData = {};
        TArray <UPointDataEntry> LevelPointData = {};

        FString fPath3 = FPaths::ProjectContentDir() + TEXT("Data/Earth/Polygons/Level_1/0_0/Triangles.txt");
        TArray<FString> take3;
        FFileHelper::LoadANSITextFileToStrings(*fPath3, NULL, take3);

        int terrainData = 0;
        for (int i = 0; i < take3.Num(); i++) {
            terrainData = getTerrain(take3[i], terrainData);
            LevelTriangleData.Add(getTriangleData(take3[i], terrainData));
        }

        UE_LOG(LogTemp, Log, TEXT("Number of triangles: %d, %d"), TriangleData.Num(), take3.Num());
        TriangleData.Add(LevelTriangleData);

        FString fPath2 = FPaths::ProjectContentDir() + TEXT("Data/Earth/Polygons/Level_1/0_0/Edges.txt");
        TArray<FString> take2;
        FFileHelper::LoadANSITextFileToStrings(*fPath2, NULL, take2);

        for (int i = 0; i < take2.Num(); i++) {
            LevelEdgeData.Add(getEdgeData(take2[i]));
        }

        UE_LOG(LogTemp, Log, TEXT("Number of edges: %d, %d"), EdgeData.Num(), take2.Num());
        EdgeData.Add(LevelEdgeData);

        FString fPath1 = FPaths::ProjectContentDir() + TEXT("Data/Earth/Polygons/Level_1/0_0/Points.txt");
        TArray<FString> take1;
        FFileHelper::LoadANSITextFileToStrings(*fPath1, NULL, take1);

        for (int i = 0; i < take1.Num(); i++) {
            LevelPointData.Add(getPointData(take1[i]));
        }

        UE_LOG(LogTemp, Log, TEXT("Number of points: %d, %d"), PointData.Num(), take1.Num());
        PointData.Add(LevelPointData);
    }*/
}


int floorMod(int A, int B) {
    return ((A % B) + B) % B;
}

TArray<FCanvasUVTri> UMapLowZoom::GetTriangles(UTerrain* terrain, int mode, int zoomCategory, double minLat, double minLon, double maxLat, double maxLon, int width, int height) {
    TArray<FCanvasUVTri> result = {};
    if (mode == 0) return result;

    int xBase = (int)FMath::RoundToInt32(minLon / (maxLon - minLon));
    int yBase = (int)FMath::RoundToInt32(minLat / (maxLat - minLat));
    int x = floorMod(xBase, TriangleData[zoomCategory].Num());
    int y = floorMod(yBase, TriangleData[zoomCategory][0].Num());

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
        TArray<TArray<uint8_t>> colors;
        uint8_t stepsize = 64;
        for (uint8_t k = 0; k <= 256 / stepsize; k++) {
            for (uint8_t l = 0; l <= 256 / stepsize; l++) {
                for (uint8_t m = 0; m <= 256 / stepsize; m++) {
                    uint8_t r = std::min(stepsize * k, 255);
                    uint8_t g = std::min(stepsize * l, 255);
                    uint8_t b = std::min(stepsize * m, 255);
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

        for (int i = 0; i < TriangleData[zoomCategory][x][y].Num(); i++) {
            UPointDataEntry p1 = getFirstPoint(TriangleData[zoomCategory][x][y][i].b1, TriangleData[zoomCategory][x][y][i].e1, zoomCategory, x, y);
            UPointDataEntry p2 = getFirstPoint(TriangleData[zoomCategory][x][y][i].b2, TriangleData[zoomCategory][x][y][i].e2, zoomCategory, x, y);
            UPointDataEntry p3 = getFirstPoint(TriangleData[zoomCategory][x][y][i].b3, TriangleData[zoomCategory][x][y][i].e3, zoomCategory, x, y);
            result.Add(*convertToTri(colors[i % colors.Num()], p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, minX, maxX, minY, maxY, width, height));
        }
        return result;
    }

    for (int i = 0; i < TriangleData[zoomCategory][x][y].Num(); i++) {
        UPointDataEntry p1 = getFirstPoint(TriangleData[zoomCategory][x][y][i].b1, TriangleData[zoomCategory][x][y][i].e1, zoomCategory, x, y);
        UPointDataEntry p2 = getFirstPoint(TriangleData[zoomCategory][x][y][i].b2, TriangleData[zoomCategory][x][y][i].e2, zoomCategory, x, y);
        UPointDataEntry p3 = getFirstPoint(TriangleData[zoomCategory][x][y][i].b3, TriangleData[zoomCategory][x][y][i].e3, zoomCategory, x, y);
        result.Add(*convertToTri(terrain->GetColor(TriangleData[zoomCategory][x][y][i].terrainData, mode), p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, minX, maxX, minY, maxY, width, height));
        //if(TriangleData[zoomCategory][x][y].Num() < 12000) UE_LOG(LogTemp, Log, TEXT("Triangle: %f, %f  %f, %f  %f, %f ; %f, %f"), p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, (maxLon - minLon) * width, (maxLat - minLat) * height);
    }
    UE_LOG(LogTemp, Log, TEXT("Num render triangles: %d"), result.Num());
    return result;
}

TArray<FCanvasUVTri> UMapLowZoom::GetMaterialTriangles(UTerrain* terrain, int mode, int zoomCategory, int x, int y) {
    TArray<FCanvasUVTri> result = {};
    if (mode > 0) return result;

    for (int i = 0; i < TriangleData[zoomCategory][x][y].Num(); i++) {
        UPointDataEntry p1 = getFirstPoint(TriangleData[zoomCategory][x][y][i].b1, TriangleData[zoomCategory][x][y][i].e1, zoomCategory, x, y);
        UPointDataEntry p2 = getFirstPoint(TriangleData[zoomCategory][x][y][i].b2, TriangleData[zoomCategory][x][y][i].e2, zoomCategory, x, y);
        UPointDataEntry p3 = getFirstPoint(TriangleData[zoomCategory][x][y][i].b3, TriangleData[zoomCategory][x][y][i].e3, zoomCategory, x, y);
        result.Add(*convertToTri(terrain->GetColor(TriangleData[zoomCategory][x][y][i].terrainData, mode), p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, -180, 180, -90, 90, 16384, 16384));
    }

    return result;
}

TArray<FLineDisplayData> UMapLowZoom::GetBorders(int mode, int zoomCategory, int x, int y) {
    if (mode != 6) return {};
    TArray<FLineDisplayData> result = {};
    for (UEdgeDataEntry& edgeData : EdgeData[zoomCategory][x][y]) {
        if (edgeData.t2 >= 0) {
            bool i1 = TriangleData[zoomCategory][x][y][edgeData.t1].terrainData % 16 == 0;
            bool i2 = TriangleData[zoomCategory][x][y][edgeData.t2].terrainData % 16 == 0;
            if (i1 != i2) {
                double x1 = (PointData[zoomCategory][x][y][edgeData.p1].x + 180) / 360 * 16384;
                double y1 = (PointData[zoomCategory][x][y][edgeData.p1].y + 90) / 180 * 16384;
                double x2 = (PointData[zoomCategory][x][y][edgeData.p2].x + 180) / 360 * 16384;
                double y2 = (PointData[zoomCategory][x][y][edgeData.p2].y + 90) / 180 * 16384;
                result.Add(FLineDisplayData{ FVector2D{ x1, y1 }, FVector2D{ x2, y2 } });
            }
        }
        else  if (edgeData.t2 < -1) {
            bool i1 = TriangleData[zoomCategory][x][y][edgeData.t1].terrainData % 16 == 0;
            bool i2 = (-edgeData.t2-2) % 16 == 0;
            if (i1 != i2) {
                double x1 = (PointData[zoomCategory][x][y][edgeData.p1].x + 180) / 360 * 16384;
                double y1 = (PointData[zoomCategory][x][y][edgeData.p1].y + 90) / 180 * 16384;
                double x2 = (PointData[zoomCategory][x][y][edgeData.p2].x + 180) / 360 * 16384;
                double y2 = (PointData[zoomCategory][x][y][edgeData.p2].y + 90) / 180 * 16384;
                result.Add(FLineDisplayData{ FVector2D{ x1, y1 }, FVector2D{ x2, y2 } });
            }
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Number of border lines: %d"), result.Num());
    return result;
}

TArray<FLineDisplayData> UMapLowZoom::GetRivers(int mode, int zoomCategory, int x, int y) {
    if (mode != 6) return {};
    int s = 1;
    TArray<FLineDisplayData> result = {};
    for (UEdgeDataEntry& edgeData : EdgeData[zoomCategory][x][y]) {
        if (edgeData.riverData >= 0) {
            double x1 = (PointData[zoomCategory][x][y][edgeData.p1].x + 180) / 360 * 16384;
            double y1 = (PointData[zoomCategory][x][y][edgeData.p1].y + 90) / 180 * 16384;
            double x2 = (PointData[zoomCategory][x][y][edgeData.p2].x + 180) / 360 * 16384;
            double y2 = (PointData[zoomCategory][x][y][edgeData.p2].y + 90) / 180 * 16384;
            result.Add(FLineDisplayData{ FVector2D{ x1, y1 }, FVector2D{ x2, y2 } });
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Number of river lines: %d"), result.Num());
    return result;
}

TArray<FCanvasUVTri> UMapLowZoom::GetProvinceTriangles(TArray<UProvince> provinces, int mode, int zoomCategory, int x, int y) {
    TArray<FCanvasUVTri> result = {};

    //for (int i = 0; i < provinces.Num(); i++) {
    //    for (int j = 0; j < provinces[i].Triangles.Num(); j++) {
    //        int k = provinces[i].Triangles[j];
    //        UPointDataEntry p1 = getFirstPoint(TriangleData[zoomCategory][x][y][k].b1, TriangleData[zoomCategory][x][y][k].e1);
    //        UPointDataEntry p2 = getFirstPoint(TriangleData[zoomCategory][x][y][k].b2, TriangleData[zoomCategory][x][y][k].e2);
    //        UPointDataEntry p3 = getFirstPoint(TriangleData[zoomCategory][x][y][k].b3, TriangleData[zoomCategory][x][y][k].e3);
    //        {
    //            result.Add(*convertToTri({255, 0, 0}, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y));
    //        }
    //    }
    //}
    
    return result;
}

