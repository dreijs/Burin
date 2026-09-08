// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "UGlobeMarkerWidget.generated.h"

/** Which dataset on UBurinWorld a layer draws its markers from. */
UENUM(BlueprintType)
enum class EGlobeLabelSource : uint8
{
	/** Terrain->GeographicLabelData: continents, deserts, mountain ranges. */
	GeographicFeatures,

	/** World->Places: settlements that exist as of UBurinWorld::CurrentYear. */
	HistoricalPlaces
};

/**
 * One dot + label pinned to a geographic coordinate. Positioned every frame by UGlobeMarkerLayer,
 * so it must never move itself. The Blueprint subclass owns the visuals only.
 *
 * Build the Blueprint so the dot sits at the exact centre of the widget: the layer aligns the
 * widget's centre with the projected screen position.
 */
UCLASS(Abstract, Blueprintable, meta = (DisableNativeTick))
class BURIN_API UGlobeMarkerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Called once when the layer builds this marker. Apply the name and the two colours here.
	 *
	 * source says which dataset this marker came from, so the Blueprint can look different for each
	 * without a second widget class -- a named place earns a dot marking a point, while a region
	 * label like "Iberia" names an area that has no point to mark, and a dot on it is a claim about
	 * a location the data does not make.
	 *
	 * markerColor tints the dot, textColor the label; they are separate because they answer to
	 * different things. The label is coloured for legibility against the globe, while the dot says
	 * who holds the place -- its polity's mapcolor2, or white where nothing owns it. Tinting
	 * multiplies, so a white-with-black-outline dot keeps its outline and takes the colour in the
	 * middle, which is the whole reason the artwork is drawn that way.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "GlobeLabels")
	void OnMarkerInitialized(const FString& name, FLinearColor textColor, FLinearColor markerColor, int32 importance, EGlobeLabelSource source);

	/** Called when the declutter/zoom rules change. Toggle the text only; leave the dot visible. */
	UFUNCTION(BlueprintImplementableEvent, Category = "GlobeLabels")
	void SetLabelVisible(bool labelVisible);
};
