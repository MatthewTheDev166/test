#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include "../stats/StatsManager.hpp"
#include "../ui/ConnectionBadge.hpp"

using namespace geode::prelude;

class $modify(MobileStatsPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        StatsManager::get().onPause();

        auto badge = ConnectionBadge::createButton(this);
        if (auto menu = this->getChildByID("right-button-menu")) {
            menu->addChild(badge);
            menu->updateLayout();
        } else if (auto bottomMenu = this->getChildByID("bottom-button-menu")) {
            bottomMenu->addChild(badge);
            bottomMenu->updateLayout();
        }
    }

    void onResume(CCObject* sender) {
        PauseLayer::onResume(sender);
        StatsManager::get().onResume();
    }
};
