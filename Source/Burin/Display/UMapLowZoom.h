// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Burin/UBurinWorld.h>
#include "UTriangleDataEntry.h"

#include "UMapLowZoom.generated.h"

UCLASS(Blueprintable)
class BURIN_API UMapLowZoom : public UObject
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	UMapLowZoom();
	~UMapLowZoom();

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<double> GetColorAtCoordinate(UBurinWorld* world, double x, double y);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	TArray<FCanvasUVTri> GetTriangles(UBurinWorld* world, int mode);

	UFUNCTION(BlueprintCallable, Category = "RenderMap")
	void Initialize(UBurinWorld* world);

private:
	TArray<UTriangleDataEntry> TriangleData;
};
