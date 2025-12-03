#include "TerrainChunk.h"
#include "Error.h"

#include <cmath>
#include <iostream>

ChunkCoord TerrainChunkManager::worldToChunk(const glm::vec3 &worldPos) const
{
    ChunkCoord coord;
    coord.x = static_cast<int>(std::floor(worldPos.x / m_chunkSize));
    coord.z = static_cast<int>(std::floor(worldPos.z / m_chunkSize));
    return coord;
}

std::unique_ptr<Chunk> TerrainChunkManager::generateChunk(const ChunkCoord &coord)
{
    std::vector<TerrainVertex> vertices;
    std::vector<unsigned int> indices;

    // Optimization: Pre-allocate memory to avoid reallocations
    int quadsPerChunk = (m_chunkSize / m_vertexStep) * (m_chunkSize / m_vertexStep);
    vertices.reserve(quadsPerChunk * 6); // 6 vertices per quad (2 triangles, flat shading)
    indices.reserve(quadsPerChunk * 6);  // 6 indices per quad

    // Calculate world offset for this chunk
    int worldOffsetX = coord.x * m_chunkSize;
    int worldOffsetZ = coord.z * m_chunkSize;

    // Generate vertices for this chunk using flat shading with vertex stepping
    // Note: We iterate up to but NOT including m_chunkSize to avoid overlap with next chunk
    // Each chunk goes from [0, chunkSize) in local coordinates
    for (int z = 0; z < m_chunkSize; z += m_vertexStep)
    {
        for (int x = 0; x < m_chunkSize; x += m_vertexStep)
        {
            int worldX = worldOffsetX + x;
            int worldZ = worldOffsetZ + z;

            // Sample heights at the four corners of the quad
            float h_tl = m_generator->getPerlinHeight((float)worldX, (float)worldZ);
            float h_tr = m_generator->getPerlinHeight((float)(worldX + m_vertexStep), (float)worldZ);
            float h_bl = m_generator->getPerlinHeight((float)worldX, (float)(worldZ + m_vertexStep));
            float h_br = m_generator->getPerlinHeight((float)(worldX + m_vertexStep), (float)(worldZ + m_vertexStep));

            // Get water masks
            float w_tl = m_generator->getWaterMask((float)worldX, (float)worldZ);
            float w_tr = m_generator->getWaterMask((float)(worldX + m_vertexStep), (float)worldZ);
            float w_bl = m_generator->getWaterMask((float)worldX, (float)(worldZ + m_vertexStep));
            float w_br = m_generator->getWaterMask((float)(worldX + m_vertexStep), (float)(worldZ + m_vertexStep));

            // Get height scale from config (must match terraintest!)
            float heightScale = 100.0f; // Match terraintest.cpp config

            // Sea level in world units
            const float seaLevel = TerrainConstants::SEA_LEVEL_OFFSET * heightScale;

            // Calculate positions - always use terrain height for terrain mesh
            glm::vec3 pos_tl((float)worldX, h_tl * heightScale, (float)worldZ);
            glm::vec3 pos_tr((float)(worldX + m_vertexStep), h_tr * heightScale, (float)worldZ);
            glm::vec3 pos_bl((float)worldX, h_bl * heightScale, (float)(worldZ + m_vertexStep));
            glm::vec3 pos_br((float)(worldX + m_vertexStep), h_br * heightScale, (float)(worldZ + m_vertexStep));

            // First triangle (flat shading)
            {
                glm::vec3 edge1 = pos_bl - pos_tl;
                glm::vec3 edge2 = pos_tr - pos_tl;
                glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

                unsigned int baseIdx = vertices.size();

                TerrainVertex v1;
                v1.position = pos_tl;
                v1.normal = normal;
                v1.texCoord = glm::vec2((float)worldX / TerrainConstants::TEXTURE_SCALE, (float)worldZ / TerrainConstants::TEXTURE_SCALE);
                v1.height = h_tl;
                v1.waterMask = 0.0f;
                vertices.push_back(v1);

                TerrainVertex v2;
                v2.position = pos_bl;
                v2.normal = normal;
                v2.texCoord = glm::vec2((float)worldX / TerrainConstants::TEXTURE_SCALE, (float)(worldZ + 1) / TerrainConstants::TEXTURE_SCALE);
                v2.height = h_bl;
                v2.waterMask = 0.0f;
                vertices.push_back(v2);

                TerrainVertex v3;
                v3.position = pos_tr;
                v3.normal = normal;
                v3.texCoord = glm::vec2((float)(worldX + 1) / TerrainConstants::TEXTURE_SCALE, (float)worldZ / TerrainConstants::TEXTURE_SCALE);
                v3.height = h_tr;
                v3.waterMask = 0.0f;
                vertices.push_back(v3);

                indices.push_back(baseIdx);
                indices.push_back(baseIdx + 1);
                indices.push_back(baseIdx + 2);
            }

            // Second triangle
            {
                glm::vec3 edge1 = pos_bl - pos_tr;
                glm::vec3 edge2 = pos_br - pos_tr;
                glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

                unsigned int baseIdx = vertices.size();

                TerrainVertex v1;
                v1.position = pos_tr;
                v1.normal = normal;
                v1.texCoord = glm::vec2((float)(worldX + 1) / TerrainConstants::TEXTURE_SCALE, (float)worldZ / TerrainConstants::TEXTURE_SCALE);
                v1.height = h_tr;
                v1.waterMask = 0.0f;
                vertices.push_back(v1);

                TerrainVertex v2;
                v2.position = pos_bl;
                v2.normal = normal;
                v2.texCoord = glm::vec2((float)worldX / TerrainConstants::TEXTURE_SCALE, (float)(worldZ + 1) / TerrainConstants::TEXTURE_SCALE);
                v2.height = h_bl;
                v2.waterMask = 0.0f;
                vertices.push_back(v2);

                TerrainVertex v3;
                v3.position = pos_br;
                v3.normal = normal;
                v3.texCoord = glm::vec2((float)(worldX + 1) / TerrainConstants::TEXTURE_SCALE, (float)(worldZ + 1) / TerrainConstants::TEXTURE_SCALE);
                v3.height = h_br;
                v3.waterMask = 0.0f;
                vertices.push_back(v3);

                indices.push_back(baseIdx);
                indices.push_back(baseIdx + 1);
                indices.push_back(baseIdx + 2);
            }
        }
    }

    // Generate single water plane for entire chunk at sea level (only 6 vertices!)
    {
        float heightScale = 100.0f;
        const float seaLevel = TerrainConstants::SEA_LEVEL_NORMALIZED * heightScale + TerrainConstants::WATER_PLANE_OFFSET; // Slightly higher than terrain sea level
        glm::vec3 normal(0.0f, 1.0f, 0.0f);

        // Calculate chunk corners in world space
        float chunkMinX = (float)worldOffsetX;
        float chunkMaxX = (float)(worldOffsetX + m_chunkSize);
        float chunkMinZ = (float)worldOffsetZ;
        float chunkMaxZ = (float)(worldOffsetZ + m_chunkSize);

        // Create two triangles covering the entire chunk
        glm::vec3 pos_tl(chunkMinX, seaLevel, chunkMinZ);
        glm::vec3 pos_tr(chunkMaxX, seaLevel, chunkMinZ);
        glm::vec3 pos_bl(chunkMinX, seaLevel, chunkMaxZ);
        glm::vec3 pos_br(chunkMaxX, seaLevel, chunkMaxZ);

        // First triangle
        unsigned int baseIdx = vertices.size();

        TerrainVertex v1;
        v1.position = pos_tl;
        v1.normal = normal;
        v1.texCoord = glm::vec2(chunkMinX / TerrainConstants::TEXTURE_SCALE, chunkMinZ / TerrainConstants::TEXTURE_SCALE);
        v1.height = TerrainConstants::SEA_LEVEL_NORMALIZED;
        v1.waterMask = 1.0f;
        vertices.push_back(v1);

        TerrainVertex v2;
        v2.position = pos_bl;
        v2.normal = normal;
        v2.texCoord = glm::vec2(chunkMinX / TerrainConstants::TEXTURE_SCALE, chunkMaxZ / TerrainConstants::TEXTURE_SCALE);
        v2.height = TerrainConstants::SEA_LEVEL_NORMALIZED;
        v2.waterMask = 1.0f;
        vertices.push_back(v2);

        TerrainVertex v3;
        v3.position = pos_tr;
        v3.normal = normal;
        v3.texCoord = glm::vec2(chunkMaxX / TerrainConstants::TEXTURE_SCALE, chunkMinZ / TerrainConstants::TEXTURE_SCALE);
        v3.height = TerrainConstants::SEA_LEVEL_NORMALIZED;
        v3.waterMask = 1.0f;
        vertices.push_back(v3);

        indices.push_back(baseIdx);
        indices.push_back(baseIdx + 1);
        indices.push_back(baseIdx + 2);

        // Second triangle
        baseIdx = vertices.size();

        TerrainVertex v4;
        v4.position = pos_tr;
        v4.normal = normal;
        v4.texCoord = glm::vec2(chunkMaxX / TerrainConstants::TEXTURE_SCALE, chunkMinZ / TerrainConstants::TEXTURE_SCALE);
        v4.height = TerrainConstants::SEA_LEVEL_NORMALIZED;
        v4.waterMask = 1.0f;
        vertices.push_back(v4);

        TerrainVertex v5;
        v5.position = pos_bl;
        v5.normal = normal;
        v5.texCoord = glm::vec2(chunkMinX / TerrainConstants::TEXTURE_SCALE, chunkMaxZ / TerrainConstants::TEXTURE_SCALE);
        v5.height = TerrainConstants::SEA_LEVEL_NORMALIZED;
        v5.waterMask = 1.0f;
        vertices.push_back(v5);

        TerrainVertex v6;
        v6.position = pos_br;
        v6.normal = normal;
        v6.texCoord = glm::vec2(chunkMaxX / TerrainConstants::TEXTURE_SCALE, chunkMaxZ / TerrainConstants::TEXTURE_SCALE);
        v6.height = TerrainConstants::SEA_LEVEL_NORMALIZED;
        v6.waterMask = 1.0f;
        vertices.push_back(v6);

        indices.push_back(baseIdx);
        indices.push_back(baseIdx + 1);
        indices.push_back(baseIdx + 2);
    }

#ifdef DEBUG
    DEBUG_PRINT("Generated chunk at (" << coord.x << ", " << coord.z << ") with " << vertices.size() << " vertices and " << indices.size() << " indices.");
#endif

    // Create OpenGL buffers
    auto va_ptr = std::make_unique<VertexArray>();
    auto vb_ptr = std::make_unique<VertexBuffer>(vertices.data(), vertices.size() * sizeof(TerrainVertex), va_ptr.get());

    VertexBufferLayout layout;
    layout.push<float>(3); // position
    layout.push<float>(3); // normal
    layout.push<float>(2); // texCoord
    layout.push<float>(1); // height
    layout.push<float>(1); // waterMask

    va_ptr->addBuffer(vb_ptr.get(), layout);
    auto ibo_ptr = std::make_unique<IndexBuffer>(indices.data(), indices.size());

    auto mesh_ptr = std::make_shared<Mesh>(std::move(va_ptr), std::move(vb_ptr), std::move(ibo_ptr));

    std::unique_ptr<MeshRenderable> chunkTerrain_mr = std::make_unique<MeshRenderable>(mesh_ptr, m_terrainShader);
    chunkTerrain_mr->m_textureReferences = m_terrainTextures;
    
    std::unique_ptr<Chunk> chunk_ptr = std::make_unique<Chunk>();
    chunk_ptr->coord = coord;
    chunk_ptr->terrain_mr = std::move(chunkTerrain_mr);
    return chunk_ptr;
}

void TerrainChunkManager::loadChunk(const ChunkCoord &coord)
{
    // Check if chunk already exists. Dont load again
    for (auto &c : m_chunks)
    {
        if (c->coord == coord)
        {
            // Activate
            c->setActiveStatus(true);
            return;
        }
    }

    // Generate new chunk
    std::unique_ptr<Chunk> chunk = generateChunk(coord);

    m_chunks.push_back(std::move(chunk));

    /*
    // Create renderable
    if (m_terrainShader)
    {
        std::shared_ptr<MeshRenderable> renderable = std::make_shared<MeshRenderable>(mesh, m_terrainShader);
        renderable->setTransform(glm::mat4(1.0f));
        m_loadedChunks[coord] = renderable;

        // Reduced debug output - only print occasionally to reduce spam
        static int loadCount = 0;
        if (++loadCount % 10 == 0)
        {
            std::cout << "[TerrainChunk] Loaded " << loadCount << " chunks total" << std::endl;
        }
    }
    */
}

/*
void TerrainChunkManager::unloadChunk(const ChunkCoord &coord)
{
    // Remove renderable
    for (auto &c : m_chunks)
    {
        if (c->coord == coord)
        {
            c->deactivate();
            return;
        }
    }
}
*/

void TerrainChunkManager::garbageCollectChunks()
{
#ifdef DEBUG
    DEBUG_PRINT("Garbage collecting chunks. Before: " << m_chunks.size() << " chunks.");
#endif
    m_chunks.erase(std::remove_if(m_chunks.begin(), m_chunks.end(),
                                  [](const std::unique_ptr<Chunk> &chunk)
                                  { return !chunk->isActive(); }),
                   m_chunks.end());

#ifdef DEBUG
    DEBUG_PRINT("After: " << m_chunks.size() << " chunks.");
#endif
}

void TerrainChunkManager::updateChunks(const glm::vec3 &cameraPosition, float renderDistance)
{

    // Optimization: Only update chunks if camera moved significantly
    /*
    float distanceMoved = glm::distance(cameraPosition, m_lastCameraPosition);
    if (distanceMoved < m_updateThreshold)
    {
        return; // Skip update, we havent moved far enough
    }
    */

    m_lastCameraPosition = cameraPosition;

    // Calculate which chunks should be loaded based on camera position
    ChunkCoord cameraChunk = worldToChunk(cameraPosition);

    int chunkRadius = static_cast<int>(std::ceil(renderDistance / m_chunkSize));

    ChunkCoord minCoord = {cameraChunk.x - chunkRadius, cameraChunk.z - chunkRadius};
    ChunkCoord maxCoord = {cameraChunk.x + chunkRadius, cameraChunk.z + chunkRadius};

    // Load all chunks in the range
    for (ChunkCoord c = minCoord; c.x <= maxCoord.x; c.x++)
    {
        for (c.z = minCoord.z; c.z <= maxCoord.z; c.z++)
        {
            loadChunk(c);
        }
    }

    // deactivate unloaded chunks (and activate loaded ones, but that has no effect as they should already be active)
    for (auto &chunkPtr : m_chunks)
    {
        bool inBounds = chunkPtr->inBounds(minCoord, maxCoord);
        chunkPtr->setActiveStatus(inBounds);
    }

    if (m_chunks.size() > 50)
    {
        garbageCollectChunks();
    }

    /*
    // Set to track which chunks should be loaded
    std::unordered_map<ChunkCoord, bool> shouldBeLoaded;

    // Mark chunks that should be loaded
    for (int z = -chunkRadius; z <= chunkRadius; z++)
    {
        for (int x = -chunkRadius; x <= chunkRadius; x++)
        {
            ChunkCoord coord;
            coord.x = cameraChunk.x + x;
            coord.z = cameraChunk.z + z;

            // Calculate distance to chunk center
            float chunkCenterX = (coord.x + 0.5f) * m_chunkSize;
            float chunkCenterZ = (coord.z + 0.5f) * m_chunkSize;
            float distanceToChunk = glm::distance(
                glm::vec2(cameraPosition.x, cameraPosition.z),
                glm::vec2(chunkCenterX, chunkCenterZ));

            if (distanceToChunk <= renderDistance)
            {
                shouldBeLoaded[coord] = true;
                loadChunk(coord);
            }
        }
    }

    // Unload chunks that are too far
    std::vector<ChunkCoord> chunksToUnload;
    for (const auto &pair : m_loadedChunks)
    {
        if (shouldBeLoaded.find(pair.first) == shouldBeLoaded.end())
        {
            chunksToUnload.push_back(pair.first);
        }
    }

    for (const auto &coord : chunksToUnload)
    {
        unloadChunk(coord);
    }
    */
}
