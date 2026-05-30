#include "../../include/core/InputManager.h"

InputManager::InputManager() = default;

void InputManager::update() {
    previousKeys_ = currentKeys_;
}

bool InputManager::isKeyPressed(sf::Keyboard::Key key) const {
    auto it = currentKeys_.find(key);
    return it != currentKeys_.end() && it->second;
}

bool InputManager::isKeyJustPressed(sf::Keyboard::Key key) const {
    bool current = false;
    auto curIt = currentKeys_.find(key);
    if (curIt != currentKeys_.end()) current = curIt->second;

    bool previous = false;
    auto prevIt = previousKeys_.find(key);
    if (prevIt != previousKeys_.end()) previous = prevIt->second;

    return current && !previous;
}

bool InputManager::isKeyJustReleased(sf::Keyboard::Key key) const {
    bool current = false;
    auto curIt = currentKeys_.find(key);
    if (curIt != currentKeys_.end()) current = curIt->second;

    bool previous = false;
    auto prevIt = previousKeys_.find(key);
    if (prevIt != previousKeys_.end()) previous = prevIt->second;

    return !current && previous;
}

void InputManager::onKeyPressed(sf::Keyboard::Key key) {
    currentKeys_[key] = true;
}

void InputManager::onKeyReleased(sf::Keyboard::Key key) {
    currentKeys_[key] = false;
}

// -----------------------------------------------------------------------
// Internal helper: check if any of up to 3 alternative keys match
// -----------------------------------------------------------------------
bool InputManager::matchesAny(const sf::Keyboard::Key* a,
                              const sf::Keyboard::Key* b,
                              const sf::Keyboard::Key* c,
                              bool justPressed) const {
    if (!bindings_) return false;
    auto test = justPressed ? &InputManager::isKeyJustPressed
                            : &InputManager::isKeyPressed;
    if (a && (this->*test)(*a)) return true;
    if (b && (this->*test)(*b)) return true;
    if (c && (this->*test)(*c)) return true;
    return false;
}

// -----------------------------------------------------------------------
// Action queries
// -----------------------------------------------------------------------
bool InputManager::isJumpPressed() const {
    if (!bindings_) {
        // Fallback defaults
        return isKeyJustPressed(sf::Keyboard::Space) ||
               isKeyJustPressed(sf::Keyboard::Up) ||
               isKeyJustPressed(sf::Keyboard::W);
    }
    return matchesAny(&bindings_->jump, &bindings_->jumpAlt1, &bindings_->jumpAlt2, true);
}

bool InputManager::isCrouchPressed() const {
    if (!bindings_) return isKeyPressed(sf::Keyboard::Down);
    return isKeyPressed(bindings_->crouch);
}

bool InputManager::isDashPressed() const {
    if (!bindings_) {
        return isKeyJustPressed(sf::Keyboard::LShift) ||
               isKeyJustPressed(sf::Keyboard::RShift);
    }
    return matchesAny(&bindings_->dash, &bindings_->dashAlt, nullptr, true);
}

bool InputManager::isPausePressed() const {
    if (!bindings_) {
        return isKeyJustPressed(sf::Keyboard::Escape) ||
               isKeyJustPressed(sf::Keyboard::P);
    }
    return matchesAny(&bindings_->pause, &bindings_->pauseAlt, nullptr, true);
}

// -----------------------------------------------------------------------
// Menu action queries
// -----------------------------------------------------------------------
bool InputManager::isMenuUpPressed() const {
    if (!bindings_) {
        return isKeyJustPressed(sf::Keyboard::Up) ||
               isKeyJustPressed(sf::Keyboard::W);
    }
    return matchesAny(&bindings_->menuUp, &bindings_->menuUpAlt, nullptr, true);
}

bool InputManager::isMenuDownPressed() const {
    if (!bindings_) {
        return isKeyJustPressed(sf::Keyboard::Down) ||
               isKeyJustPressed(sf::Keyboard::S);
    }
    return matchesAny(&bindings_->menuDown, &bindings_->menuDownAlt, nullptr, true);
}

bool InputManager::isMenuSelectPressed() const {
    if (!bindings_) {
        return isKeyJustPressed(sf::Keyboard::Space) ||
               isKeyJustPressed(sf::Keyboard::Enter);
    }
    return matchesAny(&bindings_->menuSelect, &bindings_->menuSelectAlt, nullptr, true);
}

bool InputManager::isMenuBackPressed() const {
    if (!bindings_) return isKeyJustPressed(sf::Keyboard::Escape);
    return isKeyJustPressed(bindings_->menuBack);
}
