// Fill out your copyright notice in the Description page of Project Settings.


#include "UWorldCreator.h"

FArea UWorldCreator::CreateArea(FString name, double latitude, double longitude) {
	FArea area;
	//province.
	return area;
}

void UWorldCreator::DestroyArea(FString name) {

}

static FColor ToFColor(const TArray<uint8>& rgb) {
	return (rgb.Num() >= 3) ? FColor(rgb[0], rgb[1], rgb[2]) : FColor::Black;
}

static FPolity ToFPolity(const FPolityDataEntry& entry) {
	FPolity polity;

	polity.Name = entry.Id;
	polity.MapColor1 = ToFColor(entry.MapColor1);
	polity.MapColor2 = ToFColor(entry.MapColor2);
	polity.MapColor3 = ToFColor(entry.MapColor3);

	return polity;
}

// An independent place is its own polity, and the data carries no map colour for it -- most of the
// early map is independent (48 of the 71 places existing in 2500 BC), so leaving them on FPolity's
// default black would paint most of the world black. Derive a colour from the name instead, so the
// same place keeps the same colour across every year and every run.
//
// This is a placeholder: 256 hues over dozens of independents will occasionally give two neighbours
// similar colours. Assigning evenly spaced hues by index would avoid that, but the colours would
// then shift whenever the set of places changes from year to year, which reads worse on a map you
// scrub through time with.
static FColor MakeIndependentMapColor(const FString& name) {
	const uint8 hue = static_cast<uint8>(FCrc::StrCrc32(*name) % 256);
	return FLinearColor::MakeFromHSV8(hue, 140, 225).ToFColor(true);
}

static void CreateIndependentPlace(UBurinWorld* world, const FPlaceDataEntry& entry) {
	// An unclaimed ("null" owner) place governs itself: it gets its own single-place polity,
	// named after the place, rather than an unresolved controller.
	FPolity independentPolity;
	independentPolity.Name = entry.Name;
	independentPolity.MapColor1 = MakeIndependentMapColor(entry.Name);

	const int32 controllerIndex = world->Polities.Add(independentPolity);
	UWorldCreator::CreatePlace(world, entry.Name, entry.Latitude, entry.Longitude, controllerIndex);
}

void UWorldCreator::CreateHistoricalPlace(UBurinWorld* world, const FPlaceDataEntry& entry, int32 controllerIndex) {
	CreatePlace(
		world,
		entry.Name,
		entry.Latitude,
		entry.Longitude,
		controllerIndex
	);
}

int32 UWorldCreator::CreateHistoricalPolity(UBurinWorld* world, const FPolityDataEntry& entry) {
	return world->Polities.Add(ToFPolity(entry));
}

void UWorldCreator::CreatePlace(UBurinWorld* world, FString name, double latitude, double longitude, int32 controllerIndex) {
	FPlace place;

	place.Name = name;
	place.CommonName = name;
	place.Latitude = latitude;
	place.Longitude = longitude;
	place.ControllerIndex = controllerIndex;

	// FindSeedTriangleForPlace() handles the mesh's southward y axis and the offshore-coastal
	// fallback. The domain itself is grown later, once every place for the year exists, by
	// UBurinWorld::BuildPlaceDomains().
	place.SeedTriangle = world->FindSeedTriangleForPlace(latitude, longitude);
	place.Triangles = {};

	if (place.SeedTriangle == INDEX_NONE) {
		UE_LOG(LogTemp, Warning, TEXT("Place '%s' at (%f, %f) has no land triangle within reach at level 1; it gets no domain."), *name, latitude, longitude);
	}

	world->Places.Add(place);
}

void UWorldCreator::DestroyProvince(FString name) {

}

void UWorldCreator::CreateRandomWorld(UBurinWorld* world) {

}

// One place that exists in the requested year, held between the two passes below so that
// polities can be created before the places that index them.
struct FLivePlace
{
	// Index into FPlaces::HistoricalPlaceData.
	int32 EntryIndex = INDEX_NONE;

	// Index into FPolities::HistoricalPolityData, or INDEX_NONE if the place is independent or
	// its owner id didn't resolve to a known polity.
	int32 OwnerPolityIndex = INDEX_NONE;

	bool bIndependent = false;
};

void UWorldCreator::CreateHistoricalWorld(UBurinWorld* world, int32 year) {
	world->Places = {};
	world->Polities = {};

	// Polities carry no "dissolved" marker of their own, so a polity is only loaded if some
	// existing place is owned by it this year, rather than checking the polity's own history.
	TSet<int32> ownerPolityIndices;
	TArray<FLivePlace> livePlaces;

	for (int32 i = 0; i < world->HistoricalPlaces->HistoricalPlaceData.Num(); i++) {
		const FPlaceDataEntry& entry = world->HistoricalPlaces->HistoricalPlaceData[i];
		if (!entry.ExistsInYear(year)) {
			continue; // not founded yet, or currently ruined
		}

		// ExistsInYear() guarantees this is non-null and its Owner isn't "ruined".
		const FPlaceHistoryItemEntry* current = entry.GetHistoryItemForYear(year);

		FLivePlace live;
		live.EntryIndex = i;
		live.bIndependent = current->Owner.Equals(TEXT("null"), ESearchCase::IgnoreCase);
		live.OwnerPolityIndex = live.bIndependent ? INDEX_NONE : current->OwnerPolityIndex;
		livePlaces.Add(live);

		if (live.OwnerPolityIndex != INDEX_NONE) {
			ownerPolityIndices.Add(live.OwnerPolityIndex);
		}
	}

	// Historical polities are appended first, in HistoricalPolityData order, so a polity's slot
	// in world->Polities depends only on which other historical polities exist this year -- not
	// on how many independent places happen to precede it in the place list.
	TArray<int32> historicalToWorldPolity;
	historicalToWorldPolity.Init(INDEX_NONE, world->HistoricalPolities->HistoricalPolityData.Num());

	for (int32 i = 0; i < world->HistoricalPolities->HistoricalPolityData.Num(); i++) {
		if (ownerPolityIndices.Contains(i)) {
			historicalToWorldPolity[i] = CreateHistoricalPolity(world, world->HistoricalPolities->HistoricalPolityData[i]);
		}
	}

	for (const FLivePlace& live : livePlaces) {
		const FPlaceDataEntry& entry = world->HistoricalPlaces->HistoricalPlaceData[live.EntryIndex];

		if (live.bIndependent) {
			CreateIndependentPlace(world, entry);
			continue;
		}

		// An owner id that didn't match any known polity leaves the place with no controller;
		// FPlaces::InitializeHistoricalPlaces already warned about it when the data was loaded.
		const int32 controllerIndex = (live.OwnerPolityIndex != INDEX_NONE)
			? historicalToWorldPolity[live.OwnerPolityIndex]
			: INDEX_NONE;

		CreateHistoricalPlace(world, entry, controllerIndex);
	}

	// Every place for the year exists now, so the domains can be grown against each other.
	world->BuildPlaceDomains();
}
