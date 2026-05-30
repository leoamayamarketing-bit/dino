#include "../../include/entities/EnemyFactory.h"
#include "../../include/components/TransformComponent.h"
#include "../../include/components/SpriteComponent.h"
#include "../../include/components/PhysicsComponent.h"
#include "../../include/components/CollisionComponent.h"
#include "../../include/components/EnemyComponent.h"
#include "../../include/components/AnimationComponent.h"

// Helper: set sprite origin to bottom-center so the entity stands ON the ground.
static void setOriginBottom(SpriteComponent& sprite) {
    if (sprite.sprite.getTexture()) {
        sf::Vector2u texSize = sprite.sprite.getTexture()->getSize();
        sprite.origin = sf::Vector2f(texSize.x / 2.0f, static_cast<float>(texSize.y));
        sprite.sprite.setOrigin(sprite.origin);
    }
}

// Helper: set sprite origin to center for flying entities.
static void setOriginCenter(SpriteComponent& sprite) {
    if (sprite.sprite.getTexture()) {
        sf::Vector2u texSize = sprite.sprite.getTexture()->getSize();
        sprite.origin = sf::Vector2f(texSize.x / 2.0f, texSize.y / 2.0f);
        sprite.sprite.setOrigin(sprite.origin);
    }
}

std::unique_ptr<Entity> EnemyFactory::createEnemy(
    Constants::EnemyType type, AssetManager& assets, float startX) {

    static uint32_t nextId = 100;
    auto entity = std::make_unique<Entity>(nextId++);

    float y = Constants::GROUND_Y;
    std::string texName;
    sf::FloatRect bounds(0, 0, 30, 50);
    sf::Vector2f localOffset{0.0f, 0.0f};
    float damage = 1.0f;

    switch (type) {
        case Constants::EnemyType::SMALL_CACTUS:
            texName = "cactus_small";
            // With bottom-center origin, Y = GROUND_Y means feet on ground
            y = Constants::GROUND_Y;
            bounds = sf::FloatRect(0, 0, 100, 230);
            localOffset = sf::Vector2f(-50.0f, -230.0f);
            damage = 1.0f;
            break;
        case Constants::EnemyType::LARGE_CACTUS:
            texName = "cactus_large";
            y = Constants::GROUND_Y;
            bounds = sf::FloatRect(0, 0, 200, 240);
            localOffset = sf::Vector2f(-100.0f, -240.0f);
            damage = 2.0f;
            break;
        case Constants::EnemyType::PTERODACTYL:
            texName = "pterodactyl";
            // With center origin, Y is the center of the bird
            // ave.png is 314x207, so collision box matches full sprite
            y = Constants::GROUND_Y - 180;
            bounds = sf::FloatRect(0, 0, 280, 160);
            localOffset = sf::Vector2f(-140.0f, -80.0f);
            damage = 1.0f;
            break;
        case Constants::EnemyType::ROLLING_ROCK:
            texName = "rock";
            y = Constants::GROUND_Y;
            bounds = sf::FloatRect(0, 0, 50, 40);
            localOffset = sf::Vector2f(-25.0f, -40.0f);
            damage = 2.0f;
            break;
        case Constants::EnemyType::GROUND_ENEMY:
            texName = "ground_enemy";
            y = Constants::GROUND_Y;
            bounds = sf::FloatRect(0, 0, 40, 30);
            localOffset = sf::Vector2f(-20.0f, -30.0f);
            damage = 3.0f;
            break;
    }

    entity->addComponent<TransformComponent>(startX, y);
    auto& sprite = entity->addComponent<SpriteComponent>(texName);
    if (assets.hasTexture(texName)) {
        sprite.setTexture(assets.getTexture(texName));
    }
    sprite.zOrder = 5;

    // Set appropriate origin based on entity type
    if (type == Constants::EnemyType::PTERODACTYL) {
        setOriginCenter(sprite);
    } else {
        // Cacti and ground entities stand on the ground
        setOriginBottom(sprite);
    }

    // Note: AnimationComponent is intentionally NOT added for PNG-based enemies
    // (cactus, pterodactyl) because they are single images, not sprite sheets.
    // The RenderSystem's setTextureRect would shrink them to 1x1 pixels.
    // Movement/behavior is handled by EnemyAISystem (scroll, bob, etc.).
    // Scale-based sway/flap animation can be added in EnemyAISystem if desired.

    auto& coll = entity->addComponent<CollisionComponent>(bounds, "enemy");
    coll.localOffset = localOffset;
    coll.isTrigger = false;
    coll.isStatic = true;
    coll.damageAmount = static_cast<int>(damage);

    entity->addComponent<EnemyComponent>(type);

    return entity;
}
