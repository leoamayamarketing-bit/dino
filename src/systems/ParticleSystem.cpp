#include "../../include/systems/ParticleSystem.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <cmath>
#include <cstdlib>

ParticleSystem::ParticleSystem() {
    vertexArray_.setPrimitiveType(sf::Points);
}

void ParticleSystem::update(float, std::vector<Entity*>&) {
    // Particles updated separately via updateParticles
}

void ParticleSystem::render(sf::RenderWindow& window) {
    if (particles_.empty()) return;

    vertexArray_.clear();
    for (const auto& p : particles_) {
        if (!p.active) continue;
        float alpha = (p.lifetime / p.maxLifetime) * 255.0f;
        sf::Color c = p.color;
        c.a = static_cast<sf::Uint8>(alpha);

        sf::Vertex v(p.position, c);
        vertexArray_.append(v);
    }

    if (texture_) {
        sf::RenderStates states;
        states.texture = texture_;
        window.draw(vertexArray_, states);
    } else {
        window.draw(vertexArray_);
    }
}

void ParticleSystem::emit(sf::Vector2f position, int count,
                           sf::Color color, float speed, float lifetime, float size) {
    for (int i = 0; i < count; i++) {
        Particle p;
        p.position = position;
        float angle = (std::rand() % 360) * 3.14159f / 180.0f;
        float spd = (std::rand() % static_cast<int>(speed)) + speed * 0.3f;
        p.velocity = sf::Vector2f(std::cos(angle) * spd, std::sin(angle) * spd);
        p.lifetime = lifetime;
        p.maxLifetime = lifetime;
        p.color = color;
        p.size = size;
        p.active = true;
        particles_.push_back(p);
    }
}

void ParticleSystem::updateParticles(float dt) {
    for (auto& p : particles_) {
        if (!p.active) continue;
        p.position += p.velocity * dt;
        p.velocity.y += 100.0f * dt; // gravity on particles
        p.lifetime -= dt;
        if (p.lifetime <= 0.0f) {
            p.active = false;
        }
    }

    // Remove dead particles
    particles_.erase(
        std::remove_if(particles_.begin(), particles_.end(),
            [](const Particle& p) { return !p.active; }),
        particles_.end());
}
