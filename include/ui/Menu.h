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

class Menu {
public:
    enum class MenuOption {
        START_GAME,
        SELECT_DINO,
        SELECT_LEVEL,
        HIGH_SCORES,
        QUIT
    };

    Menu();

    void init(sf::Font& font, AssetManager& assets);
    void handleInput(const GameState& state);
    void update(float deltaTime);
    void render(sf::RenderWindow& window);

    MenuOption getSelectedOption() const { return selectedOption_; }
    Constants::DinoType getSelectedDino() const { return selectedDino_; }
    Constants::LevelType getSelectedLevel() const { return selectedLevel_; }
    bool isDinoSelected() const { return dinoSelected_; }
    bool isLevelSelected() const { return levelSelected_; }
    bool shouldStart() const { return shouldStart_; }

    void reset();
    void setMenuState(int state) { menuState_ = static_cast<MenuState>(state); }

private:
    enum class MenuState {
        MAIN,
        DINO_SELECT,
        LEVEL_SELECT
    };

    MenuState menuState_ = MenuState::MAIN;
    MenuOption selectedOption_ = MenuOption::START_GAME;
    Constants::DinoType selectedDino_ = Constants::DinoType::TREX;
    Constants::LevelType selectedLevel_ = Constants::LevelType::DESERT_DAY;
    bool dinoSelected_ = false;
    bool levelSelected_ = false;
    bool shouldStart_ = false;

    int menuIndex_ = 0;
    int dinoIndex_ = 0;
    int levelIndex_ = 0;

    sf::Text titleText_;
    std::vector<sf::Text> menuItems_;
    std::vector<sf::Text> dinoItems_;
    std::vector<sf::Text> levelItems_;
    sf::Text instructionsText_;
    sf::Sprite dinoPreview_;
    sf::Texture* dinoTexture_ = nullptr;

    float titleBobTimer_ = 0.0f;
    float blinkTimer_ = 0.0f;
    bool showBlink_ = true;

    void updateMenuSelection();
    void renderMain(sf::RenderWindow& window);
    void renderDinoSelect(sf::RenderWindow& window);
    void renderLevelSelect(sf::RenderWindow& window);
};

#endif
