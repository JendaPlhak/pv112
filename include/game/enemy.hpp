#pragma once
#include "cuboid.hpp"

class Enemy : public Cube {
public:
    struct Textures {
        std::vector<GLuint> texs;
    };
private:
    static constexpr float DISAPPEAR_AFTER = 5.;
    size_t m_hits = 0;
    std::vector<GLuint> m_textures;
    irrklang::ISoundEngine *m_sound;
public:

    Enemy() = default;
    template <class... Args>
    Enemy(const std::vector<GLuint>& textures, irrklang::ISoundEngine *sound,
        Args... args)
     : Cube(args...), m_textures(textures), m_sound(sound)
    {}

    void render(const float time) final override {
        const GLuint tex = m_textures.at(std::min(m_hits, m_textures.size() - 1));
        glBindVertexArray(m_geometry.VAO);

        glUniform1i(m_tex_loc, 0); // Choose proper texture unit
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        DrawGeometry(m_geometry);
    }

    bool is_alive() const {
        return m_hits < m_textures.size() - 1;
    }

    virtual void got_hit(const uint32_t other_id, const float time) {
        if (m_last_contact != other_id) {
            ++m_hits;
            if (is_alive()) {
                // Leaks memory
                m_sound->play2D("audio/hit.wav", GL_FALSE);
            } else if (m_hits == m_textures.size() - 1) {
                m_motion.active = true;
                // Leaks memory
                m_sound->play2D("audio/death.wav", GL_FALSE);
                this->set_expiration_time(time + DISAPPEAR_AFTER);
            }
        }
    }
    virtual float get_max_scale() const {
        return 1.;
    }
};
