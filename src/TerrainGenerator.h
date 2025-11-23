#pragma once

#include "Common.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"
#include "MeshRenderable.h"
#include <vector>

struct TerrainConfig
{
    int width = 256;
    int height = 256;
    float scale = 20.0f;        // How stretched the terrain is horizontally
    float heightScale = 30.0f;  // Maximum height of the terrain
    int octaves = 6;            // Number of noise layers
    float persistence = 0.5f;   // How much each octave contributes
    float lacunarity = 2.0f;    // Frequency increase per octave
    int vertexStep = 1;         // Spacing between vertices (1 = every point, 2 = every other point, etc.)
};

struct TerrainVertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    float height; // Normalized height for texture blending (0-1)
};

class TerrainGenerator
{
public:
    TerrainGenerator(const TerrainConfig& config);
    ~TerrainGenerator();

    // Generate terrain mesh with Perlin noise
    Mesh* generateTerrain();

    // Get height at a specific x, z coordinate (useful for placing objects)
    float getHeightAt(float x, float z) const;
    
    // Generate Perlin noise value (public for chunk generation)
    float getPerlinHeight(float x, float z);

private:
    TerrainConfig m_config;
    std::vector<std::vector<float>> m_heightMap;

    // Generate height map using Perlin noise
    void generateHeightMap();

    // Calculate normal for a vertex based on surrounding heights
    glm::vec3 calculateNormal(int x, int z);
};
