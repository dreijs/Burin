// Fill out your copyright notice in the Description page of Project Settings.


#include "UWorldCreator.h"

UWorldCreator::UWorldCreator()
{
}

UWorldCreator::~UWorldCreator()
{
}

UArea UWorldCreator::CreateArea(FString name, double latitude, double longitude) {
	UProvince province;
	//province.
	return {};
}

void UWorldCreator::DestroyArea(FString name) {

}

UProvince UWorldCreator::CreateProvince(FString name, double latitude, double longitude) {
	UProvince province;
	//province.
	return {};
}

void UWorldCreator::DestroyProvince(FString name) {
	
}

void UWorldCreator::CreateRandomWorld(UBurinWorld* world) {

}

void UWorldCreator::CreateHistoricalWorld(UBurinWorld* world) {
	world->Areas = {};
	for (int i = 0; i < world->Places->PlaceData.Num(); i++) {
		UArea area = CreateArea(world->Places->PlaceData[i].name, world->Places->PlaceData[i].latitude, world->Places->PlaceData[i].longitude);
		world->Areas.Add(area);
	}
}