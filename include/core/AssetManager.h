#ifndef CORE_ASSETMANAGER_H
#define CORE_ASSETMANAGER_H

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Font.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include "Constants.h"

class AssetManager {
public:
    AssetManager();

    // Generate procedural textures
    sf::Texture& getTexture(const std::string& name);
    sf::Font& getFont(const std::string& name);

    bool hasTexture(const std::string& name) const;
    bool hasFont(const std::string& name) const;

    // Generate all game textures procedurally
    void generateAllAssets();

    // Export all generated textures to PNG files in resources/
    void exportTexturesToPNG(const std::string& outputDir = "resources");

    // ─── PNG sprite loading (cross-platform) ───────────────────────────────
    /// Load a single PNG texture from the assets/ directory.
    /// Uses OS-appropriate path separators automatically.
    bool loadPNGTexture(const std::string& name, const std::string& filename);

    /// Load all dino animation PNGs from assets/ into individual textures.
    /// Creates textures named "dino_frame_XX" for each loaded PNG.
    void loadDinoAnimationPNGs();

    /// Get the list of loaded dino animation frame texture names.
    const std::vector<std::string>& getDinoFrameNames() const { return dinoFrameNames_; }

    /// Load gameplay PNG assets (ground, cloud, cactus, bird) with alpha transparency.
    /// Replaces procedural textures with PNG-loaded ones.
    void loadGameplayPNGAssets();

    /// Returns the assets/ directory path using OS-native separators.
    static std::string getAssetsPath();

private:
    void generateDinoTextures();
    void generateEnemyTextures();
    void generateEnvironmentTextures();
    void generateUITextures();
    void generateFont();

    std::unordered_map<std::string, sf::Texture> textures_;
    std::unordered_map<std::string, sf::Font> fonts_;

    // Loaded PNG frame names (for dino animation)
    std::vector<std::string> dinoFrameNames_;

    sf::Texture createRectTexture(int w, int h, sf::Color color);
    sf::Texture createCircleTexture(int radius, sf::Color color);
    sf::Texture createDinoSpriteSheet(const std::string& type, int frameW, int frameH);
    sf::Texture createCactusSpriteSheet(bool small);
    sf::Texture createPterodactylSpriteSheet();
    sf::Texture createRockTexture();
    sf::Texture createCoinTexture();
    sf::Texture createPowerUpTexture(Constants::PowerUpType type);
    sf::Texture createParticleTexture();
    sf::Texture createGroundTexture();
    sf::Texture createCloudTexture();
    sf::Texture createStalactiteTexture();
    sf::Texture createLavaTexture();
    sf::Texture createHeartTexture();
    sf::Texture createFireflyTexture();
    sf::Texture createAshTexture();
};

#endif
