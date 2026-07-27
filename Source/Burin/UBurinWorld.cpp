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

int UBurinWorld::GetTerrainDataAtCoordinate(int zoomCategory, double x, double y) {
	return MapLowZoom->GetTerrainDataAtCoordinate(Terrain, zoomCategory, x, y);
}

int UBurinWorld::GetTriangleIDAtCoordinate(int zoomCategory, double x, double y) {
	return MapLowZoom->GetTriangleIDAtCoordinate(zoomCategory, x, y);
}

FString UBurinWorld::GetTerrainText(int v) {
	return MapLowZoom->GetTerrainText(Terrain, v);
}

TArray<FCanvasUVTri> UBurinWorld::GetTriangles(int mode, int zoomCategory, double minLat, double minLon, double maxLat, double maxLon, int offsetX, int offsetY, int width, int height) {
	return MapLowZoom->GetTriangles(Terrain, mode, zoomCategory, minLat, minLon, maxLat, maxLon, offsetX, offsetY, width, height);
}

TArray<FCanvasUVTri> UBurinWorld::GetMaterialTriangles(int mode, int zoomCategory, int x, int y) {
	return MapLowZoom->GetMaterialTriangles(Terrain, mode, zoomCategory, x, y);
}

TArray<FLineDisplayData> UBurinWorld::GetBorders(int mode, int zoomCategory, int x, int y) {
	return MapLowZoom->GetBorders(mode, zoomCategory, x, y);
}

TArray<FLineDisplayData> UBurinWorld::GetRivers(int mode, int zoomCategory, int x, int y) {
	return MapLowZoom->GetRivers(mode, zoomCategory, x, y);
}

TArray<FCanvasUVTri> UBurinWorld::GetProvinceTriangles(int mode, int zoomCategory, int x, int y) {
	return MapLowZoom->GetProvinceTriangles(Provinces, mode, zoomCategory, x, y);
}

TArray<int> UBurinWorld::GetSubregionIndices(int zoomCategory, double lat, double lon, double latDelta, double lonDelta) {
	return MapLowZoom->GetSubregionIndices( zoomCategory, lat, lon, latDelta, lonDelta);
}

int UBurinWorld::GetNumSubregions(int zoomCategory, bool isX) {
	return MapLowZoom->GetNumSubregions(zoomCategory, isX);
}