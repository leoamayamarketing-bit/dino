#include "../../include/ui/Menu.h"
#include "../../include/core/InputManager.h"
#include "../../include/core/Constants.h"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <cmath>
#include <cstdlib>

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
        "Options",
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

    // Options items
    const char* optionsLabels[] = {
        "Controls: Remap Keys",
        "Reset to Defaults",
        "Back"
    };

    for (int i = 0; i < 3; i++) {
        sf::Text item;
        item.setFont(font);
        item.setCharacterSize(26);
        item.setFillColor(sf::Color::White);
        item.setString(optionsLabels[i]);
        item.setPosition(
            Constants::WINDOW_WIDTH / 2 - item.getGlobalBounds().width / 2,
            220 + i * 55);
        optionsItems_.push_back(item);
    }

    // Remap hint text (hidden by default)
    remapHintText_.setFont(font);
    remapHintText_.setCharacterSize(22);
    remapHintText_.setFillColor(sf::Color(255, 220, 100));
    remapHintText_.setString("Press a key...");
    remapHintText_.setPosition(
        Constants::WINDOW_WIDTH / 2 - remapHintText_.getGlobalBounds().width / 2,
        520);

    // Instructions
    instructionsText_.setFont(font);
    instructionsText_.setCharacterSize(16);
    instructionsText_.setFillColor(sf::Color(150, 150, 150));
    instructionsText_.setString("Use W/S or UP/DOWN to navigate | SPACE/ENTER to select");
    instructionsText_.setPosition(
        Constants::WINDOW_WIDTH / 2 - instructionsText_.getGlobalBounds().width / 2,
        Constants::WINDOW_HEIGHT - 60);

    // Populate action list for remap
    if (bindings_) bindings_->getActions(remapActions_);

    updateMenuSelection();
}

void Menu::handleInput(const GameState& state) {
    // Determine navigation keys from the input manager if available,
    // otherwise fall back to hardcoded checks.
    // For menu navigation we use isKeyJustPressed to avoid repeated scrolling.

    // Use bindings_ directly with sf::Keyboard::isKeyPressed for menu navigation
    // so that holding a key scrolls quickly (key repeat works naturally).
    auto navUp = [&]() -> bool {
        if (bindings_) {
            return sf::Keyboard::isKeyPressed(bindings_->menuUp) ||
                   sf::Keyboard::isKeyPressed(bindings_->menuUpAlt);
        }
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Up) ||
               sf::Keyboard::isKeyPressed(sf::Keyboard::W);
    };
    auto navDown = [&]() -> bool {
        if (bindings_) {
            return sf::Keyboard::isKeyPressed(bindings_->menuDown) ||
                   sf::Keyboard::isKeyPressed(bindings_->menuDownAlt);
        }
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ||
               sf::Keyboard::isKeyPressed(sf::Keyboard::S);
    };
    auto navSelect = [&]() -> bool {
        if (bindings_) {
            return sf::Keyboard::isKeyPressed(bindings_->menuSelect) ||
                   sf::Keyboard::isKeyPressed(bindings_->menuSelectAlt);
        }
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Space) ||
               sf::Keyboard::isKeyPressed(sf::Keyboard::Enter);
    };
    auto navBack = [&]() -> bool {
        if (bindings_) {
            return sf::Keyboard::isKeyPressed(bindings_->menuBack);
        }
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Escape);
    };

    // ----- KEY REMAP: waiting for key capture -----
    if (menuState_ == MenuState::KEY_REMAP && waitingForKey_) {
        // We handle this from Game::handleEvents() via captureKey()
        return;
    }

    // ----- Navigation (just-pressed style) -----
    // Use SFML direct checks since input_->isMenuUpPressed uses isKeyJustPressed internally
    // which depends on InputManager event processing. For menu, simple polling is fine.
    bool moved = false;
    int maxIndex = 0;

    switch (menuState_) {
        case MenuState::MAIN: maxIndex = 4; break;
        case MenuState::DINO_SELECT: maxIndex = 2; break;
        case MenuState::LEVEL_SELECT: maxIndex = 4; break;
        case MenuState::OPTIONS: maxIndex = 2; break;
        case MenuState::KEY_REMAP: maxIndex = static_cast<int>(remapItems_.size()) - 1; break;
    }

    if (navUp()) {
        menuIndex_--;
        if (menuIndex_ < 0) menuIndex_ = 0;
        moved = true;
    }
    if (navDown()) {
        menuIndex_++;
        if (menuIndex_ > maxIndex) menuIndex_ = maxIndex;
        moved = true;
    }

    if (navSelect()) {
        moved = true;
        switch (menuState_) {
            case MenuState::MAIN:
                switch (menuIndex_) {
                    case 0: shouldStart_ = true; break;                    // Start Game
                    case 1: menuState_ = MenuState::DINO_SELECT; menuIndex_ = 0; break;
                    case 2: menuState_ = MenuState::LEVEL_SELECT; menuIndex_ = 0; break;
                    case 3:                                                 // Options
                        menuState_ = MenuState::OPTIONS;
                        menuIndex_ = 0;
                        if (bindings_) bindings_->getActions(remapActions_);
                        break;
                    case 4: std::exit(0); break;                            // Quit
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

            case MenuState::OPTIONS:
                switch (menuIndex_) {
                    case 0: // Controls: Remap Keys
                        menuState_ = MenuState::KEY_REMAP;
                        menuIndex_ = 0;
                        remapIndex_ = 0;
                        waitingForKey_ = false;
                        buildRemapItems();
                        break;
                    case 1: // Reset to Defaults
                        if (bindings_) {
                            bindings_->setDefaults();
                            bindings_->getActions(remapActions_);
                            buildRemapItems();
                        }
                        break;
                    case 2: // Back
                        menuState_ = MenuState::MAIN;
                        menuIndex_ = 3; // back to Options
                        break;
                }
                break;

            case MenuState::KEY_REMAP:
                if (!waitingForKey_ && menuIndex_ >= 0 && menuIndex_ < static_cast<int>(remapActions_.size())) {
                    startRemapping();
                    buildRemapItems();
                }
                break;
        }
    }

    if (navBack()) {
        switch (menuState_) {
            case MenuState::DINO_SELECT:
            case MenuState::LEVEL_SELECT:
                menuState_ = MenuState::MAIN;
                menuIndex_ = 0;
                break;
            case MenuState::OPTIONS:
                menuState_ = MenuState::MAIN;
                menuIndex_ = 3;
                break;
            case MenuState::KEY_REMAP:
                if (waitingForKey_) {
                    waitingForKey_ = false; // cancel waiting
                } else {
                    menuState_ = MenuState::OPTIONS;
                    menuIndex_ = 0;
                }
                buildRemapItems();
                break;
            default:
                break;
        }
    }

    // When in KEY_REMAP, rebuild items if waiting state changed
    if (menuState_ == MenuState::KEY_REMAP) {
        updateMenuSelection();
    } else {
        updateMenuSelection();
    }
}

void Menu::startRemapping() {
    if (menuIndex_ < 0 || menuIndex_ >= static_cast<int>(remapActions_.size())) return;
    waitingForKey_ = true;
    remapBlinkTimer_ = 0.0f;
    showRemapCursor_ = true;
    remapHintText_.setString("Press any key for \"" + remapActions_[menuIndex_].displayName + "\" | ESC to cancel");
    remapHintText_.setPosition(
        Constants::WINDOW_WIDTH / 2 - remapHintText_.getGlobalBounds().width / 2,
        520);
}

bool Menu::captureKey(sf::Keyboard::Key key) {
    if (!waitingForKey_ || menuState_ != MenuState::KEY_REMAP) return false;
    if (key == sf::Keyboard::Escape) {
        waitingForKey_ = false;
        buildRemapItems();
        return true;
    }
    if (menuIndex_ >= 0 && menuIndex_ < static_cast<int>(remapActions_.size())) {
        auto& entry = remapActions_[menuIndex_];
        if (entry.primaryKey) {
            *entry.primaryKey = key;
        }
    }
    waitingForKey_ = false;
    // Re-fetch actions from bindings to keep pointers fresh
    if (bindings_) {
        bindings_->getActions(remapActions_);
        bindings_->saveToFile("keybindings.cfg");
    }
    buildRemapItems();
    updateMenuSelection();
    return true;
}

void Menu::finishRemapping() {
    waitingForKey_ = false;
    buildRemapItems();
}

void Menu::buildRemapItems() {
    remapItems_.clear();
    sf::Font* font = nullptr;
    if (!remapActions_.empty() && !menuItems_.empty())
        font = const_cast<sf::Font*>(menuItems_[0].getFont());

    if (!font) return;

    for (size_t i = 0; i < remapActions_.size(); ++i) {
        auto& entry = remapActions_[i];
        std::string label = entry.displayName + ": ";
        if (waitingForKey_ && static_cast<int>(i) == menuIndex_) {
            label += "[ ... ]";
        } else if (entry.primaryKey) {
            label += KeyBindings::keyName(*entry.primaryKey);
        }

        sf::Text item;
        item.setFont(*font);
        item.setCharacterSize(24);
        item.setFillColor(sf::Color::White);
        item.setString(label);
        item.setPosition(
            Constants::WINDOW_WIDTH / 2 - item.getGlobalBounds().width / 2,
            180 + static_cast<int>(i) * 50);
        remapItems_.push_back(item);
    }

    // "Back" option at the end
    sf::Text backItem;
    backItem.setFont(*font);
    backItem.setCharacterSize(24);
    backItem.setFillColor(sf::Color(180, 180, 180));
    backItem.setString("Back");
    backItem.setPosition(
        Constants::WINDOW_WIDTH / 2 - backItem.getGlobalBounds().width / 2,
        180 + static_cast<int>(remapActions_.size()) * 50 + 10);
    remapItems_.push_back(backItem);

    if (menuState_ == MenuState::KEY_REMAP && menuIndex_ >= static_cast<int>(remapItems_.size())) {
        menuIndex_ = static_cast<int>(remapItems_.size()) - 1;
    }
    if (menuState_ == MenuState::KEY_REMAP && menuIndex_ < 0) menuIndex_ = 0;
}

void Menu::update(float deltaTime) {
    titleBobTimer_ += deltaTime;
    blinkTimer_ += deltaTime;
    if (blinkTimer_ > 0.5f) {
        blinkTimer_ = 0.0f;
        showBlink_ = !showBlink_;
    }
    if (waitingForKey_) {
        remapBlinkTimer_ += deltaTime;
        if (remapBlinkTimer_ > 0.4f) {
            remapBlinkTimer_ = 0.0f;
            showRemapCursor_ = !showRemapCursor_;
        }
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
    if (menuState_ != MenuState::KEY_REMAP) {
        window.draw(titleText_);
    }

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
        case MenuState::OPTIONS:
            renderOptions(window);
            break;
        case MenuState::KEY_REMAP:
            renderKeyRemap(window);
            break;
    }

    // Only show instructions in non-remap states
    if (menuState_ != MenuState::KEY_REMAP) {
        window.draw(instructionsText_);
    }
}

void Menu::reset() {
    menuState_ = MenuState::MAIN;
    menuIndex_ = 0;
    shouldStart_ = false;
    dinoSelected_ = false;
    levelSelected_ = false;
    waitingForKey_ = false;
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
        case MenuState::OPTIONS:
            for (size_t i = 0; i < optionsItems_.size(); i++) {
                optionsItems_[i].setFillColor(
                    static_cast<int>(i) == menuIndex_ ? sf::Color::Yellow : sf::Color::White);
            }
            break;
        case MenuState::KEY_REMAP:
            for (size_t i = 0; i < remapItems_.size(); i++) {
                remapItems_[i].setFillColor(
                    static_cast<int>(i) == menuIndex_ ? sf::Color::Yellow : sf::Color::White);
            }
            break;
    }
}

// ===========================================================================
// Render sub-screens
// ===========================================================================
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

void Menu::renderOptions(sf::RenderWindow& window) {
    sf::Text title;
    title.setFont(*optionsItems_[0].getFont());
    title.setCharacterSize(32);
    title.setFillColor(sf::Color::White);
    title.setString("OPTIONS");
    title.setPosition(
        Constants::WINDOW_WIDTH / 2 - title.getGlobalBounds().width / 2,
        150);
    window.draw(title);

    for (auto& item : optionsItems_) {
        window.draw(item);
    }

    // Show current key binding summary
    if (bindings_) {
        sf::Text summary;
        summary.setFont(*optionsItems_[0].getFont());
        summary.setCharacterSize(14);
        summary.setFillColor(sf::Color(120, 120, 120));
        summary.setString("Jump: " + KeyBindings::keyName(bindings_->jump) +
                          " | Crouch: " + KeyBindings::keyName(bindings_->crouch) +
                          " | Dash: " + KeyBindings::keyName(bindings_->dash));
        summary.setPosition(
            Constants::WINDOW_WIDTH / 2 - summary.getGlobalBounds().width / 2,
            480);
        window.draw(summary);
    }
}

void Menu::renderKeyRemap(sf::RenderWindow& window) {
    sf::Text title;
    if (!remapItems_.empty()) {
        title.setFont(*remapItems_[0].getFont());
    } else {
        return;
    }
    title.setCharacterSize(28);
    title.setFillColor(sf::Color::White);
    title.setString("CONTROLS — REMAP KEYS");
    title.setPosition(
        Constants::WINDOW_WIDTH / 2 - title.getGlobalBounds().width / 2,
        110);
    window.draw(title);

    // Draw action list
    for (size_t i = 0; i < remapItems_.size(); ++i) {
        auto& item = remapItems_[i];
        if (waitingForKey_ && static_cast<int>(i) == menuIndex_ && showRemapCursor_) {
            // Highlight the waiting item
            sf::Text highlight = item;
            highlight.setFillColor(sf::Color(255, 220, 100));
            window.draw(highlight);
        } else {
            window.draw(item);
        }
    }

    // Hint text
    if (waitingForKey_) {
        window.draw(remapHintText_);
    }
    if (!waitingForKey_) {
        sf::Text hint;
        if (!remapItems_.empty()) {
            hint.setFont(*remapItems_[0].getFont());
        }
        hint.setCharacterSize(14);
        hint.setFillColor(sf::Color(130, 130, 130));
        hint.setString("Select an action, press ENTER/SPACE, then press the new key");
        hint.setPosition(
            Constants::WINDOW_WIDTH / 2 - hint.getGlobalBounds().width / 2,
            550);
        window.draw(hint);
    }
}
