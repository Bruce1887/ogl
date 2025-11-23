#include "AssimpModel.h"

// ============ IMPLEMENTATION ============

AssimpModel::AssimpModel(const aiScene *scene, Shader *shader)
{
	if (!scene || scene->mNumMeshes == 0)
	{
		std::cerr << "AssimpModel: Scene is null or has no meshes!" << std::endl;
		return;
	}

	// Process each mesh in the scene
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		processMesh(scene->mMeshes[i], shader);
	}
}

void AssimpModel::processMesh(const aiMesh *mesh, Shader *shader)
{
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	extractMeshData(mesh, vertices, indices);

	// Create vertex array and buffers
	auto va = std::make_unique<VertexArray>();
	va->bind();

	auto vb = std::make_unique<VertexBuffer>(vertices.data(), vertices.size() * sizeof(float), va.get());

	VertexBufferLayout layout;
	layout.push<float>(3); // position
	layout.push<float>(3); // normal
	layout.push<float>(2); // texture coord
	va->addBuffer(*vb, layout);

	auto ib = std::make_unique<IndexBuffer>(indices.data(), indices.size());

	auto mesh_obj = std::make_unique<Mesh>(va.get(), ib.get());

	// Create renderable
	auto renderable = std::make_unique<MeshRenderable>(mesh_obj.get(), shader);

	// Store everything
	MeshData meshData{
		.va = std::move(va),
		.vb = std::move(vb),
		.ib = std::move(ib),
		.mesh = std::move(mesh_obj),
		.renderable = std::move(renderable)};

	m_meshes.push_back(std::move(meshData));
}

void AssimpModel::extractMeshData(const aiMesh *mesh,
								  std::vector<float> &vertices,
								  std::vector<unsigned int> &indices)
{
	// Extract vertex positions, normals, and texture coordinates
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		// Position (3 floats)
		vertices.push_back(mesh->mVertices[i].x);
		vertices.push_back(mesh->mVertices[i].y);
		vertices.push_back(mesh->mVertices[i].z);

		// Normal (3 floats)
		if (mesh->HasNormals())
		{
			vertices.push_back(mesh->mNormals[i].x);
			vertices.push_back(mesh->mNormals[i].y);
			vertices.push_back(mesh->mNormals[i].z);
		}
		else
		{
			// Default normal if none exist
			vertices.push_back(0.0f);
			vertices.push_back(1.0f);
			vertices.push_back(0.0f);
		}

		// Texture coordinates (2 floats)
		if (mesh->mTextureCoords[0])
		{
			vertices.push_back(mesh->mTextureCoords[0][i].x);
			vertices.push_back(mesh->mTextureCoords[0][i].y);
		}
		else
		{
			vertices.push_back(0.0f);
			vertices.push_back(0.0f);
		}
	}

	// Extract indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}
}

void AssimpModel::render(glm::mat4 view, glm::mat4 projection, PhongLightConfig *phongLight)
{
	// Apply this model's transform to all its meshes
	for (auto &meshData : m_meshes)
	{
		// Set the mesh's transform to this model's transform
		meshData.renderable->setTransform(getTransform());
		// Render the mesh
		meshData.renderable->render(view, projection, phongLight);
	}
}