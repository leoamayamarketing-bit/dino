#include "../../include/ui/GameOverScreen.h"
#include "../../include/core/Constants.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <sstream>
#include <iomanip>

GameOverScreen::GameOverScreen() = default;

void GameOverScreen::init(sf::Font& font) {
    gameOverText_.setFont(font);
    gameOverText_.setCharacterSize(64);
    gameOverText_.setFillColor(sf::Color::Red);
    gameOverText_.setString("GAME OVER");
    gameOverText_.setPosition(
        Constants::WINDOW_WIDTH / 2 - gameOverText_.getGlobalBounds().width / 2,
        80);

    scoreText_.setFont(font);
    scoreText_.setCharacterSize(28);
    scoreText_.setFillColor(sf::Color::White);
    scoreText_.setPosition(Constants::WINDOW_WIDTH / 2 - 180, 170);

    highScoreText_.setFont(font);
    highScoreText_.setCharacterSize(22);
    highScoreText_.setFillColor(sf::Color::Yellow);
    highScoreText_.setPosition(Constants::WINDOW_WIDTH / 2 - 180, 205);

    newHighScoreText_.setFont(font);
    newHighScoreText_.setCharacterSize(20);
    newHighScoreText_.setFillColor(sf::Color::Green);
    newHighScoreText_.setString("NEW HIGH SCORE!");
    newHighScoreText_.setPosition(
        Constants::WINDOW_WIDTH / 2 - newHighScoreText_.getGlobalBounds().width / 2,
        240);

    // Detailed stats
    enemiesText_.setFont(font);
    enemiesText_.setCharacterSize(18);
    enemiesText_.setFillColor(sf::Color(200, 180, 160));
    enemiesText_.setPosition(Constants::WINDOW_WIDTH / 2 - 180, 290);

    comboText_.setFont(font);
    comboText_.setCharacterSize(18);
    comboText_.setFillColor(sf::Color(255, 200, 100));
    comboText_.setPosition(Constants::WINDOW_WIDTH / 2 - 180, 315);

    powerUpsText_.setFont(font);
    powerUpsText_.setCharacterSize(18);
    powerUpsText_.setFillColor(sf::Color(150, 220, 255));
    powerUpsText_.setPosition(Constants::WINDOW_WIDTH / 2 - 180, 340);

    timeText_.setFont(font);
    timeText_.setCharacterSize(18);
    timeText_.setFillColor(sf::Color(180, 180, 180));
    timeText_.setPosition(Constants::WINDOW_WIDTH / 2 - 180, 365);

    // Action texts
    restartText_.setFont(font);
    restartText_.setCharacterSize(22);
    restartText_.setFillColor(sf::Color::White);
    restartText_.setString("Press SPACE to Restart");
    restartText_.setPosition(
        Constants::WINDOW_WIDTH / 2 - restartText_.getGlobalBounds().width / 2,
        440);

    menuText_.setFont(font);
    menuText_.setCharacterSize(22);
    menuText_.setFillColor(sf::Color::White);
    menuText_.setString("Press ESC for Menu");
    menuText_.setPosition(
        Constants::WINDOW_WIDTH / 2 - menuText_.getGlobalBounds().width / 2,
        475);

    // Distance and coins (moved to right column)
    distanceText_.setFont(font);
    distanceText_.setCharacterSize(18);
    distanceText_.setFillColor(sf::Color(200, 200, 200));
    distanceText_.setPosition(Constants::WINDOW_WIDTH / 2 + 20, 290);

    coinsText_.setFont(font);
    coinsText_.setCharacterSize(18);
    coinsText_.setFillColor(sf::Color::Yellow);
    coinsText_.setPosition(Constants::WINDOW_WIDTH / 2 + 20, 315);

    // High scores title
    hsTitleText_.setFont(font);
    hsTitleText_.setCharacterSize(16);
    hsTitleText_.setFillColor(sf::Color(180, 160, 100));
    hsTitleText_.setString("TOP SCORES");
    hsTitleText_.setPosition(
        Constants::WINDOW_WIDTH / 2 - hsTitleText_.getGlobalBounds().width / 2,
        510);

    // High scores entries
    hsEntriesText_.setFont(font);
    hsEntriesText_.setCharacterSize(13);
    hsEntriesText_.setFillColor(sf::Color(150, 140, 120));
    hsEntriesText_.setPosition(Constants::WINDOW_WIDTH / 2 - 140, 530);
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
        80);

    score_ = state.score;
    highScore_ = state.highScore;
    coins_ = state.coins;
    distance_ = state.distance;
    enemies_ = state.enemiesDestroyed;
    maxCombo_ = state.maxCombo;
    powerUps_ = state.powerUpsCollected;
    gameTime_ = state.gameTime;
    isNewHighScore_ = (score_ >= highScore_ && score_ > 0);
    runAchievements_ = state.achievements;

    // Animate score counting up
    displayScore_ = std::min(score_, displayScore_ + score_ * deltaTime * 2.0f);

    // Update text
    std::ostringstream ss;
    ss << "Score: " << static_cast<int>(displayScore_);
    scoreText_.setString(ss.str());

    ss.str("");
    ss << "Best: " << static_cast<int>(highScore_);
    highScoreText_.setString(ss.str());

    // Stats (left column)
    ss.str("");
    ss << "Enemies destroyed: " << enemies_;
    enemiesText_.setString(ss.str());

    ss.str("");
    ss << "Max combo: " << maxCombo_ << "×";
    comboText_.setString(ss.str());

    ss.str("");
    ss << "Power-ups collected: " << powerUps_;
    powerUpsText_.setString(ss.str());

    ss.str("");
    int mins = static_cast<int>(gameTime_) / 60;
    int secs = static_cast<int>(gameTime_) % 60;
    ss << "Time: " << mins << ":" << std::setw(2) << std::setfill('0') << secs;
    timeText_.setString(ss.str());

    // Stats (right column)
    ss.str("");
    ss << "Distance: " << static_cast<int>(distance_) << "m";
    distanceText_.setString(ss.str());

    ss.str("");
    ss << "Coins collected: " << coins_;
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
        float pulse = 1.0f + std::sin(animationTimer_ * 4.0f) * 0.1f;
        newHighScoreText_.setScale(pulse, pulse);
        newHighScoreText_.setPosition(
            Constants::WINDOW_WIDTH / 2 - newHighScoreText_.getGlobalBounds().width / 2,
            240);
        window.draw(newHighScoreText_);
    }

    // Separator line
    sf::RectangleShape sep(sf::Vector2f(360.0f, 1.0f));
    sep.setFillColor(sf::Color(255, 255, 255, 40));
    sep.setPosition(Constants::WINDOW_WIDTH / 2 - 180, 275);
    window.draw(sep);

    // Stats
    window.draw(enemiesText_);
    window.draw(comboText_);
    window.draw(powerUpsText_);
    window.draw(timeText_);
    window.draw(distanceText_);
    window.draw(coinsText_);

    // High scores
    window.draw(hsTitleText_);
    window.draw(hsEntriesText_);

    // Separator before achievements
    sf::RectangleShape sep3(sf::Vector2f(280.0f, 1.0f));
    sep3.setFillColor(sf::Color(255, 215, 0, 30));
    sep3.setPosition(Constants::WINDOW_WIDTH / 2 - 140, 518);
    window.draw(sep3);

    // Achievement unlocks in this run
    if (runAchievements_ != 0) {
        sf::Text achText;
        achText.setFont(*enemiesText_.getFont());
        achText.setCharacterSize(14);
        achText.setFillColor(sf::Color(255, 215, 0, 180));

        int count = 0;
        uint32_t temp = runAchievements_;
        while (temp) { count += temp & 1; temp >>= 1; }

        std::ostringstream ss;
        ss << "Achievements unlocked: " << count;
        achText.setString(ss.str());
        achText.setPosition(
            Constants::WINDOW_WIDTH / 2 - achText.getGlobalBounds().width / 2,
            405);
        window.draw(achText);
    }

    // Separator before actions
    sf::RectangleShape sep2(sf::Vector2f(360.0f, 1.0f));
    sep2.setFillColor(sf::Color(255, 255, 255, 40));
    sep2.setPosition(Constants::WINDOW_WIDTH / 2 - 180, 425);
    window.draw(sep2);

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

void GameOverScreen::setHighScores(const std::vector<HighScoreEntry>& scores) {
    topScores_ = scores;

    // Update the display text
    std::ostringstream ss;
    int count = std::min(static_cast<int>(scores.size()), 5);
    for (int i = 0; i < count; i++) {
        if (i > 0) ss << "\n";
        ss << (i + 1) << ".  " << scores[i].score << " pts";
        if (scores[i].hardMode) ss << " ⚠";
        int mins = static_cast<int>(scores[i].gameTime) / 60;
        int secs = static_cast<int>(scores[i].gameTime) % 60;
        ss << "  (" << mins << ":" << std::setw(2) << std::setfill('0') << secs << ")";
    }
    hsEntriesText_.setString(ss.str());
    hsEntriesText_.setPosition(
        Constants::WINDOW_WIDTH / 2 - hsEntriesText_.getGlobalBounds().width / 2,
        530);
}

void GameOverScreen::setKeyPressed(bool restart, bool menu) {
    shouldRestart_ = restart;
    shouldMenu_ = menu;
}
