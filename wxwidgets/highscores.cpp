#include "highscores.h"
#include <fstream>
#include <algorithm>
#include <cstdlib>

#ifdef _WIN32
    #include <direct.h>
    #define MKDIR(dir) _mkdir(dir)
    #define PATH_SEP "\\"
#else
    #include <sys/stat.h>
    #include <sys/types.h>
    #define MKDIR(dir) mkdir(dir, 0700)
    #define PATH_SEP "/"
#endif

Highscores::Highscores() {
#ifdef _WIN32
    const char* home = getenv("APPDATA");
#else
    const char* home = getenv("HOME");
#endif
    if (!home) home = ".";

    std::string dirPath = std::string(home) + PATH_SEP + ".sudoku_solver";
    MKDIR(dirPath.c_str());
    scorePath = dirPath + PATH_SEP + "scores.txt";
    loadScores();
}

void Highscores::addScore(const Score& score) {
    auto& diffScores = scoresByDifficulty[score.difficulty];
    diffScores.push_back(score);

    std::sort(diffScores.begin(), diffScores.end(),
        [](const Score& a, const Score& b) { return a.time < b.time; });

    if (diffScores.size() > MAX_SCORES_PER_DIFFICULTY)
        diffScores.resize(MAX_SCORES_PER_DIFFICULTY);

    // Rebuild main scores vector
    scores.clear();
    for (const auto& pair : scoresByDifficulty)
        scores.insert(scores.end(), pair.second.begin(), pair.second.end());

    saveScores();
}

const std::vector<Score>& Highscores::getScores() const {
    return scores;
}

std::vector<Score> Highscores::getScoresByDifficulty(const std::string& difficulty) const {
    auto it = scoresByDifficulty.find(difficulty);
    if (it != scoresByDifficulty.end()) return it->second;
    return {};
}

bool Highscores::isHighScore(int time, const std::string& difficulty) const {
    auto it = scoresByDifficulty.find(difficulty);
    if (it == scoresByDifficulty.end()) return true;
    const auto& ds = it->second;
    if (ds.size() < MAX_SCORES_PER_DIFFICULTY) return true;
    return time < ds.back().time;
}

void Highscores::loadScores() {
    std::ifstream file(scorePath);
    if (!file) return;

    scores.clear();
    scoresByDifficulty.clear();

    std::string line;
    while (std::getline(file, line)) {
        size_t p1 = line.find('|');
        size_t p2 = line.find('|', p1 + 1);
        if (p1 == std::string::npos || p2 == std::string::npos) continue;
        Score score;
        score.name       = line.substr(0, p1);
        score.time       = std::stoi(line.substr(p1 + 1, p2 - p1 - 1));
        score.difficulty = line.substr(p2 + 1);
        scores.push_back(score);
        scoresByDifficulty[score.difficulty].push_back(score);
    }

    for (auto& pair : scoresByDifficulty) {
        auto& ds = pair.second;
        std::sort(ds.begin(), ds.end(),
            [](const Score& a, const Score& b) { return a.time < b.time; });
        if (ds.size() > MAX_SCORES_PER_DIFFICULTY)
            ds.resize(MAX_SCORES_PER_DIFFICULTY);
    }

    scores.clear();
    for (const auto& pair : scoresByDifficulty)
        scores.insert(scores.end(), pair.second.begin(), pair.second.end());
}

void Highscores::saveScores() {
    std::ofstream file(scorePath);
    if (!file) return;
    for (const auto& score : scores)
        file << score.name << '|' << score.time << '|' << score.difficulty << '\n';
}
