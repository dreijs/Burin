// Fill out your copyright notice in the Description page of Project Settings.


#include "UWorldCreator.h"

UWorldCreator::UWorldCreator()
{
}

UWorldCreator::~UWorldCreator()
{
}

UArea UWorldCreator::CreateArea(FString name, double latitude, double longitude) {
	UArea area;
	//province.
	return area;
}

void UWorldCreator::DestroyArea(FString name) {

}

UProvince UWorldCreator::CreateProvince(UBurinWorld* world, FString name, double latitude, double longitude) {
	UProvince province;

	province.Name = name;
	province.CommonName = name;
	province.Latitude = latitude;
	province.Longitude = longitude;

	province.Triangles = {};
	province.Triangles.Add(world->GetTriangleIDAtCoordinate(0, latitude, longitude));
	
	return province;
}

void UWorldCreator::DestroyProvince(FString name) {
	
}

void UWorldCreator::CreateRandomWorld(UBurinWorld* world) {

}

void UWorldCreator::CreateHistoricalWorld(UBurinWorld* world) {
	world->Provinces = {};
	world->Polities = {};
	for (int i = 0; i < world->HistoricalPlaces->PlaceData.Num(); i++) {
		//UProvince province = CreateProvince(world, world->HistoricalPlaces->PlaceData[i].name, world->HistoricalPlaces->PlaceData[i].latitude, world->HistoricalPlaces->PlaceData[i].longitude);
		//world->Provinces.Add(province);
	}
}