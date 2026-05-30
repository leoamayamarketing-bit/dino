#include "../../include/levels/InfiniteLevel.h"
#include "../../include/core/Constants.h"
#include "../../include/entities/ObstacleFactory.h"
#include "../../include/entities/EnemyFactory.h"
#include "../../include/components/TransformComponent.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <algorithm>
#include <cstdlib>

void InfiniteLevel::init(AssetManager& assets, GameState& state) {
    spawnInterval_ = 1.8f;
    spawnTimer_ = 0.0f;
    difficulty_ = 1.0f;
    minSpawnInterval_ = 1.0f;
    themeChangeTimer_ = 0.0f;
    currentTheme_ = Constants::LevelType::DESERT_DAY;
    bgColor_ = sf::Color(135, 206, 235);

    if (assets.hasTexture("ground")) {
        parallax_.addLayer(assets.getTexture("ground"), 1.0f, Constants::GROUND_Y);
    }
    if (assets.hasTexture("cloud")) {
        parallax_.addLayer(assets.getTexture("cloud"), 0.3f, 30.0f);
    }
    parallax_.setScrollDirection(1.0f);

    spawnObstacles(state, assets);
}

void InfiniteLevel::update(float deltaTime, AssetManager& assets, GameState& state) {
    parallax_.setScrollDirection(state.currentSpeed);
    std::vector<Entity*> emptyVec;
    parallax_.update(deltaTime, emptyVec);

    // Increase difficulty over time
    difficulty_ = 1.0f + state.gameTime / 60.0f;

    // Change theme every 30 seconds
    themeChangeTimer_ += deltaTime;
    if (themeChangeTimer_ >= 30.0f) {
        themeChangeTimer_ = 0.0f;
        changeTheme();
    }

    spawnTimer_ += deltaTime * difficulty_;
    if (spawnTimer_ >= spawnInterval_) {
        spawnTimer_ = 0.0f;
        spawnObstacles(state, assets);
        spawnInterval_ = std::max(minSpawnInterval_, 1.8f - difficulty_ * 0.04f);
    }

    cleanupOffscreen(state);
}

void InfiniteLevel::render(sf::RenderWindow& window) {
    sf::RectangleShape bg(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
    bg.setFillColor(bgColor_);
    window.draw(bg);

    // Draw ground fill below GROUND_Y
    sf::RectangleShape groundFill(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT - Constants::GROUND_Y));
    groundFill.setFillColor(sf::Color(100, 80, 50));
    groundFill.setPosition(0, Constants::GROUND_Y);
    window.draw(groundFill);

    parallax_.render(window);
}

void InfiniteLevel::spawnObstacles(GameState& state, AssetManager& assets) {
    float startX = Constants::WINDOW_WIDTH + 100;
    
    // Use pattern generation for more varied obstacle layouts
    auto pattern = ObstacleFactory::generateObstaclePattern(
        Constants::LevelType::INFINITE, assets, startX,
        3 + static_cast<int>(difficulty_));
    
    for (auto& entity : pattern) {
        state.entities.push_back(std::move(entity));
    }

    // Extra enemies at higher difficulty
    if (difficulty_ > 3.0f && std::rand() % 100 < 30) {
        auto e = EnemyFactory::createEnemy(
            Constants::EnemyType::PTERODACTYL, assets,
            startX + 300 + std::rand() % 200);
        state.entities.push_back(std::move(e));
    }

    // Powerups more frequent at higher difficulty
    if (std::rand() % 100 < 10 + static_cast<int>(difficulty_ * 2)) {
        auto pu = ObstacleFactory::createPowerUp(assets,
            static_cast<Constants::PowerUpType>(std::rand() % 5),
            startX + 200, Constants::GROUND_Y - 60);
        state.entities.push_back(std::move(pu));
    }
}

void InfiniteLevel::changeTheme() {
    // Cycle through themes randomly
    int theme = std::rand() % 4;
    switch (theme) {
        case 0:
            currentTheme_ = Constants::LevelType::DESERT_DAY;
            bgColor_ = sf::Color(135, 206, 235);
            break;
        case 1:
            currentTheme_ = Constants::LevelType::DESERT_NIGHT;
            bgColor_ = sf::Color(26, 26, 46);
            break;
        case 2:
            currentTheme_ = Constants::LevelType::CAVE;
            bgColor_ = sf::Color(13, 13, 13);
            break;
        case 3:
            currentTheme_ = Constants::LevelType::VOLCANO;
            bgColor_ = sf::Color(74, 0, 0);
            break;
    }
}
