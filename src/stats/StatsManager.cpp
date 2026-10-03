#include "StatsManager.hpp"
#include "../server/WebSocketServer.hpp"
#include "../utils/InputSimulator.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

StatsManager& StatsManager::get() {
    static StatsManager instance;
    return instance;
}

StatsManager::StatsManager() = default;

void StatsManager::onMenu() {
    m_state = "in_menu";
    m_levelName = "";
    m_creatorName = "";
    m_levelID = 0;
    m_currentPercent = 0.0f;
    m_sessionTime = 0.0f;
    broadcastCurrentState();
}

void StatsManager::onEnterLevel(GJGameLevel* level, bool isPractice) {
    m_state = "playing";
    if (level) {
        m_levelName = level->m_levelName;
        m_creatorName = level->m_creatorName;
        if (m_creatorName.empty()) {
            m_creatorName = "RobTop";
        }
        m_levelID = level->m_levelID;
        m_bestPercent = level->m_normalPercent;
        m_totalAttempts = level->m_attempts;
    }
    m_isPractice = isPractice;
    m_currentPercent = 0.0f;
    m_sessionTime = 0.0f;
    m_lastBroadcastTimer = 0.0f;

    // Reset attempt CPS stats
    {
        std::lock_guard<std::mutex> lock(m_cpsMutex);
        m_attemptClicks = 0;
        m_currentCPS = 0;
        m_peakCPS = 0;
        m_clickTimestamps.clear();
    }

    broadcastCurrentState();
}

void StatsManager::onUpdateLevel(float percent, float dt) {
    if (m_state == "level_completed") {
        m_currentPercent = 100.0f;
    } else {
        m_currentPercent = percent;
    }

    // Only count gameplay time while actively playing (not paused)
    if (m_state == "playing") {
        m_sessionTime += dt;
    }

    m_lastBroadcastTimer += dt;

    // Decay current CPS over sliding 1-second window
    {
        std::lock_guard<std::mutex> lock(m_cpsMutex);
        auto now = std::chrono::steady_clock::now();
        while (!m_clickTimestamps.empty() && 
               std::chrono::duration<float>(now - m_clickTimestamps.front()).count() > 1.0f) {
            m_clickTimestamps.pop_front();
        }
        m_currentCPS = static_cast<int>(m_clickTimestamps.size());
    }

    // Throttle live stream updates to ~20 Hz (every 0.05s) to preserve network & CPU
    if (m_lastBroadcastTimer >= 0.05f) {
        m_lastBroadcastTimer = 0.0f;
        broadcastCurrentState();
    }
}

void StatsManager::onResetRun() {
    m_state = "playing";
    m_totalAttempts++;
    m_currentPercent = 0.0f;

    // Reset CPS for new attempt
    {
        std::lock_guard<std::mutex> lock(m_cpsMutex);
        m_attemptClicks = 0;
        m_currentCPS = 0;
        m_peakCPS = 0;
        m_clickTimestamps.clear();
    }

    InputSimulator::triggerJumpUp();
    broadcastCurrentState();
}

void StatsManager::onResumeRun() {
    m_state = "playing";
    broadcastCurrentState();
}

void StatsManager::onDeath() {
    if (m_state == "in_menu" || m_state == "level_completed") return;
    m_state = "dead";
    m_currentCPS = 0;
    InputSimulator::triggerJumpUp();
    broadcastCurrentState();
}

void StatsManager::onPause() {
    if (m_state == "in_menu") return;
    m_state = "paused";
    InputSimulator::triggerJumpUp();
    broadcastCurrentState();
}

void StatsManager::onResume() {
    if (m_state == "in_menu") return;
    m_state = "playing";
    broadcastCurrentState();
}

void StatsManager::onComplete() {
    m_state = "level_completed";
    m_currentPercent = 100.0f;
    InputSimulator::triggerJumpUp();
    broadcastCurrentState();
}

void StatsManager::registerClick() {
    std::lock_guard<std::mutex> lock(m_cpsMutex);
    auto now = std::chrono::steady_clock::now();
    m_clickTimestamps.push_back(now);
    m_attemptClicks++;

    while (!m_clickTimestamps.empty() && 
           std::chrono::duration<float>(now - m_clickTimestamps.front()).count() > 1.0f) {
        m_clickTimestamps.pop_front();
    }

    m_currentCPS = static_cast<int>(m_clickTimestamps.size());
    if (m_currentCPS > m_peakCPS) {
        m_peakCPS = m_currentCPS;
    }
}

void StatsManager::broadcastCurrentState() {
    matjson::Value json;
    json["type"] = "stats_update";
    json["state"] = m_state;

    matjson::Value lvl;
    lvl["name"] = m_levelName;
    lvl["creator"] = m_creatorName;
    lvl["id"] = m_levelID;
    lvl["is_practice"] = m_isPractice;
    json["level"] = lvl;

    matjson::Value prog;
    prog["current_percent"] = m_currentPercent;
    prog["best_percent"] = m_bestPercent;
    json["progress"] = prog;

    matjson::Value att;
    att["total"] = m_totalAttempts;
    json["attempts"] = att;

    matjson::Value t;
    t["session_seconds"] = m_sessionTime;
    json["time"] = t;

    matjson::Value cps;
    cps["current"] = m_currentCPS;
    cps["peak"] = m_peakCPS;
    cps["total_clicks"] = m_attemptClicks;
    json["cps"] = cps;

    WebSocketServer::get().broadcast(json.dump());
}

void StatsManager::handleClientMessage(const std::string& message) {
    auto parsed = matjson::parse(message);
    if (!parsed.isOk()) {
        return;
    }

    auto json = parsed.unwrap();
    if (!json.contains("action") || !json["action"].isString()) {
        return;
    }

    std::string action = json["action"].asString().unwrapOr("");
    bool simulateKeys = Mod::get()->getSettingValue<bool>("simulate-keys");

    if (action == "jump_down" || action == "jump") {
        geode::queueInMainThread([]() {
            InputSimulator::triggerJumpDown();
        });
    } else if (action == "jump_up") {
        geode::queueInMainThread([]() {
            InputSimulator::triggerJumpUp();
        });
    } else if (action == "prev_startpos" || action == "q") {
        geode::queueInMainThread([simulateKeys]() {
            if (simulateKeys) {
                InputSimulator::triggerPrevStartPos();
            }
        });
    } else if (action == "next_startpos" || action == "e") {
        geode::queueInMainThread([simulateKeys]() {
            if (simulateKeys) {
                InputSimulator::triggerNextStartPos();
            }
        });
    } else if (action == "respawn" || action == "r") {
        geode::queueInMainThread([simulateKeys]() {
            if (simulateKeys) {
                InputSimulator::triggerRespawn();
            } else if (auto pl = PlayLayer::get()) {
                pl->resetLevel();
            }
        });
    } else if (action == "request_sync") {
        broadcastCurrentState();
    }
}
