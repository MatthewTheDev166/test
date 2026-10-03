#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../stats/StatsManager.hpp"

using namespace geode::prelude;

#include <Geode/modify/GJBaseGameLayer.hpp>

class $modify(MobileStatsBaseGameLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        GJBaseGameLayer::handleButton(down, button, isPlayer1);

        // Track clicks when pushed down for player 1 in gameplay
        if (down && isPlayer1 && PlayLayer::get()) {
            StatsManager::get().registerClick();
        }
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

    void update(float dt) {
        PlayLayer::update(dt);

        float percent = 0.0f;

        if (m_hasCompletedLevel) {
            percent = 100.0f;
        } else {
            // 1. Native getCurrentPercent()
            percent = this->getCurrentPercent();

            // Detect 0.0 to 1.0 fraction
            if (percent > 0.0f && percent <= 1.0f && m_player1 && m_player1->getPositionX() > 300.0f) {
                percent *= 100.0f;
            }

            // 2. Read in-game percentage label if active
            if (m_percentageLabel && m_percentageLabel->getString()) {
                std::string_view labelStr = m_percentageLabel->getString();
                auto pctPos = labelStr.find('%');
                if (pctPos != std::string_view::npos) {
                    try {
                        float parsed = std::stof(std::string(labelStr.substr(0, pctPos)));
                        if (parsed > percent) {
                            percent = parsed;
                        }
                    } catch (...) {}
                }
            }

            // 3. Fallback: player X / level length
            if (percent <= 0.0f && m_player1 && !m_isPlatformer) {
                float len = m_levelLength;
                if (len <= 0.0f && m_endPortal) {
                    len = m_endPortal->getPositionX();
                }
                if (len > 0.0f) {
                    float curX = m_player1->getPositionX();
                    percent = std::clamp((curX / len) * 100.0f, 0.0f, 100.0f);
                }
            }
        }

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
