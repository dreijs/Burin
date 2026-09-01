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
	MaxImportanceByZoomLevel = { 0, 1, 2, 3, 4, 5 };
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

void UGlobeMarkerLayer::InitializeLayer(AActor* globeActor, UBurinWorld* world, EGlobeLabelSource source)
{
	GlobeActor = globeActor;

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
		marker.Widget = widget;

		widget->OnMarkerInitialized(label.Name, marker.TextColor, label.Importance);
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

	const double radius = GetEffectiveRadius();
	if (!GlobeActor.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UGlobeMarkerLayer: no globe actor was passed to InitializeLayer. No markers will be drawn."));
	}
	else if (radius <= 0.0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGlobeMarkerLayer: globe actor '%s' has no renderable component with usable bounds. Set GlobeRadiusOverride. No markers will be drawn."), *GlobeActor->GetName());

		for (const UActorComponent* component : GlobeActor->GetComponents())
		{
			if (component != nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("UGlobeMarkerLayer:   '%s' is a %s"), *component->GetName(), *component->GetClass()->GetName());
			}
		}
	}

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
	const AActor* globe = GlobeActor.Get();
	if (playerController == nullptr || globe == nullptr || Markers.Num() == 0)
	{
		return;
	}

	const double radius = GetEffectiveRadius();
	if (radius <= 0.0)
	{
		return;
	}

	FVector cameraLocation;
	FRotator cameraRotation;
	playerController->GetPlayerViewPoint(cameraLocation, cameraRotation);

	const FTransform globeTransform = globe->GetActorTransform();
	const FVector centre = globeTransform.GetLocation();
	const double cameraDistance = FVector::Dist(cameraLocation, centre);

	// Cosine of the horizon half-angle: markers whose surface normal falls below this are
	// on the far side of the globe and must be hidden.
	const double horizonCos = (cameraDistance > radius) ? (radius / cameraDistance) : -1.0;
	const FVector toCamera = (cameraLocation - centre).GetSafeNormal();

	const int32 maxImportance = GetMaxImportanceForZoomLevel(CurrentZoomLevel);

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
			continue;
		}

		const FVector worldDirection = globeTransform.TransformVectorNoScale(marker.LocalDirection);
		if (FVector::DotProduct(worldDirection, toCamera) <= horizonCos)
		{
			marker.Widget->SetVisibility(ESlateVisibility::Collapsed);
			continue;	// behind the globe
		}

		const FVector worldPosition = centre + worldDirection * radius;

		FVector2D screenPosition;
		if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(playerController, worldPosition, screenPosition, false))
		{
			marker.Widget->SetVisibility(ESlateVisibility::Collapsed);
			continue;	// behind the camera plane
		}

		if (screenPosition.X < -OffScreenMargin || screenPosition.Y < -OffScreenMargin
			|| screenPosition.X > viewportSize.X + OffScreenMargin || screenPosition.Y > viewportSize.Y + OffScreenMargin)
		{
			marker.Widget->SetVisibility(ESlateVisibility::Collapsed);
			continue;	// off screen
		}

		marker.ScreenPos = screenPosition;
		marker.CameraDistance = FVector::Dist(cameraLocation, worldPosition);
		marker.bOnScreen = true;

		marker.Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		marker.Widget->SetRenderTranslation(screenPosition);
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
