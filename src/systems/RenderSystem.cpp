#include "../../include/systems/RenderSystem.h"
#include "../../include/components/TransformComponent.h"
#include "../../include/components/SpriteComponent.h"
#include "../../include/components/AnimationComponent.h"
#include <algorithm>

RenderSystem::RenderSystem(sf::RenderWindow& window)
    : window_(window) {}

void RenderSystem::update(float, std::vector<Entity*>& entities) {
    // Sort by z-order
    std::sort(entities.begin(), entities.end(), [](Entity* a, Entity* b) {
        auto* sa = a->getComponent<SpriteComponent>();
        auto* sb = b->getComponent<SpriteComponent>();
        int za = sa ? sa->zOrder : 0;
        int zb = sb ? sb->zOrder : 0;
        return za < zb;
    });

    for (auto* entity : entities) {
        auto* sprite = entity->getComponent<SpriteComponent>();
        auto* transform = entity->getComponent<TransformComponent>();
        auto* anim = entity->getComponent<AnimationComponent>();

        if (!sprite || !sprite->visible) continue;

        if (anim && anim->currentFrames && !anim->currentFrames->empty()) {
            sf::IntRect frame = anim->getCurrentFrame();
            sprite->sprite.setTextureRect(frame);
            if (anim->flipped) {
                sprite->sprite.setScale(-std::abs(transform->scale.x), transform->scale.y);
            } else {
                sprite->sprite.setScale(transform->scale);
            }
        }

        if (transform) {
            sprite->sprite.setPosition(transform->position);
        }

        window_.draw(sprite->sprite);
    }
}

void RenderSystem::setView(const sf::View& view) {
    view_ = view;
    window_.setView(view_);
}
