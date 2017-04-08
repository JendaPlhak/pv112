#pragma once
#include <cstdint>
#include "libs.hpp"

class Ball;
class Cube;

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

    bool check_collision(const AABB& other) const {
        return std::abs(m_center.x - other.m_center.x) <= m_halfwidths.x + other.m_halfwidths.x
            && std::abs(m_center.y - other.m_center.y) <= m_halfwidths.y + other.m_halfwidths.y
            && std::abs(m_center.z - other.m_center.z) <= m_halfwidths.z + other.m_halfwidths.z;
    }
};

struct Motion {
    Motion(const glm::vec3& dir, const float speed)
     : v(speed * glm::normalize(dir))
    {}
    glm::vec3 v;
};

class Object {
private:
    static uint32_t COUNT;
protected:
    const uint32_t m_id;
    uint32_t m_last_contact = -1;
    AABB m_aabb;
    Motion m_motion;

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
    virtual glm::mat4 update_geometry(const float time_delta) = 0;
    virtual void render(const float time) = 0;
    virtual bool check_collision(const Object&) const = 0;
    virtual bool check_collision_what(const Ball&) const = 0;
    virtual bool check_collision_what(const Cube&) const = 0;

    virtual float mass() const = 0;
    virtual void bounce(Object& other) {
        if (m_last_contact == other.m_id || other.m_last_contact == m_id) {
            return;
        } else {
            m_last_contact = other.m_id;
            other.m_last_contact = m_id;
        }
        // std::cout << "__________________________________\n\n";
        auto n = glm::normalize(this->bounce_normal(other));
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
    virtual glm::vec3 bounce_normal(const Object&) const = 0;
    virtual glm::vec3 bounce_normal_what(const Ball&) const = 0;
    virtual glm::vec3 bounce_normal_what(const Cube&) const = 0;
};
