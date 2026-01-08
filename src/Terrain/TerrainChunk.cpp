#include "TerrainChunk.h"
#include "Error.h"

#include <cmath>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

std::unique_ptr<Chunk> TerrainChunkManager::generateNewChunk(const ChunkCoord &coord)
{
    std::vector<TerrainVertex> vertices;
    std::vector<unsigned int> indices;

    // Pre-allocate memory
    vertices.reserve(TC_CELLS_PER_CHUNK * 6 + 6); // terrain + water plane
    indices.reserve(TC_CELLS_PER_CHUNK * 6 + 6);

    // Constants
    constexpr float heightScale = 100.0f;
    constexpr float seaLevel = 0.13f * heightScale + 0.1f;
    const int worldOffsetX = coord.x * TC_CHUNK_SIZE;
    const int worldOffsetZ = coord.z * TC_CHUNK_SIZE;

    // Helper to add a triangle with flat shading
    auto addTriangle = [&](const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3,
                           float h1, float h2, float h3, float waterMask)
    {
        glm::vec3 normal = glm::normalize(glm::cross(p2 - p1, p3 - p1));
        unsigned int baseIdx = vertices.size();

        auto addVertex = [&](const glm::vec3 &pos, float h)
        {
            vertices.push_back({pos, normal,
                                glm::vec2(pos.x / 10.0f, pos.z / 10.0f),
                                h, waterMask});
        };

        addVertex(p1, h1);
        addVertex(p2, h2);
        addVertex(p3, h3);

        indices.insert(indices.end(), {baseIdx, baseIdx + 1, baseIdx + 2});
    };

    // Generate terrain vertices
    for (int z = 0; z < TC_CHUNK_SIZE; z += TC_VERTEX_STEP)
    {
        for (int x = 0; x < TC_CHUNK_SIZE; x += TC_VERTEX_STEP)
        {
            int worldX = worldOffsetX + x;
            int worldZ = worldOffsetZ + z;

            // Sample heights at quad corners
            float h_tl = m_generator->getPerlinHeight((float)worldX, (float)worldZ);
            float h_tr = m_generator->getPerlinHeight((float)(worldX + TC_VERTEX_STEP), (float)worldZ);
            float h_bl = m_generator->getPerlinHeight((float)worldX, (float)(worldZ + TC_VERTEX_STEP));
            float h_br = m_generator->getPerlinHeight((float)(worldX + TC_VERTEX_STEP), (float)(worldZ + TC_VERTEX_STEP));

            // Calculate positions
            glm::vec3 pos_tl((float)worldX, h_tl * heightScale, (float)worldZ);
            glm::vec3 pos_tr((float)(worldX + TC_VERTEX_STEP), h_tr * heightScale, (float)worldZ);
            glm::vec3 pos_bl((float)worldX, h_bl * heightScale, (float)(worldZ + TC_VERTEX_STEP));
            glm::vec3 pos_br((float)(worldX + TC_VERTEX_STEP), h_br * heightScale, (float)(worldZ + TC_VERTEX_STEP));

            // Add two triangles for the quad
            addTriangle(pos_tl, pos_bl, pos_tr, h_tl, h_bl, h_tr, 0.0f);
            addTriangle(pos_tr, pos_bl, pos_br, h_tr, h_bl, h_br, 0.0f);
        }
    }

    // Water is now rendered globally by TerrainChunkManager to avoid seams

    // Create meshrenderable
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
    auto chunkTerrain_mr = std::make_unique<MeshRenderable>(mesh_ptr, m_terrainShader);
    chunkTerrain_mr->m_textureReferences = m_terrainTextures;

    // Create chunk
    std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>(coord, std::move(chunkTerrain_mr));

    
    chunk->heightGrid.assign(TC_VERTICES_PER_AXIS, std::vector<float>(TC_VERTICES_PER_AXIS));

    for (int gz = 0; gz < TC_VERTICES_PER_AXIS; gz++)
    {
        for (int gx = 0; gx < TC_VERTICES_PER_AXIS; gx++)
        {
            float worldX = worldOffsetX + gx * TC_VERTEX_STEP;
            float worldZ = worldOffsetZ + gz * TC_VERTEX_STEP;

            // EXACT match to mesh vertex samples
            chunk->heightGrid[gz][gx] = m_generator->getPerlinHeight(worldX, worldZ);
        }
    }

    // Populate chunk with tree positions (for instanced rendering)

    for (int z = 0; z < TC_CHUNK_SIZE; z += TC_VERTEX_STEP)
    {
        for (int x = 0; x < TC_CHUNK_SIZE; x += TC_VERTEX_STEP)
        {
            float tree_perlin = m_generator->foo_treePerlin((float)(worldOffsetX + x), (float)(worldOffsetZ + z));

            if (tree_perlin > 0.3f) // threshold for tree placement
                continue;

            float worldX = worldOffsetX + x;
            float worldZ = worldOffsetZ + z;
            float y = m_generator->getPerlinHeight((float)worldX, (float)worldZ);
            y *= heightScale;
            
            // Don't place trees below or at sea level (in water)
            if (y <= seaLevel)
                continue;
            
            // Just store the position - trees will be rendered via instancing
            chunk->treePositions.push_back(glm::vec3((float)worldX, y, (float)worldZ));
        }
    }
    
    // Mark trees as needing update
    m_treesNeedUpdate = true;

    return chunk;
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
    std::unique_ptr<Chunk> chunk = generateNewChunk(coord);

    m_chunks.push_back(std::move(chunk));
}

void TerrainChunkManager::garbageCollectChunks()
{

#ifdef DEBUG
    size_t before = m_chunks.size();
#endif
    m_chunks.erase(std::remove_if(m_chunks.begin(), m_chunks.end(),
                                  [](const std::unique_ptr<Chunk> &chunk)
                                  { return !chunk->isActive(); }),
                   m_chunks.end());
#ifdef DEBUG
    DEBUG_PRINT("Garbage collecting chunks. Before: " << before << " chunks. After: " << m_chunks.size() << " chunks.");
#endif
}

void TerrainChunkManager::updateChunks(const glm::vec3 &cameraPosition)
{
    // Optimization: Only update chunks if camera moved significantly
    float distanceMoved = glm::distance(cameraPosition, m_lastCameraPosition);
    if (distanceMoved < TC_UPDATE_THRESHOLD)
    {
        return; // Skip update, we havent moved far enough
    }

    m_lastCameraPosition = cameraPosition;

    // Calculate which chunks should be loaded based on camera position
    ChunkCoord cameraChunk = worldToChunk(cameraPosition);

    int chunkRadius = static_cast<int>(std::ceil(TC_RENDER_DISTANCE / TC_CHUNK_SIZE)); // TODO: make this a constant? since it is defined by two constants?

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
    bool anyStateChanged = false;
    for (auto &chunkPtr : m_chunks)
    {
        bool inBounds = chunkPtr->inBounds(minCoord, maxCoord);
        if (chunkPtr->isActive() != inBounds)
        {
            anyStateChanged = true;
        }
        chunkPtr->setActiveStatus(inBounds);
    }
    
    // Mark trees for update if any chunk state changed
    if (anyStateChanged)
    {
        m_treesNeedUpdate = true;
    }

    if (m_chunks.size() > TC_GC_THRESHOLD)
    {
        garbageCollectChunks();
        m_treesNeedUpdate = true;  // Trees need update after GC
    }
}


float Chunk::getPreciseHeightAt(float worldX, float worldZ, int chunkSize, int vertexStep) const
{
    // Convert world → local chunk coordinates
    float localX = worldX - coord.x * chunkSize;
    float localZ = worldZ - coord.z * chunkSize;

    // If completely out of range, bail out
    if (localX < 0 || localZ < 0 ||
        localX > chunkSize || localZ > chunkSize)
        return 0.0f;

    // Compute raw grid indices
    int raw_i = floor(localX / vertexStep);
    int raw_j = floor(localZ / vertexStep);

    // --- FIX 1: Clamp so that (i+1) and (j+1) are valid ---------------------
    int maxIndex = (chunkSize / vertexStep) - 1; // valid quads are 0..maxIndex

    int i = std::clamp(raw_i, 0, maxIndex);
    int j = std::clamp(raw_j, 0, maxIndex);

    // Fractions inside the quad
    float fx = (localX - i * vertexStep) / vertexStep;
    float fz = (localZ - j * vertexStep) / vertexStep;

    // --- FIX 2: If clamping caused us to be exactly on border,
    //            shift inside 1mm to avoid reading missing border vertices ----
    if (raw_i != i) fx = 0.999f;
    if (raw_j != j) fz = 0.999f;

    // --- Fetch quad heights (these are guaranteed valid now) ---------------
    float h00 = heightGrid[j][i];
    float h10 = heightGrid[j][i + 1];
    float h01 = heightGrid[j + 1][i];
    float h11 = heightGrid[j + 1][i + 1];

    // --- FIX 3: Detect "border zeros" and re-sample from neighbor chunk ----
    bool zeroRow = (h00 == 0 && h01 == 0);
    bool zeroCol = (h00 == 0 && h10 == 0);

        // Border is invalid — push sampling slightly inward
    if (zeroRow || zeroCol)
    {
        // Adjust fx / fz inward so we never hit missing vertices
        fx = std::min(fx, 0.99f);
        fz = std::min(fz, 0.99f);

        // (Optional) clamp heights to avoid 0s
        h00 = std::max(h00, 0.0001f);
        h10 = std::max(h10, 0.0001f);
        h01 = std::max(h01, 0.0001f);
        h11 = std::max(h11, 0.0001f);
    }


    // --- Standard interpolation using correct diagonal ---------------------
    float h;
    if (fx + fz < 1.0f)
    {
        // TL–BL–TR
        h = h00 * (1 - fx - fz)
          + h01 * fz
          + h10 * fx;
    }
    else
    {
        // TR–BL–BR
        h = h10 * (1 - fz)
          + h11 * (fx + fz - 1)
          + h01 * (1 - fx);
    }

    return h * 100.0f; // your terrain heightScale
}

float TerrainChunkManager::getPreciseHeightAt(float x, float z)
{
    ChunkCoord cc = worldToChunk(glm::vec3(x,0,z));

    for (auto &chunk : m_chunks)
    {
        if (chunk->coord.x == cc.x && chunk->coord.z == cc.z)
        {
            return chunk->getPreciseHeightAt(x, z, TC_CHUNK_SIZE, TC_VERTEX_STEP);
        }
    }

    loadChunk(cc);

    // Now find it again
    for (auto &chunk : m_chunks)
    {
        if (chunk->coord.x == cc.x && chunk->coord.z == cc.z)
        {
            return chunk->getPreciseHeightAt(x, z, TC_CHUNK_SIZE, TC_VERTEX_STEP);
        }
    }

    return 0.0f;
}

void TerrainChunkManager::renderTrees(const glm::mat4& view, const glm::mat4& projection, PhongLightConfig* light)
{
    if (!m_treeRenderer)
        return;

    // Rebuild instance data if chunks changed
    if (m_treesNeedUpdate)
    {
        m_treeRenderer->clearInstances();
        
        for (const auto& chunk : m_chunks)
        {
            if (!chunk->isActive())
                continue;
                
            for (const auto& pos : chunk->treePositions)
            {
                // Add some random-ish rotation based on position for variety
                float rotation = std::fmod(pos.x * 17.3f + pos.z * 31.7f, 360.0f);
                m_treeRenderer->addInstance(pos, 1.0f, rotation);
            }
        }
        
        m_treeRenderer->uploadInstanceData();
        m_treesNeedUpdate = false;
    }

    m_treeRenderer->render(view, projection, light);
}

void TerrainChunkManager::renderWater(const glm::mat4& view, const glm::mat4& projection, PhongLightConfig* light, const glm::vec3& cameraPosition, float renderDistance)
{
    if (!m_terrainShader)
        return;

    constexpr float seaLevel = 0.13f * 100.0f + 0.1f;
    
    // Single water layer that covers the same area as terrain
    // Fog is applied identically to water and terrain in the shader
    float waterSize = renderDistance;  // Same as terrain render distance
    
    // Create a subdivided water grid so fog interpolates correctly
    // (A simple 4-vertex quad would have fog interpolated from corners only)
    const int gridSize = 10;  // 10x10 grid = 121 vertices
    const float step = (waterSize * 2.0f) / gridSize;
    
    glm::vec3 normal(0.0f, 1.0f, 0.0f);
    
    std::vector<TerrainVertex> vertices;
    vertices.reserve((gridSize + 1) * (gridSize + 1));
    
    for (int z = 0; z <= gridSize; z++)
    {
        for (int x = 0; x <= gridSize; x++)
        {
            float worldX = cameraPosition.x - waterSize + x * step;
            float worldZ = cameraPosition.z - waterSize + z * step;
            
            vertices.push_back({
                {worldX, seaLevel, worldZ},
                normal,
                {worldX / 10.0f, worldZ / 10.0f},
                0.13f,
                1.0f  // waterMask = 1.0 for textured water
            });
        }
    }
    
    std::vector<unsigned int> indices;
    indices.reserve(gridSize * gridSize * 6);
    
    for (int z = 0; z < gridSize; z++)
    {
        for (int x = 0; x < gridSize; x++)
        {
            int topLeft = z * (gridSize + 1) + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * (gridSize + 1) + x;
            int bottomRight = bottomLeft + 1;
            
            // Two triangles per quad
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
    
    auto va_ptr = std::make_unique<VertexArray>();
    auto vb_ptr = std::make_unique<VertexBuffer>(vertices.data(), vertices.size() * sizeof(TerrainVertex), va_ptr.get());
    
    VertexBufferLayout layout;
    layout.push<float>(3);
    layout.push<float>(3);
    layout.push<float>(2);
    layout.push<float>(1);
    layout.push<float>(1);
    va_ptr->addBuffer(vb_ptr.get(), layout);
    
    auto ibo_ptr = std::make_unique<IndexBuffer>(indices.data(), indices.size());
    auto mesh_ptr = std::make_shared<Mesh>(std::move(va_ptr), std::move(vb_ptr), std::move(ibo_ptr));
    
    m_waterMesh = std::make_unique<MeshRenderable>(mesh_ptr, m_terrainShader);
    m_waterMesh->m_textureReferences = m_terrainTextures;
    m_waterMesh->setUniform("u_fogColor", m_fogColor);
    m_waterMesh->setUniform("u_fogStart", m_fogStart);
    m_waterMesh->setUniform("u_fogEnd", m_fogEnd);
    
    m_terrainShader->bind();
    for (size_t i = 0; i < m_terrainTextures.size(); i++)
    {
        m_terrainTextures[i]->bindNew(static_cast<GLuint>(i));
        m_terrainShader->setUniform(m_terrainTextures[i]->m_targetUniform, static_cast<int>(i));
    }
    
    m_waterMesh->render(view, projection, light);
}

void TerrainChunkManager::collectNearbyObstacles(
    const glm::vec3& pos,
    float range,
    std::vector<StaticObstacle>& out) const
{
    float rangeSq = range * range;
    constexpr float TREE_RADIUS = 1.0f;

    for (const auto& chunk : m_chunks)
    {
        if (!chunk->isActive())
            continue;

        for (const glm::vec3& p : chunk->treePositions)
        {
            glm::vec2 d = glm::vec2(p.x, p.z) - glm::vec2(pos.x, pos.z);
            if (glm::dot(d, d) <= rangeSq)
            {
                out.push_back({
                    glm::vec2(p.x, p.z),
                    TREE_RADIUS
                });
            }
        }
    }
}
