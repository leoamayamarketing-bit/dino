#include "../../include/entities/PlayerFactory.h"
#include "../../include/components/TransformComponent.h"
#include "../../include/components/SpriteComponent.h"
#include "../../include/components/PhysicsComponent.h"
#include "../../include/components/CollisionComponent.h"
#include "../../include/components/PlayerComponent.h"
#include "../../include/components/AnimationComponent.h"

std::unique_ptr<Entity> PlayerFactory::createPlayer(
    Constants::DinoType type, AssetManager& assets, float startX, float startY) {

    auto entity = std::make_unique<Entity>(1);

    std::string texName;
    switch (type) {
        case Constants::DinoType::TREX:
            texName = "dino_trex"; break;
        case Constants::DinoType::VELOCIRAPTOR:
            texName = "dino_raptor"; break;
        case Constants::DinoType::TRICERATOPS:
            texName = "dino_triceratops"; break;
    }

    entity->addComponent<TransformComponent>(startX, startY);
    auto& sprite = entity->addComponent<SpriteComponent>(texName);
    if (assets.hasTexture(texName)) {
        sprite.setTexture(assets.getTexture(texName));
    }
    sprite.zOrder = 10;

    // Set up named animation states from the sprite sheet (4 frames, 128x72 each)
    auto& anim = entity->addComponent<AnimationComponent>();
    anim.addAnimation("run", {
        {sf::IntRect(0, 0, 128, 72), 0.12f},    // run pose 1
        {sf::IntRect(128, 0, 128, 72), 0.12f}    // run pose 2
    });
    anim.addAnimation("jump", {
        {sf::IntRect(256, 0, 128, 72), 0.15f}    // jump pose (single frame)
    });
    anim.addAnimation("crouch", {
        {sf::IntRect(384, 0, 128, 72), 0.15f}    // crouch pose (single frame)
    });
    anim.play("run");
    anim.looping = true;
    anim.speed = 1.5f;

    auto& physics = entity->addComponent<PhysicsComponent>();
    physics.usesGravity = true;
    physics.jumpsAvailable = 2;

    auto& coll = entity->addComponent<CollisionComponent>(
        sf::FloatRect(startX + 20, startY + 10, 70, 55), "player");
    coll.isTrigger = false;
    coll.isStatic = false;

    entity->addComponent<PlayerComponent>(type);

    return entity;
}
