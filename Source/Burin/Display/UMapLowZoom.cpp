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
#include "../Data/Earth/UTerrain.h"
#include <Burin/UBurinWorld.h>

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

static bool isPointInCoordTriangle(double xp, double yp, int x1, int y1, int x2, int y2, int x3, int y3) {
    return isPointInTriangle(
        (xp + 180) / 360 * 4096,
        (yp + 90) / 180 * 4096,
        1. * x1,
        1. * y1,
        1. * x2,
        1. * y2,
        1. * x3,
        1. * y3
    );
}

FString UMapLowZoom::GetTerrainText(UBurinWorld* world, int v) {
    return world->Terrain->GetTerrainText(v);
}

int UMapLowZoom::GetTerrainDataAtCoordinate(UBurinWorld* world, double x, double y) {
    for (UTriangleDataEntry& triangleData : TriangleData) {
        int x1, y1, x2, y2, x3, y3;
        if (triangleData.b1) { x1 = EdgeData[triangleData.e1].x1; y1 = EdgeData[triangleData.e1].y1; }
        else { x1 = EdgeData[triangleData.e1].x2; y1 = EdgeData[triangleData.e1].y2; }
        if (triangleData.b2) { x2 = EdgeData[triangleData.e2].x1; y2 = EdgeData[triangleData.e2].y1; }
        else { x2 = EdgeData[triangleData.e2].x2; y2 = EdgeData[triangleData.e2].y2; }
        if (triangleData.b3) { x3 = EdgeData[triangleData.e3].x1; y3 = EdgeData[triangleData.e3].y1; }
        else { x3 = EdgeData[triangleData.e3].x2; y3 = EdgeData[triangleData.e3].y2; }
        if (isPointInCoordTriangle(x, y, x1, y1, x2, y2, x3, y3)) {
            return world->Terrain->GetTerrainFromCache(triangleData.terrainData);
        }
    }

    return -1;
}

FCanvasUVTri* convertToTri(UBurinWorld* world, int mode, int x1, int y1, int x2, int y2, int x3, int y3, int terrainData) {
    int s = 4;
    FCanvasUVTri* result = new FCanvasUVTri();
    FVector2D* v0 = new FVector2D();

    v0->X = s * x1;
    v0->Y = s * y1;
    result->V0_Pos = *v0;
    FVector2D* v1 = new FVector2D();
    v1->X = s * x2;
    v1->Y = s * y2;
    result->V1_Pos = *v1;
    FVector2D* v2 = new FVector2D();
    v2->X = s * x3;
    v2->Y = s * y3;
    result->V2_Pos = *v2;

    TArray<int> rgb = world->Terrain->GetColor(terrainData, mode);

    FLinearColor* color = new FLinearColor(1.f * rgb[0]/255, 1.f * rgb[1] / 255, 1.f * rgb[2] / 255, 1.f);

    result->V0_Color = *color;
    result->V1_Color = *color;
    result->V2_Color = *color;

    return result;
}

void UMapLowZoom::Initialize(UBurinWorld* world) {
    TriangleData = {};
    EdgeData = {};

    FString fPath1 = FPaths::ProjectContentDir() + TEXT("Data/Earth/Polygons/Scale_8/Edges_0_0.txt");
    TArray<FString> take1;
    FFileHelper::LoadANSITextFileToStrings(*fPath1, NULL, take1);

    for (int i = 0; i < take1.Num(); i++) {
        FString aString = take1[i];
        TArray<FString> stringArray = {};
        aString.ParseIntoArray(stringArray, TEXT(","), false);

        UEdgeDataEntry edge = {};
        edge.x1 = FCString::Atoi(*stringArray[0]);
        edge.y1 = FCString::Atoi(*stringArray[1]);
        edge.x2 = FCString::Atoi(*stringArray[2]);
        edge.y2 = FCString::Atoi(*stringArray[3]);
        edge.t1 = FCString::Atoi(*stringArray[4]);
        edge.t2 = FCString::Atoi(*stringArray[5]);
        if (stringArray.Num() > 6) {
            edge.riverData = FCString::Atoi(*stringArray[4]);
        }

        EdgeData.Add(edge);
    }

    UE_LOG(LogTemp, Log, TEXT("Number of edges: %d"), EdgeData.Num());

    FString fPath2 = FPaths::ProjectContentDir() + TEXT("Data/Earth/Polygons/Scale_8/Triangles_0_0.txt");
    TArray<FString> take2;
    FFileHelper::LoadANSITextFileToStrings(*fPath2, NULL, take2);

    int terrainData = 0;

    for (int i = 0; i < take2.Num(); i++) {
        FString aString = take2[i];
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

    UE_LOG(LogTemp, Log, TEXT("Number of triangles: %d"), TriangleData.Num());
}

TArray<FCanvasUVTri> UMapLowZoom::GetTriangles(UBurinWorld* world, int mode) {
    TArray<FCanvasUVTri> result = {};

    for (UTriangleDataEntry& triangleData : TriangleData) {
        int x1, y1, x2, y2, x3, y3;
        if (triangleData.b1) { x1 = EdgeData[triangleData.e1].x1; y1 = EdgeData[triangleData.e1].y1; }
        else { x1 = EdgeData[triangleData.e1].x2; y1 = EdgeData[triangleData.e1].y2; }
        if (triangleData.b2) { x2 = EdgeData[triangleData.e2].x1; y2 = EdgeData[triangleData.e2].y1; }
        else { x2 = EdgeData[triangleData.e2].x2; y2 = EdgeData[triangleData.e2].y2; }
        if (triangleData.b3) { x3 = EdgeData[triangleData.e3].x1; y3 = EdgeData[triangleData.e3].y1; }
        else { x3 = EdgeData[triangleData.e3].x2; y3 = EdgeData[triangleData.e3].y2; }

        result.Add(*convertToTri(world, mode, x1, y1, x2, y2, x3, y3, triangleData.terrainData));
    }
    return result;
}

TArray<FLineDisplayData> UMapLowZoom::GetBorders(UBurinWorld* world, int mode) {
    if (mode < 5) return {};
    int s = 4;
    TArray<FLineDisplayData> result = {};
    for (UEdgeDataEntry& edgeData : EdgeData) {
        if (edgeData.t2 >= 0) {
            int i1 = TriangleData[edgeData.t1].terrainData % 16 == 0;
            int i2 = TriangleData[edgeData.t2].terrainData % 16 == 0;
            if (i1 != i2) result.Add(FLineDisplayData{ FVector2D{ 1. * edgeData.x1 * s, 1. * edgeData.y1 * s }, FVector2D{ 1. * edgeData.x2 * s, 1. * edgeData.y2 * s } });
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Number of lines: %d"), result.Num());
    return result;
}