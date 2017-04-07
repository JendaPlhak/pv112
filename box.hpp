#pragma once

class Box : public Object {
private:
    glm::vec3 m_center;
    glm::vec3 m_x_dir;
    glm::vec3 m_y_dir;
    glm::vec3 m_z_dir;
public:
    Box()
     : m_center(0.f, 0.f, 0.f), m_x_dir({1., 0, 0}), m_y_dir({0., 1., 0}),
       m_z_dir({0., 0, 1})
    {

    }
    virtual bool check_collision(const Object& other) const final override {
        return other.check_collision_what(*this);
    }

private:
    virtual bool check_collision_what(const Ball& other) const final override {
        return m_aabb.check_collision(other.m_aabb);
    }
    virtual bool check_collision_what(const Box& other) const final override {
        return m_aabb.check_collision(other.m_aabb);
    }
};
