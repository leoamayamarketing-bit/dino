#include "../../include/core/AchievementSystem.h"
#include "../../include/core/GameState.h"
#include "../../include/core/Constants.h"
#include <cmath>
#include <algorithm>
#include <fstream>
#include <iostream>

static const std::vector<AchievementDef> s_definitions = {
    {ACH_FIRST_BLOOD,   "First Blood",    "Destroy your first enemy"},
    {ACH_RICH,          "Rich!",           "Collect 100 coins"},
    {ACH_UNSTOPPABLE,   "Unstoppable",     "Reach a 10× combo"},
    {ACH_MARATHON,      "Marathon",        "Run 1000 meters"},
    {ACH_SHIELD_MASTER, "Shield Master",   "Collect 5 shields"},
    {ACH_COMBO_KING,    "Combo King",      "Reach a 20× combo"},
    {ACH_MILLIONAIRE,   "Millionaire",     "Score 100,000 points"},
    {ACH_SURVIVOR,      "Survivor",        "Survive 5 minutes"},
    {ACH_MAGNETIC,      "Magnetic",        "Collect 10 magnets"},
    {ACH_SPEED_DEMON,   "Speed Demon",     "Reach max speed (900 km/h)"},
};

AchievementSystem::AchievementSystem() = default;

const std::vector<AchievementDef>& AchievementSystem::getDefinitions() {
    return s_definitions;
}

void AchievementSystem::init(sf::Font& font) {
    toastBg_.setSize(sf::Vector2f(320.0f, 60.0f));
    toastBg_.setFillColor(sf::Color(0, 0, 0, 200));
    toastBg_.setOutlineColor(sf::Color(255, 215, 0, 200));
    toastBg_.setOutlineThickness(1.5f);

    toastNameText_.setFont(font);
    toastNameText_.setCharacterSize(16);
    toastNameText_.setStyle(sf::Text::Bold);
    toastNameText_.setFillColor(sf::Color(255, 215, 0));

    toastDescText_.setFont(font);
    toastDescText_.setCharacterSize(13);
    toastDescText_.setFillColor(sf::Color(200, 200, 200));
}

void AchievementSystem::update(float deltaTime) {
    for (auto it = toasts_.begin(); it != toasts_.end(); ) {
        it->lifetime += deltaTime;

        // Slide in from right
        float progress = std::min(1.0f, it->lifetime / 0.4f);
        it->slideOffset = (1.0f - progress) * 400.0f;

        // Fade out at end
        if (it->lifetime > it->maxLifetime - 0.5f) {
            float fadeProgress = (it->lifetime - (it->maxLifetime - 0.5f)) / 0.5f;
            float alpha = 255.0f * (1.0f - fadeProgress);
            sf::Color bgColor = it->bg.getFillColor();
            bgColor.a = static_cast<sf::Uint8>(alpha * 0.8f);
            it->bg.setFillColor(bgColor);

            sf::Color nameColor = it->nameText.getFillColor();
            nameColor.a = static_cast<sf::Uint8>(alpha);
            it->nameText.setFillColor(nameColor);

            sf::Color descColor = it->descText.getFillColor();
            descColor.a = static_cast<sf::Uint8>(alpha);
            it->descText.setFillColor(descColor);
        }

        if (it->lifetime >= it->maxLifetime) {
            it = toasts_.erase(it);
        } else {
            ++it;
        }
    }
}

void AchievementSystem::render(sf::RenderWindow& window) {
    // Calculate vertical position from bottom-up
    float startY = Constants::WINDOW_HEIGHT - 80.0f;

    for (size_t i = 0; i < toasts_.size(); ++i) {
        auto& toast = toasts_[i];
        float x = Constants::WINDOW_WIDTH - toast.bg.getSize().x - 20.0f + toast.slideOffset;
        float y = startY - static_cast<float>(i) * 70.0f;

        toast.bg.setPosition(x, y);
        window.draw(toast.bg);

        toast.nameText.setPosition(x + 15.0f, y + 6.0f);
        window.draw(toast.nameText);

        toast.descText.setPosition(x + 15.0f, y + 30.0f);
        window.draw(toast.descText);
    }
}

bool AchievementSystem::checkAchievements(const GameState& state) {
    bool newUnlock = false;

    auto tryUnlock = [&](AchievementDef def) -> bool {
        if (unlockedFlags_ & def.id) return false;
        unlockedFlags_ |= def.id;
        spawnToast(def);
        // Save immediately on unlock
        saveToFile();
        // Play sound via callback
        if (onAchievementUnlock) {
            onAchievementUnlock();
        }
        return true;
    };

    // Check each achievement condition
    if (state.enemiesDestroyed >= 1) {
        newUnlock |= tryUnlock({ACH_FIRST_BLOOD, "First Blood", "Destroy your first enemy"});
    }
    if (state.totalCoinsCollected >= 100) {
        newUnlock |= tryUnlock({ACH_RICH, "Rich!", "Collect 100 coins"});
    }
    if (state.maxCombo >= 10) {
        newUnlock |= tryUnlock({ACH_UNSTOPPABLE, "Unstoppable", "Reach a 10× combo"});
    }
    if (state.distance >= 1000.0f) {
        newUnlock |= tryUnlock({ACH_MARATHON, "Marathon", "Run 1000 meters"});
    }
    if (state.powerUpsCollected >= 10) {
        newUnlock |= tryUnlock({ACH_SHIELD_MASTER, "Power Collector", "Collect 10 power-ups"});
    }
    if (state.maxCombo >= 20) {
        newUnlock |= tryUnlock({ACH_COMBO_KING, "Combo King", "Reach a 20× combo"});
    }
    if (state.score >= 100000.0f) {
        newUnlock |= tryUnlock({ACH_MILLIONAIRE, "Millionaire", "Score 100,000 points"});
    }
    if (state.gameTime >= 300.0f) {
        newUnlock |= tryUnlock({ACH_SURVIVOR, "Survivor", "Survive 5 minutes"});
    }
    if (state.powerUpsCollected >= 20) {
        newUnlock |= tryUnlock({ACH_MAGNETIC, "Power Magnet", "Collect 20 power-ups"});
    }
    if (state.currentSpeed >= Constants::MAX_SPEED) {
        newUnlock |= tryUnlock({ACH_SPEED_DEMON, "Speed Demon", "Reach max speed (900 km/h)"});
    }

    return newUnlock;
}

void AchievementSystem::saveToFile(const std::string& path) {
    std::ofstream file(path, std::ios::binary);
    if (file) {
        file.write(reinterpret_cast<const char*>(&unlockedFlags_), sizeof(unlockedFlags_));
        file.close();
    }
}

uint32_t AchievementSystem::loadFromFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (file) {
        uint32_t flags = 0;
        file.read(reinterpret_cast<char*>(&flags), sizeof(flags));
        if (file.gcount() == sizeof(flags)) {
            unlockedFlags_ = flags;
            file.close();
            // Count bits portably
            int count = 0;
            uint32_t temp = flags;
            while (temp) { count += temp & 1; temp >>= 1; }
            std::cout << "[Achievements] Loaded " << count << " achievements from " << path << std::endl;
            return flags;
        }
        file.close();
    }
    return 0;
}

void AchievementSystem::spawnToast(const AchievementDef& def) {
    ToastNotification toast;
    toast.name = def.name;
    toast.description = def.description;

    // Copy visual elements
    toast.bg = toastBg_;
    toast.nameText = toastNameText_;
    toast.descText = toastDescText_;

    toast.nameText.setString("🏆 " + std::string(def.name));
    toast.descText.setString(def.description);

    toasts_.push_back(std::move(toast));
}
