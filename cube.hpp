#pragma once

// Include the most important GLM functions
#include "libs.hpp"
#include "object.hpp"

class Cube : public Object {
private:
    GLuint m_program;
    GLint  m_loc;
    GLuint m_tex;
    GLint m_tex_loc;

    PV112::PV112Geometry m_cube;
    glm::vec3 m_center;
    float m_halfwidth;
public:
    Cube() = default;
    Cube(const GLuint program, const glm::vec3& center, const float halfwidth);

    void bind_cube_texture();

    virtual glm::mat4 update_geometry(const float time) final override;
    virtual void render(const float time_delta) final override;
    virtual bool check_collision(const Object& other) const final override;
    virtual float mass() const final override;

    virtual bool check_collision_what(const Ball& other) const final override;
    virtual bool check_collision_what(const Cube& other) const final override;

    virtual glm::vec3 bounce_normal(const Object& other) const final override;
    virtual glm::vec3 bounce_normal_what(const Ball& other) const final override;
    virtual glm::vec3 bounce_normal_what(const Cube& other) const final override;
};
