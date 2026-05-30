#ifndef SYSTEMS_RENDER_H
#define SYSTEMS_RENDER_H

#include "../ecs/Entity.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

class RenderSystem : public System {
public:
    explicit RenderSystem(sf::RenderWindow& window);

    void update(float deltaTime, std::vector<Entity*>& entities) override;
    void setView(const sf::View& view);

private:
    sf::RenderWindow& window_;
    sf::View view_;
};

#endif
