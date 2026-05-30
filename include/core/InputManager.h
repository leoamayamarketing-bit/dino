#ifndef CORE_INPUTMANAGER_H
#define CORE_INPUTMANAGER_H

#include <SFML/Window/Keyboard.hpp>
#include <unordered_map>
#include "KeyBindings.h"

class InputManager {
public:
    InputManager();

    void update();
    bool isKeyPressed(sf::Keyboard::Key key) const;
    bool isKeyJustPressed(sf::Keyboard::Key key) const;
    bool isKeyJustReleased(sf::Keyboard::Key key) const;

    void onKeyPressed(sf::Keyboard::Key key);
    void onKeyReleased(sf::Keyboard::Key key);

    /// Bind a keybindings config for action lookups.
    void setBindings(const KeyBindings* bindings) { bindings_ = bindings; }

    // --- Action queries (use configurable bindings) ---
    bool isJumpPressed() const;
    bool isCrouchPressed() const;
    bool isDashPressed() const;
    bool isPausePressed() const;

    // --- Menu action queries (for menu navigation) ---
    bool isMenuUpPressed() const;
    bool isMenuDownPressed() const;
    bool isMenuSelectPressed() const;
    bool isMenuBackPressed() const;

private:
    bool matchesAny(const sf::Keyboard::Key* a,
                    const sf::Keyboard::Key* b,
                    const sf::Keyboard::Key* c,
                    bool justPressed) const;

    std::unordered_map<sf::Keyboard::Key, bool> currentKeys_;
    std::unordered_map<sf::Keyboard::Key, bool> previousKeys_;
    const KeyBindings* bindings_ = nullptr;
};

#endif
