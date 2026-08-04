// Fill out your copyright notice in the Description page of Project Settings.


#include "UGeoCoordinateLibrary.h"

FVector UGeoCoordinateLibrary::LatLonToUnitVector(double latitudeDeg, double longitudeDeg, double longitudeOffsetDeg, bool flipLongitude, bool flipLatitude)
{
	const double lonDeg = (flipLongitude ? -longitudeDeg : longitudeDeg) + longitudeOffsetDeg;
	const double latDeg = flipLatitude ? -latitudeDeg : latitudeDeg;

	const double latRad = FMath::DegreesToRadians(latDeg);
	const double lonRad = FMath::DegreesToRadians(lonDeg);

	const double cosLat = FMath::Cos(latRad);

	return FVector(cosLat * FMath::Cos(lonRad),		// X
		cosLat * FMath::Sin(lonRad),				// Y
		FMath::Sin(latRad));						// Z (north pole)
}
