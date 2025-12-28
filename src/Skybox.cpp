#include "Skybox.h"
#include "Shader.h"
#include <glad/glad.h>
#include "vendor/stb_image/stb_image.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

Skybox::Skybox()
    : m_shader(nullptr)
{
    setUpShader();

    // Skybox faces are hardcoded here (its annoying to pass them every time)
    std::vector<std::filesystem::path> faces = {
            TEXTURE_DIR / "skybox" / "right.jpg",
            TEXTURE_DIR / "skybox" / "left.jpg",
            TEXTURE_DIR / "skybox" / "top.jpg",
            TEXTURE_DIR / "skybox" / "bottom.jpg",
            TEXTURE_DIR / "skybox" / "front.jpg",
            TEXTURE_DIR / "skybox" / "back.jpg"};

    m_cubemapTexture = Texture::CreateCubemap(faces, "u_cubemap");

    setUpMR();
}

void Skybox::setUpShader()
{
    m_shader = std::make_shared<Shader>();
    m_shader->addShader("Skybox.vert", ShaderType::VERTEX);
    m_shader->addShader("Skybox.frag", ShaderType::FRAGMENT);
    m_shader->createProgram();
}

void Skybox::setUpMR()
{
    float skyboxVertices[] = {
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f};

    std::unique_ptr<VertexArray> skyboxVAO = std::make_unique<VertexArray>();
    std::unique_ptr<VertexBuffer> skyboxVBO = std::make_unique<VertexBuffer>(skyboxVertices, sizeof(skyboxVertices), skyboxVAO.get(), BufferUsage::STATIC_DRAW);
    VertexBufferLayout skyboxLayout;
    skyboxLayout.push<float>(3); // position
    skyboxVAO->addBuffer(skyboxVBO.get(), skyboxLayout);

    auto mesh = std::make_shared<Mesh>(std::move(skyboxVAO), std::move(skyboxVBO));
    m_skybox_mr = std::make_unique<MeshRenderable>(mesh, m_shader);

    m_skybox_mr->m_textureReferences.push_back(m_cubemapTexture);

    // glGenVertexArrays(1, &m_vao);
    // glGenBuffers(1, &m_vbo);
    // glBindVertexArray(m_vao);
    // glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    // glEnableVertexAttribArray(0);
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // glBindVertexArray(0);
}

void Skybox::render(const glm::mat4 view, const glm::mat4 projection, const PhongLightConfig *phongLight)
{
    assert(phongLight == nullptr && "Skybox does not use lighting as of 5dec2025");

    glDepthFunc(GL_LEQUAL);

    m_skybox_mr->render(view, projection, nullptr);

    // m_shader->bind();
    // glm::mat4 skyView = glm::mat4(glm::mat3(view));
    // m_shader->setUniform("u_view", skyView);
    // m_shader->setUniform("u_projection", projection);
    // m_shader->setUniform("u_cubemap", 0);
    // glBindVertexArray(m_vao);
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemapTextureID);
    // glDrawArrays(GL_TRIANGLES, 0, 36);
    // glBindVertexArray(0);

    glDepthFunc(GL_LESS);
}