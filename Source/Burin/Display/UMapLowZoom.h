// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Burin/UBurinWorld.h>
#include "UMapLowZoom.generated.h"

UCLASS(Blueprintable)
class BURIN_API UMapLowZoom : public UObject
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	UMapLowZoom();
	~UMapLowZoom();

	UPROPERTY()
	UTextureRenderTarget2D* RenderTarget;

	UFUNCTION(BlueprintCallable, Category = "RenderVcMap")
	void RenderVcMap();

	UFUNCTION(BlueprintCallable, Category = "RenderVcMap")
	TArray<FCanvasUVTri> GetTriangles(UBurinWorld* world, int mode);

	//UFUNCTION(BlueprintCallable, Category = "RenderVcMap")
	//void DrawToRenderTarget();
};
