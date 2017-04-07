#pragma once

class Ball;
class Box;

class AABB {
private:
    glm::vec3 m_center;
    glm::vec3 m_halfwidths;
public:
    AABB(const glm::vec3 center, const glm::vec3 halfwidths)
     : m_center(center), m_halfwidths(halfwidths)
    { }

    bool check_collision(const AABB& other) {
        return std::abs(m_center.x - other.m_center.x) <= m_halfwidths.x + other.m_halfwidths.x
            && std::abs(m_center.y - other.m_center.y) <= m_halfwidths.y + other.m_halfwidths.y
            && std::abs(m_center.z - other.m_center.z) <= m_halfwidths.z + other.m_halfwidths.z;
    }
};

class Object {
private:
    AABB m_aabb;

public:
    Object(const AABB& aabb)
     : m_aabb(aabb)
    { }
    virtual void render(const float time) = 0;
    virtual bool check_collision(const Object&) const = 0;
    virtual bool check_collision_what(const Ball&) const = 0;
    virtual bool check_collision_what(const Box&) const = 0;
};
