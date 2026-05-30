#include "../../include/levels/CaveLevel.h"
#include "../../include/core/Constants.h"
#include "../../include/entities/ObstacleFactory.h"
#include "../../include/entities/EnemyFactory.h"
#include "../../include/components/TransformComponent.h"
#include "../../include/components/SpriteComponent.h"
#include "../../include/components/CollisionComponent.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <algorithm>
#include <cstdlib>

void CaveLevel::init(AssetManager& assets, GameState& state) {
    spawnInterval_ = 1.6f;
    spawnTimer_ = 0.0f;

    if (assets.hasTexture("ground")) {
        parallax_.addLayer(assets.getTexture("ground"), 1.0f);
    }
    parallax_.setScrollDirection(1.0f);

    stalactites_.clear();
    for (int i = 0; i < 8; i++) {
        stalactites_.push_back(sf::Vector2f(
            std::rand() % Constants::WINDOW_WIDTH,
            0.0f));
    }

    spawnObstacles(state, assets);
}

void CaveLevel::update(float deltaTime, AssetManager& assets, GameState& state) {
    parallax_.setScrollDirection(state.currentSpeed);
    std::vector<Entity*> emptyVec;
    parallax_.update(deltaTime, emptyVec);

    // Cave ambiance: water drips as particles
    static float dripTimer = 0.0f;
    dripTimer += deltaTime;
    if (dripTimer > 0.5f) {
        dripTimer = 0.0f;
        dripParticles_.emit(
            sf::Vector2f(std::rand() % Constants::WINDOW_WIDTH, 0),
            1, sf::Color(100, 150, 255, 150), 50.0f, 1.5f, 3.0f);
    }

    // Stalactite falling hazard
    stalactiteTimer_ += deltaTime;
    if (stalactiteTimer_ > 3.0f) {
        stalactiteTimer_ = 0.0f;
        auto stalactite = ObstacleFactory::createStalactite(assets,
            Constants::WINDOW_WIDTH + std::rand() % 200);
        state.entities.push_back(std::move(stalactite));
    }

    spawnTimer_ += deltaTime;
    if (spawnTimer_ >= spawnInterval_) {
        spawnTimer_ = 0.0f;
        spawnObstacles(state, assets);
        spawnInterval_ = std::max(1.2f, spawnInterval_ - 0.05f);
    }

    cleanupOffscreen(state);
    dripParticles_.updateParticles(deltaTime);
}

void CaveLevel::render(sf::RenderWindow& window) {
    // Dark cave background with gradient effect
    sf::RectangleShape bg(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::GROUND_Y + 120));
    bg.setFillColor(caveColor_);
    window.draw(bg);

    // Dim ambient light circle around player
    sf::CircleShape ambientLight(180.0f);
    ambientLight.setFillColor(sf::Color(30, 20, 10, 80));
    ambientLight.setPosition(200, Constants::GROUND_Y - 200);
    window.draw(ambientLight);

    // Stalactite decorations on ceiling
    sf::RectangleShape stalactite(sf::Vector2f(8, 40 + std::rand() % 30));
    stalactite.setFillColor(sf::Color(60, 50, 45));
    for (const auto& s : stalactites_) {
        stalactite.setPosition(s);
        window.draw(stalactite);
    }

    // Ground
    sf::RectangleShape ground(sf::Vector2f(Constants::WINDOW_WIDTH, 20));
    ground.setFillColor(groundColor_);
    ground.setPosition(0, Constants::GROUND_Y);
    window.draw(ground);

    parallax_.render(window);
    dripParticles_.render(window);
}

void CaveLevel::spawnObstacles(GameState& state, AssetManager& assets) {
    float startX = Constants::WINDOW_WIDTH + 100;
    int roll = std::rand() % 100;

    if (roll < 30) {
        auto e = EnemyFactory::createEnemy(Constants::EnemyType::SMALL_CACTUS, assets, startX);
        state.entities.push_back(std::move(e));
    } else if (roll < 50) {
        auto e = EnemyFactory::createEnemy(Constants::EnemyType::GROUND_ENEMY, assets, startX);
        state.entities.push_back(std::move(e));
    } else if (roll < 65) {
        auto e = EnemyFactory::createEnemy(Constants::EnemyType::PTERODACTYL, assets, startX);
        state.entities.push_back(std::move(e));
    } else if (roll < 80) {
        auto rock = ObstacleFactory::createRock(assets, startX);
        state.entities.push_back(std::move(rock));
    } else {
        for (int i = 0; i < 3; i++) {
            auto coin = ObstacleFactory::createCoin(assets, startX + i * 40,
                Constants::GROUND_Y - 80 - (std::rand() % 60));
            state.entities.push_back(std::move(coin));
        }
    }

    if (std::rand() % 100 < 8) {
        auto pu = ObstacleFactory::createPowerUp(assets,
            static_cast<Constants::PowerUpType>(std::rand() % 5),
            startX + 200, Constants::GROUND_Y - 60);
        state.entities.push_back(std::move(pu));
    }
}
