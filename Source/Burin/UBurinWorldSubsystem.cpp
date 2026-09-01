// Fill out your copyright notice in the Description page of Project Settings.

#include "UBurinWorldSubsystem.h"
#include "UBurinWorld.h"

void UBurinWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	BurinWorld = NewObject<UBurinWorld>(this);
}
