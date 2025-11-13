#pragma once
#include "Common.h"
#include "Renderable.h"

struct PhongLightConfig
{
    glm::vec3 lightPosition;
    glm::vec3 ambientLight;
    glm::vec3 diffuseLight;
    glm::vec3 specularLight;
};

