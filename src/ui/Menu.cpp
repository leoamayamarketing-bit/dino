#include "../../include/ui/Menu.h"
#include "../../include/core/Constants.h"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <cmath>

Menu::Menu() = default;

void Menu::init(sf::Font& font, AssetManager& assets) {
    // Title
    titleText_.setFont(font);
    titleText_.setCharacterSize(48);
    titleText_.setFillColor(sf::Color(255, 165, 0));
    titleText_.setString("DinoRunner: Primal Rush");
    titleText_.setPosition(
        Constants::WINDOW_WIDTH / 2 - titleText_.getGlobalBounds().width / 2,
        80);

    // Menu items
    const char* menuLabels[] = {
        "Start Game",
        "Select Dinosaur",
        "Select Level",
        "High Scores",
        "Quit"
    };

    for (int i = 0; i < 5; i++) {
        sf::Text item;
        item.setFont(font);
        item.setCharacterSize(28);
        item.setFillColor(sf::Color::White);
        item.setString(menuLabels[i]);
        item.setPosition(
            Constants::WINDOW_WIDTH / 2 - item.getGlobalBounds().width / 2,
            200 + i * 50);
        menuItems_.push_back(item);
    }

    // Dinosaur selection items
    const char* dinoLabels[] = {
        "T-Rex (Balanced) [UNLOCKED]",
        "Velociraptor (Fast) [LOCKED]",
        "Triceratops (Tank) [LOCKED]"
    };

    for (int i = 0; i < 3; i++) {
        sf::Text item;
        item.setFont(font);
        item.setCharacterSize(26);
        item.setFillColor(sf::Color::White);
        item.setString(dinoLabels[i]);
        item.setPosition(
            Constants::WINDOW_WIDTH / 2 - item.getGlobalBounds().width / 2,
            200 + i * 60);
        dinoItems_.push_back(item);
    }

    // Level selection items
    const char* levelLabels[] = {
        "Desert (Day)",
        "Desert (Night)",
        "Caverns",
        "Volcano",
        "Infinite Mode"
    };

    for (int i = 0; i < 5; i++) {
        sf::Text item;
        item.setFont(font);
        item.setCharacterSize(26);
        item.setFillColor(sf::Color::White);
        item.setString(levelLabels[i]);
        item.setPosition(
            Constants::WINDOW_WIDTH / 2 - item.getGlobalBounds().width / 2,
            180 + i * 55);
        levelItems_.push_back(item);
    }

    // Instructions
    instructionsText_.setFont(font);
    instructionsText_.setCharacterSize(16);
    instructionsText_.setFillColor(sf::Color(150, 150, 150));
    instructionsText_.setString("Use W/S or UP/DOWN to navigate | SPACE/ENTER to select");
    instructionsText_.setPosition(
        Constants::WINDOW_WIDTH / 2 - instructionsText_.getGlobalBounds().width / 2,
        Constants::WINDOW_HEIGHT - 60);

    updateMenuSelection();
}

void Menu::handleInput(const GameState& state) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        menuIndex_--;
        if (menuIndex_ < 0) menuIndex_ = 0;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        int maxIndex = 0;
        switch (menuState_) {
            case MenuState::MAIN: maxIndex = 4; break;
            case MenuState::DINO_SELECT: maxIndex = 2; break;
            case MenuState::LEVEL_SELECT: maxIndex = 4; break;
        }
        menuIndex_++;
        if (menuIndex_ > maxIndex) menuIndex_ = maxIndex;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
        switch (menuState_) {
            case MenuState::MAIN:
                switch (menuIndex_) {
                    case 0: // Start Game
                        shouldStart_ = true;
                        break;
                    case 1: // Select Dino
                        menuState_ = MenuState::DINO_SELECT;
                        menuIndex_ = 0;
                        break;
                    case 2: // Select Level
                        menuState_ = MenuState::LEVEL_SELECT;
                        menuIndex_ = 0;
                        break;
                    case 3: // High Scores
                        // TODO: Show high scores
                        break;
                    case 4: // Quit
                        std::exit(0);
                        break;
                }
                break;

            case MenuState::DINO_SELECT:
                selectedDino_ = static_cast<Constants::DinoType>(menuIndex_);
                menuState_ = MenuState::MAIN;
                menuIndex_ = 0;
                break;

            case MenuState::LEVEL_SELECT:
                selectedLevel_ = static_cast<Constants::LevelType>(menuIndex_);
                menuState_ = MenuState::MAIN;
                menuIndex_ = 0;
                break;
        }
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
        menuState_ = MenuState::MAIN;
        menuIndex_ = 0;
        shouldStart_ = false;
    }

    updateMenuSelection();
}

void Menu::update(float deltaTime) {
    titleBobTimer_ += deltaTime;
    blinkTimer_ += deltaTime;
    if (blinkTimer_ > 0.5f) {
        blinkTimer_ = 0.0f;
        showBlink_ = !showBlink_;
    }
}

void Menu::render(sf::RenderWindow& window) {
    // Background
    sf::RectangleShape bg(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
    bg.setFillColor(sf::Color(20, 20, 30));
    window.draw(bg);

    // Title with slight bob animation
    float bobOffset = std::sin(titleBobTimer_ * 2.0f) * 5.0f;
    titleText_.setPosition(
        Constants::WINDOW_WIDTH / 2 - titleText_.getGlobalBounds().width / 2,
        80 + bobOffset);
    window.draw(titleText_);

    switch (menuState_) {
        case MenuState::MAIN:
            renderMain(window);
            break;
        case MenuState::DINO_SELECT:
            renderDinoSelect(window);
            break;
        case MenuState::LEVEL_SELECT:
            renderLevelSelect(window);
            break;
    }

    window.draw(instructionsText_);
}

void Menu::reset() {
    menuState_ = MenuState::MAIN;
    menuIndex_ = 0;
    shouldStart_ = false;
    dinoSelected_ = false;
    levelSelected_ = false;
}

void Menu::updateMenuSelection() {
    switch (menuState_) {
        case MenuState::MAIN:
            for (size_t i = 0; i < menuItems_.size(); i++) {
                menuItems_[i].setFillColor(
                    static_cast<int>(i) == menuIndex_ ? sf::Color::Yellow : sf::Color::White);
            }
            break;
        case MenuState::DINO_SELECT:
            for (size_t i = 0; i < dinoItems_.size(); i++) {
                dinoItems_[i].setFillColor(
                    static_cast<int>(i) == menuIndex_ ? sf::Color::Yellow : sf::Color::White);
            }
            break;
        case MenuState::LEVEL_SELECT:
            for (size_t i = 0; i < levelItems_.size(); i++) {
                levelItems_[i].setFillColor(
                    static_cast<int>(i) == menuIndex_ ? sf::Color::Yellow : sf::Color::White);
            }
            break;
    }
}

void Menu::renderMain(sf::RenderWindow& window) {
    for (auto& item : menuItems_) {
        window.draw(item);
    }

    // Selected dino/level info
    sf::Text infoText;
    infoText.setFont(*menuItems_[0].getFont());
    infoText.setCharacterSize(16);
    infoText.setFillColor(sf::Color(150, 150, 150));

    std::string info = "Dino: ";
    switch (selectedDino_) {
        case Constants::DinoType::TREX: info += "T-Rex"; break;
        case Constants::DinoType::VELOCIRAPTOR: info += "Velociraptor"; break;
        case Constants::DinoType::TRICERATOPS: info += "Triceratops"; break;
    }
    info += " | Level: ";
    switch (selectedLevel_) {
        case Constants::LevelType::DESERT_DAY: info += "Desert Day"; break;
        case Constants::LevelType::DESERT_NIGHT: info += "Desert Night"; break;
        case Constants::LevelType::CAVE: info += "Caverns"; break;
        case Constants::LevelType::VOLCANO: info += "Volcano"; break;
        case Constants::LevelType::INFINITE: info += "Infinite"; break;
    }

    infoText.setString(info);
    infoText.setPosition(
        Constants::WINDOW_WIDTH / 2 - infoText.getGlobalBounds().width / 2,
        480);
    window.draw(infoText);
}

void Menu::renderDinoSelect(sf::RenderWindow& window) {
    sf::Text title;
    title.setFont(*dinoItems_[0].getFont());
    title.setCharacterSize(32);
    title.setFillColor(sf::Color::White);
    title.setString("SELECT YOUR DINOSAUR");
    title.setPosition(
        Constants::WINDOW_WIDTH / 2 - title.getGlobalBounds().width / 2,
        130);
    window.draw(title);

    for (auto& item : dinoItems_) {
        window.draw(item);
    }

    if (showBlink_) {
        sf::Text hint;
        hint.setFont(*dinoItems_[0].getFont());
        hint.setCharacterSize(16);
        hint.setFillColor(sf::Color(150, 150, 150));
        hint.setString("Unlock more by collecting 100 coins in a run!");
        hint.setPosition(
            Constants::WINDOW_WIDTH / 2 - hint.getGlobalBounds().width / 2,
            450);
        window.draw(hint);
    }
}

void Menu::renderLevelSelect(sf::RenderWindow& window) {
    sf::Text title;
    title.setFont(*levelItems_[0].getFont());
    title.setCharacterSize(32);
    title.setFillColor(sf::Color::White);
    title.setString("SELECT LEVEL");
    title.setPosition(
        Constants::WINDOW_WIDTH / 2 - title.getGlobalBounds().width / 2,
        110);
    window.draw(title);

    for (auto& item : levelItems_) {
        window.draw(item);
    }

    if (showBlink_) {
        sf::Text hint;
        hint.setFont(*levelItems_[0].getFont());
        hint.setCharacterSize(16);
        hint.setFillColor(sf::Color(150, 150, 150));
        hint.setString("ARROW KEYS to navigate | ENTER to select | ESC to go back");
        hint.setPosition(
            Constants::WINDOW_WIDTH / 2 - hint.getGlobalBounds().width / 2,
            500);
        window.draw(hint);
    }
}
