#pragma once
#include <string>
#include <cstdint>
#include <chrono>
#include <deque>
#include <mutex>
#include <Geode/Geode.hpp>

class StatsManager {
public:
    static StatsManager& get();

    void onMenu();
    void onEnterLevel(GJGameLevel* level, bool isPractice);
    void onUpdateLevel(float percent, float dt);
    void onResetRun();
    void onResumeRun();
    void onDeath();
    void onPause();
    void onResume();
    void onComplete();

    void registerClick();

    void broadcastCurrentState();
    void handleClientMessage(const std::string& message);

    std::string getState() const { return m_state; }
    std::string getLevelName() const { return m_levelName; }
    std::string getCreatorName() const { return m_creatorName; }
    float getCurrentPercent() const { return m_currentPercent; }
    int getBestPercent() const { return m_bestPercent; }
    int getTotalAttempts() const { return m_totalAttempts; }
    int getSessionAttempts() const { return m_sessionAttempts; }
    float getSessionTime() const { return m_sessionTime; }
    bool isPractice() const { return m_isPractice; }

    int getAttemptClicks() const { return m_attemptClicks; }
    int getCurrentCPS() const { return m_currentCPS; }
    int getPeakCPS() const { return m_peakCPS; }

private:
    StatsManager();
    ~StatsManager() = default;

    std::string m_state{"in_menu"};
    std::string m_levelName{""};
    std::string m_creatorName{""};
    int m_levelID{0};
    bool m_isPractice{false};

    float m_currentPercent{0.0f};
    int m_bestPercent{0};
    int m_totalAttempts{0};
    int m_sessionAttempts{0};
    float m_sessionTime{0.0f};

    // CPS & Spam Tracking (for current attempt)
    int m_attemptClicks{0};
    int m_currentCPS{0};
    int m_peakCPS{0};
    std::deque<std::chrono::steady_clock::time_point> m_clickTimestamps;
    std::mutex m_cpsMutex;

    float m_lastBroadcastTimer{0.0f};
};
