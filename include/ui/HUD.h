#ifndef UI_HUD_H
#define UI_HUD_H

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include "../core/GameState.h"

class HUD {
public:
    HUD();

    void init(sf::Font& font);
    void update(const GameState& state);
    void render(sf::RenderWindow& window);

private:
    sf::Text scoreText_;
    sf::Text distanceText_;
    sf::Text speedText_;
    sf::Text livesText_;
    sf::Text coinsText_;
    sf::Text powerupText_;
    sf::Text shieldIndicator_;

    struct PowerUpIndicator {
        sf::Text text;
        float timer = 0.0f;
        bool active = false;
    };
    PowerUpIndicator shieldInd_;
    PowerUpIndicator magnetInd_;
    PowerUpIndicator doublePointsInd_;
    PowerUpIndicator speedInd_;

    sf::RectangleShape healthBarBg_;
    sf::RectangleShape healthBarFill_;

    void updateHealthBar(int health, int maxHealth);
};

#endif
