#ifndef CORE_ACHIEVEMENT_H
#define CORE_ACHIEVEMENT_H

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <string>
#include <vector>
#include <functional>

struct GameState;

// Achievement IDs (bitmask flags matching GameState::achievements)
enum AchievementID : uint32_t {
    ACH_FIRST_BLOOD    = 1 << 0,  // Kill first enemy
    ACH_RICH           = 1 << 1,  // Collect 100 total coins
    ACH_UNSTOPPABLE    = 1 << 2,  // 10× combo
    ACH_MARATHON       = 1 << 3,  // Run 1000m
    ACH_SHIELD_MASTER  = 1 << 4,  // Collect 5 shields
    ACH_COMBO_KING     = 1 << 5,  // 20× combo
    ACH_MILLIONAIRE    = 1 << 6,  // Score 100,000 points
    ACH_SURVIVOR       = 1 << 7,  // Survive 5 minutes
    ACH_MAGNETIC       = 1 << 8,  // Collect 10 magnets
    ACH_SPEED_DEMON    = 1 << 9,  // Reach max speed (900)
};

struct AchievementDef {
    uint32_t id;
    const char* name;
    const char* description;
};

class AchievementSystem {
public:
    AchievementSystem();

    void init(sf::Font& font);
    void update(float deltaTime);
    void render(sf::RenderWindow& window);

    /// Check all achievements against current game state.
    /// Returns true if any new achievement was unlocked.
    bool checkAchievements(const GameState& state);

    /// Check if a specific achievement was already unlocked.
    bool isUnlocked(uint32_t achId) const { return (unlockedFlags_ & achId) != 0; }

    /// Get the achievement flags.
    uint32_t getUnlockedFlags() const { return unlockedFlags_; }

    /// Set achievement flags (for restoring from save).
    void setUnlockedFlags(uint32_t flags) { unlockedFlags_ = flags; }

    /// Save achievements to file.
    void saveToFile(const std::string& path = "achievements.dat");

    /// Load achievements from file. Returns loaded flags.
    uint32_t loadFromFile(const std::string& path = "achievements.dat");

    /// Callback for playing achievement sound (set by Game).
    std::function<void()> onAchievementUnlock;

    /// Get the achievement definitions list.
    static const std::vector<AchievementDef>& getDefinitions();

private:
    struct ToastNotification {
        std::string name;
        std::string description;
        sf::Text nameText;
        sf::Text descText;
        sf::RectangleShape bg;
        float lifetime = 0.0f;
        float maxLifetime = 3.5f;
        float slideOffset = 0.0f;
        bool active = true;
    };

    std::vector<ToastNotification> toasts_;
    uint32_t unlockedFlags_ = 0;
    float spawnTimer_ = 0.0f;

    sf::RectangleShape toastBg_;
    sf::Text toastNameText_;
    sf::Text toastDescText_;

    void spawnToast(const AchievementDef& def);
};

#endif
