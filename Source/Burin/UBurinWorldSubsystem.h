// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UBurinWorldSubsystem.generated.h"

class UBurinWorld;

/**
 * Owns the single UBurinWorld instance for the game session, so it's reachable from any
 * Blueprint or C++ code via GetGameInstance()->GetSubsystem<UBurinWorldSubsystem>() (or the
 * "Get Game Instance Subsystem" Blueprint node) instead of threading a reference through the
 * player pawn/controller.
 */
UCLASS()
class BURIN_API UBurinWorldSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Owned here for its whole lifetime; callers still need to call BurinWorld->Initialize()
	// (and InitializeProvinces()/SetCurrentYear()) themselves at whatever point they currently do.
	UFUNCTION(BlueprintPure, Category = "Burin")
	UBurinWorld* GetBurinWorld() const { return BurinWorld; }

private:
	UPROPERTY()
	TObjectPtr<UBurinWorld> BurinWorld;
};
