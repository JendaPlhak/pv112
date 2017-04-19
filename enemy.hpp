#pragma once
#include "cuboid.hpp"

class Enemy : public Cube {
public:
    struct Textures {
        std::vector<GLuint> texs;
    };
private:
    size_t m_hits = 0;
    std::vector<GLuint> m_textures;
public:

    Enemy() = default;
    template <class... Args>
    Enemy(const std::vector<GLuint>& textures, Args... args)
     : Cube(args...), m_textures(textures)
    {}

    void render(const float time) final override {
        const GLuint tex = m_textures.at(std::min(m_hits, m_textures.size() - 1));
        glBindVertexArray(m_cube.VAO);

        glUniform1i(m_tex_loc, 0); // Choose proper texture unit
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        DrawGeometry(m_cube);
    }

    virtual void got_hit(const uint32_t other_id) {
        if (m_last_contact != other_id) {
            ++m_hits;
            if (m_hits >= m_textures.size() - 1) {
                m_motion.active = true;
            }
        }
    }
};