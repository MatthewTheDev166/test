#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../stats/StatsManager.hpp"

using namespace geode::prelude;

class $modify(MobileStatsPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }

        StatsManager::get().onEnterLevel(level, m_isPracticeMode);
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);

        float percent = this->getCurrentPercent();
        bool isDead = (m_player1 != nullptr && m_player1->m_isDead);
        StatsManager::get().onUpdateLevel(percent, dt, isDead);
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        StatsManager::get().onResetRun();
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        PlayLayer::destroyPlayer(player, object);
        if (player == m_player1) {
            StatsManager::get().onDeath();
        }
    }

    void levelComplete() {
        PlayLayer::levelComplete();
        StatsManager::get().onComplete();
    }

    void onQuit() {
        PlayLayer::onQuit();
        StatsManager::get().onMenu();
    }
};
