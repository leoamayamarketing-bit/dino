#include "../../include/levels/DesertDayLevel.h"
#include "../../include/core/Constants.h"
#include "../../include/entities/ObstacleFactory.h"
#include "../../include/entities/EnemyFactory.h"
#include "../../include/components/TransformComponent.h"
#include "../../include/components/SpriteComponent.h"
#include "../../include/components/CollisionComponent.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <algorithm>
#include <cstdlib>

void DesertDayLevel::init(AssetManager& assets, GameState& state) {
    spawnInterval_ = 2.0f;
    spawnTimer_ = 0.0f;

    // Setup parallax layers
    if (assets.hasTexture("cloud")) {
        parallax_.addLayer(assets.getTexture("cloud"), 0.2f);
    }
    if (assets.hasTexture("ground")) {
        parallax_.addLayer(assets.getTexture("ground"), 1.0f);
    }
    parallax_.setScrollDirection(1.0f);

    // Initial obstacles
    spawnObstacles(state, assets);
}

void DesertDayLevel::update(float deltaTime, AssetManager& assets, GameState& state) {
    // Update parallax with game speed
    parallax_.setScrollDirection(state.currentSpeed);
    std::vector<Entity*> emptyVec;
    parallax_.update(deltaTime, emptyVec);

    // Heat distortion particles
    static float heatTimer = 0.0f;
    heatTimer += deltaTime;
    if (heatTimer > 0.1f) {
        heatTimer = 0.0f;
        particles_.emit(
            sf::Vector2f(static_cast<float>(std::rand() % Constants::WINDOW_WIDTH),
                         Constants::GROUND_Y - 50),
            1, sf::Color(255, 200, 100, 30), 20.0f, 0.5f, 2.0f);
    }

    // Spawn obstacles
    spawnTimer_ += deltaTime;
    if (spawnTimer_ >= spawnInterval_) {
        spawnTimer_ = 0.0f;
        spawnObstacles(state, assets);
        spawnInterval_ = std::max(1.0f, spawnInterval_ - 0.05f);
    }

    // Clean up offscreen entities
    cleanupOffscreen(state);
}

void DesertDayLevel::render(sf::RenderWindow& window) {
    // Draw background
    sf::RectangleShape sky(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::GROUND_Y + 120));
    sky.setFillColor(skyColor_);
    window.draw(sky);

    // Parallax layers
    parallax_.render(window);

    // Particles
    particles_.render(window);
}

void DesertDayLevel::spawnObstacles(GameState& state, AssetManager& assets) {
    float startX = Constants::WINDOW_WIDTH + 100;
    int roll = std::rand() % 100;

    if (roll < 40) {
        auto entity = EnemyFactory::createEnemy(Constants::EnemyType::SMALL_CACTUS, assets, startX);
        state.entities.push_back(std::move(entity));
    } else if (roll < 65) {
        auto entity = EnemyFactory::createEnemy(Constants::EnemyType::LARGE_CACTUS, assets, startX);
        state.entities.push_back(std::move(entity));
    } else if (roll < 80) {
        auto entity = EnemyFactory::createEnemy(Constants::EnemyType::PTERODACTYL, assets, startX + 50);
        state.entities.push_back(std::move(entity));
    } else {
        // Coin group
        for (int i = 0; i < 3; i++) {
            auto coin = ObstacleFactory::createCoin(assets, startX + i * 40,
                Constants::GROUND_Y - 80 - static_cast<float>(std::rand() % 60));
            state.entities.push_back(std::move(coin));
        }
    }

    // Random powerup
    if (std::rand() % 100 < 10) {
        auto pu = ObstacleFactory::createPowerUp(assets,
            static_cast<Constants::PowerUpType>(std::rand() % 5),
            startX + 200, Constants::GROUND_Y - 60);
        state.entities.push_back(std::move(pu));
    }
}

void Level::cleanupOffscreen(GameState& state) {
    state.entities.erase(
        std::remove_if(state.entities.begin(), state.entities.end(),
            [](const std::unique_ptr<Entity>& e) {
                auto* trans = e->getComponent<TransformComponent>();
                return trans && trans->position.x < -200.0f;
            }),
        state.entities.end());
}

void Level::spawnGround(Entity*, GameState&, AssetManager&) {
    // Ground handled by parallax system
}
