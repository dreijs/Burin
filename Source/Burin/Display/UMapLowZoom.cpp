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

// Sets default values
UMapLowZoom::UMapLowZoom()
{

}
UMapLowZoom::~UMapLowZoom()
{

}

// from Google search AI:

struct Point {
    double x, y;
};

struct Triangle {
    Point p1, p2, p3;
    int r, g, b;
};

TArray<Triangle> getWorldPoints(UBurinWorld* world, int mode) {
    TArray<Triangle> triangles;

    FString fPath = FPaths::ProjectContentDir() + TEXT("Data/Polygons/Scale_8/Polygons_0_0.txt");

    TArray<FString> take;
    FFileHelper::LoadANSITextFileToStrings(*fPath, NULL, take);

    int r = 0, g = 0, b = 0;

    for (int i = 0; i < take.Num(); i++) {
        FString aString = take[i];
        TArray<FString> stringArray = {};
        aString.ParseIntoArray(stringArray, TEXT(","), false);

        if (stringArray.Num() == 1) {
            TArray<int> rgb = UTerrain::GetColor(world, FCString::Atoi(*stringArray[0]), mode);

            r = rgb[0];
            g = rgb[1];
            b = rgb[2];
        }
        else if (stringArray.Num() == 6) {
            Point p1 = { FCString::Atoi(*stringArray[0]), FCString::Atoi(*stringArray[1]) };
            Point p2 = { FCString::Atoi(*stringArray[2]), FCString::Atoi(*stringArray[3]) };
            Point p3 = { FCString::Atoi(*stringArray[4]), FCString::Atoi(*stringArray[5]) };
            Triangle ptTriangle = { p1, p2, p3, r, g, b};
            triangles.Add(ptTriangle);
        }
    }

    return triangles;
}

static double crossProduct(Point p1, Point p2, Point p3) {
    return (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);
}

double signedArea(Point p1, Point p2, Point p3) {
    return 0.5 * ((p2.x - p1.x) * (p3.y - p1.y) - (p3.x - p1.x) * (p2.y - p1.y));
}

bool isInsideTriangle(Point p, Point a, Point b, Point c) {
    // Calculate the signed areas of the main triangle and sub-triangles
    double totalArea = signedArea(a, b, c);
    double areaPBC = signedArea(p, b, c);
    double areaPCA = signedArea(p, c, a);
    double areaPAB = signedArea(p, a, b);

    // If the totalArea is zero, the triangle is degenerate (a line or point)
    // Handle this case as needed, e.g., return false or check if P is on the line segment.
    if (totalArea == 0) {
        // Example: if the triangle is degenerate, consider the point outside.
        return false;
    }

    // Calculate barycentric coordinates
    double alpha = areaPBC / totalArea;
    double beta = areaPCA / totalArea;
    double gamma = areaPAB / totalArea;

    // Check conditions for point being inside the triangle (with a small epsilon for floating-point comparisons)
    const double EPSILON = 1e-9;
    return (alpha >= -EPSILON && alpha <= 1.0 + EPSILON) &&
        (beta >= -EPSILON && beta <= 1.0 + EPSILON) &&
        (gamma >= -EPSILON && gamma <= 1.0 + EPSILON);
}

TArray<TArray<Point>> triangulatePolygon(TArray < Point>& polygon) {
    TArray<TArray<Point>> triangles;

    // Ensure polygon has at least 3 vertices
    if (polygon.Num() < 3) {
        return triangles;
    }

    // Loop until only 3 vertices remain (forming the last triangle)
    while (polygon.Num() > 3) {
        bool earFound = false;
        for (int32 i = 0; i < polygon.Num(); ++i) {
            Point pPrev = polygon[(i == 0) ? polygon.Num() - 1 : i - 1];
            Point pCurr = polygon[i];
            Point pNext = polygon[(i == polygon.Num() - 1) ? 0 : i + 1];
                bool isEar = true;

                if (isEar) {
                    triangles.Add({ pPrev, pCurr, pNext });
                    // Remove pCurr from the polygon
                    polygon.RemoveAt(i);
                    earFound = true;
                    break; // Restart search for ears
                }
            //}
        }
        if (!earFound) {
            // Handle cases where no ear is found (e.g., self-intersecting polygon)
            break;
        }
    }

    // Add the final remaining triangle
    if (polygon.Num() == 3) {
        triangles.Add(polygon);
    }

    return triangles;
}

FCanvasUVTri* convertToTri(Triangle triangle) {
    int s = 4;
    FCanvasUVTri* result = new FCanvasUVTri();
    FVector2D* v0 = new FVector2D();
    v0->X = triangle.p1.x * s;
    v0->Y = triangle.p1.y * s;
    result->V0_Pos = *v0;
    FVector2D* v1 = new FVector2D();
    v1->X = triangle.p2.x * s;
    v1->Y = triangle.p2.y * s;
    result->V1_Pos = *v1;
    FVector2D* v2 = new FVector2D();
    v2->X = triangle.p3.x * s;
    v2->Y = triangle.p3.y * s;
    result->V2_Pos = *v2;

    FLinearColor* color = new FLinearColor(1.f*triangle.r/255, 1.f * triangle.g / 255, 1.f * triangle.b / 255, 1.f);

    result->V0_Color = *color;
    result->V1_Color = *color;
    result->V2_Color = *color;

    return result;
}

TArray<FCanvasUVTri> convertArrayToTri(TArray<Triangle> triangles) {
    TArray<FCanvasUVTri> result;
    for (Triangle& t : triangles) {
        result.Add(*convertToTri(t));
    }
    return result;
}

TArray<FCanvasUVTri> UMapLowZoom::GetTriangles(UBurinWorld* world, int mode) {
    TArray<Triangle> triangles = getWorldPoints(world, mode);
    
    TArray<FCanvasUVTri> result = convertArrayToTri(triangles);

    return result;
}

void UMapLowZoom::RenderVcMap()
{

}