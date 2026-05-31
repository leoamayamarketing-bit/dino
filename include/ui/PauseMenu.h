#ifndef UI_PAUSE_MENU_H
#define UI_PAUSE_MENU_H

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <vector>
#include <functional>

class PauseMenu {
public:
    PauseMenu();

    void init(sf::Font& font);
    void update(float deltaTime);
    void render(sf::RenderWindow& window);
    void handleInput();  // Deprecated — use event-based methods below

    /// Event-based navigation (called from Game::handleEvents)
    void navigateUp();
    void navigateDown();
    void selectCurrent();

    bool shouldResume() const { return shouldResume_; }
    bool shouldRestart() const { return shouldRestart_; }
    bool shouldQuit() const { return shouldQuit_; }

    void reset();

    // Called after consuming the action
    void consumeResume() { shouldResume_ = false; }
    void consumeRestart() { shouldRestart_ = false; }
    void consumeQuit() { shouldQuit_ = false; }

private:
    enum class Option {
        RESUME,
        RESTART,
        QUIT
    };

    Option selectedOption_ = Option::RESUME;
    int menuIndex_ = 0;

    bool shouldResume_ = false;
    bool shouldRestart_ = false;
    bool shouldQuit_ = false;

    sf::RectangleShape overlay_;
    sf::Text titleText_;
    std::vector<sf::Text> menuItems_;

    float blinkTimer_ = 0.0f;
    bool showBlink_ = true;

    void updateSelection();
};

#endif
