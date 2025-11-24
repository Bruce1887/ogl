#pragma once

#include "Common.h"
#include "MeshRenderable.h"
#include <string>
#include <unordered_map>
#include <map>

// Minimal Wavefront OBJ loader producing a Mesh (positions, normals, UVs)
// Limitations:
// - Triangulates polygon faces using a simple fan (f v1 v2 v3 v4 -> (v1,v2,v3),(v1,v3,v4))
// - Supports v, vt, vn, f, usemtl, mtllib
// - Handles positive and negative (relative) indices
namespace ObjLoader
{
    struct Material {
        std::string name;
        glm::vec3 diffuse{0.8f, 0.8f, 0.8f}; // Kd
        glm::vec3 ambient{0.2f, 0.2f, 0.2f}; // Ka
        glm::vec3 specular{0.5f, 0.5f, 0.5f}; // Ks
        float shininess = 32.0f; // Ns
    };

    // Parse MTL file and return map of material name -> Material
    std::map<std::string, Material> ParseMTL(const fs::path &filepath);

    // Load an OBJ file from disk and create GPU buffers.
    // Returns nullptr on failure and logs to std::cerr.
    Mesh *LoadOBJ(const fs::path &filepath, bool generateNormalsIfMissing = true);

    // Load OBJ with materials. Returns map of material name -> Mesh.
    // Vertex layout: position(3), normal(3), color(3), uv(2) - 11 floats per vertex
    std::map<std::string, Mesh*> LoadOBJWithMaterials(const fs::path &filepath, bool generateNormalsIfMissing = true);
}
