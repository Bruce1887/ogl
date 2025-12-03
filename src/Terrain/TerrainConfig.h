#pragma once

#define TC_WIDTH        256
#define TC_HEIGHT       256 
// How stretched the terrain is horizontally
#define TC_SCALE        20.0f
// Maximum height of the terrain
#define TC_HEIGHT_SCALE 30.0f 
// Number of noise layers
#define TC_OCTAVES      6 
// How much each octave contributes
#define TC_PERSISTENCE  0.5f
// Frequency increase per octave
#define TC_LACUNARITY   2.0f
// Spacing between vertices (1 = every point, 2 = every other point, etc.)
#define TC_VERTEX_STEP  3

// Sample factors for Perlin noise 
#define TC_TERRAIN_SAMPLE_FACTOR_X 0.01f
#define TC_TERRAIN_SAMPLE_FACTOR_Z 0.01f

// Ridge noise parameters
#define TC_RIDGE_NOISE_LACUNARITY 1.0f
#define TC_RIDGE_NOISE_GAIN       0.5f
#define TC_RIDGE_NOISE_OFFSET     1.0f
#define TC_RIDGE_NOISE_OCTAVES   5
// ridge detail
#define TC_RIDGE_DETAIL_FACTOR 0.08f

// Hill noise parameters
#define TC_HILL_NOISE_LACUNARITY  3.0f
#define TC_HILL_NOISE_GAIN        0.55f
#define TC_HILL_NOISE_OCTAVES     4

// Sea sample parameters 
#define TC_SEA_SAMPLE_FACTOR_X 0.0008f
#define TC_SEA_SAMPLE_FACTOR_Z 0.0008f
#define TC_SEA_SAMPLE_LACUNARITY 2.0f	
#define TC_SEA_SAMPLE_GAIN       0.5f
#define TC_SEA_SAMPLE_OCTAVES 3
// other sea parameters
#define TC_SEA_LEVEL 0.13f
#define TC_SEA_LEVEL_OFFSET 0.05f