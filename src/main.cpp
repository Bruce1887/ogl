#include <iostream>

// #include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"
#include "Common.h"
#include "Camera.h"
#include "Scene.h"
#include "Frametimer.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cassert>

int main(int, char **)
{
    // Initialise GLAD and GLFW
    if (oogaboogaInit(__FILE__))
        goto out;

    {
        CameraConfiguration cam_config{
            .fov = 45.0f,
            .aspect = (float)window_X / (float)window_Y,
            .near = 1.0f,
            .far = 300.0f};
        Camera camera(cam_config);
        camera.m_Position = glm::vec3(0.0f, 20.0f, 35.0f);
        camera.m_Target = glm::vec3(0.0f, 0.0f, 0.0f);
        camera.m_Up = glm::vec3(0.0f, 1.0f, 0.0f);

        Scene scene(camera);

        FrameTimer frameTimer;

        while (!glfwWindowShouldClose(window))
        {
            scene.tick();

            MovementInput movementInput = getUserMovementInput(window);
            cameraOrbitControl(scene.m_activeCamera, movementInput, frameTimer.getDeltaTime());

            scene.renderScene();
        }
    }

out:
    oogaboogaExit();

    return 0;
}