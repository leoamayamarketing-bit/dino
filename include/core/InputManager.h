#ifndef CORE_INPUTMANAGER_H
#define CORE_INPUTMANAGER_H

#include <SFML/Window/Keyboard.hpp>
#include <unordered_map>
#include <functional>

class InputManager {
public:
    InputManager();

    void update();
    bool isKeyPressed(sf::Keyboard::Key key) const;
    bool isKeyJustPressed(sf::Keyboard::Key key) const;
    bool isKeyJustReleased(sf::Keyboard::Key key) const;

    void onKeyPressed(sf::Keyboard::Key key);
    void onKeyReleased(sf::Keyboard::Key key);

    // Convenience
    bool isJumpPressed() const;
    bool isCrouchPressed() const;
    bool isDashPressed() const;
    bool isPausePressed() const;

private:
    std::unordered_map<sf::Keyboard::Key, bool> currentKeys_;
    std::unordered_map<sf::Keyboard::Key, bool> previousKeys_;
};

#endif
