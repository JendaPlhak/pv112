#include <cassert>
#include "cube.hpp"
#include "ball.hpp"
#include "linalg.hpp"


Cube::Cube(const GLuint program, const glm::vec3& center, const float halfwidth)
 : Object({center, {halfwidth, halfwidth, halfwidth}}),
   m_program(program), m_center(center), m_halfwidth(halfwidth)
{
    int position_loc  = glGetAttribLocation(m_program, "position");
    int normal_loc    = glGetAttribLocation(m_program, "normal");
    int tex_coord_loc = glGetAttribLocation(m_program, "tex_coord");
    m_loc = glGetUniformLocation(m_program, "ball_tex");
    m_cube = PV112::CreateCube(position_loc, normal_loc, tex_coord_loc);

    this->bind_cube_texture();
}

void
Cube::bind_cube_texture() {
    m_tex_loc = glGetUniformLocation(m_program, "ball_tex");

    m_tex = PV112::CreateAndLoadTexture("img/wood.jpg");
    glBindTexture(GL_TEXTURE_2D, m_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

glm::mat4
Cube::update_geometry(const float time_delta) {
    m_center = m_center + time_delta * m_motion.v;
    m_aabb.set_center(m_center);

    auto model_matrix = glm::translate(glm::mat4(1.f), m_center);
    model_matrix = glm::scale(model_matrix, glm::vec3(m_halfwidth));
    return model_matrix;
}

void
Cube::render(const float time) {
    glBindVertexArray(m_cube.VAO);

    glUniform1i(m_tex_loc, 0); // Choose proper texture unit
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_tex);
    DrawGeometry(m_cube);
}

float
Cube::mass() const {
    return 8 * m_halfwidth * m_halfwidth * m_halfwidth;
}

bool
Cube::check_collision(const Object& other) const {
    return other.check_collision_what(*this);
}
bool
Cube::check_collision_what(const Ball& ball) const {
    float w = m_halfwidth;
    glm::vec3 clamp(0);
    clamp.x = std::max(m_center.x - w, std::min(ball.get_center().x, m_center.x + w));
    clamp.y = std::max(m_center.y - w, std::min(ball.get_center().y, m_center.y + w));
    clamp.z = std::max(m_center.z - w, std::min(ball.get_center().z, m_center.z + w));

    // this is the same as isPointInsideSphere
    float dst = glm::length(clamp - ball.get_center());

    // return m_aabb.check_collision(ball.get_aabb());
    return dst < ball.get_radius();
}
bool
Cube::check_collision_what(const Cube& other) const {
    return m_aabb.check_collision(other.get_aabb());
}

glm::vec3
Cube::bounce_normal(const Object& other) const {
    return other.bounce_normal_what(*this);
}
glm::vec3
Cube::bounce_normal_what(const Ball& ball) const {
    std::vector<glm::vec3> normals = {
        {1, 0, 0}, {-1, 0, 0},
        {0, 1, 0}, {0, -1, 0},
        {0, 0, -1}, {0, 0, -1}
    };
    auto ball_c = ball.get_center();
    float best_dst = std::numeric_limits<float>::max();
    glm::vec3 best_n(0);
    for (const auto& n : normals) {
        auto p_center = m_center + n * m_halfwidth;
        auto p = plane_line_inter(n, m_center + n * m_halfwidth, m_center, ball_c);
        float dst = glm::length(m_center - p);
        if (glm::dot(m_center - p, ball_c - p) < 0 && dst < best_dst) {
            return n;
        }
    }
    // std::cout << best_n.x << " " << best_n.y << " " << best_n.z << std::endl;
    return best_n;
}
glm::vec3
Cube::bounce_normal_what(const Cube& other) const {
    assert(false);
}
