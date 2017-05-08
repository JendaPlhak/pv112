#pragma once

#include "game/libs.hpp"

struct MaterialProperties {
    glm::vec3 ambient_color;
    glm::vec3 diffuse_color;
    glm::vec3 specular_color;
    float shininess;

    MaterialProperties(glm::vec3 ambient = {0.3f, 0.3f, 0.3f},
        glm::vec3 diffuse = {1.0f, 1.0f, 1.0f},
        glm::vec3 specular = {1.0f, 1.0f, 1.0f},
        float shininess = 40.0f)
     : ambient_color(ambient), diffuse_color(diffuse), specular_color(specular),
       shininess(shininess)
    {}
};
