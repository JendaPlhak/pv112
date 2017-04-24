#include <cassert>
#include "cuboid.hpp"
#include "ball.hpp"
#include "linalg.hpp"


Cuboid::Cuboid(const GLuint program, const PV112::PV112Geometry& geometry,
        const GLuint tex, const glm::vec3& center, const glm::vec3& scale)
 : Object(init_aabb(geometry.aabb, center, scale)),
   m_program(program), m_geometry(geometry), m_tex(tex), m_center(center),
   m_scale(scale), m_halfw(init_aabb(geometry.aabb, center, scale).get_halfwidths())
{
    this->init();
}

Cuboid::Cuboid(const GLuint program, const PV112::PV112Geometry& geometry,
        const GLuint tex, const glm::vec3& center, const glm::vec3& scale,
        const Motion& motion)
 : Object(init_aabb(geometry.aabb, center, scale), motion),
   m_program(program), m_geometry(geometry), m_tex(tex), m_center(center),
   m_scale(scale), m_halfw(init_aabb(geometry.aabb, center, scale).get_halfwidths())
{
    this->init();
}

AABB
Cuboid::init_aabb(AABB aabb, const glm::vec3& center, const glm::vec3& scale)
{
    aabb.set_center(center);
    aabb.apply_scale(scale);
    return aabb;
}

void
Cuboid::init() {
    this->bind_cube_texture();
}

void
Cuboid::bind_cube_texture() {
    m_tex_loc = glGetUniformLocation(m_program, "my_tex");
}

glm::mat4
Cuboid::update_geometry(const float time_delta) {
    if (m_motion.active) {
        m_motion.account_gravity(time_delta);
        m_center = m_center + time_delta * m_motion.v;
        m_aabb.set_center(m_center);
    }

    auto model_matrix = glm::translate(glm::mat4(1.f), m_center);
    model_matrix = glm::scale(model_matrix, m_scale);
    return model_matrix;
}

void
Cuboid::render(const float time) {
    glBindVertexArray(m_geometry.VAO);

    glUniform1i(m_tex_loc, 0); // Choose proper texture unit
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_tex);
    DrawGeometry(m_geometry);
}

float
Cuboid::mass() const {
    return 8 * m_halfw.x * m_halfw.y * m_halfw.z;
}

bool
Cuboid::check_collision(const Object& other) const {
    return other.check_collision_what(*this);
}
bool
Cuboid::check_collision_what(const Ball& ball) const {
    auto up = m_center + m_halfw;
    auto low = m_center - m_halfw;
    glm::vec3 clamp(0);
    clamp.x = std::max(low.x, std::min(ball.get_center().x, up.x));
    clamp.y = std::max(low.y, std::min(ball.get_center().y, up.y));
    clamp.z = std::max(low.z, std::min(ball.get_center().z, up.z));

    // this is the same as isPointInsideSphere
    float dst = glm::length(clamp - ball.get_center());

    // return m_aabb.check_collision(ball.get_aabb());
    return dst < ball.get_radius();
}
bool
Cuboid::check_collision_what(const Cuboid& other) const {
    return m_aabb.check_collision(other.get_aabb());
}

glm::vec3
Cuboid::bounce_normal(const Object& other) const {
    return -other.bounce_normal_what(*this);
}
glm::vec3
Cuboid::bounce_normal_what(const Ball& ball) const {
    std::vector<glm::vec3> normals = {
        {1, 0, 0}, {-1, 0, 0},
        {0, 1, 0}, {0, -1, 0},
        {0, 0, 1}, {0, 0, -1}
    };
    auto ball_c = ball.get_center();
    float best_dst = std::numeric_limits<float>::max();
    glm::vec3 best_n(0);

    for (const auto& n : normals) {
        auto p = plane_line_inter(n, m_center + n * m_halfw, m_center, ball_c);
        float dst = glm::length(m_center - p);
        // std::cout << "dst: "<< dst <<" normal: " << n.x << " " << n.y << " " << n.z << std::endl<< std::endl;
        if (glm::dot(m_center - p, ball_c - p) < 0 && dst < best_dst) {
            best_dst = dst;
            best_n = n;
        }
    }
    // std::cout << "Normal: " << best_n.x << " " << best_n.y << " " << best_n.z << std::endl<< std::endl;
    return best_n;
}
glm::vec3
Cuboid::bounce_normal_what(const Cuboid& other) const {
    float best_dst = std::numeric_limits<float>::max();
    glm::vec3 best_n(0);
    for (const auto sign : {1, -1}) {
        for (uint32_t i = 0; i < 3; ++i) {
            float a1 = m_center[i] + sign * m_halfw[i];
            float a2 = other.m_center[i] - sign * other.m_halfw[i];
            if (std::abs(a1 - a2) < best_dst) {
                best_dst = std::abs(a1 - a2);
                glm::vec3 n(0);
                n[i] = sign;
                best_n = n;
            }
        }
    }
    // std::cout << "Normal: " << best_n.x << " " << best_n.y << " " << best_n.z << std::endl;
    return best_n;
}
