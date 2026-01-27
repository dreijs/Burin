// Fill out your copyright notice in the Description page of Project Settings.

#include "UTerrain.h"
#include <Burin/UBurinWorld.h>

const TArray<int> UTerrain::ELEVATION_N1 = { 5, 98, 155 };
const TArray<int> UTerrain::ELEVATION_0 = { 0, 136, 0 };
const TArray<int> UTerrain::ELEVATION_1 = { 0, 187, 0 };
const TArray<int> UTerrain::ELEVATION_2 = { 0, 238, 0 };
const TArray<int> UTerrain::ELEVATION_3 = { 136, 255, 0 };
const TArray<int> UTerrain::ELEVATION_4 = { 255, 255, 0 };
const TArray<int> UTerrain::ELEVATION_5 = { 221, 187, 34 };
const TArray<int> UTerrain::ELEVATION_6 = { 187, 119, 68 };
const TArray<int> UTerrain::ELEVATION_7 = { 170, 102, 51 };
const TArray<int> UTerrain::ELEVATION_8 = { 153, 85, 34 };
const TArray<int> UTerrain::ELEVATION_9 = { 136, 68, 17 };
const TArray<int> UTerrain::ELEVATION_10 = { 119, 51, 0 };
const TArray<int> UTerrain::ELEVATION_11 = { 102, 34, 0 };
const TArray<int> UTerrain::ELEVATION_12 = { 85, 17, 0 };

const TArray<int> UTerrain::TUNDRA_COLOR = { 140, 204, 189 };
const TArray<int> UTerrain::TAIGA = { 0, 87, 78 };
const TArray<int> UTerrain::TEMPERATE = { 146, 216, 71 };
const TArray<int> UTerrain::STEPPE_COLOR = { 245, 231, 89 };
const TArray<int> UTerrain::WET_SUBTROPICAL = { 6, 104, 6 };
const TArray<int> UTerrain::MEDITERRANEAN = { 124, 96, 134 };
const TArray<int> UTerrain::MONSOON = { 89, 129, 89 };
const TArray<int> UTerrain::ARID = { 129, 66, 41 };
const TArray<int> UTerrain::XERIC = { 170, 95, 61 };
const TArray<int> UTerrain::DRY_STEPPE_COLOR = { 136, 111, 51 };
const TArray<int> UTerrain::SEMIARID = { 214, 169, 114 };
const TArray<int> UTerrain::GRASS_SAVANNA_COLOR = { 193, 189, 62 };
const TArray<int> UTerrain::TREE_SAVANNA_COLOR = { 155, 149, 14 };
const TArray<int> UTerrain::DRY_SUBTROPICAL = { 96, 122, 34 };
const TArray<int> UTerrain::TROPICAL_RAINFOREST_COLOR = { 0, 70, 0 };
const TArray<int> UTerrain::GLACIAL = { 178, 178, 178 };

const TArray<int> UTerrain::ALFISOL = { 175, 206, 99 };
const TArray<int> UTerrain::ANDISOL = { 150, 76, 141 };
const TArray<int> UTerrain::ARIDISOL = { 255, 234, 183 };
const TArray<int> UTerrain::ENTISOL = { 158, 207, 186 };
const TArray<int> UTerrain::GELISOL = { 146, 175, 213 };
const TArray<int> UTerrain::HISTOSOL = { 132, 71, 67 };
const TArray<int> UTerrain::INCEPTISOL = { 247, 163, 44 };
const TArray<int> UTerrain::MOLLISOL = { 71, 156, 61 };
const TArray<int> UTerrain::OXISOL = { 239, 123, 120 };
const TArray<int> UTerrain::SPODOSOL = { 199, 156, 193 };
const TArray<int> UTerrain::ULTISOL = { 245, 233, 41 };
const TArray<int> UTerrain::VERTISOL = { 80, 90, 150 };
const TArray<int> UTerrain::ROCKY = { 209, 201, 194 };
const TArray<int> UTerrain::SHIFTING_SAND = { 134, 125, 118 };

const int UTerrain::POLAR_DESERT = 0;
const int UTerrain::TUNDRA = 1;
const int UTerrain::ALPINE_TUNDRA = 2;
const int UTerrain::VOLCANIC_TUNDRA = 3;
const int UTerrain::BOREAL_FOREST = 4;
const int UTerrain::SPARSE_BOREAL_FOREST = 5;
const int UTerrain::DENSE_BOREAL_FOREST = 6;
const int UTerrain::SUBARCTIC_BOG = 7;
const int UTerrain::BOREAL_SANDY_PLAINS = 8;
const int UTerrain::TEMPERATE_PINE_FOREST = 9;
const int UTerrain::TEMPERATE_FOREST = 10;
const int UTerrain::SPARSE_TEMPERATE_FOREST = 11;
const int UTerrain::DENSE_TEMPERATE_FOREST = 12;
const int UTerrain::TEMPERATE_GRASSLAND = 13;
const int UTerrain::TEMPERATE_MUDLAND = 14;
const int UTerrain::FERTILE_MEADOWS = 15;
const int UTerrain::TEMPERATE_BOG = 16;
const int UTerrain::VOLCANIC_PLAINS = 17;
const int UTerrain::FERTILE_PLAINS = 18;
const int UTerrain::STEPPE = 19;
const int UTerrain::TEMPERATE_SANDY_PLAINS = 20;
const int UTerrain::SUBTROPICAL_RAINFOREST = 21;
const int UTerrain::SPARSE_SUBTROPICAL_RAINFOREST = 22;
const int UTerrain::DENSE_SUBTROPICAL_RAINFOREST = 23;
const int UTerrain::SUBTROPICAL_GRASSLAND = 24;
const int UTerrain::WET_SUBTROPICAL_BOG = 25;
const int UTerrain::WET_SUBTROPICAL_MUDLAND = 26;
const int UTerrain::SUBTROPICAL_SANDY_PLAINS = 27;
const int UTerrain::VOLCANIC_FOREST = 28;
const int UTerrain::MEDITERRANEAN_FOREST = 29;
const int UTerrain::SPARSE_MEDITERRANEAN_FOREST = 30;
const int UTerrain::DENSE_MEDITERRANEAN_FOREST = 31;
const int UTerrain::MEDITERRANEAN_PINE_FOREST = 32;
const int UTerrain::MEDITERRANEAN_SHRUBLAND = 33;
const int UTerrain::MEDITERRANEAN_PLAINS = 34;
const int UTerrain::VOLCANIC_SHRUBLAND = 35;
const int UTerrain::DRY_SUBTROPICAL_BOG = 36;
const int UTerrain::DRY_SUBTROPICAL_MUDLAND = 37;
const int UTerrain::MONSOON_FOREST = 38;
const int UTerrain::SPARSE_MONSOON_FOREST = 39;
const int UTerrain::DENSE_MONSOON_FOREST = 40;
const int UTerrain::MUD_DESERT = 41;
const int UTerrain::SAND_DESERT = 42;
const int UTerrain::MUDDY_XERIC_SHRUBLAND = 43;
const int UTerrain::SANDY_XERIC_SHRUBLAND = 44;
const int UTerrain::SEMIARID_DESERT = 45;
const int UTerrain::SEMIARID_BUSHLAND = 46;
const int UTerrain::SEMIARID_MUDLAND = 47;
const int UTerrain::VOLCANIC_DESERT = 48;
const int UTerrain::DRY_STEPPE = 49;
const int UTerrain::DRY_SHRUBLAND = 50;
const int UTerrain::DRY_SANDY_PLAINS = 51;
const int UTerrain::DRY_FERTILE_PLAINS = 52;
const int UTerrain::GRASS_SAVANNA = 53;
const int UTerrain::SPARSE_TREE_SAVANNA = 54;
const int UTerrain::TREE_SAVANNA = 55;
const int UTerrain::VOLCANIC_SAVANNA = 56;
const int UTerrain::DENSE_TREE_SAVANNA = 57;
const int UTerrain::DRY_TROPICAL_BOG = 58;
const int UTerrain::DRY_TROPICAL_MUDLAND = 59;
const int UTerrain::DRY_SUBTROPICAL_FOREST = 60;
const int UTerrain::SPARSE_DRY_SUBTROPICAL_FOREST = 61;
const int UTerrain::DENSE_DRY_SUBTROPICAL_FOREST = 62;
const int UTerrain::TROPICAL_RAINFOREST = 63;
const int UTerrain::SPARSE_TROPICAL_RAINFOREST = 64;
const int UTerrain::DENSE_TROPICAL_RAINFOREST = 65;
const int UTerrain::WET_TROPICAL_GRASSLAND = 66;
const int UTerrain::WET_TROPICAL_BOG = 67;
const int UTerrain::WET_TROPICAL_MUDLAND = 68;
const int UTerrain::VOLCANIC_RAINFOREST = 69;
const int UTerrain::TROPICAL_SANDY_PLAINS = 70;
const int UTerrain::ROCKLANDS = 71;
const int UTerrain::SHIFTING_SANDS = 72;

UTerrain::UTerrain(){

}

UTerrain::~UTerrain() {

}

TArray<int> UTerrain::GetColor(UBurinWorld* world, int i, int mode) {
	int elevation = i % 16;
	int biome = (i / 16) % 16;
	int soil = (i / 256) % 16;

	if (mode == 1) {
		if (elevation > 12) return UTerrain::ELEVATION_12;
		if (elevation > 11) return UTerrain::ELEVATION_11;
		if (elevation > 10) return UTerrain::ELEVATION_10;
		if (elevation > 9) return UTerrain::ELEVATION_9;
		if (elevation > 8) return UTerrain::ELEVATION_8;
		if (elevation > 7) return UTerrain::ELEVATION_7;
		if (elevation > 6) return UTerrain::ELEVATION_6;
		if (elevation > 5) return UTerrain::ELEVATION_5;
		if (elevation > 4) return UTerrain::ELEVATION_4;
		if (elevation > 3) return UTerrain::ELEVATION_3;
		if (elevation > 2) return UTerrain::ELEVATION_2;
		if (elevation > 1) return UTerrain::ELEVATION_1;
		if (elevation > 0) return UTerrain::ELEVATION_0;
			return ELEVATION_N1;
	}
	if(mode == 2) {
		if(elevation == 0) return ELEVATION_N1;
		if (biome == 0) return UTerrain::TUNDRA_COLOR;
		if (biome == 1) return UTerrain::TAIGA;
		if (biome == 2) return UTerrain::TEMPERATE;
		if (biome == 3) return UTerrain::STEPPE_COLOR;
		if (biome == 4) return UTerrain::WET_SUBTROPICAL;
		if (biome == 5) return UTerrain::MEDITERRANEAN;
		if (biome == 6) return UTerrain::MONSOON;
		if (biome == 7) return UTerrain::ARID;
		if (biome == 8) return UTerrain::XERIC;
		if (biome == 9) return UTerrain::DRY_STEPPE_COLOR;
		if (biome == 10) return UTerrain::SEMIARID;
		if (biome == 11) return UTerrain::GRASS_SAVANNA_COLOR;
		if (biome == 12) return UTerrain::TREE_SAVANNA_COLOR;
		if (biome == 13) return UTerrain::DRY_SUBTROPICAL;
		if (biome == 14) return UTerrain::TROPICAL_RAINFOREST_COLOR;
		if (biome == 15) return UTerrain::GLACIAL;
	}
	if (mode == 3) {
		if (elevation == 0) return ELEVATION_N1;
		if (soil == 0) return UTerrain::ALFISOL;
		if (soil == 1) return UTerrain::ANDISOL;
		if (soil == 2) return UTerrain::ARIDISOL;
		if (soil == 3) return UTerrain::ENTISOL;
		if (soil == 4) return UTerrain::GELISOL;
		if (soil == 5) return UTerrain::HISTOSOL;
		if (soil == 6) return UTerrain::INCEPTISOL;
		if (soil == 7) return UTerrain::MOLLISOL;
		if (soil == 8) return UTerrain::OXISOL;
		if (soil == 9) return UTerrain::SPODOSOL;
		if (soil == 10) return UTerrain::ULTISOL;
		if (soil == 11) return UTerrain::VERTISOL;
		if (soil == 12) return UTerrain::ROCKY;
		if (soil == 13) return UTerrain::SHIFTING_SAND;
	}
	if (mode == 0) {
		if (elevation == 0 && soil == 14) return {255, 255, 255};
		if (soil == 14) return { 210, 210, 210 };
		if (elevation == 0) return { 10, 17, 35 };
		if ((biome == 0 || biome == 1) && soil == 1)					return world->GetDisplayColor0(UTerrain::VOLCANIC_TUNDRA);
		if ((biome == 0 || biome == 1) && soil == 2)					return world->GetDisplayColor0(UTerrain::BOREAL_SANDY_PLAINS);
		if (biome == 0 || (biome == 1 && soil == 4))					return world->GetDisplayColor0(UTerrain::TUNDRA);
		if (soil == 4)													return world->GetDisplayColor0(UTerrain::ALPINE_TUNDRA);
		if (biome == 1 && soil == 5)									return world->GetDisplayColor0(UTerrain::SUBARCTIC_BOG);
		if (biome == 1 && soil == 9)									return world->GetDisplayColor0(UTerrain::BOREAL_FOREST);
		if (biome == 1 && (soil == 3 || soil == 6))						return world->GetDisplayColor0(UTerrain::SPARSE_BOREAL_FOREST);
		if (biome == 1 && soil == 0)									return world->GetDisplayColor0(UTerrain::DENSE_BOREAL_FOREST);
		if (biome == 2 && soil == 9)									return world->GetDisplayColor0(UTerrain::TEMPERATE_PINE_FOREST);
		if (biome == 2 && soil == 0)									return world->GetDisplayColor0(UTerrain::TEMPERATE_FOREST);
		if (biome == 2 && soil == 6 || biome == 3 && soil == 0)			return world->GetDisplayColor0(UTerrain::SPARSE_TEMPERATE_FOREST);
		if ((biome == 2 || biome == 3) && (soil == 10 || soil == 8))	return world->GetDisplayColor0(UTerrain::DENSE_TEMPERATE_FOREST);
		if (biome == 2 && soil == 3)									return world->GetDisplayColor0(UTerrain::TEMPERATE_GRASSLAND);
		if ((biome == 2 || biome == 3) && soil == 11)					return world->GetDisplayColor0(UTerrain::TEMPERATE_MUDLAND);
		if ((biome == 2 || biome == 1) && soil == 7)					return world->GetDisplayColor0(UTerrain::FERTILE_MEADOWS);
		if ((biome == 2 || biome == 3) && soil == 5)					return world->GetDisplayColor0(UTerrain::TEMPERATE_BOG);
		if ((biome == 2 || biome == 3) && soil == 2)					return world->GetDisplayColor0(UTerrain::TEMPERATE_SANDY_PLAINS);
		if ((biome == 2 || biome == 3) && soil == 1)					return world->GetDisplayColor0(UTerrain::VOLCANIC_PLAINS);
		if ((biome == 2 || biome == 3) && soil == 4)					return world->GetDisplayColor0(UTerrain::ROCKLANDS);
		if (biome == 3 && soil == 7)									return world->GetDisplayColor0(UTerrain::FERTILE_PLAINS);
		if (biome == 3 && (soil == 3 || soil == 6 || soil == 9))		return world->GetDisplayColor0(UTerrain::STEPPE);
		if (biome == 4 && (soil == 0 || soil == 10))					return world->GetDisplayColor0(UTerrain::SUBTROPICAL_RAINFOREST);
		if (biome == 4 && (soil == 3 || soil == 6))						return world->GetDisplayColor0(UTerrain::SPARSE_SUBTROPICAL_RAINFOREST);
		if (biome == 4 && (soil == 10 || soil == 8))					return world->GetDisplayColor0(UTerrain::DENSE_SUBTROPICAL_RAINFOREST);
		if (biome == 4 && soil == 9)									return world->GetDisplayColor0(UTerrain::SUBTROPICAL_GRASSLAND);
		if (biome == 4 && soil == 5)									return world->GetDisplayColor0(UTerrain::WET_SUBTROPICAL_BOG);
		if (biome == 4 && (soil == 7 || soil == 11))					return world->GetDisplayColor0(UTerrain::WET_SUBTROPICAL_MUDLAND);
		if (biome == 4 && soil == 2)									return world->GetDisplayColor0(UTerrain::SUBTROPICAL_SANDY_PLAINS);
		if (biome == 4 && soil == 1)									return world->GetDisplayColor0(UTerrain::VOLCANIC_FOREST);
		if ((biome == 5 || biome == 9) && soil == 0)					return world->GetDisplayColor0(UTerrain::MEDITERRANEAN_FOREST);
		if (biome == 5 && soil == 6)									return world->GetDisplayColor0(UTerrain::SPARSE_MEDITERRANEAN_FOREST);
		if ((biome == 5 || biome == 9) && soil == 10)					return world->GetDisplayColor0(UTerrain::DENSE_MEDITERRANEAN_FOREST);
		if (biome == 5 && soil == 9)									return world->GetDisplayColor0(UTerrain::MEDITERRANEAN_PINE_FOREST);
		if (biome == 5 && soil == 11)									return world->GetDisplayColor0(UTerrain::MEDITERRANEAN_PLAINS);
		if (biome == 5 && soil == 3)									return world->GetDisplayColor0(UTerrain::MEDITERRANEAN_SHRUBLAND);
		if ((biome == 5 || biome == 9 || biome == 13) && soil == 5)		return world->GetDisplayColor0(UTerrain::DRY_SUBTROPICAL_BOG);
		if ((biome == 5 || biome == 9 || biome == 13) && soil == 11)	return world->GetDisplayColor0(UTerrain::DRY_SUBTROPICAL_MUDLAND);
		if ((biome == 5 || biome == 9 || biome == 13) && soil == 3)		return world->GetDisplayColor0(UTerrain::ROCKLANDS);
		if ((biome == 5 || biome == 9 || biome == 13) && soil == 2)		return world->GetDisplayColor0(UTerrain::DRY_SANDY_PLAINS);
		if ((biome == 5 || biome == 9 || biome == 13) && soil == 7)		return world->GetDisplayColor0(UTerrain::DRY_FERTILE_PLAINS);
		if ((biome == 5 || biome == 9 || biome == 13) && soil == 1)		return world->GetDisplayColor0(UTerrain::VOLCANIC_SHRUBLAND);
		if (biome == 6 && soil == 0)									return world->GetDisplayColor0(UTerrain::MONSOON_FOREST);
		if (biome == 6 && soil == 6)									return world->GetDisplayColor0(UTerrain::SPARSE_MONSOON_FOREST);
		if (biome == 6 && soil == 10)									return world->GetDisplayColor0(UTerrain::DENSE_MONSOON_FOREST);
		if (biome == 7 && soil == 3)									return world->GetDisplayColor0(UTerrain::SAND_DESERT);
		if (biome == 8 && soil == 3)									return world->GetDisplayColor0(UTerrain::SANDY_XERIC_SHRUBLAND);
		if ((biome == 9 || biome == 13) && soil == 6)					return world->GetDisplayColor0(UTerrain::DRY_STEPPE);
		if ((biome == 9 || biome == 13) && soil == 3)					return world->GetDisplayColor0(UTerrain::DRY_SHRUBLAND);
		if (biome == 10 && (soil == 2 || soil == 3 || soil == 6))		return world->GetDisplayColor0(UTerrain::SEMIARID_DESERT);
		if ((biome == 7 || biome == 8 || biome == 10) && soil == 0)		return world->GetDisplayColor0(UTerrain::SEMIARID_DESERT);
		if ((biome == 7 || biome == 8 || biome == 10) && soil == 1)		return world->GetDisplayColor0(UTerrain::VOLCANIC_DESERT);
		if (biome == 10 && (soil == 7 || soil == 11))					return world->GetDisplayColor0(UTerrain::SEMIARID_MUDLAND);
		if (biome == 11 && (soil == 0 || soil == 6))					return world->GetDisplayColor0(UTerrain::GRASS_SAVANNA);
		if ((biome == 11 || biome == 12) && soil == 3)					return world->GetDisplayColor0(UTerrain::GRASS_SAVANNA);
		if ((biome == 11 || biome == 12) && soil == 2)					return world->GetDisplayColor0(UTerrain::DRY_SHRUBLAND);
		if (biome == 11 && soil == 10)									return world->GetDisplayColor0(UTerrain::SPARSE_TREE_SAVANNA);
		if (biome == 12 && (soil == 0 || soil == 6))					return world->GetDisplayColor0(UTerrain::TREE_SAVANNA);
		if (biome == 11 && soil == 8)									return world->GetDisplayColor0(UTerrain::TREE_SAVANNA);
		if (biome == 12 && (soil == 8 || soil == 10))					return world->GetDisplayColor0(UTerrain::DENSE_TREE_SAVANNA);
		if ((biome == 11 || biome == 12) && soil == 5)					return world->GetDisplayColor0(UTerrain::DRY_TROPICAL_BOG);
		if ((biome == 11 || biome == 12) && soil == 11)					return world->GetDisplayColor0(UTerrain::DRY_TROPICAL_MUDLAND);
		if ((biome == 11 || biome == 12) && soil == 1)					return world->GetDisplayColor0(UTerrain::VOLCANIC_SAVANNA);
		if (biome == 13 && soil == 0)									return world->GetDisplayColor0(UTerrain::DRY_SUBTROPICAL_FOREST);
		if (biome == 13 && (soil == 6 || soil == 9))					return world->GetDisplayColor0(UTerrain::SPARSE_DRY_SUBTROPICAL_FOREST);
		if (biome == 13 && (soil == 8 || soil == 10))					return world->GetDisplayColor0(UTerrain::DENSE_DRY_SUBTROPICAL_FOREST);
		if (biome == 14 && (soil == 0 || soil == 10))					return world->GetDisplayColor0(UTerrain::TROPICAL_RAINFOREST);
		if (biome == 14 && soil == 6)									return world->GetDisplayColor0(UTerrain::SPARSE_TROPICAL_RAINFOREST);
		if ((biome == 14 || biome == 6) && soil == 8)					return world->GetDisplayColor0(UTerrain::DENSE_TROPICAL_RAINFOREST);
		if ((biome == 14 || biome == 6) && soil == 3)					return world->GetDisplayColor0(UTerrain::WET_TROPICAL_GRASSLAND);
		if ((biome == 14 || biome == 6) && soil == 5)					return world->GetDisplayColor0(UTerrain::WET_TROPICAL_BOG);
		if ((biome == 14 || biome == 6) && soil == 11)					return world->GetDisplayColor0(UTerrain::WET_TROPICAL_MUDLAND);
		if ((biome == 14 || biome == 6) && soil == 2)					return world->GetDisplayColor0(UTerrain::TROPICAL_SANDY_PLAINS);
		if ((biome == 14 || biome == 6) && soil == 1)					return world->GetDisplayColor0(UTerrain::VOLCANIC_RAINFOREST);
		if (soil == 12)													return world->GetDisplayColor0(UTerrain::ROCKLANDS);
		if (soil == 13)													return world->GetDisplayColor0(UTerrain::SHIFTING_SANDS);
		// edge cases
		if (biome == 0)													return world->GetDisplayColor0(UTerrain::TUNDRA);
		if (biome == 1)													return world->GetDisplayColor0(UTerrain::BOREAL_FOREST);
		if (biome == 2)													return world->GetDisplayColor0(UTerrain::TEMPERATE_FOREST);
		if (biome == 3)													return world->GetDisplayColor0(UTerrain::STEPPE);
		if (biome == 4)													return world->GetDisplayColor0(UTerrain::SUBTROPICAL_RAINFOREST);
		if (biome == 5)													return world->GetDisplayColor0(UTerrain::MEDITERRANEAN_SHRUBLAND);
		if (biome == 6)													return world->GetDisplayColor0(UTerrain::MONSOON_FOREST);
		if (biome == 7)													return world->GetDisplayColor0(UTerrain::MUD_DESERT);
		if (biome == 8)													return world->GetDisplayColor0(UTerrain::MUDDY_XERIC_SHRUBLAND);
		if (biome == 9)													return world->GetDisplayColor0(UTerrain::DRY_STEPPE);
		if (biome == 10)												return world->GetDisplayColor0(UTerrain::SEMIARID_DESERT);
		if (biome == 11)												return world->GetDisplayColor0(UTerrain::GRASS_SAVANNA);
		if (biome == 12)												return world->GetDisplayColor0(UTerrain::TREE_SAVANNA);
		if (biome == 13)												return world->GetDisplayColor0(UTerrain::DRY_SUBTROPICAL_FOREST);
		if (biome == 14)												return world->GetDisplayColor0(UTerrain::TROPICAL_RAINFOREST);
	}
		return { 0, 0, 0 };
}
