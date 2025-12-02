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
    //     // bone indexes which will influence this vertex
    //     int m_BoneIDs[MAX_BONE_INFLUENCE];
    //     // weights from each bone
    //     float m_Weights[MAX_BONE_INFLUENCE];
};

ModelData::~ModelData()
{
    DEBUG_PRINT("Destroying ModelData with " << m_meshRenderables.size() << " mesh renderables.");
}

Model::Model(const std::filesystem::path &path)
{
    Assimp::Importer importer;
    const aiScene *ai_scene = importer.ReadFile(path.string(),
                                                aiProcess_Triangulate |
                                                    aiProcess_FlipUVs |
                                                    aiProcess_CalcTangentSpace);

    if (!ai_scene || ai_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !ai_scene->mRootNode)
    {
        std::cerr << "Error loading model: " << importer.GetErrorString() << std::endl;
        return;
    }

    DEBUG_PRINT("Processing model at path: " << path << " with " << ai_scene->mNumMeshes << " meshes.");

    m_modelData = std::make_shared<ModelData>();

    // Process all meshes in the scene
    for (unsigned int i = 0; i < ai_scene->mNumMeshes; i++)
    {
        aiMesh *mesh = ai_scene->mMeshes[i];
        DEBUG_PRINT("Processing mesh with " << mesh->mNumVertices << " vertices.");

        aiMaterial *material = ai_scene->mMaterials[mesh->mMaterialIndex];

        // Extract material properties
        aiColor3D ambientColor(0.0f, 0.0f, 0.0f);
        material->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor);
        glm::vec3 ambient_glm(ambientColor.r, ambientColor.g, ambientColor.b);

        aiColor3D diffuseColor(0.0f, 0.0f, 0.0f);
        material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
        glm::vec3 diffuse_glm(diffuseColor.r, diffuseColor.g, diffuseColor.b);

        aiColor3D specularColor(0.0f, 0.0f, 0.0f);
        material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
        glm::vec3 specular_glm(specularColor.r, specularColor.g, specularColor.b);

        float shininess = 32.0f;
        material->Get(AI_MATKEY_SHININESS, shininess);
        // OpenGL power function requires a positive exponent.
        if (shininess == 0.0f)
            shininess = 32.0f;

        // Check for diffuse texture
        aiString texPath;
        m_modelData->m_hasTextureDiffuse = material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS;

#ifdef DEBUG
        assert(mesh != nullptr);
        assert(mesh->HasPositions());
        assert(mesh->HasNormals());
        assert(mesh->HasTextureCoords(0));
        assert(mesh->HasTangentsAndBitangents());
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
        auto va_ptr = std::make_unique<VertexArray>();
        va_ptr->bind();
        auto vb_ptr = std::make_unique<VertexBuffer>(vertices.data(), vertices.size() * sizeof(Vertex), va_ptr.get());

        VertexBufferLayout layout;
        layout.push<float>(3); // position
        layout.push<float>(3); // normal
        layout.push<float>(2); // texCoords
        layout.push<float>(3); // tangent
        layout.push<float>(3); // bitangent
        va_ptr->addBuffer(vb_ptr.get(), layout);

        // Extract index data
        std::vector<unsigned int> indices;
        for (unsigned int f = 0; f < mesh->mNumFaces; f++)
        {
            aiFace face = mesh->mFaces[f];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        auto ibo_ptr = std::make_unique<IndexBuffer>(indices.data(), indices.size());

        // create Mesh
        auto mesh_ptr = std::make_shared<Mesh>(std::move(va_ptr), std::move(vb_ptr), std::move(ibo_ptr));

        // create Shader
        auto shader_ptr = std::make_shared<Shader>();
        shader_ptr->addShader("3D_POS_NORM_TEX_TAN_BIT.vert", ShaderType::VERTEX);

        if (m_modelData->m_hasTextureDiffuse)
            shader_ptr->addShader("PhongMTL_diffTEX.frag", ShaderType::FRAGMENT);
        else
            shader_ptr->addShader("PhongMTL.frag", ShaderType::FRAGMENT);

        shader_ptr->createProgram();

        /*
        std::cout << "u_material_ambient " << ambient_glm.x << ", " << ambient_glm.y << ", " << ambient_glm.z << std::endl
                  << "u_material_diffuse " << diffuse_glm.x << ", " << diffuse_glm.y << ", " << diffuse_glm.z << std::endl
                  << "u_material_specular " << specular_glm.x << ", " << specular_glm.y << ", " << specular_glm.z << std::endl
                  << "u_material_shininess " << shininess
                  << std::endl;
        */

        // create MeshRenderable and store it
        auto mr = std::make_shared<MeshRenderable>(mesh_ptr, shader_ptr);
        mr->setUniform("u_material_ambient", ambient_glm);
        mr->setUniform("u_material_diffuse", diffuse_glm);
        mr->setUniform("u_material_specular", specular_glm);
        mr->setUniform("u_material_shininess", shininess);

        if (m_modelData->m_hasTextureDiffuse)
        {
            std::cout << "Loading diffuse texture: " << (path.parent_path() / texPath.C_Str()) << std::endl;
            auto diffuseTex = std::make_shared<Texture>((path.parent_path() / texPath.C_Str()).string(), 0);
            diffuseTex->targetUniform = "u_texture_diffuse";
            mr->m_textureReferences.push_back(diffuseTex);
        }
        m_modelData->addMeshRenderable(std::shared_ptr<MeshRenderable>(mr));
    }

    DEBUG_PRINT("Finished processing model.");
}

Model::Model(std::shared_ptr<ModelData> modelData)
    : m_modelData(modelData)
{
}

void Model::render(glm::mat4 view, glm::mat4 projection, PhongLightConfig *phongLight)
{
    for (auto &mr : m_modelData->getMeshRenderables())
    {
        mr->setTransform(this->getTransform());
        mr->render(view, projection, phongLight);
    }
}
