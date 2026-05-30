#include <SFML/Graphics.hpp>
#include "../include/core/Game.h"

int main() {
    try {
        Game game;
        game.run();
    } catch (const std::exception& e) {
        sf::err() << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
