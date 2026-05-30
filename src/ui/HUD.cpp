#include "../../include/ui/HUD.h"
#include "../../include/core/Constants.h"
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <algorithm>

HUD::HUD() {
    hearts_.resize(MAX_VISIBLE_HEARTS);
}

void HUD::init(sf::Font& font) {
    const float w = static_cast<float>(Constants::WINDOW_WIDTH);

    // ── Top Bar Background ──────────────────────────────────────────
    topBarBg_.setSize(sf::Vector2f(w, TOP_BAR_HEIGHT));
    topBarBg_.setFillColor(sf::Color(0, 0, 0, 160));
    topBarBg_.setPosition(0.0f, 0.0f);

    // ── Score Section (left) ────────────────────────────────────────
    scoreLabelText_.setFont(font);
    scoreLabelText_.setCharacterSize(11);
    scoreLabelText_.setFillColor(sf::Color(180, 180, 180));
    scoreLabelText_.setString("SCORE");
    scoreLabelText_.setPosition(20.0f, 6.0f);

    scoreValueText_.setFont(font);
    scoreValueText_.setCharacterSize(30);
    scoreValueText_.setFillColor(sf::Color::White);
    scoreValueText_.setString("00000");
    scoreValueText_.setPosition(20.0f, 20.0f);

    highScoreLabelText_.setFont(font);
    highScoreLabelText_.setCharacterSize(10);
    highScoreLabelText_.setFillColor(sf::Color(160, 140, 80));
    highScoreLabelText_.setString("BEST");
    // Position updated every frame in updateScoreSection

    highScoreValueText_.setFont(font);
    highScoreValueText_.setCharacterSize(10);
    highScoreValueText_.setFillColor(sf::Color(200, 180, 100));
    highScoreValueText_.setString("---");

    // ── Distance Section (center) ───────────────────────────────────
    distanceText_.setFont(font);
    distanceText_.setCharacterSize(26);
    distanceText_.setFillColor(sf::Color(200, 220, 255));
    distanceText_.setString("0m");
    // Centered dynamically

    // ── Coins Section (right) ───────────────────────────────────────
    coinsLabelText_.setFont(font);
    coinsLabelText_.setCharacterSize(11);
    coinsLabelText_.setFillColor(sf::Color(220, 190, 60));
    coinsLabelText_.setString("COINS");
    // Positioned dynamically

    coinsValueText_.setFont(font);
    coinsValueText_.setCharacterSize(26);
    coinsValueText_.setFillColor(sf::Color(255, 215, 0));
    coinsValueText_.setString("0");
    // Positioned dynamically

    // ── Health Section ──────────────────────────────────────────────
    healthLabelText_.setFont(font);
    healthLabelText_.setCharacterSize(11);
    healthLabelText_.setFillColor(sf::Color(220, 80, 80));
    healthLabelText_.setString("HP");
    // Positioned dynamically

    // Heart shapes
    for (int i = 0; i < MAX_VISIBLE_HEARTS; ++i) {
        auto& h = hearts_[i];
        h.outer.setRadius(7.0f);
        h.outer.setOutlineThickness(1.5f);
        h.outer.setOutlineColor(sf::Color(200, 50, 50));

        h.inner.setRadius(4.5f);
    }
    // Positions set in updateCoinsAndHealth

    // ── Speed Indicator (bottom of bar) ─────────────────────────────
    speedText_.setFont(font);
    speedText_.setCharacterSize(10);
    speedText_.setFillColor(sf::Color(150, 150, 150));
    // Positioned dynamically

    speedBarBg_.setSize(sf::Vector2f(80.0f, 4.0f));
    speedBarBg_.setFillColor(sf::Color(40, 40, 40));
    speedBarBg_.setOutlineColor(sf::Color(70, 70, 70));
    speedBarBg_.setOutlineThickness(0.5f);

    speedBarFill_.setSize(sf::Vector2f(0.0f, 4.0f));
    speedBarFill_.setFillColor(sf::Color(100, 200, 255));

    // ── Combo Text ──────────────────────────────────────────────────
    comboText_.setFont(font);
    comboText_.setCharacterSize(22);
    comboText_.setStyle(sf::Text::Bold);
    comboText_.setFillColor(sf::Color(255, 200, 50));
    comboText_.setOutlineColor(sf::Color(200, 100, 0));
    comboText_.setOutlineThickness(1.5f);

    // ── Power-Up Bars ───────────────────────────────────────────────
    auto initPowerUpBar = [&](PowerUpBar& bar, const sf::Color& color, const std::string& label) {
        bar.label.setFont(font);
        bar.label.setCharacterSize(11);
        bar.label.setFillColor(color);
        bar.label.setString(label);
        bar.color = color;

        bar.bg.setSize(sf::Vector2f(120.0f, 8.0f));
        bar.bg.setFillColor(sf::Color(30, 30, 30, 180));
        bar.bg.setOutlineColor(sf::Color(60, 60, 60));
        bar.bg.setOutlineThickness(0.5f);
        bar.maxWidth = 120.0f;

        bar.fill.setSize(sf::Vector2f(0.0f, 8.0f));
        bar.fill.setFillColor(color);
    };

    initPowerUpBar(shieldBar_,       sf::Color(0, 200, 255), "SHIELD");
    initPowerUpBar(magnetBar_,       sf::Color(200, 50, 255), "MAGNET");
    initPowerUpBar(doublePointsBar_, sf::Color(255, 200, 0),  "2×PTS");
    initPowerUpBar(speedBoostBar_,   sf::Color(50, 220, 80),  "BOOST");
}

void HUD::update(const GameState& state, float deltaTime) {
    // Auto-detect score/coin changes BEFORE updating visual sections
    if (state.coins > prevCoins_) {
        int diff = state.coins - prevCoins_;
        for (int i = 0; i < diff; ++i) {
            addScorePopup(100); // COIN_SCORE = 100
        }
        
        // Combo tracking: if within time window, increment combo
        if (lastCoinTime_ > 0.0f && lastCoinTime_ < comboTimeWindow_) {
            lastComboCount_++;
            // Check for combo milestones (3, 5, 10, 15, 20...)
            int milestones[] = {3, 5, 10, 15, 20, 30, 50};
            for (int m : milestones) {
                if (lastComboCount_ == m && m != lastMilestoneTriggered_) {
                    lastMilestoneTriggered_ = m;
                    if (onComboMilestone) {
                        sf::Vector2f screenPos(
                            Constants::WINDOW_WIDTH / 2.0f,
                            Constants::GROUND_Y - 200.0f
                        );
                        onComboMilestone(m, screenPos);
                    }
                    break;
                }
            }
        } else {
            lastComboCount_ = 1;
            lastMilestoneTriggered_ = 0;
        }
        comboVisibleTimer_ = 1.5f;
        lastCoinTime_ = 0.0f;
    } else {
        lastCoinTime_ += deltaTime;
        if (lastCoinTime_ > comboTimeWindow_) {
            lastComboCount_ = 0;
            lastMilestoneTriggered_ = 0;
        }
    }
    
    prevCoins_ = state.coins;
    prevScore_ = state.score;

    // Update all visual sections
    updateScoreSection(state);
    updateDistanceSection(state);
    updateCoinsAndHealth(state);
    updateSpeedIndicator(state);
    updatePowerUpBars(state);
    updateComboAndPopups(deltaTime);
}

void HUD::updateScoreSection(const GameState& state) {
    // Format score with leading zeros (5 digits)
    int score = static_cast<int>(state.score);
    std::ostringstream ss;
    ss << std::setw(5) << std::setfill('0') << score;
    scoreValueText_.setString(ss.str());

    // High score
    if (state.highScore > 0) {
        int hi = static_cast<int>(state.highScore);
        std::ostringstream hs;
        hs << std::setw(5) << std::setfill('0') << hi;
        highScoreValueText_.setString(hs.str());
    } else {
        highScoreValueText_.setString("---");
    }

    // Position high score below score value
    sf::FloatRect scoreBounds = scoreValueText_.getGlobalBounds();
    highScoreLabelText_.setPosition(20.0f, scoreBounds.top + scoreBounds.height + 2.0f);
    highScoreValueText_.setPosition(
        20.0f + highScoreLabelText_.getGlobalBounds().width + 8.0f,
        scoreBounds.top + scoreBounds.height + 1.0f
    );
}

void HUD::updateDistanceSection(const GameState& state) {
    int dist = static_cast<int>(state.distance);
    std::ostringstream ss;
    ss << dist << "m";
    distanceText_.setString(ss.str());

    // Center the distance text
    sf::FloatRect bounds = distanceText_.getGlobalBounds();
    distanceText_.setPosition(
        (Constants::WINDOW_WIDTH - bounds.width) / 2.0f,
        18.0f
    );
}

void HUD::updateCoinsAndHealth(const GameState& state) {
    // ── Coins ───────────────────────────────────────────────────────
    std::ostringstream ss;
    ss << state.coins;
    coinsValueText_.setString(ss.str());

    // Position coins on the right side (right-aligned)
    sf::FloatRect coinBounds = coinsValueText_.getGlobalBounds();
    float coinRightX = Constants::WINDOW_WIDTH - 20.0f - coinBounds.width;
    coinsValueText_.setPosition(coinRightX, 16.0f);
    coinsLabelText_.setPosition(coinRightX, 4.0f);

    // ── Health Hearts (fixed position, anchored to right edge) ──────
    const float HEART_AREA_X = Constants::WINDOW_WIDTH - 110.0f;
    healthLabelText_.setPosition(HEART_AREA_X, TOP_BAR_HEIGHT - 18.0f);

    float heartStartX = HEART_AREA_X + 28.0f;
    int health = std::min(state.lives, MAX_VISIBLE_HEARTS);
    for (int i = 0; i < MAX_VISIBLE_HEARTS; ++i) {
        auto& h = hearts_[i];
        float hx = heartStartX + i * 18.0f;
        float hy = TOP_BAR_HEIGHT - 19.0f;

        h.outer.setPosition(hx, hy);
        h.outer.setFillColor(i < health ? sf::Color(220, 60, 60) : sf::Color(40, 20, 20));
        h.filled = (i < health);

        h.inner.setPosition(hx + 2.5f, hy + 2.5f);
        h.inner.setFillColor(i < health ? sf::Color(255, 120, 120) : sf::Color(60, 40, 40));
    }
}

void HUD::updateSpeedIndicator(const GameState& state) {
    float ratio = (state.currentSpeed - Constants::BASE_SPEED) /
                  (Constants::MAX_SPEED - Constants::BASE_SPEED);
    ratio = std::max(0.0f, std::min(1.0f, ratio));

    speedBarFill_.setSize(sf::Vector2f(80.0f * ratio, 4.0f));

    // Color gradient: blue → yellow → red based on speed
    if (ratio < 0.5f) {
        float t = ratio / 0.5f;
        sf::Uint8 r = static_cast<sf::Uint8>(100 + 155 * t);
        sf::Uint8 g = static_cast<sf::Uint8>(200 + 55 * t);
        speedBarFill_.setFillColor(sf::Color(r, g, 255 - static_cast<sf::Uint8>(255 * t)));
    } else {
        float t = (ratio - 0.5f) / 0.5f;
        sf::Uint8 r = 255;
        sf::Uint8 g = static_cast<sf::Uint8>(255 - 200 * t);
        speedBarFill_.setFillColor(sf::Color(r, g, 0));
    }

    // Position below score section
    speedText_.setPosition(20.0f, TOP_BAR_HEIGHT - 14.0f);

    std::ostringstream ss;
    ss << "SPD " << static_cast<int>(state.currentSpeed * 0.36f) << " km/h";
    speedText_.setString(ss.str());

    sf::FloatRect spdBounds = speedText_.getGlobalBounds();
    speedBarBg_.setPosition(spdBounds.left + spdBounds.width + 6.0f, TOP_BAR_HEIGHT - 12.0f);
    speedBarFill_.setPosition(spdBounds.left + spdBounds.width + 6.0f, TOP_BAR_HEIGHT - 12.0f);
}

void HUD::updatePowerUpBars(const GameState& state) {
    auto updateBar = [](PowerUpBar& bar, bool active, float remaining, float total) {
        bar.active = active;
        bar.remaining = remaining;
        bar.total = total;
    };

    updateBar(shieldBar_,       state.shieldActive,       state.shieldTimer,       Constants::POWERUP_DURATION);
    updateBar(magnetBar_,       state.magnetActive,       state.magnetTimer,       Constants::POWERUP_DURATION);
    updateBar(doublePointsBar_, state.doublePointsActive, state.doublePointsTimer, Constants::POWERUP_DURATION);
    updateBar(speedBoostBar_,   state.speedBoostActive,   state.speedBoostTimer,   Constants::POWERUP_DURATION);
}

void HUD::updateComboAndPopups(float deltaTime) {
    // Update popup lifetimes
    for (auto it = popups_.begin(); it != popups_.end(); ) {
        it->lifetime += deltaTime;
        if (it->lifetime >= it->maxLifetime) {
            it = popups_.erase(it);
        } else {
            ++it;
        }
    }

    // Update combo visibility timer
    if (comboVisibleTimer_ > 0.0f) {
        comboVisibleTimer_ -= deltaTime;
    }
}

void HUD::addScorePopup(int value) {
    // Create popup at a fixed screen position (above center, near dino area)
    ScorePopup popup;
    popup.text.setFont(*scoreValueText_.getFont());
    popup.text.setCharacterSize(18);
    popup.text.setStyle(sf::Text::Bold);
    popup.text.setFillColor(sf::Color(255, 215, 0));
    popup.text.setOutlineColor(sf::Color(200, 100, 0));
    popup.text.setOutlineThickness(1.0f);

    std::ostringstream ss;
    ss << "+" << value;
    popup.text.setString(ss.str());

    // Position above the dino area
    float x = 150.0f;
    float y = Constants::GROUND_Y - 250.0f;
    popup.text.setPosition(x, y);
    popup.startY = y;

    // Random horizontal offset for variety
    popup.x = x + (std::rand() % 40 - 20);
    popup.maxLifetime = 0.8f + (std::rand() % 40) / 100.0f;

    popups_.push_back(std::move(popup));
}

// ── Render Methods ─────────────────────────────────────────────────────

void HUD::render(sf::RenderWindow& window) {
    renderTopBar(window);
    renderPowerUpBars(window);
    renderCombo(window);
    renderPopups(window);
}

void HUD::renderTopBar(sf::RenderWindow& window) {
    window.draw(topBarBg_);

    // Score section
    window.draw(scoreLabelText_);
    window.draw(scoreValueText_);
    window.draw(highScoreLabelText_);
    if (!highScoreValueText_.getString().isEmpty()) {
        window.draw(highScoreValueText_);
    }

    // Distance
    window.draw(distanceText_);

    // Coins
    window.draw(coinsLabelText_);
    window.draw(coinsValueText_);

    // Health
    window.draw(healthLabelText_);
    for (auto& h : hearts_) {
        window.draw(h.outer);
        window.draw(h.inner);
    }

    // Speed
    window.draw(speedText_);
    window.draw(speedBarBg_);
    window.draw(speedBarFill_);
}

void HUD::renderPopups(sf::RenderWindow& window) {
    for (auto& popup : popups_) {
        float progress = popup.lifetime / popup.maxLifetime; // 0→1
        float alpha = 1.0f - progress;
        
        // Fade out
        sf::Color color = popup.text.getFillColor();
        color.a = static_cast<sf::Uint8>(alpha * 255);
        popup.text.setFillColor(color);
        popup.text.setOutlineColor(sf::Color(200, 100, 0, color.a));

        // Float upward
        float offsetY = -progress * 50.0f;
        popup.text.setPosition(popup.x, popup.startY + offsetY);

        window.draw(popup.text);
    }
}

void HUD::renderCombo(sf::RenderWindow& window) {
    if (comboVisibleTimer_ > 0.0f && lastComboCount_ > 1) {
        float alpha = std::min(1.0f, comboVisibleTimer_ / 0.3f);

        // Pulse effect: scale combo text
        float pulse = 1.0f + 0.08f * std::sin(comboVisibleTimer_ * 12.0f);

        std::ostringstream ss;
        ss << "✧ " << lastComboCount_ << "× COMBO ✧";
        comboText_.setString(ss.str());

        // Color intensity based on combo count
        sf::Uint8 r = 255;
        sf::Uint8 g = static_cast<sf::Uint8>(std::min(255, 200 + lastComboCount_ * 10));
        sf::Uint8 b = static_cast<sf::Uint8>(std::max(0, 255 - lastComboCount_ * 30));
        comboText_.setFillColor(sf::Color(r, g, b, static_cast<sf::Uint8>(alpha * 255)));
        comboText_.setOutlineColor(sf::Color(200, 80, 0, static_cast<sf::Uint8>(alpha * 180)));

        // Center below the top bar
        sf::FloatRect bounds = comboText_.getGlobalBounds();
        comboText_.setPosition(
            (Constants::WINDOW_WIDTH - bounds.width) / 2.0f,
            TOP_BAR_HEIGHT + 15.0f
        );

        // Apply scale via a text transform (SFML doesn't support scale on text natively,
        // so we use a workaround with outline thickness to simulate "pulse")
        comboText_.setOutlineThickness(1.5f + pulse * 0.5f);

        window.draw(comboText_);
    }
}

void HUD::renderPowerUpBars(sf::RenderWindow& window) {
    float startX = Constants::WINDOW_WIDTH / 2.0f - 130.0f;
    float y = TOP_BAR_HEIGHT + 10.0f;

    auto renderBar = [&](PowerUpBar& bar, float& yPos) {
        if (!bar.active) return;

        bar.label.setPosition(startX, yPos);
        window.draw(bar.label);

        // Timer bar
        float ratio = bar.total > 0.0f ? (bar.remaining / bar.total) : 0.0f;
        bar.bg.setPosition(startX + 70.0f, yPos + 2.0f);
        window.draw(bar.bg);

        bar.fill.setSize(sf::Vector2f(bar.maxWidth * ratio, 8.0f));
        bar.fill.setPosition(startX + 70.0f, yPos + 2.0f);
        window.draw(bar.fill);

        // Timer text
        sf::Text timerText;
        timerText.setFont(*bar.label.getFont());
        timerText.setCharacterSize(9);
        timerText.setFillColor(sf::Color(180, 180, 180));
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << bar.remaining << "s";
        timerText.setString(ss.str());
        timerText.setPosition(startX + 70.0f + bar.maxWidth + 4.0f, yPos + 1.0f);
        window.draw(timerText);

        yPos += 18.0f;
    };

    renderBar(shieldBar_, y);
    renderBar(magnetBar_, y);
    renderBar(doublePointsBar_, y);
    renderBar(speedBoostBar_, y);
}

// ── Public API for external calls ─────────────────────────────────────
// Note: Popups are spawned automatically in update() by detecting coin/score
// changes. Game::update() calls HUD::update(state, deltaTime) which tracks
// prevScore_ and prevCoins_ internally to create +100 popups on each coin.
