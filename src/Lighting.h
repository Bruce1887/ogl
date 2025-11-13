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
    MeshRenderable *visualRepresentation = nullptr; // Optional renderable to visualize the light source
};