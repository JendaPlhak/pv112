#pragma once
#include "cuboid.hpp"

class Enemy : public Cube {
public:
    struct Textures {
        std::vector<GLuint> texs;
    };
private:
    static constexpr float DISAPPEAR_AFTER = 2.;
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

    bool kills_player(const glm::vec3 positon) {
        return glm::distance(m_center, positon) < 1.f && is_alive();
    }

    virtual glm::mat4 update_geometry(const float) {
        auto model_matrix = glm::translate(glm::mat4(1.f), m_center);
        model_matrix = glm::scale(model_matrix, m_scale);
        return model_matrix;
    }

    void maybe_activate(const glm::vec3 dir) {
        if (!m_motion.active) {
            m_motion = Motion(dir, 2.);
        }
    }

    void follow_player(const float time_delta, const glm::vec3 player_position) {


        m_motion.account_gravity(time_delta);
        auto to_player = player_position - m_center;
        this->maybe_activate(to_player);
        to_player.y = 0;

        m_center = m_center + time_delta * m_motion.v;
        m_aabb.set_center(m_center);
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
