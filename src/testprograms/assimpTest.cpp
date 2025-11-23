#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"

#include "Shader.h"
#include "Texture.h"
#include "Common.h"
#include "Camera.h"
#include "Scene.h"
#include "Frametimer.h"
#include "MeshRenderable.h"
#include "AssimpModel.h"

#include <fstream>
#include <string>
#include <sstream>
#include <cassert>

#include <assimp/Importer.hpp>	// C++ importer interface
#include <assimp/scene.h>		// Output data structure
#include <assimp/postprocess.h> // Post processing flags

int main(int, char **)
{
	Assimp::Importer importer;

	// Initialise GLAD and GLFW
	if (oogaboogaInit(__FILE__))
		goto out;

	{
		// ######## Camera setup ########
		CameraConfiguration cam_config{
			.fov = 45.0f,
			.aspect = (float)window_X / (float)window_Y,
			.near = 1.0f,
			.far = 300.0f};

		Camera camera(cam_config);
		camera.m_Position = glm::vec3(0.0f, 20.0f, 35.0f);
		camera.m_Target = glm::vec3(0.0f, 0.0f, 0.0f);
		camera.m_Up = glm::vec3(0.0f, 1.0f, 0.0f);

		// ######## Light source ########
		PhongLightConfig lightConfig{
			.lightPosition = glm::vec3(15.0f, 10.0f, 10.0f),
			.ambientLight = glm::vec3(0.2f, 0.2f, 0.2f),
			.diffuseLight = glm::vec3(1.0f, 1.0f, 0.7f),
			.specularLight = glm::vec3(1.0f, 1.0f, 1.0f)};
		LightSource lightSource{
			.config = lightConfig,
			.visualRepresentation = nullptr};

		// Create a small box to visualize the light source
		VertexArray lightBox_VA;
		lightBox_VA.bind();
		VertexBuffer lightBox_VB(BOX_VERTICES, BOX_VERTICES_SIZE, &lightBox_VA);
		VertexBufferLayout lightBox_layout;
		lightBox_layout.push<float>(3); // position
		lightBox_VA.addBuffer(lightBox_VB, lightBox_layout);
		IndexBuffer lightBox_IBO(BOX_INDICES, BOX_INDICES_COUNT);
		Mesh lightBox_mesh(&lightBox_VA, &lightBox_IBO);
		Shader lightBox_shader;
		lightBox_shader.addShader("3D.vert", ShaderType::VERTEX);
		lightBox_shader.addShader("constColor.frag", ShaderType::FRAGMENT);
		lightBox_shader.createProgram();
		lightBox_shader.bind();
		lightBox_shader.setUniform("u_color", lightSource.config.diffuseLight); // Set the box color to the light's diffuse color
		MeshRenderable lightBox_renderable(&lightBox_mesh, &lightBox_shader);
		lightSource.visualRepresentation = &lightBox_renderable;

		// create the scene
		Scene scene(camera, lightSource);

		Shader phongShader;
		phongShader.addShader("3DLighting_Tex.vert", ShaderType::VERTEX);
		phongShader.addShader("PhongTEX.frag", ShaderType::FRAGMENT);
		phongShader.createProgram();

		const aiScene *ai_scene = importer.ReadFile((MODELS_DIR / "low-poly-pinetree/low-poly-pinetree.obj").string(),
													aiProcess_CalcTangentSpace |
														aiProcess_Triangulate |
														aiProcess_JoinIdenticalVertices |
														aiProcess_SortByPType);
		// If the import failed, report it
		if (nullptr == ai_scene)
		{
			std::cout << "Error loading model: " << importer.GetErrorString() << std::endl;
			return false;
		}
		
		AssimpModel model(ai_scene, &phongShader);
		model.setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		model.setTransform(glm::scale(model.getTransform(), glm::vec3(1.0f))); // scale if needed

		scene.addRenderable(&model);

		FrameTimer frameTimer;

		while (!glfwWindowShouldClose(g_window))
		{
			scene.tick();

			float slowedTime = frameTimer.getCurrentTime() * 0.5f;
			scene.m_lightSource.config.lightPosition = glm::vec3(15.0f * sinf(slowedTime), 15.0f * sinf(slowedTime), 15.0f * cosf(slowedTime));

			scene.m_activeCamera.flyControl(g_InputManager, frameTimer.getDeltaTime());
			// scene.m_activeCamera.orbitControl(g_InputManager, frameTimer.getDeltaTime());

			scene.renderScene();
		}
	}

out:
	oogaboogaExit();

	return 0;
}