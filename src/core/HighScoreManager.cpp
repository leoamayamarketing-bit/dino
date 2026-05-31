#include "../../include/core/HighScoreManager.h"
#include <algorithm>
#include <fstream>
#include <sstream>

HighScoreManager::HighScoreManager() = default;

int HighScoreManager::levelIndex(Constants::LevelType level) {
    switch (level) {
        case Constants::LevelType::DESERT_DAY:    return 0;
        case Constants::LevelType::DESERT_NIGHT:  return 1;
        case Constants::LevelType::CAVE:          return 2;
        case Constants::LevelType::VOLCANO:       return 3;
        case Constants::LevelType::INFINITE:      return 4;
        default: return 0;
    }
}

std::string HighScoreManager::serialize(const HighScoreEntry& e) {
    std::ostringstream ss;
    ss << e.score << ","
       << e.enemiesDestroyed << ","
       << e.maxCombo << ","
       << e.powerUpsCollected << ","
       << static_cast<int>(e.distance) << ","
       << static_cast<int>(e.gameTime) << ","
       << (e.hardMode ? 1 : 0);
    return ss.str();
}

HighScoreEntry HighScoreManager::deserialize(const std::string& line) {
    HighScoreEntry e;
    std::istringstream ss(line);
    std::string token;
    int idx = 0;
    while (std::getline(ss, token, ',')) {
        try {
            switch (idx) {
                case 0: e.score = std::stoi(token); break;
                case 1: e.enemiesDestroyed = std::stoi(token); break;
                case 2: e.maxCombo = std::stoi(token); break;
                case 3: e.powerUpsCollected = std::stoi(token); break;
                case 4: e.distance = static_cast<float>(std::stoi(token)); break;
                case 5: e.gameTime = static_cast<float>(std::stoi(token)); break;
                case 6: e.hardMode = (std::stoi(token) != 0); break;
            }
        } catch (...) {
            // Malformed line — return default entry
            return HighScoreEntry();
        }
        idx++;
    }
    return e;
}

void HighScoreManager::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        // Format: LEVEL_INDEX|score,enemies,combo,powerups,distance,time,hardmode
        auto pipePos = line.find('|');
        if (pipePos == std::string::npos) continue;

        int lvlIdx = std::stoi(line.substr(0, pipePos));
        if (lvlIdx < 0 || lvlIdx >= 5) continue;

        std::string data = line.substr(pipePos + 1);
        auto entry = deserialize(data);
        scores_[lvlIdx].push_back(entry);
    }

    // Sort each level's scores descending
    for (auto& scores : scores_) {
        std::sort(scores.begin(), scores.end(),
            [](const HighScoreEntry& a, const HighScoreEntry& b) {
                return a.score > b.score;
            });
        if (static_cast<int>(scores.size()) > MAX_ENTRIES_PER_LEVEL) {
            scores.resize(MAX_ENTRIES_PER_LEVEL);
        }
    }
}

void HighScoreManager::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) return;

    for (int i = 0; i < 5; i++) {
        for (const auto& entry : scores_[i]) {
            file << i << "|" << serialize(entry) << "\n";
        }
    }
}

bool HighScoreManager::submitScore(Constants::LevelType level, const HighScoreEntry& entry) {
    int idx = levelIndex(level);
    auto& scores = scores_[idx];

    // Check if this score qualifies as a top score
    bool isNewHigh = false;
    if (static_cast<int>(scores.size()) < MAX_ENTRIES_PER_LEVEL) {
        isNewHigh = true;
    } else {
        // Check if it beats the lowest top score (scores are sorted descending)
        int lowest = scores.back().score;
        if (entry.score > lowest) {
            isNewHigh = true;
        }
    }

    if (!isNewHigh) return false;

    scores.push_back(entry);
    std::sort(scores.begin(), scores.end(),
        [](const HighScoreEntry& a, const HighScoreEntry& b) {
            return a.score > b.score;
        });
    if (static_cast<int>(scores.size()) > MAX_ENTRIES_PER_LEVEL) {
        scores.resize(MAX_ENTRIES_PER_LEVEL);
    }

    return true;
}

const std::vector<HighScoreEntry>& HighScoreManager::getScores(Constants::LevelType level) const {
    return scores_[levelIndex(level)];
}

int HighScoreManager::getHighScore(Constants::LevelType level, bool hardMode) const {
    const auto& scores = scores_[levelIndex(level)];
    for (const auto& entry : scores) {
        if (entry.hardMode == hardMode) {
            return entry.score;
        }
    }
    return 0;
}

int HighScoreManager::getOverallHighScore() const {
    int best = 0;
    for (int i = 0; i < 5; i++) {
        if (!scores_[i].empty()) {
            best = std::max(best, scores_[i][0].score);
        }
    }
    return best;
}

void HighScoreManager::clear() {
    for (auto& scores : scores_) {
        scores.clear();
    }
}
