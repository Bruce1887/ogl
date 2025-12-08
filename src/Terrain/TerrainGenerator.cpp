#include "TerrainGenerator.h"

#define STB_PERLIN_IMPLEMENTATION
#include "vendor/stb_image/stb_perlin.h"

#include <algorithm>
#include <iostream>

TerrainGenerator::TerrainGenerator(/*const TerrainConfig &config*/)
// : m_config(config)
{
    // Initialize height map
    // m_heightMap.resize(m_config.width);
    m_heightMap.resize(TC_WIDTH);
    for (int i = 0; i < TC_WIDTH; i++)
    {
        m_heightMap[i].resize(TC_HEIGHT);
    }
}

TerrainGenerator::~TerrainGenerator()
{
}

float TerrainGenerator::getPerlinHeight(float x, float z)
{
    float sampleX = x * TC_TERRAIN_SAMPLE_FACTOR_X;
    float sampleZ = z * TC_TERRAIN_SAMPLE_FACTOR_Z;

    // Use ridge noise for sharp mountain features (primary)
    float ridgeNoise = stb_perlin_ridge_noise3(sampleX, 0.0f, sampleZ,
                                               TC_RIDGE_NOISE_LACUNARITY, // lacunarity
                                               TC_RIDGE_NOISE_GAIN,       // gain
                                               TC_RIDGE_NOISE_OFFSET,     // offset
                                               TC_RIDGE_NOISE_OCTAVES);   // octaves

    // Use FBM for gentle hills/variation (secondary) - increased for more ondulation
    float hillNoise = stb_perlin_fbm_noise3(sampleX * 0.8f, 0.0f, sampleZ * 0.8f,
                                            TC_HILL_NOISE_LACUNARITY, // lacunarity
                                            TC_HILL_NOISE_GAIN,       // gain - increased for more variation
                                            TC_HILL_NOISE_OCTAVES);   // octaves - more for ondulation

    // Normalize ridge noise (it can be negative)
    ridgeNoise = (ridgeNoise + 1.0f) * 0.5f;
    hillNoise = (hillNoise + 1.0f) * 0.5f;

    // Lake depression noise - sparse, large areas
    float lakeNoise = stb_perlin_fbm_noise3(x * TC_SEA_SAMPLE_FACTOR_X, 0.0f, z * TC_SEA_SAMPLE_FACTOR_Z,
                                            TC_SEA_SAMPLE_LACUNARITY,
                                            TC_SEA_SAMPLE_GAIN,
                                            TC_SEA_SAMPLE_OCTAVES);

    lakeNoise = (lakeNoise + 1.0f) * 0.5f; // normalise to 0-1

    // Create lake depressions (only in specific areas - higher threshold = fewer, larger lakes)
    float lakeDepression = 0.0f;
    if (lakeNoise > 0.65f)
    { // Higher threshold - only ~35% of terrain can have lakes
        // Stronger depression for larger lakes
        lakeDepression = (lakeNoise - 0.65f) * 0.7f; // Up to 0.245 units lower
    }

    // Base terrain centered above sea level to prevent random puddles
    // Only intentional lake depressions should go below sea level
    float baseHeight = hillNoise * TC_SEA_LEVEL + TC_SEA_LEVEL_OFFSET + 0.02f;

    // Add ridge details for variety across the map
    float ridgeDetail = ridgeNoise * TC_RIDGE_DETAIL_FACTOR;
    float total = baseHeight + ridgeDetail - lakeDepression;

    // Center in bottom-left quadrant but make it MUCH wider
    float mountainCenterX = TC_WIDTH * 0.05f; //0.15f;
    float mountainCenterZ = TC_HEIGHT * 0.05f; //0.15f;

    // Use ridge noise to define the mountain "domain" - not circular!
    float domainX = x * 0.002f; // Very low frequency for large structures
    float domainZ = z * 0.002f;

    // Create an irregular mountain domain using ridge noise
    float mountainDomain = stb_perlin_ridge_noise3(domainX, domainZ, 100.0f,
                                                   15.0f, 0.5f, 1.0f, 3);
    mountainDomain = (mountainDomain + 1.0f) * 0.5f;

    // Add position bias toward our target area
    float distX = x - mountainCenterX;
    float distZ = z - mountainCenterZ;
    float distFromCenter = sqrt(distX * distX + distZ * distZ);

    // Very wide influence area
    float mountainInfluence = 1.0f - std::min(distFromCenter / 140.0f, 1.0f); // 250 unit radius!

    // Combine domain noise with position bias for irregular boundary
    float inMountainArea = mountainDomain * 0.4f + mountainInfluence * 0.6f;

    if (inMountainArea > 0.32f) // Raised threshold to prevent ground mimicking mountain shape
    {
        // Sample ridge noise at multiple scales for natural variation
        float detailX = x * 0.015f;
        float detailZ = z * 0.015f;

        // Large-scale mountain ridges
        float ridgeLarge = stb_perlin_ridge_noise3(detailX * 0.6f, detailZ * 0.6f, 100.0f,
                                                   2.0f, 0.5f, 1.0f, 5);

        // Medium-scale features
        float ridgeMedium = stb_perlin_ridge_noise3(detailX, detailZ, 200.0f,
                                                    2.0f, 0.5f, 1.0f, 4);

        // Fine detail
        float ridgeFine = stb_perlin_ridge_noise3(detailX * 2.0f, detailZ * 2.0f, 300.0f,
                                                  2.0f, 0.5f, 1.0f, 3);

        // Normalize
        ridgeLarge = (ridgeLarge + 1.0f) * 0.5f;
        ridgeMedium = (ridgeMedium + 1.0f) * 0.5f;
        ridgeFine = (ridgeFine + 1.0f) * 0.5f;

        // Combine at different scales
        float combinedHeight = ridgeLarge * 0.5f + ridgeMedium * 0.3f + ridgeFine * 0.2f;

        // Create varied slopes by modulating height with another noise layer
        float slopeMod = stb_perlin_fbm_noise3(x * 0.008f, z * 0.008f, 400.0f,
                                               2.0f, 0.5f, 3);
        slopeMod = (slopeMod + 1.0f) * 0.5f;

        // Some areas have steep cliffs (high multiplier), others gradual slopes (low multiplier)
        float slopeVariation = 0.5f + slopeMod * 1.5f; // Range: 0.5 to 2.0

        // Apply slope variation
        combinedHeight = pow(combinedHeight, slopeVariation);

        // SHARP cutoff for mountain strength - prevents gradual height increase on plains
        float mountainStrength = (inMountainArea - 0.3f) / 0.7f; // Remap 0.3-1.0 to 0.0-1.0
        mountainStrength = std::max(0.0f, mountainStrength);

        // Apply a sharp transition using exponential function
        mountainStrength = pow(mountainStrength, 0.5f); // Less sharp at base for some foothills

        // Edge variation but keep it subtle
        float edgeNoise = stb_perlin_noise3(x * 0.01f, z * 0.01f, 500.0f, 0, 0, 0);
        edgeNoise = (edgeNoise + 1.0f) * 0.5f;
        mountainStrength = mountainStrength * (0.85f + edgeNoise * 0.15f); // Less variation at edges

        // Final mountain height - MUCH TALLER
        float mountainHeight = combinedHeight * mountainStrength;

        // Boost height significantly and ensure tall peaks
        mountainHeight = mountainHeight * 1.2f + mountainStrength * 0.2f; // Higher multiplier

        // Only apply if we have significant mountain strength
        if (mountainStrength > 0.1f)
        {
            total = std::max(total, mountainHeight);
        }
    }

    return total;
}

void TerrainGenerator::generateHeightMap()
{
    for (int z = 0; z < TC_HEIGHT; z++)
    {
        for (int x = 0; x < TC_WIDTH; x++)
        {
            float height = getPerlinHeight((float)x, (float)z);
            m_heightMap[x][z] = height;
        }
    }
}

glm::vec3 TerrainGenerator::calculateNormal(int x, int z)
{
    // Use central difference to calculate normal
    float heightL = (x > 0) ? m_heightMap[x - 1][z] : m_heightMap[x][z];
    float heightR = (x < TC_WIDTH - 1) ? m_heightMap[x + 1][z] : m_heightMap[x][z];
    float heightD = (z > 0) ? m_heightMap[x][z - 1] : m_heightMap[x][z];
    float heightU = (z < TC_HEIGHT - 1) ? m_heightMap[x][z + 1] : m_heightMap[x][z];

    glm::vec3 normal;
    normal.x = (heightL - heightR) * TC_HEIGHT_SCALE;
    normal.y = 2.0f; // Scale factor for smoothness
    normal.z = (heightD - heightU) * TC_HEIGHT_SCALE;

    return glm::normalize(normal);
}

std::shared_ptr<Mesh> TerrainGenerator::generateTerrainMesh()
{
    // Generate height map first
    generateHeightMap();

    std::vector<TerrainVertex> vertices;
    std::vector<unsigned int> indices;

    // Center the terrain around origin
    float offsetX = TC_WIDTH * 0.5f;
    float offsetZ = TC_HEIGHT * 0.5f;

    int step = TC_VERTEX_STEP;

    // Sea level in world units
    const float seaLevel = 0.13f * TC_HEIGHT_SCALE;

    // For flat shading, we need to generate separate vertices for each triangle
    // rather than sharing vertices between triangles
    // Use vertexStep to skip vertices and reduce polygon count
    for (int z = 0; z < TC_HEIGHT - step; z += step)
    {
        for (int x = 0; x < TC_WIDTH - step; x += step)
        {
            // Get the four corner heights
            float h_tl = m_heightMap[x][z];
            float h_tr = m_heightMap[x + step][z];
            float h_bl = m_heightMap[x][z + step];
            float h_br = m_heightMap[x + step][z + step];

            // Get water masks
            float w_tl = getWaterMask((float)x, (float)z);
            float w_tr = getWaterMask((float)(x + step), (float)z);
            float w_bl = getWaterMask((float)x, (float)(z + step));
            float w_br = getWaterMask((float)(x + step), (float)(z + step));

            // Calculate positions - always use terrain height for terrain mesh
            glm::vec3 pos_tl((float)x - offsetX, h_tl * TC_HEIGHT_SCALE, (float)z - offsetZ);
            glm::vec3 pos_tr((float)(x + step) - offsetX, h_tr * TC_HEIGHT_SCALE, (float)z - offsetZ);
            glm::vec3 pos_bl((float)x - offsetX, h_bl * TC_HEIGHT_SCALE, (float)(z + step) - offsetZ);
            glm::vec3 pos_br((float)(x + step) - offsetX, h_br * TC_HEIGHT_SCALE, (float)(z + step) - offsetZ);

            // First triangle (top-left, bottom-left, top-right)
            {
                glm::vec3 edge1 = pos_bl - pos_tl;
                glm::vec3 edge2 = pos_tr - pos_tl;
                glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

                unsigned int baseIdx = vertices.size();

                // Vertex 1 (top-left)
                TerrainVertex v1;
                v1.position = pos_tl;
                v1.normal = normal;
                v1.texCoord = glm::vec2((float)x / 10.0f, (float)z / 10.0f);
                v1.height = h_tl;
                v1.waterMask = 0.0f;
                vertices.push_back(v1);

                // Vertex 2 (bottom-left)
                TerrainVertex v2;
                v2.position = pos_bl;
                v2.normal = normal;
                v2.texCoord = glm::vec2((float)x / 10.0f, (float)(z + 1) / 10.0f);
                v2.height = h_bl;
                v2.waterMask = 0.0f;
                vertices.push_back(v2);

                // Vertex 3 (top-right)
                TerrainVertex v3;
                v3.position = pos_tr;
                v3.normal = normal;
                v3.texCoord = glm::vec2((float)(x + 1) / 10.0f, (float)z / 10.0f);
                v3.height = h_tr;
                v3.waterMask = 0.0f;
                vertices.push_back(v3);

                indices.push_back(baseIdx);
                indices.push_back(baseIdx + 1);
                indices.push_back(baseIdx + 2);
            }

            // Second triangle (top-right, bottom-left, bottom-right)
            {
                glm::vec3 edge1 = pos_bl - pos_tr;
                glm::vec3 edge2 = pos_br - pos_tr;
                glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

                unsigned int baseIdx = vertices.size();

                // Vertex 1 (top-right)
                TerrainVertex v1;
                v1.position = pos_tr;
                v1.normal = normal;
                v1.texCoord = glm::vec2((float)(x + 1) / 10.0f, (float)z / 10.0f);
                v1.height = h_tr;
                v1.waterMask = 0.0f;
                vertices.push_back(v1);

                // Vertex 2 (bottom-left)
                TerrainVertex v2;
                v2.position = pos_bl;
                v2.normal = normal;
                v2.texCoord = glm::vec2((float)x / 10.0f, (float)(z + 1) / 10.0f);
                v2.height = h_bl;
                v2.waterMask = 0.0f;
                vertices.push_back(v2);

                // Vertex 3 (bottom-right)
                TerrainVertex v3;
                v3.position = pos_br;
                v3.normal = normal;
                v3.texCoord = glm::vec2((float)(x + 1) / 10.0f, (float)(z + 1) / 10.0f);
                v3.height = h_br;
                v3.waterMask = 0.0f;
                vertices.push_back(v3);

                indices.push_back(baseIdx);
                indices.push_back(baseIdx + 1);
                indices.push_back(baseIdx + 2);
            }
        }
    }

    // Generate continuous water layer across entire map at sea level
    for (int z = 0; z < TC_HEIGHT - step; z += step)
    {
        for (int x = 0; x < TC_WIDTH - step; x += step)
        {
            // Create flat water quad at sea level (always, for entire map)
            glm::vec3 pos_tl((float)x - offsetX, seaLevel, (float)z - offsetZ);
            glm::vec3 pos_tr((float)(x + step) - offsetX, seaLevel, (float)z - offsetZ);
            glm::vec3 pos_bl((float)x - offsetX, seaLevel, (float)(z + step) - offsetZ);
            glm::vec3 pos_br((float)(x + step) - offsetX, seaLevel, (float)(z + step) - offsetZ);

            glm::vec3 normal(0.0f, 1.0f, 0.0f); // Flat water surface

            // First triangle
            {
                unsigned int baseIdx = vertices.size();

                TerrainVertex v1;
                v1.position = pos_tl;
                v1.normal = normal;
                v1.texCoord = glm::vec2((float)x / 10.0f, (float)z / 10.0f);
                v1.height = 0.13f;
                v1.waterMask = 1.0f;
                vertices.push_back(v1);

                TerrainVertex v2;
                v2.position = pos_bl;
                v2.normal = normal;
                v2.texCoord = glm::vec2((float)x / 10.0f, (float)(z + step) / 10.0f);
                v2.height = 0.13f;
                v2.waterMask = 1.0f;
                vertices.push_back(v2);

                TerrainVertex v3;
                v3.position = pos_tr;
                v3.normal = normal;
                v3.texCoord = glm::vec2((float)(x + step) / 10.0f, (float)z / 10.0f);
                v3.height = 0.13f;
                v3.waterMask = 1.0f;
                vertices.push_back(v3);

                indices.push_back(baseIdx);
                indices.push_back(baseIdx + 1);
                indices.push_back(baseIdx + 2);
            }

            // Second triangle
            {
                unsigned int baseIdx = vertices.size();

                TerrainVertex v1;
                v1.position = pos_tr;
                v1.normal = normal;
                v1.texCoord = glm::vec2((float)(x + step) / 10.0f, (float)z / 10.0f);
                v1.height = 0.13f;
                v1.waterMask = 1.0f;
                vertices.push_back(v1);

                TerrainVertex v2;
                v2.position = pos_bl;
                v2.normal = normal;
                v2.texCoord = glm::vec2((float)x / 10.0f, (float)(z + step) / 10.0f);
                v2.height = 0.13f;
                v2.waterMask = 1.0f;
                vertices.push_back(v2);

                TerrainVertex v3;
                v3.position = pos_br;
                v3.normal = normal;
                v3.texCoord = glm::vec2((float)(x + step) / 10.0f, (float)(z + step) / 10.0f);
                v3.height = 0.13f;
                v3.waterMask = 1.0f;
                vertices.push_back(v3);

                indices.push_back(baseIdx);
                indices.push_back(baseIdx + 1);
                indices.push_back(baseIdx + 2);
            }
        }
    }

    // Create OpenGL buffers
    std::unique_ptr<VertexArray> va_ptr = std::make_unique<VertexArray>();
    std::unique_ptr<VertexBuffer> vb_ptr = std::make_unique<VertexBuffer>(vertices.data(), vertices.size() * sizeof(TerrainVertex), va_ptr.get());

    VertexBufferLayout layout;
    layout.push<float>(3); // position
    layout.push<float>(3); // normal
    layout.push<float>(2); // texCoord
    layout.push<float>(1); // height
    layout.push<float>(1); // waterMask

    va_ptr->addBuffer(vb_ptr.get(), layout);

    std::unique_ptr<IndexBuffer> ib_ptr = std::make_unique<IndexBuffer>(indices.data(), indices.size());

    auto mesh_ptr = std::make_shared<Mesh>(std::move(va_ptr), std::move(vb_ptr), std::move(ib_ptr));
    return mesh_ptr;
}

float TerrainGenerator::getHeightAt(float x, float z) const
{
    // Convert world coordinates to grid coordinates
    float offsetX = TC_WIDTH * 0.5f;
    float offsetZ = TC_HEIGHT * 0.5f;

    int gridX = (int)(x + offsetX);
    int gridZ = (int)(z + offsetZ);

    if (gridX < 0 || gridX >= TC_WIDTH || gridZ < 0 || gridZ >= TC_HEIGHT)
        return 0.0f;

    return m_heightMap[gridX][gridZ] * TC_HEIGHT_SCALE;
}

float TerrainGenerator::getWaterMask(float x, float z)
{
    // Get the terrain height
    float height = getPerlinHeight(x, z);

    // Sea level - if terrain is below this, it's underwater
    const float seaLevel = 0.13f;

    // If terrain is above sea level, no water
    if (height >= seaLevel)
        return 0.0f;

    // Use noise to create large bodies of water only (no small puddles)
    float sampleX = x * 0.0015f; // Even lower frequency for larger bodies
    float sampleZ = z * 0.0015f;

    float waterNoise = stb_perlin_fbm_noise3(sampleX, sampleZ, 50.0f,
                                             2.0f, // lacunarity
                                             0.5f, // gain
                                             4);   // more octaves for smoother areas
    waterNoise = (waterNoise + 1.0f) * 0.5f;       // Normalize to 0-1

    // Much higher threshold - only large continuous areas get water
    const float waterAreaThreshold = 0.55f;
    if (waterNoise < waterAreaThreshold)
        return 0.0f;

    // Additionally check if terrain is significantly below sea level
    // This prevents tiny dips from becoming puddles
    float depth = seaLevel - height;
    if (depth < 0.02f) // Must be at least 0.02 below sea level
        return 0.0f;

    // Water exists here - return full strength for water plane
    return 1.0f;
}

float TerrainGenerator::foo_treePerlin(float x, float z)
{
    float sampleX = x * 0.03f;
    float sampleZ = z * 0.02f;

    // random ass parameters (ändra till nåt vettigt nån dag)
    float tree_perlin = stb_perlin_fbm_noise3(sampleX * 0.8f, sampleZ * 0.8f, 0.0f,
                                              6.0f,  // lacunarity
                                              0.75f, // gain - increased for more variation
                                              8);    // octaves - more for ondulation

    float normalised_tp = (tree_perlin + 1.0f) * 0.5f; // Normalize to 0-1
    return normalised_tp;
}