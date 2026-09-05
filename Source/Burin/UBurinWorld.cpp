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

int32 UBurinWorld::FindSeedTriangleForPlace(double latitude, double longitude, double maxDistanceKm) {
	if (!EnsureInitialized(TEXT("FindSeedTriangleForPlace"))) return INDEX_NONE;

	// The mesh's y axis points south, so a real latitude has to be negated to index it; see the
	// FMapLowZoom class comment.
	return MapLowZoom->FindLandTriangleNear(0, longitude, -latitude, maxDistanceKm);
}

void UBurinWorld::BuildPlaceDomains() {
	if (!EnsureInitialized(TEXT("BuildPlaceDomains"))) return;

	// Domains are per level -- each level has its own mesh, its own coastline and its own triangle
	// ids -- so the places changing invalidates all of them. Level 1 is built now because the click
	// lookups and the overview both want it immediately; the finer levels, which together hold
	// roughly ten times its triangles, are built the first time the camera actually reaches one.
	MapLowZoom->InvalidatePlaceDomains();
	MapLowZoom->EnsurePlaceDomains(Places, Polities, 0, DomainRadiusKm, bRiversBlockDomains);
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
	//if (GEngine)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("Year %d: %d places, %d polities"), CurrentYear, Places.Num(), Polities.Num()));
	//}

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

TArray<FCanvasUVTri> UBurinWorld::GetTriangles(int32 mode, int32 zoomCategory, double minFracY, double minFracX, double maxFracY, double maxFracX, int32 offsetX, int32 offsetY, int32 width, int32 height) {
	if (!EnsureInitialized(TEXT("GetTriangles"))) return {};

	return MapLowZoom->GetTriangles(Terrain.Get(), mode, zoomCategory, minFracY, minFracX, maxFracY, maxFracX, offsetX, offsetY, width, height);
}

TArray<FCanvasUVTri> UBurinWorld::GetMaterialTriangles(int32 mode, int32 zoomCategory, int32 tileX, int32 tileY) {
	if (!EnsureInitialized(TEXT("GetMaterialTriangles"))) return {};

	return MapLowZoom->GetMaterialTriangles(Terrain.Get(), mode, zoomCategory, tileX, tileY);
}

TArray<FLineDisplayData> UBurinWorld::GetBorders(int32 mode, int32 zoomCategory, int32 tileX, int32 tileY) {
	if (!EnsureInitialized(TEXT("GetBorders"))) return {};

	return MapLowZoom->GetBorders(mode, zoomCategory, tileX, tileY);
}

TArray<FLineDisplayData> UBurinWorld::GetRivers(int32 mode, int32 zoomCategory, int32 tileX, int32 tileY) {
	if (!EnsureInitialized(TEXT("GetRivers"))) return {};

	return MapLowZoom->GetRivers(mode, zoomCategory, tileX, tileY);
}

TArray<FCanvasUVTri> UBurinWorld::GetProvinceTriangles(int32 mode, int32 zoomCategory, double minFracY, double minFracX, double maxFracY, double maxFracX, int32 offsetX, int32 offsetY, int32 width, int32 height) {
	if (!EnsureInitialized(TEXT("GetProvinceTriangles"))) return {};
	if (!bShowPlaceDomains) return {};

	// Asked before building, not after. A level's domains cost the same to build whether or not
	// anything draws them, so a geographic mode that discards the result would otherwise pay a
	// level-3 build in full for nothing -- and pay it again after every year change.
	if (!FMapLowZoom::ModeShowsDomains(mode)) return {};

	// Builds this level's domains if the camera has not been here since the year last changed. Once
	// per level per year, not once per frame -- EnsurePlaceDomains() returns immediately after that.
	MapLowZoom->EnsurePlaceDomains(Places, Polities, zoomCategory, DomainRadiusKm, bRiversBlockDomains);

	return MapLowZoom->GetProvinceTriangles(mode, zoomCategory, minFracY, minFracX, maxFracY, maxFracX, offsetX, offsetY, width, height);
}

FDomainInfo UBurinWorld::GetDomainAtCoordinate(int32 zoomCategory, double x, double y) {
    FDomainInfo info;
    if (!MapLowZoom.IsValid()) {
        return info;
    }

    // A click can arrive at a level the camera has drawn domains for or, if the caller asks about
    // a level it is not showing, at one that has never been built. Either way this is what makes
    // the answer exist.
    MapLowZoom->EnsurePlaceDomains(Places, Polities, zoomCategory, DomainRadiusKm, bRiversBlockDomains);

    const int32 placeIndex = MapLowZoom->GetPlaceAtCoordinate(zoomCategory, x, y);
    if (!Places.IsValidIndex(placeIndex)) {
        return info; // water, off the mesh, or ground no place reached
    }

    const FPlace& place = Places[placeIndex];
    info.bValid = true;
    info.PlaceIndex = placeIndex;
    info.PlaceName = place.CommonName.IsEmpty() ? place.Name : place.CommonName;

    if (Polities.IsValidIndex(place.ControllerIndex)) {
        info.bHasPolity = true;
        info.PolityIndex = place.ControllerIndex;
        info.PolityName = Polities[place.ControllerIndex].Name;
        info.PolityColor = Polities[place.ControllerIndex].MapColor1;
    }

    TMap<int32, double> areasByTerrainData;
    MapLowZoom->GetDomainTerrainAreas(zoomCategory, placeIndex, areasByTerrainData, info.TotalAreaKm2);

    // Group by the terrain index the rest of the project uses rather than the raw packed value, so
    // that what comes back can be compared against GetTerrainDataAtCoordinate() and so two triangles
    // the project calls the same terrain are one row here, not two.
    TMap<int32, double> areasByTerrainType;
    for (const TPair<int32, double>& entry : areasByTerrainData) {
        const int32 terrainType = Terrain.IsValid() ? Terrain->GetTerrainFromCache(entry.Key) : entry.Key;
        areasByTerrainType.FindOrAdd(terrainType) += entry.Value;
    }

    info.Terrain.Reserve(areasByTerrainType.Num());
    for (const TPair<int32, double>& entry : areasByTerrainType) {
        FDomainTerrainShare share;
        share.TerrainType = entry.Key;
        share.TerrainName = Terrain.IsValid() ? Terrain->GetTerrainText(entry.Key) : FString();
        share.AreaKm2 = entry.Value;
        share.Fraction = (info.TotalAreaKm2 > 0.0) ? entry.Value / info.TotalAreaKm2 : 0.0;
        info.Terrain.Add(MoveTemp(share));
    }

    info.Terrain.Sort([](const FDomainTerrainShare& a, const FDomainTerrainShare& b) {
        return a.AreaKm2 > b.AreaKm2;
    });

    return info;
}

TArray<int32> UBurinWorld::GetSubregionIndices(int32 zoomCategory, double y, double x, double yDelta, double xDelta) {
	if (!EnsureInitialized(TEXT("GetSubregionIndices"))) return {};

	return MapLowZoom->GetSubregionIndices(zoomCategory, y, x, yDelta, xDelta);
}

int32 UBurinWorld::GetNumSubregions(int32 zoomCategory, bool isX) {
	if (!EnsureInitialized(TEXT("GetNumSubregions"))) return 0;

	return MapLowZoom->GetNumSubregions(zoomCategory, isX);
}
