// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UGlobeMarkerWidget.h"

#include "UGlobeMarkerLayer.generated.h"

class AActor;
class UBurinWorld;
class UCanvasPanel;
class UCanvasPanelSlot;

/** Which dataset on UBurinWorld a layer draws its markers from. */
UENUM(BlueprintType)
enum class EGlobeLabelSource : uint8
{
	/** Terrain->GeographicLabelData: continents, deserts, mountain ranges. */
	GeographicFeatures,

	/** HistoricalPlaces->HistoricalPlaceData: ancient cities and settlements. */
	HistoricalPlaces
};

/**
 * Screen-space overlay that pins a marker widget to every geographic label on the globe.
 *
 * The markers live in a UMG canvas rather than on the sphere, so they keep a constant screen
 * size no matter how far the camera zooms. Every frame each label's world position is projected
 * to screen space and its widget is moved there; labels on the far side of the globe are hidden
 * by a horizon test.
 *
 * The Blueprint subclass must contain a Canvas Panel named exactly "MarkerCanvas".
 */
UCLASS(Abstract, Blueprintable)
class BURIN_API UGlobeMarkerLayer : public UUserWidget
{
	GENERATED_BODY()

public:
	UGlobeMarkerLayer(const FObjectInitializer& objectInitializer);

	/**
	 * Builds the marker set from the chosen dataset on the world.
	 * Safe to call again to rebuild after the source data changes.
	 */
	UFUNCTION(BlueprintCallable, Category = "GlobeLabels")
	void InitializeLayer(AActor* globeActor, UBurinWorld* world, EGlobeLabelSource source);

	/** Colour used for sources that carry no colour of their own, such as historical places. */
	UPROPERTY(EditDefaultsOnly, Category = "GlobeLabels|Data")
	FLinearColor DefaultLabelColor = FLinearColor::White;

	/** Importance assigned to every historical place, which has no importance of its own yet. */
	UPROPERTY(EditDefaultsOnly, Category = "GlobeLabels|Data")
	int32 PlaceImportance = 1;

	/** Marker Blueprint spawned for every label. Required. */
	UPROPERTY(EditDefaultsOnly, Category = "GlobeLabels|Data")
	TSubclassOf<UGlobeMarkerWidget> MarkerWidgetClass;

	/**
	 * Hard cap on the number of marker widgets created, keeping the highest Importance first.
	 * 0 means no cap. Raise or lower this if the label file is large.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "GlobeLabels|Data", meta = (ClampMin = "0"))
	int32 MaxMarkers = 0;

	/**
	 * Sphere radius in world units at actor scale 1; the actor's largest scale component is applied
	 * on top. Leave at 0 to derive the radius from the globe actor's mesh bounds instead.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "GlobeLabels|Geometry")
	double GlobeRadiusOverride = 0.0;

	/** Rotates all markers around the polar axis to line up with the sphere's texture. */
	UPROPERTY(EditDefaultsOnly, Category = "GlobeLabels|Geometry")
	double LongitudeOffsetDeg = 0.0;

	/** Set when the sphere's texture runs east-west the other way. */
	UPROPERTY(EditDefaultsOnly, Category = "GlobeLabels|Geometry")
	bool bFlipLongitude = false;

	/** Set when the sphere's texture runs north-south the other way. */
	UPROPERTY(EditDefaultsOnly, Category = "GlobeLabels|Geometry")
	bool bFlipLatitude = false;

	/** Markers further than this many slate units outside the viewport are hidden. */
	UPROPERTY(EditDefaultsOnly, Category = "GlobeLabels|Geometry", meta = (ClampMin = "0.0"))
	float OffScreenMargin = 128.0f;

	/**
	 * Indexed by zoom level counter: the highest importance value still displayed at that zoom.
	 * A marker is shown when Importance <= MaxImportanceByZoomLevel[zoomLevel], so LOWER
	 * importance numbers are the more important ones and appear first as you zoom in.
	 *
	 * e.g. { -1, 10, 20 } means: nothing at zoom 0, importance 0..10 at zoom 1, 0..20 at zoom 2.
	 * Zoom levels past the end of the array clamp to the last entry; an empty array disables
	 * the gate entirely and shows every marker.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "GlobeLabels|Zoom")
	TArray<int32> MaxImportanceByZoomLevel;

	/** Push your zoom level counter in here whenever it changes. */
	UFUNCTION(BlueprintCallable, Category = "GlobeLabels")
	void SetZoomLevel(int32 zoomLevel);

	UFUNCTION(BlueprintPure, Category = "GlobeLabels")
	int32 GetZoomLevel() const { return CurrentZoomLevel; }

	/** Looks up the table. Exposed so you can sanity-check it from Blueprint. */
	UFUNCTION(BlueprintPure, Category = "GlobeLabels")
	int32 GetMaxImportanceForZoomLevel(int32 zoomLevel) const;

	/** Extra padding in slate units around a label when testing it for overlap. */
	UPROPERTY(EditDefaultsOnly, Category = "GlobeLabels|Declutter")
	float DeclutterPadding = 6.0f;

	/** Turn off to show every eligible label regardless of overlap. */
	UPROPERTY(EditDefaultsOnly, Category = "GlobeLabels|Declutter")
	bool bDeclutterLabels = true;

protected:
	virtual void NativeTick(const FGeometry& myGeometry, float deltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> MarkerCanvas;

	/** Keeps the spawned markers referenced for the garbage collector. Index-aligned with Markers. */
	UPROPERTY()
	TArray<TObjectPtr<UGlobeMarkerWidget>> MarkerWidgets;

private:
	/** Source-agnostic description of one marker, so the build path doesn't care where it came from. */
	struct FMarkerSeed
	{
		FString Name;
		double Latitude = 0.0;
		double Longitude = 0.0;
		int32 Importance = 0;
		FLinearColor Color = FLinearColor::White;
	};

	struct FLiveMarker
	{
		// Snapshot taken from the source data at build time; the source array is never held.
		FLinearColor TextColor = FLinearColor::White;
		int32 Importance = 0;
		FVector LocalDirection = FVector::ZeroVector;

		// Per-frame state.
		UGlobeMarkerWidget* Widget = nullptr;
		UCanvasPanelSlot* Slot = nullptr;
		FVector2D ScreenPos = FVector2D::ZeroVector;
		double CameraDistance = 0.0;
		bool bOnScreen = false;
		bool bLabelVisible = false;
		int32 CachedZOrder = -1;
	};

	TArray<FLiveMarker> Markers;
	TWeakObjectPtr<AActor> GlobeActor;
	int32 CurrentZoomLevel = 0;

	void BuildMarkers(const TArray<FMarkerSeed>& seeds);
	double GetEffectiveRadius() const;
	void ApplyDeclutter();
	void ApplyZOrder();
};
