#include "ObjLoader.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include <fstream>
#include <sstream>
#include <map>

namespace
{
    struct PackedVertex
    {
        int v = 0;   // position index (1-based in OBJ, stored here as 0-based)
        int vt = -1; // texcoord index (optional)
        int vn = -1; // normal index (optional)

        bool operator==(const PackedVertex &other) const
        {
            return v == other.v && vt == other.vt && vn == other.vn;
        }
    };

    struct PackedVertexHasher
    {
        size_t operator()(const PackedVertex &pv) const noexcept
        {
            // Simple hash combine
            size_t h = std::hash<int>{}(pv.v);
            h ^= std::hash<int>{}(pv.vt + 1) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<int>{}(pv.vn + 1) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };

    // Convert possibly-negative OBJ index (1-based, negative relative) to 0-based
    inline int toZeroBasedIndex(int idx, int count)
    {
        if (idx > 0)
            return idx - 1; // 1..N -> 0..N-1
        // negative indices refer from the end
        return count + idx; // e.g., -1 -> last element (count-1)
    }
}

Mesh *ObjLoader::LoadOBJ(const fs::path &filepath, bool generateNormalsIfMissing)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "[ObjLoader] Failed to open OBJ file: " << filepath << std::endl;
        return nullptr;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;

    // Final unique vertex buffer (interleaved P,N,UV) and indices
    std::vector<float> interleaved;
    std::vector<unsigned int> indices;

    // Map a unique vertex combination to its index in interleaved
    std::unordered_map<PackedVertex, unsigned int, PackedVertexHasher> vertexCache;

    std::string line;
    bool anyNormals = false;
    bool anyTexcoords = false;

    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream ss(line);
        std::string tag;
        ss >> tag;

        if (tag == "v")
        {
            glm::vec3 p;
            ss >> p.x >> p.y >> p.z;
            positions.push_back(p);
        }
        else if (tag == "vt")
        {
            glm::vec2 t{0.f, 0.f};
            ss >> t.x >> t.y;
            texcoords.push_back(t);
            anyTexcoords = true;
        }
        else if (tag == "vn")
        {
            glm::vec3 n;
            ss >> n.x >> n.y >> n.z;
            normals.push_back(n);
            anyNormals = true;
        }
        else if (tag == "f")
        {
            // Parse all vertex specs on this face
            std::vector<PackedVertex> faceVerts;
            std::string vertSpec;
            while (ss >> vertSpec)
            {
                // Supported formats: v, v/vt, v//vn, v/vt/vn
                int v = 0, vt = 0, vn = 0;

                size_t firstSlash = vertSpec.find('/');
                if (firstSlash == std::string::npos)
                {
                    // Only position
                    v = std::stoi(vertSpec);
                    vt = 0; vn = 0;
                }
                else
                {
                    size_t secondSlash = vertSpec.find('/', firstSlash + 1);
                    std::string sV = vertSpec.substr(0, firstSlash);
                    std::string sVT;
                    std::string sVN;

                    if (secondSlash == std::string::npos)
                    {
                        // v/vt
                        sVT = vertSpec.substr(firstSlash + 1);
                    }
                    else
                    {
                        // v//vn or v/vt/vn
                        sVT = vertSpec.substr(firstSlash + 1, secondSlash - firstSlash - 1);
                        sVN = vertSpec.substr(secondSlash + 1);
                    }

                    v = sV.empty() ? 0 : std::stoi(sV);
                    vt = sVT.empty() ? 0 : std::stoi(sVT);
                    vn = sVN.empty() ? 0 : std::stoi(sVN);
                }

                PackedVertex pv;
                pv.v = toZeroBasedIndex(v, static_cast<int>(positions.size()));
                pv.vt = (vt == 0) ? -1 : toZeroBasedIndex(vt, static_cast<int>(texcoords.size()));
                pv.vn = (vn == 0) ? -1 : toZeroBasedIndex(vn, static_cast<int>(normals.size()));
                faceVerts.push_back(pv);
            }

            if (faceVerts.size() < 3)
                continue; // degenerate

            auto emitVertex = [&](const PackedVertex &pv) -> unsigned int {
                auto it = vertexCache.find(pv);
                if (it != vertexCache.end())
                    return it->second;

                glm::vec3 pos = positions.at(pv.v);
                glm::vec3 nrm = (pv.vn >= 0) ? normals.at(pv.vn) : glm::vec3(0.f, 0.f, 0.f);
                glm::vec2 uv = (pv.vt >= 0) ? texcoords.at(pv.vt) : glm::vec2(0.f, 0.f);

                unsigned int newIndex = static_cast<unsigned int>(interleaved.size() / 8);
                interleaved.push_back(pos.x);
                interleaved.push_back(pos.y);
                interleaved.push_back(pos.z);
                interleaved.push_back(nrm.x);
                interleaved.push_back(nrm.y);
                interleaved.push_back(nrm.z);
                interleaved.push_back(uv.x);
                interleaved.push_back(uv.y);

                vertexCache.emplace(pv, newIndex);
                return newIndex;
            };

            // Triangulate face (fan)
            for (size_t i = 1; i + 1 < faceVerts.size(); ++i)
            {
                unsigned int i0 = emitVertex(faceVerts[0]);
                unsigned int i1 = emitVertex(faceVerts[i]);
                unsigned int i2 = emitVertex(faceVerts[i + 1]);
                indices.push_back(i0);
                indices.push_back(i1);
                indices.push_back(i2);
            }
        }
        // ignore: mtllib, usemtl, o, g, s, etc. for now
    }

    // Optional: generate flat normals if none present in file and requested
    if ((!anyNormals) && generateNormalsIfMissing)
    {
        // Reset normals to recompute (interleaved has zeros in normal slots right now)
        // We'll compute a flat normal per triangle and accumulate per vertex, then normalize.
        size_t vertexCount = interleaved.size() / 8;
        std::vector<glm::vec3> accum(vertexCount, glm::vec3(0));
        for (size_t i = 0; i + 2 < indices.size(); i += 3)
        {
            unsigned int ia = indices[i];
            unsigned int ib = indices[i + 1];
            unsigned int ic = indices[i + 2];

            auto readPos = [&](unsigned int idx) {
                size_t base = idx * 8;
                return glm::vec3(interleaved[base + 0], interleaved[base + 1], interleaved[base + 2]);
            };
            glm::vec3 a = readPos(ia);
            glm::vec3 b = readPos(ib);
            glm::vec3 c = readPos(ic);
            glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));
            if (!std::isfinite(n.x) || !std::isfinite(n.y) || !std::isfinite(n.z))
                n = glm::vec3(0, 1, 0);
            accum[ia] += n;
            accum[ib] += n;
            accum[ic] += n;
        }

        for (size_t i = 0; i < vertexCount; ++i)
        {
            glm::vec3 n = glm::normalize(accum[i]);
            size_t base = i * 8;
            interleaved[base + 3] = n.x;
            interleaved[base + 4] = n.y;
            interleaved[base + 5] = n.z;
        }
    }

    if (interleaved.empty() || indices.empty())
    {
        std::cerr << "[ObjLoader] No geometry found in OBJ: " << filepath << std::endl;
        return nullptr;
    }

    // Create GPU objects
    auto *va = new VertexArray();
    // Allocate VBO on heap to persist beyond function scope (Mesh/VAO don't currently own the VBO explicitly)
    auto *vb = new VertexBuffer(interleaved.data(), static_cast<unsigned int>(interleaved.size() * sizeof(float)), va);
    VertexBufferLayout layout;
    layout.push<float>(3); // position
    layout.push<float>(3); // normal
    layout.push<float>(2); // uv
    va->addBuffer(*vb, layout);

    auto *ib = new IndexBuffer(indices.data(), static_cast<unsigned int>(indices.size()));

    return new Mesh(va, ib);
}

std::map<std::string, ObjLoader::Material> ObjLoader::ParseMTL(const fs::path &filepath)
{
    std::map<std::string, Material> materials;
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "[ObjLoader::ParseMTL] Failed to open MTL file: " << filepath << std::endl;
        return materials;
    }

    Material *current = nullptr;
    std::string line;

    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream ss(line);
        std::string tag;
        ss >> tag;

        if (tag == "newmtl")
        {
            std::string name;
            ss >> name;
            materials[name] = Material{};
            materials[name].name = name;
            current = &materials[name];
        }
        else if (current)
        {
            if (tag == "Kd") // diffuse color
            {
                ss >> current->diffuse.r >> current->diffuse.g >> current->diffuse.b;
            }
            else if (tag == "Ka") // ambient
            {
                ss >> current->ambient.r >> current->ambient.g >> current->ambient.b;
            }
            else if (tag == "Ks") // specular
            {
                ss >> current->specular.r >> current->specular.g >> current->specular.b;
            }
            else if (tag == "Ns") // shininess
            {
                ss >> current->shininess;
            }
        }
    }

    std::cout << "[ObjLoader::ParseMTL] Loaded " << materials.size() << " materials from " << filepath << std::endl;
    return materials;
}

std::map<std::string, Mesh*> ObjLoader::LoadOBJWithMaterials(const fs::path &filepath, bool generateNormalsIfMissing)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "[ObjLoader] Failed to open OBJ file: " << filepath << std::endl;
        return {};
    }

    // Try to load MTL file
    std::map<std::string, Material> materials;
    std::string line;
    
    // First pass: find mtllib reference
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;
        std::istringstream ss(line);
        std::string tag;
        ss >> tag;
        if (tag == "mtllib")
        {
            std::string mtlFile;
            std::getline(ss, mtlFile);
            // trim leading whitespace
            size_t start = mtlFile.find_first_not_of(" \t\r\n");
            if (start != std::string::npos)
                mtlFile = mtlFile.substr(start);
            
            fs::path mtlPath = filepath.parent_path() / mtlFile;
            materials = ParseMTL(mtlPath);
            break;
        }
    }

    // Rewind file for second pass
    file.clear();
    file.seekg(0);

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;

    // Per-material data - now with color in vertex buffer
    struct MaterialData {
        std::vector<float> interleaved; // pos(3) + normal(3) + color(3) + uv(2) = 11 floats
        std::vector<unsigned int> indices;
        std::unordered_map<PackedVertex, unsigned int, PackedVertexHasher> vertexCache;
        glm::vec3 color{0.8f, 0.8f, 0.8f}; // default gray
    };
    std::map<std::string, MaterialData> matData;
    std::string currentMaterial = "default";

    bool anyNormals = false;
    bool anyTexcoords = false;

    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream ss(line);
        std::string tag;
        ss >> tag;

        if (tag == "v")
        {
            glm::vec3 p;
            ss >> p.x >> p.y >> p.z;
            positions.push_back(p);
        }
        else if (tag == "vt")
        {
            glm::vec2 t{0.f, 0.f};
            ss >> t.x >> t.y;
            texcoords.push_back(t);
            anyTexcoords = true;
        }
        else if (tag == "vn")
        {
            glm::vec3 n;
            ss >> n.x >> n.y >> n.z;
            normals.push_back(n);
            anyNormals = true;
        }
        else if (tag == "usemtl")
        {
            ss >> currentMaterial;
            if (matData.find(currentMaterial) == matData.end())
            {
                matData[currentMaterial] = MaterialData();
                // Set color from MTL if available
                auto it = materials.find(currentMaterial);
                if (it != materials.end())
                {
                    matData[currentMaterial].color = it->second.diffuse;
                }
            }
        }
        else if (tag == "f")
        {
            if (matData.find(currentMaterial) == matData.end())
            {
                matData[currentMaterial] = MaterialData();
            }

            MaterialData &md = matData[currentMaterial];
            glm::vec3 materialColor = md.color;

            std::vector<PackedVertex> faceVerts;
            std::string vertSpec;
            while (ss >> vertSpec)
            {
                int v = 0, vt = 0, vn = 0;

                size_t firstSlash = vertSpec.find('/');
                if (firstSlash == std::string::npos)
                {
                    v = std::stoi(vertSpec);
                }
                else
                {
                    size_t secondSlash = vertSpec.find('/', firstSlash + 1);
                    std::string sV = vertSpec.substr(0, firstSlash);
                    std::string sVT, sVN;

                    if (secondSlash == std::string::npos)
                    {
                        sVT = vertSpec.substr(firstSlash + 1);
                    }
                    else
                    {
                        sVT = vertSpec.substr(firstSlash + 1, secondSlash - firstSlash - 1);
                        sVN = vertSpec.substr(secondSlash + 1);
                    }

                    v = sV.empty() ? 0 : std::stoi(sV);
                    vt = sVT.empty() ? 0 : std::stoi(sVT);
                    vn = sVN.empty() ? 0 : std::stoi(sVN);
                }

                PackedVertex pv;
                pv.v = toZeroBasedIndex(v, static_cast<int>(positions.size()));
                pv.vt = (vt == 0) ? -1 : toZeroBasedIndex(vt, static_cast<int>(texcoords.size()));
                pv.vn = (vn == 0) ? -1 : toZeroBasedIndex(vn, static_cast<int>(normals.size()));
                faceVerts.push_back(pv);
            }

            if (faceVerts.size() < 3)
                continue;

            auto emitVertex = [&](const PackedVertex &pv) -> unsigned int {
                auto it = md.vertexCache.find(pv);
                if (it != md.vertexCache.end())
                    return it->second;

                glm::vec3 pos = positions.at(pv.v);
                glm::vec3 nrm = (pv.vn >= 0) ? normals.at(pv.vn) : glm::vec3(0.f);
                glm::vec2 uv = (pv.vt >= 0) ? texcoords.at(pv.vt) : glm::vec2(0.f);

                unsigned int newIndex = static_cast<unsigned int>(md.interleaved.size() / 11);
                md.interleaved.push_back(pos.x);
                md.interleaved.push_back(pos.y);
                md.interleaved.push_back(pos.z);
                md.interleaved.push_back(nrm.x);
                md.interleaved.push_back(nrm.y);
                md.interleaved.push_back(nrm.z);
                md.interleaved.push_back(materialColor.r);
                md.interleaved.push_back(materialColor.g);
                md.interleaved.push_back(materialColor.b);
                md.interleaved.push_back(uv.x);
                md.interleaved.push_back(uv.y);

                md.vertexCache.emplace(pv, newIndex);
                return newIndex;
            };

            for (size_t i = 1; i + 1 < faceVerts.size(); ++i)
            {
                unsigned int i0 = emitVertex(faceVerts[0]);
                unsigned int i1 = emitVertex(faceVerts[i]);
                unsigned int i2 = emitVertex(faceVerts[i + 1]);
                md.indices.push_back(i0);
                md.indices.push_back(i1);
                md.indices.push_back(i2);
            }
        }
    }

    // Generate normals if missing
    if ((!anyNormals) && generateNormalsIfMissing)
    {
        for (auto &pair : matData)
        {
            MaterialData &md = pair.second;
            size_t vertexCount = md.interleaved.size() / 11;
            std::vector<glm::vec3> accum(vertexCount, glm::vec3(0));

            for (size_t i = 0; i + 2 < md.indices.size(); i += 3)
            {
                unsigned int ia = md.indices[i];
                unsigned int ib = md.indices[i + 1];
                unsigned int ic = md.indices[i + 2];

                auto readPos = [&](unsigned int idx) {
                    size_t base = idx * 11;
                    return glm::vec3(md.interleaved[base], md.interleaved[base + 1], md.interleaved[base + 2]);
                };
                glm::vec3 a = readPos(ia);
                glm::vec3 b = readPos(ib);
                glm::vec3 c = readPos(ic);
                glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));
                if (!std::isfinite(n.x))
                    n = glm::vec3(0, 1, 0);
                accum[ia] += n;
                accum[ib] += n;
                accum[ic] += n;
            }

            for (size_t i = 0; i < vertexCount; ++i)
            {
                glm::vec3 n = glm::normalize(accum[i]);
                size_t base = i * 11;
                md.interleaved[base + 3] = n.x;
                md.interleaved[base + 4] = n.y;
                md.interleaved[base + 5] = n.z;
            }
        }
    }

    // Build GPU meshes
    std::map<std::string, Mesh*> result;
    for (auto &pair : matData)
    {
        const std::string &matName = pair.first;
        MaterialData &md = pair.second;

        if (md.interleaved.empty() || md.indices.empty())
            continue;

        auto *va = new VertexArray();
        auto *vb = new VertexBuffer(md.interleaved.data(), static_cast<unsigned int>(md.interleaved.size() * sizeof(float)), va);
        VertexBufferLayout layout;
        layout.push<float>(3); // position
        layout.push<float>(3); // normal
        layout.push<float>(3); // color (material diffuse)
        layout.push<float>(2); // uv
        va->addBuffer(*vb, layout);

        auto *ib = new IndexBuffer(md.indices.data(), static_cast<unsigned int>(md.indices.size()));

        result[matName] = new Mesh(va, ib);
    }

    std::cout << "[ObjLoader] Loaded " << result.size() << " material groups from " << filepath << std::endl;
    return result;
}
