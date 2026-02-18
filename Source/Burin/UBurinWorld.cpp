// Fill out your copyright notice in the Description page of Project Settings.


#include "UBurinWorld.h"
#include "Data/Earth/UTerrainDataEntry.h"
#include "Data/Earth/UTerrain.h"

UBurinWorld::UBurinWorld()
{
}

UBurinWorld::~UBurinWorld()
{
}

UTerrain* Terrain;

void UBurinWorld::InitializeTerrain()
{
	Terrain = new UTerrain();

	Terrain->InitializeElevation();
	Terrain->InitializeVegetation();
	Terrain->InitializeSoil();
	Terrain->InitializeFeatures();
	Terrain->InitializeTerrain();

	Terrain->InitializeTerrainMapping();
}

void UBurinWorld::Initialize()
{
	InitializeTerrain();
}