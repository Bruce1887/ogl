#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"

#include "Shader.h"
#include "Texture.h"
#include "Common.h"

#include <fstream>
#include <string>
#include <sstream>
#include <cassert>
#include <cmath>
#include <memory>

int main(int, char **)
{
    // Initialise GLAD and GLFW
    if (oogaboogaInit(__FILE__))
        goto out;

    {
        // Reuse the cube geometry from boxes but we'll scale via model matrix for the house base
        float cubeVertices[] = {
            // Front face (z = 0.5)
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // [0]
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f,  // [1]
            0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // [2]
            -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,  // [3]

            // Back face (z = -0.5)
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // [4]
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f,  // [5]
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   // [6]
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,  // [7]

            // Left face (x = -0.5)
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // [8]
            -0.5f, -0.5f, 0.5f, 1.0f, 0.0f,  // [9]
            -0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // [10]
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,  // [11]

            // Right face (x = 0.5)
            0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // [12]
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f,  // [13]
            0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // [14]
            0.5f, 0.5f, -0.5f, 0.0f, 1.0f,  // [15]

            // Top face (y = 0.5)
            -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, // [16]
            -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,  // [17]
            0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // [18]
            0.5f, 0.5f, -0.5f, 0.0f, 1.0f,  // [19]

            // Bottom face (y = -0.5)
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // [20]
            -0.5f, -0.5f, 0.5f, 1.0f, 0.0f,  // [21]
            0.5f, -0.5f, 0.5f, 1.0f, 1.0f,   // [22]
            0.5f, -0.5f, -0.5f, 0.0f, 1.0f   // [23]
        };

        unsigned int cubeIndices[] = {
            // Front face
            0, 1, 2,
            2, 3, 0,
            // Back face
            4, 5, 6,
            6, 7, 4,
            // Left face
            8, 9, 10,
            10, 11, 8,
            // Right face
            12, 13, 14,
            14, 15, 12,
            // Top face
            16, 17, 18,
            18, 19, 16,
            // Bottom face
            20, 21, 22,
            22, 23, 20};

        // Build VAO/VBO/IBO for base cube (we will scale in model matrix)
        auto baseVA_ptr = std::make_unique<VertexArray>();
        baseVA_ptr->bind();

        auto baseVB_ptr = std::make_unique<VertexBuffer>(cubeVertices, sizeof(cubeVertices), baseVA_ptr.get());

        VertexBufferLayout baseLayout;
        baseLayout.push<float>(3);
        baseLayout.push<float>(2);

        baseVA_ptr->addBuffer(baseVB_ptr.get(), baseLayout);

        IndexBuffer baseIB(cubeIndices, sizeof(cubeIndices) / sizeof(unsigned int));

        // Roof geometry: a pyramid whose base slightly extends the walls
        // Coordinates are authored to match the scaled base (width ~2.0, depth ~1.5).
        // Base y is slightly below the top of the walls (0.45 vs walls' 0.5).
        // Increase roof base extents so it overhangs the walls slightly.
        // Base of walls (after model scale) runs roughly x: [-1.0, 1.0], z: [-0.75, 0.75].
        // Make roof base a bit larger (overhang) and raise apex a little.
        // Roof geometry: make each triangular face use its own vertices so we can
        // orient UVs independently. We'll rotate (swap u/v) the UVs for the long
        // sides (front and back) so the roof texture appears rotated 90 degrees
        // there.
        float roofVertices[] = {
            // Back face (rotated UVs): apex, back-left (swapped uv), back-right (swapped uv)
             0.0f,  1.15f,  0.0f,    0.5f, 0.5f,  // apex
            -1.2f,  0.42f, -0.9f,    0.0f, 0.0f,  // back-left (swapped)
             1.2f,  0.42f, -0.9f,    0.0f, 1.0f,  // back-right (swapped)

            // Right face (original UVs): apex, back-right, front-right
             0.0f,  1.15f,  0.0f,    0.5f, 0.5f,  // apex
             1.2f,  0.42f, -0.9f,    1.0f, 0.0f,  // back-right
             1.2f,  0.42f,  0.9f,    1.0f, 1.0f,  // front-right

            // Front face (rotated UVs): apex, front-right (swapped), front-left (swapped)
             0.0f,  1.15f,  0.0f,    0.5f, 0.5f,  // apex
             1.2f,  0.42f,  0.9f,    1.0f, 1.0f,  // front-right (swapped)
            -1.2f,  0.42f,  0.9f,    1.0f, 0.0f,  // front-left (swapped)

            // Left face (original UVs): apex, front-left, back-left
             0.0f,  1.15f,  0.0f,    0.5f, 0.5f,  // apex
            -1.2f,  0.42f,  0.9f,    0.0f, 1.0f,  // front-left
            -1.2f,  0.42f, -0.9f,    0.0f, 0.0f   // back-left
        };

        unsigned int roofIndices[] = {
            0,1,2,
            3,4,5,
            6,7,8,
            9,10,11
        };

        auto roofVA_ptr = std::make_unique<VertexArray>();
        roofVA_ptr->bind();
        auto roofVB_ptr = std::make_unique<VertexBuffer>(roofVertices, sizeof(roofVertices), roofVA_ptr.get());
        VertexBufferLayout roofLayout;
        roofLayout.push<float>(3);
        roofLayout.push<float>(2);
        roofVA_ptr->addBuffer(roofVB_ptr.get(), roofLayout);
        IndexBuffer roofIB(roofIndices, sizeof(roofIndices) / sizeof(unsigned int));

        // Door geometry (world-space quad placed just in front of the front wall)
        float doorZ = 0.76f; // slightly in front of front face to avoid z-fighting
        float doorHalfWidth = 0.2f;
        float doorBottom = -0.5f;
        float doorTop = 0.2f;
        float doorVerts[] = {
            // x, y, z,  u, v
            -doorHalfWidth, doorBottom, doorZ, 0.0f, 0.0f,
             doorHalfWidth, doorBottom, doorZ, 1.0f, 0.0f,
             doorHalfWidth, doorTop,    doorZ, 1.0f, 1.0f,
            -doorHalfWidth, doorTop,    doorZ, 0.0f, 1.0f
        };
        unsigned int doorIdx[] = {0,1,2, 2,3,0};

        auto doorVA_ptr = std::make_unique<VertexArray>();
        doorVA_ptr->bind();
        auto doorVB_ptr = std::make_unique<VertexBuffer>(doorVerts, sizeof(doorVerts), doorVA_ptr.get());
        VertexBufferLayout doorLayout;
        doorLayout.push<float>(3);
        doorLayout.push<float>(2);
        doorVA_ptr->addBuffer(doorVB_ptr.get(), doorLayout);
        IndexBuffer doorIB(doorIdx, sizeof(doorIdx) / sizeof(unsigned int));

        // Door texture
        
        // Shaders and textures
        Shader shader;
        shader.addShader("3D_TEX.vert", ShaderType::VERTEX);
        shader.addShader("TEX.frag", ShaderType::FRAGMENT);
        shader.createProgram();
        
        Texture stone((TEXTURE_DIR / "stoneWall.png").string(), 0);
        Texture door((TEXTURE_DIR / "door.jpg").string(), 1);
        Texture roofTex((TEXTURE_DIR / "roof.jpg").string(), 2);
       

        // Simple camera settings similar to boxes.cpp
        float fov = 45.0f;
        struct settings { float fov; float aspect; } w_settings{fov, (float)window_X / (float)window_Y};
        glfwSetWindowUserPointer(g_window, &w_settings);

        while (!glfwWindowShouldClose(g_window))
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            float frameTime = static_cast<float>(glfwGetTime());

            // Orbiting camera
            glm::vec3 cameraPos = glm::vec3(cos(frameTime * 0.3f), 0.3f, sin(frameTime * 0.3f)) * 6.0f;
            glm::mat4 viewMatrix = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 projectionMatrix = glm::perspective(glm::radians(w_settings.fov), w_settings.aspect, 0.1f, 100.0f);

            shader.bind();
            shader.setUniform("u_view", viewMatrix);
            shader.setUniform("u_projection", projectionMatrix);

            // Draw base (scale cube to be house rectangle)
            baseVA_ptr->bind();
            baseIB.bind();
            // bind stone to unit 0 and transparent unit 1

            // 0 for stone wall
            shader.setUniform("u_texture", 0);

            // glActiveTexture(GL_TEXTURE1);
            // glBindTexture(GL_TEXTURE_2D, transparentTex);

            glm::mat4 modelBase = glm::mat4(1.0f);
            modelBase = glm::scale(modelBase, glm::vec3(2.0f, 1.0f, 1.5f)); // width, height, depth
            modelBase = glm::translate(modelBase, glm::vec3(0.0f, 0.0f, 0.0f));
            shader.setUniform("u_model", modelBase);
            GLCALL(glDrawElements(GL_TRIANGLES, baseIB.getCount(), GL_UNSIGNED_INT, nullptr));

            // Draw roof
            // bind roof texture to unit 0 and transparent unit 1
            // 2 for roof
            shader.setUniform("u_texture", 2);
            // glActiveTexture(GL_TEXTURE1);
            // glBindTexture(GL_TEXTURE_2D, transparentTex);

            roofVA_ptr->bind();
            roofIB.bind();
            glm::mat4 modelRoof = glm::mat4(1.0f);
            // roof vertices were authored in world-space sized for base width ~2.0 and depth ~1.5
            shader.setUniform("u_model", modelRoof);
            GLCALL(glDrawElements(GL_TRIANGLES, roofIB.getCount(), GL_UNSIGNED_INT, nullptr));

            // Draw door: bind door texture to unit 0 and transparent unit 1
            
            // 1 for door
            shader.setUniform("u_texture", 1);
            // glActiveTexture(GL_TEXTURE1);
            // glBindTexture(GL_TEXTURE_2D, transparentTex);

            doorVA_ptr->bind();
            doorIB.bind();
            glm::mat4 modelDoor = glm::mat4(1.0f);
            shader.setUniform("u_model", modelDoor);

            GLCALL(glDrawElements(GL_TRIANGLES, doorIB.getCount(), GL_UNSIGNED_INT, nullptr));

            glfwSwapBuffers(g_window);
            glfwPollEvents();
        }
    }

out:
    oogaboogaExit();

    return 0;
}
