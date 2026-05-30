#include "../../include/systems/AnimationSystem.h"
#include "../../include/components/AnimationComponent.h"

void AnimationSystem::update(float deltaTime, std::vector<Entity*>& entities) {
    for (auto* entity : entities) {
        auto* anim = entity->getComponent<AnimationComponent>();
        if (anim) {
            anim->update(deltaTime);
        }
    }
}
