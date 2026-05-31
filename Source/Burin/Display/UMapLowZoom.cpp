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

UPointDataEntry UMapLowZoom::getFirstPoint(bool b, int edge) {
    if (b) { return PointData[EdgeData[edge].p1]; }
    return PointData[EdgeData[edge].p2];
}


FString UMapLowZoom::GetTerrainText(UTerrain* terrain, int v) {
    return terrain->GetTerrainText(v);
}

int UMapLowZoom::GetTerrainDataAtCoordinate(UTerrain* terrain, double x, double y) {
    int idx = GetTriangleIDAtCoordinate(x, y);
    if(idx >= 0) return terrain->GetTerrainFromCache(TriangleData[idx].terrainData);

    return -1;
}

int UMapLowZoom::GetTriangleIDAtCoordinate(double x, double y) {
    for (int i = TriangleData.Num() - 1; i >= 0; i--) {
        UPointDataEntry p1 = getFirstPoint(TriangleData[i].b1, TriangleData[i].e1);
        UPointDataEntry p2 = getFirstPoint(TriangleData[i].b2, TriangleData[i].e2);
        UPointDataEntry p3 = getFirstPoint(TriangleData[i].b3, TriangleData[i].e3);
        if (isPointInCoordTriangle(x, y, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y)) {
            return i;
        }
    }
    return -1;
}

FCanvasUVTri* convertToTri(TArray<uint8_t> rgb, double x1, double y1, double x2, double y2, double x3, double y3) {
    FCanvasUVTri* result = new FCanvasUVTri();

    FVector2D* v0 = new FVector2D();
    
    v0->X = (x1 + 180) / 360 * 16384;
    v0->Y = (y1 + 90) / 180 * 16384;
    result->V0_Pos = *v0;

    FVector2D* v1 = new FVector2D();
    v1->X = (x2 + 180) / 360 * 16384;
    v1->Y = (y2 + 90) / 180 * 16384;
    result->V1_Pos = *v1;

    FVector2D* v2 = new FVector2D();
    v2->X = (x3 + 180) / 360 * 16384;
    v2->Y = (y3 + 90) / 180 * 16384;
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

void UMapLowZoom::Initialize() {
    TriangleData = {};
    EdgeData = {};
    PointData = {};

    FString fPath1 = FPaths::ProjectContentDir() + TEXT("Data/Earth/Polygons/Scale_8/Points.txt");
    TArray<FString> take1;
    FFileHelper::LoadANSITextFileToStrings(*fPath1, NULL, take1);

    for (int i = 0; i < take1.Num(); i++) {
        FString aString = take1[i];
        TArray<FString> stringArray = {};
        aString.ParseIntoArray(stringArray, TEXT(","), false);

        UPointDataEntry point = {};
        point.x = FCString::Atod(*stringArray[0]);
        point.y = FCString::Atod(*stringArray[1]);
        point.neighbors = {};
        for (int j = 2; j < stringArray.Num(); j++) {
            point.neighbors.Add(FCString::Atoi(*stringArray[j]));
        }

        PointData.Add(point);
    }

    UE_LOG(LogTemp, Log, TEXT("Number of points: %d, %d"), PointData.Num(), take1.Num());

    FString fPath2 = FPaths::ProjectContentDir() + TEXT("Data/Earth/Polygons/Scale_8/Edges.txt");
    TArray<FString> take2;
    FFileHelper::LoadANSITextFileToStrings(*fPath2, NULL, take2);

    for (int i = 0; i < take2.Num(); i++) {
        FString aString = take2[i];
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
        } else {
            edge.riverData = -1;
        }

        EdgeData.Add(edge);
    }

    UE_LOG(LogTemp, Log, TEXT("Number of edges: %d, %d"), EdgeData.Num(), take2.Num());

    FString fPath3 = FPaths::ProjectContentDir() + TEXT("Data/Earth/Polygons/Scale_8/Triangles.txt");
    TArray<FString> take3;
    FFileHelper::LoadANSITextFileToStrings(*fPath3, NULL, take3);

    int terrainData = 0;

    for (int i = 0; i < take3.Num(); i++) {
        FString aString = take3[i];
        TArray<FString> stringArray = {};
        aString.ParseIntoArray(stringArray, TEXT(","), false);

        if (stringArray.Num() >  6) {
            terrainData = FCString::Atoi(*stringArray[6]);
        }
        UTriangleDataEntry triangle = {};
        triangle.e1 = FCString::Atoi(*stringArray[0]);
        triangle.e2 = FCString::Atoi(*stringArray[1]);
        triangle.e3 = FCString::Atoi(*stringArray[2]);
        triangle.b1 = FCString::Atoi(*stringArray[3]) == 1;
        triangle.b2 = FCString::Atoi(*stringArray[4]) == 1;
        triangle.b3 = FCString::Atoi(*stringArray[5]) == 1;
        triangle.terrainData = terrainData;

        TriangleData.Add(triangle);
    }

    UE_LOG(LogTemp, Log, TEXT("Number of triangles: %d, %d"), TriangleData.Num(), take3.Num());
}

TArray<FCanvasUVTri> UMapLowZoom::GetTriangles(UTerrain* terrain, int mode) {
    TArray<FCanvasUVTri> result = {};
    if (mode == 0) return result;

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

        for (int i = 0; i < TriangleData.Num(); i++) {
            UPointDataEntry p1 = getFirstPoint(TriangleData[i].b1, TriangleData[i].e1);
            UPointDataEntry p2 = getFirstPoint(TriangleData[i].b2, TriangleData[i].e2);
            UPointDataEntry p3 = getFirstPoint(TriangleData[i].b3, TriangleData[i].e3);
            result.Add(*convertToTri(colors[i % colors.Num()], p1.x, p1.y, p2.x, p2.y, p3.x, p3.y));
        }
        return result;
    }

    for (int i = 0; i < TriangleData.Num(); i++) {
        UPointDataEntry p1 = getFirstPoint(TriangleData[i].b1, TriangleData[i].e1);
        UPointDataEntry p2 = getFirstPoint(TriangleData[i].b2, TriangleData[i].e2);
        UPointDataEntry p3 = getFirstPoint(TriangleData[i].b3, TriangleData[i].e3);
        result.Add(*convertToTri(terrain->GetColor(TriangleData[i].terrainData, mode), p1.x, p1.y, p2.x, p2.y, p3.x, p3.y));
    }
    return result;
}

TArray<FCanvasUVTri> UMapLowZoom::GetMaterialTriangles(UTerrain* terrain, int mode) {
    TArray<FCanvasUVTri> result = {};
    if (mode > 0) return result;

    for (int i = 0; i < TriangleData.Num(); i++) {
        UPointDataEntry p1 = getFirstPoint(TriangleData[i].b1, TriangleData[i].e1);
        UPointDataEntry p2 = getFirstPoint(TriangleData[i].b2, TriangleData[i].e2);
        UPointDataEntry p3 = getFirstPoint(TriangleData[i].b3, TriangleData[i].e3);
        result.Add(*convertToTri(terrain->GetColor(TriangleData[i].terrainData, mode), p1.x, p1.y, p2.x, p2.y, p3.x, p3.y));
    }

    return result;
}

TArray<FLineDisplayData> UMapLowZoom::GetBorders(int mode) {
    if (mode != 6) return {};
    TArray<FLineDisplayData> result = {};
    for (UEdgeDataEntry& edgeData : EdgeData) {
        if (edgeData.t2 >= 0) {
            bool i1 = TriangleData[edgeData.t1].terrainData % 16 == 0;
            bool i2 = TriangleData[edgeData.t2].terrainData % 16 == 0;
            if (i1 != i2) {
                double x1 = (PointData[edgeData.p1].x + 180) / 360 * 16384;
                double y1 = (PointData[edgeData.p1].y + 90) / 180 * 16384;
                double x2 = (PointData[edgeData.p2].x + 180) / 360 * 16384;
                double y2 = (PointData[edgeData.p2].y + 90) / 180 * 16384;
                result.Add(FLineDisplayData{ FVector2D{ x1, y1 }, FVector2D{ x2, y2 } });
            }
        }
        else  if (edgeData.t2 < -1) {
            bool i1 = TriangleData[edgeData.t1].terrainData % 16 == 0;
            bool i2 = (-edgeData.t2-2) % 16 == 0;
            if (i1 != i2) {
                double x1 = (PointData[edgeData.p1].x + 180) / 360 * 16384;
                double y1 = (PointData[edgeData.p1].y + 90) / 180 * 16384;
                double x2 = (PointData[edgeData.p2].x + 180) / 360 * 16384;
                double y2 = (PointData[edgeData.p2].y + 90) / 180 * 16384;
                result.Add(FLineDisplayData{ FVector2D{ x1, y1 }, FVector2D{ x2, y2 } });
            }
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Number of border lines: %d"), result.Num());
    return result;
}

TArray<FLineDisplayData> UMapLowZoom::GetRivers(int mode) {
    if (mode != 6) return {};
    int s = 1;
    TArray<FLineDisplayData> result = {};
    for (UEdgeDataEntry& edgeData : EdgeData) {
        if (edgeData.riverData >= 0) {
            double x1 = (PointData[edgeData.p1].x + 180) / 360 * 16384;
            double y1 = (PointData[edgeData.p1].y + 90) / 180 * 16384;
            double x2 = (PointData[edgeData.p2].x + 180) / 360 * 16384;
            double y2 = (PointData[edgeData.p2].y + 90) / 180 * 16384;
            result.Add(FLineDisplayData{ FVector2D{ x1, y1 }, FVector2D{ x2, y2 } });
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Number of river lines: %d"), result.Num());
    return result;
}

TArray<FCanvasUVTri> UMapLowZoom::GetProvinceTriangles(TArray<UProvince> provinces, int mode) {
    TArray<FCanvasUVTri> result = {};

    for (int i = 0; i < provinces.Num(); i++) {
        for (int j = 0; j < provinces[i].Triangles.Num(); j++) {
            int k = provinces[i].Triangles[j];
            UPointDataEntry p1 = getFirstPoint(TriangleData[k].b1, TriangleData[k].e1);
            UPointDataEntry p2 = getFirstPoint(TriangleData[k].b2, TriangleData[k].e2);
            UPointDataEntry p3 = getFirstPoint(TriangleData[k].b3, TriangleData[k].e3);
            {
                result.Add(*convertToTri({255, 0, 0}, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y));
            }
        }
    }
    
    return result;
}

