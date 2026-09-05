// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FDomainInfo.generated.h"

/**
 * One terrain type's share of a domain.
 *
 * TerrainType is the canonical index GetTerrainDataAtCoordinate() returns, not the raw packed value
 * off the triangle -- so a caller can compare it against that function's result directly, and two
 * triangles the project considers the same terrain are counted together even if their packed values
 * differ in a field nobody displays.
 */
USTRUCT(BlueprintType)
struct FDomainTerrainShare
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Domain")
	int32 TerrainType = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Domain")
	FString TerrainName;

	UPROPERTY(BlueprintReadOnly, Category = "Domain")
	double AreaKm2 = 0.0;

	/** Of the domain's total area, 0..1. Saves every caller dividing, and keeps them agreeing. */
	UPROPERTY(BlueprintReadOnly, Category = "Domain")
	double Fraction = 0.0;
};

/**
 * Everything about the domain under a map coordinate, answered in one lookup.
 *
 * The place, its polity and its terrain breakdown are returned together rather than by separate
 * calls, because they are one question about one state of the world. Places and Polities are rebuilt
 * by SetCurrentYear(), so two calls could straddle a year change and describe two different worlds --
 * and they would repeat the same triangle lookup and the same walk over the domain to do it.
 */
USTRUCT(BlueprintType)
struct FDomainInfo
{
	GENERATED_BODY()

	/** False when the coordinate is on water, outside the mesh, or on land no place has claimed. */
	UPROPERTY(BlueprintReadOnly, Category = "Domain")
	bool bValid = false;

	UPROPERTY(BlueprintReadOnly, Category = "Domain")
	FString PlaceName;

	/** Empty when the place's owner never resolved to a known polity -- check bHasPolity, not this. */
	UPROPERTY(BlueprintReadOnly, Category = "Domain")
	FString PolityName;

	UPROPERTY(BlueprintReadOnly, Category = "Domain")
	bool bHasPolity = false;

	UPROPERTY(BlueprintReadOnly, Category = "Domain")
	FColor PolityColor = FColor::Black;

	/** Indices into UBurinWorld's Places and Polities, for callers that want to go further. */
	UPROPERTY(BlueprintReadOnly, Category = "Domain")
	int32 PlaceIndex = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Domain")
	int32 PolityIndex = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Domain")
	double TotalAreaKm2 = 0.0;

	/** Largest contributor first. */
	UPROPERTY(BlueprintReadOnly, Category = "Domain")
	TArray<FDomainTerrainShare> Terrain;
};
