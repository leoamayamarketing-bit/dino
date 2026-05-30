#ifndef SYSTEMS_ANIMATION_H
#define SYSTEMS_ANIMATION_H

#include "../ecs/Entity.h"

class AnimationSystem : public System {
public:
    void update(float deltaTime, std::vector<Entity*>& entities) override;
};

#endif
