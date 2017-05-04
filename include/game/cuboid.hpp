#pragma once

// Include the most important GLM functions
#include "libs.hpp"
#include "object.hpp"
#include "PV112.h"

class Cuboid : public Object {
protected:
    GLuint m_program;
    GLuint m_tex;
    GLint m_tex_loc;

    PV112::PV112Geometry m_geometry;
    glm::vec3 m_center;
    glm::vec3 m_scale;
    glm::vec3 m_halfw;
public:
    Cuboid() = default;
    Cuboid(const GLuint program, const PV112::PV112Geometry& geometry,
        const GLuint tex, const glm::vec3& center, const glm::vec3& scale);
    Cuboid(const GLuint program, const PV112::PV112Geometry& geometry,
        const GLuint tex, const glm::vec3& center, const glm::vec3& scale,
        const Motion& motion);

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

private:
    static AABB init_aabb(AABB aabb, const glm::vec3& center,
        const glm::vec3& scale);
};

class Cube : public Cuboid {
public:
    Cube() = default;
    Cube(const GLuint program, const GLuint tex, const glm::vec3& center,
        const float scale)
     : Cuboid(program, get_cube_geometry(program), tex, center, glm::vec3(scale))
    {}
    Cube(const GLuint program, const GLuint tex, const glm::vec3& center,
        const float scale, const Motion& motion)
     : Cuboid(program, get_cube_geometry(program), tex, center, glm::vec3(scale),
        motion)
    {}

    static PV112::PV112Geometry get_cube_geometry(const GLuint program) {
        int position_loc  = glGetAttribLocation(program, "position");
        int normal_loc    = glGetAttribLocation(program, "normal");
        int tex_coord_loc = glGetAttribLocation(program, "tex_coord");
        return PV112::CreateCube(position_loc, normal_loc, tex_coord_loc);
    }
};
