#ifndef UI_MENU_H
#define UI_MENU_H

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <vector>
#include "../core/GameState.h"
#include "../core/AssetManager.h"
#include "../core/KeyBindings.h"

class Menu {
public:
    enum class MenuOption {
        START_GAME,
        SELECT_DINO,
        SELECT_LEVEL,
        OPTIONS,
        QUIT
    };

    Menu();

    void init(sf::Font& font, AssetManager& assets);
    void handleInput(const GameState& state);
    void update(float deltaTime);
    void render(sf::RenderWindow& window);

    /// Set the KeyBindings for the options/remap screen
    void setBindings(KeyBindings* bindings) { bindings_ = bindings; }

    MenuOption getSelectedOption() const { return selectedOption_; }
    Constants::DinoType getSelectedDino() const { return selectedDino_; }
    Constants::LevelType getSelectedLevel() const { return selectedLevel_; }
    bool isDinoSelected() const { return dinoSelected_; }
    bool isLevelSelected() const { return levelSelected_; }
    bool shouldStart() const { return shouldStart_; }
    bool isHardMode() const { return hardMode_; }

    bool isWaitingForKey() const { return waitingForKey_; }
    bool captureKey(sf::Keyboard::Key key);  // returns true if key was captured

    void reset();
    void setMenuState(int state) { menuState_ = static_cast<MenuState>(state); }
    void buildRemapItems();
    void setGameStatePointer(const GameState* state) { gameState_ = state; }

private:
    enum class MenuState {
        MAIN,
        DINO_SELECT,
        LEVEL_SELECT,
        OPTIONS,
        KEY_REMAP
    };

    MenuState menuState_ = MenuState::MAIN;
    MenuOption selectedOption_ = MenuOption::START_GAME;
    Constants::DinoType selectedDino_ = Constants::DinoType::TREX;
    Constants::LevelType selectedLevel_ = Constants::LevelType::DESERT_DAY;
    bool hardMode_ = false;
    bool dinoSelected_ = false;
    bool levelSelected_ = false;
    bool shouldStart_ = false;

    int menuIndex_ = 0;
    int dinoIndex_ = 0;
    int levelIndex_ = 0;
    int optionsIndex_ = 0;   // which option item is selected
    int remapIndex_ = 0;     // which action is being remapped
    bool waitingForKey_ = false;  // true when listening for a new key press

    KeyBindings* bindings_ = nullptr;
    const GameState* gameState_ = nullptr;

    std::vector<KeyBindings::ActionEntry> remapActions_;

    sf::Text titleText_;
    std::vector<sf::Text> menuItems_;
    std::vector<sf::Text> dinoItems_;
    std::vector<sf::Text> levelItems_;
    std::vector<sf::Text> optionsItems_;
    std::vector<sf::Text> remapItems_;
    sf::Text instructionsText_;
    sf::Text remapHintText_;     // "Press a key..." indicator
    sf::Sprite dinoPreview_;
    sf::Texture* dinoTexture_ = nullptr;

    float titleBobTimer_ = 0.0f;
    float blinkTimer_ = 0.0f;
    bool showBlink_ = true;
    float remapBlinkTimer_ = 0.0f;
    bool showRemapCursor_ = true;

    void updateMenuSelection();
    void renderMain(sf::RenderWindow& window);
    void renderDinoSelect(sf::RenderWindow& window);
    void renderLevelSelect(sf::RenderWindow& window);
    void renderOptions(sf::RenderWindow& window);
    void renderKeyRemap(sf::RenderWindow& window);

    void startRemapping();
    void finishRemapping();
    void updateHardModeLabel();
};

#endif
