#include "TerrainGenerator.h"

#define STB_PERLIN_IMPLEMENTATION
#include "vendor/stb_image/stb_perlin.h"

#include <algorithm>
#include <iostream>

TerrainGenerator::TerrainGenerator(const TerrainConfig& config)
    : m_config(config)
{
    // Initialize height map
    m_heightMap.resize(m_config.width);
    for (int i = 0; i < m_config.width; i++)
    {
        m_heightMap[i].resize(m_config.height);
    }
}

TerrainGenerator::~TerrainGenerator()
{
}

float TerrainGenerator::getPerlinHeight(float x, float z)
{
    float sampleX = x * TerrainConstants::NOISE_SAMPLE_SCALE;
    float sampleZ = z * TerrainConstants::NOISE_SAMPLE_SCALE;
    
    // Use ridge noise for sharp mountain features (primary)
    float ridgeNoise = stb_perlin_ridge_noise3(sampleX, sampleZ, 0.0f,
                                                1.0f,   // lacunarity
                                                0.5f,   // gain
                                                1.0f,   // offset
                                                5);     // octaves
    
    // Use FBM for gentle hills/variation (secondary) - increased for more ondulation
    float hillNoise = stb_perlin_fbm_noise3(sampleX * 0.8f, sampleZ * 0.8f, 0.0f,
                                            3.0f,   // lacunarity
                                            0.55f,  // gain - increased for more variation
                                            4);     // octaves - more for ondulation
    
    // Normalize ridge noise (it can be negative)
    ridgeNoise = (ridgeNoise + 1.0f) * 0.5f;
    hillNoise = (hillNoise + 1.0f) * 0.5f;
    
    // Lake depression noise - sparse, large areas
    float lakeNoise = stb_perlin_fbm_noise3(x * TerrainConstants::LAKE_NOISE_SCALE, 
                                            z * TerrainConstants::LAKE_NOISE_SCALE, 42.0f,
                                            2.0f,   // lacunarity
                                            0.5f,   // gain
                                            3);     // octaves
    lakeNoise = (lakeNoise + 1.0f) * 0.5f;
    
    // Create lake depressions (more common now)
    float lakeDepression = 0.0f;
    if (lakeNoise > TerrainConstants::LAKE_THRESHOLD) {  // More common threshold - ~55% of terrain
        // Stronger depression for larger lakes
        lakeDepression = (lakeNoise - TerrainConstants::LAKE_THRESHOLD) * TerrainConstants::LAKE_DEPRESSION_FACTOR;
    }
    
    // Base terrain centered around sea level (0.13) with variation
    // Range: ~0.08 to 0.20 (some below, some above sea level)
    float baseHeight = hillNoise * 0.12f + 0.05f;
    
    // Add ridge details for variety across the map
    float ridgeDetail = ridgeNoise * 0.08f;
    float total = baseHeight + ridgeDetail - lakeDepression;

    // Center in bottom-left quadrant but make it MUCH wider
    float mountainCenterX = m_config.width * TerrainConstants::MOUNTAIN_CENTER_X_FACTOR;
    float mountainCenterZ = m_config.height * TerrainConstants::MOUNTAIN_CENTER_Z_FACTOR;
    
    // Use ridge noise to define the mountain "domain" - not circular!
    float domainX = x * TerrainConstants::MOUNTAIN_DOMAIN_SCALE;  // Very low frequency for large structures
    float domainZ = z * TerrainConstants::MOUNTAIN_DOMAIN_SCALE;
    
    // Create an irregular mountain domain using ridge noise
    float mountainDomain = stb_perlin_ridge_noise3(domainX, domainZ, 100.0f,
                                                   15.0f, 0.5f, 1.0f, 3);
    mountainDomain = (mountainDomain + 1.0f) * 0.5f;
    
    // Add position bias toward our target area
    float distX = x - mountainCenterX;
    float distZ = z - mountainCenterZ;
    float distFromCenter = sqrt(distX * distX + distZ * distZ);
    
    // Very wide influence area
    float mountainInfluence = 1.0f - std::min(distFromCenter / TerrainConstants::MOUNTAIN_INFLUENCE_RADIUS, 1.0f);
    
    // Combine domain noise with position bias for irregular boundary
    float inMountainArea = mountainDomain * 0.4f + mountainInfluence * 0.6f;
    
    if (inMountainArea > TerrainConstants::MOUNTAIN_THRESHOLD) // Raised threshold to prevent ground mimicking mountain shape
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
    for (int z = 0; z < m_config.height; z++)
    {
        for (int x = 0; x < m_config.width; x++)
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
    float heightR = (x < m_config.width - 1) ? m_heightMap[x + 1][z] : m_heightMap[x][z];
    float heightD = (z > 0) ? m_heightMap[x][z - 1] : m_heightMap[x][z];
    float heightU = (z < m_config.height - 1) ? m_heightMap[x][z + 1] : m_heightMap[x][z];

    glm::vec3 normal;
    normal.x = (heightL - heightR) * m_config.heightScale;
    normal.y = 2.0f; // Scale factor for smoothness
    normal.z = (heightD - heightU) * m_config.heightScale;

    return glm::normalize(normal);
}

std::shared_ptr<Mesh> TerrainGenerator::generateTerrainMesh()
{
    // Generate height map first
    generateHeightMap();

    std::vector<TerrainVertex> vertices;
    std::vector<unsigned int> indices;

    // Center the terrain around origin
    float offsetX = m_config.width * 0.5f;
    float offsetZ = m_config.height * 0.5f;

    int step = m_config.vertexStep;

    // Sea level in world units
    const float seaLevel = 0.13f * m_config.heightScale;
    
    // For flat shading, we need to generate separate vertices for each triangle
    // rather than sharing vertices between triangles
    // Use vertexStep to skip vertices and reduce polygon count
    for (int z = 0; z < m_config.height - step; z += step)
    {
        for (int x = 0; x < m_config.width - step; x += step)
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
            glm::vec3 pos_tl((float)x - offsetX, h_tl * m_config.heightScale, (float)z - offsetZ);
            glm::vec3 pos_tr((float)(x + step) - offsetX, h_tr * m_config.heightScale, (float)z - offsetZ);
            glm::vec3 pos_bl((float)x - offsetX, h_bl * m_config.heightScale, (float)(z + step) - offsetZ);
            glm::vec3 pos_br((float)(x + step) - offsetX, h_br * m_config.heightScale, (float)(z + step) - offsetZ);

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
                v1.texCoord = glm::vec2((float)x / TerrainConstants::TEXTURE_SCALE, (float)z / TerrainConstants::TEXTURE_SCALE);
                v1.height = h_tl;
                v1.waterMask = 0.0f;
                vertices.push_back(v1);

                // Vertex 2 (bottom-left)
                TerrainVertex v2;
                v2.position = pos_bl;
                v2.normal = normal;
                v2.texCoord = glm::vec2((float)x / TerrainConstants::TEXTURE_SCALE, (float)(z + 1) / TerrainConstants::TEXTURE_SCALE);
                v2.height = h_bl;
                v2.waterMask = 0.0f;
                vertices.push_back(v2);

                // Vertex 3 (top-right)
                TerrainVertex v3;
                v3.position = pos_tr;
                v3.normal = normal;
                v3.texCoord = glm::vec2((float)(x + 1) / TerrainConstants::TEXTURE_SCALE, (float)z / TerrainConstants::TEXTURE_SCALE);
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
                v1.texCoord = glm::vec2((float)(x + 1) / TerrainConstants::TEXTURE_SCALE, (float)z / TerrainConstants::TEXTURE_SCALE);
                v1.height = h_tr;
                v1.waterMask = 0.0f;
                vertices.push_back(v1);

                // Vertex 2 (bottom-left)
                TerrainVertex v2;
                v2.position = pos_bl;
                v2.normal = normal;
                v2.texCoord = glm::vec2((float)x / TerrainConstants::TEXTURE_SCALE, (float)(z + 1) / TerrainConstants::TEXTURE_SCALE);
                v2.height = h_bl;
                v2.waterMask = 0.0f;
                vertices.push_back(v2);

                // Vertex 3 (bottom-right)
                TerrainVertex v3;
                v3.position = pos_br;
                v3.normal = normal;
                v3.texCoord = glm::vec2((float)(x + 1) / TerrainConstants::TEXTURE_SCALE, (float)(z + 1) / TerrainConstants::TEXTURE_SCALE);
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
    for (int z = 0; z < m_config.height - step; z += step)
    {
        for (int x = 0; x < m_config.width - step; x += step)
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
                v1.texCoord = glm::vec2((float)x / TerrainConstants::TEXTURE_SCALE, (float)z / TerrainConstants::TEXTURE_SCALE);
                v1.height = TerrainConstants::SEA_LEVEL_NORMALIZED;
                v1.waterMask = 1.0f;
                vertices.push_back(v1);
                
                TerrainVertex v2;
                v2.position = pos_bl;
                v2.normal = normal;
                v2.texCoord = glm::vec2((float)x / TerrainConstants::TEXTURE_SCALE, (float)(z + step) / TerrainConstants::TEXTURE_SCALE);
                v2.height = TerrainConstants::SEA_LEVEL_NORMALIZED;
                v2.waterMask = 1.0f;
                vertices.push_back(v2);
                
                TerrainVertex v3;
                v3.position = pos_tr;
                v3.normal = normal;
                v3.texCoord = glm::vec2((float)(x + step) / TerrainConstants::TEXTURE_SCALE, (float)z / TerrainConstants::TEXTURE_SCALE);
                v3.height = TerrainConstants::SEA_LEVEL_NORMALIZED;
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
                v1.texCoord = glm::vec2((float)(x + step) / TerrainConstants::TEXTURE_SCALE, (float)z / TerrainConstants::TEXTURE_SCALE);
                v1.height = TerrainConstants::SEA_LEVEL_NORMALIZED;
                v1.waterMask = 1.0f;
                vertices.push_back(v1);
                
                TerrainVertex v2;
                v2.position = pos_bl;
                v2.normal = normal;
                v2.texCoord = glm::vec2((float)x / TerrainConstants::TEXTURE_SCALE, (float)(z + step) / TerrainConstants::TEXTURE_SCALE);
                v2.height = TerrainConstants::SEA_LEVEL_NORMALIZED;
                v2.waterMask = 1.0f;
                vertices.push_back(v2);
                
                TerrainVertex v3;
                v3.position = pos_br;
                v3.normal = normal;
                v3.texCoord = glm::vec2((float)(x + step) / TerrainConstants::TEXTURE_SCALE, (float)(z + step) / TerrainConstants::TEXTURE_SCALE);
                v3.height = TerrainConstants::SEA_LEVEL_NORMALIZED;
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
    float offsetX = m_config.width * 0.5f;
    float offsetZ = m_config.height * 0.5f;
    
    int gridX = (int)(x + offsetX);
    int gridZ = (int)(z + offsetZ);

    if (gridX < 0 || gridX >= m_config.width || gridZ < 0 || gridZ >= m_config.height)
        return 0.0f;

    return m_heightMap[gridX][gridZ] * m_config.heightScale;
}

float TerrainGenerator::getWaterMask(float x, float z)
{
    // Get the terrain height
    float height = getPerlinHeight(x, z);
    
    // Sea level - if terrain is below this, it's underwater
    const float seaLevel = TerrainConstants::SEA_LEVEL_NORMALIZED;
    
    // If terrain is above sea level, no water
    if (height >= seaLevel)
        return 0.0f;
    
    // Use noise to create large bodies of water only (no small puddles)
    float sampleX = x * TerrainConstants::WATER_NOISE_SCALE; // Even lower frequency for larger bodies
    float sampleZ = z * TerrainConstants::WATER_NOISE_SCALE;
    
    float waterNoise = stb_perlin_fbm_noise3(sampleX, sampleZ, 50.0f, 
                                             2.0f,   // lacunarity
                                             0.5f,   // gain
                                             4);     // more octaves for smoother areas
    waterNoise = (waterNoise + 1.0f) * 0.5f; // Normalize to 0-1
    
    // Much higher threshold - only large continuous areas get water
    if (waterNoise < TerrainConstants::WATER_AREA_THRESHOLD)
        return 0.0f;
    
    // Additionally check if terrain is significantly below sea level
    // This prevents tiny dips from becoming puddles
    float depth = seaLevel - height;
    if (depth < TerrainConstants::WATER_MIN_DEPTH) // Must be at least this deep below sea level
        return 0.0f;
    
    // Water exists here - return full strength for water plane
    return 1.0f;
}
