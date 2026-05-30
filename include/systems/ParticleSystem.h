#ifndef SYSTEMS_PARTICLE_H
#define SYSTEMS_PARTICLE_H

#include "../ecs/Entity.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Color.hpp>
#include <vector>

struct Particle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float lifetime;
    float maxLifetime;
    sf::Color color;
    float size;
    bool active = true;
};

class ParticleSystem : public System {
public:
    ParticleSystem();

    void update(float deltaTime, std::vector<Entity*>& entities) override;
    void render(sf::RenderWindow& window);

    void emit(sf::Vector2f position, int count = 5,
              sf::Color color = sf::Color::White,
              float speed = 100.0f, float lifetime = 1.0f,
              float size = 4.0f);

    void setTexture(sf::Texture* tex) { texture_ = tex; }

    void updateParticles(float dt);

private:
    std::vector<Particle> particles_;
    sf::Texture* texture_ = nullptr;
    sf::VertexArray vertexArray_;
};

#endif
