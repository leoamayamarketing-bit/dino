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

bool InputManager::isJumpPressed() const {
    return isKeyJustPressed(sf::Keyboard::Space) ||
           isKeyJustPressed(sf::Keyboard::W) ||
           isKeyJustPressed(sf::Keyboard::Up);
}

bool InputManager::isCrouchPressed() const {
    return isKeyPressed(sf::Keyboard::Down);
}

bool InputManager::isDashPressed() const {
    return isKeyJustPressed(sf::Keyboard::LShift) ||
           isKeyJustPressed(sf::Keyboard::RShift);
}

bool InputManager::isPausePressed() const {
    return isKeyJustPressed(sf::Keyboard::Escape) ||
           isKeyJustPressed(sf::Keyboard::P);
}
