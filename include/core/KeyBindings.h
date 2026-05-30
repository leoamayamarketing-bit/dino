#ifndef CORE_KEYBINDINGS_H
#define CORE_KEYBINDINGS_H

#include <SFML/Window/Keyboard.hpp>
#include <string>
#include <unordered_map>
#include <vector>

/// Maps action names to SFML key codes for fully configurable controls.
struct KeyBindings {
    // -- Actions with just-pressed semantics (trigger on initial press) --
    sf::Keyboard::Key jump       = sf::Keyboard::Space;
    sf::Keyboard::Key jumpAlt1   = sf::Keyboard::Up;
    sf::Keyboard::Key jumpAlt2   = sf::Keyboard::W;
    sf::Keyboard::Key dash       = sf::Keyboard::LShift;
    sf::Keyboard::Key dashAlt    = sf::Keyboard::RShift;
    sf::Keyboard::Key pause      = sf::Keyboard::Escape;
    sf::Keyboard::Key pauseAlt   = sf::Keyboard::P;
    sf::Keyboard::Key menuSelect = sf::Keyboard::Space;
    sf::Keyboard::Key menuSelectAlt = sf::Keyboard::Enter;
    sf::Keyboard::Key menuBack   = sf::Keyboard::Escape;

    // -- Actions with held semantics (continuous while down) --
    sf::Keyboard::Key crouch    = sf::Keyboard::Down;
    sf::Keyboard::Key menuUp    = sf::Keyboard::Up;
    sf::Keyboard::Key menuUpAlt = sf::Keyboard::W;
    sf::Keyboard::Key menuDown  = sf::Keyboard::Down;
    sf::Keyboard::Key menuDownAlt = sf::Keyboard::S;

    /// Reset all bindings to defaults.
    void setDefaults();

    /// Save bindings to a text file. Returns true on success.
    bool saveToFile(const std::string& filePath) const;

    /// Load bindings from a text file. Returns true on success.
    bool loadFromFile(const std::string& filePath);

    /// Human-readable name for a key (e.g. sf::Keyboard::Space → "Space").
    static std::string keyName(sf::Keyboard::Key key);

    /// Map of action name → key pointer (for remapping UI iteration).
    /// Each action shows its primary key for remapping.
    struct ActionEntry {
        std::string displayName;       // "Jump"
        sf::Keyboard::Key* primaryKey; // &jump
        sf::Keyboard::Key* altKey;     // &jumpAlt1 (can be nullptr)
        sf::Keyboard::Key* altKey2;    // &jumpAlt2 (can be nullptr)
    };

    /// Returns all remappable actions for the UI.
    void getActions(std::vector<ActionEntry>& out) const;

private:
    // Helper: write one binding line
    static bool writeBinding(std::ofstream& f, const std::string& name, sf::Keyboard::Key key);

    // Helper: read one binding line
    static bool readBinding(const std::string& line, const std::string& name, sf::Keyboard::Key& key);
};

#endif
