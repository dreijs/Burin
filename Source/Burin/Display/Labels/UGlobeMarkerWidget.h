// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "UGlobeMarkerWidget.generated.h"

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
	/** Called once when the layer builds this marker. Apply the name and text colour here. */
	UFUNCTION(BlueprintImplementableEvent, Category = "GlobeLabels")
	void OnMarkerInitialized(const FString& name, FLinearColor textColor, int32 importance);

	/** Called when the declutter/zoom rules change. Toggle the text only; leave the dot visible. */
	UFUNCTION(BlueprintImplementableEvent, Category = "GlobeLabels")
	void SetLabelVisible(bool labelVisible);
};
