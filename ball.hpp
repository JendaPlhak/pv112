#pragma once

#include "helpers.hpp"
#include "object.hpp"

struct Motion {
    float speed;
    glm::vec3 dir;
};

class Ball : public Object {
private:
    GLuint m_program;
    GLint  m_loc;
    GLuint m_tex;
    GLint m_tex_loc;

    PV112::PV112Geometry m_sphere;
    glm::vec3 m_center;
    float m_radius;
    Motion m_motion;
public:
    Ball() = default;
    Ball(const GLuint program)
     : Object({{1., 0, 0}, {1., 0, 0}}),
       m_program(program), m_center(0.f, 0.f, 0.f), m_radius(1.),
       m_motion({0.5, {1., 0, 0}})
    {
        int position_loc  = glGetAttribLocation(m_program, "position");
        int normal_loc    = glGetAttribLocation(m_program, "normal");
        int tex_coord_loc = glGetAttribLocation(m_program, "tex_coord");
        m_loc = glGetUniformLocation(m_program, "ball_tex");
        m_sphere = PV112::CreateSphere(position_loc, normal_loc, tex_coord_loc);

        this->bind_ball_texture();
    }

    void bind_ball_texture() {
        m_tex_loc = glGetUniformLocation(m_program, "ball_tex");

        m_tex = PV112::CreateAndLoadTexture("img/basketball-texture.jpg");
        glBindTexture(GL_TEXTURE_2D, m_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glm::mat4 get_model_matrix(const float time) {
        m_center.x = glm::sin(time * m_motion.speed);
        m_center.y = glm::cos(time * m_motion.speed);
        std::cout << m_center.x << std::endl;
        std::cout << m_center.y << std::endl;

        // m_center = m_center + time * m_motion.speed * m_motion.dir;
        return glm::translate(glm::mat4(1.f), m_center);;
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

private:
    virtual bool check_collision_what(const Ball& other) const final override {
        return glm::length(m_center - other.m_center) <= m_radius + other.m_radius;
    }
    virtual bool check_collision_what(const Box& other) const final override {
        return m_aabb.check_collision(other.m_aabb);
    }
};
