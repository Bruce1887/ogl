#include "Model.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "Shader.h"

#define MAX_BONE_INFLUENCE 4
struct Vertex
{
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
    // bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    // weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];
};

Model::Model(const std::string &path)
{
    Assimp::Importer importer;
    const aiScene *ai_scene = importer.ReadFile(path,
                                                aiProcess_Triangulate |
                                                    aiProcess_FlipUVs |
                                                    aiProcess_CalcTangentSpace);

    if (!ai_scene || ai_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !ai_scene->mRootNode)
    {
        std::cerr << "Error loading model: " << importer.GetErrorString() << std::endl;
        return;
    }

#ifdef DEBUG
    DEBUG_PRINT("Processing model at path: " << path << " with " << ai_scene->mNumMeshes << " meshes.");

#endif

    // Process all meshes in the scene
    for (unsigned int i = 0; i < ai_scene->mNumMeshes; i++)
    {
#ifdef DEBUG
        std::cerr << "Processing mesh " << i << std::endl;
#endif
        aiMesh *mesh = ai_scene->mMeshes[i];

#ifdef DEBUG
        assert(mesh != nullptr);
        std::cerr << "(mesh->HasPositions()): " << (mesh->HasPositions()) << std::endl;
        std::cerr << "(mesh->HasNormals()): " << (mesh->HasNormals()) << std::endl;
        std::cerr << "(mesh->HasTextureCoords(0)): " << (mesh->HasTextureCoords(0)) << std::endl;
        std::cerr << "(mesh->HasTangentsAndBitangents()): " << (mesh->HasTangentsAndBitangents()) << std::endl;

#endif
        // Extract vertex data
        std::vector<Vertex> vertices;
        for (unsigned int v = 0; v < mesh->mNumVertices; v++)
        {
            Vertex vertex;
            // #### Check for positions ####
            if (mesh->HasPositions())
            {
                vertex.Position = glm::vec3(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
            }
            else
            {
                vertex.Position = glm::vec3(0.0f, 0.0f, 0.0f);
            }
            // #### Check for normals ####
            if (mesh->HasNormals())
            {
                vertex.Normal = glm::vec3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
            }
            else
            {
                vertex.Normal = glm::vec3(0.0f, 0.0f, 0.0f);
            }

            // #### Check for texture coordinates ####
            if (mesh->HasTextureCoords(0)) // Does the mesh contain texture coordinates?
            {
                vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y);
            }
            else
            {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            // #### Check for tangents and bitangents ####
            if (mesh->HasTangentsAndBitangents())
            {
                vertex.Tangent = glm::vec3(mesh->mTangents[v].x, mesh->mTangents[v].y, mesh->mTangents[v].z);
                vertex.Bitangent = glm::vec3(mesh->mBitangents[v].x, mesh->mBitangents[v].y, mesh->mBitangents[v].z);
            }
            else
            {
                vertex.Tangent = glm::vec3(0.0f);
                vertex.Bitangent = glm::vec3(0.0f);
            }

            vertices.push_back(vertex);
        }

        // Create buffers and upload data to GPU
        VertexArray va;
        va.bind();
        VertexBuffer vb(vertices.data(), vertices.size() * sizeof(Vertex), &va);
        VertexBufferLayout layout;
        layout.push<float>(3); // position
        layout.push<float>(3); // normal
        layout.push<float>(2); // texCoords
        layout.push<float>(3); // tangent
        layout.push<float>(3); // bitangent
        va.addBuffer(vb, layout);

        // Extract index data
        std::vector<unsigned int> indices;
        for (unsigned int f = 0; f < mesh->mNumFaces; f++)
        {
            aiFace face = mesh->mFaces[f];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        IndexBuffer ibo(indices.data(), indices.size());
        // create Mesh
        std::shared_ptr<Mesh> mesh_ptr = std::make_shared<Mesh>(std::move(va), std::move(ibo));
        
        // create Shader
        std::shared_ptr<Shader> shader_ptr = std::make_shared<Shader>();
        shader_ptr->addShader("3DLighting_Tex.vert", ShaderType::VERTEX); // TEMPORARY
                                                                          // shader_ptr->addShader("PhongTEX.frag", ShaderType::FRAGMENT);
        shader_ptr->addShader("constColor.frag", ShaderType::FRAGMENT);
        shader_ptr->createProgram();                                    // TEMPORARY
        shader_ptr->bind();                                             // TEMPORARY
        shader_ptr->setUniform("u_color", glm::vec3(0.2f, 0.2f, 0.6f)); // TEMPORARY

        // create MeshRenderable and store it



        
        MeshRenderable *meshRenderable = new MeshRenderable(mesh_ptr, shader_ptr);

        m_meshRenderables.push_back(meshRenderable);
    }

#ifdef DEBUG
    DEBUG_PRINT("Finished processing model. Created " << m_meshRenderables.size() << " MeshRenderables.");
#endif
}

void Model::render(glm::mat4 view, glm::mat4 projection, PhongLightConfig *phongLight)
{
    for (auto &mr : m_meshRenderables)
    {
        mr->render(view, projection, phongLight);
    }
}

Model::~Model()
{
    for (auto &mr : m_meshRenderables)
    {
        delete mr;
    }
    m_meshRenderables.clear();
}