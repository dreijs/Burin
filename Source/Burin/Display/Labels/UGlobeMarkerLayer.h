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

/** How a marker's latitude and longitude become a position in the world. */
UENUM(BlueprintType)
enum class EGlobeMarkerProjection : uint8
{
	/** On the sphere: a direction from its centre, times the radius. Hidden past the horizon. */
	Globe,

	/** On the flat high-zoom map: an offset east and north of the view's centre coordinate. */
	FlatMap
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
	 *
	 * Takes no globe actor: which markers exist has nothing to do with what they are drawn on, and
	 * bundling the two meant a layer could not be built before a sphere existed -- or at all, for a
	 * view that has no sphere. Hand the geometry over separately, with SetGlobeActor() or
	 * SetMapView() depending on the projection.
	 */
	UFUNCTION(BlueprintCallable, Category = "GlobeLabels")
	void InitializeLayer(UBurinWorld* world, EGlobeLabelSource source);

	/**
	 * The sphere the Globe projection places markers on, and whose transform and bounds it reads
	 * every frame. What SetMapView() is to the flat map: the owning representation supplies its own
	 * geometry, when it has it.
	 */
	UFUNCTION(BlueprintCallable, Category = "GlobeLabels")
	void SetGlobeActor(AActor* globeActor);

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

	/**
	 * Which projection places the markers. Switch it when the view switches between the sphere and
	 * the flat map; the marker set itself does not change, only where each one lands.
	 */
	UFUNCTION(BlueprintCallable, Category = "GlobeLabels")
	void SetProjectionMode(EGlobeMarkerProjection projection);

	/**
	 * The flat map's current view, in the map's own terms. Call it every frame the view moves, from
	 * the same numbers the tiles are placed with.
	 *
	 * A marker lands at mapOrigin + eastPerDegree * (its longitude - centreLongitude)
	 *                             + northPerDegree * (its latitude  - centreLatitude),
	 * with the longitude difference wrapped to +-180 so a view across the antimeridian keeps its
	 * markers beside it rather than a map away.
	 *
	 * The two axis vectors are supplied rather than derived because the map's plane, its handedness
	 * and its units-per-degree all live in the Blueprint that positions the tiles. Feed them from
	 * exactly the values MoveTile uses and the markers cannot drift from the terrain under them --
	 * including the cosine narrowing of longitude, which belongs in eastPerDegree.
	 */
	UFUNCTION(BlueprintCallable, Category = "GlobeLabels")
	void SetMapView(double centreLatitude, double centreLongitude, FVector mapOrigin, FVector eastPerDegree, FVector northPerDegree);

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

		// The dot's tint, as distinct from Color, which is the label's. For a historical place this
		// is its controlling polity's mapcolor2; white where nothing owns it, so the artwork shows
		// through unchanged.
		FLinearColor MarkerColor = FLinearColor::White;
	};

	struct FLiveMarker
	{
		// Snapshot taken from the source data at build time; the source array is never held.
		FLinearColor TextColor = FLinearColor::White;
		int32 Importance = 0;

		// Both forms of the same coordinate, decided once at build time. The globe wants a direction
		// and the flat map wants degrees; converting either way per marker per frame would be work
		// done to recover something that was already known.
		FVector LocalDirection = FVector::ZeroVector;
		double Latitude = 0.0;
		double Longitude = 0.0;

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

	EGlobeMarkerProjection ProjectionMode = EGlobeMarkerProjection::Globe;

	// Which dataset this layer was built from, kept so each marker can be told at creation.
	EGlobeLabelSource LayerSource = EGlobeLabelSource::GeographicFeatures;

	// The flat map's view, as last given to SetMapView(). Meaningless in Globe mode.
	double MapCentreLatitude = 0.0;
	double MapCentreLongitude = 0.0;
	FVector MapOrigin = FVector::ZeroVector;
	FVector MapEastPerDegree = FVector::ZeroVector;
	FVector MapNorthPerDegree = FVector::ZeroVector;

	// Why each marker did or did not appear on the last tick, logged whenever the tally changes.
	// A layer showing nothing has half a dozen equally silent causes -- no globe actor, a zoom gate
	// nobody pushes a level into, a projection mode that does not match what is on screen -- and
	// they are indistinguishable from the outside. This says which one it is.
	struct FTickTally
	{
		int32 Shown = -1;
		int32 GatedByZoom = -1;
		int32 BehindGlobe = -1;
		int32 OffScreen = -1;
		int32 ZoomLevel = -1;

		bool operator!=(const FTickTally& other) const
		{
			return Shown != other.Shown || GatedByZoom != other.GatedByZoom
				|| BehindGlobe != other.BehindGlobe || OffScreen != other.OffScreen
				|| ZoomLevel != other.ZoomLevel;
		}
	};
	FTickTally LastTally;

	// Where this marker sits in the world right now, and whether it is on the visible side of
	// whatever it is drawn on. False in Globe mode for a marker past the horizon.
	bool GetMarkerWorldPosition(const FLiveMarker& marker, const FTransform& globeTransform, double radius,
		const FVector& toCamera, double horizonCos, FVector& outWorldPosition) const;

	void BuildMarkers(const TArray<FMarkerSeed>& seeds);
	double GetEffectiveRadius() const;
	void ApplyDeclutter();
	void ApplyZOrder();
};
