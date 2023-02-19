#pragma once

#define ASSERT(x) if((x)) __debugbreak();

// Window
#define c_win_Width 1920
#define c_win_Height 1080

// Rendering
#define c_BatchFaceCount 5000
#define c_TextureSize 16
#define c_RenderDistanceStatic 3

// Game
// Terrain
#define c_ChunkSize 16
#define c_BlockSize 1.f
#define c_ChunkHeight 20
#define c_ChunkVolume c_ChunkHeight * c_ChunkSize * c_ChunkSize

#define c_TerrainYStretch 5
#define c_TerrainXStretch 1.f

#define c_TerrainMinHeight 2.f

// Environment
#define c_FogAffectDistance 18.f
#define c_FogDensity 0.3f