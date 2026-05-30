#ifndef SYSTEMS_RENDER_H
#define SYSTEMS_RENDER_H

#include "../ecs/Entity.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

class AssetManager;

class RenderSystem : public System {
public:
    explicit RenderSystem(sf::RenderWindow& window);

    void update(float deltaTime, std::vector<Entity*>& entities) override;
    void setView(const sf::View& view);
    void setAssetManager(AssetManager* mgr);

private:
    sf::RenderWindow& window_;
    sf::View view_;
    AssetManager* assetManager_ = nullptr;
};

#endif
