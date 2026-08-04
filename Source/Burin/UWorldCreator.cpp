// Fill out your copyright notice in the Description page of Project Settings.


#include "UWorldCreator.h"

FArea UWorldCreator::CreateArea(FString name, double latitude, double longitude) {
	FArea area;
	//province.
	return area;
}

void UWorldCreator::DestroyArea(FString name) {

}

void UWorldCreator::CreateHistoricalPlace(UBurinWorld* world, FPlaceDataEntry entry) {
	CreatePlace(
		world,
		entry.Name,
		entry.Latitude,
		entry.Longitude
	);
}

void UWorldCreator::CreatePlace(UBurinWorld* world, FString name, double latitude, double longitude) {
	FPlace place;

	place.Name = name;
	place.CommonName = name;
	place.Latitude = latitude;
	place.Longitude = longitude;

	place.Triangles = {};
	place.Triangles.Add(world->GetTriangleIDAtCoordinate(0, latitude, longitude));

	world->Places.Add(place);
}

void UWorldCreator::DestroyProvince(FString name) {

}

void UWorldCreator::CreateRandomWorld(UBurinWorld* world) {

}

void UWorldCreator::CreateHistoricalWorld(UBurinWorld* world) {
	world->Places = {};
	world->Polities = {};
	for (int32 i = 0; i < world->HistoricalPlaces->HistoricalPlaceData.Num(); i++) {
		CreateHistoricalPlace(world, world->HistoricalPlaces->HistoricalPlaceData[i]);
		//world->Provinces.Add(province);
	}
}
