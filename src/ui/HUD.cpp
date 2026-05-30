#include "../../include/ui/HUD.h"
#include "../../include/core/Constants.h"
#include <sstream>
#include <iomanip>

HUD::HUD() = default;

void HUD::init(sf::Font& font) {
    scoreText_.setFont(font);
    scoreText_.setCharacterSize(24);
    scoreText_.setFillColor(sf::Color::White);
    scoreText_.setPosition(10, 10);

    distanceText_.setFont(font);
    distanceText_.setCharacterSize(18);
    distanceText_.setFillColor(sf::Color(200, 200, 200));
    distanceText_.setPosition(10, 40);

    speedText_.setFont(font);
    speedText_.setCharacterSize(16);
    speedText_.setFillColor(sf::Color(180, 180, 180));
    speedText_.setPosition(10, 65);

    livesText_.setFont(font);
    livesText_.setCharacterSize(20);
    livesText_.setFillColor(sf::Color::Red);
    livesText_.setPosition(10, 90);

    coinsText_.setFont(font);
    coinsText_.setCharacterSize(20);
    coinsText_.setFillColor(sf::Color::Yellow);
    coinsText_.setPosition(Constants::WINDOW_WIDTH - 150, 10);

    powerupText_.setFont(font);
    powerupText_.setCharacterSize(16);
    powerupText_.setFillColor(sf::Color::Green);
    powerupText_.setPosition(Constants::WINDOW_WIDTH / 2 - 100, 10);

    shieldIndicator_.setFont(font);
    shieldIndicator_.setCharacterSize(14);
    shieldIndicator_.setFillColor(sf::Color::Cyan);
    shieldIndicator_.setPosition(Constants::WINDOW_WIDTH / 2 - 100, 35);

    // Health bar background
    healthBarBg_.setSize(sf::Vector2f(150, 12));
    healthBarBg_.setFillColor(sf::Color(60, 60, 60));
    healthBarBg_.setOutlineColor(sf::Color(100, 100, 100));
    healthBarBg_.setOutlineThickness(1.0f);
    healthBarBg_.setPosition(Constants::WINDOW_WIDTH - 170, 40);

    healthBarFill_.setSize(sf::Vector2f(150, 12));
    healthBarFill_.setFillColor(sf::Color::Green);
    healthBarFill_.setPosition(Constants::WINDOW_WIDTH - 170, 40);

    // Power-up indicators
    shieldInd_.text.setFont(font);
    shieldInd_.text.setCharacterSize(14);
    shieldInd_.text.setFillColor(sf::Color::Cyan);
    shieldInd_.text.setString("SHIELD");

    magnetInd_.text.setFont(font);
    magnetInd_.text.setCharacterSize(14);
    magnetInd_.text.setFillColor(sf::Color::Magenta);
    magnetInd_.text.setString("MAGNET");

    doublePointsInd_.text.setFont(font);
    doublePointsInd_.text.setCharacterSize(14);
    doublePointsInd_.text.setFillColor(sf::Color::Yellow);
    doublePointsInd_.text.setString("2X PTS");

    speedInd_.text.setFont(font);
    speedInd_.text.setCharacterSize(14);
    speedInd_.text.setFillColor(sf::Color::Green);
    speedInd_.text.setString("SPEED");
}

void HUD::update(const GameState& state) {
    std::ostringstream ss;
    ss << "SCORE: " << static_cast<int>(state.score);
    scoreText_.setString(ss.str());

    ss.str("");
    ss << "DIST: " << std::fixed << std::setprecision(0) << state.distance << "m";
    distanceText_.setString(ss.str());

    ss.str("");
    ss << "SPEED: " << static_cast<int>(state.currentSpeed * 0.36f) << " km/h";
    speedText_.setString(ss.str());

    ss.str("");
    ss << "LIVES: " << state.lives;
    livesText_.setString(ss.str());

    ss.str("");
    ss << "COINS: " << state.coins;
    coinsText_.setString(ss.str());

    // Power-up active timers
    shieldInd_.active = state.shieldActive;
    shieldInd_.timer = state.shieldTimer;
    magnetInd_.active = state.magnetActive;
    magnetInd_.timer = state.magnetTimer;
    doublePointsInd_.active = state.doublePointsActive;
    doublePointsInd_.timer = state.doublePointsTimer;
    speedInd_.active = state.speedBoostActive;
    speedInd_.timer = state.speedBoostTimer;

    // Find player for health
}

void HUD::render(sf::RenderWindow& window) {
    window.draw(scoreText_);
    window.draw(distanceText_);
    window.draw(speedText_);
    window.draw(livesText_);
    window.draw(coinsText_);
    window.draw(healthBarBg_);
    window.draw(healthBarFill_);

    // Power-up indicators
    float y = 60;
    if (shieldInd_.active) {
        shieldInd_.text.setPosition(Constants::WINDOW_WIDTH / 2 - 100, y);
        window.draw(shieldInd_.text);
        y += 20;
    }
    if (magnetInd_.active) {
        magnetInd_.text.setPosition(Constants::WINDOW_WIDTH / 2 - 100, y);
        window.draw(magnetInd_.text);
        y += 20;
    }
    if (doublePointsInd_.active) {
        doublePointsInd_.text.setPosition(Constants::WINDOW_WIDTH / 2 - 100, y);
        window.draw(doublePointsInd_.text);
        y += 20;
    }
    if (speedInd_.active) {
        speedInd_.text.setPosition(Constants::WINDOW_WIDTH / 2 - 100, y);
        window.draw(speedInd_.text);
    }
}

void HUD::updateHealthBar(int health, int maxHealth) {
    float ratio = static_cast<float>(health) / static_cast<float>(maxHealth);
    healthBarFill_.setSize(sf::Vector2f(150.0f * ratio, 12.0f));

    if (ratio > 0.6f) {
        healthBarFill_.setFillColor(sf::Color::Green);
    } else if (ratio > 0.3f) {
        healthBarFill_.setFillColor(sf::Color(255, 165, 0)); // Orange
    } else {
        healthBarFill_.setFillColor(sf::Color::Red);
    }
}
