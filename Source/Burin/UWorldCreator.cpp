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

static void CreateIndependentPlace(UBurinWorld* world, const FPlaceDataEntry& entry) {
	// An unclaimed ("null" owner) place governs itself: it gets its own single-place polity,
	// named after the place, rather than an empty/default Controller.
	FPolity independentPolity;
	independentPolity.Name = entry.Name;

	world->Polities.Add(independentPolity);
	UWorldCreator::CreatePlace(world, entry.Name, entry.Latitude, entry.Longitude, independentPolity);
}

void UWorldCreator::CreateHistoricalPlace(UBurinWorld* world, FPlaceDataEntry entry, int32 ownerPolityIndex) {
	FPolity controller;
	if (ownerPolityIndex != INDEX_NONE) {
		controller = ToFPolity(world->HistoricalPolities->HistoricalPolityData[ownerPolityIndex]);
	}

	CreatePlace(
		world,
		entry.Name,
		entry.Latitude,
		entry.Longitude,
		controller
	);
}

void UWorldCreator::CreateHistoricalPolity(UBurinWorld* world, FPolityDataEntry entry) {
	world->Polities.Add(ToFPolity(entry));
}

void UWorldCreator::CreatePlace(UBurinWorld* world, FString name, double latitude, double longitude, FPolity controller) {
	FPlace place;

	place.Name = name;
	place.CommonName = name;
	place.Latitude = latitude;
	place.Longitude = longitude;
	place.Controller = controller;

	place.Triangles = {};
	place.Triangles.Add(world->GetTriangleIDAtCoordinate(0, longitude, latitude));

	world->Places.Add(place);
}

void UWorldCreator::DestroyProvince(FString name) {

}

void UWorldCreator::CreateRandomWorld(UBurinWorld* world) {

}

void UWorldCreator::CreateHistoricalWorld(UBurinWorld* world, int32 year) {
	world->Places = {};
	world->Polities = {};

	// Polities carry no "dissolved" marker of their own, so a polity is only loaded if some
	// existing place is owned by it this year, rather than checking the polity's own history.
	TSet<int32> ownerPolityIndices;

	for (int32 i = 0; i < world->HistoricalPlaces->HistoricalPlaceData.Num(); i++) {
		const FPlaceDataEntry& entry = world->HistoricalPlaces->HistoricalPlaceData[i];
		if (!entry.ExistsInYear(year)) {
			continue; // not founded yet, or currently ruined
		}

		// ExistsInYear() guarantees this is non-null and its Owner isn't "ruined".
		const FPlaceHistoryItemEntry* current = entry.GetHistoryItemForYear(year);

		if (current->Owner.Equals(TEXT("null"), ESearchCase::IgnoreCase)) {
			CreateIndependentPlace(world, entry);
			continue;
		}

		CreateHistoricalPlace(world, entry, current->OwnerPolityIndex);
		if (current->OwnerPolityIndex != INDEX_NONE) {
			ownerPolityIndices.Add(current->OwnerPolityIndex);
		}
	}

	for (int32 i = 0; i < world->HistoricalPolities->HistoricalPolityData.Num(); i++) {
		if (ownerPolityIndices.Contains(i)) {
			CreateHistoricalPolity(world, world->HistoricalPolities->HistoricalPolityData[i]);
		}
	}
}
