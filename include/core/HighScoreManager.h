#ifndef CORE_HIGHSCOREMANAGER_H
#define CORE_HIGHSCOREMANAGER_H

#include <string>
#include <vector>
#include <cstdint>
#include "Constants.h"

struct HighScoreEntry {
    int score = 0;
    int enemiesDestroyed = 0;
    int maxCombo = 0;
    int powerUpsCollected = 0;
    float distance = 0.0f;
    float gameTime = 0.0f;
    bool hardMode = false;
};

class HighScoreManager {
public:
    static constexpr int MAX_ENTRIES_PER_LEVEL = 10;

    HighScoreManager();

    /// Load scores from file
    void loadFromFile(const std::string& filename);

    /// Save scores to file
    void saveToFile(const std::string& filename) const;

    /// Try to insert a new score. Returns true if it's a new high score (top 10).
    bool submitScore(Constants::LevelType level, const HighScoreEntry& entry);

    /// Get top scores for a specific level
    const std::vector<HighScoreEntry>& getScores(Constants::LevelType level) const;

    /// Get the highest score for a specific level (returns 0 if none)
    int getHighScore(Constants::LevelType level, bool hardMode) const;

    /// Get all scores across all levels (for the main menu display)
    int getOverallHighScore() const;

    /// Clear all scores
    void clear();

private:
    std::vector<HighScoreEntry> scores_[5]; // one per LevelType (0-4)

    static int levelIndex(Constants::LevelType level);

    /// Serialize/deserialize helpers
    static std::string serialize(const HighScoreEntry& e);
    static HighScoreEntry deserialize(const std::string& line);
};

#endif
