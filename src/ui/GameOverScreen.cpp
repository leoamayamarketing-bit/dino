#include "../../include/ui/GameOverScreen.h"
#include "../../include/core/Constants.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <sstream>

GameOverScreen::GameOverScreen() = default;

void GameOverScreen::init(sf::Font& font) {
    gameOverText_.setFont(font);
    gameOverText_.setCharacterSize(64);
    gameOverText_.setFillColor(sf::Color::Red);
    gameOverText_.setString("GAME OVER");
    gameOverText_.setPosition(
        Constants::WINDOW_WIDTH / 2 - gameOverText_.getGlobalBounds().width / 2,
        100);

    scoreText_.setFont(font);
    scoreText_.setCharacterSize(32);
    scoreText_.setFillColor(sf::Color::White);
    scoreText_.setPosition(
        Constants::WINDOW_WIDTH / 2 - 150,
        200);

    highScoreText_.setFont(font);
    highScoreText_.setCharacterSize(28);
    highScoreText_.setFillColor(sf::Color::Yellow);
    highScoreText_.setPosition(
        Constants::WINDOW_WIDTH / 2 - 150,
        250);

    newHighScoreText_.setFont(font);
    newHighScoreText_.setCharacterSize(24);
    newHighScoreText_.setFillColor(sf::Color::Green);
    newHighScoreText_.setString("NEW HIGH SCORE!");
    newHighScoreText_.setPosition(
        Constants::WINDOW_WIDTH / 2 - newHighScoreText_.getGlobalBounds().width / 2,
        300);

    distanceText_.setFont(font);
    distanceText_.setCharacterSize(24);
    distanceText_.setFillColor(sf::Color(200, 200, 200));
    distanceText_.setPosition(
        Constants::WINDOW_WIDTH / 2 - 150,
        340);

    coinsText_.setFont(font);
    coinsText_.setCharacterSize(24);
    coinsText_.setFillColor(sf::Color::Yellow);
    coinsText_.setPosition(
        Constants::WINDOW_WIDTH / 2 - 150,
        370);

    restartText_.setFont(font);
    restartText_.setCharacterSize(24);
    restartText_.setFillColor(sf::Color::White);
    restartText_.setString("Press SPACE to Restart");
    restartText_.setPosition(
        Constants::WINDOW_WIDTH / 2 - restartText_.getGlobalBounds().width / 2,
        450);

    menuText_.setFont(font);
    menuText_.setCharacterSize(24);
    menuText_.setFillColor(sf::Color::White);
    menuText_.setString("Press ESC for Menu");
    menuText_.setPosition(
        Constants::WINDOW_WIDTH / 2 - menuText_.getGlobalBounds().width / 2,
        490);
}

void GameOverScreen::update(float deltaTime, const GameState& state) {
    animationTimer_ += deltaTime;
    textAlpha_ = std::min(1.0f, textAlpha_ + deltaTime);
    gameOverScale_ = std::min(1.0f, gameOverScale_ + deltaTime * 0.5f);

    // Animate title scale
    float scale = gameOverScale_;
    gameOverText_.setScale(scale, scale);
    gameOverText_.setPosition(
        Constants::WINDOW_WIDTH / 2 - gameOverText_.getGlobalBounds().width / 2,
        100);

    score_ = state.score;
    highScore_ = state.highScore;
    coins_ = state.coins;
    distance_ = state.distance;
    isNewHighScore_ = (score_ >= highScore_ && score_ > 0);

    // Animate score counting up
    displayScore_ = std::min(score_, displayScore_ + score_ * deltaTime * 2.0f);

    // Update text
    std::ostringstream ss;
    ss << "Score: " << static_cast<int>(displayScore_);
    scoreText_.setString(ss.str());

    ss.str("");
    ss << "Best: " << static_cast<int>(highScore_);
    highScoreText_.setString(ss.str());

    ss.str("");
    ss << "Distance: " << static_cast<int>(distance_) << "m";
    distanceText_.setString(ss.str());

    ss.str("");
    ss << "Coins: " << coins_;
    coinsText_.setString(ss.str());

    // Blink effect for options
    static float blinkTimer = 0.0f;
    blinkTimer += deltaTime;
    bool visible = static_cast<int>(blinkTimer * 2) % 2 == 0;

    restartText_.setFillColor(visible ? sf::Color::White : sf::Color(255, 255, 255, 50));
    menuText_.setFillColor(visible ? sf::Color::White : sf::Color(255, 255, 255, 50));
}

void GameOverScreen::render(sf::RenderWindow& window) {
    // Semi-transparent background
    sf::RectangleShape overlay(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(overlay);

    window.draw(gameOverText_);
    window.draw(scoreText_);
    window.draw(highScoreText_);

    if (isNewHighScore_) {
        // Pulsing new high score text
        float pulse = 1.0f + std::sin(animationTimer_ * 4.0f) * 0.1f;
        newHighScoreText_.setScale(pulse, pulse);
        newHighScoreText_.setPosition(
            Constants::WINDOW_WIDTH / 2 - newHighScoreText_.getGlobalBounds().width / 2,
            300);
        window.draw(newHighScoreText_);
    }

    window.draw(distanceText_);
    window.draw(coinsText_);
    window.draw(restartText_);
    window.draw(menuText_);
}

void GameOverScreen::reset() {
    shouldRestart_ = false;
    shouldMenu_ = false;
    animationTimer_ = 0.0f;
    textAlpha_ = 0.0f;
    gameOverScale_ = 0.5f;
    displayScore_ = 0.0f;
    isNewHighScore_ = false;
}

void GameOverScreen::handleInput(const GameState& state) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
        shouldRestart_ = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
        shouldMenu_ = true;
    }
}

void GameOverScreen::setScore(float score, float highScore) {
    score_ = score;
    highScore_ = highScore;
    displayScore_ = 0.0f;
    isNewHighScore_ = (score >= highScore && score > 0);
}

void GameOverScreen::setKeyPressed(bool restart, bool menu) {
    shouldRestart_ = restart;
    shouldMenu_ = menu;
}
