#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include "../stats/StatsManager.hpp"
#include <cmath>
#include <algorithm>

using namespace geode::prelude;

class $modify(MobileStatsPlayerObject, PlayerObject) {
    bool pushButton(PlayerButton button) {
        bool res = PlayerObject::pushButton(button);

        // Track player 1 jump clicks reliably across keyboard, mouse, touch, and controller
        if (button == PlayerButton::Jump) {
            if (auto pl = PlayLayer::get()) {
                if (this == pl->m_player1) {
                    StatsManager::get().registerClick();
                }
            }
        }

        return res;
    }
};

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

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        float percent = 0.0f;
        if (m_hasCompletedLevel) {
            percent = 100.0f;
        } else if (m_isPlatformer) {
            percent = 0.0f;
        } else {
            percent = this->getCurrentPercent();
            if (std::isnan(percent) || std::isinf(percent)) {
                percent = 0.0f;
            }
            percent = std::clamp(percent, 0.0f, 100.0f);
        }

        StatsManager::get().onUpdateLevel(percent, dt);
    }

    void resetLevel() {
        PlayLayer::resetLevel();
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
        // Guard against false death if level has already been completed
        if (m_fields->m_initialized && !m_hasCompletedLevel && (player == m_player1 || player == m_player2)) {
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
