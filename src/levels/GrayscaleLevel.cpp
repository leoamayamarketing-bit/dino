#include "../../include/levels/GrayscaleLevel.h"
#include "../../include/core/Constants.h"
#include "../../include/entities/ObstacleFactory.h"
#include "../../include/entities/EnemyFactory.h"
#include "../../include/components/TransformComponent.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>

void GrayscaleLevel::init(AssetManager& assets, GameState& state) {
    spawnInterval_ = 2.2f;
    spawnTimer_ = 0.0f;
    minSpawnInterval_ = 1.3f;

    // ONLY add clouds to parallax — ground is rendered manually in grayscale
    if (assets.hasTexture("cloud")) {
        parallax_.addLayer(assets.getTexture("cloud"), 0.2f, 30.0f);
    }
    // Apply grayscale tint to parallax layers
    parallax_.setGrayscaleTint(true);
    parallax_.setScrollDirection(1.0f);

    // Generate city skyline silhouette — widths, heights, and window data generated ONCE here
    citySilhouette_.clear();
    for (int i = 0; i < 12; i++) {
        GrayscaleBuilding b;
        b.x = i * 110.0f + (std::rand() % 40);
        b.width = 60.0f + (std::rand() % 20);
        b.height = 80.0f + (std::rand() % 150);
        // Pre-determine window lights (not randomized per frame in render())
        b.hasWindow = (b.x + b.width) > 100.0f && (std::rand() % 3 == 0);
        b.hasSecondWindow = b.hasWindow && (std::rand() % 2 == 0);
        citySilhouette_.push_back(b);
    }

    spawnObstacles(state, assets);
}

void GrayscaleLevel::update(float deltaTime, AssetManager& assets, GameState& state) {
    parallax_.setScrollDirection(state.currentSpeed);
    std::vector<Entity*> emptyVec;
    parallax_.update(deltaTime, emptyVec);

    // Animate ground detail offset for subtle scrolling texture
    groundScrollOffset_ = std::fmod(groundScrollOffset_ + state.currentSpeed * deltaTime * 0.5f, 12.0f);

    // Fog/smog particles drifting across
    fogTimer_ += deltaTime;
    if (fogTimer_ > 0.2f) {
        fogTimer_ = 0.0f;
        smokeParticles_.emit(
            sf::Vector2f(std::rand() % Constants::WINDOW_WIDTH,
                         Constants::GROUND_Y - 80 - (std::rand() % 100)),
            1, sf::Color(180, 180, 180, 60), 20.0f, 3.0f, 8.0f);
    }

    spawnTimer_ += deltaTime;
    if (spawnTimer_ >= spawnInterval_) {
        spawnTimer_ = 0.0f;
        spawnObstacles(state, assets);
        spawnInterval_ = std::max(minSpawnInterval_, spawnInterval_ - (state.hardMode ? 0.08f : 0.04f));
    }

    cleanupOffscreen(state);
    smokeParticles_.updateParticles(deltaTime);
}

void GrayscaleLevel::render(sf::RenderWindow& window) {
    // Light gray gradient sky
    sf::RectangleShape sky(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
    sky.setFillColor(skyColor_);
    window.draw(sky);

    // Darker horizontal band for atmospheric depth
    sf::RectangleShape horizonBand(sf::Vector2f(Constants::WINDOW_WIDTH, 80));
    horizonBand.setFillColor(sf::Color(160, 160, 160));
    horizonBand.setPosition(0, Constants::GROUND_Y - 80);
    window.draw(horizonBand);

    // City silhouette buildings on the horizon (static geometry from init())
    sf::RectangleShape building(sf::Vector2f(60, 0));
    building.setFillColor(sf::Color(80, 80, 80));
    for (const auto& b : citySilhouette_) {
        building.setSize(sf::Vector2f(b.width, b.height));
        building.setPosition(b.x, Constants::GROUND_Y - b.height);
        window.draw(building);

        // Some buildings have a slightly lighter top
        sf::RectangleShape roof(sf::Vector2f(b.width, 8));
        roof.setFillColor(sf::Color(100, 100, 100));
        roof.setPosition(b.x, Constants::GROUND_Y - b.height - 4);
        window.draw(roof);

        // Tiny window lights on some buildings (pre-determined in init())
        if (b.hasWindow) {
            sf::RectangleShape win(sf::Vector2f(4, 5));
            win.setFillColor(sf::Color(140, 140, 140));
            win.setPosition(b.x + b.width * 0.3f, Constants::GROUND_Y - b.height + 20);
            window.draw(win);
        }
        // Extra second window on some buildings
        if (b.hasSecondWindow) {
            sf::RectangleShape win2(sf::Vector2f(4, 5));
            win2.setFillColor(sf::Color(130, 130, 130));
            win2.setPosition(b.x + b.width * 0.6f, Constants::GROUND_Y - b.height + 40);
            window.draw(win2);
        }
    }

    // Ground fill below GROUND_Y — using a neutral dark gray
    sf::RectangleShape groundFill(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT - Constants::GROUND_Y));
    groundFill.setFillColor(sf::Color(55, 55, 55));
    groundFill.setPosition(0, Constants::GROUND_Y);
    window.draw(groundFill);

    // Ground surface line (top edge of ground)
    sf::RectangleShape groundLine(sf::Vector2f(Constants::WINDOW_WIDTH, 3));
    groundLine.setFillColor(sf::Color(35, 35, 35));
    groundLine.setPosition(0, Constants::GROUND_Y);
    window.draw(groundLine);

    // Subtle ground-surface highlight
    sf::RectangleShape groundHighlight(sf::Vector2f(Constants::WINDOW_WIDTH, 1));
    groundHighlight.setFillColor(sf::Color(80, 80, 80));
    groundHighlight.setPosition(0, Constants::GROUND_Y + 3);
    window.draw(groundHighlight);

    // Gravel / texture dots on the ground (scrolling subtly)
    sf::CircleShape gravel(1.0f);
    for (int i = 0; i < 30; i++) {
        float gx = (i * 43.0f + groundScrollOffset_) - 10.0f;
        while (gx < 0.0f) gx += Constants::WINDOW_WIDTH;
        while (gx >= Constants::WINDOW_WIDTH) gx -= Constants::WINDOW_WIDTH;
        float gy = Constants::GROUND_Y + 8.0f + static_cast<float>((i * 17) % 60);
        int grayVal = 70 + (i * 7) % 25;
        gravel.setFillColor(sf::Color(grayVal, grayVal, grayVal));
        gravel.setPosition(gx, gy);
        window.draw(gravel);
    }

    // Render clouds from parallax (grayscale tinted)
    parallax_.render(window);
    smokeParticles_.render(window);
}

void GrayscaleLevel::spawnObstacles(GameState& state, AssetManager& assets) {
    float startX = Constants::WINDOW_WIDTH + 100.0f + static_cast<float>(std::rand() % 200);
    int roll = std::rand() % 100;

    if (roll < 30) {
        auto e = EnemyFactory::createEnemy(Constants::EnemyType::SMALL_CACTUS, assets, startX);
        state.entities.push_back(std::move(e));
    } else if (roll < 55) {
        auto e = EnemyFactory::createEnemy(Constants::EnemyType::LARGE_CACTUS, assets, startX);
        state.entities.push_back(std::move(e));
    } else if (roll < 70) {
        auto e = EnemyFactory::createEnemy(Constants::EnemyType::PTERODACTYL, assets, startX + 100);
        state.entities.push_back(std::move(e));
    } else if (roll < 85) {
        // Rock cluster
        auto rock = ObstacleFactory::createRock(assets, startX + 50);
        state.entities.push_back(std::move(rock));
        if (std::rand() % 2) {
            auto rock2 = ObstacleFactory::createRock(assets, startX + 120);
            state.entities.push_back(std::move(rock2));
        }
    } else {
        // Coin row
        for (int i = 0; i < 3; i++) {
            auto coin = ObstacleFactory::createCoin(assets, startX + i * 50,
                Constants::GROUND_Y - 120 - (std::rand() % 80));
            state.entities.push_back(std::move(coin));
        }
    }

    // Powerup chance
    if (std::rand() % 100 < 8) {
        auto pu = ObstacleFactory::createPowerUp(assets,
            static_cast<Constants::PowerUpType>(std::rand() % 5),
            startX + 250, Constants::GROUND_Y - 100);
        state.entities.push_back(std::move(pu));
    }
}
