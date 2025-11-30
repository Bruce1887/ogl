#pragma once
#include "Common.h"

// Forward declaration to avoid circular dependency
class MeshRenderable;

struct PhongLightConfig
{
    glm::vec3 lightPosition;
    glm::vec3 ambientLight;
    glm::vec3 diffuseLight;
    glm::vec3 specularLight;
};

struct LightSource
{
    PhongLightConfig config; // Light properties
    std::unique_ptr<MeshRenderable> visualRepresentation = nullptr; // Optional renderable to visualize the light source

    static LightSource createDefaultLightSource()
{        
    PhongLightConfig lightConfig{
        .lightPosition = glm::vec3(15.0f, 10.0f, 10.0f),
        .ambientLight = glm::vec3(0.2f, 0.2f, 0.2f),
        .diffuseLight = glm::vec3(1.0f, 1.0f, 0.7f),
        .specularLight = glm::vec3(1.0f, 1.0f, 1.0f)};
    
    return LightSource{.config = lightConfig, .visualRepresentation = nullptr};
}
};