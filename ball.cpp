#include "ball.h"

bool
Ball::check_collision_what(const Box& other) const{
    return m_aabb.check_collision(other.m_aabb);
}
