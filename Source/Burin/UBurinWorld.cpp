// Fill out your copyright notice in the Description page of Project Settings.


#include "UBurinWorld.h"
#include "Data/Earth/UTerrainDataEntry.h"
#include "Data/Earth/UTerrain.h"
#include "UWorldCreator.h"

UBurinWorld::UBurinWorld()
{
}

UBurinWorld::~UBurinWorld()
{
}

void UBurinWorld::initializeTerrain() {
	Terrain = new UTerrain();

	Terrain->InitializeElevation();
	Terrain->InitializeVegetation();
	Terrain->InitializeSoil();
	Terrain->InitializeFeatures();
	Terrain->InitializeTerrain();

	Terrain->InitializeTerrainMapping();
}

void UBurinWorld::initializeHistory() {
	HistoricalPolities = new UPolities();
	HistoricalPlaces = new UPlaces();

	HistoricalPolities->InitializePolities();
	HistoricalPlaces->InitializePlaces();
}

void UBurinWorld::initializeMap() {
	MapLowZoom = new UMapLowZoom();

	MapLowZoom->Initialize();
}

void UBurinWorld::SetWorldCreatorSettings() {

}

void UBurinWorld::InitializeProvinces()
{
	UWorldCreator::CreateHistoricalWorld(this);
}

void UBurinWorld::Initialize()
{
	initializeTerrain();
	initializeHistory();
	initializeMap();
}

int UBurinWorld::GetTerrainDataAtCoordinate(double x, double y) {
	return MapLowZoom->GetTerrainDataAtCoordinate(Terrain, x, y);
}

int UBurinWorld::GetTriangleIDAtCoordinate(double x, double y) {
	return MapLowZoom->GetTriangleIDAtCoordinate(x, y);
}

FString UBurinWorld::GetTerrainText(int v) {
	return MapLowZoom->GetTerrainText(Terrain, v);
}

TArray<FCanvasUVTri> UBurinWorld::GetTriangles(int mode) {
	return MapLowZoom->GetTriangles(Terrain, mode);
}

TArray<FCanvasUVTri> UBurinWorld::GetMaterialTriangles(int mode) {
	return MapLowZoom->GetMaterialTriangles(Terrain, mode);
}

TArray<FLineDisplayData> UBurinWorld::GetBorders(int mode) {
	return MapLowZoom->GetBorders(mode);
}

TArray<FCanvasUVTri> UBurinWorld::GetProvinceTriangles(int mode) {
	return MapLowZoom->GetProvinceTriangles(Provinces, mode);
}