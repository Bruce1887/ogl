#pragma once

#include "Common.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"
#include "MeshRenderable.h"
#include "TerrainGenerator.h"
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
    std::vector<std::unique_ptr<Renderable>> renderables_in_chunk; // trees etc.
    std::unique_ptr<MeshRenderable> terrain_mr;   // terrain and water

    std::vector<std::vector<float>> heightGrid; // stores unscaled perlin heights
    int gridSize = 0; // (chunkSize / vertexStep) + 1



    void render(glm::mat4 view, glm::mat4 projection, PhongLightConfig *phongLight) override
    {
        // Only render if active
        if (!m_active)
            return;

        for (auto &r : renderables_in_chunk)
        {
            r->render(view, projection, phongLight);
        }
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
    TerrainChunkManager(TerrainGenerator *generator, int chunkSize, int vertexStep, std::vector<std::shared_ptr<Texture>> terrainTextures, int gc_threshold)
        : m_generator(generator), m_chunkSize(chunkSize), m_vertexStep(vertexStep), m_updateThreshold(8.0f), m_gc_threshold(gc_threshold), m_terrainTextures(terrainTextures) {
          };

    ~TerrainChunkManager() = default;
    
    // Load/unload chunks based on camera position
    void updateChunks(const glm::vec3 &cameraPosition, float renderDistance);

    // Get all currently loaded chunks
    // const std::unordered_map<ChunkCoord, std::shared_ptr<MeshRenderable>> &getLoadedChunks() const { return m_loadedChunks; }

    // Get chunk size
    int getChunkSize() const { return m_chunkSize; }

    // Garbage collect unused chunks (chunnks that are not active)
    void garbageCollectChunks();

    std::vector<std::unique_ptr<Chunk>> m_chunks;
    void setShader(std::shared_ptr<Shader> shader) { m_terrainShader = shader; }


    float getPreciseHeightAt(float x, float z);

private:
    TerrainGenerator *m_generator;

    int m_chunkSize;         // Size of each chunk in grid units (e.g., 32x32)
    int m_vertexStep;        // Spacing between vertices for LOD
    float m_updateThreshold; // Only update chunks when camera moves this far
    int m_gc_threshold;      // Number of chunks before triggering garbage collection

    glm::vec3 m_lastCameraPosition = glm::vec3(0.0f);

    std::shared_ptr<Shader> m_terrainShader;                 // Reference to shader (not owned)
    std::vector<std::shared_ptr<Texture>> m_terrainTextures; // Textures for terrain rendering

    // Optimization: track last camera position to avoid redundant updates

    // Generate a single chunk
    std::unique_ptr<Chunk> generateNewChunk(const ChunkCoord &coord);

    // Load a chunk
    void loadChunk(const ChunkCoord &coord);

    // Get which chunk (its coordinates) a world coordinate belongs to
    ChunkCoord worldToChunk(const glm::vec3 &worldPos) const
    {
        ChunkCoord coord;
        coord.x = static_cast<int>(std::floor(worldPos.x / m_chunkSize));
        coord.z = static_cast<int>(std::floor(worldPos.z / m_chunkSize));
        return coord;
    }
};
