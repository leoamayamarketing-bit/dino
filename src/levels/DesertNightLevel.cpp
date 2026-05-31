#include "../../include/levels/DesertNightLevel.h"
#include "../../include/core/Constants.h"
#include "../../include/entities/ObstacleFactory.h"
#include "../../include/entities/EnemyFactory.h"
#include "../../include/components/TransformComponent.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>

void DesertNightLevel::init(AssetManager& assets, GameState& state) {
    spawnInterval_ = 2.5f;
    spawnTimer_ = 0.0f;
    minSpawnInterval_ = 1.5f;

    if (assets.hasTexture("cloud")) {
        parallax_.addLayer(assets.getTexture("cloud"), 0.2f, 30.0f);
    }
    if (assets.hasTexture("ground")) {
        parallax_.addLayer(assets.getTexture("ground"), 1.0f, Constants::GROUND_Y);
    }
    parallax_.setScrollDirection(1.0f);

    // Generate star positions, brightness levels, and twinkle phases
    starsGenerated_ = false;
    starPositions_.clear();
    starBrightness_.clear();
    starPhase_.clear();
    starTwinkleTimer_ = 0.0f;

    spawnObstacles(state, assets);
}

void DesertNightLevel::update(float deltaTime, AssetManager& assets, GameState& state) {
    parallax_.setScrollDirection(state.currentSpeed);
    std::vector<Entity*> emptyVec;
    parallax_.update(deltaTime, emptyVec);

    // Mars subtle pulsing
    marsPhase_ += deltaTime * 0.5f;

    // Star twinkle timer
    starTwinkleTimer_ += deltaTime;

    spawnTimer_ += deltaTime;
    if (spawnTimer_ >= spawnInterval_) {
        spawnTimer_ = 0.0f;
        spawnObstacles(state, assets);
        spawnInterval_ = std::max(minSpawnInterval_, spawnInterval_ - (state.hardMode ? 0.08f : 0.04f));
    }

    cleanupOffscreen(state);
}

void DesertNightLevel::render(sf::RenderWindow& window) {
    /// --- Sky background ---
    sf::RectangleShape sky(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
    sky.setFillColor(skyColor_);
    window.draw(sky);

    /// --- Stars (generated once, with varying brightness) ---
    if (!starsGenerated_) {
        starPositions_.clear();
        starBrightness_.clear();
        starPhase_.clear();
        for (int i = 0; i < 120; i++) {
            starPositions_.push_back(sf::Vector2f(
                static_cast<float>(std::rand() % Constants::WINDOW_WIDTH),
                static_cast<float>(std::rand() % static_cast<int>(Constants::GROUND_Y - 120))));
            // Varied brightness: some dim, some bright
            starBrightness_.push_back(100 + (std::rand() % 155));
            // Random phase offset for twinkle animation
            starPhase_.push_back((std::rand() % 1000) * 0.001f * 6.2832f);
        }
        starsGenerated_ = true;
    }

    for (size_t i = 0; i < starPositions_.size(); i++) {
        float baseBrightness = static_cast<float>(starBrightness_[i]);
        // Twinkle: brightness oscillates with a gentle sine wave
        float twinkle = 0.7f + 0.3f * std::sin(starTwinkleTimer_ * 0.8f + starPhase_[i]);
        float brightness = baseBrightness * twinkle;
        float radius = (baseBrightness > 200.0f) ? 2.0f : 1.2f;
        if (brightness > 180.0f) radius = std::max(radius, 2.0f); // brighter stars appear larger
        sf::CircleShape star(radius);
        int b = static_cast<int>(std::clamp(brightness, 0.0f, 255.0f));
        int alpha = static_cast<int>(std::clamp(brightness, 30.0f, 255.0f));
        star.setFillColor(sf::Color(b, b, b, alpha));
        star.setPosition(starPositions_[i]);
        window.draw(star);
    }

    /// --- Moon with eclipse (crescent/partially shadowed) ---
    // Outer glow
    sf::CircleShape moonGlow(moonRadius_ * 2.5f);
    moonGlow.setFillColor(sf::Color(200, 200, 220, 20));
    moonGlow.setPosition(moonPos_.x - moonRadius_ * 2.5f,
                         moonPos_.y - moonRadius_ * 2.5f);
    window.draw(moonGlow);

    // Medium glow
    sf::CircleShape moonGlow2(moonRadius_ * 1.6f);
    moonGlow2.setFillColor(sf::Color(200, 200, 220, 40));
    moonGlow2.setPosition(moonPos_.x - moonRadius_ * 1.6f,
                          moonPos_.y - moonRadius_ * 1.6f);
    window.draw(moonGlow2);

    // Moon base (pale cream)
    sf::CircleShape moon(moonRadius_);
    moon.setFillColor(sf::Color(230, 225, 210));
    moon.setPosition(moonPos_.x - moonRadius_, moonPos_.y - moonRadius_);
    window.draw(moon);

    // Moon surface details (craters)
    sf::CircleShape crater1(moonRadius_ * 0.15f);
    crater1.setFillColor(sf::Color(210, 205, 190));
    crater1.setPosition(moonPos_.x - moonRadius_ * 0.3f,
                        moonPos_.y - moonRadius_ * 0.2f);
    window.draw(crater1);

    sf::CircleShape crater2(moonRadius_ * 0.1f);
    crater2.setFillColor(sf::Color(215, 210, 195));
    crater2.setPosition(moonPos_.x + moonRadius_ * 0.15f,
                        moonPos_.y + moonRadius_ * 0.3f);
    window.draw(crater2);

    // Eclipse shadow: a circle the same color as the sky, offset to create
    // a crescent/eclipse effect
    sf::CircleShape eclipseShadow(moonRadius_);
    eclipseShadow.setFillColor(skyColor_);
    eclipseShadow.setPosition(
        moonPos_.x - moonRadius_ + eclipseOffsetX_,
        moonPos_.y - moonRadius_ + eclipseOffsetY_);
    window.draw(eclipseShadow);

    // Second smaller shadow for a more complex eclipse look
    sf::CircleShape eclipseShadow2(moonRadius_ * 0.7f);
    eclipseShadow2.setFillColor(skyColor_);
    eclipseShadow2.setPosition(
        moonPos_.x - moonRadius_ * 0.7f + eclipseOffsetX_ * 1.4f,
        moonPos_.y - moonRadius_ * 0.7f + eclipseOffsetY_ * 1.2f);
    window.draw(eclipseShadow2);

    /// --- Mars (reddish planet) ---
    // Mars glow
    sf::CircleShape marsGlow(marsRadius_ * 2.0f);
    marsGlow.setFillColor(sf::Color(220, 100, 60, 25));
    marsGlow.setPosition(marsPos_.x - marsRadius_ * 2.0f,
                         marsPos_.y - marsRadius_ * 2.0f);
    window.draw(marsGlow);

    // Mars body
    sf::CircleShape mars(marsRadius_);
    mars.setFillColor(sf::Color(210, 90, 55));
    mars.setPosition(marsPos_.x - marsRadius_, marsPos_.y - marsRadius_);
    window.draw(mars);

    // Mars surface features (darker red patches)
    float marsPulse = 1.0f + std::sin(marsPhase_) * 0.03f;
    sf::CircleShape marsSpot1(marsRadius_ * 0.3f * marsPulse);
    marsSpot1.setFillColor(sf::Color(170, 65, 40));
    marsSpot1.setPosition(marsPos_.x - marsRadius_ * 0.2f,
                          marsPos_.y - marsRadius_ * 0.15f);
    window.draw(marsSpot1);

    sf::CircleShape marsSpot2(marsRadius_ * 0.2f);
    marsSpot2.setFillColor(sf::Color(180, 75, 45));
    marsSpot2.setPosition(marsPos_.x + marsRadius_ * 0.15f,
                          marsPos_.y + marsRadius_ * 0.2f);
    window.draw(marsSpot2);

    // Mars polar ice cap (small white cap at top)
    sf::CircleShape marsIce(marsRadius_ * 0.2f);
    marsIce.setFillColor(sf::Color(230, 210, 200));
    marsIce.setPosition(marsPos_.x - marsRadius_ * 0.1f,
                        marsPos_.y - marsRadius_ * 0.7f);
    window.draw(marsIce);

    // Mars highlight
    sf::CircleShape marsHighlight(marsRadius_ * 0.15f);
    marsHighlight.setFillColor(sf::Color(240, 140, 100, 120));
    marsHighlight.setPosition(marsPos_.x - marsRadius_ * 0.5f,
                              marsPos_.y - marsRadius_ * 0.4f);
    window.draw(marsHighlight);

    /// --- Ground ---
    // Draw ground fill below GROUND_Y
    sf::RectangleShape groundFill(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT - Constants::GROUND_Y));
    groundFill.setFillColor(sf::Color(40, 30, 20));
    groundFill.setPosition(0, Constants::GROUND_Y);
    window.draw(groundFill);

    // Ground surface line with subtle dark highlight
    sf::RectangleShape groundLine(sf::Vector2f(Constants::WINDOW_WIDTH, 2));
    groundLine.setFillColor(sf::Color(25, 18, 12));
    groundLine.setPosition(0, Constants::GROUND_Y);
    window.draw(groundLine);

    // Subtle moon reflection on the ground (faint horizontal band)
    sf::RectangleShape moonReflection(sf::Vector2f(Constants::WINDOW_WIDTH, 1));
    moonReflection.setFillColor(sf::Color(60, 55, 50, 40));
    moonReflection.setPosition(0, Constants::GROUND_Y + 4);
    window.draw(moonReflection);

    // Render parallax (clouds + ground texture)
    parallax_.render(window);


}

void DesertNightLevel::spawnObstacles(GameState& state, AssetManager& assets) {
    float startX = Constants::WINDOW_WIDTH + 100.0f + static_cast<float>(std::rand() % 200);
    int roll = std::rand() % 100;

    if (roll < 35) {
        auto e = EnemyFactory::createEnemy(Constants::EnemyType::SMALL_CACTUS, assets, startX);
        state.entities.push_back(std::move(e));
    } else if (roll < 60) {
        auto e = EnemyFactory::createEnemy(Constants::EnemyType::LARGE_CACTUS, assets, startX);
        state.entities.push_back(std::move(e));
    } else if (roll < 75) {
        auto e = EnemyFactory::createEnemy(Constants::EnemyType::PTERODACTYL, assets, startX + 100);
        state.entities.push_back(std::move(e));
    } else if (roll < 88) {
        // Coin cluster
        for (int i = 0; i < 3; i++) {
            auto coin = ObstacleFactory::createCoin(assets, startX + i * 50,
                Constants::GROUND_Y - 120 - (std::rand() % 80));
            state.entities.push_back(std::move(coin));
        }
    } else {
        auto e = EnemyFactory::createEnemy(Constants::EnemyType::ROLLING_ROCK, assets, startX);
        state.entities.push_back(std::move(e));
    }

    // Powerup chance (less frequent)
    if (std::rand() % 100 < 8) {
        auto pu = ObstacleFactory::createPowerUp(assets,
            static_cast<Constants::PowerUpType>(std::rand() % 5),
            startX + 250, Constants::GROUND_Y - 100);
        state.entities.push_back(std::move(pu));
    }
}
