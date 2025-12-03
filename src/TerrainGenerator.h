#pragma once

#include "Common.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"
#include "MeshRenderable.h"
#include <vector>

// Terrain constants - centralized configuration for all terrain-related magic numbers
struct TerrainConstants
{
    // Sea level configuration
    static constexpr float SEA_LEVEL_NORMALIZED = 0.13f;  // Normalized sea level (0-1 range)
    static constexpr float SEA_LEVEL_OFFSET = 0.12f;      // Sea level for terrain height comparison
    static constexpr float WATER_PLANE_OFFSET = 0.1f;     // How much to raise water plane above terrain sea level
    
    // Texture scaling
    static constexpr float TEXTURE_SCALE = 10.0f;         // Divisor for texture coordinates
    
    // Noise sampling scales
    static constexpr float NOISE_SAMPLE_SCALE = 0.01f;    // Base scale for perlin sampling
    
    // Mountain generation
    static constexpr float MOUNTAIN_CENTER_X_FACTOR = 0.15f;  // X position factor for mountain center
    static constexpr float MOUNTAIN_CENTER_Z_FACTOR = 0.15f;  // Z position factor for mountain center
    static constexpr float MOUNTAIN_DOMAIN_SCALE = 0.002f;    // Scale for mountain domain noise
    static constexpr float MOUNTAIN_INFLUENCE_RADIUS = 140.0f; // Radius of mountain influence
    static constexpr float MOUNTAIN_THRESHOLD = 0.32f;        // Threshold for mountain area
    
    // Lake generation
    static constexpr float LAKE_NOISE_SCALE = 0.0008f;    // Scale for lake noise
    static constexpr float LAKE_THRESHOLD = 0.45f;         // Threshold for lake formation
    static constexpr float LAKE_DEPRESSION_FACTOR = 0.5f;  // How much lakes depress terrain
    
    // Water mask generation
    static constexpr float WATER_NOISE_SCALE = 0.0015f;   // Scale for water area noise
    static constexpr float WATER_AREA_THRESHOLD = 0.55f;  // Threshold for water areas
    static constexpr float WATER_MIN_DEPTH = 0.02f;       // Minimum depth for water to appear
};

struct TerrainConfig
{
    int width = 256;
    int height = 256;
    float scale = 20.0f;        // How stretched the terrain is horizontally
    float heightScale = 30.0f;  // Maximum height of the terrain
    int octaves = 6;            // Number of noise layers
    float persistence = 0.5f;   // How much each octave contributes
    float lacunarity = 2.0f;    // Frequency increase per octave
    int vertexStep = 3;         // Spacing between vertices (1 = every point, 2 = every other point, etc.)
};

struct TerrainVertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    float height; // Normalized height for texture blending (0-1)
    float waterMask; // 0.0 = no water, 1.0 = full water
};

class TerrainGenerator
{
public:
    TerrainGenerator(const TerrainConfig& config);
    ~TerrainGenerator();

    // Generate terrain mesh with Perlin noise
    std::shared_ptr<Mesh> generateTerrainMesh();

    // Get height at a specific x, z coordinate (useful for placing objects)
    float getHeightAt(float x, float z) const;
    
    // Generate Perlin noise value (public for chunk generation)
    float getPerlinHeight(float x, float z);
    
    // Check if location should have water (based on height and area)
    float getWaterMask(float x, float z);

private:    

    TerrainConfig m_config;
    std::vector<std::vector<float>> m_heightMap;

    // Generate height map using Perlin noise
    void generateHeightMap();

    // Calculate normal for a vertex based on surrounding heights
    glm::vec3 calculateNormal(int x, int z);
};
