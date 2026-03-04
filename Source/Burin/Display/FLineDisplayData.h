// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "FLineDisplayData.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct BURIN_API FLineDisplayData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RenderMap")
	FVector2D v1; // Initialize with a default value

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RenderMap")
	FVector2D v2;

	FLineDisplayData(FVector2D vv1, FVector2D vv2);
	FLineDisplayData();
	~FLineDisplayData();
};
