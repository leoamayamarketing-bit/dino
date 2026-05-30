#ifndef SYSTEMS_PHYSICS_H
#define SYSTEMS_PHYSICS_H

#include "../ecs/Entity.h"
#include "../core/Constants.h"

class PhysicsSystem : public System {
public:
    void update(float deltaTime, std::vector<Entity*>& entities) override;
    void setGravity(float gravity) { gravity_ = gravity; }
    float getGravity() const { return gravity_; }

private:
    float gravity_ = 1800.0f;
    float groundY_ = Constants::GROUND_Y;
};

#endif
