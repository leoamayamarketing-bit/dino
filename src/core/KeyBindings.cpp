#include "../../include/core/KeyBindings.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

// ===========================================================================
// Default bindings
// ===========================================================================
void KeyBindings::setDefaults() {
    jump          = sf::Keyboard::Space;
    jumpAlt1      = sf::Keyboard::Up;
    jumpAlt2      = sf::Keyboard::W;
    crouch        = sf::Keyboard::Down;
    dash          = sf::Keyboard::LShift;
    dashAlt       = sf::Keyboard::RShift;
    pause         = sf::Keyboard::Escape;
    pauseAlt      = sf::Keyboard::P;
    menuUp        = sf::Keyboard::Up;
    menuUpAlt     = sf::Keyboard::W;
    menuDown      = sf::Keyboard::Down;
    menuDownAlt   = sf::Keyboard::S;
    menuSelect    = sf::Keyboard::Space;
    menuSelectAlt = sf::Keyboard::Enter;
    menuBack      = sf::Keyboard::Escape;
}

// ===========================================================================
// Save / Load
// ===========================================================================
bool KeyBindings::writeBinding(std::ofstream& f, const std::string& name, sf::Keyboard::Key key) {
    f << name << "=" << static_cast<int>(key) << "\n";
    return f.good();
}

bool KeyBindings::readBinding(const std::string& line, const std::string& name, sf::Keyboard::Key& key) {
    if (line.compare(0, name.size(), name) != 0 || line[name.size()] != '=')
        return false;
    int val = 0;
    std::istringstream(line.substr(name.size() + 1)) >> val;
    if (val < 0 || val >= sf::Keyboard::KeyCount) return false;
    key = static_cast<sf::Keyboard::Key>(val);
    return true;
}

bool KeyBindings::saveToFile(const std::string& filePath) const {
    std::ofstream f(filePath);
    if (!f.is_open()) {
        std::cerr << "[KeyBindings] Could not write " << filePath << std::endl;
        return false;
    }
    f << "# DinoRunner Key Bindings\n";
    f << "# Format: action=keycode (0-255)\n\n";
    writeBinding(f, "jump",        jump);
    writeBinding(f, "jumpAlt1",    jumpAlt1);
    writeBinding(f, "jumpAlt2",    jumpAlt2);
    writeBinding(f, "crouch",      crouch);
    writeBinding(f, "dash",        dash);
    writeBinding(f, "dashAlt",     dashAlt);
    writeBinding(f, "pause",       pause);
    writeBinding(f, "pauseAlt",    pauseAlt);
    writeBinding(f, "menuUp",      menuUp);
    writeBinding(f, "menuUpAlt",   menuUpAlt);
    writeBinding(f, "menuDown",    menuDown);
    writeBinding(f, "menuDownAlt", menuDownAlt);
    writeBinding(f, "menuSelect",  menuSelect);
    writeBinding(f, "menuSelectAlt", menuSelectAlt);
    writeBinding(f, "menuBack",    menuBack);
    f.close();
    std::cout << "[KeyBindings] Saved to " << filePath << std::endl;
    return true;
}

bool KeyBindings::loadFromFile(const std::string& filePath) {
    std::ifstream f(filePath);
    if (!f.is_open()) {
        std::cout << "[KeyBindings] No config file at " << filePath << ", using defaults" << std::endl;
        return false;
    }
    std::string line;
    int loaded = 0;
    while (std::getline(f, line)) {
        // Strip comments and whitespace
        auto commentPos = line.find('#');
        if (commentPos != std::string::npos) line = line.substr(0, commentPos);
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t\r") + 1);
        if (line.empty()) continue;

        if      (readBinding(line, "jump",        jump))        loaded++;
        else if (readBinding(line, "jumpAlt1",    jumpAlt1))    loaded++;
        else if (readBinding(line, "jumpAlt2",    jumpAlt2))    loaded++;
        else if (readBinding(line, "crouch",      crouch))      loaded++;
        else if (readBinding(line, "dash",        dash))        loaded++;
        else if (readBinding(line, "dashAlt",     dashAlt))     loaded++;
        else if (readBinding(line, "pause",       pause))       loaded++;
        else if (readBinding(line, "pauseAlt",    pauseAlt))    loaded++;
        else if (readBinding(line, "menuUp",      menuUp))      loaded++;
        else if (readBinding(line, "menuUpAlt",   menuUpAlt))   loaded++;
        else if (readBinding(line, "menuDown",    menuDown))    loaded++;
        else if (readBinding(line, "menuDownAlt", menuDownAlt)) loaded++;
        else if (readBinding(line, "menuSelect",  menuSelect))  loaded++;
        else if (readBinding(line, "menuSelectAlt", menuSelectAlt)) loaded++;
        else if (readBinding(line, "menuBack",    menuBack))    loaded++;
    }
    std::cout << "[KeyBindings] Loaded " << loaded << " bindings from " << filePath << std::endl;
    return true;
}

// ===========================================================================
// Key name lookup
// ===========================================================================
std::string KeyBindings::keyName(sf::Keyboard::Key key) {
    // Map the most common keys to readable names; fallback to "Key N"
    switch (key) {
        case sf::Keyboard::A: return "A";
        case sf::Keyboard::B: return "B";
        case sf::Keyboard::C: return "C";
        case sf::Keyboard::D: return "D";
        case sf::Keyboard::E: return "E";
        case sf::Keyboard::F: return "F";
        case sf::Keyboard::G: return "G";
        case sf::Keyboard::H: return "H";
        case sf::Keyboard::I: return "I";
        case sf::Keyboard::J: return "J";
        case sf::Keyboard::K: return "K";
        case sf::Keyboard::L: return "L";
        case sf::Keyboard::M: return "M";
        case sf::Keyboard::N: return "N";
        case sf::Keyboard::O: return "O";
        case sf::Keyboard::P: return "P";
        case sf::Keyboard::Q: return "Q";
        case sf::Keyboard::R: return "R";
        case sf::Keyboard::S: return "S";
        case sf::Keyboard::T: return "T";
        case sf::Keyboard::U: return "U";
        case sf::Keyboard::V: return "V";
        case sf::Keyboard::W: return "W";
        case sf::Keyboard::X: return "X";
        case sf::Keyboard::Y: return "Y";
        case sf::Keyboard::Z: return "Z";
        case sf::Keyboard::Num0: return "0";
        case sf::Keyboard::Num1: return "1";
        case sf::Keyboard::Num2: return "2";
        case sf::Keyboard::Num3: return "3";
        case sf::Keyboard::Num4: return "4";
        case sf::Keyboard::Num5: return "5";
        case sf::Keyboard::Num6: return "6";
        case sf::Keyboard::Num7: return "7";
        case sf::Keyboard::Num8: return "8";
        case sf::Keyboard::Num9: return "9";
        case sf::Keyboard::Escape:    return "Escape";
        case sf::Keyboard::LControl:  return "L-Ctrl";
        case sf::Keyboard::LShift:    return "L-Shift";
        case sf::Keyboard::LAlt:      return "L-Alt";
        case sf::Keyboard::LSystem:   return "L-Win";
        case sf::Keyboard::RControl:  return "R-Ctrl";
        case sf::Keyboard::RShift:    return "R-Shift";
        case sf::Keyboard::RAlt:      return "R-Alt";
        case sf::Keyboard::RSystem:   return "R-Win";
        case sf::Keyboard::Menu:      return "Menu";
        case sf::Keyboard::LBracket:  return "[";
        case sf::Keyboard::RBracket:  return "]";
        case sf::Keyboard::SemiColon: return ";";
        case sf::Keyboard::Comma:     return ",";
        case sf::Keyboard::Period:    return ".";
        case sf::Keyboard::Quote:     return "'";
        case sf::Keyboard::Slash:     return "/";
        case sf::Keyboard::BackSlash: return "\\";
        case sf::Keyboard::Tilde:     return "~";
        case sf::Keyboard::Equal:     return "=";
        case sf::Keyboard::Dash:      return "-";
        case sf::Keyboard::Space:     return "Space";
        case sf::Keyboard::Enter:     return "Enter";
        case sf::Keyboard::BackSpace: return "Backspace";
        case sf::Keyboard::Tab:       return "Tab";
        case sf::Keyboard::PageUp:    return "PageUp";
        case sf::Keyboard::PageDown:  return "PageDown";
        case sf::Keyboard::End:       return "End";
        case sf::Keyboard::Home:      return "Home";
        case sf::Keyboard::Insert:    return "Insert";
        case sf::Keyboard::Delete:    return "Delete";
        case sf::Keyboard::Add:       return "Numpad+";
        case sf::Keyboard::Subtract:  return "Numpad-";
        case sf::Keyboard::Multiply:  return "Numpad*";
        case sf::Keyboard::Divide:    return "Numpad/";
        case sf::Keyboard::Left:      return "Left";
        case sf::Keyboard::Right:     return "Right";
        case sf::Keyboard::Up:        return "Up";
        case sf::Keyboard::Down:      return "Down";
        case sf::Keyboard::Numpad0:   return "Numpad0";
        case sf::Keyboard::Numpad1:   return "Numpad1";
        case sf::Keyboard::Numpad2:   return "Numpad2";
        case sf::Keyboard::Numpad3:   return "Numpad3";
        case sf::Keyboard::Numpad4:   return "Numpad4";
        case sf::Keyboard::Numpad5:   return "Numpad5";
        case sf::Keyboard::Numpad6:   return "Numpad6";
        case sf::Keyboard::Numpad7:   return "Numpad7";
        case sf::Keyboard::Numpad8:   return "Numpad8";
        case sf::Keyboard::Numpad9:   return "Numpad9";
        case sf::Keyboard::F1:  return "F1";
        case sf::Keyboard::F2:  return "F2";
        case sf::Keyboard::F3:  return "F3";
        case sf::Keyboard::F4:  return "F4";
        case sf::Keyboard::F5:  return "F5";
        case sf::Keyboard::F6:  return "F6";
        case sf::Keyboard::F7:  return "F7";
        case sf::Keyboard::F8:  return "F8";
        case sf::Keyboard::F9:  return "F9";
        case sf::Keyboard::F10: return "F10";
        case sf::Keyboard::F11: return "F11";
        case sf::Keyboard::F12: return "F12";
        case sf::Keyboard::F13: return "F13";
        case sf::Keyboard::F14: return "F14";
        case sf::Keyboard::F15: return "F15";
        default: {
            std::string fallback = "Key";
            fallback += std::to_string(static_cast<int>(key));
            return fallback;
        }
    }
}

// ===========================================================================
// Actions list for remapping UI
// ===========================================================================
void KeyBindings::getActions(std::vector<ActionEntry>& out) const {
    out.clear();
    out.push_back({"Jump",        const_cast<sf::Keyboard::Key*>(&jump),     const_cast<sf::Keyboard::Key*>(&jumpAlt1), const_cast<sf::Keyboard::Key*>(&jumpAlt2)});
    out.push_back({"Crouch",      const_cast<sf::Keyboard::Key*>(&crouch),   nullptr, nullptr});
    out.push_back({"Dash",        const_cast<sf::Keyboard::Key*>(&dash),     const_cast<sf::Keyboard::Key*>(&dashAlt),  nullptr});
    out.push_back({"Pause",       const_cast<sf::Keyboard::Key*>(&pause),    const_cast<sf::Keyboard::Key*>(&pauseAlt), nullptr});
    out.push_back({"Menu Up",     const_cast<sf::Keyboard::Key*>(&menuUp),   const_cast<sf::Keyboard::Key*>(&menuUpAlt), nullptr});
    out.push_back({"Menu Down",   const_cast<sf::Keyboard::Key*>(&menuDown), const_cast<sf::Keyboard::Key*>(&menuDownAlt), nullptr});
    out.push_back({"Menu Select", const_cast<sf::Keyboard::Key*>(&menuSelect), const_cast<sf::Keyboard::Key*>(&menuSelectAlt), nullptr});
    out.push_back({"Menu Back",   const_cast<sf::Keyboard::Key*>(&menuBack), nullptr, nullptr});
}
