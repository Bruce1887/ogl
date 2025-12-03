#include "TerrainChunk.h"
#include "Error.h"

#include <cmath>
#include <iostream>

void addTriangle(std::vector<TerrainVertex> &vertices,
                 std::vector<unsigned int> &indices,
                 const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3,
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
}

std::unique_ptr<Chunk> TerrainChunkManager::generateNewChunk(const ChunkCoord &coord)
{
    std::vector<TerrainVertex> vertices;
    std::vector<unsigned int> indices;

    // Pre-allocate memory
    int quadsPerChunk = (m_chunkSize / m_vertexStep) * (m_chunkSize / m_vertexStep);
    vertices.reserve(quadsPerChunk * 6 + 6); // terrain + water plane
    indices.reserve(quadsPerChunk * 6 + 6);

    // Constants
    constexpr float heightScale = 100.0f;
    constexpr float seaLevel = 0.13f * heightScale + 0.1f;
    const int worldOffsetX = coord.x * m_chunkSize;
    const int worldOffsetZ = coord.z * m_chunkSize;

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
    for (int z = 0; z < m_chunkSize; z += m_vertexStep)
    {
        for (int x = 0; x < m_chunkSize; x += m_vertexStep)
        {
            int worldX = worldOffsetX + x;
            int worldZ = worldOffsetZ + z;

            // Sample heights at quad corners
            float h_tl = m_generator->getPerlinHeight((float)worldX, (float)worldZ);
            float h_tr = m_generator->getPerlinHeight((float)(worldX + m_vertexStep), (float)worldZ);
            float h_bl = m_generator->getPerlinHeight((float)worldX, (float)(worldZ + m_vertexStep));
            float h_br = m_generator->getPerlinHeight((float)(worldX + m_vertexStep), (float)(worldZ + m_vertexStep));

            // Calculate positions
            glm::vec3 pos_tl((float)worldX, h_tl * heightScale, (float)worldZ);
            glm::vec3 pos_tr((float)(worldX + m_vertexStep), h_tr * heightScale, (float)worldZ);
            glm::vec3 pos_bl((float)worldX, h_bl * heightScale, (float)(worldZ + m_vertexStep));
            glm::vec3 pos_br((float)(worldX + m_vertexStep), h_br * heightScale, (float)(worldZ + m_vertexStep));

            // Add two triangles for the quad
            addTriangle(pos_tl, pos_bl, pos_tr, h_tl, h_bl, h_tr, 0.0f);
            addTriangle(pos_tr, pos_bl, pos_br, h_tr, h_bl, h_br, 0.0f);
        }
    }

    // Generate water plane covering entire chunk
    {
        float minX = (float)worldOffsetX, maxX = (float)(worldOffsetX + m_chunkSize);
        float minZ = (float)worldOffsetZ, maxZ = (float)(worldOffsetZ + m_chunkSize);

        glm::vec3 pos_tl(minX, seaLevel, minZ);
        glm::vec3 pos_tr(maxX, seaLevel, minZ);
        glm::vec3 pos_bl(minX, seaLevel, maxZ);
        glm::vec3 pos_br(maxX, seaLevel, maxZ);

        addTriangle(pos_tl, pos_bl, pos_tr, 0.13f, 0.13f, 0.13f, 1.0f);
        addTriangle(pos_tr, pos_bl, pos_br, 0.13f, 0.13f, 0.13f, 1.0f);
    }

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

    // Populate chunk with trees

    for (int z = 0; z < m_chunkSize; z += m_vertexStep)
    {
        for (int x = 0; x < m_chunkSize; x += m_vertexStep)
        {
            float tree_perlin = m_generator->foo_treePerlin((float)(worldOffsetX + x), (float)(worldOffsetZ + z));

            if (tree_perlin > 0.3f) // threshold for tree placement
                continue;

            float worldX = worldOffsetX + x;
            float worldZ = worldOffsetZ + z;
            float y = m_generator->getPerlinHeight((float)worldX, (float)worldZ);
            y *= heightScale;
            // std::cout << "Placing tree at (" << (worldOffsetX + x) << ", " << y << ", " << (worldOffsetZ + z) << ")\n";s
            
            Model *granPtr = m_generator->m_terrainRenderables.gran.get();
            std::unique_ptr<Model> treeModel = std::make_unique<Model>(Model::copyFrom(granPtr));
            treeModel->setPosition(glm::vec3((float)(worldOffsetX + x), y, (float)(worldOffsetZ + z)));
            chunk->renderables_in_chunk.push_back(std::move(treeModel));
        }
    }

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

void TerrainChunkManager::updateChunks(const glm::vec3 &cameraPosition, float renderDistance)
{
    // Optimization: Only update chunks if camera moved significantly
    float distanceMoved = glm::distance(cameraPosition, m_lastCameraPosition);
    if (distanceMoved < m_updateThreshold)
    {
        return; // Skip update, we havent moved far enough
    }

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

    if (m_chunks.size() > m_gc_threshold)
    {
        garbageCollectChunks();
    }
}
