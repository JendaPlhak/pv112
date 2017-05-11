#pragma once
#include <array>
#include <cstdint>
#include <limits>
#include <vector>
#include "libs.hpp"
#include "game/material_properties.hpp"

class Ball;
class Cuboid;

class AABB {
private:
    glm::vec3 m_center;
    glm::vec3 m_halfwidths;
public:
    AABB() = default;
    AABB(const glm::vec3 center, const glm::vec3 halfwidths)
     : m_center(center), m_halfwidths(halfwidths)
    { }
    AABB(const std::vector<glm::vec3>& vertices) {
        std::array<std::pair<float, float>, 3> boundaries;
        boundaries.fill({
            std::numeric_limits<float>::max(), std::numeric_limits<float>::min()
        });
        for (const auto& vertex: vertices) {
            for (uint i = 0; i < 3; ++i) {
                boundaries[i].first  = std::min(boundaries[i].first, vertex[i]);
                boundaries[i].second = std::max(boundaries[i].second, vertex[i]);
            }
        }
        for (uint i = 0; i < 3; ++i) {
            m_center[i] = (boundaries[i].first + boundaries[i].second) / 2.;
            m_halfwidths[i] = (boundaries[i].second - boundaries[i].first) / 2.;
        }
    }

    void set_center(const glm::vec3& new_center) {
        m_center = new_center;
    }
    void set_halfwidths(const glm::vec3& new_halfwidths) {
        m_halfwidths = new_halfwidths;
    }
    glm::vec3 get_center() const {
        return m_center;
    }
    glm::vec3 get_halfwidths() const {
        return m_halfwidths;
    }
    void apply_scale(const glm::vec3& scale) {
        m_halfwidths *= scale;
    }


    bool check_collision(const AABB& other) const {
        return std::abs(m_center.x - other.m_center.x) <= m_halfwidths.x + other.m_halfwidths.x
            && std::abs(m_center.y - other.m_center.y) <= m_halfwidths.y + other.m_halfwidths.y
            && std::abs(m_center.z - other.m_center.z) <= m_halfwidths.z + other.m_halfwidths.z;
    }
};

struct Motion {
    Motion(const glm::vec3& dir, const float speed)
     : v(speed * glm::normalize(dir)), bounciness(1.), active(true)
    {}
    Motion(const glm::vec3& dir, const float speed, const float bounciness)
     : v(speed * glm::normalize(dir)), bounciness(bounciness), active(true)
    {}
    Motion(const bool is_active)
     : v(0), bounciness(0), active(is_active)
    {
        assert(!active);
    }
    void account_gravity(const float time_delta) {
        if (this->active) {
            this->v += time_delta * 3.f * glm::normalize(glm::vec3(0, -1., 0.));
        }
    }
    glm::vec3 v;
    float bounciness;
    bool active;
};

class Object {
private:
    static uint32_t COUNT;
    MaterialProperties m_mat_properties;
protected:
    const uint32_t m_id;
    uint32_t m_last_contact = -1;
    AABB m_aabb;
    Motion m_motion;
    bool m_fixed = false;
    float m_expiration_time;

public:
    Object() = default;
    Object(const AABB& aabb)
     : m_id(COUNT++), m_aabb(aabb), m_motion(glm::vec3(1), 0),
       m_expiration_time(std::numeric_limits<float>::max())
    { }
    Object(const AABB& aabb, const Motion& motion)
     : m_id(COUNT++), m_aabb(aabb), m_motion(motion),
       m_expiration_time(std::numeric_limits<float>::max())
    { }
    const AABB& get_aabb() const {
        return m_aabb;
    }
    const bool is_active() const {
        return m_motion.active;
    }
    const MaterialProperties& get_material_properties() const {
        return m_mat_properties;
    }
    void set_material_properties(const MaterialProperties& properties) {
        m_mat_properties = properties;
    }
    bool is_expired(const float time) {
        return m_expiration_time <= time;
    }
    void set_expiration_time(const float time) {
        m_expiration_time = time;
    }

    virtual float get_max_scale() const {
        auto widths = m_aabb.get_halfwidths();
        widths *= 2.;
        return std::max(std::max(widths[0], widths[1]), widths[2]);
    }
    virtual glm::mat4 update_geometry(const float time_delta) = 0;
    virtual void render(const float time) = 0;
    virtual bool check_collision(const Object&) const = 0;
    virtual bool check_collision_what(const Ball&) const = 0;
    virtual bool check_collision_what(const Cuboid&) const = 0;

    virtual float mass() const = 0;
    virtual void bounce(Object& other, const float time) {
        if (!this->is_active() && !other.is_active()) {
            return;
        } else if (m_last_contact == other.m_id && other.m_last_contact == m_id) {
            if (m_motion.active && other.m_motion.active) {
                return;
            }
        }

        auto n = glm::normalize(this->bounce_normal(other));
        // std::cout << "Normal loaded: " << n.x << " " << n.y << " " << n.z << std::endl;
        auto update_motion = [](auto& v, glm::vec3 n) {
            v = v - 2 * std::min(0.f, glm::dot(v, n)) * n;
        };
        if (!m_motion.active) {
            // std::cout << "This is static\n";
            update_motion(other.m_motion.v, n);
        } else if (!other.m_motion.active) {
            // std::cout << "Other is static\n";
            update_motion(m_motion.v, -n);
        } else {
            // Find the length of the component of each of the movement
            // vectors along n.
            // a1 = v1 . n
            // a2 = v2 . n
            // std::cout << "this: " << m_motion.v.x << " " << m_motion.v.y << " " << m_motion.v.z << std::endl;
            // std::cout << "other: " << other.m_motion.v.x << " " << other.m_motion.v.y << " " <<other.m_motion.v.z << std::endl;
            float a1 = glm::dot(m_motion.v, n);
            float a2 = glm::dot(other.m_motion.v, n);
            // std::cout << "a1 = " << a1 << ", a2 = " << a2 << std::endl;

            // Using the optimized version,
            float optimizedP = (2.0 * (a1 - a2)) / (this->mass() + other.mass());

            m_motion.v = m_motion.v - optimizedP * other.mass() * n;
            other.m_motion.v = other.m_motion.v + optimizedP * this->mass() * n;
        }

        if (m_last_contact != other.m_id || other.m_last_contact != m_id) {
            float bounciness = std::max(m_motion.bounciness, other.m_motion.bounciness);
            m_motion.v *= bounciness;
            other.m_motion.v *= bounciness;
        }
        this->got_hit(other.m_id, time);
        other.got_hit(m_id, time);
        m_last_contact = other.m_id;
        other.m_last_contact = m_id;
    }
    virtual glm::vec3 bounce_normal(const Object&) const = 0;
    virtual glm::vec3 bounce_normal_what(const Ball&) const = 0;
    virtual glm::vec3 bounce_normal_what(const Cuboid&) const = 0;
    virtual void got_hit(const uint32_t other_id, const float time) {
    }
};
