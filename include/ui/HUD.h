#ifndef UI_HUD_H
#define UI_HUD_H

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <functional>
#include <vector>
#include "../core/GameState.h"

class HUD {
public:
    HUD();

    void init(sf::Font& font);
    void update(const GameState& state, float deltaTime);
    void render(sf::RenderWindow& window);
    int getComboCount() const { return lastComboCount_; }

private:
    // ── Top Status Bar ─────────────────────────────────────────────────
    sf::RectangleShape topBarBg_;
    static constexpr float TOP_BAR_HEIGHT = 58.0f;

    // Score section (left)
    sf::Text scoreLabelText_;
    sf::Text scoreValueText_;
    sf::Text highScoreLabelText_;
    sf::Text highScoreValueText_;

    // Distance section (center)
    sf::Text distanceText_;

    // Coins section (right)
    sf::Text coinsLabelText_;
    sf::Text coinsValueText_;

    // Health section (right, below coins)
    sf::Text healthLabelText_;
    struct HeartShape {
        sf::CircleShape outer;
        sf::CircleShape inner;
        bool filled = false;
    };
    std::vector<HeartShape> hearts_;
    static constexpr int MAX_VISIBLE_HEARTS = 5;

    // Speed indicator (subtle, bottom of bar)
    sf::Text speedText_;
    sf::RectangleShape speedBarBg_;
    sf::RectangleShape speedBarFill_;

    // ── Floating Score Popups ──────────────────────────────────────────
    struct ScorePopup {
        sf::Text text;
        float lifetime = 0.0f;
        float maxLifetime = 0.8f;
        float startY = 0.0f;
        float x = 0.0f;
    };
    std::vector<ScorePopup> popups_;

    // ── Combo Indicator ────────────────────────────────────────────────
    sf::Text comboText_;
    float comboVisibleTimer_ = 0.0f;
    int lastComboCount_ = 0;

    // ── Hard Mode Indicator ────────────────────────────────────────────
    sf::Text hardModeText_;

    // ── Power-Up Indicators (below top bar) ────────────────────────────
    struct PowerUpBar {
        sf::Text label;
        sf::RectangleShape bg;
        sf::RectangleShape fill;
        sf::Color color;
        float maxWidth = 0.0f;
        bool active = false;
        float remaining = 0.0f;
        float total = 0.0f;
    };
    PowerUpBar shieldBar_;
    PowerUpBar magnetBar_;
    PowerUpBar doublePointsBar_;
    PowerUpBar speedBoostBar_;

    // ── Tracking for auto-detect popups & combo ─────────────────────────
    const GameState* state_ = nullptr;
    float prevScore_ = 0.0f;
    int prevCoins_ = 0;
    float lastCoinTime_ = 0.0f;
    float comboTimeWindow_ = 1.5f;

    // ── Combo milestone callback ────────────────────────────────────────
    int lastMilestoneTriggered_ = 0;

public:
    // Callback for combo milestones (set by Game)
    std::function<void(int comboCount, sf::Vector2f screenPos)> onComboMilestone;

private:
    // ── Internal helpers ───────────────────────────────────────────────
    void updateScoreSection(const GameState& state);
    void updateDistanceSection(const GameState& state);
    void updateCoinsAndHealth(const GameState& state);
    void updateSpeedIndicator(const GameState& state);
    void updatePowerUpBars(const GameState& state);
    void updateComboAndPopups(float deltaTime);

    void addScorePopup(int value);

    void renderTopBar(sf::RenderWindow& window);
    void renderPopups(sf::RenderWindow& window);
    void renderCombo(sf::RenderWindow& window);
    void renderPowerUpBars(sf::RenderWindow& window);
};

#endif
