#include "../../include/entities/EnemyFactory.h"
#include "../../include/components/TransformComponent.h"
#include "../../include/components/SpriteComponent.h"
#include "../../include/components/PhysicsComponent.h"
#include "../../include/components/CollisionComponent.h"
#include "../../include/components/EnemyComponent.h"
#include "../../include/components/AnimationComponent.h"

std::unique_ptr<Entity> EnemyFactory::createEnemy(
    Constants::EnemyType type, AssetManager& assets, float startX) {

    static uint32_t nextId = 100;
    auto entity = std::make_unique<Entity>(nextId++);

    float y = Constants::GROUND_Y;
    std::string texName;
    sf::FloatRect bounds(startX, y, 30, 50);
    float damage = 1.0f;
    bool isAnimated = false;

    // Animation frame definitions (sprite sheet layout)
    enum AnimStyle { NONE, SWAY, FLY };
    AnimStyle animStyle = NONE;

    switch (type) {
        case Constants::EnemyType::SMALL_CACTUS:
            texName = "cactus_small";
            y = Constants::GROUND_Y - 40;
            bounds = sf::FloatRect(startX, y, 30, 50);
            damage = 1.0f;
            animStyle = SWAY;
            isAnimated = true;
            break;
        case Constants::EnemyType::LARGE_CACTUS:
            texName = "cactus_large";
            y = Constants::GROUND_Y - 60;
            bounds = sf::FloatRect(startX, y, 45, 70);
            damage = 2.0f;
            animStyle = SWAY;
            isAnimated = true;
            break;
        case Constants::EnemyType::PTERODACTYL:
            texName = "pterodactyl";
            y = Constants::GROUND_Y - 150;
            bounds = sf::FloatRect(startX, y, 60, 40);
            damage = 1.0f;
            animStyle = FLY;
            isAnimated = true;
            break;
        case Constants::EnemyType::ROLLING_ROCK:
            texName = "rock";
            y = Constants::GROUND_Y - 35;
            bounds = sf::FloatRect(startX, y, 50, 40);
            damage = 2.0f;
            break;
        case Constants::EnemyType::GROUND_ENEMY:
            texName = "ground_enemy";
            y = Constants::GROUND_Y - 25;
            bounds = sf::FloatRect(startX, y, 40, 30);
            damage = 3.0f;
            break;
    }

    entity->addComponent<TransformComponent>(startX, y);
    auto& sprite = entity->addComponent<SpriteComponent>(texName);
    if (assets.hasTexture(texName)) {
        sprite.setTexture(assets.getTexture(texName));
    }
    sprite.zOrder = 5;

    // Add animation for animated enemies (cactus sway, pterodactyl wings)
    if (isAnimated) {
        auto& anim = entity->addComponent<AnimationComponent>();

        switch (animStyle) {
            case SWAY: {
                // Cactus: 2 frames (sway left, sway right) in sprite sheet
                int fw = (type == Constants::EnemyType::SMALL_CACTUS) ? 30 : 45;
                int fh = (type == Constants::EnemyType::SMALL_CACTUS) ? 50 : 70;
                anim.addAnimation("idle", {
                    {sf::IntRect(0, 0, fw, fh), 0.6f},
                    {sf::IntRect(fw, 0, fw, fh), 0.6f}
                });
                anim.play("idle");
                anim.speed = 0.8f;
                break;
            }
            case FLY: {
                // Pterodactyl: 2 frames (wings up, wings down) in sprite sheet
                anim.addAnimation("fly", {
                    {sf::IntRect(0, 0, 60, 40), 0.15f},
                    {sf::IntRect(60, 0, 60, 40), 0.15f}
                });
                anim.play("fly");
                anim.speed = 1.2f;
                break;
            }
            default: break;
        }
    }

    auto& coll = entity->addComponent<CollisionComponent>(bounds, "enemy");
    coll.isTrigger = false;
    coll.isStatic = true;
    coll.damageAmount = static_cast<int>(damage);

    entity->addComponent<EnemyComponent>(type);

    return entity;
}
