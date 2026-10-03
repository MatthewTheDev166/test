#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../stats/StatsManager.hpp"

using namespace geode::prelude;

class $modify(MobileStatsPlayLayer, PlayLayer) {
    struct Fields {
        bool m_initialized = false;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        m_fields->m_initialized = false;
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }

        StatsManager::get().onEnterLevel(level, m_isPracticeMode);
        m_fields->m_initialized = true;
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);

        float percent = this->getCurrentPercent();
        StatsManager::get().onUpdateLevel(percent, dt);
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        if (m_fields->m_initialized) {
            StatsManager::get().onResetRun();
        }
    }

    void delayedResetLevel() {
        PlayLayer::delayedResetLevel();
        if (m_fields->m_initialized) {
            StatsManager::get().onResetRun();
        }
    }

    void loadFromCheckpoint(CheckpointObject* checkpoint) {
        PlayLayer::loadFromCheckpoint(checkpoint);
        if (m_fields->m_initialized) {
            StatsManager::get().onResumeRun();
        }
    }

    void resume() {
        PlayLayer::resume();
        StatsManager::get().onResume();
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        PlayLayer::destroyPlayer(player, object);
        if (m_fields->m_initialized && (player == m_player1 || player == m_player2)) {
            StatsManager::get().onDeath();
        }
    }

    void levelComplete() {
        PlayLayer::levelComplete();
        StatsManager::get().onComplete();
    }

    void onQuit() {
        m_fields->m_initialized = false;
        PlayLayer::onQuit();
        StatsManager::get().onMenu();
    }
};
