#include "../../include/levels/DesertNightLevel.h"
#include "../../include/core/Constants.h"
#include "../../include/entities/ObstacleFactory.h"
#include "../../include/entities/EnemyFactory.h"
#include "../../include/components/TransformComponent.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <algorithm>
#include <cstdlib>

void DesertNightLevel::init(AssetManager& assets, GameState& state) {
    spawnInterval_ = 1.8f;
    spawnTimer_ = 0.0f;

    if (assets.hasTexture("cloud")) {
        parallax_.addLayer(assets.getTexture("cloud"), 0.2f);
    }
    if (assets.hasTexture("ground")) {
        parallax_.addLayer(assets.getTexture("ground"), 1.0f);
    }
    parallax_.setScrollDirection(1.0f);
    spawnObstacles(state, assets);
}

void DesertNightLevel::update(float deltaTime, AssetManager& assets, GameState& state) {
    parallax_.setScrollDirection(state.currentSpeed);
    std::vector<Entity*> emptyVec;
    parallax_.update(deltaTime, emptyVec);

    // Fireflies
    fireflyTimer_ += deltaTime;
    if (fireflyTimer_ > 0.3f) {
        fireflyTimer_ = 0.0f;
        fireflies_.emit(
            sf::Vector2f(static_cast<float>(std::rand() % Constants::WINDOW_WIDTH),
                         Constants::GROUND_Y - 100),
            2, sf::Color(200, 255, 100, 200), 30.0f, 2.0f, 3.0f);
    }

    spawnTimer_ += deltaTime;
    if (spawnTimer_ >= spawnInterval_) {
        spawnTimer_ = 0.0f;
        spawnObstacles(state, assets);
        spawnInterval_ = std::max(1.0f, spawnInterval_ - 0.05f);
    }

    cleanupOffscreen(state);
}

void DesertNightLevel::render(sf::RenderWindow& window) {
    sf::RectangleShape sky(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::GROUND_Y + 120));
    sky.setFillColor(skyColor_);
    window.draw(sky);

    // Stars
    static bool starsGenerated = false;
    static std::vector<sf::Vector2f> stars;
    if (!starsGenerated) {
        for (int i = 0; i < 100; i++) {
            stars.push_back(sf::Vector2f(
                static_cast<float>(std::rand() % Constants::WINDOW_WIDTH),
                static_cast<float>(std::rand() % static_cast<int>(Constants::GROUND_Y - 100))));
        }
        starsGenerated = true;
    }

    sf::CircleShape star(1.5f);
    star.setFillColor(sf::Color::White);
    for (const auto& s : stars) {
        star.setPosition(s);
        window.draw(star);
    }

    parallax_.render(window);
    fireflies_.render(window);
}

void DesertNightLevel::spawnObstacles(GameState& state, AssetManager& assets) {
    float startX = Constants::WINDOW_WIDTH + 100;
    int roll = std::rand() % 100;

    if (roll < 35) {
        auto e = EnemyFactory::createEnemy(Constants::EnemyType::SMALL_CACTUS, assets, startX);
        state.entities.push_back(std::move(e));
    } else if (roll < 60) {
        auto e = EnemyFactory::createEnemy(Constants::EnemyType::LARGE_CACTUS, assets, startX);
        state.entities.push_back(std::move(e));
    } else if (roll < 75) {
        auto e = EnemyFactory::createEnemy(Constants::EnemyType::PTERODACTYL, assets, startX);
        state.entities.push_back(std::move(e));
    } else if (roll < 88) {
        auto e = EnemyFactory::createEnemy(Constants::EnemyType::ROLLING_ROCK, assets, startX);
        state.entities.push_back(std::move(e));
    } else {
        auto e = EnemyFactory::createEnemy(Constants::EnemyType::GROUND_ENEMY, assets, startX);
        state.entities.push_back(std::move(e));
    }
}
