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

TArray<FCanvasUVTri> UBurinWorld::GetTerrainTriangles(int32 mode, int32 zoomCategory, double minFracY, double minFracX, double maxFracY, double maxFracX, int32 offsetX, int32 offsetY, int32 width, int32 height) {
	if (!EnsureInitialized(TEXT("GetTerrainTriangles"))) return {};

	// Applied before the draw rather than at edit time, so changing it in the details panel takes
	// effect on the next frame instead of needing the level reopened.
	MapLowZoom->SetVertexBlendSettings(BlendRadiusDegrees, HillshadeStrength);
	MapLowZoom->SetProbeCoordinate(ProbeLatitude, ProbeLongitude);

	return MapLowZoom->GetTerrainTriangles(Terrain.Get(), mode, zoomCategory, minFracY, minFracX, maxFracY, maxFracX, offsetX, offsetY, width, height);
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

TArray<FCanvasUVTri> UBurinWorld::GetDomainTriangles(int32 mode, int32 zoomCategory, double minFracY, double minFracX, double maxFracY, double maxFracX, int32 offsetX, int32 offsetY, int32 width, int32 height) {
	if (!EnsureInitialized(TEXT("GetDomainTriangles"))) return {};
	if (!bShowPlaceDomains) return {};

	// Asked before building, not after. A level's domains cost the same to build whether or not
	// anything draws them, so a geographic mode that discards the result would otherwise pay a
	// level-3 build in full for nothing -- and pay it again after every year change.
	if (!FMapLowZoom::ModeShowsDomains(mode)) return {};

	// Builds this level's domains if the camera has not been here since the year last changed. Once
	// per level per year, not once per frame -- EnsurePlaceDomains() returns immediately after that.
	MapLowZoom->EnsurePlaceDomains(Places, Polities, zoomCategory, DomainRadiusKm, bRiversBlockDomains);

	return MapLowZoom->GetDomainTriangles(mode, zoomCategory, minFracY, minFracX, maxFracY, maxFracX, offsetX, offsetY, width, height);
}

TArray<FMapDrawCall> UBurinWorld::PlanDraws(int32 mapMode, int32 zoomCategory, double y, double x, double yDelta, double xDelta, int32 renderTargetWidth, int32 renderTargetHeight) {
	if (!EnsureInitialized(TEXT("PlanDraws"))) return {};

	return MapLowZoom->PlanDraws(mapMode, zoomCategory, y, x, yDelta, xDelta, renderTargetWidth, renderTargetHeight);
}

TArray<FMapDrawCall> UBurinWorld::PlanDrawsForView(int32 mapMode, int32 zoomCategory, double latitude, double longitude, double halfAngleDegrees, int32 renderTargetWidth, int32 renderTargetHeight) {
	const double halfAngle = FMath::Max(halfAngleDegrees, 0.0);

	// Latitude flipped, because the mesh counts y southward from the north pole.
	const double y = -latitude;

	// Longitude degrees shorten by cos(latitude), so covering the same ground takes more of them
	// away from the equator. Floored so the poles, where the factor runs to infinity, ask for half
	// a turn rather than an unbounded number, and capped at half a turn for the same reason.
	const double cosLatitude = FMath::Max(FMath::Cos(FMath::DegreesToRadians(latitude)), 0.01);
	const double xDelta = FMath::Min(halfAngle / cosLatitude, 180.0);

	return PlanDraws(mapMode, zoomCategory, y, longitude, halfAngle, xDelta, renderTargetWidth, renderTargetHeight);
}

FMapDrawCall UBurinWorld::MakeTileDrawCall(int32 zoomCategory, int32 tileX, int32 tileY, int32 renderTargetWidth, int32 renderTargetHeight) {
	if (!EnsureInitialized(TEXT("MakeTileDrawCall"))) return FMapDrawCall();

	return MapLowZoom->MakeTileDrawCall(zoomCategory, tileX, tileY, renderTargetWidth, renderTargetHeight);
}

void UBurinWorld::ResetMapCoverage() {
	if (!EnsureInitialized(TEXT("ResetMapCoverage"))) return;

	MapLowZoom->ResetCoverage();
}

TArray<FCanvasUVTri> UBurinWorld::GetTerrainTrianglesForDrawCall(int32 mode, const FMapDrawCall& drawCall) {
	return GetTerrainTriangles(mode, drawCall.ZoomCategory,
		drawCall.MinFracY, drawCall.MinFracX, drawCall.MaxFracY, drawCall.MaxFracX,
		drawCall.OffsetX, drawCall.OffsetY, drawCall.Width, drawCall.Height);
}

TArray<FCanvasUVTri> UBurinWorld::GetDomainTrianglesForDrawCall(int32 mode, const FMapDrawCall& drawCall) {
	return GetDomainTriangles(mode, drawCall.ZoomCategory,
		drawCall.MinFracY, drawCall.MinFracX, drawCall.MaxFracY, drawCall.MaxFracX,
		drawCall.OffsetX, drawCall.OffsetY, drawCall.Width, drawCall.Height);
}

TArray<FCanvasUVTri> UBurinWorld::GetBorderTriangles(int32 mode, int32 zoomCategory, double minFracY, double minFracX, double maxFracY, double maxFracX, int32 offsetX, int32 offsetY, int32 width, int32 height, double thickness) {
	if (!EnsureInitialized(TEXT("GetBorderTriangles"))) return {};

	return MapLowZoom->GetBorderTriangles(mode, zoomCategory, minFracY, minFracX, maxFracY, maxFracX, offsetX, offsetY, width, height, thickness);
}

TArray<FCanvasUVTri> UBurinWorld::GetBorderTrianglesForDrawCall(int32 mode, const FMapDrawCall& drawCall, double thickness) {
	return GetBorderTriangles(mode, drawCall.ZoomCategory,
		drawCall.MinFracY, drawCall.MinFracX, drawCall.MaxFracY, drawCall.MaxFracX,
		drawCall.OffsetX, drawCall.OffsetY, drawCall.Width, drawCall.Height, thickness);
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

TArray<int32> UBurinWorld::GetSubregionIndicesForView(int32 zoomCategory, double latitude, double longitude, double halfAngleDegrees) {
	if (!EnsureInitialized(TEXT("GetSubregionIndicesForView"))) return {};

	// Deliberately the same two conversions PlanDrawsForView makes, so the map and the sphere agree
	// about which tiles a given view wants.
	const double halfAngle = FMath::Max(halfAngleDegrees, 0.0);
	const double y = -latitude;

	const double cosLatitude = FMath::Max(FMath::Cos(FMath::DegreesToRadians(latitude)), 0.01);
	const double xDelta = FMath::Min(halfAngle / cosLatitude, 180.0);

	return MapLowZoom->GetSubregionIndices(zoomCategory, y, longitude, halfAngle, xDelta);
}

void UBurinWorld::GetRenderTargetPixel(double latitude, double longitude, double minLatitude, double maxLatitude, double minLongitude, double maxLongitude, int32 renderTargetWidth, int32 renderTargetHeight, int32& outX, int32& outY) {
	outX = 0;
	outY = 0;

	const double spanLongitude = maxLongitude - minLongitude;
	const double spanLatitude = maxLatitude - minLatitude;
	if (FMath::IsNearlyZero(spanLongitude) || FMath::IsNearlyZero(spanLatitude)) {
		return;
	}

	// Y counts down from the north edge, because a render target's first row is its top one and the
	// mesh counts y southward from the pole. This is the same negation that mirrored the tile window
	// about the equator when it was got wrong in Blueprint, so it lives here now.
	const double fractionX = (longitude - minLongitude) / spanLongitude;
	const double fractionY = (maxLatitude - latitude) / spanLatitude;

	outX = FMath::Clamp(FMath::FloorToInt32(fractionX * renderTargetWidth), 0, FMath::Max(renderTargetWidth - 1, 0));
	outY = FMath::Clamp(FMath::FloorToInt32(fractionY * renderTargetHeight), 0, FMath::Max(renderTargetHeight - 1, 0));
}

void UBurinWorld::GetTileCentreCoordinate(int32 zoomCategory, int32 tileX, int32 tileY, double& latitude, double& longitude) {
	latitude = 0.0;
	longitude = 0.0;
	if (!EnsureInitialized(TEXT("GetTileCentreCoordinate"))) return;

	const int32 w = MapLowZoom->GetNumSubregions(zoomCategory, true);
	const int32 h = MapLowZoom->GetNumSubregions(zoomCategory, false);
	if (w <= 0 || h <= 0) return;

	longitude = -180.0 + (tileX + 0.5) * 360.0 / w;

	// Row 0 is the north pole's row: mesh y counts southward from -90, and latitude is its negation.
	latitude = 90.0 - (tileY + 0.5) * 180.0 / h;
}

int32 UBurinWorld::GetNumSubregions(int32 zoomCategory, bool isX) {
	if (!EnsureInitialized(TEXT("GetNumSubregions"))) return 0;

	return MapLowZoom->GetNumSubregions(zoomCategory, isX);
}
