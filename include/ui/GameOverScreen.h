#ifndef UI_GAMEOVER_H
#define UI_GAMEOVER_H

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <vector>
#include "..//core/GameState.h"
#include "..//core/HighScoreManager.h"

class GameOverScreen {
public:
    GameOverScreen();

    void init(sf::Font& font);
    void update(float deltaTime, const GameState& state);
    void render(sf::RenderWindow& window);

    bool shouldRestart() const { return shouldRestart_; }
    bool shouldMenu() const { return shouldMenu_; }
    void reset();
    void handleInput(const GameState& state);
    void setScore(float score, float highScore);
    void setHighScores(const std::vector<HighScoreEntry>& scores);
    void setKeyPressed(bool restart, bool menu);

private:
    sf::Text gameOverText_;
    sf::Text scoreText_;
    sf::Text highScoreText_;
    sf::Text coinsText_;
    sf::Text distanceText_;
    sf::Text restartText_;
    sf::Text menuText_;
    sf::Text newHighScoreText_;

    // Detailed stats
    sf::Text enemiesText_;
    sf::Text comboText_;
    sf::Text powerUpsText_;
    sf::Text timeText_;

    bool shouldRestart_ = false;
    bool shouldMenu_ = false;
    bool isNewHighScore_ = false;
    float animationTimer_ = 0.0f;
    float textAlpha_ = 0.0f;
    float gameOverScale_ = 0.5f;

    float score_ = 0.0f;
    float displayScore_ = 0.0f;
    float highScore_ = 0.0f;
    int coins_ = 0;
    float distance_ = 0.0f;
    int enemies_ = 0;
    int maxCombo_ = 0;
    int powerUps_ = 0;
    float gameTime_ = 0.0f;

    // Unlocked achievements in this run
    uint32_t runAchievements_ = 0;

    // Top scores for display
    std::vector<HighScoreEntry> topScores_;
    sf::Text hsTitleText_;
    sf::Text hsEntriesText_;
};

#endif
