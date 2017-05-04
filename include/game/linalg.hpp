#pragma once
#include "libs.hpp"

inline glm::vec3 point_to_plane_proj(glm::vec3 a, glm::vec3 n, glm::vec3 p) {
    n = glm::normalize(n);
    auto v = p - a;
    float dst = glm::dot(v, n);
    return p - dst * n;
}

inline glm::vec3 plane_line_inter(glm::vec3 n, glm::vec3 p, glm::vec3 a, glm::vec3 b) {
    n = glm::normalize(n);
    float d = glm::dot(n, p);
    auto ba = b-a;
    float nDotA = glm::dot(n, a);
    float nDotBA = glm::dot(n, ba);

    return a + (((d - nDotA)/nDotBA) * ba);
}


