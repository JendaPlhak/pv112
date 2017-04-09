#pragma once
#include <cstdint>
#include "libs.hpp"

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

    void set_center(const glm::vec3& new_center) {
        m_center = new_center;
    }
    glm::vec3 get_center() const {
        return m_center;
    }

    bool check_collision(const AABB& other) const {
        return std::abs(m_center.x - other.m_center.x) <= m_halfwidths.x + other.m_halfwidths.x
            && std::abs(m_center.y - other.m_center.y) <= m_halfwidths.y + other.m_halfwidths.y
            && std::abs(m_center.z - other.m_center.z) <= m_halfwidths.z + other.m_halfwidths.z;
    }
};

struct Motion {
    Motion(const glm::vec3& dir, const float speed)
     : v(speed * glm::normalize(dir)), bounciness(1), active(true)
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
protected:
    const uint32_t m_id;
    uint32_t m_last_contact = -1;
    AABB m_aabb;
    Motion m_motion;
    bool m_fixed = false;

public:
    Object() = default;
    Object(const AABB& aabb)
     : m_id(COUNT++), m_aabb(aabb), m_motion(glm::vec3(1), 0)
    { }
    Object(const AABB& aabb, const Motion& motion)
     : m_id(COUNT++), m_aabb(aabb), m_motion(motion)
    { }
    const AABB& get_aabb() const {
        return m_aabb;
    }
    const bool is_active() const {
        return m_motion.active;
    }
    virtual glm::mat4 update_geometry(const float time_delta) = 0;
    virtual void render(const float time) = 0;
    virtual bool check_collision(const Object&) const = 0;
    virtual bool check_collision_what(const Ball&) const = 0;
    virtual bool check_collision_what(const Cuboid&) const = 0;

    virtual float mass() const = 0;
    virtual void bounce(Object& other) {
        if (!this->is_active() && !other.is_active()) {
            return;
        } else if (m_last_contact == other.m_id && other.m_last_contact == m_id) {
            if (m_motion.active && other.m_motion.active) {
                return;
            }
        }

        auto n = glm::normalize(this->bounce_normal(other));
        std::cout << "Normal loaded: " << n.x << " " << n.y << " " << n.z << std::endl;
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
        this->got_hit(other.m_id);
        other.got_hit(m_id);
        m_last_contact = other.m_id;
        other.m_last_contact = m_id;
    }
    virtual glm::vec3 bounce_normal(const Object&) const = 0;
    virtual glm::vec3 bounce_normal_what(const Ball&) const = 0;
    virtual glm::vec3 bounce_normal_what(const Cuboid&) const = 0;
    virtual void got_hit(const uint32_t other_id) {
    }
};
