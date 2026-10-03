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
        m_levelID = level->m_levelID;
        m_bestPercent = level->m_normalPercent;
        m_totalAttempts = level->m_attempts;
    }
    m_isPractice = isPractice;
    m_sessionAttempts = 1;
    m_currentPercent = 0.0f;
    m_sessionTime = 0.0f;
    m_lastBroadcastTimer = 0.0f;
    broadcastCurrentState();
}

void StatsManager::onUpdateLevel(float percent, float dt) {
    m_currentPercent = percent;
    m_sessionTime += dt;
    m_lastBroadcastTimer += dt;

    // Throttle live stream updates to ~20 Hz (every 0.05s) to preserve network & CPU
    if (m_lastBroadcastTimer >= 0.05f) {
        m_lastBroadcastTimer = 0.0f;
        broadcastCurrentState();
    }
}

void StatsManager::onResetRun() {
    m_sessionAttempts++;
    m_totalAttempts++;
    m_currentPercent = 0.0f;
    broadcastCurrentState();
}

void StatsManager::onDeath() {
    m_state = "dead";
    broadcastCurrentState();
}

void StatsManager::onPause() {
    m_state = "paused";
    broadcastCurrentState();
}

void StatsManager::onResume() {
    m_state = "playing";
    broadcastCurrentState();
}

void StatsManager::onComplete() {
    m_state = "level_completed";
    m_currentPercent = 100.0f;
    broadcastCurrentState();
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
    att["session"] = m_sessionAttempts;
    json["attempts"] = att;

    matjson::Value t;
    t["session_seconds"] = m_sessionTime;
    json["time"] = t;

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

    if (action == "prev_startpos" || action == "q") {
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
