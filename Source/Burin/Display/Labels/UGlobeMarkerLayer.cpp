// Fill out your copyright notice in the Description page of Project Settings.


#include "UGlobeMarkerLayer.h"

#include "UGeoCoordinateLibrary.h"
#include "UGlobeMarkerWidget.h"
#include "Burin/UBurinWorld.h"
#include "Burin/Data/Earth/FTerrain.h"
#include "Burin/Data/Earth/FGeographicLabel.h"
#include "Burin/Concepts/Provinces/FPlace.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"

namespace
{
	// FGeographicLabel::Color holds sRGB bytes read from the XML; UMG wants linear.
	FLinearColor LabelColorToLinear(const TArray<uint8>& bytes)
	{
		if (bytes.Num() < 3)
		{
			return FLinearColor::White;
		}

		const uint8 alpha = (bytes.Num() >= 4) ? bytes[3] : 255;
		return FLinearColor(FColor(bytes[0], bytes[1], bytes[2], alpha));
	}

	// Source data carries line breaks as the literal two characters \n, since the XML loader
	// reads a tag's content as a single line. Whitespace around the break is trimmed so the
	// centred lines don't sit off-centre.
	FString ExpandLineBreaks(const FString& text)
	{
		if (!text.Contains(TEXT("\\n")))
		{
			return text;
		}

		TArray<FString> lines;
		text.ParseIntoArray(lines, TEXT("\\n"), false);
		for (FString& line : lines)
		{
			line.TrimStartAndEndInline();
		}

		return FString::Join(lines, TEXT("\n"));
	}
}

UGlobeMarkerLayer::UGlobeMarkerLayer(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{
	// Indexed by zoom level; value is the highest importance still displayed at that zoom.
	// -1 and 0 blank zoom levels entirely, since importance values start at 1.
	MaxImportanceByZoomLevel = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
}

void UGlobeMarkerLayer::SetZoomLevel(int32 zoomLevel)
{
	CurrentZoomLevel = zoomLevel;
}

int32 UGlobeMarkerLayer::GetMaxImportanceForZoomLevel(int32 zoomLevel) const
{
	if (MaxImportanceByZoomLevel.Num() == 0)
	{
		return TNumericLimits<int32>::Max();	// no table configured: show everything
	}

	// Zoom levels past the end of the table stay at the most permissive entry.
	const int32 index = FMath::Clamp(zoomLevel, 0, MaxImportanceByZoomLevel.Num() - 1);
	return MaxImportanceByZoomLevel[index];
}

void UGlobeMarkerLayer::SetProjectionMode(EGlobeMarkerProjection projection)
{
	ProjectionMode = projection;
}

void UGlobeMarkerLayer::SetMapView(double centreLatitude, double centreLongitude, FVector mapOrigin, FVector eastPerDegree, FVector northPerDegree)
{
	MapCentreLatitude = centreLatitude;
	MapCentreLongitude = centreLongitude;
	MapOrigin = mapOrigin;
	MapEastPerDegree = eastPerDegree;
	MapNorthPerDegree = northPerDegree;
}

bool UGlobeMarkerLayer::GetMarkerWorldPosition(const FLiveMarker& marker, const FTransform& globeTransform, double radius,
	const FVector& toCamera, double horizonCos, FVector& outWorldPosition) const
{
	if (ProjectionMode == EGlobeMarkerProjection::FlatMap)
	{
		// Wrapped, so a view sitting on the antimeridian keeps the markers just across it beside
		// itself instead of a whole map away. FRotator::NormalizeAxis does exactly this.
		const double deltaLongitude = FRotator::NormalizeAxis(marker.Longitude - MapCentreLongitude);
		const double deltaLatitude = marker.Latitude - MapCentreLatitude;

		outWorldPosition = MapOrigin + MapEastPerDegree * deltaLongitude + MapNorthPerDegree * deltaLatitude;
		return true;	// a flat map has no far side; the off-screen test does the rest
	}

	const FVector worldDirection = globeTransform.TransformVectorNoScale(marker.LocalDirection);
	if (FVector::DotProduct(worldDirection, toCamera) <= horizonCos)
	{
		return false;	// behind the globe
	}

	outWorldPosition = globeTransform.GetLocation() + worldDirection * radius;
	return true;
}

void UGlobeMarkerLayer::SetGlobeActor(AActor* globeActor)
{
	GlobeActor = globeActor;
}

void UGlobeMarkerLayer::InitializeLayer(UBurinWorld* world, EGlobeLabelSource source)
{
	LayerSource = source;

	if (world == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGlobeMarkerLayer::InitializeLayer called with no UBurinWorld; no markers created."));
		return;
	}

	TArray<FMarkerSeed> seeds;

	switch (source)
	{
	case EGlobeLabelSource::GeographicFeatures:
	{
		if (!world->Terrain.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("UGlobeMarkerLayer: Terrain is not initialized; no geographic markers created."));
			return;
		}

		const TArray<FGeographicLabel>& labels = world->Terrain->GeographicLabelData;
		seeds.Reserve(labels.Num());
		for (const FGeographicLabel& label : labels)
		{
			FMarkerSeed seed;
			seed.Name = ExpandLineBreaks(label.Name);
			seed.Latitude = label.Latitude;
			seed.Longitude = label.Longitude;
			seed.Importance = label.Importance;
			seed.Color = (label.Color.Num() >= 3) ? LabelColorToLinear(label.Color) : DefaultLabelColor;

			// A geographic feature has no owner, so its dot carries its own label colour.
			seed.MarkerColor = seed.Color;
			seeds.Add(MoveTemp(seed));
		}
		break;
	}

	case EGlobeLabelSource::HistoricalPlaces:
	{
		// Places is rebuilt by UBurinWorld::SetCurrentYear(), so this always reflects
		// whichever settlements exist as of the world's current year.
		const TArray<FPlace>& places = world->Places;
		seeds.Reserve(places.Num());
		for (const FPlace& place : places)
		{
			FMarkerSeed seed;
			// CommonName is the display form where one exists; Name is the canonical fallback.
			seed.Name = ExpandLineBreaks(place.CommonName.IsEmpty() ? place.Name : place.CommonName);
			seed.Latitude = place.Latitude;
			seed.Longitude = place.Longitude;
			seed.Importance = PlaceImportance;	// places carry no importance of their own yet
			seed.Color = DefaultLabelColor;

			// Whoever holds the place as of the world's current year. ControllerIndex is already
			// interned against UBurinWorld::Polities by the place loader, so this is a lookup rather
			// than a search -- and it is INDEX_NONE for a place whose owner did not resolve to a
			// known polity, which keeps its dot white rather than black.
			seed.MarkerColor = world->Polities.IsValidIndex(place.ControllerIndex)
				? FLinearColor(world->Polities[place.ControllerIndex].MapColor2)
				: FLinearColor::White;
			seeds.Add(MoveTemp(seed));
		}
		break;
	}
	}

	BuildMarkers(seeds);
}

void UGlobeMarkerLayer::BuildMarkers(const TArray<FMarkerSeed>& labels)
{
	if (MarkerWidgetClass == nullptr || MarkerCanvas == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGlobeMarkerLayer::BuildMarkers: MarkerWidgetClass or MarkerCanvas is not set."));
		return;
	}

	MarkerCanvas->ClearChildren();
	MarkerWidgets.Reset();
	Markers.Reset();

	// Lowest Importance first, so a MaxMarkers cap keeps the labels that matter most.
	TArray<int32> order;
	order.Reserve(labels.Num());
	for (int32 i = 0; i < labels.Num(); i++)
	{
		order.Add(i);
	}
	order.Sort([&labels](int32 a, int32 b) { return labels[a].Importance < labels[b].Importance; });

	const int32 numToBuild = (MaxMarkers > 0) ? FMath::Min(MaxMarkers, order.Num()) : order.Num();
	if (numToBuild < order.Num())
	{
		UE_LOG(LogTemp, Log, TEXT("UGlobeMarkerLayer: capping %d labels to the %d most important."), order.Num(), numToBuild);
	}

	Markers.Reserve(numToBuild);
	MarkerWidgets.Reserve(numToBuild);

	for (int32 i = 0; i < numToBuild; i++)
	{
		const FMarkerSeed& label = labels[order[i]];

		UGlobeMarkerWidget* widget = CreateWidget<UGlobeMarkerWidget>(GetOwningPlayer(), MarkerWidgetClass);
		if (widget == nullptr)
		{
			continue;
		}

		FLiveMarker marker;
		marker.TextColor = label.Color;
		marker.Importance = label.Importance;
		marker.LocalDirection = UGeoCoordinateLibrary::LatLonToUnitVector(label.Latitude, label.Longitude, LongitudeOffsetDeg, bFlipLongitude, bFlipLatitude);
		marker.Latitude = label.Latitude;
		marker.Longitude = label.Longitude;
		marker.Widget = widget;

		widget->OnMarkerInitialized(label.Name, marker.TextColor, label.MarkerColor, label.Importance, LayerSource);
		widget->SetVisibility(ESlateVisibility::Collapsed);

		marker.Slot = MarkerCanvas->AddChildToCanvas(widget);
		if (marker.Slot != nullptr)
		{
			marker.Slot->SetAnchors(FAnchors(0.0f, 0.0f));
			marker.Slot->SetAlignment(FVector2D(0.5f, 0.5f));	// widget centre lands on the coordinate
			marker.Slot->SetPosition(FVector2D::ZeroVector);	// all movement goes through the render transform
			marker.Slot->SetAutoSize(true);
		}

		MarkerWidgets.Add(widget);
		Markers.Add(MoveTemp(marker));
	}

	// Nothing here checks for a globe actor any more. InitializeLayer() is not given one -- the
	// sphere hands it over separately, and quite legitimately later than this -- so a complaint
	// raised here would fire on every correctly built layer. NativeTick() reports it instead, where
	// the answer is the one that matters: whether there was an actor at the moment it was needed.
	const double radius = GetEffectiveRadius();

	UE_LOG(LogTemp, Log, TEXT("[%s] UGlobeMarkerLayer: built %d markers, globe radius %.1f uu."), *GetName(), Markers.Num(), radius);

	// Histogram of the importance levels actually present in the data, cross-referenced against
	// MaxImportanceByZoomLevel, so the table can be authored and checked against real values.
	TMap<int32, int32> importanceCounts;
	for (const FMarkerSeed& label : labels)
	{
		importanceCounts.FindOrAdd(label.Importance)++;
	}
	importanceCounts.KeySort([](int32 a, int32 b) { return a < b; });

	for (const TPair<int32, int32>& pair : importanceCounts)
	{
		// Reverse lookup: the first zoom level whose ceiling admits this importance.
		int32 firstZoomLevel = INDEX_NONE;
		for (int32 zoom = 0; zoom < MaxImportanceByZoomLevel.Num(); zoom++)
		{
			if (pair.Key <= MaxImportanceByZoomLevel[zoom])
			{
				firstZoomLevel = zoom;
				break;
			}
		}

		if (MaxImportanceByZoomLevel.Num() == 0)
		{
			UE_LOG(LogTemp, Log, TEXT("UGlobeMarkerLayer:   importance %d: %d labels, always shown (no zoom table set)"), pair.Key, pair.Value);
		}
		else if (firstZoomLevel == INDEX_NONE)
		{
			UE_LOG(LogTemp, Warning, TEXT("UGlobeMarkerLayer:   importance %d: %d labels, NEVER shown at any zoom level"), pair.Key, pair.Value);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("UGlobeMarkerLayer:   importance %d: %d labels, shown from zoom level %d"), pair.Key, pair.Value, firstZoomLevel);
		}
	}
}

double UGlobeMarkerLayer::GetEffectiveRadius() const
{
	const AActor* globe = GlobeActor.Get();
	if (globe == nullptr)
	{
		return 0.0;
	}

	if (GlobeRadiusOverride > 0.0)
	{
		return GlobeRadiusOverride * globe->GetActorScale3D().GetMax();
	}

	if (const UStaticMeshComponent* mesh = globe->FindComponentByClass<UStaticMeshComponent>())
	{
		if (mesh->Bounds.SphereRadius > 0.0)
		{
			return mesh->Bounds.SphereRadius;	// already in world space
		}
	}

	// Procedural / dynamic mesh globes have no static mesh component, so fall back to the
	// largest renderable primitive on the actor.
	double best = 0.0;
	for (const UActorComponent* component : globe->GetComponents())
	{
		if (const UPrimitiveComponent* primitive = Cast<UPrimitiveComponent>(component))
		{
			best = FMath::Max(best, static_cast<double>(primitive->Bounds.SphereRadius));
		}
	}

	return best;
}

void UGlobeMarkerLayer::NativeTick(const FGeometry& myGeometry, float deltaTime)
{
	Super::NativeTick(myGeometry, deltaTime);

	APlayerController* playerController = GetOwningPlayer();
	if (playerController == nullptr || Markers.Num() == 0)
	{
		return;
	}

	// The globe actor and its radius belong to the Globe projection. The flat map has neither, and
	// requiring them there would make a map layer that silently never ticks.
	const bool bOnGlobe = (ProjectionMode == EGlobeMarkerProjection::Globe);

	// SetMapView() not called, or called with nothing in it. Worth saying, because the symptom is
	// every marker stacked on the origin rather than an empty screen -- which reads as a placement
	// bug rather than as a step that was never wired up.
	if (!bOnGlobe && MapEastPerDegree.IsNearlyZero() && MapNorthPerDegree.IsNearlyZero())
	{
		if (LastTally.Shown != 0)
		{
			LastTally = FTickTally();
			LastTally.Shown = 0;
			UE_LOG(LogTemp, Warning, TEXT("[%s] UGlobeMarkerLayer: ticking in FlatMap mode with no map view. Call SetMapView() from whatever positions the tiles, every frame the view moves."), *GetName());
		}
		return;
	}

	const AActor* globe = GlobeActor.Get();
	if (bOnGlobe && globe == nullptr)
	{
		if (LastTally.Shown != 0)
		{
			LastTally = FTickTally();
			LastTally.Shown = 0;
			UE_LOG(LogTemp, Warning, TEXT("[%s] UGlobeMarkerLayer: ticking in Globe mode with no globe actor, so nothing is drawn. Call SetGlobeActor() before the first tick, or SetProjectionMode(FlatMap)."), *GetName());
		}
		return;
	}

	const double radius = bOnGlobe ? GetEffectiveRadius() : 0.0;
	if (bOnGlobe && radius <= 0.0)
	{
		if (LastTally.Shown != 0)
		{
			LastTally = FTickTally();
			LastTally.Shown = 0;
			UE_LOG(LogTemp, Warning, TEXT("[%s] UGlobeMarkerLayer: globe actor '%s' gives a radius of 0, so nothing is drawn. Set GlobeRadiusOverride."), *GetName(), *globe->GetName());
		}
		return;
	}

	FVector cameraLocation;
	FRotator cameraRotation;
	playerController->GetPlayerViewPoint(cameraLocation, cameraRotation);

	const FTransform globeTransform = (globe != nullptr) ? globe->GetActorTransform() : FTransform::Identity;
	const FVector centre = globeTransform.GetLocation();
	const double cameraDistance = FVector::Dist(cameraLocation, centre);

	// Cosine of the horizon half-angle: markers whose surface normal falls below this are
	// on the far side of the globe and must be hidden.
	const double horizonCos = (bOnGlobe && cameraDistance > radius) ? (radius / cameraDistance) : -1.0;
	const FVector toCamera = (cameraLocation - centre).GetSafeNormal();

	const int32 maxImportance = GetMaxImportanceForZoomLevel(CurrentZoomLevel);

	FTickTally tally;
	tally.Shown = 0;
	tally.GatedByZoom = 0;
	tally.BehindGlobe = 0;
	tally.OffScreen = 0;
	tally.ZoomLevel = CurrentZoomLevel;

	const float viewportScale = UWidgetLayoutLibrary::GetViewportScale(this);
	FVector2D viewportSize = UWidgetLayoutLibrary::GetViewportSize(this);
	if (viewportScale > 0.0f)
	{
		viewportSize /= viewportScale;	// slate units, matching the projected positions below
	}

	for (FLiveMarker& marker : Markers)
	{
		marker.bOnScreen = false;

		if (marker.Widget == nullptr)
		{
			continue;
		}

		// Zoom gate first: cheapest rejection, and it hides the dot as well as the label.
		if (marker.Importance > maxImportance)
		{
			marker.Widget->SetVisibility(ESlateVisibility::Collapsed);
			tally.GatedByZoom++;
			continue;
		}

		FVector worldPosition;
		if (!GetMarkerWorldPosition(marker, globeTransform, radius, toCamera, horizonCos, worldPosition))
		{
			marker.Widget->SetVisibility(ESlateVisibility::Collapsed);
			tally.BehindGlobe++;
			continue;	// behind the globe
		}

		FVector2D screenPosition;
		if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(playerController, worldPosition, screenPosition, false))
		{
			marker.Widget->SetVisibility(ESlateVisibility::Collapsed);
			tally.OffScreen++;
			continue;	// behind the camera plane
		}

		if (screenPosition.X < -OffScreenMargin || screenPosition.Y < -OffScreenMargin
			|| screenPosition.X > viewportSize.X + OffScreenMargin || screenPosition.Y > viewportSize.Y + OffScreenMargin)
		{
			marker.Widget->SetVisibility(ESlateVisibility::Collapsed);
			tally.OffScreen++;
			continue;	// off screen
		}

		marker.ScreenPos = screenPosition;
		marker.CameraDistance = FVector::Dist(cameraLocation, worldPosition);
		marker.bOnScreen = true;

		marker.Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		marker.Widget->SetRenderTranslation(screenPosition);
		tally.Shown++;
	}

	if (tally != LastTally)
	{
		LastTally = tally;
		UE_LOG(LogTemp, Log, TEXT("[%s] UGlobeMarkerLayer: %d of %d shown at zoom level %d (importance ceiling %d); %d gated by zoom, %d behind the globe, %d off screen."),
			*GetName(), tally.Shown, Markers.Num(), CurrentZoomLevel, maxImportance,
			tally.GatedByZoom, tally.BehindGlobe, tally.OffScreen);
	}

	ApplyDeclutter();
	ApplyZOrder();
}

void UGlobeMarkerLayer::ApplyDeclutter()
{
	// The zoom gate in NativeTick has already decided which markers exist at all; this pass
	// only decides which of the survivors get to show their text.
	TArray<int32> candidates;
	candidates.Reserve(Markers.Num());
	for (int32 i = 0; i < Markers.Num(); i++)
	{
		if (Markers[i].bOnScreen)
		{
			candidates.Add(i);
		}
	}

	// Most important first, ties broken by proximity to the camera.
	candidates.Sort([this](int32 a, int32 b)
		{
			if (Markers[a].Importance != Markers[b].Importance)
			{
				return Markers[a].Importance < Markers[b].Importance;
			}
			return Markers[a].CameraDistance < Markers[b].CameraDistance;
		});

	TBitArray<> wantsLabel(false, Markers.Num());

	if (bDeclutterLabels)
	{
		TArray<FBox2D> accepted;
		accepted.Reserve(candidates.Num());

		for (int32 index : candidates)
		{
			const FVector2D extent = Markers[index].Widget->GetDesiredSize() * 0.5f + FVector2D(DeclutterPadding, DeclutterPadding);
			const FBox2D rect(Markers[index].ScreenPos - extent, Markers[index].ScreenPos + extent);

			bool blocked = false;
			for (const FBox2D& other : accepted)
			{
				if (rect.Intersect(other))
				{
					blocked = true;
					break;
				}
			}

			if (!blocked)
			{
				accepted.Add(rect);
				wantsLabel[index] = true;
			}
		}
	}
	else
	{
		for (int32 index : candidates)
		{
			wantsLabel[index] = true;
		}
	}

	for (int32 i = 0; i < Markers.Num(); i++)
	{
		const bool showLabel = wantsLabel[i];
		if (Markers[i].bLabelVisible != showLabel)
		{
			Markers[i].bLabelVisible = showLabel;
			Markers[i].Widget->SetLabelVisible(showLabel);	// the dot stays, only the text toggles
		}
	}
}

void UGlobeMarkerLayer::ApplyZOrder()
{
	TArray<int32> visible;
	visible.Reserve(Markers.Num());
	for (int32 i = 0; i < Markers.Num(); i++)
	{
		if (Markers[i].bOnScreen)
		{
			visible.Add(i);
		}
	}

	// Furthest first, so nearer markers paint on top.
	visible.Sort([this](int32 a, int32 b) { return Markers[a].CameraDistance > Markers[b].CameraDistance; });

	for (int32 rank = 0; rank < visible.Num(); rank++)
	{
		FLiveMarker& marker = Markers[visible[rank]];
		if (marker.CachedZOrder != rank && marker.Slot != nullptr)
		{
			marker.CachedZOrder = rank;
			marker.Slot->SetZOrder(rank);	// guarded: SetZOrder re-sorts the whole canvas
		}
	}
}
