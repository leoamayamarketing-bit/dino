#include "../../include/entities/ObstacleFactory.h"
#include "../../include/entities/EnemyFactory.h"
#include "../../include/core/GameState.h"
#include "../../include/components/TransformComponent.h"
#include "../../include/components/SpriteComponent.h"
#include "../../include/components/CollisionComponent.h"
#include "../../include/components/CoinComponent.h"
#include "../../include/components/PowerUpComponent.h"
#include <cstdlib>

std::unique_ptr<Entity> ObstacleFactory::createSmallCactus(AssetManager& assets, float x) {
    return EnemyFactory::createEnemy(Constants::EnemyType::SMALL_CACTUS, assets, x);
}

std::unique_ptr<Entity> ObstacleFactory::createLargeCactus(AssetManager& assets, float x) {
    return EnemyFactory::createEnemy(Constants::EnemyType::LARGE_CACTUS, assets, x);
}

std::unique_ptr<Entity> ObstacleFactory::createRock(AssetManager& assets, float x) {
    return EnemyFactory::createEnemy(Constants::EnemyType::ROLLING_ROCK, assets, x);
}

std::unique_ptr<Entity> ObstacleFactory::createStalactite(AssetManager& assets, float x) {
    static uint32_t nextId = 500;
    auto entity = std::make_unique<Entity>(nextId++);
    float y = 0;

    entity->addComponent<TransformComponent>(x, y);
    auto& sprite = entity->addComponent<SpriteComponent>("stalactite");
    if (assets.hasTexture("stalactite")) {
        sprite.setTexture(assets.getTexture("stalactite"));
    }
    sprite.zOrder = 5;

    auto& coll = entity->addComponent<CollisionComponent>(
        sf::FloatRect(x, y, 30, 60), "enemy");
    coll.isTrigger = false;

    return entity;
}

std::unique_ptr<Entity> ObstacleFactory::createCoin(AssetManager& assets, float x, float y) {
    static uint32_t nextId = 200;
    auto entity = std::make_unique<Entity>(nextId++);

    entity->addComponent<TransformComponent>(x, y);
    auto& sprite = entity->addComponent<SpriteComponent>("coin");
    if (assets.hasTexture("coin")) {
        sprite.setTexture(assets.getTexture("coin"));
    }
    sprite.zOrder = 5;

    auto& coll = entity->addComponent<CollisionComponent>(
        sf::FloatRect(x, y, 24, 24), "coin");
    coll.isTrigger = true;

    entity->addComponent<CoinComponent>(Constants::COIN_SCORE);

    return entity;
}

std::unique_ptr<Entity> ObstacleFactory::createPowerUp(
    AssetManager& assets, Constants::PowerUpType type, float x, float y) {
    static uint32_t nextId = 300;
    auto entity = std::make_unique<Entity>(nextId++);

    entity->addComponent<TransformComponent>(x, y);
    
    PowerUpComponent pu(type);
    auto& sprite = entity->addComponent<SpriteComponent>(pu.getTextureName());
    if (assets.hasTexture(pu.getTextureName())) {
        sprite.setTexture(assets.getTexture(pu.getTextureName()));
    }
    sprite.zOrder = 5;

    auto& coll = entity->addComponent<CollisionComponent>(
        sf::FloatRect(x, y, 30, 30), "powerup");
    coll.isTrigger = true;

    entity->addComponent<PowerUpComponent>(type);

    return entity;
}

std::vector<std::unique_ptr<Entity>> ObstacleFactory::generateObstaclePattern(
    Constants::LevelType level, AssetManager& assets,
    float startX, int patternLength) {
    
    std::vector<std::unique_ptr<Entity>> pattern;
    float x = startX;

    for (int i = 0; i < patternLength; i++) {
        int roll = std::rand() % 100;
        
        if (roll < 30) {
            pattern.push_back(createSmallCactus(assets, x));
            x += 40 + (std::rand() % 60);
        } else if (roll < 50) {
            pattern.push_back(createLargeCactus(assets, x));
            x += 50 + (std::rand() % 70);
        } else if (roll < 65) {
            pattern.push_back(createCoin(assets, x, Constants::GROUND_Y - 60 - (std::rand() % 80)));
            x += 30;
        } else if (level == Constants::LevelType::INFINITE && roll < 75) {
            pattern.push_back(createRock(assets, x));
            x += 50 + (std::rand() % 40);
        } else {
            x += 80 + (std::rand() % 100);
        }
    }

    return pattern;
}

void ObstacleFactory::spawnRandomObstacle(AssetManager& assets, GameState& state, Constants::LevelType level) {
    float startX = Constants::WINDOW_WIDTH + 100;
    int roll = std::rand() % 100;

    std::unique_ptr<Entity> entity;
    if (roll < 40) {
        entity = EnemyFactory::createEnemy(Constants::EnemyType::SMALL_CACTUS, assets, startX);
    } else if (roll < 65) {
        entity = EnemyFactory::createEnemy(Constants::EnemyType::LARGE_CACTUS, assets, startX);
    } else if (roll < 80) {
        entity = EnemyFactory::createEnemy(Constants::EnemyType::PTERODACTYL, assets, startX);
    } else if (roll < 92) {
        entity = EnemyFactory::createEnemy(Constants::EnemyType::ROLLING_ROCK, assets, startX);
    } else {
        entity = EnemyFactory::createEnemy(Constants::EnemyType::GROUND_ENEMY, assets, startX);
    }

    if (entity) {
        state.entities.push_back(std::move(entity));
    }
}

void ObstacleFactory::spawnCoinCluster(AssetManager& assets, GameState& state, float x, float y, int count) {
    for (int i = 0; i < count; i++) {
        auto coin = createCoin(assets, x + i * 30.0f, y + (std::rand() % 40 - 20));
        state.entities.push_back(std::move(coin));
    }
}

void ObstacleFactory::spawnRandomPowerUp(AssetManager& assets, GameState& state) {
    Constants::PowerUpType types[] = {
        Constants::PowerUpType::SHIELD,
        Constants::PowerUpType::MAGNET,
        Constants::PowerUpType::DOUBLE_POINTS,
        Constants::PowerUpType::SPEED_BOOST,
        Constants::PowerUpType::EXTRA_LIFE
    };

    Constants::PowerUpType type = types[std::rand() % 5];
    float x = Constants::WINDOW_WIDTH + 50;
    float y = Constants::GROUND_Y - 80 - (std::rand() % 60);

    auto pu = createPowerUp(assets, type, x, y);
    state.entities.push_back(std::move(pu));
}
