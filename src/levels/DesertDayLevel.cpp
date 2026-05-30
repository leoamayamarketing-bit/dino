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
    spawnInterval_ = 2.5f;
    spawnTimer_ = 0.0f;
    minSpawnInterval_ = 1.5f;

    // Setup parallax layers
    if (assets.hasTexture("cloud")) {
        parallax_.addLayer(assets.getTexture("cloud"), 0.2f, 30.0f);
    }
    if (assets.hasTexture("ground")) {
        parallax_.addLayer(assets.getTexture("ground"), 1.0f, Constants::GROUND_Y);
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
        spawnInterval_ = std::max(minSpawnInterval_, spawnInterval_ - 0.04f);
    }

    // Clean up offscreen entities
    cleanupOffscreen(state);
}

void DesertDayLevel::render(sf::RenderWindow& window) {
    // Draw background
    sf::RectangleShape sky(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
    sky.setFillColor(skyColor_);
    window.draw(sky);

    // Draw ground fill below GROUND_Y
    sf::RectangleShape groundFill(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT - Constants::GROUND_Y));
    groundFill.setFillColor(sf::Color(100, 80, 50));  // dirt color below ground
    groundFill.setPosition(0, Constants::GROUND_Y);
    window.draw(groundFill);

    // Parallax layers (ground texture at GROUND_Y, clouds above)
    parallax_.render(window);

    // Particles
    particles_.render(window);
}

void DesertDayLevel::spawnObstacles(GameState& state, AssetManager& assets) {
    float startX = Constants::WINDOW_WIDTH + 100.0f + static_cast<float>(std::rand() % 200);
    int roll = std::rand() % 100;

    if (roll < 40) {
        auto entity = EnemyFactory::createEnemy(Constants::EnemyType::SMALL_CACTUS, assets, startX);
        state.entities.push_back(std::move(entity));
    } else if (roll < 65) {
        auto entity = EnemyFactory::createEnemy(Constants::EnemyType::LARGE_CACTUS, assets, startX);
        state.entities.push_back(std::move(entity));
    } else if (roll < 80) {
        auto entity = EnemyFactory::createEnemy(Constants::EnemyType::PTERODACTYL, assets, startX + 100);
        state.entities.push_back(std::move(entity));
    } else {
        // Coin group
        for (int i = 0; i < 3; i++) {
            auto coin = ObstacleFactory::createCoin(assets, startX + i * 50,
                Constants::GROUND_Y - 100 - static_cast<float>(std::rand() % 80));
            state.entities.push_back(std::move(coin));
        }
    }

    // Random powerup (less frequent)
    if (std::rand() % 100 < 8) {
        auto pu = ObstacleFactory::createPowerUp(assets,
            static_cast<Constants::PowerUpType>(std::rand() % 5),
            startX + 250, Constants::GROUND_Y - 100);
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
