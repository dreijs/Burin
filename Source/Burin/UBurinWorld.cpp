// Fill out your copyright notice in the Description page of Project Settings.


#include "UBurinWorld.h"
#include "Data/Earth/FTerrainDataEntry.h"
#include "Data/Earth/FTerrain.h"
#include "UWorldCreator.h"
#include "Engine/Engine.h"

bool UBurinWorld::IsInitialized() const
{
	return Terrain.IsValid()
		&& HistoricalPolities.IsValid()
		&& HistoricalPlaces.IsValid()
		&& MapLowZoom.IsValid();
}

bool UBurinWorld::EnsureInitialized(const TCHAR* callerName) const
{
	if (IsInitialized())
	{
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("UBurinWorld::%s called before Initialize(); returning an empty result."), callerName);
	return false;
}

void UBurinWorld::InitializeTerrain() {
	Terrain = MakeUnique<FTerrain>();

	Terrain->InitializeElevation();
	Terrain->InitializeVegetation();
	Terrain->InitializeSoil();
	Terrain->InitializeFeatures();
	Terrain->InitializeTerrain();
	Terrain->InitializeGeographicLabels();

	Terrain->InitializeTerrainMapping();
}

void UBurinWorld::InitializeHistory() {
	HistoricalPolities = MakeUnique<FPolities>();
	HistoricalPlaces = MakeUnique<FPlaces>();

	HistoricalPolities->InitializeHistoricalPolities();
	HistoricalPlaces->InitializeHistoricalPlaces(*HistoricalPolities);
}

void UBurinWorld::InitializeMap() {
	MapLowZoom = MakeUnique<FMapLowZoom>();

	MapLowZoom->Initialize();
}

void UBurinWorld::SetWorldCreatorSettings() {

}

void UBurinWorld::InitializeProvinces()
{
	if (!EnsureInitialized(TEXT("InitializeProvinces"))) return;

	UWorldCreator::CreateHistoricalWorld(this, CurrentYear);
}

void UBurinWorld::SetCurrentYear(int32 year)
{
	if (!EnsureInitialized(TEXT("SetCurrentYear"))) return;

	UE_LOG(LogTemp, Log, TEXT("SetCurrentYear(%d) on world '%s': master lists hold %d places, %d polities before rebuild"),
		year, *GetName(), HistoricalPlaces->HistoricalPlaceData.Num(), HistoricalPolities->HistoricalPolityData.Num());

	CurrentYear = year;
	UWorldCreator::CreateHistoricalWorld(this, CurrentYear);

	UE_LOG(LogTemp, Log, TEXT("SetCurrentYear(%d): loaded %d places, %d polities"), CurrentYear, Places.Num(), Polities.Num());
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("Year %d: %d places, %d polities"), CurrentYear, Places.Num(), Polities.Num()));
	}

	OnCurrentYearChanged.Broadcast(CurrentYear);
}

void UBurinWorld::Initialize()
{
	if (IsInitialized())
	{
		UE_LOG(LogTemp, Warning, TEXT("UBurinWorld::Initialize called more than once; ignoring."));
		return;
	}

	InitializeTerrain();
	InitializeHistory();
	InitializeMap();

	UE_LOG(LogTemp, Log, TEXT("UBurinWorld::Initialize complete on world '%s'"), *GetName());
}

int32 UBurinWorld::GetTerrainDataAtCoordinate(int32 zoomCategory, double x, double y) {
	if (!EnsureInitialized(TEXT("GetTerrainDataAtCoordinate"))) return -1;

	return MapLowZoom->GetTerrainDataAtCoordinate(Terrain.Get(), zoomCategory, x, y);
}

int32 UBurinWorld::GetTriangleIDAtCoordinate(int32 zoomCategory, double x, double y) {
	if (!EnsureInitialized(TEXT("GetTriangleIDAtCoordinate"))) return -1;

	return MapLowZoom->GetTriangleIDAtCoordinate(zoomCategory, x, y);
}

FString UBurinWorld::GetTerrainText(int32 v) {
	if (!EnsureInitialized(TEXT("GetTerrainText"))) return FString();

	return MapLowZoom->GetTerrainText(Terrain.Get(), v);
}

TArray<FCanvasUVTri> UBurinWorld::GetTriangles(int32 mode, int32 zoomCategory, double minLat, double minLon, double maxLat, double maxLon, int32 offsetX, int32 offsetY, int32 width, int32 height) {
	if (!EnsureInitialized(TEXT("GetTriangles"))) return {};

	return MapLowZoom->GetTriangles(Terrain.Get(), mode, zoomCategory, minLat, minLon, maxLat, maxLon, offsetX, offsetY, width, height);
}

TArray<FCanvasUVTri> UBurinWorld::GetMaterialTriangles(int32 mode, int32 zoomCategory, int32 x, int32 y) {
	if (!EnsureInitialized(TEXT("GetMaterialTriangles"))) return {};

	return MapLowZoom->GetMaterialTriangles(Terrain.Get(), mode, zoomCategory, x, y);
}

TArray<FLineDisplayData> UBurinWorld::GetBorders(int32 mode, int32 zoomCategory, int32 x, int32 y) {
	if (!EnsureInitialized(TEXT("GetBorders"))) return {};

	return MapLowZoom->GetBorders(mode, zoomCategory, x, y);
}

TArray<FLineDisplayData> UBurinWorld::GetRivers(int32 mode, int32 zoomCategory, int32 x, int32 y) {
	if (!EnsureInitialized(TEXT("GetRivers"))) return {};

	return MapLowZoom->GetRivers(mode, zoomCategory, x, y);
}

TArray<FCanvasUVTri> UBurinWorld::GetProvinceTriangles(int32 mode, int32 zoomCategory, int32 x, int32 y) {
	if (!EnsureInitialized(TEXT("GetProvinceTriangles"))) return {};

	return MapLowZoom->GetProvinceTriangles(Places, mode, zoomCategory, x, y);
}

TArray<int32> UBurinWorld::GetSubregionIndices(int32 zoomCategory, double lat, double lon, double latDelta, double lonDelta) {
	if (!EnsureInitialized(TEXT("GetSubregionIndices"))) return {};

	return MapLowZoom->GetSubregionIndices( zoomCategory, lat, lon, latDelta, lonDelta);
}

int32 UBurinWorld::GetNumSubregions(int32 zoomCategory, bool isX) {
	if (!EnsureInitialized(TEXT("GetNumSubregions"))) return 0;

	return MapLowZoom->GetNumSubregions(zoomCategory, isX);
}
