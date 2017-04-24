#pragma once
#include <cassert>
#include "libs.hpp"
#include "helpers.hpp"
#include "object.hpp"


class Ball : public Object {
private:
    GLuint m_program;
    GLint  m_loc;
    GLuint m_tex;
    GLint m_tex_loc;

    PV112::PV112Geometry m_sphere;
    glm::vec3 m_center;
    float m_radius;
public:
    Ball() = default;
    Ball(const GLuint program, const glm::vec3& center, const float radius)
     : Object({center, {radius, radius, radius}}),
       m_program(program), m_center(center), m_radius(radius)
    {
        this->init();
    }
    Ball(const GLuint program, const glm::vec3& center, const float radius,
        const Motion& motion)
     : Object({center, {radius, radius, radius}}, motion),
       m_program(program), m_center(center), m_radius(radius)
    {
        this->init();
    }

    virtual float mass() const final override {
        return 4. * 3.14 * m_radius * m_radius * m_radius / 3.;
    }
    glm::vec3 get_center() const {
        return m_center;
    }
    float get_radius() const {
        return m_radius;
    }

    void init() {
        int position_loc  = glGetAttribLocation(m_program, "position");
        int normal_loc    = glGetAttribLocation(m_program, "normal");
        int tex_coord_loc = glGetAttribLocation(m_program, "tex_coord");
        m_loc = glGetUniformLocation(m_program, "ball_tex");
        m_sphere = PV112::CreateSphere(position_loc, normal_loc, tex_coord_loc);

        this->bind_ball_texture();
    }

    void bind_ball_texture() {
        m_tex_loc = glGetUniformLocation(m_program, "ball_tex");

        m_tex = PV112::CreateAndLoadTexture("img/metal.jpg");
        glBindTexture(GL_TEXTURE_2D, m_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    virtual glm::mat4 update_geometry(const float time_delta) final override {
        // auto tmp_center = m_center;
        // tmp_center.x = m_center.x + glm::sin(time * m_motion.speed) / 2.;
        // tmp_center.y = m_center.y + glm::cos(time * m_motion.speed) / 2.;
        if (m_motion.active) {
            m_motion.account_gravity(time_delta);
            m_center = m_center + time_delta * m_motion.v;
            m_aabb.set_center(m_center);
        }
        auto model_matrix = glm::translate(glm::mat4(1.f), m_center);
        model_matrix = glm::scale(model_matrix, glm::vec3(m_radius));

        return model_matrix;
    }

    void render(const float time) final override {
        glBindVertexArray(m_sphere.VAO);

        glUniform1i(m_tex_loc, 0); // Choose proper texture unit
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_tex);
        DrawGeometry(m_sphere);
    }

    virtual bool check_collision(const Object& other) const final override {
        return other.check_collision_what(*this);
    }
    virtual glm::vec3 bounce_normal(const Object& other) const final override {
        return -other.bounce_normal_what(*this);
    }

    virtual bool check_collision_what(const Ball& other) const final override {
        return glm::length(m_center - other.m_center) <= m_radius + other.m_radius;
    }
    virtual bool check_collision_what(const Cuboid& other) const final override {
        return other.check_collision_what(*this);
    }
    virtual glm::vec3 bounce_normal_what(const Ball& other) const final override {
        // First, find the normalized vector n from the center of
        // circle1 to the center of circle2
        return glm::normalize(other.m_center - m_center);
    }
    virtual glm::vec3 bounce_normal_what(const Cuboid& other) const final override {
        return -other.bounce_normal_what(*this);
    }
};
