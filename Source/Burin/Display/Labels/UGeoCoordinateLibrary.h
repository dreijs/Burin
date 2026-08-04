// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "UGeoCoordinateLibrary.generated.h"

/**
 * Conversions between geographic coordinates and the globe's local space.
 */
UCLASS()
class BURIN_API UGeoCoordinateLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Converts latitude/longitude in degrees to a unit direction in the globe actor's LOCAL space.
	 * Assumes the globe's local +Z points at the north pole and longitude 0 lies along local +X.
	 * Use longitudeOffsetDeg / flipLongitude / flipLatitude to line the result up with the sphere's texture.
	 */
	UFUNCTION(BlueprintPure, Category = "GlobeLabels")
	static FVector LatLonToUnitVector(double latitudeDeg, double longitudeDeg, double longitudeOffsetDeg = 0.0, bool flipLongitude = false, bool flipLatitude = false);
};
