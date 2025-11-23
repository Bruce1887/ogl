#pragma once

#include "Common.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"
#include "MeshRenderable.h"
#include "TerrainGenerator.h"
#include <unordered_map>
#include <memory>

// Terrain chunk represents a subdivided portion of the terrain
struct ChunkCoord
{
    int x;
    int z;
    
    bool operator==(const ChunkCoord& other) const
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
        size_t operator()(const ChunkCoord& coord) const
        {
            return hash<int>()(coord.x) ^ (hash<int>()(coord.z) << 1);
        }
    };
}

class TerrainChunkManager
{
public:
    TerrainChunkManager(TerrainGenerator* generator, int chunkSize, int vertexStep = 1);
    ~TerrainChunkManager();

    // Load/unload chunks based on camera position
    void updateChunks(const glm::vec3& cameraPosition, float renderDistance);

    // Get all currently loaded chunks
    const std::unordered_map<ChunkCoord, MeshRenderable*>& getLoadedChunks() const { return m_loadedChunks; }

    // Get chunk size
    int getChunkSize() const { return m_chunkSize; }

private:
    TerrainGenerator* m_generator;
    int m_chunkSize; // Size of each chunk in grid units (e.g., 32x32)
    int m_vertexStep; // Spacing between vertices for LOD
    
    std::unordered_map<ChunkCoord, MeshRenderable*> m_loadedChunks;
    std::unordered_map<ChunkCoord, Mesh*> m_chunkMeshes; // Store mesh data separately
    
    Shader* m_terrainShader; // Reference to shader (not owned)
    
    // Optimization: track last camera position to avoid redundant updates
    glm::vec3 m_lastCameraPosition;
    float m_updateThreshold; // Only update chunks when camera moves this far

    // Generate a single chunk
    Mesh* generateChunk(const ChunkCoord& coord);
    
    // Load a chunk
    void loadChunk(const ChunkCoord& coord);
    
    // Unload a chunk
    void unloadChunk(const ChunkCoord& coord);
    
    // Get chunk coordinate from world position
    ChunkCoord worldToChunk(const glm::vec3& worldPos) const;

public:
    // Set shader reference
    void setShader(Shader* shader) { m_terrainShader = shader; }
};
