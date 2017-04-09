#pragma once

// Include the most important GLM functions
#include "libs.hpp"
#include "object.hpp"

class Cuboid : public Object {
protected:
    GLuint m_program;
    GLint  m_loc;
    GLuint m_tex;
    GLint m_tex_loc;

    PV112::PV112Geometry m_cube;
    glm::vec3 m_center;
    glm::vec3 m_halfw;
public:
    Cuboid() = default;
    Cuboid(const GLuint program, const glm::vec3& center,
        const glm::vec3& widths);
    Cuboid(const GLuint program, const glm::vec3& center,
        const glm::vec3& widths, const Motion& motion);

    virtual glm::mat4 update_geometry(const float time) final override;
    virtual void render(const float time_delta) override;
    virtual bool check_collision(const Object& other) const final override;
    virtual float mass() const final override;

    virtual bool check_collision_what(const Ball& other) const final override;
    virtual bool check_collision_what(const Cuboid& other) const final override;

    virtual glm::vec3 bounce_normal(const Object& other) const final override;
    virtual glm::vec3 bounce_normal_what(const Ball& other) const final override;
    virtual glm::vec3 bounce_normal_what(const Cuboid& other) const final override;

protected:
    void bind_cube_texture();
    void init();
};

class Cube : public Cuboid {
public:
    Cube() = default;
    Cube(const GLuint program, const glm::vec3& center, const float width)
     : Cuboid(program, center, glm::vec3(width, width, width))
    {}
    Cube(const GLuint program, const glm::vec3& center, const float width,
        const Motion& motion)
     : Cuboid(program, center, glm::vec3(width, width, width), motion)
    {}
};
