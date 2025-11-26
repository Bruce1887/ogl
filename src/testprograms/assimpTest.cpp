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
#include "Model.h"

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
		auto lightBox_VA_ptr = std::make_unique<VertexArray>();
		lightBox_VA_ptr->bind();
		auto lightBox_VB = std::make_unique<VertexBuffer>(BOX_VERTICES, BOX_VERTICES_SIZE, lightBox_VA_ptr.get());
		VertexBufferLayout lightBox_layout;
		lightBox_layout.push<float>(3); // position
		lightBox_VA_ptr->addBuffer(lightBox_VB.get(), lightBox_layout);
		auto lightbox_ibo_ptr = std::make_unique<IndexBuffer>(BOX_INDICES, BOX_INDICES_COUNT);
		auto lightBox_mesh_ptr = std::make_shared<Mesh>(std::move(lightBox_VA_ptr), std::move(lightBox_VB), std::move(lightbox_ibo_ptr));
		auto lightBox_shader_ptr = std::make_shared<Shader>();
		lightBox_shader_ptr->addShader("3D.vert", ShaderType::VERTEX);
		lightBox_shader_ptr->addShader("constColor.frag", ShaderType::FRAGMENT);
		lightBox_shader_ptr->createProgram();
		lightBox_shader_ptr->bind();
		lightBox_shader_ptr->setUniform("u_color", lightSource.config.diffuseLight); // Set the box color to the light's diffuse color
		
		MeshRenderable lightBox_renderable(lightBox_mesh_ptr, lightBox_shader_ptr);
		lightSource.visualRepresentation = &lightBox_renderable;

		// create the scene
		Scene scene(camera, lightSource);		

		// Load model		
		// Model testModel((MODELS_DIR / "low-poly-pinetree2/pineTree.obj").string()); // funkar 
		Model testModel((MODELS_DIR / "gran" / "gran.obj")); // funkar		
		// Model testModel((MODELS_DIR /  "wooden-box-low-poly" / "source" / "box_low.fbx").string()); // funkar ej
		scene.addRenderable(&testModel);
		
		FrameTimer frameTimer;

		while (!glfwWindowShouldClose(g_window))
		{
			scene.tick();

			float slowedTime = frameTimer.getCurrentTime() * 0.5f;
			scene.m_lightSource.config.lightPosition = glm::vec3(15.0f * sinf(slowedTime), 15.0f * sinf(slowedTime), 15.0f * cosf(slowedTime));

			// scene.m_activeCamera.flyControl(g_InputManager, frameTimer.getDeltaTime());
			scene.m_activeCamera.orbitControl(g_InputManager, frameTimer.getDeltaTime());

			scene.renderScene();
		}
	}

out:
	oogaboogaExit();

	return 0;
}