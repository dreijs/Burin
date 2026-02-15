// Fill out your copyright notice in the Description page of Project Settings.


#include "UMapLowZoom.h"
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
    for (UTriangleDataEntry& t : TriangleData) {
        if (isPointInCoordTriangle(x, y, t.x1, t.y1, t.x2, t.y2, t.x3, t.y3)) {
            return world->Terrain->GetTerrain(t.biomeData);
        }
    }

    return -1;
}

FCanvasUVTri* convertToTri(UBurinWorld* world, int mode, UTriangleDataEntry triangleData) {
    int s = 4;
    FCanvasUVTri* result = new FCanvasUVTri();
    FVector2D* v0 = new FVector2D();
    v0->X = triangleData.x1 * s;
    v0->Y = triangleData.y1 * s;
    result->V0_Pos = *v0;
    FVector2D* v1 = new FVector2D();
    v1->X = triangleData.x2 * s;
    v1->Y = triangleData.y2 * s;
    result->V1_Pos = *v1;
    FVector2D* v2 = new FVector2D();
    v2->X = triangleData.x3 * s;
    v2->Y = triangleData.y3 * s;
    result->V2_Pos = *v2;

    TArray<int> rgb = world->Terrain->GetColor(triangleData.biomeData, mode);

    FLinearColor* color = new FLinearColor(1.f * rgb[0]/255, 1.f * rgb[1] / 255, 1.f * rgb[2] / 255, 1.f);

    result->V0_Color = *color;
    result->V1_Color = *color;
    result->V2_Color = *color;

    return result;
}

void UMapLowZoom::Initialize(UBurinWorld* world) {
    TriangleData = {};

    FString fPath = FPaths::ProjectContentDir() + TEXT("Data/Polygons/Scale_8/Polygons_0_0.txt");

    TArray<FString> take;
    FFileHelper::LoadANSITextFileToStrings(*fPath, NULL, take);

    int biomeData = 0;

    for (int i = 0; i < take.Num(); i++) {
        FString aString = take[i];
        TArray<FString> stringArray = {};
        aString.ParseIntoArray(stringArray, TEXT(","), false);

        if (stringArray.Num() == 1) {
            biomeData = FCString::Atoi(*stringArray[0]);
        }
        else if (stringArray.Num() == 6) {
            UTriangleDataEntry triangle = {};
            triangle.x1 = FCString::Atoi(*stringArray[0]);
            triangle.y1 = FCString::Atoi(*stringArray[1]);
            triangle.x2 = FCString::Atoi(*stringArray[2]);
            triangle.y2 = FCString::Atoi(*stringArray[3]);
            triangle.x3 = FCString::Atoi(*stringArray[4]);
            triangle.y3 = FCString::Atoi(*stringArray[5]);
            triangle.biomeData = biomeData;

            TriangleData.Add(triangle);
        }
    }
}
TArray<FCanvasUVTri> UMapLowZoom::GetTriangles(UBurinWorld* world, int mode) {
    TArray<FCanvasUVTri> result = {};

    for (UTriangleDataEntry& t : TriangleData) {
        result.Add(*convertToTri(world, mode, t));
    }
    return result;
}