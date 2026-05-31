#include "../../include/ui/PauseMenu.h"
#include "../../include/core/Constants.h"
#include <SFML/Window/Keyboard.hpp>
#include <cmath>

PauseMenu::PauseMenu() = default;

void PauseMenu::init(sf::Font& font) {
    // Overlay
    overlay_.setSize(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
    overlay_.setFillColor(sf::Color(0, 0, 0, 180));

    // Title
    titleText_.setFont(font);
    titleText_.setCharacterSize(56);
    titleText_.setStyle(sf::Text::Bold);
    titleText_.setFillColor(sf::Color::White);
    titleText_.setString("PAUSED");
    titleText_.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - titleText_.getGlobalBounds().width / 2.0f,
        180.0f);

    // Menu items
    const char* labels[] = {"Resume", "Restart", "Quit to Menu"};
    for (int i = 0; i < 3; i++) {
        sf::Text item;
        item.setFont(font);
        item.setCharacterSize(30);
        item.setFillColor(sf::Color::White);
        item.setString(labels[i]);
        item.setPosition(
            Constants::WINDOW_WIDTH / 2.0f - item.getGlobalBounds().width / 2.0f,
            310.0f + i * 60.0f);
        menuItems_.push_back(item);
    }

    updateSelection();
}

void PauseMenu::update(float deltaTime) {
    blinkTimer_ += deltaTime;
    if (blinkTimer_ > 0.4f) {
        blinkTimer_ = 0.0f;
        showBlink_ = !showBlink_;
    }
}

void PauseMenu::render(sf::RenderWindow& window) {
    window.draw(overlay_);

    // Title with slight glow effect
    sf::Text glow = titleText_;
    glow.setFillColor(sf::Color(255, 215, 0, 60));
    glow.setPosition(titleText_.getPosition().x + 2.0f, titleText_.getPosition().y + 2.0f);
    window.draw(glow);
    window.draw(titleText_);

    // Separator line
    sf::RectangleShape line(sf::Vector2f(200.0f, 2.0f));
    line.setFillColor(sf::Color(255, 215, 0, 120));
    line.setPosition(
        Constants::WINDOW_WIDTH / 2.0f - 100.0f,
        260.0f);
    window.draw(line);

    for (auto& item : menuItems_) {
        window.draw(item);
    }

    // Hint text
    if (!menuItems_.empty()) {
        sf::Text hint;
        hint.setFont(*menuItems_[0].getFont());
        hint.setCharacterSize(14);
        hint.setFillColor(sf::Color(130, 130, 130));
        hint.setString("ESC to resume | UP/DOWN to navigate | ENTER to select");
        hint.setPosition(
            Constants::WINDOW_WIDTH / 2.0f - hint.getGlobalBounds().width / 2.0f,
            Constants::WINDOW_HEIGHT - 80.0f);
        window.draw(hint);
    }
}

void PauseMenu::handleInput() {
    // Deprecated: use event-based navigateUp/Down/Select instead
    // Kept for compatibility but not called from Game anymore
}

void PauseMenu::navigateUp() {
    menuIndex_ = (menuIndex_ - 1 + 3) % 3;
    updateSelection();
}

void PauseMenu::navigateDown() {
    menuIndex_ = (menuIndex_ + 1) % 3;
    updateSelection();
}

void PauseMenu::selectCurrent() {
    switch (selectedOption_) {
        case Option::RESUME: shouldResume_ = true; break;
        case Option::RESTART: shouldRestart_ = true; break;
        case Option::QUIT: shouldQuit_ = true; break;
    }
}

void PauseMenu::reset() {
    menuIndex_ = 0;
    selectedOption_ = Option::RESUME;
    shouldResume_ = false;
    shouldRestart_ = false;
    shouldQuit_ = false;
    updateSelection();
}

void PauseMenu::updateSelection() {
    selectedOption_ = static_cast<Option>(menuIndex_);
    for (size_t i = 0; i < menuItems_.size(); i++) {
        menuItems_[i].setFillColor(
            static_cast<int>(i) == menuIndex_ ? sf::Color(255, 215, 0) : sf::Color::White);
        menuItems_[i].setCharacterSize(
            static_cast<int>(i) == menuIndex_ ? 32 : 30);
    }
}
