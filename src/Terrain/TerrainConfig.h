#pragma once
// #### Chunkmanager configuration parameters ####
#define TC_CHUNK_SIZE 100		  // Size of each chunk in grid units (e.g., 32x32)
#define TC_RENDER_DISTANCE 100.0f  // Distance in world units to render chunks
#define TC_UPDATE_THRESHOLD 10.0f // Minimum camera movement to trigger chunk update
#define TC_GC_THRESHOLD 100		  // Number of chunks before triggering garbage collection
#define TC_CELLS_PER_AXIS (TC_CHUNK_SIZE / TC_VERTEX_STEP)  // Number of cells (triangles) along one side of a chunk  (bad name)
#define TC_VERTICES_PER_AXIS (TC_CELLS_PER_AXIS + 1) // Number of vertices along one side of a chunk
#define TC_CELLS_PER_CHUNK (TC_CELLS_PER_AXIS * TC_CELLS_PER_AXIS) // Total number of cells (triangles) in a chunk

// #### Terrain generation parameters ####
#define TC_WIDTH 256
#define TC_HEIGHT 256
// How stretched the terrain is horizontally
#define TC_SCALE 20.0f
// Maximum height of the terrain
#define TC_HEIGHT_SCALE 30.0f
// Number of noise layers
#define TC_OCTAVES 6
// How much each octave contributes
#define TC_PERSISTENCE 0.5f
// Frequency increase per octave
#define TC_LACUNARITY 3.0f
// Spacing between vertices (1 = every point, 2 = every other point, etc.)
#define TC_VERTEX_STEP 5

// Ridge noise parameters
#define TC_RIDGE_SAMPLE_FACTOR 0.005f
#define TC_RIDGE_NOISE_LACUNARITY 2.0f
#define TC_RIDGE_NOISE_GAIN 0.4f
#define TC_RIDGE_NOISE_OFFSET 1.0f
#define TC_RIDGE_NOISE_OCTAVES 5
#define TC_RIDGE_DETAIL_FACTOR 0.08f

// Hill noise parameters
#define TC_HILL_SAMPLE_FACTOR 0.001f // lower number for biger/smoother hills
#define TC_HILL_NOISE_LACUNARITY 3.0f
#define TC_HILL_NOISE_GAIN 1.4f
#define TC_HILL_NOISE_OCTAVES 4

// Sea sample parameters
#define TC_SEA_SAMPLE_FACTOR_X 0.0008f
#define TC_SEA_SAMPLE_FACTOR_Z 0.0008f
#define TC_SEA_SAMPLE_LACUNARITY 2.0f
#define TC_SEA_SAMPLE_GAIN 0.5f
#define TC_SEA_SAMPLE_OCTAVES 3
// other sea parameters
#define TC_SEA_LEVEL 0.13f
#define TC_SEA_LEVEL_OFFSET 0.05f