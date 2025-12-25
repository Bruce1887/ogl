#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"

#include "Shader.h"
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
			.aspect = (float)WINDOW_X / (float)WINDOW_Y,
			.near = 1.0f,
			.far = 300.0f};

		Camera camera(cam_config);
		camera.m_Position = glm::vec3(0.0f, 20.0f, 35.0f);
		camera.m_Target = glm::vec3(0.0f, 0.0f, 0.0f);
		camera.m_Up = glm::vec3(0.0f, 1.0f, 0.0f);

		// ######## Light source ########
		LightSource lightSource = LightSource::createDefaultLightSource();

		// Create a small box to visualize the light source
		std::shared_ptr<Mesh> box_mesh_ptr = Mesh::createBoxMesh();
		std::shared_ptr<Shader> constcolor_3D = std::make_shared<Shader>();
		constcolor_3D->addShader("3D.vert", ShaderType::VERTEX);
		constcolor_3D->addShader("constColor.frag", ShaderType::FRAGMENT);
		constcolor_3D->createProgram();

		std::unique_ptr<MeshRenderable> lightBox_renderable = std::make_unique<MeshRenderable>(box_mesh_ptr, constcolor_3D);
		lightBox_renderable->setUniform("u_color", lightSource.config.diffuseLight); // Set the box color to the light's diffuse color
		lightSource.visualRepresentation = std::move(lightBox_renderable);

		// create the scene
		Scene scene(camera, std::move(lightSource));
		std::cout << "lightVisualRepresentation id: " << scene.m_lightSource.visualRepresentation->getID() << std::endl;

		std::unique_ptr<MeshRenderable> another_box = std::make_unique<MeshRenderable>(box_mesh_ptr, constcolor_3D);
		another_box->setUniform("u_color", glm::vec3(1.0f, 0.0f, 0.0f));
		another_box->setPosition(glm::vec3(-5.0f, 0.0f, 0.0f));
		scene.addRenderable(another_box.get());

		// Model instance1((MODELS_DIR / "low-poly-pinetree2/pineTree.obj").string()); // funkar
		// Model instance1((MODELS_DIR /  "wooden-box-low-poly" / "source" / "box_low.fbx").string()); // funkar ej
		// Model instance1((MODELS_DIR / "gran" / "gran.obj")); // funkar
		Model instance1((MODELS_DIR / "cow" / "cow.obj")); // funkar
		std::cout << "instance1 id: " << instance1.getID() << std::endl;
		scene.addRenderable(&instance1);

		Model instance2 = Model::copyFrom(&instance1);
		std::cout << "instance2 id: " << instance2.getID() << std::endl;
		instance2.setPosition(glm::vec3(10.0f, 0.0f, 0.0f));
		scene.addRenderable(&instance2);

		FrameTimer frameTimer;

		int frameCount = 0;
		while (!glfwWindowShouldClose(g_window))
		{
			scene.tick();

			float slowedTime = frameTimer.getCurrentTime() * 0.5f;
			instance2.setPosition(glm::vec3(10.0f * sinf(slowedTime), 0.0f, 10.0f * cosf(slowedTime)));
			scene.m_lightSource.config.lightPosition = glm::vec3(-15.0f * sinf(slowedTime), scene.m_lightSource.config.lightPosition.y, 15.0f * cosf(slowedTime));

			// scene.m_activeCamera.flyControl(g_InputManager, frameTimer.getDeltaTime());
			scene.m_activeCamera.flyControl(g_InputManager, frameTimer.getDeltaTime());

			scene.renderScene();

			if (frameCount % 120 == 0)
			{
				DEBUG_PRINT("FPS: " << std::fixed << std::setprecision(1) << (1.0f / frameTimer.getDeltaTime()) << "     ");
			}
			frameCount++;
		}
	}

out:
	oogaboogaExit();

	return 0;
}