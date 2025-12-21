#pragma once

#include "Common.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"
#include "MeshRenderable.h"
#include "TerrainGenerator.h"
#include "../InstancedRenderer.h"
#include "Model.h"

#include <unordered_map>
#include <memory>

// Terrain chunk represents a subdivided portion of the terrain
struct ChunkCoord
{
    int x;
    int z;

    bool operator==(const ChunkCoord &other) const
    {
        return x == other.x && z == other.z;
    }
};

// Hash function for ChunkCoord to use in unordered_map
namespace std
{
    template <>
    struct hash<ChunkCoord>
    {
        size_t operator()(const ChunkCoord &coord) const
        {
            return hash<int>()(coord.x) ^ (hash<int>()(coord.z) << 1);
        }
    };
}

class Chunk : public Renderable
{
public:
    Chunk(ChunkCoord c, std::unique_ptr<MeshRenderable> tmr)
        : coord(c), terrain_mr(std::move(tmr)) {
          };
    ChunkCoord coord;
    std::vector<glm::vec3> treePositions;  // Store just positions, rendered via instancing
    std::unique_ptr<MeshRenderable> terrain_mr;                    // terrain and water

    std::vector<std::vector<float>> heightGrid; // stores unscaled perlin heights
    int gridSize = 0;                           // (chunkSize / vertexStep) + 1

    void render(glm::mat4 view, glm::mat4 projection, PhongLightConfig *phongLight) override
    {
        // Only render if active
        if (!m_active)
            return;

        // Trees are now rendered via instanced rendering in TerrainChunkManager
        terrain_mr->render(view, projection, phongLight);
    }

    bool isActive() const { return m_active; }
    void setActiveStatus(bool status) { m_active = status; }

    bool inBounds(const ChunkCoord &minCoord, const ChunkCoord &maxCoord) const
    {
        return coord.x >= minCoord.x && coord.x <= maxCoord.x &&
               coord.z >= minCoord.z && coord.z <= maxCoord.z;
    }

    float getPreciseHeightAt(float worldX, float worldZ, int chunkSize, int vertexStep) const;

private:
    // Active status indicates whether the chunk is currently in use and should be rendered.
    bool m_active = true;
};

class TerrainChunkManager
{
public:
    TerrainChunkManager(TerrainGenerator *generator, std::vector<std::shared_ptr<Texture>> terrainTextures)
        : m_generator(generator), m_terrainTextures(terrainTextures) {
            std::cout << "Initialized TerrainChunkManager with chunk size " << TC_CHUNK_SIZE << " and vertex step " << TC_VERTEX_STEP << std::endl;
            
            // Initialize instanced tree renderer
            m_treeRenderer = std::make_unique<InstancedRenderer>();
            m_treeRenderer->init(m_generator->m_terrainRenderables.gran.get());
          };

    ~TerrainChunkManager() = default;

    // Load/unload chunks based on camera position
    void updateChunks(const glm::vec3 &cameraPosition);    

    // Garbage collect unused chunks (chunnks that are not active)
    void garbageCollectChunks();

    std::vector<std::unique_ptr<Chunk>> m_chunks;
    void setShader(std::shared_ptr<Shader> shader) { m_terrainShader = shader; }

    void setFogUniforms(const glm::vec3 &fogColor, float fogStart, float fogEnd)
    {
        m_fogColor = fogColor;
        m_fogStart = fogStart;
        m_fogEnd = fogEnd;
        
        // Also set on tree renderer
        if (m_treeRenderer)
            m_treeRenderer->setFogUniforms(fogColor, fogStart, fogEnd);
    }

    float getPreciseHeightAt(float x, float z);
    
    // Render all trees using instanced rendering (call after rendering chunks)
    void renderTrees(const glm::mat4& view, const glm::mat4& projection, PhongLightConfig* light);

    // Render global water plane (call after terrain, before trees for proper transparency)
    void renderWater(const glm::mat4& view, const glm::mat4& projection, PhongLightConfig* light, const glm::vec3& cameraPosition, float renderDistance);

private:
    TerrainGenerator *m_generator;
    
    glm::vec3 m_lastCameraPosition = glm::vec3(0.0f);

    std::shared_ptr<Shader> m_terrainShader;                 // Reference to shader (not owned)
    std::vector<std::shared_ptr<Texture>> m_terrainTextures; // Textures for terrain rendering

    // Fog parameters
    glm::vec3 m_fogColor = glm::vec3(0.0f);
    float m_fogStart = 0.0f;
    float m_fogEnd = 0.0f;

    // Instanced tree renderer
    std::unique_ptr<InstancedRenderer> m_treeRenderer;
    bool m_treesNeedUpdate = true;

    // Global water plane (single mesh to avoid seams between chunks)
    std::unique_ptr<MeshRenderable> m_waterMesh;

    // Optimization: track last camera position to avoid redundant updates

    // Generate a single chunk
    std::unique_ptr<Chunk> generateNewChunk(const ChunkCoord &coord);

    // Load a chunk
    void loadChunk(const ChunkCoord &coord);

    // Get which chunk (its coordinates) a world coordinate belongs to
    ChunkCoord worldToChunk(const glm::vec3 &worldPos) const
    {
        ChunkCoord coord;
        coord.x = static_cast<int>(std::floor(worldPos.x / TC_CHUNK_SIZE));
        coord.z = static_cast<int>(std::floor(worldPos.z / TC_CHUNK_SIZE));
        return coord;
    }
};
