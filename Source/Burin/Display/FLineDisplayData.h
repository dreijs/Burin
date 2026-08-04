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
	FVector2D V1 = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RenderMap")
	FVector2D V2 = FVector2D::ZeroVector;

	FLineDisplayData() = default;

	FLineDisplayData(const FVector2D& InV1, const FVector2D& InV2)
		: V1(InV1)
		, V2(InV2)
	{
	}
};
