#include "../../include/core/AssetManager.h"
#include "../../include/core/Constants.h"
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>

AssetManager::AssetManager() = default;

sf::Texture& AssetManager::getTexture(const std::string& name) {
    return textures_[name];
}

sf::Font& AssetManager::getFont(const std::string& name) {
    return fonts_[name];
}

bool AssetManager::hasTexture(const std::string& name) const {
    return textures_.find(name) != textures_.end();
}

bool AssetManager::hasFont(const std::string& name) const {
    return fonts_.find(name) != fonts_.end();
}

sf::Texture AssetManager::createRectTexture(int w, int h, sf::Color color) {
    sf::Image img;
    img.create(w, h, color);
    sf::Texture tex;
    tex.loadFromImage(img);
    return tex;
}

sf::Texture AssetManager::createCircleTexture(int radius, sf::Color color) {
    sf::RenderTexture rt;
    rt.create(radius * 2, radius * 2);
    rt.clear(sf::Color::Transparent);
    sf::CircleShape circle(radius);
    circle.setFillColor(color);
    rt.draw(circle);
    rt.display();
    return rt.getTexture();
}

sf::Texture AssetManager::createDinoSpriteSheet(const std::string& type, int frameW, int frameH) {
    // Create a sprite sheet: 4 poses side-by-side (run1, run2, jump, crouch)
    sf::RenderTexture rt;
    int totalW = frameW * 4;
    rt.create(totalW, frameH);
    rt.clear(sf::Color::Transparent);

    sf::Color bodyColor;
    if (type == "trex") bodyColor = sf::Color(80, 180, 80);
    else if (type == "raptor") bodyColor = sf::Color(180, 130, 50);
    else if (type == "triceratops") bodyColor = sf::Color(100, 150, 200);
    else bodyColor = sf::Color(80, 180, 80);

    float s = frameW / 128.0f;
    float cx = frameW * 0.45f;
    float cy = frameH * 0.65f;

    sf::Color darkColor(
        std::max(0, static_cast<int>(bodyColor.r) - 40),
        std::max(0, static_cast<int>(bodyColor.g) - 40),
        std::max(0, static_cast<int>(bodyColor.b) - 40)
    );
    sf::Color lightColor(
        std::min(255, static_cast<int>(bodyColor.r) + 30),
        std::min(255, static_cast<int>(bodyColor.g) + 30),
        std::min(255, static_cast<int>(bodyColor.b) + 30)
    );

    // Leg offsets for each pose: run1, run2, jump, crouch
    float legOffsets[4][2] = {
        { 8.0f, -6.0f },  // run1: right leg forward
        { -6.0f, 8.0f },  // run2: left leg forward
        { 2.0f, 2.0f },   // jump: legs together/tucked
        { 0.0f, 0.0f }    // crouch: legs bent
    };
    float bodySquash[4] = { 1.0f, 1.0f, 0.85f, 0.6f };
    float headTilt[4] = { 0.0f, 0.0f, 2.0f, -3.0f };
    for (int p = 0; p < 4; p++) {
        float bx = p * frameW;
        float headS = 1.0f + headTilt[p] * 0.01f;
        float bS = bodySquash[p];
        float lo1 = legOffsets[p][0];
        float lo2 = legOffsets[p][1];
        // Tail
        sf::ConvexShape tail;
        tail.setPointCount(5);
        tail.setPoint(0, sf::Vector2f(bx + cx - 28*s*bS, cy - 22*s));
        tail.setPoint(1, sf::Vector2f(bx + cx - 48*s*bS, cy - 8*s));
        tail.setPoint(2, sf::Vector2f(bx + cx - 38*s*bS, cy - 12*s));
        tail.setPoint(3, sf::Vector2f(bx + cx - 25*s*bS, cy - 18*s));
        tail.setPoint(4, sf::Vector2f(bx + cx - 20*s*bS, cy - 20*s));
        tail.setFillColor(darkColor);
        rt.draw(tail);

        // Body
        sf::ConvexShape body;
        body.setPointCount(8);
        float bw = 32 * s * bS;
        float bh = 28 * s;
        float bx2 = bx + cx - bw * 0.35f;
        float by = cy - bh - 2*s * bS;
        body.setPoint(0, sf::Vector2f(bx2, by));
        body.setPoint(1, sf::Vector2f(bx2 + bw*0.7f, by - 2*s*headS));
        body.setPoint(2, sf::Vector2f(bx2 + bw*1.0f, by + 4*s*headS));
        body.setPoint(3, sf::Vector2f(bx2 + bw*1.1f, by + bh*0.4f));
        body.setPoint(4, sf::Vector2f(bx2 + bw*1.0f, by + bh*0.7f));
        body.setPoint(5, sf::Vector2f(bx2 + bw*0.8f, by + bh*0.9f));
        body.setPoint(6, sf::Vector2f(bx2 + bw*0.3f, by + bh*1.0f));
        body.setPoint(7, sf::Vector2f(bx2 - bw*0.1f, by + bh*0.6f));
        body.setFillColor(bodyColor);
        body.setOutlineColor(darkColor);
        body.setOutlineThickness(0.5f);
        rt.draw(body);

        // Head
        sf::ConvexShape head;
        head.setPointCount(6);
        float hx = bx2 + bw * 0.85f;
        float hy = by - 3*s * headS;
        float hw = 18 * s * headS;
        float hh = 16 * s;
        head.setPoint(0, sf::Vector2f(hx, hy));
        head.setPoint(1, sf::Vector2f(hx + hw, hy));
        head.setPoint(2, sf::Vector2f(hx + hw*1.2f, hy + hh*0.3f));
        head.setPoint(3, sf::Vector2f(hx + hw*1.15f, hy + hh*0.7f));
        head.setPoint(4, sf::Vector2f(hx + hw*0.8f, hy + hh*1.0f));
        head.setPoint(5, sf::Vector2f(hx, hy + hh*0.8f));
        head.setFillColor(bodyColor);
        head.setOutlineColor(darkColor);
        head.setOutlineThickness(0.5f);
        rt.draw(head);

        // Jaw
        sf::ConvexShape jaw;
        jaw.setPointCount(4);
        jaw.setPoint(0, sf::Vector2f(hx + hw*0.2f, hy + hh*0.65f));
        jaw.setPoint(1, sf::Vector2f(hx + hw*1.0f, hy + hh*0.6f));
        jaw.setPoint(2, sf::Vector2f(hx + hw*1.1f, hy + hh*0.9f));
        jaw.setPoint(3, sf::Vector2f(hx + hw*0.3f, hy + hh*0.95f));
        jaw.setFillColor(darkColor);
        rt.draw(jaw);

        // Eye
        sf::CircleShape eye(2.5f*s);
        eye.setFillColor(sf::Color::White);
        eye.setPosition(bx + hx + hw*0.6f - 2.5f*s, hy + hh*0.25f - 2.5f*s);
        rt.draw(eye);
        sf::CircleShape pupil(1.5f*s);
        pupil.setFillColor(sf::Color::Black);
        pupil.setPosition(bx + hx + hw*0.6f - 1.5f*s, hy + hh*0.25f - 1.5f*s);
        rt.draw(pupil);

        // Legs
        float legY = cy;
        float legH = 18 * s * bS;
        float legW = 4 * s;
        float lx1 = bx2 + bw * 0.3f + lo1 * s * 0.5f;
        float lx2 = bx2 + bw * 0.6f + lo2 * s * 0.5f;

        sf::ConvexShape leg1;
        leg1.setPointCount(4);
        leg1.setPoint(0, sf::Vector2f(lx1 - legW*0.5f, legY - legH));
        leg1.setPoint(1, sf::Vector2f(lx1 + legW*0.5f, legY - legH*0.9f));
        leg1.setPoint(2, sf::Vector2f(lx1 + legW*0.8f, legY));
        leg1.setPoint(3, sf::Vector2f(lx1 - legW*0.3f, legY));
        leg1.setFillColor(darkColor);
        rt.draw(leg1);

        sf::ConvexShape leg2;
        leg2.setPointCount(4);
        leg2.setPoint(0, sf::Vector2f(lx2 - legW*0.5f, legY - legH*0.95f));
        leg2.setPoint(1, sf::Vector2f(lx2 + legW*0.5f, legY - legH*0.85f));
        leg2.setPoint(2, sf::Vector2f(lx2 + legW*0.8f, legY));
        leg2.setPoint(3, sf::Vector2f(lx2 - legW*0.3f, legY));
        leg2.setFillColor(bodyColor);
        rt.draw(leg2);

        // Belly highlight
        sf::ConvexShape belly;
        belly.setPointCount(5);
        belly.setPoint(0, sf::Vector2f(bx2 + bw*0.2f, by + bh*0.6f));
        belly.setPoint(1, sf::Vector2f(bx2 + bw*0.5f, by + bh*0.55f));
        belly.setPoint(2, sf::Vector2f(bx2 + bw*0.8f, by + bh*0.6f));
        belly.setPoint(3, sf::Vector2f(bx2 + bw*0.7f, by + bh*0.85f));
        belly.setPoint(4, sf::Vector2f(bx2 + bw*0.3f, by + bh*0.85f));
        belly.setFillColor(lightColor);
        rt.draw(belly);
    }

    rt.display();
    return rt.getTexture();
}

sf::Texture AssetManager::createCactusSpriteSheet(bool small) {
    // Sprite sheet: 2 frames (sway left, sway right)
    int fw = small ? 30 : 45;
    int fh = small ? 50 : 70;
    sf::RenderTexture rt;
    rt.create(fw * 2, fh);
    rt.clear(sf::Color::Transparent);

    sf::Color green(50, 140, 50);
    sf::Color darkGreen(38, 100, 38);
    float w = small ? 10.0f : 14.0f;
    float h = small ? 40.0f : 60.0f;

    for (int frame = 0; frame < 2; frame++) {
        float bx = frame * fw;
        float lean = (frame == 0) ? -1.5f : 1.5f;  // sway offset

        // Main trunk
        sf::RectangleShape trunk(sf::Vector2f(w, h));
        trunk.setFillColor(green);
        trunk.setPosition(bx + (small ? 10 : 15) + lean, 0);
        rt.draw(trunk);

        // Left arms
        sf::RectangleShape arm1(sf::Vector2f(h * 0.3f, w * 0.6f));
        arm1.setFillColor(green);
        arm1.setPosition(bx + (small ? 0 : 5) + lean * 0.5f, small ? 15 : 20);
        rt.draw(arm1);

        // Right arms
        sf::RectangleShape arm2(sf::Vector2f(h * 0.25f, w * 0.6f));
        arm2.setFillColor(green);
        arm2.setPosition(bx + (small ? 20 : 30) + lean * 0.5f, small ? 10 : 15);
        rt.draw(arm2);

        // Trunk details (vertical lines)
        sf::RectangleShape detail(sf::Vector2f(1.5f, h * 0.4f));
        detail.setFillColor(darkGreen);
        detail.setPosition(bx + (small ? 13 : 19) + lean, h * 0.1f);
        rt.draw(detail);
        sf::RectangleShape detail2(sf::Vector2f(1.5f, h * 0.35f));
        detail2.setFillColor(darkGreen);
        detail2.setPosition(bx + (small ? 16 : 24) + lean, h * 0.15f);
        rt.draw(detail2);
    }

    rt.display();
    return rt.getTexture();
}

sf::Texture AssetManager::createPterodactylSpriteSheet() {
    // Sprite sheet: 2 frames (wings up, wings down), each 60x40
    sf::RenderTexture rt;
    rt.create(120, 40);
    rt.clear(sf::Color::Transparent);

    sf::Color brown(160, 100, 60);
    sf::Color darkBrown(110, 70, 40);
    sf::Color beakColor(200, 150, 50);

    for (int frame = 0; frame < 2; frame++) {
        float bx = frame * 60.0f;
        bool wingsUp = (frame == 0);

        // Body (ellipse)
        sf::CircleShape body(14);
        body.setScale(1.4f, 0.8f);
        body.setFillColor(brown);
        body.setPosition(bx + 8, 12);
        rt.draw(body);

        // Wing
        sf::ConvexShape wing;
        if (wingsUp) {
            wing.setPointCount(4);
            wing.setPoint(0, sf::Vector2f(bx + 15, 18));
            wing.setPoint(1, sf::Vector2f(bx + 22, 2));
            wing.setPoint(2, sf::Vector2f(bx + 50, 8));
            wing.setPoint(3, sf::Vector2f(bx + 42, 20));
        } else {
            wing.setPointCount(4);
            wing.setPoint(0, sf::Vector2f(bx + 15, 18));
            wing.setPoint(1, sf::Vector2f(bx + 20, 28));
            wing.setPoint(2, sf::Vector2f(bx + 48, 32));
            wing.setPoint(3, sf::Vector2f(bx + 40, 22));
        }
        wing.setFillColor(brown);
        wing.setOutlineColor(darkBrown);
        wing.setOutlineThickness(0.5f);
        rt.draw(wing);

        // Wing membrane (translucent)
        sf::ConvexShape membrane;
        if (wingsUp) {
            membrane.setPointCount(4);
            membrane.setPoint(0, sf::Vector2f(bx + 20, 14));
            membrane.setPoint(1, sf::Vector2f(bx + 45, 10));
            membrane.setPoint(2, sf::Vector2f(bx + 30, 16));
            membrane.setPoint(3, sf::Vector2f(bx + 18, 10));
        } else {
            membrane.setPointCount(4);
            membrane.setPoint(0, sf::Vector2f(bx + 18, 22));
            membrane.setPoint(1, sf::Vector2f(bx + 45, 28));
            membrane.setPoint(2, sf::Vector2f(bx + 30, 24));
            membrane.setPoint(3, sf::Vector2f(bx + 18, 20));
        }
        membrane.setFillColor(sf::Color(170, 115, 75, 140));
        rt.draw(membrane);

        // Head/neck
        sf::ConvexShape head;
        head.setPointCount(4);
        head.setPoint(0, sf::Vector2f(bx + 42, 10));
        head.setPoint(1, sf::Vector2f(bx + 56, 12));
        head.setPoint(2, sf::Vector2f(bx + 54, 20));
        head.setPoint(3, sf::Vector2f(bx + 40, 20));
        head.setFillColor(brown);
        rt.draw(head);

        // Beak
        sf::ConvexShape beak;
        beak.setPointCount(3);
        beak.setPoint(0, sf::Vector2f(bx + 54, 13));
        beak.setPoint(1, sf::Vector2f(bx + 59, 16));
        beak.setPoint(2, sf::Vector2f(bx + 54, 19));
        beak.setFillColor(beakColor);
        rt.draw(beak);

        // Eye
        sf::CircleShape eye(2);
        eye.setFillColor(sf::Color::White);
        eye.setPosition(bx + 44, 12);
        rt.draw(eye);
        sf::CircleShape pupil(1);
        pupil.setFillColor(sf::Color::Black);
        pupil.setPosition(bx + 45, 13);
        rt.draw(pupil);

        // Tail feathers
        sf::ConvexShape tail;
        tail.setPointCount(3);
        tail.setPoint(0, sf::Vector2f(bx + 8, 16));
        tail.setPoint(1, sf::Vector2f(bx, 22));
        tail.setPoint(2, sf::Vector2f(bx + 6, 24));
        tail.setFillColor(darkBrown);
        rt.draw(tail);
    }

    rt.display();
    return rt.getTexture();
}

sf::Texture AssetManager::createRockTexture() {
    sf::RenderTexture rt;
    rt.create(50, 40);
    rt.clear(sf::Color::Transparent);

    sf::ConvexShape rock;
    rock.setPointCount(8);
    rock.setPoint(0, sf::Vector2f(5, 35));
    rock.setPoint(1, sf::Vector2f(2, 20));
    rock.setPoint(2, sf::Vector2f(8, 8));
    rock.setPoint(3, sf::Vector2f(20, 3));
    rock.setPoint(4, sf::Vector2f(35, 5));
    rock.setPoint(5, sf::Vector2f(45, 15));
    rock.setPoint(6, sf::Vector2f(48, 28));
    rock.setPoint(7, sf::Vector2f(42, 35));
    rock.setFillColor(sf::Color(120, 110, 100));
    rt.draw(rock);

    sf::ConvexShape highlight;
    highlight.setPointCount(4);
    highlight.setPoint(0, sf::Vector2f(15, 12));
    highlight.setPoint(1, sf::Vector2f(25, 8));
    highlight.setPoint(2, sf::Vector2f(30, 15));
    highlight.setPoint(3, sf::Vector2f(20, 18));
    highlight.setFillColor(sf::Color(160, 150, 140));
    rt.draw(highlight);

    rt.display();
    return rt.getTexture();
}

sf::Texture AssetManager::createCoinTexture() {
    sf::RenderTexture rt;
    rt.create(24, 24);
    rt.clear(sf::Color::Transparent);

    sf::CircleShape coin(10);
    coin.setFillColor(sf::Color(255, 215, 0));
    coin.setOutlineColor(sf::Color(200, 170, 0));
    coin.setOutlineThickness(2);
    coin.setPosition(2, 2);
    rt.draw(coin);

    sf::CircleShape inner(6);
    inner.setFillColor(sf::Color(255, 235, 100));
    inner.setPosition(6, 6);
    rt.draw(inner);

    sf::CircleShape symbol(3);
    symbol.setFillColor(sf::Color(200, 170, 0));
    symbol.setPosition(9, 9);
    rt.draw(symbol);

    rt.display();
    return rt.getTexture();
}

sf::Texture AssetManager::createPowerUpTexture(Constants::PowerUpType type) {
    sf::RenderTexture rt;
    rt.create(32, 32);
    rt.clear(sf::Color::Transparent);

    sf::Color color;
    switch (type) {
        case Constants::PowerUpType::SHIELD: color = sf::Color::Cyan; break;
        case Constants::PowerUpType::MAGNET: color = sf::Color::Magenta; break;
        case Constants::PowerUpType::DOUBLE_POINTS: color = sf::Color::Yellow; break;
        case Constants::PowerUpType::SPEED_BOOST: color = sf::Color::Green; break;
        case Constants::PowerUpType::EXTRA_LIFE: color = sf::Color::Red; break;
    }

    sf::CircleShape bg(14);
    bg.setFillColor(color);
    bg.setOutlineColor(sf::Color::White);
    bg.setOutlineThickness(2);
    bg.setPosition(2, 2);
    rt.draw(bg);

    // Inner glow
    sf::CircleShape inner(8);
    sf::Color light = color;
    light.a = 120;
    inner.setFillColor(light);
    inner.setPosition(8, 8);
    rt.draw(inner);

    rt.display();
    return rt.getTexture();
}

sf::Texture AssetManager::createParticleTexture() {
    return createCircleTexture(4, sf::Color::White);
}

sf::Texture AssetManager::createGroundTexture() {
    sf::RenderTexture rt;
    rt.create(128, 60);
    rt.clear(sf::Color::Transparent);

    sf::RectangleShape ground(sf::Vector2f(128, 25));
    ground.setFillColor(sf::Color(140, 110, 70));
    ground.setPosition(0, 30);
    rt.draw(ground);

    // Ground line
    sf::RectangleShape line(sf::Vector2f(128, 3));
    line.setFillColor(sf::Color(100, 80, 50));
    line.setPosition(0, 30);
    rt.draw(line);

    // Small rocks decoration
    for (int i = 0; i < 5; i++) {
        sf::CircleShape rock(1.5f + (std::rand() % 3));
        rock.setFillColor(sf::Color(160, 130, 90));
        rock.setPosition(10.0f + i * 25.0f, 45.0f);
        rt.draw(rock);
    }

    rt.display();
    return rt.getTexture();
}

sf::Texture AssetManager::createFishTexture() {
    // Create a simple fish: elliptical body + tail fin
    sf::RenderTexture rt;
    rt.create(50, 24);
    rt.clear(sf::Color::Transparent);

    sf::Color bodyColor(100, 160, 200);
    sf::Color darkColor(70, 120, 160);
    sf::Color finColor(80, 140, 180);

    // Tail fin
    sf::ConvexShape tail;
    tail.setPointCount(3);
    tail.setPoint(0, sf::Vector2f(2, 12));
    tail.setPoint(1, sf::Vector2f(2, 2));
    tail.setPoint(2, sf::Vector2f(-6, 7));
    tail.setFillColor(finColor);
    rt.draw(tail);

    // Body (ellipse)
    sf::CircleShape body(10);
    body.setScale(1.6f, 0.8f);
    body.setFillColor(bodyColor);
    body.setPosition(8, 4);
    rt.draw(body);

    // Belly highlight
    sf::CircleShape belly(7);
    belly.setScale(1.4f, 0.5f);
    belly.setFillColor(sf::Color(140, 200, 230));
    belly.setPosition(12, 10);
    rt.draw(belly);

    // Dorsal fin
    sf::ConvexShape dorsal;
    dorsal.setPointCount(3);
    dorsal.setPoint(0, sf::Vector2f(20, 3));
    dorsal.setPoint(1, sf::Vector2f(26, 3));
    dorsal.setPoint(2, sf::Vector2f(23, -2));
    dorsal.setFillColor(darkColor);
    rt.draw(dorsal);

    // Eye
    sf::CircleShape eye(2.5f);
    eye.setFillColor(sf::Color::White);
    eye.setPosition(30, 7);
    rt.draw(eye);
    sf::CircleShape pupil(1.5f);
    pupil.setFillColor(sf::Color::Black);
    pupil.setPosition(31, 8);
    rt.draw(pupil);

    // Mouth line
    sf::RectangleShape mouth(sf::Vector2f(5, 1));
    mouth.setFillColor(darkColor);
    mouth.setPosition(32, 11);
    rt.draw(mouth);

    rt.display();
    return rt.getTexture();
}

sf::Texture AssetManager::createCloudTexture() {
    sf::RenderTexture rt;
    rt.create(120, 50);
    rt.clear(sf::Color::Transparent);

    sf::CircleShape puff1(18);
    puff1.setFillColor(sf::Color(255, 255, 255, 180));
    puff1.setPosition(10, 10);
    rt.draw(puff1);

    sf::CircleShape puff2(14);
    puff2.setFillColor(sf::Color(255, 255, 255, 160));
    puff2.setPosition(35, 5);
    rt.draw(puff2);

    sf::CircleShape puff3(20);
    puff3.setFillColor(sf::Color(255, 255, 255, 170));
    puff3.setPosition(55, 12);
    rt.draw(puff3);

    sf::CircleShape puff4(12);
    puff4.setFillColor(sf::Color(255, 255, 255, 140));
    puff4.setPosition(80, 15);
    rt.draw(puff4);

    rt.display();
    return rt.getTexture();
}

void AssetManager::generateAllAssets() {
    generateDinoTextures();
    generateEnemyTextures();
    generateEnvironmentTextures();
    generateUITextures();
    generateFont();

    // Load PNG sprite animations from assets/ directory
    loadDinoAnimationPNGs();
    
    // Load gameplay PNG assets (ground, cloud, cactus, bird)
    // These replace the procedural textures
    loadGameplayPNGAssets();

    // Note: PNG files can be generated by the standalone tools/generate_textures.exe
    // Run it from the dino_game directory to re-generate all 88 textures
}

// ===========================================================================
// PNG sprite loading (cross-platform)
// ===========================================================================

std::string AssetManager::getAssetsPath() {
    // Try to find the assets directory relative to the executable
    // We try several paths to cover: running from project root, from build dir,
    // and from various subdirectories
    std::vector<std::filesystem::path> candidates = {
        std::filesystem::path("assets"),                    // from project root
        std::filesystem::path("dino_game") / "assets",     // from parent of dino_game
        std::filesystem::path("..") / "assets",            // from build/ dir
        std::filesystem::path("..") / "dino_game" / "assets",  // from build/ dir
        std::filesystem::path("..") / ".." / "assets",    // deeper nesting
    };

    for (const auto& candidate : candidates) {
        std::error_code ec;
        if (std::filesystem::exists(candidate, ec)) {
            return candidate.string();
        }
    }

    // Fallback: just return "assets"
    return "assets";
}

// Static helper: remove white/light background from a texture by setting
// near-white pixels to fully transparent. Used for PNGs with solid white backgrounds.
// Higher tolerance = more aggressive removal of near-white edge pixels.
static void makeBackgroundTransparent(sf::Texture& tex, int tolerance = 30) {
    sf::Image img = tex.copyToImage();
    const sf::Color keyColor(255, 255, 255);

    for (unsigned y = 0; y < img.getSize().y; ++y) {
        for (unsigned x = 0; x < img.getSize().x; ++x) {
            sf::Color c = img.getPixel(x, y);
            if (c.a < 255) continue;
            int dr = static_cast<int>(c.r) - static_cast<int>(keyColor.r);
            int dg = static_cast<int>(c.g) - static_cast<int>(keyColor.g);
            int db = static_cast<int>(c.b) - static_cast<int>(keyColor.b);
            if (dr * dr + dg * dg + db * db <= tolerance * tolerance) {
                c.a = 0;
                img.setPixel(x, y, c);
            }
        }
    }
    tex.loadFromImage(img);
}

// Replace white/near-white pixels with a solid fill color instead of transparent.
// Used for ground textures that need opaque edges for seamless tiling.
static void replaceWhiteWithColor(sf::Texture& tex, const sf::Color& fillColor) {
    sf::Image img = tex.copyToImage();
    const sf::Color keyColor(255, 255, 255);
    const int tolerance = 45;  // more aggressive for ground to avoid any white remnants

    for (unsigned y = 0; y < img.getSize().y; ++y) {
        for (unsigned x = 0; x < img.getSize().x; ++x) {
            sf::Color c = img.getPixel(x, y);
            if (c.a < 255) continue;
            int dr = static_cast<int>(c.r) - static_cast<int>(keyColor.r);
            int dg = static_cast<int>(c.g) - static_cast<int>(keyColor.g);
            int db = static_cast<int>(c.b) - static_cast<int>(keyColor.b);
            if (dr * dr + dg * dg + db * db <= tolerance * tolerance) {
                img.setPixel(x, y, fillColor);
            }
        }
    }

    // Force edge columns to the fill color so tiling copies are seamless.
    // The leftmost and rightmost pixel columns of piso.png (903px wide)
    // often have near-white residuals that create visible seams between
    // sprite copies when the parallax offsets cross the tile boundary.
    unsigned w = img.getSize().x;
    unsigned h = img.getSize().y;
    for (unsigned y = 0; y < h; ++y) {
        img.setPixel(0, y, fillColor);      // left edge
        img.setPixel(w - 1, y, fillColor);  // right edge
    }

    tex.loadFromImage(img);
}

bool AssetManager::loadPNGTexture(const std::string& name, const std::string& filename) {
    std::filesystem::path assetsPath(getAssetsPath());
    std::filesystem::path fullPath = assetsPath / filename;

    sf::Texture tex;
    if (!tex.loadFromFile(fullPath.string())) {
        std::cerr << "[AssetManager] Warning: Could not load PNG: "
                  << fullPath.string() << std::endl;
        return false;
    }

    textures_[name] = std::move(tex);
    std::cout << "[AssetManager] Loaded PNG: " << fullPath.string()
              << " -> texture '" << name << "'" << std::endl;
    return true;
}

void AssetManager::loadDinoAnimationPNGs() {
    // Load dino_move_XX.png files (walking/crouching animation frames)
    // Format: dino_move_00.png through dino_move_07.png, plus dino_08.png
    const std::string frameFiles[] = {
        "dino_move_00.png",
        "dino_move_01.png",
        "dino_move_02.png",
        "dino_move_03.png",
        "dino_move_04.png",
        "dino_move_05.png",
        "dino_move_06.png",
        "dino_move_07.png",
    };
    const std::string extraFiles[] = {
        "dino_08.png",
        "dino.png",
    };

    dinoFrameNames_.clear();

    // Load move frames (00-07)
    for (int i = 0; i < 8; ++i) {
        std::string texName = std::string("dino_frame_") + (i < 10 ? "0" : "") + std::to_string(i);
        if (loadPNGTexture(texName, frameFiles[i])) {
            makeBackgroundTransparent(textures_[texName]);
            dinoFrameNames_.push_back(texName);
        }
    }

    // Load extra frames
    loadPNGTexture("dino_frame_08", extraFiles[0]);
    if (hasTexture("dino_frame_08")) {
        makeBackgroundTransparent(textures_["dino_frame_08"]);
        dinoFrameNames_.push_back("dino_frame_08");
    }

    loadPNGTexture("dino_sprite", extraFiles[1]);
    if (hasTexture("dino_sprite")) {
        makeBackgroundTransparent(textures_["dino_sprite"]);
    }

    std::cout << "[AssetManager] Loaded " << dinoFrameNames_.size()
              << " dino animation PNG frames" << std::endl;
}

void AssetManager::loadGameplayPNGAssets() {
    // Load all gameplay PNG assets and replace procedural textures
    // These PNGs have white backgrounds that need alpha transparency
    
    struct PNGAsset {
        std::string texName;
        std::string filename;
        bool isGround;  // ground needs opaque fill instead of transparency for seamless tiling
    };
    
    const PNGAsset assets[] = {
        {"ground",        "piso.png",       true},
        {"cloud",         "nube.png",       false},
        {"cactus_small",  "un_cactus.png",  false},
        {"cactus_large",  "dos_cactus.png", false},
        {"pterodactyl",   "ave.png",        false},
    };
    
    for (const auto& a : assets) {
        if (loadPNGTexture(a.texName, a.filename)) {
            if (a.isGround) {
                // Replace white background with solid dirt color for seamless tiling.
                // This prevents 1-pixel translucent white seams between tiled copies.
                replaceWhiteWithColor(textures_[a.texName], sf::Color(100, 80, 50));
            } else {
                makeBackgroundTransparent(textures_[a.texName]);
            }
            std::cout << "[AssetManager] Loaded gameplay asset: "
                      << a.filename << " -> '" << a.texName << "'" << std::endl;
        }
    }
}

void AssetManager::exportTexturesToPNG(const std::string& outputDir) {
    // Create output directory
    std::filesystem::create_directories(outputDir);

    for (const auto& [name, texture] : textures_) {
        sf::Image img = texture.copyToImage();
        std::filesystem::path p = std::filesystem::path(outputDir) / (name + ".png");
        img.saveToFile(p.string());
    }
}



void AssetManager::generateDinoTextures() {
    // Generate sprite sheets: 4 animation frames (128x72 each) in one texture
    textures_["dino_trex"] = createDinoSpriteSheet("trex", 128, 72);
    textures_["dino_raptor"] = createDinoSpriteSheet("raptor", 128, 72);
    textures_["dino_triceratops"] = createDinoSpriteSheet("triceratops", 128, 72);
}

void AssetManager::generateEnemyTextures() {
    // Sprite sheets: 2 frames each (sway left/right for cactus, wings up/down for pterodactyl)
    textures_["cactus_small"] = createCactusSpriteSheet(true);
    textures_["cactus_large"] = createCactusSpriteSheet(false);
    textures_["pterodactyl"] = createPterodactylSpriteSheet();
    textures_["rock"] = createRockTexture();
    textures_["ground_enemy"] = createRectTexture(40, 30, sf::Color(180, 80, 50));
}

void AssetManager::generateEnvironmentTextures() {
    textures_["ground"] = createGroundTexture();
    textures_["cloud"] = createCloudTexture();
    textures_["particle"] = createParticleTexture();
    textures_["fish"] = createFishTexture();

    // Lava texture
    sf::Texture lavaTex = createRectTexture(64, 12, sf::Color(255, 80, 0));
    textures_["lava"] = lavaTex;

    // Stalactite texture
    sf::RenderTexture stalRt;
    stalRt.create(20, 50);
    stalRt.clear(sf::Color::Transparent);
    sf::ConvexShape stal;
    stal.setPointCount(3);
    stal.setPoint(0, sf::Vector2f(0, 0));
    stal.setPoint(1, sf::Vector2f(20, 0));
    stal.setPoint(2, sf::Vector2f(10, 50));
    stal.setFillColor(sf::Color(120, 100, 80));
    stalRt.draw(stal);
    stalRt.display();
    textures_["stalactite"] = stalRt.getTexture();
}

void AssetManager::generateUITextures() {
    textures_["coin"] = createCoinTexture();
    textures_["powerup_shield"] = createPowerUpTexture(Constants::PowerUpType::SHIELD);
    textures_["powerup_magnet"] = createPowerUpTexture(Constants::PowerUpType::MAGNET);
    textures_["powerup_double"] = createPowerUpTexture(Constants::PowerUpType::DOUBLE_POINTS);
    textures_["powerup_speed"] = createPowerUpTexture(Constants::PowerUpType::SPEED_BOOST);
    textures_["powerup_life"] = createPowerUpTexture(Constants::PowerUpType::EXTRA_LIFE);
}

void AssetManager::generateFont() {
    // Try to load a system font
    sf::Font font;
    std::vector<std::string> fontPaths = {
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/consola.ttf",
        "C:/Windows/Fonts/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/System/Library/Fonts/Helvetica.ttc"
    };

    for (const auto& path : fontPaths) {
        if (font.loadFromFile(path)) {
            break;
        }
    }

    fonts_["main"] = font;
}
