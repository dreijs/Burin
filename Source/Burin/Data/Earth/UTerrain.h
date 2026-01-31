// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Burin/UBurinWorld.h>

/**
 * 
 */
class BURIN_API UTerrain 
{
public:

	static const TArray<int> ELEVATION_N1;
	static const TArray<int> ELEVATION_0;
	static const TArray<int> ELEVATION_1;
	static const TArray<int> ELEVATION_2;
	static const TArray<int> ELEVATION_3;
	static const TArray<int> ELEVATION_4;
	static const TArray<int> ELEVATION_5;
	static const TArray<int> ELEVATION_6;
	static const TArray<int> ELEVATION_7;
	static const TArray<int> ELEVATION_8;
	static const TArray<int> ELEVATION_9;
	static const TArray<int> ELEVATION_10;
	static const TArray<int> ELEVATION_11;
	static const TArray<int> ELEVATION_12;
	static const TArray<int> ELEVATION_13;
	static const TArray<int> ELEVATION_14;

	static const TArray<int> TUNDRA_COLOR;
	static const TArray<int> TAIGA;
	static const TArray<int> TEMPERATE;
	static const TArray<int> STEPPE_COLOR;
	static const TArray<int> WET_SUBTROPICAL;
	static const TArray<int> MEDITERRANEAN;
	static const TArray<int> MONSOON;
	static const TArray<int> ARID;
	static const TArray<int> XERIC;
	static const TArray<int> DRY_STEPPE_COLOR;
	static const TArray<int> SEMIARID;
	static const TArray<int> GRASS_SAVANNA_COLOR;
	static const TArray<int> TREE_SAVANNA_COLOR;
	static const TArray<int> DRY_SUBTROPICAL;
	static const TArray<int> TROPICAL_RAINFOREST_COLOR;
	static const TArray<int> GLACIAL;

	static const TArray<int> ALFISOL;
	static const TArray<int> ANDISOL;
	static const TArray<int> ARIDISOL;
	static const TArray<int> ENTISOL;
	static const TArray<int> GELISOL;
	static const TArray<int> HISTOSOL;
	static const TArray<int> INCEPTISOL;
	static const TArray<int> MOLLISOL;
	static const TArray<int> OXISOL;
	static const TArray<int> SPODOSOL;
	static const TArray<int> ULTISOL;
	static const TArray<int> VERTISOL;
	static const TArray<int> ROCKY;
	static const TArray<int> SHIFTING_SAND;

	static const int POLAR_DESERT;
	static const int TUNDRA;
	static const int ALPINE_TUNDRA;
	static const int VOLCANIC_TUNDRA;
	static const int BOREAL_FOREST;
	static const int SPARSE_BOREAL_FOREST;
	static const int DENSE_BOREAL_FOREST;
	static const int SUBARCTIC_BOG;
	static const int BOREAL_SANDY_PLAINS;
	static const int TEMPERATE_PINE_FOREST;
	static const int TEMPERATE_FOREST;
	static const int SPARSE_TEMPERATE_FOREST;
	static const int DENSE_TEMPERATE_FOREST;
	static const int TEMPERATE_GRASSLAND;
	static const int TEMPERATE_MUDLAND;
	static const int FERTILE_MEADOWS;
	static const int TEMPERATE_BOG;
	static const int VOLCANIC_PLAINS;
	static const int FERTILE_PLAINS;
	static const int STEPPE;
	static const int TEMPERATE_SANDY_PLAINS;
	static const int SUBTROPICAL_RAINFOREST;
	static const int SPARSE_SUBTROPICAL_RAINFOREST;
	static const int DENSE_SUBTROPICAL_RAINFOREST;
	static const int SUBTROPICAL_GRASSLAND;
	static const int WET_SUBTROPICAL_BOG;
	static const int WET_SUBTROPICAL_MUDLAND;
	static const int SUBTROPICAL_SANDY_PLAINS;
	static const int VOLCANIC_FOREST;
	static const int MEDITERRANEAN_FOREST;
	static const int SPARSE_MEDITERRANEAN_FOREST;
	static const int DENSE_MEDITERRANEAN_FOREST;
	static const int MEDITERRANEAN_PINE_FOREST;
	static const int MEDITERRANEAN_SHRUBLAND;
	static const int MEDITERRANEAN_PLAINS;
	static const int VOLCANIC_SHRUBLAND;
	static const int DRY_SUBTROPICAL_BOG;
	static const int DRY_SUBTROPICAL_MUDLAND;
	static const int MONSOON_FOREST;
	static const int SPARSE_MONSOON_FOREST;
	static const int DENSE_MONSOON_FOREST;
	static const int MUD_DESERT;
	static const int SAND_DESERT;
	static const int MUDDY_XERIC_SHRUBLAND;
	static const int SANDY_XERIC_SHRUBLAND;
	static const int SEMIARID_DESERT;
	static const int SEMIARID_BUSHLAND;
	static const int SEMIARID_MUDLAND;
	static const int VOLCANIC_DESERT;
	static const int DRY_STEPPE;
	static const int DRY_SHRUBLAND;
	static const int DRY_SANDY_PLAINS;
	static const int DRY_FERTILE_PLAINS;
	static const int GRASS_SAVANNA;
	static const int SPARSE_TREE_SAVANNA;
	static const int TREE_SAVANNA;
	static const int VOLCANIC_SAVANNA;
	static const int DENSE_TREE_SAVANNA;
	static const int DRY_TROPICAL_BOG;
	static const int DRY_TROPICAL_MUDLAND;
	static const int DRY_SUBTROPICAL_FOREST;
	static const int SPARSE_DRY_SUBTROPICAL_FOREST;
	static const int DENSE_DRY_SUBTROPICAL_FOREST;
	static const int TROPICAL_RAINFOREST;
	static const int SPARSE_TROPICAL_RAINFOREST;
	static const int DENSE_TROPICAL_RAINFOREST;
	static const int WET_TROPICAL_GRASSLAND;
	static const int WET_TROPICAL_BOG;
	static const int WET_TROPICAL_MUDLAND;
	static const int VOLCANIC_RAINFOREST;
	static const int TROPICAL_SANDY_PLAINS;
	static const int ROCKLANDS;
	static const int SHIFTING_SANDS;
	static const int GLACIAL_PLAINS;
	static const int POLAR_ICE;

	UTerrain();
	~UTerrain();

	static TArray<int> GetColor(UBurinWorld* world, int i, int mode);
};
