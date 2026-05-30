#include "../../include/levels/VolcanoLevel.h"
#include "../../include/core/Constants.h"
#include "../../include/entities/ObstacleFactory.h"
#include "../../include/entities/EnemyFactory.h"
#include "../../include/components/TransformComponent.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <algorithm>
#include <cstdlib>

void VolcanoLevel::init(AssetManager& assets, GameState& state) {
    spawnInterval_ = 2.0f;
    spawnTimer_ = 0.0f;
    minSpawnInterval_ = 1.3f;

    if (assets.hasTexture("ground")) {
        parallax_.addLayer(assets.getTexture("ground"), 1.0f, Constants::GROUND_Y);
    }
    parallax_.setScrollDirection(1.0f);

    // Lava pools scattered on ground
    lavaPools_.clear();
    for (int i = 0; i < 5; i++) {
        lavaPools_.push_back(sf::FloatRect(
            std::rand() % Constants::WINDOW_WIDTH,
            Constants::GROUND_Y + 5,
            60 + std::rand() % 80,
            8 + std::rand() % 6));
    }

    spawnObstacles(state, assets);
}

void VolcanoLevel::update(float deltaTime, AssetManager& assets, GameState& state) {
    parallax_.setScrollDirection(state.currentSpeed);
    std::vector<Entity*> emptyVec;
    parallax_.update(deltaTime, emptyVec);

    // Ash particles from sky
    static float ashTimer = 0.0f;
    ashTimer += deltaTime;
    if (ashTimer > 0.15f) {
        ashTimer = 0.0f;
        ashParticles_.emit(
            sf::Vector2f(std::rand() % Constants::WINDOW_WIDTH, -10),
            2, sf::Color(80, 70, 60, 180), 40.0f, 3.0f, 3.0f);
    }

    // Lava bubble particles
    lavaTimer_ += deltaTime;
    if (lavaTimer_ > 0.4f) {
        lavaTimer_ = 0.0f;
        lavaParticles_.emit(
            sf::Vector2f(std::rand() % Constants::WINDOW_WIDTH, Constants::GROUND_Y),
            1, lavaColor_, 60.0f, 0.8f, 5.0f);
    }

    spawnTimer_ += deltaTime;
    if (spawnTimer_ >= spawnInterval_) {
        spawnTimer_ = 0.0f;
        spawnObstacles(state, assets);
        spawnInterval_ = std::max(minSpawnInterval_, spawnInterval_ - 0.04f);
    }

    cleanupOffscreen(state);
    ashParticles_.updateParticles(deltaTime);
    lavaParticles_.updateParticles(deltaTime);
}

void VolcanoLevel::render(sf::RenderWindow& window) {
    // Dark red sky
    sf::RectangleShape sky(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
    sky.setFillColor(skyColor_);
    window.draw(sky);

    // Lava glow effect
    sf::CircleShape glow(300.0f);
    glow.setFillColor(sf::Color(255, 69, 0, 40));
    glow.setPosition(-50, Constants::GROUND_Y - 250);
    window.draw(glow);

    // Ground fill below GROUND_Y
    sf::RectangleShape groundFill(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT - Constants::GROUND_Y));
    groundFill.setFillColor(sf::Color(30, 10, 5));
    groundFill.setPosition(0, Constants::GROUND_Y);
    window.draw(groundFill);

    // Lava pools
    sf::RectangleShape lavaRect;
    for (const auto& pool : lavaPools_) {
        lavaRect.setSize(sf::Vector2f(pool.width, pool.height));
        lavaRect.setFillColor(lavaColor_);
        lavaRect.setPosition(pool.left, pool.top);
        window.draw(lavaRect);
    }

    parallax_.render(window);
    ashParticles_.render(window);
    lavaParticles_.render(window);
}

void VolcanoLevel::spawnObstacles(GameState& state, AssetManager& assets) {
    float startX = Constants::WINDOW_WIDTH + 100.0f + static_cast<float>(std::rand() % 200);
    int roll = std::rand() % 100;

    if (roll < 25) {
        auto e = EnemyFactory::createEnemy(Constants::EnemyType::SMALL_CACTUS, assets, startX);
        state.entities.push_back(std::move(e));
    } else if (roll < 45) {
        auto e = EnemyFactory::createEnemy(Constants::EnemyType::LARGE_CACTUS, assets, startX);
        state.entities.push_back(std::move(e));
    } else if (roll < 60) {
        auto e = EnemyFactory::createEnemy(Constants::EnemyType::GROUND_ENEMY, assets, startX);
        state.entities.push_back(std::move(e));
    } else if (roll < 75) {
        auto rock = ObstacleFactory::createRock(assets, startX + 50);
        state.entities.push_back(std::move(rock));
    } else {
        for (int i = 0; i < 3; i++) {
            auto coin = ObstacleFactory::createCoin(assets, startX + i * 50,
                Constants::GROUND_Y - 120 - (std::rand() % 80));
            state.entities.push_back(std::move(coin));
        }
    }

    if (std::rand() % 100 < 10) {
        auto pu = ObstacleFactory::createPowerUp(assets,
            static_cast<Constants::PowerUpType>(std::rand() % 5),
            startX + 250, Constants::GROUND_Y - 100);
        state.entities.push_back(std::move(pu));
    }
}
